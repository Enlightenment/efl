/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EVAS_CONVERT_MAIN_H
#define _EVAS_CONVERT_MAIN_H


EAPI void             evas_common_convert_init          (void);
EAPI Gfx_Func_Convert evas_common_convert_func_get      (DATA8 *dest, int w, int h, int depth, DATA32 rmask, DATA32 gmask, DATA32 bmask, Convert_Pal_Mode pal_mode, int rotation);


#endif /* _EVAS_CONVERT_MAIN_H */
