/**
 * cd.h - main includes
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */

#ifndef CD_H
#define CD_H


#include <stdlib.h>             /* everything */
#include <stdio.h>              /* most I/O */
#include <sys/types.h>          /* syscalls */
#include <sys/stat.h>           /* umask, fstat */
#include <sys/socket.h>         /* socket.h etc. */
#include <netinet/in.h>         /* networking */
#include <linux/netdevice.h>
#include <linux/limits.h>       /* PATH_MAX */
#include <arpa/inet.h>
#include <string.h>             /* memset, strdup, etc. */
#include <fcntl.h>              /* open, R_ONLY */
#include <unistd.h>             /* close */
#include <errno.h>
#include <assert.h>
#include <pthread.h>            /* POSIX threads */
#include <stddef.h>             /* offsetof */
#include <signal.h>
#include <syslog.h>
#include <stdint.h>
#include <ctype.h>              /* isprint */
#include <libgen.h>             /* dirname */
#include <uuid/uuid.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stddef.h>

#include <sys/resource.h>

#include "cd_list.h"
#include "cd_hash.h"
#include "cd_wq.h"
#include "cd_log.h"

#define CD_NANOSEC_PER_SEC 1000000000U
#define CD_STR_BUF_LEN 1000

enum cd_error {
	CD_ERR_OK,
	CD_ERR_FAIL,
	CD_ERR_BUSY,
	CD_ERR_MEM,
	CD_ERR_SETSID,
	CD_ERR_WORK_CREATE,
	CD_ERR_WORKQUEUE_CREATE,
	CD_ERR_WORKQUEUE_ACTIVE,
	CD_ERR_BAD_CALL,
	CD_ERR_FORK,
	CD_ERR_CHDIR,
	CD_ERR_IO_ERROR,
	CD_ERR_FOPEN_STDOUT,
	CD_ERR_FOPEN_STDERR,
	CD_ERR_FREOPEN_STDOUT,
	CD_ERR_FREOPEN_STDERR
};

typedef enum cd_endpoint_type {
	CD_ENDPOINT_TYPE_UDP,
	CD_ENDPOINT_TYPE_TCP,
	CD_ENDPOINT_TYPE_HTTP,
	CD_ENDPOINT_TYPE_HTTPS,
	CD_ENDPOINT_TYPE_WEBSOCKET
} cd_endpoint_type_t;

struct cd_endpoint_s;
struct cd_msg_s;
typedef void* (*cd_endpoint_on_msg_cb)(void *);
typedef void (*cd_endpoint_sig_handler_t)(int signo);

typedef struct cd_endpoint_s {
	cd_endpoint_type_t		type;
	uint16_t				port;
	struct cd_workqueue		*wq;
	uint32_t				wq_workers_n;
	char					wq_name[CD_STR_BUF_LEN];
	cd_endpoint_on_msg_cb	cb_on_msg;
	int						sockfd;
	struct sockaddr_in		servaddr, cliaddr;
} cd_endpoint_t;

int cd_endpoint_init(cd_endpoint_t* endpoint, cd_endpoint_type_t type);
int cd_endpoint_set_port(cd_endpoint_t *endpoint, uint16_t port);
int cd_endpoint_set_on_message_callback(cd_endpoint_t *endpoint, cd_endpoint_on_msg_cb cb);
int cd_endpoint_start(cd_endpoint_t *endpoint);

// User interface

typedef struct cd_msg_s {
	char *data;
	size_t len;
} cd_msg_t;

cd_msg_t* cd_endpoint_msg_create(char *buf, size_t len);
void cd_endpoint_msg_destroy(cd_msg_t **msg);
void cd_endpoint_msg_dctor_f(void *o);

typedef struct cd_udp_endpoint_s {
	cd_endpoint_t	base;
} cd_udp_endpoint_t;

typedef struct cd_tcp_endpoint_s {
	cd_endpoint_t	base;
} cd_tcp_endpoint_t;

cd_udp_endpoint_t* cd_udp_endpoint_create(void);
cd_tcp_endpoint_t* cd_tcp_endpoint_create(void);

void cd_udp_endpoint_destroy(cd_udp_endpoint_t** udp);
void cd_tcp_endpoint_destroy(cd_tcp_endpoint_t** tcp);

int cd_udp_endpoint_init(cd_udp_endpoint_t *udp);
int cd_tcp_endpoint_init(cd_tcp_endpoint_t *tcp);

int cd_udp_endpoint_set_port(cd_udp_endpoint_t *udp, uint16_t port);
int cd_tcp_endpoint_set_port(cd_tcp_endpoint_t *tcp, uint16_t port);

int cd_udp_endpoint_set_workqueue_name(cd_udp_endpoint_t *udp, const char *name);
int cd_tcp_endpoint_set_workqueue_name(cd_tcp_endpoint_t *tcp, const char *name);

int cd_udp_endpoint_set_workqueue_threads_n(cd_udp_endpoint_t *udp, uint32_t workers_n);
int cd_tcp_endpoint_set_workqueue_threads_n(cd_tcp_endpoint_t *tcp, uint32_t workers_n);

int cd_udp_endpoint_set_on_message_callback(cd_udp_endpoint_t *udp, cd_endpoint_on_msg_cb cb);
int cd_tcp_endpoint_set_on_message_callback(cd_tcp_endpoint_t *tcp, cd_endpoint_on_msg_cb cb);

int cd_udp_endpoint_set_signal_handler(int signo, cd_endpoint_sig_handler_t sig_handler);
int cd_tcp_endpoint_set_signal_handler(int signo, cd_endpoint_sig_handler_t sig_handler);

int cd_udp_endpoint_loop(cd_udp_endpoint_t *udp);
int cd_tcp_endpoint_loop(cd_tcp_endpoint_t *tcp);

int cd_udp_endpoint_stop(cd_udp_endpoint_t *udp);
int cd_tcp_endpoint_stop(cd_tcp_endpoint_t *tcp);

#define CD_UDP_BUFLEN 2000

#endif // CD_H
