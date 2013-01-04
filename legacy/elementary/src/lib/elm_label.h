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

#define ELM_OBJ_LABEL_CLASS elm_obj_label_class_get()

const Eo_Class *elm_obj_label_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_LABEL_BASE_ID;

enum
{
   ELM_OBJ_LABEL_SUB_ID_LINE_WRAP_SET,
   ELM_OBJ_LABEL_SUB_ID_LINE_WRAP_GET,
   ELM_OBJ_LABEL_SUB_ID_WRAP_WIDTH_SET,
   ELM_OBJ_LABEL_SUB_ID_WRAP_WIDTH_GET,
   ELM_OBJ_LABEL_SUB_ID_ELLIPSIS_SET,
   ELM_OBJ_LABEL_SUB_ID_ELLIPSIS_GET,
   ELM_OBJ_LABEL_SUB_ID_SLIDE_SET,
   ELM_OBJ_LABEL_SUB_ID_SLIDE_GET,
   ELM_OBJ_LABEL_SUB_ID_SLIDE_DURATION_SET,
   ELM_OBJ_LABEL_SUB_ID_SLIDE_DURATION_GET,
   ELM_OBJ_LABEL_SUB_ID_LAST
};

#define ELM_OBJ_LABEL_ID(sub_id) (ELM_OBJ_LABEL_BASE_ID + sub_id)


/**
 * @def elm_obj_label_line_wrap_set
 * @since 1.8
 *
 * @brief Set the wrapping behavior of the label
 *
 * @param[in] wrap
 *
 * @see elm_label_line_wrap_set
 */
#define elm_obj_label_line_wrap_set(wrap) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_LINE_WRAP_SET), EO_TYPECHECK(Elm_Wrap_Type, wrap)

/**
 * @def elm_obj_label_line_wrap_get
 * @since 1.8
 *
 * @brief Get the wrapping behavior of the label
 *
 * @param[out] ret
 *
 * @see elm_label_line_wrap_get
 */
#define elm_obj_label_line_wrap_get(ret) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_LINE_WRAP_GET), EO_TYPECHECK(Elm_Wrap_Type *, ret)

/**
 * @def elm_obj_label_wrap_width_set
 * @since 1.8
 *
 * @brief Set wrap width of the label
 *
 * @param[in] w
 *
 * @see elm_label_wrap_width_set
 */
#define elm_obj_label_wrap_width_set(w) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_WRAP_WIDTH_SET), EO_TYPECHECK(Evas_Coord, w)

/**
 * @def elm_obj_label_wrap_width_get
 * @since 1.8
 *
 * @brief Get wrap width of the label
 *
 * @param[out] ret
 *
 * @see elm_label_wrap_width_get
 */
#define elm_obj_label_wrap_width_get(ret) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_WRAP_WIDTH_GET), EO_TYPECHECK(Evas_Coord *, ret)

/**
 * @def elm_obj_label_ellipsis_set
 * @since 1.8
 *
 * @brief Set the ellipsis behavior of the label
 *
 * @param[in] ellipsis
 *
 * @see elm_label_ellipsis_set
 */
#define elm_obj_label_ellipsis_set(ellipsis) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_ELLIPSIS_SET), EO_TYPECHECK(Eina_Bool, ellipsis)

/**
 * @def elm_obj_label_ellipsis_get
 * @since 1.8
 *
 * @brief Get the ellipsis behavior of the label
 *
 * @param[out] ret
 *
 * @see elm_label_ellipsis_get
 */
#define elm_obj_label_ellipsis_get(ret) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_ELLIPSIS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_label_slide_set
 * @since 1.8
 *
 * @brief Set slide effect of label widget.
 *
 * @param[in] slide
 *
 * @see elm_label_slide_set
 */
#define elm_obj_label_slide_set(slide) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_SLIDE_SET), EO_TYPECHECK(Eina_Bool, slide)

/**
 * @def elm_obj_label_slide_get
 * @since 1.8
 *
 * @brief Get whether slide effect is shown or not.
 *
 * @param[out] ret
 *
 * @see elm_label_slide_get
 */
#define elm_obj_label_slide_get(ret) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_SLIDE_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_label_slide_duration_set
 * @since 1.8
 *
 * @brief Set the slide duration (speed) of the label
 *
 * @param[in] duration
 *
 * @see elm_label_slide_duration_set
 */
#define elm_obj_label_slide_duration_set(duration) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_SLIDE_DURATION_SET), EO_TYPECHECK(double, duration)

/**
 * @def elm_obj_label_slide_duration_get
 * @since 1.8
 *
 * @brief Get the slide duration(speed) of the label
 *
 * @param[out] ret
 *
 * @see elm_label_slide_duration_get
 */
#define elm_obj_label_slide_duration_get(ret) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_SLIDE_DURATION_GET), EO_TYPECHECK(double *, ret)
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
 * @brief Set slide effect of label widget.
 *
 * @param obj The label object
 * @param slide If true, slide effect will be shown
 *
 * If set to true, the text of the label will slide/scroll through the length of
 * label.
 *
 * @warning This only works with the themes "slide_short", "slide_long" and
 * "slide_bounce".
 * @warning This doesn't work if the line wrap(elm_label_line_wrap_set()) or
 * ellipsis(elm_label_ellipsis_set()) is set.
 *
 * @ingroup Label
 */
EAPI void                        elm_label_slide_set(Evas_Object *obj, Eina_Bool slide);

/**
 * @brief Get whether slide effect is shown or not.
 *
 * @param obj The label object
 * @return If true, slide effect is shown.
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
