#include "evas_gl_common.h"

/* nvidia extensions */
extern void glPixelDataRangeNV(int target, int length, void *pointer);
#define GL_WRITE_PIXEL_DATA_RANGE_NV 0x8878
#define GL_READ_PIXEL_DATA_RANGE_NV 0x8879
#define GL_TEXTURE_RECTANGLE_NV 0x84f5


static void _evas_gl_common_texture_mipmaps_build(Evas_GL_Texture *tex, RGBA_Image *im, int smooth);
static void _evas_gl_common_gradient_texture_build(Evas_GL_Context *gc, Evas_GL_Gradient *gr);
static void _evas_gl_common_viewport_set(Evas_GL_Context *gc);
static void _evas_gl_common_dither_set(Evas_GL_Context *gc);
static void _evas_gl_common_blend_set(Evas_GL_Context *gc);
static void _evas_gl_common_color_set(Evas_GL_Context *gc);
static void _evas_gl_common_texture_set(Evas_GL_Context *gc);
static void _evas_gl_common_clip_set(Evas_GL_Context *gc);
static void _evas_gl_common_buf_set(Evas_GL_Context *gc);
static void _evas_gl_common_other_set(Evas_GL_Context *gc);

static Evas_GL_Context *_evas_gl_common_context = NULL;

Evas_GL_Context *
evas_gl_common_context_new(void)
{
   Evas_GL_Context *gc;
   
   if (_evas_gl_common_context)
     {
	_evas_gl_common_context->references++;
	return _evas_gl_common_context;
     }
   gc = calloc(1, sizeof(Evas_GL_Context));
   if (!gc) return NULL;
   gc->max_texture_depth = 32;
   gc->max_texture_size = 2048;
   gc->read_buf = GL_BACK;
   gc->write_buf = GL_BACK;
   gc->dither = 1;
   gc->blend = 0;
   gc->references     = 1;
   
   gc->change.size    = 1;
   gc->change.dither  = 1;
   gc->change.blend   = 1;
   gc->change.color   = 1;
   gc->change.texture = 1;
   gc->change.clip    = 1;
   gc->change.buf     = 1;
   gc->change.other   = 1;
   
//   _evas_gl_common_context = gc;
   return gc;
}

void
evas_gl_common_context_free(Evas_GL_Context *gc)
{
   gc->references--;
   if (gc->references > 0) return;
   
   if (gc == _evas_gl_common_context) _evas_gl_common_context = NULL;
   free(gc);
}

void
evas_gl_common_context_use(Evas_GL_Context *gc)
{
   if (_evas_gl_common_context == gc) return;
   _evas_gl_common_context = gc;
   _evas_gl_common_viewport_set(gc);
   _evas_gl_common_dither_set(gc);
   _evas_gl_common_blend_set(gc);
   _evas_gl_common_color_set(gc);
   _evas_gl_common_texture_set(gc);
   _evas_gl_common_clip_set(gc);
   _evas_gl_common_buf_set(gc);
   _evas_gl_common_other_set(gc);
}

void
evas_gl_common_context_resize(Evas_GL_Context *gc, int w, int h)
{
   if ((gc->w == w) && (gc->h == h)) return;
   gc->change.size = 1;
   gc->w = w;
   gc->h = h;
   if (_evas_gl_common_context == gc) _evas_gl_common_viewport_set(gc);
}

void
evas_gl_common_context_color_set(Evas_GL_Context *gc, int r, int g, int b, int a)
{
   if (r < 0) r = 0;
   else if (r > 255) r = 255;
   if (g < 0) g = 0;
   else if (g > 255) g = 255;
   if (b < 0) b = 0;
   else if (b > 255) b = 255;
   if (a < 0) a = 0;
   else if (a > 255) a = 255;
   if ((gc->r == r) && (gc->g == g) && (gc->b == b) && (gc->a == a)) return;
   gc->change.color = 1;
   gc->r = r;
   gc->g = g;
   gc->b = b;
   gc->a = a;
   if (_evas_gl_common_context == gc) _evas_gl_common_color_set(gc);
}

void
evas_gl_common_context_blend_set(Evas_GL_Context *gc, int blend)
{
   if (((blend) && (gc->blend)) || ((!blend) && (!gc->blend))) return;
   gc->change.blend = 1;
   gc->blend = blend;
   if (_evas_gl_common_context == gc) _evas_gl_common_blend_set(gc);
}

void
evas_gl_common_context_dither_set(Evas_GL_Context *gc, int dither)
{
   if (((dither) && (gc->dither)) || ((!dither) && (!gc->dither))) return;
   gc->change.dither = 1;
   gc->dither = dither;
   if (_evas_gl_common_context == gc) _evas_gl_common_dither_set(gc);
}

void
evas_gl_common_context_texture_set(Evas_GL_Context *gc, Evas_GL_Texture *tex, int smooth, int w, int h)
{
   if (gc->texture != tex)
     {
	if (gc->texture) gc->texture->references--;
	gc->texture = tex;
	gc->change.texture = 1;
	if (tex) tex->references++;
     }
   if (tex)
     {
	if (((smooth) && (!tex->smooth)) || 
	    ((!smooth) && (tex->smooth)))
	  {
	     tex->smooth = smooth;
	     tex->changed = 1;
	  }
	tex->uw = w;
	tex->uh = h;
     }
   if (_evas_gl_common_context == gc) _evas_gl_common_texture_set(gc);
}

void
evas_gl_common_context_clip_set(Evas_GL_Context *gc, int on, int x, int y, int w, int h)
{
   if (((!on) && (!gc->clip.active)) ||
       ((on) && (gc->clip.active) && 
	(x == gc->clip.x) && (y == gc->clip.y) &&
	(w == gc->clip.w) && (h == gc->clip.h)))
     return;
   gc->change.clip = 1;
   gc->clip.active = on;
   gc->clip.x = x;
   gc->clip.y = y;
   gc->clip.w = w;
   gc->clip.h = h;
   if (_evas_gl_common_context == gc) _evas_gl_common_clip_set(gc);
}

void
evas_gl_common_context_read_buf_set(Evas_GL_Context *gc, GLenum buf)
{
   if (gc->read_buf == buf) return;
   gc->change.buf = 1;
   gc->read_buf = buf;
   if (_evas_gl_common_context == gc) _evas_gl_common_buf_set(gc);
}

void
evas_gl_common_context_write_buf_set(Evas_GL_Context *gc, GLenum buf)
{
   if (gc->write_buf == buf) return;
   gc->change.buf = 1;
   gc->write_buf = buf;
   if (_evas_gl_common_context == gc) _evas_gl_common_buf_set(gc);
}

Evas_GL_Texture *
evas_gl_common_texture_new(Evas_GL_Context *gc, RGBA_Image *im, int smooth)
{
   Evas_GL_Texture *tex;
   int *im_data;
   int im_w, im_h;
   int tw, th;
   int shift;
   GLenum pixfmt, texfmt;
   
   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;

//#define NV_RECT_EXT 
   
#ifdef NV_RECT_EXT
     {
	printf("new rect tex %ix%i\n", im->image->w, im->image->h);
	
	tex->gc = gc;
	tex->w = im->image->w;
	tex->h = im->image->h;
	tex->not_power_of_two = 1;
	tex->tw = im->image->w;
	tex->th = im->image->h;
	tex->references = 0;
	tex->smooth = 0;
	tex->changed = 1;
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glGenTextures(1, &(tex->texture));
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex->texture);
	
	if (gc->texture) gc->texture->references--;
	gc->texture = tex;
	gc->change.texture = 1;
	tex->references++;

	if (im->flags & RGBA_IMAGE_HAS_ALPHA) texfmt = GL_RGBA8;
	else texfmt = GL_RGB8;
	pixfmt = GL_BGRA;

	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
		     texfmt, tex->w, tex->h, 0,
		     pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV, im->image->data);
	return tex;
     }
#endif   
   shift = 1; while (im->image->w > shift) shift = shift << 1; tw = shift;
   shift = 1; while (im->image->h > shift) shift = shift << 1; th = shift;
   tex->gc = gc;
   tex->w = tw;
   tex->h = th;
   printf("new tex %ix%i\n", tw, th);
   tex->tw = im->image->w;
   tex->th = im->image->h;
   tex->references = 0;
   tex->smooth = 0;
   tex->changed = 1;
   glEnable(GL_TEXTURE_2D);
   glGenTextures(1, &(tex->texture));
   glBindTexture(GL_TEXTURE_2D, tex->texture);

   if (gc->texture) gc->texture->references--;
   gc->texture = tex;
   gc->change.texture = 1;
   tex->references++;

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   
   im_data = im->image->data;
   im_w = im->image->w;
   im_h = im->image->h;

   if (im->flags & RGBA_IMAGE_HAS_ALPHA) texfmt = GL_RGBA8;
   else texfmt = GL_RGB8;
   pixfmt = GL_BGRA;
   
   glTexImage2D(GL_TEXTURE_2D, 0, 
		texfmt, tw, th, 0,
		pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
#ifdef NVIDIA_HACK
/* NVIDIA HACK 1,part-1 */   
   /* Nvidia's 4496 drivers and below have a bug. unless i "allocate" the
    * mipmap space here before i go and use a texture, it will never accept
    * mipmaps being added to a texture later on, or at the least it will never
    * use added mipmaps.
    * 
    * so as a workaround i allocate the mipmap texels here with a NULL pointer
    * passed to glTexImage2D() and i fill in the mipmap pixels later on in
    * _evas_gl_common_texture_mipmaps_build(). this works, but is ugly.
    * 
    * i currently have no reason to believe GL can't do what i want to do,
    * that is add mipmaps and allocate space for them any time i want. see
    */
     {
	int ttw, tth;
	int l;
	
	ttw = tw;
	tth = th;	
	l = 0;
	while ((ttw > 1) || (tth > 1))
	  {
	     l++;
	     ttw /= 2;
	     tth /= 2;
	     if (ttw < 1) ttw = 1;
	     if (tth < 1) tth = 1;
	     glTexImage2D(GL_TEXTURE_2D, l,
			  texfmt, ttw, tth, 0,
			  pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	  }
     }
/* END ENVIDIA HACK */
#endif   
   glTexSubImage2D(GL_TEXTURE_2D, 0, 
		   0, 0, im_w, im_h,
		   pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
		   im_data);
   if (im_w < tw)
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     im_w, 0, 1, im_h,
		     pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
		     im_data + im_w - 1);
   if (im_h < th)
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     0, im_h, im_w, 1,
		     pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
		     im_data + (im_w * (im_h - 1)));
   if ((im_w < tw) && (im_h < th))
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     im_w, im_h, 1, 1,
		     pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
		     im_data + (im_w * (im_h - 1)) + im_w - 1);
#ifdef RADEON_HACK   
/* RADEON HACK 1 */
   /* similar to nvidia's bug (fixed in hack 1) ati's drivers SEGV if i try
    * and upload textures later on than now, not even alloocating them first
    * like nvidias hack does helps this, so if i EVER need mipmaps, it's now or
    * never, and that just SUCKS. i'm forever generating mipmaps for textures
    * where i might never need them. this is just silly! must report this to
    * ati
    */
     {
	int ttw, tth;
	int w, h;
	int l;
	RGBA_Image *im1 = NULL, *im2 = NULL;
#ifdef BUILD_MMX
	int mmx, sse, sse2;
#endif

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#ifdef BUILD_MMX
	evas_common_cpu_can_do(&mmx, &sse, &sse2);
#endif
	w = im->image->w;
	h = im->image->h;
	im1 = im;
   
	ttw = tw;
	tth = th;
	l = 0;
	
	while ((ttw > 1) || (tth > 1))
	  {
	     int pw, ph;
	     
	     l++;
	     
	     pw = w;
	     ph = h;
	     
	     w /= 2;
	     h /= 2;
	     if (w < 1) w = 1;
	     if (h < 1) h = 1;
	     
	     ttw /= 2;
	     tth /= 2;
	     if (ttw < 1) ttw = 1;
	     if (tth < 1) tth = 1;

	     im2 = evas_common_image_create(w, h);
#ifdef BUILD_MMX
	     if (mmx)
	       {  
		  evas_common_scale_rgba_mipmap_down_2x2_mmx(im1->image->data,
							     im2->image->data, 
							     pw, ph);
	       }
	     else
#endif	  
	       {
		  if (im->flags & RGBA_IMAGE_HAS_ALPHA)
		    evas_common_scale_rgba_mipmap_down_2x2_c(im1->image->data,
							     im2->image->data, 
							     pw, ph);
		  else
		    evas_common_scale_rgb_mipmap_down_2x2_c(im1->image->data,
							    im2->image->data, 
							    pw, ph);
	       }
	     if (im1 != im) evas_common_image_free(im1);
	     im1 = NULL;
	     
	     im_data = im2->image->data;
	     im_w = w;
	     im_h = h;
	     glTexImage2D(GL_TEXTURE_2D, l,
			  texfmt, ttw, tth, 0,
			  pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	     glTexSubImage2D(GL_TEXTURE_2D, l,
			     0, 0, im_w, im_h,
			     pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			     im_data);
	     if (im_w < ttw)
	       glTexSubImage2D(GL_TEXTURE_2D, l,
			       im_w, 0, 1, im_h,
			       pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			       im_data + im_w - 1);
	     if (im_h < tth)
	       glTexSubImage2D(GL_TEXTURE_2D, l,
			       0, im_h, im_w, 1,
			       pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			       im_data + (im_w * (im_h - 1)));
	     if ((im_w < ttw) && (im_h < tth))
	       glTexSubImage2D(GL_TEXTURE_2D, l, 
			       im_w, im_h, 1, 1,
			       pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			       im_data + (im_w * (im_h - 1)) + im_w - 1);
	     im1 = im2;
	     im2 = NULL;
	  }
	if ((im1 != im) && (im1)) evas_common_image_free(im1);
	tex->have_mipmaps = 1;
	tex->smooth = 1;
     }
/* END RADEON HACK */
#endif   
   return tex;
}

void
evas_gl_common_texture_update(Evas_GL_Texture *tex, RGBA_Image *im, int smooth)
{
   int *im_data;
   int im_w, im_h;
   int tw, th;
   int shift;
   GLenum pixfmt, texfmt, target;

   if (tex->not_power_of_two)
     {
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex->texture);

/*	
	if (!tex->opt)
	  {
	     glPixelDataRangeNV(GL_WRITE_PIXEL_DATA_RANGE_NV, 
				im->image->w * im->image->h * 4, 
			 im->image->data);
	     tex->opt = 1;
	  }
*/
	
	if (tex->gc->texture) tex->gc->texture->references--;
	tex->gc->texture = tex;
	tex->gc->change.texture = 1;
	tex->references++;

	if (im->flags & RGBA_IMAGE_HAS_ALPHA) texfmt = GL_RGBA8;
	else texfmt = GL_RGB8;
	pixfmt = GL_BGRA;
	
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
			0, 0, tex->w, tex->h,
			pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			im->image->data);
	return;
     }
   tw = tex->w;
   th = tex->h;
   tex->changed = 1;
   glEnable(GL_TEXTURE_2D);
   if (tex->not_power_of_two)
     {
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	target = GL_TEXTURE_RECTANGLE_NV;
     }
   else
     {
	glDisable(GL_TEXTURE_RECTANGLE_NV);
	target = GL_TEXTURE_2D;
     }
     
   glBindTexture(GL_TEXTURE_2D, tex->texture);

   if (tex->gc->texture) tex->gc->texture->references--;
   tex->gc->texture = tex;
   tex->gc->change.texture = 1;
   tex->references++;

   /*
   if (!tex->opt)
     {
	glPixelDataRangeNV(GL_WRITE_PIXEL_DATA_RANGE_NV, 
			 im->image->w * im->image->h * 4, 
			 im->image->data);
	tex->opt = 1;
     }
   */
   
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   
   im_data = im->image->data;
   im_w = im->image->w;
   im_h = im->image->h;

   if (im->flags & RGBA_IMAGE_HAS_ALPHA) texfmt = GL_RGBA8;
   else texfmt = GL_RGB8;
   pixfmt = GL_BGRA;
   
   glTexSubImage2D(GL_TEXTURE_2D, 0, 
		   0, 0, im_w, im_h,
		   pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
		   im_data);
#if 1 // this is sloooow... well slower than just the above...
   if (im_w < tw)
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     im_w, 0, 1, im_h,
		     pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
		     im_data + im_w - 1);
   if (im_h < th)
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     0, im_h, im_w, 1,
		     pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
		     im_data + (im_w * (im_h - 1)));
   if ((im_w < tw) && (im_h < th))
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     im_w, im_h, 1, 1,
		     pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
		     im_data + (im_w * (im_h - 1)) + im_w - 1);
#endif   
#ifdef RADEON_HACK   
/* RADEON HACK 1 */
   /* similar to nvidia's bug (fixed in hack 1) ati's drivers SEGV if i try
    * and upload textures later on than now, not even alloocating them first
    * like nvidias hack does helps this, so if i EVER need mipmaps, it's now or
    * never, and that just SUCKS. i'm forever generating mipmaps for textures
    * where i might never need them. this is just silly! must report this to
    * ati
    */
     {
	int ttw, tth;
	int w, h;
	int l;
	RGBA_Image *im1 = NULL, *im2 = NULL;
#ifdef BUILD_MMX
	int mmx, sse, sse2;
#endif

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
#ifdef BUILD_MMX
	evas_common_cpu_can_do(&mmx, &sse, &sse2);
#endif
	w = im->image->w;
	h = im->image->h;
	im1 = im;
   
	ttw = tw;
	tth = th;
	l = 0;
	
	while ((ttw > 1) || (tth > 1))
	  {
	     int pw, ph;
	     
	     l++;
	     
	     pw = w;
	     ph = h;
	     
	     w /= 2;
	     h /= 2;
	     if (w < 1) w = 1;
	     if (h < 1) h = 1;
	     
	     ttw /= 2;
	     tth /= 2;
	     if (ttw < 1) ttw = 1;
	     if (tth < 1) tth = 1;

	     im2 = evas_common_image_create(w, h);
#ifdef BUILD_MMX
	     if (mmx)
	       {  
		  evas_common_scale_rgba_mipmap_down_2x2_mmx(im1->image->data,
							     im2->image->data, 
							     pw, ph);
	       }
	     else
#endif	  
	       {
		  if (im->flags & RGBA_IMAGE_HAS_ALPHA)
		    evas_common_scale_rgba_mipmap_down_2x2_c(im1->image->data,
							     im2->image->data, 
							     pw, ph);
		  else
		    evas_common_scale_rgb_mipmap_down_2x2_c(im1->image->data,
							    im2->image->data, 
							    pw, ph);
	       }
	     if (im1 != im) evas_common_image_free(im1);
	     im1 = NULL;
	     
	     im_data = im2->image->data;
	     im_w = w;
	     im_h = h;
//	     glTexImage2D(GL_TEXTURE_2D, l,
//			  texfmt, ttw, tth, 0,
//			  pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
	     glTexSubImage2D(GL_TEXTURE_2D, l,
			     0, 0, im_w, im_h,
			     pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			     im_data);
	     if (im_w < ttw)
	       glTexSubImage2D(GL_TEXTURE_2D, l,
			       im_w, 0, 1, im_h,
			       pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			       im_data + im_w - 1);
	     if (im_h < tth)
	       glTexSubImage2D(GL_TEXTURE_2D, l,
			       0, im_h, im_w, 1,
			       pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			       im_data + (im_w * (im_h - 1)));
	     if ((im_w < ttw) && (im_h < tth))
	       glTexSubImage2D(GL_TEXTURE_2D, l, 
			       im_w, im_h, 1, 1,
			       pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			       im_data + (im_w * (im_h - 1)) + im_w - 1);
	     im1 = im2;
	     im2 = NULL;
	  }
	if ((im1 != im) && (im1)) evas_common_image_free(im1);
	tex->have_mipmaps = 1;
	tex->smooth = 1;
     }
/* END RADEON HACK */
#endif   
}

void
evas_gl_common_texture_free(Evas_GL_Texture *tex)
{
   if (tex->references > 0)
     {
	/* FIXME: could be in more gc's */
     }
   if (tex->gc->texture == tex)
     {
	tex->gc->texture = NULL;
	tex->gc->change.texture = 1;
     }
   glDeleteTextures(1, &tex->texture);
   free(tex);
}

Evas_GL_Image *
evas_gl_common_image_load(Evas_GL_Context *gc, char *file, char *key)
{
   Evas_GL_Image *im;
   RGBA_Image *im_im;
   Evas_List *l;
   
   im_im = evas_common_load_image_from_file(file, key);
   if (!im_im) return NULL;
   
   for (l = gc->images; l; l = l->next)
     {
	im = l->data;
	if (im->im == im_im)
	  {
	     evas_common_image_unref(im_im);
	     gc->images = evas_list_remove_list(gc->images, l);
	     gc->images = evas_list_prepend(gc->images, im);
	     im->references++;
	     return im;
	  }
     }
   
   im = calloc(1, sizeof(Evas_GL_Image));
   if (!im) return NULL;
   im->references = 1;
   im->im = im_im;
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->gc = gc;
   im->references = 1;
   im->cached = 1;
   gc->images = evas_list_prepend(gc->images, im);
   return im;
}

Evas_GL_Image *
evas_gl_common_image_new_from_data(Evas_GL_Context *gc, int w, int h, int *data)
{
   Evas_GL_Image *im;
   Evas_List *l;
   
   for (l = gc->images; l; l = l->next)
     {
	im = l->data;
	if (((void *)(im->im->image->data) == (void *)data) &&
	    (im->im->image->w == w) &&
	    (im->im->image->h == h))
	  {
	     gc->images = evas_list_remove_list(gc->images, l);
	     gc->images = evas_list_prepend(gc->images, im);
	     im->references++;
	     return im;
	  }
     }
   im = calloc(1, sizeof(Evas_GL_Image));
   im->references = 1;
   im->im = evas_common_image_new();
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   im->im->image = evas_common_image_surface_new();
   if (!im->im->image)
     {
	evas_common_image_free(im->im);
	free(im);
	return NULL;
     }
   im->im->image->w = w;
   im->im->image->h = h;
   im->im->image->data = data;
   im->im->image->no_free = 1;
   im->cached = 1;
   gc->images = evas_list_prepend(gc->images, im);
   return im;
}

Evas_GL_Image *
evas_gl_common_image_new_from_copied_data(Evas_GL_Context *gc, int w, int h, int *data)
{
   Evas_GL_Image *im;
   
   im = calloc(1, sizeof(Evas_GL_Image));
   im->references = 1;
   im->im = evas_common_image_create(w, h);
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   if (data)
     memcpy(im->im->image->data, data, w * h * sizeof(DATA32));
   return im;
}

Evas_GL_Image *
evas_gl_common_image_new(Evas_GL_Context *gc, int w, int h)
{
   Evas_GL_Image *im;
   
   im = calloc(1, sizeof(Evas_GL_Image));
   im->references = 1;
   im->im = evas_common_image_create(w, h);
   if (!im->im)
     {
	free(im);
	return NULL;
     }
   return im;
}

void
evas_gl_common_image_free(Evas_GL_Image *im)
{
   im->references--;
   if (im->references > 0) return;

   if (im->cached)
     im->gc->images = evas_list_remove(im->gc->images, im);
   if (im->im) evas_common_image_unref(im->im);
   if (im->tex) evas_gl_common_texture_free(im->tex);
   free(im);
}

void
evas_gl_common_image_dirty(Evas_GL_Image *im)
{
   evas_common_image_dirty(im->im);
   im->dirty = 1;
}

Evas_GL_Polygon *
evas_gl_common_poly_point_add(Evas_GL_Polygon *poly, int x, int y)
{
   Evas_GL_Polygon_Point *pt;

   if (!poly) poly = calloc(1, sizeof(Evas_GL_Polygon));
   if (!poly) return NULL;
   pt = calloc(1, sizeof(Evas_GL_Polygon_Point));
   if (!pt) return NULL;
   pt->x = x;
   pt->y = y;
   poly->points = evas_list_append(poly->points, pt);
   return poly;
}

Evas_GL_Polygon *
evas_gl_common_poly_points_clear(Evas_GL_Polygon *poly)
{
   if (!poly) return NULL;
   while (poly->points)
     {
	Evas_GL_Polygon_Point *pt;
	
	pt = poly->points->data;
	poly->points = evas_list_remove(poly->points, pt);
	free(pt);
     }
   free(poly);
   return NULL;
}

Evas_GL_Gradient *
evas_gl_common_gradient_color_add(Evas_GL_Gradient *gr, int r, int g, int b, int a, int distance)
{
   if (!gr) gr = calloc(1, sizeof(Evas_GL_Gradient));
   if (!gr) return NULL;
   if (!gr->grad) gr->grad = evas_common_gradient_new();
   evas_common_gradient_color_add(gr->grad, r, g, b, a, distance);
   if (gr->tex) evas_gl_common_texture_free(gr->tex);
   gr->tex = NULL;
   return gr;
}

Evas_GL_Gradient *
evas_gl_common_gradient_colors_clear(Evas_GL_Gradient *gr)
{
   if (!gr) return NULL;
   if (gr->grad) evas_common_gradient_free(gr->grad);
   if (gr->tex) evas_gl_common_texture_free(gr->tex);
   gr->tex = NULL;
   free(gr);
   return NULL;
}



void
evas_gl_common_swap_rect(Evas_GL_Context *gc, int x, int y, int w, int h)
{
   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_FRONT);
   evas_gl_common_context_blend_set(gc, 0);
   evas_gl_common_context_clip_set(gc, 0, 0, 0, 0, 0);
   evas_gl_common_context_dither_set(gc, 0);
   y = gc->h - y - h;
   glRasterPos2i(x, gc->h - y);
   glCopyPixels(x, y, w, h, GL_COLOR);   
}


void
evas_gl_common_rect_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   int r, g, b, a;

   a = (dc->col.col >> 24) & 0xff;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   evas_gl_common_context_color_set(gc, r, g, b, a);
   if (a < 255) evas_gl_common_context_blend_set(gc, 1);
   else evas_gl_common_context_blend_set(gc, 0);
   if (dc->clip.use)
     evas_gl_common_context_clip_set(gc, 1,
				     dc->clip.x, dc->clip.y,
				     dc->clip.w, dc->clip.h);
   else
     evas_gl_common_context_clip_set(gc, 0,
				     0, 0, 0, 0);
   evas_gl_common_context_texture_set(gc, NULL, 0, 0, 0);
   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_BACK);
   glBegin(GL_QUADS);
   glVertex2i(x, y);
   glVertex2i(x + w, y);
   glVertex2i(x + w, y + h);
   glVertex2i(x, y + h);
   glEnd();
}

void
evas_gl_common_image_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, Evas_GL_Image *im, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int smooth)
{
   int r, g, b, a;
   double tx1, ty1, tx2, ty2;
   int    ow, oh;

   if (sw < 1) sw = 1;
   if (sh < 1) sh = 1;
   if (dc->mul.use)
     {
	a = (dc->mul.col >> 24) & 0xff;
	r = (dc->mul.col >> 16) & 0xff;
	g = (dc->mul.col >> 8 ) & 0xff;
	b = (dc->mul.col      ) & 0xff;
     }
   else
     {
	r = g = b = a = 255;
     }
   evas_common_load_image_data_from_file(im->im);
   if ((im->tex) && (im->dirty))
     {
	evas_gl_common_texture_update(im->tex, im->im, im->tex->smooth);
	im->dirty = 0;
     }
   if (!im->tex)
     im->tex = evas_gl_common_texture_new(gc, im->im, smooth);
   ow = (dw * im->tex->tw) / sw;
   oh = (dh * im->tex->th) / sh;
   evas_gl_common_context_texture_set(gc, im->tex, smooth, ow, oh);
   if ((!im->tex->have_mipmaps) && (smooth) &&
       ((im->tex->uw < im->tex->tw) || (im->tex->uh < im->tex->th)))
     _evas_gl_common_texture_mipmaps_build(im->tex, im->im, smooth);

   if (im->tex->not_power_of_two)
     {
	tx1 = sx;
	ty1 = sy;
	tx2 = sx + sw;
	ty2 = sy + sh;
     }
   else
     {
	tx1 = (double)(sx     ) / (double)(im->tex->w);
	ty1 = (double)(sy     ) / (double)(im->tex->h);
	tx2 = (double)(sx + sw) / (double)(im->tex->w);
	ty2 = (double)(sy + sh) / (double)(im->tex->h);
     }
   evas_gl_common_context_color_set(gc, r, g, b, a);
   if ((a < 255) || (im->im->flags & RGBA_IMAGE_HAS_ALPHA))
     evas_gl_common_context_blend_set(gc, 1);
   else evas_gl_common_context_blend_set(gc, 0);
   if (dc->clip.use)
     evas_gl_common_context_clip_set(gc, 1,
				     dc->clip.x, dc->clip.y,
				     dc->clip.w, dc->clip.h);
   else
     evas_gl_common_context_clip_set(gc, 0,
				     0, 0, 0, 0);
   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_BACK);
   
   glBegin(GL_QUADS);
   glTexCoord2d(tx1, ty1); glVertex2i(dx     , dy     );
   glTexCoord2d(tx2, ty1); glVertex2i(dx + dw, dy     );
   glTexCoord2d(tx2, ty2); glVertex2i(dx + dw, dy + dh);
   glTexCoord2d(tx1, ty2); glVertex2i(dx     , dy + dh);
   glEnd();
}

void
evas_gl_common_line_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2)
{
   int r, g, b, a;

   a = (dc->col.col >> 24) & 0xff;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   evas_gl_common_context_color_set(gc, r, g, b, a);
   if (a < 255) evas_gl_common_context_blend_set(gc, 1);
   else evas_gl_common_context_blend_set(gc, 0);
   if (dc->clip.use)
     evas_gl_common_context_clip_set(gc, 1,
				     dc->clip.x, dc->clip.y,
				     dc->clip.w, dc->clip.h);
   else
     evas_gl_common_context_clip_set(gc, 0,
				     0, 0, 0, 0);
   evas_gl_common_context_texture_set(gc, NULL, 0, 0, 0);
   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_BACK);
   glBegin(GL_LINES);
   glVertex2i(x1, y1);
   glVertex2i(x2, y2);
   glEnd();
}

#define GLU_TESS

#ifdef GLU_TESS   
static void _evas_gl_tess_begin_cb(GLenum which);
static void _evas_gl_tess_end_cb(void);
static void _evas_gl_tess_error_cb(GLenum errorcode);
static void _evas_gl_tess_vertex_cb(GLvoid *vertex);
static void _evas_gl_tess_combine_cb(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], GLdouble **data_out);

static void
_evas_gl_tess_begin_cb(GLenum which)
{
   glBegin(which);
}

static void
_evas_gl_tess_end_cb(void)
{
   glEnd();
}

static void
_evas_gl_tess_error_cb(GLenum errorcode)
{
}

static void
_evas_gl_tess_vertex_cb(GLvoid *vertex)
{
   GLdouble *v;
   
   v = vertex;
   glVertex2d(v[0], v[1]);
}

static void
_evas_gl_tess_combine_cb(GLdouble coords[3], GLdouble *vertex_data[4], GLfloat weight[4], GLdouble **data_out)
{
   GLdouble *vertex;
   
   vertex = (GLdouble *) malloc(6 * sizeof(GLdouble));
   vertex[0] = coords[0];
   vertex[1] = coords[1];
   *data_out = vertex;
}
#endif

void
evas_gl_common_poly_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, Evas_GL_Polygon *poly)
{
   int r, g, b, a;
   Evas_List *l;
   static void *tess = NULL;
   GLdouble *glp = NULL;
   int i, num;
   
   a = (dc->col.col >> 24) & 0xff;
   r = (dc->col.col >> 16) & 0xff;
   g = (dc->col.col >> 8 ) & 0xff;
   b = (dc->col.col      ) & 0xff;
   evas_gl_common_context_color_set(gc, r, g, b, a);
   if (a < 255) evas_gl_common_context_blend_set(gc, 1);
   else evas_gl_common_context_blend_set(gc, 0);
   if (dc->clip.use)
     evas_gl_common_context_clip_set(gc, 1,
				     dc->clip.x, dc->clip.y,
				     dc->clip.w, dc->clip.h);
   else
     evas_gl_common_context_clip_set(gc, 0,
				     0, 0, 0, 0);
   evas_gl_common_context_texture_set(gc, NULL, 0, 0, 0);
   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_BACK);

#ifdef GLU_TESS   
   if (!tess)
     {
	tess = gluNewTess();
	
	gluTessCallback(tess, GLU_TESS_BEGIN, _evas_gl_tess_begin_cb);
	gluTessCallback(tess, GLU_TESS_END, _evas_gl_tess_end_cb);
	gluTessCallback(tess, GLU_TESS_ERROR, _evas_gl_tess_error_cb);
	gluTessCallback(tess, GLU_TESS_VERTEX, _evas_gl_tess_vertex_cb);
	gluTessCallback(tess, GLU_TESS_COMBINE, _evas_gl_tess_combine_cb);
     }
   num = 0;
   num = evas_list_count(poly->points);
   i = 0;
   glp = malloc(num * 6 * sizeof(GLdouble));
   gluTessNormal(tess, 0, 0, 1);
   gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
   gluTessBeginPolygon(tess, NULL);
   gluTessBeginContour(tess);
   for (l = poly->points; l; l = l->next)
     {
	Evas_GL_Polygon_Point *p;
	
	p = l->data;
	glp[i++] = p->x;
	glp[i++] = p->y;
	glp[i++] = 0;
	gluTessVertex(tess, &(glp[i - 3]), &(glp[i - 3]));
	i += 3;
     }
   gluTessEndContour(tess);
   gluTessEndPolygon(tess);
   free(glp);
#else
   glBegin(GL_POLYGON);
   for (l = poly->points; l; l = l->next)
     {
	Evas_GL_Polygon_Point *p;
	
	p = l->data;
	glVertex2i(p->x, p->y);
     }
   glEnd();
#endif   
}

void
evas_gl_common_gradient_draw(Evas_GL_Context *gc, RGBA_Draw_Context *dc, Evas_GL_Gradient *gr, int x, int y, int w, int h, double angle)
{    
   int r, g, b, a;

   if (dc->mul.use)
     {
	a = (dc->mul.col >> 24) & 0xff;
	r = (dc->mul.col >> 16) & 0xff;
	g = (dc->mul.col >> 8 ) & 0xff;
	b = (dc->mul.col      ) & 0xff;
     }
   else
     {
	r = g = b = a = 255;
     }
   evas_gl_common_context_color_set(gc, r, g, b, a);
   evas_gl_common_context_blend_set(gc, 1);
   if (dc->clip.use)
     evas_gl_common_context_clip_set(gc, 1,
				     dc->clip.x, dc->clip.y,
				     dc->clip.w, dc->clip.h);
   else
     evas_gl_common_context_clip_set(gc, 0,
				     0, 0, 0, 0);
   if (!gr->tex)
     _evas_gl_common_gradient_texture_build(gc, gr);
   evas_gl_common_context_texture_set(gc, gr->tex, 255, 3, 1);
   evas_gl_common_context_read_buf_set(gc, GL_BACK);
   evas_gl_common_context_write_buf_set(gc, GL_BACK);
     {
	double max, t[8];
	int tw, th, i;
	
	tw = 256;
	th = 4;
	
	t[0] = cos(((-angle + 45 + 90) * 2 * 3.141592654) / 360);
	t[1] = sin(((-angle + 45 + 90) * 2 * 3.141592654) / 360);
	
	t[2] = cos(((-angle + 45 + 180) * 2 * 3.141592654) / 360);
	t[3] = sin(((-angle + 45 + 180) * 2 * 3.141592654) / 360);
	
	t[4] = cos(((-angle + 45 + 270) * 2 * 3.141592654) / 360);
	t[5] = sin(((-angle + 45 + 270) * 2 * 3.141592654) / 360);
	
	t[6] = cos(((-angle + 45 + 0) * 2 * 3.141592654) / 360);
	t[7] = sin(((-angle + 45 + 0) * 2 * 3.141592654) / 360);
	max = 0;
	
	for (i = 0; i < 8; i++)
	  {
	     if ((t[i] < 0) && (-t[i] > max)) max = -t[i];
	     else if ((t[i] > max)) max = t[i];
	  }
	if (max > 0)
	  {
	     for (i = 0; i < 8; i++) t[i] *= 1 / max;
	  }
	for (i = 0; i < 8; i+=2)
	  {
	     t[i] = (1.0 + ((((0.5) + (t[i] / 2.0)) * (256.0 - 2.0)))) / 256.0;
	     t[i + 1] = (1.0 + ((((0.5) - (t[i + 1] / 2.0))) * 2.0)) / 4.0;
	  }
	glBegin(GL_QUADS);
	glTexCoord2d(t[0],  t[1]); glVertex2i(x, y);
	glTexCoord2d(t[2],  t[3]); glVertex2i(x + w, y);
	glTexCoord2d(t[4],  t[5]); glVertex2i(x + w, y + h);
	glTexCoord2d(t[6],  t[7]); glVertex2i(x, y + h);
	glEnd();
     }
}






static void
_evas_gl_common_texture_mipmaps_build(Evas_GL_Texture *tex, RGBA_Image *im, int smooth)
{
   int *im_data;
   int tw, th;
   int im_w, im_h, w, h;
   GLenum pixfmt, texfmt;
   int level;
   RGBA_Image *im1 = NULL, *im2 = NULL;
#ifdef BUILD_MMX
   int mmx, sse, sse2;
#endif

   if (!smooth) return;
   if (tex->not_power_of_two) return;
#ifdef BUILD_MMX
   evas_common_cpu_can_do(&mmx, &sse, &sse2);
#endif
   tw = tex->w;
   th = tex->h;
   w = im->image->w;
   h = im->image->h;
   level = 0;
   im1 = im;
   
   if (tex->gc->texture != tex)
     {
	if (tex->gc->texture) tex->gc->texture->references--;
	tex->gc->texture = tex;
	tex->gc->change.texture = 1;
	if (tex) tex->references++;
     }

   if (im->flags & RGBA_IMAGE_HAS_ALPHA) texfmt = GL_RGBA8;
   else texfmt = GL_RGB8;
   pixfmt = GL_BGRA;
   
   printf("building mipmaps... [%i x %i]\n", tw, th);
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D, tex->texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   while ((w > 1) || (h > 1))
     {
	int pw, ph;
	
	pw = w;
	ph = h;
	w /= 2;
	h /= 2;
	if (w < 1) w = 1;
	if (h < 1) h = 1;
	tw /= 2;
	th /= 2;
	
	level++;
	im2 = evas_common_image_create(w, h);
#ifdef BUILD_MMX
	if (mmx)
	  {  
	     evas_common_scale_rgba_mipmap_down_2x2_mmx(im1->image->data,
							im2->image->data, 
							pw, ph);
	  }
	else
#endif	  
	  {
	     if (im->flags & RGBA_IMAGE_HAS_ALPHA)
	       evas_common_scale_rgba_mipmap_down_2x2_c(im1->image->data,
							im2->image->data, 
							pw, ph);
	     else
	       evas_common_scale_rgb_mipmap_down_2x2_c(im1->image->data,
						       im2->image->data, 
						       pw, ph);
	  }
	if (im1 != im) evas_common_image_free(im1);
	im1 = NULL;
	
	im_data = im2->image->data;
	im_w = w;
	im_h = h;
#ifdef NVIDIA_HACK
/* NVIDIA HACK 1,part-2 */
   /* Nvidia's 4496 drivers and below have a bug. unless i "allocate" the
    * mipmap space here before i go and use a texture, it will never accept
    * mipmaps being added to a texture later on, or at the least it will never
    * use added mipmaps.
    * 
    * so as a workaround i allocate the mipmap texels here with a NULL pointer
    * passed to glTexImage2D() and i fill in the mipmap pixels later on in
    * _evas_gl_common_texture_mipmaps_build(). this works, but is ugly.
    * 
    * i currently have no reason to believe GL can't do what i want to do,
    * that is add mipmaps and allocate space for them any time i want. see
    */
/* disable this as the mipmap was already allocated earlier on
	glTexImage2D(GL_TEXTURE_2D, level,
		     texfmt, tw, th, 0,
		     pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
 */
#else
	glTexImage2D(GL_TEXTURE_2D, level,
		     texfmt, tw, th, 0,
		     pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);
#endif	
/* END NVIDIA HACK */	
	glTexSubImage2D(GL_TEXTURE_2D, level, 
			0, 0, im_w, im_h,
			pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			im_data);
	if (im_w < tw)
	  glTexSubImage2D(GL_TEXTURE_2D, level, 
			  im_w, 0, 1, im_h,
			  pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			  im_data + im_w - 1);
	if (im_h < th)
	  glTexSubImage2D(GL_TEXTURE_2D, level,
			  0, im_h, im_w, 1,
			  pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			  im_data + (im_w * (im_h - 1)));
	if ((im_w < tw) && (im_h < th))
	  glTexSubImage2D(GL_TEXTURE_2D, level, 
			  im_w, im_h, 1, 1,
			  pixfmt, GL_UNSIGNED_INT_8_8_8_8_REV,
			  im_data + (im_w * (im_h - 1)) + im_w - 1);
	im1 = im2;
	im2 = NULL;
     }
   if ((im1 != im) && (im1)) evas_common_image_free(im1);
   tex->have_mipmaps = 1;
#ifdef BUILD_MMX
   if (mmx) evas_common_cpu_end_opt();
#endif   
}

static void
_evas_gl_common_gradient_texture_build(Evas_GL_Context *gc, Evas_GL_Gradient *gr)
{
   DATA32 *map;
   RGBA_Draw_Context *dc;
   RGBA_Image *im;

   dc = evas_common_draw_context_new();
   if (!dc) return;
   map = evas_common_gradient_map(gr->grad, dc, 256);
   if (map)
     {
	im = evas_common_image_create(256, 4);
	if (im)
	  {
	     memcpy(im->image->data      , map, 256 * sizeof(DATA32));
	     memcpy(im->image->data + 256, map, 256 * sizeof(DATA32));
	     memcpy(im->image->data + 512, map, 256 * sizeof(DATA32));
	     memcpy(im->image->data + 768, map, 256 * sizeof(DATA32));
	     im->flags |= RGBA_IMAGE_HAS_ALPHA;
	     gr->tex = evas_gl_common_texture_new(gc, im, 1);
	     if (gr->tex)
	       {
		  if (!gr->tex->have_mipmaps)
		    _evas_gl_common_texture_mipmaps_build(gr->tex, im, 1);
	       }
	     evas_common_image_free(im);
	  }
	free(map);
     }
   evas_common_draw_context_free(dc);
}

static void
_evas_gl_common_viewport_set(Evas_GL_Context *gc)
{
   if (!gc->change.size) return;
   glViewport(0, 0, gc->w, gc->h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(0, gc->w, 0, gc->h, -1, 1);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glScalef(1, -1, 1);
   glTranslatef(0, - gc->h, 0);	
   gc->change.size = 0;
}

static void
_evas_gl_common_dither_set(Evas_GL_Context *gc)
{
   if (!gc->change.dither) return;
   if (gc->dither)
     glEnable(GL_DITHER);
   else
     glDisable(GL_DITHER);
   gc->change.dither = 0;
}

static void
_evas_gl_common_blend_set(Evas_GL_Context *gc)
{
   if (!gc->change.blend) return;
   if (gc->blend) 
     {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
     }
   else
     glDisable(GL_BLEND);
   gc->change.blend = 0;
}

static void
_evas_gl_common_color_set(Evas_GL_Context *gc)
{
   int r, g, b, a;
   
   if (!gc->change.color) return;
   glColor4d((double)gc->r / 255.0,
	     (double)gc->g / 255.0,
	     (double)gc->b / 255.0,
	     (double)gc->a / 255.0);
   gc->change.color = 0;
}

static void
_evas_gl_common_texture_set(Evas_GL_Context *gc)
{
   if (!gc->change.texture) return;
   if (gc->texture)
     {
	if (gc->texture->not_power_of_two)
	  {
	     glEnable(GL_TEXTURE_2D);
	     glEnable(GL_TEXTURE_RECTANGLE_NV);
	     glBindTexture(GL_TEXTURE_RECTANGLE_NV, gc->texture->texture);
	  }
	else
	  {
	     glDisable(GL_TEXTURE_RECTANGLE_NV);
	     glEnable(GL_TEXTURE_2D);
	     glBindTexture(GL_TEXTURE_2D, gc->texture->texture);
	  }
	if (gc->texture->not_power_of_two)
	  {
	     if (gc->texture->changed)
	       {
		  gc->texture->changed = 0;
	       }
	  }
	else
	  {
	     if (gc->texture->changed)
	       {
		  if (gc->texture->smooth)
		    {
		       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		       if (gc->texture->have_mipmaps)
			 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		       else
			 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		    }
		  else
		    {
		       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		    }
		  gc->texture->changed = 0;
	       }
	  }
     }
   else
     glDisable(GL_TEXTURE_2D);   
   gc->change.texture = 0;
}

static void
_evas_gl_common_clip_set(Evas_GL_Context *gc)
{
   if (!gc->change.clip) return;
   if (gc->clip.active)
     {
	glEnable(GL_SCISSOR_TEST);
	glScissor(gc->clip.x,
		  gc->h - gc->clip.y - gc->clip.h, 
		  gc->clip.w, 
		  gc->clip.h);
     }
   else
     glDisable(GL_SCISSOR_TEST);
   gc->change.clip = 0;
}

static void
_evas_gl_common_buf_set(Evas_GL_Context *gc)
{
   if (!gc->change.buf) return;
   glDrawBuffer(gc->write_buf);
   glReadBuffer(gc->read_buf);
   gc->change.buf = 0;
}

static void
_evas_gl_common_other_set(Evas_GL_Context *gc)
{
   if (!gc->change.other) return;
   glShadeModel(GL_FLAT);
   glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
//   glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
   glDisable(GL_LINE_SMOOTH);
   glDisable(GL_CULL_FACE);
   glDepthMask(GL_FALSE);
   glPixelStorei(GL_PACK_ALIGNMENT, 1);
   gc->change.other = 0;
}
