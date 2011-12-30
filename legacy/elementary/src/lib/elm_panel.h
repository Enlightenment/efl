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
 * Default contents parts of the panel widget that you can use for are:
 * @li "default" - A content of the panel
 *
 * @ref tutorial_panel shows one way to use this widget.
 * @{
 */
typedef enum _Elm_Panel_Orient
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
EAPI Evas_Object *
                                  elm_panel_add(Evas_Object *parent)
EINA_ARG_NONNULL(1);

/**
 * @brief Sets the orientation of the panel
 *
 * @param parent The parent object
 * @param orient The panel orientation. Can be one of the following:
 * @li ELM_PANEL_ORIENT_TOP
 * @li ELM_PANEL_ORIENT_LEFT
 * @li ELM_PANEL_ORIENT_RIGHT
 *
 * Sets from where the panel will (dis)appear.
 */
EAPI void                         elm_panel_orient_set(Evas_Object *obj, Elm_Panel_Orient orient) EINA_ARG_NONNULL(1);

/**
 * @brief Get the orientation of the panel.
 *
 * @param obj The panel object
 * @return The Elm_Panel_Orient, or ELM_PANEL_ORIENT_LEFT on failure.
 */
EAPI Elm_Panel_Orient             elm_panel_orient_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Set the content of the panel.
 *
 * @param obj The panel object
 * @param content The panel content
 *
 * Once the content object is set, a previously set one will be deleted.
 * If you want to keep that old content object, use the
 * elm_panel_content_unset() function.
 *
 * @deprecated use elm_object_content_set() instead
 *
 */
EINA_DEPRECATED EAPI void         elm_panel_content_set(Evas_Object *obj, Evas_Object *content) EINA_ARG_NONNULL(1);

/**
 * @brief Get the content of the panel.
 *
 * @param obj The panel object
 * @return The content that is being used
 *
 * Return the content object which is set for this widget.
 *
 * @see elm_panel_content_set()
 *
 * @deprecated use elm_object_content_get() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panel_content_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Unset the content of the panel.
 *
 * @param obj The panel object
 * @return The content that was being used
 *
 * Unparent and return the content object which was set for this widget.
 *
 * @see elm_panel_content_set()
 *
 * @deprecated use elm_object_content_unset() instead
 *
 */
EINA_DEPRECATED EAPI Evas_Object *elm_panel_content_unset(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Set the state of the panel.
 *
 * @param obj The panel object
 * @param hidden If true, the panel will run the animation to contract
 */
EAPI void                         elm_panel_hidden_set(Evas_Object *obj, Eina_Bool hidden) EINA_ARG_NONNULL(1);

/**
 * @brief Get the state of the panel.
 *
 * @param obj The panel object
 * @param hidden If true, the panel is in the "hide" state
 */
EAPI Eina_Bool                    elm_panel_hidden_get(const Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @brief Toggle the hidden state of the panel from code
 *
 * @param obj The panel object
 */
EAPI void                         elm_panel_toggle(Evas_Object *obj) EINA_ARG_NONNULL(1);

/**
 * @}
 */
