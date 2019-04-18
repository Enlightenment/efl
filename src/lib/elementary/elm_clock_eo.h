#ifndef _ELM_CLOCK_EO_H_
#define _ELM_CLOCK_EO_H_

#ifndef _ELM_CLOCK_EO_CLASS_TYPE
#define _ELM_CLOCK_EO_CLASS_TYPE

typedef Eo Elm_Clock;

#endif

#ifndef _ELM_CLOCK_EO_TYPES
#define _ELM_CLOCK_EO_TYPES

/**
 * @brief Identifiers for which clock digits should be editable, when a clock
 * widget is in editing mode. Values may be OR-ed together to make a mask,
 * naturally.
 *
 * See also @ref elm_obj_clock_edit_set, @ref elm_obj_clock_edit_mode_set.
 *
 * @ingroup Elm_Clock
 */
typedef enum
{
  ELM_CLOCK_EDIT_DEFAULT = 0, /**< Default value. Means that all digits are
                               * editable, when in editing mode. */
  ELM_CLOCK_EDIT_HOUR_DECIMAL = 1 /* 1 >> 0 */, /**< Decimal digit of hours
                                                 * value should be editable. */
  ELM_CLOCK_EDIT_HOUR_UNIT = 2 /* 1 >> 1 */, /**< Unit digit of hours value
                                              * should be editable. */
  ELM_CLOCK_EDIT_MIN_DECIMAL = 4 /* 1 >> 2 */, /**< Decimal digit of minutes
                                                * value should be editable. */
  ELM_CLOCK_EDIT_MIN_UNIT = 8 /* 1 >> 3 */, /**< Unit digit of minutes value
                                             * should be editable. */
  ELM_CLOCK_EDIT_SEC_DECIMAL = 16 /* 1 >> 4 */, /**< Decimal digit of seconds
                                                 * value should be editable. */
  ELM_CLOCK_EDIT_SEC_UNIT = 32 /* 1 >> 5 */, /**< Unit digit of seconds value
                                              * should be editable. */
  ELM_CLOCK_EDIT_ALL = 63 /* (1 >> 6) * 1 */ /**< All digits should be editable.
                                              */
} Elm_Clock_Edit_Mode;


#endif
/**
 * @brief Digital clock widget
 *
 * This is a digital clock widget. In its default theme, it has a vintage
 * "flipping numbers clock" appearance, which will animate sheets of individual
 * algorisms individually as time goes by.
 *
 * A newly created clock will fetch system's time (already considering local
 * time adjustments) to start with, and will tick accordingly.
 *
 * @ingroup Elm_Clock
 */
#define ELM_CLOCK_CLASS elm_clock_class_get()

EWAPI const Efl_Class *elm_clock_class_get(void);

/**
 * @brief If the given clock widget must show hours in military or am/pm mode
 *
 * Set if the clock must show hours in military or am/pm mode. In some
 * countries like Brazil the military mode (00-24h-format) is used, in
 * opposition to the USA, where the am/pm mode is more commonly used.
 *
 * @param[in] obj The object.
 * @param[in] am_pm @c true to put it in am/pm mode, @c false to military mode
 *
 * @ingroup Elm_Clock
 */
EOAPI void elm_obj_clock_show_am_pm_set(Eo *obj, Eina_Bool am_pm);

/**
 * @brief If the given clock widget must show hours in military or am/pm mode
 *
 * Set if the clock must show hours in military or am/pm mode. In some
 * countries like Brazil the military mode (00-24h-format) is used, in
 * opposition to the USA, where the am/pm mode is more commonly used.
 *
 * @param[in] obj The object.
 *
 * @return @c true to put it in am/pm mode, @c false to military mode
 *
 * @ingroup Elm_Clock
 */
EOAPI Eina_Bool elm_obj_clock_show_am_pm_get(const Eo *obj);

/**
 * @brief The first interval on time updates for a user mouse button hold on
 * clock widgets' time editing.
 *
 * This interval value is decreased while the user holds the mouse pointer
 * either incrementing or decrementing a given the clock digit's value.
 *
 * This helps the user to get to a given time distant from the current one
 * easier/faster, as it will start to flip quicker and quicker on mouse button
 * holds.
 *
 * The calculation for the next flip interval value, starting from the one set
 * with this call, is the previous interval divided by 1.05, so it decreases a
 * little bit.
 *
 * The default starting interval value for automatic flips is 0.85 seconds.
 *
 * @param[in] obj The object.
 * @param[in] interval The first interval value in seconds
 *
 * @ingroup Elm_Clock
 */
EOAPI void elm_obj_clock_first_interval_set(Eo *obj, double interval);

/**
 * @brief The first interval on time updates for a user mouse button hold on
 * clock widgets' time editing.
 *
 * This interval value is decreased while the user holds the mouse pointer
 * either incrementing or decrementing a given the clock digit's value.
 *
 * This helps the user to get to a given time distant from the current one
 * easier/faster, as it will start to flip quicker and quicker on mouse button
 * holds.
 *
 * The calculation for the next flip interval value, starting from the one set
 * with this call, is the previous interval divided by 1.05, so it decreases a
 * little bit.
 *
 * The default starting interval value for automatic flips is 0.85 seconds.
 *
 * @param[in] obj The object.
 *
 * @return The first interval value in seconds
 *
 * @ingroup Elm_Clock
 */
EOAPI double elm_obj_clock_first_interval_get(const Eo *obj);

/**
 * @brief If the given clock widget must show time with seconds or not
 *
 * This function sets if the given clock must show or not elapsed seconds. By
 * default, they are not shown.
 *
 * @param[in] obj The object.
 * @param[in] seconds @c true to show seconds, @c false otherwise.
 *
 * @ingroup Elm_Clock
 */
EOAPI void elm_obj_clock_show_seconds_set(Eo *obj, Eina_Bool seconds);

/**
 * @brief If the given clock widget must show time with seconds or not
 *
 * This function sets if the given clock must show or not elapsed seconds. By
 * default, they are not shown.
 *
 * @param[in] obj The object.
 *
 * @return @c true to show seconds, @c false otherwise.
 *
 * @ingroup Elm_Clock
 */
EOAPI Eina_Bool elm_obj_clock_show_seconds_get(const Eo *obj);

/**
 * @brief Whether a given clock widget is under editing mode or under (default)
 * displaying-only mode.
 *
 * This function makes a clock's time to be editable or not by user
 * interaction. When in editing mode, clocks stop ticking, until one brings
 * them back to display mode. The @ref elm_obj_clock_edit_mode_set function
 * will influence which digits of the clock will be editable.
 *
 * @note am/pm sheets, if being shown, will always be editable under editing
 * mode.
 *
 * @param[in] obj The object.
 * @param[in] edit @c true to put it in editing, @c false to put it back to
 * "displaying only" mode
 *
 * @ingroup Elm_Clock
 */
EOAPI void elm_obj_clock_edit_set(Eo *obj, Eina_Bool edit);

/**
 * @brief Whether a given clock widget is under editing mode or under (default)
 * displaying-only mode.
 *
 * This function makes a clock's time to be editable or not by user
 * interaction. When in editing mode, clocks stop ticking, until one brings
 * them back to display mode. The @ref elm_obj_clock_edit_mode_set function
 * will influence which digits of the clock will be editable.
 *
 * @note am/pm sheets, if being shown, will always be editable under editing
 * mode.
 *
 * @param[in] obj The object.
 *
 * @return @c true to put it in editing, @c false to put it back to "displaying
 * only" mode
 *
 * @ingroup Elm_Clock
 */
EOAPI Eina_Bool elm_obj_clock_edit_get(const Eo *obj);

/**
 * @brief Whether the given clock widget should be paused or not.
 *
 * This function pauses or starts the clock widget.
 *
 * @param[in] obj The object.
 * @param[in] paused @c true to pause clock, @c false otherwise
 *
 * @since 1.9
 *
 * @ingroup Elm_Clock
 */
EOAPI void elm_obj_clock_pause_set(Eo *obj, Eina_Bool paused);

/**
 * @brief Whether the given clock widget should be paused or not.
 *
 * This function pauses or starts the clock widget.
 *
 * @param[in] obj The object.
 *
 * @return @c true to pause clock, @c false otherwise
 *
 * @since 1.9
 *
 * @ingroup Elm_Clock
 */
EOAPI Eina_Bool elm_obj_clock_pause_get(const Eo *obj);

/**
 * @brief Clock widget's time
 *
 * Values must be set within 0-23 for hours and 0-59 for minutes and seconds,
 * even if the clock is not in "military" mode.
 *
 * @warning The behavior for values set out of those ranges is undefined.
 *
 * @param[in] obj The object.
 * @param[in] hrs The hours to set
 * @param[in] min The minutes to set
 * @param[in] sec The seconds to set
 *
 * @ingroup Elm_Clock
 */
EOAPI void elm_obj_clock_time_set(Eo *obj, int hrs, int min, int sec);

/**
 * @brief Clock widget's time
 *
 * Values must be set within 0-23 for hours and 0-59 for minutes and seconds,
 * even if the clock is not in "military" mode.
 *
 * @note Use @c null pointers on the time values you're not interested in:
 * they'll be ignored by the function.
 *
 * @param[in] obj The object.
 * @param[out] hrs The hours to set
 * @param[out] min The minutes to set
 * @param[out] sec The seconds to set
 *
 * @ingroup Elm_Clock
 */
EOAPI void elm_obj_clock_time_get(const Eo *obj, int *hrs, int *min, int *sec);

/**
 * @brief Digits of the given clock widget should be editable when in editing
 * mode.
 *
 * @param[in] obj The object.
 * @param[in] digedit Bit mask indicating the digits to be editable (values in
 * @ref Elm_Clock_Edit_Mode).
 *
 * @ingroup Elm_Clock
 */
EOAPI void elm_obj_clock_edit_mode_set(Eo *obj, Elm_Clock_Edit_Mode digedit);

/**
 * @brief Digits of the given clock widget should be editable when in editing
 * mode.
 *
 * @param[in] obj The object.
 *
 * @return Bit mask indicating the digits to be editable (values in
 * @ref Elm_Clock_Edit_Mode).
 *
 * @ingroup Elm_Clock
 */
EOAPI Elm_Clock_Edit_Mode elm_obj_clock_edit_mode_get(const Eo *obj);

EWAPI extern const Efl_Event_Description _ELM_CLOCK_EVENT_CHANGED;

/** The clock's user changed the time
 *
 * @ingroup Elm_Clock
 */
#define ELM_CLOCK_EVENT_CHANGED (&(_ELM_CLOCK_EVENT_CHANGED))

#endif
