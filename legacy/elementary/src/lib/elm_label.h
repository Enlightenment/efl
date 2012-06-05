/**
 * @defgroup Label Label
 * @ingroup Elementary
 *
 * @image html label_inheritance_tree.png
 * @image latex label_inheritance_tree.eps
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
 * @li marker - Centers the text in the label and makes it bold by default
 * @li slide_long - The entire text appears from the right of the screen and
 * slides until it disappears in the left of the screen(reappearing on the
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
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for label objects.
 *
 * This widget emits the following signals, besides the ones sent from
 * @ref Layout:
 * @li @c "language,changed": The program's language changed.
 *
 * See @ref tutorial_label for a demonstration of how to use a label widget.
 * @{
 */

/**
 * @brief Add a new label to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Label
 */
EAPI Evas_Object                *elm_label_add(Evas_Object *parent);

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
 *
 * @ingroup Label
 */
EAPI void                        elm_label_line_wrap_set(Evas_Object *obj, Elm_Wrap_Type wrap);

/**
 * @brief Get the wrapping behavior of the label
 *
 * @param obj The label object
 * @return Wrap type
 *
 * @see elm_label_line_wrap_set()
 *
 * @ingroup Label
 */
EAPI Elm_Wrap_Type               elm_label_line_wrap_get(const Evas_Object *obj);

/**
 * @brief Set wrap width of the label
 *
 * @param obj The label object
 * @param w The wrap width in pixels at a minimum where words need to wrap
 *
 * This function sets the maximum width size hint of the label.
 *
 * @warning This is only relevant if the label is inside a container.
 *
 * @ingroup Label
 */
EAPI void                        elm_label_wrap_width_set(Evas_Object *obj, Evas_Coord w);

/**
 * @brief Get wrap width of the label
 *
 * @param obj The label object
 * @return The wrap width in pixels at a minimum where words need to wrap
 *
 * @see elm_label_wrap_width_set()
 *
 * @ingroup Label
 */
EAPI Evas_Coord                  elm_label_wrap_width_get(const Evas_Object *obj);

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
 * chosen wrap method was #ELM_WRAP_WORD.
 *
 * @ingroup Label
 */
EAPI void                        elm_label_ellipsis_set(Evas_Object *obj, Eina_Bool ellipsis);

/**
 * @brief Get the ellipsis behavior of the label
 *
 * @param obj The label object
 * @return If true, an ellipsis will be shown at the end of the label area.
 *
 * @see elm_label_ellipsis_set()
 *
 * @ingroup Label
 */
EAPI Eina_Bool                   elm_label_ellipsis_get(const Evas_Object *obj);

/**
 * @brief Set sliding effect of label widget.
 *
 * @param obj The label object
 * @param slide If true, sliding effect will be shown
 *
 * If set to true, the text of the label will slide/scroll through the length of
 * label.
 *
 * @warning This only works with the themes "slide_short", "slide_long" and
 * "slide_bounce".
 *
 * @ingroup Label
 */
EAPI void                        elm_label_slide_set(Evas_Object *obj, Eina_Bool slide);

/**
 * @brief Get whether sliding effect is shown or not.
 *
 * @param obj The label object
 * @return If true, sliding effect is shown.
 *
 * @see elm_label_slide_set()
 *
 * @ingroup Label
 */
EAPI Eina_Bool                   elm_label_slide_get(const Evas_Object *obj);

/**
 * @brief Set the slide duration (speed) of the label
 *
 * @param obj The label object
 * @param duration The duration in seconds in moving text from slide begin position
 * to slide end position
 *
 * @ingroup Label
 */
EAPI void                        elm_label_slide_duration_set(Evas_Object *obj, double duration);

/**
 * @brief Get the slide duration(speed) of the label
 *
 * @param obj The label object
 * @return The duration time in moving text from slide begin position to slide end position
 *
 * @see elm_label_slide_duration_set()
 *
 * @ingroup Label
 */
EAPI double                      elm_label_slide_duration_get(const Evas_Object *obj);

/**
 * @}
 */
