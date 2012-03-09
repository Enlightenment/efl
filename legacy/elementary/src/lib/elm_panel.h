/**
 * @defgroup Panel Panel
 *
 * @image html img/widget/panel/preview-00.png
 * @image latex img/widget/panel/preview-00.eps
 *
 * @brief A panel is a type of animated container that contains subobjects.
 * It can be expanded or contracted by clicking the button on it's edge.
 *
 * Orientations are as follows:
 * @li ELM_PANEL_ORIENT_TOP
 * @li ELM_PANEL_ORIENT_LEFT
 * @li ELM_PANEL_ORIENT_RIGHT
 *
 * Default content parts of the panel widget that you can use for are:
 * @li "default" - A content of the panel
 *
 * Supported elm_object common APIs.
 * @li elm_object_signal_emit
 * @li elm_object_signal_callback_add
 * @li elm_object_signal_callback_del
 * @li elm_object_part_content_set
 * @li elm_object_part_content_get
 * @li elm_object_part_content_unset
 *
 * @ref tutorial_panel shows one way to use this widget.
 * @{
 */
typedef enum
{
   ELM_PANEL_ORIENT_TOP, /**< Panel (dis)appears from the top */
   ELM_PANEL_ORIENT_BOTTOM, /**< Not implemented */
   ELM_PANEL_ORIENT_LEFT, /**< Panel (dis)appears from the left */
   ELM_PANEL_ORIENT_RIGHT, /**< Panel (dis)appears from the right */
} Elm_Panel_Orient;

/**
 * @brief Adds a panel object
 *
 * @param parent The parent object
 *
 * @return The panel object, or NULL on failure
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
 */
EAPI void                         elm_panel_orient_set(Evas_Object *obj, Elm_Panel_Orient orient);

/**
 * @brief Get the orientation of the panel.
 *
 * @param obj The panel object
 * @return The Elm_Panel_Orient, or ELM_PANEL_ORIENT_LEFT on failure.
 */
EAPI Elm_Panel_Orient             elm_panel_orient_get(const Evas_Object *obj);

/**
 * @brief Set the state of the panel.
 *
 * @param obj The panel object
 * @param hidden If true, the panel will run the animation to disappear.
 */
EAPI void                         elm_panel_hidden_set(Evas_Object *obj, Eina_Bool hidden);

/**
 * @brief Get the state of the panel.
 *
 * @param obj The panel object
 * @return EINA_TRUE if it is hidden state
 */
EAPI Eina_Bool                    elm_panel_hidden_get(const Evas_Object *obj);

/**
 * @brief Toggle the hidden state of the panel from code
 *
 * @param obj The panel object
 */
EAPI void                         elm_panel_toggle(Evas_Object *obj);

/**
 * @}
 */
