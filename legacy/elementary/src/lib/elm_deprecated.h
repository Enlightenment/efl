EINA_DEPRECATED EAPI void          elm_gen_clear(Evas_Object *obj);
EINA_DEPRECATED EAPI void          elm_gen_item_selected_set(Elm_Gen_Item *it, Eina_Bool selected);
EINA_DEPRECATED EAPI Eina_Bool     elm_gen_item_selected_get(const Elm_Gen_Item *it);
EINA_DEPRECATED EAPI void          elm_gen_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select);
EINA_DEPRECATED EAPI Eina_Bool     elm_gen_always_select_mode_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void          elm_gen_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select);
EINA_DEPRECATED EAPI Eina_Bool     elm_gen_no_select_mode_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void          elm_gen_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);
EINA_DEPRECATED EAPI void          elm_gen_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);
EINA_DEPRECATED EAPI void          elm_gen_page_relative_set(Evas_Object *obj, double h_pagerel, double v_pagerel);
EINA_DEPRECATED EAPI void          elm_gen_page_relative_get(const Evas_Object *obj, double *h_pagerel, double *v_pagerel);

EINA_DEPRECATED EAPI void          elm_gen_page_size_set(Evas_Object *obj, Evas_Coord h_pagesize, Evas_Coord v_pagesize);
EINA_DEPRECATED EAPI void          elm_gen_current_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);
EINA_DEPRECATED EAPI void          elm_gen_last_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);
EINA_DEPRECATED EAPI void          elm_gen_page_show(const Evas_Object *obj, int h_pagenumber, int v_pagenumber);
EINA_DEPRECATED EAPI void          elm_gen_page_bring_in(const Evas_Object *obj, int h_pagenumber, int v_pagenumber);
EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_first_item_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_last_item_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_item_next_get(const Elm_Gen_Item *it);
EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_item_prev_get(const Elm_Gen_Item *it);
EINA_DEPRECATED EAPI Evas_Object  *elm_gen_item_widget_get(const Elm_Gen_Item *it);

/**
 * Get the widget object's handle which contains a given item
 *
 * @param it The Elementary object item
 * @return The widget object
 *
 * @note This returns the widget object itself that an item belongs to.
 * @note Every elm_object_item supports this API
 * @deprecated Use elm_object_item_widget_get() instead
 * @ingroup General
 */
EINA_DEPRECATED EAPI Evas_Object                 *elm_object_item_object_get(const Elm_Object_Item *it);

/**
 * @defgroup Toggle Toggle
 *
 * @image html img/widget/toggle/preview-00.png
 * @image latex img/widget/toggle/preview-00.eps
 *
 * @brief A toggle is a slider which can be used to toggle between
 * two values.  It has two states: on and off.
 *
 * This widget is deprecated. Please use elm_check_add() instead using the
 * toggle style like:
 *
 * @code
 * obj = elm_check_add(parent);
 * elm_object_style_set(obj, "toggle");
 * elm_object_part_text_set(obj, "on", "ON");
 * elm_object_part_text_set(obj, "off", "OFF");
 * @endcode
 *
 * Signals that you can add callbacks for are:
 * @li "changed" - Whenever the toggle value has been changed.  Is not called
 *                 until the toggle is released by the cursor (assuming it
 *                 has been triggered by the cursor in the first place).
 *
 * Default contents parts of the toggle widget that you can use for are:
 * @li "icon" - An icon of the toggle
 *
 * Default text parts of the toggle widget that you can use for are:
 * @li "elm.text" - Label of the toggle
 *
 * @ref tutorial_toggle show how to use a toggle.
 * @{
 */

/**
 * @brief Add a toggle to @p parent.
 *
 * @param parent The parent object
 *
 * @return The toggle object
 */
EINA_DEPRECATED EAPI Evas_Object *elm_toggle_add(Evas_Object *parent);

/**
 * @brief Sets the label to be displayed with the toggle.
 *
 * @param obj The toggle object
 * @param label The label to be displayed
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Gets the label of the toggle
 *
 * @param obj  toggle object
 * @return The label of the toggle
 *
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_toggle_label_get(const Evas_Object *obj);

/**
 * @brief Set the icon used for the toggle
 *
 * @param obj The toggle object
 * @param icon The icon object for the button
 *
 * Once the icon object is set, a previously set one will be deleted
 * If you want to keep that old content object, use the
 * elm_toggle_icon_unset() function.
 *
 * @deprecated use elm_object_part_content_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * @brief Get the icon used for the toggle
 *
 * @param obj The toggle object
 * @return The icon object that is being used
 *
 * Return the icon object which is set for this widget.
 *
 * @see elm_toggle_icon_set()
 *
 * @deprecated use elm_object_part_content_get() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_toggle_icon_get(const Evas_Object *obj);

/**
 * @brief Unset the icon used for the toggle
 *
 * @param obj The toggle object
 * @return The icon object that was being used
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @see elm_toggle_icon_set()
 *
 * @deprecated use elm_object_part_content_unset() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_toggle_icon_unset(Evas_Object *obj);

/**
 * @brief Sets the labels to be associated with the on and off states of the toggle.
 *
 * @param obj The toggle object
 * @param onlabel The label displayed when the toggle is in the "on" state
 * @param offlabel The label displayed when the toggle is in the "off" state
 *
 * @deprecated use elm_object_part_text_set() for "on" and "off" parts
 * instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_states_labels_set(Evas_Object *obj, const char *onlabel, const char *offlabel);

/**
 * @brief Gets the labels associated with the on and off states of the
 * toggle.
 *
 * @param obj The toggle object
 * @param onlabel A char** to place the onlabel of @p obj into
 * @param offlabel A char** to place the offlabel of @p obj into
 *
 * @deprecated use elm_object_part_text_get() for "on" and "off" parts
 * instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_states_labels_get(const Evas_Object *obj, const char **onlabel, const char **offlabel);

/**
 * @brief Sets the state of the toggle to @p state.
 *
 * @param obj The toggle object
 * @param state The state of @p obj
 *
 * @deprecated use elm_check_state_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_state_set(Evas_Object *obj, Eina_Bool state);

/**
 * @brief Gets the state of the toggle to @p state.
 *
 * @param obj The toggle object
 * @return The state of @p obj
 *
 * @deprecated use elm_check_state_get() instead.
 */
EINA_DEPRECATED EAPI Eina_Bool    elm_toggle_state_get(const Evas_Object *obj);

/**
 * @brief Sets the state pointer of the toggle to @p statep.
 *
 * @param obj The toggle object
 * @param statep The state pointer of @p obj
 *
 * @deprecated use elm_check_state_pointer_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toggle_state_pointer_set(Evas_Object *obj, Eina_Bool *statep);

/**
 * @}
 */

EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_add(Evas_Object *parent);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_single_line_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_password_set(Evas_Object *obj, Eina_Bool password);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_password_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_entry_set(Evas_Object *obj, const char *entry);
EINA_DEPRECATED EAPI const char  *elm_scrolled_entry_entry_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_entry_append(Evas_Object *obj, const char *entry);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_is_empty(const Evas_Object *obj);
EINA_DEPRECATED EAPI const char  *elm_scrolled_entry_selection_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_entry_insert(Evas_Object *obj, const char *entry);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_line_wrap_set(Evas_Object *obj, Elm_Wrap_Type wrap);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_editable_set(Evas_Object *obj, Eina_Bool editable);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_editable_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_select_none(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_select_all(Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_next(Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_prev(Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_up(Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_down(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_begin_set(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_end_set(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_line_begin_set(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_line_end_set(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_selection_begin(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_selection_end(Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_is_format_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_is_visible_format_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI const char  *elm_scrolled_entry_cursor_content_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_pos_set(Evas_Object *obj, int pos);
EINA_DEPRECATED EAPI int          elm_scrolled_entry_cursor_pos_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_selection_cut(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_selection_copy(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_selection_paste(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_context_menu_clear(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_context_menu_disabled_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_scrollbar_policy_set(Evas_Object *obj, Elm_Scroller_Policy h, Elm_Scroller_Policy v);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_icon_set(Evas_Object *obj, Evas_Object *icon);
EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_icon_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_icon_unset(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_icon_visible_set(Evas_Object *obj, Eina_Bool setting);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_end_set(Evas_Object *obj, Evas_Object *end);
EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_end_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_end_unset(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_end_visible_set(Evas_Object *obj, Eina_Bool setting);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_item_provider_append(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * entry, const char *item), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_item_provider_prepend(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * entry, const char *item), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_item_provider_remove(Evas_Object *obj, Evas_Object * (*func)(void *data, Evas_Object * entry, const char *item), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_text_filter_append(Evas_Object *obj, void (*func)(void *data, Evas_Object *entry, char **text), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_text_filter_prepend(Evas_Object *obj, void (*func)(void *data, Evas_Object *entry, char **text), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_text_filter_remove(Evas_Object *obj, void (*func)(void *data, Evas_Object *entry, char **text), void *data);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_file_get(const Evas_Object *obj, const char **file, Elm_Text_Format *format);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_file_save(Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_autosave_set(Evas_Object *obj, Eina_Bool autosave);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_autosave_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void         elm_scrolled_entry_cnp_textonly_set(Evas_Object *obj, Eina_Bool textonly);
EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cnp_textonly_get(Evas_Object *obj);

/**
 * Set the text to show in the anchorblock
 *
 * Sets the text of the anchorblock to @p text. This text can include markup
 * format tags, including <c>\<a href=anchorname\></a></c> to begin a segment
 * of text that will be specially styled and react to click events, ended
 * with either of \</a\> or \</\>. When clicked, the anchor will emit an
 * "anchor,clicked" signal that you can attach a callback to with
 * evas_object_smart_callback_add(). The name of the anchor given in the
 * event info struct will be the one set in the href attribute, in this
 * case, anchorname.
 *
 * Other markup can be used to style the text in different ways, but it's
 * up to the style defined in the theme which tags do what.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_anchorblock_text_set(Evas_Object *obj, const char *text);

/**
 * Get the markup text set for the anchorblock
 *
 * Retrieves the text set on the anchorblock, with markup tags included.
 *
 * @param obj The anchorblock object
 * @return The markup text set or @c NULL if nothing was set or an error
 * occurred
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char *elm_anchorblock_text_get(const Evas_Object *obj);

/**
 * Set the text to show in the anchorview
 *
 * Sets the text of the anchorview to @p text. This text can include markup
 * format tags, including <c>\<a href=anchorname\></c> to begin a segment of
 * text that will be specially styled and react to click events, ended with
 * either of \</a\> or \</\>. When clicked, the anchor will emit an
 * "anchor,clicked" signal that you can attach a callback to with
 * evas_object_smart_callback_add(). The name of the anchor given in the
 * event info struct will be the one set in the href attribute, in this
 * case, anchorname.
 *
 * Other markup can be used to style the text in different ways, but it's
 * up to the style defined in the theme which tags do what.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_anchorview_text_set(Evas_Object *obj, const char *text);

/**
 * Get the markup text set for the anchorview
 *
 * Retrieves the text set on the anchorview, with markup tags included.
 *
 * @param obj The anchorview object
 * @return The markup text set or @c NULL if nothing was set or an error
 * occurred
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char *elm_anchorview_text_get(const Evas_Object *obj);

/**
 * @brief Get the ctxpopup item's disabled/enabled state.
 *
 * @param it Ctxpopup item to be enabled/disabled
 * @return disabled @c EINA_TRUE, if disabled, @c EINA_FALSE otherwise
 *
 * @see elm_ctxpopup_item_disabled_set()
 * @deprecated use elm_object_item_disabled_get() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_ctxpopup_item_disabled_get(const Elm_Object_Item *it);

/**
 * @brief Set the ctxpopup item's state as disabled or enabled.
 *
 * @param it Ctxpopup item to be enabled/disabled
 * @param disabled @c EINA_TRUE to disable it, @c EINA_FALSE to enable it
 *
 * When disabled the item is greyed out to indicate it's state.
 * @deprecated use elm_object_item_disabled_set() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI void         elm_ctxpopup_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled);

/**
 * @brief Get the icon object for the given ctxpopup item.
 *
 * @param it Ctxpopup item
 * @return icon object or @c NULL, if the item does not have icon or an error
 * occurred
 *
 * @see elm_ctxpopup_item_append()
 * @see elm_ctxpopup_item_icon_set()
 *
 * @deprecated use elm_object_item_part_content_get() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI Evas_Object *elm_ctxpopup_item_icon_get(const Elm_Object_Item *it);

/**
 * @brief Sets the side icon associated with the ctxpopup item
 *
 * @param it Ctxpopup item
 * @param icon Icon object to be set
 *
 * Once the icon object is set, a previously set one will be deleted.
 * @warning Setting the same icon for two items will cause the icon to
 * dissapear from the first item.
 *
 * @see elm_ctxpopup_item_append()
 *
 * @deprecated use elm_object_item_part_content_set() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI void         elm_ctxpopup_item_icon_set(Elm_Object_Item *it, Evas_Object *icon);

/**
 * @brief Get the label for the given ctxpopup item.
 *
 * @param it Ctxpopup item
 * @return label string or @c NULL, if the item does not have label or an
 * error occured
 *
 * @see elm_ctxpopup_item_append()
 * @see elm_ctxpopup_item_label_set()
 *
 * @deprecated use elm_object_item_text_get() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI const char  *elm_ctxpopup_item_label_get(const Elm_Object_Item *it);

/**
 * @brief (Re)set the label on the given ctxpopup item.
 *
 * @param it Ctxpopup item
 * @param label String to set as label
 *
 * @deprecated use elm_object_item_text_set() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI void         elm_ctxpopup_item_label_set(Elm_Object_Item *it, const char *label);

/**
 * @brief Set an elm widget as the content of the ctxpopup.
 *
 * @param obj Ctxpopup object
 * @param content Content to be swallowed
 *
 * If the content object is already set, a previous one will bedeleted. If
 * you want to keep that old content object, use the
 * elm_ctxpopup_content_unset() function.
 *
 * @warning Ctxpopup can't hold both a item list and a content at the same
 * time. When a content is set, any previous items will be removed.
 *
 * @deprecated use elm_object_content_set() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI void         elm_ctxpopup_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Unset the ctxpopup content
 *
 * @param obj Ctxpopup object
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @deprecated use elm_object_content_unset()
 *
 * @see elm_ctxpopup_content_set()
 *
 * @deprecated use elm_object_content_unset() instead
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI Evas_Object *elm_ctxpopup_content_unset(Evas_Object *obj);

/**
 * @brief Delete the given item in a ctxpopup object.
 *
 * @param it Ctxpopup item to be deleted
 *
 * @deprecated Use elm_object_item_del() instead
 * @see elm_ctxpopup_item_append()
 *
 * @ingroup Ctxpopup
 */
EINA_DEPRECATED EAPI void                         elm_ctxpopup_item_del(Elm_Object_Item *it);

/**
 * Set the label for a given file selector button widget
 *
 * @param obj The file selector button widget
 * @param label The text label to be displayed on @p obj
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_fileselector_button_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label set for a given file selector button widget
 *
 * @param obj The file selector button widget
 * @return The button label
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char *elm_fileselector_button_label_get(const Evas_Object *obj);

/**
 * Set the icon on a given file selector button widget
 *
 * @param obj The file selector button widget
 * @param icon The icon object for the button
 *
 * Once the icon object is set, a previously set one will be
 * deleted. If you want to keep the latter, use the
 * elm_fileselector_button_icon_unset() function.
 *
 * @deprecated Use elm_object_part_content_set() instead
 * @see elm_fileselector_button_icon_get()
 */
EINA_DEPRECATED EAPI void                        elm_fileselector_button_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Get the icon set for a given file selector button widget
 *
 * @param obj The file selector button widget
 * @return The icon object currently set on @p obj or @c NULL, if
 * none is
 *
 * @deprecated Use elm_object_part_content_get() instead
 * @see elm_fileselector_button_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object                *elm_fileselector_button_icon_get(const Evas_Object *obj);

/**
 * Unset the icon used in a given file selector button widget
 *
 * @param obj The file selector button widget
 * @return The icon object that was being used on @p obj or @c
 * NULL, on errors
 *
 * Unparent and return the icon object which was set for this
 * widget.
 *
 * @deprecated Use elm_object_part_content_unset() instead
 * @see elm_fileselector_button_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object                *elm_fileselector_button_icon_unset(Evas_Object *obj);

/**
 * Set the label for a given file selector entry widget's button
 *
 * @param obj The file selector entry widget
 * @param label The text label to be displayed on @p obj widget's
 * button
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_fileselector_entry_button_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label set for a given file selector entry widget's button
 *
 * @param obj The file selector entry widget
 * @return The widget button's label
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char *elm_fileselector_entry_button_label_get(const Evas_Object *obj);

/**
 * Set the icon on a given file selector entry widget's button
 *
 * @param obj The file selector entry widget
 * @param icon The icon object for the entry's button
 *
 * Once the icon object is set, a previously set one will be
 * deleted. If you want to keep the latter, use the
 * elm_fileselector_entry_button_icon_unset() function.
 *
 * @deprecated Use elm_object_part_content_set() instead
 * @see elm_fileselector_entry_button_icon_get()
 */
EINA_DEPRECATED EAPI void                        elm_fileselector_entry_button_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Get the icon set for a given file selector entry widget's button
 *
 * @param obj The file selector entry widget
 * @return The icon object currently set on @p obj widget's button
 * or @c NULL, if none is
 *
 * @deprecated Use elm_object_part_content_get() instead
 * @see elm_fileselector_entry_button_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object                *elm_fileselector_entry_button_icon_get(const Evas_Object *obj);

/**
 * Unset the icon used in a given file selector entry widget's
 * button
 *
 * @param obj The file selector entry widget
 * @return The icon object that was being used on @p obj widget's
 * button or @c NULL, on errors
 *
 * Unparent and return the icon object which was set for this
 * widget's button.
 *
 * @deprecated Use elm_object_part_content_unset() instead
 * @see elm_fileselector_entry_button_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object                *elm_fileselector_entry_button_icon_unset(Evas_Object *obj);

/**
 * @brief Sets the content of the hover object and the direction in which it
 * will pop out.
 *
 * @param obj The hover object
 * @param swallow The direction that the object will be displayed
 * at. Accepted values are "left", "top-left", "top", "top-right",
 * "right", "bottom-right", "bottom", "bottom-left", "middle" and
 * "smart".
 * @param content The content to place at @p swallow
 *
 * Once the content object is set for a given direction, a previously
 * set one (on the same direction) will be deleted. If you want to
 * keep that old content object, use the elm_object_part_content_unset()
 * function.
 *
 * All directions may have contents at the same time, except for
 * "smart". This is a special placement hint and its use case
 * independs of the calculations coming from
 * elm_hover_best_content_location_get(). Its use is for cases when
 * one desires only one hover content, but with a dynamic special
 * placement within the hover area. The content's geometry, whenever
 * it changes, will be used to decide on a best location, not
 * extrapolating the hover's parent object view to show it in (still
 * being the hover's target determinant of its medium part -- move and
 * resize it to simulate finger sizes, for example). If one of the
 * directions other than "smart" are used, a previously content set
 * using it will be deleted, and vice-versa.
 *
 * @deprecated Use elm_object_part_content_set() instead
 */
EINA_DEPRECATED EAPI void         elm_hover_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content);

/**
 * @brief Get the content of the hover object, in a given direction.
 *
 * Return the content object which was set for this widget in the
 * @p swallow direction.
 *
 * @param obj The hover object
 * @param swallow The direction that the object was display at.
 * @return The content that was being used
 *
 * @deprecated Use elm_object_part_content_get() instead
 * @see elm_object_part_content_set()
 */
EINA_DEPRECATED EAPI Evas_Object *elm_hover_content_get(const Evas_Object *obj, const char *swallow);

/**
 * @brief Unset the content of the hover object, in a given direction.
 *
 * Unparent and return the content object set at @p swallow direction.
 *
 * @param obj The hover object
 * @param swallow The direction that the object was display at.
 * @return The content that was being used.
 *
 * @deprecated Use elm_object_part_content_unset() instead
 * @see elm_object_part_content_set()
 */
EINA_DEPRECATED EAPI Evas_Object *elm_hover_content_unset(Evas_Object *obj, const char *swallow);

/**
 * @brief Set the hoversel button label
 *
 * @param obj The hoversel object
 * @param label The label text.
 *
 * This sets the label of the button that is always visible (before it is
 * clicked and expanded).
 *
 * @deprecated elm_object_text_set()
 */
EINA_DEPRECATED EAPI void         elm_hoversel_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the hoversel button label
 *
 * @param obj The hoversel object
 * @return The label text.
 *
 * @deprecated elm_object_text_get()
 */
EINA_DEPRECATED EAPI const char  *elm_hoversel_label_get(const Evas_Object *obj);

/**
 * @brief Set the icon of the hoversel button
 *
 * @param obj The hoversel object
 * @param icon The icon object
 *
 * Sets the icon of the button that is always visible (before it is clicked
 * and expanded).  Once the icon object is set, a previously set one will be
 * deleted, if you want to keep that old content object, use the
 * elm_hoversel_icon_unset() function.
 *
 * @see elm_object_content_set() for the button widget
 * @deprecated Use elm_object_item_part_content_set() instead
 */
EINA_DEPRECATED EAPI void         elm_hoversel_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * @brief Get the icon of the hoversel button
 *
 * @param obj The hoversel object
 * @return The icon object
 *
 * Get the icon of the button that is always visible (before it is clicked
 * and expanded). Also see elm_object_content_get() for the button widget.
 *
 * @see elm_hoversel_icon_set()
 * @deprecated Use elm_object_item_part_content_get() instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_hoversel_icon_get(const Evas_Object *obj);

/**
 * @brief Get and unparent the icon of the hoversel button
 *
 * @param obj The hoversel object
 * @return The icon object that was being used
 *
 * Unparent and return the icon of the button that is always visible
 * (before it is clicked and expanded).
 *
 * @see elm_hoversel_icon_set()
 * @see elm_object_content_unset() for the button widget
 * @deprecated Use elm_object_item_part_content_unset() instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_hoversel_icon_unset(Evas_Object *obj);

/**
 * @brief This returns the data pointer supplied with elm_hoversel_item_add()
 * that will be passed to associated function callbacks.
 *
 * @param it The item to get the data from
 * @return The data pointer set with elm_hoversel_item_add()
 *
 * @see elm_hoversel_item_add()
 * @deprecated Use elm_object_item_data_get() instead
 */
EINA_DEPRECATED EAPI void        *elm_hoversel_item_data_get(const Elm_Object_Item *it);

/**
 * @brief This returns the label text of the given hoversel item.
 *
 * @param it The item to get the label
 * @return The label text of the hoversel item
 *
 * @see elm_hoversel_item_add()
 * @deprecated Use elm_object_item_text_get() instead
 */
EINA_DEPRECATED EAPI const char  *elm_hoversel_item_label_get(const Elm_Object_Item *it);

/**
 * @brief Set the function to be called when an item from the hoversel is
 * freed.
 *
 * @param it The item to set the callback on
 * @param func The function called
 *
 * That function will receive these parameters:
 * @li void * item data
 * @li Evas_Object * hoversel object
 * @li Elm_Object_Item * hoversel item
 *
 * @see elm_hoversel_item_add()
 * @deprecated Use elm_object_item_del_cb_set() instead
 */
EINA_DEPRECATED EAPI void                         elm_hoversel_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * @brief Delete an item from the hoversel
 *
 * @param it The item to delete
 *
 * This deletes the item from the hoversel (should not be called while the
 * hoversel is active; use elm_hoversel_expanded_get() to check first).
 *
 * @deprecated Use elm_object_item_del() instead
 * @see elm_hoversel_item_add()
 */
EINA_DEPRECATED EAPI void                         elm_hoversel_item_del(Elm_Object_Item *it);

/**
 * Set actionslider labels.
 *
 * @param obj The actionslider object
 * @param left_label The label to be set on the left.
 * @param center_label The label to be set on the center.
 * @param right_label The label to be set on the right.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_actionslider_labels_set(Evas_Object *obj, const char *left_label, const char *center_label, const char *right_label);

/**
 * Get actionslider labels.
 *
 * @param obj The actionslider object
 * @param left_label A char** to place the left_label of @p obj into.
 * @param center_label A char** to place the center_label of @p obj into.
 * @param right_label A char** to place the right_label of @p obj into.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_actionslider_labels_get(const Evas_Object *obj, const char **left_label, const char **center_label, const char **right_label);

/**
 * Set the label used on the indicator.
 *
 * @param obj The actionslider object
 * @param label The label to be set on the indicator.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void        elm_actionslider_indicator_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label used on the indicator object.
 *
 * @param obj The actionslider object
 * @return The indicator label
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char *elm_actionslider_indicator_label_get(Evas_Object *obj);

/**
 * Set the overlay object used for the background object.
 *
 * @param obj The bg object
 * @param overlay The overlay object
 *
 * This provides a way for elm_bg to have an 'overlay' that will be on top
 * of the bg. Once the over object is set, a previously set one will be
 * deleted, even if you set the new one to NULL. If you want to keep that
 * old content object, use the elm_bg_overlay_unset() function.
 *
 * @deprecated use elm_object_part_content_set() instead
 *
 * @ingroup Bg
 */

EINA_DEPRECATED EAPI void         elm_bg_overlay_set(Evas_Object *obj, Evas_Object *overlay);

/**
 * Get the overlay object used for the background object.
 *
 * @param obj The bg object
 * @return The content that is being used
 *
 * Return the content object which is set for this widget
 *
 * @deprecated use elm_object_part_content_get() instead
 *
 * @ingroup Bg
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bg_overlay_get(const Evas_Object *obj);

/**
 * Get the overlay object used for the background object.
 *
 * @param obj The bg object
 * @return The content that was being used
 *
 * Unparent and return the overlay object which was set for this widget
 *
 * @deprecated use elm_object_part_content_unset() instead
 *
 * @ingroup Bg
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bg_overlay_unset(Evas_Object *obj);


/**
 * Set the label of the bubble
 *
 * @param obj The bubble object
 * @param label The string to set in the label
 *
 * This function sets the title of the bubble. Where this appears depends on
 * the selected corner.
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_bubble_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label of the bubble
 *
 * @param obj The bubble object
 * @return The string of set in the label
 *
 * This function gets the title of the bubble.
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_bubble_label_get(const Evas_Object *obj);

/**
 * Set the info of the bubble
 *
 * @param obj The bubble object
 * @param info The given info about the bubble
 *
 * This function sets the info of the bubble. Where this appears depends on
 * the selected corner.
 * @deprecated use elm_object_part_text_set() instead. (with "info" as the parameter).
 */
EINA_DEPRECATED EAPI void         elm_bubble_info_set(Evas_Object *obj, const char *info);

/**
 * Get the info of the bubble
 *
 * @param obj The bubble object
 *
 * @return The "info" string of the bubble
 *
 * This function gets the info text.
 * @deprecated use elm_object_part_text_get() instead. (with "info" as the parameter).
 */
EINA_DEPRECATED EAPI const char  *elm_bubble_info_get(const Evas_Object *obj);

/**
 * Set the content to be shown in the bubble
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep the old content object, use the
 * elm_bubble_content_unset() function.
 *
 * @param obj The bubble object
 * @param content The given content of the bubble
 *
 * This function sets the content shown on the middle of the bubble.
 *
 * @deprecated use elm_object_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_bubble_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * Get the content shown in the bubble
 *
 * Return the content object which is set for this widget.
 *
 * @param obj The bubble object
 * @return The content that is being used
 *
 * @deprecated use elm_object_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_content_get(const Evas_Object *obj);

/**
 * Unset the content shown in the bubble
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @param obj The bubble object
 * @return The content that was being used
 *
 * @deprecated use elm_object_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_content_unset(Evas_Object *obj);

/**
 * Set the icon of the bubble
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep the old content object, use the
 * elm_icon_content_unset() function.
 *
 * @param obj The bubble object
 * @param icon The given icon for the bubble
 *
 * @deprecated use elm_object_part_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_bubble_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Get the icon of the bubble
 *
 * @param obj The bubble object
 * @return The icon for the bubble
 *
 * This function gets the icon shown on the top left of bubble.
 *
 * @deprecated use elm_object_part_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_icon_get(const Evas_Object *obj);

/**
 * Unset the icon of the bubble
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @param obj The bubble object
 * @return The icon that was being used
 *
 * @deprecated use elm_object_part_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_bubble_icon_unset(Evas_Object *obj);


/**
 * Set the label used in the button
 *
 * The passed @p label can be NULL to clean any existing text in it and
 * leave the button as an icon only object.
 *
 * @param obj The button object
 * @param label The text will be written on the button
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_button_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label set for the button
 *
 * The string returned is an internal pointer and should not be freed or
 * altered. It will also become invalid when the button is destroyed.
 * The string returned, if not NULL, is a stringshare, so if you need to
 * keep it around even after the button is destroyed, you can use
 * eina_stringshare_ref().
 *
 * @param obj The button object
 * @return The text set to the label, or NULL if nothing is set
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_button_label_get(const Evas_Object *obj);

/**
 * Set the icon used for the button
 *
 * Setting a new icon will delete any other that was previously set, making
 * any reference to them invalid. If you need to maintain the previous
 * object alive, unset it first with elm_button_icon_unset().
 *
 * @param obj The button object
 * @param icon The icon object for the button
 * @deprecated use elm_object_part_content_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_button_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Get the icon used for the button
 *
 * Return the icon object which is set for this widget. If the button is
 * destroyed or another icon is set, the returned object will be deleted
 * and any reference to it will be invalid.
 *
 * @param obj The button object
 * @return The icon object that is being used
 *
 * @deprecated use elm_object_part_content_get() instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_button_icon_get(const Evas_Object *obj);

/**
 * Remove the icon set without deleting it and return the object
 *
 * This function drops the reference the button holds of the icon object
 * and returns this last object. It is used in case you want to remove any
 * icon, or set another one, without deleting the actual object. The button
 * will be left without an icon set.
 *
 * @param obj The button object
 * @return The icon object that was being used
 * @deprecated use elm_object_part_content_unset() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_button_icon_unset(Evas_Object *obj);

/**
 * Set a day text color to the same that represents Saturdays.
 *
 * @param obj The calendar object.
 * @param pos The text position. Position is the cell counter, from left
 * to right, up to down. It starts on 0 and ends on 41.
 *
 * @deprecated use elm_calendar_mark_add() instead like:
 *
 * @code
 * struct tm t = { 0, 0, 12, 6, 0, 0, 6, 6, -1 };
 * elm_calendar_mark_add(obj, "sat", &t, ELM_CALENDAR_WEEKLY);
 * @endcode
 *
 * @see elm_calendar_mark_add()
 *
 * @ingroup Calendar
 */
EINA_DEPRECATED EAPI void elm_calendar_text_saturday_color_set(Evas_Object *obj, int pos);

/**
 * Set a day text color to the same that represents Sundays.
 *
 * @param obj The calendar object.
 * @param pos The text position. Position is the cell counter, from left
 * to right, up to down. It starts on 0 and ends on 41.

 * @deprecated use elm_calendar_mark_add() instead like:
 *
 * @code
 * struct tm t = { 0, 0, 12, 7, 0, 0, 0, 0, -1 };
 * elm_calendar_mark_add(obj, "sat", &t, ELM_CALENDAR_WEEKLY);
 * @endcode
 *
 * @see elm_calendar_mark_add()
 *
 * @ingroup Calendar
 */
EINA_DEPRECATED EAPI void elm_calendar_text_sunday_color_set(Evas_Object *obj, int pos);

/**
 * Set a day text color to the same that represents Weekdays.
 *
 * @param obj The calendar object
 * @param pos The text position. Position is the cell counter, from left
 * to right, up to down. It starts on 0 and ends on 41.
 *
 * @deprecated use elm_calendar_mark_add() instead like:
 *
 * @code
 * struct tm t = { 0, 0, 12, 1, 0, 0, 0, 0, -1 };
 *
 * elm_calendar_mark_add(obj, "week", &t, ELM_CALENDAR_WEEKLY); // monday
 * t.tm_tm_mday++; t.tm_wday++; t.tm_yday++;
 * elm_calendar_mark_add(obj, "week", &t, ELM_CALENDAR_WEEKLY); // tuesday
 * t.tm_tm_mday++; t.tm_wday++; t.tm_yday++;
 * elm_calendar_mark_add(obj, "week", &t, ELM_CALENDAR_WEEKLY); // wednesday
 * t.tm_tm_mday++; t.tm_wday++; t.tm_yday++;
 * elm_calendar_mark_add(obj, "week", &t, ELM_CALENDAR_WEEKLY); // thursday
 * t.tm_tm_mday++; t.tm_wday++; t.tm_yday++;
 * elm_calendar_mark_add(obj, "week", &t, ELM_CALENDAR_WEEKLY); // friday
 * @endcode
 *
 * @see elm_calendar_mark_add()
 *
 * @ingroup Calendar
 */
EINA_DEPRECATED EAPI void elm_calendar_text_weekday_color_set(Evas_Object *obj, int pos);


/**
 * @brief Set the text label of the check object
 *
 * @param obj The check object
 * @param label The text label string in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_check_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the text label of the check object
 *
 * @param obj The check object
 * @return The text label string in UTF-8
 *
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_check_label_get(const Evas_Object *obj);

/**
 * @brief Set the icon object of the check object
 *
 * @param obj The check object
 * @param icon The icon object
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_object_content_unset() function.
 *
 * @deprecated use elm_object_part_content_set() instead.
 *
 */
EINA_DEPRECATED EAPI void         elm_check_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * @brief Get the icon object of the check object
 *
 * @param obj The check object
 * @return The icon object
 *
 * @deprecated use elm_object_part_content_get() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_check_icon_get(const Evas_Object *obj);

/**
 * @brief Unset the icon used for the check object
 *
 * @param obj The check object
 * @return The icon object that was being used
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @deprecated use elm_object_part_content_unset() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_check_icon_unset(Evas_Object *obj);

/**
 * @brief Set the text label of the check object
 *
 * @param obj The check object
 * @param label The text label string in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_check_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the text label of the check object
 *
 * @param obj The check object
 * @return The text label string in UTF-8
 *
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_check_label_get(const Evas_Object *obj);

/**
 * @brief Set the icon object of the check object
 *
 * @param obj The check object
 * @param icon The icon object
 *
 * Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_object_content_unset() function.
 *
 * @deprecated use elm_object_part_content_set() instead.
 *
 */
EINA_DEPRECATED EAPI void         elm_check_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * @brief Get the icon object of the check object
 *
 * @param obj The check object
 * @return The icon object
 *
 * @deprecated use elm_object_part_content_get() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_check_icon_get(const Evas_Object *obj);

/**
 * @brief Unset the icon used for the check object
 *
 * @param obj The check object
 * @return The icon object that was being used
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @deprecated use elm_object_part_content_unset() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_check_icon_unset(Evas_Object *obj);

EINA_DEPRECATED EAPI void         elm_check_states_labels_set(Evas_Object *obj, const char *ontext, const char *offtext);
EINA_DEPRECATED EAPI void         elm_check_states_labels_get(const Evas_Object *obj, const char **ontext, const char **offtext);


/**
 * Set the content of the conformant widget.
 *
 * @param obj The conformant object.
 * @param content The content to be displayed by the conformant.
 *
 * Content will be sized and positioned considering the space required
 * to display a virtual keyboard. So it won't fill all the conformant
 * size. This way is possible to be sure that content won't resize
 * or be re-positioned after the keyboard is displayed.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_object_content_unset() function.
 *
 * @see elm_object_content_unset()
 * @see elm_object_content_get()
 *
 * @deprecated use elm_object_content_set() instead
 *
 * @ingroup Conformant
 */
EINA_DEPRECATED EAPI void         elm_conformant_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * Get the content of the conformant widget.
 *
 * @param obj The conformant object.
 * @return The content that is being used.
 *
 * Return the content object which is set for this widget.
 * It won't be unparent from conformant. For that, use
 * elm_object_content_unset().
 *
 * @see elm_object_content_set().
 * @see elm_object_content_unset()
 *
 * @deprecated use elm_object_content_get() instead
 *
 * @ingroup Conformant
 */
EINA_DEPRECATED EAPI Evas_Object *elm_conformant_content_get(const Evas_Object *obj);

/**
 * Unset the content of the conformant widget.
 *
 * @param obj The conformant object.
 * @return The content that was being used.
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @see elm_object_content_set().
 *
 * @deprecated use elm_object_content_unset() instead
 *
 * @ingroup Conformant
 */
EINA_DEPRECATED EAPI Evas_Object *elm_conformant_content_unset(Evas_Object *obj);

/**
 * Get the side labels max length.
 *
 * @deprecated use elm_diskselector_side_label_length_get() instead:
 *
 * @param obj The diskselector object.
 * @return The max length defined for side labels, or 0 if not a valid
 * diskselector.
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI int    elm_diskselector_side_label_lenght_get(const Evas_Object *obj);

/**
 * Set the side labels max length.
 *
 * @deprecated use elm_diskselector_side_label_length_set() instead:
 *
 * @param obj The diskselector object.
 * @param len The max length defined for side labels.
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void   elm_diskselector_side_label_lenght_set(Evas_Object *obj, int len);

/**
 * Get the data associated to the item.
 *
 * @param it The diskselector item
 * @return The data associated to @p it
 *
 * The return value is a pointer to data associated to @p item when it was
 * created, with function elm_diskselector_item_append(). If no data
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_diskselector_item_append()
 * @deprecated Use elm_object_item_data_get()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                  *elm_diskselector_item_data_get(const Elm_Object_Item *it);

/**
 * Set the icon associated to the item.
 *
 * @param it The diskselector item
 * @param icon The icon object to associate with @p it
 *
 * The icon object to use at left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 *
 * Once the icon object is set, a previously set one will be deleted.
 * @warning Setting the same icon for two items will cause the icon to
 * dissapear from the first item.
 *
 * If an icon was passed as argument on item creation, with function
 * elm_diskselector_item_append(), it will be already
 * associated to the item.
 *
 * @see elm_diskselector_item_append()
 * @see elm_diskselector_item_icon_get()
 *
 * @deprecated Use elm_object_item_part_content_set() instead
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_icon_set(Elm_Object_Item *it, Evas_Object *icon);

/**
 * Get the icon associated to the item.
 *
 * @param item The diskselector item
 * @return The icon associated to @p it
 *
 * The return value is a pointer to the icon associated to @p item when it was
 * created, with function elm_diskselector_item_append(), or later
 * with function elm_diskselector_item_icon_set. If no icon
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_diskselector_item_append()
 * @see elm_diskselector_item_icon_set()
 *
 * @deprecated Use elm_object_item_part_content_set() instead
 * @ingroup Diskselector
 */
EAPI Evas_Object           *elm_diskselector_item_icon_get(const Elm_Object_Item *it);

/**
 * Set the label of item.
 *
 * @param it The item of diskselector.
 * @param label The label of item.
 *
 * The label to be displayed by the item.
 *
 * If no icon is set, label will be centered on item position, otherwise
 * the icon will be placed at left of the label, that will be shifted
 * to the right.
 *
 * An item with label "January" would be displayed on side position as
 * "Jan" if max length is set to 3 with function
 * elm_diskselector_side_label_lenght_set(), or "Janu", if this property
 * is set to 4.
 *
 * When this @p item is selected, the entire label will be displayed,
 * except for width restrictions.
 * In this case label will be cropped and "..." will be concatenated,
 * but only for display purposes. It will keep the entire string, so
 * if diskselector is resized the remaining characters will be displayed.
 *
 * If a label was passed as argument on item creation, with function
 * elm_diskselector_item_append(), it will be already
 * displayed by the item.
 *
 * @see elm_diskselector_side_label_lenght_set()
 * @see elm_diskselector_item_label_get()
 * @see elm_diskselector_item_append()
 *
 * @deprecated Use elm_object_item_text_set() instead
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_label_set(Elm_Object_Item *it, const char *label);

/**
 * Get the label of item.
 *
 * @param it The item of diskselector.
 * @return The label of item.
 *
 * The return value is a pointer to the label associated to @p item when it was
 * created, with function elm_diskselector_item_append(), or later
 * with function elm_diskselector_item_label_set. If no label
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_diskselector_item_label_set() for more details.
 * @see elm_diskselector_item_append()
 * @use elm_object_item_text_get()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI const char            *elm_diskselector_item_label_get(const Elm_Object_Item *it);

/**
 * Set the function called when a diskselector item is freed.
 *
 * @param it The item to set the callback on
 * @param func The function called
 *
 * If there is a @p func, then it will be called prior item's memory release.
 * That will be called with the following arguments:
 * @li item's data;
 * @li item's Evas object;
 * @li item itself;
 *
 * This way, a data associated to a diskselector item could be properly
 * freed.
 * @deprecated Use elm_object_item_del_cb_set() instead
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * Delete them item from the diskselector.
 *
 * @param it The item of diskselector to be deleted.
 *
 * If deleting all diskselector items is required, elm_diskselector_clear()
 * should be used instead of getting items list and deleting each one.
 *
 * @deprecated Use elm_object_item_del() instead
 * @see elm_diskselector_clear()
 * @see elm_diskselector_item_append()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_del(Elm_Object_Item *it);

/**
 * Set the text to be shown in the diskselector item.
 *
 * @param it Target item
 * @param text The text to set in the content
 *
 * Setup the text as tooltip to object. The item can have only one tooltip,
 * so any previous tooltip data is removed.
 *
 * @deprecated Use elm_object_item_tooltip_text_set() instead
 * @see elm_object_tooltip_text_set() for more details.
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_tooltip_text_set(Elm_Object_Item *it, const char *text);

/**
 * Set the content to be shown in the tooltip item.
 *
 * Setup the tooltip to item. The item can have only one tooltip,
 * so any previous tooltip data is removed. @p func(with @p data) will
 * be called every time that need show the tooltip and it should
 * return a valid Evas_Object. This object is then managed fully by
 * tooltip system and is deleted when the tooltip is gone.
 *
 * @param it the diskselector item being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @p func, the tooltip is unset with
 *        elm_diskselector_item_tooltip_unset() or the owner @a item
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is the item.
 *
 * @deprecated Use elm_object_item_tooltip_content_cb_set() instead
 * @see elm_object_tooltip_content_cb_set() for more details.
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

/**
 * Unset tooltip from item.
 *
 * @param it diskselector item to remove previously set tooltip.
 *
 * Remove tooltip from item. The callback provided as del_cb to
 * elm_diskselector_item_tooltip_content_cb_set() will be called to notify
 * it is not used anymore.
 *
 * @deprecated Use elm_object_item_tooltip_unset() instead
 * @see elm_object_tooltip_unset() for more details.
 * @see elm_diskselector_item_tooltip_content_cb_set()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_tooltip_unset(Elm_Object_Item *it);

/**
 * Sets a different style for this item tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_diskselector_item_tooltip_content_cb_set() or
 *       elm_diskselector_item_tooltip_text_set()
 *
 * @param it diskselector item with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @deprecated Use elm_object_item_tooltip_style_set() instead
 * @see elm_object_tooltip_style_set() for more details.
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI void                   elm_diskselector_item_tooltip_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the style for this item tooltip.
 *
 * @param it diskselector item with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @deprecated Use elm_object_item_tooltip_style_get() instead
 * @see elm_object_tooltip_style_get() for more details.
 * @see elm_diskselector_item_tooltip_style_set()
 *
 * @ingroup Diskselector
 */
EINA_DEPRECATED EAPI const char            *elm_diskselector_item_tooltip_style_get(const Elm_Object_Item *it);

EINA_DEPRECATED EAPI void         elm_factory_content_set(Evas_Object *obj, Evas_Object *content);
EINA_DEPRECATED EAPI Evas_Object *elm_factory_content_get(const Evas_Object *obj);

/**
 * Get the label of a given flip selector widget's item.
 *
 * @param it The item to get label from
 * @return The text label of @p item or @c NULL, on errors
 *
 * @see elm_object_item_text_set()
 *
 * @deprecated see elm_object_item_text_get() instead
 * @ingroup Flipselector
 */
EINA_DEPRECATED EAPI const char *elm_flipselector_item_label_get(const Elm_Object_Item *it);

/**
 * Set the label of a given flip selector widget's item.
 *
 * @param it The item to set label on
 * @param label The text label string, in UTF-8 encoding
 *
 * @see elm_object_item_text_get()
 *
 * @deprecated see elm_object_item_text_set() instead
 * @ingroup Flipselector
 */
EINA_DEPRECATED EAPI void        elm_flipselector_item_label_set(Elm_Object_Item *it, const char *label);

/**
 * Delete a given item from a flip selector widget.
 *
 * @param it The item to delete
 *
 * @deprecated Use elm_object_item_del() instead
 * @ingroup Flipselector
 */
EINA_DEPRECATED EAPI void       elm_flipselector_item_del(Elm_Object_Item *it);

/**
 * Make a given Elementary object the focused one.
 *
 * @param obj The Elementary object to make focused.
 *
 * @note This object, if it can handle focus, will take the focus
 * away from the one who had it previously and will, for now on, be
 * the one receiving input events.
 *
 * @see elm_object_focus_get()
 * @deprecated use elm_object_focus_set() instead.
 *
 * @ingroup Focus
 */
EINA_DEPRECATED EAPI void elm_object_focus(Evas_Object *obj);

/**
 * Remove the focus from an Elementary object
 *
 * @param obj The Elementary to take focus from
 *
 * This removes the focus from @p obj, passing it back to the
 * previous element in the focus chain list.
 *
 * @see elm_object_focus() and elm_object_focus_custom_chain_get()
 * @deprecated use elm_object_focus_set() instead.
 *
 * @ingroup Focus
 */
EINA_DEPRECATED EAPI void elm_object_unfocus(Evas_Object *obj);

/**
 * @brief Set the frame label
 *
 * @param obj The frame object
 * @param label The label of this frame object
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_frame_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the frame label
 *
 * @param obj The frame object
 *
 * @return The label of this frame objet or NULL if unable to get frame
 *
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_frame_label_get(const Evas_Object *obj);

/**
 * @brief Set the content of the frame widget
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_frame_content_unset() function.
 *
 * @param obj The frame object
 * @param content The content will be filled in this frame object
 *
 * @deprecated use elm_object_content_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_frame_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Get the content of the frame widget
 *
 * Return the content object which is set for this widget
 *
 * @param obj The frame object
 * @return The content that is being used
 *
 * @deprecated use elm_object_content_get() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_frame_content_get(const Evas_Object *obj);

/**
 * @brief Unset the content of the frame widget
 *
 * Unparent and return the content object which was set for this widget
 *
 * @param obj The frame object
 * @return The content that was being used
 *
 * @deprecated use elm_object_content_unset() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_frame_content_unset(Evas_Object *obj);

EINA_DEPRECATED EAPI void          elm_genlist_horizontal_mode_set(Evas_Object *obj, Elm_List_Mode mode);
EINA_DEPRECATED EAPI Elm_List_Mode elm_genlist_horizontal_mode_get(const Evas_Object *obj);
EINA_DEPRECATED EAPI void          elm_genlist_item_icons_orphan(Elm_Object_Item *it);

#define ELM_IMAGE_ROTATE_90_CW 1
#define ELM_IMAGE_ROTATE_180_CW 2
#define ELM_IMAGE_ROTATE_90_CCW 3

/**
 * Return the data associated with a given index widget item
 *
 * @param it The index widget item handle
 * @return The data associated with @p it
 * @deprecated Use elm_object_item_data_get() instead
 *
 * @see elm_index_item_data_set()
 *
 * @ingroup Index
 */
EINA_DEPRECATED EAPI void *elm_index_item_data_get(const Elm_Object_Item *item);

/**
 * Set the data associated with a given index widget item
 *
 * @param it The index widget item handle
 * @param data The new data pointer to set to @p it
 *
 * This sets new item data on @p it.
 *
 * @warning The old data pointer won't be touched by this function, so
 * the user had better to free that old data himself/herself.
 *
 * @deprecated Use elm_object_item_data_set() instead
 * @ingroup Index
 */
EINA_DEPRECATED EAPI void  elm_index_item_data_set(Elm_Object_Item *it, const void *data);

/**
 * Set the function to be called when a given index widget item is freed.
 *
 * @param it The item to set the callback on
 * @param func The function to call on the item's deletion
 *
 * When called, @p func will have both @c data and @c event_info
 * arguments with the @p it item's data value and, naturally, the
 * @c obj argument with a handle to the parent index widget.
 *
 * @deprecated Use elm_object_item_del_cb_set() instead
 * @ingroup Index
 */
EINA_DEPRECATED EAPI void                  elm_index_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * Remove an item from a given index widget, <b>to be referenced by
 * it's data value</b>.
 *
 * @param obj The index object
 * @param item The item to be removed from @p obj
 *
 * If a deletion callback is set, via elm_object_item_del_cb_set(),
 * that callback function will be called by this one.
 *
 * @deprecated Use elm_object_item_del() instead
 * @ingroup Index
 */
EINA_DEPRECATED EAPI void                  elm_index_item_del(Evas_Object *obj, Elm_Object_Item *item);

/**
* @brief Set the label on the label object
 *
 * @param obj The label object
 * @param label The label will be used on the label object
 * @deprecated See elm_object_text_set()
 */
EINA_DEPRECATED EAPI void        elm_label_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the label used on the label object
 *
 * @param obj The label object
 * @return The string inside the label
 * @deprecated See elm_object_text_get()
 */
EINA_DEPRECATED EAPI const char *elm_label_label_get(const Evas_Object *obj);


/**
 * Set the layout content.
 *
 * @param obj The layout object
 * @param swallow The swallow part name in the edje file
 * @param content The child that will be added in this layout object
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_object_part_content_unset() function.
 *
 * @note In an Edje theme, the part used as a content container is called @c
 * SWALLOW. This is why the parameter name is called @p swallow, but it is
 * expected to be a part name just like the second parameter of
 * elm_layout_box_append().
 *
 * @see elm_layout_box_append()
 * @see elm_object_part_content_get()
 * @see elm_object_part_content_unset()
 * @see @ref secBox
 * @deprecated use elm_object_part_content_set() instead
 *
 * @ingroup Layout
 */
EINA_DEPRECATED EAPI void         elm_layout_content_set(Evas_Object *obj, const char *swallow, Evas_Object *content);

/**
 * Get the child object in the given content part.
 *
 * @param obj The layout object
 * @param swallow The SWALLOW part to get its content
 *
 * @return The swallowed object or NULL if none or an error occurred
 *
 * @deprecated use elm_object_part_content_get() instead
 *
 * @ingroup Layout
 */
EINA_DEPRECATED EAPI Evas_Object *elm_layout_content_get(const Evas_Object *obj, const char *swallow);

/**
 * Unset the layout content.
 *
 * @param obj The layout object
 * @param swallow The swallow part name in the edje file
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this part.
 *
 * @deprecated use elm_object_part_content_unset() instead
 *
 * @ingroup Layout
 */
EINA_DEPRECATED EAPI Evas_Object *elm_layout_content_unset(Evas_Object *obj, const char *swallow);

/**
 * Set the text of the given part
 *
 * @param obj The layout object
 * @param part The TEXT part where to set the text
 * @param text The text to set
 *
 * @ingroup Layout
 * @deprecated use elm_object_part_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_layout_text_set(Evas_Object *obj, const char *part, const char *text);

/**
 * Get the text set in the given part
 *
 * @param obj The layout object
 * @param part The TEXT part to retrieve the text off
 *
 * @return The text set in @p part
 *
 * @ingroup Layout
 * @deprecated use elm_object_part_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_layout_text_get(const Evas_Object *obj, const char *part);

/**
 * @def elm_layout_label_set
 * Convienience macro to set the label in a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @ingroup Layout
 * @deprecated use elm_object_text_set() instead.
 */
#define elm_layout_label_set(_ly, _txt) \
  elm_layout_text_set((_ly), "elm.text", (_txt))

/**
 * @def elm_layout_label_get
 * Convenience macro to get the label in a layout that follows the
 * Elementary naming convention for its parts.
 *
 * @ingroup Layout
 * @deprecated use elm_object_text_set() instead.
 */
#define elm_layout_label_get(_ly) \
  elm_layout_text_get((_ly), "elm.text")

EINA_DEPRECATED EAPI Evas_Object *elm_list_item_base_get(const Elm_Object_Item *it);


/**
 * Set the content of the mapbuf.
 *
 * @param obj The mapbuf object.
 * @param content The content that will be filled in this mapbuf object.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_mapbuf_content_unset() function.
 *
 * To enable map, elm_mapbuf_enabled_set() should be used.
 *
 * @deprecated use elm_object_content_set() instead
 *
 * @ingroup Mapbuf
 */
EINA_DEPRECATED EAPI void         elm_mapbuf_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * Get the content of the mapbuf.
 *
 * @param obj The mapbuf object.
 * @return The content that is being used.
 *
 * Return the content object which is set for this widget.
 *
 * @see elm_mapbuf_content_set() for details.
 *
 * @deprecated use elm_object_content_get() instead
 *
 * @ingroup Mapbuf
 */
EINA_DEPRECATED EAPI Evas_Object *elm_mapbuf_content_get(const Evas_Object *obj);

/**
 * Unset the content of the mapbuf.
 *
 * @param obj The mapbuf object.
 * @return The content that was being used.
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @see elm_mapbuf_content_set() for details.
 *
 * @deprecated use elm_object_content_unset() instead
 *
 * @ingroup Mapbuf
 */
EINA_DEPRECATED EAPI Evas_Object *elm_mapbuf_content_unset(Evas_Object *obj);

/**
 * @brief Set the label of a menu item
 *
 * @param it The menu item object.
 * @param label The label to set for @p item
 *
 * @warning Don't use this funcion on items created with
 * elm_menu_item_add_object() or elm_menu_item_separator_add().
 *
 * @deprecated Use elm_object_item_text_set() instead
 */
EINA_DEPRECATED EAPI void         elm_menu_item_label_set(Elm_Object_Item *it, const char *label);

/**
 * @brief Get the label of a menu item
 *
 * @param it The menu item object.
 * @return The label of @p item
 * @deprecated Use elm_object_item_text_get() instead
 */
EINA_DEPRECATED EAPI const char  *elm_menu_item_label_get(const Elm_Object_Item *it);

/**
 * @brief Set the content object of a menu item
 *
 * @param it The menu item object
 * @param The content object or NULL
 * @return EINA_TRUE on success, else EINA_FALSE
 *
 * Use this function to change the object swallowed by a menu item, deleting
 * any previously swallowed object.
 *
 * @deprecated Use elm_object_item_content_set() instead
 */
EINA_DEPRECATED EAPI Eina_Bool    elm_menu_item_object_content_set(Elm_Object_Item *it, Evas_Object *obj);

/**
 * @brief Get the content object of a menu item
 *
 * @param it The menu item object
 * @return The content object or NULL
 * @note If @p item was added with elm_menu_item_add_object, this
 * function will return the object passed, else it will return the
 * icon object.
 *
 * @see elm_menu_item_object_content_set()
 *
 * @deprecated Use elm_object_item_content_get() instead
 */
EINA_DEPRECATED EAPI Evas_Object *elm_menu_item_object_content_get(const Elm_Object_Item *it);

/**
 * @brief Set the disabled state of @p item.
 *
 * @param it The menu item object.
 * @param disabled The enabled/disabled state of the item
 * @deprecated Use elm_object_item_disabled_set() instead
 */
EINA_DEPRECATED EAPI void         elm_menu_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled);

/**
 * @brief Get the disabled state of @p item.
 *
 * @param it The menu item object.
 * @return The enabled/disabled state of the item
 *
 * @see elm_menu_item_disabled_set()
 * @deprecated Use elm_object_item_disabled_get() instead
 */
EINA_DEPRECATED EAPI Eina_Bool    elm_menu_item_disabled_get(const Elm_Object_Item *it);

/**
 * @brief Returns the data associated with menu item @p item.
 *
 * @param it The item
 * @return The data associated with @p item or NULL if none was set.
 *
 * This is the data set with elm_menu_add() or elm_menu_item_data_set().
 *
 * @deprecated Use elm_object_item_data_get() instead
 */
EINA_DEPRECATED EAPI void        *elm_menu_item_data_get(const Elm_Object_Item *it);

/**
 * @brief Set the function called when a menu item is deleted.
 *
 * @param it The item to set the callback on
 * @param func The function called
 *
 * @see elm_menu_item_add()
 * @see elm_menu_item_del()
 * @deprecated Use elm_object_item_del_cb_set() instead
 */
EINA_DEPRECATED EAPI void                         elm_menu_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * @brief Sets the data to be associated with menu item @p item.
 *
 * @param it The item
 * @param data The data to be associated with @p item
 *
 * @deprecated Use elm_object_item_data_set() instead
 */
EINA_DEPRECATED EAPI void         elm_menu_item_data_set(Elm_Object_Item *it, const void *data);

/**
 * @brief Deletes an item from the menu.
 *
 * @param it The item to delete.
 *
 * @deprecated Use elm_object_item_del() instead
 * @see elm_menu_item_add()
 */
EINA_DEPRECATED EAPI void                         elm_menu_item_del(Elm_Object_Item *it);

/**
 * @brief Set the content of the notify widget
 *
 * @param obj The notify object
 * @param content The content will be filled in this notify object
 *
 * Once the content object is set, a previously set one will be deleted. If
 * you want to keep that old content object, use the
 * elm_notify_content_unset() function.
 *
 * @deprecated use elm_object_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_notify_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Unset the content of the notify widget
 *
 * @param obj The notify object
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this widget
 *
 * @see elm_notify_content_set()
 * @deprecated use elm_object_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_notify_content_unset(Evas_Object *obj);

/**
 * @brief Return the content of the notify widget
 *
 * @param obj The notify object
 * @return The content that is being used
 *
 * @see elm_notify_content_set()
 * @deprecated use elm_object_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_notify_content_get(const Evas_Object *obj);

/**
 * Set a label of an object
 *
 * @param obj The Elementary object
 * @param part The text part name to set (NULL for the default label)
 * @param label The new text of the label
 *
 * @note Elementary objects may have many labels (e.g. Action Slider)
 * @deprecated Use elm_object_part_text_set() instead.
 * @ingroup General
 */
EINA_DEPRECATED EAPI void         elm_object_text_part_set(Evas_Object *obj, const char *part, const char *label);

/**
 * Get a label of an object
 *
 * @param obj The Elementary object
 * @param part The text part name to get (NULL for the default label)
 * @return text of the label or NULL for any error
 *
 * @note Elementary objects may have many labels (e.g. Action Slider)
 * @deprecated Use elm_object_part_text_get() instead.
 * @ingroup General
 */
EINA_DEPRECATED EAPI const char  *elm_object_text_part_get(const Evas_Object *obj, const char *part);

/**
 * Set a content of an object
 *
 * @param obj The Elementary object
 * @param part The content part name to set (NULL for the default content)
 * @param content The new content of the object
 *
 * @note Elementary objects may have many contents
 * @deprecated Use elm_object_part_content_set instead.
 * @ingroup General
 */
EINA_DEPRECATED EAPI void         elm_object_content_part_set(Evas_Object *obj, const char *part, Evas_Object *content);

/**
 * Get a content of an object
 *
 * @param obj The Elementary object
 * @param part The content part name to get (NULL for the default content)
 * @return content of the object or NULL for any error
 *
 * @note Elementary objects may have many contents
 * @deprecated Use elm_object_part_content_get instead.
 * @ingroup General
 */
EINA_DEPRECATED EAPI Evas_Object *elm_object_content_part_get(const Evas_Object *obj, const char *part);

/**
 * Unset a content of an object
 *
 * @param obj The Elementary object
 * @param part The content part name to unset (NULL for the default content)
 *
 * @note Elementary objects may have many contents
 * @deprecated Use elm_object_part_content_unset instead.
 * @ingroup General
 */
EINA_DEPRECATED EAPI Evas_Object *elm_object_content_part_unset(Evas_Object *obj, const char *part);

/**
 * Set a content of an object item
 *
 * @param it The Elementary object item
 * @param part The content part name to set (NULL for the default content)
 * @param content The new content of the object item
 *
 * @note Elementary object items may have many contents
 * @deprecated Use elm_object_item_part_content_set instead.
 * @ingroup General
 */
EINA_DEPRECATED EAPI void         elm_object_item_content_part_set(Elm_Object_Item *it, const char *part, Evas_Object *content);

/**
 * Get a content of an object item
 *
 * @param it The Elementary object item
 * @param part The content part name to unset (NULL for the default content)
 * @return content of the object item or NULL for any error
 *
 * @note Elementary object items may have many contents
 * @deprecated Use elm_object_item_part_content_get instead.
 * @ingroup General
 */
EINA_DEPRECATED EAPI Evas_Object                 *elm_object_item_content_part_get(const Elm_Object_Item *it, const char *part);

/**
 * Unset a content of an object item
 *
 * @param it The Elementary object item
 * @param part The content part name to unset (NULL for the default content)
 *
 * @note Elementary object items may have many contents
 * @deprecated Use elm_object_item_part_content_unset instead.
 * @ingroup General
 */
EINA_DEPRECATED EAPI Evas_Object *elm_object_item_content_part_unset(Elm_Object_Item *it, const char *part);

/**
 * Get a label of an object item
 *
 * @param it The Elementary object item
 * @param part The text part name to get (NULL for the default label)
 * @return text of the label or NULL for any error
 *
 * @note Elementary object items may have many labels
 * @deprecated Use elm_object_item_part_text_get instead.
 * @ingroup General
 */
EINA_DEPRECATED EAPI const char  *elm_object_item_text_part_get(const Elm_Object_Item *it, const char *part);

/**
 * Set a label of an object item
 *
 * @param it The Elementary object item
 * @param part The text part name to set (NULL for the default label)
 * @param label The new text of the label
 *
 * @note Elementary object items may have many labels
 * @deprecated Use elm_object_item_part_text_set instead.
 * @ingroup General
 */
EINA_DEPRECATED EAPI void         elm_object_item_text_part_set(Elm_Object_Item *it, const char *part, const char *label);

/**
 * @brief Set the content of the panel.
 *
 * @param obj The panel object
 * @param content The panel content
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_panel_content_unset() function.
 *
 * @deprecated use elm_object_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_panel_content_set(Evas_Object *obj, Evas_Object *content);

/**
 * @brief Get the content of the panel.
 *
 * @param obj The panel object
 * @return The content that is being used
 *
 * Return the content object which is set for this widget.
 *
 * @see elm_panel_content_set()
 *
 * @deprecated use elm_object_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panel_content_get(const Evas_Object *obj);

/**
 * @brief Unset the content of the panel.
 *
 * @param obj The panel object
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @see elm_panel_content_set()
 *
 * @deprecated use elm_object_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panel_content_unset(Evas_Object *obj);

/**
 * Set the left content of the panes widget.
 *
 * @param obj The panes object.
 * @param content The new left content object.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_panes_content_left_unset() function.
 *
 * If panes is displayed vertically, left content will be displayed at
 * top.
 *
 * @see elm_panes_content_left_get()
 * @see elm_panes_content_right_set() to set content on the other side.
 *
 * @deprecated use elm_object_part_content_set() instead
 *
 * @ingroup Panes
 */
EINA_DEPRECATED EAPI void         elm_panes_content_left_set(Evas_Object *obj, Evas_Object *content);

/**
 * Set the right content of the panes widget.
 *
 * @param obj The panes object.
 * @param content The new right content object.
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_panes_content_right_unset() function.
 *
 * If panes is displayed vertically, left content will be displayed at
 * bottom.
 *
 * @see elm_panes_content_right_get()
 * @see elm_panes_content_left_set() to set content on the other side.
 *
 * @deprecated use elm_object_part_content_set() instead
 *
 * @ingroup Panes
 */
EINA_DEPRECATED EAPI void         elm_panes_content_right_set(Evas_Object *obj, Evas_Object *content);

/**
 * Get the left content of the panes.
 *
 * @param obj The panes object.
 * @return The left content object that is being used.
 *
 * Return the left content object which is set for this widget.
 *
 * @see elm_panes_content_left_set() for details.
 *
 * @deprecated use elm_object_part_content_get() instead
 *
 * @ingroup Panes
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panes_content_left_get(const Evas_Object *obj);

/**
 * Get the right content of the panes.
 *
 * @param obj The panes object
 * @return The right content object that is being used
 *
 * Return the right content object which is set for this widget.
 *
 * @see elm_panes_content_right_set() for details.
 *
 * @deprecated use elm_object_part_content_get() instead
 *
 * @ingroup Panes
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panes_content_right_get(const Evas_Object *obj);

/**
 * Unset the left content used for the panes.
 *
 * @param obj The panes object.
 * @return The left content object that was being used.
 *
 * Unparent and return the left content object which was set for this widget.
 *
 * @see elm_panes_content_left_set() for details.
 * @see elm_panes_content_left_get().
 *
 * @deprecated use elm_object_part_content_unset() instead
 *
 * @ingroup Panes
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panes_content_left_unset(Evas_Object *obj);

/**
 * Unset the right content used for the panes.
 *
 * @param obj The panes object.
 * @return The right content object that was being used.
 *
 * Unparent and return the right content object which was set for this
 * widget.
 *
 * @see elm_panes_content_right_set() for details.
 * @see elm_panes_content_right_get().
 *
 * @deprecated use elm_object_part_content_unset() instead
 *
 * @ingroup Panes
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panes_content_right_unset(Evas_Object *obj);
/**
 * Set the label of a given progress bar widget
 *
 * @param obj The progress bar object
 * @param label The text label string, in UTF-8
 *
 * @ingroup Progressbar
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_progressbar_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label of a given progress bar widget
 *
 * @param obj The progressbar object
 * @return The text label string, in UTF-8
 *
 * @ingroup Progressbar
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_progressbar_label_get(const Evas_Object *obj);

/**
 * Set the icon object of a given progress bar widget
 *
 * @param obj The progress bar object
 * @param icon The icon object
 *
 * Use this call to decorate @p obj with an icon next to it.
 *
 * @note Once the icon object is set, a previously set one will be
 * deleted. If you want to keep that old content object, use the
 * elm_progressbar_icon_unset() function.
 *
 * @see elm_progressbar_icon_get()
 * @deprecated use elm_object_part_content_set() instead.
 *
 * @ingroup Progressbar
 */
EINA_DEPRECATED EAPI void         elm_progressbar_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Retrieve the icon object set for a given progress bar widget
 *
 * @param obj The progress bar object
 * @return The icon object's handle, if @p obj had one set, or @c NULL,
 * otherwise (and on errors)
 *
 * @see elm_progressbar_icon_set() for more details
 * @deprecated use elm_object_part_content_get() instead.
 *
 * @ingroup Progressbar
 */
EINA_DEPRECATED EAPI Evas_Object *elm_progressbar_icon_get(const Evas_Object *obj);

/**
 * Unset an icon set on a given progress bar widget
 *
 * @param obj The progress bar object
 * @return The icon object that was being used, if any was set, or
 * @c NULL, otherwise (and on errors)
 *
 * This call will unparent and return the icon object which was set
 * for this widget, previously, on success.
 *
 * @see elm_progressbar_icon_set() for more details
 * @deprecated use elm_object_part_content_unset() instead.
 *
 * @ingroup Progressbar
 */
EINA_DEPRECATED EAPI Evas_Object *elm_progressbar_icon_unset(Evas_Object *obj);

/**
 * @brief Set the text label of the radio object
 *
 * @param obj The radio object
 * @param label The text label string in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_radio_label_set(Evas_Object *obj, const char *label);

/**
 * @brief Get the text label of the radio object
 *
 * @param obj The radio object
 * @return The text label string in UTF-8
 *
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_radio_label_get(const Evas_Object *obj);

/**
 * @brief Set the icon object of the radio object
 *
 * @param obj The radio object
 * @param icon The icon object
 *
 * Once the icon object is set, a previously set one will be deleted. If you
 * want to keep that old content object, use the elm_radio_icon_unset()
 * function.
 *
 * @deprecated use elm_object_part_content_set() instead.
 *
 */
EINA_DEPRECATED EAPI void         elm_radio_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * @brief Get the icon object of the radio object
 *
 * @param obj The radio object
 * @return The icon object
 *
 * @see elm_radio_icon_set()
 *
 * @deprecated use elm_object_part_content_get() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_radio_icon_get(const Evas_Object *obj);

/**
 * @brief Unset the icon used for the radio object
 *
 * @param obj The radio object
 * @return The icon object that was being used
 *
 * Unparent and return the icon object which was set for this widget.
 *
 * @see elm_radio_icon_set()
 * @deprecated use elm_object_part_content_unset() instead.
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_radio_icon_unset(Evas_Object *obj);

/**
 * @brief Set the content of the scroller widget (the object to be scrolled around).
 *
 * @param obj The scroller object
 * @param content The new content object
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_scroller_content_unset() function.
 * @deprecated use elm_object_content_set() instead
 */
EINA_DEPRECATED EAPI void         elm_scroller_content_set(Evas_Object *obj, Evas_Object *child);

/**
 * @brief Get the content of the scroller widget
 *
 * @param obj The slider object
 * @return The content that is being used
 *
 * Return the content object which is set for this widget
 *
 * @see elm_scroller_content_set()
 * @deprecated use elm_object_content_get() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_scroller_content_get(const Evas_Object *obj);

/**
 * @brief Unset the content of the scroller widget
 *
 * @param obj The slider object
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this widget
 *
 * @see elm_scroller_content_set()
 * @deprecated use elm_object_content_unset() instead.
 */
EINA_DEPRECATED EAPI Evas_Object *elm_scroller_content_unset(Evas_Object *obj);

/**
 * Set the label of a given slider widget
 *
 * @param obj The progress bar object
 * @param label The text label string, in UTF-8
 *
 * @ingroup Slider
 * @deprecated use elm_object_text_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_slider_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label of a given slider widget
 *
 * @param obj The progressbar object
 * @return The text label string, in UTF-8
 *
 * @ingroup Slider
 * @deprecated use elm_object_text_get() instead.
 */
EINA_DEPRECATED EAPI const char  *elm_slider_label_get(const Evas_Object *obj);

/**
 * Set the icon object of the slider object.
 *
 * @param obj The slider object.
 * @param icon The icon object.
 *
 * On horizontal mode, icon is placed at left, and on vertical mode,
 * placed at top.
 *
 * @note Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_slider_icon_unset() function.
 *
 * @warning If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @ingroup Slider
 * @deprecated use elm_object_part_content_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_slider_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Unset an icon set on a given slider widget.
 *
 * @param obj The slider object.
 * @return The icon object that was being used, if any was set, or
 * @c NULL, otherwise (and on errors).
 *
 * On horizontal mode, icon is placed at left, and on vertical mode,
 * placed at top.
 *
 * This call will unparent and return the icon object which was set
 * for this widget, previously, on success.
 *
 * @see elm_slider_icon_set() for more details
 * @see elm_slider_icon_get()
 * @deprecated use elm_object_part_content_unset() instead.
 *
 * @ingroup Slider
 */
EINA_DEPRECATED EAPI Evas_Object *elm_slider_icon_unset(Evas_Object *obj);

/**
 * Retrieve the icon object set for a given slider widget.
 *
 * @param obj The slider object.
 * @return The icon object's handle, if @p obj had one set, or @c NULL,
 * otherwise (and on errors).
 *
 * On horizontal mode, icon is placed at left, and on vertical mode,
 * placed at top.
 *
 * @see elm_slider_icon_set() for more details
 * @see elm_slider_icon_unset()
 *
 * @deprecated use elm_object_part_content_get() instead.
 *
 * @ingroup Slider
 */
EINA_DEPRECATED EAPI Evas_Object *elm_slider_icon_get(const Evas_Object *obj);

/**
 * Set the end object of the slider object.
 *
 * @param obj The slider object.
 * @param end The end object.
 *
 * On horizontal mode, end is placed at left, and on vertical mode,
 * placed at bottom.
 *
 * @note Once the icon object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_slider_end_unset() function.
 *
 * @warning If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @deprecated use elm_object_part_content_set() instead.
 *
 * @ingroup Slider
 */
EINA_DEPRECATED EAPI void         elm_slider_end_set(Evas_Object *obj, Evas_Object *end);

/**
 * Unset an end object set on a given slider widget.
 *
 * @param obj The slider object.
 * @return The end object that was being used, if any was set, or
 * @c NULL, otherwise (and on errors).
 *
 * On horizontal mode, end is placed at left, and on vertical mode,
 * placed at bottom.
 *
 * This call will unparent and return the icon object which was set
 * for this widget, previously, on success.
 *
 * @see elm_slider_end_set() for more details.
 * @see elm_slider_end_get()
 *
 * @deprecated use elm_object_part_content_unset() instead
 * instead.
 *
 * @ingroup Slider
 */
EINA_DEPRECATED EAPI Evas_Object *elm_slider_end_unset(Evas_Object *obj);

/**
 * Retrieve the end object set for a given slider widget.
 *
 * @param obj The slider object.
 * @return The end object's handle, if @p obj had one set, or @c NULL,
 * otherwise (and on errors).
 *
 * On horizontal mode, icon is placed at right, and on vertical mode,
 * placed at bottom.
 *
 * @see elm_slider_end_set() for more details.
 * @see elm_slider_end_unset()
 *
 *
 * @deprecated use elm_object_part_content_get() instead
 * instead.
 *
 * @ingroup Slider
 */
EINA_DEPRECATED EAPI Evas_Object *elm_slider_end_get(const Evas_Object *obj);

/**
 * Return the data associated with a given slideshow item
 *
 * @param it The slideshow item
 * @return Returns the data associated to this item
 *
 * @deprecated use elm_object_item_data_get() instead
 * @ingroup Slideshow
 */
EINA_DEPRECATED EAPI void *elm_slideshow_item_data_get(const Elm_Object_Item *it);

/**
 * Delete a given item from a slideshow widget.
 *
 * @param it The slideshow item
 *
 * @deprecated Use elm_object_item_de() instead
 * @ingroup Slideshow
 */
EINA_DEPRECATED EAPI void                  elm_slideshow_item_del(Elm_Object_Item *it);

/**
 * Get the toolbar object from an item.
 *
 * @param it The item.
 * @return The toolbar object.
 *
 * This returns the toolbar object itself that an item belongs to.
 *
 * @deprecated use elm_object_item_object_get() instead.
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI Evas_Object *elm_toolbar_item_toolbar_get(const Elm_Object_Item *it);

/**
 * Get the label of item.
 *
 * @param it The item of toolbar.
 * @return The label of item.
 *
 * The return value is a pointer to the label associated to @p item when
 * it was created, with function elm_toolbar_item_append() or similar,
 * or later,
 * with function elm_toolbar_item_label_set. If no label
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_toolbar_item_label_set() for more details.
 * @see elm_toolbar_item_append()
 *
 * @deprecated use elm_object_item_text_get() instead.
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI const char  *elm_toolbar_item_label_get(const Elm_Object_Item *it);

/**
 * Set the label of item.
 *
 * @param it The item of toolbar.
 * @param text The label of item.
 *
 * The label to be displayed by the item.
 * Label will be placed at icons bottom (if set).
 *
 * If a label was passed as argument on item creation, with function
 * elm_toolbar_item_append() or similar, it will be already
 * displayed by the item.
 *
 * @see elm_toolbar_item_label_get()
 * @see elm_toolbar_item_append()
 *
 * @deprecated use elm_object_item_text_set() instead
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void         elm_toolbar_item_label_set(Elm_Object_Item *it, const char *label);

/**
 * Return the data associated with a given toolbar widget item.
 *
 * @param it The toolbar widget item handle.
 * @return The data associated with @p item.
 *
 * @see elm_toolbar_item_data_set()
 *
 * @deprecated use elm_object_item_data_get() instead.
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void        *elm_toolbar_item_data_get(const Elm_Object_Item *it);

/**
 * Set the data associated with a given toolbar widget item.
 *
 * @param it The toolbar widget item handle
 * @param data The new data pointer to set to @p item.
 *
 * This sets new item data on @p item.
 *
 * @warning The old data pointer won't be touched by this function, so
 * the user had better to free that old data himself/herself.
 *
 * @deprecated use elm_object_item_data_set() instead.
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void         elm_toolbar_item_data_set(Elm_Object_Item *it, const void *data);

/**
 * Get a value whether toolbar item is disabled or not.
 *
 * @param it The item.
 * @return The disabled state.
 *
 * @see elm_toolbar_item_disabled_set() for more details.
 *
 * @deprecated use elm_object_item_disabled_get() instead.
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI Eina_Bool    elm_toolbar_item_disabled_get(const Elm_Object_Item *it);

/**
 * Sets the disabled/enabled state of a toolbar item.
 *
 * @param it The item.
 * @param disabled The disabled state.
 *
 * A disabled item cannot be selected or unselected. It will also
 * change its appearance (generally greyed out). This sets the
 * disabled state (@c EINA_TRUE for disabled, @c EINA_FALSE for
 * enabled).
 *
 * @deprecated use elm_object_item_disabled_set() instead.
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void         elm_toolbar_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled);

/**
 * Change a toolbar's orientation
 * @param obj The toolbar object
 * @param vertical If @c EINA_TRUE, the toolbar is vertical
 * By default, a toolbar will be horizontal. Use this function to create a vertical toolbar.
 * @ingroup Toolbar
 * @deprecated use elm_toolbar_horizontal_set() instead.
 */
EINA_DEPRECATED EAPI void         elm_toolbar_orientation_set(Evas_Object *obj, Eina_Bool vertical);

/**
 * Get a toolbar's orientation
 * @param obj The toolbar object
 * @return If @c EINA_TRUE, the toolbar is vertical
 * By default, a toolbar will be horizontal. Use this function to determine whether a toolbar is vertical.
 * @ingroup Toolbar
 * @deprecated use elm_toolbar_horizontal_get() instead.
 */
EINA_DEPRECATED EAPI Eina_Bool    elm_toolbar_orientation_get(const Evas_Object *obj);

/**
 * Set the function called when a toolbar item is freed.
 *
 * @param it The item to set the callback on.
 * @param func The function called.
 *
 * If there is a @p func, then it will be called prior item's memory release.
 * That will be called with the following arguments:
 * @li item's data;
 * @li item's Evas object;
 * @li item itself;
 *
 * This way, a data associated to a toolbar item could be properly freed.
 *
 * @deprecated Use elm_object_item_del_cb_set() instead 
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * Delete them item from the toolbar.
 *
 * @param it The item of toolbar to be deleted.
 *
 * @deprecated Use elm_object_item_del() instead
 * @see elm_toolbar_item_append()
 *
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_del(Elm_Object_Item *it);

/**
 * Set the text to be shown in a given toolbar item's tooltips.
 *
 * @param it toolbar item.
 * @param text The text to set in the content.
 *
 * Setup the text as tooltip to object. The item can have only one tooltip,
 * so any previous tooltip data - set with this function or
 * elm_toolbar_item_tooltip_content_cb_set() - is removed.
 *
 * @deprecated Use elm_object_item_tooltip_text_set() instead
 * @see elm_object_tooltip_text_set() for more details.
 *
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_tooltip_text_set(Elm_Object_Item *it, const char *text);

/**
 * Set the content to be shown in the tooltip item.
 *
 * Setup the tooltip to item. The item can have only one tooltip,
 * so any previous tooltip data is removed. @p func(with @p data) will
 * be called every time that need show the tooltip and it should
 * return a valid Evas_Object. This object is then managed fully by
 * tooltip system and is deleted when the tooltip is gone.
 *
 * @param it the toolbar item being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @a func, the tooltip is unset with
 *        elm_toolbar_item_tooltip_unset() or the owner @a item
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is the item.
 *
 * @deprecated Use elm_object_item_tooltip_content_cb_set() instead
 * @see elm_object_tooltip_content_cb_set() for more details.
 *
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

/**
 * Unset tooltip from item.
 *
 * @param it toolbar item to remove previously set tooltip.
 *
 * Remove tooltip from item. The callback provided as del_cb to
 * elm_toolbar_item_tooltip_content_cb_set() will be called to notify
 * it is not used anymore.
 *
 * @deprecated Use elm_object_item_tooltip_unset() instead
 * @see elm_object_tooltip_unset() for more details.
 * @see elm_toolbar_item_tooltip_content_cb_set()
 *
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_tooltip_unset(Elm_Object_Item *it);

/**
 * Sets a different style for this item tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_toolbar_item_tooltip_content_cb_set() or
 *       elm_toolbar_item_tooltip_text_set()
 *
 * @param it toolbar item with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 * @deprecated Use elm_object_item_tooltip_style_set() instead
 * @see elm_object_tooltip_style_set() for more details.
 *
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_item_tooltip_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the style for this item tooltip.
 *
 * @param it toolbar item with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @deprecated Use elm_object_item_style_get() instead
 * @see elm_object_tooltip_style_get() for more details.
 * @see elm_toolbar_item_tooltip_style_set()
 *
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI const char                  *elm_toolbar_item_tooltip_style_get(const Elm_Object_Item *it);

/**
 * @brief Link a Elm_Payer with an Elm_Video object.
 *
 * @param player the Elm_Player object.
 * @param video The Elm_Video object.
 *
 * This mean that action on the player widget will affect the
 * video object and the state of the video will be reflected in
 * the player itself.
 *
 * @see elm_player_add()
 * @see elm_video_add()
 * @deprecated use elm_object_part_content_set() instead
 *
 * @ingroup Video
 */
EINA_DEPRECATED EAPI void elm_player_video_set(Evas_Object *player, Evas_Object *video);

/**
 * Set the label of item.
 *
 * @param it The item of segment control.
 * @param text The label of item.
 *
 * The label to be displayed by the item.
 * Label will be at right of the icon (if set).
 *
 * If a label was passed as argument on item creation, with function
 * elm_control_segment_item_add(), it will be already
 * displayed by the item.
 *
 * @see elm_segment_control_item_label_get()
 * @see elm_segment_control_item_add()
 * @deprecated Use elm_object_item_text_set() instead
 *
 * @ingroup SegmentControl
 */
EINA_DEPRECATED EAPI void              elm_segment_control_item_label_set(Elm_Object_Item *it, const char *label);


/**
 * Set the icon associated to the item.
 *
 * @param it The segment control item.
 * @param icon The icon object to associate with @p it.
 *
 * The icon object to use at left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 *
 * Once the icon object is set, a previously set one will be deleted.
 * @warning Setting the same icon for two items will cause the icon to
 * dissapear from the first item.
 *
 * If an icon was passed as argument on item creation, with function
 * elm_segment_control_item_add(), it will be already
 * associated to the item.
 *
 * @see elm_segment_control_item_add()
 * @see elm_segment_control_item_icon_get()
 * @deprecated Use elm_object_item_part_content_set() instead
 *
 * @ingroup SegmentControl
 */
EINA_DEPRECATED EAPI void              elm_segment_control_item_icon_set(Elm_Object_Item *it, Evas_Object *icon);

/**
 * Remove a segment control item from its parent, deleting it.
 *
 * @param it The item to be removed.
 *
 * Items can be added with elm_segment_control_item_add() or
 * elm_segment_control_item_insert_at().
 *
 * @deprecated Use elm_object_item_del() instead
 * @ingroup SegmentControl
 */
EINA_DEPRECATED EAPI void              elm_segment_control_item_del(Elm_Object_Item *it);

/**
 * Get the label
 *
 * @param obj The multibuttonentry object
 * @return The label, or NULL if none
 *
 * @deprecated Use elm_object_text_get() instead 
 *
 */
EINA_DEPRECATED EAPI const char                *elm_multibuttonentry_label_get(const Evas_Object *obj);

/**
 * Set the label
 *
 * @param obj The multibuttonentry object
 * @param label The text label string
 *
 * @deprecated Use elm_object_text_set() instead
 *
 */
EINA_DEPRECATED EAPI void                       elm_multibuttonentry_label_set(Evas_Object *obj, const char *label);

/**
 * Get the label of a given item
 *
 * @param it The item
 * @return The label of a given item, or NULL if none
 *
 * @deprecated Use elm_object_item_text_get() instead
 *
 */
EINA_DEPRECATED EAPI const char                *elm_multibuttonentry_item_label_get(const Elm_Object_Item *it);

/**
 * Set the label of a given item
 *
 * @param it The item
 * @param str The text label string
 *
 * @deprecated Use elm_object_item_text_set() instead
 */
EINA_DEPRECATED EAPI void                       elm_multibuttonentry_item_label_set(Elm_Object_Item *it, const char *str);

/**
 * Delete a given item
 *
 * @param it The item
 * 
 * @deprecated Use elm_object_item_del() instead 
 *
 */
EINA_DEPRECATED EAPI void                       elm_multibuttonentry_item_del(Elm_Object_Item *it);

/**
 * @brief Delete the given item instantly.
 *
 * @param it The naviframe item
 *
 * This just deletes the given item from the naviframe item list instantly.
 * So this would not emit any signals for view transitions but just change
 * the current view if the given item is a top one.
 *
 * @deprecated Use elm_object_item_del() instead
 * @ingroup Naviframe
 */
EINA_DEPRECATED EAPI void             elm_naviframe_item_del(Elm_Object_Item *it);



/**
 * Sets the disabled/enabled state of a list item.
 *
 * @param it The item.
 * @param disabled The disabled state.
 *
 * A disabled item cannot be selected or unselected. It will also
 * change its appearance (generally greyed out). This sets the
 * disabled state (@c EINA_TRUE for disabled, @c EINA_FALSE for
 * enabled).
 *
 * @deprecated Use elm_object_item_disabled_set() instead
 * 
 * @ingroup List
 */
EINA_DEPRECATED EAPI void                         elm_list_item_disabled_set(Elm_Object_Item *it, Eina_Bool disabled);

/**
 * Get a value whether list item is disabled or not.
 *
 * @param it The item.
 * @return The disabled state.
 *
 * @see elm_list_item_disabled_set() for more details.
 *
 * @deprecated Use elm_object_item_disabled_get() instead
 * 
 * @ingroup List
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_list_item_disabled_get(const Elm_Object_Item *it);

/**
 * Set the function called when a list item is freed.
 *
 * @param it The item to set the callback on
 * @param func The function called
 *
 * If there is a @p func, then it will be called prior item's memory release.
 * That will be called with the following arguments:
 * @li item's data;
 * @li item's Evas object;
 * @li item itself;
 *
 * This way, a data associated to a list item could be properly freed.
 * 
 * @deprecated Please use elm_object_item_del_cb_set() instead.
 *
 * @ingroup List
 */
EINA_DEPRECATED EAPI void                         elm_list_item_del_cb_set(Elm_Object_Item *it, Evas_Smart_Cb func);

/**
 * Get the data associated to the item.
 *
 * @param it The list item
 * @return The data associated to @p item
 *
 * The return value is a pointer to data associated to @p item when it was
 * created, with function elm_list_item_append() or similar. If no data
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_list_item_append()
 *
 * @deprecated Please use elm_object_item_data_get() instead.
 * 
 * @ingroup List
 */
EINA_DEPRECATED EAPI void                        *elm_list_item_data_get(const Elm_Object_Item *it);

/**
 * Get the left side icon associated to the item.
 *
 * @param it The list item
 * @return The left side icon associated to @p item
 *
 * The return value is a pointer to the icon associated to @p item when
 * it was
 * created, with function elm_list_item_append() or similar, or later
 * with function elm_list_item_icon_set(). If no icon
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_list_item_append()
 * @see elm_list_item_icon_set()
 *
 * @deprecated Please use elm_object_item_part_content_get(item, NULL);
 * @ingroup List
 */
EINA_DEPRECATED EAPI Evas_Object                 *elm_list_item_icon_get(const Elm_Object_Item *it);

/**
 * Set the left side icon associated to the item.
 *
 * @param it The list item
 * @param icon The left side icon object to associate with @p item
 *
 * The icon object to use at left side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 *
 * Once the icon object is set, a previously set one will be deleted.
 * @warning Setting the same icon for two items will cause the icon to
 * dissapear from the first item.
 *
 * If an icon was passed as argument on item creation, with function
 * elm_list_item_append() or similar, it will be already
 * associated to the item.
 *
 * @see elm_list_item_append()
 * @see elm_list_item_icon_get()
 *
 * @deprecated Please use elm_object_item_part_content_set(item, NULL, icon);
 * @ingroup List
 */
EINA_DEPRECATED EAPI void                         elm_list_item_icon_set(Elm_Object_Item *it, Evas_Object *icon);

/**
 * Get the right side icon associated to the item.
 *
 * @param it The list item
 * @return The right side icon associated to @p item
 *
 * The return value is a pointer to the icon associated to @p item when
 * it was
 * created, with function elm_list_item_append() or similar, or later
 * with function elm_list_item_icon_set(). If no icon
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_list_item_append()
 * @see elm_list_item_icon_set()
 *
 * @deprecated Please use elm_object_item_part_content_get(item, "end");
 * @ingroup List
 */
EINA_DEPRECATED EAPI Evas_Object                 *elm_list_item_end_get(const Elm_Object_Item *it);

/**
 * Set the right side icon associated to the item.
 *
 * @param it The list item
 * @param end The right side icon object to associate with @p item
 *
 * The icon object to use at right side of the item. An
 * icon can be any Evas object, but usually it is an icon created
 * with elm_icon_add().
 *
 * Once the icon object is set, a previously set one will be deleted.
 * @warning Setting the same icon for two items will cause the icon to
 * dissapear from the first item.
 *
 * If an icon was passed as argument on item creation, with function
 * elm_list_item_append() or similar, it will be already
 * associated to the item.
 *
 * @see elm_list_item_append()
 * @see elm_list_item_end_get()
 *
 * @deprecated Please use elm_object_item_part_content_set(item, "end", end);
 * @ingroup List
 */
EINA_DEPRECATED EAPI void                         elm_list_item_end_set(Elm_Object_Item *it, Evas_Object *end);

/**
 * Get the label of item.
 *
 * @param it The item of list.
 * @return The label of item.
 *
 * The return value is a pointer to the label associated to @p item when
 * it was created, with function elm_list_item_append(), or later
 * with function elm_list_item_label_set. If no label
 * was passed as argument, it will return @c NULL.
 *
 * @see elm_list_item_label_set() for more details.
 * @see elm_list_item_append()
 *
 * @deprecated Please use elm_object_item_text_get(item);
 * @ingroup List
 */
EINA_DEPRECATED EAPI const char                  *elm_list_item_label_get(const Elm_Object_Item *it);

/**
 * Set the label of item.
 *
 * @param it The item of list.
 * @param text The label of item.
 *
 * The label to be displayed by the item.
 * Label will be placed between left and right side icons (if set).
 *
 * If a label was passed as argument on item creation, with function
 * elm_list_item_append() or similar, it will be already
 * displayed by the item.
 *
 * @see elm_list_item_label_get()
 * @see elm_list_item_append()
 *
 * @deprecated Please use elm_object_item_text_set(item, text);
 * @ingroup List
 */
EINA_DEPRECATED EAPI void                         elm_list_item_label_set(Elm_Object_Item *it, const char *text);

/**
 * Set the text to be shown in a given list item's tooltips.
 *
 * @param it Target item.
 * @param text The text to set in the content.
 *
 * Setup the text as tooltip to object. The item can have only one tooltip,
 * so any previous tooltip data - set with this function or
 * elm_list_item_tooltip_content_cb_set() - is removed.
 *
 * @deprecated Use elm_object_item_tooltip_text_set() instead
 * @see elm_object_tooltip_text_set() for more details.
 *
 * @ingroup List
 */
EINA_DEPRECATED EAPI void                         elm_list_item_tooltip_text_set(Elm_Object_Item *it, const char *text);

/**
 * @brief Disable size restrictions on an object's tooltip
 * @param it The tooltip's anchor object
 * @param disable If EINA_TRUE, size restrictions are disabled
 * @return EINA_FALSE on failure, EINA_TRUE on success
 *
 * This function allows a tooltip to expand beyond its parant window's canvas.
 * It will instead be limited only by the size of the display.
 *
 * @deprecated Use elm_object_item_tooltip_window_mode_set() instead
 *
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_list_item_tooltip_window_mode_set(Elm_Object_Item *it, Eina_Bool disable);

/**
 * @brief Retrieve size restriction state of an object's tooltip
 * @param obj The tooltip's anchor object
 * @return If EINA_TRUE, size restrictions are disabled
 *
 * This function returns whether a tooltip is allowed to expand beyond
 * its parant window's canvas.
 * It will instead be limited only by the size of the display.
 *
 * @deprecated Use elm_object_item_tooltip_window_mode_get() instead
 *
 */
EINA_DEPRECATED EAPI Eina_Bool                    elm_list_item_tooltip_window_mode_get(const Elm_Object_Item *it);

/**
 * Set the content to be shown in the tooltip item.
 *
 * Setup the tooltip to item. The item can have only one tooltip,
 * so any previous tooltip data is removed. @p func(with @p data) will
 * be called every time that need show the tooltip and it should
 * return a valid Evas_Object. This object is then managed fully by
 * tooltip system and is deleted when the tooltip is gone.
 *
 * @param it the list item being attached a tooltip.
 * @param func the function used to create the tooltip contents.
 * @param data what to provide to @a func as callback data/context.
 * @param del_cb called when data is not needed anymore, either when
 *        another callback replaces @a func, the tooltip is unset with
 *        elm_list_item_tooltip_unset() or the owner @a item
 *        dies. This callback receives as the first parameter the
 *        given @a data, and @c event_info is the item.
 *
 * @deprecated Use elm_object_item_tooltip_content_cb_set() instead
 *
 * @see elm_object_tooltip_content_cb_set() for more details.
 *
 * @ingroup List
 */
EINA_DEPRECATED EAPI void                         elm_list_item_tooltip_content_cb_set(Elm_Object_Item *it, Elm_Tooltip_Item_Content_Cb func, const void *data, Evas_Smart_Cb del_cb);

/**
 * Unset tooltip from item.
 *
 * @param it list item to remove previously set tooltip.
 *
 * Remove tooltip from item. The callback provided as del_cb to
 * elm_list_item_tooltip_content_cb_set() will be called to notify
 * it is not used anymore.
 *
 * @deprecated Use elm_object_item_tooltip_unset() instead
 * @see elm_object_tooltip_unset() for more details.
 * @see elm_list_item_tooltip_content_cb_set()
 *
 * @ingroup List
 */
EINA_DEPRECATED EAPI void                         elm_list_item_tooltip_unset(Elm_Object_Item *it);

/**
 * Sets a different style for this item tooltip.
 *
 * @note before you set a style you should define a tooltip with
 *       elm_list_item_tooltip_content_cb_set() or
 *       elm_list_item_tooltip_text_set()
 *
 * @param it list item with tooltip already set.
 * @param style the theme style to use (default, transparent, ...)
 *
 *
 * @deprecated Use elm_object_item_tooltip_style_set() instead
 * @see elm_object_tooltip_style_set() for more details.
 *
 * @ingroup List
 */
EINA_DEPRECATED EAPI void                         elm_list_item_tooltip_style_set(Elm_Object_Item *it, const char *style);

/**
 * Get the style for this item tooltip.
 *
 * @param item list item with tooltip already set.
 * @return style the theme style in use, defaults to "default". If the
 *         object does not have a tooltip set, then NULL is returned.
 *
 * @deprecated Use elm_object_item_tooltip_style_get() instead
 *
 * @see elm_object_tooltip_style_get() for more details.
 * @see elm_list_item_tooltip_style_set()
 *
 * @ingroup List
 */
EINA_DEPRECATED EAPI const char                  *elm_list_item_tooltip_style_get(const Elm_Object_Item *it);


/**
 * Set the shrink state of toolbar @p obj.
 *
 * @param obj The toolbar object.
 * @param shrink_mode Toolbar's items display behavior.
 *
 * The toolbar won't scroll if #ELM_TOOLBAR_SHRINK_NONE,
 * but will enforce a minimun size so all the items will fit, won't scroll
 * and won't show the items that don't fit if #ELM_TOOLBAR_SHRINK_HIDE,
 * will scroll if #ELM_TOOLBAR_SHRINK_SCROLL, and will create a button to
 * pop up excess elements with #ELM_TOOLBAR_SHRINK_MENU.
 *
 * @deprecated Please use elm_toolbar_shrink_mode_set(obj, shrink_mode);
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI void                         elm_toolbar_mode_shrink_set(Evas_Object *obj, Elm_Toolbar_Shrink_Mode shrink_mode);

/**
 * Get the shrink mode of toolbar @p obj.
 *
 * @param obj The toolbar object.
 * @return Toolbar's items display behavior.
 *
 * @see elm_toolbar_shrink_mode_set() for details.
 *
 * @deprecated Please use elm_toolbar_shrink_mode_get(obj);
 * @ingroup Toolbar
 */
EINA_DEPRECATED EAPI Elm_Toolbar_Shrink_Mode      elm_toolbar_mode_shrink_get(const Evas_Object *obj);

/**
 * This sets a widget to be displayed to the left of a scrolled entry.
 *
 * @param obj The scrolled entry object
 * @param icon The widget to display on the left side of the scrolled
 * entry.
 *
 * @note A previously set widget will be destroyed.
 * @note If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @deprecated Use elm_object_part_content_set(entry, "icon", content) instead
 * @see elm_entry_end_set()
 */
EINA_DEPRECATED EAPI void               elm_entry_icon_set(Evas_Object *obj, Evas_Object *icon);

/**
 * Gets the leftmost widget of the scrolled entry. This object is
 * owned by the scrolled entry and should not be modified.
 *
 * @param obj The scrolled entry object
 * @return the left widget inside the scroller
 *
 * @deprecated Use elm_object_part_content_get(entry, "icon") instead
 */
EINA_DEPRECATED EAPI Evas_Object       *elm_entry_icon_get(const Evas_Object *obj);

/**
 * Unset the leftmost widget of the scrolled entry, unparenting and
 * returning it.
 *
 * @param obj The scrolled entry object
 * @return the previously set icon sub-object of this entry, on
 * success.
 *
 * @deprecated Use elm_object_part_content_unset(entry, "icon") instead
 * @see elm_entry_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object       *elm_entry_icon_unset(Evas_Object *obj);

/**
 * This sets a widget to be displayed to the end of a scrolled entry.
 *
 * @param obj The scrolled entry object
 * @param end The widget to display on the right side of the scrolled
 * entry.
 *
 * @note A previously set widget will be destroyed.
 * @note If the object being set does not have minimum size hints set,
 * it won't get properly displayed.
 *
 * @deprecated Use elm_object_part_content_set(entry, "end", content) instead
 * @see elm_entry_icon_set
 */
EINA_DEPRECATED EAPI void               elm_entry_end_set(Evas_Object *obj, Evas_Object *end);

/**
 * Gets the endmost widget of the scrolled entry. This object is owned
 * by the scrolled entry and should not be modified.
 *
 * @param obj The scrolled entry object
 * @return the right widget inside the scroller
 *
 * @deprecated Use elm_object_part_content_get(entry, "end") instead
 */
EAPI Evas_Object       *elm_entry_end_get(const Evas_Object *obj);

/**
 * Unset the endmost widget of the scrolled entry, unparenting and
 * returning it.
 *
 * @param obj The scrolled entry object
 * @return the previously set icon sub-object of this entry, on
 * success.
 *
 * @deprecated Use elm_object_part_content_unset(entry, "end") instead
 * @see elm_entry_icon_set()
 */
EINA_DEPRECATED EAPI Evas_Object       *elm_entry_end_unset(Evas_Object *obj);

/**
 * Convert a pixel coordinate into a rotated pixel coordinate.
 *
 * @param obj The map object.
 * @param x horizontal coordinate of the point to rotate.
 * @param y vertical coordinate of the point to rotate.
 * @param cx rotation's center horizontal position.
 * @param cy rotation's center vertical position.
 * @param degree amount of degrees from 0.0 to 360.0 to rotate arount Z axis.
 * @param xx Pointer where to store rotated x.
 * @param yy Pointer where to store rotated y.
 *
 * @ingroup Map
 * @deprecated Rotation is not needed to know. Use elm_map_canvas_to_geo_convert() instead
 */
EINA_DEPRECATED EAPI void                  elm_map_utils_rotate_coord(const Evas_Object *obj, const Evas_Coord x, const Evas_Coord y, const Evas_Coord cx, const Evas_Coord cy, const double degree, Evas_Coord *xx, Evas_Coord *yy);

