{
   int      x, y, w;
   int      *lin_ptr, *lp;
   int      *interp_x, *interp_y, *ix, *iy;
   DATA32   *buf, *dptr;
   DATA32   **row_ptr, **rp;
   DATA32   *ptr, *ptr2, *ptr3, *ptr4;
   DATA32   *src_data, *src_end;
   RGBA_Image  *line_buf;

   Gfx_Func_Blend_Src_Cmod_Dst func_cmod;
   Gfx_Func_Blend_Src_Mul_Dst  func_mul;
   Gfx_Func_Blend_Src_Dst      func;
		
   lin_ptr = (int *)malloc(dst_clip_w * sizeof(int));
   row_ptr = (DATA32 **)malloc(dst_clip_h * sizeof(DATA32 *));
   interp_x = (int *)malloc(dst_clip_w * sizeof(int));
   interp_y = (int *)malloc(dst_clip_h * sizeof(int));
   if ( (!lin_ptr) || (!row_ptr) || (!interp_x) || (!interp_y) )
      goto done_scale_up;

   /* a scanline buffer */
   line_buf = evas_common_image_line_buffer_obtain(dst_clip_w);
   if (!line_buf)
     goto done_scale_up;
   buf = line_buf->image->data;
	
   src_data = src->image->data;

   for (x = 0; x < dst_clip_w; x++)
     {
	if (src_region_w > 1)
	  {
	     lin_ptr[x] = (((x + dst_clip_x - dst_region_x) *
			    (src_region_w)) / dst_region_w);
	     if (dst_region_w > 262144)
	       interp_x[x] = (((((x + dst_clip_x - dst_region_x) *
				 (src_region_w))) / dst_region_w) -
			      (lin_ptr[x])) << 8;
	     else if (dst_region_w > 131072)
	       interp_x[x] = (((((x + dst_clip_x - dst_region_x) *
				 (src_region_w)) << 1) / dst_region_w) -
			      (lin_ptr[x] << 1)) << 7;
	     else if (dst_region_w > 65536)
	       interp_x[x] = (((((x + dst_clip_x - dst_region_x) *
				 (src_region_w)) << 2) / dst_region_w) -
			      (lin_ptr[x] << 2)) << 6;
	     else if (dst_region_w > 37268)
	       interp_x[x] = (((((x + dst_clip_x - dst_region_x) *
				 (src_region_w)) << 3) / dst_region_w) -
			      (lin_ptr[x] << 3)) << 5;
	     else if (dst_region_w > 16384)
	       interp_x[x] = (((((x + dst_clip_x - dst_region_x) *
				 (src_region_w)) << 4) / dst_region_w) -
			      (lin_ptr[x] << 4)) << 4;
	     else if (dst_region_w > 8192)
	       interp_x[x] = (((((x + dst_clip_x - dst_region_x) *
				 (src_region_w)) << 5) / dst_region_w) -
			      (lin_ptr[x] << 5)) << 3;
	     else if (dst_region_w > 4096)
	       interp_x[x] = (((((x + dst_clip_x - dst_region_x) *
				 (src_region_w)) << 6) / dst_region_w) -
			      (lin_ptr[x] << 6)) << 2;
	     else if (dst_region_w > 2048)
	       interp_x[x] = (((((x + dst_clip_x - dst_region_x) *
				 (src_region_w)) << 7) / dst_region_w) -
			      (lin_ptr[x] << 7)) << 1;
	     else
	       interp_x[x] = (((((x + dst_clip_x - dst_region_x) *
				 (src_region_w)) << 8) / dst_region_w) -
			      (lin_ptr[x] << 8));
	     lin_ptr[x] += src_region_x;
	  }
	else
	  {
	     lin_ptr[x] = (((x + dst_clip_x - dst_region_x) *
			    src_region_w) / dst_region_w);
	     interp_x[x] = 0;
	     lin_ptr[x] += src_region_x;
	  }
     }
   for (y = 0; y < dst_clip_h; y++)
     {
	int pos;

	if (src_region_h > 1)
	  {
	     pos = (((y + dst_clip_y - dst_region_y) *
		     (src_region_h)) / dst_region_h);
	     row_ptr[y] = src_data + ((pos + src_region_y) * src_w);
	     if (dst_region_h > 262144)
	       interp_y[y] = (((((y + dst_clip_y - dst_region_y) *
				 (src_region_h))) / dst_region_h) -
			      (pos)) << 8;
	     else if (dst_region_h > 131072)
	       interp_y[y] = (((((y + dst_clip_y - dst_region_y) *
				 (src_region_h)) << 1) / dst_region_h) -
			      (pos << 1)) << 7;
	     else if (dst_region_h > 65536)
	       interp_y[y] = (((((y + dst_clip_y - dst_region_y) *
				 (src_region_h)) << 2) / dst_region_h) -
			      (pos << 2)) << 6;
	     else if (dst_region_h > 32768)
	       interp_y[y] = (((((y + dst_clip_y - dst_region_y) *
				 (src_region_h)) << 3) / dst_region_h) -
			      (pos << 3)) << 5;
	     else if (dst_region_h > 16384)
	       interp_y[y] = (((((y + dst_clip_y - dst_region_y) *
				 (src_region_h)) << 4) / dst_region_h) -
			      (pos << 4)) << 4;
	     else if (dst_region_h > 8192)
	       interp_y[y] = (((((y + dst_clip_y - dst_region_y) *
				 (src_region_h)) << 5) / dst_region_h) -
			      (pos << 5)) << 3;
	     else if (dst_region_h > 4096)
	       interp_y[y] = (((((y + dst_clip_y - dst_region_y) *
				 (src_region_h)) << 6) / dst_region_h) -
			      (pos << 6)) << 2;
	     else if (dst_region_h > 2048)
	       interp_y[y] = (((((y + dst_clip_y - dst_region_y) *
				 (src_region_h)) << 7) / dst_region_h) -
			      (pos << 7)) << 1;
	     else
	       interp_y[y] = (((((y + dst_clip_y - dst_region_y) *
				 (src_region_h)) << 8) / dst_region_h) -
			      (pos << 8));
	  }
	else
	  {
	     pos = (((y + dst_clip_y - dst_region_y) *
		     src_region_h) / dst_region_h);
	     row_ptr[y] = src_data + ((pos + src_region_y) * src_w);
	     interp_y[y] = 0;
	  }
     }

   src_end = src_data + (src_w * src_h) - 1;
   func      = evas_common_draw_func_blend_get      (src, dst, dst_clip_w);
   func_cmod = evas_common_draw_func_blend_cmod_get (src, dst, dst_clip_w);
   func_mul  = evas_common_draw_func_blend_mul_get  (src, dc->mul.col, dst, dst_clip_w);
	
   dptr = dst_ptr;
   rp = row_ptr;  iy = interp_y;
   lp = lin_ptr;  ix = interp_x; 
   w = dst_clip_w;

//#undef SCALE_USING_MMX
#ifndef SCALE_USING_MMX
   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	int     k, px, i, pv;
	DATA8   p1r = 0, p1g = 0, p1b = 0, p1a = 0;
	DATA8   p2r = 0, p2g = 0, p2b = 0, p2a = 0;
	DATA32 *lptr1, *lptr2;

	while (dst_clip_h--)
	  {
	     lptr1 = *rp;
	     lptr2 = lptr1 + src_w;
	     if (lptr2 > src_end) lptr2 = lptr1;
	     k = *iy;
	     dst_ptr = buf;

	     lp = lin_ptr;  ix = interp_x;
	     ptr = lptr1 + *lp;
	     ptr2 = lptr2 + *lp;
	     p2r = R_VAL(ptr) + ((k * (R_VAL(ptr2) - R_VAL(ptr))) >> 8);
	     p2g = G_VAL(ptr) + ((k * (G_VAL(ptr2) - G_VAL(ptr))) >> 8);
	     p2b = B_VAL(ptr) + ((k * (B_VAL(ptr2) - B_VAL(ptr))) >> 8);
	     p2a = A_VAL(ptr) + ((k * (A_VAL(ptr2) - A_VAL(ptr))) >> 8);
	     pv = 1;
	     px = -1;
	     while (dst_clip_w--)
	       {
		  if (px != *lp)
		    {
		       px = *lp;

		       ptr = lptr1 + px;
		       ptr2 = ptr + 1;
		       ptr3 = lptr2 + px;
		       ptr4 = ptr3 + 1;
		       if (px >= (src_w - 1))
			 {
			    ptr2 = ptr;
			    ptr4 = ptr3;
			 }

		       if ((*ptr | *ptr2 | *ptr3 | *ptr4) & 0xff000000)
			 {
			    if (pv)
			      {
				 p1r = p2r;
				 p1g = p2g;
				 p1b = p2b;
				 p1a = p2a;
			      }
			    else
			      {
				 p1r = R_VAL(ptr) + ((k * (R_VAL(ptr3) - R_VAL(ptr))) >> 8);
				 p1g = G_VAL(ptr) + ((k * (G_VAL(ptr3) - G_VAL(ptr))) >> 8);
				 p1b = B_VAL(ptr) + ((k * (B_VAL(ptr3) - B_VAL(ptr))) >> 8);
				 p1a = A_VAL(ptr) + ((k * (A_VAL(ptr3) - A_VAL(ptr))) >> 8);
			      }
			    p2r = R_VAL(ptr2) + ((k * (R_VAL(ptr4) - R_VAL(ptr2))) >> 8);
			    p2g = G_VAL(ptr2) + ((k * (G_VAL(ptr4) - G_VAL(ptr2))) >> 8);
			    p2b = B_VAL(ptr2) + ((k * (B_VAL(ptr4) - B_VAL(ptr2))) >> 8);
			    p2a = A_VAL(ptr2) + ((k * (A_VAL(ptr4) - A_VAL(ptr2))) >> 8);
			    pv = 1;
			 }
		       else
			 {
			    pv = 0;
			    p1a = p2a = 0;
			 }
		    }
		  
		  *dst_ptr = 0;
		  if (p1a | p2a)
		    {
		       i = *ix;
		       R_VAL(dst_ptr) = p1r + ((i * (p2r - p1r)) >> 8);
		       G_VAL(dst_ptr) = p1g + ((i * (p2g - p1g)) >> 8);
		       B_VAL(dst_ptr) = p1b + ((i * (p2b - p1b)) >> 8);
		       A_VAL(dst_ptr) = p1a + ((i * (p2a - p1a)) >> 8);
		    }
		  
		  dst_ptr++;  ix++;  lp++;
	       }
	     dst_clip_w = w;  ix = interp_x;  lp = lin_ptr;
	     /* * blend here [clip_w *] buf -> dptr * */
	     if (dc->mod.use)
	       func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
	     else if (dc->mul.use)
	       func_mul(buf, dptr, dst_clip_w, dc->mul.col);
	     else
	       func(buf, dptr, dst_clip_w);
	     dptr += dst_w;  iy++;  rp++;
	  }
     }
   else
     {
	int     k, px, i;
	DATA8   p1r = 0, p1g = 0, p1b = 0;
	DATA8   p2r = 0, p2g = 0, p2b = 0;
	DATA32 *lptr1, *lptr2;

#ifdef DIRECT_SCALE
	if ((!(src->flags & RGBA_IMAGE_HAS_ALPHA)) &&
	    (!(dst->flags & RGBA_IMAGE_HAS_ALPHA)) &&
	    (!dc->mod.use) && (!dc->mul.use))
	  {
	     while (dst_clip_h--)
	       {
		  lptr1 = *rp;
		  lptr2 = lptr1 + src_w;
		  if (lptr2 > src_end) lptr2 = lptr1;
		  k = *iy;
		  dst_ptr = dptr;
		  
		  lp = lin_ptr;  ix = interp_x;
		  ptr = lptr1 + *lp;
		  ptr2 = lptr2 + *lp;
		  p2r = R_VAL(ptr) + ((k * (R_VAL(ptr2) - R_VAL(ptr))) >> 8);
		  p2g = G_VAL(ptr) + ((k * (G_VAL(ptr2) - G_VAL(ptr))) >> 8);
		  p2b = B_VAL(ptr) + ((k * (B_VAL(ptr2) - B_VAL(ptr))) >> 8);
		  px = -1;
		  while (dst_clip_w--)
		    {
		       if (px != *lp)
			 {
			    px = *lp;
			    
			    ptr2 = lptr1 + px + 1;
			    ptr4 = lptr2 + px + 1;
			    if (px >= (src_w - 1))
			      {
				 ptr2--;
				 ptr4--;
			      }
			    p1r = p2r;
			    p1g = p2g;
			    p1b = p2b;
			    p2r = R_VAL(ptr2) + ((k * (R_VAL(ptr4) - R_VAL(ptr2))) >> 8);
			    p2g = G_VAL(ptr2) + ((k * (G_VAL(ptr4) - G_VAL(ptr2))) >> 8);
			    p2b = B_VAL(ptr2) + ((k * (B_VAL(ptr4) - B_VAL(ptr2))) >> 8);
			 }
		       i = *ix;
		       R_VAL(dst_ptr) = p1r + ((i * (p2r - p1r)) >> 8);
		       G_VAL(dst_ptr) = p1g + ((i * (p2g - p1g)) >> 8);
		       B_VAL(dst_ptr) = p1b + ((i * (p2b - p1b)) >> 8);
//		       A_VAL(dst_ptr) = 0xff;
		       
		       dst_ptr++;  ix++;  lp++;
		    }
		  dst_clip_w = w;  ix = interp_x;  lp = lin_ptr;
		  dptr += dst_w;  iy++;  rp++;
	       }
	  }
	else
#endif	  
	  {
	     while (dst_clip_h--)
	       {
		  lptr1 = *rp;
		  lptr2 = lptr1 + src_w;
		  if (lptr2 > src_end) lptr2 = lptr1;
		  k = *iy;
		  dst_ptr = buf;
		  
		  lp = lin_ptr;  ix = interp_x;
		  ptr = lptr1 + *lp;
		  ptr2 = lptr2 + *lp;
		  p2r = R_VAL(ptr) + ((k * (R_VAL(ptr2) - R_VAL(ptr))) >> 8);
		  p2g = G_VAL(ptr) + ((k * (G_VAL(ptr2) - G_VAL(ptr))) >> 8);
		  p2b = B_VAL(ptr) + ((k * (B_VAL(ptr2) - B_VAL(ptr))) >> 8);
		  px = -1;
		  while (dst_clip_w--)
		    {
		       if (px != *lp)
			 {
			    px = *lp;
			    
			    ptr2 = lptr1 + px + 1;
			    ptr4 = lptr2 + px + 1;
			    if (px >= (src_w - 1))
			      {
				 ptr2--;
				 ptr4--;
			      }
			    p1r = p2r;
			    p1g = p2g;
			    p1b = p2b;
			    p2r = R_VAL(ptr2) + ((k * (R_VAL(ptr4) - R_VAL(ptr2))) >> 8);
			    p2g = G_VAL(ptr2) + ((k * (G_VAL(ptr4) - G_VAL(ptr2))) >> 8);
			    p2b = B_VAL(ptr2) + ((k * (B_VAL(ptr4) - B_VAL(ptr2))) >> 8);
			 }
		       i = *ix;
		       R_VAL(dst_ptr) = p1r + ((i * (p2r - p1r)) >> 8);
		       G_VAL(dst_ptr) = p1g + ((i * (p2g - p1g)) >> 8);
		       B_VAL(dst_ptr) = p1b + ((i * (p2b - p1b)) >> 8);
		       A_VAL(dst_ptr) = 0xff;
		       
		       dst_ptr++;  ix++;  lp++;
		    }
		  dst_clip_w = w;  ix = interp_x;  lp = lin_ptr;
		  /* * blend here [clip_w *] buf -> dptr * */
		  if (dc->mod.use)
		    func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
		  else if (dc->mul.use)
		    func_mul(buf, dptr, dst_clip_w, dc->mul.col);
		  else
		    func(buf, dptr, dst_clip_w);
		  dptr += dst_w;  iy++;  rp++;
	       }
	  }
     }
#else
   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	int     k, px, a, i;
	DATA32 *lptr1, *lptr2;

	while (dst_clip_h--)
	  {
	     
	     lptr1 = *rp;
	     lptr2 = lptr1 + src_w;
	     if (lptr2 > src_end) lptr2 = lptr1;
	     k = *iy / 2;
	     dst_ptr = buf;
	     
	     /* mm5 = k */
	     movd_m2r(k, mm5);
	     punpcklwd_r2r(mm5, mm5);
	     punpckldq_r2r(mm5, mm5);
	     
	     px = -1;
	     a = 0;
	     lp = lin_ptr;  ix = interp_x;
	     while (dst_clip_w--)
	       {
		  /* if we have a new pair of horizontal pixels to */
		  /* interpolate between them vertically */
		  if (px != *lp)
		    {
		       px = *lp;
		       
		       ptr = lptr1 + px;
		       ptr2 = ptr + 1;
		       ptr3 = lptr2 + px;
		       ptr4 = ptr3 + 1;
		       if (px >= (src_w - 1))
			 {
			    ptr2 = ptr;
			    ptr4 = ptr3;
			 }
		       
		       a = 0;
		       if ((*ptr | *ptr2 | *ptr3 | *ptr4) & 0xff000000)
			 {
			    a = 1;
			    /* left edge */
			    movd_m2r(*ptr, mm0);
			    pxor_r2r(mm7, mm7);
			    punpcklbw_r2r(mm7, mm0);
			    
			    movd_m2r(*ptr3, mm1);
			    pxor_r2r(mm7, mm7);
			    punpcklbw_r2r(mm7, mm1);
			    psubsw_r2r(mm0, mm1);
			    pmullw_r2r(mm5, mm1);
			    psraw_i2r(7, mm1);
			    
			    paddsw_r2r(mm1, mm0);
			    /* mm0 = left edge */
			    
			    /* right edge */
			    movd_m2r(*ptr2, mm1);
			    pxor_r2r(mm7, mm7);
			    punpcklbw_r2r(mm7, mm1);
			    
			    movd_m2r(*ptr4, mm2);
			    pxor_r2r(mm7, mm7);
			    punpcklbw_r2r(mm7, mm2);
			    psubsw_r2r(mm1, mm2);
			    pmullw_r2r(mm5, mm2);
			    psraw_i2r(7, mm2);
			    
			    paddsw_r2r(mm2, mm1);
			    /* mm1 = right edge */
			 }
		    }
		  
		  if (a)
		    {
		       i = *ix / 2;
		       movq_r2r(mm1, mm4);
		       
		       movd_m2r(i, mm2);
		       punpcklwd_r2r(mm2, mm2);
		       punpckldq_r2r(mm2, mm2);
		       
		       movq_r2r(mm0, mm3);
		       
		       psubsw_r2r(mm3, mm4);
		       pmullw_r2r(mm2, mm4);
		       psraw_i2r(7, mm4);
		       paddsw_r2r(mm4, mm3);
		       
		       packuswb_r2r(mm3, mm3);
		       /* blend mm3... */
		       movd_r2m(mm3, *dst_ptr);
		    }
		  else
		    *dst_ptr = 0;
		  
		  dst_ptr++;  ix++;  lp++;
	       }
	     /* * blend here [clip_w *] buf -> dptr * */
	     dst_clip_w = w;  ix = interp_x;  lp = lin_ptr;
	     if (dc->mod.use)
	       func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
	     else if (dc->mul.use)
	       func_mul(buf, dptr, dst_clip_w, dc->mul.col);
	     else
	       func(buf, dptr, dst_clip_w);
	     dptr += dst_w;  iy++;  rp++;
	  }
     }
   else
     {
	int     k, px, i;
	DATA32 *lptr1, *lptr2;

#ifdef DIRECT_SCALE
	if ((!(src->flags & RGBA_IMAGE_HAS_ALPHA)) &&
	    (!(dst->flags & RGBA_IMAGE_HAS_ALPHA)) &&
	    (!dc->mod.use) && (!dc->mul.use))
	  {
	     while (dst_clip_h--)
	       {
		  lptr1 = *rp;
		  lptr2 = lptr1 + src_w;
		  if (lptr2 > src_end) lptr2 = lptr1;
		  k = *iy / 2;
		  dst_ptr = dptr;
		  
		  /* mm5 = k */
		  movd_m2r(k, mm5);
		  punpcklwd_r2r(mm5, mm5);
		  punpckldq_r2r(mm5, mm5);
		  
		  lp = lin_ptr;  ix = interp_x;
		  ptr = lptr1 + *lp;
		  ptr2 = lptr2 + *lp;
		  /* right edge */
		  movd_m2r(*ptr, mm1);
		  pxor_r2r(mm7, mm7);
		  punpcklbw_r2r(mm7, mm1);
		  
		  movd_m2r(*ptr2, mm2);
		  pxor_r2r(mm7, mm7);
		  punpcklbw_r2r(mm7, mm2);
		  
		  psubsw_r2r(mm1, mm2);
		  pmullw_r2r(mm5, mm2);
		  psraw_i2r(7, mm2);
		  
		  paddsw_r2r(mm2, mm1);
		  /* mm1 = right edge */
		  
		  px = -1;
		  while (dst_clip_w--)
		    {
		       /* if we have a new pair of horizontal pixels to */
		       /* interpolate between them vertically */
		       if (px != *lp)
			 {
			    px = *lp;
			    
			    ptr2 = lptr1 + px + 1;
			    ptr4 = lptr2 + px + 1;
			    if (px >= (src_w - 1))
			      {
				 ptr2--;
				 ptr4--;
			      }
			    movq_r2r(mm1, mm0);
			    
			    /* right edge */
			    movd_m2r(*ptr2, mm1);
			    pxor_r2r(mm7, mm7);
			    punpcklbw_r2r(mm7, mm1);
			    
			    movd_m2r(*ptr4, mm2);
			    pxor_r2r(mm7, mm7);
			    punpcklbw_r2r(mm7, mm2);
			    
			    psubsw_r2r(mm1, mm2);
			    pmullw_r2r(mm5, mm2);
			    psraw_i2r(7, mm2);
			    
			    paddsw_r2r(mm2, mm1);
			    /* mm1 = right edge */
			 }
		       
		       i = *ix / 2;
		       
		       movq_r2r(mm1, mm4);
		       
		       movd_m2r(i, mm2);
		       punpcklwd_r2r(mm2, mm2);
		       punpckldq_r2r(mm2, mm2);
		       
		       movq_r2r(mm0, mm3);
		       
		       psubsw_r2r(mm3, mm4);
		       pmullw_r2r(mm2, mm4);
		       psraw_i2r(7, mm4);
		       paddsw_r2r(mm4, mm3);
		       
		       packuswb_r2r(mm3, mm3);
		       /* blend mm3... */
		       movd_r2m(mm3, *dst_ptr);
		       
		       dst_ptr++;  ix++;  lp++;
		    }
		  /* * blend here [clip_w *] buf -> dptr * */
		  dst_clip_w = w;  ix = interp_x;  lp = lin_ptr;
		  dptr += dst_w;  iy++;  rp++;
	       }
	  }
	else
#endif	  
	  {
	     while (dst_clip_h--)
	       {
		  lptr1 = *rp;
		  lptr2 = lptr1 + src_w;
		  if (lptr2 > src_end) lptr2 = lptr1;
		  k = *iy / 2;
		  dst_ptr = buf;
		  
		  /* mm5 = k */
		  movd_m2r(k, mm5);
		  punpcklwd_r2r(mm5, mm5);
		  punpckldq_r2r(mm5, mm5);
		  
		  lp = lin_ptr;  ix = interp_x;
		  ptr = lptr1 + *lp;
		  ptr2 = lptr2 + *lp;
		  /* right edge */
		  movd_m2r(*ptr, mm1);
		  pxor_r2r(mm7, mm7);
		  punpcklbw_r2r(mm7, mm1);
		  
		  movd_m2r(*ptr2, mm2);
		  pxor_r2r(mm7, mm7);
		  punpcklbw_r2r(mm7, mm2);
		  
		  psubsw_r2r(mm1, mm2);
		  pmullw_r2r(mm5, mm2);
		  psraw_i2r(7, mm2);
		  
		  paddsw_r2r(mm2, mm1);
		  /* mm1 = right edge */
		  
		  px = -1;
		  while (dst_clip_w--)
		    {
		       /* if we have a new pair of horizontal pixels to */
		       /* interpolate between them vertically */
		       if (px != *lp)
			 {
			    px = *lp;
			    
			    ptr2 = lptr1 + px + 1;
			    ptr4 = lptr2 + px + 1;
			    if (px >= (src_w - 1))
			      {
				 ptr2--;
				 ptr4--;
			      }
			    movq_r2r(mm1, mm0);
			    
			    /* right edge */
			    movd_m2r(*ptr2, mm1);
			    pxor_r2r(mm7, mm7);
			    punpcklbw_r2r(mm7, mm1);
			    
			    movd_m2r(*ptr4, mm2);
			    pxor_r2r(mm7, mm7);
			    punpcklbw_r2r(mm7, mm2);
			    
			    psubsw_r2r(mm1, mm2);
			    pmullw_r2r(mm5, mm2);
			    psraw_i2r(7, mm2);
			    
			    paddsw_r2r(mm2, mm1);
			    /* mm1 = right edge */
			 }
		       
		       i = *ix / 2;
		       
		       movq_r2r(mm1, mm4);
		       
		       movd_m2r(i, mm2);
		       punpcklwd_r2r(mm2, mm2);
		       punpckldq_r2r(mm2, mm2);
		       
		       movq_r2r(mm0, mm3);
		       
		       psubsw_r2r(mm3, mm4);
		       pmullw_r2r(mm2, mm4);
		       psraw_i2r(7, mm4);
		       paddsw_r2r(mm4, mm3);
		       
		       packuswb_r2r(mm3, mm3);
		       /* blend mm3... */
		       movd_r2m(mm3, *dst_ptr);
		       
		       dst_ptr++;  ix++;  lp++;
		    }
		  /* * blend here [clip_w *] buf -> dptr * */
		  dst_clip_w = w;  ix = interp_x;  lp = lin_ptr;
		  if (dc->mod.use)
		    func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
		  else if (dc->mul.use)
		    func_mul(buf, dptr, dst_clip_w, dc->mul.col);
		  else
		    func(buf, dptr, dst_clip_w);
		  dptr += dst_w;  iy++;  rp++;
	       }
	  }
     }
#endif

   done_scale_up:
   evas_common_image_line_buffer_release();
   if (interp_y) free(interp_y);
   if (interp_x) free(interp_x);
   if (row_ptr) free(row_ptr);
   if (lin_ptr) free(lin_ptr);
}
