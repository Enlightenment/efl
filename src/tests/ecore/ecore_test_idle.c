#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>

#include "ecore_suite.h"


static Eina_Bool
_quit_cb(void *data)
{
   Eina_Bool *val = data;
   if (val) *val = EINA_TRUE;
   ecore_main_loop_quit();
   return EINA_FALSE;
}

static Eina_Bool
_dummy_cb(void *data)
{
   return !!data;
}

EFL_START_TEST(ecore_test_ecore_main_loop_idler)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Idler *idler;

   idler = ecore_idler_add(_quit_cb, &did);
   fail_if(idler == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_main_loop_idle_enterer)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Idle_Enterer *idle_enterer;


   idle_enterer = ecore_idle_enterer_add(_quit_cb, &did);
   fail_if(idle_enterer == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_main_loop_idle_before_enterer)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Idle_Enterer *idle_enterer;


   idle_enterer = ecore_idle_enterer_before_add(_quit_cb, &did);
   fail_if(idle_enterer == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_main_loop_idle_exiter)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Timer *timer;
   Ecore_Idle_Exiter *idle_exiter;


   /* make system exit idle */
   timer = ecore_timer_add(0.0, _dummy_cb, (void *)(long)0);
   fail_if(timer == NULL);

   idle_exiter = ecore_idle_exiter_add(_quit_cb, &did);
   fail_if(idle_exiter == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

}
EFL_END_TEST


void ecore_test_ecore_idle(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_main_loop_idler);
   tcase_add_test(tc, ecore_test_ecore_main_loop_idle_enterer);
   tcase_add_test(tc, ecore_test_ecore_main_loop_idle_before_enterer);
   tcase_add_test(tc, ecore_test_ecore_main_loop_idle_exiter);
}
