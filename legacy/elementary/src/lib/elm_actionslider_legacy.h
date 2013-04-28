/**
 * Add a new actionslider to the parent.
 *
 * @param parent The parent object
 * @return The new actionslider object or NULL if it cannot be created
 *
 * @ingroup Actionslider
 */
EAPI Evas_Object                *elm_actionslider_add(Evas_Object *parent);

/**
 * Set actionslider indicator position.
 *
 * @param obj The actionslider object.
 * @param pos The position of the indicator.
 *
 * @ingroup Actionslider
 */
EAPI void                        elm_actionslider_indicator_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos);

/**
 * Get actionslider indicator position.
 *
 * @param obj The actionslider object.
 * @return The position of the indicator.
 *
 * @ingroup Actionslider
 */
EAPI Elm_Actionslider_Pos        elm_actionslider_indicator_pos_get(const Evas_Object *obj);

/**
 * Set actionslider magnet position. To make multiple positions magnets @c or
 * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT)
 *
 * @param obj The actionslider object.
 * @param pos Bit mask indicating the magnet positions.
 *
 * @ingroup Actionslider
 */
EAPI void                        elm_actionslider_magnet_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos);

/**
 * Get actionslider magnet position.
 *
 * @param obj The actionslider object.
 * @return The positions with magnet property.
 *
 * @ingroup Actionslider
 */
EAPI Elm_Actionslider_Pos        elm_actionslider_magnet_pos_get(const Evas_Object *obj);

/**
 * Set actionslider enabled position. To set multiple positions as enabled @c or
 * them together(e.g.: ELM_ACTIONSLIDER_LEFT | ELM_ACTIONSLIDER_RIGHT).
 *
 * @note All the positions are enabled by default.
 *
 * @param obj The actionslider object.
 * @param pos Bit mask indicating the enabled positions.
 *
 * @ingroup Actionslider
 */
EAPI void                        elm_actionslider_enabled_pos_set(Evas_Object *obj, Elm_Actionslider_Pos pos);

/**
 * Get actionslider enabled position.
 *
 * @param obj The actionslider object.
 * @return The enabled positions.
 *
 * @ingroup Actionslider
 */
EAPI Elm_Actionslider_Pos        elm_actionslider_enabled_pos_get(const Evas_Object *obj);


/**
 * Get actionslider selected label.
 *
 * @param obj The actionslider object
 * @return The selected label
 *
 * @ingroup Actionslider
 */
EAPI const char                 *elm_actionslider_selected_label_get(const Evas_Object *obj);
