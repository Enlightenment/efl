#define ELM_OBJ_SEPARATOR_CLASS elm_obj_separator_class_get()

const Eo_Class *elm_obj_separator_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_SEPARATOR_BASE_ID;

enum
{
   ELM_OBJ_SEPARATOR_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_SEPARATOR_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_SEPARATOR_SUB_ID_LAST
};

#define ELM_OBJ_SEPARATOR_ID(sub_id) (ELM_OBJ_SEPARATOR_BASE_ID + sub_id)


/**
 * @def elm_obj_separator_horizontal_set
 * @since 1.8
 *
 * @brief Set the horizontal mode of a separator object
 *
 * @param[in] horizontal
 *
 * @see elm_separator_horizontal_set
 *
 * @ingroup Separator
 */
#define elm_obj_separator_horizontal_set(horizontal) ELM_OBJ_SEPARATOR_ID(ELM_OBJ_SEPARATOR_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_separator_horizontal_get
 * @since 1.8
 *
 * @brief Get the horizontal mode of a separator object
 *
 * @param[out] ret
 *
 * @see elm_separator_horizontal_get
 *
 * @ingroup Separator
 */
#define elm_obj_separator_horizontal_get(ret) ELM_OBJ_SEPARATOR_ID(ELM_OBJ_SEPARATOR_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)
