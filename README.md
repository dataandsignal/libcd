# libcd

Support for C programs with queue processors, from Data And Signal's Piotr Gregor.


## Quick setup

```
git clone https://github.com/dataandsignal/libcd.git
cd libcd
make
make examples
make test
```

Tested on Debian 10 (buster):

```
image: debian-10.10.0-amd64-netinst.iso
kernel: Linux buster 4.19.0-17-amd64 #1 SMP Debian 4.19.194-3 (2021-07-18) x86_64 GNU/Linux
platform: VirtualBox
```


## Basic Example

In this example workqueue is created with cd_wq_workqueue_default_create(). This gives you a default queue, which is blocking, thus waiting for all the enqueued jobs to get processed before stopping (see how cd_wq_workqueue_stop() gets called straight after cd_wq_queue_work() and this works as expected - job gets processed).

The job (struct cd_work) gets created by a call to cd_wq_work_create() with CD_WORK_ASYNC type, which tells workqueue that user's data (here a "Mario" string) doesn't need any special handling after job got processed (note also NULL as last argument to cd_wq_work_create(), otherwise you could pass your callback there).

Finally, let's note that workqueue will care about releasing memory allocated for work if you enqueued the work with cd_wq_queue_work(), thus you do not release this memory if you called cd_wq_queue_work(), otherwise you have to do it. 

```
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
```

For more examples see /examples folder and /test folder.


## Features

- Two types of queues are available: blocking and nonblocking. They differ in how they react to stop request. Blocking queue will wait for all the already enqueued jobs to be processed, before terminating processing of the queue, nonblocking queue will stop processing immediately, leaving any unprocessed jobs behind. Blocking is default. To configure this behaviour choose appropriate queue stop option:

	Nonblocking:
	```
	wq = cd_wq_workqueue_create(workers_n, name, CD_WQ_QUEUE_OPTION_STOP_HARD);
	```

	Blocking (default):
	```
	wq = cd_wq_workqueue_create(workers_n, name, CD_WQ_QUEUE_OPTION_STOP_SOFT);
	// or
	wq = cd_wq_workqueue_default_create(workers_n, name);
	```

- Support for automatic calling (or no calling) of destructors for processed jobs (think of callbacks on user data here, rather than memory release, since it is up to you how you handle your data. This may be free() or your cleanup function, but can be simply a counter or other logging util). If job is created with SYNC option, user's destructor (or a callback rather) will get called on user's data once processing of the work wrapping this data has finished, or when queue is terminated. If work is ASYNC, destructor will not be called (user handles user data cleanup, if any). To configure this option choose appropriate setting when creating work:

	SYNC:
	```
	w = cd_wq_work_create(CD_WORK_SYNC, 
				(void *) user_data, 
				user_ref,
				user_function,
				user_destructor);
	```

	ASYNC:
	```
	w = cd_wq_work_create(CD_WORK_ASYNC, 
				(void *) user_data, 
				user_ref, 
				user_function, 
				NULL);
	``` 

	For SYNC jobs, it is guaranteed that there will be a single call to user's dectructor (once the job is done or terminated).	

## UDP endpoint

UDP endpoint is implemented with libcd's work queue. It will open UDP port, create work queue, and start as many workers as configured, distributing work evenly.
User callback gets called for each UDP packet.

```
#include "../include/cd.h"


static void* on_udp_msg(void *msg)
{
    cd_msg_t *m = msg;

    if (!m)
	return NULL;

    printf("Got %zu bytes to process\n", m->len);

    // process m->data
    return NULL;
}

static void sigint_handler(int signo)
{
    printf("Signal %d (%s) received\n", signo, strsignal(signo));

    if (SIGINT == signo) {

	printf("Do widzenia!\n");

	cd_udp_endpoint_stop(udp);
	cd_udp_endpoint_destroy(&udp);
	exit(EXIT_SUCCESS);
    } else {
	printf ("Ignored\n");
    }
}

int main(void)
{
    cd_udp_endpoint_t *udp = NULL;

    /**
     * By default all log from libcd goes into stderr, so you can redirect it wherever you want.
     * If however all log output from lib should be redirected to some file then use this
     *
     * if (-1 == cd_util_openlog("/tmp/", "libcd"))
     *     return -1;
     *
     * and all log will be there. You can also print to that file with CD_LOG_INFO/ERR/PERR/ALERT/WARN/CRIT:
     *     CD_LOG_INFO("This goes wherever stdout/stderr output from libcd goes");
     */

    udp = cd_udp_endpoint_create();
    if (!udp)
	return -1;

    cd_udp_endpoint_set_port(udp, 33226);
    cd_udp_endpoint_set_workqueue_name(udp, "UDP workqueue");
    cd_udp_endpoint_set_workqueue_threads_n(udp, 4);
    cd_udp_endpoint_set_on_message_callback(udp, on_udp_msg);
    cd_udp_endpoint_set_signal_handler(SIGINT, sigint_handler);

    if (cd_udp_endpoint_init(udp) != 0) {
	// Maybe could not bind or create a socket
	cd_udp_endpoint_destroy(&udp);
	return -1;
    }

    // This will block and process messages (send SIGINT to stop the endpoint and exit)
    cd_udp_endpoint_loop(udp);

    return 0;
}
```

## BUILD

This builds on Linux Debian. make, make debug and make release produce shared library in build/debug or build/release folders.

```
make debug			-> for debug library build
make release			-> for release library build
make test-debug			-> for build and run of debug version of tests 
make test-release		-> for build and run of release version of tests
make examples-debug		-> for build and run of debug version of examples 
make examples-release		-> for build and run of release version of examples

make				-> same as make release
make test			-> same as make test-release
make examples			-> same as make examples-release

make clean			-> remove library binaries
make test-clean			-> remove test binaries
make examples-clean		-> remove examples binaries
make clean-all			-> remove library and test and examples binaries

make install-headers		-> create /usr/local/include/cd folder and install lib headers to it
make install-debug		-> install debug version of library to /lib
make install-release		-> install release version of library to /lib
make install			-> same as make install-release

make uninstall			-> remove library from /lib and headers from /usr/local/include/cd
```

## Contribute

Please submit any issues [here](https://github.com/dataandsignal/libcd/issues).

All contributions are welcome, please submit PRs [here](https://github.com/dataandsignal/libcd/pulls).

Enjoy!
