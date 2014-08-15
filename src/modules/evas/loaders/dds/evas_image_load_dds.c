/* @file evas_image_load_dds.c
 * @author Jean-Philippe ANDRE <jpeg@videolan.org>
 *
 * Load Microsoft DirectDraw Surface files.
 * Decode S3TC image format.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Evas_Loader.h"
#include "s3tc.h"

#ifdef _WIN32
# include <ddraw.h>
#endif

#define DDS_HEADER_SIZE 128

typedef struct _Evas_Loader_Internal Evas_Loader_Internal;
struct _Evas_Loader_Internal
{
   Eina_File *f;

   Evas_Colorspace format;
   unsigned int stride, block_size, data_size;

   struct {
      unsigned int flags;
      unsigned int fourcc;
      unsigned int rgb_bitcount;
      unsigned int r_mask;
      unsigned int g_mask;
      unsigned int b_mask;
      unsigned int a_mask;
      // TODO: check mipmaps to load faster a small image :)
   } pf; // pixel format
};

#undef FOURCC
#ifndef WORDS_BIGENDIAN
# define FOURCC(a,b,c,d) ((d << 24) | (c << 16) | (b << 8) | a)
#else
# define FOURCC(a,b,c,d) ((a << 24) | (b << 16) | (c << 8) | d)
#endif

#ifndef DIRECTDRAW_VERSION
// DIRECTDRAW_VERSION is defined in ddraw.h
// These definitions are from the MSDN reference.

enum DDSFlags {
   DDSD_CAPS = 0x1,
   DDSD_HEIGHT = 0x2,
   DDSD_WIDTH = 0x4,
   DDSD_PITCH = 0x8,
   DDSD_PIXELFORMAT = 0x1000,
   DDSD_MIPMAPCOUNT = 0x20000,
   DDSD_LINEARSIZE = 0x80000,
   DDSD_DEPTH = 0x800000
};

enum DDSPixelFormatFlags {
   DDPF_ALPHAPIXELS = 0x1,
   DDPF_ALPHA = 0x2,
   DDPF_FOURCC = 0x4,
   DDPF_RGB = 0x40,
   DDPF_YUV = 0x200,
   DDPF_LUMINANCE = 0x20000
};

enum DDSCaps {
   DDSCAPS_COMPLEX = 0x8,
   DDSCAPS_MIPMAP = 0x400000,
   DDSCAPS_TEXTURE = 0x1000
};

#endif

static const Evas_Colorspace cspaces_s3tc_dxt1_rgb[] = {
   EVAS_COLORSPACE_RGB_S3TC_DXT1,
   EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_s3tc_dxt1_rgba[] = {
   //EVAS_COLORSPACE_RGBA_S3TC_DXT1,
   EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_s3tc_dxt2[] = {
   EVAS_COLORSPACE_RGBA_S3TC_DXT2,
   EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_s3tc_dxt3[] = {
   //EVAS_COLORSPACE_RGBA_S3TC_DXT3,
   EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_s3tc_dxt4[] = {
   EVAS_COLORSPACE_RGBA_S3TC_DXT4,
   EVAS_COLORSPACE_ARGB8888
};

static const Evas_Colorspace cspaces_s3tc_dxt5[] = {
   //EVAS_COLORSPACE_RGBA_S3TC_DXT5,
   EVAS_COLORSPACE_ARGB8888
};

static void *
evas_image_load_file_open_dds(Eina_File *f, Eina_Stringshare *key EINA_UNUSED,
                              Evas_Image_Load_Opts *opts EINA_UNUSED,
                              Evas_Image_Animated *animated EINA_UNUSED,
                              int *error)
{
   Evas_Loader_Internal *loader;

   if (eina_file_size_get(f) <= DDS_HEADER_SIZE)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return NULL;
     }

   loader = calloc(1, sizeof (Evas_Loader_Internal));
   if (!loader)
     {
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }


   loader->f = eina_file_dup(f);
   if (!loader->f)
     {
        free(loader);
        *error = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
        return NULL;
     }

   return loader;
}

static void
evas_image_load_file_close_dds(void *loader_data)
{
   Evas_Loader_Internal *loader = loader_data;

   eina_file_close(loader->f);
   free(loader);
}

static inline unsigned int
_dword_read(const char **m)
{
   unsigned int val = *((unsigned int *) *m);
   *m += 4;
   return val;
}

#define FAIL() do { /*fprintf(stderr, "DDS: ERROR at %s:%d\n", __FUNCTION__, __LINE__);*/ goto on_error; } while (0)

static Eina_Bool
evas_image_load_file_head_dds(void *loader_data,
                              Evas_Image_Property *prop,
                              int *error)
{
   static const unsigned int base_flags = /* 0x1007 */
         DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;

   Evas_Loader_Internal *loader = loader_data;
   unsigned int flags, height, width, pitchOrLinearSize, caps, caps2;
   Eina_Bool has_linearsize, has_mipmapcount;
   const char *m;
   char *map;

   map = eina_file_map_all(loader->f, EINA_FILE_SEQUENTIAL);
   if (!map)
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        return EINA_FALSE;
     }

   m = map;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
   if (strncmp(m, "DDS ", 4) != 0)
     // TODO: Add support for DX10
     FAIL();
   m += 4;

   // Read DDS_HEADER
   if (_dword_read(&m) != 124)
     FAIL();

   flags = _dword_read(&m);
   if ((flags & base_flags) != (base_flags))
     FAIL();

   if ((flags & ~(DDSD_MIPMAPCOUNT | DDSD_LINEARSIZE)) != base_flags)
     {
        // TODO: A lot of modes are not supported.
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        FAIL();
     }

   has_linearsize = !!(flags & DDSD_LINEARSIZE);
   if (!has_linearsize)
     FAIL();

   has_mipmapcount = !!(flags & DDSD_MIPMAPCOUNT);
   (void) has_mipmapcount; // We don't really care about it.

   height = _dword_read(&m);
   width = _dword_read(&m);
   pitchOrLinearSize = _dword_read(&m);
   if (!width || !height)
     FAIL();

   // Skip depth & mipmap count + reserved[11]
   m += 13 * sizeof(unsigned int);
   // Entering DDS_PIXELFORMAT ddspf
   if (_dword_read(&m) != 32)
     FAIL();
   loader->pf.flags = _dword_read(&m);
   if (!(loader->pf.flags & DDPF_FOURCC))
     FAIL(); // Unsupported (uncompressed formats may not have a FOURCC)
   loader->pf.fourcc = _dword_read(&m);
   loader->block_size = 16;
   switch (loader->pf.fourcc)
     {
      case FOURCC('D', 'X', 'T', '1'):
        loader->block_size = 8;
        if ((loader->pf.flags & DDPF_ALPHAPIXELS) == 0)
          {
             prop->alpha = EINA_FALSE;
             prop->cspaces = cspaces_s3tc_dxt1_rgb;
             loader->format = EVAS_COLORSPACE_RGB_S3TC_DXT1;
          }
        else
          {
             prop->alpha = EINA_TRUE;
             prop->cspaces = cspaces_s3tc_dxt1_rgba;
             loader->format = EVAS_COLORSPACE_RGBA_S3TC_DXT1;
          }
        break;
      case FOURCC('D', 'X', 'T', '2'):
        loader->format = EVAS_COLORSPACE_RGBA_S3TC_DXT2;
        prop->alpha = EINA_TRUE;
        prop->cspaces = cspaces_s3tc_dxt2;
        break;
      case FOURCC('D', 'X', 'T', '3'):
        loader->format = EVAS_COLORSPACE_RGBA_S3TC_DXT3;
        prop->alpha = EINA_TRUE;
        prop->cspaces = cspaces_s3tc_dxt5;
        break;
      case FOURCC('D', 'X', 'T', '4'):
        loader->format = EVAS_COLORSPACE_RGBA_S3TC_DXT4;
        prop->alpha = EINA_TRUE;
        prop->cspaces = cspaces_s3tc_dxt4;
        break;
      case FOURCC('D', 'X', 'T', '5'):
        loader->format = EVAS_COLORSPACE_RGBA_S3TC_DXT5;
        prop->alpha = EINA_TRUE;
        prop->cspaces = cspaces_s3tc_dxt5;
        break;
      case FOURCC('D', 'X', '1', '0'):
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        FAIL();
      default:
        // TODO: Implement decoding support for uncompressed formats
        FAIL();
     }
   loader->pf.rgb_bitcount = _dword_read(&m);
   loader->pf.r_mask = _dword_read(&m);
   loader->pf.g_mask = _dword_read(&m);
   loader->pf.b_mask = _dword_read(&m);
   loader->pf.a_mask = _dword_read(&m);
   caps = _dword_read(&m);
   if ((caps & DDSCAPS_TEXTURE) == 0)
     {
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        FAIL();
     }
   caps2 = _dword_read(&m);
   if (caps2 != 0)
     {
        // Cube maps not supported
        *error = EVAS_LOAD_ERROR_UNKNOWN_FORMAT;
        FAIL();
     }
   // Since the rest is unused, just ignore it.

   loader->stride = ((width + 3) >> 2) * loader->block_size;
   loader->data_size = loader->stride * ((height + 3) >> 2);
   if (loader->data_size != pitchOrLinearSize)
     FAIL(); // Invalid size!

   // Check file size
   if (eina_file_size_get(loader->f) < (DDS_HEADER_SIZE + loader->data_size))
     FAIL();

   prop->h = height;
   prop->w = width;
   prop->borders.l = 4;
   prop->borders.t = 4;
   prop->borders.r = 4 - (prop->w & 0x3);
   prop->borders.b = 4 - (prop->h & 0x3);
   *error = EVAS_LOAD_ERROR_NONE;

on_error:
   eina_file_map_free(loader->f, map);
   return (*error == EVAS_LOAD_ERROR_NONE);
}

static Eina_Bool
_dds_data_load(Evas_Loader_Internal *loader, Evas_Image_Property *prop,
               unsigned char *map, void *pixels, int *error)
{
   const unsigned char *src;
   int bsize = 16, srcstride, dststride, w, h;
   unsigned char *dst;

   void (* flip) (unsigned char *, const unsigned char *, int) = NULL;

   *error = EVAS_LOAD_ERROR_GENERIC;

   if (loader->format != prop->cspace)
     FAIL();

   switch (loader->format)
     {
      case EVAS_COLORSPACE_RGB_S3TC_DXT1:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT1:
        flip = s3tc_encode_dxt1_flip;
        bsize = 8;
        break;
      case EVAS_COLORSPACE_RGBA_S3TC_DXT2:
        flip = s3tc_encode_dxt2_rgba_flip;
        bsize = 16;
        break;
      case EVAS_COLORSPACE_RGBA_S3TC_DXT3:
        flip = s3tc_encode_dxt3_rgba_flip;
        bsize = 16;
        break;
      case EVAS_COLORSPACE_RGBA_S3TC_DXT4:
        flip = s3tc_encode_dxt4_rgba_flip;
        bsize = 16;
        break;
      case EVAS_COLORSPACE_RGBA_S3TC_DXT5:
        flip = s3tc_encode_dxt5_rgba_flip;
        bsize = 16;
        break;
      default: FAIL();
     }

   src = map + DDS_HEADER_SIZE;
   w = prop->w;
   h = prop->h;
   srcstride = ((prop->w + 3) / 4) * bsize;
   dststride = ((prop->w + prop->borders.l + prop->borders.r) / 4) * bsize;

   // asserts
   EINA_SAFETY_ON_FALSE_GOTO(prop->borders.l == 4, on_error);
   EINA_SAFETY_ON_FALSE_GOTO(prop->borders.t == 4, on_error);
   EINA_SAFETY_ON_FALSE_GOTO(prop->borders.r == (4 - (w & 0x3)), on_error);
   EINA_SAFETY_ON_FALSE_GOTO(prop->borders.b == (4 - (h & 0x3)), on_error);

   if (eina_file_size_get(loader->f) <
       (size_t) (DDS_HEADER_SIZE + srcstride * h / 4))
     {
        *error = EVAS_LOAD_ERROR_CORRUPT_FILE;
        goto on_error;
     }

   // First, copy the real data
   for (int y = 0; y < h; y += 4)
     {
        dst = ((unsigned char *) pixels) + ((y / 4) + 1) * dststride + bsize;
        memcpy(dst, src, srcstride);
        src += srcstride;
     }
   // Top
   for (int x = 0; x < w; x += 4)
     {
        src = map + DDS_HEADER_SIZE + (x / 4) * bsize;
        dst = ((unsigned char *) pixels) + ((x / 4) + 1) * bsize;
        flip(dst, src, EINA_TRUE);
     }
   // Left
   for (int y = 0; y < h; y += 4)
     {
        src = map + DDS_HEADER_SIZE + (y / 4) * srcstride;
        dst = ((unsigned char *) pixels) + ((y / 4) + 1) * dststride;
        flip(dst, src, EINA_FALSE);
     }
   // Top-left
   dst = pixels;
   src = dst + bsize;
   flip(dst, src, EINA_FALSE);
   // Right
   if ((prop->w & 0x3) == 0)
     {
        for (int y = 0; y < h; y += 4)
          {
             src = map + DDS_HEADER_SIZE + ((y / 4) + 1) * srcstride - bsize;
             dst = ((unsigned char *) pixels) + ((y / 4) + 2) * dststride - bsize;
             flip(dst, src, EINA_FALSE);
          }
        // Top-right
        dst = ((unsigned char *) pixels) + dststride - bsize;
        src = dst - bsize;
        flip(dst, src, EINA_FALSE);
     }
   // Bottom
   if ((prop->h & 0x3) == 0)
     {
        for (int x = 0; x < w; x += 4)
          {
             src = map + DDS_HEADER_SIZE + ((h / 4) - 1) * srcstride + (x / 4) * bsize;
             dst = ((unsigned char *) pixels) + ((h / 4) + 1) * dststride + ((x / 4) + 1) * bsize;
             flip(dst, src, EINA_TRUE);
          }
        // Bottom-left
        dst = ((unsigned char *) pixels) + ((h / 4) + 1) * dststride;
        src = dst + bsize;
        flip(dst, src, EINA_FALSE);
        if ((prop->w & 0x3) == 0)
          {
             // Bottom-right
             dst = ((unsigned char *) pixels) + ((h / 4) + 2) * dststride - bsize;
             src = dst - bsize;
             flip(dst, src, EINA_FALSE);
          }
     }

   *error = EVAS_LOAD_ERROR_NONE;

on_error:
   eina_file_map_free(loader->f, (void *) map);
   return (*error == EVAS_LOAD_ERROR_NONE);
}

Eina_Bool
evas_image_load_file_data_dds(void *loader_data,
                              Evas_Image_Property *prop,
                              void *pixels,
                              int *error)
{
   void (*func) (unsigned int *bgra, const unsigned char *s3tc) = NULL;
   Evas_Loader_Internal *loader = loader_data;
   unsigned int *pix = pixels;
   unsigned char *map = NULL;
   const unsigned char *src;

   *error = EVAS_LOAD_ERROR_CORRUPT_FILE;

   map = eina_file_map_all(loader->f, EINA_FILE_WILLNEED);
   if (!map)
     return EINA_FALSE;

   src = map + DDS_HEADER_SIZE;
   if (eina_file_size_get(loader->f) < (DDS_HEADER_SIZE + loader->data_size))
     FAIL();

   if (prop->cspace != EVAS_COLORSPACE_ARGB8888)
     return _dds_data_load(loader, prop, map, pixels, error);

   // Decode to BGRA
   switch (loader->format)
     {
      case EVAS_COLORSPACE_RGB_S3TC_DXT1:
        func = s3tc_decode_dxt1_rgb;
        prop->premul = EINA_FALSE;
        break;
      case EVAS_COLORSPACE_RGBA_S3TC_DXT1:
        func = s3tc_decode_dxt1_rgba;
        prop->premul = EINA_FALSE;
        break;
      case EVAS_COLORSPACE_RGBA_S3TC_DXT2:
        func = s3tc_decode_dxt2_rgba;
        prop->premul = EINA_FALSE;
        break;
      case EVAS_COLORSPACE_RGBA_S3TC_DXT3:
        func = s3tc_decode_dxt3_rgba;
        prop->premul = EINA_TRUE;
        break;
      case EVAS_COLORSPACE_RGBA_S3TC_DXT4:
        func = s3tc_decode_dxt4_rgba;
        prop->premul = EINA_FALSE;
        break;
      case EVAS_COLORSPACE_RGBA_S3TC_DXT5:
        func = s3tc_decode_dxt5_rgba;
        prop->premul = EINA_TRUE;
        break;
      default:
        FAIL();
     }
   if (!func) FAIL();

   for (unsigned int y = 0; y < prop->h; y += 4)
     {
        int blockh = prop->h - y;
        if (blockh > 4) blockh = 4;

        for (unsigned int x = 0; x < prop->w; x += 4)
          {
             unsigned int bgra[16];
             int k, j;

             func(bgra, src);
             src += loader->block_size;

             j = prop->w - x;
             if (j > 4) j = 4;
             for (k = 0; k < blockh; k++)
               {
                  memcpy(pix + (((y + k) * prop->w) + x), bgra + (k * 4),
                         j * sizeof (unsigned int));
               };
          }
     }

   *error = EVAS_LOAD_ERROR_NONE;

on_error:
   eina_file_map_free(loader->f, (void *) map);
   return (*error == EVAS_LOAD_ERROR_NONE);
}

Evas_Image_Load_Func evas_image_load_dds_func =
{
  evas_image_load_file_open_dds,
  evas_image_load_file_close_dds,
  evas_image_load_file_head_dds,
  evas_image_load_file_data_dds,
  NULL,
  EINA_TRUE,
  EINA_FALSE
};

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   em->functions = (void *)(&evas_image_load_dds_func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "dds",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_IMAGE_LOADER, image_loader, dds);

#ifndef EVAS_STATIC_BUILD_DDS
EVAS_EINA_MODULE_DEFINE(image_loader, dds);
#endif
