/**
 * @addtogroup Elm_Colorselector
 *
 * @{
 */

typedef struct _Elm_Color_RGBA Elm_Color_RGBA;
struct _Elm_Color_RGBA
{
   unsigned int r;
   unsigned int g;
   unsigned int b;
   unsigned int a;
};

typedef struct _Elm_Custom_Palette Elm_Custom_Palette;
struct _Elm_Custom_Palette
{
   const char *palette_name;
   Eina_List  *color_list;
};

/**
 * @brief Get Palette item's color.
 *
 * @param[in] it The color palette item.
 * @param[out] r integer pointer for r-value of color
 * @param[out] g integer pointer for g-value of color
 * @param[out] b integer pointer for b-value of color
 * @param[out] a integer pointer for a-value of color
 *
 * @ingroup Elm_Colorselector
 */
EAPI void elm_colorselector_palette_item_color_get(const Elm_Object_Item *it, int *r, int *g, int *b, int *a);

/**
 * @brief Set palette item's color.
 *
 * @param[in] it The color palette item.
 * @param[in] r r-value of color
 * @param[in] g g-value of color
 * @param[in] b b-value of color
 * @param[in] a a-value of color
 *
 * @ingroup Elm_Colorselector
 */
EAPI void elm_colorselector_palette_item_color_set(Elm_Object_Item *it, int r, int g, int b, int a);

/**
 * @brief Get the selected state of color palette item.
 *
 * @param[in] it The colorpalette item
 * @return @c EINA_TRUE if the item is selected, @c EINA_FALSE otherwise.
 *
 * @ingroup Elm_Colorselector
 */
EAPI Eina_Bool elm_colorselector_palette_item_selected_get(const Elm_Object_Item *it);

/**
 * @brief Set the selected state of color palette item.
 *
 * @param[in] it The colorpalette item
 * @param[in] selected The selected state
 *
 * @ingroup Elm_Colorselector
 */
EAPI void elm_colorselector_palette_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);

/**
 * @}
 */
