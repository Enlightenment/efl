// File: rg_etc1.h - Fast, high quality ETC1 block packer/unpacker - Rich Geldreich <richgel99@gmail.com>
// Please see ZLIB license at the end of this file.
#ifndef __RG_ETC1_H__
#define __RG_ETC1_H__

#include <Eina.h>

// Unpacks an 8-byte ETC1 compressed block to a block of 4x4 32bpp RGBA pixels.
// Returns false if the block is invalid. Invalid blocks will still be unpacked with clamping.
// This function is thread safe, and does not dynamically allocate any memory.
// If preserve_alpha is true, the alpha channel of the destination pixels will not be overwritten. Otherwise, alpha will be set to 255.
Eina_Bool rg_etc1_unpack_block(const void *pETC1_block, unsigned int* pDst_pixels_rgba, Eina_Bool preserve_alpha);

// Quality setting = the higher the quality, the slower.
// To pack large textures, it is highly recommended to call pack_etc1_block() in parallel, on different blocks, from multiple threads (particularly when using cHighQuality).
typedef enum {
  rg_etc1_low_quality,
  rg_etc1_medium_quality,
  rg_etc1_high_quality
} rg_etc1_quality;


typedef struct
{
   rg_etc1_quality m_quality;
   Eina_Bool m_dithering;
} rg_etc1_pack_params;

// pack_etc1_block_init() should be called before calling pack_etc1_block(),
// Otherwise rg_etc1_pack_block() will call rg_etc1_pack_block_init() for you.
void rg_etc1_pack_block_init(void);

// Packs a 4x4 block of 32bpp BGRA pixels to an 8-byte ETC1 block.
// 32-bit BGRA pixels must always be arranged as (B,G,R,A) (B first, A last) in memory, independent of platform endianness. A should always be 255.
// Returns squared error of result.
// This function is thread safe, and does not dynamically allocate any memory.
// pack_etc1_block() does not currently support "perceptual" colorspace metrics - it primarily optimizes for RGB RMSE.
unsigned int rg_etc1_pack_block(void* pETC1_block, const unsigned int* pSrc_pixels_BGRA, rg_etc1_pack_params *pack_params);

// Pack a 4x4 block of 32bpp BGRA pixels to a 16-byte RGBA8_ETC2_EAC block (supports alpha).
unsigned int etc2_rgba8_block_pack(unsigned char *etc2, const unsigned int *bgra, rg_etc1_pack_params *params);

// Pack a 4x4 block of 32bpp BGRA pixels to a 8-byte RGB8_ETC2 block (opaque).
unsigned int etc2_rgb8_block_pack(unsigned char *etc2, const unsigned int *bgra, rg_etc1_pack_params *params);

// ETC2 support: RGB8_ETC2
void rg_etc2_rgb8_decode_block(const unsigned char *etc_block, unsigned int *bgra);

// ETC2 support: RGBA8_ETC2_EAC
void rg_etc2_rgba8_decode_block(const unsigned char *etc_block, unsigned int *bgra);

//------------------------------------------------------------------------------
//
// rg_etc1 uses the ZLIB license:
// http://opensource.org/licenses/Zlib
//
// Copyright (c) 2012 Rich Geldreich
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
//------------------------------------------------------------------------------
#endif // __RG_ETC1_H__
