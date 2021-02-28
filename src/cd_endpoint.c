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


int cd_endpoint_init(cd_endpoint_t* endpoint, cd_endpoint_type_t type)
{
    if (!endpoint)
	return -1;

    switch (type) {
	case CD_ENDPOINT_TYPE_UDP:
	    endpoint->type = CD_ENDPOINT_TYPE_UDP;
	    break;
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

    if (-1 == cd_endpoint_init(&udp->base, CD_ENDPOINT_TYPE_UDP)) {
	free(udp);
	return NULL;
    }

    return udp;
}

int cd_udp_endpoint_set_port(cd_udp_endpoint_t *udp, uint16_t port)
{
    if (!udp)
	return -1;

    udp->base.port = port;
    return 0;
}

int cd_udp_endpoint_set_on_message_callback(cd_udp_endpoint_t *udp, cd_endpoint_on_msg_cb cb)
{
    if (!udp)
	return -1;

    udp->base.cb_on_msg = cb;
    return 0;
}

int cd_udp_endpoint_start(cd_udp_endpoint_t *udp)
{
    if (!udp)
	return -1;
    CD_LOG_INFO("Udp endpoint started");
    return 0;
}

void cd_udp_endpoint_destroy(cd_udp_endpoint_t** udp)
{
    if (!(*udp))
	return;
    free(*udp);
}
