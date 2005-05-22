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
