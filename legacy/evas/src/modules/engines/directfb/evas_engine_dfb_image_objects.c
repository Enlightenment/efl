#include "evas_engine_dfb.h"
#include <math.h>
#include <string.h>

static Evas_Hash        * images = NULL;
static Evas_List        * cache = NULL;
static int                cache_size = 0;
static int                cache_usage = 0;

static RGBA_Image *_dfb_image_create(Render_Engine *, int w, int h);
static void _dfb_image_free(RGBA_Image *im);
static void _dfb_image_unref(RGBA_Image *im);
static void _dfb_image_dirty(RGBA_Image *im);
static void _dfb_image_set_cache(int size);
static int _dfb_image_get_cache();
static void _dfb_image_flush_cache();
static void _dfb_image_cache(RGBA_Image *im);
static void _dfb_image_uncache(RGBA_Image *im);
static void _dfb_image_store(RGBA_Image *im);
static void _dfb_image_unstore(RGBA_Image *im);
static void _dfb_image_ref(RGBA_Image *im);
static void _dfb_image_unref(RGBA_Image *im);

static RGBA_Image *_dfb_image_find(const char *filename, const char *key, DATA64 timestamp);

/*
 * Image objects
 */

void               *
evas_engine_directfb_image_load(void *data, char *file, char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine      *re;
   DFBSurfaceDescription dsc;
   DFBImageDescription img_desc;
   IDirectFBImageProvider *provider;
   IDirectFBSurface   *image;
   RGBA_Image         *im = NULL;
   DATA64 mod_time;

   re = (Render_Engine *) data;
   *error = 0;

   if (!file) return NULL;
//   mod_time = evas_file_modified_time(file);
   im = _dfb_image_find(file, key, mod_time);
   if (im)
     {
	_dfb_image_ref(im);
	return im;
     }

   /* Image is not in cache or not already used -> create it */
   provider = NULL;
   re->dfb->CreateImageProvider(re->dfb, file, &provider);
   if (!provider) return NULL;
   provider->GetSurfaceDescription(provider, &dsc);
   provider->GetImageDescription(provider, &img_desc);

   dsc.flags |= DSDESC_PIXELFORMAT;
   dsc.pixelformat = DSPF_ARGB;

   re->dfb->CreateSurface(re->dfb, &dsc, &image);
   if (!image)
     {
	provider->Release(provider);
	return NULL;
     }
   provider->RenderTo(provider, image, NULL);
   provider->Release(provider);

   im = evas_common_image_new();
   im->image = evas_common_image_surface_new(im);
   if (!im->image)
     {
	image->Release(image);
	_dfb_image_free(im);
	return NULL;
     }
   im->image->w = dsc.width;
   im->image->h = dsc.height;
   im->image->data = (void *)image;
   im->image->no_free = 0;
   if (img_desc.caps & DICAPS_ALPHACHANNEL)
      im->flags |= RGBA_IMAGE_HAS_ALPHA;

//   im->timestamp = mod_time;
   if (file)
     im->info.file = strdup(file);
   if (key)
     im->info.key = strdup(key);
   _dfb_image_ref(im);

   return im;
}

void               *
evas_engine_directfb_image_new_from_data(void *data, int w, int h,
					 DATA32 * image_data)
{
   /* FIXME document this peculiarity */
   return evas_engine_directfb_image_new_from_copied_data(data, w, h, image_data);
}

void               *
evas_engine_directfb_image_new_from_copied_data(void *data, int w, int h,
						DATA32 * image_data)
{

   Render_Engine      *re;
   RGBA_Image         *im = NULL;
   IDirectFBSurface   *surf;
   void               *p;
   int                pitch;

   re = (Render_Engine *) data;
   im = _dfb_image_create(re, w, h);

   if (im)
   {
      surf = (IDirectFBSurface *) im->image->data;
      if (surf->Lock(surf, DSLF_WRITE, &p, &pitch) == DFB_OK)
      {
	 memcpy(p, image_data, w * h * sizeof(DATA32));
	 surf->Unlock(surf);
      }
   }
   /* FIXME */
   free(image_data);
   return im;
}

void
evas_engine_directfb_image_free(void *data, void *image)
{
   Render_Engine      *re;

   re = (Render_Engine *) data;
   _dfb_image_unref(image);
}

void
evas_engine_directfb_image_size_get(void *data, void *image, int *w, int *h)
{
   Render_Engine      *re;
   RGBA_Image         *im;

   re = (Render_Engine *) data;
   im = image;
   if (w)
      *w = im->image->w;
   if (h)
      *h = im->image->h;
}

void               *
evas_engine_directfb_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine      *re;
   IDirectFBSurface   *old_surf;
   IDirectFBSurface   *new_surf;
   DFBRectangle        outrect;
   RGBA_Image         *im, *im_old;

   re = (Render_Engine *) data;
   im_old = image;
   im = _dfb_image_create(re, w,h);
   old_surf = (IDirectFBSurface *) im_old->image->data;
   if (im)
     {
	outrect.x = 0;
	outrect.y = 0;
	outrect.w = w;
	outrect.h = h;
	new_surf = (IDirectFBSurface *) im->image->data;
	new_surf->StretchBlit(new_surf, old_surf, NULL, &outrect);
     }
   evas_common_cpu_end_opt;
   _dfb_image_unref(im_old);

   return im;
}

void               *
evas_engine_directfb_image_dirty_region(void *data, void *image, int x, int y,
					int w, int h)
{
   _dfb_image_dirty(image);
   return image;
   x = 0;
   y = 0;
   w = 0;
   h = 0;
}

void               *
evas_engine_directfb_image_data_get(void *data, void *image, int to_write,
				    DATA32 ** image_data)
{
   Render_Engine      *re;
   RGBA_Image         *im;
   IDirectFBSurface   *surf;
   void               *p;
   int                 pitch;
   DATA32             *buf = NULL;
   int                 size;

   re = (Render_Engine *) data;
   im = image;
   surf = (IDirectFBSurface *) im->image->data;
   size = im->image->w * im->image->h * sizeof(DATA32);
   surf->Lock(surf, DSLF_READ, &p, &pitch);

   if ((buf = malloc(size)))
      buf = memcpy(buf, p, size);
   *image_data = buf;

   surf->Unlock(surf);
   return im;
}

void               *
evas_engine_directfb_image_data_put(void *data, void *image,
				    DATA32 * image_data)
{
   Render_Engine      *re;
   RGBA_Image         *im;

   re = (Render_Engine *)data;
   im = image;
   if (image_data != im->image->data)
     {
	int w, h;

	w = im->image->w;
	h = im->image->h;
	_dfb_image_unref(im);
	return evas_engine_directfb_image_new_from_data(data, w, h, image_data);
     }
   _dfb_image_dirty(im);

   return im;
}

void               *
evas_engine_directfb_image_alpha_set(void *data, void *image, int has_alpha)
{
   RGBA_Image         *im;

   im = image;
   if (has_alpha)
      im->flags |= RGBA_IMAGE_HAS_ALPHA;
   else
      im->flags &= ~RGBA_IMAGE_HAS_ALPHA;
   return im;
}

int
evas_engine_directfb_image_alpha_get(void *data, void *image)
{
   Render_Engine      *re;
   RGBA_Image         *im;

   re = (Render_Engine *) data;
   im = image;
   if (im->flags & RGBA_IMAGE_HAS_ALPHA)
      return 1;
   return 0;
}

void  *
evas_engine_directfb_image_border_set(void *data, void *image, int l, int r, int t, int b)
{
   return image;
}

void
evas_engine_directfb_image_border_get(void *data, void *image, int *l, int *r, int *t, int *b)
{
   Render_Engine      *re;
   RGBA_Image         *im;

   re = (Render_Engine *) data;
   im = image;
}

void
evas_engine_directfb_image_draw(void *data, void *context, void *surface,
				void *image, int src_region_x, int src_region_y,
				int src_region_w, int src_region_h,
				int dst_region_x, int dst_region_y,
				int dst_region_w, int dst_region_h, int smooth)
{
   int                 src_w, src_h, dst_w, dst_h;
   int                 dst_clip_x, dst_clip_y, dst_clip_w, dst_clip_h;
   double              horiz_stretch, vert_stretch;

   Render_Engine      *re = (Render_Engine *) data;
   DFBRectangle        inrect;
   DFBRectangle        outrect;
   RGBA_Image         *im = (RGBA_Image *) image;
   RGBA_Draw_Context  *dc = (RGBA_Draw_Context *) context;
   IDirectFBSurface   *img = (IDirectFBSurface *) im->image->data;

   src_w = im->image->w;
   src_h = im->image->h;
   dst_w = re->tb->outbuf_w;
   dst_h = re->tb->outbuf_h;

   if (!
       (RECTS_INTERSECT
	(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst_w,
	 dst_h)))
      return;
   if (!
       (RECTS_INTERSECT
	(src_region_x, src_region_y, src_region_w, src_region_h, 0, 0, src_w,
	 src_h)))
      return;
   if (dc->clip.use)
     {
	dst_clip_x = dc->clip.x;
	dst_clip_y = dc->clip.y;
	dst_clip_w = dc->clip.w;
	dst_clip_h = dc->clip.h;
	if (dst_clip_x < 0)
	  {
	     dst_clip_w += dst_clip_x;
	     dst_clip_x = 0;
	  }
	if (dst_clip_y < 0)
	  {
	     dst_clip_h += dst_clip_y;
	     dst_clip_y = 0;
	  }
	if ((dst_clip_x + dst_clip_w) > dst_w)
	   dst_clip_w = dst_w - dst_clip_x;
	if ((dst_clip_y + dst_clip_h) > dst_h)
	   dst_clip_h = dst_h - dst_clip_y;
     }
   else
     {
	dst_clip_x = 0;
	dst_clip_y = 0;
	dst_clip_w = dst_w;
	dst_clip_h = dst_h;
     }

   if (dst_clip_x < dst_region_x)
     {
	dst_clip_w += dst_clip_x - dst_region_x;
	dst_clip_x = dst_region_x;
     }
   if ((dst_clip_x + dst_clip_w) > (dst_region_x + dst_region_w))
      dst_clip_w = dst_region_x + dst_region_w - dst_clip_x;
   if (dst_clip_y < dst_region_y)
     {
	dst_clip_h += dst_clip_y - dst_region_y;
	dst_clip_y = dst_region_y;
     }
   if ((dst_clip_y + dst_clip_h) > (dst_region_y + dst_region_h))
      dst_clip_h = dst_region_y + dst_region_h - dst_clip_y;

   if ((src_region_w <= 0) || (src_region_h <= 0) ||
       (dst_region_w <= 0) || (dst_region_h <= 0) ||
       (dst_clip_w <= 0) || (dst_clip_h <= 0))
      return;
   /* sanitise x */
   if (src_region_x < 0)
     {
	dst_region_x -= (src_region_x * dst_region_w) / src_region_w;
	dst_region_w += (src_region_x * dst_region_w) / src_region_w;
	src_region_w += src_region_x;
	src_region_x = 0;
     }
   if (src_region_x >= src_w)
      return;
   if ((src_region_x + src_region_w) > src_w)
     {
	dst_region_w = (dst_region_w * (src_w - src_region_x)) / (src_region_w);
	src_region_w = src_w - src_region_x;
     }
   if (dst_region_w <= 0)
      return;
   if (src_region_w <= 0)
      return;
   if (dst_clip_x < 0)
     {
	dst_clip_w += dst_clip_x;
	dst_clip_x = 0;
     }
   if (dst_clip_w <= 0)
      return;
   if (dst_clip_x >= dst_w)
      return;
   if (dst_clip_x < dst_region_x)
     {
	dst_clip_w += (dst_clip_x - dst_region_x);
	dst_clip_x = dst_region_x;
     }
   if ((dst_clip_x + dst_clip_w) > dst_w)
     {
	dst_clip_w = dst_w - dst_clip_x;
     }
   if (dst_clip_w <= 0)
      return;

   /* sanitise y */
   if (src_region_y < 0)
     {
	dst_region_y -= (src_region_y * dst_region_h) / src_region_h;
	dst_region_h += (src_region_y * dst_region_h) / src_region_h;
	src_region_h += src_region_y;
	src_region_y = 0;
     }
   if (src_region_y >= src_h)
      return;
   if ((src_region_y + src_region_h) > src_h)
     {
	dst_region_h = (dst_region_h * (src_h - src_region_y)) / (src_region_h);
	src_region_h = src_h - src_region_y;
     }
   if (dst_region_h <= 0)
      return;
   if (src_region_h <= 0)
      return;
   if (dst_clip_y < 0)
     {
	dst_clip_h += dst_clip_y;
	dst_clip_y = 0;
     }
   if (dst_clip_h <= 0)
      return;
   if (dst_clip_y >= dst_h)
      return;
   if (dst_clip_y < dst_region_y)
     {
	dst_clip_h += (dst_clip_y - dst_region_y);
	dst_clip_y = dst_region_y;
     }
   if ((dst_clip_y + dst_clip_h) > dst_h)
     {
	dst_clip_h = dst_h - dst_clip_y;
     }
   if (dst_clip_h <= 0)
      return;

   /* Figure out scale ratios */
   horiz_stretch = (double)dst_region_w / (double)src_region_w;
   vert_stretch = (double)dst_region_h / (double)src_region_h;

   inrect.x = src_region_x + floor((dst_clip_x - dst_region_x) / horiz_stretch);
   inrect.y = src_region_y + floor((dst_clip_y - dst_region_y) / vert_stretch);
   inrect.w = floor(src_region_w * dst_clip_w / dst_region_w);
   inrect.h = floor(src_region_h * dst_clip_h / dst_region_h);

   outrect.x = dst_clip_x;
   outrect.y = dst_clip_y;
   outrect.w = dst_clip_w;
   outrect.h = dst_clip_h;

   {
      int                 r, g, b, a;
      int                 mulr, mulg, mulb, mula;
      int                 flags;

      if (im->flags & RGBA_IMAGE_HAS_ALPHA)
	 flags = DSBLIT_BLEND_COLORALPHA | DSBLIT_BLEND_ALPHACHANNEL;
      else
	 flags = DSBLIT_NOFX;

      evas_engine_directfb_context_color_get(data, context, &r, &g, &b, &a);
      if (evas_engine_directfb_context_multiplier_get
	  (data, context, &mulr, &mulg, &mulb, &mula))
	{
	   re->backbuf->SetColor(re->backbuf, mulr, mulg, mulb, mula);
	   flags |= DSBLIT_COLORIZE;
	}
      else
	{
	   re->backbuf->SetColor(re->backbuf, r, g, b, a);
	}

      re->backbuf->SetSrcBlendFunction(re->backbuf, DSBF_SRCALPHA);
      img->SetSrcBlendFunction(img, DSBF_INVSRCALPHA);

      re->backbuf->SetBlittingFlags(re->backbuf, flags);
      re->backbuf->StretchBlit(re->backbuf, img, &inrect, &outrect);
   }

   evas_common_cpu_end_opt();
}

void
evas_engine_directfb_image_cache_flush(void *data)
{
   Render_Engine      *re;
   int                 tmp_size;

   re = (Render_Engine *) data;
   tmp_size = cache_size;
   cache_size = 0;
   _dfb_image_set_cache(0);
   _dfb_image_set_cache(tmp_size);
}

void
evas_engine_directfb_image_cache_set(void *data, int bytes)
{
   _dfb_image_set_cache(bytes);
}

int
evas_engine_directfb_image_cache_get(void *data)
{
   return _dfb_image_get_cache();
}

char               *
evas_engine_directfb_image_comment_get(void *data, void *image, char *key)
{
   Render_Engine      *re;
   RGBA_Image         *im;

   re = (Render_Engine *) data;
   im = image;
   return im->info.comment;
}

char               *
evas_engine_directfb_image_format_get(void *data, void *image)
{
   Render_Engine      *re;
   RGBA_Image         *im;

   re = (Render_Engine *) data;
   im = image;
//   if (im->info.format == 1) return "png";
   return NULL;
}




/*
 * Private routines. These are slightly modified versions of the ones in
 * engines/common/evas_image_main.c
 */

static void
_dfb_image_surface_free(RGBA_Surface *is)
{
   IDirectFBSurface *surf;
   if ( (is->data) && (!is->no_free) )
   {
      surf = (IDirectFBSurface *)is->data;
      surf->Release(surf);
      is->data = NULL;
   }
   free(is);
}


static RGBA_Image *
_dfb_image_create(Render_Engine *re, int w, int h)
{
   RGBA_Image *im;
   DFBSurfaceDescription dsc;
   IDirectFBSurface   *surf;

   im = evas_common_image_new();
   if (!im) return NULL;
   im->image = evas_common_image_surface_new(im);
   if (!im->image)
     {
	_dfb_image_free(im);
	return NULL;
     }
   im->image->w = w;
   im->image->h = h;

   dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
   dsc.width = w;
   dsc.height = h;
   dsc.pixelformat = DSPF_ARGB;

   if (re->dfb->CreateSurface(re->dfb, &dsc, &surf) != DFB_OK)
   {
      _dfb_image_free(im);
      return NULL;
   }
   im->image->data = (void*) surf;
   im->flags = RGBA_IMAGE_IS_DIRTY;
   im->references = 1;
   return im;
}


static void
_dfb_image_free(RGBA_Image *im)
{
   if (im->image) _dfb_image_surface_free(im->image);
   if (im->info.file) free(im->info.file);
   if (im->info.key) free(im->info.key);
   if (im->info.comment) free(im->info.comment);
   free(im);
}

static void
_dfb_image_ref(RGBA_Image *im)
{
   im->references++;
   if (im->references == 1) /* we were in cache - take us out */
     {
	_dfb_image_uncache(im);
	_dfb_image_store(im);
     }
}

static void
_dfb_image_unref(RGBA_Image *im)
{
   im->references--;
   if (im->references <= 0) /* we were are now in cache - put us in */
     {
	_dfb_image_unstore(im);
	if ((cache_size > 0) &&
	    (!(im->flags & RGBA_IMAGE_IS_DIRTY)))
	  {
	     _dfb_image_cache(im);
	     _dfb_image_flush_cache();
	  }
	else
	  {
	     _dfb_image_free(im);
	  }
     }
}

static void
_dfb_image_set_cache(int size)
{
   cache_size = size;
   _dfb_image_flush_cache();
}

static int
_dfb_image_get_cache(void)
{
   return cache_size;
}



static void
_dfb_image_cache(RGBA_Image *im)
{
   int ram;

   if (im->flags & RGBA_IMAGE_INDEXED) return;
   im->flags |= RGBA_IMAGE_INDEXED;
   cache = evas_list_prepend(cache, im);
   ram = evas_common_image_ram_usage(im);
   cache_usage += ram;
   _dfb_image_flush_cache();
}

static void
_dfb_image_uncache(RGBA_Image *im)
{
   int ram;

   if (!(im->flags & RGBA_IMAGE_INDEXED)) return;
   im->flags &= ~RGBA_IMAGE_INDEXED;
   cache = evas_list_remove(cache, im);
   ram = evas_common_image_ram_usage(im);
   cache_usage -= ram;
}


static void
_dfb_image_flush_cache(void)
{
   Evas_List *l, *l_next;

   if (!cache) return;
   if (cache_usage < cache_size) return;

   for (l = evas_list_last(cache); l;)
     {
	RGBA_Image *im;

	l_next = l->prev;
	im = (RGBA_Image *)l;
	_dfb_image_uncache(im);
	_dfb_image_free(im);
	if (cache_usage <= cache_size) return;
	l = l_next;
     }
}

static void
_dfb_image_dirty(RGBA_Image *im)
{
   _dfb_image_unstore(im);
   im->flags |= RGBA_IMAGE_IS_DIRTY;
}


static void
_dfb_image_store(RGBA_Image *im)
{
   char *key;
   int l1, l2, l3;
   char buf[256];

   if (im->flags & RGBA_IMAGE_IS_DIRTY) return;
   if (im->flags & RGBA_IMAGE_INDEXED) return;
   if ((!im->info.file) && (!im->info.key)) return;
   l1 = 0;
   if (im->info.file) l1 = strlen(im->info.file);
   l2 = 0;
   if (im->info.key) l2 = strlen(im->info.key);
//   snprintf(buf, sizeof(buf), "%llx", im->timestamp);
//   l3 = strlen(buf);
   buf[0] = 0;
   l3 = 0;
   key = malloc(l1 + 3 + l2 + 3 + l3 +1);
   if (!key) return;
   key[0] = 0;
   if (im->info.file) strcpy(key, im->info.file);
   strcat(key, "/:/");
   if (im->info.key) strcat(key, im->info.key);
   strcat(key, "/:/");
   strcat(key, buf);
   images = evas_hash_add(images, key, im);
   free(key);
   im->flags |= RGBA_IMAGE_INDEXED;
}

static void
_dfb_image_unstore(RGBA_Image *im)
{
   char *key;
   int l1, l2, l3;
   char buf[256];

   if (!(im->flags & RGBA_IMAGE_INDEXED)) return;
   if ((!im->info.file) && (!im->info.key)) return;
   l1 = 0;
   if (im->info.file) l1 = strlen(im->info.file);
   l2 = 0;
   if (im->info.key) l2 = strlen(im->info.key);
//   snprintf(buf, sizeof(buf), "%llx", im->timestamp);
//   l3 = strlen(buf);
   buf[0] = 0;
   l3 = 0;
   key = malloc(l1 + 3 + l2 + 3 + l3 +1);
   if (!key) return;
   key[0] = 0;
   if (im->info.file) strcpy(key, im->info.file);
   strcat(key, "/:/");
   if (im->info.key) strcat(key, im->info.key);
   strcat(key, "/:/");
   strcat(key, buf);
   images = evas_hash_del(images, key, im);
   free(key);
   im->flags &= ~RGBA_IMAGE_INDEXED;
}


static RGBA_Image *
_dfb_image_find(const char *filename, const char *key, DATA64 timestamp)
{
   Evas_List *l;
   RGBA_Image *im;
   char *str;
   int l1, l2, l3;
   char buf[256];

   if ((!filename) && (!key)) return NULL;
   l1 = 0;
   if (filename) l1 = strlen(filename);
   l2 = 0;
   if (key) l2 = strlen(key);
//   sprintf(buf, "%llx", timestamp);
//   l3 = strlen(buf);
   buf[0] = 0;
   l3 = 0;
   str = malloc(l1 + 3 + l2 + 3 + l3 +1);
   if (!str) return NULL;
   str[0] = 0;
   if (filename) strcpy(str, filename);
   strcat(str, "/:/");
   if (key) strcat(str, key);
   strcat(str, "/:/");
   strcat(str, buf);
   im = evas_hash_find(images, str);
   free(str);
   if (im) return im;

   for (l = cache; l; l = l->next)
     {
	int ok;

	im = (RGBA_Image *)l;
	ok = 0;
	if ((filename) && (im->info.file) &&
	    (!strcmp(filename, im->info.file)))
	  ok++;
	if ((!filename) && (!im->info.file))
	  ok++;
	if ((key) && (im->info.key) &&
	    (!strcmp(key, im->info.key)))
	  ok++;
	if ((!key) && (!im->info.key))
	  ok++;
//	if (im->timestamp == timestamp)
//	  ok++;
	if (ok >= 2) return im;
     }
   return NULL;
}


