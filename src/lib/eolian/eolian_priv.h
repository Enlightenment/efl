#ifndef EOLIAN_PRIV_H
#define EOLIAN_PRIV_H

#include <Eina.h>
#include <stdio.h>
#include <stdarg.h>

static inline void _eolian_log_line(const char *file, int line, int column, const char *str) EINA_ARG_NONNULL(1, 4);
static inline void _eolian_log(const char *str) EINA_ARG_NONNULL(1);

static inline void
_eolian_log_line(const char *file, int line, int column, const char *str)
{
   if (!eina_log_color_disable_get())
     {
        fprintf(stderr, EINA_COLOR_RED "eolian" EINA_COLOR_RESET ": "
                EINA_COLOR_WHITE "%s" EINA_COLOR_RESET ":%d:%d: "
                EINA_COLOR_ORANGE "%s\n" EINA_COLOR_RESET,
                file, line, column, str);
     }
   else
     {
        fprintf(stderr, "eolian: %s:%d:%d: %s\n", file, line, column, str);
     }
}

static inline void
_eolian_log(const char *str)
{
   if (!eina_log_color_disable_get())
     {
        fprintf(stderr, EINA_COLOR_RED "eolian" EINA_COLOR_RESET ": "
                EINA_COLOR_ORANGE "%s\n" EINA_COLOR_RESET, str);
     }
   else
     {
        fprintf(stderr, "eolian: %s\n", str);
     }
}

#endif
