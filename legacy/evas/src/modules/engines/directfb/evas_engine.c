#include <assert.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "evas_engine.h"

/* Uses Evas own image_draw primitive, for comparison purposes only. */
//#define DFB_USE_EVAS_IMAGE_DRAW 1
//#define DFB_USE_EVAS_RECT_DRAW 1
//#define DFB_USE_EVAS_POLYGON_DRAW 1
//#define DFB_UPDATE_INDIVIDUAL_RECTS 1
#define DFB_FLIP_FLAGS DSFLIP_NONE
//#define DFB_FLIP_FLAGS (DSFLIP_ONSYNC | DSFLIP_WAIT)

/* Turn on debug */
//#define DFB_DEBUG_IMAGE 1
//#define DFB_DEBUG_FLAGS 1
//#define DFB_DEBUG_ACCELERATION 1

static Evas_Func func = {};
static Evas_Func parent_func = {};
static IDirectFB *dfb = NULL; /* XXX HACK to work around evas image cache
			       * lack of extra data. Fix it instead later.
			       */


/***********************************************************************
 * Evas helpers
 **********************************************************************/
static void
_rect_set(Evas_Rectangle *r, int x, int y, int w, int h)
{
   r->x = x;
   r->y = y;
   r->w = w;
   r->h = h;
}

static void
_context_get_color(RGBA_Draw_Context *dc, int *r, int *g, int *b, int *a)
{
   DATA32 col;

   if (dc->mul.use)
     col = dc->mul.col;
   else
     col = dc->col.col;

   *r = R_VAL(&col);
   *g = G_VAL(&col);
   *b = B_VAL(&col);
   *a = A_VAL(&col);
}


/***********************************************************************
 * DirectFB helpers
 **********************************************************************/
static void
_dfb_surface_clear(IDirectFBSurface *surface, int x, int y, int w, int h)
{
   DFBRegion cr;
   DFBResult r;

   cr.x1 = x;
   cr.y1 = y;
   cr.x2 = x + w - 1;
   cr.y2 = y + h - 1;
   r = surface->SetClip(surface, &cr);
   if (r != DFB_OK)
     goto error;

   r = surface->Clear(surface, 0, 0, 0, 0);
   if (r != DFB_OK)
     goto error;

   return;

 error:
   fprintf(stderr, "ERROR: could not clear surface: %s\n",
	   DirectFBErrorString(r));
}

static void
_image_clear(DirectFB_Engine_Image_Entry *image, int x, int y, int w, int h)
{
   if (image->cache_entry.src->flags.alpha)
     _dfb_surface_clear(image->surface, x, y, w, h);
}

static void
_image_autoset_alpha(DirectFB_Engine_Image_Entry *image)
{
   DFBResult r;
   DFBSurfacePixelFormat fmt;
   IDirectFBSurface *surface;
   RGBA_Image *im;
   int has_alpha;

   surface = image->surface;
   r = surface->GetPixelFormat(surface, &fmt);
   if (r != DFB_OK)
     {
	fprintf(stderr, "ERROR: could not get pixel format: %s\n",
		DirectFBErrorString(r));
	return;
     }

   /* XXX: check this in more depth in future, if other PF are supported */
   image->cache_entry.src->flags.alpha = (fmt == DSPF_ARGB);
}

static void
_dfb_surface_update(IDirectFBSurface *surface, int x, int y, int w, int h)
{
   DFBRegion cr;
   DFBResult r;

   cr.x1 = x;
   cr.y1 = y;
   cr.x2 = x + w - 1;
   cr.y2 = y + h - 1;
   r = surface->Flip(surface, &cr, DSFLIP_NONE);
   if (r != DFB_OK)
     fprintf(stderr, "ERROR: could not update surface: %s\n",
	     DirectFBErrorString(r));
}

static IDirectFBSurface *
_dfb_surface_from_data(IDirectFB *dfb, int w, int h, void *data)
{
   IDirectFBSurface *s;
   DFBSurfaceDescription desc;
   DFBResult r;

   desc.flags = (DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT |
		 DSDESC_PIXELFORMAT | DSDESC_PREALLOCATED);
   desc.caps = DSCAPS_PREMULTIPLIED;
   desc.width = w;
   desc.height = h;
   desc.preallocated[0].data = data;
   desc.preallocated[0].pitch = w * 4;
   desc.preallocated[1].data = NULL;
   desc.preallocated[1].pitch = 0;
   desc.pixelformat = DSPF_ARGB;
   r = dfb->CreateSurface(dfb, &desc, &s);
   if (r != DFB_OK)
     {
	fprintf(stderr, "ERROR: cannot create DirectFB surface: %s\n",
		DirectFBErrorString(r));
	return NULL;
     }

   s->SetPorterDuff(s, DSPD_SRC_OVER);

   return s;
}

static void
_dfb_surface_free(IDirectFBSurface *surface)
{
   if (surface)
     surface->Release(surface);
}

static void
_dfb_blit_accel_caps_print(IDirectFBSurface *dst, IDirectFBSurface *src)
{
#ifdef DFB_DEBUG_ACCELERATION
   DFBAccelerationMask mask;
   DFBResult r;

   r = dst->GetAccelerationMask(dst, src, &mask);
   if (r != DFB_OK)
     {
	fprintf(stderr, "ERROR: Could not retrieve acceleration mask: %s\n",
		DirectFBErrorString(r));
	return;
     }

   fputs("Acceleration: ", stderr);

#define O(m) if (mask & m) fputs(#m " ", stderr)
   O(DFXL_FILLRECTANGLE);
   O(DFXL_DRAWRECTANGLE);
   O(DFXL_DRAWLINE);
   O(DFXL_FILLTRIANGLE);
   O(DFXL_BLIT);
   O(DFXL_STRETCHBLIT);
   O(DFXL_TEXTRIANGLES);
   O(DFXL_DRAWSTRING);
#undef O

   if (mask == DFXL_NONE) fputs("<NONE>", stderr);
   fputc('\n', stderr);
#endif /* DFB_DEBUG_ACCELERATION */
}

#ifdef DFB_DEBUG_FLAGS
static const char *
_dfb_blit_flags_str(DFBSurfaceBlittingFlags flags)
{
   static char buf[1024];

   buf[0] = 0;

#define T(m, n)					\
   do {						\
      if (flags & m) {				\
	 if (buf[0] != 0) strcat(buf, " | ");	\
	 strcat(buf, n);			\
      }						\
   } while (0)

   T(DSBLIT_BLEND_ALPHACHANNEL, "BLEND_ALPHACHANNEL");
   T(DSBLIT_BLEND_COLORALPHA, "BLEND_COLORALPHA");
   T(DSBLIT_COLORIZE, "COLORIZE");
   T(DSBLIT_SRC_COLORKEY, "SRC_COLORKEY");
   T(DSBLIT_DST_COLORKEY, "DST_COLORKEY");
   T(DSBLIT_SRC_PREMULTIPLY, "SRC_PREMULTIPLY");
   T(DSBLIT_DST_PREMULTIPLY, "DST_PREMULTIPLY");
   T(DSBLIT_DEMULTIPLY, "DEMULTIPLY");
   T(DSBLIT_DEINTERLACE, "DSBLIT_DEINTERLACE");
   T(DSBLIT_SRC_PREMULTCOLOR, "SRC_PREMULTCOLOR");
   T(DSBLIT_XOR, "XOR");
   T(DSBLIT_INDEX_TRANSLATION, "INDEX_TRANSLATION");
#undef T

   if (buf[0] == 0)
     strcpy(buf, "NOFX");

   return buf;
}

static const char *
_dfb_draw_flags_str(DFBSurfaceDrawingFlags flags)
{
   static char buf[1024];

   buf[0] = 0;

#define T(m, n)					\
   do {						\
      if (flags & m) {				\
	 if (buf[0] != 0) strcat(buf, " | ");	\
	 strcat(buf, n);			\
      }						\
   } while (0)
   T(DSDRAW_BLEND, "BLEND");
   T(DSDRAW_DST_COLORKEY, "DST_COLORKEY");
   T(DSDRAW_SRC_PREMULTIPLY, "SRC_PREMULTIPLY");
   T(DSDRAW_DST_PREMULTIPLY, "DST_PREMULTIPLY");
   T(DSDRAW_DEMULTIPLY, "DEMULTIPLY");
   T(DSDRAW_XOR, "DSDRAW_XOR");
#undef T
   if (buf[0] == 0)
     strcpy(buf, "NOFX");

   return buf;
}

static const char *
_dfb_blend_func_str(DFBSurfaceBlendFunction func)
{
   static char *names[] = {
     "ZERO",
     "ONE",
     "SRCCOLOR",
     "INVSRCCOLOR",
     "SRCALPHA",
     "INVSRCALPHA",
     "DESTALPHA",
     "INVDESTALPHA",
     "DESTCOLOR",
     "INVDESTCOLOR",
     "SRCALPHASAT"
   };
   func--;
   if ((func >= 0) && (func <= sizeof(names)/sizeof(*names)))
     return names[func];
   else
     return NULL;
}
#endif /* DFB_DEBUG_FLAGS */

int
_dfb_surface_set_color_from_context(IDirectFBSurface *surface, RGBA_Draw_Context *dc)
{
   DFBSurfaceDrawingFlags flags;
   int r, g, b, a;
   DFBResult res;

   _context_get_color(dc, &r, &g, &b, &a);
   if (a == 0)
     return 0;

   r = 0xff * r / a;
   g = 0xff * g / a;
   b = 0xff * b / a;

   res = surface->SetColor(surface, r, g, b, a);
   if (res != DFB_OK)
     goto error;

   flags = (a != 255) ? DSDRAW_BLEND : DSDRAW_NOFX;
   res = surface->SetDrawingFlags(surface, flags);
   if (res != DFB_OK)
     goto error;

#ifdef DFB_DEBUG_FLAGS
   printf("DRAW: color=%d %d %d %d, flags=%s\n",
	  r, g, b, a, _dfb_draw_flags_str(flags));
#endif /* DFB_DEBUG_FLAGS */

   return 1;

 error:
   fprintf(stderr, "ERROR: could not set color from context: %s\n",
	   DirectFBErrorString(res));
   return 0;
}

static int
_dfb_surface_set_blit_params(DirectFB_Engine_Image_Entry *d, DirectFB_Engine_Image_Entry *s, RGBA_Draw_Context *dc)
{
   IDirectFBSurface *surface;
   DFBSurfaceBlittingFlags blit_flags = DSBLIT_NOFX;
   DFBResult res;
   int r, g, b, a;

   _context_get_color(dc, &r, &g, &b, &a);
   if (a == 0)
     return 0;

   if (a != 255)
     blit_flags = DSBLIT_BLEND_COLORALPHA | DSBLIT_SRC_PREMULTCOLOR;

   if ((r != a) || (g != a) || (b != a))
     {
	blit_flags |= DSBLIT_COLORIZE;

	r = 0xff * r / a;
	g = 0xff * g / a;
	b = 0xff * b / a;
   }

   if (s->cache_entry.src->flags.alpha)
     blit_flags |= DSBLIT_BLEND_ALPHACHANNEL;

   surface = d->surface;

   if (blit_flags &
       (DSBLIT_BLEND_COLORALPHA | DSBLIT_SRC_PREMULTCOLOR | DSBLIT_COLORIZE))
     {
	res = surface->SetColor(surface, r, g, b, a);
	if (res != DFB_OK)
	  goto error;
     }

   res = surface->SetBlittingFlags(surface, blit_flags);
   if (res != DFB_OK)
     goto error;

#ifdef DFB_DEBUG_FLAGS
   printf("BLIT: sfunc=%s, dfunc=%s, color=%d %d %d %d\n\tblit=%s\n\tdraw=%s\n",
	  _dfb_blend_func_str(src_func), _dfb_blend_func_str(dst_func),
	  r, g, b, a,
	  _dfb_blit_flags_str(blit_flags), _dfb_draw_flags_str(draw_flags));
#endif /* DFB_DEBUG_FLAGS */

   return 1;

 error:
   fprintf(stderr, "ERROR: Could not set blit params: %s\n",
	   DirectFBErrorString(res));
   return 0;
}

static int
_dfb_lock_and_sync_image(IDirectFBSurface *surface, RGBA_Image *image, DFBSurfaceLockFlags flags)
{
   DFBResult r;
   int pitch, sw, sh;
   void *pixels;

   r = surface->GetSize(surface, &sw, &sh);
   if (r != DFB_OK)
     return 0;

   r = surface->Lock(surface, flags, &pixels, &pitch);
   if (r != DFB_OK)
     return 0;

   if (pitch != (sw * 4))
     {
	/* XXX TODO: support other pixel formats. */
	fprintf(stderr,
		"ERROR: IDirectFBSurface pitch(%d) is not supported: "
		"should be %d.\n",
		pitch, sw * 4);
	surface->Unlock(surface);
	return 0;
     }

   image->cache_entry.w = sw;
   image->cache_entry.h = sh;
   image->image.data = pixels;
   return 1;
}

typedef void (*_cb_for_each_cutout_t)(IDirectFBSurface *surface, RGBA_Draw_Context *dc, const DFBRegion *region, void *data);
static void
_dfb_surface_for_each_cutout(IDirectFBSurface *surface, RGBA_Draw_Context *dc, _cb_for_each_cutout_t cb, void *data)
{
   Cutout_Rects *rects;
   int i;

   rects = evas_common_draw_context_apply_cutouts(dc);
   for (i = 0; i < rects->active; ++i)
     {
	Cutout_Rect *r;
	DFBRegion cr;

	r = rects->rects + i;

	cr.x1 = r->x;
	cr.y1 = r->y;
	cr.x2 = r->x + r->w - 1;
	cr.y2 = r->y + r->h - 1;
	surface->SetClip(surface, &cr);
	cb(surface, dc, &cr, data);
     }
   evas_common_draw_context_apply_clear_cutouts(rects);
}

static void
_dfb_rect_set(DFBRectangle *r, int x, int y, int w, int h)
{
   r->x = x;
   r->y = y;
   r->w = w;
   r->h = h;
}


/***********************************************************************
 * Image Cache
 **********************************************************************/
static Engine_Image_Entry *
evas_cache_image_dfb_alloc(void)
{
   DirectFB_Engine_Image_Entry *deie;

   deie = calloc(1, sizeof (DirectFB_Engine_Image_Entry));

   return (Engine_Image_Entry *)deie;
}

static void
evas_cache_image_dfb_delete(Engine_Image_Entry *eie)
{
   free(eie);
}

static int
evas_cache_image_dfb_constructor(Engine_Image_Entry *eie, void *data)
{
   DirectFB_Engine_Image_Entry *deie = (DirectFB_Engine_Image_Entry *)eie;
   Render_Engine *re = data;
   IDirectFBSurface *s;
   RGBA_Image *im;

   im = (RGBA_Image *)eie->src;
   if (!im)
     return 0;

   evas_cache_image_load_data(&im->cache_entry);
   if (!im->image.data)
     return 0;

   s = _dfb_surface_from_data(re->spec->dfb, eie->w, eie->h, im->image.data);
   if (!s)
     return -1;

   deie->surface = s;
   deie->flags.engine_surface = 0;

   return 0;
}

static void
evas_cache_image_dfb_destructor(Engine_Image_Entry *eie)
{
   DirectFB_Engine_Image_Entry *deie = (DirectFB_Engine_Image_Entry *)eie;

   if (!deie->flags.engine_surface)
     _dfb_surface_free(deie->surface);
   deie->surface = NULL;
}

/* note: dst have some properties set, like desired size (w, h) */
static int
_cache_image_copy(Engine_Image_Entry *dst, const Engine_Image_Entry *src)
{
   DirectFB_Engine_Image_Entry *dst_deie;
   const DirectFB_Engine_Image_Entry *src_deie;
   IDirectFBSurface *s;
   RGBA_Image *im;

   dst_deie = (DirectFB_Engine_Image_Entry *)dst;
   src_deie = (const DirectFB_Engine_Image_Entry *)src;
   im = (RGBA_Image *)dst->src;
   s = _dfb_surface_from_data(dfb, dst->w, dst->h, im->image.data);
   if (!s)
     return -1;

   dst_deie->surface = s;
   dst_deie->flags.engine_surface = 0;

   return 0;
}

static int
evas_cache_image_dfb_dirty(Engine_Image_Entry *dst, const Engine_Image_Entry *src)
{
   return _cache_image_copy(dst, src);
}

static void
evas_cache_image_dfb_dirty_region(Engine_Image_Entry *eim, int x, int y, int w, int h)
{
   RGBA_Image *im;

   im = (RGBA_Image *)eim->src;
   im->flags |= RGBA_IMAGE_IS_DIRTY;
}

static int
evas_cache_image_dfb_update_data(Engine_Image_Entry *dst, void *engine_data)
{
   DirectFB_Engine_Image_Entry *deie = (DirectFB_Engine_Image_Entry *)dst;
   IDirectFBSurface *s = engine_data;
   Image_Entry *ie;
   RGBA_Image *im;

   ie = dst->src;
   im = (RGBA_Image *)ie;

   if (s)
     {
	deie->surface = s;

	/* XXX why size is required here? */
	s->GetSize(s, &dst->w, &dst->h);

        if (im)
          {
             im->image.data = NULL; /* IDirectFBSurface requires lock */
             im->image.no_free = 1;
             ie->w = dst->w;
	     ie->h = dst->h;
	     _image_autoset_alpha(deie);
          }
     }
   else
     {
	_dfb_surface_free(deie->surface);
	s = _dfb_surface_from_data(dfb, dst->w, dst->h, im->image.data);
	deie->surface = s;
     }

   return 0;
}

static int
evas_cache_image_dfb_size_set(Engine_Image_Entry *dst, const Engine_Image_Entry *src)
{
   return _cache_image_copy(dst, src);
}

static void
evas_cache_image_dfb_load(Engine_Image_Entry *eim, const Image_Entry *ie)
{
   DirectFB_Engine_Image_Entry *deie = (DirectFB_Engine_Image_Entry *)eim;
   IDirectFBSurface *s;
   const RGBA_Image *im;

   if (deie->surface)
     return;

   im = (const RGBA_Image *)ie;
   s = _dfb_surface_from_data(dfb, eim->w, eim->h, im->image.data);
   deie->surface = s;
}

static int
evas_cache_image_dfb_mem_size_get(Engine_Image_Entry *eie)
{
   DirectFB_Engine_Image_Entry *deie = (DirectFB_Engine_Image_Entry *)eie;
   DFBResult r;
   int size, w, h;

   if (!deie->surface)
     return 0;

   size = sizeof(*deie->surface);

   r = deie->surface->GetSize(deie->surface, &w, &h);
   if (r != DFB_OK)
     {
	fprintf(stderr, "ERROR: Could not get surface size: %s\n",
		DirectFBErrorString(r));
	return size;
     }

   size += w * h * 4; // XXX get correct surface size using pixelformat

   return size;
}

#ifdef DFB_DEBUG_IMAGE
static void
evas_cache_image_dfb_debug(const char *context, Engine_Image_Entry* eie)
{
   DirectFB_Engine_Image_Entry *eim = (DirectFB_Engine_Image_Entry *)eie;

   fprintf(stderr, "*** %s image (%p) ***\n", context, eim);
   if (eim)
     {
        fprintf(stderr,
		"* W: %d\n"
		"* H: %d\n"
		"* R: %d\n"
		"* Key: %s\n"
		"* DFB Surface: %p\n",
		eie->w, eie->h, eie->references, eie->cache_key, eim->surface);

        if (eie->src)
          fprintf(stderr,
		  "* Pixels: %p\n", ((RGBA_Image*) eie->src)->image.data);
     }
   fputs("*** ***\n", stderr);
}
#endif

static const Evas_Cache_Engine_Image_Func _dfb_cache_engine_image_cb = {
  NULL /* key */,
  evas_cache_image_dfb_alloc /* alloc */,
  evas_cache_image_dfb_delete /* dealloc */,
  evas_cache_image_dfb_constructor /* constructor */,
  evas_cache_image_dfb_destructor /* destructor */,
  evas_cache_image_dfb_dirty_region /* dirty_region */,
  evas_cache_image_dfb_dirty /* dirty */,
  evas_cache_image_dfb_size_set /* size_set */,
  evas_cache_image_dfb_update_data /* update_data */,
  evas_cache_image_dfb_load /* load */,
  evas_cache_image_dfb_mem_size_get /* mem_size_get */,
#ifdef DFB_DEBUG_IMAGE  /* debug */
  evas_cache_image_dfb_debug
#else
  NULL
#endif
};


/***********************************************************************
 * Evas Engine
 **********************************************************************/
static void *
evas_engine_dfb_info(Evas* e)
{
   Evas_Engine_Info_DirectFB *info;

   info = calloc(1, sizeof(Evas_Engine_Info_DirectFB));
   if (!info)
     return NULL;

   info->magic.magic = rand();

   return info;
}

static void
evas_engine_dfb_info_free(Evas *e, void *in)
{
   Evas_Engine_Info_DirectFB *info = in;

   free(info);
}

static Evas_Bool
_is_dfb_data_ok(IDirectFB *idfb, IDirectFBSurface *surface, int w, int h)
{
   DFBResult r;
   int sw, sh;

   if (!idfb)
     {
	fputs("ERROR: missing IDirectFB\n", stderr);
	return 0;
     }
   dfb = idfb;

   if (!surface)
     {
	fputs("ERROR: missing IDirectFBSurface\n", stderr);
	return 0;
     }

   r = surface->GetSize(surface, &sw, &sh);
   if (r != DFB_OK)
     {
	fprintf(stderr, "ERROR: could not get surface %p size: %s\n",
		surface, DirectFBErrorString(r));
	return 0;
     }

   if ((w > sw) || (h > sh))
     {
	fprintf(stderr,
		"ERROR: requested size is larger than surface: %dx%d > %dx%d\n",
		w, h, sw, sh);
	return 0;
     }
   else if ((w <= 0) || (h <= 0))
     {
	w = sw;
	h = sh;
     }

   return 1;
}

static void
_evas_common_init(void)
{
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
}

static int
evas_engine_dfb_output_reconfigure(Render_Engine *re, int w, int h)
{
   if (re->screen_image)
     evas_cache_engine_image_drop(&re->screen_image->cache_entry);

   if (re->tb)
     evas_common_tilebuf_free(re->tb);

   re->tb = evas_common_tilebuf_new(w, h);
   if (!re->tb)
     {
	fputs("ERROR: could not allocate tile buffer.\n", stderr);
	goto failed_tilebuf;
     }
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   /* We create a "fake" RGBA_Image which points to the IDirectFB surface.
    * Each access to that surface is wrapped in Lock / Unlock calls whenever
    * the data is manipulated directly.
    */
   re->screen_image = (DirectFB_Engine_Image_Entry *)
     evas_cache_engine_image_engine(re->cache, re->spec->surface);
   if (!re->screen_image)
     {
	fputs("ERROR: RGBA_Image allocation from DFB failed\n", stderr);
	goto failed_image;
     }
   re->screen_image->flags.engine_surface = 1;

   _image_autoset_alpha(re->screen_image);
   _image_clear(re->screen_image, 0, 0, w, h);

   return 1;

 failed_image:
   evas_common_tilebuf_free(re->tb);
   re->tb = NULL;
 failed_tilebuf:
   re->screen_image = NULL;
   fputs("ERROR: Evas DirectFB reconfigure failed\n", stderr);
   return 0;
}

static void *
_dfb_output_setup(int w, int h, const struct Evas_Engine_DirectFB_Spec *spec)
{
   Render_Engine *re;

   if (!_is_dfb_data_ok(spec->dfb, spec->surface, w, h))
     goto fatal;

   _evas_common_init();

   re = calloc(1, sizeof(Render_Engine));
   if (!re)
     {
	perror("calloc");
	goto fatal;
     }
   re->dfb = spec->dfb;
   re->spec = spec;
   re->cache = evas_cache_engine_image_init(&_dfb_cache_engine_image_cb,
					    evas_common_image_cache_get());
   if (!re->cache)
     {
	fputs("ERROR: Evas_Cache_Engine_Image allocation failed!\n", stderr);
	goto fatal_after_engine;
     }

   if (!evas_engine_dfb_output_reconfigure(re, w, h))
     {
	fputs("ERROR: Could not reconfigure evas engine.\n", stderr);
	goto fatal_after_reconfigure;
     }

   _dfb_blit_accel_caps_print(spec->surface, NULL);

   return re;


 fatal_after_reconfigure:
   evas_cache_engine_image_shutdown(re->cache);
 fatal_after_engine:
   free(re);
 fatal:
   fputs("FATAL: unable to continue, abort()!\n", stderr);
   abort();
   return NULL;
}

static void
evas_engine_dfb_setup(Evas *e, void *in)
{
   Evas_Engine_Info_DirectFB *info = in;

   if (!e->engine.data.output)
     e->engine.data.output = _dfb_output_setup(e->output.w, e->output.h,
					       &info->info);
   // XXX TODO: else reconfigure existing...

   if (!e->engine.data.output)
     return;

   if (!e->engine.data.context)
     e->engine.data.context =
       e->engine.func->context_new(e->engine.data.output);
}

static void
evas_engine_dfb_output_free(void *data)
{
   Render_Engine *re = data;

   if (!re)
     return;

   if (re->cache)
     evas_cache_engine_image_shutdown(re->cache);

   evas_common_tilebuf_free(re->tb);
   if (re->rects) evas_common_tilebuf_free_render_rects(re->rects);
   free(re);

   evas_common_font_shutdown();
   evas_common_image_shutdown();
}

static void
evas_engine_dfb_output_resize(void *data, int w, int h)
{
   if (!evas_engine_dfb_output_reconfigure(data, w, h))
     fputs("ERROR: failed to resize DirectFB evas\n", stderr);
}

static void
evas_engine_dfb_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re = data;

   evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void
evas_engine_dfb_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re = data;

   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void
evas_engine_dfb_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re = data;

   evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void
evas_engine_dfb_output_redraws_clear(void *data)
{
   Render_Engine *re = data;

   evas_common_tilebuf_clear(re->tb);
}

static void *
evas_engine_dfb_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re = data;
   Tilebuf_Rect *tb_rect;

   if (re->end)
     {
	re->end = 0;
	return NULL;
     }
   if (!re->rects)
     {
	re->rects = evas_common_tilebuf_get_render_rects(re->tb);
	re->cur_rect = EINA_INLIST_GET(re->rects);
     }
   if (!re->cur_rect)
      return NULL;

   tb_rect = (Tilebuf_Rect*) re->cur_rect;
   *cx = *x = tb_rect->x;
   *cy = *y = tb_rect->y;
   *cw = *w = tb_rect->w;
   *ch = *h = tb_rect->h;
   re->cur_rect = re->cur_rect->next;
   if (!re->cur_rect)
     {
	evas_common_tilebuf_free_render_rects(re->rects);
	re->rects = NULL;
	re->end = 1;
     }

   _image_clear(re->screen_image, *x, *y, *w, *h );

   return re->screen_image->surface;
}

static void
evas_engine_dfb_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re = data;
   DFBRegion *r;

   if (re->update_regions_count >= re->update_regions_limit)
     {
	void *tmp;

	re->update_regions_limit += 16;

	tmp = realloc(re->update_regions,
		      sizeof(DFBRegion) * re->update_regions_limit);
	if (!tmp)
	  {
	     perror("realloc");
	     return;
	  }
	re->update_regions = tmp;
     }

   r = re->update_regions + re->update_regions_count;
   re->update_regions_count++;

   r->x1 = x;
   r->y1 = y;
   r->x2 = x + w - 1;
   r->y2 = y + h - 1;
}

static void
evas_engine_dfb_output_flush(void *data)
{
   Render_Engine *re = data;
   IDirectFBSurface *s = re->screen_image->surface;
   DFBRegion *r, *r_end;

   r = re->update_regions;
   r_end = re->update_regions + re->update_regions_count;

#ifdef DFB_UPDATE_INDIVIDUAL_RECTS
   for (; r < r_end; r++)
     s->Flip(s, r, DFB_FLIP_FLAGS);
#else
   DFBRegion bb;

   bb.x1 = bb.y1 = 10000;
   bb.x2 = bb.y2 = 0;
   for (; r < r_end; r++)
     {
	if (bb.x1 > r->x1)
	  bb.x1 = r->x1;
	if (bb.y1 > r->y1)
	  bb.y1 = r->y1;

	if (bb.x2 < r->x2)
	  bb.x2 = r->x2;
	if (bb.y2 < r->y2)
	  bb.y2 = r->y2;
     }

   s->Flip(s, &bb, DFB_FLIP_FLAGS);
#endif

   re->update_regions_count = 0;
}

static void
evas_engine_dfb_output_idle_flush(void *data)
{
   Render_Engine *re = data;

   if (re->update_regions_count != 0)
     fputs("ERROR: update_regions_count not 0 as it should be!\n", stderr);

   free(re->update_regions);
   re->update_regions_count = 0;
   re->update_regions_limit = 0;
   re->update_regions = NULL;
}

/* HACK!!! -- KLUDGE!!!
 *
 * This should really use IDirectFBFont and IDirectFBSurface::DrawString(),
 * but to be edje-compatible IDirectFBFont::CreateFont() should be able to
 * load fonts from non-files, which it does not.
 *
 * Try to find a way to create own IDirectFBFont in future and load from
 * memory.
 */
static void
evas_engine_dfb_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w, int h, int ow, int oh, const char *text)
{
   Render_Engine *re = data;
   RGBA_Image *im;
   IDirectFBSurface *screen = surface;

   im = (RGBA_Image *)re->screen_image->cache_entry.src;

   if (!_dfb_lock_and_sync_image(screen, im, DSLF_READ | DSLF_WRITE))
     return;

   evas_common_font_draw(im, context, font, x, y, text);
   evas_common_cpu_end_opt();

   im->image.data = NULL;

   screen->Unlock(screen);
}


static void
_cb_draw_line(IDirectFBSurface *surface, RGBA_Draw_Context *dc, const DFBRegion *region, void *data)
{
   const Evas_Rectangle *r = data;

   surface->DrawLine(surface, r->x, r->y, r->w, r->h); /* x2, y2 really */
}

static void
evas_engine_dfb_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   IDirectFBSurface *screen = surface;
   Evas_Rectangle r;

   if (!_dfb_surface_set_color_from_context(screen, context))
     return;

   _rect_set(&r, x1, y1, x2, y2); /* x2, y2 (ab)used as w, h */
   _dfb_surface_for_each_cutout(screen, context, _cb_draw_line, &r);
}

#ifndef DFB_USE_EVAS_RECT_DRAW
static void
_cb_draw_rectangle(IDirectFBSurface *surface, RGBA_Draw_Context *dc, const DFBRegion *region, void *data)
{
   const Evas_Rectangle *r = data;

   surface->FillRectangle(surface, r->x, r->y, r->w, r->h);
}

static void
evas_engine_dfb_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   IDirectFBSurface *screen = surface;
   Evas_Rectangle r;

   if (!_dfb_surface_set_color_from_context(screen, context))
     return;

   _rect_set(&r, x, y, w, h);
   _dfb_surface_for_each_cutout(screen, context, _cb_draw_rectangle, &r);
}
#else
static void
evas_engine_dfb_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re = data;
   IDirectFBSurface *screen = surface;
   RGBA_Image *dst;

   dst = (RGBA_Image *)re->screen_image->cache_entry.src;
   if (!_dfb_lock_and_sync_image(screen, dst, DSLF_READ | DSLF_WRITE))
     return;

   evas_common_rectangle_draw(dst, context, x, y, w, h);
   evas_common_cpu_end_opt();

   dst->image.data = NULL;

   screen->Unlock(screen);
}
#endif

#ifndef DFB_USE_EVAS_POLYGON_DRAW
static void
evas_engine_dfb_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   _dfb_polygon_draw(surface, context, polygon);
}
#else
static void
evas_engine_dfb_polygon_draw(void *data, void *context, void *surface, void *polygon)
{
   Render_Engine *re = data;
   IDirectFBSurface *screen = surface;
   RGBA_Image *dst;

   dst = (RGBA_Image *)re->screen_image->cache_entry.src;
   if (!_dfb_lock_and_sync_image(screen, dst, DSLF_READ | DSLF_WRITE))
     return;

   evas_common_polygon_draw(dst, context, polygon);
   evas_common_cpu_end_opt();

   dst->image.data = NULL;

   screen->Unlock(screen);
}
#endif

static void
evas_engine_dfb_gradient_draw(void *data, void *context, void *surface, void *gradient, int x, int y, int w, int h)
{
   Render_Engine *re = data;
   RGBA_Image *dst, *src;
   IDirectFBSurface *screen = surface;

   dst = (RGBA_Image *)re->screen_image->cache_entry.src;
   if (!_dfb_lock_and_sync_image(screen, dst, DSLF_READ | DSLF_WRITE))
     return;

   evas_common_gradient_draw(dst, context, x, y, w, h, gradient);
   evas_common_cpu_end_opt();

   dst->image.data = NULL;

   screen->Unlock(screen);

   return;
}

/** Image Object *******************************************************/
static void *
evas_engine_dfb_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re = data;

   *error = 0;
   return evas_cache_engine_image_request(re->cache, file, key, lo,
					  data, error);
}

static int
evas_engine_dfb_image_alpha_get(void *data, void *image)
{
   DirectFB_Engine_Image_Entry *eim = image;
   Image_Entry *ie;
   RGBA_Image *im;

   if (!eim) return 1;
   ie = eim->cache_entry.src;
   im = (RGBA_Image *)ie;
   switch (ie->space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	 if (ie->flags.alpha) return 1;
      default:
	 break;
     }
   return 0;
}

static void
evas_engine_dfb_image_size_get(void *data, void *image, int *w, int *h)
{
   DirectFB_Engine_Image_Entry *eim = image;
   Image_Entry *ie;

   ie = eim->cache_entry.src;
   if (w) *w = ie->w;
   if (h) *h = ie->h;
}

static int
evas_engine_dfb_image_colorspace_get(void *data, void *image)
{
   DirectFB_Engine_Image_Entry *eim = image;

   if (!eim) return EVAS_COLORSPACE_ARGB8888;
   return eim->cache_entry.src->space;
}

static void
evas_engine_dfb_image_colorspace_set(void *data, void *image, int cspace)
{
   DirectFB_Engine_Image_Entry *eim = image;

   if (!eim) return;
   if (eim->cache_entry.src->space == cspace) return;

   evas_cache_engine_image_colorspace(&eim->cache_entry, cspace, data);
}

static void *
evas_engine_dfb_image_new_from_copied_data(void *data, int w, int h, DATA32* image_data, int alpha, int cspace)
{
   Render_Engine *re = data;

   return evas_cache_engine_image_copied_data(re->cache, w, h, image_data,
					      alpha, cspace, NULL);
}

static void *
evas_engine_dfb_image_new_from_data(void *data, int w, int h, DATA32* image_data, int alpha, int cspace)
{
   Render_Engine *re = data;

   return evas_cache_engine_image_data(re->cache, w, h, image_data,
				       alpha, cspace, NULL);
}

static void
evas_engine_dfb_image_free(void *data, void *image)
{
   DirectFB_Engine_Image_Entry *eim = image;

   evas_cache_engine_image_drop(&eim->cache_entry);
}

static void *
evas_engine_dfb_image_size_set(void *data, void *image, int w, int h)
{
   DirectFB_Engine_Image_Entry *eim = image;

   return evas_cache_engine_image_size_set(&eim->cache_entry, w, h);
}

static void *
evas_engine_dfb_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   DirectFB_Engine_Image_Entry *eim = image;

   return evas_cache_engine_image_dirty(&eim->cache_entry, x, y, w, h);
}

static void *
evas_engine_dfb_image_data_get(void *data, void *image, int to_write, DATA32** image_data)
{
   DirectFB_Engine_Image_Entry *deie = image;
   Engine_Image_Entry *ce;
   Image_Entry *ie;
   RGBA_Image *im;

   if (!deie)
     {
        *image_data = NULL;
        return NULL;
     }

   ce = (Engine_Image_Entry *)deie;
   ie = ce->src;
   im = (RGBA_Image *)ie;

   switch (ie->space)
     {
     case EVAS_COLORSPACE_ARGB8888:
       {
	  DFBResult r;
	  IDirectFBSurface *s;
	  void *pixels;
	  int pitch;

	  if (to_write)
	    deie = (DirectFB_Engine_Image_Entry *)
	      evas_cache_engine_image_dirty(ce, 0, 0, ie->w, ie->h);

	  evas_cache_engine_image_load_data(ce);

	  ce = (Engine_Image_Entry *)deie;
	  ie = ce->src;
	  im = (RGBA_Image *)ie;
	  s = deie->surface;

	  if (to_write)
	    {
	       r = s->Lock(s, DSLF_WRITE, &pixels, &pitch);
	       if (r != DFB_OK)
		 goto error;
	       deie->flags.is_locked = 1;
	    }
	  else
	    {
	       r = s->Lock(s, DSLF_READ, &pixels, &pitch);
	       if (r != DFB_OK)
		 goto error;
	       s->Unlock(s);
	    }

	  *image_data = pixels;
	  im->image.data = pixels; /* remember for _put() */
	  break;

       error:
	  fprintf(stderr, "ERROR: could not lock surface %p: %s\n",
		  s, DirectFBErrorString(r));
	  *image_data = NULL;
	  break;
       }
     case EVAS_COLORSPACE_YCBCR422P709_PL:
     case EVAS_COLORSPACE_YCBCR422P601_PL:
	/* XXX untested */
        *image_data = im->cs.data;
        break;
     default:
        abort();
        break;
     }
   return deie;
}

static void *
evas_engine_dfb_image_data_put(void *data, void *image, DATA32* image_data)
{
   DirectFB_Engine_Image_Entry *deie = image;
   Render_Engine *re = data;
   Engine_Image_Entry *ce;
   Image_Entry *ie;
   RGBA_Image *im;

   if (!deie) return NULL;

   ce = (Engine_Image_Entry *)deie;
   ie = ce->src;
   im = (RGBA_Image*)ie;

   switch (ie->space)
     {
     case EVAS_COLORSPACE_ARGB8888:
	if (image_data == im->image.data)
	  {
	     if (deie->flags.is_locked)
	       {
		  deie->surface->Unlock(deie->surface);
		  deie->flags.is_locked = 0;
	       }
	  }
	else
          {
	     int alpha, cspace;

	     alpha = func.image_alpha_get(re, deie);
	     cspace = func.image_colorspace_get(re, deie);

             evas_cache_engine_image_drop(ce);
             deie = (DirectFB_Engine_Image_Entry *)
	       evas_cache_engine_image_data(re->cache, ce->w, ce->h,
					    image_data, alpha, cspace, data);
          }
        break;
     case EVAS_COLORSPACE_YCBCR422P601_PL:
     case EVAS_COLORSPACE_YCBCR422P709_PL:
	/* XXX untested */
        if (image_data != im->cs.data)
          {
             if (im->cs.data)
               if (!im->cs.no_free)
                 free(im->cs.data);
             im->cs.data = image_data;
             evas_common_image_colorspace_dirty(im);
          }
        break;
     default:
        abort();
        break;
     }
   return deie;
}

static void
evas_engine_dfb_image_data_preload_request(void *data, void *image, void *target)
{
   DirectFB_Engine_Image_Entry *deie = image;
   RGBA_Image *im;

   if (!deie) return ;
   im = (RGBA_Image*) deie->cache_entry.src;
   if (!im) return ;
   evas_cache_image_preload_data(&im->cache_entry, target);
}

static void
evas_engine_dfb_image_data_preload_cancel(void *data, void *image)
{
   DirectFB_Engine_Image_Entry *deie = image;
   RGBA_Image *im;

   if (!deie) return ;
   im = (RGBA_Image*) deie->cache_entry.src;
   if (!im) return ;
   evas_cache_image_preload_cancel(&im->cache_entry);
}

static void *
evas_engine_dfb_image_alpha_set(void *data, void *image, int has_alpha)
{
   DirectFB_Engine_Image_Entry *eim = image;
   Engine_Image_Entry *ce;
   Image_Entry *ie;
   RGBA_Image *im;

   if (!eim) return NULL;

   ce = &eim->cache_entry;
   ie = ce->src;
   im = (RGBA_Image*)ie;

   if (ie->space != EVAS_COLORSPACE_ARGB8888)
     {
        ie->flags.alpha = 0;
        return eim;
     }

   eim = (DirectFB_Engine_Image_Entry *)
     evas_cache_engine_image_dirty(ce, 0, 0, ce->w, ce->h);

   ie->flags.alpha = !!has_alpha;
   return eim;
}

struct _for_each_cutout_image
{
   IDirectFBSurface *image;
   DFBRectangle src, dst;
};

static void
_cb_draw_image_unscaled(IDirectFBSurface *surface, RGBA_Draw_Context *dc, const DFBRegion *region, void *data)
{
   const struct _for_each_cutout_image *p = data;

   surface->Blit(surface, p->image, &p->src, p->dst.x, p->dst.y);
}

static void
_cb_draw_image_scaled(IDirectFBSurface *surface, RGBA_Draw_Context *dc, const DFBRegion *region, void *data)
{
   const struct _for_each_cutout_image *p = data;

   surface->StretchBlit(surface, p->image, &p->src, &p->dst);
}

#ifndef DFB_USE_EVAS_IMAGE_DRAW
static void
evas_engine_dfb_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re = data;
   IDirectFBSurface *screen = surface;
   DirectFB_Engine_Image_Entry *deie = image;
   struct _for_each_cutout_image p;
   _cb_for_each_cutout_t cb;

   if (deie->cache_entry.src->space == EVAS_COLORSPACE_ARGB8888)
     evas_cache_engine_image_load_data(&deie->cache_entry);

   evas_common_image_colorspace_normalize((RGBA_Image *)deie->cache_entry.src);

   _dfb_surface_set_blit_params(re->screen_image, deie, context);

   _dfb_rect_set(&p.src, src_x, src_y, src_w, src_h);
   _dfb_rect_set(&p.dst, dst_x, dst_y, dst_w, dst_h);
   p.image = deie->surface;

   if ((src_w == dst_w) && (src_h == dst_h))
     cb = _cb_draw_image_unscaled;
   else
     cb = _cb_draw_image_scaled;

   _dfb_blit_accel_caps_print(screen, deie->surface);
   _dfb_surface_for_each_cutout(screen, context, cb, &p);
}
#else /* DFB_USE_EVAS_IMAGE_DRAW */
static void
evas_engine_dfb_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   DirectFB_Engine_Image_Entry *deie = image;
   Render_Engine *re = data;
   RGBA_Image *dst, *src;
   IDirectFBSurface *screen = surface;

   if (deie->cache_entry.src->space == EVAS_COLORSPACE_ARGB8888)
     evas_cache_engine_image_load_data(&deie->cache_entry);

   evas_common_image_colorspace_normalize((RGBA_Image *)deie->cache_entry.src);

   dst = (RGBA_Image *)re->screen_image->cache_entry.src;
   if (!_dfb_lock_and_sync_image(screen, dst, DSLF_READ | DSLF_WRITE))
     return;

   src = (RGBA_Image *)deie->cache_entry.src;
   if (!_dfb_lock_and_sync_image(deie->surface, src, DSLF_READ))
     goto error_src;

   if (smooth)
     evas_common_scale_rgba_in_to_out_clip_smooth(src, dst, context,
						  src_x, src_y, src_w, src_h,
						  dst_x, dst_y, dst_w, dst_h);
   else
     evas_common_scale_rgba_in_to_out_clip_sample(src, dst, context,
						  src_x, src_y, src_w, src_h,
						  dst_x, dst_y, dst_w, dst_h);
   evas_common_cpu_end_opt();

   dst->image.data = NULL;

   screen->Unlock(screen);
   deie->surface->Unlock(deie->surface);

   return;

 error_src:
   screen->Unlock(screen);
}
#endif

static void
evas_engine_dfb_image_cache_flush(void *data)
{
   Render_Engine *re = data;
   int size;

   size = evas_cache_engine_image_get(re->cache);
   evas_cache_engine_image_set(re->cache, 0);
   evas_cache_engine_image_set(re->cache, size);
}

static void
evas_engine_dfb_image_cache_set(void *data, int bytes)
{
   Render_Engine *re = data;

   evas_cache_engine_image_set(re->cache, bytes);
}

static int
evas_engine_dfb_image_cache_get(void *data)
{
   Render_Engine *re = data;

   return evas_cache_engine_image_get(re->cache);
}

static char *
evas_engine_dfb_image_comment_get(void *data, void *image, char *key)
{
   DirectFB_Engine_Image_Entry *eim = image;
   RGBA_Image *im;

   if (!eim) return NULL;
   im = (RGBA_Image *)eim->cache_entry.src;

   return im->info.comment;
}

EAPI int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&parent_func, "software_generic")) return 0;
   /* store it for later use */
   func = parent_func;
   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, evas_engine_dfb_)
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
   ORD(image_load);
   ORD(image_new_from_data);
   ORD(image_new_from_copied_data);
   ORD(image_colorspace_set);
   ORD(image_colorspace_get);
   ORD(image_free);
   ORD(image_size_set);
   ORD(image_size_get);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_data_preload_request);
   ORD(image_data_preload_cancel);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_draw);
   ORD(image_comment_get);
   ORD(image_cache_flush);
   ORD(image_cache_set);
   ORD(image_cache_get);
   ORD(font_draw);
   ORD(line_draw);
   ORD(rectangle_draw);
   ORD(polygon_draw);
   ORD(gradient_draw);
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
   "ProFUSION embedded systems"
};
