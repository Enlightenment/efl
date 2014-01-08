#ifndef _EO_ERROR_MSGS_H
#define _EO_ERROR_MSGS_H

#include "Eo.h"
#include "eo_suite.h"

/* The Max level to consider when working with the print cb. */
#define _EINA_LOG_MAX 2
/* #define SHOW_LOG 1 */

struct log_ctx {
   const char *msg;
   const char *fnc;
   Eina_Bool did;
   int expected_level;
};

void
eo_test_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args EINA_UNUSED);

void
eo_test_safety_print_cb(const Eina_Log_Domain *d, Eina_Log_Level level, const char *file, const char *fnc, int line, const char *fmt, void *data, va_list args EINA_UNUSED);

#define TEST_EO_ERROR(fn, _msg)                  \
  ctx.msg = _msg;                                \
  ctx.fnc = fn;                                  \
  ctx.did = EINA_FALSE;                          \
  ctx.expected_level = EINA_LOG_LEVEL_ERR

#endif /* _EO_ERROR_MSGS_H */
