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

RGBA_Gfx_Func        evas_common_gfx_func_composite_pixel_span_get       (Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, Eina_Bool dst_alpha, int pixels, int op);
RGBA_Gfx_Func        evas_common_gfx_func_composite_color_span_get       (DATA32 col, Eina_Bool dst_alpha, int pixels, int op);
RGBA_Gfx_Func        evas_common_gfx_func_composite_pixel_color_span_get (Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, DATA32 col, Eina_Bool dst_alpha, int pixels, int op);
RGBA_Gfx_Func        evas_common_gfx_func_composite_mask_color_span_get  (DATA32 col, Eina_Bool dst_alpha, int pixels, int op);
RGBA_Gfx_Func        evas_common_gfx_func_composite_pixel_mask_span_get  (Eina_Bool src_alpha, Eina_Bool src_sparse_alpha, Eina_Bool dst_alpha, int pixels, int op);

RGBA_Gfx_Pt_Func     evas_common_gfx_func_composite_pixel_pt_get         (Eina_Bool src_alpha, Eina_Bool dst_alpha, int op);
RGBA_Gfx_Pt_Func     evas_common_gfx_func_composite_color_pt_get         (DATA32 col, Eina_Bool dst_alpha, int op);
RGBA_Gfx_Pt_Func     evas_common_gfx_func_composite_pixel_color_pt_get   (Eina_Bool src_alpha, DATA32 col, Eina_Bool dst_alpha, int op);
RGBA_Gfx_Pt_Func     evas_common_gfx_func_composite_mask_color_pt_get    (DATA32 col, Eina_Bool dst_alpha, int op);
RGBA_Gfx_Pt_Func     evas_common_gfx_func_composite_pixel_mask_pt_get    (Eina_Bool src_alpha, Eina_Bool dst_alpha, int op);

#endif /* _EVAS_BLEND_PRIVATE_H */
