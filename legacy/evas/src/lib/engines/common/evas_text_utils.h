#ifndef _EVAS_TEXT_UTILS_H
# define _EVAS_TEXT_UTILS_H

typedef struct _Evas_Text_Props Evas_Text_Props;
typedef struct _Evas_Text_Props_Info Evas_Text_Props_Info;

# include "evas_font_ot.h"
# include "language/evas_bidi_utils.h"
# include "language/evas_language_utils.h"
# include "evas_font_glyph_info.h"

struct _Evas_Text_Props
{
   /* Start and len represent the start offset and the length in the
    * glyphs_info and ot_data fields, they are both internal */
   size_t start;
   size_t len;
   Evas_BiDi_Props bidi;
   Evas_Script_Type script;
   Evas_Text_Props_Info *info;
};

struct _Evas_Text_Props_Info
{
   unsigned int refcount;
   Evas_Font_Glyph_Info *glyph;
   Evas_Font_OT_Info *ot;
};

void
evas_common_text_props_bidi_set(Evas_Text_Props *props,
      Evas_BiDi_Paragraph_Props *bidi_par_props, size_t start);

void
evas_common_text_props_script_set(Evas_Text_Props *props,
      const Eina_Unicode *str);

void
evas_common_text_props_content_copy_and_ref(Evas_Text_Props *dst,
      const Evas_Text_Props *src);

void
evas_common_text_props_content_ref(Evas_Text_Props *props);

void
evas_common_text_props_content_unref(Evas_Text_Props *props);

EAPI void
evas_common_text_props_split(Evas_Text_Props *base, Evas_Text_Props *ext,
      int cutoff);
EAPI void
evas_common_text_props_merge(Evas_Text_Props *item1, const Evas_Text_Props *item2);
#endif
