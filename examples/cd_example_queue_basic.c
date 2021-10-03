/**
 * cd_example_queue_basic.c - Queue example
 *
 * Part of the libcd - Libcd implements queue and queue processing with multiple worker threads, from Data And Signal's Piotr Gregor.
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */

#include <cd.h>


const char *mario = "Mario";


static void* my_function(void *arg)
{
	printf("I am %s\n", (char *) arg);
	return NULL;
}

int main(void)
{
	const char *queue_name = "My workqueue";
	uint32_t workers_n = 2;
	struct cd_workqueue *wq = NULL;
	struct cd_work *work = NULL;


	wq = cd_wq_workqueue_default_create(workers_n, queue_name);
	work = cd_wq_work_create(CD_WORK_ASYNC, (void *) mario, 0, my_function, NULL);
	cd_wq_queue_work(wq, work);

	cd_wq_workqueue_stop(wq);
	cd_wq_workqueue_free(&wq);

	return 0;
}
