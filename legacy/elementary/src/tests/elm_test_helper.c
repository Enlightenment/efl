#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Ecore.h>
#include "elm_suite.h"

static Eina_Bool
timer_expired_cb(void *user_data)
{
   Eina_Bool *did_timeout = user_data;

   *did_timeout = EINA_TRUE;
   ecore_main_loop_quit();

   return EINA_TRUE;
}

static Eina_Bool
idler_done_cb(void *user_data)
{
   Eina_Bool *done = user_data;

   if (*done) ecore_main_loop_quit();

   return EINA_TRUE;
}

Eina_Bool
elm_test_helper_wait_flag(double in, Eina_Bool *done)
{
   Eina_Bool did_timeout = EINA_FALSE;
   Ecore_Timer *tm;
   Ecore_Idle_Enterer *idle;

   tm = ecore_timer_add(in, timer_expired_cb, &did_timeout);
   idle = ecore_idle_enterer_add(idler_done_cb, done);

   ecore_main_loop_begin();

   ecore_idle_enterer_del(idle);
   ecore_timer_del(tm);

   return !did_timeout;
}
