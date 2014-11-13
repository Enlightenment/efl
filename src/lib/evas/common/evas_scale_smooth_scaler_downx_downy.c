{
   int Cx, Cy, i, j;
   DATA32 *dptr, *sptr, *pix, *pbuf;
   DATA8 *mask;
   int a, r, g, b, rx, gx, bx, ax;
   int xap, yap, pos;
   int y;
   //int dyy, dxx;

   DATA32  **yp;
   int *xp;
   int w = dst_clip_w;

   dptr = dst_ptr;
   pos = (src_region_y * src_w) + src_region_x;
   //dyy = dst_clip_y - dst_region_y;
   //dxx = dst_clip_x - dst_region_x;

   xp = xpoints;// + dxx;
   yp = ypoints;// + dyy;
   xapp = xapoints;// + dxx;
   yapp = yapoints;// + dyy;
   pbuf = buf;
/*#ifndef SCALE_USING_MMX */
/* for now there's no mmx down scaling - so C only */
#if 1
   if (src->cache_entry.flags.alpha)
     {
        y = 0;
	while (dst_clip_h--)
	  {
	    Cy = *yapp >> 16;
	    yap = *yapp & 0xffff;

	    while (dst_clip_w--)
	      {
		Cx = *xapp >> 16;
		xap = *xapp & 0xffff;

		sptr = *yp + *xp + pos;
		pix = sptr;
		sptr += src_w;

		ax = (A_VAL(pix) * xap) >> 9;
		rx = (R_VAL(pix) * xap) >> 9;
		gx = (G_VAL(pix) * xap) >> 9;
		bx = (B_VAL(pix) * xap) >> 9;
		pix++;
		for (i = (1 << 14) - xap; i > Cx; i -= Cx)
		  {
		    ax += (A_VAL(pix) * Cx) >> 9;
		    rx += (R_VAL(pix) * Cx) >> 9;
		    gx += (G_VAL(pix) * Cx) >> 9;
		    bx += (B_VAL(pix) * Cx) >> 9;
		    pix++;
		  }
		if (i > 0)
		  {
		    ax += (A_VAL(pix) * i) >> 9;
		    rx += (R_VAL(pix) * i) >> 9;
		    gx += (G_VAL(pix) * i) >> 9;
		    bx += (B_VAL(pix) * i) >> 9;
		  }

		a = (ax * yap) >> 14;
		r = (rx * yap) >> 14;
		g = (gx * yap) >> 14;
		b = (bx * yap) >> 14;

		for (j = (1 << 14) - yap; j > Cy; j -= Cy)
		  {
		    pix = sptr;
		    sptr += src_w;
		    ax = (A_VAL(pix) * xap) >> 9;
		    rx = (R_VAL(pix) * xap) >> 9;
		    gx = (G_VAL(pix) * xap) >> 9;
		    bx = (B_VAL(pix) * xap) >> 9;
		    pix++;
		    for (i = (1 << 14) - xap; i > Cx; i -= Cx)
		      {
			ax += (A_VAL(pix) * Cx) >> 9;
			rx += (R_VAL(pix) * Cx) >> 9;
			gx += (G_VAL(pix) * Cx) >> 9;
			bx += (B_VAL(pix) * Cx) >> 9;
			pix++;
		      }
		    if (i > 0)
		      {
			ax += (A_VAL(pix) * i) >> 9;
			rx += (R_VAL(pix) * i) >> 9;
			gx += (G_VAL(pix) * i) >> 9;
			bx += (B_VAL(pix) * i) >> 9;
		      }

		    a += (ax * Cy) >> 14;
		    r += (rx * Cy) >> 14;
		    g += (gx * Cy) >> 14;
		    b += (bx * Cy) >> 14;
		  }
		if (j > 0)
		  {
		    pix = sptr;
		    sptr += src_w;
		    ax = (A_VAL(pix) * xap) >> 9;
		    rx = (R_VAL(pix) * xap) >> 9;
		    gx = (G_VAL(pix) * xap) >> 9;
		    bx = (B_VAL(pix) * xap) >> 9;
		    pix++;
		    for (i = (1 << 14) - xap; i > Cx; i -= Cx)
		      {
			ax += (A_VAL(pix) * Cx) >> 9;
			rx += (R_VAL(pix) * Cx) >> 9;
			gx += (G_VAL(pix) * Cx) >> 9;
			bx += (B_VAL(pix) * Cx) >> 9;
			pix++;
		      }
		    if (i > 0)
		      {
			ax += (A_VAL(pix) * i) >> 9;
			rx += (R_VAL(pix) * i) >> 9;
			gx += (G_VAL(pix) * i) >> 9;
			bx += (B_VAL(pix) * i) >> 9;
		      }

		    a += (ax * j) >> 14;
		    r += (rx * j) >> 14;
		    g += (gx * j) >> 14;
		    b += (bx * j) >> 14;
		  }
		*pbuf++ = ARGB_JOIN(((a + (1 << 4)) >> 5),
				    ((r + (1 << 4)) >> 5),
				    ((g + (1 << 4)) >> 5),
				    ((b + (1 << 4)) >> 5));
		xp++;  xapp++;
	      }

            if (!mask_ie)
              func(buf, NULL, mul_col, dptr, w);
            else
              {
                 mask = mask_ie->image.data8
                    + ((dst_clip_y - mask_y + y) * mask_ie->cache_entry.w)
                    + (dst_clip_x - mask_x);

                 if (mul_col != 0xffffffff) func2(buf, NULL, mul_col, buf, w);
                 func(buf, mask, 0, dptr, w);
              }
            y++;

	    pbuf = buf;
	    dptr += dst_w;   dst_clip_w = w;
	    xp = xpoints;// + dxx;
	    xapp = xapoints;// + dxx;
	    yp++;  yapp++;
	  }
     }
   else
     {
#ifdef DIRECT_SCALE
        if ((!src->cache_entry.flags.alpha) &&
            (!dst->cache_entry.flags.alpha) &&
            (mul_col == 0xffffffff) &&
            (!mask_ie))
	  {
	     while (dst_clip_h--)
	       {
		 Cy = *yapp >> 16;
		 yap = *yapp & 0xffff;

		 pbuf = dptr;
		 while (dst_clip_w--)
		   {
		     Cx = *xapp >> 16;
		     xap = *xapp & 0xffff;

		     sptr = *yp + *xp + pos;
		     pix = sptr;
		     sptr += src_w;

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
			 sptr += src_w;
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
			 sptr += src_w;
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
		     *pbuf++ = ARGB_JOIN(0xff,
					 ((r + (1 << 4)) >> 5),
					 ((g + (1 << 4)) >> 5),
					 ((b + (1 << 4)) >> 5));
		     xp++;  xapp++;
		   }

		 dptr += dst_w;   dst_clip_w = w;
		 xp = xpoints;// + dxx;
		 xapp = xapoints;// + dxx;
		 yp++;  yapp++;
	       }
	  }
	else
#endif
	  {
             y = 0;
	     while (dst_clip_h--)
	       {
		 Cy = *yapp >> 16;
		 yap = *yapp & 0xffff;

		 while (dst_clip_w--)
		   {
		     Cx = *xapp >> 16;
		     xap = *xapp & 0xffff;

		     sptr = *yp + *xp + pos;
		     pix = sptr;
		     sptr += src_w;

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
			 sptr += src_w;
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
			 sptr += src_w;
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
		     *pbuf++ = ARGB_JOIN(0xff,
					 ((r + (1 << 4)) >> 5),
					 ((g + (1 << 4)) >> 5),
					 ((b + (1 << 4)) >> 5));
		     xp++;  xapp++;
		   }

                 if (!mask_ie)
                   func(buf, NULL, mul_col, dptr, w);
                 else
                   {
                      mask = mask_ie->image.data8
                         + ((dst_clip_y - mask_y + y) * mask_ie->cache_entry.w)
                         + (dst_clip_x - mask_x);

                      if (mul_col != 0xffffffff) func2(buf, NULL, mul_col, buf, w);
                      func(buf, mask, 0, dptr, w);
                   }
                 y++;

		 pbuf = buf;
		 dptr += dst_w;   dst_clip_w = w;
		 xp = xpoints;// + dxx;
		 xapp = xapoints;// + dxx;
		 yp++;  yapp++;
	       }
	  }
     }
#else
   /* MMX scaling down would go here */
#endif
}
