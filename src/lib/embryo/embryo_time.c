#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/time.h>
#include <time.h>

#ifdef _WIN32
# include <evil_private.h> /* setenv unsetenv */
#endif

#ifdef HAVE_EXOTIC
# include <Exotic.h>
#endif

#include <Eina.h>

#include "Embryo.h"
#include "embryo_private.h"

#define STRGET(ep, str, par) {                                \
     Embryo_Cell *___cptr;                                    \
     str = NULL;                                              \
     if ((___cptr = embryo_data_address_get(ep, par))) {      \
          int ___l;                                           \
          ___l = embryo_data_string_length_get(ep, ___cptr);  \
          (str) = alloca(___l + 1);                           \
          if (str) embryo_data_string_get(ep, ___cptr, str);  \
       } }
/* exported time api */

static Embryo_Cell
_embryo_time_seconds(Embryo_Program *ep EINA_UNUSED, Embryo_Cell *params EINA_UNUSED)
{
   struct timeval timev;
   double t;
   float f;

   gettimeofday(&timev, NULL);
   t = (double)(timev.tv_sec - ((timev.tv_sec / (60 * 60 * 24)) * (60 * 60 * 24)))
     + (((double)timev.tv_usec) / 1000000);
   f = (float)t;
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_time_date(Embryo_Program *ep, Embryo_Cell *params)
{
   static time_t last_tzset = 0;
   struct timeval timev;
   struct tm *tm;
   time_t tt;

   if (params[0] != (8 * sizeof(Embryo_Cell))) return 0;
   gettimeofday(&timev, NULL);
   tt = (time_t)(timev.tv_sec);
   if ((tt > (last_tzset + 1)) ||
       (tt < (last_tzset - 1)))
     {
        last_tzset = tt;
        tzset();
     }
   tm = localtime(&tt);
   if (tm)
     {
        Embryo_Cell *cptr;
        double t;
        float f;

        cptr = embryo_data_address_get(ep, params[1]);
        if (cptr) *cptr = tm->tm_year + 1900;
        cptr = embryo_data_address_get(ep, params[2]);
        if (cptr) *cptr = tm->tm_mon + 1;
        cptr = embryo_data_address_get(ep, params[3]);
        if (cptr) *cptr = tm->tm_mday;
        cptr = embryo_data_address_get(ep, params[4]);
        if (cptr) *cptr = tm->tm_yday;
        cptr = embryo_data_address_get(ep, params[5]);
        if (cptr) *cptr = (tm->tm_wday + 6) % 7;
        cptr = embryo_data_address_get(ep, params[6]);
        if (cptr) *cptr = tm->tm_hour;
        cptr = embryo_data_address_get(ep, params[7]);
        if (cptr) *cptr = tm->tm_min;
        cptr = embryo_data_address_get(ep, params[8]);
        t = (double)tm->tm_sec + (((double)timev.tv_usec) / 1000000);
        f = (float)t;
        if (cptr) *cptr = EMBRYO_FLOAT_TO_CELL(f);
     }
   return 0;
}

static Embryo_Cell
_embryo_time_tzdate(Embryo_Program *ep, Embryo_Cell *params)
{
   struct timeval timev;
   struct tm *tm;
   time_t tt;
   const char *tzenv;
   char *tz, prevtz[128] = {0};

   if (params[0] != (9 * sizeof(Embryo_Cell))) return 0;
   STRGET(ep, tz, params[1]);
   tzenv = getenv("TZ");
   if (tzenv)
     strncpy(prevtz, tzenv, sizeof(prevtz) - 1);
   if (tz && tz[0])
     {
        setenv("TZ", tz, 1);
        tzset();
     }
   gettimeofday(&timev, NULL);
   tt = (time_t)(timev.tv_sec);
   tm = localtime(&tt);
   if (tz && tz[0])
     {
        if (prevtz[0])
          setenv("TZ", prevtz, 1);
        else
          unsetenv("TZ");
        tzset();
     }
   if (tm)
     {
        Embryo_Cell *cptr;
        double t;
        float f;

        cptr = embryo_data_address_get(ep, params[2]);
        if (cptr) *cptr = tm->tm_year + 1900;
        cptr = embryo_data_address_get(ep, params[3]);
        if (cptr) *cptr = tm->tm_mon + 1;
        cptr = embryo_data_address_get(ep, params[4]);
        if (cptr) *cptr = tm->tm_mday;
        cptr = embryo_data_address_get(ep, params[5]);
        if (cptr) *cptr = tm->tm_yday;
        cptr = embryo_data_address_get(ep, params[6]);
        if (cptr) *cptr = (tm->tm_wday + 6) % 7;
        cptr = embryo_data_address_get(ep, params[7]);
        if (cptr) *cptr = tm->tm_hour;
        cptr = embryo_data_address_get(ep, params[8]);
        if (cptr) *cptr = tm->tm_min;
        cptr = embryo_data_address_get(ep, params[9]);
        t = (double)tm->tm_sec + (((double)timev.tv_usec) / 1000000);
        f = (float)t;
        if (cptr) *cptr = EMBRYO_FLOAT_TO_CELL(f);
     }
   return 0;
}

/* functions used by the rest of embryo */

void
_embryo_time_init(Embryo_Program *ep)
{
   embryo_program_native_call_add(ep, "seconds", _embryo_time_seconds);
   embryo_program_native_call_add(ep, "date", _embryo_time_date);
   embryo_program_native_call_add(ep, "tzdate", _embryo_time_tzdate);
}

