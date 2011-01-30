#ifndef _EVAS_FONT_METRICS_H
# define _EVAS_FONT_METRICS_H
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
   Evas_Coord advance;
};

typedef struct _Evas_Font_Glyph_Info Evas_Font_Glyph_Info;

EAPI Eina_Bool
evas_common_font_glyph_info_create(void *_fn, const Eina_Unicode *text,
      Evas_Text_Props *text_props, int len);

#endif

