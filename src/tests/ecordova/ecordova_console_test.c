#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecordova.h>

#include "ecordova_suite.h"

#include <stdbool.h>

static void
_setup(void)
{
   int ret = ecordova_init();
   ck_assert_int_eq(ret, 1);
}

static void
_teardown(void)
{
   int ret = ecordova_shutdown();
   ck_assert_int_eq(ret, 0);
}

static Ecordova_Device *
_console_new(void)
{
   fprintf(stderr, "Creating class\n"); fflush(stderr);
   return eo_add(ECORDOVA_CONSOLE_CLASS, NULL);
   fprintf(stderr, "Created class\n"); fflush(stderr);
}

START_TEST(smoke)
{
   Ecordova_Device *console = _console_new();
   eo_unref(console);
}
END_TEST

START_TEST(level)
{
   Ecordova_Device *console = _console_new();

   Ecordova_Console_LoggerLevel level = eo_do_ret(console,
                                                  level,
                                                  ecordova_console_level_get());
   ck_assert_int_eq(level, ECORDOVA_CONSOLE_LOGGERLEVEL_LOG); // default value

   for (Ecordova_Console_LoggerLevel new_level = ECORDOVA_CONSOLE_LOGGERLEVEL_LOG;
        new_level < ECORDOVA_CONSOLE_LOGGERLEVEL_LAST;
        ++new_level)
     {
        eo_do(console, ecordova_console_level_set(new_level));
        level = eo_do_ret(console, level, ecordova_console_level_get());
        ck_assert_int_eq(level, new_level);
     }

   eo_unref(console);
}
END_TEST

START_TEST(console_use)
{
   Ecordova_Device *console = _console_new();

   Eina_Bool actual_value = eo_do_ret(console,
                                      actual_value,
                                      ecordova_console_use_get());
   ck_assert_int_eq(actual_value, EINA_FALSE); // default value

   Eina_Bool expected_value = !actual_value;
   eo_do(console, ecordova_console_use_set(expected_value));
   actual_value = eo_do_ret(console, actual_value, ecordova_console_use_get());
   ck_assert_int_eq(expected_value, actual_value);

   expected_value = !actual_value;
   eo_do(console, ecordova_console_use_set(expected_value));
   actual_value = eo_do_ret(console, actual_value, ecordova_console_use_get());
   ck_assert_int_eq(expected_value, actual_value);

   eo_unref(console);
}
END_TEST

START_TEST(logger_use)
{
   Ecordova_Device *console = _console_new();

   Eina_Bool actual_value = eo_do_ret(console,
                                      actual_value,
                                      ecordova_console_logger_use_get());
   ck_assert_int_eq(actual_value, EINA_TRUE); // default value

   Eina_Bool expected_value = !actual_value;
   eo_do(console, ecordova_console_logger_use_set(expected_value));
   actual_value = eo_do_ret(console, actual_value, ecordova_console_logger_use_get());
   ck_assert_int_eq(expected_value, actual_value);

   expected_value = !actual_value;
   eo_do(console, ecordova_console_logger_use_set(expected_value));
   actual_value = eo_do_ret(console, actual_value, ecordova_console_logger_use_get());
   ck_assert_int_eq(expected_value, actual_value);

   eo_unref(console);
}
END_TEST

START_TEST(logging)
{
   Ecordova_Device *console = _console_new();

   eo_do(console, ecordova_console_logger_use_set(EINA_TRUE));
   eo_do(console, ecordova_console_use_set(EINA_TRUE));

   for (Ecordova_Console_LoggerLevel new_level = ECORDOVA_CONSOLE_LOGGERLEVEL_LOG;
        new_level < ECORDOVA_CONSOLE_LOGGERLEVEL_LAST;
        ++new_level)
     {
        eo_do(console, ecordova_console_level_set(new_level));

        eo_do(console, ecordova_console_log("log"));
        eo_do(console, ecordova_console_error("error"));
        eo_do(console, ecordova_console_warn("warn"));
        eo_do(console, ecordova_console_info("info"));
        eo_do(console, ecordova_console_debug("debug"));
     }

   eo_unref(console);
}
END_TEST

void
ecordova_console_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, level);
   tcase_add_test(tc, logger_use);
   tcase_add_test(tc, console_use);
   tcase_add_test(tc, logging);
}
