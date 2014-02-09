/**
 * @ingroup Label
 *
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
   ELM_OBJ_LABEL_SUB_ID_SLIDE_MODE_SET,
   ELM_OBJ_LABEL_SUB_ID_SLIDE_MODE_GET,
   ELM_OBJ_LABEL_SUB_ID_SLIDE_DURATION_SET,
   ELM_OBJ_LABEL_SUB_ID_SLIDE_DURATION_GET,
   ELM_OBJ_LABEL_SUB_ID_SLIDE_GO,
   ELM_OBJ_LABEL_SUB_ID_SLIDE_SPEED_SET,
   ELM_OBJ_LABEL_SUB_ID_SLIDE_SPEED_GET,
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
 *
 * @ingroup Label
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
 *
 * @ingroup Label
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
 *
 * @ingroup Label
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
 *
 * @ingroup Label
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
 *
 * @ingroup Label
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
 *
 * @ingroup Label
 */
#define elm_obj_label_ellipsis_get(ret) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_ELLIPSIS_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_label_slide_mode_set
 * @since 1.8
 *
 * @brief Set slide effect mode of label widget.
 *
 * @param[in] mode
 *
 * @see elm_label_slide_mode_set
 *
 * @ingroup Label
 */
#define elm_obj_label_slide_mode_set(mode) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_SLIDE_MODE_SET), EO_TYPECHECK(Elm_Label_Slide_Mode, mode)

/**
 * @def elm_obj_label_slide_mode_get
 * @since 1.8
 *
 * @brief Get current slide effect mode.
 *
 * @param[out] ret
 *
 * @see elm_label_slide_mode_get
 *
 * @ingroup Label
 */
#define elm_obj_label_slide_mode_get(ret) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_SLIDE_MODE_GET), EO_TYPECHECK(Elm_Label_Slide_Mode *, ret)

/**
 * @def elm_obj_label_slide_duration_set
 * @since 1.8
 *
 * @brief Set the slide duration of the label
 *
 * @param[in] duration
 *
 * @see elm_label_slide_duration_set
 *
 * @ingroup Label
 */
#define elm_obj_label_slide_duration_set(duration) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_SLIDE_DURATION_SET), EO_TYPECHECK(double, duration)

/**
 * @def elm_obj_label_slide_duration_get
 * @since 1.8
 *
 * @brief Get the slide duration of the label
 *
 * @param[out] ret
 *
 * @see elm_label_slide_duration_get
 *
 * @ingroup Label
 */
#define elm_obj_label_slide_duration_get(ret) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_SLIDE_DURATION_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_label_slide_speed_set
 * @since 1.9
 *
 * @brief Set the slide speed of the label
 *
 * @param[in] speed
 *
 * @see elm_label_slide_speed_set
 *
 * @ingroup Label
 */
#define elm_obj_label_slide_speed_set(speed) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_SLIDE_SPEED_SET), EO_TYPECHECK(double, speed)

/**
 * @def elm_obj_label_slide_speed_get
 * @since 1.9
 *
 * @brief Get the slide speed of the label
 *
 * @param[out] ret
 *
 * @see elm_label_slide_speed_get
 *
 * @ingroup Label
 */
#define elm_obj_label_slide_speed_get(ret) ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_SLIDE_SPEED_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_label_slide_go
 * @since 1.8
 *
 * @brief Start slide effect
 *
 * @see elm_label_slide_mode_set
 *
 * @ingroup Label
 */
#define elm_obj_label_slide_go() ELM_OBJ_LABEL_ID(ELM_OBJ_LABEL_SUB_ID_SLIDE_GO)
/**
 * @}
 */
