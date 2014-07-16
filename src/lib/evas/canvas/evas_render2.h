#ifndef EVAS_RENDER2_H
#define EVAS_RENDER2_H
#include "evas_common_private.h"
#include "evas_private.h"
#include <math.h>
#include <assert.h>
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

#ifdef EVAS_RENDER_DEBUG_TIMING
#include <sys/time.h>
#endif

/* render stuff here */
void _evas_render2_stage_generate_object_updates(Evas_Public_Data *e);

#ifndef _WIN32
static inline double
get_time(void)
{
   struct timeval      timev;
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}
#else
static inline double
get_time(void)
{
   return (double)GetTickCount()/1000.0;
}
#endif

static inline void
out_time(double t)
{
   double b;

   b = (t * 100.0) / (1.0 / 60.0);
   printf("%1.8fs (%1.2f%% 60fps budget)\n", t, b);
}

#endif
