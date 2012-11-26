/**
 * @defgroup Separator Separator
 * @ingroup Elementary
 *
 * @image html separator_inheritance_tree.png
 * @image latex separator_inheritance_tree.eps
 *
 * @brief Separator is a very thin object used to separate other objects.
 *
 * A separator can be vertical or horizontal.
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for separator objects.
 *
 * This widget emits the signals coming from @ref Layout.
 *
 * @ref tutorial_separator is a good example of how to use a separator.
 * @{
 */

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
 */
#define elm_obj_separator_horizontal_get(ret) ELM_OBJ_SEPARATOR_ID(ELM_OBJ_SEPARATOR_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @brief Add a separator object to @p parent
 *
 * @param parent The parent object
 *
 * @return The separator object, or NULL upon failure
 *
 * @ingroup Separator
 */
EAPI Evas_Object *elm_separator_add(Evas_Object *parent);

/**
 * @brief Set the horizontal mode of a separator object
 *
 * @param obj The separator object
 * @param horizontal If true, the separator is horizontal
 *
 * @ingroup Separator
 */
EAPI void      elm_separator_horizontal_set(Evas_Object *obj, Eina_Bool horizontal);

/**
 * @brief Get the horizontal mode of a separator object
 *
 * @param obj The separator object
 * @return If true, the separator is horizontal
 *
 * @see elm_separator_horizontal_set()
 *
 * @ingroup Separator
 */
EAPI Eina_Bool elm_separator_horizontal_get(const Evas_Object *obj);

/**
 * @}
 */
