/**
 * cd_endpoint.c - Endpoints
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */

#include "../include/cd.h"


int cd_udp_endpoint_do_init(cd_udp_endpoint_t *udp)
{
    if (!udp)
	return -1;

    return 0;
}

int cd_endpoint_init(cd_endpoint_t* endpoint, cd_endpoint_type_t type)
{
    if (!endpoint)
	return -1;

    switch (type) {
	case CD_ENDPOINT_TYPE_UDP:
	    endpoint->type = CD_ENDPOINT_TYPE_UDP;
	    return cd_udp_endpoint_do_init((cd_udp_endpoint_t*) endpoint);

	case CD_ENDPOINT_TYPE_TCP:
	    endpoint->type = CD_ENDPOINT_TYPE_TCP;
	    break;

	case CD_ENDPOINT_TYPE_HTTP:
	    endpoint->type = CD_ENDPOINT_TYPE_HTTP;
	    break;

	case CD_ENDPOINT_TYPE_HTTPS:
	    endpoint->type = CD_ENDPOINT_TYPE_HTTPS;
	    break;

	case CD_ENDPOINT_TYPE_WEBSOCKET:
	    endpoint->type = CD_ENDPOINT_TYPE_WEBSOCKET;
	    break;
	default:
	    return -1;
    }

    return 0;
}

cd_udp_endpoint_t* cd_udp_endpoint_create(void)
{
    cd_udp_endpoint_t *udp = malloc(sizeof(cd_udp_endpoint_t));
    if (!udp)
	return NULL;
    memset(udp, 0, sizeof(cd_udp_endpoint_t));

    if (-1 == cd_endpoint_init(&udp->base, CD_ENDPOINT_TYPE_UDP)) {
	free(udp);
	return NULL;
    }

    return udp;
}

cd_msg_t* cd_endpoint_msg_create(char *buf, size_t len)
{
    cd_msg_t *m = NULL;

    m = malloc(sizeof(*m));
    if (!m)
	return NULL;
    memset(m, 0, sizeof(*m));

    if (len) {
	m->data = malloc(len);
	if (!m->data) {
	    free(m);
	    return NULL;
	}
	memcpy(m->data, buf, len);
    }

    m->len = len;
    return m;
}

void cd_endpoint_msg_destroy(cd_msg_t **msg)
{
    if (!msg || !(*msg))
	return;

    if ((*msg)->data) {
	free((*msg)->data);
	(*msg)->data = NULL;
    }
    free(*msg);
    *msg = NULL;
    printf("Something died\n");
}

void cd_endpoint_msg_dctor_f(void *o)
{
    cd_msg_t *msg = (cd_msg_t*) o;
    if (!o)
	return;
    cd_endpoint_msg_destroy(&msg);
}

int cd_udp_endpoint_set_port(cd_udp_endpoint_t *udp, uint16_t port)
{
    if (!udp)
	return -1;

    udp->base.port = port;
    return 0;
}

int cd_udp_endpoint_set_workqueue_name(cd_udp_endpoint_t *udp, const char *name)
{
    if (!udp)
	return -1;

    strncpy(udp->base.wq_name, name, sizeof(udp->base.wq_name));
    return 0;
}

int cd_udp_endpoint_set_workqueue_threads_n(cd_udp_endpoint_t *udp, uint32_t workers_n)
{
    if (!udp)
	return -1;

    udp->base.wq_workers_n = workers_n;
    return 0;
}

int cd_udp_endpoint_set_on_message_callback(cd_udp_endpoint_t *udp, cd_endpoint_on_msg_cb cb)
{
    if (!udp)
	return -1;

    udp->base.cb_on_msg = cb;
    return 0;
}

static int cd_udp_endpoint_do_loop(cd_udp_endpoint_t *udp)
{
    socklen_t len = 0;
    char msg[CD_UDP_BUFLEN] = { 0 };
    ssize_t bytes_n = 0;
    char buf[400] = { 0 }, portstr[100] = { 0 };

    struct cd_work *w = NULL;
    cd_msg_t *m = NULL;


    if (!udp)
	return -1;

    // TODO instal signal handler
    // Signal(SIGINT, recvfrom_int);

    for ( ; ; ) {
	struct sockaddr_in *sin = NULL;

	len = sizeof(udp->base.cliaddr);
	bytes_n = recvfrom(udp->base.sockfd, msg, sizeof(msg), 0, (struct sockaddr*) &udp->base.cliaddr, &len);
	if (bytes_n < 0) {
	    CD_LOG_ERR("recvfrom failed");
	    return -1;
	}

	inet_ntop(AF_INET, &((struct sockaddr_in*) &udp->base.cliaddr)->sin_addr, buf, 400);
	sin = (struct sockaddr_in*) &udp->base.cliaddr;
	snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
	strcat(buf, portstr);

	CD_LOG_INFO("recvfrom %ld bytes (UDP) - from: %s\n", bytes_n, buf);

	m = cd_endpoint_msg_create(buf, bytes_n);
	if (!m) {
	    CD_LOG_ERR("Cannot allocate memory for UDP message");
	    return -1;
	}

	w = cd_wq_work_create(CD_WORK_SYNC, m, 0, udp->base.cb_on_msg, cd_endpoint_msg_dctor_f);
	if (!w) {
	    CD_LOG_ERR("Cannot create work");
	    return -1;
	}

	if (CD_ERR_OK != cd_wq_queue_work(udp->base.wq, w)) {
	    CD_LOG_ERR("Cannot enqueue work");
	    return -1;
	}
    }

    return 0;
}

int cd_udp_endpoint_loop(cd_udp_endpoint_t *udp)
{
    int n = 0;

    if (!udp)
	return -1;

    udp->base.wq = cd_wq_workqueue_default_create(udp->base.wq_workers_n, udp->base.wq_name);
    if (!udp->base.wq) {
	CD_LOG_ERR("Cannot create work queue");
	return -1;
    }

    n = socket(AF_INET, SOCK_DGRAM, 0);
    if (n < 0)
	return -1;

    udp->base.sockfd = n;

    bzero(&udp->base.servaddr, sizeof(udp->base.servaddr));
    udp->base.servaddr.sin_family = AF_INET;
    udp->base.servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    udp->base.servaddr.sin_port = htons(udp->base.port);

    if (bind(udp->base.sockfd, (struct sockaddr *) &udp->base.servaddr, sizeof(udp->base.servaddr)) < 0) {
	CD_LOG_ERR("Cannot bind to port %u", udp->base.port);
	return -2;
    }

    return cd_udp_endpoint_do_loop(udp);
}

void cd_udp_endpoint_destroy(cd_udp_endpoint_t** udp)
{
    if (!(*udp))
	return;
    free(*udp);
    *udp = NULL;
}
