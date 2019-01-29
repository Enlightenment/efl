#include "evas_engine.h"
#include <drm_fourcc.h>

Outbuf *
_outbuf_setup(Evas_Engine_Info_Drm *einfo, int w, int h)
{
   Outbuf *ob;

   ob = calloc(1, sizeof(Outbuf));
   if (!ob) return NULL;

   DBG("Outbuf %p Setup: %d %d", ob, w, h);

   ob->w = w;
   ob->h = h;
   ob->bpp = einfo->info.bpp;
   ob->depth = einfo->info.depth;
   ob->alpha = einfo->info.alpha;
   ob->format = einfo->info.format;
   ob->rotation = einfo->info.rotation;

   return ob;
}

void
_outbuf_free(Outbuf *ob)
{
   DBG("Outbuf %p Free", ob);
   free(ob);
}

int
_outbuf_get_rotation(Outbuf *ob)
{
   return ob->rotation;
}

void
_outbuf_reconfigure(Outbuf *ob, int w, int h, int rotation, Outbuf_Depth depth)
{
   unsigned int format = DRM_FORMAT_ARGB8888;

   switch (depth)
     {
      case OUTBUF_DEPTH_RGB_16BPP_565_565_DITHERED:
        format = DRM_FORMAT_RGB565;
        break;
      case OUTBUF_DEPTH_RGB_16BPP_555_555_DITHERED:
        format = DRM_FORMAT_RGBX5551;
        break;
      case OUTBUF_DEPTH_RGB_16BPP_444_444_DITHERED:
        format = DRM_FORMAT_RGBX4444;
        break;
      case OUTBUF_DEPTH_RGB_16BPP_565_444_DITHERED:
        format = DRM_FORMAT_RGB565;
        break;
      case OUTBUF_DEPTH_RGB_32BPP_888_8888:
        format = DRM_FORMAT_RGBX8888;
        break;
      case OUTBUF_DEPTH_ARGB_32BPP_8888_8888:
        format = DRM_FORMAT_ARGB8888;
        break;
      case OUTBUF_DEPTH_BGRA_32BPP_8888_8888:
        format = DRM_FORMAT_BGRA8888;
        break;
      case OUTBUF_DEPTH_BGR_32BPP_888_8888:
        format = DRM_FORMAT_BGRX8888;
        break;
      case OUTBUF_DEPTH_RGB_24BPP_888_888:
        format = DRM_FORMAT_RGB888;
        break;
      case OUTBUF_DEPTH_BGR_24BPP_888_888:
        format = DRM_FORMAT_BGR888;
        break;
      case OUTBUF_DEPTH_INHERIT:
      default:
        depth = ob->depth;
        format = ob->format;
        break;
     }

   if ((ob->w == w) && (ob->h == h) && (ob->rotation == rotation) &&
       (ob->depth == depth) && (ob->format == format))
     return;

   DBG("Outbuf %p Reconfigure: %d %d %d", ob, w, h, rotation);

   ob->w = w;
   ob->h = h;
   ob->depth = depth;
   ob->format = format;
   ob->rotation = rotation;
}

void *
_outbuf_new_region_for_update(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch)
{
   DBG("Outbuf %p New Region: %d %d %d %d", ob, x, y, w, h);
   if (cx) *cx = 0;
   if (cy) *cy = 0;
   if (cw) *cw = w;
   if (ch) *ch = h;
   return NULL;
}
