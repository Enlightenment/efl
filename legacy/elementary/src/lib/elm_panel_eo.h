#define ELM_OBJ_PANEL_CLASS elm_obj_panel_class_get()

const Eo_Class *elm_obj_panel_class_get(void) EINA_CONST;

extern EAPI Eo_Op ELM_OBJ_PANEL_BASE_ID;

enum
{
   ELM_OBJ_PANEL_SUB_ID_ORIENT_SET,
   ELM_OBJ_PANEL_SUB_ID_ORIENT_GET,
   ELM_OBJ_PANEL_SUB_ID_HIDDEN_SET,
   ELM_OBJ_PANEL_SUB_ID_HIDDEN_GET,
   ELM_OBJ_PANEL_SUB_ID_TOGGLE,
   ELM_OBJ_PANEL_SUB_ID_LAST
};

#define ELM_OBJ_PANEL_ID(sub_id) (ELM_OBJ_PANEL_BASE_ID + sub_id)


/**
 * @def elm_obj_panel_orient_set
 * @since 1.8
 *
 * @brief Sets the orientation of the panel
 *
 * @param[in] orient
 *
 * @see elm_panel_orient_set
 *
 * @ingroup Panel
 */
#define elm_obj_panel_orient_set(orient) ELM_OBJ_PANEL_ID(ELM_OBJ_PANEL_SUB_ID_ORIENT_SET), EO_TYPECHECK(Elm_Panel_Orient, orient)

/**
 * @def elm_obj_panel_orient_get
 * @since 1.8
 *
 * @brief Get the orientation of the panel.
 *
 * @param[out] ret
 *
 * @see elm_panel_orient_get
 *
 * @ingroup Panel
 */
#define elm_obj_panel_orient_get(ret) ELM_OBJ_PANEL_ID(ELM_OBJ_PANEL_SUB_ID_ORIENT_GET), EO_TYPECHECK(Elm_Panel_Orient *, ret)

/**
 * @def elm_obj_panel_hidden_set
 * @since 1.8
 *
 * @brief Set the state of the panel.
 *
 * @param[in] hidden
 *
 * @see elm_panel_hidden_set
 *
 * @ingroup Panel
 */
#define elm_obj_panel_hidden_set(hidden) ELM_OBJ_PANEL_ID(ELM_OBJ_PANEL_SUB_ID_HIDDEN_SET), EO_TYPECHECK(Eina_Bool, hidden)

/**
 * @def elm_obj_panel_hidden_get
 * @since 1.8
 *
 * @brief Get the state of the panel.
 *
 * @param[out] ret
 *
 * @see elm_panel_hidden_get
 *
 * @ingroup Panel
 */
#define elm_obj_panel_hidden_get(ret) ELM_OBJ_PANEL_ID(ELM_OBJ_PANEL_SUB_ID_HIDDEN_GET), EO_TYPECHECK(Eina_Bool *, ret)

/**
 * @def elm_obj_panel_toggle
 * @since 1.8
 *
 * @brief Toggle the hidden state of the panel from code
 *
 *
 * @see elm_panel_toggle
 *
 * @ingroup Panel
 */
#define elm_obj_panel_toggle() ELM_OBJ_PANEL_ID(ELM_OBJ_PANEL_SUB_ID_TOGGLE)
