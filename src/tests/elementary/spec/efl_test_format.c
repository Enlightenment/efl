#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifdef _WIN32
# include <evil_private.h> /* setlocale strptime */
#endif

#define EFL_UI_FORMAT_PROTECTED /* To access internal methods */

#include <Efl_Ui.h>
#include "efl_ui_spec_suite.h"
#include "suite_helpers.h"

/* spec-meta-start
      {"test-interface":"Efl.Ui.Format",
       "test-widgets": ["Efl.Ui.Progressbar", "Efl.Ui.Calendar", "Efl.Ui.Tags", "Efl.Ui.Spin", "Efl.Ui.Spin_Button"]}
   spec-meta-end */

static const Efl_Ui_Format_Value values[] = {{15, "fifteen"}, {16, "sixteen"}, {17, "seventeen"}, {18, "eighteen"}};

EFL_START_TEST(format_values)
{
  Eina_Strbuf *buf = eina_strbuf_new();
  Eina_Value eina_val;

  efl_ui_format_values_set(widget, EINA_C_ARRAY_ACCESSOR_PTR_NEW(values));
  eina_val = eina_value_int_init(17);
  efl_ui_format_formatted_value_get(widget, buf, eina_val);
  ck_assert_str_eq(eina_strbuf_string_get(buf), "seventeen"); // Check that value works
  eina_val = eina_value_float_init(16.f);
  efl_ui_format_formatted_value_get(widget, buf, eina_val);
  ck_assert_str_eq(eina_strbuf_string_get(buf), "sixteen"); // Check built-in conversion

  eina_value_flush(&eina_val);
  eina_strbuf_free(buf);
}
EFL_END_TEST

static Eina_Bool
_format_func(void *data, Eina_Strbuf *str, const Eina_Value value)
{
  int i = *(int *)data;
  int v;
  ck_assert_int_eq(i, 1234); // Check that data ptr is passed along correctly
  if (eina_value_type_get(&value) != EINA_VALUE_TYPE_INT) return EINA_FALSE;
  eina_value_get(&value, &v);
  eina_strbuf_append_printf(str, "You said '%d'", v);

  return EINA_TRUE;
}

static void
_format_free_func(void *data)
{
  int i = *(int *)data;
  ck_assert_int_eq(i, 1234); // Check that data ptr is passed along correctly
  *(int *)data = 12345; // Change it to indicate that free func was called
}

EFL_START_TEST(format_func)
{
  int data = 1234;
  Eina_Strbuf *buf = eina_strbuf_new();
  Eina_Value eina_val;

  efl_ui_format_func_set(widget, &data, _format_func, _format_free_func);
  eina_val = eina_value_int_init(15);
  efl_ui_format_formatted_value_get(widget, buf, eina_val);
  ck_assert_str_eq(eina_strbuf_string_get(buf), "You said '15'"); // Check that format func works
  efl_ui_format_func_set(widget, NULL, NULL, NULL);
  ck_assert_int_eq(data, 12345); // Check that free func is called
  eina_value_flush(&eina_val);

  eina_strbuf_free(buf);
}
EFL_END_TEST

EFL_START_TEST(format_string)
{
  Eina_Strbuf *buf = eina_strbuf_new();
  Eina_Value eina_val;
  struct tm t = { 0 };
  const char *old_locale = setlocale(LC_TIME, NULL);

#define CHECK(fmt_str, fmt_type, val, val_type, result) \
  efl_ui_format_string_set(widget, fmt_str, fmt_type); \
  eina_value_setup(&eina_val, val_type); \
  eina_value_set(&eina_val, val); \
  efl_ui_format_formatted_value_get(widget, buf, eina_val); \
  ck_assert_str_eq(eina_strbuf_string_get(buf), result); \
  eina_value_flush(&eina_val)

  CHECK("%d", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, 1234, EINA_VALUE_TYPE_INT, "1234");
  CHECK("%d", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, 1234.f, EINA_VALUE_TYPE_FLOAT, "1234"); // built-in conversion
  CHECK("%d units", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, 1234, EINA_VALUE_TYPE_INT, "1234 units"); // complex format

  CHECK("%.0f", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, 12.34f, EINA_VALUE_TYPE_FLOAT, "12");
  CHECK("%.1f", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, 12.34f, EINA_VALUE_TYPE_FLOAT, "12.3");
  CHECK("%.2f", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, 12.34f, EINA_VALUE_TYPE_FLOAT, "12.34");
  CHECK("%.2f", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, 1234, EINA_VALUE_TYPE_INT, "1234.00"); // built-in conversion
  CHECK("%.0f%%", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, 75.f, EINA_VALUE_TYPE_FLOAT, "75%"); // complex format

  CHECK("%s", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, "Hello!", EINA_VALUE_TYPE_STRING, "Hello!");
  CHECK("%s", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, 1234, EINA_VALUE_TYPE_INT, "1234"); // built-in conversion
  CHECK("He said '%s'", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, "Hello!", EINA_VALUE_TYPE_STRING, "He said 'Hello!'"); // complex format

  CHECK("Static string", EFL_UI_FORMAT_STRING_TYPE_SIMPLE, 1234, EINA_VALUE_TYPE_INT, "Static string");

  strptime("2019 7 3 11:49:3", "%Y %m %d %H:%M:%S", &t);
  setlocale(LC_TIME, "C");
  CHECK("%F", EFL_UI_FORMAT_STRING_TYPE_TIME, t, EINA_VALUE_TYPE_TM, "2019-07-03");
  CHECK("%T", EFL_UI_FORMAT_STRING_TYPE_TIME, t, EINA_VALUE_TYPE_TM, "11:49:03");
  CHECK("%A", EFL_UI_FORMAT_STRING_TYPE_TIME, t, EINA_VALUE_TYPE_TM, "Wednesday");
  CHECK("<%B %Y>", EFL_UI_FORMAT_STRING_TYPE_TIME, t, EINA_VALUE_TYPE_TM, "<July 2019>");
  setlocale(LC_TIME, old_locale);

  eina_strbuf_free(buf);
#undef CHECK
}
EFL_END_TEST

static Eina_Bool
_partial_format_func(void *data EINA_UNUSED, Eina_Strbuf *str, const Eina_Value value)
{
  int v;
  if (eina_value_type_get(&value) != EINA_VALUE_TYPE_INT) return EINA_FALSE;
  eina_value_get(&value, &v);
  if (v < 10)
     {
       eina_strbuf_append_printf(str, "You said '%d'", v);
       return EINA_TRUE;
     }
  return EINA_FALSE;
}

EFL_START_TEST(format_mixed)
{
  Eina_Strbuf *buf = eina_strbuf_new();
  Eina_Value eina_val;

  // Now we check combinations of different options
  // Each one should be used in turn when the previous one fails: values, func, string, fallback
  efl_ui_format_values_set(widget, EINA_C_ARRAY_ACCESSOR_PTR_NEW(values));
  efl_ui_format_func_set(widget, NULL, _partial_format_func, NULL);
  efl_ui_format_string_set(widget, "%d rabbits", EFL_UI_FORMAT_STRING_TYPE_SIMPLE);

  eina_val = eina_value_int_init(1);
  efl_ui_format_formatted_value_get(widget, buf, eina_val);
  ck_assert_str_eq(eina_strbuf_string_get(buf), "You said '1'"); // Func
  eina_value_set(&eina_val, 15);
  efl_ui_format_formatted_value_get(widget, buf, eina_val);
  ck_assert_str_eq(eina_strbuf_string_get(buf), "fifteen"); // Values
  eina_value_set(&eina_val, 25);
  efl_ui_format_formatted_value_get(widget, buf, eina_val);
  ck_assert_str_eq(eina_strbuf_string_get(buf), "25 rabbits"); // Values

  EXPECT_ERROR_START;
  // This is an invalid format string (it has two placeholders) which should
  // trigger the fallback mechanism
  efl_ui_format_string_set(widget, "%d %d", EFL_UI_FORMAT_STRING_TYPE_SIMPLE);
  EXPECT_ERROR_END;
  efl_ui_format_formatted_value_get(widget, buf, eina_val);
  ck_assert_str_eq(eina_strbuf_string_get(buf), "25"); // Fallback

  eina_value_flush(&eina_val);
  eina_strbuf_free(buf);
}
EFL_END_TEST

void
efl_ui_format_behavior_test(TCase *tc)
{
   tcase_add_test(tc, format_values);
   tcase_add_test(tc, format_func);
   tcase_add_test(tc, format_string);
   tcase_add_test(tc, format_mixed);
}
