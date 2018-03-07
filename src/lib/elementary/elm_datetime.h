/**
 * @defgroup Elm_Datetime Datetime
 * @ingroup Elementary
 *
 * @image html datetime_inheritance_tree.png
 * @image latex datetime_inheritance_tree.eps
 *
 * @image html img/widget/datetime/preview-00.png
 * @image latex img/widget/datetime/preview-00.eps
 *
 * @image html img/widget/datetime/preview-01.png
 * @image latex img/widget/datetime/preview-01.eps
 *
 * @image html img/widget/datetime/preview-02.png
 * @image latex img/widget/datetime/preview-02.eps
 *
 * Datetime widget is used to display and input date & time values.
 * This widget displays date and time as per the <b>system's locale</b> settings (Date
 * includes Day, Month & Year along with the defined separators and
 * Time includes Hour, Minute & AM/PM fields. Separator for AM/PM field is ignored.
 *
 * The corresponding Month, AM/PM strings are displayed according to the
 * system’s language settings.
 *
 * Datetime format is a combination of LIBC standard characters like
 * “%%d %%b %%Y %%I : %%M  %%p” which, as a whole represents both Date as well as Time
 * format.
 *
 * Elm_datetime supports only the following sub set of libc date format specifiers:
 *
 * @b %%Y : The year as a decimal number including the century (example: 2011).
 *
 * @b %%y : The year as a decimal number without a century (range 00 to 99)
 *
 * @b %%m : The month as a decimal number (range 01 to 12).
 *
 * @b %%b : The abbreviated month name according to the current locale.
 *
 * @b %%B : The full month name according to the current locale.
 *
 * @b %%h : The abbreviated month name according to the current locale(same as %%b).
 *
 * @b %%d : The day of the month as a decimal number (range 01 to 31).
 *
 * @b %%e : The day of the month as a decimal number (range 1 to 31). single
 * digits are preceded by a blank.
 *
 * @b %%I : The hour as a decimal number using a 12-hour clock (range 01 to 12).
 *
 * @b %%H : The hour as a decimal number using a 24-hour clock (range 00 to 23).
 *
 * @b %%k : The hour (24-hour clock) as a decimal number (range 0 to 23). single
 * digits are preceded by a blank.
 *
 * @b %%l : The hour (12-hour clock) as a decimal number (range 1 to 12); single
 * digits are preceded by a blank.
 *
 * @b %%M : The minute as a decimal number (range 00 to 59).
 *
 * @b %%p : Either 'AM' or 'PM' according to the given time value, or the
 * corresponding strings for the current locale. Noon is treated as 'PM'
 * and midnight as 'AM'
 *
 * @b %%P : Like %p but in lower case: 'am' or 'pm' or a corresponding string for
 * the current locale.
 *
 * @b %%c : The preferred date and time representation for the current locale.
 *
 * @b %%x : The preferred date representation for the current locale without the time.
 *
 * @b %%X : The preferred time representation for the current locale without the date.
 *
 * @b %%r : The complete calendar time using the AM/PM format of the current locale.
 *
 * @b %%R : The hour and minute in decimal numbers using the format %H:%M.
 *
 * @b %%T : The time of day in decimal numbers using the format %H:%M:%S.
 *
 * @b %%D : The date using the format %%m/%%d/%%y.
 *
 * @b %%F : The date using the format %%Y-%%m-%%d.
 *
 * (For more reference on the available <b>LIBC date format specifiers</b>, please
 * visit the link:
 * http://www.gnu.org/s/hello/manual/libc.html#Formatting-Calendar-Time )
 *
 * Datetime widget can provide Unicode @b separators in between its fields
 * except for AM/PM field.
 * A separator can be any <b>Unicode character</b> other than the LIBC standard
 * date format specifiers.( Example: In the format %%b %%d @b , %%y %%H @b : %%M
 * comma(,) is separator for date field %%d and colon(:) is separator for
 * hour field %%H ).
 *
 * The default format is a predefined one, based on the system Locale.
 *
 * Hour format 12hr(1-12) or 24hr(0-23) display can be selected by setting
 * the corresponding user format.
 *
 * Datetime supports six fields: Year, Month, Date, Hour, Minute, AM/PM.
 * Depending on the Datetime module that is loaded, the user can see
 * different UI to select the individual field values.
 *
 * The individual fields of Datetime can be arranged in any order according to the format
 * set by application.
 *
 * There is a provision to set the visibility of a particular field as TRUE/ FALSE
 * so that <b>only time/ only date / only required fields</b> will be displayed.
 *
 * Each field is having a default minimum and maximum values just like the daily
 * calendar information. These min/max values can be modified as per the application usage.
 *
 * User can enter the values only in between the range of maximum and minimum.
 * Apart from these APIs, there is a provision to display only a limited set of
 * values out of the possible values. APIs to select the individual field limits
 * are intended for this purpose.
 *
 * The whole widget is left aligned and its size grows horizontally depending
 * on the current format and each field's visible/disabled state.
 *
 * Datetime individual field selection is implemented in a modular style.
 * Module can be implemented as a Ctxpopup based selection or an ISE based
 * selection or even a spinner like selection etc.
 *
 * <b>Datetime Module design:</b>
 *
 * The following functions are expected to be implemented in a Datetime module:
 *
 * <b>Field creation:</b>
 * <pre>
 *
 *  __________                                            __________
 * |          |----- obj_hook() ---------------------->>>|          |
 * |          |<<<----------------returns Mod_data ------|          |
 * | Datetime |_______                                   |          |
 * |  widget  |       |Assign module call backs          |  Module  |
 * |   base   |<<<____|                                  |          |
 * |          |                                          |          |
 * |          |----- field_create() ------------------>>>|          |
 * |__________|<<<----------------returns field_obj -----|__________|
 *
 * </pre>
 *
 * <b>Field value setting:</b>
 * <pre>
 *
 *  __________                                          __________
 * |          |                                        |          |
 * | Datetime |<<<----------elm_datetime_value_set()---|          |
 * |  widget  |                                        |  Module  |
 * |   base   |----display_field_value()------------>>>|          |
 * |__________|                                        |__________|
 *
 * </pre>
 *
 * <b>del_hook:</b>
 * <pre>
 *  __________                                          __________
 * |          |                                        |          |
 * | Datetime |----obj_unhook()-------------------->>>>|          |
 * |  widget  |                                        |  Module  |
 * |   base   |         <<<-----frees mod_data---------|          |
 * |__________|                                        |__________|
 *
 * </pre>
 *
 *
 * Any module can use the following shared functions that are implemented in elm_datetime.c :
 *
 * <b>field_format_get()</b> - gives the field format.
 *
 * <b>field_limit_get()</b>  - gives the field minimum, maximum limits.
 *
 * To enable a module, set the ELM_MODULES environment variable as shown:
 *
 * <b>export ELM_MODULES="datetime_input_ctxpopup>datetime/api"</b>
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for datetime objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @b "changed" - whenever Datetime field value is changed, this
 * signal is sent.
 * @li @b "language,changed" - whenever system locale changes, this
 * signal is sent.
 * @li @c "focused" - When the datetime has received focus. (since 1.8)
 * @li @c "unfocused" - When the datetime has lost focus. (since 1.8)
 *
 * Here is an example on its usage:
 * @li @ref datetime_example
 *
 */

/**
 * @addtogroup Elm_Datetime
 * @{
 */

/** Identifies a Datetime field, The widget supports 6 fields : Year, month,
 ** Date, Hour, Minute, AM/PM
 **
 ** @ingroup Elm_Datetime
 **/

#ifndef _ELM_DATE_TIME_H
#define _ELM_DATE_TIME_H
typedef enum
{
   ELM_DATETIME_YEAR = 0, /**< Indicates Year field. */
   ELM_DATETIME_MONTH = 1, /**< Indicates Month field. */
   ELM_DATETIME_DATE = 2, /**< Indicates Date field. */
   ELM_DATETIME_HOUR = 3, /**< Indicates Hour field. */
   ELM_DATETIME_MINUTE = 4, /**< Indicates Minute field. */
   ELM_DATETIME_AMPM = 5 /**< Indicates AM/PM field . */
} Elm_Datetime_Field_Type;

typedef Eo Elm_Datetime;

/**
 * @brief Adds a new datetime Widget
 *
 * The default datetime format and corresponding strings are based on current locale.
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * This function inserts a new datetime widget on the canvas.
 *
 * @ingroup Elm_Datetime
 */
EAPI Evas_Object *elm_datetime_add(Evas_Object *parent);

/**
 * @brief Set the datetime format. Format is a combination of allowed Libc date
 * format specifiers like: "%b %d, %Y %I : %M %p".
 *
 * Maximum allowed format length is 64 chars.
 *
 * Format can include separators for each individual datetime field except for
 * AM/PM field.
 *
 * Each separator can be a maximum of 6 UTF-8 bytes. Space is also taken as a
 * separator.
 *
 * These specifiers can be arranged in any order and the widget will display
 * the fields accordingly.
 *
 * Default format is taken as per the system locale settings.
 *
 * @param[in] fmt The datetime format.
 *
 * @ingroup Elm_Datetime
 */
EAPI void elm_datetime_format_set(Evas_Object *obj, const char *fmt);

/**
 * @brief Get the datetime format.
 *
 * @return The datetime format.
 *
 * @ingroup Elm_Datetime
 */
EAPI const char *elm_datetime_format_get(const Evas_Object *obj);

/**
 * @brief Set the field limits of a field.
 *
 * Limits can be set to individual fields, independently, except for AM/PM
 * field. Any field can display the values only in between these minimum and
 * maximum limits unless the corresponding time value is restricted from
 * MinTime to MaxTime. That is, min/max field limits always works under the
 * limitations of mintime/maxtime.
 *
 * There is no provision to set the limits of AM/PM field.
 *
 * @param[in] fieldtype Type of the field. #ELM_DATETIME_YEAR etc.
 * @param[in] min Reference to field's minimum value.
 * @param[in] max Reference to field's maximum value.
 *
 * @ingroup Elm_Datetime
 */
EAPI void elm_datetime_field_limit_set(Evas_Object *obj, Elm_Datetime_Field_Type type, int min, int max);

/**
 * @brief Get the field limits of a field.
 *
 * Limits can be set to individual fields, independently, except for AM/PM
 * field. Any field can display the values only in between these minimum and
 * maximum limits unless the corresponding time value is restricted from
 * MinTime to MaxTime. That is, min/max field limits always works under the
 * limitations of mintime/maxtime.
 *
 * There is no provision to set the limits of AM/PM field.
 *
 * @param[in] fieldtype Type of the field. #ELM_DATETIME_YEAR etc.
 * @param[out] min Reference to field's minimum value.
 * @param[out] max Reference to field's maximum value.
 *
 * @ingroup Elm_Datetime
 */
EAPI void elm_datetime_field_limit_get(const Evas_Object *obj, Elm_Datetime_Field_Type fieldtype, int *min, int *max);

/**
 * @brief Set the lower boundary of a field.
 *
 * Year: years since 1900. Negative value represents year below 1900 (year
 * value -30 represents 1870). Year default range is from 70 to 137.
 *
 * Month: default value range is from 0 to 11.
 *
 * Date: default value range is from 1 to 31 according to the month value.
 *
 * Hour: default value will be in terms of 24 hr format (0~23)
 *
 * Minute: default value range is from 0 to 59.
 *
 * @param[in] mintime Time structure containing the minimum time value.
 *
 * @return @c true if minimum value is accepted.
 *
 * @ingroup Elm_Datetime
 */
EAPI Eina_Bool elm_datetime_value_min_set(Evas_Object *obj, const Efl_Time *mintime);

/**
 * @brief Get the lower boundary of a field.
 *
 * Year: years since 1900. Negative value represents year below 1900 (year
 * value -30 represents 1870). Year default range is from 70 to 137.
 *
 * Month: default value range is from 0 to 11.
 *
 * Date: default value range is from 1 to 31 according to the month value.
 *
 * Hour: default value will be in terms of 24 hr format (0~23)
 *
 * Minute: default value range is from 0 to 59.
 *
 * @param[in,out] mintime Time structure.
 *
 * @return @c true if minimum value is successfully returned.
 *
 * @ingroup Elm_Datetime
 */
EAPI Eina_Bool elm_datetime_value_min_get(const Evas_Object *obj, Efl_Time *mintime);

/**
 * @brief Set the current value of a Datetime object.
 *
 * Year: years since 1900. Negative value represents year below 1900 (year
 * value -30 represents 1870). Year default range is from 70 to 137.
 *
 * Month: default value range is from 0 to 11.
 *
 * Date: default value range is from 1 to 31 according to the month value.
 *
 * Hour: default value will be in terms of 24 hr format (0~23)
 *
 * Minute: default value range is from 0 to 59.
 *
 * @param[in] newtime Time structure filled with values to be set.
 *
 * @return @c true if current time is set successfully.
 *
 * @ingroup Elm_Datetime
 */
EAPI Eina_Bool elm_datetime_value_set(Evas_Object *obj, const Efl_Time *newtime);

/**
 * @brief Get the current value of a Datetime object.
 *
 * Year: years since 1900. Negative value represents year below 1900 (year
 * value -30 represents 1870). Year default range is from 70 to 137.
 *
 * Month: default value range is from 0 to 11.
 *
 * Date: default value range is from 1 to 31 according to the month value.
 *
 * Hour: default value will be in terms of 24 hr format (0~23)
 *
 * Minute: default value range is from 0 to 59.
 *
 * @param[in,out] currtime Time structure.
 *
 * @return @c true if current time is returned successfully.
 *
 * @ingroup Elm_Datetime
 */
EAPI Eina_Bool elm_datetime_value_get(const Evas_Object *obj, Efl_Time *currtime);

/**
 * @brief Set a field to be visible or not.
 *
 * Setting this API to @c true does not ensure that the field is visible, apart
 * from this, the field's format must be present in Datetime overall format.
 *  If a field's visibility is set to @c false then it won't appear even though
 * its format is present in overall format. So if and only if this API is set
 * true and the corresponding field's format is present in Datetime format, the
 * field is visible.
 *
 * By default the field visibility is set to @c true.
 *
 * @param[in] fieldtype Type of the field. #ELM_DATETIME_YEAR etc.
 * @param[in] visible @c true field can be visible, @c false otherwise.
 *
 * @ingroup Elm_Datetime
 */
EAPI void elm_datetime_field_visible_set(Evas_Object *obj, Elm_Datetime_Field_Type fieldtype, Eina_Bool visible);

/**
 * @brief Get whether a field can be visible/not.
 *
 * @param[in] fieldtype Type of the field. #ELM_DATETIME_YEAR etc.
 *
 * @return @c true, if field can be visible. @c false otherwise.
 *
 * @ingroup Elm_Datetime
 */
EAPI Eina_Bool elm_datetime_field_visible_get(const Evas_Object *obj, Elm_Datetime_Field_Type fieldtype);

/**
 * @brief Set the upper boundary of a field.
 *
 * Year: years since 1900. Negative value represents year below 1900 (year
 * value -30 represents 1870). Year default range is from 70 to 137.
 *
 * Month: default value range is from 0 to 11.
 *
 * Date: default value range is from 1 to 31 according to the month value.
 *
 * Hour: default value will be in terms of 24 hr format (0~23)
 *
 * Minute: default value range is from 0 to 59.
 *
 * @param[in] maxtime Time structure containing the maximum time value.
 *
 * @return @c true if maximum value is accepted.
 *
 * @ingroup Elm_Datetime
 */
EAPI Eina_Bool elm_datetime_value_max_set(Evas_Object *obj, const Efl_Time *maxtime);

/**
 * @brief Get the upper boundary of a field.
 *
 * Year: years since 1900. Negative value represents year below 1900 (year
 * value -30 represents 1870). Year default range is from 70 to 137.
 *
 * Month: default value range is from 0 to 11.
 *
 * Date: default value range is from 1 to 31 according to the month value.
 *
 * Hour: default value will be in terms of 24 hr format (0~23)
 *
 * Minute: default value range is from 0 to 59.
 *
 * @param[in,out] maxtime Time structure containing the maximum time value.
 *
 * @return @c true if maximum value is returned successfully.
 *
 * @ingroup Elm_Datetime
 */
EAPI Eina_Bool elm_datetime_value_max_get(const Evas_Object *obj, Efl_Time *maxtime);

#define ELM_DATETIME_EVENT_CHANGED EFL_UI_CLOCK_EVENT_CHANGED

#endif
/**
 * @}
 */
