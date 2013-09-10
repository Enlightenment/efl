/**
 * Add a new progress bar widget to the given parent Elementary
 * (container) object
 *
 * @param parent The parent object
 * @return a new progress bar widget handle or @c NULL, on errors
 *
 * This function inserts a new progress bar widget on the canvas.
 *
 * @ingroup Progressbar
 */
EAPI Evas_Object                 *elm_progressbar_add(Evas_Object *parent);

/**
 * Set whether a given progress bar widget is at "pulsing mode" or
 * not.
 *
 * @param obj The progress bar object
 * @param pulse @c EINA_TRUE to put @p obj in pulsing mode,
 * @c EINA_FALSE to put it back to its default one
 *
 * By default, progress bars will display values from the low to
 * high value boundaries. There are, though, contexts in which the
 * progress of a given task is @b unknown.  For such cases,
 * one can set a progress bar widget to a "pulsing state", to give
 * the user an idea that some computation is being held, but
 * without exact progress values. In the default theme, it will
 * animate its bar with the contents filling in constantly and back
 * to non-filled, in a loop. To start and stop this pulsing
 * animation, one has to explicitly call elm_progressbar_pulse().
 *
 * @see elm_progressbar_pulse_get()
 * @see elm_progressbar_pulse()
 *
 * @ingroup Progressbar
 */
EAPI void                         elm_progressbar_pulse_set(Evas_Object *obj, Eina_Bool pulse);

/**
 * Get whether a given progress bar widget is at "pulsing mode" or
 * not.
 *
 * @param obj The progress bar object
 * @return @c EINA_TRUE, if @p obj is in pulsing mode, @c EINA_FALSE
 * if it's in the default one (and on errors)
 *
 * @ingroup Progressbar
 */
EAPI Eina_Bool                    elm_progressbar_pulse_get(const Evas_Object *obj);

/**
 * Start/stop a given progress bar "pulsing" animation, if its
 * under that mode
 *
 * @param obj The progress bar object
 * @param state @c EINA_TRUE, to @b start the pulsing animation,
 * @c EINA_FALSE to @b stop it
 *
 * @note This call won't do anything if @p obj is not under "pulsing mode".
 *
 * @see elm_progressbar_pulse_set() for more details.
 *
 * @ingroup Progressbar
 */
EAPI void                         elm_progressbar_pulse(Evas_Object *obj, Eina_Bool state);

/**
 * Set the progress value (in percentage) on a given progress bar
 * widget
 *
 * @param obj The progress bar object
 * @param val The progress value (@b must be between @c 0.0 and @c
 * 1.0)
 *
 * Use this call to set progress bar levels.
 *
 * @note If you passes a value out of the specified range for @p
 * val, it will be interpreted as the @b closest of the @b boundary
 * values in the range.
 *
 * @ingroup Progressbar
 */
EAPI void                         elm_progressbar_value_set(Evas_Object *obj, double val);

/**
 * Get the progress value (in percentage) on a given progress bar
 * widget
 *
 * @param obj The progress bar object
 * @return The value of the progressbar
 *
 * @see elm_progressbar_value_set() for more details
 *
 * @ingroup Progressbar
 */
EAPI double                       elm_progressbar_value_get(const Evas_Object *obj);

/**
 * Set the (exact) length of the bar region of a given progress bar
 * widget
 *
 * @param obj The progress bar object
 * @param size The length of the progress bar's bar region
 *
 * This sets the minimum width (when in horizontal mode) or height
 * (when in vertical mode) of the actual bar area of the progress
 * bar @p obj. This in turn affects the object's minimum size. Use
 * this when you're not setting other size hints expanding on the
 * given direction (like weight and alignment hints) and you would
 * like it to have a specific size.
 *
 * @note Icon, label and unit text around @p obj will require their
 * own space, which will make @p obj to require more the @p size,
 * actually.
 *
 * @see elm_progressbar_span_size_get()
 *
 * @ingroup Progressbar
 */
EAPI void                         elm_progressbar_span_size_set(Evas_Object *obj, Evas_Coord size);

/**
 * Get the length set for the bar region of a given progress bar
 * widget
 *
 * @param obj The progress bar object
 * @return The length of the progress bar's bar region
 *
 * If that size was not set previously, with
 * elm_progressbar_span_size_set(), this call will return @c 0.
 *
 * @ingroup Progressbar
 */
EAPI Evas_Coord                   elm_progressbar_span_size_get(const Evas_Object *obj);

/**
 * Set the format string for a given progress bar widget's units
 * label
 *
 * @param obj The progress bar object
 * @param format The format string for @p obj's units label
 *
 * If @c NULL is passed on @p format, it will make @p obj's units
 * area to be hidden completely. If not, it'll set the <b>format
 * string</b> for the units label's @b text. The units label is
 * provided a floating point value, so the units text is up display
 * at most one floating point value. Note that the units label is
 * optional. Use a format string such as "%1.2f meters" for
 * example.
 *
 * @note The default format string for a progress bar is an integer
 * percentage, as in @c "%.0f %%".
 *
 * @see elm_progressbar_unit_format_get()
 *
 * @ingroup Progressbar
 */
EAPI void                         elm_progressbar_unit_format_set(Evas_Object *obj, const char *format);

/**
 * Retrieve the format string set for a given progress bar widget's
 * units label
 *
 * @param obj The progress bar object
 * @return The format set string for @p obj's units label or
 * @c NULL, if none was set (and on errors)
 *
 * @see elm_progressbar_unit_format_set() for more details
 *
 * @ingroup Progressbar
 */
EAPI const char                  *elm_progressbar_unit_format_get(const Evas_Object *obj);

/**
 * Set the format function pointer for the units label
 *
 * @param obj The progress bar object
 * @param func The unit format function
 * @param free_func The freeing function for the format string.
 *
 * Set the callback function to format the unit string.
 *
 * @see elm_progressbar_unit_format_set() for more info on how this works.
 *
 * @since 1.7
 *
 * @ingroup Progressbar
 */
EAPI void                         elm_progressbar_unit_format_function_set(Evas_Object *obj, char *(func)(double), void (*free_func) (char *));

/**
 * Set the orientation of a given progress bar widget
 *
 * @param obj The progress bar object
 * @param horizontal Use @c EINA_TRUE to make @p obj to be
 * @b horizontal, @c EINA_FALSE to make it @b vertical
 *
 * Use this function to change how your progress bar is to be
 * disposed: vertically or horizontally.
 *
 * @see elm_progressbar_horizontal_get()
 *
 * @ingroup Progressbar
 */
EAPI void                         elm_progressbar_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * Retrieve the orientation of a given progress bar widget
 *
 * @param obj The progress bar object
 * @return @c EINA_TRUE, if @p obj is set to be @b horizontal,
 * @c EINA_FALSE if it's @b vertical (and on errors)
 *
 * @see elm_progressbar_horizontal_set() for more details
 *
 * @ingroup Progressbar
 */
EAPI Eina_Bool                    elm_progressbar_horizontal_get(const Evas_Object *obj);

/**
 * Invert a given progress bar widget's displaying values order
 *
 * @param obj The progress bar object
 * @param inverted Use @c EINA_TRUE to make @p obj inverted,
 * @c EINA_FALSE to bring it back to default, non-inverted values.
 *
 * A progress bar may be @b inverted, in which state it gets its
 * values inverted, with high values being on the left or top and
 * low values on the right or bottom, as opposed to normally have
 * the low values on the former and high values on the latter,
 * respectively, for horizontal and vertical modes.
 *
 * @see elm_progressbar_inverted_get()
 *
 * @ingroup Progressbar
 */
EAPI void                         elm_progressbar_inverted_set(Evas_Object *obj, Eina_Bool inverted);

/**
 * Get whether a given progress bar widget's displaying values are
 * inverted or not
 *
 * @param obj The progress bar object
 * @return @c EINA_TRUE, if @p obj has inverted values,
 * @c EINA_FALSE otherwise (and on errors)
 *
 * @see elm_progressbar_inverted_set() for more details
 *
 * @ingroup Progressbar
 */
EAPI Eina_Bool                    elm_progressbar_inverted_get(const Evas_Object *obj);

/**
 * Set the progress value (in percentage) on a given progress bar
 * widget for the given part name
 *
 * @since 1.8
 *
 * @param obj  The progress bar object
 * @param part The partname to which val have to set
 * @param val The progress value (@b must be between @c 0.0 and @c
 * 1.0)
 *
 * Use this call to set progress bar status for more than one progress status .
 *
 * @ingroup Progressbar
 */
EAPI void                         elm_progressbar_part_value_set(Evas_Object *obj, const char *part, double val);

/**
 * Get the progress value (in percentage) on a given progress bar
 * widget for a particular part
 *
 * @since 1.8
 *
 * @param obj The progress bar object
 * @param part The part name of the progress bar
 * @return The value of the progressbar
 *
 * @see elm_progressbar_value_set() for more details
 *
 * @ingroup Progressbar
 */
EAPI double                       elm_progressbar_part_value_get(const Evas_Object *obj, const char *part);
