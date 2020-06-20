#include "edje_private.h"
#include "edje_part_helper.h"
#include "efl_canvas_layout_part_text.eo.h"
#define MY_CLASS EFL_CANVAS_LAYOUT_PART_TEXT_CLASS

PROXY_IMPLEMENTATION(text, MY_CLASS, EINA_FALSE)
#undef PROXY_IMPLEMENTATION

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_text_set(Eo *obj,
      void *_pd EINA_UNUSED, const char *text)
{
   PROXY_DATA_GET(obj, pd);
   _edje_efl_text_text_set(obj, pd->ed, pd->part, text, EINA_FALSE, EINA_FALSE);
}

EOLIAN static const char *
_efl_canvas_layout_part_text_efl_text_text_get(const Eo *obj,
      void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_efl_text_text_get(obj, pd->ed, pd->part, EINA_FALSE, EINA_FALSE);
}

EOLIAN static const char *
_efl_canvas_layout_part_text_efl_text_markup_markup_get(const Eo *obj,
      void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_efl_text_text_get(obj, pd->ed, pd->part, EINA_FALSE, EINA_TRUE);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_markup_markup_set(Eo *obj,
      void *_pd EINA_UNUSED, const char *text)
{
   PROXY_DATA_GET(obj, pd);
   _edje_efl_text_text_set(obj, pd->ed, pd->part, text, EINA_FALSE, EINA_TRUE);
}

/* More Efl.Text.* API (@since 1.22) */

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_style_text_background_type_set(Eo *obj,
      void *_pd EINA_UNUSED,
      Efl_Text_Style_Background_Type type)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_NONE;
   efl_text_background_type_set(pd->rp->object, type);
}

EOLIAN static Efl_Text_Style_Background_Type
_efl_canvas_layout_part_text_efl_text_style_text_background_type_get(const Eo *obj,
      void *_pd EINA_UNUSED)
{

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT)
      return EFL_TEXT_STYLE_BACKGROUND_TYPE_NONE;

   return efl_text_background_type_get(pd->rp->object);
}

#define TEXT_COLOR_IMPL(x, X) \
EOLIAN static void \
_efl_canvas_layout_part_text_efl_text_style_ ##x ##_color_set(Eo *obj, \
      void *_pd EINA_UNUSED, \
      unsigned char r, unsigned char g, unsigned char b, unsigned char a) \
{ \
   Edje_User_Defined *eud; \
 \
   PROXY_DATA_GET(obj, pd); \
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return; \
 \
   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part); \
 \
   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_COLOR_ ##X; \
   efl_ ##x ##_color_set(pd->rp->object, r, g, b, a); \
} \
\
EOLIAN static void \
_efl_canvas_layout_part_text_efl_text_style_ ##x ##_color_get(const Eo *obj, \
      void *_pd EINA_UNUSED, \
      unsigned char *r, unsigned char *g, unsigned char *b, unsigned char *a) \
{ \
   PROXY_DATA_GET(obj, pd); \
   *r = *g = *b = *a = 0; \
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return; \
   efl_ ##x ##_color_get(pd->rp->object, r, g, b, a); \
}

TEXT_COLOR_IMPL(text_background, BACKING)
TEXT_COLOR_IMPL(text_glow, GLOW)
TEXT_COLOR_IMPL(text_secondary_glow, GLOW2)
TEXT_COLOR_IMPL(text, NORMAL)
TEXT_COLOR_IMPL(text_outline, OUTLINE)
TEXT_COLOR_IMPL(text_shadow, SHADOW)
TEXT_COLOR_IMPL(text_strikethrough, STRIKETHROUGH)
TEXT_COLOR_IMPL(text_underline, UNDERLINE)
TEXT_COLOR_IMPL(text_secondary_underline, UNDERLINE2)
TEXT_COLOR_IMPL(text_underline_dashed, UNDERLINE_DASHED)

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_style_text_effect_type_set(Eo *obj,
      void *_pd EINA_UNUSED,
      Efl_Text_Style_Effect_Type type)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_EFFECT_TYPE;
   efl_text_effect_type_set(pd->rp->object, type);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_format_ellipsis_set(Eo *obj,
      void *_pd EINA_UNUSED, double value)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_ELLIPSIS;
   efl_text_ellipsis_set(pd->rp->object, value);
}

EOLIAN static double
_efl_canvas_layout_part_text_efl_text_format_ellipsis_get(const Eo *obj,
      void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return 0.0;

   return efl_text_ellipsis_get(pd->rp->object);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_font_properties_font_family_set(Eo *obj,
      void *_pd EINA_UNUSED, const char *font)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;


   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_FONT;
   efl_text_font_family_set(pd->rp->object, font);
}

EOLIAN static const char *
_efl_canvas_layout_part_text_efl_text_font_properties_font_family_get(const Eo *obj,
      void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return NULL;

   return efl_text_font_family_get(pd->rp->object);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_font_properties_font_size_set(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Font_Size size)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;


   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_FONT;
   efl_text_font_size_set(pd->rp->object, size);
}

EOLIAN static Efl_Font_Size
_efl_canvas_layout_part_text_efl_text_font_properties_font_size_get(const Eo *obj,
      void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return 0;

   return efl_text_font_size_get(pd->rp->object);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_style_text_shadow_direction_set(Eo *obj,
      void *_pd EINA_UNUSED,
      Efl_Text_Style_Shadow_Direction type)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_SHADOW_DIRECTION;
   efl_text_shadow_direction_set(pd->rp->object, type);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_style_text_strikethrough_type_set(Eo *obj,
      void *_pd EINA_UNUSED,
      Efl_Text_Style_Strikethrough_Type type)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_STRIKETHROUGH_TYPE;
   efl_text_strikethrough_type_set(pd->rp->object, type);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_style_text_underline_type_set(Eo *obj,
      void *_pd EINA_UNUSED,
      Efl_Text_Style_Underline_Type type)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;


   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types = EDJE_PART_TEXT_PROP_UNDERLINE_TYPE;
   efl_text_underline_type_set(pd->rp->object, type);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_style_text_underline_height_set(Eo *obj,
      void *_pd EINA_UNUSED,
      double value)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_UNDERLINE_HEIGHT;
   efl_text_underline_height_set(pd->rp->object, value);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_style_text_underline_dashed_width_set(Eo *obj,
      void *_pd EINA_UNUSED,
      int value)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_UNDERLINE_DASHED_WIDTH;
   efl_text_underline_dashed_width_set(pd->rp->object, value);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_style_text_underline_dashed_gap_set(Eo *obj,
      void *_pd EINA_UNUSED,
      int value)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_UNDERLINE_DASHED_GAP;
   efl_text_underline_dashed_gap_set(pd->rp->object, value);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_format_wrap_set(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Format_Wrap wrap)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_WRAP;
   efl_text_wrap_set(pd->rp->object, wrap);
}

EOLIAN static Efl_Text_Format_Wrap
_efl_canvas_layout_part_text_efl_text_format_wrap_get(const Eo *obj,
      void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);

   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT)
      return EFL_TEXT_FORMAT_WRAP_NONE;

   return efl_text_wrap_get(pd->rp->object);
}

static Edje_Part_Text_Prop *
_prop_new(Eina_List **props, Edje_Part_Text_Prop_Type type)
{
   Edje_Part_Text_Prop *prop;

   prop = malloc(sizeof(*prop));
   prop->type = type;

   *props = eina_list_append(*props, prop);

   return prop;
}

void
_canvas_layout_user_text_collect(Edje *ed, Edje_User_Defined *eud)
{
   Edje_Real_Part *rp;
   Eina_List **props = &eud->u.text_style.props;

   rp = _edje_real_part_recursive_get(&ed, eud->part);
   if (!rp) return;

   if (eud->u.text_style.types == EDJE_PART_TEXT_PROP_NONE) return;

   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_BACKING_TYPE)
     {
        Edje_Part_Text_Prop *prop;

        prop = _prop_new(props, EDJE_PART_TEXT_PROP_BACKING_TYPE);
        prop->val.backing = efl_text_background_type_get(rp->object);
     }
#define STYLE_COLOR_COLLECT(x, X) \
   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_COLOR_ ##X) \
     { \
        Edje_Part_Text_Prop *prop; \
        prop = _prop_new(props, EDJE_PART_TEXT_PROP_COLOR_ ##X); \
        efl_ ##x ##_color_get(rp->object, \
              &prop->val.color.r, &prop->val.color.g, \
              &prop->val.color.b, &prop->val.color.a); \
     } \

      STYLE_COLOR_COLLECT(text_background, BACKING)
      STYLE_COLOR_COLLECT(text_glow, GLOW)
      STYLE_COLOR_COLLECT(text_secondary_glow, GLOW2)
      STYLE_COLOR_COLLECT(text, NORMAL)
      STYLE_COLOR_COLLECT(text_outline, OUTLINE)
      STYLE_COLOR_COLLECT(text_shadow, SHADOW)
      STYLE_COLOR_COLLECT(text_strikethrough, STRIKETHROUGH)
      STYLE_COLOR_COLLECT(text_underline, UNDERLINE)
      STYLE_COLOR_COLLECT(text_secondary_underline, UNDERLINE2)
      STYLE_COLOR_COLLECT(text_underline_dashed, UNDERLINE_DASHED)
#undef STYLE_COLOR_COLLECT

      if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_EFFECT_TYPE)
        {
           Edje_Part_Text_Prop *prop;

           prop = _prop_new(props, EDJE_PART_TEXT_PROP_EFFECT_TYPE);
           prop->val.effect = efl_text_effect_type_get(rp->object);
        }

   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_ELLIPSIS)
     {
        Edje_Part_Text_Prop *prop;

        prop = _prop_new(props, EDJE_PART_TEXT_PROP_ELLIPSIS);
        prop->val.nd = efl_text_ellipsis_get(rp->object);
     }

   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_FONT)
     {
        Edje_Part_Text_Prop *prop;

        prop = _prop_new(props, EDJE_PART_TEXT_PROP_FONT);
        prop->val.font.font = efl_text_font_family_get(rp->object);
        prop->val.font.size = efl_text_font_size_get(rp->object);
     }

   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_SHADOW_DIRECTION)
     {
        Edje_Part_Text_Prop *prop;

        prop = _prop_new(props, EDJE_PART_TEXT_PROP_SHADOW_DIRECTION);
        prop->val.shadow = efl_text_shadow_direction_get(rp->object);
     }

   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_STRIKETHROUGH_TYPE)
     {
        Edje_Part_Text_Prop *prop;

        prop = _prop_new(props, EDJE_PART_TEXT_PROP_STRIKETHROUGH_TYPE);
        prop->val.strikethrough_type = efl_text_strikethrough_type_get(rp->object);
     }

   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_UNDERLINE_DASHED_GAP)
     {
        Edje_Part_Text_Prop *prop;

        prop = _prop_new(props, EDJE_PART_TEXT_PROP_UNDERLINE_DASHED_GAP);
        prop->val.ni = efl_text_underline_dashed_gap_get(rp->object);
     }

   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_UNDERLINE_DASHED_WIDTH)
     {
        Edje_Part_Text_Prop *prop;

        prop = _prop_new(props, EDJE_PART_TEXT_PROP_UNDERLINE_DASHED_WIDTH);
        prop->val.ni = efl_text_underline_dashed_width_get(rp->object);
     }

   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_UNDERLINE_TYPE)
     {
        Edje_Part_Text_Prop *prop;

        prop = _prop_new(props, EDJE_PART_TEXT_PROP_UNDERLINE_TYPE);
        prop->val.ni = efl_text_underline_type_get(rp->object);
     }

   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_UNDERLINE_HEIGHT)
     {
        Edje_Part_Text_Prop *prop;

        prop = _prop_new(props, EDJE_PART_TEXT_PROP_UNDERLINE_HEIGHT);
        prop->val.ni = efl_text_underline_height_get(rp->object);
     }

   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_WRAP)
     {
        Edje_Part_Text_Prop *prop;

        prop = _prop_new(props, EDJE_PART_TEXT_PROP_WRAP);
        prop->val.wrap = efl_text_wrap_get(rp->object);

     }
}

void
_canvas_layout_user_text_apply(Edje_User_Defined *eud, Eo *obj,
      Edje_Part_Text_Prop *prop)
{
   switch (prop->type)
     {

      case EDJE_PART_TEXT_PROP_BACKING_TYPE:
        efl_text_background_type_set(
              efl_part(obj,
                 eud->part),
              prop->val.backing);
        break;

#define STYLE_COLOR_CASE(x, X) \
      case EDJE_PART_TEXT_PROP_COLOR_##X : \
        efl_##x ##_color_set(efl_part(obj, \
                 eud->part), \
                 prop->val.color.r, \
                 prop->val.color.g, \
                 prop->val.color.b, \
                 prop->val.color.a); \
        break;

      STYLE_COLOR_CASE(text_background, BACKING)
      STYLE_COLOR_CASE(text_glow, GLOW)
      STYLE_COLOR_CASE(text_secondary_glow, GLOW2)
      STYLE_COLOR_CASE(text, NORMAL)
      STYLE_COLOR_CASE(text_outline, OUTLINE)
      STYLE_COLOR_CASE(text_shadow, SHADOW)
      STYLE_COLOR_CASE(text_strikethrough, STRIKETHROUGH)
      STYLE_COLOR_CASE(text_underline, UNDERLINE)
      STYLE_COLOR_CASE(text_secondary_underline, UNDERLINE2)
      STYLE_COLOR_CASE(text_underline_dashed, UNDERLINE_DASHED)
#undef STYLE_COLOR_CASE

      case EDJE_PART_TEXT_PROP_EFFECT_TYPE:
        efl_text_effect_type_set(
              efl_part(obj,
                 eud->part),
              prop->val.effect);
        break;

      case EDJE_PART_TEXT_PROP_ELLIPSIS:
        efl_text_ellipsis_set(efl_part(obj,
                 eud->part),
              prop->val.nd);
        break;

      case EDJE_PART_TEXT_PROP_FONT:
        efl_text_font_family_set(efl_part(obj,
                 eud->part),
              prop->val.font.font);
        efl_text_font_size_set(efl_part(obj,
                 eud->part),
              prop->val.font.size);
        break;

      case EDJE_PART_TEXT_PROP_SHADOW_DIRECTION:
        efl_text_shadow_direction_set(
              efl_part(obj,
                 eud->part),
              prop->val.shadow);
        break;

      case EDJE_PART_TEXT_PROP_STRIKETHROUGH_TYPE:
        efl_text_strikethrough_type_set(
              efl_part(obj,
                 eud->part),
              prop->val.strikethrough_type);
        break;

      case EDJE_PART_TEXT_PROP_UNDERLINE_DASHED_WIDTH:
        efl_text_underline_dashed_width_set(
              efl_part(obj,
                 eud->part),
              prop->val.ni);
        break;

      case EDJE_PART_TEXT_PROP_UNDERLINE_DASHED_GAP:
        efl_text_underline_dashed_gap_set(
              efl_part(obj,
                 eud->part),
              prop->val.ni);
        break;

      case EDJE_PART_TEXT_PROP_UNDERLINE_TYPE:
        efl_text_underline_type_set(
              efl_part(obj,
                 eud->part),
              prop->val.underline_type);
        break;

      case EDJE_PART_TEXT_PROP_UNDERLINE_HEIGHT:
        efl_text_underline_height_set(
              efl_part(obj,
                 eud->part),
              prop->val.nd);
        break;

      case EDJE_PART_TEXT_PROP_WRAP:
        efl_text_wrap_set(efl_part(obj,
                 eud->part),
              prop->val.wrap);
        break;

      default:
        break;
     }
}


EOLIAN static void
_efl_canvas_layout_part_text_text_expand_set(Eo *obj,
      void *_pd EINA_UNUSED,
      Efl_Canvas_Layout_Part_Text_Expand type)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   eud = _edje_user_text_expand_definition_fetch(pd->ed, pd->part);
   eud->u.text_expand.expand = type;
   pd->rp->typedata.text->expand = type;

}

EOLIAN static Efl_Canvas_Layout_Part_Text_Expand
_efl_canvas_layout_part_text_text_expand_get(const Eo *obj,
      void *_pd EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   return pd->rp->typedata.text->expand;
}

#include "efl_canvas_layout_part_text.eo.c"

