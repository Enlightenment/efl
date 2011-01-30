#ifndef _EVAS_TEXT_UTILS_H
# define _EVAS_TEXT_UTILS_H

typedef struct _Evas_Text_Props Evas_Text_Props;

# include "evas_font_ot.h"
# include "language/evas_bidi_utils.h"
# include "language/evas_language_utils.h"

struct _Evas_Text_Props
{
   Evas_BiDi_Props bidi;
   Evas_Script_Type script;
   Evas_Font_OT_Data *ot_data;
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
evas_common_text_props_content_unref(Evas_Text_Props *props);

EAPI void
evas_common_text_props_cutoff(Evas_Text_Props *props, int cutoff);

#endif
