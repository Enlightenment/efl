/**
 * @brief Add a new colorselector to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Colorselector
 */
EAPI Evas_Object *elm_colorselector_add(Evas_Object *parent);

/**
 * Set color to colorselector
 *
 * @param obj Colorselector object
 * @param r r-value of color
 * @param g g-value of color
 * @param b b-value of color
 * @param a a-value of color
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_color_set(Evas_Object *obj, int r, int g, int b, int a);

/**
 * Get current color from colorselector
 *
 * @param obj Colorselector object
 * @param r integer pointer for r-value of color
 * @param g integer pointer for g-value of color
 * @param b integer pointer for b-value of color
 * @param a integer pointer for a-value of color
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a);

/**
 * Set Colorselector's mode.
 *
 * @param obj Colorselector object
 * @param mode Elm_Colorselector_Mode
 *
 * Colorselector supports three modes palette only, selector only and both.
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_mode_set(Evas_Object *obj, Elm_Colorselector_Mode mode);

/**
 * Get Colorselector's mode.
 *
 * @param obj Colorselector object
 * @return mode The current mode of colorselector
 *
 * @ingroup Colorselector
 */
EAPI Elm_Colorselector_Mode elm_colorselector_mode_get(const Evas_Object *obj);

/**
 * Add a new color item to palette.
 *
 * @param obj The Colorselector object
 * @param r r-value of color
 * @param g g-value of color
 * @param b b-value of color
 * @param a a-value of color
 * @return A new color palette Item.
 *
 * @ingroup Colorselector
 */
EAPI Elm_Object_Item *elm_colorselector_palette_color_add(Evas_Object *obj, int r, int g, int b, int a);

/**
 * Clear the palette items.
 *
 * @param obj The Colorselector object
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_palette_clear(Evas_Object *obj);

/**
 * Get list of palette items.
 *
 * @param obj The Colorselector object
 * @return The list of color palette items.
 *
 * Note That palette item list is internally managed by colorselector widget and
 * it should not be freed/modified by application.
 *
 * @since 1.9
 *
 * @ingroup Colorselector
 */
EAPI const Eina_List *elm_colorselector_palette_items_get(const Evas_Object *obj);

/**
 * Get the selected item in colorselector palette.
 *
 * @param obj The Colorselector object
 * @return The selected item, or NULL if none is selected.
 *
 * @since 1.9
 * @ingroup Colorselector
 */
EAPI Elm_Object_Item *elm_colorselector_palette_selected_item_get(const Evas_Object *obj);

/**
 * Set current palette's name
 *
 * @param obj The Colorselector object
 * @param palette_name Name of palette
 *
 * When colorpalette name is set, colors will be loaded from and saved to config
 * using the set name. If no name is set then colors will be loaded from or
 * saved to "default" config.
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_palette_name_set(Evas_Object *obj, const char *palette_name);

/**
 * Get current palette's name
 *
 * @param obj The Colorselector object
 * @return Name of palette
 *
 * Returns the currently set palette name using which colors will be
 * saved/loaded in to config.
 *
 * @ingroup Colorselector
 */
EAPI const char *elm_colorselector_palette_name_get(const Evas_Object *obj);
