#ifndef EVAS_ENGINE_SOFTWARE_GENERIC_H_
# define EVAS_ENGINE_SOFTWARE_GENERIC_H_

#include "Evas_Engine_Software_Shared.h"

typedef enum _Outbuf_Depth
{
   OUTBUF_DEPTH_NONE,
   OUTBUF_DEPTH_INHERIT,

   /* From X11 */
   OUTBUF_DEPTH_RGB_16BPP_565_565_DITHERED,
   OUTBUF_DEPTH_RGB_16BPP_555_555_DITHERED,
   OUTBUF_DEPTH_RGB_16BPP_444_444_DITHERED,
   OUTBUF_DEPTH_RGB_16BPP_565_444_DITHERED,
   OUTBUF_DEPTH_RGB_32BPP_888_8888,

   /* From buffer */
   OUTBUF_DEPTH_ARGB_32BPP_8888_8888,
   OUTBUF_DEPTH_BGRA_32BPP_8888_8888,
   OUTBUF_DEPTH_BGR_32BPP_888_8888,
   OUTBUF_DEPTH_RGB_24BPP_888_888,
   OUTBUF_DEPTH_BGR_24BPP_888_888,

   OUTBUF_DEPTH_LAST
} Outbuf_Depth;

typedef enum
{
   MERGE_BOUNDING = 1,
   MERGE_FULL = 2,
   MERGE_SMART = 4
} Render_Output_Merge_Mode;

typedef struct _Render_Output_Software_Generic Render_Output_Software_Generic;
typedef struct _Outbuf Outbuf;

typedef Render_Output_Swap_Mode (*Outbuf_Swap_Mode_Get)(Outbuf *ob);
typedef void (*Outbuf_Reconfigure)(Outbuf *ob, int w, int h, int rot, Outbuf_Depth depth);
typedef Eina_Bool (*Outbuf_Region_First_Rect)(Outbuf *ob);
typedef void (*Outbuf_Damage_Region_Set)(Outbuf *ob, Tilebuf_Rect *rects);
typedef void *(*Outbuf_New_Region_For_Update)(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
typedef void (*Outbuf_Push_Updated_Region)(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
typedef void (*Outbuf_Idle_Flush)(Outbuf *ob);
typedef void (*Outbuf_Free_Region_For_Update)(Outbuf *ob, RGBA_Image *update);
typedef void (*Outbuf_Free)(Outbuf *ob);
typedef int (*Outbuf_Get_Rot)(Outbuf *ob);
typedef void (*Outbuf_Flush)(Outbuf *ob, Tilebuf_Rect *surface_damage, Tilebuf_Rect *buffer_damage, Evas_Render_Mode render_mode);
typedef void (*Outbuf_Redraws_Clear)(Outbuf *ob);

struct _Render_Output_Software_Generic
{
   Outbuf *ob;
   Tilebuf *tb;
   Tilebuf_Rect *rects;
   Tilebuf_Rect *rects_prev[4];
   Eina_Inlist *cur_rect;

   Outbuf_Swap_Mode_Get outbuf_swap_mode_get;
   Outbuf_Get_Rot outbuf_get_rot;
   Outbuf_Reconfigure outbuf_reconfigure;
   Outbuf_Region_First_Rect outbuf_region_first_rect;
   Outbuf_Damage_Region_Set outbuf_damage_region_set;
   Outbuf_New_Region_For_Update outbuf_new_region_for_update;
   Outbuf_Push_Updated_Region outbuf_push_updated_region;
   Outbuf_Idle_Flush outbuf_idle_flush;
   Outbuf_Free_Region_For_Update outbuf_free_region_for_update;
   Outbuf_Free outbuf_free;
   Outbuf_Flush outbuf_flush;
   Outbuf_Redraws_Clear outbuf_redraws_clear;

   unsigned int w, h;

   Render_Output_Swap_Mode swap_mode;
   Render_Output_Merge_Mode merge_mode;

   unsigned char end : 1;
   unsigned char lost_back : 1;
   unsigned char tile_strict : 1;
};

static inline Eina_Bool
evas_render_engine_software_generic_init(Render_Output_Software_Generic *re,
                                         Outbuf *ob,
                                         Outbuf_Swap_Mode_Get outbuf_swap_mode_get,
                                         Outbuf_Get_Rot outbuf_get_rot,
                                         Outbuf_Reconfigure outbuf_reconfigure,
                                         Outbuf_Region_First_Rect outbuf_region_first_rect,
                                         Outbuf_Damage_Region_Set outbuf_damage_region_set,
                                         Outbuf_New_Region_For_Update outbuf_new_region_for_update,
                                         Outbuf_Push_Updated_Region outbuf_push_updated_region,
                                         Outbuf_Free_Region_For_Update outbuf_free_region_for_update,
                                         Outbuf_Idle_Flush outbuf_idle_flush,
                                         Outbuf_Flush outbuf_flush,
                                         Outbuf_Redraws_Clear outbuf_redraws_clear,
                                         Outbuf_Free outbuf_free,
                                         int w, int h)
{
   unsigned int i;

   re->ob = ob;
   re->outbuf_swap_mode_get = outbuf_swap_mode_get;
   re->outbuf_get_rot = outbuf_get_rot;
   re->outbuf_reconfigure = outbuf_reconfigure;
   re->outbuf_region_first_rect = outbuf_region_first_rect;
   re->outbuf_damage_region_set = outbuf_damage_region_set;
   re->outbuf_new_region_for_update = outbuf_new_region_for_update;
   re->outbuf_push_updated_region = outbuf_push_updated_region;
   re->outbuf_idle_flush = outbuf_idle_flush;
   re->outbuf_free_region_for_update = outbuf_free_region_for_update;
   re->outbuf_free = outbuf_free;
   re->outbuf_flush = outbuf_flush;
   re->outbuf_redraws_clear = outbuf_redraws_clear;

   re->rects = NULL;
   for (i = 0; i < 4; i++)
     re->rects_prev[i] = NULL;
   re->cur_rect = NULL;

   re->w = w;
   re->h = h;
   re->swap_mode = MODE_FULL;
   re->merge_mode = MERGE_FULL;
   re->end = 0;
   re->lost_back = 0;
   re->tile_strict = 0;

   re->tb = evas_common_tilebuf_new(w, h);
   if (!re->tb) return EINA_FALSE;

   /* in preliminary tests 16x16 gave highest framerates */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   return EINA_TRUE;
}

static inline void
evas_render_engine_software_generic_clean(Render_Output_Software_Generic *re)
{
   if (re->tb) evas_common_tilebuf_free(re->tb);
   if (re->ob) re->outbuf_free(re->ob);

   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   if (re->rects_prev[0]) evas_common_tilebuf_free_render_rects(re->rects_prev[0]);
   if (re->rects_prev[1]) evas_common_tilebuf_free_render_rects(re->rects_prev[1]);
   if (re->rects_prev[2]) evas_common_tilebuf_free_render_rects(re->rects_prev[2]);
   if (re->rects_prev[3]) evas_common_tilebuf_free_render_rects(re->rects_prev[3]);

   memset(re, 0, sizeof (Render_Output_Software_Generic));
}

static inline void
evas_render_engine_software_generic_merge_mode_set(Render_Output_Software_Generic *re)
{
   Render_Output_Merge_Mode merge_mode = MERGE_SMART;
   const char *s;

   s = getenv("EVAS_GL_PARTIAL_MERGE");
   if (s)
     {
        if ((!strcmp(s, "bounding")) || (!strcmp(s, "b")))
          merge_mode = MERGE_BOUNDING;
        else if ((!strcmp(s, "full")) || (!strcmp(s, "f")))
          merge_mode = MERGE_FULL;
        else if ((!strcmp(s, "smart")) || (!strcmp(s, "s")))
          merge_mode = MERGE_SMART;
     }

   re->merge_mode = merge_mode;
}

static inline void
evas_render_engine_software_generic_tile_strict_set(Render_Output_Software_Generic *re,
                                                    Eina_Bool tile_strict)
{
   re->tile_strict = !!tile_strict;
   evas_common_tilebuf_tile_strict_set(re->tb, re->tile_strict);
}

static inline Eina_Bool
evas_render_engine_software_generic_update(Render_Output_Software_Generic *re,
                                           Outbuf *ob,
                                           int w, int h)
{
   if ((re->ob) && (re->ob != ob)) re->outbuf_free(re->ob);
   re->ob = ob;

   evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(w, h);
   if (!re->tb) return EINA_FALSE;
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   evas_render_engine_software_generic_tile_strict_set(re, re->tile_strict);
   return EINA_TRUE;
}

#endif
