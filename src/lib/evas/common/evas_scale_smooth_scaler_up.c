{
   int         srx = src_region_x, sry = src_region_y;
   int         srw = src_region_w, srh = src_region_h;
   int         drx = dst_region_x, dry = dst_region_y;
   int         drw = dst_region_w, drh = dst_region_h;

   int         dsxx, dsyy, sxx, syy, sx, sy;
   int         cx, cy;
   int         direct_scale = 0, buf_step = 0;

   DATA32      *psrc, *pdst, *pdst_end;
   DATA32      *buf, *pbuf, *pbuf_end;
   DATA8       *mask;
   RGBA_Gfx_Func  func = NULL, func2 = NULL;

   /* check value  to make overflow(only check value related with overflow) */
   if ((src_region_w > SCALE_SIZE_MAX) ||
       (src_region_h > SCALE_SIZE_MAX)) return;

   /* a scanline buffer */
   pdst = dst_ptr;  // it's been set at (dst_clip_x, dst_clip_y)
   pdst_end = pdst + (dst_clip_h * dst_w);
   if (mul_col == 0xffffffff && !mask_ie)
     {
	if ((render_op == _EVAS_RENDER_BLEND) && !src->cache_entry.flags.alpha)
	  { direct_scale = 1;  buf_step = dst->cache_entry.w; }
	else if (render_op == _EVAS_RENDER_COPY)
	  {
	    direct_scale = 1;  buf_step = dst->cache_entry.w;
	    if (src->cache_entry.flags.alpha)
		dst->cache_entry.flags.alpha = 1;
	  }
     }
   if (!direct_scale)
     {
	buf = alloca(dst_clip_w * sizeof(DATA32));
        if (!mask_ie)
          {
             if (mul_col != 0xffffffff)
               func = evas_common_gfx_func_composite_pixel_color_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, mul_col, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
             else
               func = evas_common_gfx_func_composite_pixel_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
          }
        else
          {
             if (mul_col != 0xffffffff)
               {
                  func = evas_common_gfx_func_composite_pixel_mask_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
                  func2 = evas_common_gfx_func_composite_pixel_color_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, mul_col, dst->cache_entry.flags.alpha, dst_clip_w, EVAS_RENDER_COPY);
               }
             else
               func = evas_common_gfx_func_composite_pixel_mask_span_get(src->cache_entry.flags.alpha, src->cache_entry.flags.alpha_sparse, dst->cache_entry.flags.alpha, dst_clip_w, render_op);
          }
     }
   else
	buf = pdst;

   if ((srw > 1) && (drw > 1))
	dsxx = ((srw - 1) << 16) / (drw - 1);
   else
	dsxx = (srw << 16) / drw;
   if ((srh > 1) && (drh > 1))
	dsyy = ((srh - 1) << 16) / (drh - 1);
   else
	dsyy = (srh << 16) / drh;

   cx = dst_clip_x - drx;
   cy = dst_clip_y - dry;

   sxx = (dsxx * cx);
   syy = (dsyy * cy);

   sy = syy >> 16;

   if (drh == srh)
     {
	int  sxx0 = sxx;
        int y = 0;
	psrc = src->image.data + (src_w * (sry + cy)) + srx;
	while (pdst < pdst_end)
	  {
	    pbuf = buf;  pbuf_end = buf + dst_clip_w;
	    sxx = sxx0;
#ifdef SCALE_USING_MMX
	    pxor_r2r(mm0, mm0);
	    MOV_A2R(ALPHA_255, mm5)
#endif
	      while (pbuf < pbuf_end)
		{
		  DATA32   p0, p1;
		  int      ax;

		  sx = (sxx >> 16);
		  ax = 1 + ((sxx - (sx << 16)) >> 8);
		  p0 = p1 = *(psrc + sx);
		  if ((sx + 1) < srw)
		    p1 = *(psrc + sx + 1);
#ifdef SCALE_USING_MMX
		  MOV_P2R(p0, mm1, mm0)
		    if (p0 | p1)
		      {
			MOV_A2R(ax, mm3)
			MOV_P2R(p1, mm2, mm0)
			INTERP_256_R2R(mm3, mm2, mm1, mm5)
		       }
		  MOV_R2P(mm1, *pbuf, mm0)
		  pbuf++;
#else
		  if (p0 | p1)
		    p0 = INTERP_256(ax, p1, p0);
		  *pbuf++ = p0;
#endif
		  sxx += dsxx;
		}
	    /* * blend here [clip_w *] buf -> dptr * */
	    if (!direct_scale)
              {
                 if (!mask_ie)
                   func(buf, NULL, mul_col, pdst, dst_clip_w);
                 else
                   {
                      mask = mask_ie->image.data8
                         + ((dst_clip_y - mask_y + y) * mask_ie->cache_entry.w)
                         + (dst_clip_x - mask_x);

                      if (mul_col != 0xffffffff) func2(buf, NULL, mul_col, buf, dst_clip_w);
                      func(buf, mask, 0, pdst, dst_clip_w);
                   }
                 y++;
              }

	    pdst += dst_w;
	    psrc += src_w;
	    buf += buf_step;
	  }

	goto done_scale_up;
     }
   else if (drw == srw)
     {
	DATA32  *ps = src->image.data + (src_w * sry) + srx + cx;
        int y = 0;

	while (pdst < pdst_end)
	  {
	    int        ay;

	    sy = syy >> 16;
	    psrc = ps + (sy * src_w);
	    ay = 1 + ((syy - (sy << 16)) >> 8);
#ifdef SCALE_USING_MMX
	    pxor_r2r(mm0, mm0);
	    MOV_A2R(ALPHA_255, mm5)
	    MOV_A2R(ay, mm4)
#endif
	    pbuf = buf;  pbuf_end = buf + dst_clip_w;
	    while (pbuf < pbuf_end)
	      {
		DATA32  p0 = *psrc, p2 = p0;

		if ((sy + 1) < srh)
		  p2 = *(psrc + src_w);
#ifdef SCALE_USING_MMX
		MOV_P2R(p0, mm1, mm0)
		if (p0 | p2)
		  {
		    MOV_P2R(p2, mm2, mm0)
		    INTERP_256_R2R(mm4, mm2, mm1, mm5)
		  }
		MOV_R2P(mm1, *pbuf, mm0)
		pbuf++;
#else
		if (p0 | p2)
		  p0 = INTERP_256(ay, p2, p0);
		*pbuf++ = p0;
#endif
		psrc++;
	      }
	    /* * blend here [clip_w *] buf -> dptr * */
	    if (!direct_scale)
              {
                 if (!mask_ie)
                   func(buf, NULL, mul_col, pdst, dst_clip_w);
                 else
                   {
                      mask = mask_ie->image.data8
                         + ((dst_clip_y - mask_y + y) * mask_ie->cache_entry.w)
                         + (dst_clip_x - mask_x);

                      if (mul_col != 0xffffffff) func2(buf, NULL, mul_col, buf, dst_clip_w);
                      func(buf, mask, 0, pdst, dst_clip_w);
                   }
                 y++;
              }
	    pdst += dst_w;
	    syy += dsyy;
	    buf += buf_step;
	  }
	goto done_scale_up;
     }

     {
	DATA32  *ps = src->image.data + (src_w * sry) + srx;
	int     sxx0 = sxx;
        int     y = 0;

	while (pdst < pdst_end)
	  {
	    int   ay;

	    sy = syy >> 16;
	    psrc = ps + (sy * src_w);
	    ay = 1 + ((syy - (sy << 16)) >> 8);
#ifdef SCALE_USING_MMX
	    MOV_A2R(ay, mm4)
	    pxor_r2r(mm0, mm0);
	    MOV_A2R(ALPHA_255, mm5)
#elif defined SCALE_USING_NEON
            uint16x8_t vay = vdupq_n_u16(ay);
#endif
	    pbuf = buf;  pbuf_end = buf + dst_clip_w;
	    sxx = sxx0;
#ifdef SCALE_USING_NEON
	    while (pbuf+1 < pbuf_end) // 2 iterations only for NEON
#else
	    while (pbuf < pbuf_end)
#endif
	      {
		int     ax;
		DATA32  *p, *q;
#ifdef SCALE_USING_NEON
                int     ax1;
                DATA32  *p1, *q1;
                uint32x2x2_t vp0, vp1;
                uint16x8_t vax;
                uint16x8_t vax1;
                DATA32 pa[2][4];
#else
		DATA32  p0, p1, p2, p3;
#endif

		sx = sxx >> 16;
		ax = 1 + ((sxx - (sx << 16)) >> 8);
		p = psrc + sx;  q = p + src_w;
#ifdef SCALE_USING_NEON
                pa[0][0] = pa[0][1] = pa[0][2] = pa[0][3] = *p;
                if ((sx + 1) < srw)
                  pa[0][1] = *(p + 1);
                if ((sy + 1) < srh)
                  {
                    pa[0][2] = *q;  pa[0][3] = pa[0][2];
                    if ((sx + 1) < srw)
                      pa[0][3] = *(q + 1);
                  }
                vax = vdupq_n_u16(ax);
                vp0.val[0] = vld1_u32(&pa[0][0]);
                vp0.val[1] = vld1_u32(&pa[0][2]);
                sxx += dsxx;
                sx = sxx >> 16;
                ax1 = 1 + ((sxx - (sx << 16)) >> 8);
                p1 = psrc + sx; q1 = p1 + src_w;
                pa[1][0] = pa[1][1] = pa[1][2] = pa[1][3] = *p1;
                if ((sx + 1) < srw)
                  pa[1][1] = *(p1 + 1);
                if ((sy + 1) < srh)
                  {
                    pa[1][2] = *q1;  pa[1][3] = pa[1][2];
                    if ((sx + 1) < srw)
                      pa[1][3] = *(q1 + 1);
                  }
                vax1 = vdupq_n_u16(ax1);
                vp1.val[0] = vld1_u32(&pa[1][0]);
                vp1.val[1] = vld1_u32(&pa[1][2]);
#else
		p0 = p1 = p2 = p3 = *p;
		if ((sx + 1) < srw)
		  p1 = *(p + 1);
		if ((sy + 1) < srh)
		  {
		    p2 = *q;  p3 = p2;
		    if ((sx + 1) < srw)
		      p3 = *(q + 1);
		  }
#endif
#ifdef SCALE_USING_MMX
		MOV_A2R(ax, mm6)
		MOV_P2R(p0, mm1, mm0)
		if (p0 | p1)
		  {
		    MOV_P2R(p1, mm2, mm0)
		    INTERP_256_R2R(mm6, mm2, mm1, mm5)
		  }
		MOV_P2R(p2, mm2, mm0)
		if (p2 | p3)
		  {
		    MOV_P2R(p3, mm3, mm0)
		    INTERP_256_R2R(mm6, mm3, mm2, mm5)
		  }
		INTERP_256_R2R(mm4, mm2, mm1, mm5)
		MOV_R2P(mm1, *pbuf, mm0)
		pbuf++;
#elif defined SCALE_USING_NEON
                    // (p0, p1), (p2, p3) ==> (p0, p2), (p1, p3)
                vp0 = vzip_u32(vp0.val[0], vp0.val[1]);
                    // (p1 - p0, p3 - p2)
                uint16x8_t vtmpq = vsubl_u8(vreinterpret_u8_u32(vp0.val[1]), vreinterpret_u8_u32(vp0.val[0]));
                    // p0 + (p1 - p0)*ax, p2 + (p3 - p2)*ax
                vp0.val[0] = vreinterpret_u32_u8(vadd_u8(vreinterpret_u8_u32(vp0.val[0]), vshrn_n_u16(vmulq_u16(vtmpq, vax), 8)));
                vp1 = vzip_u32(vp1.val[0], vp1.val[1]);
                vtmpq = vsubl_u8(vreinterpret_u8_u32(vp1.val[1]), vreinterpret_u8_u32(vp1.val[0]));
                vp1.val[0] = vreinterpret_u32_u8(vadd_u8(vreinterpret_u8_u32(vp1.val[0]), vshrn_n_u16(vmulq_u16(vtmpq, vax1), 8)));
                    // (p0, p2), (p4, p6) ==> (p0, p4), (p2, p6)
                vp0 = vzip_u32(vp0.val[0], vp1.val[0]);
                    // (p2 - p0), (p6 - p4)
                vtmpq = vsubl_u8(vreinterpret_u8_u32(vp0.val[1]), vreinterpret_u8_u32(vp0.val[0]));
                    // p0 + (p2 - p0)*ay, p4 + (p6 - p4)*ay
                vp0.val[0] = vreinterpret_u32_u8(vadd_u8(vreinterpret_u8_u32(vp0.val[0]), vshrn_n_u16(vmulq_u16(vtmpq, vay), 8)));
                vst1_u32(pbuf, vp0.val[0]);
                pbuf += 2;
#else
		if (p0 | p1)
		  p0 = INTERP_256(ax, p1, p0);
		if (p2 | p3)
		  p2 = INTERP_256(ax, p3, p2);
		if (p0 | p2)
		  p0 = INTERP_256(ay, p2, p0);
		*pbuf++ = p0;
#endif
		sxx += dsxx;
	      }
#if defined SCALE_USING_NEON
              if (pbuf < pbuf_end) // For non-even length case
                {
                  int     ax;
                  DATA32  *p, *q;
                  DATA32  p0, p1, p2, p3;

                  sx = sxx >> 16;
                  ax = 1 + ((sxx - (sx << 16)) >> 8);
                  p = psrc + sx;  q = p + src_w;
                  p0 = p1 = p2 = p3 = *p;
                  if ((sx + 1) < srw)
                    p1 = *(p + 1);
                  if ((sy + 1) < srh)
                    {
                       p2 = *q;  p3 = p2;
                       if ((sx + 1) < srw)
                         p3 = *(q + 1);
                    }
                  if (p0 | p1)
                    p0 = INTERP_256(ax, p1, p0);
                  if (p2 | p3)
                    p2 = INTERP_256(ax, p3, p2);
                  if (p0 | p2)
                    p0 = INTERP_256(ay, p2, p0);
                  *pbuf++ = p0;
                  sxx += dsxx;
                }
#endif
	    /* * blend here [clip_w *] buf -> dptr * */
	    if (!direct_scale)
              {
                 if (!mask_ie)
                   func(buf, NULL, mul_col, pdst, dst_clip_w);
                 else
                   {
                      mask = mask_ie->image.data8
                         + ((dst_clip_y - mask_y + y) * mask_ie->cache_entry.w)
                         + (dst_clip_x - mask_x);

                      if (mul_col != 0xffffffff) func2(buf, NULL, mul_col, buf, dst_clip_w);
                      func(buf, mask, 0, pdst, dst_clip_w);
                   }
                 y++;
              }

	    pdst += dst_w;
	    syy += dsyy;
	    buf += buf_step;
	  }
     }
   done_scale_up:
   return;
}
