#define BILINEAR_HALF_TO_FULL_SCALE 1

{
   int Cx, Cy, i, j;
   DATA32 *dptr, *sptr, *pix, *pbuf;
   DATA8 *mask;
   int a, r, g, b, rx, gx, bx, ax;
   int xap, yap, pos;
   int y = 0;
#ifdef BILINEAR_HALF_TO_FULL_SCALE
   Eina_Bool bilinear_downscale = EINA_FALSE;
#endif

   DATA32  **yp;
   int *xp;
   int w = dst_clip_w;

#ifdef BILINEAR_HALF_TO_FULL_SCALE
   if (// image is not too big so that cululative error on steps might be
       // noticeable
       (dst_region_w <= 4096) &&
       (dst_region_h <= 4096) &&
       (src_region_w <= 4096) &&
       (src_region_h <= 4096) &&
       // if image scale is between 50% size and up to 100% of size of
       // original, then do a special case bilinear interplation scale
       (dst_region_w >= (src_region_w / 2)) &&
       (dst_region_h >= (src_region_h / 2)))
     bilinear_downscale = EINA_TRUE;
#endif

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
#ifdef BILINEAR_HALF_TO_FULL_SCALE
        if (bilinear_downscale)
          {
             DATA32 *lptr, *p1, *p2, *p3, *p4;
             unsigned int xpos, ypos, xfrac, yfrac, invxfrac, invyfrac;
             unsigned int xstep, ystep;
             unsigned int a1, a2, r1, g1, b1, r2, g2, b2;

             pix = src_data + (src_region_y * src_w) + src_region_x;
             xstep = (src_region_w << 16) / dst_region_w;
             ystep = (src_region_h << 16) / dst_region_h;
             ypos = (dst_clip_y - dst_region_y) * ystep;

             while (dst_clip_h--)
               {
                  xpos = (dst_clip_x - dst_region_x) * xstep;
                  lptr = pix + ((ypos >> 16) * src_w);

                  if ((ypos >> 16) < ((unsigned int)src_h - 1))
                    {
                       yfrac = ypos & 0xffff;
                       invyfrac = 0x10000 - yfrac;
                       while (dst_clip_w--)
                         {
                            p1 = lptr + (xpos >> 16);
                            p2 = p1 + 1;
                            p3 = p1 + src_w;
                            p4 = p3 + 1;
                            xfrac = xpos & 0xffff;
                            invxfrac = 0x10000 - xfrac;

                            if (xfrac > 0)
                              {
                                 a1 = ((invxfrac * A_VAL(p1)) + (xfrac * A_VAL(p2))) >> 16;
                                 a2 = ((invxfrac * A_VAL(p3)) + (xfrac * A_VAL(p4))) >> 16;
                                 r1 = ((invxfrac * R_VAL(p1)) + (xfrac * R_VAL(p2))) >> 16;
                                 r2 = ((invxfrac * R_VAL(p3)) + (xfrac * R_VAL(p4))) >> 16;
                                 g1 = ((invxfrac * G_VAL(p1)) + (xfrac * G_VAL(p2))) >> 16;
                                 g2 = ((invxfrac * G_VAL(p3)) + (xfrac * G_VAL(p4))) >> 16;
                                 b1 = ((invxfrac * B_VAL(p1)) + (xfrac * B_VAL(p2))) >> 16;
                                 b2 = ((invxfrac * B_VAL(p3)) + (xfrac * B_VAL(p4))) >> 16;
                              }
                            else
                              {
                                 a1 = A_VAL(p1);
                                 a2 = A_VAL(p3);
                                 r1 = R_VAL(p1);
                                 r2 = R_VAL(p3);
                                 g1 = G_VAL(p1);
                                 g2 = G_VAL(p3);
                                 b1 = B_VAL(p1);
                                 b2 = B_VAL(p3);
                              }

                            a = ((invyfrac * a1) + (yfrac * a2)) >> 16;
                            r = ((invyfrac * r1) + (yfrac * r2)) >> 16;
                            g = ((invyfrac * g1) + (yfrac * g2)) >> 16;
                            b = ((invyfrac * b1) + (yfrac * b2)) >> 16;

                            *pbuf++ = ARGB_JOIN(a, r, g, b);
                            xpos += xstep;
                         }
                    }
                  else
                    {
                       while (dst_clip_w--)
                         {
                            p1 = lptr + (xpos >> 16);
                            p2 = p1 + 1;
                            xfrac = xpos & 0xffff;
                            invxfrac = 0x10000 - xfrac;

                            if (xfrac > 0)
                              {
                                 a = ((invxfrac * A_VAL(p1)) + (xfrac * A_VAL(p2))) >> 16;
                                 r = ((invxfrac * R_VAL(p1)) + (xfrac * R_VAL(p2))) >> 16;
                                 g = ((invxfrac * G_VAL(p1)) + (xfrac * G_VAL(p2))) >> 16;
                                 b = ((invxfrac * B_VAL(p1)) + (xfrac * B_VAL(p2))) >> 16;
                              }
                            else
                              {
                                 a = A_VAL(p1);
                                 r = R_VAL(p1);
                                 g = G_VAL(p1);
                                 b = B_VAL(p1);
                              }

                            *pbuf++ = ARGB_JOIN(a, r, g, b);
                            xpos += xstep;
                         }
                    }
                  if (!mask_ie)
                    func(buf, NULL, mul_col, dptr, w);
                  else
                    {
                       mask = mask_ie->image.data8
                       + ((dst_clip_y - mask_y + y) * mask_ie->cache_entry.w)
                       + (dst_clip_x - mask_x);

                       if (mul_col != 0xffffffff)
                         func2(buf, NULL, mul_col, buf, w);
                       func(buf, mask, 0, dptr, w);
                    }
                  y++;

                  pbuf = buf;
                  ypos += ystep;
                  dptr += dst_w;   dst_clip_w = w;
               }
          }
        else
#endif
          {
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

                       if (mul_col != 0xffffffff)
                         func2(buf, NULL, mul_col, buf, w);
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
   else
     {
#ifdef DIRECT_SCALE
        if ((!src->cache_entry.flags.alpha) &&
            (!dst->cache_entry.flags.alpha) &&
            (mul_col == 0xffffffff) &&
            (!mask_ie))
          {
             // RGBA_Image *src, RGBA_Image *dst
             // dst_clip_x, dst_clip_x\y, dst_clip_w, dst_clip_h
             // src_region_x, src_region_y, src_region_w, src_region_h
             // dst_region_x, dst_region_y, dst_region_w, dst_region_h
#ifdef BILINEAR_HALF_TO_FULL_SCALE
             if (bilinear_downscale)
               {
                  DATA32 *lptr, *p1, *p2, *p3, *p4;
                  unsigned int xpos, ypos, xfrac, yfrac, invxfrac, invyfrac;
                  unsigned int xstep, ystep;
                  unsigned int r1, g1, b1, r2, g2, b2;

                  pix = src_data + (src_region_y * src_w) + src_region_x;
                  xstep = (src_region_w << 16) / dst_region_w;
                  ystep = (src_region_h << 16) / dst_region_h;
                  ypos = (dst_clip_y - dst_region_y) * ystep;

                  while (dst_clip_h--)
                    {
                       pbuf = dptr;
                       xpos = (dst_clip_x - dst_region_x) * xstep;
                       lptr = pix + ((ypos >> 16) * src_w);

                       if ((ypos >> 16) < ((unsigned int)src_h - 1))
                         {
                            yfrac = ypos & 0xffff;
                            invyfrac = 0x10000 - yfrac;
                            while (dst_clip_w--)
                              {
                                 p1 = lptr + (xpos >> 16);
                                 p2 = p1 + 1;
                                 p3 = p1 + src_w;
                                 p4 = p3 + 1;
                                 xfrac = xpos & 0xffff;
                                 invxfrac = 0x10000 - xfrac;

                                 if (xfrac > 0)
                                   {
                                      r1 = ((invxfrac * R_VAL(p1)) + (xfrac * R_VAL(p2))) >> 16;
                                      r2 = ((invxfrac * R_VAL(p3)) + (xfrac * R_VAL(p4))) >> 16;
                                      g1 = ((invxfrac * G_VAL(p1)) + (xfrac * G_VAL(p2))) >> 16;
                                      g2 = ((invxfrac * G_VAL(p3)) + (xfrac * G_VAL(p4))) >> 16;
                                      b1 = ((invxfrac * B_VAL(p1)) + (xfrac * B_VAL(p2))) >> 16;
                                      b2 = ((invxfrac * B_VAL(p3)) + (xfrac * B_VAL(p4))) >> 16;
                                   }
                                 else
                                   {
                                      r1 = R_VAL(p1);
                                      r2 = R_VAL(p3);
                                      g1 = G_VAL(p1);
                                      g2 = G_VAL(p3);
                                      b1 = B_VAL(p1);
                                      b2 = B_VAL(p3);
                                   }

                                 r = ((invyfrac * r1) + (yfrac * r2)) >> 16;
                                 g = ((invyfrac * g1) + (yfrac * g2)) >> 16;
                                 b = ((invyfrac * b1) + (yfrac * b2)) >> 16;

                                 *pbuf++ = ARGB_JOIN(0xff, r, g, b);
                                 xpos += xstep;
                              }
                         }
                       else
                         {
                            while (dst_clip_w--)
                              {
                                 p1 = lptr + (xpos >> 16);
                                 p2 = p1 + 1;
                                 xfrac = xpos & 0xffff;
                                 invxfrac = 0x10000 - xfrac;

                                 if (xfrac > 0)
                                   {
                                      r = ((invxfrac * R_VAL(p1)) + (xfrac * R_VAL(p2))) >> 16;
                                      g = ((invxfrac * G_VAL(p1)) + (xfrac * G_VAL(p2))) >> 16;
                                      b = ((invxfrac * B_VAL(p1)) + (xfrac * B_VAL(p2))) >> 16;
                                   }
                                 else
                                   {
                                      r = R_VAL(p1);
                                      g = G_VAL(p1);
                                      b = B_VAL(p1);
                                   }

                                 *pbuf++ = ARGB_JOIN(0xff, r, g, b);
                                 xpos += xstep;
                              }
                         }
                       ypos += ystep;
                       dptr += dst_w;   dst_clip_w = w;
                    }
               }
             else
#endif
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
          }
        else
#endif
          {
#ifdef BILINEAR_HALF_TO_FULL_SCALE
             if (bilinear_downscale)
               {
                  DATA32 *lptr, *p1, *p2, *p3, *p4;
                  unsigned int xpos, ypos, xfrac, yfrac, invxfrac, invyfrac;
                  unsigned int xstep, ystep;
                  unsigned int r1, g1, b1, r2, g2, b2;

                  pix = src_data + (src_region_y * src_w) + src_region_x;
                  xstep = (src_region_w << 16) / dst_region_w;
                  ystep = (src_region_h << 16) / dst_region_h;
                  ypos = (dst_clip_y - dst_region_y) * ystep;

                  while (dst_clip_h--)
                    {
                       xpos = (dst_clip_x - dst_region_x) * xstep;
                       lptr = pix + ((ypos >> 16) * src_w);

                       if ((ypos >> 16) < ((unsigned int)src_h - 1))
                         {
                            yfrac = ypos & 0xffff;
                            invyfrac = 0x10000 - yfrac;
                            while (dst_clip_w--)
                              {
                                 p1 = lptr + (xpos >> 16);
                                 p2 = p1 + 1;
                                 p3 = p1 + src_w;
                                 p4 = p3 + 1;
                                 xfrac = xpos & 0xffff;
                                 invxfrac = 0x10000 - xfrac;

                                 if (xfrac > 0)
                                   {
                                      r1 = ((invxfrac * R_VAL(p1)) + (xfrac * R_VAL(p2))) >> 16;
                                      r2 = ((invxfrac * R_VAL(p3)) + (xfrac * R_VAL(p4))) >> 16;
                                      g1 = ((invxfrac * G_VAL(p1)) + (xfrac * G_VAL(p2))) >> 16;
                                      g2 = ((invxfrac * G_VAL(p3)) + (xfrac * G_VAL(p4))) >> 16;
                                      b1 = ((invxfrac * B_VAL(p1)) + (xfrac * B_VAL(p2))) >> 16;
                                      b2 = ((invxfrac * B_VAL(p3)) + (xfrac * B_VAL(p4))) >> 16;
                                   }
                                 else
                                   {
                                      r1 = R_VAL(p1);
                                      r2 = R_VAL(p3);
                                      g1 = G_VAL(p1);
                                      g2 = G_VAL(p3);
                                      b1 = B_VAL(p1);
                                      b2 = B_VAL(p3);
                                   }

                                 r = ((invyfrac * r1) + (yfrac * r2)) >> 16;
                                 g = ((invyfrac * g1) + (yfrac * g2)) >> 16;
                                 b = ((invyfrac * b1) + (yfrac * b2)) >> 16;

                                 *pbuf++ = ARGB_JOIN(0xff, r, g, b);
                                 xpos += xstep;
                              }
                         }
                       else
                         {
                            while (dst_clip_w--)
                              {
                                 p1 = lptr + (xpos >> 16);
                                 p2 = p1 + 1;
                                 xfrac = xpos & 0xffff;
                                 invxfrac = 0x10000 - xfrac;

                                 if (xfrac > 0)
                                   {
                                      r = ((invxfrac * R_VAL(p1)) + (xfrac * R_VAL(p2))) >> 16;
                                      g = ((invxfrac * G_VAL(p1)) + (xfrac * G_VAL(p2))) >> 16;
                                      b = ((invxfrac * B_VAL(p1)) + (xfrac * B_VAL(p2))) >> 16;
                                   }
                                 else
                                   {
                                      r = R_VAL(p1);
                                      g = G_VAL(p1);
                                      b = B_VAL(p1);
                                   }

                                 *pbuf++ = ARGB_JOIN(0xff, r, g, b);
                                 xpos += xstep;
                              }
                         }
                       if (!mask_ie)
                         func(buf, NULL, mul_col, dptr, w);
                       else
                         {
                            mask = mask_ie->image.data8
                            + ((dst_clip_y - mask_y + y) * mask_ie->cache_entry.w)
                            + (dst_clip_x - mask_x);

                            if (mul_col != 0xffffffff)
                              func2(buf, NULL, mul_col, buf, w);
                            func(buf, mask, 0, dptr, w);
                         }
                       y++;

                       pbuf = buf;
                       ypos += ystep;
                       dptr += dst_w;   dst_clip_w = w;
                    }
               }
             else
#endif
               {
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

                            if (mul_col != 0xffffffff)
                              func2(buf, NULL, mul_col, buf, w);
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
     }
#else
   /* MMX scaling down would go here */
#endif
}
