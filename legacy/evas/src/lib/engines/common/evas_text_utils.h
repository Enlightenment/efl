#ifndef _EVAS_TEXT_UTILS_H
# define _EVAS_TEXT_UTILS_H

typedef struct _Evas_Text_Props Evas_Text_Props;
typedef struct _Evas_Text_Props_Info Evas_Text_Props_Info;
typedef struct _Evas_Font_Glyph_Info Evas_Font_Glyph_Info;

# include "evas_font_ot.h"
# include "language/evas_bidi_utils.h"
# include "language/evas_language_utils.h"

/* Used for showing "malformed" or missing chars */
#define REPLACEMENT_CHAR 0xFFFD

struct _Evas_Text_Props
{
   /* Start and len represent the start offset and the length in the
    * glyphs_info and ot_data fields, they are both internal */
   size_t start;
   size_t len;
   size_t text_offset; /* The text offset from the start of the info */
   size_t text_len; /* The length of the original text */
   Evas_BiDi_Props bidi;
   Evas_Script_Type script;
   Evas_Text_Props_Info *info;
   void *font_instance;
};

struct _Evas_Text_Props_Info
{
   unsigned int refcount;
   Evas_Font_Glyph_Info *glyph;
   Evas_Font_OT_Info *ot;
};

/* Sorted in visual order when created */
struct _Evas_Font_Glyph_Info
{
   unsigned int index; /* Should conform to FT */
   Evas_Coord x_bear;
#if 0
   /* This one is rarely used, only in draw, in which we already get the glyph
    * so it doesn't really save time. Leaving it here just so no one will
    * add it thinking it was accidentally skipped */
   Evas_Coord y_bear;
#endif
   Evas_Coord width;
   Evas_Coord pen_after;
};


void
evas_common_text_props_bidi_set(Evas_Text_Props *props,
      Evas_BiDi_Paragraph_Props *bidi_par_props, size_t start);

void
evas_common_text_props_script_set(Evas_Text_Props *props, Evas_Script_Type scr);

EAPI Eina_Bool
evas_common_text_props_content_create(void *_fi, const Eina_Unicode *text,
      Evas_Text_Props *text_props, const Evas_BiDi_Paragraph_Props *par_props,
      size_t par_pos, int len);

void
evas_common_text_props_content_copy_and_ref(Evas_Text_Props *dst,
      const Evas_Text_Props *src);

void
evas_common_text_props_content_ref(Evas_Text_Props *props);

void
evas_common_text_props_content_unref(Evas_Text_Props *props);

EAPI int
evas_common_text_props_cluster_next(const Evas_Text_Props *props, int pos);

EAPI int
evas_common_text_props_cluster_prev(const Evas_Text_Props *props, int pos);

EAPI int
evas_common_text_props_index_find(const Evas_Text_Props *props, int _cutoff);

EAPI void
evas_common_text_props_split(Evas_Text_Props *base, Evas_Text_Props *ext,
      int cutoff);
EAPI void
evas_common_text_props_merge(Evas_Text_Props *item1, const Evas_Text_Props *item2);

#endif
