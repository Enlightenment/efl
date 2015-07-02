#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_globalization_private.h"

#include <utils_i18n.h>

#define MY_CLASS ECORDOVA_GLOBALIZATION_CLASS
#define MY_CLASS_NAME "Ecordova_Globalization"

static void _date_time_options_to_style(const Ecordova_Globalization_DateTimeOptions *, i18n_udate_format_style_e *, i18n_udate_format_style_e *);


static Eo_Base *
_ecordova_globalization_eo_base_constructor(Eo *obj,
                                            Ecordova_Globalization_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_globalization_constructor(Eo *obj EINA_UNUSED,
                                    Ecordova_Globalization_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_globalization_eo_base_destructor(Eo *obj,
                                           Ecordova_Globalization_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_globalization_preferred_language_get(Eo *obj,
                                               Ecordova_Globalization_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
   const char *locale;
   int ret = i18n_ulocale_get_default(&locale);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error);

   char language[64] = {0};
   int32_t language_len;
   ret = i18n_ulocale_get_language(locale,
                                   language,
                                   sizeof(language),
                                   &language_len);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error);

   Ecordova_Globalization_Language glanguage = {
     .value = language
   };
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_PREFERRED_LANGUAGE_SUCCESS,
                                &glanguage));
   return;

on_error:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error getting preferred language"
                                }));
}

static void
_ecordova_globalization_locale_name_get(Eo *obj,
                                        Ecordova_Globalization_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
   const char *locale;
   int ret = i18n_ulocale_get_default(&locale);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error);

   Ecordova_Globalization_Locale glocale = {
     .value = locale
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_LOCALE_NAME_SUCCESS,
                                     &glocale));
   return;

on_error:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error getting locale name"
                                }));
}

static void
_ecordova_globalization_date_to_string(Eo *obj,
                                       Ecordova_Globalization_Data *pd EINA_UNUSED,
                                       time_t date,
                                       const Ecordova_Globalization_DateTimeOptions *options)
{
   DBG("(%p)", obj);

   i18n_udate_format_style_e date_style;
   i18n_udate_format_style_e time_style;
   _date_time_options_to_style(options, &date_style, &time_style);

   const char *locale;
   int ret = i18n_ulocale_get_default(&locale);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   i18n_udate_format_h format = NULL;
   ret = i18n_udate_create(time_style,
                           date_style,
                           locale,
                           NULL,
                           0,
                           NULL,
                           0,
                           &format);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   struct tm date_time;
   localtime_r(&date, &date_time);

   i18n_ucalendar_h calendar = NULL;
   ret = i18n_ucalendar_create(NULL,
                               0,
                               locale,
                               I18N_UCALENDAR_DEFAULT,
                               &calendar);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   ret = i18n_ucalendar_set_date_time(calendar,
                                      date_time.tm_year,
                                      date_time.tm_mon,
                                      date_time.tm_mday,
                                      date_time.tm_hour,
                                      date_time.tm_min,
                                      date_time.tm_sec);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   i18n_udate udate;
   ret = i18n_ucalendar_get_milliseconds(calendar, &udate);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   i18n_uchar uch_result[64] = {0};
   int32_t uch_result_len;
   ret = i18n_udate_format_date(format,
                                udate,
                                uch_result,
                                sizeof(uch_result),
                                NULL,
                                &uch_result_len);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   char result[64] = {0};
   i18n_ustring_copy_au_n(result, uch_result, sizeof(uch_result));

   Ecordova_Globalization_String gstring = {
     .value = result
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_DATE_TO_STRING_SUCCESS,
                                     &gstring));

   i18n_ucalendar_destroy(calendar);
   i18n_udate_destroy(format);
   return;

on_error_2:
   i18n_ucalendar_destroy(calendar);

on_error_1:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error in date_to_string"
                                }));
}

static void
_ecordova_globalization_currency_pattern_get(Eo *obj,
                                             Ecordova_Globalization_Data *pd EINA_UNUSED,
                                             const char *currency_code EINA_UNUSED)
{
   DBG("(%p)", obj);

   i18n_unumber_format_h num_format = NULL;
   int ret = i18n_unumber_create(I18N_UNUMBER_CURRENCY,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &num_format);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);


   i18n_uchar uch_buffer[64] = {0};
   int32_t uch_buffer_len;
   ret = i18n_unumber_get_symbol(num_format,
                                 I18N_UNUMBER_INTL_CURRENCY_SYMBOL,
                                 uch_buffer,
                                 sizeof(uch_buffer),
                                 &uch_buffer_len);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   char code[64] = {0};
   i18n_ustring_copy_au_n(code, uch_buffer, sizeof(uch_buffer));


   ret = i18n_unumber_get_symbol(num_format,
                                 I18N_UNUMBER_DECIMAL_SEPARATOR_SYMBOL,
                                 uch_buffer,
                                 sizeof(uch_buffer),
                                 &uch_buffer_len);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   char decimal[64] = {0};
   i18n_ustring_copy_au_n(decimal, uch_buffer, sizeof(uch_buffer));


   ret = i18n_unumber_get_symbol(num_format,
                                 I18N_UNUMBER_GROUPING_SEPARATOR_SYMBOL,
                                 uch_buffer,
                                 sizeof(uch_buffer),
                                 &uch_buffer_len);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   char grouping[64] = {0};
   i18n_ustring_copy_au_n(grouping, uch_buffer, sizeof(uch_buffer));


   int32_t len = i18n_unumber_to_pattern(num_format,
                                         1,
                                         uch_buffer,
                                         sizeof(uch_buffer));
   ret = get_last_result();
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   EINA_SAFETY_ON_TRUE_GOTO(len > (int32_t)sizeof(uch_buffer), on_error_2);
   char pattern[64] = {0};
   i18n_ustring_copy_au_n(pattern, uch_buffer, sizeof(uch_buffer));


   double rounding = i18n_unumber_get_double_attribute(num_format,
                                                       I18N_UNUMBER_ROUNDING_INCREMENT);
   ret = get_last_result();
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   DBG("rounding: %f", rounding);


   int32_t fraction = i18n_unumber_get_attribute(num_format,
                                                 I18N_UNUMBER_MAX_FRACTION_DIGITS);
   ret = get_last_result();
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   DBG("fraction: %d", fraction);

   Ecordova_Globalization_CurrencyPattern number_pattern = {
      .pattern = pattern,
      .code = code,
      .fraction = fraction,
      .rounding = rounding,
      .decimal = decimal,
      .grouping = grouping
   };
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_CURRENCY_PATTERN_SUCCESS,
                                &number_pattern));

   i18n_unumber_destroy(num_format);
   return;

on_error_2:
   i18n_unumber_destroy(num_format);

on_error_1:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error getting currency pattern"
                                }));
}

static void
_ecordova_globalization_date_names_get(Eo *obj,
                                       Ecordova_Globalization_Data *pd EINA_UNUSED,
                                       const Ecordova_Globalization_DateNamesOptions *options)
{
   DBG("(%p)", obj);
   if (!options)
     {
        static const Ecordova_Globalization_DateNamesOptions default_options = {
           .type = ECORDOVA_GLOBALIZATION_DATENAMESTYPE_WIDE,
           .item = ECORDOVA_GLOBALIZATION_DATENAMESITEM_MONTHS
        };
        options = &default_options;
     };

   const char *locale;
   int ret = i18n_ulocale_get_default(&locale);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   i18n_udatepg_h dtpg = NULL;
   ret = i18n_udatepg_create(locale, &dtpg);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   const char *custom_format = NULL;
   int count = 0;
   switch (options->item)
     {
      case ECORDOVA_GLOBALIZATION_DATENAMESITEM_MONTHS:
        count = 12;
        if (ECORDOVA_GLOBALIZATION_DATENAMESTYPE_WIDE == options->type)
          custom_format = "LLLL";
        else
          custom_format = "LLL";
        break;
      case ECORDOVA_GLOBALIZATION_DATENAMESITEM_DAYS:
        count = 7;
        if (ECORDOVA_GLOBALIZATION_DATENAMESTYPE_WIDE == options->type)
          custom_format = "EEEE";
        else
          custom_format = "EEE";
        break;
     }

   i18n_uchar uch_custom_format[64];
   i18n_ustring_copy_ua(uch_custom_format, custom_format);
   i18n_uchar uch_pattern[64] = {0};
   int32_t uch_pattern_len;
   ret = i18n_udatepg_get_best_pattern(dtpg,
                                       uch_custom_format,
                                       i18n_ustring_get_length(uch_custom_format),
                                       uch_pattern,
                                       sizeof(uch_pattern),
                                       &uch_pattern_len);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   char pattern[64] = {0};
   i18n_ustring_copy_au_n(pattern, uch_pattern, sizeof(uch_pattern));
   INF("pattern=%s", pattern);

   i18n_udate_format_h format = NULL;
   ret = i18n_udate_create(I18N_UDATE_PATTERN,
                           I18N_UDATE_PATTERN,
                           locale,
                           NULL,
                           0,
                           uch_pattern,
                           i18n_ustring_get_length(uch_pattern),
                           &format);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   i18n_ucalendar_h calendar = NULL;
   ret = i18n_ucalendar_create(NULL,
                               0,
                               locale,
                               I18N_UCALENDAR_DEFAULT,
                               &calendar);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   Eina_Array *names = eina_array_new(count);

   int month = 0;
   int day = 1;
   for (int i = 0; i < count; ++i)
     {
        ret = i18n_ucalendar_set_date_time(calendar,
                                           2012,
                                           month,
                                           day,
                                           12,
                                           0,
                                           0);
        EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_3);

        i18n_udate udate;
        ret = i18n_ucalendar_get_milliseconds(calendar, &udate);
        EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_3);

        i18n_uchar uch_result[64] = {0};
        int32_t uch_result_len;
        ret = i18n_udate_format_date(format,
                                     udate,
                                     uch_result,
                                     sizeof(uch_result),
                                     NULL,
                                     &uch_result_len);
        EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_3);

        char result[64] = {0};
        i18n_ustring_copy_au_n(result, uch_result, sizeof(uch_result));
        DBG("result=%s", result);

        eina_array_push(names, strdup(result));

        if (ECORDOVA_GLOBALIZATION_DATENAMESITEM_MONTHS == options->item)
          ++month;
        else
          ++day;
     }

   Ecordova_Globalization_DateNames date_names = {
     .value = names
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_DATE_NAMES_SUCCESS,
                                     &date_names));

   char *name;
   unsigned int i;
   Eina_Array_Iterator it;
   EINA_ARRAY_ITER_NEXT(names, i, name, it)
     free(name);
   eina_array_free(names);

   i18n_ucalendar_destroy(calendar);
   i18n_udate_destroy(format);
   i18n_udatepg_destroy(dtpg);
   return;

on_error_3:
   EINA_ARRAY_ITER_NEXT(names, i, name, it)
     free(name);
   eina_array_free(names);

   i18n_ucalendar_destroy(calendar);

on_error_2:
   i18n_udatepg_destroy(dtpg);

on_error_1:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error getting date names"
                                }));
}

static void
_ecordova_globalization_date_pattern_get(Eo *obj,
                                         Ecordova_Globalization_Data *pd EINA_UNUSED,
                                         const Ecordova_Globalization_DateTimeOptions *options)
{
   DBG("(%p)", obj);
   const char *locale;
   int ret = i18n_ulocale_get_default(&locale);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   i18n_timezone_h timezone = NULL;
   ret = i18n_timezone_create_default(&timezone);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   char *timezone_str = NULL;
   ret = i18n_timezone_get_id(timezone, &timezone_str);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   int32_t utc_offset = 0;
   ret = i18n_timezone_get_raw_offset(timezone, &utc_offset);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_3);

   int32_t dst_offset = 0;
   ret = i18n_timezone_get_dst_savings(timezone, &dst_offset);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_3);

   i18n_udate_format_style_e date_style;
   i18n_udate_format_style_e time_style;
   _date_time_options_to_style(options, &date_style, &time_style);

   i18n_udate_format_h format = NULL;
   ret = i18n_udate_create(time_style,
                           date_style,
                           locale,
                           NULL,
                           0,
                           NULL,
                           0,
                           &format);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_3);


   i18n_uchar uch_pattern[64] = {0};
   int32_t uch_pattern_len = i18n_udate_to_pattern(format,
                                                   1,
                                                   uch_pattern,
                                                   sizeof(uch_pattern));
   ret = get_last_result();
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_4);
   EINA_SAFETY_ON_TRUE_GOTO(uch_pattern_len > (int32_t)sizeof(uch_pattern), on_error_4);

   char pattern[64] = {0};
   i18n_ustring_copy_au_n(pattern, uch_pattern, sizeof(uch_pattern));

   Ecordova_Globalization_DatePattern date_pattern = {
     .pattern = pattern,
     .timezone = timezone_str,
     .utc_offset = utc_offset / 1000,
     .dst_offset = dst_offset / 1000
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_DATE_PATTERN_SUCCESS,
                                     &date_pattern));
   i18n_udate_destroy(format);
   free(timezone_str);
   i18n_timezone_destroy(timezone);
   return;

on_error_4:
   i18n_udate_destroy(format);

on_error_3:
   free(timezone_str);

on_error_2:
   i18n_timezone_destroy(timezone);

on_error_1:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error getting date pattern"
                                }));
}

static void
_ecordova_globalization_first_day_of_week_get(Eo *obj,
                                              Ecordova_Globalization_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
   const char *locale;
   int ret = i18n_ulocale_get_default(&locale);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   i18n_ucalendar_h calendar = NULL;
   ret = i18n_ucalendar_create(NULL,
                               0,
                               locale,
                               I18N_UCALENDAR_DEFAULT,
                               &calendar);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   int32_t value = 0;
   ret = i18n_ucalendar_get_attribute(calendar,
                                      I18N_UCALENDAR_FIRST_DAY_OF_WEEK,
                                      &value);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   Ecordova_Globalization_FirstDayOfWeek first_day_of_week = {
      .value = value
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_FIRST_DAY_OF_WEEK_SUCCESS,
                                     &first_day_of_week));
   i18n_ucalendar_destroy(calendar);
   return;

on_error_2:
   i18n_ucalendar_destroy(calendar);

on_error_1:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error getting first day of week"
                                }));
}

static void
_ecordova_globalization_number_pattern_get(Eo *obj,
                                           Ecordova_Globalization_Data *pd EINA_UNUSED,
                                           const Ecordova_Globalization_NumberPatternOptions *options)
{
   DBG("(%p)", obj);

   if (!options)
     {
        static const Ecordova_Globalization_NumberPatternOptions default_options = {
           .type = ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_DECIMAL
        };
        options = &default_options;
     }

   i18n_unumber_format_style_e format_style = I18N_UNUMBER_CURRENCY;
   switch (options->type)
     {
      case ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_DECIMAL:
        format_style = I18N_UNUMBER_DECIMAL;
        break;
      case ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_PERCENT:
        format_style = I18N_UNUMBER_PERCENT;
        break;
      case ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_CURRENCY:
        format_style = I18N_UNUMBER_CURRENCY;
        break;
     }

   i18n_unumber_format_h num_format = NULL;
   int ret = i18n_unumber_create(format_style,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &num_format);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);


   i18n_uchar uch_buffer[64] = {0};
   int32_t uch_buffer_len;
   ret = i18n_unumber_get_symbol(num_format,
                                 I18N_UNUMBER_PLUS_SIGN_SYMBOL,
                                 uch_buffer,
                                 sizeof(uch_buffer),
                                 &uch_buffer_len);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   char positive[64] = {0};
   i18n_ustring_copy_au_n(positive, uch_buffer, sizeof(uch_buffer));


   ret = i18n_unumber_get_symbol(num_format,
                                 I18N_UNUMBER_MINUS_SIGN_SYMBOL,
                                 uch_buffer,
                                 sizeof(uch_buffer),
                                 &uch_buffer_len);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   char negative[64] = {0};
   i18n_ustring_copy_au_n(negative, uch_buffer, sizeof(uch_buffer));


   ret = i18n_unumber_get_symbol(num_format,
                                 I18N_UNUMBER_DECIMAL_SEPARATOR_SYMBOL,
                                 uch_buffer,
                                 sizeof(uch_buffer),
                                 &uch_buffer_len);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   char decimal[64] = {0};
   i18n_ustring_copy_au_n(decimal, uch_buffer, sizeof(uch_buffer));


   ret = i18n_unumber_get_symbol(num_format,
                                 I18N_UNUMBER_GROUPING_SEPARATOR_SYMBOL,
                                 uch_buffer,
                                 sizeof(uch_buffer),
                                 &uch_buffer_len);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   char grouping[64] = {0};
   i18n_ustring_copy_au_n(grouping, uch_buffer, sizeof(uch_buffer));


   int32_t len = i18n_unumber_to_pattern(num_format,
                                         1,
                                         uch_buffer,
                                         sizeof(uch_buffer));
   ret = get_last_result();
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   EINA_SAFETY_ON_TRUE_GOTO(len > (int32_t)sizeof(uch_buffer), on_error_2);
   char pattern[64] = {0};
   i18n_ustring_copy_au_n(pattern, uch_buffer, sizeof(uch_buffer));


   double rounding = i18n_unumber_get_double_attribute(num_format,
                                                       I18N_UNUMBER_ROUNDING_INCREMENT);
   ret = get_last_result();
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   DBG("rounding: %f", rounding);


   int32_t fraction = i18n_unumber_get_attribute(num_format,
                                                 I18N_UNUMBER_MAX_FRACTION_DIGITS);
   ret = get_last_result();
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);
   DBG("fraction: %d", fraction);

   Ecordova_Globalization_NumberPattern number_pattern = {
      .pattern = pattern,
      //.symbol =
      .fraction = fraction,
      .rounding = rounding,
      .positive = positive,
      .negative = negative,
      .decimal = decimal,
      .grouping = grouping
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_NUMBER_PATTERN_SUCCESS,
                                     &number_pattern));

   i18n_unumber_destroy(num_format);
   return;

on_error_2:
   i18n_unumber_destroy(num_format);

on_error_1:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error getting currency pattern"
                                }));
}

static void
_ecordova_globalization_day_light_savings_time_is(Eo *obj,
                                                  Ecordova_Globalization_Data *pd EINA_UNUSED,
                                                  time_t date)
{
   DBG("(%p)", obj);

   const char *locale;
   int ret = i18n_ulocale_get_default(&locale);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   struct tm date_time;
   localtime_r(&date, &date_time);

   i18n_ucalendar_h calendar = NULL;
   ret = i18n_ucalendar_create(NULL,
                               0,
                               locale,
                               I18N_UCALENDAR_DEFAULT,
                               &calendar);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   ret = i18n_ucalendar_set_date_time(calendar,
                                      date_time.tm_year,
                                      date_time.tm_mon,
                                      date_time.tm_mday,
                                      date_time.tm_hour,
                                      date_time.tm_min,
                                      date_time.tm_sec);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   i18n_ubool value = 0;
   ret = i18n_ucalendar_is_in_daylight_time(calendar, &value);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   Ecordova_Globalization_DayLightSavingsTime dst = {
     .dst = !!value
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_DAY_LIGHT_SAVINGS_TIME_SUCCESS,
                                     &dst));

   i18n_ucalendar_destroy(calendar);
   return;

on_error_2:
   i18n_ucalendar_destroy(calendar);

on_error_1:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error getting if the date/time is in dst"
                                }));
}

static void
_ecordova_globalization_number_to_string(Eo *obj,
                                         Ecordova_Globalization_Data *pd EINA_UNUSED,
                                         double number,
                                         const Ecordova_Globalization_NumberPatternOptions *options)
{
   DBG("(%p)", obj);

   if (!options)
     {
        static const Ecordova_Globalization_NumberPatternOptions default_options = {
           .type = ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_DECIMAL
        };
        options = &default_options;
     }

   i18n_unumber_format_style_e format_style = I18N_UNUMBER_CURRENCY;
   switch (options->type)
     {
      case ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_DECIMAL:
        format_style = I18N_UNUMBER_DECIMAL;
        break;
      case ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_PERCENT:
        format_style = I18N_UNUMBER_PERCENT;
        break;
      case ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_CURRENCY:
        format_style = I18N_UNUMBER_CURRENCY;
        break;
     }

   i18n_unumber_format_h num_format = NULL;
   int ret = i18n_unumber_create(format_style,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &num_format);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error);

   i18n_uchar uch_buffer[64] = {0};
   int32_t uch_buffer_len = i18n_unumber_format_double(num_format,
                                                       number,
                                                       uch_buffer,
                                                       sizeof(uch_buffer),
                                                       NULL);
   ret = get_last_result();
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, format_error);
   EINA_SAFETY_ON_TRUE_GOTO(uch_buffer_len > (int32_t)sizeof(uch_buffer), format_error);

   char value[64] = {0};
   i18n_ustring_copy_au_n(value, uch_buffer, sizeof(uch_buffer));

   Ecordova_Globalization_String gstring = {
     .value = value
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_NUMBER_TO_STRING_SUCCESS,
                                     &gstring));

   i18n_unumber_destroy(num_format);
   return;

on_error:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error formatting number"
                                }));
   return;

format_error:
   i18n_unumber_destroy(num_format);
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_FORMATTING_ERROR,
                                   .message = "Format error"
                                }));
}

static void
_ecordova_globalization_string_to_date(Eo *obj,
                                       Ecordova_Globalization_Data *pd EINA_UNUSED,
                                       const char *dateString,
                                       const Ecordova_Globalization_DateTimeOptions *options)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(dateString);

   i18n_udate_format_style_e date_style;
   i18n_udate_format_style_e time_style;
   _date_time_options_to_style(options, &date_style, &time_style);

   const char *locale;
   int ret = i18n_ulocale_get_default(&locale);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   i18n_udate_format_h format = NULL;
   ret = i18n_udate_create(time_style,
                           date_style,
                           locale,
                           NULL,
                           0,
                           NULL,
                           0,
                           &format);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);


   i18n_uchar uch_date[64] = {0};
   i18n_ustring_copy_ua(uch_date, dateString);

   i18n_udate udate;
   ret = i18n_udate_parse(format,
                          uch_date,
                          -1,
                          NULL,
                          &udate);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, parse_error);

   i18n_ucalendar_h calendar = NULL;
   ret = i18n_ucalendar_create(NULL,
                               0,
                               locale,
                               I18N_UCALENDAR_DEFAULT,
                               &calendar);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_1);

   ret = i18n_ucalendar_set_milliseconds(calendar, udate);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   int32_t year = 0;
   ret = i18n_ucalendar_get(calendar, I18N_UCALENDAR_YEAR, &year);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   int32_t month = 0;
   ret = i18n_ucalendar_get(calendar, I18N_UCALENDAR_MONTH, &month);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   int32_t day = 0;
   ret = i18n_ucalendar_get(calendar, I18N_UCALENDAR_DATE, &day);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   int32_t hour = 0;
   ret = i18n_ucalendar_get(calendar, I18N_UCALENDAR_HOUR_OF_DAY, &hour);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   int32_t minute = 0;
   ret = i18n_ucalendar_get(calendar, I18N_UCALENDAR_MINUTE, &minute);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   int32_t second = 0;
   ret = i18n_ucalendar_get(calendar, I18N_UCALENDAR_SECOND, &second);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);

   int32_t millisecond = 0;
   ret = i18n_ucalendar_get(calendar, I18N_UCALENDAR_MILLISECOND, &millisecond);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error_2);


   Ecordova_Globalization_DateTime date_time = {
     .year = year,
     .month = month,
     .day = day,
     .hour = hour,
     .minute = minute,
     .second = second,
     .millisecond = millisecond
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_STRING_TO_DATE_SUCCESS,
                                     &date_time));

   i18n_ucalendar_destroy(calendar);
   i18n_udate_destroy(format);
   return;

on_error_2:
   i18n_ucalendar_destroy(calendar);

on_error_1:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error in string_to_date"
                                }));
   return;

parse_error:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_PARSING_ERROR,
                                   .message = "Parse error"
                                }));
}

static void
_ecordova_globalization_string_to_number(Eo *obj,
                                         Ecordova_Globalization_Data *pd EINA_UNUSED,
                                         const char *string,
                                         const Ecordova_Globalization_NumberPatternOptions *options)
{
   DBG("(%p)", obj);
   EINA_SAFETY_ON_NULL_RETURN(string);

   if (!options)
     {
        static const Ecordova_Globalization_NumberPatternOptions default_options = {
           .type = ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_DECIMAL
        };
        options = &default_options;
     }

   i18n_unumber_format_style_e format_style = I18N_UNUMBER_CURRENCY;
   switch (options->type)
     {
      case ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_DECIMAL:
        format_style = I18N_UNUMBER_DECIMAL;
        break;
      case ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_PERCENT:
        format_style = I18N_UNUMBER_PERCENT;
        break;
      case ECORDOVA_GLOBALIZATION_NUMBERPATTERNTYPE_CURRENCY:
        format_style = I18N_UNUMBER_CURRENCY;
        break;
     }

   i18n_uchar uch_number[strlen(string) + 1];

   i18n_unumber_format_h num_format = NULL;
   int ret = i18n_unumber_create(format_style,
                                 NULL,
                                 0,
                                 NULL,
                                 NULL,
                                 &num_format);
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, on_error);


   i18n_ustring_copy_ua(uch_number, string);

   double value = i18n_unumber_parse_double(num_format,
                                            uch_number,
                                            -1,
                                            NULL);
   ret = get_last_result();
   EINA_SAFETY_ON_FALSE_GOTO(I18N_ERROR_NONE == ret, parse_error);

   Ecordova_Globalization_Number gnumber = {
     .value = value
   };
   eo_do(obj, eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_STRING_TO_NUMBER_SUCCESS,
                                     &gnumber));

   i18n_unumber_destroy(num_format);
   return;

on_error:
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_UNKNOWN_ERROR,
                                   .message = "Unknown error parsing string"
                                }));
   return;

parse_error:
   i18n_unumber_destroy(num_format);
   eo_do(obj,
         eo_event_callback_call(ECORDOVA_GLOBALIZATION_EVENT_ERROR,
                                &(Ecordova_Globalization_Error){
                                   .code = ECORDOVA_GLOBALIZATION_ERRORCODE_PARSING_ERROR,
                                   .message = "Parser error"
                                }));
}

static void
_date_time_options_to_style(const Ecordova_Globalization_DateTimeOptions *options,
                            i18n_udate_format_style_e *date_style,
                            i18n_udate_format_style_e *time_style)
{
   EINA_SAFETY_ON_NULL_RETURN(date_style);
   EINA_SAFETY_ON_NULL_RETURN(time_style);

   if (!options)
     {
        static const Ecordova_Globalization_DateTimeOptions default_options = {
           .format_length = ECORDOVA_GLOBALIZATION_DATEFORMATLENGTH_SHORT,
           .selector = ECORDOVA_GLOBALIZATION_DATESELECTOR_DATE_AND_TIME
        };
        options = &default_options;
     };

   *date_style = I18N_UDATE_NONE;
   *time_style = I18N_UDATE_NONE;

   switch (options->format_length)
     {
      case ECORDOVA_GLOBALIZATION_DATEFORMATLENGTH_SHORT:
        switch (options->selector)
          {
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_DATE:
             *date_style = I18N_UDATE_SHORT;
             break;
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_TIME:
             *time_style = I18N_UDATE_SHORT;
             break;
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_DATE_AND_TIME:
             *date_style = I18N_UDATE_SHORT;
             *time_style = I18N_UDATE_SHORT;
             break;
          }
        break;
      case ECORDOVA_GLOBALIZATION_DATEFORMATLENGTH_MEDIUM:
        switch (options->selector)
          {
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_DATE:
             *date_style = I18N_UDATE_MEDIUM;
             break;
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_TIME:
             *time_style = I18N_UDATE_MEDIUM;
             break;
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_DATE_AND_TIME:
             *date_style = I18N_UDATE_MEDIUM;
             *time_style = I18N_UDATE_MEDIUM;
             break;
          }
        break;
      case ECORDOVA_GLOBALIZATION_DATEFORMATLENGTH_LONG:
        switch (options->selector)
          {
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_DATE:
             *date_style = I18N_UDATE_LONG;
             break;
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_TIME:
             *time_style = I18N_UDATE_LONG;
             break;
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_DATE_AND_TIME:
             *date_style = I18N_UDATE_LONG;
             *time_style = I18N_UDATE_LONG;
             break;
          }
        break;
      case ECORDOVA_GLOBALIZATION_DATEFORMATLENGTH_FULL:
        switch (options->selector)
          {
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_DATE:
             *date_style = I18N_UDATE_FULL;
             break;
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_TIME:
             *time_style = I18N_UDATE_FULL;
             break;
           case ECORDOVA_GLOBALIZATION_DATESELECTOR_DATE_AND_TIME:
             *date_style = I18N_UDATE_FULL;
             *time_style = I18N_UDATE_FULL;
             break;
          }
        break;
     };
}

#include "ecordova_globalization.eo.c"
