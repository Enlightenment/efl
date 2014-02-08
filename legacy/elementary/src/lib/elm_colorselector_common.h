typedef struct _Elm_Color_RGBA
{
   unsigned int r;
   unsigned int g;
   unsigned int b;
   unsigned int a;
} Elm_Color_RGBA;

typedef struct _Elm_Custom_Palette
{
   const char *palette_name;
   Eina_List  *color_list;
} Elm_Custom_Palette;

/**
 * @enum Elm_Colorselector_Mode
 * @typedef Elm_Colorselector_Mode
 *
 * Different modes supported by Colorselector
 *
 * @see elm_colorselector_mode_set()
 * @see elm_colorselector_mode_get()
 */
typedef enum
{
   ELM_COLORSELECTOR_PALETTE = 0, /**< only color palette is displayed */
   ELM_COLORSELECTOR_COMPONENTS, /**< only color selector is displayed */
   ELM_COLORSELECTOR_BOTH, /**< Both Palette and selector is displayed, default */
   ELM_COLORSELECTOR_PICKER, /**< only color picker is displayed */
   ELM_COLORSELECTOR_ALL /**< all possible color selector is displayed */
} Elm_Colorselector_Mode;

/**
 * Get Palette item's color.
 *
 * @param it The color palette item.
 * @param r integer pointer for r-value of color
 * @param g integer pointer for g-value of color
 * @param b integer pointer for b-value of color
 * @param a integer pointer for a-value of color
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_palette_item_color_get(const Elm_Object_Item *it, int *r, int *g, int *b, int *a);

/**
 * Set palette item's color.
 *
 * @param it The color palette item.
 * @param r r-value of color
 * @param g g-value of color
 * @param b b-value of color
 * @param a a-value of color
 *
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_palette_item_color_set(Elm_Object_Item *it, int r, int g, int b, int a);

/**
 * Get the selected state of color palette item.
 *
 * @param it The Colorpalette item
 * @return EINA_TRUE if the item is selected, EINA_FALSE otherwise.
 *
 * @since 1.9
 * @ingroup Colorselector
 */
EAPI Eina_Bool elm_colorselector_palette_item_selected_get(const Elm_Object_Item *it);

/**
 * Set the selected state of color palette item
 *
 * @param it The Colorpalette item
 * @param selected if it's EINA_TRUE, select the item otherwise, unselect the item
 *
 * @since 1.9
 * @ingroup Colorselector
 */
EAPI void elm_colorselector_palette_item_selected_set(Elm_Object_Item *it, Eina_Bool selected);
