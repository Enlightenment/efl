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
	     /* blend mm3... */
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
