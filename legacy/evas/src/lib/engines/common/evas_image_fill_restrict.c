
/* restrict funcs */


/* identity transform */

static void
src_a8r8g8b8_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
			int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   DATA32 *p;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= ih))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }
   p = im + (y * iw) + x;
   while (d < e)
     {
	DATA32 p0 = 0;

	if (((unsigned) x) < iw)
	    p0 = *p;
	*d++ = p0;  p++;  x++;
     }
}

static void
src_a8r8g8b8_scaled_x_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   DATA32 *p;
   int iaxx = idata->iaxx;
   int fw = idata->fw;
   int ixx;

   y += idata->ayz >> 16;
   if ((y < 0) || (y >= ih))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }
   x += idata->axz >> 16;
   ixx = iaxx * x;
   p = im + (y * iw);
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (d < e)
     {
	DATA32 p0 = 0;

	if (((unsigned) x) < fw)
	  {
	    int  ix = ixx >> 16;
	    int  ax = 1 + ((ixx >> 8) & 0xff);
	    DATA32  p1 = p0 = *(p + ix);

	    if ((ix + 1) < iw)
		p1 = *(p + ix + 1);
	    if (p0 | p1)
#ifdef BUILD_MMX
	      {
		MOV_A2R(ax, mm3)
		MOV_P2R(p0, mm1, mm0)
		MOV_P2R(p1, mm2, mm0)
		INTERP_256_R2R(mm3, mm2, mm1, mm5)
		MOV_R2P(mm1, p0, mm0)
	      }
#else
		p0 = INTERP_256(ax, p1, p0);
#endif
          }
	*d++ = p0;  x++;  ixx += iaxx;
     }
}

static void
src_a8r8g8b8_scaled_y_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   DATA32 *p;
   int iayy = idata->iayy;
   int fw = idata->fw, fh = idata->fh;
   int iyy, iy;
   int ay;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }

   iyy = iayy * y;
   iy = iyy >> 16;
   ay = 1 + ((iyy >> 8) & 0xff);
   p = im + (iy * iw);
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(ay, mm3)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	if (((unsigned) x) < fw)
	  {
	    DATA32 p2 = p0 = *(p + x);

	    if ((iy + 1) < ih)
		p2 = *(p + iw + x);
	    if (p0 | p2)
#ifdef BUILD_MMX
	      {
		MOV_P2R(p0, mm1, mm0)
		MOV_P2R(p2, mm2, mm0)
		INTERP_256_R2R(mm3, mm2, mm1, mm5)
		MOV_R2P(mm1, p0, mm0)
	      }
#else
		p0 = INTERP_256(ay, p2, p0);
#endif
          }
	*d++ = p0;  x++;
     }
}

static void
src_a8r8g8b8_scaled_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
				int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   DATA32 *q;
   int iaxx = idata->iaxx, iayy = idata->iayy;
   int fw = idata->fw, fh = idata->fh;
   int ixx, iyy, iy;
   int ay;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }

   ixx = iaxx * x;  iyy = iayy * y;
   iy = iyy >> 16;
   q = im + (iy * iw);
   ay = 1 + ((iyy >> 8) & 0xff);
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(ay, mm4)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	if (((unsigned) x) < fw)
	  {
	    int  ix = ixx >> 16;
	    int  ax = 1 + ((ixx >> 8) & 0xff);
	    DATA32 *p = q + ix, p3, p2, p1;

	    p0 = p1 = p2 = p3 = *p;
	    if ((ix + 1) < iw)
		p1 = *(p + 1);
	    if ((iy + 1) < ih)
	      {
		p2 = *(p + iw);
		if ((ix + 1) < iw)
		   p3 = *(p + iw + 1);
	      }
	    if (p0 | p1 | p2 | p3)
#ifdef BUILD_MMX
	      {
		MOV_A2R(ax, mm6)
		MOV_P2R(p0, mm1, mm0)
		MOV_P2R(p1, mm2, mm0)
		INTERP_256_R2R(mm6, mm2, mm1, mm5)
		MOV_P2R(p2, mm2, mm0)
		MOV_P2R(p3, mm3, mm0)
		INTERP_256_R2R(mm6, mm3, mm2, mm5)
		INTERP_256_R2R(mm4, mm2, mm1, mm5)
		MOV_R2P(mm1, p0, mm0)
	      }
#else
	      {
		p0 = INTERP_256(ax, p1, p0);
		p2 = INTERP_256(ax, p3, p2);
		p0 = INTERP_256(ay, p2, p0);
	      }
#endif
          }
	*d++ = p0;  x++;  ixx += iaxx;
     }
}

static void
src_a8r8g8b8_scaled_dx_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx;
   int dax = idata->dax;
   int ibxx = idata->ibxx;
   int fw = idata->fw, fh = idata->fh;
   int axz = idata->axz;
   int ayz = idata->ayz;
   int ixx;
   DATA32 *q;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }

   ixx = (iaxx * x);
   q = im + (y * iw);
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(dax, mm4)
#endif
   while (d < e)
     {
#ifndef BUILD_MMX
	DATA32  p0 = 0;
#else
	pxor_r2r(mm1, mm1);
#endif
	if ((unsigned) x < fw)
	  {
	    int  ix;
	    int  txx, ntxx, tx, ntx;
	    DATA32 *p;
	    int  ay;

	    ix = ixx >> 16;
	    txx = (ibxx * ix);  tx = txx >> 16;
	    ntxx = txx + ibxx;  ntx = ntxx >> 16;
	    p = q + ix;
	    while (ix < iw)
	      {
		DATA32  p1 = *p;

#ifdef BUILD_MMX
		MOV_P2R(p1, mm2, mm0)
#endif
		if (ntx != tx)
		  {
		    int  ax;

		    if (ntx != x)
		      {
			ax = 256 - ((txx >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ax, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ax, p1);
#endif
			break;
		      }
		    ax = 1 + ((ntxx >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ax, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ax, p1);
#endif
		    tx = ntx;
		  }
		else if (p1 && (tx == x))
#ifdef BUILD_MMX
		  {
		    MUL4_256_R2R(mm4, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		    p0 += MUL_256(dax, p1);
#endif
		p++;  ix++;
		txx = ntxx;  ntxx += ibxx;  ntx = ntxx >> 16;
	      }
          }
#ifdef BUILD_MMX
	MOV_R2P(mm1, *d, mm0)
#else
	*d = p0;
#endif
	d++;  x++;  ixx += iaxx;
     }
}

static void
src_a8r8g8b8_scaled_dy_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iayy = idata->iayy;
   int day = idata->day;
   int ibyy = idata->ibyy;
   int fw = idata->fw, fh = idata->fh;
   int axz = idata->axz;
   int ayz = idata->ayz;
   int iyy0, iy0;
   int tyy0, ty0, ntyy0, nty0;
   DATA32 *q;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }

   iyy0 = (iayy * y);  iy0 = iyy0 >> 16;
   tyy0 = (ibyy * iy0);  ty0 = tyy0 >> 16;
   ntyy0 = tyy0 + ibyy;  nty0 = ntyy0 >> 16;
   q = im + (iy0 * iw);
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(day, mm4)
#endif
   while (d < e)
     {
#ifndef BUILD_MMX
	DATA32  p0 = 0;
#else
	pxor_r2r(mm1, mm1);
#endif
	if ((unsigned) x < fw)
	  {
	    int   iy = iy0;
	    int   tyy = tyy0, ntyy = ntyy0, ty = ty0, nty = nty0;
	    DATA32 *p = q + x;

	    while (iy < ih)
	      {
		DATA32 p2 = *p;
#ifdef BUILD_MMX
		MOV_P2R(p2, mm2, mm0)
#endif
		if (nty != ty)
		  {
		    int  ay;

		    if (nty != y)
		      {
			ay = 256 - ((tyy >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ay, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ay, p2);
#endif
			break;
		      }
		    ay = 1 + ((ntyy >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ay, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ay, p2);
#endif
		    ty = nty;
		  }
		else if (p2 && (ty == y))
#ifdef BUILD_MMX
		  {
		    MUL4_256_R2R(mm4, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		    p0 += MUL_256(day, p2);
#endif
		p += iw;  iy++;
		tyy = ntyy;  ntyy += ibyy;  nty = ntyy >> 16;
	      }
	  }
#ifdef BUILD_MMX
	MOV_R2P(mm1, *d, mm0)
#else
	*d = p0;
#endif
	d++;  x++;
     }
}

static void
src_a8r8g8b8_scaled_d_u_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx, iayy = idata->iayy;
   int dax = idata->dax;
   int ibxx = idata->ibxx;
   int fw = idata->fw, fh = idata->fh;
   int axz = idata->axz;
   int ayz = idata->ayz;
   int ixx, iyy, iy;
   int ay;
   DATA32 *q;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }

   ixx = (iaxx * x);
   iyy = (iayy * y);  iy = iyy >> 16;
   ay = 1 + ((iyy >> 8) & 0xff);
   q = im + (iy * iw);
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(dax, mm4)
   MOV_A2R(ay, mm6)
#endif
   while (d < e)
     {
#ifndef BUILD_MMX
	DATA32  p0 = 0;
#else
	pxor_r2r(mm1, mm1);
#endif
	if ((unsigned) x < fw)
	  {
	    int  ix;
	    int  txx, ntxx, tx, ntx;
	    DATA32 *p;

	    ix = ixx >> 16;
	    txx = (ibxx * ix);  tx = txx >> 16;
	    ntxx = txx + ibxx;  ntx = ntxx >> 16;
	    p = q + ix;
	    while (ix < iw)
	      {
		DATA32 p3, p1 = p3 = *p;

		if ((iy + 1) < ih)
		   p3 = *(p + iw);
#ifdef BUILD_MMX
		MOV_P2R(p1, mm2, mm0)
		if (p1 | p3)
		  {
		    MOV_P2R(p3, mm3, mm0)
		    INTERP_256_R2R(mm6, mm3, mm2, mm5)
		  }
#else
		if (p1 | p3)
		    p1 = INTERP_256(ay, p3, p1);
#endif
		if (ntx != tx)
		  {
		    int  ax;

		    if (ntx != x)
		      {
			ax = 256 - ((txx >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ax, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ax, p1);
#endif
			break;
		      }
		    ax = 1 + ((ntxx >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ax, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ax, p1);
#endif
		    tx = ntx;
		  }
#ifdef BUILD_MMX
		else if (tx == x)
		  {
		    MUL4_256_R2R(mm4, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		else if (p1 && (tx == x))
		    p0 += MUL_256(dax, p1);
#endif
		p++;  ix++;
		txx = ntxx;  ntxx += ibxx;  ntx = ntxx >> 16;
	      }
	  }
#ifdef BUILD_MMX
	MOV_R2P(mm1, *d, mm0)
#else
	*d = p0;
#endif
	d++;  x++;  ixx += iaxx;
     }
}

static void
src_a8r8g8b8_scaled_u_d_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx, iayy = idata->iayy;
   int day = idata->day;
   int ibyy = idata->ibyy;
   int fw = idata->fw, fh = idata->fh;
   int axz = idata->axz;
   int ayz = idata->ayz;
   int ixx, iyy0, iy0;
   int tyy0, ty0, ntyy0, nty0;
   DATA32 *q;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }

   ixx = (iaxx * x);
   iyy0 = (iayy * y);  iy0 = iyy0 >> 16;
   tyy0 = (ibyy * iy0);  ty0 = tyy0 >> 16;
   ntyy0 = tyy0 + ibyy;  nty0 = ntyy0 >> 16;
   q = im + (iy0 * iw);
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(day, mm4)
#endif
   while (d < e)
     {
#ifndef BUILD_MMX
	DATA32  p0 = 0;
#else
	pxor_r2r(mm1, mm1);
#endif
	if ((unsigned) x < fw)
	  {
	    int   ix, iy = iy0;
	    int   tyy = tyy0, ntyy = ntyy0, ty = ty0, nty = nty0;
	    DATA32 *p;
	    int  ax;

	    ix = ixx >> 16;
	    ax = 1 + ((ixx >> 8) & 0xff);
#ifdef BUILD_MMX
	    MOV_A2R(ax, mm6)
#endif
	    p = q + ix;
	    while (iy < ih)
	      {
		DATA32 p3, p2 = p3 = *p;

		if ((ix + 1) < iw)
		   p3 = *(p + 1);
#ifdef BUILD_MMX
		MOV_P2R(p2, mm2, mm0)
		if (p2 | p3)
		  {
		    MOV_P2R(p3, mm3, mm0)
		    INTERP_256_R2R(mm6, mm3, mm2, mm5)
		  }
#else
		if (p2 | p3)
		    p2 = INTERP_256(ax, p3, p2);
#endif
		if (nty != ty)
		  {
		    int  ay;

		    if (nty != y)
		      {
			ay = 256 - ((tyy >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ay, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ay, p2);
#endif
			break;
		      }
		    ay = 1 + ((ntyy >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ay, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ay, p2);
#endif
		    ty = nty;
		  }
#ifdef BUILD_MMX
		else if (ty == y)
		  {
		    MUL4_256_R2R(mm4, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		else if (p2 && (ty == y))
		    p0 += MUL_256(day, p2);
#endif
		p += iw;  iy++;
		tyy = ntyy;  ntyy += ibyy;  nty = ntyy >> 16;
	      }
	  }
#ifdef BUILD_MMX
	MOV_R2P(mm1, *d, mm0)
#else
	*d = p0;
#endif
	d++;  x++;  ixx += iaxx;
     }
}

static void
src_a8r8g8b8_scaled_d_d_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx, iayy = idata->iayy;
   int day = idata->day, dax = idata->dax;
   int ibyy = idata->ibyy, ibxx = idata->ibxx;
   int fw = idata->fw, fh = idata->fh;
   int axz = idata->axz;
   int ayz = idata->ayz;
   int ixx, iyy0, iy0;
   int tyy0, ty0, ntyy0, nty0;
   DATA32 *q;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }

   ixx = (iaxx * x);
   iyy0 = (iayy * y);  iy0 = iyy0 >> 16;
   tyy0 = (ibyy * iy0);  ty0 = tyy0 >> 16;
   ntyy0 = tyy0 + ibyy;  nty0 = ntyy0 >> 16;
   q = im + (iy0 * iw);
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(dax, mm4)
   MOV_A2R(day, mm7)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	if ((unsigned) x < fw)
	  {
	    int  ix, ix0, iy = iy0;
	    int  txx, txx0, ntxx, ntxx0, tx, tx0, ntx, ntx0;
	    int  tyy = tyy0, ntyy = ntyy0, ty = ty0, nty = nty0;
	    DATA32 *p, *ps;

	    ix0 = (ixx >> 16);
	    txx0 = (ibxx * ix0);  tx0 = (txx0 >> 16);
	    ntxx0 = (txx0 + ibxx);  ntx0 = (ntxx0 >> 16);
	    ps = q;
#ifdef BUILD_MMX
	    pxor_r2r(mm1, mm1);
#endif
	    while (iy < ih)
	      {
#ifdef BUILD_MMX
		pxor_r2r(mm2, mm2);
#else
		DATA32  p2 = 0;
#endif
		txx = txx0;  tx = tx0;
		ntxx = ntxx0;  ntx = ntx0;
		ix = ix0;  p = ps + ix;
		while (ix < iw)
		  {
		    DATA32  p3 = *p;

#ifdef BUILD_MMX
		    MOV_P2R(p3, mm6, mm0)
#endif
		    if (ntx != tx)
		      {
			int  ax;

			if (ntx != x)
			  {
			    ax = 256 - ((txx >> 8) & 0xff);
#ifdef BUILD_MMX
			    MOV_A2R(ax, mm3)
			    MUL4_256_R2R(mm3, mm6)
			    paddw_r2r(mm6, mm2);
#else
			    p2 += MUL_256(ax, p3);
#endif
			    break;
			  }
			ax = 1 + ((ntxx >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ax, mm3)
			MUL4_256_R2R(mm3, mm6)
			movq_r2r(mm6, mm2);
#else
			p2 = MUL_256(ax, p3);
#endif
			tx = ntx;
		      }
		    else if (p3 && (tx == x))
#ifdef BUILD_MMX
		      {
			MUL4_256_R2R(mm4, mm6)
			paddw_r2r(mm6, mm2);
		      }
#else
			p2 += MUL_256(dax, p3);
#endif
		    p++;  ix++;
		    txx = ntxx;  ntxx += ibxx;  ntx = ntxx >> 16;
		  }

		if (nty != ty)
		  {
		    int  ay;

		    if (nty != y)
		      {
			ay = 256 - ((tyy >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ay, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ay, p2);
#endif
			break;
		      }
		    ay = 1 + ((ntyy >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ay, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ay, p2);
#endif
		    ty = nty;
		  }
#ifdef BUILD_MMX
		else if (ty == y)
		  {
		    MUL4_256_R2R(mm7, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		else if (p2 && (ty == y))
		    p0 += MUL_256(day, p2);
#endif
		ps += iw;  iy++;
		tyy = ntyy;  ntyy += ibyy;  nty = ntyy >> 16;
	      }
	  }
	*d++ = p0;  x++;  ixx += iaxx;
     }
}

static void
src_a8r8g8b8_border_scaled_x_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx;
   int l = idata->l, r = idata->r;
   int fw = idata->fw, fh = idata->fh;
   int ixx;
   DATA32 *p;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }
   ixx = iaxx * (x - l);
   p = im + (y * iw);
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	if (((unsigned) x) < fw)
	  {
	    int  ix;
	    int  ax;
	    DATA32  p1;

	    ix = l + (ixx >> 16);
	    ax = 1 + ((ixx >> 8) & 0xff);
	    if ((x < l) || ((x + 2) > (fw - r)))
	      {
		ax = 0;
		ix = x;
		if ((x + 2) > (fw - r))
		   ix += (iw - fw);
	      }
	    p0 = p1 = *(p + ix);
	    if ((ix + 1) < iw)
		p1 = *(p + ix + 1);
	    if (ax && (p0 | p1))
#ifdef BUILD_MMX
	      {
		MOV_P2R(p0, mm1, mm0)
		MOV_P2R(p1, mm2, mm0)
		MOV_A2R(ax, mm3)
		INTERP_256_R2R(mm3, mm2, mm1, mm5)
		MOV_R2P(mm1, p0, mm0)
	      }
#else
		p0 = INTERP_256(ax, p1, p0);
#endif
	  }
	*d++ = p0;  x++;  ixx += iaxx;
     }
}

static void
src_a8r8g8b8_border_scaled_y_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iayy = idata->iayy;
   int t = idata->t, b = idata->b;
   int fh = idata->fh;
   int iyy, iy;
   int ay;
   DATA32 *p;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }
   if ((y < t) || ((y + 2) > (fh - b)))
     {
	if ((y + 2) > (fh - b))
	   y += (ih - fh);
	p = im + (y * iw) + x;
	while (d < e)
	  {
	    DATA32 p0 = 0;

	    if (((unsigned) x) < iw)
		p0 = *p;
	    *d++ = p0;  p++;  x++;
	  }
	return;
     }

   iyy = iayy * (y - t);
   iy = t + (iyy >> 16);
   ay = 1 + ((iyy >> 8) & 0xff);
   p = im + (iy * iw) + x;
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(ay, mm3)
#endif
   while (d < e)
     {
	DATA32 p0 = 0;

	if (((unsigned) x) < iw)
	  {
	    DATA32 p2 = p0 = *p;

	    if ((iy + 1) < ih)
		p2 = *(p + iw);
	    if (p0 | p2)
#ifdef BUILD_MMX
	      {
		MOV_P2R(p0, mm1, mm0)
		MOV_P2R(p2, mm2, mm0)
		INTERP_256_R2R(mm3, mm2, mm1, mm5)
		MOV_R2P(mm1, p0, mm0)
	      }
#else
		p0 = INTERP_256(ay, p2, p0);
#endif
	  }
	*d++ = p0;  p++;  x++;
     }
}

static void
src_a8r8g8b8_border_scaled_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx, iayy = idata->iayy;
   int l = idata->l, ll = (l << 16), r = idata->r;
   int t = idata->t, tt = (t << 16), b = idata->b;
   int fw = idata->fw, fh = idata->fh;
   int ixx, iyy, iy;
   int ay;
   DATA32 *q;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }

   ixx = iaxx * (x - l);
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   if ((y < t) || ((y + 2) > (fh - b)))
     {
	iy = y;
	if ((y + 2) > (fh - b))
	   iy += (ih - fh);

	q = im + (iy * iw);
	while (d < e)
	  {
	    DATA32  p0 = 0;

	    if (((unsigned) x) < fw)
	      {
		int  ix;
		int  ax;
		DATA32  p1;

		ix = l + (ixx >> 16);
		ax = 1 + ((ixx >> 8) & 0xff);
		if ((x < l) || ((x + 2) > (fw - r)))
		  {
		    ax = 0;
		    ix = x;
		    if ((x + 2) > (fw - r))
			ix += (iw - fw);
		  }

		p0 = *(q + ix);  p1 = *q;
		if (ix + 1 < iw)
		   p1 = *(q + ix + 1);

		if (ax && (p0 | p1))
#ifdef BUILD_MMX
		  {
		    MOV_P2R(p0, mm1, mm0)
		    MOV_P2R(p1, mm2, mm0)
		    MOV_A2R(ax, mm3)
		    INTERP_256_R2R(mm3, mm2, mm1, mm5)
		    MOV_R2P(mm1, p0, mm0)
		  }
#else
		    p0 = INTERP_256(ax, p1, p0);
#endif
	      }
	    *d++ = p0;  x++;  ixx += iaxx;
	  }
	return;
   }

   iyy = iayy * (y - t);
   ay = 1 + ((iyy >> 8) & 0xff);
   iy = t + (iyy >> 16);
   q = im + (iy * iw);
#ifdef BUILD_MMX
   MOV_A2R(ay, mm4)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	if (((unsigned) x) < fw)
	  {
	    int  ix;
	    int  ax;
	    DATA32 *p, p3, p2, p1;

	    ix = l + (ixx >> 16);
	    ax = 1 + ((ixx >> 8) & 0xff);
	    if ((x < l) || ((x + 2) > (fw - r)))
	      {
		ax = 0;
		ix = x;
		if ((x + 2) > (fw - r))
		   ix += (iw - fw);
	      }
	    p = q + ix;
	    p0 = p1 = p2 = p3 = *p;
	    if ((ix + 1) < iw)
		p1 = *(p + 1);
	    if ((iy + 1) < ih)
	      {
		p2 = *(p + iw);
		if ((ix + 1) < iw)
		   p3 = *(p + iw + 1);
	      }
	    if (p0 | p1 | p2 | p3)
#ifdef BUILD_MMX
	      {
		MOV_P2R(p0, mm1, mm0)
		MOV_P2R(p2, mm2, mm0)
		if (ax)
		  {
		    MOV_P2R(p1, mm3, mm0)
		    MOV_A2R(ax, mm6)
		    INTERP_256_R2R(mm6, mm3, mm1, mm5)
		    MOV_P2R(p3, mm3, mm0)
		    INTERP_256_R2R(mm6, mm3, mm2, mm5)
		  }
		INTERP_256_R2R(mm4, mm2, mm1, mm5)
		MOV_R2P(mm1, p0, mm0)
	      }
#else
	      {
		if (ax)
		  {
		    p0 = INTERP_256(ax, p1, p0);
		    p2 = INTERP_256(ax, p3, p2);
		  }
		p0 = INTERP_256(ay, p2, p0);
	      }
#endif
	  }
	*d++ = p0;  x++;  ixx += iaxx;
     }
}

static void
src_a8r8g8b8_border_scaled_d_u_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
						int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx, iayy = idata->iayy;
   int ibxx = idata->ibxx;
   int dax = idata->dax;
   int l = idata->l, ll = (l << 16), r = idata->r;
   int t = idata->t, tt = (t << 16), b = idata->b;
   int fw = idata->fw, fh = idata->fh;
   int iyy, iy;
   int ay;
   DATA32 *q;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }

   iyy = (iayy * (y - t));  iy = iyy >> 16;
   ay = 1 + ((iyy >> 8) & 0xff);
   iy += t;
   if ((y < t) || ((y + 2) > (fh - b)))
     {
	ay = 0;
	iy = y;
	if ((y + 2) > (fh - b))
	   iy += (ih - fh);
     }
   q = im + (iy * iw);
#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(ay, mm6)
   MOV_A2R(dax, mm4)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	if ((unsigned) x < fw)
	  {
	    int  ixx, ix;
	    int  txx, ntxx, tx, ntx;
	    DATA32 *p, p2;
	    int  ax;

	    if ((x < l) || ((x + 2) > (fw - r)))
	      {
		ax = 0;
		ix = x;
		if ((x + 2) > (fw - r))
		   ix += (iw - fw);

		p = q + ix;
		p0 = *p;
		if (ay)
		  {
		    p2 = p0;
		    if ((iy + 1) < ih)
			p2 = *(p + iw);
		    if (p0 | p2)
#ifdef BUILD_MMX
		      {
			MOV_P2R(p0, mm1, mm0)
			MOV_P2R(p2, mm2, mm0)
			INTERP_256_R2R(mm6, mm2, mm1, mm5)
			MOV_R2P(mm1, p0, mm0)
		      }
#else
			p0 = INTERP_256(ay, p2, p0);
#endif
		  }
		goto done;
	      }

#ifdef BUILD_MMX
	    pxor_r2r(mm1, mm1);
#endif
	    ixx = (iaxx * (x - l));  ix = ixx >> 16;
	    txx = (ibxx * ix) + ll;
	    tx = txx >> 16;
	    ntxx = txx + ibxx;  ntx = ntxx >> 16;
	    ix += l;
	    p = q + ix;
	    while (ix < iw)
	      {
		DATA32  p1 = p2 = *p;
#ifdef BUILD_MMX
		MOV_P2R(p1, mm2, mm0)
#endif
		if (ay && ((iy + 1) < ih))
		  {
		    p2 = *(p + iw);
		    if (p1 | p2)
#ifdef BUILD_MMX
		      {
			MOV_P2R(p2, mm3, mm0)
			INTERP_256_R2R(mm6, mm3, mm2, mm5)
		      }
#else
			p1 = INTERP_256(ay, p2, p1);
#endif
		  }

		if (ntx != tx)
		  {
		    if (ntx != x)
		      {
			ax = 256 - ((txx >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ax, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ax, p1);
#endif
			break;
		      }
		    ax = 1 + ((ntxx >> 8) & 0xff);
		    p0 = MUL_256(ax, p1);
		    tx = ntx;
		  }
#ifdef BUILD_MMX
		else if (tx == x)
	      {
		MUL4_256_R2R(mm4, mm2)
		paddw_r2r(mm2, mm1);
	      }
#else
		else if (p1 && (tx == x))
		    p0 += MUL_256(dax, p1);
#endif
		p++;  ix++;
		txx = ntxx;  ntxx += ibxx;  ntx = ntxx >> 16;
	      }
	  }
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	done:
	*d++ = p0;  x++;
     }
}

static void
src_a8r8g8b8_border_scaled_u_d_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
						int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx, iayy = idata->iayy;
   int ibyy = idata->ibyy;
   int day = idata->day;
   int l = idata->l, ll = (l << 16), r = idata->r;
   int t = idata->t, tt = (t << 16), b = idata->b;
   int fw = idata->fw, fh = idata->fh;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(day, mm4)
#endif
   if ((y < t) || ((y + 2) > (fh - b)))
     {
	DATA32 *p;
	int iy = y;

	if ((y + 2) > (fh - b))
	   iy += (ih - fh);
	p = im + (iy * iw);
	while (d < e)
	  {
	    DATA32  p0 = 0;

	    if ((unsigned) x < fw)
	      {
		int  ixx, ix;
		DATA32  p1;
		int  ax;

		ixx = (iaxx * (x - l));  ix = ixx >> 16;
		ax = 1 + ((ixx >> 8) & 0xff);
		ix += l;
		if ((x < l) || ((x + 2) > (fw - r)))
		  {
		    ax = 0;
		    ix = x;
		    if ((x + 2) > (fw - r))
			ix += (iw - fw);
		  }
		p0 = *(p + ix);
		if (ax)
		  {
		    p1 = p0;
		    if ((ix + 1) < iw)
			p1 = *(p + ix + 1);
		    if (p0 | p1)
#ifdef BUILD_MMX
		      {
			MOV_P2R(p0, mm1, mm0)
			MOV_P2R(p1, mm2, mm0)
			MOV_A2R(ax, mm3)
			INTERP_256_R2R(mm3, mm2, mm1, mm5)
			MOV_R2P(mm1, p0, mm0)
		      }
#else
			p0 = INTERP_256(ax, p1, p0);
#endif
		  }
	      }
	    *d++ = p0;  x++;
	  }
	return;
     }

   while (d < e)
     {
	DATA32 p0 = 0;

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
	if ((unsigned) x < fw)
	  {
	    int  iyy, ixx, ix, iy;
	    int  tyy, ntyy, ty, nty;
	    DATA32 *p, p3, p1;
	    int  ay, ax;

	    ixx = (iaxx * (x - l));  ix = ixx >> 16;
	    ax = 1 + ((ixx >> 8) & 0xff);
	    ix += l;
	    if ((x < l) || ((x + 2) > (fw - r)))
	      {
		ax = 0;
		ix = x;
		if ((x + 2) > (fw - r))
		   ix += (iw - fw);
	      }
	    iyy = (iayy * (y - t));  iy = iyy >> 16;
	    tyy = (ibyy * iy) + tt;
	    ty = tyy >> 16;
	    ntyy = tyy + ibyy;  nty = ntyy >> 16;
	    iy += t;
	    p = im + (iy * iw) + ix;
	    while (iy < ih)
	      {
		p3 = p1 = *p;
#ifdef BUILD_MMX
		MOV_P2R(p1, mm2, mm0)
#endif
		if (ax)
		  {
		    if ((ix + 1) < iw)
			p3 = *(p + 1);
		    if (p1 | p3)
#ifdef BUILD_MMX
		      {
			MOV_P2R(p3, mm3, mm0)
			MOV_A2R(ax, mm6)
			INTERP_256_R2R(mm6, mm3, mm2, mm5)
		      }
#else
			p1 = INTERP_256(ax, p3, p1);
#endif
		  }

		if (nty != ty)
		  {
		    if (nty != y)
		      {
			ay = 256 - ((tyy >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ay, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ay, p1);
#endif
			break;
		      }
		    ay = 1 + ((ntyy >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ay, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ay, p1);
#endif
		    ty = nty;
		  }
#ifdef BUILD_MMX
		else if (ty == y)
		  {
		    MUL4_256_R2R(mm4, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		else if (p1 && (ty == y))
		    p0 += MUL_256(day, p1);
#endif
		p += iw;  iy++;
		tyy = ntyy;  ntyy += ibyy;  nty = ntyy >> 16;
	      }
	  }
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	*d++ = p0;  x++;
     }
}

static void
src_a8r8g8b8_border_scaled_d_d_restrict_simple(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
						int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx, iayy = idata->iayy;
   int ibxx = idata->ibxx, ibyy = idata->ibyy;
   int dax = idata->dax, day = idata->day;
   int l = idata->l, ll = (l << 16), r = idata->r;
   int t = idata->t, tt = (t << 16), b = idata->b;
   int fw = idata->fw, fh = idata->fh;

   x += idata->axz >> 16;  y += idata->ayz >> 16;
   if ((y < 0) || (y >= fh))
     {
	while (d < e)
	   *d++ = 0;
	return;
     }

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(dax, mm4)
   MOV_A2R(day, mm5)
#endif
   if ((y < t) || ((y + 2) > (fh - b)))
     {
	DATA32 *q;
	int iy = y;

	if ((y + 2) > (fh - b))
	   iy += (ih - fh);
	q = im + (iy * iw);
	while (d < e)
	  {
	    int  ixx, ix;
	    int  txx, ntxx, tx, ntx;
	    DATA32  *p, p1, p0 = 0;
	    int  ax;

	    if ((unsigned) x < fw)
	      {
		if ((x < l) || ((x + 2) > (fw - r)))
		  {
		    ix = x;
		    if ((x + 2) > (fw - r))
			ix += (iw - fw);
		    p0 = *(q + ix);
		    goto done_t;
		  }
		ixx = iaxx * (x - l);  ix = ixx >> 16;
		txx = (ibxx * ix) + ll;
		tx = txx >> 16;
		ntxx = txx + ibxx;  ntx = ntxx >> 16;
		ix += l;
		p = q + ix;
#ifdef BUILD_MMX
		pxor_r2r(mm1, mm1);
#endif
		while (ix < iw)
		  {
		    p1 = *p;
#ifdef BUILD_MMX
		    MOV_P2R(p1, mm2, mm0)
#endif
		    if (ntx != tx)
		      {
			if (ntx != x)
			  {
			    ax = 256 - ((txx >> 8) & 0xff);
#ifdef BUILD_MMX
			    MOV_A2R(ax, mm3)
			    MUL4_256_R2R(mm3, mm2)
			    paddw_r2r(mm2, mm1);
#else
			    p0 += MUL_256(ax, p1);
#endif
			    break;
			  }
			ax = 1 + ((ntxx >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ax, mm3)
			MUL4_256_R2R(mm3, mm2)
			movq_r2r(mm2, mm1);
#else
			p0 = MUL_256(ax, p1);
#endif
			tx = ntx;
		      }
		    else if (p1 && (tx == x))
#ifdef BUILD_MMX
		      {
			MUL4_256_R2R(mm4, mm2)
			paddw_r2r(mm2, mm1);
		      }
#else
			p0 += MUL_256(dax, p1);
#endif
		    p++;  ix++;
		    txx = ntxx;  ntxx += ibxx;  ntx = ntxx >> 16;
		  }
	      }
#ifdef BUILD_MMX
	    MOV_R2P(mm1, p0, mm0)
#endif
	    done_t:
	    *d++ = p0;  x++;
	  }
	return;
     }

   while (d < e)
     {
	DATA32  p0 = 0;

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
	if ((unsigned) x < fw)
	  {
	    int  iyy, ixx, ix, ix0, iy;
	    int  txx, txx0, ntxx, ntxx0, tx, tx0, ntx, ntx0;
	    int  tyy, ntyy, ty, nty;
	    DATA32 *p, *ps, p3, p1;
	    int  ay, ax;

	    iyy = iayy * (y - t);  iy = iyy >> 16;
	    tyy = (ibyy * iy) + tt;
	    ty = tyy >> 16;
	    ntyy = tyy + ibyy;  nty = ntyy >> 16;
	    iy += t;
	    if ((x < l) || ((x + 2) > (fw - r)))
	      {
		ix = x;
		if ((x + 2) > (fw - r))
		   ix += (iw - fw);
		p = im + (iy * iw) + ix;
		while (iy < ih)
		  {
		    p1 = *p;
#ifdef BUILD_MMX
		    MOV_P2R(p1, mm2, mm0)
#endif
		    if (nty != ty)
		      {
			if (nty != y)
			  {
			    ay = 256 - ((tyy >> 8) & 0xff);
#ifdef BUILD_MMX
			    MOV_A2R(ay, mm3)
			    MUL4_256_R2R(mm3, mm2)
			    paddw_r2r(mm2, mm1);
#else
			    p0 += MUL_256(ay, p1);
#endif
			    break;
			  }
			ay = 1 + ((ntyy >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ay, mm3)
			MUL4_256_R2R(mm3, mm2)
			movq_r2r(mm2, mm1);
#else
			p0 = MUL_256(ay, p1);
#endif
			ty = nty;
		      }
		    else if (p1 && (ty == y))
#ifdef BUILD_MMX
		      {
			MUL4_256_R2R(mm5, mm2)
			paddw_r2r(mm2, mm1);
		      }
#else
			p0 += MUL_256(day, p1);
#endif
		    p += iw;  iy++;
		    tyy = ntyy;  ntyy += ibyy;  nty = ntyy >> 16;
		  }
		goto done;
	      }

	    ixx = iaxx * (x - l);  ix0 = ixx >> 16;
	    txx0 = (ibxx * ix0) + ll;
	    tx0 = (txx0 >> 16);
	    ntxx0 = (txx0 + ibxx);  ntx0 = (ntxx0 >> 16);
	    ps = im + (iy * iw);
	    ix0 += l;
	    while (iy < ih)
	      {
		txx = txx0;  tx = tx0;
		ntxx = ntxx0;  ntx = ntx0;
		ix = ix0;  p = ps + ix;
		p1 = 0;
#ifdef BUILD_MMX
		pxor_r2r(mm2, mm2);
#endif
		while (ix < iw)
		  {
		    p3 = *p;
#ifdef BUILD_MMX
		    MOV_P2R(p3, mm6, mm0)
#endif
		    if (ntx != tx)
		      {
			if (ntx != x)
			  {
			    ax = 256 - ((txx >> 8) & 0xff);
#ifdef BUILD_MMX
			    MOV_A2R(ax, mm3)
			    MUL4_256_R2R(mm3, mm6)
			    paddw_r2r(mm6, mm2);
#else
			    p1 += MUL_256(ax, p3);
#endif
			    break;
			  }
			ax = 1 + ((ntxx >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ax, mm3)
			MUL4_256_R2R(mm3, mm6)
			movq_r2r(mm6, mm2);
#else
			p1 = MUL_256(ax, p3);
#endif
			tx = ntx;
		      }
		    else if (p3 && (tx == x))
#ifdef BUILD_MMX
		      {
			MUL4_256_R2R(mm4, mm6)
			paddw_r2r(mm6, mm2);
		      }
#else
			p1 += MUL_256(dax, p3);
#endif
		    p++;  ix++;
		    txx = ntxx;  ntxx += ibxx;  ntx = ntxx >> 16;
		  }

		if (nty != ty)
		  {
		    if (nty != y)
		      {
			ay = 256 - ((tyy >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ay, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ay, p1);
#endif
			break;
		      }
		    ay = 1 + ((ntyy >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ay, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ay, p1);
#endif
		    ty = nty;
		  }
#ifdef BUILD_MMX
		else if (ty == y)
		  {
		    MUL4_256_R2R(mm5, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		else if (p1 && (ty == y))
		   p0 += MUL_256(day, p1);
#endif
		ps += iw;  iy++;
		tyy = ntyy;  ntyy += ibyy;  nty = ntyy >> 16;
	      }
	  }
	done:
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	*d++ = p0;  x++;
     }
}

/* also maybe border_scaled_dx/dy_restrict_simple */


/* with transform */

static void
src_a8r8g8b8_restrict(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
			int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	x = (xx >> 16);
	y = (yy >> 16);

	if ( (((unsigned) (x + 1)) < (iw + 1)) && (((unsigned) (y + 1)) < (ih + 1)) )
	  {
	    DATA32  p3 = 0, p2 = 0, p1 = 0;
	    DATA32 *p = im + (y * iw) + x;
	    int  ax, ay;

	    if ((x > -1) && (y > -1))
		p0 = *p;
	    if ((y > -1) && ((x + 1) < iw))
		p1 = *(p + 1);
	    if ((y + 1) < ih)
	      {
		if (x > -1)
		   p2 = *(p + iw);
		if ((x + 1) < iw)
		   p3 = *(p + iw + 1);
	      }

	    if (p0 | p1 | p2 | p3)
#ifdef BUILD_MMX
	      {
		ax = 1 + ((xx >> 8) & 0xff);
		MOV_A2R(ax, mm6)
		MOV_P2R(p0, mm1, mm0)
		MOV_P2R(p1, mm2, mm0)
		INTERP_256_R2R(mm6, mm2, mm1, mm5)
		MOV_P2R(p2, mm2, mm0)
		MOV_P2R(p3, mm3, mm0)
		INTERP_256_R2R(mm6, mm3, mm2, mm5)
		ay = 1 + ((yy >> 8) & 0xff);
		MOV_A2R(ay, mm6)
		INTERP_256_R2R(mm6, mm2, mm1, mm5)
		MOV_R2P(mm1, p0, mm0)
	      }
#else
	      {
		ax = 1 + ((xx >> 8) & 0xff);
		p0 = INTERP_256(ax, p1, p0);
		p2 = INTERP_256(ax, p3, p2);
		ay = 1 + ((yy >> 8) & 0xff);
		p0 = INTERP_256(ay, p2, p0);
	      }
#endif
          }
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_scaled_restrict(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
				int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   long long int iaxx = idata->iaxx, iayy = idata->iayy;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

 #ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
  while (d < e)
     {
	DATA32  p0 = 0;

	x = (xx >> 16);
	y = (yy >> 16);

	if ( (((unsigned) (x + 1)) < (fw + 1)) && (((unsigned) (y + 1)) < (fh + 1)) )
	  {
	    int  ixx, ix, iyy, iy;
	    int  ax, ay;
	    DATA32 *p, p3 = 0, p2 = 0, p1 = 0;

	    ixx = (iaxx * xx) >> 16;  ix = ixx >> 16;
	    iyy = (iayy * yy) >> 16;  iy = iyy >> 16;
	    ax = 1 + ((ixx >> 8) & 0xff);
	    ay = 1 + ((iyy >> 8) & 0xff);
	    if ((x < 0) || ((x + 2) > fw))
		ax = 1 + ((xx >> 8) & 0xff);
	    if ((y < 0) || ((y + 2) > fh))
		ay = 1 + ((yy >> 8) & 0xff);

	    p = im + (iy * iw) + ix;

	    if ((ix > -1) && (iy > -1))
		p0 = *p;
	    if ((iy > -1) && ((ix + 1) < iw))
		p1 = *(p + 1);
	    if ((iy + 1) < ih)
	      {
		if (ix > -1)
		   p2 = *(p + iw);
		if ((ix + 1) < iw)
		   p3 = *(p + iw + 1);
	      }
	    if (p0 | p1 | p2 | p3)
#ifdef BUILD_MMX
	      {
		MOV_A2R(ax, mm6)
		MOV_P2R(p0, mm1, mm0)
		MOV_P2R(p1, mm2, mm0)
		INTERP_256_R2R(mm6, mm2, mm1, mm5)
		MOV_P2R(p2, mm2, mm0)
		MOV_P2R(p3, mm3, mm0)
		INTERP_256_R2R(mm6, mm3, mm2, mm5)
		MOV_A2R(ay, mm6)
		INTERP_256_R2R(mm6, mm2, mm1, mm5)
		MOV_R2P(mm1, p0, mm0)
	      }
#else
	      {
		p0 = INTERP_256(ax, p1, p0);
		p2 = INTERP_256(ax, p3, p2);
		p0 = INTERP_256(ay, p2, p0);
	      }
#endif
          }
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_scaled_d_u_restrict(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
				int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   long long int iaxx = idata->iaxx, iayy = idata->iayy;
   int dax = idata->dax;
   int ibxx = idata->ibxx;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(dax, mm4)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	x = (xx >> 16);
	y = (yy >> 16);

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
	if ( (((unsigned) (x + 1)) < (fw + 1)) && (((unsigned) (y + 1)) < (fh + 1)) )
	  {
	    int  iyy, ixx, ix, iy;
	    int  txx, ntxx, tx, ntx;
	    DATA32 *p, p3 = 0, p1 = 0;
	    int  ay;

	    iyy = (iayy * yy) >> 16;  iy = iyy >> 16;
	    ixx = (iaxx * xx) >> 16;  ix = ixx >> 16;
	    txx = (ibxx * ix) - (xx & 0xffff);  tx = txx >> 16;
	    ntxx = txx + ibxx;  ntx = ntxx >> 16;
	    ay = 1 + ((iyy >> 8) & 0xff);
	    if ((y < 0) || ((y + 2) > fh))
		ay = 1 + ((yy >> 8) & 0xff);
	    p = im + (iy * iw) + ix;
#ifdef BUILD_MMX
	    MOV_A2R(ay, mm6)
#endif
	    while (ix < iw)
	      {
		p3 = p1 = 0;
		if (ix > -1)
		  {
		    if (iy > -1)
			p1 = *p;
		    if ((iy + 1) < ih)
			p3 = *(p + iw);
		  }
#ifdef BUILD_MMX
		MOV_P2R(p1, mm2, mm0)
		if (p1 | p3)
		  {
		    MOV_P2R(p3, mm3, mm0)
		    INTERP_256_R2R(mm6, mm3, mm2, mm5)
		  }
#else
		if (p1 | p3)
		    p1 = INTERP_256(ay, p3, p1);
#endif
		if (ntx != tx)
		  {
		    int  ax;

		    if (ntx != x)
		      {
			ax = 256 - ((txx >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ax, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ax, p1);
#endif
			break;
		      }
		    ax = 1 + ((ntxx >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ax, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ax, p1);
#endif
		    tx = ntx;
		 }
#ifdef BUILD_MMX
		else if (tx == x)
		  {
		    MUL4_256_R2R(mm4, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		else if (p1 && (tx == x))
		    p0 += MUL_256(dax, p1);
#endif
		p++;  ix++;
		txx = ntxx;  ntxx += ibxx;  ntx = ntxx >> 16;
	      }
          }
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_scaled_u_d_restrict(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
				int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   long long int iaxx = idata->iaxx, iayy = idata->iayy;
   int day = idata->day;
   int ibyy = idata->ibyy;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(day, mm4)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	x = (xx >> 16);
	y = (yy >> 16);

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
	if ( (((unsigned) (x + 1)) < (fw + 1)) && (((unsigned) (y + 1)) < (fh + 1)) )
	  {
	    int   ixx, iyy, ix, iy;
	    int   tyy, ntyy, ty, nty;
	    DATA32 *p, p3 = 0, p2 = 0;
	    int  ax;

	    ixx = (iaxx * xx) >> 16;  ix = ixx >> 16;
	    iyy = (iayy * yy) >> 16;  iy = iyy >> 16;
	    tyy = (ibyy * iy) - (yy & 0xffff);  ty = tyy >> 16;
	    ntyy = tyy + ibyy;  nty = ntyy >> 16;
	    ax = 1 + ((ixx >> 8) & 0xff);
	    if ((x < 0) || ((x + 2) > fw))
		ax = 1 + ((xx >> 8) & 0xff);
#ifdef BUILD_MMX
	    MOV_A2R(ax, mm6)
#endif
	    p = im + (iy * iw) + ix;

	    while (iy < ih)
	      {
		p3 = p2 = 0;
		if (iy > -1)
		  {
		    if (ix > -1)
			p2 = *p;
		    if ((ix + 1) < iw)
			p3 = *(p + 1);
		  }
#ifdef BUILD_MMX
		MOV_P2R(p2, mm2, mm0)
		if (p2 | p3)
		  {
		    MOV_P2R(p3, mm3, mm0)
		    INTERP_256_R2R(mm6, mm3, mm2, mm5)
		  }
#else
		if (p2 | p3)
		    p2 = INTERP_256(ax, p3, p2);
#endif
		if (nty != ty)
		  {
		    int  ay;

		    if (nty != y)
		      {
			ay = 256 - ((tyy >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ay, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ay, p2);
#endif
			break;
		      }
		    ay = 1 + ((ntyy >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ay, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ay, p2);
#endif
		    ty = nty;
		  }
#ifdef BUILD_MMX
		else if (ty == y)
		  {
		    MUL4_256_R2R(mm4, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		else if (p2 && (ty == y))
		    p0 += MUL_256(day, p2);
#endif
		p += iw;  iy++;
		tyy = ntyy;  ntyy += ibyy;  nty = ntyy >> 16;
	      }
          }
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_scaled_d_d_restrict(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
				int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   long long int iaxx = idata->iaxx, iayy = idata->iayy;
   int day = idata->day, dax = idata->dax;
   int ibyy = idata->ibyy, ibxx = idata->ibxx;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(dax, mm4)
   MOV_A2R(day, mm7)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	x = (xx >> 16);
	y = (yy >> 16);

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
	if ( (((unsigned) (x + 1)) < (fw + 1)) && (((unsigned) (y + 1)) < (fh + 1)) )
	  {
	    int  ixx, iyy, ix, ix0, iy;
	    int  txx, txx0, ntxx, ntxx0, tx, tx0, ntx, ntx0;
	    int  tyy, ntyy, ty, nty;
	    DATA32 *p, *ps, p3, p2;

	    ixx = ((iaxx * xx) >> 16);  ix0 = (ixx >> 16);
	    iyy = ((iayy * yy) >> 16);  iy = (iyy >> 16);
	    tyy = (ibyy * iy) - (yy & 0xffff);  ty = (tyy >> 16);
	    ntyy = tyy + ibyy;  nty = (ntyy >> 16);
	    txx0 = (ibxx * ix0) - (xx & 0xffff);  tx0 = (txx0 >> 16);
	    ntxx0 = (txx0 + ibxx);  ntx0 = (ntxx0 >> 16);
	    ps = im + (iy * iw);

	    while (iy < ih)
	      {
		txx = txx0;  tx = tx0;
		ntxx = ntxx0;  ntx = ntx0;
		ix = ix0;  p = ps + ix;
#ifdef BUILD_MMX
		pxor_r2r(mm2, mm2);
#else
		p2 = 0;
#endif
		while (ix < iw)
		  {
		    p3 = 0;
		    if ((ix > -1) && (iy > -1))
			p3 = *p;
#ifdef BUILD_MMX
		    MOV_P2R(p3, mm6, mm0)
#endif
		    if (ntx != tx)
		      {
			int  ax;

			if (ntx != x)
			  {
			    ax = 256 - ((txx >> 8) & 0xff);
#ifdef BUILD_MMX
			    MOV_A2R(ax, mm3)
			    MUL4_256_R2R(mm3, mm6)
			    paddw_r2r(mm6, mm2);
#else
			    p2 += MUL_256(ax, p3);
#endif
			    break;
			  }
			ax = 1 + ((ntxx >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ax, mm3)
			MUL4_256_R2R(mm3, mm6)
			movq_r2r(mm6, mm2);
#else
			p2 = MUL_256(ax, p3);
#endif
			tx = ntx;
		      }
		    else if (p3 && (tx == x))
#ifdef BUILD_MMX
		      {
			MUL4_256_R2R(mm4, mm6)
			paddw_r2r(mm6, mm2);
		      }
#else
			p2 += MUL_256(dax, p3);
#endif
		    p++;  ix++;
		    txx = ntxx;  ntxx += ibxx;  ntx = ntxx >> 16;
		  }

		if (nty != ty)
		  {
		    int  ay;

		    if (nty != y)
		      {
			ay = 256 - ((tyy >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ay, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ay, p2);
#endif
			break;
		      }
		    ay = 1 + ((ntyy >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ay, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ay, p2);
#endif
		    ty = nty;
		  }
#ifdef BUILD_MMX
		else if (ty == y)
		  {
		    MUL4_256_R2R(mm7, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		else if (p2 && (ty == y))
		    p0 += MUL_256(day, p2);
#endif
		ps += iw;  iy++;
		tyy = ntyy;  ntyy += ibyy;  nty = ntyy >> 16;
	      }
          }
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_border_scaled_restrict(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
				int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   long long int iaxx = idata->iaxx, iayy = idata->iayy;
   int ibyy = idata->ibyy, ibxx = idata->ibxx;
   int l = idata->l, ll = (l << 16), r = idata->r;
   int t = idata->t, tt = (t << 16), b = idata->b;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	x = (xx >> 16);
	y = (yy >> 16);

	if ( (((unsigned) (x + 1)) < (fw + 1)) && (((unsigned) (y + 1)) < (fh + 1)) )
	  {
	    int  ixx, ix, iyy, iy;
	    int  ax, ay;
	    DATA32 *p, p3 = 0, p2 = 0, p1 = 0;

	    ixx = (iaxx * (xx - ll)) >> 16;  ix = l + (ixx >> 16);
	    iyy = (iayy * (yy - tt)) >> 16;  iy = t + (iyy >> 16);
	    ax = 1 + ((ixx >> 8) & 0xff);
	    ay = 1 + ((iyy >> 8) & 0xff);

	    if ((x < l) || ((x + 2) > (fw - r)))
	      {
		ax = 1 + ((xx >> 8) & 0xff);
		ix = x;
		if ((x + 2) > (fw - r))
		   ix += (iw - fw);
	      }
	    if ((y < t) || ((y + 2) > (fh - b)))
	      {
		ay = 1 + ((yy >> 8) & 0xff);
		iy = y;
		if ((y + 2) > (fh - b))
		   iy += (ih - fh);
	      }

	    p = im + (iy * iw) + ix;

	    if ((ix > -1) && (iy > -1))
		p0 = *p;
	    if ((iy > -1) && ((ix + 1) < iw))
		p1 = *(p + 1);
	    if ((iy + 1) < ih)
	      {
		if (ix > -1)
		   p2 = *(p + iw);
		if ((ix + 1) < iw)
		   p3 = *(p + iw + 1);
	      }
	    if (p0 | p1 | p2 | p3)
#ifdef BUILD_MMX
	      {
		MOV_A2R(ax, mm6)
		MOV_P2R(p0, mm1, mm0)
		MOV_P2R(p1, mm2, mm0)
		INTERP_256_R2R(mm6, mm2, mm1, mm5)
		MOV_P2R(p2, mm2, mm0)
		MOV_P2R(p3, mm3, mm0)
		INTERP_256_R2R(mm6, mm3, mm2, mm5)
		MOV_A2R(ay, mm6)
		INTERP_256_R2R(mm6, mm2, mm1, mm5)
		MOV_R2P(mm1, p0, mm0)
	      }
#else
	      {
		p0 = INTERP_256(ax, p1, p0);
		p2 = INTERP_256(ax, p3, p2);
		p0 = INTERP_256(ay, p2, p0);
	      }
#endif
          }
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_border_scaled_d_u_restrict(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   long long int iaxx = idata->iaxx, iayy = idata->iayy;
   int ibxx = idata->ibxx;
   int dax = idata->dax;
   int l = idata->l, ll = (l << 16), r = idata->r;
   int t = idata->t, tt = (t << 16), b = idata->b;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(dax, mm4)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	x = (xx >> 16);
	y = (yy >> 16);

	if ( (((unsigned) (x + 1)) < (fw + 1)) && (((unsigned) (y + 1)) < (fh + 1)) )
	  {
	    int  iyy, ixx, ix, iy;
	    int  txx, ntxx, tx, ntx;
	    DATA32 *p, p3 = 0, p2 = 0, p1 = 0;
	    int  ay, ax;

	    iyy = (iayy * (yy - tt)) >> 16;  iy = iyy >> 16;
	    ay = 1 + ((iyy >> 8) & 0xff);
	    iy += t;
	    if ((y < t) || ((y + 2) > (fh - b)))
	      {
		ay = 1 + ((yy >> 8) & 0xff);
		iy = y;
		if ((y + 2) > (fh - b))
		   iy += (ih - fh);
	      }
#ifdef BUILD_MMX
	    MOV_A2R(ay, mm7)
#endif
	    if ((x < l) || ((x + 2) > (fw - r)))
	      {
		ix = x;
		if ((x + 2) > (fw - r))
		   ix += (iw - fw);

		p = im + (iy * iw) + ix;

		if ((ix > -1) && (iy > -1))
		   p0 = *p;
		if ((iy > -1) && ((ix + 1) < iw))
		   p1 = *(p + 1);
		if ((iy + 1) < ih)
		  {
		    if (ix > -1)
			p2 = *(p + iw);
		    if ((ix + 1) < iw)
			p3 = *(p + iw + 1);
	          }

		if (p0 | p1 | p2 | p3)
#ifdef BUILD_MMX
		  {
		    ax = 1 + ((xx >> 8) & 0xff);
		    MOV_A2R(ax, mm6)
		    MOV_P2R(p0, mm1, mm0)
		    MOV_P2R(p1, mm2, mm0)
		    INTERP_256_R2R(mm6, mm2, mm1, mm5)
		    MOV_P2R(p2, mm2, mm0)
		    MOV_P2R(p3, mm3, mm0)
		    INTERP_256_R2R(mm6, mm3, mm2, mm5)
		    INTERP_256_R2R(mm7, mm2, mm1, mm5)
		    MOV_R2P(mm1, p0, mm0)
		  }
#else
		  {
		    ax = 1 + ((xx >> 8) & 0xff);
		    p0 = INTERP_256(ax, p1, p0);
		    p2 = INTERP_256(ax, p3, p2);
		    p0 = INTERP_256(ay, p2, p0);
		  }
#endif
		goto done;
	      }

	    ixx = (iaxx * (xx - ll)) >> 16;  ix = ixx >> 16;
	    txx = (ibxx * ix) - (xx & 0xffff) + ll;
	    tx = txx >> 16;
	    ntxx = txx + ibxx;  ntx = ntxx >> 16;
	    ix += l;
	    p = im + (iy * iw) + ix;

#ifdef BUILD_MMX
	    pxor_r2r(mm1, mm1);
#endif
	    while (ix < iw)
	      {
		p1 = p3 = 0;
		if (ix > -1)
		  {
		    if (iy > -1)
			p1 = *p;
		    if ((iy + 1) < ih)
			p3 = *(p + iw);
		  }
#ifdef BUILD_MMX
		MOV_P2R(p1, mm2, mm0)
		if (p1 | p3)
		  {
		    MOV_P2R(p3, mm3, mm0)
		    INTERP_256_R2R(mm7, mm3, mm2, mm5)
		  }
#else
		if (p1 | p3)
		   p1 = INTERP_256(ay, p3, p1);
#endif
		if (ntx != tx)
		  {
		    if (ntx != x)
		      {
			ax = 256 - ((txx >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ax, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ax, p1);
#endif
			break;
		      }
		    ax = 1 + ((ntxx >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ax, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ax, p1);
#endif
		    tx = ntx;
		  }
#ifdef BUILD_MMX
		else if (tx == x)
		  {
		    MUL4_256_R2R(mm4, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		else if (p1 && (tx == x))
		    p0 += MUL_256(dax, p1);
#endif
		p++;  ix++;
		txx = ntxx;  ntxx += ibxx;  ntx = ntxx >> 16;
	      }
          }
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	done:
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_border_scaled_u_d_restrict(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   long long int iaxx = idata->iaxx, iayy = idata->iayy;
   int ibyy = idata->ibyy;
   int day = idata->day;
   int l = idata->l, ll = (l << 16), r = idata->r;
   int t = idata->t, tt = (t << 16), b = idata->b;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(day, mm4)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	x = (xx >> 16);
	y = (yy >> 16);

	if ( (((unsigned) (x + 1)) < (fw + 1)) && (((unsigned) (y + 1)) < (fh + 1)) )
	  {
	    int  iyy, ixx, ix, iy;
	    int  tyy, ntyy, ty, nty;
	    DATA32 *p, p3 = 0, p2 = 0, p1 = 0;
	    int  ay, ax;

	    ixx = (iaxx * (xx - ll)) >> 16;  ix = ixx >> 16;
	    ax = 1 + ((ixx >> 8) & 0xff);
	    ix += l;
	    if ((x < l) || ((x + 2) > (fw - r)))
	      {
		ax = 1 + ((xx >> 8) & 0xff);
		ix = x;
		if ((x + 2) > (fw - r))
		   ix += (iw - fw);
	      }
#ifdef BUILD_MMX
	    MOV_A2R(ax, mm7)
#endif
	    if ((y < t) || ((y + 2) > (fh - b)))
	      {
		iy = y;
		if ((y + 2) > (fh - b))
		   iy += (ih - fh);

		p = im + (iy * iw) + ix;

		if ((ix > -1) && (iy > -1))
		   p0 = *p;
		if ((iy > -1) && ((ix + 1) < iw))
		   p1 = *(p + 1);
		if ((iy + 1) < ih)
		  {
		    if (ix > -1)
			p2 = *(p + iw);
		    if ((ix + 1) < iw)
			p3 = *(p + iw + 1);
	          }

		if (p0 | p1 | p2 | p3)
#ifdef BUILD_MMX
		  {
		    MOV_P2R(p0, mm1, mm0)
		    MOV_P2R(p1, mm2, mm0)
		    INTERP_256_R2R(mm7, mm2, mm1, mm5)
		    MOV_P2R(p2, mm2, mm0)
		    MOV_P2R(p3, mm3, mm0)
		    INTERP_256_R2R(mm7, mm3, mm2, mm5)
		    ay = 1 + ((yy >> 8) & 0xff);
		    MOV_A2R(ay, mm6)
		    INTERP_256_R2R(mm6, mm2, mm1, mm5)
		    MOV_R2P(mm1, p0, mm0)
		  }
#else
		  {
		    p0 = INTERP_256(ax, p1, p0);
		    p2 = INTERP_256(ax, p3, p2);
		    ay = 1 + ((yy >> 8) & 0xff);
		    p0 = INTERP_256(ay, p2, p0);
		  }
#endif
		goto done;
	      }

	    iyy = (iayy * (yy - tt)) >> 16;  iy = iyy >> 16;
	    tyy = (ibyy * iy) - (yy & 0xffff) + tt;
	    ty = tyy >> 16;
	    ntyy = tyy + ibyy;  nty = ntyy >> 16;
	    iy += t;
	    p = im + (iy * iw) + ix;

#ifdef BUILD_MMX
	    pxor_r2r(mm1, mm1);
#endif
	    while (iy < ih)
	      {
		p1 = p3 = 0;
		if (iy > -1)
		  {
		    if (ix > -1)
			p1 = *p;
		    if ((ix + 1) < iw)
			p3 = *(p + 1);
		  }
#ifdef BUILD_MMX
		MOV_P2R(p1, mm2, mm0)
		if (p1 | p3)
		  {
		    MOV_P2R(p3, mm3, mm0)
		    INTERP_256_R2R(mm7, mm3, mm2, mm5)
		  }
#else
		if (p1 | p3)
		   p1 = INTERP_256(ax, p3, p1);
#endif
		if (nty != ty)
		  {
		    if (nty != y)
		      {
			ay = 256 - ((tyy >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ay, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ay, p1);
#endif
			break;
		      }
		    ay = 1 + ((ntyy >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ay, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ay, p1);
#endif
		    ty = nty;
		  }
#ifdef BUILD_MMX
		else if (ty == y)
		  {
		    MUL4_256_R2R(mm4, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		else if (p1 && (ty == y))
		    p0 += MUL_256(day, p1);
#endif
		p += iw;  iy++;
		tyy = ntyy;  ntyy += ibyy;  nty = ntyy >> 16;
	      }
          }
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	done:
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_border_scaled_d_d_restrict(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
					int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   long long int iaxx = idata->iaxx, iayy = idata->iayy;
   int ibxx = idata->ibxx, ibyy = idata->ibyy;
   int dax = idata->dax, day = idata->day;
   int l = idata->l, ll = (l << 16), r = idata->r;
   int t = idata->t, tt = (t << 16), b = idata->b;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(dax, mm4)
   MOV_A2R(day, mm7)
#endif
   while (d < e)
     {
	DATA32  p0 = 0;

	x = (xx >> 16);
	y = (yy >> 16);

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
	if ( (((unsigned) (x + 1)) < (fw + 1)) && (((unsigned) (y + 1)) < (fh + 1)) )
	  {
	    int  iyy, ixx, ix, ix0, iy;
	    int  txx, txx0, ntxx, ntxx0, tx, tx0, ntx, ntx0;
	    int  tyy, ntyy, ty, nty;
	    DATA32 *p, *ps, p3 = 0, p2 = 0, p1 = 0;
	    int  ay, ax;

	    if ((y < t) || ((y + 2) > (fh - b)))
	      {
		ay = 1 + ((yy >> 8) & 0xff);
		iy = y;
		if ((y + 2) > (fh - b))
		   iy += (ih - fh);
#ifdef BUILD_MMX
		MOV_A2R(ay, mm6)
#endif
		if ((x < l) || ((x + 2) > (fw - r)))
		  {
		    ix = x;
		    if ((x + 2) > (fw - r))
			ix += (iw - fw);

		    p = im + (iy * iw) + ix;

		    if ((ix > -1) && (iy > -1))
			p0 = *p;
		    if ((iy > -1) && ((ix + 1) < iw))
			p1 = *(p + 1);
		    if ((iy + 1) < ih)
		      {
			if (ix > -1)
			   p2 = *(p + iw);
			if ((ix + 1) < iw)
			   p3 = *(p + iw + 1);
		      }
#ifdef BUILD_MMX
		    MOV_P2R(p0, mm1, mm0)
		    if (p0 | p1 | p2 | p3)
		      {
			ax = 1 + ((xx >> 8) & 0xff);
			MOV_A2R(ax, mm3)
			MOV_P2R(p1, mm2, mm0)
			INTERP_256_R2R(mm3, mm2, mm1, mm5)
			MOV_P2R(p2, mm2, mm0)
			MOV_P2R(p3, mm4, mm0)
			INTERP_256_R2R(mm3, mm4, mm2, mm5)
			INTERP_256_R2R(mm6, mm2, mm1, mm5)
			MOV_A2R(dax, mm4)
		      }
#else
		    if (p0 | p1 | p2 | p3)
		      {
			ax = 1 + ((xx >> 8) & 0xff);
			p0 = INTERP_256(ax, p1, p0);
			p2 = INTERP_256(ax, p3, p2);
			p0 = INTERP_256(ay, p2, p0);
		      }
#endif
		    goto done;
		  }

		ixx = (iaxx * (xx - ll)) >> 16;  ix = ixx >> 16;
		txx = (ibxx * ix) - (xx & 0xffff) + ll;
		tx = txx >> 16;
		ntxx = txx + ibxx;  ntx = ntxx >> 16;
		ix += l;
		p = im + (iy * iw) + ix;
		while (ix < iw)
		  {
		    p1 = p3 = 0;
		    if (ix > -1)
		      {
			if (iy > -1)
			   p1 = *p;
			if ((iy + 1) < ih)
			   p3 = *(p + iw);
		      }
#ifdef BUILD_MMX
		    MOV_P2R(p1, mm2, mm0)
		    if (p1 | p3)
		      {
			MOV_P2R(p3, mm3, mm0)
			INTERP_256_R2R(mm6, mm3, mm2, mm5)
		      }
#else
		    if (p1 | p3)
			p1 = INTERP_256(ay, p3, p1);
#endif
		    if (ntx != tx)
		      {
			if (ntx != x)
			  {
			    ax = 256 - ((txx >> 8) & 0xff);
#ifdef BUILD_MMX
			    MOV_A2R(ax, mm3)
			    MUL4_256_R2R(mm3, mm2)
			    paddw_r2r(mm2, mm1);
#else
			    p0 += MUL_256(ax, p1);
#endif
			    break;
			  }
			ax = 1 + ((ntxx >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ax, mm3)
			MUL4_256_R2R(mm3, mm2)
			movq_r2r(mm2, mm1);
#else
			p0 = MUL_256(ax, p1);
#endif
			tx = ntx;
		      }
#ifdef BUILD_MMX
		    else if (tx == x)
		      {
			MUL4_256_R2R(mm4, mm2)
			paddw_r2r(mm2, mm1);
		      }
#else
		    else if (p1 && (tx == x))
			p0 += MUL_256(dax, p1);
#endif
		    p++;  ix++;
		    txx = ntxx;  ntxx += ibxx;  ntx = ntxx >> 16;
		  }
		    goto done;
	      }

	    iyy = (iayy * (yy - tt)) >> 16;  iy = iyy >> 16;
	    tyy = (ibyy * iy) - (yy & 0xffff) + tt;
	    ty = tyy >> 16;
	    ntyy = tyy + ibyy;  nty = ntyy >> 16;
	    iy += t;

	    if ((x < l) || ((x + 2) > (fw - r)))
	      {
		ax = 1 + ((xx >> 8) & 0xff);
#ifdef BUILD_MMX
		MOV_A2R(ax, mm6)
#endif
		ix = x;
		if ((x + 2) > (fw - r))
		   ix += (iw - fw);

		p = im + (iy * iw) + ix;
		while (iy < ih)
		  {
		    p1 = p3 = 0;
		    if (iy > -1)
		      {
			if (ix > -1)
			   p1 = *p;
			if ((ix + 1) < iw)
			   p3 = *(p + 1);
		      }
#ifdef BUILD_MMX
		    MOV_P2R(p1, mm2, mm0)
		    if (p1 | p3)
		      {
			MOV_P2R(p3, mm3, mm0)
			INTERP_256_R2R(mm6, mm3, mm2, mm5)
		      }
#else
		    if (p1 | p3)
			p1 = INTERP_256(ax, p3, p1);
#endif
		    if (nty != ty)
		      {
			if (nty != y)
			  {
			    ay = 256 - ((tyy >> 8) & 0xff);
#ifdef BUILD_MMX
			    MOV_A2R(ay, mm3)
			    MUL4_256_R2R(mm3, mm2)
			    paddw_r2r(mm2, mm1);
#else
			    p0 += MUL_256(ay, p1);
#endif
			    break;
			  }
			ay = 1 + ((ntyy >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ay, mm3)
			MUL4_256_R2R(mm3, mm2)
			movq_r2r(mm2, mm1);
#else
			p0 = MUL_256(ay, p1);
#endif
			ty = nty;
		      }
#ifdef BUILD_MMX
		    else if (ty == y)
		      {
			MUL4_256_R2R(mm7, mm2)
			paddw_r2r(mm2, mm1);
		      }
#else
		    else if (p1 && (ty == y))
			p0 += MUL_256(day, p1);
#endif
		    p += iw;  iy++;
		    tyy = ntyy;  ntyy += ibyy;  nty = ntyy >> 16;
		  }
		goto done;
	      }

	    ixx = (iaxx * (xx - ll)) >> 16;  ix0 = ixx >> 16;
	    txx0 = (ibxx * ix0) - (xx & 0xffff) + ll;
	    tx0 = (txx0 >> 16);
	    ntxx0 = (txx0 + ibxx);  ntx0 = (ntxx0 >> 16);
	    ps = im + (iy * iw);
	    ix0 += l;

	    while (iy < ih)
	      {
		txx = txx0;  tx = tx0;
		ntxx = ntxx0;  ntx = ntx0;
		ix = ix0;  p = ps + ix;
		p1 = 0;
#ifdef BUILD_MMX
		pxor_r2r(mm2, mm2);
#endif
		while (ix < iw)
		  {
		    p3 = 0;
		    if ((ix > -1) && (iy > -1))
			p3 = *p;
#ifdef BUILD_MMX
		    MOV_P2R(p3, mm6, mm0)
#endif
		    if (ntx != tx)
		      {
			if (ntx != x)
			  {
			    ax = 256 - ((txx >> 8) & 0xff);
#ifdef BUILD_MMX
			    MOV_A2R(ax, mm3)
			    MUL4_256_R2R(mm3, mm6)
			    paddw_r2r(mm6, mm2);
#else
			    p1 += MUL_256(ax, p3);
#endif
			    break;
			  }
			ax = 1 + ((ntxx >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ax, mm3)
			MUL4_256_R2R(mm3, mm6)
			movq_r2r(mm6, mm2);
#else
			p1 = MUL_256(ax, p3);
#endif
			tx = ntx;
		      }
		    else if (p3 && (tx == x))
#ifdef BUILD_MMX
		      {
			MUL4_256_R2R(mm4, mm6)
			paddw_r2r(mm6, mm2);
		      }
#else
			p1 += MUL_256(dax, p3);
#endif
		    p++;  ix++;
		    txx = ntxx;  ntxx += ibxx;  ntx = ntxx >> 16;
		  }

		if (nty != ty)
		  {
		    if (nty != y)
		      {
			ay = 256 - ((tyy >> 8) & 0xff);
#ifdef BUILD_MMX
			MOV_A2R(ay, mm3)
			MUL4_256_R2R(mm3, mm2)
			paddw_r2r(mm2, mm1);
#else
			p0 += MUL_256(ay, p1);
#endif
			break;
		      }
		    ay = 1 + ((ntyy >> 8) & 0xff);
#ifdef BUILD_MMX
		    MOV_A2R(ay, mm3)
		    MUL4_256_R2R(mm3, mm2)
		    movq_r2r(mm2, mm1);
#else
		    p0 = MUL_256(ay, p1);
#endif
		    ty = nty;
		  }
#ifdef BUILD_MMX
		else if (ty == y)
		  {
		    MUL4_256_R2R(mm7, mm2)
		    paddw_r2r(mm2, mm1);
		  }
#else
		else if (p1 && (ty == y))
		   p0 += MUL_256(day, p1);
#endif
		ps += iw;  iy++;
		tyy = ntyy;  ntyy += ibyy;  nty = ntyy >> 16;
	      }
	  }
	done:
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

/* also maybe [border_]scaled_x/y/dx/dy_restrict */


static Gfx_Func_Image_Fill image_restrict_affine_get(void *pim, int render_op)
{
   Evas_Object_Image *oim = pim;
   Gfx_Func_Image_Fill  func = NULL;
   int  src_scaled = 0, src_smooth = 0, src_border = 0;

   if ((oim->cur.image.w != oim->cur.fill.w) || (oim->cur.image.h != oim->cur.fill.h))
	src_scaled = 1;
   if (oim->cur.smooth_scale)
	src_smooth = 1;
   if ((oim->cur.image.w <= oim->cur.fill.w) && (oim->cur.image.h <= oim->cur.fill.h))
	src_smooth = 0;
   if ( (oim->cur.border.l > 0) || (oim->cur.border.r > 0) ||
        (oim->cur.border.t > 0) || (oim->cur.border.b > 0) )
	src_border = 1;
   if (!src_scaled && oim->cur.border.center_fill)
	src_border = 0;

   func = src_a8r8g8b8_restrict;
   if (oim->cur.fill.transform.is_identity)
     {
	func = src_a8r8g8b8_restrict_simple;
	if (src_scaled)
	  {
	    if (src_border)
	      {
		func = src_a8r8g8b8_border_scaled_restrict_simple;
		if (src_smooth)
		  {
		     if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h <= oim->cur.fill.h))
			return src_a8r8g8b8_border_scaled_d_u_restrict_simple;
		     if ((oim->cur.image.w <= oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
			return src_a8r8g8b8_border_scaled_u_d_restrict_simple;
		     if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
			return src_a8r8g8b8_border_scaled_d_d_restrict_simple;
		  }
		else  // scaled, bordered, not smooth or uu
		  {
		     if (oim->cur.image.h == oim->cur.fill.h)
			return src_a8r8g8b8_border_scaled_x_restrict_simple;
		     if (oim->cur.image.w == oim->cur.fill.w)
			return src_a8r8g8b8_border_scaled_y_restrict_simple;
		  }
		return func;
	      }
	    else  // scaled, no borders
	      {
		func = src_a8r8g8b8_scaled_restrict_simple;
		if (src_smooth)
		  {
		     if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h < oim->cur.fill.h))
			return src_a8r8g8b8_scaled_d_u_restrict_simple;
		     if ((oim->cur.image.w < oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
			return src_a8r8g8b8_scaled_u_d_restrict;
		     if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
			return src_a8r8g8b8_scaled_d_d_restrict_simple;
		     if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h == oim->cur.fill.h))
			return src_a8r8g8b8_scaled_dx_restrict_simple;
		     if ((oim->cur.image.w == oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
			return src_a8r8g8b8_scaled_dy_restrict_simple;
		  }
		else  // not smooth or uu
		  {
		     if (oim->cur.image.h == oim->cur.fill.h)
			return src_a8r8g8b8_scaled_x_restrict_simple;
		     if (oim->cur.image.w == oim->cur.fill.w)
			return src_a8r8g8b8_scaled_y_restrict_simple;
		  }
		return func;
	      }
	  }
	return func;
     }

   // non-identity transform
   if (src_scaled)
     {
	if (src_border)
	  {
	    func = src_a8r8g8b8_border_scaled_restrict;
	    if (src_smooth)
	      {
		if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h <= oim->cur.fill.h))
		   return src_a8r8g8b8_border_scaled_d_u_restrict;
		if ((oim->cur.image.w <= oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
		   return src_a8r8g8b8_border_scaled_u_d_restrict;
		if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
		   return src_a8r8g8b8_border_scaled_d_d_restrict;
	      }
	  }
	else  // scaled, no borders
	  {
	    func = src_a8r8g8b8_scaled_restrict;
	    if (src_smooth)
	      {
		if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h <= oim->cur.fill.h))
		   return src_a8r8g8b8_scaled_d_u_restrict;
		if ((oim->cur.image.w <= oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
		   return src_a8r8g8b8_scaled_u_d_restrict;
		if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
		   return src_a8r8g8b8_scaled_d_d_restrict;
	      }
	   }
     }
   return func;
}


/* masked restrict funcs */

/* FIXME: implement masked restrict (fairly straightforward) */

static Gfx_Func_Image_Fill image_restrict_masked_affine_get(void *pim, int render_op)
{
   return NULL;
}
