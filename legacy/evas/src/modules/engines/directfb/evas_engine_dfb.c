#include "evas_engine_dfb.h"
#include <math.h>
#include <string.h>

static Evas_Func func, pfunc;


static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_DirectFB *info;

   info = calloc(1, sizeof(Evas_Engine_Info_DirectFB));
   if (!info)
      return NULL;

   info->magic.magic = rand();

   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e, void *info)
{
   Evas_Engine_Info_DirectFB *in;

   in = (Evas_Engine_Info_DirectFB *)info;
   free(in);
}

static void *eng_output_setup(int w, int h, IDirectFB *dfb, IDirectFBSurface *surf, DFBSurfaceDrawingFlags flags);

static void
eng_setup(Evas *e, void *in)
{
   Evas_Engine_Info_DirectFB *info;

   info = (Evas_Engine_Info_DirectFB *) in;
   if (!e->engine.data.output)
     e->engine.data.output =
       eng_output_setup(e->output.w,
			e->output.h,
			info->info.dfb,
			info->info.surface,
			info->info.flags);
   if (!e->engine.data.output)
      return;
   if (!e->engine.data.context)
     e->engine.data.context =
       e->engine.func->context_new(e->engine.data.output);
}

static void *
eng_output_setup(int w, int h, IDirectFB *dfb, IDirectFBSurface *surf, DFBSurfaceDrawingFlags flags)
{
   Render_Engine *re;
   DFBSurfaceDescription dsc;
   DFBResult res;
   RGBA_Image *im;

   re = calloc(1, sizeof(Render_Engine));
   /* if we haven't initialized - init (automatic abort if already done) */
   evas_common_cpu_init();

   evas_common_blend_init();
   evas_common_image_init();
   evas_common_convert_init();
   evas_common_scale_init();
   evas_common_rectangle_init();
   evas_common_gradient_init();
   evas_common_polygon_init();
   evas_common_line_init();
   evas_common_font_init();
   evas_common_draw_init();
   evas_common_tilebuf_init();

   re->tb = evas_common_tilebuf_new(w, h);
   /* in preliminary tests 16x16 gave highest framerates */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   re->dfb = dfb;
   re->surface = surf;

   memset(&dsc, 0, sizeof(DFBSurfaceDescription));
   dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
   dsc.width = (w > 0) ? w : 1;
   dsc.height = (h > 0) ? h : 1;
   dsc.pixelformat = DSPF_ARGB;

   res = dfb->CreateSurface(dfb, &dsc, &re->backbuf);
   if (res != DFB_OK)
     {
	printf("DFB engine: cannot create backbuf: %s\n",
	       DirectFBErrorString(res));
	exit(-1);
     }
   re->backbuf->SetDrawingFlags(re->backbuf, flags);

   /* We create a "fake" RGBA_Image which points the to DFB surface. Each access
    * to that surface is wrapped in Lock / Unlock calls whenever the data is
    * manipulated directly. */
   im = evas_cache_image_empty(evas_common_image_cache_get());
   im->image->w = w;
   im->image->h = h;
   im->image->no_free = 1;
   im->image->data = NULL;
   re->rgba_image = im;

   return re;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *) data;
   evas_common_tilebuf_free(re->tb);
   if (re->rects)
      evas_common_tilebuf_free_render_rects(re->rects);
   re->backbuf->Release(re->backbuf);
   evas_cache_image_drop(re->rgba_image);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;
   IDirectFBSurface *new_surf;
   DFBSurfaceDescription dsc;

   re = (Render_Engine *) data;
   if (w == re->tb->outbuf_w && h == re->tb->outbuf_h) return;
   evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb)
      evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   memset(&dsc, 0, sizeof(DFBSurfaceDescription));
   dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
   dsc.width = (w > 0) ? w : 1;
   dsc.height = (h > 0) ? h : 1;
   dsc.pixelformat = DSPF_ARGB;

   if (re->dfb->CreateSurface(re->dfb, &dsc, &new_surf) == DFB_OK)
   {
      new_surf->StretchBlit(new_surf, re->backbuf, NULL, NULL);
      re->backbuf->Release(re->backbuf);
      re->backbuf = new_surf;
      re->rgba_image->image->w = w;
      re->rgba_image->image->h = h;
      re->rgba_image->image->data = NULL;
   }
}

static void
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *) data;
   evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *) data;
   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *) data;
   evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *) data;
   evas_common_tilebuf_clear(re->tb);
}

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;
   Tilebuf_Rect *rect;
   int ux, uy, uw, uh, pitch;
   void *pixels;
   DFBRegion region;

   re = (Render_Engine *) data;
   if (re->end)
     {
	re->end = 0;
	return NULL;
     }
   if (!re->rects)
     {
	re->rects = evas_common_tilebuf_get_render_rects(re->tb);
	re->cur_rect = (Evas_Object_List *) re->rects;
     }
   if (!re->cur_rect)
      return NULL;
   rect = (Tilebuf_Rect *) re->cur_rect;
   ux = rect->x;
   uy = rect->y;
   uw = rect->w;
   uh = rect->h;
   re->cur_rect = re->cur_rect->next;
   if (!re->cur_rect)
     {
	evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	re->end = 1;
     }
   *cx = *x = ux;
   *cy = *y = uy;
   *cw = *w = uw;
   *ch = *h = uh;
   region.x1 = ux;
   region.y1 = uy;
   region.x2 = ux + uw - 1;
   region.y2 = uy + uh - 1;
   re->backbuf->SetClip(re->backbuf, &region);
   re->backbuf->Clear(re->backbuf, 0, 0, 0, 0);
   re->backbuf->SetClip(re->backbuf, NULL);

   re->backbuf->Lock(re->backbuf, DSLF_WRITE, &pixels, &pitch);
   re->rgba_image->image->data = pixels;
   return re->rgba_image;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;
   DFBRectangle rect = {x, y, w, h};
   DFBRegion region = {x, y, x + w, y + h};

   re = (Render_Engine *) data;
   re->backbuf->Unlock(re->backbuf);
   /* XXX TODO: store rect + x,y and use BatchBlit() */
   re->surface->Blit(re->surface, re->backbuf, &rect, x, y);
   re->surface->Flip(re->surface, &region, DSFLIP_NONE);
   evas_common_cpu_end_opt();

   re->rgba_image->image->data = NULL;
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *) data;
}

static void
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *) data;
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(output_free);
   ORD(output_resize);
   ORD(output_tile_size_set);
   ORD(output_redraws_rect_add);
   ORD(output_redraws_rect_del);
   ORD(output_redraws_clear);
   ORD(output_redraws_next_update_get);
   ORD(output_redraws_next_update_push);
   ORD(output_flush);
   ORD(output_idle_flush);
   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

EAPI void
module_close(void)
{

}

EAPI Evas_Module_Api evas_modapi = 
{
   EVAS_MODULE_API_VERSION, 
     EVAS_MODULE_TYPE_ENGINE,
     "directfb",
     "none"
};

