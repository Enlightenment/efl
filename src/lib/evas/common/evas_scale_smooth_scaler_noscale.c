{
   DATA32 *ptr;
   RGBA_Gfx_Func func;

   ptr = src->image.data + ((dst_clip_y - dst_region_y + src_region_y) * src_w) + (dst_clip_x - dst_region_x) + src_region_x;
   if (mul_col != 0xffffffff)
     func = evas_common_gfx_func_composite_pixel_color_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, mul_col, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
   else
     func = evas_common_gfx_func_composite_pixel_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, render_op);

   while (dst_clip_h--)
     {
        func(ptr, NULL, mul_col, dst_ptr, dst_clip_w);

        ptr += src_w;
        dst_ptr += dst_w;
     }
}

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
