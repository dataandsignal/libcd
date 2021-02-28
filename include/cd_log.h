/**
 * cd_log.h - Logging
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */


#ifndef CD_LOG_H
#define CD_LOG_H


#include "cd.h"
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>


void cd_util_calc_timespec_diff(struct timespec *t1, struct timespec *t2, struct timespec *dt) __attribute__ ((nonnull(1,2,3)));
int cd_util_dt(char *buf);
int cd_util_dt_detail(char *buf);
int cd_util_openlog(const char *dir, const char *name);
int cd_util_log(FILE *stream, const char *fmt, ...);
int cd_util_log_perr(FILE *stream, const char *fmt, ...);
int cd_util_daemonize(const char *dir, int noclose, const char *logname);
int cd_util_chdir_umask_openlog(const char *dir, int noclose, const char *logname, uint8_t ignore_signals);

#ifdef DDEBUG
#define CD_D_ERR(fmt, args...) fprintf(stderr, "DEBUG ERR: %s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##args)
#else
#define CD_D_ERR(fmt, ...)    do { } while (0)
#endif

#define CD_LOG_ERR(fmt, ...) cd_util_log(stderr, "%s\t%s:%d:%s():\t" fmt, "ERR ", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define CD_LOG_PERR(fmt, ...) cd_util_log_perr(stderr, "%s\t%s:%d:%s():\t" fmt, "ERR ", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define CD_LOG_INFO(fmt, ...) cd_util_log(stderr, "%s\t%s:%d:%s():\t" fmt, "INFO", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define CD_LOG_ALERT(fmt, ...) cd_util_log(stderr, "%s\t%s:%d:%s():\t" fmt, "ALERT", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define CD_LOG_WARN(fmt, ...) cd_util_log(stderr, "%s\t%s:%d:%s():\t" fmt, "WARN", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
#define CD_LOG_CRIT(fmt, ...) cd_util_log(stderr, "%s\t%s:%d:%s():\t" fmt, "CRIT", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#ifdef DDEBUG
    #define CD_DEBUG_LOG_ERR(fmt, ...) CD_LOG_ERR(fmt, ...)
    #define CD_DEBUG_LOG_PERR(fmt, ...) CD_LOG_PERR(fmt, ...)
    #define CD_DEBUG_LOG_INFO(fmt, ...) CD_LOG_INFO(fmt, ...)
    #define CD_DEBUG_LOG_ALERT(fmt, ...) CD_LOG_ALERT(fmt, ...)
    #define CD_DEBUG_LOG_WARN(fmt, ...) CD_LOG_WARN(fmt, ...)
    #define CD_DEBUG_LOG_CRIT(fmt, ...) CD_LOG_CRIT(fmt, ...)
#else
    #define CD_DEBUG_LOG_ERR(fmt, ...) do {} while (0)
    #define CD_DEBUG_LOG_PERR(fmt, ...) do {} while (0)
    #define CD_DEBUG_LOG_INFO(fmt, ...) do {} while (0)
    #define CD_DEBUG_LOG_ALERT(fmt, ...) do {} while (0)
    #define CD_DEBUG_LOG_WARN(fmt, ...) do {} while (0)
    #define CD_DEBUG_LOG_CRIT(fmt, ...) do {} while (0)
#endif


#endif	/* CD_LOG_H */
