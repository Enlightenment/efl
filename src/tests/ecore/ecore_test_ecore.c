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

#define ECORE_EVENT_CUSTOM_1 1
#define ECORE_EVENT_CUSTOM_2 2

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
   fail_if(ret < 1);

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
   fail_if(ret < 1);

   timer = ecore_timer_add(0.0, _quit_cb, &did);
   fail_if(timer == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
}
END_TEST

START_TEST(ecore_test_ecore_main_loop_idler)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Idler *idler;
   int ret;

   ret = ecore_init();
   fail_if(ret < 1);

   idler = ecore_idler_add(_quit_cb, &did);
   fail_if(idler == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
}
END_TEST

START_TEST(ecore_test_ecore_main_loop_idle_enterer)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Idle_Enterer *idle_enterer;
   int ret;

   ret = ecore_init();
   fail_if(ret < 1);

   idle_enterer = ecore_idle_enterer_add(_quit_cb, &did);
   fail_if(idle_enterer == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
}
END_TEST

START_TEST(ecore_test_ecore_main_loop_idle_exiter)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Timer *timer;
   Ecore_Idle_Exiter *idle_exiter;
   int ret;

   ret = ecore_init();
   fail_if(ret < 1);

   /* make system exit idle */
   timer = ecore_timer_add(0.0, _dummy_cb, (void *)(long)0);
   fail_if(timer == NULL);

   idle_exiter = ecore_idle_exiter_add(_quit_cb, &did);
   fail_if(idle_exiter == NULL);

   ecore_main_loop_begin();

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
}
END_TEST

START_TEST(ecore_test_ecore_main_loop_timer)
{
   Eina_Bool did = EINA_FALSE;
   Ecore_Timer *timer;
   double start, end, elapsed;
   int ret;

   ret = ecore_init();
   fail_if(ret < 1);

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
}
END_TEST

static Eina_Bool _timer3(void *data EINA_UNUSED)
{
   /* timer 3, do nothing */
   return EINA_FALSE;
}

static Eina_Bool _timer2(void *data EINA_UNUSED)
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
   fail_if(ret < 1);

   timer = ecore_timer_add(1.0, _timer1, &times);
   fail_if(timer == NULL);

   /* BEGIN: outer mainloop */
   ecore_main_loop_begin();
   /*END: outer mainloop */

   fail_if(times != 1);
}
END_TEST

static Eina_Bool
_fd_handler_cb(void *data, Ecore_Fd_Handler *handler EINA_UNUSED)
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
   fail_if(ret < 1);

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
}
END_TEST

static Eina_Bool
_event_handler_cb(void *data, int type, void *event)
{
   int *did = data;

   int t1 = type;
   int *e1 = event;

   int t2 = ecore_event_current_type_get();
   int *e2 = ecore_event_current_event_get();

   if (t1 == t2)
      (*did)++;

   if (*e1 == *e2 && (ECORE_EVENT_CUSTOM_1 == *e1 || ECORE_EVENT_CUSTOM_2 == *e1))
      (*did)++;

   return EINA_TRUE;
}

static Eina_Bool
_event_handler_cb2(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   int *did = data;
   (*did)++;
   return EINA_TRUE;
}

static Eina_Bool
_event_handler_cb3(void *data, int type EINA_UNUSED, void *event EINA_UNUSED)
{
   int *did = data;
   (*did)++;
   return EINA_TRUE;
}

static int *
_event_new(int id)
{
   int *ev = malloc(sizeof(int));

   switch(id)
   {
   case ECORE_EVENT_CUSTOM_1:
      *ev = ECORE_EVENT_CUSTOM_1;
      break;

   case ECORE_EVENT_CUSTOM_2:
      *ev = ECORE_EVENT_CUSTOM_2;
      break;

   default:
      *ev = ECORE_EVENT_NONE;
   }

   return ev;
}

static void
_event_free(void *user_data, void *func_data)
{
   int *did = user_data;
   int *ev = func_data;

   if (ECORE_EVENT_CUSTOM_1 == *ev || ECORE_EVENT_CUSTOM_2 == *ev)
      (*did)++;

   free(ev);
}

static void*
_filter_start(void *data)
{
   int *did = data;
   (*did)++;
   return NULL;
}

static Eina_Bool
_filter(void *data, void *loop_data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Eina_Bool res = EINA_TRUE;
   int *did = data;
   int *ev = event;

   if (NULL != event)
   {
      /* Ignore second event */
      if (ECORE_EVENT_CUSTOM_2 == *ev)
      {
         res = EINA_FALSE;
      }
   }

   (*did)++;

   return res;
}

static void
_filter_end(void *user_data, void *func_data EINA_UNUSED)
{
   int *did = user_data;
   (*did)++;
}

START_TEST(ecore_test_ecore_main_loop_event)
{
   Ecore_Event_Handler *handler, *handler2, *handler3;
   Ecore_Event_Filter *filter_handler;
   Ecore_Event *event;
   int res_counter;
   int type, type2;
   int *ev = NULL;
   int did = 0;
   int filter = 0;

   res_counter = ecore_init();
   fail_if(res_counter < 1);

   /* Create 2 new event types */
   type = ecore_event_type_new();
   fail_if(type < 1);

   type2 = ecore_event_type_new();
   fail_if(type < 1);

   /* Add handler for new type of event */
   handler = ecore_event_handler_add(type, _event_handler_cb, &did);
   fail_if(handler == NULL);

   /* Add another handler for event which will be deleted in next step */
   handler2 = ecore_event_handler_add(type, _event_handler_cb2, &did);
   fail_if(handler2 == NULL);

   /* Add handler for event which will be filtered */
   handler3 = ecore_event_handler_add(type2, _event_handler_cb3, &did);
   fail_if(handler3 == NULL);

   /* Add filtering mechanism */
   filter_handler = ecore_event_filter_add(_filter_start, _filter, _filter_end, &filter);

   /* Add into main loop three events: one to process, one to filter, one to quit */
   ev = _event_new(ECORE_EVENT_CUSTOM_1);
   event = ecore_event_add(type, ev, _event_free, &did);
   fail_if(event == NULL);

   ev = _event_new(ECORE_EVENT_CUSTOM_2);
   event = ecore_event_add(type2, ev, _event_free, &did);
   fail_if(event == NULL);

   event = ecore_event_add(ECORE_EVENT_SIGNAL_EXIT, NULL, NULL, NULL);
   fail_if(event == NULL);

   ecore_main_loop_begin();

   /*
      Check internal fail cases:
       event_cbx - 3 increments (4th should be ignored)
       free      - 2 increments
   */
   fail_if(did != 3 + 2); // 5

   /*
      Check filter procedures calls:
       start  - 1 call
       filter - 3 calls
       end    - 1 call
   */
   fail_if(filter != 1 + 3 + 1); // 5

   /* New loop but with new data and without filter and one callback procedure */
   int did2 = 0;
   filter = 0;

   int *old = ecore_event_handler_data_set(handler, &did2);
   int *new = ecore_event_handler_data_get(handler);

   ecore_event_handler_del(handler2);
   ecore_event_filter_del(filter_handler);

   fail_if(*old != did);
   fail_if(*new != did2);

   ev = _event_new(ECORE_EVENT_CUSTOM_1);
   event = ecore_event_add(type, ev, _event_free, &did2);
   fail_if(event == NULL);

   event = ecore_event_add(ECORE_EVENT_SIGNAL_EXIT, NULL, NULL, NULL);
   fail_if(event == NULL);

   ecore_main_loop_begin();

   /*
      Check internal fail cases:
       event_cb - 2 increments in first callback (another one was deleted)
       free     - 1 increment
   */
   fail_if(did2 != 2 + 1); // 3

   /* Filter counter shouldn't change */
   fail_if(filter != 0); // 0

   res_counter = ecore_shutdown();
}
END_TEST

static Eina_Bool
_timer_quit_recursive(void *data EINA_UNUSED)
{
   INF("   _timer_quit_recursive: begin");
   ecore_main_loop_quit(); /* quits inner main loop */
   INF("   _timer_quit_recursive: end");
   return EINA_FALSE;
}

static Eina_Bool
_event_recursive_cb(void *data EINA_UNUSED, int type EINA_UNUSED, void *event EINA_UNUSED)
{
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
   fail_if(ret < 1);


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

START_TEST(ecore_test_ecore_app)
{
   int ret;

   int argc_in = 2;
   const char *argv_in[] = {"arg_str_1", "arg_str2"};

   int argc_out = 0;
   char **argv_out = NULL;

   ret = ecore_init();
   fail_if(ret < 1);

   ecore_app_args_set(argc_in, argv_in);
   ecore_app_args_get(&argc_out, &argv_out);

   fail_if(argc_in != argc_out);
   int i;
   for (i = 0; i < argc_out; i++)
     {
        fail_if( 0 != strcmp(argv_in[i], argv_out[i]) );
     }

   ret = ecore_shutdown();
}
END_TEST

Eina_Bool _poller_cb(void *data)
{
   int *val = data;
   (*val)++;
   return ECORE_CALLBACK_RENEW;
}

START_TEST(ecore_test_ecore_main_loop_poller)
{
   int ret;

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

   ret = ecore_init();
   fail_if(ret < 1);

   /* Check ECORE_POLLER_CORE poll interval */
   double interval = 1.0;
   ecore_poller_poll_interval_set(ECORE_POLLER_CORE, interval);
   fail_unless(CHECK_FP(ecore_poller_poll_interval_get(ECORE_POLLER_CORE), interval));

   /* Create three pollers with different poller interval */
   poll1_ptr = ecore_poller_add(ECORE_POLLER_CORE, poll1_interval, _poller_cb, &poll1_counter);
   poll2_ptr = ecore_poller_add(ECORE_POLLER_CORE, poll2_interval, _poller_cb, &poll2_counter);
   poll3_ptr = ecore_poller_add(ECORE_POLLER_CORE, poll3_interval, _poller_cb, &poll3_counter);

   /* Add one time processed quit poller */
   ecore_poller_add(ECORE_POLLER_CORE, 8, _quit_cb, &did);

   /* Enter main loop and wait 8 seconds for quit */
   ecore_main_loop_begin();

   /* Check each poller poll interval */
   fail_if(ecore_poller_poller_interval_get(poll1_ptr) != poll1_interval);
   fail_if(ecore_poller_poller_interval_get(poll2_ptr) != poll2_interval);
   fail_if(ecore_poller_poller_interval_get(poll3_ptr) != poll3_interval);

   /* Check each poller call counter */
   fail_if(8 != poll1_counter);
   fail_if(4 != poll2_counter);
   fail_if(2 != poll3_counter);

   /* Destroy renewable pollers */
   ecore_poller_del(poll3_ptr);
   ecore_poller_del(poll2_ptr);
   ecore_poller_del(poll1_ptr);

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
}
END_TEST

Eina_Bool _poller_handler(void *data)
{
   int *val = data;
   (*val)++;
   return ECORE_CALLBACK_RENEW;
}

Eina_Bool _poller_loop(void *data)
{
   int *res = data;

   static Ecore_Poller *poll_ptr = NULL;
   static int count = 0;

   switch (count)
      {
      case 2:
         poll_ptr = ecore_poller_add(ECORE_POLLER_CORE, 2, _poller_handler, res);
         break;
      case 6:
         ecore_poller_poller_interval_set(poll_ptr, 1);
         break;
      case 10:
         ecore_poller_del(poll_ptr);
         break;
      default:
         // do nothing
         break;
      }
   count++;
   return ECORE_CALLBACK_RENEW;
}

START_TEST(ecore_test_ecore_main_loop_poller_add_del)
{
   int ret, count_res = 0;

   Eina_Bool did = EINA_FALSE;

   ret = ecore_init();
   fail_if(ret < 1);

   /* Create renewable main poller */
   Ecore_Poller *poll_ptr = ecore_poller_add(ECORE_POLLER_CORE, 1, _poller_loop, &count_res);

   /* One time processed poller */
   ecore_poller_add(ECORE_POLLER_CORE, 16, _quit_cb, &did);

   /* Enter main loop and wait for quit*/
   ecore_main_loop_begin();

   fprintf(stderr, "count_res: %i\n", count_res);
   /* Validation call counter */
   fail_if(6 != count_res);

   /* Destroy renewable main poller */
   ecore_poller_del(poll_ptr);

   fail_if(did == EINA_FALSE);

   ret = ecore_shutdown();
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
   tcase_add_test(tc, ecore_test_ecore_app);
   tcase_add_test(tc, ecore_test_ecore_main_loop_poller);
   tcase_add_test(tc, ecore_test_ecore_main_loop_poller_add_del);
}
