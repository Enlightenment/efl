   /**
    * @addtogroup Actionslider Actionslider
    *
    * @image html img/widget/actionslider/preview-00.png
    * @image latex img/widget/actionslider/preview-00.eps
    *
    * An actionslider is a switcher for 2 or 3 labels with customizable magnet
    * properties. The user drags and releases the indicator, to choose a label.
    *
    * Labels occupy the following positions.
    * a. Left
    * b. Right
    * c. Center
    *
    * Positions can be enabled or disabled.
    *
    * Magnets can be set on the above positions.
    *
    * When the indicator is released, it will move to its nearest "enabled and magnetized" position.
    *
    * @note By default all positions are set as enabled.
    *
    * Signals that you can add callbacks for are:
    *
    * "selected" - when user selects an enabled position (the label is passed
    *              as event info)".
    * @n
    * "pos_changed" - when the indicator reaches any of the positions("left",
    *                 "right" or "center").
    *
    * See an example of actionslider usage @ref actionslider_example_page "here"
    * @{
    */
   typedef enum _Elm_Actionslider_Pos
     {
        ELM_ACTIONSLIDER_NONE = 0,
        ELM_ACTIONSLIDER_LEFT = 1 << 0,
        ELM_ACTIONSLIDER_CENTER = 1 << 1,
        ELM_ACTIONSLIDER_RIGHT = 1 << 2,
        ELM_ACTIONSLIDER_ALL = (1 << 3) -1
     } Elm_Actionslider_Pos;

   /**
    * Add a new actionslider to the parent.
    *
    * @param parent The parent object
    * @return The new actionslider object or NULL if it cannot be created
    */
   EAPI Evas_Object          *elm_actionslider_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

   /**
    * Set actionslider labels.
    *
    * @param obj The actionslider object
    * @param left_label The label to be set on the left.
    * @param center_label The label to be set on the center.
    * @param right_label The label to be set on the right.
    * @deprecated use elm_object_text_set() instead.
    */
   EINA_DEPRECATED EAPI void                  elm_actionslider_labels_set(Evas_Object *obj, const char *left_label, const char *center_label, const char *right_label) EINA_ARG_NONNULL(1);

   /**
    * Get actionslider labels.
    *
    * @param obj The actionslider object
    * @param left_label A char** to place the left_label of @p obj into.
    * @param center_label A char** to place the center_label of @p obj into.
    * @param right_label A char** to place the right_label of @p obj into.
    * @deprecated use elm_object_text_set() instead.
    */
   EINA_DEPRECATED EAPI void                  elm_actionslider_labels_get(const Evas_Object *obj, const char **left_label, const char **center_label, const char **right_label) EINA_ARG_NONNULL(1);

   /**
    * Get actionslider selected label.
    *
    * @param obj The actionslider object
    * @return The selected label
    */
   EAPI const char           *elm_actionslider_selected_label_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * Set actionslider indicator position.
    *
    * @param obj The actionslider object.
    * @param pos The position of the indicator.
    */
   EAPI void                  elm_actionslider_indicator_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos) EINA_ARG_NONNULL(1);

   /**
    * Get actionslider indicator position.
    *
    * @param obj The actionslider object.
    * @return The position of the indicator.
    */
   EAPI Elm_Actionslider_Pos  elm_actionslider_indicator_pos_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * Set actionslider magnet position. To make multiple positions magnets @c or
    * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT)
    *
    * @param obj The actionslider object.
    * @param pos Bit mask indicating the magnet positions.
    */
   EAPI void                  elm_actionslider_magnet_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos) EINA_ARG_NONNULL(1);

   /**
    * Get actionslider magnet position.
    *
    * @param obj The actionslider object.
    * @return The positions with magnet property.
    */
   EAPI Elm_Actionslider_Pos  elm_actionslider_magnet_pos_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * Set actionslider enabled position. To set multiple positions as enabled @c or
    * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT).
    *
    * @note All the positions are enabled by default.
    *
    * @param obj The actionslider object.
    * @param pos Bit mask indicating the enabled positions.
    */
   EAPI void                  elm_actionslider_enabled_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos) EINA_ARG_NONNULL(1);

   /**
    * Get actionslider enabled position.
    *
    * @param obj The actionslider object.
    * @return The enabled positions.
    */
   EAPI Elm_Actionslider_Pos  elm_actionslider_enabled_pos_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * Set the label used on the indicator.
    *
    * @param obj The actionslider object
    * @param label The label to be set on the indicator.
    * @deprecated use elm_object_text_set() instead.
    */
   EINA_DEPRECATED EAPI void                  elm_actionslider_indicator_label_set(Evas_Object *obj, const char *label) EINA_ARG_NONNULL(1);

   /**
    * Get the label used on the indicator object.
    *
    * @param obj The actionslider object
    * @return The indicator label
    * @deprecated use elm_object_text_get() instead.
    */
   EINA_DEPRECATED EAPI const char           *elm_actionslider_indicator_label_get(Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @}
    */

