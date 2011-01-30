#include "evas_common.h"
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
   props->info->refcount++;
}

void
evas_common_text_props_content_unref(Evas_Text_Props *props)
{
   /* We allow this, because sometimes we want to have props without info,
    * and we don't want to diverge the code paths too much. */
   if (!props->info)
     return;

   if (props->info->refcount == 0)
     {
        ERR("Trying to unref props with refount == 0");
        return;
     }

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

/* Won't work in the middle of ligatures */
EAPI void
evas_common_text_props_split(Evas_Text_Props *base,
      Evas_Text_Props *ext, int cutoff)
{
   evas_common_text_props_content_copy_and_ref(ext, base);
   if (base->bidi.dir == EVAS_BIDI_DIRECTION_RTL)
     {
        ext->start = base->start;
        base->start = cutoff;
        ext->len = cutoff;
        base->len = base->len - cutoff;
     }
   else
     {
        ext->start = cutoff;
        ext->len = base->len - cutoff;
        base->len = cutoff;
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

