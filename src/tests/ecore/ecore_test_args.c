#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include "ecore_suite.h"

static Eina_Bool
_cb_args1(void *data EINA_UNUSED, const Eo_Event *event)
{
   Efl_Loop_Args *args = event->info;
   int n;

   n = efl_loop_args_arg_num_get(args);
   fail_if(n != 8);
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 0), "a"));
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 1), "b"));
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 2), "c"));
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 3), "d"));
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 4), "e"));
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 5), "f"));
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 6), "g"));
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 7), "h"));
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

START_TEST(ecore_test_args1)
{
   const char *args[] =
     {
        "a", "b", "c", "d", "e", "f", "g", "h"
     };

   ecore_init();
   eo_event_callback_add(ecore_main_loop_get(), EFL_LOOP_EVENT_ARGS,
                        _cb_args1, NULL);
   efl_loop_args_add(ecore_main_loop_get(), 8, args);
   ecore_main_loop_begin();
   ecore_shutdown();
}
END_TEST

static Eina_Bool
_cb_args2(void *data EINA_UNUSED, const Eo_Event *event)
{
   Efl_Loop_Args *args = event->info;
   int n;

   n = efl_loop_args_arg_num_get(args);
   fail_if(n != 1);
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 0), "hello world"));
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

START_TEST(ecore_test_args2)
{
   const char *args[] =
     {
        "hello world"
     };

   ecore_init();
   eo_event_callback_add(ecore_main_loop_get(), EFL_LOOP_EVENT_ARGS,
                        _cb_args2, NULL);
   efl_loop_args_add(ecore_main_loop_get(), 1, args);
   ecore_main_loop_begin();
   ecore_shutdown();
}
END_TEST

static Eina_Bool
_cb_args3(void *data EINA_UNUSED, const Eo_Event *event)
{
   Efl_Loop_Args *args = event->info;
   int n;

   n = efl_loop_args_arg_num_get(args);
   fail_if(n != 0);
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

START_TEST(ecore_test_args3)
{
   ecore_init();
   eo_event_callback_add(ecore_main_loop_get(), EFL_LOOP_EVENT_ARGS,
                        _cb_args3, NULL);
   efl_loop_args_add(ecore_main_loop_get(), 0, NULL);
   ecore_main_loop_begin();
   ecore_shutdown();
}
END_TEST

static Eina_Bool
_cb_args4(void *data EINA_UNUSED, const Eo_Event *event)
{
   Efl_Loop_Args *args = event->info;
   int n;

   n = efl_loop_args_arg_num_get(args);
   fail_if(n != 3);
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 0), "some really long string with lots more to it than is needed for an argument blah"));
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 1), "xxxxx"));
   fail_if(!!strcmp(efl_loop_args_arg_get(args, 2), "y"));
   ecore_main_loop_quit();
   return EO_CALLBACK_CONTINUE;
}

START_TEST(ecore_test_args4)
{
   const char *args[] =
     {
        "some really long string with lots more to it than is needed for an argument blah",
        "xxxxx",
        "y"
     };

   ecore_init();
   eo_event_callback_add(ecore_main_loop_get(), EFL_LOOP_EVENT_ARGS,
                        _cb_args4, NULL);
   efl_loop_args_add(ecore_main_loop_get(), 3, args);
   ecore_main_loop_begin();
   ecore_shutdown();
}
END_TEST

void ecore_test_ecore_args(TCase *tc)
{
   tcase_add_test(tc, ecore_test_args1);
   tcase_add_test(tc, ecore_test_args2);
   tcase_add_test(tc, ecore_test_args3);
   tcase_add_test(tc, ecore_test_args4);
}
