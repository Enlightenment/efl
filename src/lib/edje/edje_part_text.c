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


EOLIAN static Efl_Text_Cursor_Cursor *
_efl_canvas_layout_part_text_efl_text_cursor_text_cursor_get(const Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Get_Type get_type)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_text_cursor_get(pd->rp, (int) get_type);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_paragraph_first(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_begin(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_paragraph_last(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_end(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_position_set(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur, int pos)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_pos_set(pd->rp, cur, pos);
}

EOLIAN static int
_efl_canvas_layout_part_text_efl_text_cursor_cursor_position_get(const Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   return _edje_text_cursor_pos_get(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_coord_set(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur,
      int x, int y)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_coord_set(pd->rp, cur, x, y);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_line_char_first(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_line_begin(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_line_char_last(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_line_end(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_char_prev(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_prev(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_char_next(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_next(pd->rp, cur);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_line_jump_by(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur, int by)
{
   PROXY_DATA_GET(obj, pd);
   if (by == 1)
     {
        _edje_text_cursor_down(pd->rp, cur);
     }
   else if (by == -1)
     {
        _edje_text_cursor_up(pd->rp, cur);
     }
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_cursor_cursor_copy(Eo *obj,
      void *_pd EINA_UNUSED,
      Efl_Text_Cursor_Cursor *dst,
      const Efl_Text_Cursor_Cursor *cur)
{
   PROXY_DATA_GET(obj, pd);
   _edje_text_cursor_copy(pd->rp, (Efl_Text_Cursor_Cursor *) cur, dst);
}

EOLIAN static Eina_Unicode
_efl_canvas_layout_part_text_efl_text_cursor_cursor_content_get(const Eo *obj,
      void *_pd EINA_UNUSED,
      const Efl_Text_Cursor_Cursor *cur)
{
   Eina_Unicode *ustr;
   Eina_Unicode uc = { 0 };
   char *c;
   int len;

   PROXY_DATA_GET(obj, pd);
   c = _edje_text_cursor_content_get(pd->rp, (Efl_Text_Cursor_Cursor *) cur);
   ustr = eina_unicode_utf8_to_unicode(c, &len);
   free(c);
   if (ustr)
     {
        uc = *ustr;
        free(ustr);
     }
   return uc;
}

EOLIAN static Eina_Bool
_efl_canvas_layout_part_text_efl_text_cursor_cursor_geometry_get(const Eo *obj,
      void *_pd EINA_UNUSED,
      const Efl_Text_Cursor_Cursor *cur EINA_UNUSED,
      Efl_Text_Cursor_Type ctype EINA_UNUSED,
      Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch,
      Evas_Coord *cx2 EINA_UNUSED, Evas_Coord *cy2 EINA_UNUSED,
      Evas_Coord *cw2 EINA_UNUSED, Evas_Coord *ch2 EINA_UNUSED)
{
   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_geometry_get(pd->rp, cx, cy, cw, ch, NULL);
        if (cx) *cx -= pd->ed->x;
        if (cy) *cy -= pd->ed->y;
     }
   return EINA_FALSE;
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_markup_interactive_cursor_markup_insert(Eo *obj,
      void *_pd EINA_UNUSED, Efl_Text_Cursor_Cursor *cur EINA_UNUSED,
      const char *text)
{
   PROXY_DATA_GET(obj, pd);
   // Ignoring cursor type and always inserting with main cursor
   _edje_object_part_text_insert(pd->ed, pd->rp, text);
   // FIXME: entry should report the length of inserted text (after filtering)
}

/* More Efl.Text.* API (@since 1.22) */

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_style_backing_type_set(Eo *obj,
      void *_pd EINA_UNUSED,
      Efl_Text_Style_Backing_Type type)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_NONE;
   efl_text_backing_type_set(pd->rp->object, type);
}

EOLIAN static Efl_Text_Style_Backing_Type
_efl_canvas_layout_part_text_efl_text_style_backing_type_get(const Eo *obj,
      void *_pd EINA_UNUSED)
{

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT)
      return EFL_TEXT_STYLE_BACKING_TYPE_DISABLED;

   return efl_text_backing_type_get(pd->rp->object);
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
   efl_text_ ##x ##_color_set(pd->rp->object, r, g, b, a); \
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
   efl_text_ ##x ##_color_get(pd->rp->object, r, g, b, a); \
}

TEXT_COLOR_IMPL(backing, BACKING)
TEXT_COLOR_IMPL(glow, GLOW)
TEXT_COLOR_IMPL(glow2, GLOW2)
TEXT_COLOR_IMPL(normal, NORMAL)
TEXT_COLOR_IMPL(outline, OUTLINE)
TEXT_COLOR_IMPL(shadow, SHADOW)
TEXT_COLOR_IMPL(strikethrough, STRIKETHROUGH)
TEXT_COLOR_IMPL(underline, UNDERLINE)
TEXT_COLOR_IMPL(underline2, UNDERLINE2)
TEXT_COLOR_IMPL(underline_dashed, UNDERLINE_DASHED)

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_style_effect_type_set(Eo *obj,
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
_efl_canvas_layout_part_text_efl_text_font_font_set(Eo *obj,
      void *_pd EINA_UNUSED, const char *font, Efl_Font_Size size)
{
   Edje_User_Defined *eud;

   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;


   eud = _edje_user_text_style_definition_fetch(pd->ed, pd->part);

   eud->u.text_style.types |= EDJE_PART_TEXT_PROP_FONT;
   efl_text_font_set(pd->rp->object, font, size);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_font_font_get(const Eo *obj,
      void *_pd EINA_UNUSED, const char **font, Efl_Font_Size *size)
{
   PROXY_DATA_GET(obj, pd);
   if (pd->rp->part->type == EDJE_PART_TYPE_TEXT) return;

   efl_text_font_get(pd->rp->object, font, size);
}

EOLIAN static void
_efl_canvas_layout_part_text_efl_text_style_shadow_direction_set(Eo *obj,
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
_efl_canvas_layout_part_text_efl_text_style_strikethrough_type_set(Eo *obj,
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
_efl_canvas_layout_part_text_efl_text_style_underline_type_set(Eo *obj,
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
_efl_canvas_layout_part_text_efl_text_style_underline_height_set(Eo *obj,
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
_efl_canvas_layout_part_text_efl_text_style_underline_dashed_width_set(Eo *obj,
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
_efl_canvas_layout_part_text_efl_text_style_underline_dashed_gap_set(Eo *obj,
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
   if (eud->u.text_style.types == EDJE_PART_TEXT_PROP_NONE) return;

   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_BACKING_TYPE)
     {
        Edje_Part_Text_Prop *prop;

        prop = _prop_new(props, EDJE_PART_TEXT_PROP_BACKING_TYPE);
        prop->val.backing = efl_text_backing_type_get(rp->object);
     }
#define STYLE_COLOR_COLLECT(x, X) \
   if (eud->u.text_style.types & EDJE_PART_TEXT_PROP_COLOR_ ##X) \
     { \
        Edje_Part_Text_Prop *prop; \
        prop = _prop_new(props, EDJE_PART_TEXT_PROP_COLOR_ ##X); \
        efl_text_ ##x ##_color_get(rp->object, \
              &prop->val.color.r, &prop->val.color.g, \
              &prop->val.color.b, &prop->val.color.a); \
     } \

   STYLE_COLOR_COLLECT(backing, BACKING)
      STYLE_COLOR_COLLECT(glow, GLOW)
      STYLE_COLOR_COLLECT(glow2, GLOW2)
      STYLE_COLOR_COLLECT(normal, NORMAL)
      STYLE_COLOR_COLLECT(outline, OUTLINE)
      STYLE_COLOR_COLLECT(shadow, SHADOW)
      STYLE_COLOR_COLLECT(strikethrough, STRIKETHROUGH)
      STYLE_COLOR_COLLECT(underline, UNDERLINE)
      STYLE_COLOR_COLLECT(underline2, UNDERLINE2)
      STYLE_COLOR_COLLECT(underline_dashed, UNDERLINE_DASHED)
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
        efl_text_font_get(rp->object, &prop->val.font.font,
              &prop->val.font.size);
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
        efl_text_backing_type_set(
              efl_part(obj,
                 eud->part),
              prop->val.backing);
        break;

#define STYLE_COLOR_CASE(x, X) \
      case EDJE_PART_TEXT_PROP_COLOR_##X : \
        efl_text_##x ##_color_set(efl_part(obj, \
                 eud->part), \
                 prop->val.color.r, \
                 prop->val.color.g, \
                 prop->val.color.b, \
                 prop->val.color.a); \
        break;

      STYLE_COLOR_CASE(backing, BACKING)
      STYLE_COLOR_CASE(glow, GLOW)
      STYLE_COLOR_CASE(glow2, GLOW2)
      STYLE_COLOR_CASE(normal, NORMAL)
      STYLE_COLOR_CASE(outline, OUTLINE)
      STYLE_COLOR_CASE(shadow, SHADOW)
      STYLE_COLOR_CASE(strikethrough, STRIKETHROUGH)
      STYLE_COLOR_CASE(underline, UNDERLINE)
      STYLE_COLOR_CASE(underline2, UNDERLINE2)
      STYLE_COLOR_CASE(underline_dashed, UNDERLINE_DASHED)
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
        efl_text_font_set(efl_part(obj,
                 eud->part),
              prop->val.font.font,
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

