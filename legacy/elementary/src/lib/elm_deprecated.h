   /* TEMPORARY: DOCS WILL BE FILLED IN WITH CNP/SED */
   // XXX: elm_gen -> kill.
   typedef struct Elm_Gen_Item Elm_Gen_Item;
   typedef struct _Elm_Gen_Item_Class Elm_Gen_Item_Class;
   typedef struct _Elm_Gen_Item_Class_Func Elm_Gen_Item_Class_Func; /**< Class functions for gen item classes. */
   typedef char        *(*Elm_Gen_Item_Text_Get_Cb) (void *data, Evas_Object *obj, const char *part); /**< Label fetching class function for gen item classes. */
   typedef Evas_Object *(*Elm_Gen_Item_Content_Get_Cb)  (void *data, Evas_Object *obj, const char *part); /**< Content(swallowed object) fetching class function for gen item classes. */
   typedef Eina_Bool    (*Elm_Gen_Item_State_Get_Cb) (void *data, Evas_Object *obj, const char *part); /**< State fetching class function for gen item classes. */
   typedef void         (*Elm_Gen_Item_Del_Cb)      (void *data, Evas_Object *obj); /**< Deletion class function for gen item classes. */
   struct _Elm_Gen_Item_Class
     {
        const char             *item_style;
        struct _Elm_Gen_Item_Class_Func
          {
             Elm_Gen_Item_Text_Get_Cb  text_get;
             Elm_Gen_Item_Content_Get_Cb  content_get;
             Elm_Gen_Item_State_Get_Cb state_get;
             Elm_Gen_Item_Del_Cb       del;
          } func;
     };
   EINA_DEPRECATED EAPI void elm_gen_clear(Evas_Object *obj);
   EINA_DEPRECATED EAPI void elm_gen_item_selected_set(Elm_Gen_Item *it, Eina_Bool selected);
   EINA_DEPRECATED EAPI Eina_Bool elm_gen_item_selected_get(const Elm_Gen_Item *it);
   EINA_DEPRECATED EAPI void elm_gen_always_select_mode_set(Evas_Object *obj, Eina_Bool always_select);
   EINA_DEPRECATED EAPI Eina_Bool elm_gen_always_select_mode_get(const Evas_Object *obj);
   EINA_DEPRECATED EAPI void elm_gen_no_select_mode_set(Evas_Object *obj, Eina_Bool no_select);
   EINA_DEPRECATED EAPI Eina_Bool elm_gen_no_select_mode_get(const Evas_Object *obj);
   EINA_DEPRECATED EAPI void elm_gen_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce);
   EINA_DEPRECATED EAPI void elm_gen_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce);
   EINA_DEPRECATED EAPI void elm_gen_page_relative_set(Evas_Object *obj, double h_pagerel, double v_pagerel);
   EINA_DEPRECATED EAPI void elm_gen_page_relative_get(const Evas_Object *obj, double *h_pagerel, double *v_pagerel);

   EINA_DEPRECATED EAPI void elm_gen_page_size_set(Evas_Object *obj, Evas_Coord h_pagesize, Evas_Coord v_pagesize);
   EINA_DEPRECATED EAPI void elm_gen_current_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);
   EINA_DEPRECATED EAPI void elm_gen_last_page_get(const Evas_Object *obj, int *h_pagenumber, int *v_pagenumber);
   EINA_DEPRECATED EAPI void elm_gen_page_show(const Evas_Object *obj, int h_pagenumber, int v_pagenumber);
   EINA_DEPRECATED EAPI void elm_gen_page_bring_in(const Evas_Object *obj, int h_pagenumber, int v_pagenumber);
   EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_first_item_get(const Evas_Object *obj);
   EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_last_item_get(const Evas_Object *obj);
   EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_item_next_get(const Elm_Gen_Item *it);
   EINA_DEPRECATED EAPI Elm_Gen_Item *elm_gen_item_prev_get(const Elm_Gen_Item *it);
   EINA_DEPRECATED EAPI Evas_Object *elm_gen_item_widget_get(const Elm_Gen_Item *it);

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
   EINA_DEPRECATED EAPI Evas_Object *elm_toggle_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

   /**
    * @brief Sets the label to be displayed with the toggle.
    *
    * @param obj The toggle object
    * @param label The label to be displayed
    *
    * @deprecated use elm_object_text_set() instead.
    */
   EINA_DEPRECATED EAPI void         elm_toggle_label_set(Evas_Object *obj, const char *label) EINA_ARG_NONNULL(1);

   /**
    * @brief Gets the label of the toggle
    *
    * @param obj  toggle object
    * @return The label of the toggle
    *
    * @deprecated use elm_object_text_get() instead.
    */
   EINA_DEPRECATED EAPI const char  *elm_toggle_label_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EINA_DEPRECATED EAPI void         elm_toggle_icon_set(Evas_Object *obj, Evas_Object *icon) EINA_ARG_NONNULL(1);

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
   EINA_DEPRECATED EAPI Evas_Object *elm_toggle_icon_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EINA_DEPRECATED EAPI Evas_Object *elm_toggle_icon_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EINA_DEPRECATED EAPI void         elm_toggle_states_labels_set(Evas_Object *obj, const char *onlabel, const char *offlabel) EINA_ARG_NONNULL(1);

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
   EINA_DEPRECATED EAPI void         elm_toggle_states_labels_get(const Evas_Object *obj, const char **onlabel, const char **offlabel) EINA_ARG_NONNULL(1);

   /**
    * @brief Sets the state of the toggle to @p state.
    *
    * @param obj The toggle object
    * @param state The state of @p obj
    *
    * @deprecated use elm_check_state_set() instead.
    */
   EINA_DEPRECATED EAPI void         elm_toggle_state_set(Evas_Object *obj, Eina_Bool state) EINA_ARG_NONNULL(1);

   /**
    * @brief Gets the state of the toggle to @p state.
    *
    * @param obj The toggle object
    * @return The state of @p obj
    *
    * @deprecated use elm_check_state_get() instead.
    */
   EINA_DEPRECATED EAPI Eina_Bool    elm_toggle_state_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @brief Sets the state pointer of the toggle to @p statep.
    *
    * @param obj The toggle object
    * @param statep The state pointer of @p obj
    *
    * @deprecated use elm_check_state_pointer_set() instead.
    */
   EINA_DEPRECATED EAPI void         elm_toggle_state_pointer_set(Evas_Object *obj, Eina_Bool *statep) EINA_ARG_NONNULL(1);

   /**
    * @}
    */

   EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_add(Evas_Object *parent) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_single_line_set(Evas_Object *obj, Eina_Bool single_line) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_single_line_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_password_set(Evas_Object *obj, Eina_Bool password) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_password_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_entry_set(Evas_Object *obj, const char *entry) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI const char  *elm_scrolled_entry_entry_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_entry_append(Evas_Object *obj, const char *entry) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_is_empty(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI const char  *elm_scrolled_entry_selection_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_entry_insert(Evas_Object *obj, const char *entry) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_line_wrap_set(Evas_Object *obj, Elm_Wrap_Type wrap) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_editable_set(Evas_Object *obj, Eina_Bool editable) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_editable_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_select_none(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_select_all(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_next(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_prev(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_up(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_down(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_begin_set(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_end_set(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_line_begin_set(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_line_end_set(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_selection_begin(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_selection_end(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_is_format_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cursor_is_visible_format_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI const char  *elm_scrolled_entry_cursor_content_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_cursor_pos_set(Evas_Object *obj, int pos) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI int          elm_scrolled_entry_cursor_pos_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_selection_cut(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_selection_copy(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_selection_paste(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_context_menu_clear(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_context_menu_item_add(Evas_Object *obj, const char *label, const char *icon_file, Elm_Icon_Type icon_type, Evas_Smart_Cb func, const void *data) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_context_menu_disabled_set(Evas_Object *obj, Eina_Bool disabled) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_context_menu_disabled_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_scrollbar_policy_set(Evas_Object *obj, Elm_Scroller_Policy h, Elm_Scroller_Policy v) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_bounce_set(Evas_Object *obj, Eina_Bool h_bounce, Eina_Bool v_bounce) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_bounce_get(const Evas_Object *obj, Eina_Bool *h_bounce, Eina_Bool *v_bounce) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_icon_set(Evas_Object *obj, Evas_Object *icon) EINA_ARG_NONNULL(1, 2);
   EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_icon_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_icon_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_icon_visible_set(Evas_Object *obj, Eina_Bool setting) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_end_set(Evas_Object *obj, Evas_Object *end) EINA_ARG_NONNULL(1, 2);
   EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_end_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Evas_Object *elm_scrolled_entry_end_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_end_visible_set(Evas_Object *obj, Eina_Bool setting) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_item_provider_append(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data) EINA_ARG_NONNULL(1, 2);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_item_provider_prepend(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data) EINA_ARG_NONNULL(1, 2);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_item_provider_remove(Evas_Object *obj, Evas_Object *(*func) (void *data, Evas_Object *entry, const char *item), void *data) EINA_ARG_NONNULL(1, 2);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_text_filter_append(Evas_Object *obj, void (*func) (void *data, Evas_Object *entry, char **text), void *data) EINA_ARG_NONNULL(1, 2);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_text_filter_prepend(Evas_Object *obj, void (*func) (void *data, Evas_Object *entry, char **text), void *data) EINA_ARG_NONNULL(1, 2);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_text_filter_remove(Evas_Object *obj, void (*func) (void *data, Evas_Object *entry, char **text), void *data) EINA_ARG_NONNULL(1, 2);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_file_set(Evas_Object *obj, const char *file, Elm_Text_Format format) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_file_get(const Evas_Object *obj, const char **file, Elm_Text_Format *format) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_file_save(Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_autosave_set(Evas_Object *obj, Eina_Bool autosave) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_autosave_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI void         elm_scrolled_entry_cnp_textonly_set(Evas_Object *obj, Eina_Bool textonly) EINA_ARG_NONNULL(1);
   EINA_DEPRECATED EAPI Eina_Bool    elm_scrolled_entry_cnp_textonly_get(Evas_Object *obj) EINA_ARG_NONNULL(1);

