#include "evas_common.h"
#include "evas_font_private.h"
#include "evas_text_utils.h"
#include "language/evas_bidi_utils.h"
#include "language/evas_language_utils.h"
#include "evas_font_ot.h"

void
evas_common_text_props_bidi_set(Evas_Text_Props *props,
      Evas_BiDi_Paragraph_Props *bidi_par_props, size_t start)
{
#ifdef BIDI_SUPPORT
   props->bidi.dir = (evas_bidi_is_rtl_char(
            bidi_par_props,
            0,
            start)) ? EVAS_BIDI_DIRECTION_RTL : EVAS_BIDI_DIRECTION_LTR;
#else
   (void) start;
   (void) bidi_par_props;
   props->bidi.dir = EVAS_BIDI_DIRECTION_LTR;
#endif
}

void
evas_common_text_props_script_set(Evas_Text_Props *props, Evas_Script_Type scr)
{
   props->script = scr;
}

void
evas_common_text_props_content_copy_and_ref(Evas_Text_Props *dst,
      const Evas_Text_Props *src)
{
   memcpy(dst, src, sizeof(Evas_Text_Props));
   evas_common_text_props_content_ref(dst);
}

void
evas_common_text_props_content_ref(Evas_Text_Props *props)
{
   /* No content in this case */
   if (!props->info)
      return;

   props->info->refcount++;
}

void
evas_common_text_props_content_unref(Evas_Text_Props *props)
{
   /* No content in this case */
   if (!props->info)
      return;

   if (--(props->info->refcount) == 0)
     {
        if (props->info->glyph)
          free(props->info->glyph);
#ifdef OT_SUPPORT
        if (props->info->ot)
          free(props->info->ot);
#endif
        free(props->info);
        props->info = NULL;
     }
}

EAPI int
evas_common_text_props_cluster_next(const Evas_Text_Props *props, int pos)
{
   int prop_pos = evas_common_text_props_index_find(props, pos);
   if ((props->bidi.dir == EVAS_BIDI_DIRECTION_RTL) && (prop_pos > 0))
     {
#ifdef OT_SUPPORT
        return props->info->ot[props->start + prop_pos - 1].source_cluster -
           props->text_offset;
#else
        return props->start + prop_pos - 1 - props->text_offset;
#endif
     }
   else if ((props->bidi.dir != EVAS_BIDI_DIRECTION_RTL) &&
         (prop_pos < (int) (props->len - 1)))
     {
#ifdef OT_SUPPORT
        return props->info->ot[props->start + prop_pos + 1].source_cluster -
           props->text_offset;
#else
        return props->start + prop_pos + 1 - props->text_offset;
#endif
     }

   return pos;
}

/* Returns the index of the logical char in the props. */
EAPI int
evas_common_text_props_index_find(const Evas_Text_Props *props, int _cutoff)
{
#ifdef OT_SUPPORT
   Evas_Font_OT_Info *ot_info;
   int min = 0;
   int max = props->len - 1;
   int mid;

   _cutoff += props->text_offset;
   ot_info = props->info->ot + props->start;
   /* Should get us closer to the right place. */
   if ((min <= _cutoff) && (_cutoff <= max))
      mid = _cutoff;
   else
      mid = (min + max) / 2;

   if (props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        /* Monotonic in a descending order */
        do
          {
             if (_cutoff > (int) ot_info[mid].source_cluster)
                max = mid - 1;
             else if (_cutoff < (int) ot_info[mid].source_cluster)
                min = mid + 1;
             else
                break;

             mid = (min + max) / 2;
          }
        while (min <= max);
     }
   else
     {
        /* Monotonic in an ascending order */
        do
          {
             if (_cutoff < (int) ot_info[mid].source_cluster)
                max = mid - 1;
             else if (_cutoff > (int) ot_info[mid].source_cluster)
                min = mid + 1;
             else
                break;

             mid = (min + max) / 2;
          }
        while (min <= max);
     }

   /* If we didn't find, abort */
   if (min > max)
      return -1;

   ot_info += mid;
   if (props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        /* Walk to the last one of the same cluster */
        for ( ; mid < (int) props->len ; mid++, ot_info++)
          {
             if (ot_info->source_cluster != (size_t) _cutoff)
                break;
          }
        mid = props->len - mid;
     }
   else
     {
        /* Walk to the last one of the same cluster */
        for ( ; mid >= 0 ; mid--, ot_info--)
          {
             if (ot_info->source_cluster != (size_t) _cutoff)
                break;
          }
        mid++;
     }

   return mid;
#else
   return _cutoff;
   (void) props;
#endif
}

/* Won't work in the middle of ligatures, assumes cutoff < len.
 * Also won't work in the middle of indic words, should handle that in a
 * smart way. */
EAPI void
evas_common_text_props_split(Evas_Text_Props *base,
      Evas_Text_Props *ext, int _cutoff)
{
   size_t cutoff;

   /* Translate text cutoff pos to string object cutoff point */
#ifdef OT_SUPPORT
   _cutoff = evas_common_text_props_index_find(base, _cutoff);

   if (_cutoff > 0)
     {
        cutoff = (size_t) _cutoff;
     }
   else
     {
        ERR("Couldn't find the cutoff position. Is it inside a cluster?");
        return;
     }
#else
   cutoff = (size_t) _cutoff;
#endif

   evas_common_text_props_content_copy_and_ref(ext, base);
   if (base->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        ext->start = base->start;
        ext->len = base->len - cutoff;
        base->start = (base->start + base->len) - cutoff;
        base->len = cutoff;

#ifdef OT_SUPPORT
        ext->text_offset =
           ext->info->ot[ext->start + ext->len - 1].source_cluster;
#else
        ext->text_offset = base->text_offset + base->len;
#endif
     }
   else
     {
        ext->start = base->start + cutoff;
        ext->len = base->len - cutoff;
        base->len = cutoff;

#ifdef OT_SUPPORT
        ext->text_offset = ext->info->ot[ext->start].source_cluster;
#else
        ext->text_offset = base->text_offset + base->len;
#endif
     }
   ext->text_len = base->text_len - (ext->text_offset - base->text_offset);
   base->text_len = (ext->text_offset - base->text_offset);
}

/* Won't work in the middle of ligatures */
EAPI void
evas_common_text_props_merge(Evas_Text_Props *item1,
      const Evas_Text_Props *item2)
{
   if (item1->info != item2->info)
     {
        ERR("tried merge back items that weren't together in the first place.");
        return;
     }
   if (item1->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        item1->start = item2->start;
     }

   item1->len += item2->len;
   item1->text_len += item2->text_len;
}

EAPI Eina_Bool
evas_common_text_props_content_create(void *_fi, const Eina_Unicode *text,
      Evas_Text_Props *text_props, const Evas_BiDi_Paragraph_Props *par_props,
      size_t par_pos, int len)
{
   RGBA_Font_Int *fi = (RGBA_Font_Int *) _fi;

   if (text_props->info)
     {
        evas_common_text_props_content_unref(text_props);
     }
   if (len == 0)
     {
        text_props->info = NULL;
        text_props->start = text_props->len = text_props->text_offset = 0;
     }
   text_props->info = calloc(1, sizeof(Evas_Text_Props_Info));

   text_props->font_instance = fi;

   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }

#ifdef OT_SUPPORT
   size_t char_index;
   Evas_Font_Glyph_Info *gl_itr;
   Evas_Coord pen_x = 0, adjust_x = 0;
   (void) par_props;
   (void) par_pos;

   evas_common_font_ot_populate_text_props(text, text_props, len);

   gl_itr = text_props->info->glyph;
   for (char_index = 0 ; char_index < text_props->len ; char_index++)
     {
        FT_UInt idx;
        RGBA_Font_Glyph *fg;
        Eina_Bool is_replacement = EINA_FALSE;
        /* If we got a malformed index, show the replacement char instead */
        if (gl_itr->index == 0)
          {
             gl_itr->index = evas_common_get_char_index(fi, REPLACEMENT_CHAR);
             is_replacement = EINA_TRUE;
          }
        idx = gl_itr->index;
        LKL(fi->ft_mutex);
        fg = evas_common_font_int_cache_glyph_get(fi, idx);
        if (!fg)
          {
             LKU(fi->ft_mutex);
             continue;
          }
        LKU(fi->ft_mutex);

        gl_itr->x_bear = fg->glyph_out->left;
        gl_itr->width = fg->glyph_out->bitmap.width;
        /* text_props->info->glyph[char_index].advance =
         * text_props->info->glyph[char_index].index =
         * already done by the ot function */
        if (EVAS_FONT_CHARACTER_IS_INVISIBLE(
              text[text_props->info->ot[char_index].source_cluster]))
          {
             gl_itr->index = 0;
             /* Reduce the current advance */
             if (gl_itr > text_props->info->glyph)
               {
                  adjust_x -= gl_itr->pen_after - (gl_itr - 1)->pen_after;
               }
             else
               {
                  adjust_x -= gl_itr->pen_after;
               }
          }
        else
          {
             if (is_replacement)
               {
                  /* Update the advance accordingly */
                  adjust_x += (pen_x + (fg->glyph->advance.x >> 16)) -
                     gl_itr->pen_after;
               }
             pen_x = gl_itr->pen_after;
          }
        gl_itr->pen_after += adjust_x;

        fi = text_props->font_instance;
        gl_itr++;
     }
#else
   /* We are walking the string in visual ordering */
   Evas_Font_Glyph_Info *gl_itr;
   Eina_Bool use_kerning;
   FT_UInt prev_index;
   FT_Face pface = NULL;
   Evas_Coord pen_x = 0;
   int adv_d, i;
#if !defined(OT_SUPPORT) && defined(BIDI_SUPPORT)
   Eina_Unicode *base_str = NULL;
   if (text_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        text = base_str = eina_unicode_strndup(text, len);
        evas_bidi_shape_string(base_str, par_props, par_pos, len);
     }
#else
   (void) par_props;
   (void) par_pos;
#endif

   FTLOCK();
   use_kerning = FT_HAS_KERNING(fi->src->ft.face);
   FTUNLOCK();
   prev_index = 0;

   i = len;
   text_props->info->glyph = calloc(len,
                                    sizeof(Evas_Font_Glyph_Info));

   if (text_props->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        text += len - 1;
        adv_d = -1;
     }
   else
     {
        adv_d = 1;
     }

   gl_itr = text_props->info->glyph;
   for ( ; i > 0 ; gl_itr++, text += adv_d, i--)
     {
        FT_UInt idx;
        RGBA_Font_Glyph *fg;
        int _gl, kern;
        Evas_Coord adv;
        _gl = *text;
        if (_gl == 0) break;

        idx = evas_common_get_char_index(fi, _gl);
        if (idx == 0)
          {
             idx = evas_common_get_char_index(fi, REPLACEMENT_CHAR);
          }

        LKL(fi->ft_mutex);
        fg = evas_common_font_int_cache_glyph_get(fi, idx);
        if (!fg)
          {
             LKU(fi->ft_mutex);
             continue;
          }
        kern = 0;

        if ((use_kerning) && (prev_index) && (idx) &&
            (pface == fi->src->ft.face))
          {
             if (evas_common_font_query_kerning(fi, prev_index, idx, &kern))
               {
                  pen_x += kern;
                  (gl_itr - 1)->pen_after +=
                     EVAS_FONT_ROUND_26_6_TO_INT(kern);
               }
          }

        pface = fi->src->ft.face;
        LKU(fi->ft_mutex);

        gl_itr->index = idx;
        gl_itr->x_bear = fg->glyph_out->left;
        adv = fg->glyph->advance.x >> 10;
        gl_itr->width = fg->glyph_out->bitmap.width;

        if (EVAS_FONT_CHARACTER_IS_INVISIBLE(_gl))
          {
             gl_itr->index = 0;
          }
        else
          {
             pen_x += adv;
          }

        gl_itr->pen_after = EVAS_FONT_ROUND_26_6_TO_INT(pen_x);

        prev_index = idx;
     }
   text_props->len = len;
# if !defined(OT_SUPPORT) && defined(BIDI_SUPPORT)
   if (base_str)
      free(base_str);
# endif
#endif
   text_props->text_len = len;
   text_props->info->refcount = 1;
   return EINA_TRUE;
}
