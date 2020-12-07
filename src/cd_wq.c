/**
 * cd_wq.c - Workqueue implementation
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */

#include "cd_wq.h"


static void
cd_wq_call_sync_dtor(struct cd_work *work) {
	switch (work->task)
	{
		case RM_WORK_PROCESS_MSG_PUSH:
			work->f_dtor(work);                 /* must at least free the memory allocated for the message and work struct itself, may close the TCP socket, etc... */
			work = NULL;
			break;

		case RM_WORK_PROCESS_MSG_PULL:
		case RM_WORK_PROCESS_MSG_BYE:
		default:
			break;                              /* do not call destructor for these types synchronously */
	}
}

static void*
cd_wq_worker_f(void *arg) {
	cd_fifo_queue            *q;
	struct cd_work          *work;              /* iterator over enqueued work elements */
	struct cd_list_head      *lh;

	struct cd_worker *w = (struct cd_worker*) arg;

	pthread_mutex_lock(&w->mutex);              /* sleep on the queue and process queued work element once awoken */
	q = &w->queue;

	while (w->active == 1) {
		for (cd_fifo_dequeue(q, lh); lh != NULL; cd_fifo_dequeue(q, lh)) {
			work = cd_container_of(lh, struct cd_work, link);
			pthread_mutex_unlock(&w->mutex);    /* allow for further enquing while work is being processed */
			work->f(work);
			cd_wq_call_sync_dtor(work);         /* process destructors for synchronous jobs */
			pthread_mutex_lock(&w->mutex);
		}

		w->busy = 0;
		pthread_cond_wait(&w->signal, &w->mutex);
	}

	pthread_mutex_unlock(&w->mutex);
	return NULL;
}

static void
cd_wq_worker_init(struct cd_worker *w, struct cd_workqueue *wq) {
	memset(w, 0, sizeof(struct cd_worker));
	pthread_mutex_init(&w->mutex, NULL);
	w->active = 0;
	w->busy = 0;
	w->wq = wq;
	TWINIT_LIST_HEAD(&w->queue);
	pthread_cond_init(&w->signal, NULL);
}

static enum cd_error cd_wq_worker_deinit(struct cd_worker *w) {
	if (w->busy == 1) {
		RM_LOG_WARN("Skipping deinit of worker [%u], this worker is still busy", w->idx);
		return RM_ERR_BUSY;
	}

	assert(cd_list_empty(&w->queue) != 0 && "Queue NOT EMPTY!\n");
	if (cd_list_empty(&w->queue) == 0) {
		RM_LOG_CRIT("QUEUE NOT EMPTY, worker [%u]", w->idx);
		return RM_ERR_BUSY;
	}

	pthread_mutex_destroy(&w->mutex);
	pthread_cond_destroy(&w->signal);

	return RM_ERR_OK;
}

enum cd_error
cd_wq_workqueue_init(struct cd_workqueue *wq, uint32_t workers_n, const char *name) {
	struct cd_worker    *w = NULL;
	uint8_t             first_active_set = 0;

	memset(wq, 0, sizeof(struct cd_workqueue));
	wq->workers = malloc(workers_n * sizeof(struct cd_worker));
	if (wq->workers == NULL) {
		return RM_ERR_MEM;
	}
	wq->workers_n = workers_n;

	if (workers_n > 0) {
		wq->workers_active_n = 0;
		while (workers_n) {
			--workers_n;
			w = &wq->workers[workers_n];
			cd_wq_worker_init(w, wq);
			w->idx = workers_n;
			w->active = 1;
			if (cd_launch_thread(&w->tid, cd_wq_worker_f, w, PTHREAD_CREATE_JOINABLE) == RM_ERR_OK) {
				wq->workers_active_n++;																	/* increase the number of running workers */
				if (first_active_set == 0) {
					wq->first_active_worker_idx = w->idx;
					first_active_set = 1;
				}
			} else {
				w->active = 0;
			}
		}
	}

	wq->name = strdup(name);
	wq->running = 1;

	wq->next_worker_idx_to_use = (first_active_set == 1 ? wq->first_active_worker_idx : 0);
	if (wq->workers_active_n > 0) {																		/* if we have at least one worker thread then queue creation was successful */
		return RM_ERR_OK;
	} else {
		return RM_ERR_WORKQUEUE_CREATE;
	}
}

enum cd_error cd_wq_workqueue_deinit(struct cd_workqueue *wq) {
	struct cd_worker    *w = NULL;
	uint32_t            workers_n = wq->workers_n;

	free((void*)wq->name);
	while (workers_n) {
		--workers_n;
		w = &wq->workers[workers_n];
		if (cd_wq_worker_deinit(w) != RM_ERR_OK) {
			return RM_ERR_FAIL;
		}
	}
	free(wq->workers);
	return RM_ERR_OK;
}

void cd_wq_workqueue_free(struct cd_workqueue *wq) {                /* queue MUST be empty now */
	cd_wq_workqueue_deinit(wq);
	free(wq);
	return;
}

struct cd_workqueue*
cd_wq_workqueue_create(uint32_t workers_n, const char *name) {
	enum cd_error   err = RM_ERR_OK;
	struct cd_workqueue* wq;
	wq = malloc(sizeof(struct cd_workqueue));
	if (wq == NULL) {
		return NULL;
	}
	err = cd_wq_workqueue_init(wq, workers_n, name);
	if (err  != RM_ERR_OK) {
		switch (err) {

			case RM_ERR_MEM:
				free(wq);
				return NULL;

			case RM_ERR_WORKQUEUE_CREATE:
				cd_wq_workqueue_free(wq);
				return NULL;

			default:
				break;
		}
	}
	return wq;
}

enum cd_error cd_wq_workqueue_stop(struct cd_workqueue *wq) {
	struct cd_worker    *w = NULL;
	uint8_t             workers_n = 0;
	enum cd_error       err = RM_ERR_OK;

	workers_n = wq->workers_n;
	if ((workers_n > 0) && (wq->workers_active_n > 0)) {
		while (workers_n) {
			--workers_n;
			w = &wq->workers[workers_n];
			pthread_mutex_lock(&w->mutex);                                      /* lock worker thread */
			if (w->active == 1) {
				w->active = 0;                                                  /* tell the worker to stop */
				pthread_cond_signal(&w->signal);                                /* signal the worker */
				pthread_mutex_unlock(&w->mutex);                                /* let worker exit */
				wq->workers_active_n--;                                         /* decrease the number of active/running workers */
				if (pthread_join(w->tid, NULL) != RM_ERR_OK) {                  /* join worker thread */
					err = RM_ERR_FAIL;
				}
			} else {
				pthread_mutex_unlock(&w->mutex);
			}
		}
	}
	return err;
}

struct cd_work*
cd_work_init(struct cd_work* work, enum cd_work_type task, struct cd_msg* msg, int fd, void*(*f)(void*), void(*f_dtor)(void*)) {
	TWINIT_LIST_HEAD(&work->link);
	work->task = task;
	work->rm = rm;
	work->msg = msg;
	work->fd = fd;
	work->f = f;
	work->f_dtor = f_dtor;
	return work;
}

struct cd_work*
cd_work_create(enum cd_work_type task, struct cd_msg* msg, int fd, void*(*f)(void*), void(*f_dtor)(void*)) {
	struct cd_work* work = malloc(sizeof(struct cd_work));
	if (work == NULL)
		return NULL;

	return cd_work_init(work, task, rm, msg, fd, f, f_dtor);
}

void
cd_work_free(struct cd_work* work) {
	if (work->msg != NULL)
		free(work->msg);

	free(work);
}

/*  @brief  Work dispatcher (round-robin). */
void
cd_wq_queue_work(struct cd_workqueue *wq, struct cd_work* work) {
	struct cd_worker    *w = NULL;
	uint8_t             idx = wq->next_worker_idx_to_use, sanity = 0xFF;

	assert(wq->workers_active_n > 0 && "NO ACTIVE THREAD in the workqueue");

	if (wq->workers_active_n == 0) {
		RM_LOG_CRIT("NO ACTIVE WORKER THREAD in the workqueue [%s]", wq->name);
		return;
	}

	if (wq->workers_active_n > 1) {													/* get next worker */
		do {
			w = &wq->workers[idx];
			idx = (idx + 1) % wq->workers_n;
		} while (w->active == 0 && (--sanity));
	} else {																		/* there is only one active thread in the workers table */
		w = &wq->workers[wq->first_active_worker_idx];
	}

	work->worker_idx = w->idx;														/* save the worker's index into work */
	wq->next_worker_idx_to_use = idx;

	pthread_mutex_lock(&w->mutex);													/* enqueue work (and move ownership to worker!) */
	cd_fifo_enqueue(&work->link, &w->queue);
	w->busy = 1;
	pthread_cond_signal(&w->signal);
	pthread_mutex_unlock(&w->mutex);

	return;
}

/* TODO */
void
cd_wq_queue_delayed_work(struct cd_workqueue *q, struct cd_work* work, unsigned int delay) {
	(void)q;
	(void)work;
	(void)delay;
	return;
}
