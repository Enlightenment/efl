{
   int Cx, j;
   DATA32 *pix, *dptr, *pbuf, **yp;
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

   if (src->flags & RGBA_IMAGE_HAS_ALPHA)
     {
	while (dst_clip_h--)
	  {
#ifdef EVAS_SLI
	     if (((ysli) % dc->sli.h) == dc->sli.y)
#endif
	       {
		  while (dst_clip_w--)
		    {
		       Cx = *xapp >> 16;
		       xap = *xapp & 0xffff;
		       pix = *yp + *xp + pos;
		       
		       a = (A_VAL(pix) * xap) >> 10;
		       r = (R_VAL(pix) * xap) >> 10;
		       g = (G_VAL(pix) * xap) >> 10;
		       b = (B_VAL(pix) * xap) >> 10;
		       for (j = (1 << 14) - xap; j > Cx; j -= Cx)
			 {
			    pix++;
			    a += (A_VAL(pix) * Cx) >> 10;
			    r += (R_VAL(pix) * Cx) >> 10;
			    g += (G_VAL(pix) * Cx) >> 10;
			    b += (B_VAL(pix) * Cx) >> 10;
			 }
		       if (j > 0)
			 {
			    pix++;
			    a += (A_VAL(pix) * j) >> 10;
			    r += (R_VAL(pix) * j) >> 10;
			    g += (G_VAL(pix) * j) >> 10;
			    b += (B_VAL(pix) * j) >> 10;
			 }
		       if ((yap = *yapp) > 0)
			 {
			    pix = *yp + *xp + src_w + pos;
			    aa = (A_VAL(pix) * xap) >> 10;
			    rr = (R_VAL(pix) * xap) >> 10;
			    gg = (G_VAL(pix) * xap) >> 10;
			    bb = (B_VAL(pix) * xap) >> 10;
			    for (j = (1 << 14) - xap; j > Cx; j -= Cx)
			      {
				 pix++;
				 aa += (A_VAL(pix) * Cx) >> 10;
				 rr += (R_VAL(pix) * Cx) >> 10;
				 gg += (G_VAL(pix) * Cx) >> 10;
				 bb += (B_VAL(pix) * Cx) >> 10;
			      }
			    if (j > 0)
			      {
				 pix++;
				 aa += (A_VAL(pix) * j) >> 10;
				 rr += (R_VAL(pix) * j) >> 10;
				 gg += (G_VAL(pix) * j) >> 10;
				 bb += (B_VAL(pix) * j) >> 10;
			      }
			    a += ((aa - a) * yap) >> 8;
			    r += ((rr - r) * yap) >> 8;
			    g += ((gg - g) * yap) >> 8;
			    b += ((bb - b) * yap) >> 8;
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
        if ((!(src->flags & RGBA_IMAGE_HAS_ALPHA)) &&
	    (!(dst->flags & RGBA_IMAGE_HAS_ALPHA)) &&
	    (!dc->mul.use))
	  {
	     while (dst_clip_h--)
	       {
                  pbuf = dptr;
#ifdef EVAS_SLI
		  if (((ysli) % dc->sli.h) == dc->sli.y)
#endif
		    {
		       while (dst_clip_w--)
			 {
			    Cx = *xapp >> 16;
			    xap = *xapp & 0xffff;
			    pix = *yp + *xp + pos;
			    
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
			    if ((yap = *yapp) > 0)
			      {
				 pix = *yp + *xp + src_w + pos;
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
				 r += ((rr - r) * yap) >> 8;
				 g += ((gg - g) * yap) >> 8;
				 b += ((bb - b) * yap) >> 8;
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
		       while (dst_clip_w--)
			 {
			    Cx = *xapp >> 16;
			    xap = *xapp & 0xffff;
			    pix = *yp + *xp + pos;
			    
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
			    if ((yap = *yapp) > 0)
			      {
				 pix = *yp + *xp + src_w + pos;
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
				 r += ((rr - r) * yap) >> 8;
				 g += ((gg - g) * yap) >> 8;
				 b += ((bb - b) * yap) >> 8;
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
