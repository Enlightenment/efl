/**
 * @defgroup Colorselector Colorselector
 *
 * @{
 *
 * @image html img/widget/colorselector/preview-00.png
 * @image latex img/widget/colorselector/preview-00.eps
 *
 * @brief Widget for user to select a color.
 *
 * Signals that you can add callbacks for are:
 * "changed" - When the color value changes(event_info is NULL).
 *
 * See @ref tutorial_colorselector.
 */

typedef enum
{
   ELM_COLORSELECTOR_PALETTE = 0,
   ELM_COLORSELECTOR_COMPONENTS,
   ELM_COLORSELECTOR_BOTH
} Elm_Colorselector_Mode;

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
 * Set a color for the colorselector
 *
 * @param obj   Colorselector object
 * @param r     r-value of color
 * @param g     g-value of color
 * @param b     b-value of color
 * @param a     a-value of color
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_color_set(Evas_Object *obj, int r, int g, int b, int a);

/**
 * Get a color from the colorselector
 *
 * @param obj   Colorselector object
 * @param r     integer pointer for r-value of color
 * @param g     integer pointer for g-value of color
 * @param b     integer pointer for b-value of color
 * @param a     integer pointer for a-value of color
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a);

/**
 * Set a Colorselector mode.
 * Colorselector 
 *
 * @param obj Colorselector object
 * @param mode 
 * @param g G color value to be returned
 * @param b B color value to be returned
 * @param a A color value to be returned
 * 
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_mode_set(Evas_Object *obj, Elm_Colorselector_Mode mode);

/**
 * Get a Colorselector mode.
 *
 * @param item The color palette item.
 * @param r R color value to be returned
 * @param g G color value to be returned
 * @param b B color value to be returned
 * @param a A color value to be returned
 * 
 * @ingroup Colorselector
 */
EAPI Elm_Colorselector_Mode elm_colorselector_mode_get(const Evas_Object *obj);

/**
 * Get a palette item's color.
 *
 * @param item The color palette item.
 * @param r integer pointer for r-value of color
 * @param g integer pointer for g-value of color
 * @param b integer pointer for b-value of color
 * @param a integer pointer for a-value of color
 * 
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_palette_item_color_get(const Elm_Object_Item *it, int *r, int *g, int *b, int *a);

/**
 * Set the palette item's color.
 *
 * @param item The color palette item.
 * @param r r-value of color
 * @param g g-value of color
 * @param b b-value of color
 * @param a a-value of color
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_palette_item_color_set(Elm_Object_Item *it, int r, int g, int b, int a);

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
 * @note This API will be available when ELM_COLORSELECTOR_PALETTE or
 * ELM_COLORSELECTOR_BOTH mode is set.
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_palette_clear(Evas_Object *obj);

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
EAPI void                    elm_colorselector_palette_name_set(Evas_Object *obj, const char *palette_name);

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
EAPI const char             *elm_colorselector_palette_name_get(const Evas_Object *obj);

/**
 * @}
 */
