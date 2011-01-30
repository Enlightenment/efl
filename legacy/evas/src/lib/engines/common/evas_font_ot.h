#ifndef _EVAS_FONT_OT_H
# define _EVAS_FONT_OT_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

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

# include "Evas.h"

# ifdef OT_SUPPORT
struct _Evas_Font_OT_Info
{
   size_t source_cluster;
   Evas_Coord x_offset;
   Evas_Coord y_offset;
};
# endif

# ifdef OT_SUPPORT
#  define EVAS_FONT_OT_X_OFF_GET(a) ((a).x_offset)
#  define EVAS_FONT_OT_Y_OFF_GET(a) ((a).y_offset)
#  define EVAS_FONT_OT_POS_GET(a)   ((a).source_cluster)
# endif

EAPI Eina_Bool
evas_common_font_ot_is_enabled(void);

EAPI void
evas_common_font_ot_load_face(void *_font);

EAPI void
evas_common_font_ot_unload_face(void *_font);

# include "evas_text_utils.h"
EAPI int
evas_common_font_ot_cluster_size_get(const Evas_Text_Props *props, size_t char_index);

EAPI Eina_Bool
evas_common_font_ot_populate_text_props(void *fn, const Eina_Unicode *text,
      Evas_Text_Props *props, int len);

EAPI void
evas_common_font_ot_cutoff_text_props(Evas_Text_Props *props, int cutoff);

EAPI void
evas_common_font_ot_split_text_props(Evas_Text_Props *base, Evas_Text_Props *ext, int cutoff);

EAPI void
evas_common_font_ot_merge_text_props(Evas_Text_Props *item1, const Evas_Text_Props *item2);
#endif

