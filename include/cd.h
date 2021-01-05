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

#define CD_NANOSEC_PER_SEC 1000000000U

enum cd_error {
	CD_ERR_OK,
	CD_ERR_FAIL,
	CD_ERR_BUSY,
	CD_ERR_MEM,
	CD_ERR_SETSID,
	CD_ERR_WORKQUEUE_CREATE,
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
