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
evas_common_text_props_script_set(Evas_Text_Props *props,
      const Eina_Unicode *str, size_t len)
{
   props->script = evas_common_language_script_type_get(str, len);
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
#if !defined(OT_SUPPORT) && defined(BIDI_SUPPORT)
        if (props->info->shaped_text)
           free(props->info->shaped_text);
#endif
        free(props->info);
        props->info = NULL;
     }
}

/* Won't work in the middle of ligatures, assumes cutoff < len */
EAPI void
evas_common_text_props_split(Evas_Text_Props *base,
      Evas_Text_Props *ext, int _cutoff)
{
   size_t cutoff;

   /* Translate text cutoff pos to string object cutoff point */
#ifdef OT_SUPPORT
   cutoff = 0;

     {
        Evas_Font_OT_Info *itr;
        size_t i;
        itr = base->info->ot + base->start;
        _cutoff += base->text_offset;
        /* FIXME: can I binary search? I don't think this is always sorted */
        for (i = 0 ; i < base->len ; i++, itr++)
          {
             if (itr->source_cluster == (size_t) _cutoff)
               {
                  if (base->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
                    {
                       /* Walk to the last one of the same cluster */
                       for ( ; i < base->len ; i++, itr++)
                         {
                            if (itr->source_cluster != (size_t) _cutoff)
                              break;
                         }
                       cutoff = base->len - i;
                    }
                  else
                    {
                       cutoff = i;
                    }
                  break;
               }
          }
     }

   /* If we didn't find a reasonable cut location, return. */
   if (cutoff == 0)
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
evas_common_text_props_content_create(void *_fn, const Eina_Unicode *text,
      Evas_Text_Props *text_props, const Evas_BiDi_Paragraph_Props *par_props,
      size_t par_pos, int len)
{
   RGBA_Font *fn = (RGBA_Font *) _fn;
   RGBA_Font_Int *fi;

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

   fi = fn->fonts->data;
   /* evas_common_font_size_use(fn); */
   evas_common_font_int_reload(fi);
   if (fi->src->current_size != fi->size)
     {
        FTLOCK();
        FT_Activate_Size(fi->ft.size);
        FTUNLOCK();
        fi->src->current_size = fi->size;
     }

#ifdef OT_SUPPORT
   /* FIXME: as soon as we start caching fi for non-harfbuzz as well, move
    * this (until text_props->fi = fi outside the ifdef */
   const Eina_Unicode *base_char;
   /* Load the glyph according to the first letter of the script, preety
    * bad, but will have to do */
     {
        /* Skip common chars */
        for (base_char = text ;
             *base_char &&
             evas_common_language_char_script_get(*base_char) ==
             EVAS_SCRIPT_COMMON ;
             base_char++)
           ;
        if (!*base_char && (base_char > text)) base_char--;
        evas_common_font_glyph_search(fn, &fi, *base_char);
     }

   text_props->font_instance = fi;


   size_t char_index;
   Evas_Font_Glyph_Info *gl_itr;
   Evas_Coord pen_x = 0, adjust_x = 0;
   (void) par_props;
   (void) par_pos;

   evas_common_font_ot_populate_text_props(fn, text, text_props, len);

   gl_itr = text_props->info->glyph;
   for (char_index = 0 ; char_index < text_props->len ; char_index++)
     {
        FT_UInt index;
        RGBA_Font_Glyph *fg;
        Eina_Bool is_replacement = EINA_FALSE;
        /* If we got a malformed index, show the replacement char instead */
        if (gl_itr->index == 0)
          {
             /* FIXME: search inside the same fi. */
             gl_itr->index =
                evas_common_font_glyph_search(fn, &fi, REPLACEMENT_CHAR);
             is_replacement = EINA_TRUE;
          }
        index = gl_itr->index;
        LKL(fi->ft_mutex);
        fg = evas_common_font_int_cache_glyph_get(fi, index);
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

                  /* FIXME: reload fi, a bit slow, but I have no choice. */
                  evas_common_font_glyph_search(fn, &fi, *base_char);
               }
             pen_x = gl_itr->pen_after;
          }
        gl_itr->pen_after += adjust_x;

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
   text = text_props->info->shaped_text = eina_unicode_strndup(text, len);
   evas_bidi_shape_string(text_props->info->shaped_text, par_props, par_pos,
                          len);
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
        FT_UInt index;
        RGBA_Font_Glyph *fg;
        int _gl, kern;
        Evas_Coord adv;
        _gl = *text;
        if (_gl == 0) break;

        index = evas_common_font_glyph_search(fn, &fi, _gl);
        if (index == 0)
          {
             index = evas_common_font_glyph_search(fn, &fi, REPLACEMENT_CHAR);
          }

        /* Should we really set the size per fi? This fixes a bug for Korean
         * because for some reason different fis are chosen for different
         * chars in some cases. But we should find the source of the problem
         * and not just fix the symptom. */
        if (fi->src->current_size != fi->size)
          {
             FTLOCK();
             FT_Activate_Size(fi->ft.size);
             FTUNLOCK();
             fi->src->current_size = fi->size;
          }

        LKL(fi->ft_mutex);
        fg = evas_common_font_int_cache_glyph_get(fi, index);
        if (!fg)
          {
             LKU(fi->ft_mutex);
             continue;
          }
        kern = 0;

        if ((use_kerning) && (prev_index) && (index) &&
            (pface == fi->src->ft.face))
          {
             if (evas_common_font_query_kerning(fi, prev_index, index, &kern))
               {
                  pen_x += kern;
                  (gl_itr - 1)->pen_after +=
                     EVAS_FONT_ROUND_26_6_TO_INT(kern);
               }
          }

        pface = fi->src->ft.face;
        LKU(fi->ft_mutex);

        gl_itr->index = index;
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

        prev_index = index;
     }
   text_props->len = len;
#endif
   text_props->text_len = len;
   text_props->info->refcount = 1;
   return EINA_TRUE;
}

