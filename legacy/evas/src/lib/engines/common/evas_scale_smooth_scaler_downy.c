{
   int Cy, j;
   DATA32 *dptr, *pix, *pbuf, **yp;
   int r, g, b, a, rr, gg, bb, aa;
   int *xp, xap, yap, pos;
   int dyy, dxx;
   int w = dst_clip_w;
#ifdef EVAS_SLI
   int ysli = dst_clip_y;
#endif

   dptr = dst_ptr;
   pos = (src_region_y * src_w) + src_region_x;
   dyy = dst_clip_y - dst_region_y;
   dxx = dst_clip_x - dst_region_x;

   xp = xpoints + dxx;
   yp = ypoints + dyy;
   xapp = xapoints + dxx;
   yapp = yapoints + dyy;
   pbuf = buf;

   if (src->cache_entry.flags.alpha)
     {
	while (dst_clip_h--)
	  {
#ifdef EVAS_SLI
	     if (((ysli) % dc->sli.h) == dc->sli.y)
#endif
	       {
		  Cy = *yapp >> 16;
		  yap = *yapp & 0xffff;

		  while (dst_clip_w--)
		    {
		       pix = *yp + *xp + pos;

		       a = (A_VAL(pix) * yap) >> 10;
		       r = (R_VAL(pix) * yap) >> 10;
		       g = (G_VAL(pix) * yap) >> 10;
		       b = (B_VAL(pix) * yap) >> 10;
		       for (j = (1 << 14) - yap; j > Cy; j -= Cy)
			 {
			    pix += src_w;
			    a += (A_VAL(pix) * Cy) >> 10;
			    r += (R_VAL(pix) * Cy) >> 10;
			    g += (G_VAL(pix) * Cy) >> 10;
			    b += (B_VAL(pix) * Cy) >> 10;
			 }
		       if (j > 0)
			 {
			    pix += src_w;
			    a += (A_VAL(pix) * j) >> 10;
			    r += (R_VAL(pix) * j) >> 10;
			    g += (G_VAL(pix) * j) >> 10;
			    b += (B_VAL(pix) * j) >> 10;
			 }
		       if ((xap = *xapp) > 0)
			 {
			    pix = *yp + *xp + 1 + pos;
			    aa = (A_VAL(pix) * yap) >> 10;
			    rr = (R_VAL(pix) * yap) >> 10;
			    gg = (G_VAL(pix) * yap) >> 10;
			    bb = (B_VAL(pix) * yap) >> 10;
			    for (j = (1 << 14) - yap; j > Cy; j -= Cy)
			      {
				 pix += src_w;
				 aa += (A_VAL(pix) * Cy) >> 10;
				 rr += (R_VAL(pix) * Cy) >> 10;
				 gg += (G_VAL(pix) * Cy) >> 10;
				 bb += (B_VAL(pix) * Cy) >> 10;
			      }
			    if (j > 0)
			      {
				 pix += src_w;
				 aa += (A_VAL(pix) * j) >> 10;
				 rr += (R_VAL(pix) * j) >> 10;
				 gg += (G_VAL(pix) * j) >> 10;
				 bb += (B_VAL(pix) * j) >> 10;
			      }
			    a += ((aa - a) * xap) >> 8;
			    r += ((rr - r) * xap) >> 8;
			    g += ((gg - g) * xap) >> 8;
			    b += ((bb - b) * xap) >> 8;
			 }
		       *pbuf++ = ARGB_JOIN(a >> 4, r >> 4, g >> 4, b >> 4);
		       xp++;  xapp++;
		    }

		  func(buf, NULL, dc->mul.col, dptr, w);
               }
#ifdef EVAS_SLI
	     ysli++;
#endif
	     pbuf = buf;
	     dptr += dst_w;  dst_clip_w = w;
	     yp++;  yapp++;
	     xp = xpoints + dxx;
	     xapp = xapoints + dxx;
	  }
     }
   else
     {
#ifdef DIRECT_SCALE
        if ((!src->cache_entry.flags.alpha) &&
	    (!dst->cache_entry.flags.alpha) &&
	    (!dc->mul.use))
	  {
	     while (dst_clip_h--)
	       {
#ifdef EVAS_SLI
		  if (((ysli) % dc->sli.h) == dc->sli.y)
#endif
		    {
		       Cy = *yapp >> 16;
		       yap = *yapp & 0xffff;

		       pbuf = dptr;
		       while (dst_clip_w--)
			 {
			    pix = *yp + *xp + pos;

			    r = (R_VAL(pix) * yap) >> 10;
			    g = (G_VAL(pix) * yap) >> 10;
			    b = (B_VAL(pix) * yap) >> 10;
			    for (j = (1 << 14) - yap; j > Cy; j -= Cy)
			      {
				 pix += src_w;
				 r += (R_VAL(pix) * Cy) >> 10;
				 g += (G_VAL(pix) * Cy) >> 10;
				 b += (B_VAL(pix) * Cy) >> 10;
			      }
			    if (j > 0)
			      {
				 pix += src_w;
				 r += (R_VAL(pix) * j) >> 10;
				 g += (G_VAL(pix) * j) >> 10;
				 b += (B_VAL(pix) * j) >> 10;
			      }
			    if ((xap = *xapp) > 0)
			      {
				 pix = *yp + *xp + 1 + pos;
				 rr = (R_VAL(pix) * yap) >> 10;
				 gg = (G_VAL(pix) * yap) >> 10;
				 bb = (B_VAL(pix) * yap) >> 10;
				 for (j = (1 << 14) - yap; j > Cy; j -= Cy)
				   {
				      pix += src_w;
				      rr += (R_VAL(pix) * Cy) >> 10;
				      gg += (G_VAL(pix) * Cy) >> 10;
				      bb += (B_VAL(pix) * Cy) >> 10;
				   }
				 if (j > 0)
				   {
				      pix += src_w;
				      rr += (R_VAL(pix) * j) >> 10;
				      gg += (G_VAL(pix) * j) >> 10;
				      bb += (B_VAL(pix) * j) >> 10;
				   }
				 r += ((rr - r) * xap) >> 8;
				 g += ((gg - g) * xap) >> 8;
				 b += ((bb - b) * xap) >> 8;
			      }
			    *pbuf++ = ARGB_JOIN(0xff, r >> 4, g >> 4, b >> 4);
			    xp++;  xapp++;
			 }
		    }
#ifdef EVAS_SLI
		  ysli++;
#endif
		  dptr += dst_w;  dst_clip_w = w;
		  yp++;  yapp++;
		  xp = xpoints + dxx;
		  xapp = xapoints + dxx;
	       }
	  }
	else
#endif
	  {
	     while (dst_clip_h--)
	       {
#ifdef EVAS_SLI
		  if (((ysli) % dc->sli.h) == dc->sli.y)
#endif
		    {
		       Cy = *yapp >> 16;
		       yap = *yapp & 0xffff;

		       while (dst_clip_w--)
			 {
			    pix = *yp + *xp + pos;

			    r = (R_VAL(pix) * yap) >> 10;
			    g = (G_VAL(pix) * yap) >> 10;
			    b = (B_VAL(pix) * yap) >> 10;
			    for (j = (1 << 14) - yap; j > Cy; j -= Cy)
			      {
				 pix += src_w;
				 r += (R_VAL(pix) * Cy) >> 10;
				 g += (G_VAL(pix) * Cy) >> 10;
				 b += (B_VAL(pix) * Cy) >> 10;
			      }
			    if (j > 0)
			      {
				 pix += src_w;
				 r += (R_VAL(pix) * j) >> 10;
				 g += (G_VAL(pix) * j) >> 10;
				 b += (B_VAL(pix) * j) >> 10;
			      }
			    if ((xap = *xapp) > 0)
			      {
				 pix = *yp + *xp + 1 + pos;
				 rr = (R_VAL(pix) * yap) >> 10;
				 gg = (G_VAL(pix) * yap) >> 10;
				 bb = (B_VAL(pix) * yap) >> 10;
				 for (j = (1 << 14) - yap; j > Cy; j -= Cy)
				   {
				      pix += src_w;
				      rr += (R_VAL(pix) * Cy) >> 10;
				      gg += (G_VAL(pix) * Cy) >> 10;
				      bb += (B_VAL(pix) * Cy) >> 10;
				   }
				 if (j > 0)
				   {
				      pix += src_w;
				      rr += (R_VAL(pix) * j) >> 10;
				      gg += (G_VAL(pix) * j) >> 10;
				      bb += (B_VAL(pix) * j) >> 10;
				   }
				 r += ((rr - r) * xap) >> 8;
				 g += ((gg - g) * xap) >> 8;
				 b += ((bb - b) * xap) >> 8;
			      }
			    *pbuf++ = ARGB_JOIN(0xff, r >> 4, g >> 4, b >> 4);
			    xp++;  xapp++;
			 }

		       func(buf, NULL, dc->mul.col, dptr, w);
		    }
#ifdef EVAS_SLI
		  ysli++;
#endif
		  pbuf = buf;
		  dptr += dst_w;  dst_clip_w = w;
		  yp++;  yapp++;
		  xp = xpoints + dxx;
		  xapp = xapoints + dxx;
	       }
	  }
     }
}
