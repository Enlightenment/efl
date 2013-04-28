#define ELM_OBJ_MENU_CLASS elm_obj_menu_class_get()

const Eo_Class *elm_obj_menu_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_MENU_BASE_ID;

enum
{
   ELM_OBJ_MENU_SUB_ID_MOVE,
   ELM_OBJ_MENU_SUB_ID_CLOSE,
   ELM_OBJ_MENU_SUB_ID_ITEM_ADD,
   ELM_OBJ_MENU_SUB_ID_ITEM_SEPARATOR_ADD,
   ELM_OBJ_MENU_SUB_ID_ITEMS_GET,
   ELM_OBJ_MENU_SUB_ID_FIRST_ITEM_GET,
   ELM_OBJ_MENU_SUB_ID_LAST_ITEM_GET,
   ELM_OBJ_MENU_SUB_ID_SELECTED_ITEM_GET,
   ELM_OBJ_MENU_SUB_ID_LAST
};

#define ELM_OBJ_MENU_ID(sub_id) (ELM_OBJ_MENU_BASE_ID + sub_id)


/**
 * @def elm_obj_menu_move
 * @since 1.8
 *
 * @brief Move the menu to a new position
 *
 * @param[in] x
 * @param[in] y
 *
 * @see elm_menu_move
 *
 * @ingroup Menu
 */
#define elm_obj_menu_move(x, y) ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_MOVE), EO_TYPECHECK(Evas_Coord, x), EO_TYPECHECK(Evas_Coord, y)

/**
 * @def elm_obj_menu_close
 * @since 1.8
 *
 * @brief Close a opened menu
 *
 *
 * @see elm_menu_close
 *
 * @ingroup Menu
 */
#define elm_obj_menu_close() ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_CLOSE)

/**
 * @def elm_obj_menu_item_add
 * @since 1.8
 *
 * @brief Add an item at the end of the given menu widget
 *
 * @param[in] parent
 * @param[in] icon
 * @param[in] label
 * @param[in] func
 * @param[in] data
 * @param[out] ret
 *
 * @see elm_menu_item_add
 *
 * @ingroup Menu
 */
#define elm_obj_menu_item_add(parent, icon, label, func, data, ret) ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_ITEM_ADD), EO_TYPECHECK(Elm_Object_Item *, parent), EO_TYPECHECK(const char *, icon), EO_TYPECHECK(const char *, label), EO_TYPECHECK(Evas_Smart_Cb, func), EO_TYPECHECK(const void *, data), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_menu_item_separator_add
 * @since 1.8
 *
 * @brief Add a separator item to menu obj under @p parent.
 *
 * @param[in] parent
 * @param[out] ret
 *
 * @see elm_menu_item_separator_add
 *
 * @ingroup Menu
 */
#define elm_obj_menu_item_separator_add(parent, ret) ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_ITEM_SEPARATOR_ADD), EO_TYPECHECK(Elm_Object_Item *, parent), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_menu_items_get
 * @since 1.8
 *
 * @brief Returns a list of item's items.
 *
 * @param[out] ret
 *
 * @see elm_menu_items_get
 *
 * @ingroup Menu
 */
#define elm_obj_menu_items_get(ret) ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_ITEMS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_menu_first_item_get
 * @since 1.8
 *
 * @brief Get the first item in the menu
 *
 * @param[out] ret
 *
 * @see elm_menu_first_item_get
 *
 * @ingroup Menu
 */
#define elm_obj_menu_first_item_get(ret) ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_FIRST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_menu_last_item_get
 * @since 1.8
 *
 * @brief Get the last item in the menu
 *
 * @param[out] ret
 *
 * @see elm_menu_last_item_get
 *
 * @ingroup Menu
 */
#define elm_obj_menu_last_item_get(ret) ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_LAST_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_menu_selected_item_get
 * @since 1.8
 *
 * @brief Get the selected item in the menu
 *
 * @param[out] ret
 *
 * @see elm_menu_selected_item_get
 *
 * @ingroup Menu
 */
#define elm_obj_menu_selected_item_get(ret) ELM_OBJ_MENU_ID(ELM_OBJ_MENU_SUB_ID_SELECTED_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)
