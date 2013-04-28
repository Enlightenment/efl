#define ELM_OBJ_ACTIONSLIDER_CLASS elm_obj_actionslider_class_get()

const Eo_Class *elm_obj_actionslider_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_ACTIONSLIDER_BASE_ID;

enum
{
   ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_SET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_GET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_SET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_GET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_SET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_GET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_SELECTED_LABEL_GET,
   ELM_OBJ_ACTIONSLIDER_SUB_ID_LAST
};

#define ELM_OBJ_ACTIONSLIDER_ID(sub_id) (ELM_OBJ_ACTIONSLIDER_BASE_ID + sub_id)


/**
 * @def elm_obj_actionslider_indicator_pos_set
 * @since 1.8
 *
 * Set actionslider indicator position.
 *
 * @param[in] pos
 *
 * @see elm_actionslider_indicator_pos_set
 *
 * @ingroup Actionslider
 */
#define elm_obj_actionslider_indicator_pos_set(pos) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_SET), EO_TYPECHECK(Elm_Actionslider_Pos, pos)

/**
 * @def elm_obj_actionslider_indicator_pos_get
 * @since 1.8
 *
 * Get actionslider indicator position.
 *
 * @param[out] ret
 *
 * @see elm_actionslider_indicator_pos_get
 *
 * @ingroup Actionslider
 */
#define elm_obj_actionslider_indicator_pos_get(ret) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_INDICATOR_POS_GET), EO_TYPECHECK(Elm_Actionslider_Pos *, ret)

/**
 * @def elm_obj_actionslider_magnet_pos_set
 * @since 1.8
 *
 * Set actionslider magnet position. To make multiple positions magnets or
 * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT)
 *
 * @param[in] pos
 *
 * @see elm_actionslider_magnet_pos_set
 *
 * @ingroup Actionslider
 */
#define elm_obj_actionslider_magnet_pos_set(pos) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_SET), EO_TYPECHECK(Elm_Actionslider_Pos, pos)

/**
 * @def elm_obj_actionslider_magnet_pos_get
 * @since 1.8
 *
 * Get actionslider magnet position.
 *
 * @param[out] ret
 *
 * @see elm_actionslider_magnet_pos_get
 *
 * @ingroup Actionslider
 */
#define elm_obj_actionslider_magnet_pos_get(ret) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_MAGNET_POS_GET), EO_TYPECHECK(Elm_Actionslider_Pos *, ret)

/**
 * @def elm_obj_actionslider_enabled_pos_set
 * @since 1.8
 *
 * Set actionslider enabled position. To set multiple positions as enabled or
 * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT).
 *
 * @param[in] pos
 *
 * @see elm_actionslider_enabled_pos_set
 *
 * @ingroup Actionslider
 */
#define elm_obj_actionslider_enabled_pos_set(pos) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_SET), EO_TYPECHECK(Elm_Actionslider_Pos, pos)

/**
 * @def elm_obj_actionslider_enabled_pos_get
 * @since 1.8
 *
 * Get actionslider enabled position.
 *
 * @param[out] ret
 *
 * @see elm_actionslider_enabled_pos_get
 *
 * @ingroup Actionslider
 */
#define elm_obj_actionslider_enabled_pos_get(ret) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_ENABLED_POS_GET), EO_TYPECHECK(Elm_Actionslider_Pos *, ret)

/**
 * @def elm_obj_actionslider_selected_label_get
 * @since 1.8
 *
 * Get actionslider selected label.
 *
 * @param[out] ret
 *
 * @see elm_actionslider_selected_label_get
 *
 * @ingroup Actionslider
 */
#define elm_obj_actionslider_selected_label_get(ret) ELM_OBJ_ACTIONSLIDER_ID(ELM_OBJ_ACTIONSLIDER_SUB_ID_SELECTED_LABEL_GET), EO_TYPECHECK(const char **, ret)
