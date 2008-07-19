#ifndef _EVAS_IMAGE_PRIVATE_H
#define _EVAS_IMAGE_PRIVATE_H

int             evas_common_load_rgba_image_module_from_file (Image_Entry *im);
int             evas_common_load_rgba_image_data_from_file   (Image_Entry *im);

int             evas_common_rgba_image_size_set              (Image_Entry* dst, const Image_Entry* im, int w, int h);
int             evas_common_rgba_image_from_copied_data      (Image_Entry* dst, int w, int h, DATA32 *image_data, int alpha, int cspace);
int             evas_common_rgba_image_from_data             (Image_Entry* dst, int w, int h, DATA32 *image_data, int alpha, int cspace);
int             evas_common_rgba_image_colorspace_set        (Image_Entry* dst, int cspace);


#endif /* _EVAS_IMAGE_PRIVATE_H */
