/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EVAS_BLEND_PRIVATE_H
#define _EVAS_BLEND_PRIVATE_H


RGBA_Gfx_Compositor *evas_common_gfx_compositor_blend_get                (void);
RGBA_Gfx_Compositor *evas_common_gfx_compositor_blend_rel_get            (void);
RGBA_Gfx_Compositor *evas_common_gfx_compositor_copy_get                 (void);
RGBA_Gfx_Compositor *evas_common_gfx_compositor_copy_rel_get             (void);
RGBA_Gfx_Compositor *evas_common_gfx_compositor_add_get                  (void);
RGBA_Gfx_Compositor *evas_common_gfx_compositor_add_rel_get              (void);
/*
RGBA_Gfx_Compositor *evas_common_gfx_compositor_sub_get(void);
RGBA_Gfx_Compositor *evas_common_gfx_compositor_sub_rel_get(void);
*/
RGBA_Gfx_Compositor *evas_common_gfx_compositor_mask_get                 (void);
RGBA_Gfx_Compositor *evas_common_gfx_compositor_mul_get                  (void);

RGBA_Gfx_Func        evas_common_gfx_func_composite_pixel_span_get       (RGBA_Image *src, RGBA_Image *dst, int pixels, int op);
RGBA_Gfx_Func        evas_common_gfx_func_composite_color_span_get       (DATA32 col, RGBA_Image *dst, int pixels, int op);
RGBA_Gfx_Func        evas_common_gfx_func_composite_pixel_color_span_get (RGBA_Image *src, DATA32 col, RGBA_Image *dst, int pixels, int op);
RGBA_Gfx_Func        evas_common_gfx_func_composite_mask_color_span_get  (DATA32 col, RGBA_Image *dst, int pixels, int op);
RGBA_Gfx_Func        evas_common_gfx_func_composite_pixel_mask_span_get  (RGBA_Image *src, RGBA_Image *dst, int pixels, int op);

RGBA_Gfx_Pt_Func     evas_common_gfx_func_composite_pixel_pt_get         (Image_Entry_Flags src_flags, RGBA_Image *dst, int op);
RGBA_Gfx_Pt_Func     evas_common_gfx_func_composite_color_pt_get         (DATA32 col, RGBA_Image *dst, int op);
RGBA_Gfx_Pt_Func     evas_common_gfx_func_composite_pixel_color_pt_get   (Image_Entry_Flags src_flags, DATA32 col, RGBA_Image *dst, int op);
RGBA_Gfx_Pt_Func     evas_common_gfx_func_composite_mask_color_pt_get    (DATA32 col, RGBA_Image *dst, int op);
RGBA_Gfx_Pt_Func     evas_common_gfx_func_composite_pixel_mask_pt_get    (Image_Entry_Flags src_flags, RGBA_Image *dst, int op);


#endif /* _EVAS_BLEND_PRIVATE_H */
