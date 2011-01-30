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
#include <stdlib.h>
typedef struct _Evas_Font_OT_Data Evas_Font_OT_Data;
typedef struct _Evas_Font_OT_Data_Item Evas_Font_OT_Data_Item;
struct _Evas_Font_OT_Data
{
   int refcount;
   size_t len;
   Evas_Font_OT_Data_Item *items;
};
# else
typedef void *Evas_Font_OT_Data;
#endif

#include "Evas.h"

#ifdef OT_SUPPORT
struct _Evas_Font_OT_Data_Item
{
   unsigned int index; /* Should conform to FT */
   size_t source_cluster;
   Evas_Coord x_offset;
   Evas_Coord y_offset;
   Evas_Coord x_advance;
};
# endif

# ifdef OT_SUPPORT
#  define EVAS_FONT_OT_X_OFF_GET(a) ((a).x_offset)
#  define EVAS_FONT_OT_Y_OFF_GET(a) ((a).y_offset)
#  define EVAS_FONT_OT_X_ADV_GET(a) ((a).x_advance)
//#  define EVAS_FONT_OT_Y_ADV_GET(a) ((a).y_advance)
#  define EVAS_FONT_OT_INDEX_GET(a) ((a).index)
#  define EVAS_FONT_OT_POS_GET(a)   ((a).source_cluster)
#else
#  define EVAS_FONT_OT_X_OFF_GET(a) (0)
#  define EVAS_FONT_OT_Y_OFF_GET(a) (0)
#  define EVAS_FONT_OT_X_ADV_GET(a) (0)
//#  define EVAS_FONT_OT_POS_Y_ADV_GET(a) (0)
#  define EVAS_FONT_OT_INDEX_GET(a) (0) /* FIXME!!! */
#  define EVAS_FONT_OT_POS_GET(a) (0) /* FIXME!!! */
# endif

EAPI Eina_Bool
evas_common_font_ot_is_enabled(void);

EAPI void
evas_common_font_ot_props_ref(Evas_Font_OT_Data *data);

EAPI void
evas_common_font_ot_props_unref(Evas_Font_OT_Data *data);

EAPI void
evas_common_font_ot_load_face(void *_font);

EAPI void
evas_common_font_ot_unload_face(void *_font);

#include "evas_text_utils.h"
EAPI int
evas_common_font_ot_cluster_size_get(const Evas_Text_Props *props, size_t char_index, int orig_len);

EAPI Eina_Bool
evas_common_font_ot_populate_text_props(void *fn, const Eina_Unicode *text,
      Evas_Text_Props *props, int len);

EAPI void
evas_common_font_ot_cutoff_text_props(Evas_Text_Props *props, int cutoff);
#endif

