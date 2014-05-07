/*
Copyright (C) 2014 Jean-Philippe ANDRE

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * ETC2 decoding functions, reimplemented from scratch based on the
 * OpenGL ES 3.0 spec (annex C.1).
 *
 * @author Jean-Philippe ANDRE
 * @license BSD-2 with advertisement clause
 */

#include <Eina.h>
#include "rg_etc1.h"

void rg_etc2_rgb8_decode_block(const uint8_t *etc_block, uint32_t *bgra);
void rg_etc2_rgba8_decode_block(const uint8_t *etc_block, uint32_t *bgra);

typedef const uint8_t etc_block;

// For T and H modes
static const int kDistances[8] = {
   3, 6, 11, 16, 23, 32, 41, 64
};

// For differential mode
static const int kSigned3bit[8] = {
   0, 1, 2, 3, -4, -3, -2, -1
};

// For alpha support
static const int kAlphaModifiers[16][8] = {
   {  -3,  -6,  -9,  -15,  2,  5,  8,  14},
   {  -3,  -7, -10,  -13,  2,  6,  9,  12},
   {  -2,  -5,  -8,  -13,  1,  4,  7,  12},
   {  -2,  -4,  -6,  -13,  1,  3,  5,  12},
   {  -3,  -6,  -8,  -12,  2,  5,  7,  11},
   {  -3,  -7,  -9,  -11,  2,  6,  8,  10},
   {  -4,  -7,  -8,  -11,  3,  6,  7,  10},
   {  -3,  -5,  -8,  -11,  2,  4,  7,  10},
   {  -2,  -6,  -8,  -10,  1,  5,  7,   9},
   {  -2,  -5,  -8,  -10,  1,  4,  7,   9},
   {  -2,  -4,  -8,  -10,  1,  3,  7,   9},
   {  -2,  -5,  -7,  -10,  1,  4,  6,   9},
   {  -3,  -4,  -7,  -10,  2,  3,  6,   9},
   {  -1,  -2,  -3,  -10,  0,  1,  2,   9},
   {  -4,  -6,  -8,   -9,  3,  5,  7,   8},
   {  -3,  -5,  -7,   -9,  2,  4,  6,   8}
};

// Use with static constants so the compiler can optimize everything
#define BITS(byteval, lowbit, highbit) \
   (((byteval) >> (lowbit)) & ((1 << ((highbit) - (lowbit) + 1)) - 1))

#define BIT(byteval, bit) \
   (((byteval) >> (bit)) & 0x1)

// Clamps only if value is > 255
#define CLAMPDOWN(a) ({ int _z = (a); ((_z <= 255) ? _z : 255); })

// Clamps only if value is < 0
#define CLAMPUP(a) ({ int _z = (a); ((_z >= 0) ? _z : 0); })

// Real clamp
#define CLAMP(a) ({ int _b = (a); (((_b) >= 0) ? (((_b) < 256) ? (_b) : 255) : 0); })

// Simple min
#define MIN(a,b) ({ int _z = (a), _y = (b); ((_z <= _y) ? _z : _y); })

// Write a BGRA value for output to Evas
#define BGRA(r,g,b,a) ((a << 24) | (r << 16) | (g << 8) | b)

#define _4to8(a) ({ int _a = (a) & ((1 << 4) - 1); ((_a << 4) | (_a & 0xf)); })
#define _5to8(a) ({ int _a = (a) & ((1 << 5) - 1); ((_a << 3) | ((_a >> 2) & 0x7)); })
#define _6to8(a) ({ int _a = (a) & ((1 << 6) - 1); ((_a << 2) | ((_a >> 4) & 0x3)); })
#define _7to8(a) ({ int _a = (a) & ((1 << 7) - 1); ((_a << 1) | ((_a >> 6) & 0x1)); })

#ifndef WORDS_BIGENDIAN
/* x86 */
#define A_VAL(p) (((uint8_t *)(p))[3])
#define R_VAL(p) (((uint8_t *)(p))[2])
#define G_VAL(p) (((uint8_t *)(p))[1])
#define B_VAL(p) (((uint8_t *)(p))[0])
#else
/* ppc */
#define A_VAL(p) (((uint8_t *)(p))[0])
#define R_VAL(p) (((uint8_t *)(p))[1])
#define G_VAL(p) (((uint8_t *)(p))[2])
#define B_VAL(p) (((uint8_t *)(p))[3])
#endif


static inline void
_T_mode_color_read(const uint8_t *etc, uint32_t *paint_colors, int alpha)
{
   // 4 bit colors
   const int r1_4 = (BITS(etc[0], 3, 4) << 2) | BITS(etc[0], 0, 1);
   const int g1_4 = BITS(etc[1], 4, 7);
   const int b1_4 = BITS(etc[1], 0, 3);
   const int r2_4 = BITS(etc[2], 4, 7);
   const int g2_4 = BITS(etc[2], 0, 3);
   const int b2_4 = BITS(etc[3], 4, 7);

   // Distance index
   const int didx = (BITS(etc[3], 2, 3) << 1) | BIT(etc[3], 0);
   const int d = kDistances[didx];

   // Write out paint colors for T mode
   paint_colors[0] = BGRA(_4to8(r1_4), _4to8(g1_4), _4to8(b1_4), alpha);
   paint_colors[1] = BGRA(CLAMPDOWN(_4to8(r2_4) + d),
                          CLAMPDOWN(_4to8(g2_4) + d),
                          CLAMPDOWN(_4to8(b2_4) + d),
                          alpha);
   paint_colors[2] = BGRA(_4to8(r2_4), _4to8(g2_4), _4to8(b2_4), alpha);
   paint_colors[3] = BGRA(CLAMPUP(_4to8(r2_4) - d),
                          CLAMPUP(_4to8(g2_4) - d),
                          CLAMPUP(_4to8(b2_4) - d),
                          alpha);
}

static inline void
_H_mode_color_read(const uint8_t *etc, uint32_t *paint_colors, int alpha)
{
   // 4 bit colors
   const int r1_4 = BITS(etc[0], 3, 6);
   const int g1_4 = (BITS(etc[0], 0, 2) << 1) | (BIT(etc[1], 4));
   const int b1_4 = (BIT(etc[1], 3) << 3) | (BITS(etc[1], 0, 1) << 1) | (BIT(etc[2], 7));
   const int r2_4 = BITS(etc[2], 3, 6);
   const int g2_4 = (BITS(etc[2], 0, 2) << 1) | (BIT(etc[3], 7));
   const int b2_4 = BITS(etc[3], 3, 6);

   // Distance index
   const int basecol1 = (_4to8(r1_4) << 16) | (_4to8(g1_4) << 8) | _4to8(b1_4);
   const int basecol2 = (_4to8(r2_4) << 16) | (_4to8(g2_4) << 8) | _4to8(b2_4);
   const int didx =
         (BIT(etc[3], 2) << 2) |
         (BIT(etc[3], 0) << 1) |
         ((basecol1 >= basecol2) ? 1 : 0);
   const int d = kDistances[didx];

   // Write out paint colors for H mode
   paint_colors[0] = BGRA(CLAMPDOWN(_4to8(r1_4) + d),
                          CLAMPDOWN(_4to8(g1_4) + d),
                          CLAMPDOWN(_4to8(b1_4) + d),
                          alpha);
   paint_colors[1] = BGRA(CLAMPUP(_4to8(r1_4) - d),
                          CLAMPUP(_4to8(g1_4) - d),
                          CLAMPUP(_4to8(b1_4) - d),
                          alpha);
   paint_colors[2] = BGRA(CLAMPDOWN(_4to8(r2_4) + d),
                          CLAMPDOWN(_4to8(g2_4) + d),
                          CLAMPDOWN(_4to8(b2_4) + d),
                          alpha);
   paint_colors[3] = BGRA(CLAMPUP(_4to8(r2_4) - d),
                          CLAMPUP(_4to8(g2_4) - d),
                          CLAMPUP(_4to8(b2_4) - d),
                          alpha);
}

static inline void
_planar_mode_color_read(const uint8_t *etc, uint32_t *bgra, int alpha)
{
   // RO: Bits 57-62
   const int RO = _6to8(BITS(etc[0], 1, 6));
   // GO: Bits 49-54,56
   const int GO = _7to8((BIT(etc[0], 0) << 6) | (BITS(etc[1], 1, 6)));
   // BO: Bits 39,40-41,43-44,48
   const int BO = _6to8((BIT(etc[1], 0) << 5) | (BITS(etc[2], 3, 4) << 3) | (BITS(etc[2], 0, 1) << 1) | BIT(etc[3], 7));
   // RH: Bits 32,34-38
   const int RH = _6to8((BITS(etc[3], 2, 6) << 1) | BIT(etc[3], 0));
   // GH: Bits 25-31
   const int GH = _7to8(BITS(etc[4], 1, 7));
   // BH: Bits 19-23,24
   const int BH = _6to8((BIT(etc[4], 0) << 5) | (BITS(etc[5], 3, 7)));
   // RV: Bits 13-15,16-18
   const int RV = _6to8((BITS(etc[5], 0, 2) << 3) | (BITS(etc[6], 5, 7)));
   // GV: Bits 6-7,8-12
   const int GV = _7to8((BITS(etc[6], 0, 4) << 2) | (BITS(etc[7], 6, 7)));
   // BV: Bits 0-5
   const int BV = _6to8(BITS(etc[7], 0, 5));

   for (int y = 0; y < 4; y++)
     for (int x = 0; x < 4; x++)
       {
          // Formulas straight from the spec
          const int R = CLAMP(((x * (RH - RO)) + y * (RV - RO) + 4 * RO + 2) >> 2);
          const int G = CLAMP(((x * (GH - GO)) + y * (GV - GO) + 4 * GO + 2) >> 2);
          const int B = CLAMP(((x * (BH - BO)) + y * (BV - BO) + 4 * BO + 2) >> 2);
          *bgra++ = BGRA(R, G, B, alpha);
       }
}

static inline void
_TH_paint(const uint8_t *etc, uint32_t paint_colors[4], uint32_t *bgra)
{
   // Common code for modes T and H.

   // a,b,c,d
   bgra[ 0] = paint_colors[(BIT(etc[5], 0) << 1) | (BIT(etc[7], 0))];
   bgra[ 4] = paint_colors[(BIT(etc[5], 1) << 1) | (BIT(etc[7], 1))];
   bgra[ 8] = paint_colors[(BIT(etc[5], 2) << 1) | (BIT(etc[7], 2))];
   bgra[12] = paint_colors[(BIT(etc[5], 3) << 1) | (BIT(etc[7], 3))];

   // e,f,g,h
   bgra[ 1] = paint_colors[(BIT(etc[5], 4) << 1) | (BIT(etc[7], 4))];
   bgra[ 5] = paint_colors[(BIT(etc[5], 5) << 1) | (BIT(etc[7], 5))];
   bgra[ 9] = paint_colors[(BIT(etc[5], 6) << 1) | (BIT(etc[7], 6))];
   bgra[13] = paint_colors[(BIT(etc[5], 7) << 1) | (BIT(etc[7], 7))];

   // i,j,k,l
   bgra[ 2] = paint_colors[(BIT(etc[4], 0) << 1) | (BIT(etc[6], 0))];
   bgra[ 6] = paint_colors[(BIT(etc[4], 1) << 1) | (BIT(etc[6], 1))];
   bgra[10] = paint_colors[(BIT(etc[4], 2) << 1) | (BIT(etc[6], 2))];
   bgra[14] = paint_colors[(BIT(etc[4], 3) << 1) | (BIT(etc[6], 3))];

   // m,n,o,p
   bgra[ 3] = paint_colors[(BIT(etc[4], 4) << 1) | (BIT(etc[6], 4))];
   bgra[ 7] = paint_colors[(BIT(etc[4], 5) << 1) | (BIT(etc[6], 5))];
   bgra[11] = paint_colors[(BIT(etc[4], 6) << 1) | (BIT(etc[6], 6))];
   bgra[15] = paint_colors[(BIT(etc[4], 7) << 1) | (BIT(etc[6], 7))];
}

void
rg_etc2_rgb8_decode_block(const uint8_t *etc, uint32_t *bgra)
{
   // Check differential mode bit
   if ((etc[3] & 0x2) == 0)
     goto etc1;

   // Read R,G,B
   const int R  = BITS(etc[0], 3, 7);
   const int dR = kSigned3bit[BITS(etc[0], 0, 2)];
   const int G  = BITS(etc[1], 3, 7);
   const int dG = kSigned3bit[BITS(etc[1], 0, 2)];
   const int B  = BITS(etc[2], 3, 7);
   const int dB = kSigned3bit[BITS(etc[2], 0, 2)];
   uint32_t paint_colors[4];

   if ((R + dR) < 0 || (R + dR) >= 32)
     {
        // T mode
        _T_mode_color_read(etc, paint_colors, 255);
        _TH_paint(etc, paint_colors, bgra);
        return;
     }
   if ((G + dG) < 0 || (G + dG) >= 32)
     {
        // H mode
        _H_mode_color_read(etc, paint_colors, 255);
        _TH_paint(etc, paint_colors, bgra);
        return;
     }
   if ((B + dB) < 0 || (B + dB) >= 32)
     {
        // Planar mode
        _planar_mode_color_read(etc, bgra, 255);
        return;
     }

etc1:
   // Valid differential mode or individual mode: ETC1
   if (!rg_etc1_unpack_block(etc, bgra, 0))
     fprintf(stderr, "ETC2: Something very strange is happening here!\n");
}

void
rg_etc2_rgba8_decode_block(const uint8_t *etc, uint32_t *bgra)
{
   const uint8_t zeros[7] = {0};
   uint32_t table_index;
   int base_codeword;
   int multiplier;

   base_codeword = etc[0];

   // Fast path if alpha is the same for all pixels
   if (!memcmp(etc + 1, zeros, 7))
     {
        if (!base_codeword)
          memset(bgra, 0, 64);
        else
          {
             rg_etc2_rgb8_decode_block(etc + 8, bgra);
             if (base_codeword != 255)
               for (int k = 0; k < 16; k++)
                 {
                    const uint32_t rgb = *bgra;
                    const int R = MIN(R_VAL(&rgb), base_codeword);
                    const int G = MIN(G_VAL(&rgb), base_codeword);
                    const int B = MIN(B_VAL(&rgb), base_codeword);
                    *bgra++ = BGRA(R, G, B, base_codeword);
                 }
          }
        return;
     }

   rg_etc2_rgb8_decode_block(etc + 8, bgra);

   multiplier = BITS(etc[1], 4, 7);
   table_index = BITS(etc[1], 0, 3);

   for (int x = 0, k = 0; x < 4; x++)
     for (int y = 0; y < 4; y++, k += 3)
       {
          const uint32_t byte = (k >> 3); // = [k/8]
          const uint32_t bit = k - (byte << 3); // = k%8
          const uint32_t rgb = bgra[(y << 2) + x];
          uint32_t index, alpha, R, G, B;

          if (bit < 6)
            index = BITS(etc[byte + 2], 5 - bit, 7 - bit);
          else if (bit == 6)
            index = (BITS(etc[byte + 2], 0, 1) << 1) | BIT(etc[byte + 3], 7);
          else // bit == 7
            index = (BIT(etc[byte + 2], 0) << 2) | BITS(etc[byte + 3], 6, 7);
          alpha = CLAMP(base_codeword + kAlphaModifiers[table_index][index] * multiplier);
          R = MIN(R_VAL(&rgb), alpha);
          G = MIN(G_VAL(&rgb), alpha);
          B = MIN(B_VAL(&rgb), alpha);
          bgra[(y << 2) + x] = BGRA(R, G, B, alpha);
       }
}
