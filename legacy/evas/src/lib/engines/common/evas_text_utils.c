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
      const Eina_Unicode *str)
{
   props->script = evas_common_language_script_type_get(str);
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
   /* No info in this case */
   if (props->len == 0)
      return;

   props->info->refcount++;
}

void
evas_common_text_props_content_unref(Evas_Text_Props *props)
{
   /* No info in this case */
   if (props->len == 0)
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

/* Won't work in the middle of ligatures, assumes cutoff < len */
EAPI void
evas_common_text_props_split(Evas_Text_Props *base,
      Evas_Text_Props *ext, int cutoff)
{
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
}

EAPI Eina_Bool
evas_common_text_props_content_create(void *_fn, const Eina_Unicode *text,
      Evas_Text_Props *text_props, int len)
{
   RGBA_Font *fn = (RGBA_Font *) _fn;
   RGBA_Font_Int *fi;
   size_t char_index;

   if (text_props->info)
     {
        evas_common_text_props_content_unref(text_props);
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
   evas_common_font_ot_populate_text_props(fn, text, text_props, len);

   /* Load the glyph according to the first letter of the script, preety
    * bad, but will have to do */
     {
        /* Skip common chars */
        const Eina_Unicode *tmp;
        for (tmp = text ;
             *tmp &&
             evas_common_language_char_script_get(*tmp) ==
             EVAS_SCRIPT_COMMON ;
             tmp++)
           ;
        if (!*tmp && (tmp > text)) tmp--;
        evas_common_font_glyph_search(fn, &fi, *tmp);
     }

   for (char_index = 0 ; char_index < text_props->len ; char_index++)
     {
        FT_UInt index;
        RGBA_Font_Glyph *fg;
        index = text_props->info->glyph[char_index].index;
        LKL(fi->ft_mutex);
        fg = evas_common_font_int_cache_glyph_get(fi, index);
        if (!fg)
          {
             LKU(fi->ft_mutex);
             continue;
          }
        LKU(fi->ft_mutex);
        text_props->info->glyph[char_index].x_bear =
           fg->glyph_out->left;
        text_props->info->glyph[char_index].width =
           fg->glyph_out->bitmap.width;
        /* text_props->info->glyph[char_index].advance =
         * text_props->info->glyph[char_index].index =
         * already done by the ot function */
        if (EVAS_FONT_CHARACTER_IS_INVISIBLE(
              text[text_props->info->ot[char_index].source_cluster]))
           text_props->info->glyph[char_index].index = 0;

     }
#else
   /* We are walking the string in visual ordering */
   Eina_Bool use_kerning;
   FT_UInt prev_index;
   FT_Face pface = NULL;
   int adv_d, i;
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
   char_index = 0;
   for ( ; i > 0 ; char_index++, text += adv_d, i--)
     {
        FT_UInt index;
        RGBA_Font_Glyph *fg;
        int _gl, kern;
        _gl = *text;
        if (_gl == 0) break;

        index = evas_common_font_glyph_search(fn, &fi, _gl);
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
# ifdef BIDI_SUPPORT
             /* if it's rtl, the kerning matching should be reversed, */
             /* i.e prev index is now the index and the other way */
             /* around. There is a slight exception when there are */
             /* compositing chars involved.*/
             if (text_props &&
                 (text_props->bidi.dir != EVAS_BIDI_DIRECTION_RTL))
               {
                  if (evas_common_font_query_kerning(fi, index, prev_index, &kern))
                    {
                       text_props->info->glyph[char_index - 1].advance +=
                          kern;
                    }
               }
             else
# endif
               {
                  if (evas_common_font_query_kerning(fi, prev_index, index, &kern))
                    {
                       text_props->info->glyph[char_index - 1].advance +=
                          kern;
                    }
               }
          }

        pface = fi->src->ft.face;
        LKU(fi->ft_mutex);

        if (EVAS_FONT_CHARACTER_IS_INVISIBLE(_gl))
           text_props->info->glyph[char_index].index = 0;

        text_props->info->glyph[char_index].index = index;
        text_props->info->glyph[char_index].x_bear =
           fg->glyph_out->left;
        text_props->info->glyph[char_index].advance =
           fg->glyph->advance.x >> 10;
        text_props->info->glyph[char_index].width =
           fg->glyph_out->bitmap.width;

        prev_index = index;
     }
   text_props->len = len;
#endif
   text_props->info->refcount = 1;
   return EINA_TRUE;
}

