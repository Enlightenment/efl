/**
 * @defgroup Panel Panel
 * @ingroup Elementary
 *
 * @image html panel_inheritance_tree.png
 * @image latex panel_inheritance_tree.eps
 *
 * @image html img/widget/panel/preview-00.png
 * @image latex img/widget/panel/preview-00.eps
 *
 * @brief A panel is an animated side-panel that contains a
 * sub-object. It can be expanded or contracted by clicking the
 * button on its edge.
 *
 * Orientations are as follows:
 * @li @c ELM_PANEL_ORIENT_TOP
 * @li @c ELM_PANEL_ORIENT_LEFT
 * @li @c ELM_PANEL_ORIENT_RIGHT
 * @li @c ELM_PANEL_ORIENT_BOTTOM
 *
 * This widget inherits from the @ref Layout one, so that all the
 * functions acting on it also work for panel objects (@since 1.8).
 *
 * Default content parts of the panel widget that you can use are:
 * @li @c "default" - A content of the panel
 *
 * @ref tutorial_panel shows one way to use this widget.
 * @{
 */



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
 */
#define elm_obj_panel_toggle() ELM_OBJ_PANEL_ID(ELM_OBJ_PANEL_SUB_ID_TOGGLE)

typedef enum
{
   ELM_PANEL_ORIENT_TOP, /**< Panel (dis)appears from the top */
   ELM_PANEL_ORIENT_BOTTOM, /**< Panel (dis)appears from the bottom */
   ELM_PANEL_ORIENT_LEFT, /**< Panel (dis)appears from the left */
   ELM_PANEL_ORIENT_RIGHT, /**< Panel (dis)appears from the right */
} Elm_Panel_Orient;

/**
 * @brief Adds a panel object
 *
 * @param parent The parent object
 *
 * @return The panel object, or NULL on failure
 *
 * @ingroup Panel
 */
EAPI Evas_Object                 *elm_panel_add(Evas_Object *parent);

/**
 * @brief Sets the orientation of the panel
 *
 * @param obj The panel object
 * @param orient The panel orientation. Can be one of the following:
 * @li ELM_PANEL_ORIENT_TOP
 * @li ELM_PANEL_ORIENT_LEFT
 * @li ELM_PANEL_ORIENT_RIGHT
 *
 * Sets from where the panel will (dis)appear.
 *
 * @ingroup Panel
 */
EAPI void                         elm_panel_orient_set(Evas_Object *obj, Elm_Panel_Orient orient);

/**
 * @brief Get the orientation of the panel.
 *
 * @param obj The panel object
 * @return The Elm_Panel_Orient, or ELM_PANEL_ORIENT_LEFT on failure.
 *
 * @ingroup Panel
 */
EAPI Elm_Panel_Orient             elm_panel_orient_get(const Evas_Object *obj);

/**
 * @brief Set the state of the panel.
 *
 * @param obj The panel object
 * @param hidden If true, the panel will run the animation to disappear.
 *
 * @ingroup Panel
 */
EAPI void                         elm_panel_hidden_set(Evas_Object *obj, Eina_Bool hidden);

/**
 * @brief Get the state of the panel.
 *
 * @param obj The panel object
 * @return EINA_TRUE if it is hidden state
 *
 * @ingroup Panel
 */
EAPI Eina_Bool                    elm_panel_hidden_get(const Evas_Object *obj);

/**
 * @brief Toggle the hidden state of the panel from code
 *
 * @param obj The panel object
 *
 * @ingroup Panel
 */
EAPI void                         elm_panel_toggle(Evas_Object *obj);

/**
 * @}
 */
