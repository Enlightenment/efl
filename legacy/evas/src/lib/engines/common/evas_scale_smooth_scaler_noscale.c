{
   DATA32 *ptr;

   ptr = src->image->data + ((dst_clip_y - dst_region_y + src_region_y) * src_w) + (dst_clip_x - dst_region_x) + src_region_x;
   if (dc->mod.use)
     {
	Gfx_Func_Blend_Src_Cmod_Dst func;

	func = evas_common_draw_func_blend_cmod_get(src, dst, dst_clip_w);
	while (dst_clip_h--)
	  {
	     func(ptr, dst_ptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
	     ptr += src_w;
	     dst_ptr += dst_w;
	  }
     }
   else if (dc->mul.use)
     {
	Gfx_Func_Blend_Src_Mul_Dst func;

	func = evas_common_draw_func_blend_mul_get(src, dc->mul.col, dst, dst_clip_w);
	while (dst_clip_h--)
	  {
	     func(ptr, dst_ptr, dst_clip_w, dc->mul.col);
	     ptr += src_w;
	     dst_ptr += dst_w;
	  }
     }
   else
     {
	Gfx_Func_Blend_Src_Dst func;

	func = evas_common_draw_func_blend_get(src, dst, dst_clip_w);
	while (dst_clip_h--)
	  {
	     func(ptr, dst_ptr, dst_clip_w);
	     ptr += src_w;
	     dst_ptr += dst_w;
	  }
     }
}
