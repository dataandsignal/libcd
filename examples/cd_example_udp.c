/**
 * cd_example_udp.c - Example UDP endpoint with workqueue
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */

#include "../include/cd.h"


cd_udp_endpoint_t *udp = NULL;

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
