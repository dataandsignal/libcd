/**
 * cd_wq.h - Workqueue implementation
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */

#ifndef CD_WORKQUEUE_H
#define CD_WORKQUEUE_H


#include "cd.h"
#include "cd_list.h"


enum cd_work_sync_async_type {
	CD_WORK_SYNC,               /* destructor will be called by worker syncronously after processing callback returned */
	CD_WORK_ASYNC               /* worker thread is not responsible for the calling of destructor of this work type - call to destructor must be configured by work's processing callback  */
};

struct cd_worker {              /* thread wrapper */
	uint8_t         idx;        /* index in workqueue table */
	pthread_t       tid;
	cd_fifo_queue    queue;      /* queue of work structs */
	pthread_mutex_t mutex;
	pthread_cond_t  signal;     /* signaled when new item is enqueued to this worker's queue */
	uint8_t         active;		/* successfully created and waiting for work */
	uint8_t         busy;		/* active thread (successfully created and waiting for work) may be busy while processing work */
	struct cd_workqueue *wq;    /* owner */
};

struct cd_workqueue {
	uint8_t             running;            /* 0 - no, 1 - yes */
	struct cd_worker    *workers;
	uint8_t             workers_n;          /* number of worker threads */
	uint32_t            workers_active_n;   /* number of active worker threads: successfully created and accepting work */
	const char          *name;
	uint8_t             first_active_worker_idx;
	uint8_t             next_worker_idx_to_use; /* index of next worker to use for enquing the work in round-robin fashion */
};

/* @brief   Start the worker threads.
 * @details After this returns the @workers_n variable in workqueue is set to the numbers of successfully created
 *          and now running threads. It isn't neccessary the same number that has been passed to this function. */
enum cd_error cd_wq_workqueue_init(struct cd_workqueue *q, uint32_t workers_n, const char *name);
enum cd_error cd_wq_workqueue_deinit(struct cd_workqueue *wq);

void cd_wq_workqueue_free(struct cd_workqueue *wq);
struct cd_workqueue* cd_wq_workqueue_create(uint32_t workers_n, const char *name);
enum cd_error cd_wq_workqueue_stop(struct cd_workqueue *wq);

struct cd_work {
	struct cd_list_head  link;
	enum cd_work_sync_async_type   type;
	uint8_t				worker_idx;			/* index of worker in the workers table of workqueue, which is processing this work */					

	void *user_data;						/* user data */
	int user_data_type;						/* demultiplex work */
	void* (*f)(void*);                      /* processing */
	void (*f_dtor)(void*);                  /* destructor */
};

#define CD_WORK_INITIALIZER(n, t, ud, udt, f, f_dtor) {      \
	.link  = { &(n).link, &(n).link },      \
	.type = (t),							\
	.user_data = (ud),						\
	.user_data_type = (udt),				\
	.f = (f),								\
	.f_dtor = (f_dtor)						\
}

#define DECLARE_WORK(n, t, ud, udt, f, f_dtor) \
	struct work_struct n = CD_WORK_INITIALIZER(n, t, ud, udt, f, f_dtor)

struct cd_work* cd_wq_work_init(struct cd_work* work, enum cd_work_sync_async_type type, void *user_data, int user_data_type, void*(*f)(void*), void(*f_dtor)(void*));
struct cd_work* cd_wq_work_create(enum cd_work_sync_async_type type, void *user_data, int user_data_type, void*(*f)(void*), void(*f_dtor)(void*));
void cd_wq_work_free(struct cd_work* work);
void cd_wq_queue_work(struct cd_workqueue *q, struct cd_work* work);
void cd_wq_queue_delayed_work(struct cd_workqueue *q, struct cd_work* work, unsigned int delay);
enum cd_error cd_launch_thread(pthread_t *t, void*(*f)(void*), void *arg, int detachstate);


#endif  /* CD_WORKQUEUE_H */

