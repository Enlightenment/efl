   /**
    * @defgroup Label Label
    *
    * @image html img/widget/label/preview-00.png
    * @image latex img/widget/label/preview-00.eps
    *
    * @brief Widget to display text, with simple html-like markup.
    *
    * The Label widget @b doesn't allow text to overflow its boundaries, if the
    * text doesn't fit the geometry of the label it will be ellipsized or be
    * cut. Elementary provides several styles for this widget:
    * @li default - No animation
    * @li marker - Centers the text in the label and make it bold by default
    * @li slide_long - The entire text appears from the right of the screen and
    * slides until it disappears in the left of the screen(reappering on the
    * right again).
    * @li slide_short - The text appears in the left of the label and slides to
    * the right to show the overflow. When all of the text has been shown the
    * position is reset.
    * @li slide_bounce - The text appears in the left of the label and slides to
    * the right to show the overflow. When all of the text has been shown the
    * animation reverses, moving the text to the left.
    *
    * Custom themes can of course invent new markup tags and style them any way
    * they like.
    *
    * The following signals may be emitted by the label widget:
    * @li "language,changed": The program's language changed.
    *
    * See @ref tutorial_label for a demonstration of how to use a label widget.
    * @{
    */

   /**
    * @brief Add a new label to the parent
    *
    * @param parent The parent object
    * @return The new object or NULL if it cannot be created
    */
   EAPI Evas_Object *elm_label_add(Evas_Object *parent) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the label on the label object
    *
    * @param obj The label object
    * @param label The label will be used on the label object
    * @deprecated See elm_object_text_set()
    */
   EINA_DEPRECATED EAPI void elm_label_label_set(Evas_Object *obj, const char *label) EINA_ARG_NONNULL(1);

   /**
    * @brief Get the label used on the label object
    *
    * @param obj The label object
    * @return The string inside the label
    * @deprecated See elm_object_text_get()
    */
   EINA_DEPRECATED EAPI const char *elm_label_label_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the wrapping behavior of the label
    *
    * @param obj The label object
    * @param wrap To wrap text or not
    *
    * By default no wrapping is done. Possible values for @p wrap are:
    * @li ELM_WRAP_NONE - No wrapping
    * @li ELM_WRAP_CHAR - wrap between characters
    * @li ELM_WRAP_WORD - wrap between words
    * @li ELM_WRAP_MIXED - Word wrap, and if that fails, char wrap
    */
   EAPI void         elm_label_line_wrap_set(Evas_Object *obj, Elm_Wrap_Type wrap) EINA_ARG_NONNULL(1);

   /**
    * @brief Get the wrapping behavior of the label
    *
    * @param obj The label object
    * @return Wrap type
    *
    * @see elm_label_line_wrap_set()
    */
   EAPI Elm_Wrap_Type elm_label_line_wrap_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @brief Set wrap width of the label
    *
    * @param obj The label object
    * @param w The wrap width in pixels at a minimum where words need to wrap
    *
    * This function sets the maximum width size hint of the label.
    *
    * @warning This is only relevant if the label is inside a container.
    */
   EAPI void         elm_label_wrap_width_set(Evas_Object *obj, Evas_Coord w) EINA_ARG_NONNULL(1);

   /**
    * @brief Get wrap width of the label
    *
    * @param obj The label object
    * @return The wrap width in pixels at a minimum where words need to wrap
    *
    * @see elm_label_wrap_width_set()
    */
   EAPI Evas_Coord   elm_label_wrap_width_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @brief Set wrap height of the label
    *
    * @param obj The label object
    * @param h The wrap height in pixels at a minimum where words need to wrap
    *
    * This function sets the maximum height size hint of the label.
    *
    * @warning This is only relevant if the label is inside a container.
    */
   EAPI void         elm_label_wrap_height_set(Evas_Object *obj, Evas_Coord h) EINA_ARG_NONNULL(1);

   /**
    * @brief get wrap width of the label
    *
    * @param obj The label object
    * @return The wrap height in pixels at a minimum where words need to wrap
    */
   EAPI Evas_Coord   elm_label_wrap_height_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the font size on the label object.
    *
    * @param obj The label object
    * @param size font size
    *
    * @warning NEVER use this. It is for hyper-special cases only. use styles
    * instead. e.g. "default", "marker", "slide_long" etc.
    */
   EAPI void         elm_label_fontsize_set(Evas_Object *obj, int fontsize) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the text color on the label object
    *
    * @param obj The label object
    * @param r Red property background color of The label object
    * @param g Green property background color of The label object
    * @param b Blue property background color of The label object
    * @param a Alpha property background color of The label object
    *
    * @warning NEVER use this. It is for hyper-special cases only. use styles
    * instead. e.g. "default", "marker", "slide_long" etc.
    */
   EAPI void         elm_label_text_color_set(Evas_Object *obj, unsigned int r, unsigned int g, unsigned int b, unsigned int a) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the text align on the label object
    *
    * @param obj The label object
    * @param align align mode ("left", "center", "right")
    *
    * @warning NEVER use this. It is for hyper-special cases only. use styles
    * instead. e.g. "default", "marker", "slide_long" etc.
    */
   EAPI void         elm_label_text_align_set(Evas_Object *obj, const char *alignmode) EINA_ARG_NONNULL(1);

   /**
    * @brief Set background color of the label
    *
    * @param obj The label object
    * @param r Red property background color of The label object
    * @param g Green property background color of The label object
    * @param b Blue property background color of The label object
    * @param a Alpha property background alpha of The label object
    *
    * @warning NEVER use this. It is for hyper-special cases only. use styles
    * instead. e.g. "default", "marker", "slide_long" etc.
    */
   EAPI void         elm_label_background_color_set(Evas_Object *obj, unsigned int r, unsigned int g, unsigned int b, unsigned int a) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the ellipsis behavior of the label
    *
    * @param obj The label object
    * @param ellipsis To ellipsis text or not
    *
    * If set to true and the text doesn't fit in the label an ellipsis("...")
    * will be shown at the end of the widget.
    *
    * @warning This doesn't work with slide(elm_label_slide_set()) or if the
    * choosen wrap method was ELM_WRAP_WORD.
    */
   EAPI void         elm_label_ellipsis_set(Evas_Object *obj, Eina_Bool ellipsis) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the text slide of the label
    *
    * @param obj The label object
    * @param slide To start slide or stop
    *
    * If set to true, the text of the label will slide/scroll through the length of
    * label.
    *
    * @warning This only works with the themes "slide_short", "slide_long" and
    * "slide_bounce".
    */
   EAPI void         elm_label_slide_set(Evas_Object *obj, Eina_Bool slide) EINA_ARG_NONNULL(1);

   /**
    * @brief Get the text slide mode of the label
    *
    * @param obj The label object
    * @return slide slide mode value
    *
    * @see elm_label_slide_set()
    */
   EAPI Eina_Bool    elm_label_slide_get(Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @brief Set the slide duration(speed) of the label
    *
    * @param obj The label object
    * @return The duration in seconds in moving text from slide begin position
    * to slide end position
    */
   EAPI void         elm_label_slide_duration_set(Evas_Object *obj, double duration) EINA_ARG_NONNULL(1);

   /**
    * @brief Get the slide duration(speed) of the label
    *
    * @param obj The label object
    * @return The duration time in moving text from slide begin position to slide end position
    *
    * @see elm_label_slide_duration_set()
    */
   EAPI double       elm_label_slide_duration_get(Evas_Object *obj) EINA_ARG_NONNULL(1);

   /**
    * @}
    */

