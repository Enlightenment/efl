// inherited from parent func
//   RGBA_Font_Glyph_Out *fgo;
//   int w, h, x1, x2, y1, y2, i, *iptr;
//   DATA32 coltab[16], col;
//   DATA16 mtab[16], v;
//   DATA8 tmp;

// blend a pixel using pre-computed multiplied col and inverse mul value
#define MMX_BLEND(_dst, _col, _mul) \
   MOV_P2R(_dst, mm1, mm0) \
   MOV_A2R(_mul, mm3) \
   MOV_P2R(_col, mm2, mm0) \
   MUL4_256_R2R(mm3, mm1) \
   paddw_r2r(mm2, mm1); \
   MOV_R2P(mm1, _dst, mm0)
#define C_BLEND(_dst, _col, _mul) \
   _dst = _col + MUL_256(_mul, _dst)

// copy 64bits in 1 go (special mmx - no such thing in C here)
#define MMX_COPY64(_dst, _src) \
   movq_r2m(_src, _dst)

// a loop of 64bit copies
#define MMX_COPY64LOOP(_dst, _len) \
   if (_len >= 2) \
   { \
      while (_len > 1) \
        { \
           MMX_COPY64(_dst[0], mm7); \
           _dst += 2; _len -= 2; \
        } \
   }

// if we build for mmx optimizations, we need to set up a few things in advance
// like the mm0 register is always all 0'd to fill in 0 padding when
// unpacking values to registers. also mm7 is reserved to hold an unpacked
// and dumpliacted coltab entry for the final entry (max color). so it's
// [col][col] in the 63bit register with both 32bit colors doublicated
#ifdef MMX
pxor_r2r(mm0, mm0);
movd_m2r(coltab[0xf], mm7);
punpckldq_r2r(mm7, mm7);
#endif

// check header for typ (rle4 or bpp4)
iptr = (int *)fgo->rle;
if (*iptr > 0) // rle4
{
   DATA8 *p = fgo->rle, *e, *s;
   DATA32 *d0, *d, t;
   DATA16 len;
   int xx, yy, dif;

   iptr = (int *)p;
   p += sizeof(int);
   d0 = dst + x + (y * dst_pitch);
// this may seem horrible to put a massive blob of logic into a macro like
// this, but this is for speed reasons, so we can generate slightly different
// versions of the same blob of code logic that hold different optimizations
// inside (eg mmx/sse/neon asm etc.)
#define EXPAND_RLE(_donelabel, _extn, _2copy, _blend) \
   if ((x1 == 0) && (x2 == w)) /* unclipped  horizontally */ \
   { \
      d0 += x1; \
      for (yy = y1; yy < y2; yy++) \
        { \
           /* figure out source ptr and end ptr based on jumptable */ \
           if (yy > 0) s = p + jumptab[yy - 1]; \
           else s = p; \
           e = p + jumptab[yy]; \
           d = d0 + (yy * dst_pitch); \
           /* walk until we hit the end of the src data */ \
           while (s < e) \
             { \
                /* read the run length from RLE data and value */ \
                len = (*s >> 4) + 1; \
                v = *s & 0xf; \
                /* if value is 0 we can just skip ahead entire run and do */ \
                /* nothng as empty space doesn't need any work */ \
                if (v == 0) d += len; \
                /* if the value ends up being solid (inverse alpha is 0) */ \
                else if (mtab[v] == 0) \
                  { \
                     /* just COPY the color data direct to destination */ \
                     t = coltab[0xf]; \
                     /* this is a special 2 pixel (64bit dest) copy for */ \
                     /* speed - eg mmx etc. */ \
                     _2copy; \
                     /* do cleanup of left-over pixels after the 2 pixel */ \
                     /* copy above (if there is any such code) */ \
                     while (len > 0) \
                       { \
                          /* just a plain copy of looked up value */ \
                          *d = t; \
                          d++; len--; \
                       } \
                  } \
                /* our font mask value is between 0 and 15 (0xf) so we */ \
                /* have to actually blend it to each dest pixel */ \
                else \
                  { \
                     while (len > 0) \
                       { \
                          /* do blend using op provided by params */ \
                          _blend; \
                          d++; len--; \
                       } \
                  } \
                s++; \
             } \
        } \
   } \
   else /* clipped horizontally (needs extra skip/cut logic) */ \
   { \
      /* init out pos to 0 here (we reset AFTER each horiz loop later */ \
      xx = 0; \
      for (yy = y1; yy < y2; yy++) \
        { \
           /* figure out source ptr and end ptr based on jumptable */ \
           if (yy > 0) s = p + jumptab[yy - 1]; \
           else s = p; \
           e = p + jumptab[yy]; \
           d = d0 + (yy * dst_pitch); \
           /* walk until we hit the end of the src data and SKIP runs */ \
           /* that are entirely before the start (x1) point and any */ \
           /* run that spans over the start point is truncated at the */ \
           /* start of the run */ \
           while (s < e) \
             { \
                len = (*s >> 4) + 1; \
                /* if current pos pluse run length go over the start (x1) */ \
                /* point of our clip area, then adjust run length and dest */ \
                /* pointer and position and break out of our RLE skip loop */ \
                if ((xx + (int)len) > x1) \
                  { \
                     dif = x1 - xx; \
                     len -= dif; d += dif; xx += dif; \
                     break; \
                  } \
                d += len; xx += len; s++; \
             } \
           /* walk until we hit the end of the REL run.. OR the end of */ \
           /* our clip region - the x2 checks are done inside */ \
           while (s < e) \
             { \
                v = *s & 0xf; \
                /* if value is 0 we can just skip ahead entire run and do */ \
                /* nothng as empty space doesn't need any work */ \
                if (v == 0) \
                  { \
                     d += len; xx += len; \
                     /* clip check to stop run */ \
                     if (xx >= x2) goto _donelabel##_extn; \
                  } \
                /* if the value ends up being solid (inverse alpha is 0) */ \
                else if (mtab[v] == 0) \
                  { \
                     /* just COPY the color data direct to destination */ \
                     t = coltab[0xf]; \
                     while (len > 0) \
                       { \
                          /* clip check to stop run */ \
                          if (xx >= x2) goto _donelabel##_extn; \
                          /* just a plain copy of looked up value */ \
                          *d = t; \
                          d++; xx++; len--; \
                       } \
                  } \
                /* our font mask value is between 0 and 15 (0xf) so we */ \
                /* have to actually blend it to each dest pixel */ \
                else \
                  { \
                     while (len > 0) \
                       { \
                          /* clip check to stop run */ \
                          if (xx >= x2) goto _donelabel##_extn; \
                          /* do blend using op provided by params */ \
                          _blend; \
                          d++; xx++; len--; \
                       } \
                  } \
                s++; \
                /* extra check here so length fetch after doesn't break */ \
                if (s >= e) break; \
                /* get length of NEXT RLE run at the end here */ \
                len = (*s >> 4) + 1; \
             } \
_donelabel##_extn: \
           /* reset horiz pos to 0 ready for next line */ \
           xx = 0; \
        } \
   }

   // and here actually run the appropriate code in the macro/func defined
   // above, based on the jumptable type (saves passing params on the stack
   // to a sub function and we'd have to generate the subfunction by macros
   // anyway, so just cust down code to assume context vars as opposed to
   // passing them)
   if (*iptr == 1) // 8 bit jump table
     {
        DATA8 *jumptab = p;
        p += (h * sizeof(DATA8));
#ifdef MMX
        EXPAND_RLE(done_8_clipped, _mmx, MMX_COPY64LOOP(d, len),
                   MMX_BLEND(d[0], coltab[v], mtab[v]))
#elif defined(NEON)
        EXPAND_RLE(done_8_clipped, _neon, ,
                   C_BLEND(d[0], coltab[v], mtab[v]))
#else
        EXPAND_RLE(done_8_clipped, _c, ,
                   C_BLEND(d[0], coltab[v], mtab[v]))
#endif
     }
   else if (*iptr == 2) // 16 bit jump table
     {
        unsigned short *jumptab = (unsigned short *)p;
        p += (h * sizeof(unsigned short));
#ifdef MMX
        EXPAND_RLE(done_16_clipped, _mmx, MMX_COPY64LOOP(d, len),
                   MMX_BLEND(d[0], coltab[v], mtab[v]))
#elif defined(NEON)
        EXPAND_RLE(done_16_clipped, _neon, ,
                   C_BLEND(d[0], coltab[v], mtab[v]))
#else
        EXPAND_RLE(done_16_clipped, _c, ,
                   C_BLEND(d[0], coltab[v], mtab[v]))
#endif
     }
   else if (*iptr == 3) // 32 bit jump table
     {
        int *jumptab = (int *)p;
        p += (h * sizeof(int));
#ifdef MMX
        EXPAND_RLE(done_32_clipped, _mmx, MMX_COPY64LOOP(d, len),
                   MMX_BLEND(d[0], coltab[v], mtab[v]))
#elif defined(NEON)
        EXPAND_RLE(done_32_clipped, _neon, ,
                   C_BLEND(d[0], coltab[v], mtab[v]))
#else
        EXPAND_RLE(done_32_clipped, _c, ,
                   C_BLEND(d[0], coltab[v], mtab[v]))
#endif
     }
#undef EXPAND_RLE
}
else // bpp4
{
   int xx, yy, djump;
   int pitch2;
   DATA8 *s, *s0, v0;
   DATA32 *d;

   d = dst + x + x1 + ((y + y1) * dst_pitch);
   djump = dst_pitch - (x2 - x1);
   pitch2 = (w + 1) / 2;
   s0 = fgo->rle + sizeof(int) + (y1 * pitch2);
   for (yy = y1; yy < y2; yy++)
     {
        s = s0 + (x1 / 2);
        xx = x1;
        // do odd pixel at start if there is any
        if (xx & 0x1)
          {
             v = (*s) & 0xf;
             // fast path - totally solid color can just be written
             // with no blending done
             if (mtab[v] == 0) d[0] = coltab[0xf];
             // blend our color from lookup table
             else if (v)
               {
                  // blend it
#ifdef MMX
                  MMX_BLEND(d[0], coltab[v], mtab[v]);
#else
                  C_BLEND(d[0], coltab[v], mtab[v]);
#endif
               }
             s++; d++; xx++;
          }
        // walk along 2 pixels at a time (1 src pixel is 4 bits packed)
        for (; xx < (x2 - 1); xx += 2)
          {
             v0 = *s;
             // fast path - totally solid color can just be written
             // with no blending done - write 2 at once
             if ((v0 == 0xff) && (mtab[v0 & 0xf] == 0))
               {
                  // blend it
#ifdef MMX
                  MMX_COPY64(d[0], mm7);
#else
                  d[0] = d[1] = coltab[0xf];
#endif
               }
             // if our 2 values are not 0 (as 0's we can skip entirely)
             else if (v0)
               {
                  // get first pixel in MSB and blend it
                  v = (v0) >> 4;
#ifdef MMX
                  MMX_BLEND(d[0], coltab[v], mtab[v]);
#else
                  C_BLEND(d[0], coltab[v], mtab[v]);
#endif
                  // get next pixel in LSB and blend it
                  v = (v0) & 0xf;
#ifdef MMX
                  MMX_BLEND(d[1], coltab[v], mtab[v]);
#else
                  C_BLEND(d[1], coltab[v], mtab[v]);
#endif
               }
             s++; d += 2;
          }
        // clean up any leftover pixels at the end
        if (xx < x2)
          {
             v = (*s) >> 4;
             // fast path - totally solid color can just be written
             // with no blending done
             if (mtab[v] == 0) d[0] = coltab[0xf];
             // blend our color from lookup table
             else if (v)
               {
                  // blend it
#ifdef MMX
                  MMX_BLEND(d[0], coltab[v], mtab[v]);
#else
                  C_BLEND(d[0], coltab[v], mtab[v]);
#endif
               }
             d++;
          }
        d += djump;
        s0 += pitch2;
     }
}
// with mmx (sse etc.) we need to say we are done with the mmx registers so
// any fpu usage is restored (early pentiums need this, later x86 do not)
#ifdef MMX
evas_common_cpu_end_opt();
#endif
