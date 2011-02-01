#include "evas_font_ot.h"

#ifdef OT_SUPPORT
# include <hb.h>
# include <hb-ft.h>
#endif

#include "evas_common.h"

#include <Eina.h>
#include "evas_font_private.h"

#ifdef OT_SUPPORT
/* FIXME: doc. returns #items */
EAPI int
evas_common_font_ot_cluster_size_get(const Evas_Text_Props *props, size_t char_index)
{
   int i;
   int items;
   int left_bound, right_bound;
   size_t base_cluster = EVAS_FONT_OT_POS_GET(props->info->ot[char_index]);
   for (i = (int) char_index ;
         (i >= (int) props->start) &&
         (EVAS_FONT_OT_POS_GET(props->info->ot[i]) == base_cluster) ;
         i--)
     ;
   left_bound = i;
   for (i = (int) char_index + 1;
         (i < (int) (props->start + props->len)) &&
         (EVAS_FONT_OT_POS_GET(props->info->ot[i]) == base_cluster) ;
         i++)
     ;
   right_bound = i;
   if (left_bound < 0)
      left_bound = 0;
   if (right_bound >= (int) (props->start + props->len))
      right_bound = props->start + props->len - 1;

   if (right_bound == left_bound)
     {
        items = 1;
     }
   else if (props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        if (left_bound < 0)
          {
             items = props->start + props->len -
                props->info->ot[left_bound + 1].source_cluster;
          }
        else
          {
             items = props->info->ot[left_bound].source_cluster -
                props->info->ot[left_bound + 1].source_cluster;
          }
     }
   else
     {
        if (right_bound == (int) (props->start + props->len))
          {
             items = props->start + props->len -
                props->info->ot[right_bound - 1].source_cluster;
          }
        else
          {
             items = props->info->ot[right_bound].source_cluster -
                props->info->ot[right_bound - 1].source_cluster;
          }
     }
   return (items > 0) ? items : 1;
}

EAPI void
evas_common_font_ot_load_face(void *_font)
{
   RGBA_Font_Source *font = (RGBA_Font_Source *) _font;
   /* Unload the face if by any chance it's already loaded */
   evas_common_font_ot_unload_face(font);
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

   props->len = hb_buffer_get_length(buffer);
   props->info->ot = calloc(props->len,
         sizeof(Evas_Font_OT_Info));
   props->info->glyph = calloc(props->len,
              sizeof(Evas_Font_Glyph_Info));
   positions = hb_buffer_get_glyph_positions(buffer);
   infos = hb_buffer_get_glyph_infos(buffer);
   for (i = 0 ; i < props->len ; i++)
     {
        props->info->ot[i].source_cluster = infos[i].cluster;
        props->info->ot[i].x_offset = positions[i].x_offset;
        props->info->ot[i].y_offset = positions[i].y_offset;
        props->info->glyph[i].index = infos[i].codepoint;
        props->info->glyph[i].advance = positions[i].x_advance;
     }

   hb_buffer_destroy(buffer);
   evas_common_font_int_use_trim();

   return EINA_FALSE;
}

#endif

