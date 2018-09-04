#define EFL_CANVAS_FILTER_INTERNAL_PROTECTED

#include "evas_common_private.h" /* Includes evas_bidi_utils stuff. */
#include "evas_private.h"

#include "../efl/interfaces/efl_gfx_filter.eo.h"
#include "efl_canvas_filter_internal.eo.h"
#include "evas_filter.h"

#define MY_CLASS EVAS_TEXT_CLASS

#define MY_CLASS_NAME "Evas_Text"

#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif

/* save typing */
#define COL_OBJECT(obj, sub) ARGB_JOIN(obj->sub->color.a, obj->sub->color.r, obj->sub->color.g, obj->sub->color.b)
#define COL_JOIN(o, sub, color) ARGB_JOIN(o->sub.color.a, o->sub.color.r, o->sub.color.g, o->sub.color.b)

/* private magic number for text objects */
static const char o_type[] = "text";

/* private struct for text object internal data */
typedef struct _Evas_Text_Data Evas_Text_Data;
typedef struct _Evas_Object_Text_Item Evas_Object_Text_Item;

struct _Evas_Text_Data
{
   DATA32               magic;

   struct {
      // WARNING - you cannot change the below outline/shadow etc. members
      // and their content without also updating _color_same() in this
      // file
      struct {
         unsigned char  r, g, b, a;
      } outline, shadow, glow, glow2;

      const char          *utf8_text; /* The text exposed to the API */
      const char          *font;
      Evas_Font_Description *fdesc;
      const char          *source;
      Eina_Unicode        *text;

      double               ellipsis;

      Evas_Font_Size       size;
      Evas_Text_Style_Type style;
      Efl_Text_Font_Bitmap_Scalable bitmap_scalable;
   } cur, prev;

   struct {
      Evas_Object_Text_Item    *ellipsis_start;
      Evas_Object_Text_Item    *ellipsis_end;
      Evas_Coord                w, h;
      int                       advance;
      int                       width_without_ellipsis;
      Eina_Bool                 ellipsis;
   } last_computed;

   Evas_BiDi_Paragraph_Props  *bidi_par_props;
   const char                 *bidi_delimiters;
   Evas_Object_Text_Item      *items;

   Evas_Font_Set              *font;

   float                       ascent, descent;
   float                       max_ascent, max_descent;

   Evas_BiDi_Direction         bidi_dir : 2;
   Evas_BiDi_Direction         paragraph_direction : 2;
   Eina_Bool                   inherit_paragraph_direction : 1;
   Eina_Bool                   changed_paragraph_direction : 1;
   Eina_Bool                   changed : 1;
   Eina_Bool                   has_filter : 1;
};

struct _Evas_Object_Text_Item
{
   EINA_INLIST;

   size_t               text_pos;
   size_t               visual_pos;
   Evas_Text_Props      text_props;
   Evas_Coord           x, w, h, adv;
};

/* private methods for text objects */
static void evas_object_text_init(Evas_Object *eo_obj);
static void evas_object_text_render(Evas_Object *eo_obj,
                                    Evas_Object_Protected_Data *obj,
                                    void *type_private_data,
                                    void *engine, void *output, void *context, void *surface,
                                    int x, int y, Eina_Bool do_async);
static void evas_object_text_free(Evas_Object *eo_obj,
				  Evas_Object_Protected_Data *obj);
static void evas_object_text_render_pre(Evas_Object *eo_obj,
					Evas_Object_Protected_Data *obj,
					void *type_private_data);
static void evas_object_text_render_post(Evas_Object *eo_obj,
					 Evas_Object_Protected_Data *obj,
					 void *type_private_data);

static unsigned int evas_object_text_id_get(Evas_Object *eo_obj);
static unsigned int evas_object_text_visual_id_get(Evas_Object *eo_obj);
static void *evas_object_text_engine_data_get(Evas_Object *eo_obj);

static int evas_object_text_is_opaque(Evas_Object *eo_obj,
				      Evas_Object_Protected_Data *obj,
				      void *type_private_data);
static int evas_object_text_was_opaque(Evas_Object *eo_obj,
				       Evas_Object_Protected_Data *obj,
				       void *type_private_data);

static void _evas_object_text_recalc(Evas_Object *eo_obj, Eina_Unicode *text);

static const Evas_Object_Func object_func =
{
   /* methods (compulsory) */
   NULL,
     evas_object_text_render,
     evas_object_text_render_pre,
     evas_object_text_render_post,
     evas_object_text_id_get,
     evas_object_text_visual_id_get,
     evas_object_text_engine_data_get,
   /* these are optional. NULL = nothing */
     NULL,
     NULL,
     NULL,
     NULL,
     evas_object_text_is_opaque,
     evas_object_text_was_opaque,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL // render_prepare
};

/* the actual api call to add a rect */
/* it has no other api calls as all properties are standard */

static int
_evas_object_text_char_coords_get(const Evas_Object *eo_obj,
      const Evas_Text_Data *o,
      size_t pos, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Text_Item *it;

   EINA_INLIST_FOREACH(EINA_INLIST_GET(o->items), it)
     {
        if ((it->text_pos <= pos) &&
              (pos < (it->text_pos + it->text_props.text_len)))
          {
             int ret;
             ret = ENFN->font_char_coords_get(ENC, o->font,
                   &it->text_props, pos - it->text_pos, x, y, w, h);
             if (x) *x += it->x;
             return ret;
          }
     }
   return 0;
}

static void
_evas_object_text_item_clean(Evas_Object_Text_Item *it)
{
   evas_common_text_props_content_unref(&it->text_props);
}

static void
_evas_object_text_item_del(Evas_Text_Data *o, Evas_Object_Text_Item *it)
{
   if (o->last_computed.ellipsis_start == it)
     o->last_computed.ellipsis_start = NULL;
   else if (o->last_computed.ellipsis_end == it)
     o->last_computed.ellipsis_end = NULL;

   if ((EINA_INLIST_GET(it)->next) ||
       (EINA_INLIST_GET(it)->prev) ||
       (EINA_INLIST_GET(o->items) == (EINA_INLIST_GET(it))))
     o->items = (Evas_Object_Text_Item *)eina_inlist_remove
     (EINA_INLIST_GET(o->items), EINA_INLIST_GET(it));
   _evas_object_text_item_clean(it);
   free(it);
}

static inline Eina_Bool
_color_same(const void *col1, const void *col2)
{
   const unsigned int *icol1 = col1, *icol2 = col2;
   return (*icol1 == *icol2);
}

static void
_evas_object_text_items_clean(Evas_Object_Protected_Data *obj, Evas_Text_Data *o)
{
   /* FIXME: also preserve item */
   if ((o->cur.font == o->prev.font) &&
       (o->cur.fdesc == o->prev.fdesc) &&
       (o->cur.size == o->prev.size) &&
       (_color_same(&o->cur.outline, &o->prev.outline)) &&
       (_color_same(&o->cur.shadow, &o->prev.shadow)) &&
       (_color_same(&o->cur.glow, &o->prev.glow)) &&
       (_color_same(&o->cur.glow2, &o->prev.glow2)) &&
       (o->cur.style == o->prev.style) &&
       (EINA_DBL_EQ(obj->cur->scale, obj->prev->scale)))
     {
        if ((o->last_computed.ellipsis_start) &&
            (o->last_computed.ellipsis_start == o->items))
          o->items = (Evas_Object_Text_Item *) eina_inlist_remove(EINA_INLIST_GET(o->items),
                                                                  EINA_INLIST_GET(o->last_computed.ellipsis_start));
        if ((o->last_computed.ellipsis_end) &&
            (EINA_INLIST_GET(o->last_computed.ellipsis_end) == EINA_INLIST_GET(o->items)->last))
          o->items = (Evas_Object_Text_Item *) eina_inlist_remove(EINA_INLIST_GET(o->items),
                                                                  EINA_INLIST_GET(o->last_computed.ellipsis_end));
     }
   else
     {
        /* It is not guaranteed that the ellipsis are still inside the items, so remove them by force  */
        if (o->last_computed.ellipsis_start)
          _evas_object_text_item_del(o, o->last_computed.ellipsis_start);
        o->last_computed.ellipsis_start = NULL;

        if (o->last_computed.ellipsis_end)
          _evas_object_text_item_del(o, o->last_computed.ellipsis_end);
        o->last_computed.ellipsis_end = NULL;
     }
   while (o->items)
     {
        _evas_object_text_item_del(o, o->items);
     }
}

static void
_evas_object_text_items_clear(Evas_Text_Data *o)
{
   if ((o->last_computed.ellipsis_start) &&
       (o->last_computed.ellipsis_start != o->items))
     {
        _evas_object_text_item_del(o, o->last_computed.ellipsis_start);
     }
   o->last_computed.ellipsis_start = NULL;
   if ((o->last_computed.ellipsis_end) &&
       (EINA_INLIST_GET(o->last_computed.ellipsis_end) != EINA_INLIST_GET(o->items)->last))
     {
        _evas_object_text_item_del(o, o->last_computed.ellipsis_end);
     }
   o->last_computed.ellipsis_end = NULL;
   while (o->items)
     {
        _evas_object_text_item_del(o, o->items);
     }
}

#ifdef BIDI_SUPPORT
static int
_evas_object_text_it_compare_logical(const void *_it1, const void *_it2)
{
   const Evas_Object_Text_Item *it1 = _it1, *it2 = _it2;
   if (it1->text_pos < it2->text_pos)
     return -1;
   else if (it1->text_pos == it2->text_pos)
     return 0;
   else
     return 1;

}
#endif

static int
_evas_object_text_last_up_to_pos(const Evas_Object *eo_obj,
      const Evas_Text_Data *o, Evas_Coord cx, Evas_Coord cy)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Text_Item *it;
   int pos = -1;

#ifdef BIDI_SUPPORT
   /* Reorder if it's a bidi text */
   if (o->bidi_par_props)
     {
        Eina_List *logical_it = NULL;
        Evas_Object_Text_Item *i;
        Eina_List *itr;
        Evas_Coord x = 0;
        /* Insert all to the logical list */
        EINA_INLIST_FOREACH(o->items, i)
          {
             logical_it = eina_list_sorted_insert(logical_it,
                   _evas_object_text_it_compare_logical, i);
          }
        EINA_LIST_FOREACH(logical_it, itr, it)
          {
             if ((x <= cx) && (cx < x + it->adv))
               {
                  pos = it->text_pos + ENFN->font_last_up_to_pos(ENC,
                        o->font,
                        &it->text_props,
                        cx - x,
                        cy, 0);
                  break;
               }
             x += it->adv;
          }
        eina_list_free(logical_it);
     }
   else
#endif
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(o->items), it)
          {
             if ((it->x <= cx) && (cx < it->x + it->adv))
               {
                  return it->text_pos + ENFN->font_last_up_to_pos(ENC,
                        o->font,
                        &it->text_props,
                        cx - it->x,
                        cy, 0);
               }
          }
     }
   return pos;
}

static int
_evas_object_text_char_at_coords(const Evas_Object *eo_obj,
      const Evas_Text_Data *o, Evas_Coord cx, Evas_Coord cy,
      Evas_Coord *rx, Evas_Coord *ry, Evas_Coord *rw, Evas_Coord *rh)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Object_Text_Item *it;

   EINA_INLIST_FOREACH(EINA_INLIST_GET(o->items), it)
     {
        if ((it->x <= cx) && (cx < it->x + it->adv))
          {
             return it->text_pos + ENFN->font_char_at_coords_get(ENC,
                   o->font,
                   &it->text_props,
                   cx - it->x,
                   cy,
                   rx, ry,
                   rw, rh);
          }
     }
   return -1;
}

static Evas_Coord
_evas_object_text_horiz_width_without_ellipsis_get(const Evas_Text_Data *o)
{
   return o->last_computed.width_without_ellipsis;
}

static Evas_Coord
_evas_object_text_horiz_advance_get(const Evas_Text_Data *o)
{
   return o->last_computed.advance;
}

static Evas_Coord
_evas_object_text_vert_advance_get(const Evas_Object *obj EINA_UNUSED,
      const Evas_Text_Data *o)
{
   return o->max_ascent + o->max_descent;
}

EAPI Evas_Object *
evas_object_text_add(Evas *e)
{
   e = evas_find(e);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(e, EVAS_CANVAS_CLASS), NULL);
   return efl_add(EVAS_TEXT_CLASS, e, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_evas_text_efl_object_constructor(Eo *eo_obj, Evas_Text_Data *o EINA_UNUSED)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
   evas_object_text_init(eo_obj);

   return eo_obj;
}

EOLIAN static void
_evas_text_efl_text_font_font_source_set(Eo *eo_obj, Evas_Text_Data *o, const char *font_source)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if (((o->cur.source) && (font_source) &&
        (!strcmp(o->cur.source, font_source))) ||
       (!o->cur.source && !font_source))
     return;
   evas_object_async_block(obj);
   /*
   if (o->cur.source) eina_stringshare_del(o->cur.source);
   if (font_source) o->cur.source = eina_stringshare_add(font_source);
   else o->cur.source = NULL;
    */
   eina_stringshare_replace(&o->cur.source, font_source);
}

EOLIAN static const char*
_evas_text_efl_text_font_font_source_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->cur.source;
}

static void
_evas_text_font_reload(Eo *eo_obj, Evas_Text_Data *o)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Eina_Bool pass = EINA_FALSE, freeze = EINA_FALSE;
   Eina_Bool source_invisible = EINA_FALSE;
   Eina_List *was = NULL;

   if (!(obj->layer->evas->is_frozen))
     {
        pass = evas_event_passes_through(eo_obj, obj);
        freeze = evas_event_freezes_through(eo_obj, obj);
        source_invisible = evas_object_is_source_invisible(eo_obj, obj);
        if ((!pass) && (!freeze) && (!source_invisible))
          was = _evas_pointer_list_in_rect_get(obj->layer->evas, eo_obj, obj,
                                               1, 1);
     }

   /* DO IT */
   evas_font_free(o->font);
   o->font = NULL;

   o->font = evas_font_load(obj->layer->evas->font_path,
                            obj->layer->evas->hinting,
                            o->cur.fdesc, o->cur.source,
                            (int)(((double) o->cur.size) * obj->cur->scale),
                            o->cur.bitmap_scalable);
     {
        o->ascent = 0;
        o->descent = 0;
        o->max_ascent = 0;
        o->max_descent = 0;
     }
   _evas_object_text_items_clear(o);
   _evas_object_text_recalc(eo_obj, o->cur.text);
   o->changed = 1;
   if (o->has_filter)
     evas_filter_changed_set(eo_obj, EINA_TRUE);
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   if (!obj->layer->evas->is_frozen && !pass && !freeze && obj->cur->visible)
     _evas_canvas_event_pointer_in_list_mouse_move_feed(obj->layer->evas, was, eo_obj, obj, 1, 1, EINA_TRUE, NULL);
   eina_list_free(was);
   evas_object_inform_call_resize(eo_obj, obj);
}

EOLIAN static void
_evas_text_efl_text_font_font_set(Eo *eo_obj, Evas_Text_Data *o, const char *font, Evas_Font_Size size)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Font_Description *fdesc;

   if ((!font) || (size <= 0)) return;

   evas_object_async_block(obj);
   if ((size == o->cur.size) &&
       (o->cur.font && !strcmp(font, o->cur.font))) return;

   /* We can't assume the given font is same with current fdesc by comparing string.
      Since Evas starts to supporting "auto" for language,
      the given font string should be parsed once before comparing it. */
   fdesc = evas_font_desc_new();

   /* Set default language according to locale. */
   eina_stringshare_replace(&(fdesc->lang), evas_font_lang_normalize("auto"));
   evas_font_name_parse(fdesc, font);

   if (o->cur.fdesc && !evas_font_desc_cmp(fdesc, o->cur.fdesc) &&
       (size == o->cur.size))
     {
        evas_font_desc_unref(fdesc);
        return;
     }

   if (o->cur.fdesc) evas_font_desc_unref(o->cur.fdesc);
   o->cur.fdesc = fdesc;

   o->cur.size = size;
   eina_stringshare_replace(&o->cur.font, font);
   o->prev.font = NULL;

   _evas_text_font_reload(eo_obj, o);
}

EOLIAN static void
_evas_text_efl_text_font_font_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, const char **font, Evas_Font_Size *size)
{
   if (font) *font = o->cur.font;
   if (size) *size = o->cur.size;
}

static void
_evas_object_text_item_update_sizes(Evas_Object_Protected_Data *obj, Evas_Text_Data *o, Evas_Object_Text_Item *it)
{
   ENFN->font_string_size_get(ENC,
         o->font,
         &it->text_props,
         &it->w, &it->h);
   it->adv = ENFN->font_h_advance_get(ENC, o->font,
         &it->text_props);
}

/**
 * @internal
 * Create a new text layout item from the string and the format.
 *
 * @param c the context to work on - Not NULL.
 * @param fmt the format to use.
 * @param str the string to use.
 */
static Evas_Object_Text_Item *
_evas_object_text_item_new(Evas_Object_Protected_Data *obj,
                           Evas_Text_Data *o,
                           Evas_Font_Instance *fi, const Eina_Unicode *str,
                           Evas_Script_Type script,
                           size_t pos, size_t visual_pos, size_t len)
{
   Evas_Object_Text_Item *it;

   it = calloc(1, sizeof(Evas_Object_Text_Item));
   it->text_pos = pos;
   it->visual_pos = visual_pos;
   evas_common_text_props_bidi_set(&it->text_props, o->bidi_par_props,
         it->text_pos);
   evas_common_text_props_script_set(&it->text_props, script);

   if (fi)
     {
        ENFN->font_text_props_info_create(ENC,
              fi, str + pos, &it->text_props,
              o->bidi_par_props, it->text_pos, len, EVAS_TEXT_PROPS_MODE_SHAPE,
              o->cur.fdesc->lang);
        _evas_object_text_item_update_sizes(obj, o, it);
     }
   o->items = (Evas_Object_Text_Item *)
      eina_inlist_append(EINA_INLIST_GET(o->items), EINA_INLIST_GET(it));
   return it;
}

/**
 * @internal
 * Orders o->items according to the visual position.
 *
 * @param obj the evas object
 * @param o the text object
 */
static void
_evas_object_text_item_order(Evas_Object *eo_obj, Evas_Text_Data *o)
{
   (void) eo_obj;
#ifdef BIDI_SUPPORT
   /* Reorder if it's a bidi text */
   if (o->bidi_par_props)
     {
        Evas_Object_Text_Item *i, *j, *min;
        i = o->items;
        while (i)
          {
             min = i;
             EINA_INLIST_FOREACH(i, j)
               {
                  if (j->visual_pos < min->visual_pos)
                    {
                       min = j;
                    }
               }
             if (min != i)
               {
                  o->items = (Evas_Object_Text_Item *) eina_inlist_remove(EINA_INLIST_GET(o->items), EINA_INLIST_GET(min));
                  o->items = (Evas_Object_Text_Item *) eina_inlist_prepend_relative(EINA_INLIST_GET(o->items), EINA_INLIST_GET(min), EINA_INLIST_GET(i));
               }

             i = (Evas_Object_Text_Item *) EINA_INLIST_GET(min)->next;
          }
     }
#endif

   /* calculate the positions according to the order. */
     {
        Evas_Object_Text_Item *it = o->items;
        Evas_Coord x = 0;

        while (it)
          {
             it->x = x;
             x += it->adv;
             it = (Evas_Object_Text_Item *) EINA_INLIST_GET(it)->next;
          }
     }
}

/**
 * Create ellipsis.
 */
static const Eina_Unicode _ellip_str[2] = { 0x2026, '\0' };

/* FIXME: We currently leak ellipsis items. */
static Evas_Object_Text_Item *
_layout_ellipsis_item_new(Evas_Object_Protected_Data *obj, Evas_Text_Data *o)
{
   Evas_Object_Text_Item *ellip_ti = NULL;
   Evas_Script_Type script;
   Evas_Font_Instance *script_fi = NULL, *cur_fi = NULL;
   size_t len = 1; /* The length of _ellip_str */

   script = evas_common_language_script_type_get(_ellip_str, 1);

   if (o->font)
     {
        (void) ENFN->font_run_end_get(ENC, o->font, &script_fi, &cur_fi,
                                      script, _ellip_str, 1);
         ellip_ti = _evas_object_text_item_new(obj, o, cur_fi,
                                               _ellip_str, script, 0, 0, len);
     }

   return ellip_ti;
}

/* EINA_TRUE if this item is ok and should be included, false if should be
 * discarded. */
static Eina_Bool
_layout_text_item_trim(Evas_Object_Protected_Data *obj, Evas_Text_Data *o, Evas_Object_Text_Item *ti, int idx, Eina_Bool want_start)
{
   Evas_Text_Props new_text_props;
   if (idx >= (int) ti->text_props.text_len)
      return EINA_FALSE;

   memset(&new_text_props, 0, sizeof (new_text_props));

   while (!evas_common_text_props_split(&ti->text_props, &new_text_props, idx))
     idx--;
   if (want_start)
     {
        evas_common_text_props_content_unref(&new_text_props);
     }
   else
     {
        evas_common_text_props_content_unref(&ti->text_props);
        memcpy(&ti->text_props, &new_text_props, sizeof(ti->text_props));
        ti->text_pos += idx;
        ti->visual_pos += idx;
     }
   _evas_object_text_item_update_sizes(obj, o, ti);

   return EINA_TRUE;
}

static void
_evas_object_text_pad_get(const Eo *eo_obj, Evas_Text_Data *o, int *l, int *r, int *t, int *b)
{
   if (l) *l = 0;
   if (r) *r = 0;
   if (t) *t = 0;
   if (b) *b = 0;
   if (!o->has_filter)
     evas_text_style_pad_get(o->cur.style, l, r, t, b);
   else
     efl_gfx_filter_padding_get(eo_obj, l, r, t, b);
}

/**
 * @internal
 * Populates o->items with the items of the text according to text
 *
 * @param obj the evas object
 * @param o the text object
 * @param text the text to layout
 */
static void
_evas_object_text_layout(Evas_Object *eo_obj, Evas_Text_Data *o, Eina_Unicode *text)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   EvasBiDiStrIndex *v_to_l = NULL;
   Evas_Coord advance = 0, width = 0;
   size_t pos, visual_pos;
   int len = eina_unicode_strlen(text);
   int l = 0, r = 0;
#ifdef BIDI_SUPPORT
   int par_len = len;
   int *segment_idxs = NULL;
   Eina_Bool is_bidi = EINA_FALSE;
#endif

   if (o->items &&
       !memcmp(&o->cur, &o->prev, sizeof (o->cur)) &&
       o->cur.text == text &&
       (EINA_DBL_EQ(obj->cur->scale, obj->prev->scale)) &&
       ((o->last_computed.advance <= obj->cur->geometry.w && !o->last_computed.ellipsis) ||
        (o->last_computed.w == obj->cur->geometry.w)) &&
       !o->changed_paragraph_direction)
     return;

   o->last_computed.ellipsis = EINA_FALSE;
   evas_object_content_change(eo_obj, obj);

   if (o->items) _evas_object_text_items_clean(obj, o);

   if (text && *text)
      o->bidi_dir = EVAS_BIDI_DIRECTION_LTR;
   else
      o->bidi_dir = EVAS_BIDI_DIRECTION_NEUTRAL;

#ifdef BIDI_SUPPORT
   if (text)
     {
        if (o->bidi_delimiters)
          segment_idxs = evas_bidi_segment_idxs_get(text, o->bidi_delimiters);
     }
   evas_bidi_paragraph_props_unref(o->bidi_par_props);
   if (text)
     {
        Evas_BiDi_Direction par_dir;
        EvasBiDiParType bidi_par_type;

        par_dir = o->paragraph_direction;

        switch (par_dir)
          {
           case EVAS_BIDI_DIRECTION_LTR:
              bidi_par_type = EVAS_BIDI_PARAGRAPH_LTR;
              break;
           case EVAS_BIDI_DIRECTION_RTL:
              bidi_par_type = EVAS_BIDI_PARAGRAPH_RTL;
              break;
           case EVAS_BIDI_DIRECTION_NEUTRAL:
           default:
              bidi_par_type = EVAS_BIDI_PARAGRAPH_NEUTRAL;
              break;
          }

        o->bidi_par_props = evas_bidi_paragraph_props_get(text, len, segment_idxs, bidi_par_type);
        is_bidi = !!o->bidi_par_props;
     }

   if (o->bidi_par_props)
      o->bidi_dir = EVAS_BIDI_PAR_TYPE_TO_DIRECTION(o->bidi_par_props->direction);

   evas_bidi_props_reorder_line(NULL, 0, len, o->bidi_par_props, &v_to_l);
   if (segment_idxs) free(segment_idxs);
#endif
   visual_pos = pos = 0;

   if (text)
     {
        const Evas_Object_Text_Item *last_it = NULL;
#ifdef BIDI_SUPPORT
        size_t max_vpos = 0;
#endif

        while (len > 0)
          {
             Evas_Font_Instance *script_fi = NULL;
             int script_len = len, tmp_cut;
             Evas_Script_Type script;
             tmp_cut = evas_common_language_script_end_of_run_get
               (text + pos,
                o->bidi_par_props,
                pos, len);
             if (tmp_cut > 0) script_len = tmp_cut;

             script = evas_common_language_script_type_get(text + pos, script_len);

             while (script_len > 0)
               {
                  const Evas_Object_Text_Item *it;
                  Evas_Font_Instance *cur_fi = NULL;
                  int run_len = script_len;
                  if (o->font)
                    {
                       run_len = ENFN->font_run_end_get
                         (ENC, o->font, &script_fi, &cur_fi,
                          script, text + pos, script_len);
                    }
#ifdef BIDI_SUPPORT
                  visual_pos = evas_bidi_position_logical_to_visual
                    (v_to_l, par_len, pos);
#else
                  visual_pos = pos;
#endif
                  it = _evas_object_text_item_new(obj, o, cur_fi, text, script,
                                                  pos, visual_pos, run_len);

                  advance += it->adv;
                  pos += run_len;
                  script_len -= run_len;
                  len -= run_len;

                  if (it->w > 0)
                    {
#ifdef BIDI_SUPPORT
                       if (is_bidi)
                         {
                            if (!last_it || (visual_pos >= max_vpos))
                              {
                                 last_it = it;
                                 max_vpos = visual_pos;
                              }
                         }
                       else
#endif
                         {
                            last_it = it;
                         }
                    }
               }
          }

        width = advance;
        if (last_it)
          width += last_it->w - last_it->adv;
     }
   o->last_computed.width_without_ellipsis = width;

   _evas_object_text_pad_get(eo_obj, o, &l, &r, NULL, NULL);

   /* Handle ellipsis */
   if (pos && (o->cur.ellipsis >= 0.0) && (width + l + r > obj->cur->geometry.w) && (obj->cur->geometry.w > 0))
     {
        Evas_Coord ellip_frame = obj->cur->geometry.w;
        Evas_Object_Text_Item *start_ellip_it = NULL, *end_ellip_it = NULL;

        o->last_computed.ellipsis = EINA_TRUE;

        /* Account of the ellipsis item width. As long as ellipsis != 0
         * we have a left ellipsis. And the same with 1 and right. */
        if (!EINA_DBL_EQ(o->cur.ellipsis, 0.0))
          {
             if (o->last_computed.ellipsis_start)
               {
                  start_ellip_it = o->last_computed.ellipsis_start;
                  o->items = (Evas_Object_Text_Item *)
                    eina_inlist_append(EINA_INLIST_GET(o->items), EINA_INLIST_GET(start_ellip_it));
               }
             else
               {
                  start_ellip_it = _layout_ellipsis_item_new(obj, o);
               }
             o->last_computed.ellipsis_start = start_ellip_it;
             ellip_frame -= start_ellip_it->w;
          }
        if (!EINA_DBL_EQ(o->cur.ellipsis, 1.0))
          {
             /* FIXME: Should take the last item's font and style and etc. *//* weird it's a text, should always have the same style/font */
             if (o->last_computed.ellipsis_end)
               {
                  end_ellip_it = o->last_computed.ellipsis_end;
                  o->items = (Evas_Object_Text_Item *)
                    eina_inlist_append(EINA_INLIST_GET(o->items), EINA_INLIST_GET(end_ellip_it));
               }
             else
               {
                  end_ellip_it = _layout_ellipsis_item_new(obj, o);
               }
             o->last_computed.ellipsis_end = end_ellip_it;
             ellip_frame -= end_ellip_it->w;
          }

        /* The point where we should start from, going for the full
         * ellip frame. */
        Evas_Coord ellipsis_coord = o->cur.ellipsis * (width - ellip_frame);
        if (start_ellip_it)
          {
             Evas_Object_Text_Item *itr = o->items;
             advance = 0;

             while (itr && (advance + l + r + itr->w < ellipsis_coord))
               {
                  Eina_Inlist *itrn = EINA_INLIST_GET(itr)->next;
                  if ((itr != start_ellip_it) && (itr != end_ellip_it))
                    {
                       advance += itr->adv;
                       _evas_object_text_item_del(o, itr);
                    }
                  itr = (Evas_Object_Text_Item *) itrn;
               }
             if (itr && (itr != start_ellip_it))
               {
                  int cut = ENFN->font_last_up_to_pos(ENC,
                        o->font,
                        &itr->text_props,
                        ellipsis_coord - (advance + l + r),
                        0, start_ellip_it->w);
                  if (cut >= 0)
                    {
                       start_ellip_it->text_pos = itr->text_pos;

                       if (itr->text_props.bidi_dir == EVAS_BIDI_DIRECTION_RTL)
                         start_ellip_it->visual_pos = itr->visual_pos + cut + 1;
                       else
                         start_ellip_it->visual_pos = itr->visual_pos;

                       if (!_layout_text_item_trim(obj, o, itr, cut + 1, EINA_FALSE))
                         {
                            _evas_object_text_item_del(o, itr);
                         }
                    }
               }

             if (!o->bidi_par_props)
               {
                  o->items = (Evas_Object_Text_Item *) eina_inlist_remove(EINA_INLIST_GET(o->items), EINA_INLIST_GET(start_ellip_it));
                  o->items = (Evas_Object_Text_Item *) eina_inlist_prepend(EINA_INLIST_GET(o->items), EINA_INLIST_GET(start_ellip_it));
               }
          }

        if (end_ellip_it)
          {
             Evas_Object_Text_Item *itr = o->items;
             advance = 0;

             while (itr)
               {
                  if (itr != end_ellip_it) /* was start_ellip_it */
                    {
                       if (advance + l + r + itr->w >= ellip_frame)
                         {
                            break;
                         }
                       advance += itr->adv;
                    }
                  itr = (Evas_Object_Text_Item *) EINA_INLIST_GET(itr)->next;
               }

             if (itr == end_ellip_it)
               {
                  /* FIXME: We shouldn't do anything. */
               }

             /* In case when we reach end of itr list, and have NULL */
             int cut = -1;
             if (itr && (itr != end_ellip_it))
               {
                  cut = ENFN->font_last_up_to_pos(ENC,
                                                  o->font,
                                                  &itr->text_props,
                                                  ellip_frame - (advance + l + r),
                                                  0, end_ellip_it->w);
                  if (cut >= 0)
                    {
                       end_ellip_it->text_pos = itr->text_pos + cut;

                        if (itr->text_props.bidi_dir == EVAS_BIDI_DIRECTION_RTL)
                         end_ellip_it->visual_pos = itr->visual_pos - 1;
                        else
                         end_ellip_it->visual_pos = itr->visual_pos + cut;

                       if (_layout_text_item_trim(obj, o, itr, cut, EINA_TRUE))
                         {
                            itr = (Evas_Object_Text_Item *) EINA_INLIST_GET(itr)->next;
                         }
                    }
               }


             /* Remove the rest of the items */
             while (itr)
               {
                  Eina_Inlist *itrn = EINA_INLIST_GET(itr)->next;
                  if ((itr != start_ellip_it) && (itr != end_ellip_it))
                    {
                       _evas_object_text_item_del(o, itr);
                    }
                  itr = (Evas_Object_Text_Item *) itrn;
               }
          }
     }
   if (o->cur.text != text) free(o->cur.text);
   o->cur.text = text;
   o->prev = o->cur;

   {
      Evas_Object_Text_Item *itr = o->items;
      advance = 0;

      while (itr)
        {
           advance += itr->adv;
           itr = (Evas_Object_Text_Item *) EINA_INLIST_GET(itr)->next;
        }
      o->last_computed.advance = advance;
   }

   _evas_object_text_item_order(eo_obj, o);

   if (v_to_l) free(v_to_l);
}

EOLIAN static void
_evas_text_efl_gfx_entity_size_set(Eo *eo_obj, Evas_Text_Data *o, Eina_Size2D sz)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->geometry.w = sz.w;
        state_write->geometry.h = sz.h;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   _evas_object_text_recalc(eo_obj, o->cur.text);
}

EOLIAN static void
_evas_text_ellipsis_set(Eo *eo_obj, Evas_Text_Data *o, double ellipsis)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (EINA_DBL_EQ(o->cur.ellipsis, ellipsis)) return;

   evas_object_async_block(obj);
   o->prev.ellipsis = o->cur.ellipsis;
   o->cur.ellipsis = ellipsis;
   o->changed = 1;
   if (o->has_filter)
     evas_filter_changed_set(eo_obj, EINA_TRUE);
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
}

EOLIAN static double
_evas_text_ellipsis_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->cur.ellipsis;
}

EOLIAN static void
_evas_text_efl_object_dbg_info_get(Eo *eo_obj, Evas_Text_Data *o EINA_UNUSED, Efl_Dbg_Info *root)
{
   efl_dbg_info_get(efl_super(eo_obj, MY_CLASS), root);
   Efl_Dbg_Info *group = EFL_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);

   const char *text;
   int size;
   efl_text_font_get(eo_obj, &text, &size);
   EFL_DBG_INFO_APPEND(group, "Font", EINA_VALUE_TYPE_STRING, text);
   EFL_DBG_INFO_APPEND(group, "Text size", EINA_VALUE_TYPE_INT, size);

   text = efl_text_font_source_get(eo_obj);
   EFL_DBG_INFO_APPEND(group, "Font source", EINA_VALUE_TYPE_STRING, text);

   text = efl_text_get(eo_obj);
   EFL_DBG_INFO_APPEND(group, "Text", EINA_VALUE_TYPE_STRING, text);
}

EOLIAN static void
_evas_text_efl_text_text_set(Eo *eo_obj, Evas_Text_Data *o, const char *_text)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   int len;
   Eina_Unicode *text;
   Eina_List *was = NULL;

   if ((o->cur.utf8_text) && (_text) && (!strcmp(o->cur.utf8_text, _text)))
      return;
   evas_object_async_block(obj);
   text = eina_unicode_utf8_to_unicode(_text, &len);

   if (!text) text = eina_unicode_strdup(EINA_UNICODE_EMPTY_STRING);
   was = _evas_pointer_list_in_rect_get(obj->layer->evas, eo_obj, obj, 1, 1);
   /* DO II */
   /*Update bidi_props*/

   _evas_object_text_items_clear(o);

   _evas_object_text_recalc(eo_obj, text);
   eina_stringshare_replace(&o->cur.utf8_text, _text);
   o->prev.utf8_text = NULL;

   if (o->cur.text != text) free(text);

   o->changed = 1;
   if (o->has_filter)
     evas_filter_changed_set(eo_obj, EINA_TRUE);
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   if (obj->cur->visible)
     _evas_canvas_event_pointer_in_list_mouse_move_feed(obj->layer->evas, was, eo_obj, obj, 1, 1, EINA_FALSE, NULL);
   eina_list_free(was);
   evas_object_inform_call_resize(eo_obj, obj);
}

EOLIAN static void
_evas_text_bidi_delimiters_set(Eo *eo_obj, Evas_Text_Data *o, const char *delim)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_async_block(obj);
   eina_stringshare_replace(&o->bidi_delimiters, delim);
}

EOLIAN static const char*
_evas_text_bidi_delimiters_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->bidi_delimiters;
}

EOLIAN static const char*
_evas_text_efl_text_text_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->cur.utf8_text;
}

EOLIAN static Evas_BiDi_Direction
_evas_text_direction_get(const Eo *eo_obj, Evas_Text_Data *o)
{
#ifdef BIDI_SUPPORT
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if (o->inherit_paragraph_direction)
     {
        Evas_BiDi_Direction parent_dir = EVAS_BIDI_DIRECTION_NEUTRAL;

        if (obj->smart.parent)
          parent_dir = evas_object_paragraph_direction_get(obj->smart.parent);

        if (parent_dir != o->paragraph_direction)
          {
             o->paragraph_direction = parent_dir;
             o->changed_paragraph_direction = EINA_TRUE;
          }
     }

   if (o->changed_paragraph_direction)
     {
        /* XXX const? */
        _evas_object_text_recalc((Evas_Object *)eo_obj, o->cur.text);
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            (Evas_Object *)eo_obj, obj);
     }
#endif

   return o->bidi_dir;
}

EOLIAN static Evas_Coord
_evas_text_ascent_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->ascent;
}

EOLIAN static Evas_Coord
_evas_text_descent_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->descent;
}

EOLIAN static Evas_Coord
_evas_text_max_ascent_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->max_ascent;
}

EOLIAN static Evas_Coord
_evas_text_max_descent_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->max_descent;
}

EOLIAN static Evas_Coord
_evas_text_inset_get(const Eo *eo_obj, Evas_Text_Data *o)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Coord inset = 0;
   if (!o->font) return inset;
   if (!o->items) return inset;
   inset = ENFN->font_inset_get(ENC, o->font, &o->items->text_props);

   return inset;
}

EOLIAN static Evas_Coord
_evas_text_horiz_advance_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   Evas_Coord horiz = 0;
   if (!o->font) return horiz;
   if (!o->items) return horiz;
   horiz = _evas_object_text_horiz_advance_get(o);

   return horiz;
}

EOLIAN static Evas_Coord
_evas_text_vert_advance_get(const Eo *eo_obj, Evas_Text_Data *o)
{
   Evas_Coord vert;
   vert = 0;
   if (!o->font) return vert;
   if (!o->items)
     {
        vert = o->max_ascent + o->max_descent;
        return vert;
     }
   vert = _evas_object_text_vert_advance_get(eo_obj, o);

   return vert;
}

EOLIAN static Eina_Bool
_evas_text_char_pos_get(const Eo *eo_obj, Evas_Text_Data *o, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Eina_Bool ret;
   ret = EINA_FALSE;

   int l = 0, r = 0, t = 0, b = 0;
   int x = 0, y = 0, w = 0, h = 0;

   if (!o->font) return ret;
   if (!o->items || (pos < 0)) return ret;

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   Eina_Bool int_ret = _evas_object_text_char_coords_get(eo_obj, o, (size_t) pos,
            &x, &y, &w, &h);

   _evas_object_text_pad_get(eo_obj, o, &l, &r, &t, &b);
   y += o->max_ascent - t;
   x -= l;
   if (x < 0)
     {
        w += x;
        x = 0;
     }
   if ((x + w) > obj->cur->geometry.w) w = obj->cur->geometry.w - x;
   if (w < 0) w = 0;
   if (y < 0)
     {
        h += y;
        y = 0;
     }
   if ((y + h) > obj->cur->geometry.h) h = obj->cur->geometry.h - y;
   if (h < 0) h = 0;
   if (cx) *cx = x;
   if (cy) *cy = y;
   if (cw) *cw = w + l + r;
   if (ch) *ch = h + t + b;
   ret = int_ret;

   return ret;
}

EOLIAN static int
_evas_text_last_up_to_pos(const Eo *eo_obj, Evas_Text_Data *o, Evas_Coord x, Evas_Coord y)
{
   int ret;
   ret = -1;

   if (!o->font) return ret;
   if (!o->items) return ret;
   int int_ret = _evas_object_text_last_up_to_pos(eo_obj, o, x, y - o->max_ascent);
   ret = int_ret;

   return ret;
}

EOLIAN static int
_evas_text_char_coords_get(const Eo *eo_obj, Evas_Text_Data *o, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   int ret;
   ret = -1;

   int l = 0, r = 0, t = 0, b = 0;
   int rx = 0, ry = 0, rw = 0, rh = 0;

   if (!o->font) return ret;
   if (!o->items) return ret;

   int int_ret = _evas_object_text_char_at_coords(eo_obj, o, x, y - o->max_ascent,
         &rx, &ry, &rw, &rh);
   _evas_object_text_pad_get(eo_obj, o, &l, &r, &t, &b);
   ry += o->max_ascent - t;
   rx -= l;
   if (rx < 0)
     {
        rw += rx;
        rx = 0;
     }
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if ((rx + rw) > obj->cur->geometry.w) rw = obj->cur->geometry.w - rx;
   if (rw < 0) rw = 0;
   if (ry < 0)
     {
        rh += ry;
        ry = 0;
     }
   if ((ry + rh) > obj->cur->geometry.h) rh = obj->cur->geometry.h - ry;
   if (rh < 0) rh = 0;
   if (cx) *cx = rx;
   if (cy) *cy = ry;
   if (cw) *cw = rw + l + r;
   if (ch) *ch = rh + t + b;
   ret = int_ret;

   return ret;
}

EOLIAN static void
_evas_text_style_set(Eo *eo_obj, Evas_Text_Data *o, Evas_Text_Style_Type style)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   int pl = 0, pr = 0, pt = 0, pb = 0, l = 0, r = 0, t = 0, b = 0;
   int w = 0, h = 0;

   if (o->cur.style == style) return;
   evas_object_async_block(obj);

   _evas_object_text_pad_get(eo_obj, o, &pl, &pr, &pt, &pb);
   //evas_text_style_pad_get(o->cur.style, &pl, &pr, &pt, &pb);
   o->cur.style = style;
   _evas_object_text_pad_get(eo_obj, o, &l, &r, &t, &b);
   //evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);

   if (o->items) w = obj->cur->geometry.w + (l - pl) + (r - pr);
   h = obj->cur->geometry.h + (t - pt) + (b - pb);

   efl_gfx_entity_size_set(efl_super(eo_obj, MY_CLASS), EINA_SIZE2D(w,  h));
   evas_object_change(eo_obj, obj);
}

EOLIAN static Evas_Text_Style_Type
_evas_text_style_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->cur.style;
}

EOLIAN static void
_evas_text_shadow_color_set(Eo *eo_obj, Evas_Text_Data *o, int r, int g, int b, int a)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if ((o->cur.shadow.r == r) && (o->cur.shadow.g == g) &&
       (o->cur.shadow.b == b) && (o->cur.shadow.a == a))
     return;
   evas_object_async_block(obj);
   o->cur.shadow.r = r;
   o->cur.shadow.g = g;
   o->cur.shadow.b = b;
   o->cur.shadow.a = a;
   o->changed = 1;
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_text_shadow_color_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, int *r, int *g, int *b, int *a)
{
   if (r) *r = o->cur.shadow.r;
   if (g) *g = o->cur.shadow.g;
   if (b) *b = o->cur.shadow.b;
   if (a) *a = o->cur.shadow.a;
}

EOLIAN static void
_evas_text_glow_color_set(Eo *eo_obj, Evas_Text_Data *o, int r, int g, int b, int a)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if ((o->cur.glow.r == r) && (o->cur.glow.g == g) &&
       (o->cur.glow.b == b) && (o->cur.glow.a == a))
     return;
   evas_object_async_block(obj);
   o->cur.glow.r = r;
   o->cur.glow.g = g;
   o->cur.glow.b = b;
   o->cur.glow.a = a;
   o->changed = 1;
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_text_glow_color_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, int *r, int *g, int *b, int *a)
{
   if (r) *r = o->cur.glow.r;
   if (g) *g = o->cur.glow.g;
   if (b) *b = o->cur.glow.b;
   if (a) *a = o->cur.glow.a;
}

EOLIAN static void
_evas_text_glow2_color_set(Eo *eo_obj, Evas_Text_Data *o, int r, int g, int b, int a)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if ((o->cur.glow2.r == r) && (o->cur.glow2.g == g) &&
       (o->cur.glow2.b == b) && (o->cur.glow2.a == a))
     return;
   evas_object_async_block(obj);
   o->cur.glow2.r = r;
   o->cur.glow2.g = g;
   o->cur.glow2.b = b;
   o->cur.glow2.a = a;
   o->changed = 1;
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_text_glow2_color_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, int *r, int *g, int *b, int *a)
{
   if (r) *r = o->cur.glow2.r;
   if (g) *g = o->cur.glow2.g;
   if (b) *b = o->cur.glow2.b;
   if (a) *a = o->cur.glow2.a;
}

EOLIAN static void
_evas_text_outline_color_set(Eo *eo_obj, Evas_Text_Data *o, int r, int g, int b, int a)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if ((o->cur.outline.r == r) && (o->cur.outline.g == g) &&
       (o->cur.outline.b == b) && (o->cur.outline.a == a))
     return;
   evas_object_async_block(obj);
   o->cur.outline.r = r;
   o->cur.outline.g = g;
   o->cur.outline.b = b;
   o->cur.outline.a = a;
   o->changed = 1;
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_text_outline_color_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, int *r, int *g, int *b, int *a)
{
   if (r) *r = o->cur.outline.r;
   if (g) *g = o->cur.outline.g;
   if (b) *b = o->cur.outline.b;
   if (a) *a = o->cur.outline.a;
}

EOLIAN static void
_evas_text_style_pad_get(const Eo *eo_obj, Evas_Text_Data *o, int *l, int *r, int *t, int *b)
{
   _evas_object_text_pad_get(eo_obj, o, l, r, t, b);
}

EAPI int
evas_string_char_next_get(const char *str, int pos, int *decoded)
{
   int p, d;

   if ((!str) || (pos < 0))
     {
        if (decoded) *decoded = 0;
        return 0;
     }
   p = pos;
   d = eina_unicode_utf8_next_get(str, &p);
   if (decoded) *decoded = d;
   return p;
}

EAPI int
evas_string_char_prev_get(const char *str, int pos, int *decoded)
{
   int p, d;

   if (decoded) *decoded = 0;
   if ((!str) || (pos < 1)) return 0;
   p = pos;
   d = eina_unicode_utf8_get_prev(str, &p);
   if (decoded) *decoded = d;
   return p;
}

EAPI int
evas_string_char_len_get(const char *str)
{
   if (!str) return 0;
   return eina_unicode_utf8_get_len(str);
}

void
evas_text_style_pad_get(Evas_Text_Style_Type style, int *l, int *r, int *t, int *b)
{
   int sl = 0, sr = 0, st = 0, sb = 0;

   /* Don't calc anything if there's no style. */
   if (style != EVAS_TEXT_STYLE_PLAIN)
     {
        int shad_sz = 0, shad_dst = 0, out_sz = 0;
        int dx = 0, minx = 0, maxx = 0;
        int dy = 0, miny = 0, maxy = 0;
        Eina_Bool have_shadow = EINA_FALSE;

        switch (style & EVAS_TEXT_STYLE_MASK_BASIC)
          {
           case EVAS_TEXT_STYLE_SHADOW:
              shad_dst = 1;
              have_shadow = EINA_TRUE;
              break;
           case EVAS_TEXT_STYLE_OUTLINE_SHADOW:
           case EVAS_TEXT_STYLE_FAR_SHADOW:
              shad_dst = 2;
              out_sz = 1;
              have_shadow = EINA_TRUE;
              break;
           case EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW:
              shad_dst = 1;
              shad_sz = 2;
              out_sz = 1;
              have_shadow = EINA_TRUE;
              break;
           case EVAS_TEXT_STYLE_FAR_SOFT_SHADOW:
              shad_dst = 2;
              shad_sz = 2;
              have_shadow = EINA_TRUE;
              break;
           case EVAS_TEXT_STYLE_SOFT_SHADOW:
              shad_dst = 1;
              shad_sz = 2;
              have_shadow = EINA_TRUE;
              break;
           case EVAS_TEXT_STYLE_GLOW:
           case EVAS_TEXT_STYLE_SOFT_OUTLINE:
              out_sz = 2;
              break;
           case EVAS_TEXT_STYLE_OUTLINE:
              out_sz = 1;
              break;
           default:
              break;
          }

        minx = -out_sz;
        maxx = out_sz;
        miny = -out_sz;
        maxy = out_sz;
        if (have_shadow)
          {
             int shx1, shx2, shy1, shy2;
             switch (style & EVAS_TEXT_STYLE_MASK_SHADOW_DIRECTION)
               {
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT:
                   dx = 1;
                   dy = 1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM:
                   dx = 0;
                   dy = 1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT:
                   dx = -1;
                   dy = 1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT:
                   dx = -1;
                   dy = 0;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT:
                   dx = -1;
                   dy = -1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP:
                   dx = 0;
                   dy = -1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT:
                   dx = 1;
                   dy = -1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT:
                   dx = 1;
                   dy = 0;
                default:
                   break;
               }
             shx1 = dx * shad_dst;
             shx1 -= shad_sz;
             shx2 = dx * shad_dst;
             shx2 += shad_sz;
             if (shx1 < minx) minx = shx1;
             if (shx2 > maxx) maxx = shx2;

             shy1 = dy * shad_dst;
             shy1 -= shad_sz;
             shy2 = dy * shad_dst;
             shy2 += shad_sz;
             if (shy1 < miny) miny = shy1;
             if (shy2 > maxy) maxy = shy2;
          }

        if (l) sl = *l;
        if (r) sr = *r;
        if (t) st = *t;
        if (b) sb = *b;

        if (sr < maxx) sr = maxx;
        if (sl < -minx) sl = -minx;
        if (sb < maxy) sb = maxy;
        if (st < -miny) st = -miny;
     }

   if (l) *l = sl;
   if (r) *r = sr;
   if (t) *t = st;
   if (b) *b = sb;
}

/* all nice and private */
static void
evas_object_text_init(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = efl_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;

   Evas_Text_Data *o = obj->private_data;
   /* alloc obj private data */
   o->prev.ellipsis = o->cur.ellipsis = -1.0;
   o->prev.bitmap_scalable = o->cur.bitmap_scalable = EFL_TEXT_FONT_BITMAP_SCALABLE_COLOR;
   o->prev = o->cur;
#ifdef BIDI_SUPPORT
   o->bidi_par_props = evas_bidi_paragraph_props_new();
   o->inherit_paragraph_direction = EINA_TRUE;
#endif
}

EOLIAN static void
_evas_text_efl_object_destructor(Eo *eo_obj, Evas_Text_Data *o EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_text_free(eo_obj, obj);
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

static void
evas_object_text_free(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Text_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);

   /* free obj */
   _evas_object_text_items_clear(o);
   if (o->cur.utf8_text) eina_stringshare_del(o->cur.utf8_text);
   if (o->cur.font) eina_stringshare_del(o->cur.font);
   if (o->cur.fdesc) evas_font_desc_unref(o->cur.fdesc);
   if (o->cur.source) eina_stringshare_del(o->cur.source);
   if (o->bidi_delimiters) eina_stringshare_del(o->bidi_delimiters);
   if (o->cur.text) free(o->cur.text);
   if (obj->layer && obj->layer->evas)
      evas_font_free(o->font);
   o->font = NULL;
   o->cur.utf8_text = NULL;
   o->cur.font = NULL;
   o->cur.fdesc = NULL;
   o->cur.source = NULL;
   o->bidi_delimiters = NULL;
   o->cur.text = NULL;
   o->prev = o->cur;
#ifdef BIDI_SUPPORT
   evas_bidi_paragraph_props_unref(o->bidi_par_props);
   o->bidi_par_props = NULL;
#endif
}

void
evas_font_draw_async_check(Evas_Object_Protected_Data *obj,
                           void *engine, void *data, void *context, void *surface,
                           Evas_Font_Set *font,
                           int x, int y, int w, int h, int ow, int oh,
                           Evas_Text_Props *intl_props, Eina_Bool do_async)
{
   Eina_Bool async_unref;

   async_unref = obj->layer->evas->engine.func->font_draw(engine, data, context, surface,
                                                          font, x, y, w, h, ow, oh,
                                                          intl_props, do_async);
   if (do_async && async_unref)
     {
        evas_common_font_glyphs_ref(intl_props->glyphs);
        evas_unref_queue_glyph_put(obj->layer->evas, intl_props->glyphs);
     }
}

/* ugly binding between evas_fitler_mixin.c and this object */

EOLIAN static void
_evas_text_efl_canvas_filter_internal_filter_dirty(Eo *eo_obj, Evas_Text_Data *o)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   _evas_object_text_items_clear(o);
   o->changed = 1;
   evas_object_change(eo_obj, obj);
   _evas_object_text_recalc(eo_obj, o->cur.text);
}

EOLIAN static Eina_Bool
_evas_text_efl_canvas_filter_internal_filter_input_alpha(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o EINA_UNUSED)
{
   return EINA_TRUE;
}

EOLIAN static void
_evas_text_efl_canvas_filter_internal_filter_state_prepare(Eo *eo_obj, Evas_Text_Data *o,
                                                           Efl_Canvas_Filter_State *state, void *data EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

#define STATE_COLOR(dst, src) dst.r = src.r; dst.g = src.g; dst.b = src.b; dst.a = src.a
   STATE_COLOR(state->color, obj->cur->color);
   STATE_COLOR(state->text.glow, o->cur.glow);
   STATE_COLOR(state->text.glow2, o->cur.glow2);
   STATE_COLOR(state->text.shadow, o->cur.shadow);
   STATE_COLOR(state->text.outline, o->cur.outline);
#undef STATE_COLOR

   state->w = obj->cur->geometry.w;
   state->h = obj->cur->geometry.h;
   state->scale = obj->cur->scale;
}

EOLIAN static Eina_Bool
_evas_text_efl_canvas_filter_internal_filter_input_render(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o,
                                                          void *_filter, void *engine, void *output, void *drawctx, void *draw EINA_UNUSED,
                                                          int l, int r EINA_UNUSED, int t, int b EINA_UNUSED,
                                                          int x, int y,
                                                          Eina_Bool do_async)
{
   Evas_Filter_Context *filter = _filter;
   Evas_Object_Text_Item *it;

   EINA_INLIST_FOREACH(EINA_INLIST_GET(o->items), it)
     if ((o->font) && (it->text_props.len > 0))
       {
          if (!evas_filter_font_draw(filter, engine, output, drawctx,
                                     EVAS_FILTER_BUFFER_INPUT_ID, o->font,
                                     x + l + it->x,
                                     y + t + (int) o->max_ascent,
                                     &it->text_props,
                                     do_async))
            return EINA_FALSE;
       }

   return EINA_TRUE;
}

static void
evas_object_text_render(Evas_Object *eo_obj,
                        Evas_Object_Protected_Data *obj,
                        void *type_private_data,
                        void *engine, void *output, void *context, void *surface,
                        int x, int y, Eina_Bool do_async)
{
   int i, j;
   Evas_Text_Data *o = type_private_data;
   Evas_Object_Text_Item *it;
   const char vals[5][5] =
     {
        {0, 1, 2, 1, 0},
        {1, 3, 4, 3, 1},
        {2, 4, 5, 4, 2},
        {1, 3, 4, 3, 1},
        {0, 1, 2, 1, 0}
     };
   int sl = 0, st = 0;
   int shad_dst = 0, shad_sz = 0, dx = 0, dy = 0, haveshad = 0;

   /* render object to surface with context, and offxet by x,y */
   _evas_object_text_pad_get(eo_obj, o, &sl, NULL, &st, NULL);
   ENFN->context_multiplier_unset(engine, context);
   ENFN->context_render_op_set(engine, context, obj->cur->render_op);
   /* FIXME: This clipping is just until we fix inset handling correctly. */
   ENFN->context_clip_clip(engine, context,
                           obj->cur->geometry.x + x,
                           obj->cur->geometry.y + y,
                           obj->cur->geometry.w,
                           obj->cur->geometry.h);

/*
   ENFN->context_color_set(output,
                           context,
                           230, 160, 30, 100);
   ENFN->rectangle_draw(output,
                        context,
                        surface,
                        obj->cur->geometry.x + x,
                        obj->cur->geometry.y + y,
                        obj->cur->geometry.w,
                        obj->cur->geometry.h);
 */
#define COLOR_ONLY_SET(object, sub, col) \
    ENFN->context_color_set(engine, context, \
                object->sub.col.r, \
                object->sub.col.g, \
                object->sub.col.b, \
                object->sub.col.a);

#define COLOR_SET(object, sub, col) \
        if (obj->cur->clipper)\
          { \
             ENFN->context_color_set(engine, context, \
                ((int)object->sub.col.r * ((int)obj->cur->clipper->cur->cache.clip.r + 1)) >> 8, \
                ((int)object->sub.col.g * ((int)obj->cur->clipper->cur->cache.clip.g + 1)) >> 8, \
                ((int)object->sub.col.b * ((int)obj->cur->clipper->cur->cache.clip.b + 1)) >> 8, \
                ((int)object->sub.col.a * ((int)obj->cur->clipper->cur->cache.clip.a + 1)) >> 8); \
          } \
        else\
          ENFN->context_color_set(engine, context, \
                object->sub.col.r, \
                object->sub.col.g, \
                object->sub.col.b, \
                object->sub.col.a);

#define COLOR_SET_AMUL(object, sub, col, amul) \
        if (obj->cur->clipper) \
          { \
             ENFN->context_color_set(engine, context, \
                (((int)object->sub.col.r) * ((int)obj->cur->clipper->cur->cache.clip.r) * (amul)) / 65025, \
                (((int)object->sub.col.g) * ((int)obj->cur->clipper->cur->cache.clip.g) * (amul)) / 65025, \
                (((int)object->sub.col.b) * ((int)obj->cur->clipper->cur->cache.clip.b) * (amul)) / 65025, \
                (((int)object->sub.col.a) * ((int)obj->cur->clipper->cur->cache.clip.a) * (amul)) / 65025); \
          } \
        else \
          ENFN->context_color_set(engine, context, \
                (((int)object->sub.col.r) * (amul)) / 255, \
                (((int)object->sub.col.g) * (amul)) / 255, \
                (((int)object->sub.col.b) * (amul)) / 255, \
                (((int)object->sub.col.a) * (amul)) / 255);

#define DRAW_TEXT(ox, oy) \
   if ((o->font) && (it->text_props.len > 0)) {                         \
      ENFN->context_cutout_target(engine, context,              \
                                  obj->cur->geometry.x + x + sl + ox + it->x, \
                                  obj->cur->geometry.y + y + st + oy,   \
                                  it->w, it->h);                        \
      evas_font_draw_async_check(obj, engine,                           \
                                 output,                                \
                                 context,                               \
                                 surface,                               \
                                 o->font,                               \
                                 obj->cur->geometry.x + x + sl + ox + it->x, \
                                 obj->cur->geometry.y + y + st + oy +   \
                                 (int)o->max_ascent,                    \
                                 obj->cur->geometry.w,                  \
                                 obj->cur->geometry.h,                  \
                                 obj->cur->geometry.w,                  \
                                 obj->cur->geometry.h,                  \
                                 &it->text_props,                       \
                                 do_async);                             \
   }

   if (o->has_filter)
     {
        if (evas_filter_object_render(eo_obj, obj,
                                      engine, output, context, surface,
                                      x, y, do_async, EINA_TRUE))
          return;
     }

   /* shadows */
   switch (o->cur.style & EVAS_TEXT_STYLE_MASK_BASIC)
     {
      case EVAS_TEXT_STYLE_SHADOW:
         shad_dst = 1;
         haveshad = 1;
         break;
      case EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW:
         shad_dst = 1;
         shad_sz = 2;
         haveshad = 1;
         break;
      case EVAS_TEXT_STYLE_OUTLINE_SHADOW:
      case EVAS_TEXT_STYLE_FAR_SHADOW:
         shad_dst = 2;
         haveshad = 1;
         break;
      case EVAS_TEXT_STYLE_FAR_SOFT_SHADOW:
         shad_dst = 2;
         shad_sz = 2;
         haveshad = 1;
         break;
      case EVAS_TEXT_STYLE_SOFT_SHADOW:
         shad_dst = 1;
         shad_sz = 2;
         haveshad = 1;
         break;
      default:
         break;
     }
   if (haveshad)
     {
        if (shad_dst > 0)
          {
             switch (o->cur.style & EVAS_TEXT_STYLE_MASK_SHADOW_DIRECTION)
               {
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_RIGHT:
                   dx = 1;
                   dy = 1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM:
                   dx = 0;
                   dy = 1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_BOTTOM_LEFT:
                   dx = -1;
                   dy = 1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_LEFT:
                   dx = -1;
                   dy = 0;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_LEFT:
                   dx = -1;
                   dy = -1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP:
                   dx = 0;
                   dy = -1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_TOP_RIGHT:
                   dx = 1;
                   dy = -1;
                   break;
                case EVAS_TEXT_STYLE_SHADOW_DIRECTION_RIGHT:
                   dx = 1;
                   dy = 0;
                default:
                   break;
               }
             dx *= shad_dst;
             dy *= shad_dst;
          }
     }
   EINA_INLIST_FOREACH(EINA_INLIST_GET(o->items), it)
     {
        ENFN->context_multiplier_set(engine, context, 0, 0, 0, 0);
        /* Shadows */
        if (haveshad)
          {
             switch (shad_sz)
               {
                case 0:
                  COLOR_SET(o, cur, shadow);
                  DRAW_TEXT(dx, dy);
                  break;
                case 2:
                  for (j = 0; j < 5; j++)
                    {
                       for (i = 0; i < 5; i++)
                         {
                            if (vals[i][j] != 0)
                              {
                                 COLOR_SET_AMUL(o, cur, shadow, vals[i][j] * 50);
                                 DRAW_TEXT(i - 2 + dx, j - 2 + dy);
                              }
                         }
                    }
                  break;
                default:
                  break;
               }
          }

        /* glows */
        if (o->cur.style == EVAS_TEXT_STYLE_GLOW)
          {
             for (j = 0; j < 5; j++)
               {
                  for (i = 0; i < 5; i++)
                    {
                       if (vals[i][j] != 0)
                         {
                            COLOR_SET_AMUL(o, cur, glow, vals[i][j] * 50);
                            DRAW_TEXT(i - 2, j - 2);
                         }
                    }
               }
             COLOR_SET(o, cur, glow2);
             DRAW_TEXT(-1, 0);
             DRAW_TEXT(1, 0);
             DRAW_TEXT(0, -1);
             DRAW_TEXT(0, 1);
          }

        /* outlines */
        if (((o->cur.style & EVAS_TEXT_STYLE_MASK_BASIC) == EVAS_TEXT_STYLE_OUTLINE) ||
            ((o->cur.style & EVAS_TEXT_STYLE_MASK_BASIC) == EVAS_TEXT_STYLE_OUTLINE_SHADOW) ||
            ((o->cur.style & EVAS_TEXT_STYLE_MASK_BASIC) == EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW))
          {
             COLOR_SET(o, cur, outline);
             DRAW_TEXT(-1, 0);
             DRAW_TEXT(1, 0);
             DRAW_TEXT(0, -1);
             DRAW_TEXT(0, 1);
          }
        else if (o->cur.style == EVAS_TEXT_STYLE_SOFT_OUTLINE)
          {
             for (j = 0; j < 5; j++)
               {
                  for (i = 0; i < 5; i++)
                    {
                       if (((i != 2) || (j != 2)) && (vals[i][j] != 0))
                         {
                            COLOR_SET_AMUL(o, cur, outline, vals[i][j] * 50);
                            DRAW_TEXT(i - 2, j - 2);
                         }
                    }
               }
          }

        /* normal text */
        ENFN->context_multiplier_unset(engine, context);

        if (obj->cur->clipper)
          ENFN->context_multiplier_set(engine, context,
                                       obj->cur->clipper->cur->cache.clip.r,
                                       obj->cur->clipper->cur->cache.clip.g,
                                       obj->cur->clipper->cur->cache.clip.b,
                                       obj->cur->clipper->cur->cache.clip.a);

        COLOR_ONLY_SET(obj, cur->cache, clip);
        DRAW_TEXT(0, 0);
        ENFN->context_multiplier_unset(engine, context);
     }
}

static void
evas_object_text_render_pre(Evas_Object *eo_obj,
			    Evas_Object_Protected_Data *obj,
			    void *type_private_data)
{
   Evas_Text_Data *o = type_private_data;
   int is_v = 0, was_v = 0;
   /* dont pre-render the obj twice! */
   if (obj->pre_render_done) return;
   obj->pre_render_done = EINA_TRUE;
   /* pre-render phase. this does anything an object needs to do just before
    rendering. This could mean loading the image data, retrieving it from
    elsewhere, decoding video etc.
    Then when this is done the object needs to figure if it changed and
    if so what and where and add the appropriate redraw rectangles */
   /* if someone is clipping this obj - go calculate the clipper */
   if (obj->cur->clipper)
     {
        if (obj->cur->cache.clip.dirty)
          {
             evas_object_clip_recalc(obj->cur->clipper);
          }
        obj->cur->clipper->func->render_pre(obj->cur->clipper->object,
                        obj->cur->clipper,
                        obj->cur->clipper->private_data);
     }

#ifdef BIDI_SUPPORT
   if (o->inherit_paragraph_direction)
     {
        Evas_BiDi_Direction parent_dir = EVAS_BIDI_DIRECTION_NEUTRAL;

        if (obj->smart.parent)
          parent_dir = evas_object_paragraph_direction_get(obj->smart.parent);

        if (parent_dir != o->paragraph_direction)
          {
             o->paragraph_direction = parent_dir;
             o->changed_paragraph_direction = EINA_TRUE;
          }
     }
#endif

   /* If object size changed and ellipsis is set */
   if (((o->cur.ellipsis >= 0.0) &&
       ((obj->cur->geometry.w != o->last_computed.w) ||
       (obj->cur->geometry.h != o->last_computed.h))) ||
       (!EINA_DBL_EQ(o->cur.ellipsis, o->prev.ellipsis)) ||
       (!EINA_DBL_EQ(obj->cur->scale, obj->prev->scale)) ||
       (o->changed_paragraph_direction))
     {
        _evas_object_text_recalc(eo_obj, o->cur.text);
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                        eo_obj, obj);
        goto done;
     }
   /* now figure what changed and add draw rects
    if it just became visible or invisible */
   is_v = evas_object_is_visible(eo_obj, obj);
   was_v = evas_object_was_visible(eo_obj, obj);
   if (is_v != was_v)
     {
        evas_object_render_pre_visible_change(&obj->layer->evas->clip_changes,
                          eo_obj, is_v, was_v);
        goto done;
     }
   if (obj->changed_map || obj->changed_src_visible)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                          eo_obj, obj);
        goto done;
     }
   /* its not visible - we accounted for it appearing or not so just abort */
   if (!is_v) goto done;
   /* clipper changed this is in addition to anything else for obj */
   evas_object_render_pre_clipper_change(&obj->layer->evas->clip_changes, eo_obj);
   /* if we restacked (layer or just within a layer) and dont clip anyone */
   if (obj->restack)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                          eo_obj, obj);
        goto done;
     }
   /* if it changed color */
   if ((obj->cur->color.r != obj->prev->color.r) ||
       (obj->cur->color.g != obj->prev->color.g) ||
       (obj->cur->color.b != obj->prev->color.b) ||
       (obj->cur->color.a != obj->prev->color.a))
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                          eo_obj, obj);
        goto done;
     }
   /* if it changed geometry - and obviously not visibility or color
    calculate differences since we have a constant color fill
    we really only need to update the differences */
   if ((obj->cur->geometry.x != obj->prev->geometry.x) ||
       (obj->cur->geometry.y != obj->prev->geometry.y) ||
       (obj->cur->geometry.w != obj->prev->geometry.w) ||
       (obj->cur->geometry.h != obj->prev->geometry.h))
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                         eo_obj, obj);
        goto done;
     }
   if (obj->cur->render_op != obj->prev->render_op)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                         eo_obj, obj);
        goto done;
     }
   if (o->changed)
     {
        evas_object_render_pre_prev_cur_add(&obj->layer->evas->clip_changes,
                                            eo_obj, obj);
        goto done;
     }
   done:
   evas_object_render_pre_effect_updates(&obj->layer->evas->clip_changes,
                     eo_obj, is_v, was_v);
}

static void
evas_object_text_render_post(Evas_Object *eo_obj EINA_UNUSED,
                             Evas_Object_Protected_Data *obj,
                             void *type_private_data EINA_UNUSED)
{
   /* this moves the current data to the previous state parts of the object
    in whatever way is safest for the object. also if we don't need object
    data anymore we can free it if the object deems this is a good idea */
   /* remove those pesky changes */
   evas_object_clip_changes_clean(obj);
   /* move cur to prev safely for object data */
   evas_object_cur_prev(obj);
}

static unsigned int
evas_object_text_id_get(Evas_Object *eo_obj)
{
   Evas_Text_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_TEXT;
}

static unsigned int
evas_object_text_visual_id_get(Evas_Object *eo_obj)
{
   Evas_Text_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_SHAPE;
}

static void *
evas_object_text_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Text_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return NULL;
   return o->font;
}

static int
evas_object_text_is_opaque(Evas_Object *eo_obj EINA_UNUSED,
                           Evas_Object_Protected_Data *obj EINA_UNUSED,
                           void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object is
    currently fully opaque over the entire gradient it occupies */
   return 0;
}

static int
evas_object_text_was_opaque(Evas_Object *eo_obj EINA_UNUSED,
                            Evas_Object_Protected_Data *obj EINA_UNUSED,
                            void *type_private_data EINA_UNUSED)
{
   /* this returns 1 if the internal object data implies that the object was
    currently fully opaque over the entire gradient it occupies */
   return 0;
}

EOLIAN static void
_evas_text_efl_gfx_entity_scale_set(Evas_Object *eo_obj, Evas_Text_Data *o,
                                 double scale)
{
   int size;
   const char *font;

   if (EINA_DBL_EQ(efl_gfx_entity_scale_get(eo_obj), scale)) return;
   efl_gfx_entity_scale_set(efl_super(eo_obj, MY_CLASS), scale);

   font = eina_stringshare_add(o->cur.font);
   size = o->cur.size;
   if (o->cur.font) eina_stringshare_del(o->cur.font);
   o->cur.font = NULL;
   o->prev.font = NULL;
   o->cur.size = 0;
   o->prev.size = 0;
   evas_object_text_font_set(eo_obj, font, size);
}

void
_evas_object_text_rehint(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Text_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);
   Eina_List *was = NULL;

   if (!o->font) return;
   evas_font_load_hinting_set(o->font, obj->layer->evas->hinting);
   was = _evas_pointer_list_in_rect_get(obj->layer->evas, eo_obj, obj, 1, 1);
   /* DO II */
   _evas_object_text_recalc(eo_obj, o->cur.text);
   o->changed = 1;
   if (o->has_filter)
     evas_filter_changed_set(eo_obj, EINA_TRUE);
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   if (obj->cur->visible)
     _evas_canvas_event_pointer_in_list_mouse_move_feed(obj->layer->evas, was, eo_obj, obj, 1, 1, EINA_FALSE, NULL);
   eina_list_free(was);
   evas_object_inform_call_resize(eo_obj, obj);
}

static void
_evas_object_text_recalc(Evas_Object *eo_obj, Eina_Unicode *text)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   Evas_Text_Data *o = efl_data_scope_get(eo_obj, MY_CLASS);

   if (!text) text = eina_unicode_strdup(EINA_UNICODE_EMPTY_STRING);

   _evas_object_text_layout(eo_obj, o, text);

   /* Calc ascent/descent. */
   if (o->items)
     {
/*
        Evas_Object_Text_Item *item;

        for (item = o->items ; item ;
              item = EINA_INLIST_CONTAINER_GET(
                 EINA_INLIST_GET(item)->next, Evas_Object_Text_Item))
          {
             int asc = 0, desc = 0;

             // Skip items without meaning full information.
             if (!item->text_props.font_instance) continue;

             asc = evas_common_font_instance_ascent_get(item->text_props.font_instance);
             desc = evas_common_font_instance_descent_get(item->text_props.font_instance);
             if (asc > o->ascent) o->ascent = asc;
             if (desc > o->descent) o->descent = desc;

             asc = evas_common_font_instance_max_ascent_get(item->text_props.font_instance);
             desc = evas_common_font_instance_max_descent_get(item->text_props.font_instance);
             if (asc > o->max_ascent) o->max_ascent = asc;
             if (desc > o->max_descent) o->max_descent = desc;
          }
 */
        if (o->font)
          {
             o->ascent = ENFN->font_ascent_get(ENC, o->font);
             o->descent = ENFN->font_descent_get(ENC, o->font);
             o->max_ascent = ENFN->font_max_ascent_get(ENC, o->font);
             o->max_descent = ENFN->font_max_descent_get(ENC, o->font);
          }
     }
   else if (o->font)
     {
        o->ascent = ENFN->font_ascent_get(ENC, o->font);
        o->descent = ENFN->font_descent_get(ENC, o->font);
        o->max_ascent = ENFN->font_max_ascent_get(ENC, o->font);
        o->max_descent = ENFN->font_max_descent_get(ENC, o->font);
     }

   if ((o->font) && (o->items))
     {
        int w, h;
        int l = 0, r = 0, t = 0, b = 0;

        w = _evas_object_text_horiz_width_without_ellipsis_get(o);
        h = _evas_object_text_vert_advance_get(eo_obj, o);
        _evas_object_text_pad_get(eo_obj, o, &l, &r, &t, &b);

        if (o->cur.ellipsis >= 0.0)
          {
             int min;

             min = w + l + r < obj->cur->geometry.w || obj->cur->geometry.w == 0 ? w + l + r : obj->cur->geometry.w;
             efl_gfx_entity_size_set(efl_super(eo_obj, MY_CLASS), EINA_SIZE2D(min,  h + t + b));
          }
        else
          {
             efl_gfx_entity_size_set(efl_super(eo_obj, MY_CLASS), EINA_SIZE2D(w + l + r,  h + t + b));
          }
////        obj->cur->cache.geometry.validity = 0;
     }
   else
     {
        int t = 0, b = 0, l = 0, r = 0;
        _evas_object_text_pad_get(eo_obj, o, &l, &r, &t, &b);
        efl_gfx_entity_size_set(efl_super(eo_obj, MY_CLASS), EINA_SIZE2D(0,  o->max_ascent + o->max_descent + t + b));
////        obj->cur->cache.geometry.validity = 0;
     }
   o->last_computed.w = obj->cur->geometry.w;
   o->last_computed.h = obj->cur->geometry.h;
#ifdef BIDI_SUPPORT
   o->changed_paragraph_direction = EINA_FALSE;
#endif
}

EAPI void
evas_object_text_font_source_set(Eo *obj, const char *font_source)
{
   efl_text_font_source_set((Eo *) obj, font_source);
}

EAPI const char *
evas_object_text_font_source_get(const Eo *obj)
{
   const char *font_source = 0;
   font_source = efl_text_font_source_get((Eo *) obj);
   return font_source;
}

EAPI void
evas_object_text_font_set(Eo *obj, const char *font, Evas_Font_Size size)
{
   efl_text_font_set((Eo *) obj, font, size);
}

EAPI void
evas_object_text_font_get(const Eo *obj, const char **font, Evas_Font_Size *size)
{
   efl_text_font_get((Eo *) obj, font, size);
}

EAPI void
evas_object_text_text_set(Eo *obj, const char *text)
{
   efl_text_set((Eo *) obj, text);
}

EAPI const char *
evas_object_text_text_get(const Eo *obj)
{
   return efl_text_get((Eo *) obj);
}

EOLIAN static void
_evas_text_efl_gfx_filter_filter_program_set(Eo *obj, Evas_Text_Data *pd EINA_UNUSED, const char *code, const char *name)
{
   pd->has_filter = (code != NULL);
   efl_gfx_filter_program_set(efl_super(obj, MY_CLASS), code, name);
}

/* deprecated */
EAPI void
evas_object_text_filter_program_set(Evas_Object *obj, const char *code)
{
   efl_gfx_filter_program_set(obj, code, NULL);
}

/* deprecated */
EAPI void
evas_object_text_filter_source_set(Evas_Object *obj, const char *name, Evas_Object *source)
{
   efl_gfx_filter_source_set(obj, name, source);
}

EOLIAN static void
_evas_text_efl_canvas_object_paragraph_direction_set(Eo *eo_obj, Evas_Text_Data *o,
                                                     Evas_BiDi_Direction dir)
{
#ifdef BIDI_SUPPORT
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);

   if ((!(o->inherit_paragraph_direction) && (o->paragraph_direction == dir)) ||
       (o->inherit_paragraph_direction && (dir == EVAS_BIDI_DIRECTION_INHERIT)))
     return;

   if (dir == EVAS_BIDI_DIRECTION_INHERIT)
     {
        o->inherit_paragraph_direction = EINA_TRUE;
        Evas_BiDi_Direction parent_dir = EVAS_BIDI_DIRECTION_NEUTRAL;

        if (obj->smart.parent)
          parent_dir = evas_object_paragraph_direction_get(obj->smart.parent);

        if (parent_dir != o->paragraph_direction)
          {
             o->paragraph_direction = parent_dir;
             o->changed_paragraph_direction = EINA_TRUE;
             evas_object_change(eo_obj, obj);
          }
     }
   else
     {
        o->inherit_paragraph_direction = EINA_FALSE;
        o->paragraph_direction = dir;
        o->changed_paragraph_direction = EINA_TRUE;
        evas_object_change(eo_obj, obj);
     }
#else
   (void) eo_obj;
   (void) o;
   (void) dir;
#endif
}

EOLIAN static Evas_BiDi_Direction
_evas_text_efl_canvas_object_paragraph_direction_get(const Eo *eo_obj EINA_UNUSED,
                                                     Evas_Text_Data *o)
{
   return o->paragraph_direction;
}

EOLIAN static void
_evas_text_efl_text_font_font_bitmap_scalable_set(Eo *eo_obj, Evas_Text_Data *o, Efl_Text_Font_Bitmap_Scalable bitmap_scalable)
{
   if (o->cur.bitmap_scalable == bitmap_scalable) return;
   o->prev.bitmap_scalable = o->cur.bitmap_scalable;
   o->cur.bitmap_scalable = bitmap_scalable;
   _evas_text_font_reload(eo_obj, o);
}

EOLIAN static Efl_Text_Font_Bitmap_Scalable
_evas_text_efl_text_font_font_bitmap_scalable_get(const Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->cur.bitmap_scalable;
}

#define EVAS_TEXT_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_dbg_info_get, _evas_text_efl_object_dbg_info_get)

#include "canvas/evas_text.eo.c"
