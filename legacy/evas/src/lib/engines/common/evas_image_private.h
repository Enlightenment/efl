#ifndef _EVAS_IMAGE_PRIVATE_H
#define _EVAS_IMAGE_PRIVATE_H

typedef struct _Image_Draw_Data Image_Draw_Data;
struct _Image_Draw_Data
{
   int     axx, axy, axz;
   int     ayx, ayy, ayz;

   int     fw, fh;
   int     iaxx, iayy;
   int     ibxx, ibyy;
   int     dax, day;

   int     l, r, t, b;
   unsigned char has_alpha : 1;
   unsigned char border_center_fill : 1;
};

int             evas_common_load_rgba_image_module_from_file (Image_Entry *im);
int             evas_common_load_rgba_image_data_from_file   (Image_Entry *im);

int             evas_common_rgba_image_size_set              (Image_Entry* dst, const Image_Entry* im, int w, int h);
int             evas_common_rgba_image_from_copied_data      (Image_Entry* dst, int w, int h, DATA32 *image_data, int alpha, int cspace);
int             evas_common_rgba_image_from_data             (Image_Entry* dst, int w, int h, DATA32 *image_data, int alpha, int cspace);
int             evas_common_rgba_image_colorspace_set        (Image_Entry* dst, int cspace);

int             evas_common_image_draw_data_setup(void *pim, Image_Draw_Data *idata);

#endif /* _EVAS_IMAGE_PRIVATE_H */
