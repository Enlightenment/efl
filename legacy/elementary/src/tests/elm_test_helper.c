#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Ecore.h>
#include "elm_suite.h"

typedef struct _Callback_Data
{
   Ecore_Timer *timer;
   Eina_Bool did_timeout;
} Callback_Data;

static Eina_Bool
timer_expired_cb(void *user_data)
{
   Callback_Data *data = user_data;
   data->did_timeout = EINA_TRUE;
   data->timer = NULL;

   return ECORE_CALLBACK_CANCEL;
}

Eina_Bool
elm_test_helper_wait_flag(double in, Eina_Bool *done)
{
   Callback_Data data;

   data.did_timeout = EINA_FALSE;
   data.timer = ecore_timer_add(in, timer_expired_cb, &data);

   while (*done == EINA_FALSE && data.did_timeout == EINA_FALSE)
     ecore_main_loop_iterate();

   if (data.timer)
     ecore_timer_del(data.timer);

   return !data.did_timeout;
}
