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
/*#ifndef SCALE_USING_MMX */
/* for now there's no mmx down scaling - so C only */
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
   /* MMX scaling down would go here */
#endif
}
