/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include "embryo_private.h"
#include <sys/time.h>
#include <time.h>

#ifndef HAVE_GETTIMEOFDAY
#ifdef WIN32
#include <sys/timeb.h>

static int gettimeofday (struct timeval *tv, void *unused)
{
   struct _timeb t;

   if (!tv)
      return -1;

   _ftime (&t);

   tv->tv_sec = t.time;
   tv->tv_usec = t.millitm * 1000;

   return 0;
}
#else
#error "Your platform isn't supported yet"
#endif
#endif

/* exported time api */

static Embryo_Cell
_embryo_time_seconds(Embryo_Program *ep, Embryo_Cell *params)
{
   struct timeval      timev;
   double t;
   float  f;
   
   gettimeofday(&timev, NULL);
   t = (double)(timev.tv_sec - ((timev.tv_sec / (60 * 60 * 24)) * (60 * 60 * 24)))
     + (((double)timev.tv_usec) / 1000000);
   f = (float)t;
   return EMBRYO_FLOAT_TO_CELL(f);
}

static Embryo_Cell
_embryo_time_date(Embryo_Program *ep, Embryo_Cell *params)
{
   struct timeval      timev;
   struct tm          *tm;
   time_t              tt;
   
   if (params[0] != (8 * sizeof(Embryo_Cell))) return 0;
   gettimeofday(&timev, NULL);
   tt = (time_t)(timev.tv_sec);
   tm = localtime(&tt);
   if (tm)
     {
	Embryo_Cell *cptr;
	double t;
	float  f;
	
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

/* functions used by the rest of embryo */

void
_embryo_time_init(Embryo_Program *ep)
{
   embryo_program_native_call_add(ep, "seconds", _embryo_time_seconds);
   embryo_program_native_call_add(ep, "date",    _embryo_time_date);
}
