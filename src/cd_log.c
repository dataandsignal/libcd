/**
 * cd_log.c - Logging
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */

#include "../include/cd_log.h"


void cd_util_calc_timespec_diff(struct timespec *t1, struct timespec *t2, struct timespec *dt)
{
	if (t2->tv_nsec < t1->tv_nsec) {                          /* if nanoseconds part is smaller in older time, then seconds part MUST be bigger */
		dt->tv_sec = t2->tv_sec - t1->tv_sec - 1;
		dt->tv_nsec = CD_NANOSEC_PER_SEC + t2->tv_nsec - t1->tv_nsec;
	} else {                                                    /* "normal" situation */
		dt->tv_sec = t2->tv_sec - t1->tv_sec;
		dt->tv_nsec = t2->tv_nsec - t1->tv_nsec;
	}
}

int cd_util_dt(char *buf)
{
	struct tm	t;
	time_t		now = time(0);

	if (localtime_r(&now, &t) != 0) { /* localtime_r is thread safe (and therefore reentrant) */
		strftime(buf, 20, "%Y-%m-%d_%H-%M-%S", &t);
		return CD_ERR_OK;
	}
	return CD_ERR_FAIL;
}

int cd_util_dt_detail(char *buf)
{
	struct tm	t;
	struct timeval	tv;

	gettimeofday(&tv, NULL);
	if (localtime_r(&tv.tv_sec, &t) != 0) { /* localtime_r is thread safe */
		char tmp[20];
		strftime(tmp, 20, "%Y-%m-%d_%H-%M-%S", &t);
		tmp[19] = '\0';
		sprintf(buf, "%s:%06ld", tmp, tv.tv_usec); /* append microseconds, buf must be at least 28 chars */
		return CD_ERR_OK;
	}
	return CD_ERR_FAIL;
}

int cd_util_openlogs(const char *dir, const char *name)
{
	FILE	*stream;
	int		err;
	char	*full_path;

	if (dir == NULL)
		return CD_ERR_BAD_CALL;

	full_path = malloc(strlen(dir) + strlen(name) + 1 + 27 + 1 + 3 + 1);
	if (full_path == NULL)
		return CD_ERR_MEM;

	sprintf(full_path, "%s%s_", dir, name);
	if (cd_util_dt_detail(full_path + strlen(dir) + strlen(name) + 1) != CD_ERR_OK) {
		err = CD_ERR_FAIL;
		goto fail;
	}
	strcpy(full_path + strlen(dir) + strlen(name) + 27, ".log"); /* stdout log */
	if ((stream = fopen(full_path, "w+")) == NULL) {
		err = CD_ERR_FOPEN_STDOUT;
		goto fail;
	}
	fclose(stream);
	if (freopen(full_path, "w", stdout) == NULL) { /* redirect stdout */
		err = CD_ERR_FREOPEN_STDOUT;
		goto fail;
	}
	strcpy(full_path + strlen(dir) + strlen(name) + 27, ".err"); /* stderr log */
	if ((stream = fopen(full_path, "w+")) == NULL) {
		err = CD_ERR_FOPEN_STDERR;
		goto fail;
	}
	fclose(stream);
	if (freopen(full_path, "w", stderr) == NULL) { /* redirect stderr */
		err = CD_ERR_FREOPEN_STDERR;
		goto fail;
	}
	free(full_path);
	return CD_ERR_OK;

fail:
	if (full_path != NULL) {
		free(full_path);
	}
	return err;
}

int cd_util_log(FILE *stream, const char *fmt, ...)
{
	va_list	args;
	char 	buf[4096];
	char	dt[28];

	if (cd_util_dt_detail(dt) != CD_ERR_OK) {
		return CD_ERR_FAIL;
	}
	va_start(args, fmt);
	snprintf(buf, sizeof(buf), "%s\t%s\n", dt, fmt);
	vfprintf(stream, buf, args);
	va_end(args);
	fflush(stream);

	return CD_ERR_OK; 
}

int cd_util_log_perr(FILE *stream, const char *fmt, ...)
{
	va_list	args;
	char 	buf[4096];
	char	dt[28];

	if (cd_util_dt_detail(dt) != CD_ERR_OK) {
		return CD_ERR_FAIL;
	}
	va_start(args, fmt);
	snprintf(buf, sizeof(buf), "%s\t%s, %s\n", dt, fmt, strerror(errno));
	vfprintf(stream, buf, args);
	va_end(args);
	fflush(stream);

	return CD_ERR_OK; 
}

int cd_util_daemonize(const char *dir, int noclose, const char *logname)
{
	pid_t	pid, sid;
	int 	fd;

	if (logname == NULL)
		return CD_ERR_BAD_CALL;

	if ((pid = fork()) < 0) {																		/* background */
		return CD_ERR_FAIL;
	} else if (pid > 0) {																			/* parent, terminate */
		exit(EXIT_SUCCESS);
	}
	if ((sid = setsid()) < 0)																		/* 1st child, become session leader */
		return CD_ERR_SETSID;

	if ((pid = fork()) < 0) {																		/* fork again, loose controlling terminal forever */
		return CD_ERR_FORK;
	} else if (pid > 0) {																			/* 1st child, terminate */
		exit(EXIT_SUCCESS);
	}

	signal(SIGINT, SIG_IGN);																		/* TODO: handle signals */
	signal(SIGHUP, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	umask(S_IWGRP | S_IWOTH);																		/* set file mode to 0x622 (rw-r--r--), umask syscall always succeedes */
	if (dir != NULL) {																				/* change dir */
		if (chdir(dir) == -1) {
			return CD_ERR_CHDIR;
		}
	}

	if (cd_util_openlogs("./log/", logname) != CD_ERR_OK)
		return CD_ERR_IO_ERROR;

	if (noclose == 0) {																				/* close open descriptors */
		fd = sysconf(_SC_OPEN_MAX);
		for (; fd > 2; fd--)
			close(fd);

	}
	return CD_ERR_OK;
}

int cd_util_chdir_umask_openlog(const char *dir, int noclose, const char *logname, uint8_t ignore_signals)
{
	int 	fd;

	if (ignore_signals != 0) {																		/* TODO: handle signals */
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGCHLD, SIG_IGN);
	}
	umask(S_IWGRP | S_IWOTH);																		/* set file mode to 0x622 (rw-r--r--) umask syscall always succeedes */
	if (dir != NULL) {																				/* change dir */
		if (chdir(dir) == -1) {
			return CD_ERR_CHDIR;
		}
	}
	if ((logname != NULL) && (cd_util_openlogs("./log/", logname) != CD_ERR_OK))
		return CD_ERR_IO_ERROR;

	if (noclose == 0) {																				/* close open descriptors */
		fd = sysconf(_SC_OPEN_MAX);
		for (; fd > 2; fd--)
			close(fd);

	}
	return CD_ERR_OK;
}