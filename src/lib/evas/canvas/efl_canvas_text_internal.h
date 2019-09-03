#ifndef _EFL_CANVAS_TEXT_INTERNAL_H
#define _EFL_CANVAS_TEXT_INTERNAL_H

#include <Evas.h>

/**
 * @internal
 * @typedef Evas_Object_Textblock_Node_Text
 * A text node.
 */
typedef struct _Evas_Object_Textblock_Node_Text   Evas_Object_Textblock_Node_Text;

struct _Efl2_Text_Cursor_Handle
{
   Evas_Object                     *obj;
   size_t                           pos;
   Evas_Object_Textblock_Node_Text *node;
   short                            ref;
   Eina_Bool                        changed : 1;
};

// FIXME: There are too many things here, trim the unused parts
// Rename accordign to the property names
// Is this too memory wasteful? If so, can compact it better to only have the fields we change here.
struct _Efl2_Text_Attribute_Format
{
   double               halign;  /**< Horizontal alignment value. */
   double               valign;  /**< Vertical alignment value. */
   struct {
      Eina_Stringshare *family;
      Evas_Font_Size    size;  /**< Size of the font. */
      Eina_Stringshare *source;
      Eina_Stringshare *fallbacks;
      Eina_Stringshare *lang;
      Efl2_Text_Font_Weight weight;
      Efl2_Text_Font_Slant slant;
      Efl2_Text_Font_Width width;
      Efl2_Text_Font_Bitmap_Scalable bitmap_scalable;  /**< Scalable for bitmap font. */
   } font;
   struct {
      struct {
         unsigned char  r, g, b, a;
      } foreground, underline, underline2, underline_dash, outline, shadow, glow, glow2, background,
        strikethrough;
   } color;
   // FIXME: Not using this one! Should it be used?
   struct {
      int               l, r;
   } margin;  /**< Left and right margin width. */
   int                  tab_width;  /**< Value of the size of the tab character. */
   int                  line_height;  /**< Value of the size of the line of the text. */
   int                  line_spacing;  /**< Value to set the line gap in text. */
   int                  underline_dash_width;  /**< Valule to set the width of the underline dash. */
   int                  underline_dash_gap;  /**< Value to set the gap of the underline dash. */
   double               underline_height;  /**< Value to set the height of the single underline. */
   double               line_height_factor;  /**< Value to set the size of line of text. */
   double               line_spacing_factor;  /**< Value for setting line gap. */
   // FIXME: Not using this one! Should it be used?
   double               linefill;  /**< The value must be a percentage. */
   Efl2_Text_Style_Underline underline : 3; // FIXME: Is the size correct?
   Efl2_Text_Style_Shadow_Direction shadow_direction : 8; // FIXME: is the size correct?
   // FIXME: Not using this one! Should it be used?
   unsigned char        style;  /**< Value from Evas_Text_Style_Type enum. */
   Eina_Bool            strikethrough : 1;  /**< EINA_TRUE if text should be stricked off, else EINA_FALSE */
   Eina_Bool            background : 1;  /**< EINA_TRUE if enable background color, else EINA_FALSE */
};

typedef struct _Efl2_Text_Attribute_Format Efl2_Text_Attribute_Format;

struct _Efl2_Text_Attribute_Handle
{
   EINA_INLIST;
   Evas_Object_Textblock_Node_Format *start_node, *end_node;
   Efl2_Text_Attribute_Format fmt;
   int ref;  /**< Value of the ref. */
};

#endif
