#ifndef EVAS_SOFT16_H
#define EVAS_SOFT16_H

#include "Evas.h"
#include "evas_private.h"
#include "evas_common.h"

typedef struct _Soft16_Image Soft16_Image;
  
struct _Soft16_Image
{
   const char    *file;       // file source - optional
   const char    *key;        // key within file source - optional
   time_t         timestamp;  // file modified timestamp
   time_t         laststat;   // last time this file was statted
   int            w, h;       // width and height in pixels
   int            stride;     // pixel stride - likely a multiple of 2
   DATA16        *pixels;     // 16bpp pixels rgb565
   DATA8         *alpha;      // 8bit alpha mask - optional. points into pixels
   int            references; // refcount
   RGBA_Image    *source_im;  // original source rgba image - if still reffed

   Evas_Image_Load_Opts lo;   // load options
   
   unsigned char  have_alpha  : 1; // 1 if we have halpha
   unsigned char  free_pixels : 1; // 1 if pixels should be freed
   unsigned char  free_alpha  : 1; // 1 if alpha mask should be freed
};

Soft16_Image *soft16_image_new(int w, int h, int stride, int have_alpha,
			       DATA16 *pixels, int copy);
void soft16_image_free(Soft16_Image *im);
Soft16_Image *soft16_image_load(const char *file, const char *key, int *error,
				Evas_Image_Load_Opts *lo);
void soft16_image_load_data(Soft16_Image *im);
void soft16_image_draw(Soft16_Image *src, Soft16_Image *dst,
		       RGBA_Draw_Context *dc,
		       int src_region_x, int src_region_y,
		       int src_region_w, int src_region_h,
		       int dst_region_x, int dst_region_y,
		       int dst_region_w, int dst_region_h,
		       int smooth);

void soft16_rectangle_draw(Soft16_Image *dst, RGBA_Draw_Context *dc,
                           int x, int y, int w, int h);
#endif
