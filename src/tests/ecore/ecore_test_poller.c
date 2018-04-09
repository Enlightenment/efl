#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>

#include "ecore_suite.h"

#define FP_ERR (0.0000001)
#define CHECK_FP(a, b) ((a - b) < FP_ERR)

static Eina_Bool
_quit_cb(void *data)
{
   Eina_Bool *val = data;
   if (val) *val = EINA_TRUE;
   ecore_main_loop_quit();
   return EINA_FALSE;
}

Eina_Bool _poller_handler(void *data)
{
   int *val = data;
   (*val)++;
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_poller_loop_add(void *data EINA_UNUSED)
{
   ecore_poller_add(ECORE_POLLER_CORE, 1, _quit_cb, NULL);
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_poller_loop_fail(void *data EINA_UNUSED)
{
   ecore_main_loop_quit();
   ck_abort();
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_poller_loop_del(void *data)
{
   static int count = 0;

   if (count++ == 0)
     ecore_poller_del(data);
   else
     ecore_main_loop_quit();
   return ECORE_CALLBACK_RENEW;
}

static Eina_Bool
_poller_loop_modify(void *data)
{
   ecore_poller_poller_interval_set(data, 1);
   return EINA_FALSE;
}

EFL_START_TEST(ecore_test_ecore_main_loop_poller_add)
{
   /* Create renewable main poller */
   Ecore_Poller *poll_ptr = ecore_poller_add(ECORE_POLLER_CORE, 1, _poller_loop_add, NULL);

   /* Enter main loop and wait for quit*/
   ecore_main_loop_begin();

   /* Destroy renewable main poller */
   ecore_poller_del(poll_ptr);
}
EFL_END_TEST


EFL_START_TEST(ecore_test_ecore_main_loop_poller_del)
{
   /* Create renewable main poller */
   Ecore_Poller *poller = ecore_poller_add(ECORE_POLLER_CORE, 2, _poller_loop_fail, NULL);
   ecore_poller_add(ECORE_POLLER_CORE, 1, _poller_loop_del, poller);

   /* Enter main loop and wait for quit*/
   ecore_main_loop_begin();
}
EFL_END_TEST

EFL_START_TEST(ecore_test_ecore_main_loop_poller_modify)
{
   /* Create renewable main poller */
   Ecore_Poller *poller = ecore_poller_add(ECORE_POLLER_CORE, 4, _quit_cb, NULL);
   ecore_poller_add(ECORE_POLLER_CORE, 1, _poller_loop_modify, poller);
   ecore_poller_add(ECORE_POLLER_CORE, 4, _poller_loop_fail, NULL);

   /* Enter main loop and wait for quit*/
   ecore_main_loop_begin();
}
EFL_END_TEST

Eina_Bool _poller_cb(void *data)
{
   int *val = data;
   (*val)++;
   return ECORE_CALLBACK_RENEW;
}

EFL_START_TEST(ecore_test_ecore_main_loop_poller)
{
   Ecore_Poller *poll1_ptr = NULL;
   int poll1_interval = 1;
   int poll1_counter = 0;

   Ecore_Poller *poll2_ptr = NULL;
   int poll2_interval = 2;
   int poll2_counter = 0;

   Ecore_Poller *poll3_ptr = NULL;
   int poll3_interval = 4;
   int poll3_counter = 0;

   Eina_Bool did = EINA_FALSE;

   /* Check ECORE_POLLER_CORE poll interval */
   double interval = 0.05;
   ecore_poller_poll_interval_set(ECORE_POLLER_CORE, interval);
   fail_unless(CHECK_FP(ecore_poller_poll_interval_get(ECORE_POLLER_CORE), interval));

   /* Create three pollers with different poller interval */
   poll1_ptr = ecore_poller_add(ECORE_POLLER_CORE, poll1_interval, _poller_cb, &poll1_counter);
   poll2_ptr = ecore_poller_add(ECORE_POLLER_CORE, poll2_interval, _poller_cb, &poll2_counter);
   poll3_ptr = ecore_poller_add(ECORE_POLLER_CORE, poll3_interval, _poller_cb, &poll3_counter);

   /* Add one time processed quit poller */
   ecore_poller_add(ECORE_POLLER_CORE, 8, _quit_cb, &did);

   /* Enter main loop and wait 8 intervals for quit */
   ecore_main_loop_begin();

   /* Check each poller poll interval */
   fail_if(ecore_poller_poller_interval_get(poll1_ptr) != poll1_interval);
   fail_if(ecore_poller_poller_interval_get(poll2_ptr) != poll2_interval);
   fail_if(ecore_poller_poller_interval_get(poll3_ptr) != poll3_interval);

   /* Check each poller call counter */
   ck_assert_int_eq(8, poll1_counter);
   ck_assert_int_eq(4, poll2_counter);
   ck_assert_int_eq(2, poll3_counter);

   /* Destroy renewable pollers */
   ecore_poller_del(poll3_ptr);
   ecore_poller_del(poll2_ptr);
   ecore_poller_del(poll1_ptr);

   fail_if(did == EINA_FALSE);

}
EFL_END_TEST

void ecore_test_ecore_poller(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_main_loop_poller);
   tcase_add_test(tc, ecore_test_ecore_main_loop_poller_add);
   tcase_add_test(tc, ecore_test_ecore_main_loop_poller_del);
   tcase_add_test(tc, ecore_test_ecore_main_loop_poller_modify);
}
