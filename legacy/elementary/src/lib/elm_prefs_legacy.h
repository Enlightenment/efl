/**
 * @ingroup Prefs
 *
 * @{
 */
EAPI Eina_Bool elm_prefs_page_widget_common_add(Evas_Object *prefs,
                                                Evas_Object *obj);
/**
 * Set file and page to populate a given prefs widget's interface.
 *
 * @param o A prefs widget handle
 * @param file The @b .epb (binary) file to get contents from
 * @param page The page, inside @a file, where to get item contents from
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * Elm prefs widgets start blank, with no child widgets. It's meant to
 * have its viewport populated with child elements coming from a
 * declaration file. That file (usually with @b .epb extension), is a
 * binary format (Eet) one, coming from a human-readable textual
 * declaration. This textual form (usually with @b .epc extension) is
 * translated to the binary form by means of the @b prefs_cc compiler.
 *
 * With this function, one thus populates a prefs widget with UI
 * elements.
 *
 * If @a file is @c NULL, "elm_app_data_dir_get()/preferences.epb"
 * will be used, by default. If @a file is a @b relative path, the
 * prefix "elm_app_data_dir_get()/" will be implicitly used with it.
 * If @a page is @c NULL, it is considered "main", as default.
 *
 * @warning If your binary is not properly installed and
 * elm_app_data_dir_get() can't be figured out, a fallback value of
 * "." will be tryed, instead.
 *
 * @see elm_prefs_file_get()
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_file_set(Evas_Object *o,
                                           const char *file,
                                           const char *page);

/**
 * Retrieve file and page bound to a given prefs widget.
 *
 * @param o A prefs widget handle
 * @param file Where to store the file @a o is bound to
 * @param page Where to store the page @a o is bound to
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * @note Use @c NULL pointers on the components you're not
 * interested in: they'll be ignored by the function.
 *
 * @see elm_prefs_file_set() for more information
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_file_get(const Evas_Object *o,
                                           const char **file,
                                           const char **page);
/**
 * Set user data for a given prefs widget
 *
 * @param obj A prefs widget handle
 * @param prefs_data A valid prefs_data handle
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * Once a prefs widget is created, after elm_prefs_file_set() is
 * issued on it, all of its UI elements will get default values, when
 * declared on that file. To fetch an user's own, personal set of
 * those values, one gets to pair a <b>prefs data</b> handle to the
 * prefs widget. This is what this call is intended for.
 *
 * Prefs data values from @a prefs_data with keys matching the ones
 * present on the file passed on elm_prefs_file_set() to @a obj will
 * have their values applied to the respective UI elements of the
 * widget.
 *
 * When @a obj dies, the values of the elements declared on its @b
 * .epb file (the one set on elm_prefs_file_set()) marked as permanent
 * <b>will be written back</b> to @a prefs_data, if it is writable.
 * One is also able to make this writing event to take place
 * automatically after each UI element modification by using
 * elm_prefs_autosave_set().
 *
 * @note @a obj will keep a reference of its own for @a prefs_data,
 * but you should still unreference it by yourself, after the widget
 * is gone.
 *
 * @see elm_prefs_data_get()
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_data_set(Evas_Object *obj,
                                           Elm_Prefs_Data *data);

/**
 * Retrieve user data for a given prefs widget
 *
 * @param obj A prefs widget handle
 * @param prefs_data A valid prefs_data handle
 * @return A pointer to the user data of a given prefs widget on success.
 *         @c NULL otherwise.
 *
 * @see elm_prefs_data_set() for more details
 *
 * @since 1.8
 */
EAPI Elm_Prefs_Data    *elm_prefs_data_get(const Evas_Object *o);

/**
 * Set whether a given prefs widget should save its values back (on
 * the user data file, if set) automatically on every UI element
 * changes.
 *
 * @param prefs A valid prefs widget handle
 * @param autosave @c EINA_TRUE to save automatically, @c EINA_FALSE
 *                 otherwise.
 *
 * If @a autosave is @c EINA_TRUE, every call to
 * elm_prefs_item_value_set(), every
 * Elm_Prefs_Data_Event_Type::ELM_PREFS_DATA_EVENT_ITEM_CHANGED event
 * coming for its prefs data and every UI element direct value
 * changing will implicitly make the prefs values to be flushed back
 * to it prefs data. If a prefs data handle with no writing
 * permissions or no prefs data is set on @a prefs, naturally nothing
 * will happen.
 *
 * @see elm_prefs_autosave_get()
 *
 * @since 1.8
 */
EAPI void               elm_prefs_autosave_set(Evas_Object *prefs,
                                               Eina_Bool autosave);

/**
 * Get whether a given prefs widget is saving its values back
 * automatically on changes.
 *
 * @param prefs A valid prefs widget handle
 * @return @c EINA_TRUE if @a prefs is saving automatically,
 *         @c EINA_FALSE otherwise.
 *
 * @see elm_prefs_autosave_set(), for more details
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_autosave_get(const Evas_Object *prefs);

/**
 * Reset the values of a given prefs widget to a previous state.
 *
 * @param prefs A valid prefs widget handle
 * @param mode The reset mode to apply on @a prefs
 *
 * As can be seen on #Elm_Prefs_Reset_Mode, there are two possible
 * actions to be taken by this call -- either to reset @a prefs'
 * values to the defaults (declared on the @c .epb file it is bound
 * to) or to reset to the state they were before the last modification
 * it got.
 *
 * @since 1.8
 */
EAPI void               elm_prefs_reset(Evas_Object *prefs,
                                        Elm_Prefs_Reset_Mode mode);

/**
 * Set the value on a given prefs widget's item.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection)
 * @param value The value to set on the item. It should be typed as
 *              the item expects, preferably, or a conversion will
 *              take place
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * This will change the value of item named @a name programatically.
 *
 * @see elm_prefs_item_value_get()
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_value_set(Evas_Object *prefs,
                                                 const char *name,
                                                 const Eina_Value *value);

/**
 * Retrieve the value of a given prefs widget's item.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to get value from
 * @param value Where to store the value of the item. It will be
 *              overwritten and setup with the type the item
 *              is bound to
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * This will retrieve the value of item named @a name.
 *
 * @see elm_prefs_item_value_set()
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_value_get(const Evas_Object *prefs,
                                                 const char *name,
                                                 Eina_Value *value);
/**
 * Retrieve the Elementary widget bound to a given prefs widget's
 * item.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to get object from
 * @return A valid widget handle, on success, or @c NULL, otherwise
 *
 * This will retrieve a handle to the real widget implementing a given
 * item of @a prefs, <b>for read-only</b> actions.
 *
 * @warning You should @b never modify the state of the returned
 * widget, because it's meant to be managed by @a prefs, solely.
 *
 * @see elm_prefs_item_value_set()
 *
 * @since 1.8
 */
EAPI const Evas_Object *elm_prefs_item_object_get(Evas_Object *prefs,
                                                  const char *name);

/**
 * Set whether the widget bound to given prefs widget's item should be
 * visible or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to change visibility of
 * @param visible @c EINA_TRUE, to make it visible, @c EINA_FALSE
 *                otherwise
 *
 * Each prefs item may have a default visibility state, declared on
 * the @c .epb @a prefs it was loaded with. By this call one may alter
 * that state, programatically.
 *
 * @see elm_prefs_item_visible_get()
 *
 * @since 1.8
 */
EAPI void               elm_prefs_item_visible_set(Evas_Object *prefs,
                                                   const char *name,
                                                   Eina_Bool visible);

/**
 * Retrieve whether the widget bound to a given prefs widget's item is
 * visible or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to get visibility state from
 * @return @c EINA_TRUE, if it is visible, @c EINA_FALSE
 *         otherwise
 *
 * @see elm_prefs_item_visible_set() for more details
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_visible_get(const Evas_Object *prefs,
                                                   const char *name);

/**
 * Set whether the widget bound to a given prefs widget's item is
 * disabled or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to act on
 * @param disabled @c EINA_TRUE, to make it disabled, @c EINA_FALSE
 *                 otherwise
 *
 * @see elm_prefs_item_disabled_get()
 *
 * @since 1.8
 */
EAPI void               elm_prefs_item_disabled_set(Evas_Object *prefs,
                                                    const char *name,
                                                    Eina_Bool disabled);

/**
 * Retrieve whether the widget bound to a given prefs widget's item is
 * disabled or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to get disabled state from
 * @return @c EINA_TRUE, if it is disabled, @c EINA_FALSE
 *         otherwise
 *
 * @see elm_prefs_item_disabled_set()
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_disabled_get(const Evas_Object *prefs,
                                                    const char *name);

/**
 * Set whether the widget bound to a given prefs widget's item is
 * editable or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to act on
 * @param editable @c EINA_TRUE, to make it editable, @c EINA_FALSE
 *                 otherwise
 *
 * @note Only @c TEXT or @c TEXTAREA items' default widgets implement
 * the 'editable' property. Custom registered widgets may as well
 * implement them.
 *
 * @see elm_prefs_item_editable_get()
 *
 * @since 1.8
 */
EAPI void               elm_prefs_item_editable_set(Evas_Object *prefs,
                                                    const char *name,
                                                    Eina_Bool editable);

/**
 * Retrieve whether the widget bound to a given prefs widget's item is
 * editable or not.
 *
 * @param prefs A valid prefs widget handle
 * @param name The name of the item (as declared in the prefs
 *             collection) to get editable state from
 * @return @c EINA_TRUE, if it is editable, @c EINA_FALSE
 *         otherwise
 *
 * @see elm_prefs_item_editable_set() for more details
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_editable_get(const Evas_Object *prefs,
                                                    const char *name);

/**
 * "Swallows" an object into a SWALLOW item of a prefs widget.
 *
 * @param obj A valid prefs widget handle
 * @param name the name of the SWALLOW item (as declared in the prefs
 *             collection)
 * @param child The object to occupy the item
 * @return @c EINA_TRUE, on success, @c EINA_FALSE otherwise
 *
 * @see elm_prefs_item_swallow() for more details
 *
 * @since 1.8
 */
EAPI Eina_Bool          elm_prefs_item_swallow(Evas_Object *obj,
                                               const char *name,
                                               Evas_Object *child);

/**
 * Unswallow an object from a SWALLOW item of a prefs widget.
 *
 * @param obj A valid prefs widget handle
 * @param name the name of the SWALLOW item (as declared in the prefs
 *             collection)
 * @return The unswallowed object, or NULL on errors
 *
 * @see elm_prefs_item_unswallow() for more details
 *
 * @since 1.8
 */
EAPI Evas_Object       *elm_prefs_item_unswallow(Evas_Object *obj,
                                                 const char *name);
/**
 * @}
 */
