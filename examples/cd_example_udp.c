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


static void* on_udp_msg(void *msg)
{
    cd_msg_t *m = msg;

    if (!m)
	return NULL;

    printf("Got %zu bytes to process\n", m->len);

    // process m->data
    return NULL;
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

    cd_udp_endpoint_loop(udp);

    cd_udp_endpoint_destroy(&udp);
    return 0;
}
