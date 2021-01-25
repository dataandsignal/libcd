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

#include "../include/cd_wq.h"
#include "../include/cd_log.h"


static void cd_wq_free_if_sync(struct cd_work *work)
{
	switch (work->type)
	{
		case CD_WORK_SYNC:
			if (work->f_dtor)
				work->f_dtor(work->user_data);	/* call destructor in sync with job processing */
			free(work);
			work = NULL;
			break;

		case CD_WORK_ASYNC:
		default:
			break;                              /* do not call destructor, do not free, user will handle this later */
	}
}

static void* cd_wq_worker_f(void *arg)
{
	cd_fifo_queue            *q;
	struct cd_work          *work;              /* iterator over enqueued work elements */
	struct cd_list_head      *lh;

	struct cd_worker *w = (struct cd_worker*) arg;

	pthread_mutex_lock(&w->mutex);              /* sleep on the queue and process queued work element once awoken */
	q = &w->queue;

	while (w->active || ((w->options.CD_WQ_QUEUE_OPTION_STOP == CD_WQ_QUEUE_OPTION_STOP_SOFT) && !cd_fifo_empty(q))) {

		cd_fifo_dequeue(q, lh);

		if (lh) {
			work = cd_container_of(lh, struct cd_work, link);
			pthread_mutex_unlock(&w->mutex);    /* allow for further enquing while work is being processed */

			work->f(work->user_data);
			cd_wq_free_if_sync(work);

			pthread_mutex_lock(&w->mutex);
		}

		// do not exit:
		// if is still active (regardless of CD_WQ_QUEUE_STOP_ type)
		// if not active but it's CD_WQ_QUEUE_STOP_SOFT and queue is not empty (should continue to do work)
		//
		// Therefore to exit queue processing:
		// if it's CD_WQ_QUEUE_STOP_HARD - set active to 0 (terminate processing, ignoring waiting work if any)
		// if it's CD_WQ_QUEUE_STOP_SOFT - set active to 0 and wait till all work has been processed
		//if (w->active || ((w->options.CD_WQ_QUEUE_OPTION_STOP == CD_WQ_QUEUE_OPTION_STOP_SOFT) && !cd_fifo_empty(q))) {
		//	pthread_cond_wait(&w->signal, &w->mutex);
		//}

		if (!w->active) {

			if ((w->options.CD_WQ_QUEUE_OPTION_STOP == CD_WQ_QUEUE_OPTION_STOP_HARD) || 
					((w->options.CD_WQ_QUEUE_OPTION_STOP == CD_WQ_QUEUE_OPTION_STOP_SOFT) && cd_fifo_empty(q))) {
				goto exit;
			}

		} else {

			if (cd_fifo_empty(q))
				pthread_cond_wait(&w->signal, &w->mutex);
		}
	}

exit:
	pthread_mutex_unlock(&w->mutex);
	return NULL;
}

static void cd_wq_worker_init(struct cd_worker *w, struct cd_workqueue *wq)
{
	memset(w, 0, sizeof(struct cd_worker));
	pthread_mutex_init(&w->mutex, NULL);
	w->active = 0;
	w->wq = wq;
	w->options = wq->options;
	CD_INIT_LIST_HEAD(&w->queue);
	pthread_cond_init(&w->signal, NULL);
}

static enum cd_error cd_wq_worker_deinit(struct cd_worker *w)
{
	if (w->options.CD_WQ_QUEUE_OPTION_STOP == CD_WQ_QUEUE_OPTION_STOP_SOFT) {
		assert(cd_list_empty(&w->queue) != 0 && "Queue NOT EMPTY! Worker terminating processing of not empty queue...\n");
	}

	if (!cd_list_empty(&w->queue)) {
		CD_LOG_CRIT("Warning, worker [%u] terminating processing of not empty queue...", w->idx);
		return CD_ERR_BUSY;
	}

	pthread_mutex_destroy(&w->mutex);
	pthread_cond_destroy(&w->signal);

	return CD_ERR_OK;
}

enum cd_error cd_wq_workqueue_init(struct cd_workqueue *wq, uint32_t workers_n, const char *name, uint8_t option_stop)
{
	struct cd_worker    *w = NULL;

	memset(wq, 0, sizeof(struct cd_workqueue));
	wq->workers = malloc(workers_n * sizeof(struct cd_worker));
	if (wq->workers == NULL) {
		return CD_ERR_MEM;
	}
	wq->workers_n = workers_n;

	wq->options.CD_WQ_QUEUE_OPTION_STOP = option_stop;

	if (workers_n > 0) {
		wq->workers_active_n = 0;
		while (workers_n) {
			--workers_n;
			w = &wq->workers[workers_n];
			cd_wq_worker_init(w, wq);
			w->idx = workers_n;
			w->active = 1;
			if (cd_launch_thread(&w->tid, cd_wq_worker_f, w, PTHREAD_CREATE_JOINABLE) == CD_ERR_OK) {
				wq->workers_active_n++;																	/* increase the number of running workers */
				if (wq->first_active_worker_idx == 0)
					wq->first_active_worker_idx = w->idx;
			} else {
				w->active = 0;
			}
		}
	}

	wq->name = strdup(name);
	wq->running = 1;

	wq->next_worker_idx_to_use = wq->first_active_worker_idx;
	if (wq->workers_active_n > 0) {																		/* if we have at least one worker thread then queue creation was successful */
		return CD_ERR_OK;
	} else {
		return CD_ERR_WORKQUEUE_CREATE;
	}
}

enum cd_error cd_wq_workqueue_default_init(struct cd_workqueue *wq, uint32_t workers_n, const char *name)
{
	return cd_wq_workqueue_init(wq, workers_n, name, CD_WQ_QUEUE_OPTION_STOP_SOFT);
}

enum cd_error cd_wq_workqueue_deinit(struct cd_workqueue *wq)
{
	struct cd_worker    *w = NULL;
	uint32_t            workers_n = wq->workers_n;

	free((void*)wq->name);
	while (workers_n) {
		--workers_n;
		w = &wq->workers[workers_n];
		if (cd_wq_worker_deinit(w) != CD_ERR_OK) {
			return CD_ERR_FAIL;
		}
	}
	free(wq->workers);
	return CD_ERR_OK;
}

void cd_wq_workqueue_free(struct cd_workqueue *wq)
{                /* queue MUST be empty now */
	cd_wq_workqueue_deinit(wq);
	free(wq);
	return;
}

struct cd_workqueue* cd_wq_workqueue_create(uint32_t workers_n, const char *name, uint8_t option_stop)
{
	enum cd_error   err = CD_ERR_OK;
	struct cd_workqueue* wq;
	wq = malloc(sizeof(struct cd_workqueue));
	if (wq == NULL) {
		return NULL;
	}
	err = cd_wq_workqueue_init(wq, workers_n, name, option_stop);
	if (err  != CD_ERR_OK) {
		switch (err) {

			case CD_ERR_MEM:
				free(wq);
				return NULL;

			case CD_ERR_WORKQUEUE_CREATE:
				cd_wq_workqueue_free(wq);
				return NULL;

			default:
				break;
		}
	}
	return wq;
}

struct cd_workqueue* cd_wq_workqueue_default_create(uint32_t workers_n, const char *name)
{
	return cd_wq_workqueue_create(workers_n, name, CD_WQ_QUEUE_OPTION_STOP_SOFT);
}

enum cd_error cd_wq_workqueue_stop(struct cd_workqueue *wq)
{
	struct cd_worker    *w = NULL;
	uint8_t             workers_n = 0;
	enum cd_error       err = CD_ERR_OK;

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
				if (pthread_join(w->tid, NULL) != CD_ERR_OK) {                  /* join worker thread */
					err = CD_ERR_FAIL;
				}
			} else {
				pthread_mutex_unlock(&w->mutex);
			}
		}
	}
	return err;
}

struct cd_work* cd_wq_work_init(struct cd_work* work, enum cd_work_sync_async_type type, void *user_data, int user_data_type, void*(*f)(void*), void(*f_dtor)(void*))
{
	CD_INIT_LIST_HEAD(&work->link);
	work->type = type;
	work->user_data = user_data;
	work->user_data_type = user_data_type;
	work->f = f;
	work->f_dtor = f_dtor;
	return work;
}

struct cd_work* cd_wq_work_create(enum cd_work_sync_async_type type, void *user_data, int user_data_type, void*(*f)(void*), void(*f_dtor)(void*))
{
	struct cd_work* work = malloc(sizeof(struct cd_work));
	if (work == NULL)
		return NULL;

	return cd_wq_work_init(work, type, user_data, user_data_type, f, f_dtor);
}

void cd_wq_work_free(struct cd_work* work)
{
	if (work->user_data != NULL) {
		free(work->user_data);
		work->user_data = NULL;
	}

	free(work);
}

enum cd_error cd_wq_queue_work(struct cd_workqueue *wq, struct cd_work* work)
{
	struct cd_worker    *w = NULL;
	uint8_t             idx = wq->next_worker_idx_to_use, sanity = 0xFF;

	if (wq->workers_active_n == 0) {
		CD_LOG_CRIT("NO ACTIVE WORKER THREAD in the workqueue [%s]", wq->name);
		return CD_ERR_WORKQUEUE_ACTIVE;
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
	wq->next_worker_idx_to_use = idx;												/* save next worker's index into workqueue */

	pthread_mutex_lock(&w->mutex);													/* enqueue work (and move ownership to worker) */
	cd_fifo_enqueue(&work->link, &w->queue);
	pthread_cond_signal(&w->signal);
	pthread_mutex_unlock(&w->mutex);

	return CD_ERR_OK;
}

void cd_wq_queue_delayed_work(struct cd_workqueue *q, struct cd_work* work, unsigned int delay)
{
	(void)q;
	(void)work;
	(void)delay;
	return;
}

enum cd_error cd_wq_queue_user(struct cd_workqueue *wq, enum cd_work_sync_async_type type, void *user_data, int user_data_type, void*(*f)(void*), void(*f_dtor)(void*))
{
	struct cd_work *work = cd_wq_work_create(type, user_data, user_data_type, f, f_dtor);
	if (!work) {
		return CD_ERR_WORK_CREATE;
	}

	return cd_wq_queue_work(wq, work);
}

enum cd_error cd_launch_thread(pthread_t *t, void*(*f)(void*), void *arg, int detachstate)
{
	int                 err;
	pthread_attr_t      attr;

	err = pthread_attr_init(&attr);
	if (err != 0)
		return CD_ERR_FAIL;

	err = pthread_attr_setdetachstate(&attr, detachstate);
	if (err != 0)
		goto fail;

	err = pthread_create(t, &attr, f, arg);
	if (err != 0)
		goto fail;

	pthread_attr_destroy(&attr);

	return 0;

fail:
	pthread_attr_destroy(&attr);
	return CD_ERR_FAIL;
}
