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
