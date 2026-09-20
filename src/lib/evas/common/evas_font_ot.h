#ifndef _EVAS_FONT_OT_H
# define _EVAS_FONT_OT_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

# ifdef HAVE_HARFBUZZ
#  define OT_SUPPORT
#  define USE_HARFBUZZ
# endif

# ifdef OT_SUPPORT
#  include <stdlib.h>
typedef struct _Evas_Font_OT_Info Evas_Font_OT_Info;
# else
typedef void *Evas_Font_OT_Info;
# endif

# ifdef OT_SUPPORT
struct _Evas_Font_OT_Info
{
   size_t source_cluster;
   int x_offset;
   int y_offset;
};
# endif

# ifdef OT_SUPPORT
#  define EVAS_FONT_OT_X_OFF_GET(a) ((a).x_offset)
#  define EVAS_FONT_OT_Y_OFF_GET(a) ((a).y_offset)
#  define EVAS_FONT_OT_POS_GET(a)   ((a).source_cluster)
# endif

#include "evas_font.h"
#include "Evas.h"

struct _RGBA_Font_Int;

/* Codepoints that can take part in a GSUB ligature or contextual
 * substitution in a font instance. */
typedef struct _Evas_Font_Liga_Triggers Evas_Font_Liga_Triggers;
struct _Evas_Font_Liga_Triggers
{
   uint64_t       ascii[2];   /* bitmap over codepoints 0x00 - 0x7F */
   Eina_Unicode  *extra;      /* ascending array of triggers >= 0x80, or NULL */
   unsigned int   extra_cnt;
};

static inline Eina_Bool
evas_common_font_liga_trigger_check(const Evas_Font_Liga_Triggers *t,
                                    Eina_Unicode cp)
{
   unsigned int lo, hi;

   if (EINA_LIKELY(cp < 0x80))
     return (Eina_Bool)((t->ascii[cp >> 6] >> (cp & 63)) & UINT64_C(1));
   if (EINA_LIKELY(!t->extra_cnt)) return EINA_FALSE;

   lo = 0;
   hi = t->extra_cnt;
   while (lo < hi)
     {
        unsigned int mid = lo + ((hi - lo) >> 1);

        if (t->extra[mid] < cp) lo = mid + 1;
        else if (t->extra[mid] > cp) hi = mid;
        else return EINA_TRUE;
     }
   return EINA_FALSE;
}

/* Fill @p out with the codepoints covered by the liga, clig, calt, dlig and
 * rlig lookups of @p fi.  Returns EINA_FALSE with @p out zeroed if there are
 * none.  Walks the whole cmap: call it once per font load. */
EVAS_API Eina_Bool
evas_common_font_ot_ligature_triggers_get(struct _RGBA_Font_Int *fi,
                                          Evas_Font_Liga_Triggers *out);

EVAS_API void
evas_common_font_ot_ligature_triggers_clear(Evas_Font_Liga_Triggers *t);

EVAS_API int
evas_common_font_ot_cluster_size_get(const Evas_Text_Props *props, size_t char_index);

EVAS_API Eina_Bool
evas_common_font_ot_populate_text_props(const Eina_Unicode *text,
      Evas_Text_Props *props, int len, Evas_Text_Props_Mode mode, const char *lang);
#endif

