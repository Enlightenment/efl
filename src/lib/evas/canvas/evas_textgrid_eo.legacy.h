#ifndef _EVAS_TEXTGRID_EO_LEGACY_H_
#define _EVAS_TEXTGRID_EO_LEGACY_H_

#ifndef _EVAS_TEXTGRID_EO_CLASS_TYPE
#define _EVAS_TEXTGRID_EO_CLASS_TYPE

typedef Eo Evas_Textgrid;

#endif

#ifndef _EVAS_TEXTGRID_EO_TYPES
#define _EVAS_TEXTGRID_EO_TYPES

/** The palette to use for the foreground and background colors.
 *
 * @since 1.7
 *
 * @ingroup Evas_Textgrid
 */
typedef enum
{
  EVAS_TEXTGRID_PALETTE_NONE = 0, /**< No palette is used */
  EVAS_TEXTGRID_PALETTE_STANDARD, /**< Standard palette (around 16 colors) */
  EVAS_TEXTGRID_PALETTE_EXTENDED, /**< Extended palette (at max 256 colors) */
  EVAS_TEXTGRID_PALETTE_LAST /**< Sentinel value to indicate last enum field
                              * during iteration */
} Evas_Textgrid_Palette;

/** The style to give to each character of the grid.
 *
 * @since 1.7
 *
 * @ingroup Evas_Textgrid
 */
typedef enum
{
  EVAS_TEXTGRID_FONT_STYLE_NORMAL = 1 /* 1 >> 0 */, /**< Normal style */
  EVAS_TEXTGRID_FONT_STYLE_BOLD = 2 /* 1 >> 1 */, /**< Bold style */
  EVAS_TEXTGRID_FONT_STYLE_ITALIC = 4 /* 1 >> 2 */ /**< Oblique style */
} Evas_Textgrid_Font_Style;

/** Evas textgrid cell data structure
 *
 * @ingroup Evas_Textgrid
 */
typedef struct _Evas_Textgrid_Cell Evas_Textgrid_Cell;


#endif

/**
 * @brief This function is not implemented yet, do not use.
 *
 * @param[in] obj The object.
 * @param[in] styles EVAS_TEXTGRID_FONT_STYLE_NORMAL
 *
 * @ingroup Evas_Object_Textgrid_Group
 */
EAPI void evas_object_textgrid_supported_font_styles_set(Evas_Textgrid *obj, Evas_Textgrid_Font_Style styles);

/**
 * @brief This function is not implemented yet, do not use.
 *
 * @param[in] obj The object.
 *
 * @return EVAS_TEXTGRID_FONT_STYLE_NORMAL
 *
 * @ingroup Evas_Object_Textgrid_Group
 */
EAPI Evas_Textgrid_Font_Style evas_object_textgrid_supported_font_styles_get(const Evas_Textgrid *obj);

/**
 * @brief Set the size of the textgrid object.
 *
 * This function sets the number of lines @c h and the number of columns @c w
 * for the textgrid object @c obj. If @c w or @c h are less than or equal to 0,
 * this function does nothing.
 *
 * @param[in] obj The object.
 * @param[in] w The number of columns (width in cells) of the grid.
 * @param[in] h The number of rows (height in cells) of the grid.
 *
 * @since 1.7
 *
 * @ingroup Evas_Object_Textgrid_Group
 */
EAPI void evas_object_textgrid_size_set(Evas_Textgrid *obj, int w, int h);

/**
 * @brief Get the size of the textgrid object.
 *
 * This function retrieves the number of lines in the buffer @c h and the
 * number of columns in the buffer @c w of the textgrid object @c obj. @c w or
 * @c h can be @c null. On error, they are set to 0.
 *
 * @param[in] obj The object.
 * @param[out] w The number of columns (width in cells) of the grid.
 * @param[out] h The number of rows (height in cells) of the grid.
 *
 * @since 1.7
 *
 * @ingroup Evas_Object_Textgrid_Group
 */
EAPI void evas_object_textgrid_size_get(const Evas_Textgrid *obj, int *w, int *h);

/**
 * @brief Retrieve the size of a cell of the given textgrid object in pixels.
 *
 * This functions retrieves the width and height, in pixels, of a cell of the
 * textgrid object @c obj and stores them respectively in the buffers @c width
 * and @c height. Their values depend on the monospace font and style used for
 * the textgrid object. @c width and @c height can be @c null. On error, they
 * are set to 0.
 *
 * See also @ref Efl.Text_Font.font.set and
 * @ref evas_object_textgrid_supported_font_styles_set
 *
 * @param[in] obj The object.
 * @param[out] width A pointer to the location to store the cell's width in
 * pixels.
 * @param[out] height A pointer to the location to store the cell's height in
 * pixels.
 *
 * @since 1.7
 *
 * @ingroup Evas_Object_Textgrid_Group
 */
EAPI void evas_object_textgrid_cell_size_get(const Evas_Textgrid *obj, int *width, int *height);

/**
 * @brief Indicate for evas that part of a textgrid region (cells) has been
 * updated.
 *
 * This function declares to evas that a region of cells was updated by code
 * and needs refreshing.
 *
 * See also @ref evas_object_textgrid_cellrow_set,
 *  @ref evas_object_textgrid_cellrow_get and
 * @ref evas_object_textgrid_size_set.
 *
 * @param[in] obj The object.
 * @param[in] x The rect region of cells top-left x (column).
 * @param[in] y The rect region of cells top-left y (row).
 * @param[in] w The rect region size in number of cells (columns).
 * @param[in] h The rect region size in number of cells (rows).
 *
 * @since 1.7
 *
 * @ingroup Evas_Object_Textgrid_Group
 */
EAPI void evas_object_textgrid_update_add(Evas_Textgrid *obj, int x, int y, int w, int h);

/**
 * @brief Set the string at the given row of the given textgrid object.
 *
 * This function returns cells to the textgrid taken by
 * evas_object_textgrid_cellrow_get. The row pointer @c row should be the same
 * row pointer returned by evas_object_textgrid_cellrow_get for the same row
 * @c y.
 *
 * See also @ref evas_object_textgrid_cellrow_get,
 * @ref evas_object_textgrid_size_set and @ref evas_object_textgrid_update_add.
 *
 * @param[in] obj The object.
 * @param[in] y The row index of the grid.
 * @param[in] row The string as a sequence of #Evas_Textgrid_Cell.
 *
 * @since 1.7
 *
 * @ingroup Evas_Object_Textgrid_Group
 */
EAPI void evas_object_textgrid_cellrow_set(Evas_Textgrid *obj, int y, const Evas_Textgrid_Cell *row);

/**
 * @brief Get the string at the given row of the given textgrid object.
 *
 * This function returns a pointer to the first cell of the line @c y of the
 * textgrid object @c obj. If @c y is not between 0 and the number of lines of
 * the grid - 1, or on error, this function return @c null.
 *
 * See also @ref evas_object_textgrid_cellrow_set,
 * @ref evas_object_textgrid_size_set and @ref evas_object_textgrid_update_add.
 *
 * @param[in] obj The object.
 * @param[in] y The row index of the grid.
 *
 * @return A pointer to the first cell of the given row.
 *
 * @since 1.7
 *
 * @ingroup Evas_Object_Textgrid_Group
 */
EAPI Evas_Textgrid_Cell *evas_object_textgrid_cellrow_get(const Evas_Textgrid *obj, int y);

/**
 * @brief The set color to the given palette at the given index of the given
 * textgrid object.
 *
 * This function sets the color for the palette of type @c pal at the index
 * @c idx of the textgrid object @c obj. The ARGB components are given by @c r,
 * @c g, @c b and @c a. This color can be used when setting the
 * #Evas_Textgrid_Cell structure. The components must set a pre-multiplied
 * color. If pal is #EVAS_TEXTGRID_PALETTE_NONE or #EVAS_TEXTGRID_PALETTE_LAST,
 * or if @c idx is not between 0 and 255, or on error, this function does
 * nothing. The color components are clamped between 0 and 255. If @c idx is
 * greater than the latest set color, the colors between this last index and
 * @c idx - 1 are set to black (0, 0, 0, 0).
 *
 * See also @ref evas_object_textgrid_palette_get.
 *
 * @param[in] obj The object.
 * @param[in] pal The type of the palette to set the color.
 * @param[in] idx The index of the palette to which the color is stored.
 * @param[in] r The red component of the color.
 * @param[in] g The green component of the color.
 * @param[in] b The blue component of the color.
 * @param[in] a The alpha component of the color.
 *
 * @since 1.7
 *
 * @ingroup Evas_Object_Textgrid_Group
 */
EAPI void evas_object_textgrid_palette_set(Evas_Textgrid *obj, Evas_Textgrid_Palette pal, int idx, int r, int g, int b, int a);

/**
 * @brief The retrieve color to the given palette at the given index of the
 * given textgrid object.
 *
 * This function retrieves the color for the palette of type @c pal at the
 * index @c idx of the textgrid object @c obj. The ARGB components are stored
 * in the buffers @c r, @c g, @c b and @c a. If @c idx is not between 0 and the
 * index of the latest set color, or if @c pal is #EVAS_TEXTGRID_PALETTE_NONE
 * or #EVAS_TEXTGRID_PALETTE_LAST, the values of the components are 0. @c r,
 * @c g, @c b and @c a can be @c null.
 *
 * See also @ref evas_object_textgrid_palette_set.
 *
 * @param[in] obj The object.
 * @param[in] pal The type of the palette to set the color.
 * @param[in] idx The index of the palette to which the color is stored.
 * @param[out] r A pointer to the red component of the color.
 * @param[out] g A pointer to the green component of the color.
 * @param[out] b A pointer to the blue component of the color.
 * @param[out] a A pointer to the alpha component of the color.
 *
 * @since 1.7
 *
 * @ingroup Evas_Object_Textgrid_Group
 */
EAPI void evas_object_textgrid_palette_get(const Evas_Textgrid *obj, Evas_Textgrid_Palette pal, int idx, int *r, int *g, int *b, int *a);

#endif
