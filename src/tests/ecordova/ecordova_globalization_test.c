#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecordova.h>

#include "ecordova_suite.h"

#include <check.h>

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

#ifdef HAVE_TIZEN_CONFIGURATION_MANAGER
static Ecordova_Device *
_globalization_new(void)
{
   return eo_add(ECORDOVA_GLOBALIZATION_CLASS,
                 NULL,
                 ecordova_globalization_constructor());
}

START_TEST(smoke)
{
   Ecordova_Device *globalization = _globalization_new();
   eo_unref(globalization);
}
END_TEST

static Eina_Bool
_preferred_language_get_cb(void *data,
                           Eo *obj EINA_UNUSED,
                           const Eo_Event_Description *desc EINA_UNUSED,
                           void *event_info)
{
   fail_if(NULL == data);
   char **language = data;
   fail_if(NULL == event_info);
   Ecordova_Globalization_Language *glanguage = event_info;

   *language = strdup(glanguage->value);
   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(preferred_language_get)
{
   Ecordova_Device *globalization = _globalization_new();

   char *language = NULL;
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_PREFERRED_LANGUAGE_SUCCESS,
                               _preferred_language_get_cb,
                               &language));
   eo_do(globalization, ecordova_globalization_preferred_language_get());
   fail_if(NULL == language);

   DBG("language: %s", language);
   free(language);

   eo_unref(globalization);
}
END_TEST

static Eina_Bool
_locale_name_get_cb(void *data,
                    Eo *obj EINA_UNUSED,
                    const Eo_Event_Description *desc EINA_UNUSED,
                    void *event_info)
{
   fail_if(NULL == data);
   char **locale = data;
   fail_if(NULL == event_info);
   Ecordova_Globalization_Locale *glocale = event_info;

   *locale = strdup(glocale->value);
   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(locale_name_get)
{
   Ecordova_Device *globalization = _globalization_new();

   char *locale = NULL;
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_LOCALE_NAME_SUCCESS,
                               _locale_name_get_cb,
                               &locale));
   eo_do(globalization, ecordova_globalization_locale_name_get());
   fail_if(NULL == locale);

   DBG("locale: %s", locale);
   free(locale);

   eo_unref(globalization);
}
END_TEST

static Eina_Bool
_to_string_cb(void *data,
                   Eo *obj EINA_UNUSED,
                   const Eo_Event_Description *desc EINA_UNUSED,
                   void *event_info)
{
   fail_if(NULL == data);
   char **string = data;
   fail_if(NULL == event_info);
   Ecordova_Globalization_String *gstring = event_info;

   *string = strdup(gstring->value);
   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(date_to_string)
{
   Ecordova_Device *globalization = _globalization_new();

   time_t date_time;
   time(&date_time);

   char *string = NULL;
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_DATE_TO_STRING_SUCCESS,
                               _to_string_cb,
                               &string));
   eo_do(globalization, ecordova_globalization_date_to_string(date_time, NULL));
   fail_if(NULL == string);

   DBG("string: %s", string);
   free(string);

   eo_unref(globalization);
}
END_TEST

typedef struct {
  bool success;
  Ecordova_Globalization_CurrencyPattern pattern;
} CurrencyPatternData;

static Eina_Bool
_currency_pattern_get_cb(void *data,
                         Eo *obj EINA_UNUSED,
                         const Eo_Event_Description *desc EINA_UNUSED,
                         void *event_info)
{
   fail_if(NULL == data);
   CurrencyPatternData *currency_pattern_data = data;
   fail_if(NULL == event_info);
   Ecordova_Globalization_CurrencyPattern *currency_pattern = event_info;

   currency_pattern_data->success = true;
   currency_pattern_data->pattern = (Ecordova_Globalization_CurrencyPattern){
      .pattern = currency_pattern->pattern ? strdup(currency_pattern->pattern) : NULL,
      .code = currency_pattern->code ? strdup(currency_pattern->code) : NULL,
      .fraction = currency_pattern->fraction,
      .rounding = currency_pattern->rounding,
      .decimal = currency_pattern->decimal ? strdup(currency_pattern->decimal) : NULL,
      .grouping = currency_pattern->grouping ? strdup(currency_pattern->grouping) : NULL
   };

   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(currency_pattern_get)
{
   Ecordova_Device *globalization = _globalization_new();

   CurrencyPatternData currency_pattern_data = {0};
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_CURRENCY_PATTERN_SUCCESS,
                               _currency_pattern_get_cb,
                               &currency_pattern_data));
   eo_do(globalization, ecordova_globalization_currency_pattern_get(NULL));
   fail_unless(currency_pattern_data.success);

   DBG("pattern: %s", currency_pattern_data.pattern.pattern);
   DBG("code: %s", currency_pattern_data.pattern.code);
   DBG("fraction: %d", currency_pattern_data.pattern.fraction);
   DBG("rounding: %f", currency_pattern_data.pattern.rounding);
   DBG("decimal: %s", currency_pattern_data.pattern.decimal);
   DBG("grouping: %s", currency_pattern_data.pattern.grouping);
   free((char*)currency_pattern_data.pattern.pattern);
   free((char*)currency_pattern_data.pattern.code);
   free((char*)currency_pattern_data.pattern.decimal);
   free((char*)currency_pattern_data.pattern.grouping);

   eo_unref(globalization);
}
END_TEST

typedef struct {
  bool success;
  Ecordova_Globalization_DateNames names;
} DateNamesData;

static Eina_Bool
_date_names_get_cb(void *data,
                   Eo *obj EINA_UNUSED,
                   const Eo_Event_Description *desc EINA_UNUSED,
                   void *event_info)
{
   fail_if(NULL == data);
   DateNamesData *date_names_data = data;
   fail_if(NULL == event_info);
   Ecordova_Globalization_DateNames *date_names = event_info;
   fail_if(NULL == date_names->value);
   date_names_data->success = true;

   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(date_names_get)
{
   Ecordova_Device *globalization = _globalization_new();

   DateNamesData date_names_data = {0};
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_DATE_NAMES_SUCCESS,
                               _date_names_get_cb,
                               &date_names_data));
   eo_do(globalization, ecordova_globalization_date_names_get(NULL));
   fail_unless(date_names_data.success);

   eo_unref(globalization);
}
END_TEST

typedef struct {
  bool success;
  Ecordova_Globalization_DatePattern pattern;
} DatePatternData;

static Eina_Bool
_date_pattern_get_cb(void *data,
                     Eo *obj EINA_UNUSED,
                     const Eo_Event_Description *desc EINA_UNUSED,
                     void *event_info)
{
   fail_if(NULL == data);
   DatePatternData *date_pattern_data = data;
   fail_if(NULL == event_info);
   Ecordova_Globalization_DatePattern *date_pattern = event_info;

   date_pattern_data->success = true;
   date_pattern_data->pattern = (Ecordova_Globalization_DatePattern){
      .pattern = date_pattern->pattern ? strdup(date_pattern->pattern) : NULL,
      .timezone = date_pattern->timezone ? strdup(date_pattern->timezone) : NULL,
      .utc_offset = date_pattern->utc_offset,
      .dst_offset = date_pattern->dst_offset,
   };

   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(date_pattern_get)
{
   Ecordova_Device *globalization = _globalization_new();

   DatePatternData date_pattern_data = {0};
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_DATE_PATTERN_SUCCESS,
                               _date_pattern_get_cb,
                               &date_pattern_data));
   eo_do(globalization, ecordova_globalization_date_pattern_get(NULL));
   fail_unless(date_pattern_data.success);

   DBG("pattern: %s", date_pattern_data.pattern.pattern);
   DBG("timezone: %s", date_pattern_data.pattern.timezone);
   DBG("utc_offset: %d", date_pattern_data.pattern.utc_offset);
   DBG("dst_offset: %d", date_pattern_data.pattern.dst_offset);
   free((char*)date_pattern_data.pattern.pattern);
   free((char*)date_pattern_data.pattern.timezone);

   eo_unref(globalization);
}
END_TEST

typedef struct {
  bool success;
  Ecordova_Globalization_FirstDayOfWeek first_day_of_week;
} FirstDayOfWeekData;

static Eina_Bool
_first_day_of_week_get_cb(void *data,
                          Eo *obj EINA_UNUSED,
                          const Eo_Event_Description *desc EINA_UNUSED,
                          void *event_info)
{
   fail_if(NULL == data);
   FirstDayOfWeekData *first_day_of_week_data = data;
   fail_if(NULL == event_info);
   Ecordova_Globalization_FirstDayOfWeek *first_day_of_week = event_info;

   first_day_of_week_data->success = true;
   first_day_of_week_data->first_day_of_week = (Ecordova_Globalization_FirstDayOfWeek){
      .value = first_day_of_week->value,
   };

   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(first_day_of_week_get)
{
   Ecordova_Device *globalization = _globalization_new();

   FirstDayOfWeekData first_day_of_week_data = {0};
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_DATE_PATTERN_SUCCESS,
                               _first_day_of_week_get_cb,
                               &first_day_of_week_data));
   eo_do(globalization, ecordova_globalization_date_pattern_get(NULL));
   fail_unless(first_day_of_week_data.success);

   DBG("first_day_of_week: %d", first_day_of_week_data.first_day_of_week.value);

   eo_unref(globalization);
}
END_TEST

typedef struct {
  bool success;
  Ecordova_Globalization_NumberPattern pattern;
} NumberPatternData;

static Eina_Bool
_number_pattern_get_cb(void *data,
                       Eo *obj EINA_UNUSED,
                       const Eo_Event_Description *desc EINA_UNUSED,
                       void *event_info)
{
   fail_if(NULL == data);
   NumberPatternData *number_pattern_data = data;
   fail_if(NULL == event_info);
   Ecordova_Globalization_NumberPattern *number_pattern = event_info;

   number_pattern_data->success = true;
   number_pattern_data->pattern = (Ecordova_Globalization_NumberPattern){
      .pattern = number_pattern->pattern ? strdup(number_pattern->pattern) : NULL,
      .symbol = number_pattern->symbol ? strdup(number_pattern->symbol) : NULL,
      .fraction = number_pattern->fraction,
      .rounding = number_pattern->rounding,
      .positive = number_pattern->positive ? strdup(number_pattern->positive) : NULL,
      .negative = number_pattern->negative ? strdup(number_pattern->negative) : NULL,
      .decimal = number_pattern->decimal ? strdup(number_pattern->decimal) : NULL,
      .grouping = number_pattern->grouping ? strdup(number_pattern->grouping) : NULL
   };

   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(number_pattern_get)
{
   Ecordova_Device *globalization = _globalization_new();

   NumberPatternData number_pattern_data = {0};
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_NUMBER_PATTERN_SUCCESS,
                               _number_pattern_get_cb,
                               &number_pattern_data));
   eo_do(globalization, ecordova_globalization_number_pattern_get(NULL));
   fail_unless(number_pattern_data.success);

   DBG("pattern: %s", number_pattern_data.pattern.pattern);
   DBG("symbol: %s", number_pattern_data.pattern.symbol);
   DBG("fraction: %d", number_pattern_data.pattern.fraction);
   DBG("rounding: %f", number_pattern_data.pattern.rounding);
   DBG("positive: %s", number_pattern_data.pattern.positive);
   DBG("negative: %s", number_pattern_data.pattern.negative);
   DBG("decimal: %s", number_pattern_data.pattern.decimal);
   DBG("grouping: %s", number_pattern_data.pattern.grouping);
   free((char*)number_pattern_data.pattern.pattern);
   free((char*)number_pattern_data.pattern.symbol);
   free((char*)number_pattern_data.pattern.positive);
   free((char*)number_pattern_data.pattern.negative);
   free((char*)number_pattern_data.pattern.decimal);
   free((char*)number_pattern_data.pattern.grouping);

   eo_unref(globalization);
}
END_TEST


typedef struct {
  bool success;
  Ecordova_Globalization_DayLightSavingsTime is_day_light_savings_time;
} DayLightSavingsTimeIsData;

static Eina_Bool
_is_day_light_savings_time_get_cb(void *data,
                          Eo *obj EINA_UNUSED,
                          const Eo_Event_Description *desc EINA_UNUSED,
                          void *event_info)
{
   fail_if(NULL == data);
   DayLightSavingsTimeIsData *is_day_light_savings_time_data = data;
   fail_if(NULL == event_info);
   Ecordova_Globalization_DayLightSavingsTime *is_day_light_savings_time = event_info;

   is_day_light_savings_time_data->success = true;
   is_day_light_savings_time_data->is_day_light_savings_time = (Ecordova_Globalization_DayLightSavingsTime){
      .dst = is_day_light_savings_time->dst,
   };

   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(is_day_light_savings_time)
{
   Ecordova_Device *globalization = _globalization_new();

   time_t date_time;
   time(&date_time);

   DayLightSavingsTimeIsData is_day_light_savings_time_data = {0};
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_DAY_LIGHT_SAVINGS_TIME_SUCCESS,
                               _is_day_light_savings_time_get_cb,
                               &is_day_light_savings_time_data));
   eo_do(globalization, ecordova_globalization_day_light_savings_time_is(date_time));
   fail_unless(is_day_light_savings_time_data.success);

   DBG("is_day_light_savings_time: %d", is_day_light_savings_time_data.is_day_light_savings_time.dst);

   eo_unref(globalization);
}
END_TEST

START_TEST(number_to_string)
{
   Ecordova_Device *globalization = _globalization_new();

   double number = 12345.6789;

   char *string = NULL;
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_NUMBER_TO_STRING_SUCCESS,
                               _to_string_cb,
                               &string));
   eo_do(globalization, ecordova_globalization_number_to_string(number, NULL));
   fail_if(NULL == string);

   DBG("string: %s", string);
   free(string);

   eo_unref(globalization);
}
END_TEST

typedef struct {
  bool success;
  Ecordova_Globalization_DateTime date_time;
} StringToDateData;

static Eina_Bool
_string_to_date_cb(void *data,
                   Eo *obj EINA_UNUSED,
                   const Eo_Event_Description *desc EINA_UNUSED,
                   void *event_info)
{
   fail_if(NULL == data);
   StringToDateData *date_time_data = data;
   fail_if(NULL == event_info);
   Ecordova_Globalization_DateTime *date_time = event_info;

   date_time_data->success = true;
   date_time_data->date_time = (Ecordova_Globalization_DateTime){
      .year = date_time->year,
      .month = date_time->month,
      .day = date_time->day,
      .hour = date_time->hour,
      .minute = date_time->minute,
      .second = date_time->second,
      .millisecond = date_time->millisecond
   };

   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(string_to_date)
{
   Ecordova_Device *globalization = _globalization_new();

   time_t date_time;
   time(&date_time);

   char *dateString = NULL;
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_DATE_TO_STRING_SUCCESS,
                               _to_string_cb,
                               &dateString));
   eo_do(globalization, ecordova_globalization_date_to_string(date_time, NULL));
   fail_if(NULL == dateString);

   StringToDateData string_to_date_data = {0};
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_STRING_TO_DATE_SUCCESS,
                               _string_to_date_cb,
                               &string_to_date_data));
   eo_do(globalization, ecordova_globalization_string_to_date(dateString, NULL));
   fail_unless(string_to_date_data.success);

   DBG("year: %d", string_to_date_data.date_time.year);
   DBG("month: %d", string_to_date_data.date_time.month);
   DBG("day: %d", string_to_date_data.date_time.day);
   DBG("hour: %d", string_to_date_data.date_time.hour);
   DBG("minute: %d", string_to_date_data.date_time.minute);
   DBG("second: %d", string_to_date_data.date_time.second);
   DBG("millisecond: %d", string_to_date_data.date_time.millisecond);

   free(dateString);
   eo_unref(globalization);
}
END_TEST

typedef struct {
  bool success;
  Ecordova_Globalization_Number number;
} StringToNumberData;

static Eina_Bool
_string_to_number_cb(void *data,
                         Eo *obj EINA_UNUSED,
                         const Eo_Event_Description *desc EINA_UNUSED,
                         void *event_info)
{
   fail_if(NULL == data);
   StringToNumberData *string_to_number_data = data;
   fail_if(NULL == event_info);
   Ecordova_Globalization_Number *number = event_info;

   string_to_number_data->success = true;
   string_to_number_data->number = (Ecordova_Globalization_Number){
      .value = number->value,
   };

   ecore_main_loop_quit();
   return EINA_FALSE;
}

START_TEST(string_to_number)
{
   Ecordova_Device *globalization = _globalization_new();

   StringToNumberData string_to_number_data = {0};
   eo_do(globalization,
         eo_event_callback_add(ECORDOVA_GLOBALIZATION_EVENT_STRING_TO_NUMBER_SUCCESS,
                               _string_to_number_cb,
                               &string_to_number_data));
   eo_do(globalization, ecordova_globalization_string_to_number("12345.6789", NULL));
   fail_unless(string_to_number_data.success);

   DBG("number: %f", string_to_number_data.number.value);

   eo_unref(globalization);
}
END_TEST

void
ecordova_globalization_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, smoke);
   tcase_add_test(tc, preferred_language_get);
   tcase_add_test(tc, locale_name_get);
   tcase_add_test(tc, date_to_string);
   tcase_add_test(tc, currency_pattern_get);
   tcase_add_test(tc, date_names_get);
   tcase_add_test(tc, date_pattern_get);
   tcase_add_test(tc, first_day_of_week_get);
   tcase_add_test(tc, number_pattern_get);
   tcase_add_test(tc, is_day_light_savings_time);
   tcase_add_test(tc, number_to_string);
   tcase_add_test(tc, string_to_date);
   tcase_add_test(tc, string_to_number);
}
#else
START_TEST(globalization_fail_load)
{
   Ecordova_Globalization *globalization = eo_add(ECORDOVA_GLOBALIZATION_CLASS, NULL);
   ck_assert_ptr_eq(globalization, NULL);
}
END_TEST

void
ecordova_globalization_test(TCase *tc)
{
   tcase_add_checked_fixture(tc, _setup, _teardown);
   tcase_add_test(tc, globalization_fail_load);
}
#endif
