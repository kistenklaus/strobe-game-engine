#ifndef STROBE_LOG_H
#define STROBE_LOG_H

#include <stdarg.h>
#include <stdio.h>

#ifdef STROBE_LOG_FATAL_ERRORS
static void logFatal(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}
#else
static void logFatal(char *fmt, ...) {}
#endif

#ifdef STROBE_LOG_ERRORS
static void logError(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}
#else
static void logError(char *fmt, ...) {}
#endif

#ifdef STROBE_LOG_WARNINGS
static void logWarn(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}
#else
static void logWarn(char *fmt, ...) {}
#endif

#ifdef STROBE_LOG_INFOS
static void logInfo(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}
#else
static void logInfo(char* fmt, ...){}
#endif

#ifdef STROBE_LOG_DEBUG
static void logDebug(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}
#else
static void logDebug(char* fmt, ...){}
#endif

#ifdef STROBE_LOG_STACK_TRACE
static void logTrace(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}
#else
static void logTrace(char* fmt, ...){}
#endif

#endif
