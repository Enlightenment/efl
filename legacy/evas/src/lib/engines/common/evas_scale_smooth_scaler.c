void
SCALE_FUNC(RGBA_Image *src, RGBA_Image *dst, 
	   RGBA_Draw_Context *dc,
	   int src_region_x, int src_region_y, 
	   int src_region_w, int src_region_h, 
	   int dst_region_x, int dst_region_y, 
	   int dst_region_w, int dst_region_h)
{
   int      x, y;
   int     *lin_ptr, *lin2_ptr;
   int     *interp_x, *interp_y;
   int     *sample_x, *sample_y;
   char    *iterate_x, *iterate_y;
   DATA32  *buf, *dptr;
   DATA32 **row_ptr, **row2_ptr;
   DATA32  *ptr, *dst_ptr, *dst_data, *ptr2, *ptr3, *ptr4;
   int      dst_jump;
   int      dst_clip_x, dst_clip_y, dst_clip_w, dst_clip_h;
   int      src_w, src_h, dst_w, dst_h; 

//   printf("in [0] dst_region_w %i\n", dst_region_w);
   
   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->image->w, dst->image->h)))
     return;
   if (!(RECTS_INTERSECT(src_region_x, src_region_y, src_region_w, src_region_h, 0, 0, src->image->w, src->image->h)))
     return;

//   printf("in [1] dst_region_w %i\n", dst_region_w);
   src_w = src->image->w;
   src_h = src->image->h;
   dst_w = dst->image->w;
   dst_h = dst->image->h;
   
   dst_data = dst->image->data;
   
   if (dc->clip.use)
     {
	dst_clip_x = dc->clip.x;
	dst_clip_y = dc->clip.y;
	dst_clip_w = dc->clip.w;
	dst_clip_h = dc->clip.h;
	if (dst_clip_x < 0)
	  {
	     dst_clip_w += dst_clip_x;
	     dst_clip_x = 0;
	  }
	if (dst_clip_y < 0)
	  {
	     dst_clip_h += dst_clip_y;
	     dst_clip_y = 0;
	  }
	if ((dst_clip_w <= 0) || (dst_clip_h <= 0)) return;
	if ((dst_clip_x + dst_clip_w) > dst_w) dst_clip_w = dst_w - dst_clip_x;
	if ((dst_clip_y + dst_clip_h) > dst_h) dst_clip_h = dst_h - dst_clip_y;
     }
   else
     {
	dst_clip_x = 0;
	dst_clip_y = 0;
	dst_clip_w = dst_w;
	dst_clip_h = dst_h;
     }
//   printf("in [2] dst_region_w %i\n", dst_region_w);
   
   if (dst_clip_x < dst_region_x)
     {
	dst_clip_w += dst_clip_x - dst_region_x;
	dst_clip_x = dst_region_x;
     }
   if ((dst_clip_x + dst_clip_w) > (dst_region_x + dst_region_w))
     dst_clip_w = dst_region_x + dst_region_w - dst_clip_x;
   if (dst_clip_y < dst_region_y)
     {
	dst_clip_h += dst_clip_y - dst_region_y;
	dst_clip_y = dst_region_y;
     }
   if ((dst_clip_y + dst_clip_h) > (dst_region_y + dst_region_h))
     dst_clip_h = dst_region_y + dst_region_h - dst_clip_y;
   
   if ((src_region_w <= 0) || (src_region_h <= 0) ||
       (dst_region_w <= 0) || (dst_region_h <= 0) ||
       (dst_clip_w <= 0) || (dst_clip_h <= 0))
     return;
//   printf("in [3] dst_region_w %i\n", dst_region_w);

   /* sanitise x */
   if (src_region_x < 0) 
     {
	dst_region_x -= (src_region_x * dst_region_w) / src_region_w;
	dst_region_w += (src_region_x * dst_region_w) / src_region_w;
	src_region_w += src_region_x;
	src_region_x = 0;
     }
//   printf("in [4] dst_region_w %i\n", dst_region_w);
   if (src_region_x >= src_w) return;
   if ((src_region_x + src_region_w) > src_w)
     {
	dst_region_w = (dst_region_w * (src_w - src_region_x)) / (src_region_w);
	src_region_w = src_w - src_region_x;
     }
//   printf("in [5] dst_region_w %i\n", dst_region_w);
   if (dst_region_w <= 0) return;
   if (src_region_w <= 0) return;
   if (dst_clip_x < 0) 
     {
	dst_clip_w += dst_clip_x;
	dst_clip_x = 0;	
     }
   if (dst_clip_w <= 0) return;
   if (dst_clip_x >= dst_w) return;
   if (dst_clip_x < dst_region_x) 
     {
	dst_clip_w += (dst_clip_x - dst_region_x);
	dst_clip_x = dst_region_x;
     }
   if ((dst_clip_x + dst_clip_w) > dst_w) 
     {
	dst_clip_w = dst_w - dst_clip_x;
     }
   if (dst_clip_w <= 0) return;
//   printf("in [6] dst_region_w %i\n", dst_region_w);
   
   /* sanitise y */
   if (src_region_y < 0) 
     {
	dst_region_y -= (src_region_y * dst_region_h) / src_region_h;
	dst_region_h += (src_region_y * dst_region_h) / src_region_h;
	src_region_h += src_region_y;
	src_region_y = 0;
     }
   if (src_region_y >= src_h) return;
   if ((src_region_y + src_region_h) > src_h)
     {
	dst_region_h = (dst_region_h * (src_h - src_region_y)) / (src_region_h);
	src_region_h = src_h - src_region_y;
     }
   if (dst_region_h <= 0) return;
   if (src_region_h <= 0) return;
   if (dst_clip_y < 0) 
     {
	dst_clip_h += dst_clip_y;
	dst_clip_y = 0;	
     }
   if (dst_clip_h <= 0) return;
   if (dst_clip_y >= dst_h) return;
   if (dst_clip_y < dst_region_y) 
     {
	dst_clip_h += (dst_clip_y - dst_region_y);
	dst_clip_y = dst_region_y;
     }
   if ((dst_clip_y + dst_clip_h) > dst_h) 
     {
	dst_clip_h = dst_h - dst_clip_y;
     }
   if (dst_clip_h <= 0) return;

   lin_ptr = malloc(dst_clip_w * sizeof(int));
   if (!lin_ptr) goto no_lin_ptr;
   row_ptr = malloc(dst_clip_h * sizeof(DATA32 *));
   if (!row_ptr) goto no_row_ptr;
   lin2_ptr = malloc(dst_clip_w * sizeof(int));
   if (!lin2_ptr) goto no_lin2_ptr;
   row2_ptr = malloc(dst_clip_h * sizeof(DATA32 *));
   if (!row2_ptr) goto no_row2_ptr;
   interp_x = malloc(dst_clip_w * sizeof(int));
   if (!interp_x) goto no_interp_x;
   interp_y = malloc(dst_clip_h * sizeof(int));
   if (!interp_y) goto no_interp_y;
   sample_x = malloc(dst_clip_w * sizeof(int) * 3);
   if (!sample_x) goto no_sample_x;
   sample_y = malloc(dst_clip_h * sizeof(int) * 3);
   if (!sample_y) goto no_sample_y;
   iterate_x = malloc(dst_clip_w * sizeof(char));
   if (!iterate_x) goto no_iterate_x;
   iterate_y = malloc(dst_clip_h * sizeof(char));
   if (!iterate_y) goto no_iterate_y;

   /* figure out dst jump */
   dst_jump = dst_w - dst_clip_w;
   
   /* figure out dest start ptr */
   dst_ptr = dst_data + dst_clip_x + (dst_clip_y * dst_w);

/* FIXME:
 * 
 * things to do later for speedups:
 * 
 * break upscale into 3 cases (as listed below - up:up, 1:up, up:1)
 * 
 * break downscale into more cases (as listed below)
 * 
 * roll func (blend/copy/cultiply/cmod) code into inner loop of scaler.
 * (578 fps vs 550 in mmx upscale in evas demo - this means probably
 *  a good 10-15% speedup over the func call, but means massively larger
 *  code)
 * 
 * anything involving downscaling has no mmx equivalent code and maybe the
 * C could do with a little work.
 * 
 * ---------------------------------------------------------------------------
 * 
 * (1 = no scaling (1:1 ratio), + = scale up, - = scale down)
 * (* == fully optimised mmx, # = fully optimised C)
 *
 * h:v mmx C
 *     
 * 1:1 *   #
 * 
 * +:+ *   #
 * 1:+ *   #
 * +:1 *   #
 * 
 * 1:- 
 * -:1
 * +:- 
 * -:+ 
 * -:- 
 *
 */
   
   /* if 1:1 scale */
   if ((dst_region_w == src_region_w) && 
       (dst_region_h == src_region_h))
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
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  func(ptr, dst_ptr, dst_clip_w);
		  ptr += src_w;
		  dst_ptr += dst_w;
	       }
	  }
     }
   else
     {
	Gfx_Func_Blend_Src_Cmod_Dst func_cmod;
	Gfx_Func_Blend_Src_Mul_Dst  func_mul;
	Gfx_Func_Blend_Src_Dst      func;
	
	/* a scanline buffer */
	buf = malloc(dst_clip_w * sizeof(DATA32));
	if (!buf) goto no_buf;
	
	func      = evas_common_draw_func_blend_get      (src, dst, dst_clip_w);
	func_cmod = evas_common_draw_func_blend_cmod_get (src, dst, dst_clip_w);
	func_mul  = evas_common_draw_func_blend_mul_get  (src, dc->mul.col, dst, dst_clip_w);
	
	/* scaling up only - dont need anything except original */
	if ((dst_region_w >= src_region_w) && (dst_region_h >= src_region_h))
	  {
	     DATA32 *src_data;
	     
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
	     
	     dptr = dst_ptr;
	     
#ifndef SCALE_USING_MMX
	     if (src->flags & RGBA_IMAGE_HAS_ALPHA)
	       {
		  for (y = 0; y < dst_clip_h; y++)
		    {
		       int i, j, k, l, px;
		       DATA8 p1r, p1g, p1b, p1a;
		       DATA8 p2r, p2g, p2b, p2a;
		       DATA32 *lptr1, *lptr2;

		       p1r = p1g = p1b = p1a = 0;
		       p2r = p2g = p2b = p2a = 0;
		       
		       lptr1 = row_ptr[y];
		       if ((lptr1 + src_w) >= 
			   (src->image->data + 
			    (src->image->w * 
			     src->image->h)))
			 lptr2 = lptr1;
		       else
			 lptr2 = lptr1 + src_w;
		       
		       k = interp_y[y];
		       l = 256 - k;
		       
		       dst_ptr = buf;
		       
		       px = -1;
		       
		       for (x = 0; x < dst_clip_w; x++)
			 {
			    i = interp_x[x];
			    j = 256 - i;
			    
			    if (px != lin_ptr[x])
			      {
				 px = lin_ptr[x];
			    
				 ptr = lptr1 + lin_ptr[x];
				 ptr3 = lptr2 + lin_ptr[x];
				 if (lin_ptr[x] >= (src->image->w - 1))
				   {
				      ptr2 = ptr;
				      ptr4 = ptr3;
				   }
				 else
				   {
				      ptr2 = ptr + 1;
				      ptr4 = ptr3 + 1;
				   }
			
				 if (A_VAL(ptr) |
				     A_VAL(ptr2) |
				     A_VAL(ptr3) |
				     A_VAL(ptr4))
				   {
				      p1r = INTERP_2(R_VAL(ptr), R_VAL(ptr3), k, l);
				      p1g = INTERP_2(G_VAL(ptr), G_VAL(ptr3), k, l);
				      p1b = INTERP_2(B_VAL(ptr), B_VAL(ptr3), k, l);
				      p1a = INTERP_2(A_VAL(ptr), A_VAL(ptr3), k, l);
				      p2r = INTERP_2(R_VAL(ptr2), R_VAL(ptr4), k, l);
				      p2g = INTERP_2(G_VAL(ptr2), G_VAL(ptr4), k, l);
				      p2b = INTERP_2(B_VAL(ptr2), B_VAL(ptr4), k, l);
				      p2a = INTERP_2(A_VAL(ptr2), A_VAL(ptr4), k, l);
				   }
				 else
				   {
				      p1a = 0;
				      p2a = 0;
				   }
			      }
			    if (p1a | p2a)
			      {
				 R_VAL(dst_ptr) = INTERP_2(p1r, p2r, i, j);
				 G_VAL(dst_ptr) = INTERP_2(p1g, p2g, i, j);
				 B_VAL(dst_ptr) = INTERP_2(p1b, p2b, i, j);
				 A_VAL(dst_ptr) = INTERP_2(p1a, p2a, i, j);
			      }
			    else
			      A_VAL(dst_ptr) = 0;
			    
			    dst_ptr++;
			 }
		       /* * blend here [clip_w *] buf -> dptr * */
		       if (dc->mod.use)
			 func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
		       else if (dc->mul.use)
			 func_mul(buf, dptr, dst_clip_w, dc->mul.col);
		       else
			 func(buf, dptr, dst_clip_w);
		       dptr += dst_w;
		    }
	       }
	     else
	       {
		  for (y = 0; y < dst_clip_h; y++)
		    {
		       int i, j, k, l, px;
		       DATA8 p1r, p1g, p1b;
		       DATA8 p2r, p2g, p2b;
		       DATA32 *lptr1, *lptr2;

		       p1r = p1g = p1b = 0;
		       p2r = p2g = p2b = 0;
		       
		       lptr1 = row_ptr[y];
		       if ((lptr1 + src_w) >= 
			   (src->image->data + 
			    (src->image->w * 
			     src->image->h)))
			 lptr2 = lptr1;
		       else
			 lptr2 = lptr1 + src_w;
		       
		       k = interp_y[y];
		       l = 256 - k;
		       
		       dst_ptr = buf;
		       
		       px = -1;
		       
		       for (x = 0; x < dst_clip_w; x++)
			 {
			    i = interp_x[x];
			    j = 256 - i;
			    
			    if (px != lin_ptr[x])
			      {
				 px = lin_ptr[x];
			    
				 ptr = lptr1 + lin_ptr[x];
				 ptr3 = lptr2 + lin_ptr[x];
				 if (lin_ptr[x] >= (src->image->w - 1))
				   {
				      ptr2 = ptr;
				      ptr4 = ptr3;
				   }
				 else
				   {
				      ptr2 = ptr + 1;
				      ptr4 = ptr3 + 1;
				   }
				 
				 p1r = INTERP_2(R_VAL(ptr), R_VAL(ptr3), k, l);
				 p1g = INTERP_2(G_VAL(ptr), G_VAL(ptr3), k, l);
				 p1b = INTERP_2(B_VAL(ptr), B_VAL(ptr3), k, l);
				 p2r = INTERP_2(R_VAL(ptr2), R_VAL(ptr4), k, l);
				 p2g = INTERP_2(G_VAL(ptr2), G_VAL(ptr4), k, l);
				 p2b = INTERP_2(B_VAL(ptr2), B_VAL(ptr4), k, l);
			      }
			    R_VAL(dst_ptr) = INTERP_2(p1r, p2r, i, j);
			    G_VAL(dst_ptr) = INTERP_2(p1g, p2g, i, j);
			    B_VAL(dst_ptr) = INTERP_2(p1b, p2b, i, j);
			    A_VAL(dst_ptr) = 0xff;
			    
			    dst_ptr++;
			 }
		       /* * blend here [clip_w *] buf -> dptr * */
		       if (dc->mod.use)
			 func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
		       else if (dc->mul.use)
			 func_mul(buf, dptr, dst_clip_w, dc->mul.col);
		       else
			 func(buf, dptr, dst_clip_w);
		       dptr += dst_w;
		    }
	       }
#else
	     pxor_r2r(mm7, mm7);
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  DATA32 *lptr1, *lptr2;
		  int i, j, k, l, px;
		  
		  lptr1 = row_ptr[y];
		  if ((lptr1 + src_w) >= 
		      (src->image->data + 
		       (src->image->w * 
			src->image->h)))
		    lptr2 = lptr1;
		  else
		    lptr2 = lptr1 + src_w;
		  
		  k = interp_y[y];
		  l = 256 - k;
		  
		  dst_ptr = buf;
		  
		  /* mm5 = k */
		  /* mm6 = l */
		  movd_m2r(k, mm5);
		  punpcklwd_r2r(mm5, mm5);
		  punpckldq_r2r(mm5, mm5);
		  movd_m2r(l, mm6);
		  punpcklwd_r2r(mm6, mm6);
		  punpckldq_r2r(mm6, mm6);
		  
		  px = -1;
		  
		  for (x = 0; x < dst_clip_w; x++)
		    {
		       i = interp_x[x];
		       j = 256 - i;
		       
		       /* if we have a new pair of horizontal pixels to */
		       /* interpolate between them vertically */
		       if (px != lin_ptr[x]) 
			 {
			    px = lin_ptr[x];
			    
			    ptr = lptr1 + lin_ptr[x];
			    ptr3 = lptr2 + lin_ptr[x];
			    if (lin_ptr[x] >= (src->image->w - 1))
			      {
				 ptr2 = ptr;
				 ptr4 = ptr3;
			      }
			    else
			      {
				 ptr2 = ptr + 1;
				 ptr4 = ptr3 + 1;
			      }
		       
			    /* left edge */
			    movd_m2r(ptr[0], mm0);
			    punpcklbw_r2r(mm7, mm0);
			    pmullw_r2r(mm6, mm0);

			    movd_m2r(ptr3[0], mm1);
			    punpcklbw_r2r(mm7, mm1);
			    pmullw_r2r(mm5, mm1);

			    paddw_r2r(mm1, mm0);
			    psrlw_i2r(8, mm0);
			    /* mm0 = left edge */
			    
			    /* right edge */
			    movd_m2r(ptr2[0], mm1);
			    punpcklbw_r2r(mm7, mm1);
			    pmullw_r2r(mm6, mm1);
			    
			    movd_m2r(ptr4[0], mm2);
			    punpcklbw_r2r(mm7, mm2);
			    pmullw_r2r(mm5, mm2);

			    paddw_r2r(mm2, mm1);
			    psrlw_i2r(8, mm1);
			    /* mm1 = right edge */
			 }
		       
		       movq_r2r(mm1, mm4);
		       
		       movd_m2r(i, mm2);
		       punpcklwd_r2r(mm2, mm2);
		       punpckldq_r2r(mm2, mm2);
		       pmullw_r2r(mm2, mm4);
		       
		       movq_r2r(mm0, mm3);
		       
		       movd_m2r(j, mm2);
		       punpcklwd_r2r(mm2, mm2);
		       punpckldq_r2r(mm2, mm2);
		       pmullw_r2r(mm2, mm3);
		       
		       paddw_r2r(mm4, mm3);
		       psrlw_i2r(8, mm3);
		       
		       packuswb_r2r(mm3, mm3);
		       movd_r2m(mm3, dst_ptr[0]);
		       
		       dst_ptr++;
		    }
		  /* * blend here [clip_w *] buf -> dptr * */
		  if (dc->mod.use)
		    func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
		  else if (dc->mul.use)
		    func_mul(buf, dptr, dst_clip_w, dc->mul.col);
		  else
		    func(buf, dptr, dst_clip_w);
		  dptr += dst_w;
	       }
#endif	     
	  }
	else
#ifdef BUILD_SCALE_TRILINEAR
	  /* scaling down... funkiness */
	  {
	     DATA32       *src_data, *src2_data;	
	     RGBA_Surface *srf1, *srf2;
	     int           mix;
	     int           divx, divy;
	     
	     /* no mipmaps at all- need to populate mipmap table */
	     if (src->mipmaps.num == 0)
	       evas_common_scale_rgba_gen_mipmap_table(src);
	     /* caclulate mix level between mipmaps */
	     mix = evas_common_scale_rgba_get_mipmap_surfaces(src, 
						  src_region_w, src_region_h, 
						  dst_region_w, dst_region_h, 
						  &srf1, &srf2);
	     /* get 2 mipmap levels */
	     src_data = srf1->data; /* high res surface (sample) */
	     src2_data = srf2->data; /* lower res surface (interp) */
	     /* fill scale + interp tables */
	     /* lin & row ptr for lowest mipmap (hi res) */
	     divx = src->image->w / srf1->w;
	     divy = src->image->h / srf1->h;
	     
	     /* merely sampled pixels in the hi-res mipmap */
	     for (x = 0; x < dst_clip_w; x++)
	       {
		  lin_ptr[x] = (((x + dst_clip_x - dst_region_x) * src_region_w) / (dst_region_w * divx)) + src_region_x;
	       }
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  row_ptr[y] = src_data + (((((y + dst_clip_y - dst_region_y) * src_region_h) / (dst_region_h * divy)) + src_region_y) * srf1->w);
	       }
	     
	     /* lin & row ptr for low-res mipmap */
	     divx = src->image->w / srf2->w;
	     divy = src->image->h / srf2->h;
	     
	     for (x = 0; x < dst_clip_w; x++)
	       {
		  if (src_region_w > 1)
		    {
		       lin2_ptr[x] = (((x + dst_clip_x - dst_region_x) * 
				       (src_region_w - 1)) / dst_region_w);
		       lin2_ptr[x] /= divx;
		       interp_x[x] = (((((x + dst_clip_x - dst_region_x) * 
					 (src_region_w - 1)) << 8) / dst_region_w) / divx) -
			 (lin2_ptr[x] << 8);
		       lin2_ptr[x] += src_region_x;
		    }
		  else
		    {
		       lin2_ptr[x] = (((x + dst_clip_x - dst_region_x) * 
				       src_region_w) / dst_region_w);
		       lin2_ptr[x] /= divx;
		       interp_x[x] = 0;
		       lin2_ptr[x] += src_region_x;
		    }
	       }
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  int pos;
		  
		  if (src_region_h > 1)
		    {
		       pos = (((y + dst_clip_y - dst_region_y) * 
			       (src_region_h - 1)) / dst_region_h);
		       pos /= divy;
		       row2_ptr[y] = src2_data + ((pos + src_region_y) * srf2->w);   
		       interp_y[y] = (((((y + dst_clip_y - dst_region_y) * 
					 (src_region_h - 1)) << 8) / dst_region_h) / divy) -
			 (pos << 8);
		    }
		  else
		    {
		       pos = (((y + dst_clip_y - dst_region_y) * 
			       src_region_h) / dst_region_h);
		       pos /= divy;
		       row2_ptr[y] = src2_data + ((pos + src_region_y) * srf2->w);
		       interp_y[y] = 0;
		    }
	       }
	     /* lin2 & row2 ptr for higher mipmap - need interp too */
	     /* ... */
	     /* scale to dst */
	     dptr = dst_ptr;
	     
	     if (src->flags & RGBA_IMAGE_HAS_ALPHA)
	       {
		  for (y = 0; y < dst_clip_h; y++)
		    {
		       dst_ptr = buf;
		       for (x = 0; x < dst_clip_w; x++)
			 {
			    DATA32 hi_col;
			    DATA32 lo_col;
			    
			    /* lowest (higest res) mipmap level - just pick it */
			    ptr = row_ptr[y] + lin_ptr[x];
			    hi_col = *ptr;
			    /* higer mipmap level - need to use interp */
			    /* lowest (higest res) mipmap level - just pick it */
			    ptr = row2_ptr[y] + lin2_ptr[x];
			    ptr2 = row2_ptr[y] + lin2_ptr[x] + 1;
			    ptr3 = row2_ptr[y] + lin2_ptr[x] + srf2->w;
			    ptr4 = row2_ptr[y] + lin2_ptr[x] + srf2->w + 1;
			    
			    /* higer mipmap level - need to use interp */
			    INTERP_VAL(A_VAL(&lo_col), 
				       A_VAL(ptr), A_VAL(ptr2), 
				       A_VAL(ptr3), A_VAL(ptr4), 
				       interp_x[x], interp_y[y]);
			    if (A_VAL(&hi_col) | A_VAL(&lo_col))
			      {
				 INTERP_VAL(R_VAL(&lo_col), 
					    R_VAL(ptr), R_VAL(ptr2), 
					    R_VAL(ptr3), R_VAL(ptr4), 
					    interp_x[x], interp_y[y]);
				 INTERP_VAL(G_VAL(&lo_col), 
					    G_VAL(ptr), G_VAL(ptr2), 
					    G_VAL(ptr3), G_VAL(ptr4), 
					    interp_x[x], interp_y[y]);
				 INTERP_VAL(B_VAL(&lo_col), 
					    B_VAL(ptr), B_VAL(ptr2), 
					    B_VAL(ptr3), B_VAL(ptr4), 
					    interp_x[x], interp_y[y]);
				 
				 /* blend pixel 1 and 2, mix & 255 - mix each */
				 R_VAL(dst_ptr) = ((R_VAL(&hi_col) * mix) + (R_VAL(&lo_col) * (256 - mix))) >> 8;
				 G_VAL(dst_ptr) = ((G_VAL(&hi_col) * mix) + (G_VAL(&lo_col) * (256 - mix))) >> 8;
				 B_VAL(dst_ptr) = ((B_VAL(&hi_col) * mix) + (B_VAL(&lo_col) * (256 - mix))) >> 8;
				 A_VAL(dst_ptr) = ((A_VAL(&hi_col) * mix) + (A_VAL(&lo_col) * (256 - mix))) >> 8;
			      }
			    else
			      {
				 A_VAL(dst_ptr) = 0;
			      }
			    dst_ptr++;
			 }
		       /* * blend here [clip_w *] buf -> dptr * */
		       if (dc->mod.use)
			 {
			    func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
			 }
		       else if (dc->mul.use)
			 {
			    func_mul(buf, dptr, dst_clip_w, dc->mul.col);
			 }
		       else
			 {
			    func(buf, dptr, dst_clip_w);
			 }
		       dptr += dst_w;
		    }
	       }
	     else
	       {
		  for (y = 0; y < dst_clip_h; y++)
		    {
		       dst_ptr = buf;
		       for (x = 0; x < dst_clip_w; x++)
			 {
			    DATA32 hi_col;
			    DATA32 lo_col;
			    
			    /* lowest (higest res) mipmap level - just pick it */
			    ptr = row_ptr[y] + lin_ptr[x];
			    hi_col = *ptr;
			    /* higer mipmap level - need to use interp */
			    /* lowest (higest res) mipmap level - just pick it */
			    ptr = row2_ptr[y] + lin2_ptr[x];
			    ptr2 = row2_ptr[y] + lin2_ptr[x] + 1;
			    ptr3 = row2_ptr[y] + lin2_ptr[x] + srf2->w;
			    ptr4 = row2_ptr[y] + lin2_ptr[x] + srf2->w + 1;
			    
			    /* higer mipmap level - need to use interp */
			    INTERP_VAL(R_VAL(&lo_col), 
				       R_VAL(ptr), R_VAL(ptr2), 
				       R_VAL(ptr3), R_VAL(ptr4), 
				       interp_x[x], interp_y[y]);
			    INTERP_VAL(G_VAL(&lo_col), 
				       G_VAL(ptr), G_VAL(ptr2), 
				       G_VAL(ptr3), G_VAL(ptr4), 
				       interp_x[x], interp_y[y]);
			    INTERP_VAL(B_VAL(&lo_col), 
				       B_VAL(ptr), B_VAL(ptr2), 
				       B_VAL(ptr3), B_VAL(ptr4), 
				       interp_x[x], interp_y[y]);
			    
			    /* blend pixel 1 and 2, mix & 255 - mix each */
			    R_VAL(dst_ptr) = ((R_VAL(&hi_col) * mix) + (R_VAL(&lo_col) * (256 - mix))) >> 8;
			    G_VAL(dst_ptr) = ((G_VAL(&hi_col) * mix) + (G_VAL(&lo_col) * (256 - mix))) >> 8;
			    B_VAL(dst_ptr) = ((B_VAL(&hi_col) * mix) + (B_VAL(&lo_col) * (256 - mix))) >> 8;
			    A_VAL(dst_ptr) = 0xff;
			    dst_ptr++;
			 }
		       /* * blend here [clip_w *] buf -> dptr * */
		       if (dc->mod.use)
			 {
			    func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
			 }
		       else if (dc->mul.use)
			 {
			    func_mul(buf, dptr, dst_clip_w, dc->mul.col);
			 }
		       else
			 {
			    func(buf, dptr, dst_clip_w);
			 }
		       dptr += dst_w;
		    }
	       }
	  }
#else
	  /* scaling down... funkiness */
	  {
#if 1
#define RGBA_COMPOSE(r, g, b, a)  ((a) << 24) | ((r) << 16) | ((g) << 8) | (b)
#define INV_XAP                   (256 - xapoints[dxx + x])
#define XAP                       (xapoints[dxx + x])
#define INV_YAP                   (256 - yapoints[dyy + y])
#define YAP                       (yapoints[dyy + y])
	     /* NEW scaling code... */
	     DATA32 *sptr;
	     int x, y;
	     DATA32 **ypoints;
	     int *xpoints;
	     int *xapoints;
	     int *yapoints;
	     DATA32 *src_data;
	     
	     src_data = src->image->data;

	     xpoints = scale_calc_x_points(src_region_w, dst_region_w);
	     ypoints = scale_calc_y_points(src_data, src->image->w, src_region_h, dst_region_h);
	     xapoints = scale_calc_a_points(src_region_w, dst_region_w);
	     yapoints = scale_calc_a_points(src_region_h, dst_region_h);
	     /* scaling down vertically */
	     if ((dst_region_w >= src_region_w) && 
		 (dst_region_h <  src_region_h))
	       {
		  int Cy, j;
		  DATA32 *pix;
		  int r, g, b, a, rr, gg, bb, aa;
		  int yap;
		  int sow;
		  int dyy, dxx;
		  
		  dptr = dst_ptr;
		  sow = src->image->w;
		  dyy = dst_clip_y - dst_region_y;
		  dxx = dst_clip_x - dst_region_x;
		  if (src->flags & RGBA_IMAGE_HAS_ALPHA)
		    {
		       for (y = 0; y < dst_clip_h; y++)
			 {
			    Cy = YAP >> 16;
			    yap = YAP & 0xffff;
			    for (x = 0; x < dst_clip_w; x++)
			      {
				 pix = ypoints[dyy + y] + xpoints[dxx + x] + (src_region_y * sow) + src_region_x;
				 r = (R_VAL(pix) * yap) >> 10;
				 g = (G_VAL(pix) * yap) >> 10;
				 b = (B_VAL(pix) * yap) >> 10;
				 a = (A_VAL(pix) * yap) >> 10;
				 for (j = (1 << 14) - yap; j > Cy; j -= Cy)
				   {
				      pix += sow;
				      r += (R_VAL(pix) * Cy) >> 10;
				      g += (G_VAL(pix) * Cy) >> 10;
				      b += (B_VAL(pix) * Cy) >> 10;
				      a += (A_VAL(pix) * Cy) >> 10;
				   }
				 if (j > 0)
				   {
				      pix += sow;
				      r += (R_VAL(pix) * j) >> 10;
				      g += (G_VAL(pix) * j) >> 10;
				      b += (B_VAL(pix) * j) >> 10;
				      a += (A_VAL(pix) * j) >> 10;
				   }
				 if (XAP > 0)
				   {
				      pix = ypoints[dyy + y] + xpoints[dxx + x] + 1 + (src_region_y * sow) + src_region_x;
				      rr = (R_VAL(pix) * yap) >> 10;
				      gg = (G_VAL(pix) * yap) >> 10;
				      bb = (B_VAL(pix) * yap) >> 10;
				      aa = (A_VAL(pix) * yap) >> 10;
				      for (j = (1 << 14) - yap; j > Cy; j -= Cy)
					{
					   pix += sow;
					   rr += (R_VAL(pix) * Cy) >> 10;
					   gg += (G_VAL(pix) * Cy) >> 10;
					   bb += (B_VAL(pix) * Cy) >> 10;
					   aa += (A_VAL(pix) * Cy) >> 10;
					}
				      if (j > 0)
					{
					   pix += sow;
					   rr += (R_VAL(pix) * j) >> 10;
					   gg += (G_VAL(pix) * j) >> 10;
					   bb += (B_VAL(pix) * j) >> 10;
					   aa += (A_VAL(pix) * j) >> 10;
					}
				      r = r * INV_XAP;
				      g = g * INV_XAP;
				      b = b * INV_XAP;
				      a = a * INV_XAP;
				      r = (r + ((rr * XAP))) >> 12;
				      g = (g + ((gg * XAP))) >> 12;
				      b = (b + ((bb * XAP))) >> 12;
				      a = (a + ((aa * XAP))) >> 12;
				   }
				 else
				   {
				      r >>= 4;
				      g >>= 4;
				      b >>= 4;
				      a >>= 4;
				   }
				 buf[x] = RGBA_COMPOSE(r, g, b, a);
			      }
			    if (dc->mod.use)
			      func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
			    else if (dc->mul.use)
			      func_mul(buf, dptr, dst_clip_w, dc->mul.col);
			    else
			      func(buf, dptr, dst_clip_w);
			    dptr += dst_w;
			 }
		    }
		  else
		    {
		       for (y = 0; y < dst_clip_h; y++)
			 {
			    Cy = YAP >> 16;
			    yap = YAP & 0xffff;
			    for (x = 0; x < dst_clip_w; x++)
			      {
				 pix = ypoints[dyy + y] + xpoints[dxx + x] + (src_region_y * sow) + src_region_x;
				 r = (R_VAL(pix) * yap) >> 10;
				 g = (G_VAL(pix) * yap) >> 10;
				 b = (B_VAL(pix) * yap) >> 10;
				 for (j = (1 << 14) - yap; j > Cy; j -= Cy)
				   {
				      pix += sow;
				      r += (R_VAL(pix) * Cy) >> 10;
				      g += (G_VAL(pix) * Cy) >> 10;
				      b += (B_VAL(pix) * Cy) >> 10;
				   }
				 if (j > 0)
				   {
				      pix += sow;
				      r += (R_VAL(pix) * j) >> 10;
				      g += (G_VAL(pix) * j) >> 10;
				      b += (B_VAL(pix) * j) >> 10;
				   }
				 if (XAP > 0)
				   {
				      pix = ypoints[dyy + y] + xpoints[dxx + x] + 1 + (src_region_y * sow) + src_region_x;
				      rr = (R_VAL(pix) * yap) >> 10;
				      gg = (G_VAL(pix) * yap) >> 10;
				      bb = (B_VAL(pix) * yap) >> 10;
				      for (j = (1 << 14) - yap; j > Cy; j -= Cy)
					{
					   pix += sow;
					   rr += (R_VAL(pix) * Cy) >> 10;
					   gg += (G_VAL(pix) * Cy) >> 10;
					   bb += (B_VAL(pix) * Cy) >> 10;
					}
				      if (j > 0)
					{
					   pix += sow;
					   rr += (R_VAL(pix) * j) >> 10;
					   gg += (G_VAL(pix) * j) >> 10;
					   bb += (B_VAL(pix) * j) >> 10;
					}
				      r = r * INV_XAP;
				      g = g * INV_XAP;
				      b = b * INV_XAP;
				      r = (r + ((rr * XAP))) >> 12;
				      g = (g + ((gg * XAP))) >> 12;
				      b = (b + ((bb * XAP))) >> 12;
				   }
				 else
				   {
				      r >>= 4;
				      g >>= 4;
				      b >>= 4;
				   }
				 buf[x] = RGBA_COMPOSE(r, g, b, 0xff);
			      }
			    if (dc->mod.use)
			      func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
			    else if (dc->mul.use)
			      func_mul(buf, dptr, dst_clip_w, dc->mul.col);
			    else
			      func(buf, dptr, dst_clip_w);
			    dptr += dst_w;
			 }
		    }
	       }
	     /* scaling down horizontally */
	     else if ((dst_region_w < src_region_w) &&
		      (dst_region_h >=  src_region_h))
	       {
		  int Cx, j;
		  DATA32 *pix;
		  int r, g, b, a, rr, gg, bb, aa;
		  int xap;
		  int sow;
		  int dyy, dxx;
		  
		  dptr = dst_ptr;
		  sow = src->image->w;
		  dyy = dst_clip_y - dst_region_y;
		  dxx = dst_clip_x - dst_region_x;
		  if (src->flags & RGBA_IMAGE_HAS_ALPHA)
		    {
		       for (y = 0; y < dst_clip_h; y++)
			 {
			    for (x = 0; x < dst_clip_w; x++)
			      {
				 Cx = XAP >> 16;
				 xap = XAP & 0xffff;
				 
				 pix = ypoints[dyy + y] + xpoints[dxx + x] + (src_region_y * sow) + src_region_x;
				 r = (R_VAL(pix) * xap) >> 10;
				 g = (G_VAL(pix) * xap) >> 10;
				 b = (B_VAL(pix) * xap) >> 10;
				 a = (A_VAL(pix) * xap) >> 10;
				 for (j = (1 << 14) - xap; j > Cx; j -= Cx)
				   {
				      pix++;
				      r += (R_VAL(pix) * Cx) >> 10;
				      g += (G_VAL(pix) * Cx) >> 10;
				      b += (B_VAL(pix) * Cx) >> 10;
				      a += (A_VAL(pix) * Cx) >> 10;
				   }
				 if (j > 0)
				   {
				      pix++;
				      r += (R_VAL(pix) * j) >> 10;
				      g += (G_VAL(pix) * j) >> 10;
				      b += (B_VAL(pix) * j) >> 10;
				      a += (A_VAL(pix) * j) >> 10;
				   }
				 if (YAP > 0)
				   {
				      pix = ypoints[dyy + y] + xpoints[dxx + x] + sow + (src_region_y * sow) + src_region_x;
				      rr = (R_VAL(pix) * xap) >> 10;
				      gg = (G_VAL(pix) * xap) >> 10;
				      bb = (B_VAL(pix) * xap) >> 10;
				      aa = (A_VAL(pix) * xap) >> 10;
				      for (j = (1 << 14) - xap; j > Cx; j -= Cx)
					{
					   pix++;
					   rr += (R_VAL(pix) * Cx) >> 10;
					   gg += (G_VAL(pix) * Cx) >> 10;
					   bb += (B_VAL(pix) * Cx) >> 10;
					   aa += (A_VAL(pix) * Cx) >> 10;
					}
				      if (j > 0)
					{
					   pix++;
					   rr += (R_VAL(pix) * j) >> 10;
					   gg += (G_VAL(pix) * j) >> 10;
					   bb += (B_VAL(pix) * j) >> 10;
					   aa += (A_VAL(pix) * j) >> 10;
					}
				      r = r * INV_YAP;
				      g = g * INV_YAP;
				      b = b * INV_YAP;
				      a = a * INV_YAP;
				      r = (r + ((rr * YAP))) >> 12;
				      g = (g + ((gg * YAP))) >> 12;
				      b = (b + ((bb * YAP))) >> 12;
				      a = (a + ((aa * YAP))) >> 12;
				   }
				 else
				   {
				      r >>= 4;
				      g >>= 4;
				      b >>= 4;
				      a >>= 4;
				   }
				 buf[x] = RGBA_COMPOSE(r, g, b, a);
			      }
			    if (dc->mod.use)
			      func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
			    else if (dc->mul.use)
			      func_mul(buf, dptr, dst_clip_w, dc->mul.col);
			    else
			      func(buf, dptr, dst_clip_w);
			    dptr += dst_w;
			 }
		    }
		  else
		    {
		       for (y = 0; y < dst_clip_h; y++)
			 {
			    for (x = 0; x < dst_clip_w; x++)
			      {
				 Cx = XAP >> 16;
				 xap = XAP & 0xffff;
				 
				 pix = ypoints[dyy + y] + xpoints[dxx + x] + (src_region_y * sow) + src_region_x;
				 r = (R_VAL(pix) * xap) >> 10;
				 g = (G_VAL(pix) * xap) >> 10;
				 b = (B_VAL(pix) * xap) >> 10;
				 for (j = (1 << 14) - xap; j > Cx; j -= Cx)
				   {
				      pix++;
				      r += (R_VAL(pix) * Cx) >> 10;
				      g += (G_VAL(pix) * Cx) >> 10;
				      b += (B_VAL(pix) * Cx) >> 10;
				   }
				 if (j > 0)
				   {
				      pix++;
				      r += (R_VAL(pix) * j) >> 10;
				      g += (G_VAL(pix) * j) >> 10;
				      b += (B_VAL(pix) * j) >> 10;
				   }
				 if (YAP > 0)
				   {
				      pix = ypoints[dyy + y] + xpoints[dxx + x] + sow + (src_region_y * sow) + src_region_x;
				      rr = (R_VAL(pix) * xap) >> 10;
				      gg = (G_VAL(pix) * xap) >> 10;
				      bb = (B_VAL(pix) * xap) >> 10;
				      for (j = (1 << 14) - xap; j > Cx; j -= Cx)
					{
					   pix++;
					   rr += (R_VAL(pix) * Cx) >> 10;
					   gg += (G_VAL(pix) * Cx) >> 10;
					   bb += (B_VAL(pix) * Cx) >> 10;
					}
				      if (j > 0)
					{
					   pix++;
					   rr += (R_VAL(pix) * j) >> 10;
					   gg += (G_VAL(pix) * j) >> 10;
					   bb += (B_VAL(pix) * j) >> 10;
					}
				      r = r * INV_YAP;
				      g = g * INV_YAP;
				      b = b * INV_YAP;
				      r = (r + ((rr * YAP))) >> 12;
				      g = (g + ((gg * YAP))) >> 12;
				      b = (b + ((bb * YAP))) >> 12;
				   }
				 else
				   {
				      r >>= 4;
				      g >>= 4;
				      b >>= 4;
				   }
				 buf[x] = RGBA_COMPOSE(r, g, b, 0xff);
			      }
			    if (dc->mod.use)
			      func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
			    else if (dc->mul.use)
			      func_mul(buf, dptr, dst_clip_w, dc->mul.col);
			    else
			      func(buf, dptr, dst_clip_w);
			    dptr += dst_w;
			 }
		    }
	       }
	     /* scaling down both vertically & horizontally */
	     else if ((dst_region_w < src_region_w) &&
		      (dst_region_h <  src_region_h))
	       {
		  int Cx, Cy, i, j;
		  DATA32 *pix;
		  int a, r, g, b, rx, gx, bx, ax;
		  int xap, yap;
		  int sow;
		  int dyy, dxx;
		  
		  dptr = dst_ptr;
		  sow = src->image->w;
		  dyy = dst_clip_y - dst_region_y;
		  dxx = dst_clip_x - dst_region_x;		  
//#ifndef SCALE_USING_MMX		  
#if 1
		  if (src->flags & RGBA_IMAGE_HAS_ALPHA)
		    {
		       for (y = 0; y < dst_clip_h; y++)
			 {
			    Cy = YAP >> 16;
			    yap = YAP & 0xffff;
			    
			    for (x = 0; x < dst_clip_w; x++)
			      {
				 Cx = XAP >> 16;
				 xap = XAP & 0xffff;
				 
				 sptr = ypoints[dyy + y] + xpoints[dxx + x] + (src_region_y * sow) + src_region_x;
				 pix = sptr;
				 sptr += sow;
				 rx = (R_VAL(pix) * xap) >> 9;
				 gx = (G_VAL(pix) * xap) >> 9;
				 bx = (B_VAL(pix) * xap) >> 9;
				 ax = (A_VAL(pix) * xap) >> 9;
				 pix++;
				 for (i = (1 << 14) - xap; i > Cx; i -= Cx)
				   {
				      rx += (R_VAL(pix) * Cx) >> 9;
				      gx += (G_VAL(pix) * Cx) >> 9;
				      bx += (B_VAL(pix) * Cx) >> 9;
				      ax += (A_VAL(pix) * Cx) >> 9;
				      pix++;
				   }
				 if (i > 0)
				   {
				      rx += (R_VAL(pix) * i) >> 9;
				      gx += (G_VAL(pix) * i) >> 9;
				      bx += (B_VAL(pix) * i) >> 9;
				      ax += (A_VAL(pix) * i) >> 9;
				   }
				 
				 r = (rx * yap) >> 14;
				 g = (gx * yap) >> 14;
				 b = (bx * yap) >> 14;
				 a = (ax * yap) >> 14;
				 
				 for (j = (1 << 14) - yap; j > Cy; j -= Cy)
				   {
				      pix = sptr;
				      sptr += sow;
				      rx = (R_VAL(pix) * xap) >> 9;
				      gx = (G_VAL(pix) * xap) >> 9;
				      bx = (B_VAL(pix) * xap) >> 9;
				      ax = (A_VAL(pix) * xap) >> 9;
				      pix++;
				      for (i = (1 << 14) - xap; i > Cx; i -= Cx)
					{
					   rx += (R_VAL(pix) * Cx) >> 9;
					   gx += (G_VAL(pix) * Cx) >> 9;
					   bx += (B_VAL(pix) * Cx) >> 9;
					   ax += (A_VAL(pix) * Cx) >> 9;
					   pix++;
					}
				      if (i > 0)
					{
					   rx += (R_VAL(pix) * i) >> 9;
					   gx += (G_VAL(pix) * i) >> 9;
					   bx += (B_VAL(pix) * i) >> 9;
					   ax += (A_VAL(pix) * i) >> 9;
					}
				      
				      r += (rx * Cy) >> 14;
				      g += (gx * Cy) >> 14;
				      b += (bx * Cy) >> 14;
				      a += (ax * Cy) >> 14;
				   }
				 if (j > 0)
				   {				 
				      pix = sptr;
				      sptr += sow;
				      rx = (R_VAL(pix) * xap) >> 9;
				      gx = (G_VAL(pix) * xap) >> 9;
				      bx = (B_VAL(pix) * xap) >> 9;
				      ax = (A_VAL(pix) * xap) >> 9;
				      pix++;
				      for (i = (1 << 14) - xap; i > Cx; i -= Cx)
					{
					   rx += (R_VAL(pix) * Cx) >> 9;
					   gx += (G_VAL(pix) * Cx) >> 9;
					   bx += (B_VAL(pix) * Cx) >> 9;
					   ax += (A_VAL(pix) * Cx) >> 9;
					   pix++;
					}
				      if (i > 0)
					{
					   rx += (R_VAL(pix) * i) >> 9;
					   gx += (G_VAL(pix) * i) >> 9;
					   bx += (B_VAL(pix) * i) >> 9;
					   ax += (A_VAL(pix) * i) >> 9;
					}
				      
				      r += (rx * j) >> 14;
				      g += (gx * j) >> 14;
				      b += (bx * j) >> 14;
				      a += (ax * j) >> 14;
				   }
				 buf[x] = RGBA_COMPOSE(r >> 5, g >> 5, b >> 5, a >> 5);
			      }
			    if (dc->mod.use)
			      func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
			    else if (dc->mul.use)
			      func_mul(buf, dptr, dst_clip_w, dc->mul.col);
			    else
			      func(buf, dptr, dst_clip_w);
			    dptr += dst_w;
			 }
		    }
		  else
		    {
		       for (y = 0; y < dst_clip_h; y++)
			 {
			    Cy = YAP >> 16;
			    yap = YAP & 0xffff;
			    
			    for (x = 0; x < dst_clip_w; x++)
			      {
				 Cx = XAP >> 16;
				 xap = XAP & 0xffff;
				 
				 sptr = ypoints[dyy + y] + xpoints[dxx + x] + (src_region_y * sow) + src_region_x;
				 pix = sptr;
				 sptr += sow;
				 rx = (R_VAL(pix) * xap) >> 9;
				 gx = (G_VAL(pix) * xap) >> 9;
				 bx = (B_VAL(pix) * xap) >> 9;
				 pix++;
				 for (i = (1 << 14) - xap; i > Cx; i -= Cx)
				   {
				      rx += (R_VAL(pix) * Cx) >> 9;
				      gx += (G_VAL(pix) * Cx) >> 9;
				      bx += (B_VAL(pix) * Cx) >> 9;
				      pix++;
				   }
				 if (i > 0)
				   {
				      rx += (R_VAL(pix) * i) >> 9;
				      gx += (G_VAL(pix) * i) >> 9;
				      bx += (B_VAL(pix) * i) >> 9;
				   }
				 
				 r = (rx * yap) >> 14;
				 g = (gx * yap) >> 14;
				 b = (bx * yap) >> 14;
				 
				 for (j = (1 << 14) - yap; j > Cy; j -= Cy)
				   {
				      pix = sptr;
				      sptr += sow;
				      rx = (R_VAL(pix) * xap) >> 9;
				      gx = (G_VAL(pix) * xap) >> 9;
				      bx = (B_VAL(pix) * xap) >> 9;
				      pix++;
				      for (i = (1 << 14) - xap; i > Cx; i -= Cx)
					{
					   rx += (R_VAL(pix) * Cx) >> 9;
					   gx += (G_VAL(pix) * Cx) >> 9;
					   bx += (B_VAL(pix) * Cx) >> 9;
					   pix++;
					}
				      if (i > 0)
					{
					   rx += (R_VAL(pix) * i) >> 9;
					   gx += (G_VAL(pix) * i) >> 9;
					   bx += (B_VAL(pix) * i) >> 9;
					}
				      
				      r += (rx * Cy) >> 14;
				      g += (gx * Cy) >> 14;
				      b += (bx * Cy) >> 14;
				   }
				 if (j > 0)
				   {				 
				      pix = sptr;
				      sptr += sow;
				      rx = (R_VAL(pix) * xap) >> 9;
				      gx = (G_VAL(pix) * xap) >> 9;
				      bx = (B_VAL(pix) * xap) >> 9;
				      pix++;
				      for (i = (1 << 14) - xap; i > Cx; i -= Cx)
					{
					   rx += (R_VAL(pix) * Cx) >> 9;
					   gx += (G_VAL(pix) * Cx) >> 9;
					   bx += (B_VAL(pix) * Cx) >> 9;
					   pix++;
					}
				      if (i > 0)
					{
					   rx += (R_VAL(pix) * i) >> 9;
					   gx += (G_VAL(pix) * i) >> 9;
					   bx += (B_VAL(pix) * i) >> 9;
					}
				      
				      r += (rx * j) >> 14;
				      g += (gx * j) >> 14;
				      b += (bx * j) >> 14;
				   }
				 buf[x] = RGBA_COMPOSE(r >> 5, g >> 5, b >> 5, 0xff);
			      }
			    if (dc->mod.use)
			      func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
			    else if (dc->mul.use)
			      func_mul(buf, dptr, dst_clip_w, dc->mul.col);
			    else
			      func(buf, dptr, dst_clip_w);
			    dptr += dst_w;
			 }
		    }
#else
#endif		  
	       }
	     free(xpoints);
	     free(ypoints);
	     free(xapoints);
	     free(yapoints);
#else
	     DATA32       *src_data;
	     RGBA_Surface *srf;

	     /* no mipmaps at all- need to populate mipmap table */
	     if (src->mipmaps.num == 0)
	       {
		  evas_common_scale_rgba_gen_mipmap_table(src);
	       }
	     /* caclulate mix level between mipmaps */
	     srf = evas_common_scale_rgba_get_mipmap_surface(src, 
						 src_region_w, src_region_h, 
						 dst_region_w, dst_region_h);
	     /* get mipmap level */
	     src_data = srf->data; /* high res surface (sample) */
	     /* lin & row ptr for lowest mipmap (hi res) */
	     divx = src->image->w / srf->w;
	     divy = src->image->h / srf->h;
	     
	     /* merely sampled pixels in the hi-res mipmap  and  interp no's */
	     for (x = 0; x < dst_clip_w; x++)
	       {
		  unsigned int val, val2;
		  
		  val = (((x + dst_clip_x - dst_region_x) * src_region_w * 256) / (dst_region_w * divx));
		  val2 = (((x + 1 + dst_clip_x - dst_region_x) * src_region_w * 256) / (dst_region_w * divx));
		  lin_ptr[x] = val >> 8;
		  if (src->image->w < 2)
		    {
		       sample_x[(x * 3) + 0] = 256;
		       sample_x[(x * 3) + 1] = 0;
		       sample_x[(x * 3) + 2] = 0;
		    }	       
		  if (!(val & 0xff))
		    /* start of pixel scan on src pixel boundary */
		    {
		       sample_x[(x * 3) + 0] = 256;
		       sample_x[(x * 3) + 1] = val2 - val - 256;
		       sample_x[(x * 3) + 2] = 0;
		    }
		  else
		    /* start pixel scan in middle of src pixel */
		    {
		       sample_x[(x * 3) + 0] = (val & 0xffffff00) + 256 - val;
		       if (val2 >= ((val & 0xffffff00) + 512))
			 {
			    sample_x[(x * 3) + 1] = 256;
			    sample_x[(x * 3) + 2] = val2 - ((val & 0xffffff00) + 512);
			 }
		       else
			 {
			    sample_x[(x * 3) + 1] = val2 - ((val & 0xffffff00) + 256);
			    sample_x[(x * 3) + 2] = 0;
			 }
		    }
		  interp_x[x] = 
		    sample_x[(x * 3) + 0] + 
		    sample_x[(x * 3) + 1] + 
		    sample_x[(x * 3) + 2];
		  sample_x[(x * 3) + 0] = (sample_x[(x * 3) + 0] * 256) / interp_x[x];
		  sample_x[(x * 3) + 1] = (sample_x[(x * 3) + 1] * 256) / interp_x[x];
		  sample_x[(x * 3) + 2] = (sample_x[(x * 3) + 2] * 256) / interp_x[x];
		  if      (sample_x[(x * 3) + 2]) iterate_x[x] = 3;
		  else if (sample_x[(x * 3) + 1]) iterate_x[x] = 2;
		  else                            iterate_x[x] = 1;
	       }
	     for (y = 0; y < dst_clip_h; y++)
	       {
		  unsigned int val, val2;
		  
		  val = (((y + dst_clip_y - dst_region_y) * src_region_h * 256) / (dst_region_h * divy));
		  val2 = (((y + 1 + dst_clip_y - dst_region_y) * src_region_h * 256) / (dst_region_h * divy));
		  row_ptr[y] = src_data + ((val >> 8) * srf->w);
		  if (src->image->h < 2)
		    {
		       sample_y[(y * 3) + 0] = 256;
		       sample_y[(y * 3) + 1] = 0;
		       sample_y[(y * 3) + 2] = 0;
		    }	       
		  else if (!(val & 0xff))
		    /* start of pixel scan on src pixel boundary */
		    {
		       sample_y[(y * 3) + 0] = 256;
		       sample_y[(y * 3) + 1] = val2 - val - 256;
		       sample_y[(y * 3) + 2] = 0;
		    }
		  else
		    /* start pixel scan in middle of src pixel */
		    {
		       sample_y[(y * 3) + 0] = (val & 0xffffff00) + 256 - val;
		       if (val2 >= ((val & 0xffffff00) + 512))
			 {
			    sample_y[(y * 3) + 1] = 256;
			    sample_y[(y * 3) + 2] = val2 - ((val & 0xffffff00) + 512);
			 }
		       else
			 {
			    sample_y[(y * 3) + 1] = val2 - ((val & 0xffffff00) + 256);
			    sample_y[(y * 3) + 2] = 0;
			 }
		    }
		  interp_y[y] = 
		    sample_y[(y * 3) + 0] + 
		    sample_y[(y * 3) + 1] + 
		    sample_y[(y * 3) + 2];
		  sample_y[(y * 3) + 0] = (sample_y[(y * 3) + 0] * 256) / interp_y[y];
		  sample_y[(y * 3) + 1] = (sample_y[(y * 3) + 1] * 256) / interp_y[y];
		  sample_y[(y * 3) + 2] = (sample_y[(y * 3) + 2] * 256) / interp_y[y];
		  if      (sample_y[(y * 3) + 2]) iterate_y[y] = 3;
		  else if (sample_y[(y * 3) + 1]) iterate_y[y] = 2;
		  else                            iterate_y[y] = 1;
	       }
	     /* scale to dst */
	     dptr = dst_ptr;
	     
	     if (src->flags & RGBA_IMAGE_HAS_ALPHA)
	       {
		  int srf_w_3;
		  
		  srf_w_3 = srf->w - 3;
		  for (y = 0; y < dst_clip_h; y++)
		    {
		       int ity;
		       
		       ity = iterate_y[y];
		       dst_ptr = buf;
		       for (x = 0; x < dst_clip_w; x++)
			 {
			    int x3, y3, i, j;
#ifndef SCALE_USING_MMX
			    DATA32 r, g, b, a;
#endif
			    
			    x3 = x * 3;
			    y3 = y * 3;
			    ptr = row_ptr[y] + lin_ptr[x];
#ifndef SCALE_USING_MMX
			    r = g = b = a = 0;
			    for (j = 0; j < ity; j++)
			      {
				 int mj;
				 int itx;
				 
				 itx = iterate_x[x];
				 mj = sample_y[y3 + j];
				 for (i = 0; i < itx; i++)
				   {
				      int multiplier;
				      
				      multiplier = sample_x[x3 + i] * mj;
				      r += R_VAL(ptr) * multiplier;
				      g += G_VAL(ptr) * multiplier;
				      b += B_VAL(ptr) * multiplier;
				      a += A_VAL(ptr) * multiplier;
				      ptr++;
				   }
				 ptr += 3 - itx + srf_w_3;
			      }
			    if (a)
			      {
				 A_VAL(dst_ptr) = a >> 16;
				 R_VAL(dst_ptr) = r >> 16;
				 G_VAL(dst_ptr) = g >> 16;
				 B_VAL(dst_ptr) = b >> 16;
			      }
			    else
			      {
				 A_VAL(dst_ptr) = 0;
			      }
#else
			    pxor_r2r(mm0, mm0);
			    for (j = 0; j < ity; j++)
			      {
				 int mj;
				 int itx;
				 
				 itx = iterate_x[x];
				 mj = sample_y[y3 + j];
				 for (i = 0; i < itx; i++)
				   {
				      int multiplier;
				      
				      multiplier = (sample_x[x3 + i] * mj) >> 8;
					   
				      punpcklbw_m2r(ptr[0], mm2);
				      psrlw_i2r(8, mm2);
				      
				      movd_m2r(multiplier, mm3);
				      punpcklwd_r2r(mm3, mm3);
				      punpckldq_r2r(mm3, mm3);
				      
				      pmullw_r2r(mm3, mm2);
				      
				      paddw_r2r(mm2, mm0);
				      ptr++;
				   }
				 ptr += 3 - itx + srf_w_3;
			      }
			    psrlw_i2r(8, mm0);
			    packuswb_r2r(mm0, mm0);
			    movd_r2m(mm0, dst_ptr[0]);
#endif			    
			    dst_ptr++;
			 }
		       /* * blend here [clip_w *] buf -> dptr * */
		       if (dc->mod.use)
			 {
			    func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
			 }
		       else if (dc->mul.use)
			 {
			    func_mul(buf, dptr, dst_clip_w, dc->mul.col);
			 }
		       else
			 {
			    func(buf, dptr, dst_clip_w);
			 }
		       dptr += dst_w;
		    }
	       }
	     else
	       {
                  int srf_w_3;
		  
		  srf_w_3 = srf->w - 3;
		  for (y = 0; y < dst_clip_h; y++)
		    {
		       int ity;
		       
		       ity = iterate_y[y];
		       dst_ptr = buf;
		       for (x = 0; x < dst_clip_w; x++)
			 {
			    int x3, y3, i, j;
#ifndef SCALE_USING_MMX
			    DATA32 r, g, b;
#endif
			    
			    x3 = x * 3;
			    y3 = y * 3;
			    ptr = row_ptr[y] + lin_ptr[x];
#ifndef SCALE_USING_MMX
			    r = g = b = 0;
			    for (j = 0; j < ity; j++)
			      {
				 int mj;
				 int itx;
				 
				 itx = iterate_x[x];
				 mj = sample_y[y3 + j];
				 for (i = 0; i < itx; i++)
				   {
				      int multiplier;
				      
				      multiplier = sample_x[x3 + i] * mj;
				      if (multiplier)
					{
					   r += R_VAL(ptr) * multiplier;
					   g += G_VAL(ptr) * multiplier;
					   b += B_VAL(ptr) * multiplier;
					}
				      ptr++;
				   }
				 ptr += 3 - itx + srf_w_3;
			      }
			    R_VAL(dst_ptr) = r >> 8;
			    G_VAL(dst_ptr) = g >> 8;
			    B_VAL(dst_ptr) = b >> 8;
			    A_VAL(dst_ptr) = 0xff;
#else
			    pxor_r2r(mm0, mm0);
			    for (j = 0; j < ity; j++)
			      {
				 int mj;
				 int itx;
				 
				 itx = iterate_x[x];
				 mj = sample_y[y3 + j];
				 for (i = 0; i < itx; i++)
				   {
				      int multiplier;
				      
				      multiplier = (sample_x[x3 + i] * mj) >> 8;
					   
				      punpcklbw_m2r(ptr[0], mm2);
				      psrlw_i2r(8, mm2);
				      
				      movd_m2r(multiplier, mm3);
				      punpcklwd_r2r(mm3, mm3);
				      punpckldq_r2r(mm3, mm3);
				      
				      pmullw_r2r(mm3, mm2);
				      
				      paddw_r2r(mm2, mm0);
				      ptr++;
				   }
				 ptr += 3 - itx + srf_w_3;
			      }
			    psrlw_i2r(8, mm0);
			    packuswb_r2r(mm0, mm0);
			    movd_r2m(mm0, dst_ptr[0]);
#endif			    
			    dst_ptr++;
			 }
		       /* * blend here [clip_w *] buf -> dptr * */
		       if (dc->mod.use)
			 {
			    func_cmod(buf, dptr, dst_clip_w, dc->mod.r, dc->mod.g, dc->mod.b, dc->mod.a);
			 }
		       else if (dc->mul.use)
			 {
			    func_mul(buf, dptr, dst_clip_w, dc->mul.col);
			 }
		       else
			 {
			    func(buf, dptr, dst_clip_w);
			 }
		       dptr += dst_w;
		    }
	       }
#endif	     
	  }	
#endif	
	free(buf);
     }
   no_buf:
   /* free scale tables */
   free(iterate_y);
   no_iterate_y:
   free(iterate_x);
   no_iterate_x:
   free(sample_y);
   no_sample_y:
   free(sample_x);
   no_sample_x:
   free(interp_y);
   no_interp_y:
   free(interp_x);
   no_interp_x:
   free(row2_ptr);
   no_row2_ptr:
   free(lin2_ptr);
   no_lin2_ptr:
   free(row_ptr);
   no_row_ptr:
   free(lin_ptr);
//_WIN32_WCE
   no_lin_ptr: ;
}
