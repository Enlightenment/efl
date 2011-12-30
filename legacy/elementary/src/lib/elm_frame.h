   /**
    * @defgroup Frame Frame
    *
    * @image html img/widget/frame/preview-00.png
    * @image latex img/widget/frame/preview-00.eps
    *
    * @brief Frame is a widget that holds some content and has a title.
    *
    * The default look is a frame with a title, but Frame supports multple
    * styles:
    * @li default
    * @li pad_small
    * @li pad_medium
    * @li pad_large
    * @li pad_huge
    * @li outdent_top
    * @li outdent_bottom
    *
    * Of all this styles only default shows the title. Frame emits no signals.
    *
    * Default contents parts of the frame widget that you can use for are:
    * @li "default" - A content of the frame
    *
    * Default text parts of the frame widget that you can use for are:
    * @li "elm.text" - Label of the frame
    *
    * For a detailed example see the @ref tutorial_frame.
    *
    * @{
    */

   /**
    * @brief Add a new frame to the parent
    *
    * @param parent The parent object
    * @return The new object or NULL if it cannot be created
    */
   EAPI Evas_Object *elm_frame_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the frame label
    *
    * @param obj The frame object
    * @param label The label of this frame object
    *
    * @deprecated use elm_object_text_set() instead.
    */
   EINA_DEPRECATED EAPI void         elm_frame_label_set(Evas_Object *obj, const char *label) EINA_ARG_NONNULL(1);

   /**
    * @brief Get the frame label
    *
    * @param obj The frame object
    *
    * @return The label of this frame objet or NULL if unable to get frame
    *
    * @deprecated use elm_object_text_get() instead.
    */
   EINA_DEPRECATED EAPI const char  *elm_frame_label_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EINA_DEPRECATED EAPI void         elm_frame_content_set(Evas_Object *obj, Evas_Object *content) EINA_ARG_NONNULL(1);

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
   EINA_DEPRECATED EAPI Evas_Object *elm_frame_content_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

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
   EINA_DEPRECATED EAPI Evas_Object *elm_frame_content_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @}
    */

