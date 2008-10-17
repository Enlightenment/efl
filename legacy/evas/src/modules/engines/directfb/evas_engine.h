#ifndef EVAS_ENGINE_DIRECTFB_H
#define EVAS_ENGINE_DIRECTFB_H

#include "evas_common.h"
#include "evas_private.h"
#include "Evas_Engine_DirectFB.h"

typedef struct _DirectFB_Engine_Image_Entry DirectFB_Engine_Image_Entry;
struct _DirectFB_Engine_Image_Entry
{
   Engine_Image_Entry            cache_entry;
   IDirectFBSurface             *surface;

   struct
   {
      Evas_Bool                   engine_surface : 1;
      Evas_Bool                   is_locked : 1;
   } flags;
};

typedef struct _Render_Engine Render_Engine;
struct _Render_Engine
{
   DirectFB_Engine_Image_Entry  *screen_image;
   const struct Evas_Engine_DirectFB_Spec *spec;
   IDirectFB                    *dfb;

   Evas_Cache_Engine_Image      *cache;

   Tilebuf                      *tb;
   Tilebuf_Rect                 *rects;
   Eina_Inlist                  *cur_rect;

   DFBRegion                    *update_regions;
   unsigned int                  update_regions_count;
   unsigned int                  update_regions_limit;

   Evas_Bool                     end : 1;
};

int _dfb_surface_set_color_from_context(IDirectFBSurface *surface, RGBA_Draw_Context *dc);
void _dfb_polygon_draw(IDirectFBSurface *surface, RGBA_Draw_Context *dc, Eina_Inlist *points);

#endif
