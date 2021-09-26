/**
 * cd_test_wq.c - Unit tests for cd_wq
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */

#include "../include/cd_wq.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>


struct test {
	uint32_t	key;
	int		val;
};

static void test_wq_create(void)
{
	uint32_t workers_n = 5;
	const char *name = "Workqueue Test Create";
	struct cd_workqueue *wq = NULL;

	printf("TEST WQ CREATE\n");

	wq = cd_wq_workqueue_create(workers_n, name, CD_WQ_QUEUE_OPTION_STOP_HARD);

	assert(wq != NULL);
	assert(wq->running == 1);
	assert(wq->workers_n == workers_n);
	assert(wq->workers_active_n == workers_n);
	assert(strcmp(wq->name, name) == 0);
	assert(wq->first_active_worker_idx == workers_n - 1);
	assert(wq->next_worker_idx_to_use == wq->first_active_worker_idx);
	assert(wq->options.CD_WQ_QUEUE_OPTION_STOP == CD_WQ_QUEUE_OPTION_STOP_HARD);
	assert(CD_ERR_OK == cd_wq_workqueue_stop(wq));

	cd_wq_workqueue_free(&wq);
	printf("CREATE: OK\n");
}

uint8_t test_wq_queue_default_counter;
pthread_mutex_t test_wq_queue_default_counter_mutex;

static void* test_wq_queue_default_f(void *arg)
{
    assert(arg == &test_wq_queue_default_counter);
    pthread_mutex_lock(&test_wq_queue_default_counter_mutex);
    test_wq_queue_default_counter++;
    printf("Default: Counter: %u\n", test_wq_queue_default_counter);
    fflush(stdout);

    // Add delay, to make tasks hanging while queue is requested to stop
    usleep(1000);

    pthread_mutex_unlock(&test_wq_queue_default_counter_mutex);
    return NULL;
}

static void test_wq_queue_default(void)
{
	uint32_t workers_n = 2;
	const char *name = "Workqueue Test Default";
	struct cd_workqueue *wq = NULL;
	struct cd_work *w1 = NULL, *w2 = NULL, *w3 = NULL;
	struct cd_work *w4 = NULL, *w5 = NULL, *w6 = NULL;
	struct cd_work *w7 = NULL, *w8 = NULL, *w9 = NULL;

	test_wq_queue_default_counter = 0;
	pthread_mutex_init(&test_wq_queue_default_counter_mutex, NULL);
	wq = cd_wq_workqueue_default_create(workers_n, name);

	assert(wq != NULL);
	assert(wq->running == 1);
	assert(wq->workers_n == workers_n);
	assert(wq->workers_active_n == workers_n);
	assert(strcmp(wq->name, name) == 0);
	assert(wq->first_active_worker_idx == workers_n - 1);
	assert(wq->next_worker_idx_to_use == wq->first_active_worker_idx);
	assert(wq->options.CD_WQ_QUEUE_OPTION_STOP == CD_WQ_QUEUE_OPTION_STOP_SOFT);

	w1 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_default_counter, 555, test_wq_queue_default_f, NULL);
	w2 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_default_counter, 556, test_wq_queue_default_f, NULL);
	w3 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_default_counter, 557, test_wq_queue_default_f, NULL);
	w4 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_default_counter, 558, test_wq_queue_default_f, NULL);
	w5 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_default_counter, 559, test_wq_queue_default_f, NULL);
	w6 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_default_counter, 560, test_wq_queue_default_f, NULL);
	w7 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_default_counter, 561, test_wq_queue_default_f, NULL);
	w8 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_default_counter, 562, test_wq_queue_default_f, NULL);
	w9 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_default_counter, 563, test_wq_queue_default_f, NULL);

	printf("TEST WQ DEFAULT (SOFT)\n");

	assert(w1 != NULL);
	assert(w2 != NULL);
	assert(w3 != NULL);
	assert(w4 != NULL);
	assert(w5 != NULL);
	assert(w6 != NULL);
	assert(w7 != NULL);
	assert(w8 != NULL);
	assert(w9 != NULL);
	
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w1));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w2));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w3));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w4));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w5));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w6));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w7));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w8));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w9));

	// Let the queue process at least 1 task
	usleep(2000);

	// Note, cd_wq_workqueue_stop() must wait for workers to finish (or terminate) processing of tasks if any of them have not yet been processed.
	// You can configure queue's behaviour in such cases with cd_wq_configure():
	// CD_WQ_QUEUE_OPTION_STOP_HARD will terminate processing,
	// CD_WQ_QUEUE_OPTION_STOP_SOFT will continue processing of tasks until all workers' queues are empty (default)
	assert(CD_ERR_OK == cd_wq_workqueue_stop(wq));

	pthread_mutex_lock(&test_wq_queue_default_counter_mutex);
	assert(test_wq_queue_default_counter == 9);
	printf("DEFAULT (SOFT): All jobs were executed\n");
	pthread_mutex_unlock(&test_wq_queue_default_counter_mutex);
	
	cd_wq_workqueue_free(&wq);

	pthread_mutex_destroy(&test_wq_queue_default_counter_mutex);
}

uint8_t test_wq_queue_soft_counter;
pthread_mutex_t test_wq_queue_soft_counter_mutex;

static void* test_wq_queue_soft_f(void *arg)
{
    assert(arg == &test_wq_queue_soft_counter);

    pthread_mutex_lock(&test_wq_queue_soft_counter_mutex);
    test_wq_queue_soft_counter++;
    printf("Soft: Counter: %u\n", test_wq_queue_soft_counter);
    fflush(stdout);

    // Add delay, to make tasks hanging when queue is requested to stop
    usleep(1000);

    pthread_mutex_unlock(&test_wq_queue_soft_counter_mutex);
    return NULL;
}

static void test_wq_queue_soft(void)
{
	uint32_t workers_n = 1;
	const char *name = "Workqueue Test Stop Soft";
	struct cd_workqueue *wq = NULL;
	struct cd_work *w1 = NULL, *w2 = NULL, *w3 = NULL;
	struct cd_work *w4 = NULL, *w5 = NULL, *w6 = NULL;
	struct cd_work *w7 = NULL, *w8 = NULL, *w9 = NULL;

	test_wq_queue_soft_counter = 0;
	pthread_mutex_init(&test_wq_queue_soft_counter_mutex, NULL);
	wq = cd_wq_workqueue_create(workers_n, name, CD_WQ_QUEUE_OPTION_STOP_SOFT);

	assert(wq != NULL);
	assert(wq->running == 1);
	assert(wq->workers_n == workers_n);
	assert(wq->workers_active_n == workers_n);
	assert(strcmp(wq->name, name) == 0);
	assert(wq->first_active_worker_idx == workers_n - 1);
	assert(wq->next_worker_idx_to_use == wq->first_active_worker_idx);
	assert(wq->options.CD_WQ_QUEUE_OPTION_STOP == CD_WQ_QUEUE_OPTION_STOP_SOFT);

	w1 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_soft_counter, 555, test_wq_queue_soft_f, NULL);
	w2 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_soft_counter, 556, test_wq_queue_soft_f, NULL);
	w3 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_soft_counter, 557, test_wq_queue_soft_f, NULL);
	w4 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_soft_counter, 558, test_wq_queue_soft_f, NULL);
	w5 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_soft_counter, 559, test_wq_queue_soft_f, NULL);
	w6 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_soft_counter, 560, test_wq_queue_soft_f, NULL);
	w7 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_soft_counter, 561, test_wq_queue_soft_f, NULL);
	w8 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_soft_counter, 562, test_wq_queue_soft_f, NULL);
	w9 = cd_wq_work_create(CD_WORK_ASYNC, (void *) &test_wq_queue_soft_counter, 563, test_wq_queue_soft_f, NULL);

	printf("TEST WQ STOP SOFT\n");

	assert(w1 != NULL);
	assert(w2 != NULL);
	assert(w3 != NULL);
	assert(w4 != NULL);
	assert(w5 != NULL);
	assert(w6 != NULL);
	assert(w7 != NULL);
	assert(w8 != NULL);
	assert(w9 != NULL);
	
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w1));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w2));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w3));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w4));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w5));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w6));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w7));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w8));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w9));

	// Let the queue process at least 1 task
	usleep(2000);

	// Test that cd_wq_workqueue_stop() waits for workers to finish processing of tasks.
	// Tasks are not processed yet, because of delay applied to user's processing function.
	assert(CD_ERR_OK == cd_wq_workqueue_stop(wq));

	pthread_mutex_lock(&test_wq_queue_soft_counter_mutex);
	assert(test_wq_queue_soft_counter == 9);
	printf("SOFT: All jobs were executed\n");
	pthread_mutex_unlock(&test_wq_queue_soft_counter_mutex);
	
	cd_wq_workqueue_free(&wq);

	pthread_mutex_destroy(&test_wq_queue_soft_counter_mutex);
}

uint8_t test_wq_queue_default_sync_counter;
uint8_t test_wq_queue_default_sync_dctor_counter;
pthread_mutex_t test_wq_queue_default_sync_counter_mutex;

struct test_wq_queue_default_sync_user_data {
    void *mutex;
    int something;
};

static void* test_wq_queue_default_sync_f(void *arg)
{
    struct test_wq_queue_default_sync_user_data *user_data = (struct test_wq_queue_default_sync_user_data*) arg;
    assert(user_data->mutex == &test_wq_queue_default_sync_counter_mutex);
    pthread_mutex_lock(user_data->mutex);
    test_wq_queue_default_sync_counter++;
    printf("Default sync: [id=%d] Counter: %u\n", user_data->something, test_wq_queue_default_sync_counter);
    fflush(stdout);

    // Add delay, to make tasks hanging when queue is requested to stop
    usleep(1000);

    pthread_mutex_unlock(user_data->mutex);
    return NULL;
}

static void test_wq_queue_default_sync_f_dtor(void *arg)
{
    struct test_wq_queue_default_sync_user_data *user_data = (struct test_wq_queue_default_sync_user_data*) arg;
    pthread_mutex_lock(user_data->mutex);
    printf("Default sync: [id=%d] -> ~() User dtor, free user data\n", user_data->something);
    fflush(stdout);
    test_wq_queue_default_sync_dctor_counter++;
    printf("Default sync: [id=%d] Dctor counter: %u\n", user_data->something, test_wq_queue_default_sync_dctor_counter);
    fflush(stdout);
    pthread_mutex_unlock(user_data->mutex);
    free(user_data);
}

static void test_wq_queue_default_sync(void)
{
	uint32_t workers_n = 4;
	const char *name = "Workqueue Test Default Sync";
	struct cd_workqueue *wq = NULL;
	struct cd_work *w1 = NULL, *w2 = NULL, *w3 = NULL;
	struct cd_work *w4 = NULL, *w5 = NULL, *w6 = NULL;
	struct cd_work *w7 = NULL, *w8 = NULL, *w9 = NULL;
	struct test_wq_queue_default_sync_user_data *user_data_1 = malloc(sizeof(struct test_wq_queue_default_sync_user_data));
	struct test_wq_queue_default_sync_user_data *user_data_2 = malloc(sizeof(struct test_wq_queue_default_sync_user_data));
	struct test_wq_queue_default_sync_user_data *user_data_3 = malloc(sizeof(struct test_wq_queue_default_sync_user_data));
	struct test_wq_queue_default_sync_user_data *user_data_4 = malloc(sizeof(struct test_wq_queue_default_sync_user_data));
	struct test_wq_queue_default_sync_user_data *user_data_5 = malloc(sizeof(struct test_wq_queue_default_sync_user_data));
	struct test_wq_queue_default_sync_user_data *user_data_6 = malloc(sizeof(struct test_wq_queue_default_sync_user_data));
	struct test_wq_queue_default_sync_user_data *user_data_7 = malloc(sizeof(struct test_wq_queue_default_sync_user_data));
	struct test_wq_queue_default_sync_user_data *user_data_8 = malloc(sizeof(struct test_wq_queue_default_sync_user_data));
	struct test_wq_queue_default_sync_user_data *user_data_9 = malloc(sizeof(struct test_wq_queue_default_sync_user_data));

	printf("TEST WQ DEFAULT SYNC\n");

	assert(user_data_1 != NULL);
	assert(user_data_2 != NULL);
	assert(user_data_3 != NULL);
	assert(user_data_4 != NULL);
	assert(user_data_5 != NULL);
	assert(user_data_6 != NULL);
	assert(user_data_7 != NULL);
	assert(user_data_8 != NULL);
	assert(user_data_9 != NULL);

	user_data_1->mutex = &test_wq_queue_default_sync_counter_mutex;
	user_data_2->mutex = &test_wq_queue_default_sync_counter_mutex;
	user_data_3->mutex = &test_wq_queue_default_sync_counter_mutex;
	user_data_4->mutex = &test_wq_queue_default_sync_counter_mutex;
	user_data_5->mutex = &test_wq_queue_default_sync_counter_mutex;
	user_data_6->mutex = &test_wq_queue_default_sync_counter_mutex;
	user_data_7->mutex = &test_wq_queue_default_sync_counter_mutex;
	user_data_8->mutex = &test_wq_queue_default_sync_counter_mutex;
	user_data_9->mutex = &test_wq_queue_default_sync_counter_mutex;
	user_data_1->something = 1;
	user_data_2->something = 2;
	user_data_3->something = 3;
	user_data_4->something = 4;
	user_data_5->something = 5;
	user_data_6->something = 6;
	user_data_7->something = 7;
	user_data_8->something = 8;
	user_data_9->something = 9;

	test_wq_queue_default_sync_counter = 0;
	test_wq_queue_default_sync_dctor_counter = 0;
	pthread_mutex_init(&test_wq_queue_default_sync_counter_mutex, NULL);
	wq = cd_wq_workqueue_default_create(workers_n, name);

	assert(wq != NULL);
	assert(wq->running == 1);
	assert(wq->workers_n == workers_n);
	assert(wq->workers_active_n == workers_n);
	assert(strcmp(wq->name, name) == 0);
	assert(wq->first_active_worker_idx == workers_n - 1);
	assert(wq->next_worker_idx_to_use == wq->first_active_worker_idx);
	assert(wq->options.CD_WQ_QUEUE_OPTION_STOP == CD_WQ_QUEUE_OPTION_STOP_SOFT);

	w1 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_1, 555, test_wq_queue_default_sync_f, test_wq_queue_default_sync_f_dtor);
	w2 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_2, 556, test_wq_queue_default_sync_f, test_wq_queue_default_sync_f_dtor);
	w3 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_3, 557, test_wq_queue_default_sync_f, test_wq_queue_default_sync_f_dtor);
	w4 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_4, 558, test_wq_queue_default_sync_f, test_wq_queue_default_sync_f_dtor);
	w5 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_5, 559, test_wq_queue_default_sync_f, test_wq_queue_default_sync_f_dtor);
	w6 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_6, 560, test_wq_queue_default_sync_f, test_wq_queue_default_sync_f_dtor);
	w7 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_7, 561, test_wq_queue_default_sync_f, test_wq_queue_default_sync_f_dtor);
	w8 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_8, 562, test_wq_queue_default_sync_f, test_wq_queue_default_sync_f_dtor);
	w9 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_9, 563, test_wq_queue_default_sync_f, test_wq_queue_default_sync_f_dtor);
	assert(w1 != NULL);
	assert(w2 != NULL);
	assert(w3 != NULL);
	assert(w4 != NULL);
	assert(w5 != NULL);
	assert(w6 != NULL);
	assert(w7 != NULL);
	assert(w8 != NULL);
	assert(w9 != NULL);
	
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w1));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w2));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w3));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w4));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w5));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w6));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w7));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w8));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w9));
    
	// Let the queue process at least 1 task
	usleep(2000);

	// Will block till all work is executed, dctors are called and queue is empty.
	assert(CD_ERR_OK == cd_wq_workqueue_stop(wq));

	pthread_mutex_lock(&test_wq_queue_default_sync_counter_mutex);
	assert(test_wq_queue_default_sync_counter == 9);
	printf("DEFAULT SYNC: All jobs were executed\n");
	assert(test_wq_queue_default_sync_dctor_counter == 9);
	printf("DEFAULT SYNC: All dctors were called\n");
	pthread_mutex_unlock(&test_wq_queue_default_sync_counter_mutex);
	
	cd_wq_workqueue_free(&wq);

	pthread_mutex_destroy(&test_wq_queue_default_sync_counter_mutex);
}

uint8_t test_wq_queue_hard_sync_counter;
uint8_t test_wq_queue_hard_sync_dctor_counter;
pthread_mutex_t test_wq_queue_hard_sync_counter_mutex;

struct test_wq_queue_hard_sync_user_data {
    void *mutex;
    int id;
};

static void* test_wq_queue_hard_sync_f(void *arg)
{
    struct test_wq_queue_hard_sync_user_data *user_data = (struct test_wq_queue_hard_sync_user_data*) arg;
    assert(user_data->mutex == &test_wq_queue_hard_sync_counter_mutex);
    pthread_mutex_lock(user_data->mutex);
    test_wq_queue_hard_sync_counter++;
    printf("Hard sync: [id=%d] Counter: %u\n", user_data->id, test_wq_queue_hard_sync_counter);
    fflush(stdout);
    
    // Add delay, to make tasks hanging when queue is requested to stop
    usleep(1000);

    pthread_mutex_unlock(user_data->mutex);
    return NULL;
}

static void test_wq_queue_hard_sync_f_dtor(void *arg)
{
    struct test_wq_queue_hard_sync_user_data *user_data = (struct test_wq_queue_hard_sync_user_data*) arg;
    pthread_mutex_lock(user_data->mutex);
    printf("Hard sync: [id=%d] -> ~() User dtor, free user data\n", user_data->id);
    fflush(stdout);
    test_wq_queue_hard_sync_dctor_counter++;
    printf("Hard sync: [id=%d] Dctor counter: %u\n", user_data->id, test_wq_queue_hard_sync_dctor_counter);
    fflush(stdout);
    pthread_mutex_unlock(user_data->mutex);
    free(user_data);
}

static void test_wq_queue_hard_sync(void)
{
	uint32_t workers_n = 2;
	const char *name = "Workqueue Test Stop Hard Sync";
	struct cd_workqueue *wq = NULL;
	struct cd_work *w1 = NULL, *w2 = NULL, *w3 = NULL;
	struct cd_work *w4 = NULL, *w5 = NULL, *w6 = NULL;
	struct cd_work *w7 = NULL, *w8 = NULL, *w9 = NULL;
	struct test_wq_queue_hard_sync_user_data *user_data_1 = malloc(sizeof(struct test_wq_queue_hard_sync_user_data));
	struct test_wq_queue_hard_sync_user_data *user_data_2 = malloc(sizeof(struct test_wq_queue_hard_sync_user_data));
	struct test_wq_queue_hard_sync_user_data *user_data_3 = malloc(sizeof(struct test_wq_queue_hard_sync_user_data));
	struct test_wq_queue_hard_sync_user_data *user_data_4 = malloc(sizeof(struct test_wq_queue_hard_sync_user_data));
	struct test_wq_queue_hard_sync_user_data *user_data_5 = malloc(sizeof(struct test_wq_queue_hard_sync_user_data));
	struct test_wq_queue_hard_sync_user_data *user_data_6 = malloc(sizeof(struct test_wq_queue_hard_sync_user_data));
	struct test_wq_queue_hard_sync_user_data *user_data_7 = malloc(sizeof(struct test_wq_queue_hard_sync_user_data));
	struct test_wq_queue_hard_sync_user_data *user_data_8 = malloc(sizeof(struct test_wq_queue_hard_sync_user_data));
	struct test_wq_queue_hard_sync_user_data *user_data_9 = malloc(sizeof(struct test_wq_queue_hard_sync_user_data));

	printf("TEST WQ STOP HARD SYNC (9 SYNC jobs)\n");

	assert(user_data_1 != NULL);
	assert(user_data_2 != NULL);
	assert(user_data_3 != NULL);
	assert(user_data_4 != NULL);
	assert(user_data_5 != NULL);
	assert(user_data_6 != NULL);
	assert(user_data_7 != NULL);
	assert(user_data_8 != NULL);
	assert(user_data_9 != NULL);

	user_data_1->mutex = &test_wq_queue_hard_sync_counter_mutex;
	user_data_2->mutex = &test_wq_queue_hard_sync_counter_mutex;
	user_data_3->mutex = &test_wq_queue_hard_sync_counter_mutex;
	user_data_4->mutex = &test_wq_queue_hard_sync_counter_mutex;
	user_data_5->mutex = &test_wq_queue_hard_sync_counter_mutex;
	user_data_6->mutex = &test_wq_queue_hard_sync_counter_mutex;
	user_data_7->mutex = &test_wq_queue_hard_sync_counter_mutex;
	user_data_8->mutex = &test_wq_queue_hard_sync_counter_mutex;
	user_data_9->mutex = &test_wq_queue_hard_sync_counter_mutex;
	user_data_1->id = 1;
	user_data_2->id = 2;
	user_data_3->id = 3;
	user_data_4->id = 4;
	user_data_5->id = 5;
	user_data_6->id = 6;
	user_data_7->id = 7;
	user_data_8->id = 8;
	user_data_9->id = 9;

	test_wq_queue_hard_sync_counter = 0;
	test_wq_queue_hard_sync_dctor_counter = 0;
	pthread_mutex_init(&test_wq_queue_hard_sync_counter_mutex, NULL);
	wq = cd_wq_workqueue_create(workers_n, name, CD_WQ_QUEUE_OPTION_STOP_HARD);

	assert(wq != NULL);
	assert(wq->running == 1);
	assert(wq->workers_n == workers_n);
	assert(wq->workers_active_n == workers_n);
	assert(strcmp(wq->name, name) == 0);
	assert(wq->first_active_worker_idx == workers_n - 1);
	assert(wq->next_worker_idx_to_use == wq->first_active_worker_idx);
	assert(wq->options.CD_WQ_QUEUE_OPTION_STOP == CD_WQ_QUEUE_OPTION_STOP_HARD);

	w1 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_1, 555, test_wq_queue_hard_sync_f, test_wq_queue_hard_sync_f_dtor);
	w2 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_2, 556, test_wq_queue_hard_sync_f, test_wq_queue_hard_sync_f_dtor);
	w3 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_3, 557, test_wq_queue_hard_sync_f, test_wq_queue_hard_sync_f_dtor);
	w4 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_4, 558, test_wq_queue_hard_sync_f, test_wq_queue_hard_sync_f_dtor);
	w5 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_5, 559, test_wq_queue_hard_sync_f, test_wq_queue_hard_sync_f_dtor);
	w6 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_6, 560, test_wq_queue_hard_sync_f, test_wq_queue_hard_sync_f_dtor);
	w7 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_7, 561, test_wq_queue_hard_sync_f, test_wq_queue_hard_sync_f_dtor);
	w8 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_8, 562, test_wq_queue_hard_sync_f, test_wq_queue_hard_sync_f_dtor);
	w9 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_9, 563, test_wq_queue_hard_sync_f, test_wq_queue_hard_sync_f_dtor);
	assert(w1 != NULL);
	assert(w2 != NULL);
	assert(w3 != NULL);
	assert(w4 != NULL);
	assert(w5 != NULL);
	assert(w6 != NULL);
	assert(w7 != NULL);
	assert(w8 != NULL);
	assert(w9 != NULL);
	
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w1));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w2));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w3));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w4));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w5));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w6));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w7));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w8));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w9));

	// Let the queue process at least 1 task
	usleep(2000);

	assert(CD_ERR_OK == cd_wq_workqueue_stop(wq));

	// Will block till all threads are joined, CD_WORK_SYNC dctors are called and queue is empty.
	cd_wq_workqueue_free(&wq);

	pthread_mutex_lock(&test_wq_queue_hard_sync_counter_mutex);
	assert(test_wq_queue_hard_sync_dctor_counter == 9);
	printf("HARD SYNC: All dctors were called\n");
	assert(test_wq_queue_hard_sync_counter <= 9);
	printf("HARD SYNC: %u jobs were executed, %u jobs were skipped\n", test_wq_queue_hard_sync_counter, 9 - test_wq_queue_hard_sync_counter);
	pthread_mutex_unlock(&test_wq_queue_hard_sync_counter_mutex);

	pthread_mutex_destroy(&test_wq_queue_hard_sync_counter_mutex);
}

uint8_t test_wq_queue_hard_sync_async_counter;
uint8_t test_wq_queue_hard_sync_async_sync_dctor_counter;
uint8_t test_wq_queue_hard_sync_async_async_dctor_counter;
pthread_mutex_t test_wq_queue_hard_sync_async_counter_mutex;

struct test_wq_queue_hard_sync_async_user_data {
    void *mutex;
    int id;
};

static void* test_wq_queue_hard_sync_async_f(void *arg)
{
    struct test_wq_queue_hard_sync_async_user_data *user_data = (struct test_wq_queue_hard_sync_async_user_data*) arg;
    assert(user_data->mutex == &test_wq_queue_hard_sync_async_counter_mutex);
    pthread_mutex_lock(user_data->mutex);
    test_wq_queue_hard_sync_async_counter++;
    printf("Hard sync/async: [id=%d] Counter: %u\n", user_data->id, test_wq_queue_hard_sync_async_counter);
    fflush(stdout);
    
    // Add delay, to make tasks hanging when queue is requested to stop
    usleep(1000);

    pthread_mutex_unlock(user_data->mutex);
    return NULL;
}

static void test_wq_queue_hard_sync_async_f_sync_dtor(void *arg)
{
    struct test_wq_queue_hard_sync_async_user_data *user_data = (struct test_wq_queue_hard_sync_async_user_data*) arg;
    pthread_mutex_lock(user_data->mutex);
    printf("Hard sync/async: [id=%d] -> ~() User dtor [sync], free user data\n", user_data->id);
    fflush(stdout);
    test_wq_queue_hard_sync_async_sync_dctor_counter++;
    printf("Hard sync/async: [id=%d] Dctor counter [sync]: %u\n", user_data->id, test_wq_queue_hard_sync_async_sync_dctor_counter);
    fflush(stdout);
    pthread_mutex_unlock(user_data->mutex);
    free(user_data);
}

static void test_wq_queue_hard_sync_async_f_async_dtor(void *arg)
{
    struct test_wq_queue_hard_sync_async_user_data *user_data = (struct test_wq_queue_hard_sync_async_user_data*) arg;
    pthread_mutex_lock(user_data->mutex);
    printf("Hard sync/async: [id=%d] -> ~() User dtor [async], free user data\n", user_data->id);
    fflush(stdout);
    test_wq_queue_hard_sync_async_async_dctor_counter++;
    printf("Hard sync/async: [id=%d] Dctor counter [async]: %u\n", user_data->id, test_wq_queue_hard_sync_async_async_dctor_counter);
    fflush(stdout);
    pthread_mutex_unlock(user_data->mutex);
    free(user_data);
}

static void test_wq_queue_hard_sync_async(void)
{
	uint32_t workers_n = 2;
	const char *name = "Workqueue Test Stop Hard Sync/Async";
	struct cd_workqueue *wq = NULL;
	struct cd_work *w1 = NULL, *w2 = NULL, *w3 = NULL;
	struct cd_work *w4 = NULL, *w5 = NULL, *w6 = NULL;
	struct cd_work *w7 = NULL, *w8 = NULL, *w9 = NULL;
	struct cd_work *w10 = NULL, *w11 = NULL, *w12 = NULL, *w13 = NULL;
	struct cd_work *w14 = NULL, *w15 = NULL, *w16 = NULL, *w17 = NULL, *w18 = NULL, *w19 = NULL, *w20 = NULL;
	struct test_wq_queue_hard_sync_async_user_data *user_data_1 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_2 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_3 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_4 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_5 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_6 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_7 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_8 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_9 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_10 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_11 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_12 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_13 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_14 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_15 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_16 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_17 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_18 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_19 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));
	struct test_wq_queue_hard_sync_async_user_data *user_data_20 = malloc(sizeof(struct test_wq_queue_hard_sync_async_user_data));

	printf("TEST WQ STOP HARD SYNC/ASYNC (20 jobs: 11 sync, 9 async)\n");

	assert(user_data_1 != NULL);
	assert(user_data_2 != NULL);
	assert(user_data_3 != NULL);
	assert(user_data_4 != NULL);
	assert(user_data_5 != NULL);
	assert(user_data_6 != NULL);
	assert(user_data_7 != NULL);
	assert(user_data_8 != NULL);
	assert(user_data_9 != NULL);
	assert(user_data_10 != NULL);
	assert(user_data_11 != NULL);
	assert(user_data_12 != NULL);
	assert(user_data_13 != NULL);
	assert(user_data_14 != NULL);
	assert(user_data_15 != NULL);
	assert(user_data_16 != NULL);
	assert(user_data_17 != NULL);
	assert(user_data_18 != NULL);
	assert(user_data_19 != NULL);
	assert(user_data_20 != NULL);

	user_data_1->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_2->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_3->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_4->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_5->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_6->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_7->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_8->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_9->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_10->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_11->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_12->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_13->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_14->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_15->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_16->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_17->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_18->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_19->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_20->mutex = &test_wq_queue_hard_sync_async_counter_mutex;
	user_data_1->id = 1;
	user_data_2->id = 2;
	user_data_3->id = 3;
	user_data_4->id = 4;
	user_data_5->id = 5;
	user_data_6->id = 6;
	user_data_7->id = 7;
	user_data_8->id = 8;
	user_data_9->id = 9;
	user_data_10->id = 10;
	user_data_11->id = 11;
	user_data_12->id = 12;
	user_data_13->id = 13;
	user_data_14->id = 14;
	user_data_15->id = 15;
	user_data_16->id = 16;
	user_data_17->id = 17;
	user_data_18->id = 18;
	user_data_19->id = 19;
	user_data_20->id = 20;

	test_wq_queue_hard_sync_async_counter = 0;
	test_wq_queue_hard_sync_async_sync_dctor_counter = 0;
	test_wq_queue_hard_sync_async_async_dctor_counter = 0;
	pthread_mutex_init(&test_wq_queue_hard_sync_async_counter_mutex, NULL);
	wq = cd_wq_workqueue_create(workers_n, name, CD_WQ_QUEUE_OPTION_STOP_HARD);

	assert(wq != NULL);
	assert(wq->running == 1);
	assert(wq->workers_n == workers_n);
	assert(wq->workers_active_n == workers_n);
	assert(strcmp(wq->name, name) == 0);
	assert(wq->first_active_worker_idx == workers_n - 1);
	assert(wq->next_worker_idx_to_use == wq->first_active_worker_idx);
	assert(wq->options.CD_WQ_QUEUE_OPTION_STOP == CD_WQ_QUEUE_OPTION_STOP_HARD);

	w1 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_1, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_sync_dtor);
	w2 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_2, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_sync_dtor);
	w3 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_3, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_sync_dtor);
	w4 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_4, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_sync_dtor);
	w5 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_5, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_sync_dtor);
	w6 = cd_wq_work_create(CD_WORK_ASYNC, (void *) user_data_6, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_async_dtor);
	w7 = cd_wq_work_create(CD_WORK_ASYNC, (void *) user_data_7, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_async_dtor);
	w8 = cd_wq_work_create(CD_WORK_ASYNC, (void *) user_data_8, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_async_dtor);
	w9 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_9, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_sync_dtor);
	w10 = cd_wq_work_create(CD_WORK_ASYNC, (void *) user_data_10, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_async_dtor);
	w11 = cd_wq_work_create(CD_WORK_ASYNC, (void *) user_data_11, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_async_dtor);
	w12 = cd_wq_work_create(CD_WORK_ASYNC, (void *) user_data_12, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_async_dtor);
	w13 = cd_wq_work_create(CD_WORK_ASYNC, (void *) user_data_13, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_async_dtor);
	w14 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_14, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_sync_dtor);
	w15 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_15, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_sync_dtor);
	w16 = cd_wq_work_create(CD_WORK_ASYNC, (void *) user_data_16, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_async_dtor);
	w17 = cd_wq_work_create(CD_WORK_ASYNC, (void *) user_data_17, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_async_dtor);
	w18 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_18, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_sync_dtor);
	w19 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_19, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_sync_dtor);
	w20 = cd_wq_work_create(CD_WORK_SYNC, (void *) user_data_20, 0, test_wq_queue_hard_sync_async_f, test_wq_queue_hard_sync_async_f_sync_dtor);
	assert(w1 != NULL);
	assert(w2 != NULL);
	assert(w3 != NULL);
	assert(w4 != NULL);
	assert(w5 != NULL);
	assert(w6 != NULL);
	assert(w7 != NULL);
	assert(w8 != NULL);
	assert(w9 != NULL);
	assert(w10 != NULL);
	assert(w11 != NULL);
	assert(w12 != NULL);
	assert(w13 != NULL);
	assert(w14 != NULL);
	assert(w15 != NULL);
	assert(w16 != NULL);
	assert(w17 != NULL);
	assert(w18 != NULL);
	assert(w19 != NULL);
	assert(w20 != NULL);
	
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w1));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w2));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w3));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w4));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w5));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w6));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w7));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w8));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w9));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w10));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w11));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w12));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w13));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w14));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w15));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w16));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w17));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w18));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w19));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w20));

	// Let the queue process at least 1 task
	usleep(2000);

	assert(CD_ERR_OK == cd_wq_workqueue_stop(wq));

	// Will block till all threads are joined, CD_WORK_SYNC dctors are called and queue is empty.
	cd_wq_workqueue_free(&wq);

	pthread_mutex_lock(&test_wq_queue_hard_sync_async_counter_mutex);
	assert(test_wq_queue_hard_sync_async_sync_dctor_counter == 11);
	printf("HARD SYNC/ASYNC: All 11 SYNC dctors were called\n");
	assert(test_wq_queue_hard_sync_async_async_dctor_counter == 0);
	printf("HARD SYNC/ASYNC: All 9 ASYNC dctors were skipped\n");
	assert(test_wq_queue_hard_sync_async_counter <= 20);
	printf("HARD SYNC/ASYNC: %u jobs were executed, %u jobs were skipped\n", test_wq_queue_hard_sync_async_counter, 20 - test_wq_queue_hard_sync_async_counter);
	pthread_mutex_unlock(&test_wq_queue_hard_sync_async_counter_mutex);

	// Free async jobs
	free(user_data_6);
	free(user_data_7);
	free(user_data_8);
	free(user_data_10);
	free(user_data_11);
	free(user_data_12);
	free(user_data_13);
	free(user_data_16);
	free(user_data_17);

	pthread_mutex_destroy(&test_wq_queue_hard_sync_async_counter_mutex);
}


int main(void)
{
	test_wq_create();
	test_wq_queue_default();
	test_wq_queue_soft();
	test_wq_queue_default_sync();
	test_wq_queue_hard_sync();
	test_wq_queue_hard_sync_async();
	printf("That's nice!\n");
	return 0;
}
