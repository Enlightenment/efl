#include "evas_common_private.h" /* Includes evas_bidi_utils stuff. */
#include "evas_private.h"
#include "evas_filter.h"

#define MY_CLASS EVAS_TEXT_CLASS

#define MY_CLASS_NAME "Evas_Text"

#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif

/* save typing */
#define ENFN obj->layer->evas->engine.func
#define ENDT obj->layer->evas->engine.data.output

/* private magic number for text objects */
static const char o_type[] = "text";

/* private struct for text object internal data */
typedef struct _Evas_Text_Data Evas_Text_Data;
typedef struct _Evas_Object_Text_Item Evas_Object_Text_Item;

struct _Evas_Text_Data
{
   DATA32               magic;

   struct {
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

      // special effects. VERY EXPERIMENTAL for now.
      struct {
         Eina_Stringshare    *code;
         Evas_Filter_Program *chain;
         Eina_Hash           *sources; // Evas_Filter_Proxy_Binding
         int                  sources_count;
         void                *output;
         Eina_Bool            changed : 1;
         Eina_Bool            invalid : 1; // Code parse failed
      } filter;
   } cur, prev;

   struct {
      Evas_Object_Text_Item    *ellipsis_start;
      Evas_Object_Text_Item    *ellipsis_end;
      Evas_Coord                w, h;
      int                       advance;
      int                       advance_without_ellipsis;
      Eina_Bool                 ellipsis;
   } last_computed;

   Evas_BiDi_Paragraph_Props  *bidi_par_props;
   const char                 *bidi_delimiters;
   Evas_Object_Text_Item      *items;

   Evas_Font_Set              *font;

   float                       ascent, descent;
   float                       max_ascent, max_descent;

   Evas_BiDi_Direction         bidi_dir : 2;
   char                        changed : 1;
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
				    void *output, void *context, void *surface, int x, int y, Eina_Bool do_async);
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

static void evas_object_text_scale_update(Evas_Object *eo_obj,
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
     evas_object_text_scale_update,
     NULL,
     NULL,
     NULL
};

/* the actual api call to add a rect */
/* it has no other api calls as all properties are standard */

static int
_evas_object_text_char_coords_get(const Evas_Object *eo_obj,
      const Evas_Text_Data *o,
      size_t pos, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Object_Text_Item *it;

   EINA_INLIST_FOREACH(EINA_INLIST_GET(o->items), it)
     {
        if ((it->text_pos <= pos) &&
              (pos < (it->text_pos + it->text_props.text_len)))
          {
             int ret;
             ret = ENFN->font_char_coords_get(ENDT, o->font,
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

static void
_evas_object_text_items_clean(Evas_Object_Protected_Data *obj, Evas_Text_Data *o)
{
   /* FIXME: also preserve item */
   if ((o->cur.font == o->prev.font) &&
       (o->cur.fdesc == o->prev.fdesc) &&
       (o->cur.size == o->prev.size) &&
       (!memcmp(&o->cur.outline, &o->prev.outline, sizeof (o->cur.outline))) &&
       (!memcmp(&o->cur.shadow, &o->prev.shadow, sizeof (o->cur.shadow))) &&
       (!memcmp(&o->cur.glow, &o->prev.glow, sizeof (o->cur.glow))) &&
       (!memcmp(&o->cur.glow2, &o->prev.glow2, sizeof (o->cur.glow2))) &&
       (o->cur.style == o->prev.style) &&
       (obj->cur->scale == obj->prev->scale))
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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Object_Text_Item *it;

#ifdef BIDI_SUPPORT
   /*FIXME: not very efficient, sort the items arrays. */
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
                  return it->text_pos + ENFN->font_last_up_to_pos(ENDT,
                        o->font,
                        &it->text_props,
                        cx - x,
                        cy);
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
                  return it->text_pos + ENFN->font_last_up_to_pos(ENDT,
                        o->font,
                        &it->text_props,
                        cx - it->x,
                        cy);
               }
          }
     }
   return -1;
}

static int
_evas_object_text_char_at_coords(const Evas_Object *eo_obj,
      const Evas_Text_Data *o, Evas_Coord cx, Evas_Coord cy,
      Evas_Coord *rx, Evas_Coord *ry, Evas_Coord *rw, Evas_Coord *rh)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Object_Text_Item *it;

   EINA_INLIST_FOREACH(EINA_INLIST_GET(o->items), it)
     {
        if ((it->x <= cx) && (cx < it->x + it->adv))
          {
             return it->text_pos + ENFN->font_char_at_coords_get(ENDT,
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
_evas_object_text_horiz_advance_without_ellipsis_get(const Evas_Text_Data *o)
{
   return o->last_computed.advance_without_ellipsis;
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
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Object *eo_obj = eo_add(EVAS_TEXT_CLASS, e);
   eo_unref(eo_obj);
   return eo_obj;
}

EOLIAN static void
_evas_text_eo_base_constructor(Eo *eo_obj, Evas_Text_Data *class_data EINA_UNUSED)
{
   eo_do_super(eo_obj, MY_CLASS, eo_constructor());
   evas_object_text_init(eo_obj);
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Eo *parent = NULL;

   eo_do(eo_obj, parent = eo_parent_get());
   evas_object_inject(eo_obj, obj, evas_object_evas_get(parent));
}

EOLIAN static void
_evas_text_font_source_set(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, const char *font_source)
{
   if ((o->cur.source) && (font_source) &&
       (!strcmp(o->cur.source, font_source)))
     return;
   /*
   if (o->cur.source) eina_stringshare_del(o->cur.source);
   if (font_source) o->cur.source = eina_stringshare_add(font_source);
   else o->cur.source = NULL;
    */
   eina_stringshare_replace(&o->cur.source, font_source);
}

EOLIAN static const char*
_evas_text_font_source_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{

   return o->cur.source;
}

EOLIAN static void
_evas_text_font_set(Eo *eo_obj, Evas_Text_Data *o, const char *font, Evas_Font_Size size)
{
   Eina_Bool is, was = EINA_FALSE;
   Eina_Bool pass = EINA_FALSE, freeze = EINA_FALSE;
   Eina_Bool source_invisible = EINA_FALSE;
   Evas_Font_Description *fdesc;


   if ((!font) || (size <= 0)) return;

   if (!(o->cur.font && !strcmp(font, o->cur.font)))
     {
        fdesc = evas_font_desc_new();
        evas_font_name_parse(fdesc, font);
     }
   else
     {
        fdesc = evas_font_desc_ref(o->cur.fdesc);
     }

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

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   if (!(obj->layer->evas->is_frozen))
     {
        pass = evas_event_passes_through(eo_obj, obj);
        freeze = evas_event_freezes_through(eo_obj, obj);
        source_invisible = evas_object_is_source_invisible(eo_obj, obj);
        if ((!pass) && (!freeze) && (!source_invisible))
          was = evas_object_is_in_output_rect(eo_obj, obj,
                                              obj->layer->evas->pointer.x,
                                              obj->layer->evas->pointer.y, 1, 1);
     }

   /* DO IT */
   if (o->font)
     {
        evas_font_free(obj->layer->evas->evas, o->font);
        o->font = NULL;
     }

   o->font = evas_font_load(obj->layer->evas->evas, o->cur.fdesc, o->cur.source,
         (int)(((double) o->cur.size) * obj->cur->scale));
     {
        o->ascent = 0;
        o->descent = 0;
        o->max_ascent = 0;
        o->max_descent = 0;
     }
   _evas_object_text_items_clear(o);
   _evas_object_text_recalc(eo_obj, o->cur.text);
   o->changed = 1;
   o->cur.filter.changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   if (!(obj->layer->evas->is_frozen))
     {
        if ((!pass) && (!freeze))
          {
             is = evas_object_is_in_output_rect(eo_obj, obj,
                                                obj->layer->evas->pointer.x,
                                                obj->layer->evas->pointer.y,
                                                1, 1);
             if ((is ^ was) && obj->cur->visible)
               evas_event_feed_mouse_move(obj->layer->evas->evas,
                                          obj->layer->evas->pointer.x,
                                          obj->layer->evas->pointer.y,
                                          obj->layer->evas->last_timestamp,
                                          NULL);
          }
     }
   evas_object_inform_call_resize(eo_obj);
}

EOLIAN static void
_evas_text_font_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, const char **font, Evas_Font_Size *size)
{
   if (font) *font = o->cur.font;
   if (size) *size = o->cur.size;
}

static void
_evas_object_text_item_update_sizes(Evas_Object_Protected_Data *obj, Evas_Text_Data *o, Evas_Object_Text_Item *it)
{
   ENFN->font_string_size_get(ENDT,
         o->font,
         &it->text_props,
         &it->w, &it->h);
   it->adv = ENFN->font_h_advance_get(ENDT, o->font,
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
        ENFN->font_text_props_info_create(ENDT,
              fi, str + pos, &it->text_props,
              o->bidi_par_props, it->text_pos, len, EVAS_TEXT_PROPS_MODE_SHAPE);
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
   /*FIXME: not very efficient, sort the items arrays. */
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
        (void) ENFN->font_run_end_get(ENDT, o->font, &script_fi, &cur_fi,
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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   EvasBiDiStrIndex *v_to_l = NULL;
   Evas_Coord advance = 0;
   size_t pos, visual_pos;
   int len = eina_unicode_strlen(text);
   int l = 0, r = 0;
#ifdef BIDI_SUPPORT
   int par_len = len;
   int *segment_idxs = NULL;
#endif

   if (o->items &&
       !memcmp(&o->cur, &o->prev, sizeof (o->cur)) &&
       o->cur.text == text &&
       obj->cur->scale == obj->prev->scale &&
       ((o->last_computed.advance <= obj->cur->geometry.w && !o->last_computed.ellipsis) ||
        o->last_computed.w == obj->cur->geometry.w))
     return;

   o->last_computed.ellipsis = EINA_FALSE;
   evas_object_content_change(eo_obj, obj);

   if (o->items) _evas_object_text_items_clean(obj, o);

   if (text && *text)
      o->bidi_dir = EVAS_BIDI_DIRECTION_LTR;
   else
      o->bidi_dir = EVAS_BIDI_DIRECTION_NEUTRAL;

#ifdef BIDI_SUPPORT
   if (o->bidi_delimiters)
      segment_idxs = evas_bidi_segment_idxs_get(text, o->bidi_delimiters);
   evas_bidi_paragraph_props_unref(o->bidi_par_props);
   o->bidi_par_props = evas_bidi_paragraph_props_get(text, len, segment_idxs);

   if (o->bidi_par_props)
      o->bidi_dir = EVAS_BIDI_PAR_TYPE_TO_DIRECTION(o->bidi_par_props->direction);

   evas_bidi_props_reorder_line(NULL, 0, len, o->bidi_par_props, &v_to_l);
   if (segment_idxs) free(segment_idxs);
#endif
   visual_pos = pos = 0;

   while (len > 0)
     {
        Evas_Font_Instance *script_fi = NULL;
        int script_len = len, tmp_cut;
        Evas_Script_Type script;
        tmp_cut = evas_common_language_script_end_of_run_get(
              text + pos,
              o->bidi_par_props,
              pos, len);
        if (tmp_cut > 0)
           script_len = tmp_cut;

        script = evas_common_language_script_type_get(text + pos, script_len);

        while (script_len > 0)
          {
             const Evas_Object_Text_Item *it;
             Evas_Font_Instance *cur_fi = NULL;
             int run_len = script_len;
             if (o->font)
               {
                  run_len = ENFN->font_run_end_get(ENDT,
                        o->font, &script_fi, &cur_fi,
                        script, text + pos, script_len);
               }
#ifdef BIDI_SUPPORT
             visual_pos = evas_bidi_position_logical_to_visual(
                   v_to_l, par_len, pos);
#else
             visual_pos = pos;
#endif
             it = _evas_object_text_item_new(obj, o, cur_fi, text, script,
                                             pos, visual_pos, run_len);

             advance += it->adv;
             pos += run_len;
             script_len -= run_len;
             len -= run_len;
          }
     }
   o->last_computed.advance_without_ellipsis = advance;

   if (!o->cur.filter.chain)
     evas_text_style_pad_get(o->cur.style, &l, &r, NULL, NULL);
   else
     evas_filter_program_padding_get(o->cur.filter.chain, &l, &r, NULL, NULL);

   /* Handle ellipsis */
   if (pos && (o->cur.ellipsis >= 0.0) && (advance + l + r > obj->cur->geometry.w) && (obj->cur->geometry.w > 0))
     {
        Evas_Coord ellip_frame = obj->cur->geometry.w;
        Evas_Object_Text_Item *start_ellip_it = NULL, *end_ellip_it = NULL;

        o->last_computed.ellipsis = EINA_TRUE;

        /* Account of the ellipsis item width. As long as ellipsis != 0
         * we have a left ellipsis. And the same with 1 and right. */
        if (o->cur.ellipsis != 0)
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
             ellip_frame -= start_ellip_it->adv;
          }
        if (o->cur.ellipsis != 1)
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
             ellip_frame -= end_ellip_it->adv;
          }

        /* The point where we should start from, going for the full
         * ellip frame. */
        Evas_Coord ellipsis_coord = o->cur.ellipsis * (advance - ellip_frame);
        if (start_ellip_it)
          {
             Evas_Object_Text_Item *itr = o->items;
             advance = 0;

             while (itr && (advance + l + r + itr->adv < ellipsis_coord))
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
                  int cut = 1 + ENFN->font_last_up_to_pos(ENDT,
                        o->font,
                        &itr->text_props,
                        ellipsis_coord - (advance + l + r),
                        0);
                  if (cut > 0)
                    {

                       start_ellip_it->text_pos = itr->text_pos;
                       start_ellip_it->visual_pos = itr->visual_pos;
                       if (!_layout_text_item_trim(obj, o, itr, cut, EINA_FALSE))
                         {
                            _evas_object_text_item_del(o, itr);
                         }
                    }
               }

             o->items = (Evas_Object_Text_Item *) eina_inlist_remove(EINA_INLIST_GET(o->items), EINA_INLIST_GET(start_ellip_it));
             o->items = (Evas_Object_Text_Item *) eina_inlist_prepend(EINA_INLIST_GET(o->items), EINA_INLIST_GET(start_ellip_it));
          }

        if (end_ellip_it)
          {
             Evas_Object_Text_Item *itr = o->items;
             advance = 0;

             while (itr)
               {
                  if (itr != end_ellip_it) /* was start_ellip_it */
                    {
                       if (advance + l + r + itr->adv >= ellip_frame)
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

             int cut = ENFN->font_last_up_to_pos(ENDT,
                   o->font,
                   &itr->text_props,
                   ellip_frame - (advance + l + r),
                   0);
             if (cut >= 0)
               {
                  end_ellip_it->text_pos = itr->text_pos + cut;
                  end_ellip_it->visual_pos = itr->visual_pos + cut;
                  if (_layout_text_item_trim(obj, o, itr, cut, EINA_TRUE))
                    {
                       itr = (Evas_Object_Text_Item *) EINA_INLIST_GET(itr)->next;
                    }
               }

             /* Remove the rest of the items */
             while (itr)
               {
                  Eina_Inlist *itrn = EINA_INLIST_GET(itr)->next;
                  if ((itr != start_ellip_it) && (itr != end_ellip_it))
		    _evas_object_text_item_del(o, itr);
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
_evas_text_evas_object_size_set(Eo *eo_obj, Evas_Text_Data *o, Evas_Coord w, Evas_Coord h)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->geometry.w = w;
        state_write->geometry.h = h;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   _evas_object_text_recalc(eo_obj, o->cur.text);
}

EOLIAN static void
_evas_text_ellipsis_set(Eo *eo_obj, Evas_Text_Data *o, double ellipsis)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   if (o->cur.ellipsis == ellipsis) return;

   o->cur.ellipsis = ellipsis;
   o->changed = 1;
   o->cur.filter.changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
}

EOLIAN static double
_evas_text_ellipsis_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->cur.ellipsis;
}

EOLIAN static void
_evas_text_eo_base_dbg_info_get(Eo *eo_obj, Evas_Text_Data *o EINA_UNUSED, Eo_Dbg_Info *root)
{
   eo_do_super(eo_obj, MY_CLASS, eo_dbg_info_get(root));
   Eo_Dbg_Info *group = EO_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);

   const char *text;
   int size;
   eo_do(eo_obj, evas_obj_text_font_get(&text, &size));
   EO_DBG_INFO_APPEND(group, "Font", EINA_VALUE_TYPE_STRING, text);
   EO_DBG_INFO_APPEND(group, "Text size", EINA_VALUE_TYPE_INT, size);

   eo_do(eo_obj, text = evas_obj_text_font_source_get());
   EO_DBG_INFO_APPEND(group, "Font source", EINA_VALUE_TYPE_STRING, text);

   eo_do(eo_obj, text = evas_obj_text_text_get());
   EO_DBG_INFO_APPEND(group, "Text", EINA_VALUE_TYPE_STRING, text);
}

EOLIAN static void
_evas_text_text_set(Eo *eo_obj, Evas_Text_Data *o, const char *_text)
{
   int is, was, len;
   Eina_Unicode *text;

   if ((o->cur.utf8_text) && (_text) && (!strcmp(o->cur.utf8_text, _text)))
      return;
   text = eina_unicode_utf8_to_unicode(_text, &len);

   if (!text) text = eina_unicode_strdup(EINA_UNICODE_EMPTY_STRING);
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   was = evas_object_is_in_output_rect(eo_obj, obj,
				       obj->layer->evas->pointer.x,
				       obj->layer->evas->pointer.y, 1, 1);
   /* DO II */
   /*Update bidi_props*/

   _evas_object_text_items_clear(o);

   _evas_object_text_recalc(eo_obj, text);
   eina_stringshare_replace(&o->cur.utf8_text, _text);
   o->prev.utf8_text = NULL;

   if (o->cur.text != text) free(text);

   o->changed = 1;
   o->cur.filter.changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   is = evas_object_is_in_output_rect(eo_obj, obj,
				      obj->layer->evas->pointer.x,
				      obj->layer->evas->pointer.y, 1, 1);
   if ((is || was) && obj->cur->visible)
     evas_event_feed_mouse_move(obj->layer->evas->evas,
				obj->layer->evas->pointer.x,
				obj->layer->evas->pointer.y,
				obj->layer->evas->last_timestamp,
				NULL);
   evas_object_inform_call_resize(eo_obj);
}

EOLIAN static void
_evas_text_bidi_delimiters_set(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, const char *delim)
{
   eina_stringshare_replace(&o->bidi_delimiters, delim);
}

EOLIAN static const char*
_evas_text_bidi_delimiters_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->bidi_delimiters;
}

EOLIAN static const char*
_evas_text_text_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->cur.utf8_text;
}

EOLIAN static Evas_BiDi_Direction
_evas_text_direction_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->bidi_dir;
}

EOLIAN static Evas_Coord
_evas_text_ascent_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->ascent;
}

EOLIAN static Evas_Coord
_evas_text_descent_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->descent;
}

EOLIAN static Evas_Coord
_evas_text_max_ascent_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->max_ascent;
}

EOLIAN static Evas_Coord
_evas_text_max_descent_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->max_descent;
}

EOLIAN static Evas_Coord
_evas_text_inset_get(Eo *eo_obj, Evas_Text_Data *o)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Coord inset = 0;
   if (!o->font) return inset;
   if (!o->items) return inset;
   inset = ENFN->font_inset_get(ENDT, o->font, &o->items->text_props);

   return inset;
}

EOLIAN static Evas_Coord
_evas_text_horiz_advance_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   Evas_Coord horiz = 0;
   if (!o->font) return horiz;
   if (!o->items) return horiz;
   horiz = _evas_object_text_horiz_advance_get(o);

   return horiz;
}

EOLIAN static Evas_Coord
_evas_text_vert_advance_get(Eo *eo_obj, Evas_Text_Data *o)
{
   Evas_Coord vert;
   vert = 0;
   if (!o->font) return vert;
   if (!o->items)
     {
        vert = o->ascent + o->descent;
        return vert;
     }
   vert = _evas_object_text_vert_advance_get(eo_obj, o);

   return vert;
}

EOLIAN static Eina_Bool
_evas_text_char_pos_get(Eo *eo_obj, Evas_Text_Data *o, int pos, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Eina_Bool ret;
   ret = EINA_FALSE;

   int l = 0, r = 0, t = 0, b = 0;
   int x = 0, y = 0, w = 0, h = 0;

   if (!o->font) return ret;
   if (!o->items || (pos < 0)) return ret;

   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   Eina_Bool int_ret = _evas_object_text_char_coords_get(eo_obj, o, (size_t) pos,
            &x, &y, &w, &h);
   if (!o->cur.filter.chain)
     evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);
   else
     evas_filter_program_padding_get(o->cur.filter.chain, &l, &r, &t, &b);
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
_evas_text_last_up_to_pos(Eo *eo_obj, Evas_Text_Data *o, Evas_Coord x, Evas_Coord y)
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
_evas_text_char_coords_get(Eo *eo_obj, Evas_Text_Data *o, Evas_Coord x, Evas_Coord y, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   int ret;
   ret = -1;

   int l = 0, r = 0, t = 0, b = 0;
   int rx = 0, ry = 0, rw = 0, rh = 0;

   if (!o->font) return ret;
   if (!o->items) return ret;

   int int_ret = _evas_object_text_char_at_coords(eo_obj, o, x, y - o->max_ascent,
         &rx, &ry, &rw, &rh);
   if (!o->cur.filter.chain)
     evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);
   else
     evas_filter_program_padding_get(o->cur.filter.chain, &l, &r, &t, &b);
   ry += o->max_ascent - t;
   rx -= l;
   if (rx < 0)
     {
	rw += rx;
	rx = 0;
     }
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
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
   int pl = 0, pr = 0, pt = 0, pb = 0, l = 0, r = 0, t = 0, b = 0;
   int w = 0, h = 0;

   if (o->cur.style == style) return;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);

   evas_text_style_pad_get(o->cur.style, &pl, &pr, &pt, &pb);
   o->cur.style = style;
   evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);

   if (o->items) w = obj->cur->geometry.w + (l - pl) + (r - pr);
   h = obj->cur->geometry.h + (t - pt) + (b - pb);

   eo_do_super(eo_obj, MY_CLASS,
               evas_obj_size_set(w, h));
}

EOLIAN static Evas_Text_Style_Type
_evas_text_style_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o)
{
   return o->cur.style;
}

EOLIAN static void
_evas_text_shadow_color_set(Eo *eo_obj, Evas_Text_Data *o, int r, int g, int b, int a)
{
   if ((o->cur.shadow.r == r) && (o->cur.shadow.g == g) &&
       (o->cur.shadow.b == b) && (o->cur.shadow.a == a))
     return;
   o->cur.shadow.r = r;
   o->cur.shadow.g = g;
   o->cur.shadow.b = b;
   o->cur.shadow.a = a;
   o->changed = 1;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_text_shadow_color_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, int *r, int *g, int *b, int *a)
{
   if (r) *r = o->cur.shadow.r;
   if (g) *g = o->cur.shadow.g;
   if (b) *b = o->cur.shadow.b;
   if (a) *a = o->cur.shadow.a;
}

EOLIAN static void
_evas_text_glow_color_set(Eo *eo_obj, Evas_Text_Data *o, int r, int g, int b, int a)
{
   if ((o->cur.glow.r == r) && (o->cur.glow.g == g) &&
       (o->cur.glow.b == b) && (o->cur.glow.a == a))
     return;
   o->cur.glow.r = r;
   o->cur.glow.g = g;
   o->cur.glow.b = b;
   o->cur.glow.a = a;
   o->changed = 1;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_text_glow_color_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, int *r, int *g, int *b, int *a)
{
   if (r) *r = o->cur.glow.r;
   if (g) *g = o->cur.glow.g;
   if (b) *b = o->cur.glow.b;
   if (a) *a = o->cur.glow.a;
}

EOLIAN static void
_evas_text_glow2_color_set(Eo *eo_obj, Evas_Text_Data *o, int r, int g, int b, int a)
{
   if ((o->cur.glow2.r == r) && (o->cur.glow2.g == g) &&
       (o->cur.glow2.b == b) && (o->cur.glow2.a == a))
     return;
   o->cur.glow2.r = r;
   o->cur.glow2.g = g;
   o->cur.glow2.b = b;
   o->cur.glow2.a = a;
   o->changed = 1;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_text_glow2_color_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, int *r, int *g, int *b, int *a)
{
   if (r) *r = o->cur.glow2.r;
   if (g) *g = o->cur.glow2.g;
   if (b) *b = o->cur.glow2.b;
   if (a) *a = o->cur.glow2.a;
}

EOLIAN static void
_evas_text_outline_color_set(Eo *eo_obj, Evas_Text_Data *o, int r, int g, int b, int a)
{
   if ((o->cur.outline.r == r) && (o->cur.outline.g == g) &&
       (o->cur.outline.b == b) && (o->cur.outline.a == a))
     return;
   o->cur.outline.r = r;
   o->cur.outline.g = g;
   o->cur.outline.b = b;
   o->cur.outline.a = a;
   o->changed = 1;
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
}

EOLIAN static void
_evas_text_outline_color_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, int *r, int *g, int *b, int *a)
{
   if (r) *r = o->cur.outline.r;
   if (g) *g = o->cur.outline.g;
   if (b) *b = o->cur.outline.b;
   if (a) *a = o->cur.outline.a;
}

EOLIAN static void
_evas_text_style_pad_get(Eo *eo_obj EINA_UNUSED, Evas_Text_Data *o, int *l, int *r, int *t, int *b)
{
   int sl = 0, sr = 0, st = 0, sb = 0;
   /* use temps to be certain we have initialized values */
   if (!o->cur.filter.chain)
     evas_text_style_pad_get(o->cur.style, &sl, &sr, &st, &sb);
   else
     evas_filter_program_padding_get(o->cur.filter.chain, &sl, &sr, &st, &sb);
   if (l) *l = sl;
   if (r) *r = sr;
   if (t) *t = st;
   if (b) *b = sb;
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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   /* set up methods (compulsory) */
   obj->func = &object_func;
   obj->private_data = eo_data_ref(eo_obj, MY_CLASS);
   obj->type = o_type;

   Evas_Text_Data *o = obj->private_data;
   /* alloc obj private data */
   o->cur.ellipsis = -1.0;
   o->prev = o->cur;
#ifdef BIDI_SUPPORT
   o->bidi_par_props = evas_bidi_paragraph_props_new();
#endif
}

EOLIAN static void
_evas_text_eo_base_destructor(Eo *eo_obj, Evas_Text_Data *o EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   evas_object_text_free(eo_obj, obj);
   eo_do_super(eo_obj, MY_CLASS, eo_destructor());
}

static void
evas_object_text_free(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Text_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

   /* free filter output */
   if (o->cur.filter.output)
     ENFN->image_free(ENDT, o->cur.filter.output);
   eina_hash_free(o->cur.filter.sources);
   evas_filter_program_del(o->cur.filter.chain);
   eina_stringshare_del(o->cur.filter.code);
   o->cur.filter.output = NULL;
   o->cur.filter.chain = NULL;
   o->cur.filter.sources = NULL;
   o->cur.filter.code = NULL;
   o->cur.filter.sources_count = 0;

   /* free obj */
   _evas_object_text_items_clear(o);
   if (o->cur.utf8_text) eina_stringshare_del(o->cur.utf8_text);
   if (o->cur.font) eina_stringshare_del(o->cur.font);
   if (o->cur.fdesc) evas_font_desc_unref(o->cur.fdesc);
   if (o->cur.source) eina_stringshare_del(o->cur.source);
   if (o->cur.text) free(o->cur.text);
   if (o->font && obj->layer && obj->layer->evas)
      evas_font_free(obj->layer->evas->evas, o->font);
   o->font = NULL;
#ifdef BIDI_SUPPORT
   evas_bidi_paragraph_props_unref(o->bidi_par_props);
#endif
}

void
evas_font_draw_async_check(Evas_Object_Protected_Data *obj,
                           void *data, void *context, void *surface,
                           Evas_Font_Set *font,
                           int x, int y, int w, int h, int ow, int oh,
                           Evas_Text_Props *intl_props, Eina_Bool do_async)
{
   Eina_Bool async_unref;

   async_unref = obj->layer->evas->engine.func->font_draw(data, context, surface,
                                                          font, x, y, w, h, ow, oh,
                                                          intl_props, do_async);
   if (do_async && async_unref)
     {
        evas_common_font_glyphs_ref(intl_props->glyphs);
        evas_unref_queue_glyph_put(obj->layer->evas, intl_props->glyphs);
     }
}

static void
_filter_cb(Evas_Filter_Context *ctx, void *data, Eina_Bool success)
{
   Eo *eo_obj = data;

   // Destroy context as we won't reuse it.
   evas_filter_context_destroy(ctx);

   // Redraw text with normal styles in case of failure
   if (!success)
     {
        Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
        Evas_Text_Data *o = (Evas_Text_Data *) obj->private_data;

        ERR("Filter failed at runtime!");
        o->cur.filter.invalid = EINA_TRUE;

        // Update object
        _evas_object_text_items_clear(o);
        o->changed = 1;
        _evas_object_text_recalc(eo_obj, o->cur.text);
        evas_object_change(eo_obj, obj);
        evas_object_clip_dirty(eo_obj, obj);
        evas_object_coords_recalc(eo_obj, obj);
        evas_object_inform_call_resize(eo_obj);
     }
}

static void
evas_object_text_render(Evas_Object *eo_obj,
                        Evas_Object_Protected_Data *obj,
                        void *type_private_data,
                        void *output, void *context, void *surface,
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
   if (!o->cur.filter.chain)
     evas_text_style_pad_get(o->cur.style, &sl, NULL, &st, NULL);
   else
     evas_filter_program_padding_get(o->cur.filter.chain, &sl, NULL, &st, NULL);
   ENFN->context_multiplier_unset(output, context);
   ENFN->context_render_op_set(output, context, obj->cur->render_op);
   /* FIXME: This clipping is just until we fix inset handling correctly. */
   ENFN->context_clip_clip(output, context,
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
	ENFN->context_color_set(output, context, \
				object->sub.col.r, \
				object->sub.col.g, \
				object->sub.col.b, \
				object->sub.col.a);

#define COLOR_SET(object, sub, col) \
        if (obj->cur->clipper)\
        { \
	   ENFN->context_color_set(output, context, \
				((int)object->sub.col.r * ((int)obj->cur->clipper->cur->cache.clip.r + 1)) >> 8, \
				((int)object->sub.col.g * ((int)obj->cur->clipper->cur->cache.clip.g + 1)) >> 8, \
				((int)object->sub.col.b * ((int)obj->cur->clipper->cur->cache.clip.b + 1)) >> 8, \
				((int)object->sub.col.a * ((int)obj->cur->clipper->cur->cache.clip.a + 1)) >> 8); \
        } \
        else\
	   ENFN->context_color_set(output, context, \
				object->sub.col.r, \
				object->sub.col.g, \
				object->sub.col.b, \
				object->sub.col.a);

#define COLOR_SET_AMUL(object, sub, col, amul) \
        if (obj->cur->clipper) \
        { \
	    ENFN->context_color_set(output, context, \
				(((int)object->sub.col.r) * ((int)obj->cur->clipper->cur->cache.clip.r) * (amul)) / 65025, \
				(((int)object->sub.col.g) * ((int)obj->cur->clipper->cur->cache.clip.g) * (amul)) / 65025, \
				(((int)object->sub.col.b) * ((int)obj->cur->clipper->cur->cache.clip.b) * (amul)) / 65025, \
				(((int)object->sub.col.a) * ((int)obj->cur->clipper->cur->cache.clip.a) * (amul)) / 65025); \
        } \
        else \
	    ENFN->context_color_set(output, context, \
				(((int)object->sub.col.r) * (amul)) / 255, \
				(((int)object->sub.col.g) * (amul)) / 255, \
				(((int)object->sub.col.b) * (amul)) / 255, \
				(((int)object->sub.col.a) * (amul)) / 255);

#define DRAW_TEXT(ox, oy)                                               \
   if ((o->font) && (it->text_props.len > 0))                           \
     evas_font_draw_async_check(obj, output,                            \
                                context,                                \
                                surface,                                \
                                o->font,                                \
                                obj->cur->geometry.x + x + sl + ox + it->x, \
                                obj->cur->geometry.y + y + st + oy +     \
                                (int) o->max_ascent,                    \
                                obj->cur->geometry.w,                    \
                                obj->cur->geometry.h,                    \
                                obj->cur->geometry.w,                    \
                                obj->cur->geometry.h,                    \
                                &it->text_props,                        \
                                do_async);

   /* FIXME/WARNING
    * The code below is EXPERIMENTAL, and not to be considered usable or even
    * remotely similar to its final form. You've been warned :)
    */

   if (!o->cur.filter.invalid && (o->cur.filter.chain || o->cur.filter.code))
     {
        int X, Y, W, H;
        Evas_Filter_Context *filter;
        const int inbuf = 1;
        const int outbuf = 2;
        void *filter_ctx;
        Eina_Bool ok;
        int ox = 0, oy = 0;
        void *previous = o->cur.filter.output;

        /* NOTE: Font effect rendering is now done ENTIRELY on CPU.
         * So we rely on cache/cache2 to allocate a real image buffer,
         * that we can draw to. The OpenGL texture will be created only
         * after the rendering has been done, as we simply push the output
         * image to GL.
         */

        W = obj->cur->geometry.w;
        H = obj->cur->geometry.h;
        X = obj->cur->geometry.x;
        Y = obj->cur->geometry.y;

        // Prepare color multiplier
        ENFN->context_color_set(ENDT, context, 255, 255, 255, 255);
        if ((obj->cur->cache.clip.r == 255) && (obj->cur->cache.clip.g == 255) &&
            (obj->cur->cache.clip.b == 255) && (obj->cur->cache.clip.a == 255))
          ENFN->context_multiplier_unset(ENDT, context);
        else
          ENFN->context_multiplier_set(ENDT, context,
                                       obj->cur->cache.clip.r,
                                       obj->cur->cache.clip.g,
                                       obj->cur->cache.clip.b,
                                       obj->cur->cache.clip.a);

        if (!o->cur.filter.chain)
          {
             Evas_Filter_Program *pgm;
             pgm = evas_filter_program_new("Evas_Text", EINA_TRUE);
             evas_filter_program_source_set_all(pgm, o->cur.filter.sources);
             if (!evas_filter_program_parse(pgm, o->cur.filter.code))
               {
                  ERR("Filter program parsing failed");
                  evas_filter_program_del(pgm);
                  o->cur.filter.invalid = EINA_TRUE;
                  goto normal_render;
               }
             o->cur.filter.chain = pgm;
             o->cur.filter.invalid = EINA_FALSE;
          }
        else if (previous)
          {
             Eina_Bool redraw = o->cur.filter.changed;

             // Scan proxies to find if any changed
             if (!redraw && o->cur.filter.sources)
               {
                  Evas_Filter_Proxy_Binding *pb;
                  Evas_Object_Protected_Data *source;
                  Eina_Iterator *iter;

                  iter = eina_hash_iterator_data_new(o->cur.filter.sources);
                  EINA_ITERATOR_FOREACH(iter, pb)
                    {
                       source = eo_data_scope_get(pb->eo_source, EVAS_OBJECT_CLASS);
                       if (source->changed)
                         {
                            redraw = EINA_TRUE;
                            break;
                         }
                    }
                  eina_iterator_free(iter);
               }

             if (!redraw)
               {
                  // Render this image only
                  ENFN->image_draw(ENDT, context,
                                   surface, previous,
                                   0, 0, W, H,         // src
                                   X + x, Y + y, W, H, // dst
                                   EINA_FALSE,         // smooth
                                   do_async);
                  return;
               }
          }

        filter = evas_filter_context_new(obj->layer->evas, do_async);
        ok = evas_filter_context_program_use(filter, o->cur.filter.chain);
        if (!filter || !ok)
          {
             ERR("Parsing failed?");
             evas_filter_context_destroy(filter);
             goto normal_render;
          }

        // Proxies
        evas_filter_context_proxy_render_all(filter, eo_obj, EINA_FALSE);

        // Draw Context
        filter_ctx = ENFN->context_new(ENDT);
        ENFN->context_color_set(ENDT, filter_ctx, 255, 255, 255, 255);

        // Allocate all buffers now
        evas_filter_context_buffers_allocate_all(filter, W, H);
        evas_filter_target_set(filter, context, surface, X + x, Y + y);

        // Steal output and release previous
        o->cur.filter.output = evas_filter_buffer_backing_steal(filter, outbuf);
        if (o->cur.filter.output != previous)
          evas_filter_buffer_backing_release(filter, previous);

        // Render text to input buffer
        EINA_INLIST_FOREACH(EINA_INLIST_GET(o->items), it)
          if ((o->font) && (it->text_props.len > 0))
            {
               evas_filter_font_draw(filter, filter_ctx, inbuf, o->font,
                                     sl + ox + it->x,
                                     st + oy + (int) o->max_ascent,
                                     &it->text_props,
                                     do_async);
            }

        ENFN->context_free(ENDT, filter_ctx);

        // Add post-run callback and run filter
        evas_filter_context_post_run_callback_set(filter, _filter_cb, eo_obj);
        ok = evas_filter_run(filter);
        o->cur.filter.changed = EINA_FALSE;

        if (ok)
          {
             DBG("Effect rendering done.");
             return;
          }
        else
          {
             ERR("Rendering failed");
             o->cur.filter.invalid = EINA_TRUE;
             goto normal_render;
          }
     }

   /* End of the EXPERIMENTAL code */

normal_render:

   /* shadows */
   switch (o->cur.style & EVAS_TEXT_STYLE_MASK_BASIC)
     {
      case EVAS_TEXT_STYLE_SHADOW:
      case EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW:
         shad_dst = 1;
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
        if ((o->cur.style == EVAS_TEXT_STYLE_OUTLINE) ||
              (o->cur.style == EVAS_TEXT_STYLE_OUTLINE_SHADOW) ||
              (o->cur.style == EVAS_TEXT_STYLE_OUTLINE_SOFT_SHADOW))
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
        COLOR_ONLY_SET(obj, cur->cache, clip);
        DRAW_TEXT(0, 0);
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
	  evas_object_clip_recalc(obj->cur->clipper);
	obj->cur->clipper->func->render_pre(obj->cur->clipper->object,
					    obj->cur->clipper,
					    obj->cur->clipper->private_data);
     }
   /* If object size changed and ellipsis is set */
   if (((o->cur.ellipsis >= 0.0 ||
	 o->cur.ellipsis != o->prev.ellipsis) &&
	((obj->cur->geometry.w != o->last_computed.w) ||
	 (obj->cur->geometry.h != o->last_computed.h))) ||
       (obj->cur->scale != obj->prev->scale))
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
evas_object_text_render_post(Evas_Object *eo_obj,
                             Evas_Object_Protected_Data *obj EINA_UNUSED,
                             void *type_private_data EINA_UNUSED)
{
   /* this moves the current data to the previous state parts of the object
    in whatever way is safest for the object. also if we don't need object
    data anymore we can free it if the object deems this is a good idea */
   /* remove those pesky changes */
   evas_object_clip_changes_clean(eo_obj);
   /* move cur to prev safely for object data */
   evas_object_cur_prev(eo_obj);
}

static unsigned int
evas_object_text_id_get(Evas_Object *eo_obj)
{
   Evas_Text_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_TEXT;
}

static unsigned int
evas_object_text_visual_id_get(Evas_Object *eo_obj)
{
   Evas_Text_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   if (!o) return 0;
   return MAGIC_OBJ_SHAPE;
}

static void *
evas_object_text_engine_data_get(Evas_Object *eo_obj)
{
   Evas_Text_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
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

static void
evas_object_text_scale_update(Evas_Object *eo_obj,
                              Evas_Object_Protected_Data *pd EINA_UNUSED,
                              void *type_private_data)
{
   Evas_Text_Data *o = type_private_data;
   int size;
   const char *font;

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
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Text_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);
   int is, was;

   if (!o->font) return;
   evas_font_load_hinting_set(obj->layer->evas->evas, o->font,
			      obj->layer->evas->hinting);
   was = evas_object_is_in_output_rect(eo_obj, obj,
				       obj->layer->evas->pointer.x,
				       obj->layer->evas->pointer.y, 1, 1);
   /* DO II */
   _evas_object_text_recalc(eo_obj, o->cur.text);
   o->changed = 1;
   o->cur.filter.changed = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   is = evas_object_is_in_output_rect(eo_obj, obj,
				      obj->layer->evas->pointer.x,
				      obj->layer->evas->pointer.y, 1, 1);
   if ((is || was) && obj->cur->visible)
     evas_event_feed_mouse_move(obj->layer->evas->evas,
				obj->layer->evas->pointer.x,
				obj->layer->evas->pointer.y,
				obj->layer->evas->last_timestamp,
				NULL);
   evas_object_inform_call_resize(eo_obj);
}

static void
_evas_object_text_recalc(Evas_Object *eo_obj, Eina_Unicode *text)
{
   Evas_Object_Protected_Data *obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   Evas_Text_Data *o = eo_data_scope_get(eo_obj, MY_CLASS);

   if (!text) text = eina_unicode_strdup(EINA_UNICODE_EMPTY_STRING);

   _evas_object_text_layout(eo_obj, o, text);

   /* Calc ascent/descent. */
   if (o->items)
     {
        Evas_Object_Text_Item *item;

        for (item = o->items ; item ;
              item = EINA_INLIST_CONTAINER_GET(
                 EINA_INLIST_GET(item)->next, Evas_Object_Text_Item))
          {
             int asc = 0, desc = 0;

             /* Skip items without meaning full information. */
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
     }
   else if (o->font)
     {
        o->ascent = ENFN->font_ascent_get(ENDT, o->font);
        o->descent = ENFN->font_descent_get(ENDT, o->font);
        o->max_ascent = ENFN->font_max_ascent_get(ENDT, o->font);
        o->max_descent = ENFN->font_max_descent_get(ENDT, o->font);
     }

   if ((o->font) && (o->items))
     {
        int w, h;
        int l = 0, r = 0, t = 0, b = 0;

        w = _evas_object_text_horiz_advance_without_ellipsis_get(o);
        h = _evas_object_text_vert_advance_get(eo_obj, o);
        if (!o->cur.filter.chain)
          evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);
        else
          evas_filter_program_padding_get(o->cur.filter.chain, &l, &r, &t, &b);

        if (o->cur.ellipsis >= 0.0)
          {
             int min;

             min = w + l + r < obj->cur->geometry.w || obj->cur->geometry.w == 0 ? w + l + r : obj->cur->geometry.w;
             eo_do_super(eo_obj, MY_CLASS,
                         evas_obj_size_set(min, h + t + b));
          }
        else
          {
             eo_do_super(eo_obj, MY_CLASS,
                         evas_obj_size_set(w + l + r, h + t + b));
          }
////        obj->cur->cache.geometry.validity = 0;
     }
   else
     {
        int t = 0, b = 0, l = 0, r = 0;

        if (!o->cur.filter.chain)
          evas_text_style_pad_get(o->cur.style, &l, &r, &t, &b);
        else
          evas_filter_program_padding_get(o->cur.filter.chain, &l, &r, &t, &b);

        eo_do_super(eo_obj, MY_CLASS,
                    evas_obj_size_set(0, o->max_ascent + o->max_descent + t + b));
////        obj->cur->cache.geometry.validity = 0;
     }
   o->last_computed.w = obj->cur->geometry.w;
   o->last_computed.h = obj->cur->geometry.h;
}

/* EXPERIMENTAL CODE BEGIN */

EOLIAN static void
_evas_text_filter_program_set(Eo *eo_obj, Evas_Text_Data *o, const char *arg)
{
   Evas_Object_Protected_Data *obj;
   Evas_Filter_Program *pgm = NULL;

   if (!o) return;
   if (o->cur.filter.code == arg) return;
   if (o->cur.filter.code && arg && !strcmp(arg, o->cur.filter.code)) return;

   // Parse filter program
   evas_filter_program_del(o->cur.filter.chain);
   if (arg)
     {
        pgm = evas_filter_program_new("Evas_Text: Filter Program", EINA_TRUE);
        evas_filter_program_source_set_all(pgm, o->cur.filter.sources);
        if (!evas_filter_program_parse(pgm, arg))
          {
             ERR("Parsing failed!");
             evas_filter_program_del(pgm);
             pgm = NULL;
          }
     }
   o->cur.filter.chain = pgm;
   o->cur.filter.changed = EINA_TRUE;
   o->cur.filter.invalid = (pgm == NULL);
   eina_stringshare_replace(&o->cur.filter.code, arg);

   // Update object
   obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   _evas_object_text_items_clear(o);
   o->changed = 1;
   _evas_object_text_recalc(eo_obj, o->cur.text);
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   evas_object_inform_call_resize(eo_obj);
}

static void
_filter_source_hash_free_cb(void *data)
{
   Evas_Filter_Proxy_Binding *pb = data;
   Evas_Object_Protected_Data *proxy, *source;
   Evas_Text_Data *o;

   proxy = eo_data_scope_get(pb->eo_proxy, EVAS_OBJECT_CLASS);
   source = eo_data_scope_get(pb->eo_source, EVAS_OBJECT_CLASS);

   if (source)
     {
        EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, source->proxy,
                             Evas_Object_Proxy_Data, source_write)
          source_write->proxies = eina_list_remove(source_write->proxies, pb->eo_proxy);
        EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, source_write)
     }

   o = eo_data_scope_get(pb->eo_proxy, MY_CLASS);

   if (o && proxy)
     {
        o->cur.filter.sources_count--;
        if (!o->cur.filter.sources_count)
          {
             EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, proxy->proxy,
                                  Evas_Object_Proxy_Data, proxy_write)
               proxy_write->is_proxy = EINA_FALSE;
             EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, proxy_write)
          }
     }

   eina_stringshare_del(pb->name);
   free(pb);
}

EOLIAN static void
_evas_text_filter_source_set(Eo *eo_obj, Evas_Text_Data *o, const char *name, Evas_Object *eo_source)
{
   Evas_Object_Protected_Data *obj;
   Evas_Filter_Program *pgm = o->cur.filter.chain;
   Evas_Filter_Proxy_Binding *pb, *pb_old = NULL;
   Evas_Object_Protected_Data *source = NULL;

   obj = eo_data_scope_get(eo_obj, EVAS_OBJECT_CLASS);
   if (eo_source) source = eo_data_scope_get(eo_source, EVAS_OBJECT_CLASS);

   if (!name)
     {
        if (!eo_source || !o->cur.filter.sources) return;
        if (eina_hash_del_by_data(o->cur.filter.sources, eo_source))
          goto update;
        return;
     }

   if (!o->cur.filter.sources)
     {
        if (!source) return;
        o->cur.filter.sources = eina_hash_string_small_new
              (EINA_FREE_CB(_filter_source_hash_free_cb));
     }
   else
     {
        pb_old = eina_hash_find(o->cur.filter.sources, name);
        if (pb_old)
          {
             if (pb_old->eo_source == eo_source) goto update;
             eina_hash_del(o->cur.filter.sources, name, pb_old);
          }
     }

   if (!source)
     {
        pb_old = eina_hash_find(o->cur.filter.sources, name);
        if (!pb_old) return;
        eina_hash_del_by_key(o->cur.filter.sources, name);
        goto update;
     }

   pb = calloc(1, sizeof(*pb));
   pb->eo_proxy = eo_obj;
   pb->eo_source = eo_source;
   pb->name = eina_stringshare_add(name);

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, source->proxy,
                        Evas_Object_Proxy_Data, source_write)
     if (!eina_list_data_find(source_write->proxies, eo_obj))
       source_write->proxies = eina_list_append(source_write->proxies, eo_obj);
   EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, source_write)

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, obj->proxy,
                        Evas_Object_Proxy_Data, proxy_write)
     proxy_write->is_proxy = EINA_TRUE;
   EINA_COW_WRITE_END(evas_object_proxy_cow, obj->proxy, proxy_write)

   eina_hash_add(o->cur.filter.sources, pb->name, pb);
   o->cur.filter.sources_count++;

   evas_filter_program_source_set_all(pgm, o->cur.filter.sources);

   // Update object
update:
   o->cur.filter.changed = EINA_TRUE;
   o->cur.filter.invalid = EINA_FALSE;
   _evas_object_text_items_clear(o);
   o->changed = 1;
   _evas_object_text_recalc(eo_obj, o->cur.text);
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   evas_object_coords_recalc(eo_obj, obj);
   evas_object_inform_call_resize(eo_obj);
}

/* EXPERIMENTAL CODE END */

#include "canvas/evas_text.eo.c"
