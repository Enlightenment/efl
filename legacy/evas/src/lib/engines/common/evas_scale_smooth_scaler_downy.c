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
