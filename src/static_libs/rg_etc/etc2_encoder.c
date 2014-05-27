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

#include <Eina.h>
#include "rg_etc1.h"

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

// Damn OpenGL people, why don't you just pack data as on a CPU???
static const int kBlockWalk[16] = {
   0, 4,  8, 12,
   1, 5,  9, 13,
   2, 6, 10, 14,
   3, 7, 11, 15
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

// Simple abs
#define ABS(a) ({ int _a = (a); ((_a >= 0) ? _a : (-_a)); })

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

#ifndef DBG
# define DBG(fmt, ...) fprintf(stderr, fmt "\n", ## __VA_ARGS__)
#endif

/** Pack alpha block given a modifier table and a multiplier
 * @returns Squared error
 */
static int
_etc2_alpha_block_pack(uint8_t *etc2_alpha,
                       const int base_codeword,
                       const int multiplier,
                       const int modifierIdx,
                       const uint32_t *bgra,
                       const Eina_Bool write)
{
   const int *alphaModifiers = kAlphaModifiers[modifierIdx];
   uint8_t alphaIndexes[16];
   int errAcc2 = 0;

   // Header
   if (write)
     {
        etc2_alpha[0] = base_codeword & 0xFF;
        etc2_alpha[1] = ((multiplier << 4) & 0xF0) | (modifierIdx & 0x0F);
     }

   // Compute alphas now
   for (int i = 0; i < 16; i++)
     {
        const int realA = A_VAL(bgra + kBlockWalk[i]);
        int minErr = INT_MAX, idx = 0;

        // Brute force -- find modifier index
        for (int k = 0; (k < 8) && minErr; k++)
          {
             int tryA = CLAMP(base_codeword + alphaModifiers[k] * multiplier);
             int err = ABS(realA - tryA);
             if (err < minErr)
               {
                  minErr = err;
                  idx = k;
                  if (!minErr) break;
               }
          }

        alphaIndexes[i] = idx;

        // Keep some stats
        errAcc2 += minErr * minErr;
     }

   if (write)
     for (int k = 0; k < 2; k++)
       {
          etc2_alpha[2 + 3 * k]  =  alphaIndexes[0 + 8 * k] << 5;        // A
          etc2_alpha[2 + 3 * k] |=  alphaIndexes[1 + 8 * k] << 2;        // B
          etc2_alpha[2 + 3 * k] |= (alphaIndexes[2 + 8 * k] >> 1) & 0x3; // C01
          etc2_alpha[3 + 3 * k]  = (alphaIndexes[2 + 8 * k] & 0x1) << 7; // C2
          etc2_alpha[3 + 3 * k] |=  alphaIndexes[3 + 8 * k] << 4;        // D
          etc2_alpha[3 + 3 * k] |=  alphaIndexes[4 + 8 * k] << 1;        // E
          etc2_alpha[3 + 3 * k] |= (alphaIndexes[5 + 8 * k] >> 2) & 0x1; // F0
          etc2_alpha[4 + 3 * k]  = (alphaIndexes[5 + 8 * k] & 0x3) << 6; // F12
          etc2_alpha[4 + 3 * k] |=  alphaIndexes[6 + 8 * k] << 3;        // G
          etc2_alpha[4 + 3 * k] |=  alphaIndexes[7 + 8 * k];             // H
       }

   return errAcc2;
}

static int
_etc2_alpha_encode(uint8_t *etc2_alpha, const uint32_t *bgra,
                   rg_etc1_pack_params *params EINA_UNUSED)
{
   int alphas[16], avg = 0, diff = 0, maxDiff = INT_MAX, minErr = INT_MAX;
   int base_codeword;
   int multiplier, bestMult = 0;
   int modifierIdx, bestIdx = 0;
   int err, base_range, base_step = 1, max_error = 0;

   // Try to select the best alpha value (avg)
   for (int i = 0; i < 16; i++)
     {
        alphas[i] = A_VAL(bgra + kBlockWalk[i]);
        avg += alphas[i];
     }
   avg /= 16;

   for (int i = 0; i < 16; i++)
     {
        int thisDiff = ABS(alphas[i] - avg);
        maxDiff = MIN(thisDiff, maxDiff);
        diff += thisDiff;
     }

   base_codeword = alphas[0];
   if (!diff)
     {
        // All same alphas
        etc2_alpha[0] = base_codeword;
        memset(etc2_alpha + 1, 0, 7);
        return 0;
     }

   // Bruteforce -- try all tables and all multipliers, oh my god this will be slow.

   switch (params->m_quality)
     {
      // The follow parameters are completely arbitrary.
      // Need some real testing.
      case rg_etc1_high_quality:
        base_range = 15;
        base_step = 0;
        max_error = 0;
        break;
      case rg_etc1_medium_quality:
        base_range = 6;
        base_step = 2;
        max_error = 2 * 2 * 16; // 42dB
        break;
      case rg_etc1_low_quality:
        base_range = 0;
        max_error = 5 * 5 * 16; // 34dB
        break;
     }

   // for loop avg, avg-1, avg+1, avg-2, avg+2, ...
   for (int step = 0; step < base_range; step += base_step)
     for (base_codeword = avg - step; base_codeword <= avg + step; base_codeword += 2 * step)
       {
          for (modifierIdx = 0; modifierIdx < 16; modifierIdx++)
            for (multiplier = 0; multiplier < 16; multiplier++)
              {
                 if ((ABS(multiplier * kAlphaModifiers[modifierIdx][3])) < maxDiff)
                   continue;

                 err = _etc2_alpha_block_pack(etc2_alpha, base_codeword,
                                              multiplier, modifierIdx, bgra, EINA_FALSE);
                 if (err < minErr)
                   {
                      minErr = err;
                      bestMult = multiplier;
                      bestIdx = modifierIdx;
                      if (err < max_error)
                        goto pack_now;

                   }
              }
          if (step <= 0) break;
       }

pack_now:
   err = _etc2_alpha_block_pack(etc2_alpha, base_codeword,
                                bestMult, bestIdx, bgra, EINA_TRUE);
   return err;
}

unsigned int
etc2_rgba8_block_pack(unsigned char *etc2, const unsigned int *bgra,
                      rg_etc1_pack_params *params)
{
   unsigned int error;

   // FIXME/TODO: For now, encode use rg_etc1 only!
   error = rg_etc1_pack_block(etc2 + 8, bgra, params);
   error += _etc2_alpha_encode(etc2, bgra, params);

   return error;
}

unsigned int
etc2_rgb8_block_pack(unsigned char *etc2, const unsigned int *bgra,
                     rg_etc1_pack_params *params)
{
  unsigned int error;

  // FIXME/TODO: For now, encode use rg_etc1 only!
  error = rg_etc1_pack_block(etc2, bgra, params);

  return error;
}
