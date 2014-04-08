#ifndef _EVAS_IMAGE_PRIVATE_H
#define _EVAS_IMAGE_PRIVATE_H

int             evas_common_rgba_image_size_set              (Image_Entry* dst, const Image_Entry* im, unsigned int w, unsigned int h);
int             evas_common_rgba_image_from_copied_data      (Image_Entry* dst, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
int             evas_common_rgba_image_from_data             (Image_Entry* dst, unsigned int w, unsigned int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace);
int             evas_common_rgba_image_colorspace_set        (Image_Entry* dst, Evas_Colorspace cspace);

void evas_common_scalecache_init(void);
void evas_common_scalecache_shutdown(void);
void evas_common_rgba_image_scalecache_dirty(Image_Entry *ie);
void evas_common_rgba_image_scalecache_orig_use(Image_Entry *ie);
int evas_common_rgba_image_scalecache_usage_get(Image_Entry *ie);

#endif /* _EVAS_IMAGE_PRIVATE_H */
