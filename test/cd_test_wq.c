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
	const char *name = "Workqueue Test 1";
	struct cd_workqueue *wq = NULL;
	
	
	wq = cd_wq_workqueue_create(workers_n, name);

	assert(wq != NULL);
	assert(wq->running == 1);
	assert(wq->workers_n == workers_n);
	assert(wq->workers_active_n == workers_n);
	assert(strcmp(wq->name, name) == 0);
	assert(wq->first_active_worker_idx == workers_n - 1);
	assert(wq->next_worker_idx_to_use == wq->first_active_worker_idx);
	assert(CD_ERR_OK == cd_wq_workqueue_stop(wq));

	cd_wq_workqueue_free(wq);
}

uint8_t test_wq_queue_counter;
pthread_mutex_t test_wq_queue_counter_mutex;

static void* test_wq_queue_f(void *arg)
{
    assert(arg == &test_wq_queue_counter);
    pthread_mutex_lock(&test_wq_queue_counter_mutex);
    test_wq_queue_counter++;
    printf("Counter: %u\n", test_wq_queue_counter);
    pthread_mutex_unlock(&test_wq_queue_counter_mutex);
    return NULL;
}

static void test_wq_queue(void)
{
	uint32_t workers_n = 5;
	const char *name = "Workqueue Test 1";
	struct cd_workqueue *wq = NULL;
	struct cd_work *w1 = NULL, *w2 = NULL, *w3 = NULL;

	test_wq_queue_counter = 0;
	pthread_mutex_init(&test_wq_queue_counter_mutex, NULL);
	wq = cd_wq_workqueue_create(workers_n, name);

	assert(wq != NULL);
	assert(wq->running == 1);
	assert(wq->workers_n == workers_n);
	assert(wq->workers_active_n == workers_n);
	assert(strcmp(wq->name, name) == 0);
	assert(wq->first_active_worker_idx == workers_n - 1);
	assert(wq->next_worker_idx_to_use == wq->first_active_worker_idx);

	w1 = cd_wq_work_create(CD_WORK_SYNC, (void *) &test_wq_queue_counter, 555, test_wq_queue_f, NULL);
	w2 = cd_wq_work_create(CD_WORK_SYNC, (void *) &test_wq_queue_counter, 556, test_wq_queue_f, NULL);
	w3 = cd_wq_work_create(CD_WORK_SYNC, (void *) &test_wq_queue_counter, 557, test_wq_queue_f, NULL);
	assert(w1 != NULL);
	assert(w2 != NULL);
	assert(w3 != NULL);
	
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w1));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w2));
	assert(CD_ERR_OK == cd_wq_queue_work(wq, w3));

	// TODO cd_wq_workqueue_stop() must wait for workers to finish (or terminate) processing of tasks
	// Add config param to configure queue's behaviour in such cases
	sleep(3);
	assert(CD_ERR_OK == cd_wq_workqueue_stop(wq));

	pthread_mutex_lock(&test_wq_queue_counter_mutex);
	assert(test_wq_queue_counter == 3);
	pthread_mutex_unlock(&test_wq_queue_counter_mutex);

	cd_wq_workqueue_free(wq);
	pthread_mutex_destroy(&test_wq_queue_counter_mutex);
}


static void test_wq(void)
{
	test_wq_create();
}

int main(void)
{
	test_wq();
	test_wq_queue();
	printf("That's nice!\n");
	return 0;
}
