#include "evas_gl_private.h"

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

   if (gc->ext.nv_texture_rectangle)
     {
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
	pixfmt = NATIVE_PIX_FORMAT;

	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
		     texfmt, tex->w, tex->h, 0,
		     pixfmt, NATIVE_PIX_UNIT, im->image->data);
	return tex;
     }

   shift = 1; while (im->image->w > shift) shift = shift << 1; tw = shift;
   shift = 1; while (im->image->h > shift) shift = shift << 1; th = shift;
   tex->gc = gc;
   tex->w = tw;
   tex->h = th;
   tex->tw = im->image->w;
   tex->th = im->image->h;
   tex->references = 0;
   tex->smooth = 0;
   tex->changed = 1;
   if (gc->ext.nv_texture_rectangle) glDisable(GL_TEXTURE_RECTANGLE_NV);
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
   pixfmt = NATIVE_PIX_FORMAT;
   
   glTexImage2D(GL_TEXTURE_2D, 0, 
		texfmt, tw, th, 0,
		pixfmt, NATIVE_PIX_UNIT, NULL);
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
			  pixfmt, NATIVE_PIX_UNIT, NULL);
	  }
     }
   if (gc->ext.sgis_generate_mipmap)
     {
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	tex->have_mipmaps = 1;
     }
   glTexSubImage2D(GL_TEXTURE_2D, 0, 
		   0, 0, im_w, im_h,
		   pixfmt, NATIVE_PIX_UNIT,
		   im_data);
   if (im_w < tw)
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     im_w, 0, 1, im_h,
		     pixfmt, NATIVE_PIX_UNIT,
		     im_data + im_w - 1);
   if (im_h < th)
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     0, im_h, im_w, 1,
		     pixfmt, NATIVE_PIX_UNIT,
		     im_data + (im_w * (im_h - 1)));
   if ((im_w < tw) && (im_h < th))
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     im_w, im_h, 1, 1,
		     pixfmt, NATIVE_PIX_UNIT,
		     im_data + (im_w * (im_h - 1)) + im_w - 1);
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

//	printf("%p\n", glXAllocateMemoryNV(64 * 64 * 4, 0.0, 1.0, 1.0));
/*
	if (!tex->opt)
	  {
	     glPixelDataRangeNV(GL_WRITE_PIXEL_DATA_RANGE_NV, 
				im->image->w * im->image->h * 4, 
				im->image->data);
	     printf("ER1: %s\n", gluErrorString(glGetError()));
	     glEnableClientState(GL_WRITE_PIXEL_DATA_RANGE_NV);
	     printf("ER2: %s\n", gluErrorString(glGetError()));
	     tex->opt = 1;
	  }
*/
	if (tex->gc->texture) tex->gc->texture->references--;
	tex->gc->texture = tex;
	tex->gc->change.texture = 1;
	tex->references++;

	if (im->flags & RGBA_IMAGE_HAS_ALPHA) texfmt = GL_RGBA8;
	else texfmt = GL_RGB8;
	pixfmt = NATIVE_PIX_FORMAT;

	/* the MOMENT i call this call at ALL to update texture
	 * i instantly hit a MASSIVE speed hit - even for 1x1 pixel
	 */
	
#if 1 /* go from 2.3 to 15.9 if this is disabled */
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
			0, 0, tex->w, tex->h,
			pixfmt, NATIVE_PIX_UNIT,
			im->image->data);
#else
	/* also no speed difference to the above */
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 
		     texfmt, tex->w, tex->h, 0,
		     pixfmt, NATIVE_PIX_UNIT,
		     im->image->data);
#endif	
	return;
     }
   tw = tex->w;
   th = tex->h;
   tex->changed = 1;
   tex->have_mipmaps = 0;
   if (tex->gc->ext.nv_texture_rectangle) glDisable(GL_TEXTURE_RECTANGLE_NV);   
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
   pixfmt = NATIVE_PIX_FORMAT;

   if (tex->gc->ext.sgis_generate_mipmap)
     {
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
	tex->have_mipmaps = 1;
     }
   glTexSubImage2D(GL_TEXTURE_2D, 0, 
		   0, 0, im_w, im_h,
		   pixfmt, NATIVE_PIX_UNIT,
		   im_data);
#if 1 // this is sloooow... well slower than just the above...
   if (im_w < tw)
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     im_w, 0, 1, im_h,
		     pixfmt, NATIVE_PIX_UNIT,
		     im_data + im_w - 1);
   if (im_h < th)
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     0, im_h, im_w, 1,
		     pixfmt, NATIVE_PIX_UNIT,
		     im_data + (im_w * (im_h - 1)));
   if ((im_w < tw) && (im_h < th))
     glTexSubImage2D(GL_TEXTURE_2D, 0, 
		     im_w, im_h, 1, 1,
		     pixfmt, NATIVE_PIX_UNIT,
		     im_data + (im_w * (im_h - 1)) + im_w - 1);
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

void
evas_gl_common_texture_mipmaps_build(Evas_GL_Texture *tex, RGBA_Image *im, int smooth)
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
   pixfmt = NATIVE_PIX_FORMAT;
   
   printf("building mipmaps... [%i x %i]\n", tw, th);
   if (tex->gc->ext.nv_texture_rectangle) glDisable(GL_TEXTURE_RECTANGLE_NV);
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
	glTexSubImage2D(GL_TEXTURE_2D, level, 
			0, 0, im_w, im_h,
			pixfmt, NATIVE_PIX_UNIT,
			im_data);
	if (im_w < tw)
	  glTexSubImage2D(GL_TEXTURE_2D, level, 
			  im_w, 0, 1, im_h,
			  pixfmt, NATIVE_PIX_UNIT,
			  im_data + im_w - 1);
	if (im_h < th)
	  glTexSubImage2D(GL_TEXTURE_2D, level,
			  0, im_h, im_w, 1,
			  pixfmt, NATIVE_PIX_UNIT,
			  im_data + (im_w * (im_h - 1)));
	if ((im_w < tw) && (im_h < th))
	  glTexSubImage2D(GL_TEXTURE_2D, level, 
			  im_w, im_h, 1, 1,
			  pixfmt, NATIVE_PIX_UNIT,
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
