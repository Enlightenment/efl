#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>

#include <Eina.h>
#include <Ecore.h>

#include "ecore_suite.h"

static int _log_dom;
#define INF(...) EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)

static Eina_Bool
_quit_cb(void *data)
{
   Eina_Bool *val = data;
   *val = EINA_TRUE;
   ecore_main_loop_quit();
   return EINA_FALSE;
}

static Eina_Bool
_dummy_cb(void *data)
{
   return !!data;
}

START_TEST(ecore_test_ecore_init)
{
   int ret;

   ret = ecore_init();
   fail_if(ret != 1);

   ret = ecore_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(ecore_test_ecore_main_loop)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Timer *timer;
   int ret;

   ret = ecore_init();
   fail_if(ret != 1);

   timer = ecore_timer_add(0.0, _quit_cb, &did);
   fail_if(timer == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(ecore_test_ecore_main_loop_idler)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Idler *idler;
   int ret;

   ret = ecore_init();
   fail_if(ret != 1);

   idler = ecore_idler_add(_quit_cb, &did);
   fail_if(idler == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(ecore_test_ecore_main_loop_idle_enterer)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Idle_Enterer *idle_enterer;
   int ret;

   ret = ecore_init();
   fail_if(ret != 1);

   idle_enterer = ecore_idle_enterer_add(_quit_cb, &did);
   fail_if(idle_enterer == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(ecore_test_ecore_main_loop_idle_exiter)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Timer *timer;
   Ecore_Idle_Exiter *idle_exiter;
   int ret;

   ret = ecore_init();
   fail_if(ret != 1);

   /* make system exit idle */
   timer = ecore_timer_add(0.0, _dummy_cb, (void *)(long)0);
   fail_if(timer == NULL);

   idle_exiter = ecore_idle_exiter_add(_quit_cb, &did);
   fail_if(idle_exiter == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
   fail_if(ret != 0);
}
END_TEST

START_TEST(ecore_test_ecore_main_loop_timer)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Timer *timer;
   double start, end, elapsed;
   int ret;

   ret = ecore_init();
   fail_if(ret != 1);

   timer = ecore_timer_add(2.0, _quit_cb, &did);
   fail_if(timer == NULL);

   start = ecore_time_get();
   ecore_main_loop_begin();
   end = ecore_time_get();
   elapsed = end - start;

   fail_if(did == EINA_FALSE);
   fail_if(elapsed < 2.0);
   fail_if(elapsed > 3.0); /* 1 second "error margin" */

   ret = ecore_shutdown();
   fail_if(ret != 0);
}
END_TEST

static Eina_Bool _timer3(void *data)
{
   /* timer 3, do nothing */
   return EINA_FALSE;
}

static Eina_Bool _timer2(void *data)
{
   /* timer 2, quit inner mainloop */
   ecore_main_loop_quit();
   return EINA_FALSE;
}

static Eina_Bool _timer1(void *data)
{
   /* timer 1, begin inner mainloop */
   int *times = data;
   (*times)++;

   ecore_timer_add(0.3, _timer2, NULL);
   ecore_timer_add(0.1, _timer3, NULL);
   ecore_main_loop_begin();

   ecore_main_loop_quit();

   return EINA_FALSE;
}

START_TEST(ecore_test_ecore_main_loop_timer_inner)
{
   Ecore_Timer *timer;
   int ret;
   int times = 0;

   ret = ecore_init();
   fail_if(ret != 1);

   timer = ecore_timer_add(1.0, _timer1, &times);
   fail_if(timer == NULL);

   /* BEGIN: outer mainloop */
   ecore_main_loop_begin();
   /*END: outer mainloop */

   fail_if(times != 1);
}
END_TEST

static Eina_Bool
_fd_handler_cb(void *data, Ecore_Fd_Handler *handler __UNUSED__)
{
   /* FIXME: why setting val if it is overwritten just after and what is its purpose ??? */
   Eina_Bool *val = data;

   *val = EINA_TRUE;
   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(ecore_test_ecore_main_loop_fd_handler)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Fd_Handler *fd_handler;
   int comm[2];
   int ret;

   ret = ecore_init();
   fail_if(ret != 1);

   ret = pipe(comm);
   fail_if(ret != 0);

   fd_handler = ecore_main_fd_handler_add
     (comm[0], ECORE_FD_READ, _fd_handler_cb, &did, NULL, NULL);
   fail_if(fd_handler == NULL);

   ret = write(comm[1], &did, 1);
   fail_if(ret != 1);

   ecore_main_loop_begin();

   close(comm[0]);
   close(comm[1]);

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
   fail_if(ret != 0);
}
END_TEST

static Eina_Bool
_event_handler_cb(void *data, int type __UNUSED__, void *event __UNUSED__)
{
   /* FIXME: why setting val if it is overwritten just after and what is its purpose ??? */
   Eina_Bool *val = data;

   *val = EINA_TRUE;
   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(ecore_test_ecore_main_loop_event)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Event_Handler *handler;
   Ecore_Event *event;
   int ret, type;

   ret = ecore_init();
   fail_if(ret != 1);

   type = ecore_event_type_new();
   fail_if(type < 1);

   handler = ecore_event_handler_add(type, _event_handler_cb, &did);
   fail_if(handler == NULL);

   event = ecore_event_add(type, NULL, NULL, NULL);
   fail_if(event == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
   fail_if(ret != 0);
}
END_TEST

static Eina_Bool
_timer_quit_recursive(void *data)
{
   INF("   _timer_quit_recursive: begin");
   ecore_main_loop_quit(); /* quits inner main loop */
   INF("   _timer_quit_recursive: end");
   return EINA_FALSE;
}

static Eina_Bool
_event_recursive_cb(void *data, int type, void *event)
{
   Ecore_Event *e;
   static int guard = 0;

   /* If we enter this callback more than once, it's wrong! */
   fail_if(guard != 0);
   guard++;

   INF("  event_recursive_cb: begin");

   ecore_timer_add(1.0, _timer_quit_recursive, NULL);
   INF("   add 1.0s timer (once) to trigger _timer_quit_recursive");

   INF("   inner main loop begin (recurse)");
   ecore_main_loop_begin();
   INF("   inner main loop end (recurse)");

   ecore_main_loop_quit(); /* quits outer main loop */

   INF("   guard = %d", guard);
   INF("  event_recursive_cb: end");
   return EINA_FALSE;
}


START_TEST(ecore_test_ecore_main_loop_event_recursive)
{
   /* This test tests if the event handlers are really called only once when
    * recursive main loops are used and any number of events may have occurred
    * between the beginning and the end of recursive main loop.
    */
   Ecore_Event *e;
   int type;
   int ret;

   _log_dom = eina_log_domain_register("test", EINA_COLOR_CYAN);

   INF("main: begin");
   ret = ecore_init();
   fail_if(ret != 1);


   type = ecore_event_type_new();
   ecore_event_handler_add(type, _event_recursive_cb, NULL);
   e = ecore_event_add(type, NULL, NULL, NULL);
   INF(" add event to trigger cb1: event=%p", e);
   INF(" main loop begin");
   ecore_main_loop_begin();
   INF(" main loop end");

   INF("main: end");
   ecore_shutdown();
}
END_TEST

void ecore_test_ecore(TCase *tc)
{
   tcase_add_test(tc, ecore_test_ecore_init);
   tcase_add_test(tc, ecore_test_ecore_main_loop);
   tcase_add_test(tc, ecore_test_ecore_main_loop_idler);
   tcase_add_test(tc, ecore_test_ecore_main_loop_idle_enterer);
   tcase_add_test(tc, ecore_test_ecore_main_loop_idle_exiter);
   tcase_add_test(tc, ecore_test_ecore_main_loop_timer);
   tcase_add_test(tc, ecore_test_ecore_main_loop_fd_handler);
   tcase_add_test(tc, ecore_test_ecore_main_loop_event);
   tcase_add_test(tc, ecore_test_ecore_main_loop_timer_inner);
   tcase_add_test(tc, ecore_test_ecore_main_loop_event_recursive);
}
