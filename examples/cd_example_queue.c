/**
 * cd_example_queue.c - Queue example
 *
 * Part of the libcd - Libcd implements queue and queue processing with multiple worker threads, from Data And Signal's Piotr Gregor.
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */

#include <cd.h>


struct task {
	uint32_t id;
	pthread_mutex_t *mutex;
	int *counter;
};

int counter;
pthread_mutex_t counter_mutex;

static void* my_function(void *arg)
{
	struct task *t = arg;

	pthread_mutex_lock(t->mutex);

	*t->counter += 1;
	printf("Task %u\n", t->id);
	fflush(stdout);

	// Add delay, to make tasks hanging while queue is requested to stop
	usleep(1000);

	pthread_mutex_unlock(t->mutex);

	return NULL;
}

int main(void)
{
	uint32_t workers_n = 2;
	const char *name = "My workqueue";
	struct cd_workqueue *wq = NULL;
	struct task t[9] = { 0 };
	struct cd_work *w[9] = { 0 };

	pthread_mutex_init(&counter_mutex, NULL);

	wq = cd_wq_workqueue_default_create(workers_n, name);

	for (int i = 0; i < 9; i++) {
		t[i].id = i;
		t[i].mutex = &counter_mutex;
		t[i].counter = &counter;
	}

	for (int i = 0; i < 9; i++) {
		w[i] = cd_wq_work_create(CD_WORK_ASYNC, (void *) &t[i], 0, my_function, NULL);
	}

	for (int i = 0; i < 9; i++) {
		if (CD_ERR_OK != cd_wq_queue_work(wq, w[i])) {
			printf("Failed to enqueue work %d\n", i);
			return -1;
		}
	}

	// Let the queue process at least 1 task
	usleep(2000);

	// Note, cd_wq_workqueue_stop() must wait for workers to finish (or terminate) processing of tasks if any of them have not yet been processed.
	// You can configure queue's behaviour in such cases with cd_wq_configure():
	// CD_WQ_QUEUE_OPTION_STOP_HARD will terminate processing,
	// CD_WQ_QUEUE_OPTION_STOP_SOFT will continue processing of tasks until all workers' queues are empty (default)
	if (CD_ERR_OK != cd_wq_workqueue_stop(wq)) {
		perror("Failed to stop the queue\n");
		return -1;
	}

	pthread_mutex_lock(&counter_mutex);
	assert(counter == 9);
	printf("Counter is %d. All jobs were executed\n", counter);
	pthread_mutex_unlock(&counter_mutex);

	cd_wq_workqueue_free(&wq);
	pthread_mutex_destroy(&counter_mutex);

	return 0;
}
