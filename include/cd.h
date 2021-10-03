/**
 * cd.h - main includes
 *
 * Part of the libcd - Libcd implements queue and queue processing with multiple worker threads, from Data And Signal's Piotr Gregor.
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


#endif // CD_H
