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


static void test_wq(void)
{
	test_wq_create();
}

int main(void)
{
	test_wq();
	printf("That's nice!\n");
	return 0;
}
