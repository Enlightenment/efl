#include "evas_font_ot.h"

#ifdef OT_SUPPORT
# include <hb.h>
# include <hb-ft.h>
#endif

#include "evas_common.h"

#include <Eina.h>
#include "evas_font_private.h"

EAPI Eina_Bool
evas_common_font_ot_is_enabled(void)
{
#ifdef OT_SUPPORT
   static int ret = -1;
   const char *env;
   if (ret != -1)
     {
        return ret;
     }

   env = getenv("EVAS_USE_OT");
   if (env && atoi(env))
     {
        ret = EINA_TRUE;
        return ret;
     }
#endif
   return EINA_FALSE;
}

#ifdef OT_SUPPORT
/* FIXME: doc. returns #items */
EAPI int
evas_common_font_ot_cluster_size_get(const Evas_Text_Props *props, size_t char_index, int orig_len)
{
   int i;
   int items;
   int left_bound, right_bound;
   size_t base_cluster = EVAS_FONT_OT_POS_GET(props->ot_data->items[char_index]);
   for (i = (int) char_index ;
         (i >= 0) &&
         (EVAS_FONT_OT_POS_GET(props->ot_data->items[i]) == base_cluster) ;
         i--)
     ;
   left_bound = i;
   for (i = (int) char_index + 1;
         (i < (int) props->ot_data->len) &&
         (EVAS_FONT_OT_POS_GET(props->ot_data->items[i]) == base_cluster) ;
         i++)
     ;
   right_bound = i;
   if (right_bound == left_bound)
     {
        items = 1;
     }
   else if (props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        if (left_bound < 0)
          {
             items = orig_len -
                props->ot_data->items[left_bound + 1].source_cluster;
          }
        else
          {
             items = props->ot_data->items[left_bound].source_cluster -
                props->ot_data->items[left_bound + 1].source_cluster;
          }
     }
   else
     {
        if (right_bound == (int) props->ot_data->len)
          {
             items = orig_len -
                props->ot_data->items[right_bound - 1].source_cluster;
          }
        else
          {
             items = props->ot_data->items[right_bound].source_cluster -
                props->ot_data->items[right_bound - 1].source_cluster;
          }
     }
   return (items > 0) ? items : 1;
}

EAPI void
evas_common_font_ot_load_face(void *_font)
{
   RGBA_Font_Source *font = (RGBA_Font_Source *) _font;
   font->hb.face = hb_ft_face_create(font->ft.face, NULL);
}

EAPI void
evas_common_font_ot_unload_face(void *_font)
{
   RGBA_Font_Source *font = (RGBA_Font_Source *) _font;
   if (!font->hb.face) return;
   hb_face_destroy(font->hb.face);
   font->hb.face = NULL;
}

static void
_evas_common_font_ot_shape(hb_buffer_t *buffer, RGBA_Font_Source *src)
{
   hb_font_t   *hb_font;

   hb_font = hb_ft_font_create(src->ft.face, NULL);

   hb_shape(hb_font, src->hb.face, buffer, NULL, 0);
   hb_font_destroy(hb_font);
}

/* Won't work in the middle of ligatures */
EAPI void
evas_common_font_ot_cutoff_text_props(Evas_Text_Props *props, int cutoff)
{
   Evas_Font_OT_Data *new_data;
   if ((cutoff <= 0) || (!props->ot_data) ||
         (((size_t) cutoff) >= props->ot_data->len))
     return;

   new_data = malloc(sizeof(Evas_Font_OT_Data));
   memcpy(new_data, props->ot_data, sizeof(Evas_Font_OT_Data));
   new_data->refcount = 1;
   new_data->len = cutoff;
   new_data->items = malloc(cutoff * sizeof(Evas_Font_OT_Data_Item));

   if (props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        memcpy(new_data->items,
              props->ot_data->items + (props->ot_data->len - cutoff),
              cutoff * sizeof(Evas_Font_OT_Data_Item));
     }
   else
     {
        memcpy(new_data->items,
              props->ot_data->items,
              cutoff * sizeof(Evas_Font_OT_Data_Item));
     }

   evas_common_font_ot_props_unref(props->ot_data);
   props->ot_data = new_data;
}

/* Won't work in the middle of ligatures
 * aissumes ext doesn't have an already init ot_data
 * we assume there's at least one char in each part */
EAPI void
evas_common_font_ot_split_text_props(Evas_Text_Props *base,
      Evas_Text_Props *ext, int cutoff)
{
   Evas_Font_OT_Data *new_data;
   int i;
   if ((cutoff <= 0) || (!base->ot_data) ||
         (((size_t) cutoff) >= base->ot_data->len))
     return;

   ext->ot_data = calloc(1, sizeof(Evas_Font_OT_Data));
   ext->ot_data->refcount = 1;
   ext->ot_data->len = base->ot_data->len - cutoff;
   ext->ot_data->items = calloc(ext->ot_data->len,
         sizeof(Evas_Font_OT_Data_Item));

   new_data = malloc(sizeof(Evas_Font_OT_Data));
   memcpy(new_data, base->ot_data, sizeof(Evas_Font_OT_Data));
   new_data->refcount = 1;
   new_data->items = malloc(cutoff * sizeof(Evas_Font_OT_Data_Item));
   new_data->len = cutoff;

   if (base->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        memcpy(ext->ot_data->items, base->ot_data->items,
              ext->ot_data->len * sizeof(Evas_Font_OT_Data_Item));
        memcpy(new_data->items,
              base->ot_data->items + ext->ot_data->len,
              cutoff * sizeof(Evas_Font_OT_Data_Item));
     }
   else
     {
        memcpy(ext->ot_data->items, base->ot_data->items + cutoff,
              ext->ot_data->len * sizeof(Evas_Font_OT_Data_Item));
        memcpy(new_data->items, base->ot_data->items,
              cutoff * sizeof(Evas_Font_OT_Data_Item));
     }
   evas_common_font_ot_props_unref(base->ot_data);
   base->ot_data = new_data;

   /* Adjust the offset of the clusters */
     {
        size_t min;
        min = ext->ot_data->items[0].source_cluster;
        for (i = 1 ; i < (int) ext->ot_data->len ; i++)
          {
             if (ext->ot_data->items[i].source_cluster < min)
               {
                  min = ext->ot_data->items[i].source_cluster;
               }
          }
        for (i = 0 ; i < (int) ext->ot_data->len ; i++)
          {
             ext->ot_data->items[i].source_cluster -= min;
          }
        ext->ot_data->offset = base->ot_data->offset + min;
     }
}

/* Won't work in the middle of ligatures
 * assumes both are init correctly and that both are from the
 * same origin item, i.e both have the same script + direction.
 * assume item1 is logically first */
EAPI void
evas_common_font_ot_merge_text_props(Evas_Text_Props *item1,
      const Evas_Text_Props *item2)
{
   Evas_Font_OT_Data *new_data;
   Evas_Font_OT_Data_Item *itr; /* Itr will be used for adding back
                                         the offsets */
   size_t len;
   if (!item1->ot_data || !item2->ot_data)
     return;
   len = item1->ot_data->len + item2->ot_data->len;

   new_data = malloc(sizeof(Evas_Font_OT_Data));
   memcpy(new_data, item1->ot_data, sizeof(Evas_Font_OT_Data));
   new_data->refcount = 1;
   new_data->items = malloc(len * sizeof(Evas_Font_OT_Data_Item));
   new_data->len = len;
   if (item1->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        memcpy(new_data->items, item2->ot_data->items,
              item2->ot_data->len * sizeof(Evas_Font_OT_Data_Item));
        memcpy(new_data->items + item2->ot_data->len, item1->ot_data->items,
              item1->ot_data->len * sizeof(Evas_Font_OT_Data_Item));
        itr = new_data->items;
     }
   else
     {
        memcpy(new_data->items, item1->ot_data->items,
              item1->ot_data->len * sizeof(Evas_Font_OT_Data_Item));
        memcpy(new_data->items + item1->ot_data->len, item2->ot_data->items,
              item2->ot_data->len * sizeof(Evas_Font_OT_Data_Item));
        itr = new_data->items + item1->ot_data->len;
     }
   evas_common_font_ot_props_unref(item1->ot_data);
   item1->ot_data = new_data;
   /* Add back the offset of item2 to the newly created */
   if (item2->ot_data->offset > 0)
     {
        int i;
        for (i = 0 ; i < (int) item2->ot_data->len ; i++, itr++)
          {
             /* This must be > 0, just because this is how it works */
             itr->source_cluster += item2->ot_data->offset -
                item1->ot_data->offset;
          }
     }
}

EAPI Eina_Bool
evas_common_font_ot_populate_text_props(void *_fn, const Eina_Unicode *text,
      Evas_Text_Props *props, int len)
{
   RGBA_Font *fn = (RGBA_Font *) _fn;
   RGBA_Font_Int *fi;
   hb_buffer_t *buffer;
   hb_glyph_position_t *positions;
   hb_glyph_info_t *infos;
   int slen;
   unsigned int i;
   if (!evas_common_font_ot_is_enabled()) return EINA_TRUE;
   if (props->ot_data)
     {
        evas_common_font_ot_props_unref(props->ot_data);
     }
   props->ot_data = calloc(1, sizeof(Evas_Font_OT_Data));
   props->ot_data->refcount = 1;

   fi = fn->fonts->data;
   /* Load the font needed for this script */
     {
        /* Skip common chars */
        const Eina_Unicode *tmp;
        for (tmp = text ;
              *tmp &&
              evas_common_language_char_script_get(*tmp) == EVAS_SCRIPT_COMMON ;
              tmp++)
          ;
        if (!*tmp && (tmp > text)) tmp--;
        evas_common_font_glyph_search(fn, &fi, *tmp);
     }
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }

   if (len < 0)
     {
        slen = eina_unicode_strlen(text);
     }
   else
     {
        slen = len;
     }

   buffer = hb_buffer_create(slen);
   hb_buffer_set_unicode_funcs(buffer, evas_common_language_unicode_funcs_get());
   hb_buffer_set_language(buffer, hb_language_from_string(
            evas_common_language_from_locale_get()));
   hb_buffer_set_script(buffer, props->script);
   hb_buffer_set_direction(buffer,
         (props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) ?
         HB_DIRECTION_RTL : HB_DIRECTION_LTR);
   /* FIXME: add run-time conversions if needed, which is very unlikely */
   hb_buffer_add_utf32(buffer, (const uint32_t *) text, slen, 0, slen);

   _evas_common_font_ot_shape(buffer, fi->src);

   props->ot_data->len = hb_buffer_get_length(buffer);
   props->ot_data->items = calloc(props->ot_data->len,
         sizeof(Evas_Font_OT_Data_Item));
   positions = hb_buffer_get_glyph_positions(buffer);
   infos = hb_buffer_get_glyph_infos(buffer);
   for (i = 0 ; i < props->ot_data->len ; i++)
     {
        props->ot_data->items[i].index = infos[i].codepoint;
        props->ot_data->items[i].source_cluster = infos[i].cluster;
        props->ot_data->items[i].x_advance = positions[i].x_advance;
        props->ot_data->items[i].x_offset = positions[i].x_offset;
        props->ot_data->items[i].y_offset = positions[i].y_offset;
     }

   hb_buffer_destroy(buffer);
   evas_common_font_int_use_trim();

   return EINA_FALSE;
}

EAPI void
evas_common_font_ot_props_ref(Evas_Font_OT_Data *data)
{
   data->refcount++;
}

EAPI void
evas_common_font_ot_props_unref(Evas_Font_OT_Data *data)
{
   OTLOCK();
   if (--data->refcount == 0)
     {
        if (data->items)
          free(data->items);
        free(data);
     }
   OTUNLOCK();
}
#endif

