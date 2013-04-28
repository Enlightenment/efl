#define ELM_OBJ_HOVERSEL_CLASS elm_obj_hoversel_class_get()

const Eo_Class *elm_obj_hoversel_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_HOVERSEL_BASE_ID;

enum
{
   ELM_OBJ_HOVERSEL_SUB_ID_HOVER_PARENT_SET,
   ELM_OBJ_HOVERSEL_SUB_ID_HOVER_PARENT_GET,
   ELM_OBJ_HOVERSEL_SUB_ID_HORIZONTAL_SET,
   ELM_OBJ_HOVERSEL_SUB_ID_HORIZONTAL_GET,
   ELM_OBJ_HOVERSEL_SUB_ID_HOVER_BEGIN,
   ELM_OBJ_HOVERSEL_SUB_ID_HOVER_END,
   ELM_OBJ_HOVERSEL_SUB_ID_EXPANDED_GET,
   ELM_OBJ_HOVERSEL_SUB_ID_CLEAR,
   ELM_OBJ_HOVERSEL_SUB_ID_ITEMS_GET,
   ELM_OBJ_HOVERSEL_SUB_ID_ITEM_ADD,
   ELM_OBJ_HOVERSEL_SUB_ID_LAST
};

#define ELM_OBJ_HOVERSEL_ID(sub_id) (ELM_OBJ_HOVERSEL_BASE_ID + sub_id)


/**
 * @def elm_obj_hoversel_hover_parent_set
 * @since 1.8
 *
 * @brief Set the Hover parent
 *
 * @param[in] parent
 *
 * @see elm_hoversel_hover_parent_set
 *
 * @ingroup Hoversel
 */
#define elm_obj_hoversel_hover_parent_set(parent) ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_PARENT_SET), EO_TYPECHECK(Evas_Object *, parent)

/**
 * @def elm_obj_hoversel_hover_parent_get
 * @since 1.8
 *
 * @brief Get the Hover parent
 *
 * @param[out] ret
 *
 * @see elm_hoversel_hover_parent_get
 *
 * @ingroup Hoversel
 */
#define elm_obj_hoversel_hover_parent_get(ret) ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_PARENT_GET), EO_TYPECHECK(Evas_Object **, ret)

/**
 * @def elm_obj_hoversel_horizontal_set
 * @since 1.8
 *
 * @brief This sets the hoversel to expand horizontally.
 *
 * @param[in] horizontal
 *
 * @see elm_hoversel_horizontal_set
 *
 * @ingroup Hoversel
 */
#define elm_obj_hoversel_horizontal_set(horizontal) ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HORIZONTAL_SET), EO_TYPECHECK(Eina_Bool, horizontal)

/**
 * @def elm_obj_hoversel_horizontal_get
 * @since 1.8
 *
 * @brief This returns whether the hoversel is set to expand horizontally.
 *
 * @param[out] ret
 *
 * @see elm_hoversel_horizontal_get
 *
 * @ingroup Hoversel
 */
#define elm_obj_hoversel_horizontal_get(ret) ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HORIZONTAL_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_hoversel_hover_begin
 * @since 1.8
 *
 * @brief This triggers the hoversel popup from code, the same as if the user
 * had clicked the button.
 *
 *
 * @see elm_hoversel_hover_begin
 *
 * @ingroup Hoversel
 */
#define elm_obj_hoversel_hover_begin() ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_BEGIN)

/**
 * @def elm_obj_hoversel_hover_end
 * @since 1.8
 *
 * @brief This dismisses the hoversel popup as if the user had clicked
 * outside the hover.
 *
 *
 * @see elm_hoversel_hover_end
 *
 * @ingroup Hoversel
 */
#define elm_obj_hoversel_hover_end() ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_HOVER_END)

/**
 * @def elm_obj_hoversel_expanded_get
 * @since 1.8
 *
 * @brief Returns whether the hoversel is expanded.
 *
 * @param[out] ret
 *
 * @see elm_hoversel_expanded_get
 *
 * @ingroup Hoversel
 */
#define elm_obj_hoversel_expanded_get(ret) ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_EXPANDED_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_hoversel_clear
 * @since 1.8
 *
 * @brief This will remove all the children items from the hoversel.
 *
 *
 * @see elm_hoversel_clear
 *
 * @ingroup Hoversel
 */
#define elm_obj_hoversel_clear() ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_CLEAR)

/**
 * @def elm_obj_hoversel_items_get
 * @since 1.8
 *
 * @brief Get the list of items within the given hoversel.
 *
 * @param[out] ret
 *
 * @see elm_hoversel_items_get
 *
 * @ingroup Hoversel
 */
#define elm_obj_hoversel_items_get(ret) ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_ITEMS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_hoversel_item_add
 * @since 1.8
 *
 * @brief Add an item to the hoversel button
 *
 * @param[in] label
 * @param[in] icon_file
 * @param[in] icon_type
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_hoversel_item_add
 *
 * @ingroup Hoversel
 */
#define elm_obj_hoversel_item_add(label, icon_file, icon_type, func, data, ret) ELM_OBJ_HOVERSEL_ID(ELM_OBJ_HOVERSEL_SUB_ID_ITEM_ADD), EO_TYPECHECK(const char *, label), EO_TYPECHECK(const char *, icon_file), EO_TYPECHECK(Elm_Icon_Type, icon_type), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)
