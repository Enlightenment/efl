#define ELM_OBJ_PANES_CLASS elm_obj_panes_class_get()

const Eo_Class *elm_obj_panes_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_PANES_BASE_ID;

enum
{
   ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_GET,
   ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_SET,
   ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_GET,
   ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_SET,
   ELM_OBJ_PANES_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_PANES_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_PANES_SUB_ID_FIXED_SET,
   ELM_OBJ_PANES_SUB_ID_FIXED_GET,
   ELM_OBJ_PANES_SUB_ID_LAST
};

#define ELM_OBJ_PANES_ID(sub_id) (ELM_OBJ_PANES_BASE_ID + sub_id)


/**
 * @def elm_obj_panes_content_left_size_get
 * @since 1.8
 *
 * Get the size proportion of panes widget's left side.
 *
 * @param[out] ret
 *
 * @see elm_panes_content_left_size_get
 *
 * @ingroup Panes
 */
#define elm_obj_panes_content_left_size_get(ret) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_panes_content_left_size_set
 * @since 1.8
 *
 * Set the size proportion of panes widget's left side.
 *
 * @param[in] size
 *
 * @see elm_panes_content_left_size_set
 *
 * @ingroup Panes
 */
#define elm_obj_panes_content_left_size_set(size) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_LEFT_SIZE_SET), EO_TYPECHECK(double, size)

/**
 * @def elm_obj_panes_content_right_size_get
 * @since 1.8
 *
 * Get the size proportion of panes widget's right side.
 *
 * @param[out] ret
 *
 * @see elm_panes_content_right_size_get
 *
 * @ingroup Panes
 */
#define elm_obj_panes_content_right_size_get(ret) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_GET), EO_TYPECHECK(double *, ret)

/**
 * @def elm_obj_panes_content_right_size_set
 * @since 1.8
 *
 * Set the size proportion of panes widget's right side.
 *
 * @param[in] size
 *
 * @see elm_panes_content_right_size_set
 *
 * @ingroup Panes
 */
#define elm_obj_panes_content_right_size_set(size) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_CONTENT_RIGHT_SIZE_SET), EO_TYPECHECK(double, size)

/**
 * @def elm_obj_panes_horizontal_set
 * @since 1.8
 *
 * Set how to split and dispose each content.
 *
 * @param[in] horizontal
 *
 * @see elm_panes_horizontal_set
 *
 * @ingroup Panes
 */
#define elm_obj_panes_horizontal_set(horizontal) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_panes_horizontal_get
 * @since 1.8
 *
 * Retrieve the split direction of a given panes widget.
 *
 * @param[out] ret
 *
 * @see elm_panes_horizontal_get
 *
 * @ingroup Panes
 */
#define elm_obj_panes_horizontal_get(ret) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_panes_fixed_set
 * @since 1.8
 *
 * Set whether the left and right panes can be resized by user interaction.
 *
 * @param[in] fixed
 *
 * @see elm_panes_fixed_set
 *
 * @ingroup Panes
 */
#define elm_obj_panes_fixed_set(fixed) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_FIXED_SET), EO_TYPECHECK(Eina_Bool, fixed)

/**
 * @def elm_obj_panes_fixed_get
 * @since 1.8
 *
 * Retrieve the resize mode for the panes of a given panes widget.
 *
 * @param[out] ret
 *
 * @see elm_panes_fixed_get
 *
 * @ingroup Panes
 */
#define elm_obj_panes_fixed_get(ret) ELM_OBJ_PANES_ID(ELM_OBJ_PANES_SUB_ID_FIXED_GET), EO_TYPECHECK(Eina_Bool *, ret)
