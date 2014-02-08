#define ELM_OBJ_COLORSELECTOR_CLASS elm_obj_colorselector_class_get()

const Eo_Class *elm_obj_colorselector_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_COLORSELECTOR_BASE_ID;

enum
{
   ELM_OBJ_COLORSELECTOR_SUB_ID_COLOR_SET,
   ELM_OBJ_COLORSELECTOR_SUB_ID_COLOR_GET,
   ELM_OBJ_COLORSELECTOR_SUB_ID_MODE_SET,
   ELM_OBJ_COLORSELECTOR_SUB_ID_MODE_GET,
   ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_COLOR_ADD,
   ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_CLEAR,
   ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_SELECTED_ITEM_GET,
   ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_ITEMS_GET,
   ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_NAME_SET,
   ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_NAME_GET,
   ELM_OBJ_COLORSELECTOR_SUB_ID_LAST
};

#define ELM_OBJ_COLORSELECTOR_ID(sub_id) (ELM_OBJ_COLORSELECTOR_BASE_ID + sub_id)


/**
 * @def elm_obj_colorselector_color_set
 * @since 1.8
 *
 * Set color to colorselector
 *
 * @param[in] r
 * @param[in] g
 * @param[in] b
 * @param[in] a
 *
 * @see elm_colorselector_color_set
 *
 * @ingroup Colorselector
 */
#define elm_obj_colorselector_color_set(r, g, b, a) ELM_OBJ_COLORSELECTOR_ID(ELM_OBJ_COLORSELECTOR_SUB_ID_COLOR_SET), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a)

/**
 * @def elm_obj_colorselector_color_get
 * @since 1.8
 *
 * Get current color from colorselector
 *
 * @param[out] r
 * @param[out] g
 * @param[out] b
 * @param[out] a
 *
 * @see elm_colorselector_color_get
 *
 * @ingroup Colorselector
 */
#define elm_obj_colorselector_color_get(r, g, b, a) ELM_OBJ_COLORSELECTOR_ID(ELM_OBJ_COLORSELECTOR_SUB_ID_COLOR_GET), EO_TYPECHECK(int *, r), EO_TYPECHECK(int *, g), EO_TYPECHECK(int *, b), EO_TYPECHECK(int *, a)

/**
 * @def elm_obj_colorselector_mode_set
 * @since 1.8
 *
 * Set Colorselector's mode.
 *
 * @param[in] mode
 *
 * @see elm_colorselector_mode_set
 *
 * @ingroup Colorselector
 */
#define elm_obj_colorselector_mode_set(mode) ELM_OBJ_COLORSELECTOR_ID(ELM_OBJ_COLORSELECTOR_SUB_ID_MODE_SET), EO_TYPECHECK(Elm_Colorselector_Mode, mode)

/**
 * @def elm_obj_colorselector_mode_get
 * @since 1.8
 *
 * Get Colorselector's mode.
 *
 * @param[out] ret
 *
 * @see elm_colorselector_mode_get
 *
 * @ingroup Colorselector
 */
#define elm_obj_colorselector_mode_get(ret) ELM_OBJ_COLORSELECTOR_ID(ELM_OBJ_COLORSELECTOR_SUB_ID_MODE_GET), EO_TYPECHECK(Elm_Colorselector_Mode *, ret)

/**
 * @def elm_obj_colorselector_palette_color_add
 * @since 1.8
 *
 * Add a new color item to palette.
 *
 * @param[in] r
 * @param[in] g
 * @param[in] b
 * @param[in] a
 * @param[out] ret
 *
 * @see elm_colorselector_palette_color_add
 *
 * @ingroup Colorselector
 */
#define elm_obj_colorselector_palette_color_add(r, g, b, a, ret) ELM_OBJ_COLORSELECTOR_ID(ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_COLOR_ADD), EO_TYPECHECK(int, r), EO_TYPECHECK(int, g), EO_TYPECHECK(int, b), EO_TYPECHECK(int, a), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_colorselector_palette_clear
 * @since 1.8
 *
 * Clear the palette items.
 *
 *
 * @see elm_colorselector_palette_clear
 *
 * @ingroup Colorselector
 */
#define elm_obj_colorselector_palette_clear() ELM_OBJ_COLORSELECTOR_ID(ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_CLEAR)

/**
 * @def elm_obj_colorselector_palette_items_get
 * @since 1.9
 *
 * Get list of palette items.
 *
 * @param[out] ret
 *
 * @ingroup Colorselector
 */
#define elm_obj_colorselector_palette_items_get(ret) ELM_OBJ_COLORSELECTOR_ID(ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_ITEMS_GET), EO_TYPECHECK(const Eina_List **, ret)

/**
 * @def elm_obj_colorselector_palette_selected_item_get
 * @since 1.9
 *
 * Get current selected palette item
 *
 * @param[out] ret
 *
 * @ingroup Colorselector
 */
#define elm_obj_colorselector_palette_selected_item_get(ret) ELM_OBJ_COLORSELECTOR_ID(ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_SELECTED_ITEM_GET), EO_TYPECHECK(Elm_Object_Item **, ret)

/**
 * @def elm_obj_colorselector_palette_name_set
 * @since 1.8
 *
 * Set current palette's name
 *
 * @param[in] palette_name
 *
 * @see elm_colorselector_palette_name_set
 *
 * @ingroup Colorselector
 */
#define elm_obj_colorselector_palette_name_set(palette_name) ELM_OBJ_COLORSELECTOR_ID(ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_NAME_SET), EO_TYPECHECK(const char *, palette_name)

/**
 * @def elm_obj_colorselector_palette_name_get
 * @since 1.8
 *
 * Get current palette's name
 *
 * @param[out] ret
 *
 * @see elm_colorselector_palette_name_get
 *
 * @ingroup Colorselector
 */
#define elm_obj_colorselector_palette_name_get(ret) ELM_OBJ_COLORSELECTOR_ID(ELM_OBJ_COLORSELECTOR_SUB_ID_PALETTE_NAME_GET), EO_TYPECHECK(const char **, ret)
