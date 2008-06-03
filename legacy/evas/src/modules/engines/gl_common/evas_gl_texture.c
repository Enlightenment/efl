#include "evas_gl_private.h"

Evas_GL_Texture *
evas_gl_common_texture_new(Evas_GL_Context *gc, RGBA_Image *im, int smooth)
{
   Evas_GL_Texture *tex;
   DATA32 *im_data;
   int im_w, im_h;
   int tw, th;
   int shift;
   GLenum pixfmt, texfmt;

   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;

   if ((gc->ext.nv_texture_rectangle) &&
       (!(gc->ext.arb_texture_non_power_of_two &&
	  gc->ext.sgis_generate_mipmap)))
     {
	tex->gc = gc;
	tex->w = im->cache_entry.w;
	tex->h = im->cache_entry.h;
	tex->rectangle = 1;
	tex->tw = im->cache_entry.w;
	tex->th = im->cache_entry.h;
	tex->references = 0;
	tex->smooth = smooth;
	tex->changed = 1;

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glGenTextures(1, &(tex->texture));
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex->texture);
	if (smooth)
	  {
	     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  }
	else
	  {
	     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	     glTexParameteri(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	  }

	if (gc->texture) gc->texture->references--;
	gc->texture = tex;
	gc->change.texture = 1;
	tex->references++;

	if (im->cache_entry.flags.alpha) texfmt = GL_RGBA8;
	else texfmt = GL_RGB8;
	pixfmt = NATIVE_PIX_FORMAT;

	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
		     texfmt, tex->w, tex->h, 0,
		     pixfmt, NATIVE_PIX_UNIT, im->image.data);
	return tex;
     }

   if ((gc->ext.arb_texture_non_power_of_two) && (gc->ext.sgis_generate_mipmap))
     {
	tw = im->cache_entry.w;
	th = im->cache_entry.h;
     }
   else
     {
	shift = 1; while (im->cache_entry.w > shift) shift = shift << 1; tw = shift;
	shift = 1; while (im->cache_entry.h > shift) shift = shift << 1; th = shift;
     }
   tex->gc = gc;
   tex->w = tw;
   tex->h = th;
   tex->tw = im->cache_entry.w;
   tex->th = im->cache_entry.h;
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

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   im_data = im->image.data;
   im_w = im->cache_entry.w;
   im_h = im->cache_entry.h;

   if (im->cache_entry.flags.alpha) texfmt = GL_RGBA8;
   else texfmt = GL_RGB8;
   pixfmt = NATIVE_PIX_FORMAT;

   glTexImage2D(GL_TEXTURE_2D, 0,
		texfmt, tw, th, 0,
		pixfmt, NATIVE_PIX_UNIT, NULL);
/*   
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
 */
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
   DATA32 *im_data;
   int im_w, im_h;
   int tw, th;
   GLenum pixfmt, texfmt, target;

   if (tex->rectangle)
     {
	void *data;

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex->texture);

	data = im->image.data;
#if 0 // trying the glXAllocateMemoryNV() thing but its abysmally slow
	tmp = glXAllocateMemoryNV(tex->w * tex->h * sizeof(DATA32),
				  0.0, 1.0, 1.0);
	if (tmp)
	  {
	     glEnableClientState(GL_WRITE_PIXEL_DATA_RANGE_NV);
	     glPixelDataRangeNV(GL_WRITE_PIXEL_DATA_RANGE_NV,
				tex->w * tex->h * sizeof(DATA32),
				tmp);
//	     evas_common_copy_pixels_rgba_to_rgba_mmx2(im->image->data, tmp,
//						       tex->w * tex->h);
	     memcpy(tmp, im->image.data,
		    tex->w * tex->h * sizeof(DATA32));
	     data = tmp;
	  }
#endif
	if (tex->gc->texture) tex->gc->texture->references--;
	tex->gc->texture = tex;
	tex->gc->change.texture = 1;
	tex->references++;

	if (im->cache_entry.flags.alpha) texfmt = GL_RGBA8;
	else texfmt = GL_RGB8;
	pixfmt = NATIVE_PIX_FORMAT;

	glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0,
			0, 0, tex->w, tex->h,
			pixfmt, NATIVE_PIX_UNIT,
			data);
#if 0 // trying the glXAllocateMemoryNV() thing but its abysmally slow
	if (tmp)
	  {
	     glFlushPixelDataRangeNV(GL_WRITE_PIXEL_DATA_RANGE_NV);
	     glXFreeMemoryNV(tmp);
	  }
#endif
	return;
     }
   tw = tex->w;
   th = tex->h;
   tex->changed = 1;
   tex->have_mipmaps = 0;
//   if (tex->gc->ext.nv_texture_rectangle) glDisable(GL_TEXTURE_RECTANGLE_NV);
   glEnable(GL_TEXTURE_2D);
   if (tex->rectangle)
     {
	glEnable(GL_TEXTURE_RECTANGLE_NV);
	target = GL_TEXTURE_RECTANGLE_NV;
     }
   else
     {
//	glDisable(GL_TEXTURE_RECTANGLE_NV);
	glEnable(GL_TEXTURE_2D);//
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
   tex->smooth = 0;

   im_data = im->image.data;
   im_w = im->cache_entry.w;
   im_h = im->cache_entry.h;

   if (im->cache_entry.flags.alpha) texfmt = GL_RGBA8;
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
   if (tex->texture2) glDeleteTextures(1, &tex->texture2);
   if (tex->texture3) glDeleteTextures(1, &tex->texture3);
/*   
   if (tex->fshad)
     {
	glDeleteObjectARB(tex->fshad);
     }
   if (tex->prog)
     {
	glDeleteObjectARB(tex->prog);
     }
 */
   free(tex);
}

void
evas_gl_common_texture_mipmaps_build(Evas_GL_Texture *tex, RGBA_Image *im, int smooth)
{
   DATA32 *im_data;
   int tw, th;
   int im_w, im_h, w, h;
   GLenum pixfmt, texfmt;
   int level;
   RGBA_Image *im1 = NULL, *im2 = NULL;
#ifdef BUILD_MMX
   int mmx, sse, sse2;
#endif

   if (!smooth) return;
   if (tex->rectangle) return;
#ifdef BUILD_MMX
   evas_common_cpu_can_do(&mmx, &sse, &sse2);
#endif
   tw = tex->w;
   th = tex->h;
   w = im->cache_entry.w;
   h = im->cache_entry.h;
   level = 0;
   im1 = im;

   if (tex->gc->texture != tex)
     {
	if (tex->gc->texture) tex->gc->texture->references--;
	tex->gc->texture = tex;
	tex->gc->change.texture = 1;
	if (tex) tex->references++;
     }

   if (im->cache_entry.flags.alpha) texfmt = GL_RGBA8;
   else texfmt = GL_RGB8;
   pixfmt = NATIVE_PIX_FORMAT;

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
	im2 = (RGBA_Image *) evas_cache_image_empty(evas_common_image_cache_get());
        im2 = (RGBA_Image *) evas_cache_image_size_set(&im2->cache_entry, w, h);
#ifdef BUILD_MMX
	if (mmx)
	  {
	     evas_common_scale_rgba_mipmap_down_2x2_mmx(im1->image.data,
							im2->image.data,
							pw, ph);
	  }
	else
#endif
	  {
	     if (im->cache_entry.flags.alpha)
	       evas_common_scale_rgba_mipmap_down_2x2_c(im1->image.data,
							im2->image.data,
							pw, ph);
	     else
	       evas_common_scale_rgb_mipmap_down_2x2_c(im1->image.data,
						       im2->image.data,
						       pw, ph);
	  }
	if (im1 != im) evas_cache_image_drop(&im1->cache_entry);
	im1 = NULL;

	im_data = im2->image.data;
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
   if ((im1 != im) && (im1)) evas_cache_image_drop(&im1->cache_entry);
   tex->have_mipmaps = 1;
#ifdef BUILD_MMX
   if (mmx) evas_common_cpu_end_opt();
#endif
}

Evas_GL_Texture *
evas_gl_common_ycbcr601pl_texture_new(Evas_GL_Context *gc, unsigned char **rows, int w, int h, int smooth)
{
   Evas_GL_Texture *tex;
   int im_w, im_h, tw, th, y;
   GLenum texfmt;
  
// on an nv 6600gt this is fast - but on a 5500fx its DEAD SLOW!!!!!   
//   if (!gc->ext.arb_texture_non_power_of_two) return NULL;
   
   tex = calloc(1, sizeof(Evas_GL_Texture));
   if (!tex) return NULL;
   
   tw = w;
   th = h;
   tex->gc = gc;
   tex->w = tw;
   tex->h = th;
   tex->tw = w;
   tex->th = h;
   tex->references = 0;
   tex->smooth = 0;
   tex->changed = 1;

   tex->prog = gc->yuv422p.prog;
   
   glEnable(GL_TEXTURE_2D);
   texfmt = GL_LUMINANCE;

   glUseProgramObjectARB(tex->prog);
   
   glGenTextures(1, &(tex->texture));
   glBindTexture(GL_TEXTURE_2D, tex->texture);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   if (smooth)
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     }
   else
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     }
   glTexImage2D(GL_TEXTURE_2D, 0, texfmt, tex->w, tex->h, 0,
		texfmt, GL_UNSIGNED_BYTE, NULL);
   if (tex->h >= 2)
     glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[1] - rows[0]);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->w, tex->h,
		   texfmt, GL_UNSIGNED_BYTE, rows[0]);
//   for (y = 0; y < tex->h; y++)
//     glTexSubImage2D(GL_TEXTURE_2D, 0,
//		     0, y, tex->w, 1,
//		     texfmt, GL_UNSIGNED_BYTE, rows[y]);
   
   glGenTextures(1, &(tex->texture2));
   glBindTexture(GL_TEXTURE_2D, tex->texture2);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   if (smooth)
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     }
   else
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     }
   glTexImage2D(GL_TEXTURE_2D, 0, texfmt, tex->w / 2, tex->h / 2, 0,
		texfmt, GL_UNSIGNED_BYTE, NULL);
   if (tex->h >= 4)
     glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[tex->h + 1] - rows[tex->h]);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->w / 2, tex->h / 2,
		   texfmt, GL_UNSIGNED_BYTE, rows[tex->h]);
//   for (y = 0; y < (tex->h / 2); y++)
//     glTexSubImage2D(GL_TEXTURE_2D, 0,
//		     0, y, tex->w / 2, 1,
//		     texfmt, GL_UNSIGNED_BYTE, rows[tex->h + y]);
   
   glGenTextures(1, &(tex->texture3));
   glBindTexture(GL_TEXTURE_2D, tex->texture3);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   if (smooth)
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     }
   else
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     }
   glTexImage2D(GL_TEXTURE_2D, 0,
		texfmt, tex->w / 2, tex->h / 2, 0,
		texfmt, GL_UNSIGNED_BYTE, NULL);
   if (tex->h >= 4)
     glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[tex->h + (tex->h / 2) + 1] - rows[tex->h + (tex->h / 2)]);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->w / 2, tex->h / 2,
		   texfmt, GL_UNSIGNED_BYTE, rows[tex->h + (tex->h / 2)]);
//   for (y = 0; y < (tex->h / 2); y++)
//     glTexSubImage2D(GL_TEXTURE_2D, 0,
//		     0, y, tex->w / 2, 1,
//		     texfmt, GL_UNSIGNED_BYTE, rows[tex->h + (tex->h / 2) + y]);
   
   glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
   glUseProgramObjectARB(0);
   
   if (gc->texture) gc->texture->references--;
   gc->texture = tex;
   gc->change.texture = 1;
   tex->references++;

   return tex;
}

void
evas_gl_common_ycbcr601pl_texture_update(Evas_GL_Texture *tex, unsigned char **rows, int w, int h, int smooth)
{
   int texfmt;
   int y;
   
   /* FIXME: should use subimage */
   glEnable(GL_TEXTURE_2D);
   texfmt = GL_LUMINANCE;
   
   glBindTexture(GL_TEXTURE_2D, tex->texture);
   if (smooth)
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     }
   else
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     }
   if (tex->h >= 2)
     glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[1] - rows[0]);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->w, tex->h,
		   texfmt, GL_UNSIGNED_BYTE, rows[0]);
//   for (y = 0; y < tex->h; y++)
//     glTexSubImage2D(GL_TEXTURE_2D, 0,
//		     0, y, tex->w, 1,
//		     texfmt, GL_UNSIGNED_BYTE, rows[y]);
   
   glBindTexture(GL_TEXTURE_2D, tex->texture2);
   if (smooth)
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     }
   else
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     }
   if (tex->h >= 4)
     glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[tex->h + 1] - rows[tex->h]);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->w / 2, tex->h / 2,
		   texfmt, GL_UNSIGNED_BYTE, rows[tex->h]);
//   for (y = 0; y < (tex->h / 2); y++)
//     glTexSubImage2D(GL_TEXTURE_2D, 0,
//		     0, y, tex->w / 2, 1,
//		     texfmt, GL_UNSIGNED_BYTE, rows[tex->h + y]);
   
   glBindTexture(GL_TEXTURE_2D, tex->texture3);
   if (smooth)
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
     }
   else
     {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     }
   if (tex->h >= 4)
     glPixelStorei(GL_UNPACK_ROW_LENGTH, rows[tex->h + (tex->h / 2) + 1] - rows[tex->h + (tex->h / 2)]);
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->w / 2, tex->h / 2,
		   texfmt, GL_UNSIGNED_BYTE, rows[tex->h + (tex->h / 2)]);
//   for (y = 0; y < (tex->h / 2); y++)
//     glTexSubImage2D(GL_TEXTURE_2D, 0,
//		     0, y, tex->w / 2, 1,
//		     texfmt, GL_UNSIGNED_BYTE, rows[tex->h + (tex->h / 2) + y]);
   glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

   if (tex->gc->texture) tex->gc->texture->references--;
   tex->gc->texture = tex;
   tex->gc->change.texture = 1;
   tex->references++;
}
