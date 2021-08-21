#ifndef MP_MACROS_H
#define MP_MACROS_H

#if !defined(LOG_DOMAIN)
#define LOG_DOMAIN "MP_LOG"
#endif

#define LogError(crit, fmt, ...)				\
  do {								\
    openlog(LOG_DOMAIN, LOG_PID | LOG_CONS, LOG_USER);		\
    syslog(LOG_ERR, fmt, __VA_ARGS__);				\
    if ((crit)) {						\
      syslog(LOG_ERR, "errno(%d): %s", errno, strerror(errno)); \
    }								\
    closelog();							\
    if ((crit)) {						\
      abort();							\
    }								\
  } while (0)

#define traceFile stderr

#define Trace(fmt, ...)					\
  do {							\
    flockfile (traceFile);				\
    (void) fprintf (traceFile,				\
                    "PID[%d]: %s:%s:%d " fmt "\n",	\
                    getpid (),				\
                    __FILE__,				\
                    __func__,				\
                    __LINE__,				\
                    __VA_ARGS__);			\
    (void) fflush (traceFile);				\
    funlockfile (traceFile);				\
  } while (0)

#define AbortIfFalse(cond)				\
  do {							\
    if (!(cond)) {					\
      LogError (true, "assertion `%s` failed", #cond);	\
    }							\
  } while (0)

#define ReturnIfFalse(cond)				\
  do {							\
    if (!(cond)) {					\
      LogError (false, "assertion `%s` failed", #cond); \
      return;						\
    }							\
  } while (0)

#define ReturnValIfFalse(cond, retval)			\
  do {							\
    if (!(cond)) {					\
      LogError (false, "assertion `%s` failed", #cond); \
      return (retval);					\
    }							\
  } while (0)

#endif
