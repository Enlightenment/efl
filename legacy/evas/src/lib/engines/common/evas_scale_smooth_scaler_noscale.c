{
   DATA32 *src_data;
   
   src_data = src->image->data;
   ptr = src_data + ((dst_clip_y - dst_region_y + src_region_y) * src_w) + (dst_clip_x - dst_region_x) + src_region_x;
   if (dc->mod.use)
     {
	Gfx_Func_Blend_Src_Cmod_Dst func;
	
	func = evas_common_draw_func_blend_cmod_get(src, dst, dst_clip_w);
	for (y = 0; y < dst_clip_h; y++)
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
	for (y = 0; y < dst_clip_h; y++)
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
#if 0
	/* part of the spans experiemnt. doesnt seem to help much on top of
	 * what we already have
	 */
	if (src->image->spans)
	  {
	     int x2, y2;
	     int xoff, woff;
	     RGBA_Image_Flags pflags;
	     Gfx_Func_Blend_Src_Dst func_solid;
	     
	     pflags = src->flags;
	     src->flags &= ~RGBA_IMAGE_HAS_ALPHA;
	     func_solid = evas_common_draw_func_blend_get(src, dst, dst_clip_w);
	     src->flags = pflags;
	     
	     x2 = (dst_clip_x - dst_region_x) + src_region_x;
	     y2 = (dst_clip_y - dst_region_y) + src_region_y;
	     for (y = 0; y < dst_clip_h; y++, y2++)
	       {
		  Evas_Object_List *l;
		  
		  for (l = src->image->spans[y2]; l; l = l->next)
		    {
		       RGBA_Image_Span *sp;
		       
		       sp = l;
		       if ((sp->x + sp->w) > x2)
			 {
			    xoff = sp->x - x2;
			    woff = sp->w;
			    if (xoff < 0)
			      {
				 woff += xoff;
				 xoff = 0;
			      }
			    if ((xoff + woff) > (dst_clip_w))
			      woff += (dst_clip_w) - (xoff + woff);
			    if (sp->v == 2)
			      func_solid(ptr + xoff, dst_ptr + xoff, woff);
			    else
			      func(ptr + xoff, dst_ptr + xoff, woff);
			 }
		    }
		  ptr += src_w;
		  dst_ptr += dst_w;
	       }
	  }
	else
#endif	  
	  {
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  func(ptr, dst_ptr, dst_clip_w);
		  ptr += src_w;
		  dst_ptr += dst_w;
	       }
	  }
     }
}
