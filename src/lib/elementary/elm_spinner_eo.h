#ifndef _ELM_SPINNER_EO_H_
#define _ELM_SPINNER_EO_H_

#ifndef _ELM_SPINNER_EO_CLASS_TYPE
#define _ELM_SPINNER_EO_CLASS_TYPE

typedef Eo Elm_Spinner;

#endif

#ifndef _ELM_SPINNER_EO_TYPES
#define _ELM_SPINNER_EO_TYPES


#endif
/** Elementary spinner class
 *
 * @ingroup Elm_Spinner
 */
#define ELM_SPINNER_CLASS elm_spinner_class_get()

EWAPI const Efl_Class *elm_spinner_class_get(void);

/**
 * @brief Control whether the spinner should wrap when it reaches its minimum
 * or maximum value.
 *
 * Disabled by default. If disabled, when the user tries to increment the
 * value, but displayed value plus step value is bigger than maximum value, the
 * new value will be the maximum value. The same happens when the user tries to
 * decrement it, but the value less step is less than minimum value. In this
 * case, the new displayed value will be the minimum value.
 *
 * When wrap is enabled, when the user tries to increment the value, but
 * displayed value plus step value is bigger than maximum value, the new value
 * will be the minimum value. When the the user tries to decrement it, but the
 * value less step is less than minimum value, the new displayed value will be
 * the maximum value.
 *
 * E.g.: @c min = 10 @c max = 50 @c step = 20 @c displayed = 20
 *
 * When the user decrement value (using left or bottom arrow), it will displays
 * $50.
 *
 * @param[in] obj The object.
 * @param[in] wrap @c true to enable wrap or @c false to disable it.
 *
 * @ingroup Elm_Spinner
 */
EOAPI void elm_obj_spinner_wrap_set(Eo *obj, Eina_Bool wrap);

/**
 * @brief Control whether the spinner should wrap when it reaches its minimum
 * or maximum value.
 *
 * Disabled by default. If disabled, when the user tries to increment the
 * value, but displayed value plus step value is bigger than maximum value, the
 * new value will be the maximum value. The same happens when the user tries to
 * decrement it, but the value less step is less than minimum value. In this
 * case, the new displayed value will be the minimum value.
 *
 * When wrap is enabled, when the user tries to increment the value, but
 * displayed value plus step value is bigger than maximum value, the new value
 * will be the minimum value. When the the user tries to decrement it, but the
 * value less step is less than minimum value, the new displayed value will be
 * the maximum value.
 *
 * E.g.: @c min = 10 @c max = 50 @c step = 20 @c displayed = 20
 *
 * When the user decrement value (using left or bottom arrow), it will displays
 * $50.
 *
 * @param[in] obj The object.
 *
 * @return @c true to enable wrap or @c false to disable it.
 *
 * @ingroup Elm_Spinner
 */
EOAPI Eina_Bool elm_obj_spinner_wrap_get(const Eo *obj);

/**
 * @brief Control the interval on time updates for an user mouse button hold on
 * spinner widgets' arrows.
 *
 * This interval value is decreased while the user holds the mouse pointer
 * either incrementing or decrementing spinner's value.
 *
 * This helps the user to get to a given value distant from the current one
 * easier/faster, as it will start to change quicker and quicker on mouse
 * button holds.
 *
 * The calculation for the next change interval value, starting from the one
 * set with this call, is the previous interval divided by $1.05, so it
 * decreases a little bit.
 *
 * The default starting interval value for automatic changes is $0.85 seconds.
 *
 * @param[in] obj The object.
 * @param[in] interval The (first) interval value in seconds.
 *
 * @ingroup Elm_Spinner
 */
EOAPI void elm_obj_spinner_interval_set(Eo *obj, double interval);

/**
 * @brief Control the interval on time updates for an user mouse button hold on
 * spinner widgets' arrows.
 *
 * This interval value is decreased while the user holds the mouse pointer
 * either incrementing or decrementing spinner's value.
 *
 * This helps the user to get to a given value distant from the current one
 * easier/faster, as it will start to change quicker and quicker on mouse
 * button holds.
 *
 * The calculation for the next change interval value, starting from the one
 * set with this call, is the previous interval divided by $1.05, so it
 * decreases a little bit.
 *
 * The default starting interval value for automatic changes is $0.85 seconds.
 *
 * @param[in] obj The object.
 *
 * @return The (first) interval value in seconds.
 *
 * @ingroup Elm_Spinner
 */
EOAPI double elm_obj_spinner_interval_get(const Eo *obj);

/**
 * @brief Control the round value for rounding
 *
 * Sets the rounding value used for value rounding in the spinner.
 *
 * @param[in] obj The object.
 * @param[in] rnd The rounding value
 *
 * @ingroup Elm_Spinner
 */
EOAPI void elm_obj_spinner_round_set(Eo *obj, int rnd);

/**
 * @brief Control the round value for rounding
 *
 * Sets the rounding value used for value rounding in the spinner.
 *
 * @param[in] obj The object.
 *
 * @return The rounding value
 *
 * @ingroup Elm_Spinner
 */
EOAPI int elm_obj_spinner_round_get(const Eo *obj);

/**
 * @brief Control whether the spinner can be directly edited by the user or
 * not.
 *
 * Spinner objects can have edition disabled, in which state they will be
 * changed only by arrows. Useful for contexts where you don't want your users
 * to interact with it writing the value. Specially when using special values,
 * the user can see real value instead of special label on edition.
 *
 * It's enabled by default.
 *
 * @param[in] obj The object.
 * @param[in] editable @c true to allow users to edit it or @c false to don't
 * allow users to edit it directly.
 *
 * @ingroup Elm_Spinner
 */
EOAPI void elm_obj_spinner_editable_set(Eo *obj, Eina_Bool editable);

/**
 * @brief Control whether the spinner can be directly edited by the user or
 * not.
 *
 * Spinner objects can have edition disabled, in which state they will be
 * changed only by arrows. Useful for contexts where you don't want your users
 * to interact with it writing the value. Specially when using special values,
 * the user can see real value instead of special label on edition.
 *
 * It's enabled by default.
 *
 * @param[in] obj The object.
 *
 * @return @c true to allow users to edit it or @c false to don't allow users
 * to edit it directly.
 *
 * @ingroup Elm_Spinner
 */
EOAPI Eina_Bool elm_obj_spinner_editable_get(const Eo *obj);

/**
 * @brief Control the base for rounding
 *
 * Rounding works as follows:
 *
 * rounded_val = base + (double)(((value - base) / round) * round)
 *
 * Where rounded_val, value and base are doubles, and round is an integer.
 *
 * This means that things will be rounded to increments (or decrements) of
 * "round" starting from value @c base. The default base for rounding is 0.
 *
 * Example: round = 3, base = 2 Values: ..., -2, 0, 2, 5, 8, 11, 14, ...
 *
 * Example: round = 2, base = 5.5 Values: ..., -0.5, 1.5, 3.5, 5.5, 7.5, 9.5,
 * 11.5, ...
 *
 * @param[in] obj The object.
 * @param[in] base The base value
 *
 * @ingroup Elm_Spinner
 */
EOAPI void elm_obj_spinner_base_set(Eo *obj, double base);

/**
 * @brief Control the base for rounding
 *
 * Rounding works as follows:
 *
 * rounded_val = base + (double)(((value - base) / round) * round)
 *
 * Where rounded_val, value and base are doubles, and round is an integer.
 *
 * This means that things will be rounded to increments (or decrements) of
 * "round" starting from value @c base. The default base for rounding is 0.
 *
 * Example: round = 3, base = 2 Values: ..., -2, 0, 2, 5, 8, 11, 14, ...
 *
 * Example: round = 2, base = 5.5 Values: ..., -0.5, 1.5, 3.5, 5.5, 7.5, 9.5,
 * 11.5, ...
 *
 * @param[in] obj The object.
 *
 * @return The base value
 *
 * @ingroup Elm_Spinner
 */
EOAPI double elm_obj_spinner_base_get(const Eo *obj);

/**
 * @brief Control the format string of the displayed label.
 *
 * If @c NULL, this sets the format to "%.0f". If not it sets the format string
 * for the label text. The label text is provided a floating point value, so
 * the label text can display up to 1 floating point value. Note that this is
 * optional.
 *
 * Use a format string such as "%1.2f meters" for example, and it will display
 * values like: "3.14 meters" for a value equal to 3.14159.
 *
 * Default is "%0.f".
 *
 * @param[in] obj The object.
 * @param[in] fmt The format string for the label display.
 *
 * @ingroup Elm_Spinner
 */
EOAPI void elm_obj_spinner_label_format_set(Eo *obj, const char *fmt);

/**
 * @brief Control the format string of the displayed label.
 *
 * If @c NULL, this sets the format to "%.0f". If not it sets the format string
 * for the label text. The label text is provided a floating point value, so
 * the label text can display up to 1 floating point value. Note that this is
 * optional.
 *
 * Use a format string such as "%1.2f meters" for example, and it will display
 * values like: "3.14 meters" for a value equal to 3.14159.
 *
 * Default is "%0.f".
 *
 * @param[in] obj The object.
 *
 * @return The format string for the label display.
 *
 * @ingroup Elm_Spinner
 */
EOAPI const char *elm_obj_spinner_label_format_get(const Eo *obj);

/**
 * @brief Control special string to display in the place of the numerical
 * value.
 *
 * It's useful for cases when a user should select an item that is better
 * indicated by a label than a value. For example, weekdays or months.
 *
 * @note If another label was previously set to @c value, it will be replaced
 * by the new label.
 *
 * @param[in] obj The object.
 * @param[in] value The value to be replaced.
 * @param[in] label The label to be used.
 *
 * @ingroup Elm_Spinner
 */
EOAPI void elm_obj_spinner_special_value_add(Eo *obj, double value, const char *label);

EWAPI extern const Efl_Event_Description _ELM_SPINNER_EVENT_CHANGED;

/** Called when spinner changed
 *
 * @ingroup Elm_Spinner
 */
#define ELM_SPINNER_EVENT_CHANGED (&(_ELM_SPINNER_EVENT_CHANGED))

EWAPI extern const Efl_Event_Description _ELM_SPINNER_EVENT_DELAY_CHANGED;

/** Called when spinner delay changed
 *
 * @ingroup Elm_Spinner
 */
#define ELM_SPINNER_EVENT_DELAY_CHANGED (&(_ELM_SPINNER_EVENT_DELAY_CHANGED))

EWAPI extern const Efl_Event_Description _ELM_SPINNER_EVENT_SPINNER_DRAG_START;

/** Called when spinner drag started
 *
 * @ingroup Elm_Spinner
 */
#define ELM_SPINNER_EVENT_SPINNER_DRAG_START (&(_ELM_SPINNER_EVENT_SPINNER_DRAG_START))

EWAPI extern const Efl_Event_Description _ELM_SPINNER_EVENT_SPINNER_DRAG_STOP;

/** Called when spinner drag stopped
 *
 * @ingroup Elm_Spinner
 */
#define ELM_SPINNER_EVENT_SPINNER_DRAG_STOP (&(_ELM_SPINNER_EVENT_SPINNER_DRAG_STOP))

EWAPI extern const Efl_Event_Description _ELM_SPINNER_EVENT_MIN_REACHED;

/** Called when spinner value reached min
 *
 * @ingroup Elm_Spinner
 */
#define ELM_SPINNER_EVENT_MIN_REACHED (&(_ELM_SPINNER_EVENT_MIN_REACHED))

EWAPI extern const Efl_Event_Description _ELM_SPINNER_EVENT_MAX_REACHED;

/** Called when spinner value reached max
 *
 * @ingroup Elm_Spinner
 */
#define ELM_SPINNER_EVENT_MAX_REACHED (&(_ELM_SPINNER_EVENT_MAX_REACHED))

#endif
