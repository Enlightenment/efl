#include "evas_common_private.h"
#include "evas_private.h"

#include "efl_canvas_text_internal.h"

#define MY_CLASS EFL2_TEXT_ATTRIBUTE_FACTORY

#define COLOR_SET(It, R, G, B, A) do \
{ \
   (It).r = R; \
   (It).g = G; \
   (It).b = B; \
   (It).a = A; \
} while (0)

#define COLOR_GET(It, R, G, B, A) do \
{ \
   *(R) = (It).r; \
   *(G) = (It).g; \
   *(B) = (It).b; \
   *(A) = (It).a; \
} while (0)

struct _Efl2_Text_Attribute_Factory_Data
{
   Efl2_Text_Attribute_Format fmt;
};

typedef struct _Efl2_Text_Attribute_Factory_Data Efl2_Text_Attribute_Factory_Data;

// FIXME: Do we need NULL checks for hnadle everywhere?

EOLIAN static void
_efl2_text_attribute_factory_reset(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   // FIXME: dealloc strings
   memset(&pd->fmt, 0, sizeof(pd->fmt));
}

EOLIAN static void
_efl2_text_attribute_factory_load(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, const Efl2_Text_Attribute_Handle *handle)
{
   // FIXME: dealloc strings (or replace with stringshare)
   memcpy(&pd->fmt, &handle->fmt, sizeof(pd->fmt));
}

EOLIAN static void
_efl2_text_attribute_factory_update(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Efl2_Text_Attribute_Handle *handle)
{
   memcpy(&handle->fmt, &pd->fmt, sizeof(pd->fmt));
}

EOLIAN static Efl2_Text_Attribute_Handle *
_efl2_text_attribute_factory_create(Eo *obj, Efl2_Text_Attribute_Factory_Data *pd)
{
   Efl2_Text_Attribute_Handle *handle = calloc(1, sizeof(*handle));
   _efl2_text_attribute_factory_update(obj, pd, handle);
   return handle;
}

EOLIAN static Efl2_Text_Attribute_Handle *
_efl2_text_attribute_factory_insert(Eo *obj, Efl2_Text_Attribute_Factory_Data *pd, const Efl2_Text_Cursor *cur1, const Efl2_Text_Cursor *cur2)
{
   Efl2_Text_Attribute_Handle *handle = _efl2_text_attribute_factory_create(obj, pd);
   // FIME: insert;
   return handle;
}

EOLIAN static Eina_Iterator *
_efl2_text_attribute_factory_range_attributes_get(const Efl2_Text_Cursor *start, const Efl2_Text_Cursor *end)
{
   // FIXME: implement
   return NULL;
}

EOLIAN static void
_efl2_text_attribute_factory_attribute_range_get(Efl2_Text_Cursor *start, Efl2_Text_Cursor *end)
{
   // FIXME: implement
}

EOLIAN static Efl2_Text_Attribute_Handle *
_efl2_text_attribute_factory_ref(Efl2_Text_Attribute_Handle *handle)
{
   // FIXME: implement
   return handle;
}

EOLIAN static void
_efl2_text_attribute_factory_unref(Efl2_Text_Attribute_Handle *handle)
{
   // FIXME: implement
   (void) handle;
}

EOLIAN static void
_efl2_text_attribute_factory_remove(Efl2_Text_Attribute_Handle *handle)
{
   // FIXME: implement
   (void) handle;
}

EOLIAN static void
_efl2_text_attribute_factory_efl2_text_font_properties_font_family_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, const char *font)
{
   eina_stringshare_replace(&pd->fmt.font.family, font);
}


EOLIAN static const char *
_efl2_text_attribute_factory_efl2_text_font_properties_font_family_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.font.family;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_font_properties_font_size_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Efl2_Font_Size size)
{
   pd->fmt.font.size = size;
}


EOLIAN static Efl2_Font_Size
_efl2_text_attribute_factory_efl2_text_font_properties_font_size_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.font.size;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_font_properties_font_source_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, const char *source)
{
   eina_stringshare_replace(&pd->fmt.font.source, source);
}


EOLIAN static const char *
_efl2_text_attribute_factory_efl2_text_font_properties_font_source_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.font.source;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_font_properties_font_fallbacks_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, const char *fallbacks)
{
   eina_stringshare_replace(&pd->fmt.font.fallbacks, fallbacks);
}


EOLIAN static const char *
_efl2_text_attribute_factory_efl2_text_font_properties_font_fallbacks_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.font.fallbacks;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_font_properties_font_weight_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Efl2_Text_Font_Weight weight)
{
   pd->fmt.font.weight = weight;
}


EOLIAN static Efl2_Text_Font_Weight
_efl2_text_attribute_factory_efl2_text_font_properties_font_weight_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.font.weight;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_font_properties_font_slant_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Efl2_Text_Font_Slant style)
{
   pd->fmt.font.slant = style;
}


EOLIAN static Efl2_Text_Font_Slant
_efl2_text_attribute_factory_efl2_text_font_properties_font_slant_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.font.slant;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_font_properties_font_width_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Efl2_Text_Font_Width width)
{
   pd->fmt.font.width = width;
}


EOLIAN static Efl2_Text_Font_Width
_efl2_text_attribute_factory_efl2_text_font_properties_font_width_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.font.width;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_font_properties_font_lang_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, const char *lang)
{
   eina_stringshare_replace(&pd->fmt.font.lang, lang);
}


EOLIAN static const char *
_efl2_text_attribute_factory_efl2_text_font_properties_font_lang_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.font.lang;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_font_properties_font_bitmap_scalable_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Efl2_Text_Font_Bitmap_Scalable scalable)
{
   pd->fmt.font.bitmap_scalable = scalable;
}


EOLIAN static Efl2_Text_Font_Bitmap_Scalable
_efl2_text_attribute_factory_efl2_text_font_properties_font_bitmap_scalable_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.font.bitmap_scalable;
}

EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_foreground_color_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   COLOR_SET(pd->fmt.color.foreground, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_foreground_color_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
   COLOR_GET(pd->fmt.color.foreground, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_background_type_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Efl2_Text_Style_Background type)
{
   pd->fmt.background = (type == EFL2_TEXT_STYLE_BACKGROUND_ON);
}


EOLIAN static Efl2_Text_Style_Background
_efl2_text_attribute_factory_efl2_text_style_properties_background_type_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   // FIXME: Turn background color and strikethrough color on by default
   return (pd->fmt.background == EFL2_TEXT_STYLE_BACKGROUND_ON);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_background_color_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   COLOR_SET(pd->fmt.color.background, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_background_color_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
   COLOR_GET(pd->fmt.color.background, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_underline_type_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Efl2_Text_Style_Underline type)
{
   pd->fmt.underline = type;
}


EOLIAN static Efl2_Text_Style_Underline
_efl2_text_attribute_factory_efl2_text_style_properties_underline_type_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.underline;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_underline_color_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   COLOR_SET(pd->fmt.color.underline, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_underline_color_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
   COLOR_GET(pd->fmt.color.underline, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_underline_height_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, double height)
{
   pd->fmt.underline_height = height;
}


EOLIAN static double
_efl2_text_attribute_factory_efl2_text_style_properties_underline_height_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.underline_height;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_underline_dashed_gap_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, int gap)
{
   pd->fmt.underline_dash_gap = gap;
}


EOLIAN static int
_efl2_text_attribute_factory_efl2_text_style_properties_underline_dashed_gap_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.underline_dash_gap;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_underline_dashed_color_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   COLOR_SET(pd->fmt.color.underline_dash, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_underline_dashed_color_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
   COLOR_GET(pd->fmt.color.underline_dash, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_underline_dashed_width_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, int width)
{
   pd->fmt.underline_dash_width = width;
}


EOLIAN static int
_efl2_text_attribute_factory_efl2_text_style_properties_underline_dashed_width_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.underline_dash_width;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_underline2_color_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   COLOR_SET(pd->fmt.color.underline2, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_underline2_color_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
   COLOR_GET(pd->fmt.color.underline2, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_strikethrough_type_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Efl2_Text_Style_Strikethrough type)
{
   pd->fmt.strikethrough = (type == EFL2_TEXT_STYLE_STRIKETHROUGH_ON);
}


EOLIAN static Efl2_Text_Style_Strikethrough
_efl2_text_attribute_factory_efl2_text_style_properties_strikethrough_type_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return (pd->fmt.strikethrough == EFL2_TEXT_STYLE_STRIKETHROUGH_ON);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_strikethrough_color_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   COLOR_SET(pd->fmt.color.strikethrough, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_strikethrough_color_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
   COLOR_GET(pd->fmt.color.strikethrough, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_shadow_direction_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, Efl2_Text_Style_Shadow_Direction type)
{
   pd->fmt.shadow_direction = type;
}


EOLIAN static Efl2_Text_Style_Shadow_Direction
_efl2_text_attribute_factory_efl2_text_style_properties_shadow_direction_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.shadow_direction;
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_shadow_color_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   COLOR_SET(pd->fmt.color.shadow, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_shadow_color_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
   COLOR_GET(pd->fmt.color.shadow, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_glow_color_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   COLOR_SET(pd->fmt.color.glow, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_glow_color_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
   COLOR_GET(pd->fmt.color.glow, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_glow2_color_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
   COLOR_SET(pd->fmt.color.glow2, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_glow2_color_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a)
{
   COLOR_GET(pd->fmt.color.glow2, r, g, b, a);
}


EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_line_spacing_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, int value)
{
   pd->fmt.line_spacing = value;
}


EOLIAN static int
_efl2_text_attribute_factory_efl2_text_style_properties_line_spacing_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.line_spacing;
}

EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_line_spacing_factor_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, double value)
{
   pd->fmt.line_spacing_factor = value;
}


EOLIAN static double
_efl2_text_attribute_factory_efl2_text_style_properties_line_spacing_factor_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.line_spacing_factor;
}

EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_line_height_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, int value)
{
   pd->fmt.line_height = value;
}


EOLIAN static int
_efl2_text_attribute_factory_efl2_text_style_properties_line_height_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.line_height;
}

EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_line_height_factor_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, double value)
{
   pd->fmt.line_height_factor = value;
}


EOLIAN static double
_efl2_text_attribute_factory_efl2_text_style_properties_line_height_factor_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.line_height_factor;
}

EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_tab_width_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, int value)
{
   pd->fmt.tab_width = value;
}


EOLIAN static int
_efl2_text_attribute_factory_efl2_text_style_properties_tab_width_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.tab_width;
}

EOLIAN static void
_efl2_text_attribute_factory_efl2_text_style_properties_href_set(Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd, const char *value)
{
   eina_stringshare_replace(&pd->fmt.href, value);
}


EOLIAN static const char
*_efl2_text_attribute_factory_efl2_text_style_properties_href_get(const Eo *obj EINA_UNUSED, Efl2_Text_Attribute_Factory_Data *pd)
{
   return pd->fmt.href;
}

#include "canvas/efl2_text_attribute_factory.eo.c"
