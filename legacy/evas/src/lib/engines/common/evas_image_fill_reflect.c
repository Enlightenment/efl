

/* reflect funcs */


/* FIXME: implement optimiztions and special-cases for this mode */


static void
src_a8r8g8b8_reflect(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
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
	int  ax = 1 + ((xx >> 8) & 0xff);
	int  ay = 1 + ((yy >> 8) & 0xff);
	DATA32  *p, p3, p2, p1, p0;

	x = (xx >> 16);  y = (yy >> 16);
	if (x < 0) { x = -x;  ax = 257 - ax; }
	if (x >= iw)
	  {
	    int  m = (x % (2 * iw));

	    x = (x % iw);
	    if (m >= iw)
	      { x = iw - x - 1;  ax = 257 - ax; }
	  }

	if (y < 0) { y = -y;  ay = 257 - ay; }
	if (y >= ih)
	  {
	    int  m = (y % (2 * ih));

	    y = (y % ih);
	    if (m >= ih)
	      { y = ih - y - 1;  ay = 257 - ay; }
	  }

	p = im + (y * iw) + x;
	p0 = p1 = p2 = p3 = *p;
	if ((x + 1) < iw)
	   p1 = *(p + 1);
	if ((y + 1) < ih)
	  {
	    p2 = *(p + iw);
	    if ((x + 1) < iw)
		p3 = *(p + iw + 1);
	  }
#ifdef BUILD_MMX
	if (p0 | p1 | p2 | p3)
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
	if (p0 | p1 | p2 | p3)
	  {
	    p0 = INTERP_256(ax, p1, p0);
	    p2 = INTERP_256(ax, p3, p2);
	    p0 = INTERP_256(ay, p2, p0);
	  }
#endif
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_scaled_reflect(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
			int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   long long int iaxx = idata->iaxx, iayy = idata->iayy;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;

   axx = (axx * iaxx) >> 16;  axy = (axy * iaxx) >> 16;
   axz = ((axz - 32768) * iaxx) >> 16;
   ayx = (ayx * iayy) >> 16;  ayy = (ayy * iayy) >> 16;
   ayz = ((ayz - 32768) * iayy) >> 16;
   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
#endif
   while (d < e)
     {
	int  ax = 1 + ((xx >> 8) & 0xff), ay = 1 + ((yy >> 8) & 0xff);
	DATA32  *p, p3, p2, p1, p0;

	x = (xx >> 16);  y = (yy >> 16);
	if (x < 0) { x = -x;  ax = 257 - ax; }
	if (x >= iw)
	  {
	    int  m = (x % (2 * iw));

	    x = (x % iw);
	    if (m >= iw)
	      { x = iw - x - 1;  ax = 257 - ax; }
	  }

	if (y < 0) { y = -y;  ay = 257 - ay; }
	if (y >= ih)
	  {
	    int  m = (y % (2 * ih));

	    y = (y % ih);
	    if (m >= ih)
	      { y = ih - y - 1;  ay = 257 - ay; }
	  }

	p = im + (y * iw) + x;
	p0 = p1 = p2 = p3 = *p;
	if ((x + 1) < iw)
	   p1 = *(p + 1);
	if ((y + 1) < ih)
	  {
	    p2 = *(p + iw);
	    if ((x + 1) < iw)
		p3 = *(p + iw + 1);
	  }

#ifdef BUILD_MMX
	if (p0 | p1 | p2 | p3)
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
	if (p0 | p1 | p2 | p3)
	  {
	    p0 = INTERP_256(ax, p1, p0);
	    p2 = INTERP_256(ax, p3, p2);
	    p0 = INTERP_256(ay, p2, p0);
	  }
#endif
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_scaled_d_u_reflect(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
				int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx, iayy = idata->iayy;
   int dax = idata->dax;
   int ibxx = idata->ibxx;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;
   int no_alpha = !idata->has_alpha;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(dax, mm4)
#endif
   while (d < e)
     {
	int  iyy, ixx, ix, iy;
	int  txx, ntxx, tx, ntx;
	DATA32 *p, p2, p1, p0 = 0;
	int  sgnx = 1, sgny = 1;
	int  ay;

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
	x = xx >> 16;  y = yy >> 16;
	if (x < 0) { x = -x;  sgnx *= -1; }
	if (x >= fw)
	  {
	    int  m = (x % (2 * fw));

	    x = (x % fw);
	    if (m >= fw)
	      { x = fw - x - 1;  sgnx *= -1; }
	  }

	if (y < 0) { y = -y;  sgny *= -1; }
	if (y >= fh)
	  {
	    int  m = (y % (2 * fh));

	    y = (y % fh);
	    if (m >= fh)
	      { y = fh - y - 1;  sgny *= -1; }
	  }

	iyy = (iayy * y);  iy = iyy >> 16;
	ixx = (iaxx * x);  ix = ixx >> 16;

	ay = 1 + ((iyy >> 8) & 0xff);
#ifdef BUILD_MMX
	MOV_A2R(ay, mm6)
#endif
	p = im + (iy * iw) + ix;

	txx = (ibxx * ix);  tx = txx >> 16;
	ntxx = txx + ibxx;  ntx = ntxx >> 16;

	while (ix < iw)
	  {
	    p1 = p2 = *p;
	    if (iy + 1 < ih) p2 = *(p + iw);

#ifdef BUILD_MMX
	    MOV_P2R(p1, mm2, mm0)
	    if (p1 | p2)
	      {
		MOV_P2R(p2, mm3, mm0)
		INTERP_256_R2R(mm6, mm3, mm2, mm5)
	      }
#else
	    if (p1 | p2)
		p1 = INTERP_256(ay, p2, p1);
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
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	if (no_alpha)  p0 |= 0xff000000;
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_scaled_u_d_reflect(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
				int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx, iayy = idata->iayy;
   int day = idata->day;
   int ibyy = idata->ibyy;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;
   int no_alpha = !idata->has_alpha;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(day, mm4)
#endif
   while (d < e)
     {
	int  ixx, iyy, ix, iy;
	int  tyy, ntyy, ty, nty;
	DATA32 *p, p3, p1, p0 = 0;
	int  sgnx = 1, sgny = 1;
	int  ax;

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
	x = (xx >> 16);  y = (yy >> 16);
	if (x < 0) { x = -x;  sgnx *= -1; }
	if (x >= fw)
	  {
	    int  m = (x % (2 * fw));

	    x = (x % fw);
	    if (m >= fw)
	      { x = fw - x - 1;  sgnx *= -1; }
	  }

	if (y < 0) { y = -y;  sgny *= -1; }
	if (y >= fh)
	  {
	    int  m = (y % (2 * fh));

	    y = (y % fh);
	    if (m >= fh)
	      { y = fh - y - 1;  sgny *= -1; }
	  }

	ixx = (iaxx * x);  ix = ixx >> 16;
	iyy = (iayy * y);  iy = iyy >> 16;

	ax = 1 + ((ixx >> 8) & 0xff);
#ifdef BUILD_MMX
	MOV_A2R(ax, mm6)
#endif
	p = im + (iy * iw) + ix;

	tyy = (ibyy * iy);  ty = tyy >> 16;
	ntyy = tyy + ibyy;  nty = ntyy >> 16;

	while (iy < ih)
	  {
	    p3 = p1 = *p;
	    if ((ix + 1) < iw) p3 = *(p + 1);

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
		int  ay;

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
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	if (no_alpha)  p0 |= 0xff000000;
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_scaled_d_d_reflect(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
				int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx, iayy = idata->iayy;
   int day = idata->day, dax = idata->dax;
   int ibyy = idata->ibyy, ibxx = idata->ibxx;
   int fw = idata->fw, fh = idata->fh;
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;
   int no_alpha = !idata->has_alpha;

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
	int  ixx, iyy, ix, ix0, iy;
	int  txx, txx0, ntxx, ntxx0, tx, tx0, ntx, ntx0;
	int  tyy, ntyy, ty, nty;
	DATA32 *p, *ps, p3, p2, p0 = 0;
	int  sgnx = 1, sgny = 1;

	x = (xx >> 16);  y = (yy >> 16);
	if (x < 0) { x = -x;  sgnx *= -1; }
	if (x >= fw)
	  {
	    int  m = (x % (2 * fw));

	    x = (x % fw);
	    if (m >= fw)
	      { x = fw - x - 1;  sgnx *= -1; }
	  }

	if (y < 0) { y = -y;  sgny *= -1; }
	if (y >= fh)
	  {
	    int  m = (y % (2 * fh));

	    y = (y % fh);
	    if (m >= fh)
	      { y = fh - y - 1;  sgny *= -1; }
	  }

	ixx = (iaxx * x);  ix0 = (ixx >> 16);
	iyy = (iayy * y);  iy = (iyy >> 16);

	ps = im + (iy * iw);

	tyy = (ibyy * iy);  ty = (tyy >> 16);
	ntyy = tyy + ibyy;  nty = (ntyy >> 16);

	txx0 = (ibxx * ix0);  tx0 = (txx0 >> 16);
	ntxx0 = (txx0 + ibxx);  ntx0 = (ntxx0 >> 16);

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
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
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	if (no_alpha)  p0 |= 0xff000000;
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_border_scaled_reflect(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
				int x, int y, void *data)
{
   Image_Draw_Data *idata = data;
   DATA32 *d = buf, *e = d + len;
   int iaxx = idata->iaxx, iayy = idata->iayy;
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
	int  ixx, ix, iyy, iy;
	int  ax, ay;
	int  sgnx = 1, sgny = 1;
	DATA32  *p, p3, p2, p1, p0;

	x = (xx >> 16);  y = (yy >> 16);
	if (x < 0) { x = -x;  sgnx *= -1; }
	if (x >= fw)
	  {
	    int  m = (x % (2 * fw));

	    x = (x % fw);
	    if (m >= fw)
	      { x = fw - x - 1;  sgnx *= -1; }
	  }

	if (y < 0) { y = -y;  sgny *= -1; }
	if (y >= fh)
	  {
	    int  m = (y % (2 * fh));

	    y = (y % fh);
	    if (m >= fh)
	      { y = fh - y - 1;  sgny *= -1; }
	  }

	ixx = (iaxx * (x - l));  ix = l + (ixx >> 16);
	iyy = (iayy * (y - t));  iy = t + (iyy >> 16);

	ax = 1 + ((ixx >> 8) & 0xff);
	ay = 1 + ((iyy >> 8) & 0xff);

	if ((x < l) || ((x + 2) > (fw - r)))
	  {
	    ax = 1 + ((xx >> 8) & 0xff);
	    ix = x;
	    if ((x + 2) > (fw - r))
		ix += (iw - fw);
	    if (sgnx < 0) ax = 257 - ax;
	  }
	if ((y < t) || ((y + 2) > (fh - b)))
	  {
	    ay = 1 + ((yy >> 8) & 0xff);
	    iy = y;
	    if ((y + 2) > (fh - b))
		iy += (ih - fh);
	    if (sgny < 0) ay = 257 - ay;
	  }

	p = im + (iy * iw) + ix;
	p0 = p1 = p2 = p3 = *p;
	if ((ix + 1) < iw)
	   p1 = *(p + 1);
	if ((iy + 1) < ih)
	  {
	    p2 = *(p + iw);
	    if ((ix + 1) < iw)
		p3 = *(p + iw + 1);
	  }

#ifdef BUILD_MMX
	if (p0 | p1 | p2 | p3)
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
	if (p0 | p1 | p2 | p3)
	  {
	    p0 = INTERP_256(ax, p1, p0);
	    p2 = INTERP_256(ax, p3, p2);
	    p0 = INTERP_256(ay, p2, p0);
	  }
#endif
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_border_scaled_d_u_reflect(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
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
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;
   int no_alpha = !idata->has_alpha && idata->border_center_fill;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(dax, mm4)
#endif
   while (d < e)
     {
	int  iyy, ixx, ix, iy;
	int  txx, ntxx, tx, ntx;
	DATA32  *p, p3, p2, p1, p0 = 0;
	int  sgnx = 1, sgny = 1;
	int  ay, ax;

	x = (xx >> 16);  y = (yy >> 16);
	if (x < 0) { x = -x;  sgnx *= -1; }
	if (x >= fw)
	  {
	    int  m = (x % (2 * fw));

	    x = (x % fw);
	    if (m >= fw)
	      { x = fw - x - 1;  sgnx *= -1; }
	  }

	if (y < 0) { y = -y;  sgny *= -1; }
	if (y >= fh)
	  {
	    int  m = (y % (2 * fh));

	    y = (y % fh);
	    if (m >= fh)
	      { y = fh - y - 1;  sgny *= -1; }
	  }

	iyy = (iayy * (y - t));  iy = t + (iyy >> 16);
	ay = 1 + ((iyy >> 8) & 0xff);

	if ((y < t) || ((y + 2) > (fh - b)))
	  {
	    ay = 1 + ((yy >> 8) & 0xff);
	    iy = y;
	    if ((y + 2) > (fh - b))
		iy += (ih - fh);
	    if (sgny < 0)  ay = 257 - ay;
	  }
#ifdef BUILD_MMX
	MOV_A2R(ay, mm7)
#endif
	if ((x < l) || ((x + 2) > (fw - r)))
	  {
	    ax = 1 + ((xx >> 8) & 0xff);
	    ix = x;
	    if ((x + 2) > (fw - r))
		ix += (iw - fw);
	    if (sgnx < 0)  ax = 257 - ax;

	    p = im + (iy * iw) + ix;
	    p0 = p1 = p2 = p3 = *p;
	    if ((ix + 1) < iw)
		p1 = *(p + 1);
	    if ((iy + 1) < ih)
	      {
		p2 = *(p + iw);
		if ((ix + 1) < iw)
		   p3 = *(p + iw + 1);
	      }

#ifdef BUILD_MMX
	    if (p0 | p1 | p2 | p3)
	      {
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
	    if (p0 | p1 | p2 | p3)
	      {
		p0 = INTERP_256(ax, p1, p0);
		p2 = INTERP_256(ax, p3, p2);
		p0 = INTERP_256(ay, p2, p0);
	      }
#endif
	    goto done;
	  }

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
	ixx = iaxx * (x - l);  ix = (ixx >> 16);
	txx = (ibxx * ix) + ll;  ix += l;
	tx = txx >> 16;
	ntxx = txx + ibxx;  ntx = ntxx >> 16;
	p = im + (iy * iw) + ix;

	while (ix < iw)
	  {
	    p3 = p1 = *p;
	    if (iy + 1 < ih) p3 = *(p + iw);
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
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	if (no_alpha)  p0 |= 0xff000000;
	done:
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_border_scaled_u_d_reflect(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
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
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;
   int no_alpha = !idata->has_alpha && idata->border_center_fill;

   xx = (axx * x) + (axx >> 1) + (axy * y) + (axy >> 1) + axz - 32768;
   yy = (ayx * x) + (ayx >> 1) + (ayy * y) + (ayy >> 1) + ayz - 32768;

#ifdef BUILD_MMX
   pxor_r2r(mm0, mm0);
   MOV_A2R(ALPHA_255, mm5)
   MOV_A2R(day, mm4)
#endif
   while (d < e)
     {
	int  iyy, ixx, ix, iy;
	int  tyy, ntyy, ty, nty;
	DATA32  *p, p3, p2, p1, p0 = 0;
	int  sgnx = 1, sgny = 1;
	int  ay, ax;

	x = (xx >> 16);  y = (yy >> 16);
	if (x < 0) { x = -x;  sgnx *= -1; }
	if (x >= fw)
	  {
	    int  m = (x % (2 * fw));

	    x = (x % fw);
	    if (m >= fw)
	      { x = fw - x - 1;  sgnx *= -1; }
	  }

	if (y < 0) { y = -y;  sgny *= -1; }
	if (y >= fh)
	  {
	    int  m = (y % (2 * fh));

	    y = (y % fh);
	    if (m >= fh)
	      { y = fh - y - 1;  sgny *= -1; }
	  }

	ixx = (iaxx * (x - l));  ix = l + (ixx >> 16);
	ax = 1 + ((ixx >> 8) & 0xff);
	if ((x < l) || ((x + 2) > (fw - r)))
	  {
	    ax = 1 + ((xx >> 8) & 0xff);
	    ix = x;
	    if ((x + 2) > (fw - r))
		ix += (iw - fw);
	    if (sgnx < 0)  ax = 257 - ax;
	  }
#ifdef BUILD_MMX
	MOV_A2R(ax, mm7)
#endif
	if ((y < t) || ((y + 2) > (fh - b)))
	  {
	    ay = 1 + ((yy >> 8) & 0xff);
	    iy = y;
	    if ((y + 2) > (fh - b))
		iy += (ih - fh);
	    if (sgny < 0)  ay = 257 - ay;

	    p = im + (iy * iw) + ix;
	    p0 = p1 = p2 = p3 = *p;
	    if ((ix + 1) < iw)
		p1 = *(p + 1);
	    if ((iy + 1) < ih)
	      {
		p2 = *(p + iw);
		if ((ix + 1) < iw)
		   p3 = *(p + iw + 1);
	      }
#ifdef BUILD_MMX
	    if (p0 | p1 | p2 | p3)
	      {
		MOV_P2R(p0, mm1, mm0)
		MOV_P2R(p1, mm2, mm0)
		INTERP_256_R2R(mm7, mm2, mm1, mm5)
		MOV_P2R(p2, mm2, mm0)
		MOV_P2R(p3, mm3, mm0)
		INTERP_256_R2R(mm7, mm3, mm2, mm5)
		MOV_A2R(ay, mm6)
		INTERP_256_R2R(mm6, mm2, mm1, mm5)
		MOV_R2P(mm1, p0, mm0)
	      }
#else
	    if (p0 | p1 | p2 | p3)
	      {
		p0 = INTERP_256(ax, p1, p0);
		p2 = INTERP_256(ax, p3, p2);
		p0 = INTERP_256(ay, p2, p0);
	      }
#endif
	    goto done;
	  }

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
	iyy = (iayy * (y - t));  iy = iyy >> 16;
	tyy = (ibyy * iy) + tt;  iy += t;
	ty = tyy >> 16;
	ntyy = tyy + ibyy;  nty = ntyy >> 16;
	p = im + (iy * iw) + ix;

	while (iy < ih)
	  {
	    p3 = p1 = *p;
	    if ((ix + 1) < iw) p3 = *(p + 1);
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
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	if (no_alpha)  p0 |= 0xff000000;
	done:
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}

static void
src_a8r8g8b8_border_scaled_d_d_reflect(DATA32 *im, int iw, int ih, DATA32 *buf, DATA8 *mask, int len,
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
   int axx = idata->axx, axy = idata->axy, axz = idata->axz;
   int ayx = idata->ayx, ayy = idata->ayy, ayz = idata->ayz;
   int xx, yy;
   int no_alpha = !idata->has_alpha && idata->border_center_fill;

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
	int  iyy, ixx, ix, ix0, iy;
	int  txx, txx0, ntxx, ntxx0, tx, tx0, ntx, ntx0;
	int  tyy, ntyy, ty, nty;
	DATA32  *p, *ps, p3, p2, p1, p0 = 0;
	int  sgnx = 1, sgny = 1;
	int  ay, ax;

	x = (xx >> 16);  y = (yy >> 16);
	if (x < 0) { x = -x;  sgnx *= -1; }
	if (x >= fw)
	  {
	    int  m = (x % (2 * fw));

	    x = (x % fw);
	    if (m >= fw)
	      { x = fw - x - 1;  sgnx *= -1; }
	  }

	if (y < 0) { y = -y;  sgny *= -1; }
	if (y >= fh)
	  {
	    int  m = (y % (2 * fh));

	    y = (y % fh);
	    if (m >= fh)
	      { y = fh - y - 1;  sgny *= -1; }
	  }

#ifdef BUILD_MMX
	pxor_r2r(mm1, mm1);
#endif
	if ((y < t) || ((y + 2) > (fh - b)))
	  {
	    ay = 1 + ((yy >> 8) & 0xff);
	    iy = y;
	    if ((y + 2) > (fh - b))
		iy += (ih - fh);
	    if (sgny < 0)  ay = 257 - ay;
#ifdef BUILD_MMX
	    MOV_A2R(ay, mm6)
#endif

	    if ((x < l) || ((x + 2) > (fw - r)))
	      {
		ax = 1 + ((xx >> 8) & 0xff);
		ix = x;
		if ((x + 2) > (fw - r))
		   ix += (iw - fw);
		if (sgnx < 0)  ax = 257 - ax;

		p = im + (iy * iw) + ix;
		p0 = p1 = p2 = p3 = *p;

		if ((ix + 1) < iw)
		   p1 = *(p + 1);
		if ((iy + 1) < ih)
		  {
		    p2 = *(p + iw);
		    if ((ix + 1) < iw)
			p3 = *(p + iw + 1);
		  }
#ifdef BUILD_MMX
		MOV_P2R(p0, mm1, mm0)
		if (p0 | p1 | p2 | p3)
		  {
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
		    p0 = INTERP_256(ax, p1, p0);
		    p2 = INTERP_256(ax, p3, p2);
		    p0 = INTERP_256(ay, p2, p0);
		  }
#endif
		goto done;
	      }

	    ixx = iaxx * (x - l);  ix = ixx >> 16;
	    txx = (ibxx * ix) + ll;  ix += l;
	    tx = txx >> 16;
	    ntxx = txx + ibxx;  ntx = ntxx >> 16;

	    p = im + (iy * iw) + ix;
	    while (ix < iw)
	      {
		p3 = p1 = *p;
		if (iy + 1 < ih) p3 = *(p + iw);
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

	iyy = iayy * (y - t);  iy = iyy >> 16;
	tyy = (ibyy * iy) + tt;  iy += t;
	ty = tyy >> 16;
	ntyy = tyy + ibyy;  nty = ntyy >> 16;

	if ((x < l) || ((x + 2) > (fw - r)))
	  {
	    ax = 1 + ((xx >> 8) & 0xff);
	    ix = x;
	    if ((x + 2) > (fw - r))
		ix += (iw - fw);
	    if (sgnx < 0)  ax = 257 - ax;
#ifdef BUILD_MMX
	    MOV_A2R(ax, mm6)
#endif

	    p = im + (iy * iw) + ix;
	    while (iy < ih)
	      {
		p3 = p1 = *p;
		if ((ix + 1) < iw) p3 = *(p + 1);
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

	ixx = iaxx * (x - l);  ix0 = ixx >> 16;
	txx0 = (ibxx * ix0) + ll;  ix0 += l;
	tx0 = (txx0 >> 16);
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
	    p1 = 0;
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
	done:
#ifdef BUILD_MMX
	MOV_R2P(mm1, p0, mm0)
#endif
	if (no_alpha)  p0 |= 0xff000000;
	*d++ = p0;  xx += axx;  yy += ayx;
     }
}



static Gfx_Func_Image_Fill image_reflect_affine_get(void *pim, int render_op)
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

   func = src_a8r8g8b8_reflect;
   if (src_scaled)
     {
	if (src_border)
	  {
	    func = src_a8r8g8b8_border_scaled_reflect;
	    if (src_smooth)
	      {
		if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h <= oim->cur.fill.h))
		   return src_a8r8g8b8_border_scaled_d_u_reflect;
		if ((oim->cur.image.w <= oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
		   return src_a8r8g8b8_border_scaled_u_d_reflect;
		if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
		   return src_a8r8g8b8_border_scaled_d_d_reflect;
	      }
	  }
	else
	  {
	    func = src_a8r8g8b8_scaled_reflect;
	    if (src_smooth)
	      {
		if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h <= oim->cur.fill.h))
		   return src_a8r8g8b8_scaled_d_u_reflect;
		if ((oim->cur.image.w <= oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
		   return src_a8r8g8b8_scaled_u_d_reflect;
		if ((oim->cur.image.w > oim->cur.fill.w) & (oim->cur.image.h > oim->cur.fill.h))
		   return src_a8r8g8b8_scaled_d_d_reflect;
	      }
	   }
     }

   return func;
}
