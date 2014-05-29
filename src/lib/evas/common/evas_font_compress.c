#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>

#include "evas_common_private.h"
#include "evas_private.h"

#include "evas_font_private.h"
#include "evas_blend_private.h"

#ifdef EVAS_CSERVE2
# include "../cserve2/evas_cs2_private.h"
#endif

#include FT_OUTLINE_H
#include FT_SYNTHESIS_H

// XXX:
// XXX: adapt cserve2 to this!
// XXX:

//--------------------------------------------------------------------------
//- UTILS ------------------------------------------------------------------
//--------------------------------------------------------------------------
static void
expand_bitmap(DATA8 *src, int pitch, int w, int h, DATA8 *dst)
{
   // some glyphs from fonts come in 1bit variety - expand it to 8bit before
   // compressing as it's easier to deal with a universal format
   static const DATA8 bitrepl[2] = { 0x00, 0xff };
   DATA8 *s, *d, bits;
   int bi, bj, y, end;

   for (y = 0; y < h; y++)
     {
        d = dst + (y * w);
        s = src + (y * pitch);
        // wall all bytes per row
        for (bi = 0; bi < w; bi += 8)
          {
             bits = *s;
             if ((w - bi) < 8) end = w - bi;
             else end = 8;
             // each byte has 8 bits - expand them out using lookup table above
             for (bj = 0; bj < end; bj++)
               {
                  *d = bitrepl[(bits >> (7 - bj)) & 0x1];
                  d++;
               }
             s++;
          }
     }
}




//--------------------------------------------------------------------------
//- RLE 4BIT ---------------------------------------------------------------
//--------------------------------------------------------------------------

// what is 4bit rle? it's 4 bit per pixel run-length encoding. this means
// that every row of pixels is compressed int a separate defined list
// of "runs" where every run is N pixles at value V. RLE works well for
// things like fonts which have vast regions that are either empty or solid
// with some transition (anti-alias) pixels in between. it could be that for
// a black and white alternating pattern it will come out the worst possible
// case, but this basically "never happens".
//
// data is encoded so it's fastr to access and decompress at runtime. we have
// both a blob of data that is the RLE encoded data for all rows which consist
// of 1 byte per run, and also a jump table - per row telling us the byte
// offset inside the RLE data blob where the row data begins. since we know
// the offset of the next run, we know how many bytes each row is based on
// this.
//
// since rle data may be small (less than 256 bytes) and in almost all cases
// less than 64k, a jump table of 8 bite per entry is good for many uses, and
// otherwise 16bits is used. it also supports 32bit jumptables but these are
// there just in case the data goes beyond 64k - but is unlikely to ever
// happen in real life. this means jumptables come in 3 formats thus have to
// have 3 different handling paths. RLE data is the same so it's common code.
//
// each byte in the RLE section encodes a run of between 1 and 16 pixels in
// length. there is no such thing as a run of 0 pixels. the upper 4 bits of
// the byte encode the length, with 0 being 1 pixel, 1 being 2 pixels,
// 2 being 3 pixels and so on up top 16 pixels (thus run length is actually
// (byte >> 4) + 1). the lower 4 bits encode the 4 bit pixel value of the
// whole run, from 0 to 15. it is accessed via masking (byte & 0xf). thus
// every run in RLE consumes exactly 1 byte of memory nice and neatly.
//
// at the start before the jumptable is a 32bit (int) header. it just has a
// value at the moment that indicates 0 for it not being RLE data (used by
// the 4bit packed bitmap), 1 for 8bit jumptable RLE, 2 for 16bit jumptable
// and 3 for 32bit jumptable. all other values are reserved
//
// so data looks like this when packed into a single blob in memory (where
// xx is the data size of the jump table - 8, 16 or 32bit). there are n
// lines of data in the jumptable matching to the height of the glyph where
// n is the height in rows
//
// each jumptable row ACTUALLY indicates the byte offset of the NEXT line.
// the FIRST row of RLE data is assumed to be at offset 0 in the RLE data
// section, so a special case is used for this. note that jumptable values
// are OFFSETS starting at 0 which is the first byte in the RLE data section
//
// [int] header (0, 1, 2 or  3)
// [xx] jump table for line 0
// [xx] jump table for line 1
// [xx] jump table for line 2
// ...
// [xx] jump table for line n - 1
// [char] first byte of RLE data (beginning of rle data)
// [char] second byte of RLE data
// ...
// [char] last byte of RLE data
// 
static DATA8 *
compress_rle4(DATA8 *src, int pitch, int w, int h, int *size_ret)
{
   unsigned char *scratch, *p, *pix, spanval;
   int *jumptab, x, y, spanlen, spannum, total, size, *iptr, *pos;
   unsigned short *sptr;
   DATA8 *dst, *buf, *dptr;

   // these macros make the code more readable and easier to follow, and
   // avoid replication of dumb blobs of logic
#define SPAN_ADD(_len, _val) do { (*pos) += 1; *p = ((_len) << 4) | (_val); p++; } while (0)
#define LAST_SPAN_VAL() (p[-1] & 0x0f)
#define LAST_SPAN_LEN() (p[-1] >> 4)
#define LAST_SPAN_DEL() do { (*pos) -= 1; p -= 1; } while (0)

   // create out scratch buffer for compression on the stack - maximum size
   scratch = p = alloca(pitch * h * 2);
   // also place our jumptable on the stack too - all ints here - become
   // smaller char/shorts after jumptable is generated and size known
   jumptab = alloca(h * sizeof(int));
   for (y = 0; y < h; y++)
     {
        pix = src + (y * pitch);
        // pos is the position offset from RLE data start that we have to
        // track to find out where this rows RLE run *ENDS* so keep a
        // pointer to it and we will keep ++ing it with each REL entry we add
        pos = &(jumptab[y]);
        *pos = (int)(p - scratch);
        // no spans now so init all span things to 0
        spanval = spanlen = spannum = 0;
        for (x = 0; x < w; x++)
          {
             // we only need upper 4 bits of value for span creation
             DATA8 v = pix[x] >> 4;
             // if the current pixel value (in 4bit) is not the same as the
             // span value (n 4 bit) OR... if the span now exceeds 16 pixels
             // then add/write out the span to our RLE span blob
             if ((v != spanval) || (spanlen >= 16))
               {
                  if (spanlen > 0)
                    {
                       SPAN_ADD(spanlen - 1, spanval);
                       spannum++;
                    }
                  spanval = v;
                  spanlen = 1;
               }
             // otherwise make span longer if values are the same
             else spanlen++;
          }
        // do we have a span still being built that we haven't added and that
        // is NOT transparent (0 value -  there is no point storing spans
        // at the end of a row that have 0 value
        if ((spanlen > 0) && (spanval > 0))
          {
             SPAN_ADD(spanlen - 1, spanval);
             spannum++;
          }
        // clean up any dangling 0 value at the end of a row as they just
        // waste space and processing time
        while ((spannum > 0) && (LAST_SPAN_VAL() == 0))
          {
             LAST_SPAN_DEL();
             spannum--;
          }
     }
   // get the size of RLE data we have plus int header
   total = (int)(p - scratch);
   size = sizeof(int) + total;
   // based on total number of bytes in RLE, use 32, 16 or 8 bit jumptable
   // and add that to our size
   if (total > 65535) size += h * 4; // 32bit
   else if (total > 255) size += h * 2; // 16bit
   else size += h; // 8bit

   *size_ret = size;
   // allocate a fresh buffer where we will merge header, jumptable and RLE
   // spans inot a single block
   buf = dst = malloc(size);
   if (!buf) return NULL;
   // 32bit int header to indicate encoding type (3, 2 or 1)
   iptr = (int *)dst;
   if (total > 65535) *iptr = 3; // 32bit jump table
   else if (total > 255) *iptr = 2; // 16 bit jump table
   else *iptr = 1; // 8 bit jump table
   // skip header and write jump table
   dst += sizeof(int);
   if (total > 65535) // 32bit jump table
     {
        iptr = (int *)dst;
        for (y = 0; y < h; y++) iptr[y] = jumptab[y];
        dst += (h * sizeof(int));
     }
   else if (total > 255) // 16bit jump table
     {
        sptr = (unsigned short *)dst;
        for (y = 0; y < h; y++) sptr[y] = jumptab[y];
        dst += (h * sizeof(unsigned short));
     }
   else // 8bit jump table
     {
        dptr = dst;
        for (y = 0; y < h; y++) dptr[y] = jumptab[y];
        dst += (h * sizeof(DATA8));
     }
   // copy rest of RLE data at the end of the jumptable and return it
   memcpy(dst, scratch, total);
   return buf;
}

// this decompresses a specific run of RLE data to the destination pointer
// and finishes reading RLE data before the "end" byte and starts AT the
// "start" byte within the array pointed to by src. this ASSUMES the dest
// buffer has already been zeroed out so we can skip runs that are "0"
static void
decompress_full_row(DATA8 *src, int start, int end, DATA8 *dst)
{
   DATA8 *p = src + start, *e = src + end, *d = dst, len, val;
   
   while (p < e)
     {
        // length is upper 4 bits + 1
        len = (*p >> 4) + 1;
        // value when EXPANDED to 8bit is the lower 4 bits REPEATEd in all
        // 8 bites to ensure it rounds properly.
        // i.e. lower 4 bits B4B3B2B1 -> B4B3B2B1B4B3B2B1
        val = *p & 0xf;
        val |= val << 4;
        // if it's 0 just skip ahead (assume dst buffer is 0'd out)
        if (val == 0) d += len;
        else
          {
             // write out "len" pixels of tghe given value
             while (len > 0)
               {
                  *d = val;
                  d++;
                  len--;
               }
          }
        // next RLE byte
        p++;
     }
}

// to save copy & paste repeating code, this macro acts as a code generator
// to create a specific decompress function per jumptable size (8, 16 or 32bit)
#define DECOMPRESS_ROW_FUNC(_name, _type) \
static void \
_name(_type *jumptab, DATA8 *src, DATA8 *dst, int pitch, int h) \
{ \
   int y, start, end; \
   for (y = 0; y < h; y++) \
     { \
        if (y > 0) start = jumptab[y - 1]; \
        else start = 0; \
        end = jumptab[y]; \
        decompress_full_row(src, start, end, dst + (y * pitch)); \
     } \
}
// 3 versions of the decompress given 3 jumptable types/sizes
DECOMPRESS_ROW_FUNC(decompress_jumptab8_rle4, DATA8)
DECOMPRESS_ROW_FUNC(decompress_jumptab16_rle4, unsigned short)
DECOMPRESS_ROW_FUNC(decompress_jumptab32_rle4, int)

// decompress a full RLE blob with header into the dst pointer. pitch is
// the number of bytes between each destination row
static void
decompress_rle4(DATA8 *src, DATA8 *dst, int pitch, int w EINA_UNUSED, int h)
{
   int header;
   DATA8 *jumptab;

   // get header value and then skip past to jump table
   header = *((int *)src);
   jumptab = src + sizeof(int);
#define DECOMPRESS_FUNC(_name, _type) _name((_type *)jumptab,  jumptab + (h * sizeof(_type)), dst, pitch, h)
   if (header == 1)
     DECOMPRESS_FUNC(decompress_jumptab8_rle4, DATA8);
   else if (header == 2)
     DECOMPRESS_FUNC(decompress_jumptab16_rle4, unsigned short);
   else if (header == 3)
     DECOMPRESS_FUNC(decompress_jumptab32_rle4, int);
}




//--------------------------------------------------------------------------
//- RAW 4BIT ---------------------------------------------------------------
//--------------------------------------------------------------------------

// this compresses 8bit per pixel font data to 4bit per pixel (with 4 bit MSB
// per byte holding the left most pixel and 4 bit LSB holding the right pixel
// data). each row is rounded up to a whole number of bytes so the last
// pixel may only contain 1, not 2 4bit values and thus we throw away the LSB
// 4 bits on odd-length rows in the last pixel. at the top of the 4bit packed
// pixel data is an integer that stores the data type - value of 0 means
// 4bit packed data. this is so we can share the same generic "rle" pointer
// between 4bit rle and 4bit packed and easily switch between these 2 encodings
// based on which one is likely more compact and/or faster at runtime.
static DATA8 *
compress_bpp4(DATA8 *src, int pitch, int w, int h, int *size_ret)
{
   int pitch2, x, y, *iptr;
   DATA8 *buf, *p, *d, *s;

   // our horizontal pitch in bytes ... rounding up to account for odd lengths
   pitch2 = (w + 1) / 2;
   // allocate the buffer size for header plus data
   buf = malloc(sizeof(int) + (pitch2 * h));
   if (!buf) return NULL;
   // write the header value of 0
   iptr = (int *)buf;
   *iptr = 0;
   // start with the 4 bit packed data body
   p = buf + sizeof(int);
   // return size
   *size_ret = (pitch2 * h) + sizeof(int);
   for (y = 0; y < h; y++)
     {
        s = src + (y * pitch);
        d = p + (y * pitch2);
        // walk source row 2 pixels at a time and reduce to 4 bit (upper
        // 4 bits only needed) and pack
        for (x = 0; x < (w - 1); x += 2)
          {
             *d = (s[0] & 0xf0) | (s[1] >> 4);
             s += 2;
             d++;
          }
        /// handle dangling "last" pixel if odd row length
        if (x < w) *d = (s[0] & 0xf0);
     }
   return buf;
}

// this decompresses packed 4bit data from the encoded data blob into a
// destination 8bit buffer assumed to be allocated and the right size with
// the given destination pitch in bytes per line and a row length of w
// pixels and height of h rows
static void
decompress_bpp4(DATA8 *src, DATA8 *dst, int pitch, int w, int h)
{
   int pitch2, x, y;
   DATA8 *d, *s, val;
   
   // deal with source pixel to round up for odd length rows
   pitch2 = (w + 1) / 2;
   // skip header int
   src += sizeof(int);
   for (y = 0; y < h; y++)
     {
        s = src + (y * pitch2);
        d = dst + (y * pitch);
        // walk 2 pixels at a time (1 source byte) and unpack
        for (x = 0; x < (w - 1); x += 2)
          {
             // take MSB 4 bits (pixel 1)
             val = (*s) >> 4;
             // replicate those 4 bits in MSB of dest so it rounds correctly
             val |= val << 4;
             // store in dest
             *d = val;
             d++;
             // take LSB 4 bits (pixel 2)
             val = (*s) & 0xf;
             // replicate those 4 bits in MSB of dest so it rounds correctly
             val |= val << 4;
             // store in dest
             *d = val;
             s++;
             d++;
          }
        // deal with odd length rows and take MSB 4 bits and store to dest
        if (x < w)
          {
             val = (*s) >> 4;
             val |= val << 4;
             *d = val;
          }
     }
}



//--------------------------------------------------------------------------
//- GENERAL ----------------------------------------------------------------
//--------------------------------------------------------------------------
EAPI void *
evas_common_font_glyph_compress(void *data, int num_grays, int pixel_mode,
                                int pitch_data, int w, int h, int *size_ret)
{
   DATA8 *inbuf, *buf;
   int size = 0, pitch = 0;

   // avoid compressing 0 sized glyph
   if ((h < 1) || (pitch_data < 1)) return NULL;
   inbuf = alloca(w * h);
   // if glyph buffer is 8bit grey - then compress straght
   if (((num_grays == 256) && (pixel_mode == FT_PIXEL_MODE_GRAY)))
     {
        inbuf = data;
        pitch = pitch_data;
     }
   // if glyph is 1bit bitmap - expand it to 8bit grey first
   else
     {
        pitch = w;
        expand_bitmap(data, pitch_data, w, h, inbuf);
     }
   // in testing for small glyphs - eg 16x16 or smaller it seems raw 4bit
   // encoding is faster (and smaller) than 4bit RLE.
   if ((w * h) < (16 * 16))
     // compress to 4bit per pixel, raw
     buf = compress_bpp4(inbuf, pitch, w, h, &size);
   else
     // compress to 4bit per pixel, run length encoded per row
     buf = compress_rle4(inbuf, pitch, w, h, &size);
   *size_ret = size;
   return buf;
}

// this decompresses a whole block of compressed font data back to 8bit
// per pixels and deals with both 4bit RLE and 4bit packed encoding modes
EAPI DATA8 *
evas_common_font_glyph_uncompress(RGBA_Font_Glyph *fg, int *wret, int *hret)
{
   RGBA_Font_Glyph_Out *fgo = fg->glyph_out;
   DATA8 *buf = calloc(1, fgo->bitmap.width * fgo->bitmap.rows);
   int *iptr;
   
   if (!buf) return NULL;
   if (wret) *wret = fgo->bitmap.width;
   if (hret) *hret = fgo->bitmap.rows;
   iptr = (int *)fgo->rle;
   if (*iptr > 0) // rle4
     decompress_rle4(fgo->rle, buf, fgo->bitmap.width,
                     fgo->bitmap.width, fgo->bitmap.rows);
   else // bpp4
     decompress_bpp4(fgo->rle, buf, fgo->bitmap.width,
                     fgo->bitmap.width, fgo->bitmap.rows);
   return buf;
}

// this draws a compressed font glyph and decompresses on the fly as it
// draws, saving memory bandwidth and providing speedups
EAPI void
evas_common_font_glyph_draw(RGBA_Font_Glyph *fg, 
                            RGBA_Draw_Context *dc,
                            RGBA_Image *dst_image, int dst_pitch,
                            int x, int y, int cx, int cy, int cw, int ch)
{
   RGBA_Font_Glyph_Out *fgo = fg->glyph_out;
   int w, h, x1, x2, y1, y2, i, *iptr;
   DATA32 *dst = dst_image->image.data;
   DATA32 coltab[16], col;
   DATA16 mtab[16], v;
   DATA8 tmp;

   w = fgo->bitmap.width; h = fgo->bitmap.rows;
   // skip if totally clipped out
   if ((y >= (cy + ch)) || ((y + h) <= cy) ||
       (x >= (cx + cw)) || ((x + w) <= cx)) return;
   // figure y1/y2 limit range
   y1 = 0; y2 = h;
   if ((y + y1) < cy) y1 = cy - y;
   if ((y + y2) > (cy + ch)) y2 = cy + ch - y;
   // figure x1/x2 limit range
   x1 = 0; x2 = w;
   if ((x + x1) < cx) x1 = cx - x;
   if ((x + x2) > (cx + cw)) x2 = cx + cw - x;
   col = dc->col.col;
   if (dst_image->cache_entry.space == EVAS_COLORSPACE_GRY8)
     {
        // FIXME: Font draw not optimized for Alpha targets! SLOW!
        // This is not pretty :)

        DATA8 *dst8 = dst_image->image.data8 + x + (y * dst_pitch);
        Alpha_Gfx_Func func;
        DATA8 *src8;
        int row;

        func = evas_common_alpha_func_get(dc->render_op);
        src8 = evas_common_font_glyph_uncompress(fg, NULL, NULL);
        if (!src8) return;

        for (row = y1; row < y2; row++)
          {
             DATA8 *d = dst8 + ((row - y1) * dst_pitch);
             DATA8 *s = src8 + (row * w) + x1;
             func(s, d, x2 - x1);
          }
        free(src8);
     }
   else
     {
        // build fast multiply + mask color tables to avoid compute. this works
        // because of our very limited 4bit range of alpha values
        for (i = 0; i <= 0xf; i++)
          {
             v = (i << 4) | i;
             coltab[i] = MUL_SYM(v, col);
             tmp = (coltab[i] >> 24);
             mtab[i] = 256 - (tmp + (tmp >> 7));
          }
#ifdef BUILD_MMX
        if (evas_common_cpu_has_feature(CPU_FEATURE_MMX))
          {
#define MMX 1
#include "evas_font_compress_draw.c"
#undef MMX
          }
        else
#endif

#ifdef BUILD_NEON
        if (evas_common_cpu_has_feature(CPU_FEATURE_NEON))
          {
#define NEON 1
#include "evas_font_compress_draw.c"
#undef NEON
          }
        else
#endif

          // Plain C
          {
#include "evas_font_compress_draw.c"
          }
     }
}
