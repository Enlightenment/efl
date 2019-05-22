#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_native_common.h"

#if defined HAVE_DLSYM && ! defined _WIN32
# include <dlfcn.h>      /* dlopen,dlclose,etc */
#elif _WIN32
# include <evil_private.h> /* dlopen dlclose dlsym */
#else
# warning native_tbm should not get compiled if dlsym is not found on the system!
#endif

#define EVAS_ROUND_UP_4(num) (((num)+3) & ~3)
#define EVAS_ROUND_UP_8(num) (((num)+7) & ~7)

#define TBM_SURF_PLANE_MAX 4 /**< maximum number of the planes  */

/* option to map the tbm_surface */
#define TBM_SURF_OPTION_READ      (1 << 0) /**< access option to read  */
#define TBM_SURF_OPTION_WRITE     (1 << 1) /**< access option to write */

#define __tbm_fourcc_code(a,b,c,d) ((uint32_t)(a) | ((uint32_t)(b) << 8) | \
			      ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#define TBM_FORMAT_ARGB8888 __tbm_fourcc_code('A', 'R', '2', '4')
#define TBM_FORMAT_ABGR8888 __tbm_fourcc_code('A', 'B', '2', '4')
#define TBM_FORMAT_RGBX8888	__tbm_fourcc_code('R', 'X', '2', '4') /* [31:0] R:G:B:x 8:8:8:8 little endian */
#define TBM_FORMAT_RGBA8888	__tbm_fourcc_code('R', 'A', '2', '4') /* [31:0] R:G:B:A 8:8:8:8 little endian */
#define TBM_FORMAT_BGRA8888	__tbm_fourcc_code('B', 'A', '2', '4') /* [31:0] B:G:R:A 8:8:8:8 little endian */
#define TBM_FORMAT_NV12		__tbm_fourcc_code('N', 'V', '1', '2') /* 2x2 subsampled Cr:Cb plane */
#define TBM_FORMAT_YUV420	__tbm_fourcc_code('Y', 'U', '1', '2') /* 2x2 subsampled Cb (1) and Cr (2) planes */
#define TBM_FORMAT_YVU420	__tbm_fourcc_code('Y', 'V', '1', '2') /* 2x2 subsampled Cr (1) and Cb (2) planes */

static void *tbm_lib = NULL;
static int   tbm_ref = 0;

typedef struct _tbm_surface * tbm_surface_h;
typedef uint32_t tbm_format;

typedef struct _tbm_surface_plane
{
    unsigned char *ptr;   /**< Plane pointer */
    uint32_t size;        /**< Plane size */
    uint32_t offset;      /**< Plane offset */
    uint32_t stride;      /**< Plane stride */

    void *reserved1;      /**< Reserved pointer1 */
    void *reserved2;      /**< Reserved pointer2 */
    void *reserved3;      /**< Reserved pointer3 */
} tbm_surface_plane_s;

typedef struct _tbm_surface_info
{
    uint32_t width;      /**< TBM surface width */
    uint32_t height;     /**< TBM surface height */
    tbm_format format;   /**< TBM surface format*/
    uint32_t bpp;        /**< TBM surface bbp */
    uint32_t size;       /**< TBM surface size */

    uint32_t num_planes;                            /**< The number of planes */
    tbm_surface_plane_s planes[TBM_SURF_PLANE_MAX]; /**< Array of planes */

    void *reserved4;   /**< Reserved pointer4 */
    void *reserved5;   /**< Reserved pointer5 */
    void *reserved6;   /**< Reserved pointer6 */
} tbm_surface_info_s;

/* returns 0 on success */
static int (*sym_tbm_surface_map) (tbm_surface_h surface, int opt, tbm_surface_info_s *info) = NULL;
static int (*sym_tbm_surface_unmap) (tbm_surface_h surface) = NULL;
static int (*sym_tbm_surface_get_info) (tbm_surface_h surface, tbm_surface_info_s *info) = NULL;

EAPI int
_evas_native_tbm_init(void)
{
   if (tbm_lib)
     {
        tbm_ref++;
        return tbm_ref;
     }

   const char *tbm_libs[] =
   {
      "libtbm.so.1",
      "libtbm.so.0",
      NULL,
   };
   int i, fail;
#define SYM(lib, xx)                            \
  do {                                          \
       sym_ ## xx = dlsym(lib, #xx);            \
       if (!(sym_ ## xx)) {                     \
            ERR("%s", dlerror());               \
            fail = 1;                           \
         }                                      \
    } while (0)

   for (i = 0; tbm_libs[i]; i++)
     {
        tbm_lib = dlopen(tbm_libs[i], RTLD_LOCAL | RTLD_LAZY);
        if (tbm_lib)
          {
             fail = 0;
             SYM(tbm_lib, tbm_surface_map);
             SYM(tbm_lib, tbm_surface_unmap);
             SYM(tbm_lib, tbm_surface_get_info);
             if (fail)
               {
                  dlclose(tbm_lib);
                  tbm_lib = NULL;
               }
             else break;
          }
     }
   if (!tbm_lib) return 0;

   tbm_ref++;
   return tbm_ref;
}

EAPI void
_evas_native_tbm_shutdown(void)
{
   if (tbm_ref > 0)
     {
        tbm_ref--;

        if (tbm_ref == 0)
          {
             if (tbm_lib)
               {
                  dlclose(tbm_lib);
                  tbm_lib = NULL;
               }
          }
     }
}

static void
_evas_video_yv12(unsigned char *evas_data, const unsigned char *source_data, unsigned int w, unsigned int h, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i, j;
   unsigned int rh;
   unsigned int stride_y, stride_uv;

   rh = output_height;

   rows = (const unsigned char **)evas_data;

   stride_y = EVAS_ROUND_UP_4(w);
   stride_uv = EVAS_ROUND_UP_8(w) / 2;

   for (i = 0; i < rh; i++)
     rows[i] = &source_data[i * stride_y];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &source_data[h * stride_y +
                            (rh / 2) * stride_uv +
                            j * stride_uv];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &source_data[h * stride_y + j * stride_uv];
}

static void
_evas_video_i420(unsigned char *evas_data, const unsigned char *source_data, unsigned int w, unsigned int h, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i, j;
   unsigned int rh;
   unsigned int stride_y, stride_uv;

   rh = output_height;

   rows = (const unsigned char **)evas_data;

   stride_y = w;
   stride_uv = w / 2;

   for (i = 0; i < rh; i++)
     rows[i] = &source_data[i * stride_y];

   for (j = 0; j < ((rh + 1) / 2); j++, i++)
     rows[i] = &source_data[h * stride_y + j * stride_uv];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &source_data[h * stride_y +
                            ((rh + 1) / 2) * stride_uv +
                            j * stride_uv];
}

static void
_evas_video_nv12(unsigned char *evas_data, const unsigned char *source_data, unsigned int w, unsigned int h EINA_UNUSED, unsigned int output_height)
{
   const unsigned char **rows;
   unsigned int i, j;
   unsigned int rh;

   rh = output_height;

   rows = (const unsigned char **)evas_data;

   for (i = 0; i < rh; i++)
     rows[i] = &source_data[i * w];

   for (j = 0; j < (rh / 2); j++, i++)
     rows[i] = &source_data[rh * w + j * w];
}

static void
_native_bind_cb(void *image, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   RGBA_Image *im = image;
   tbm_surface_info_s info;
   tbm_surface_h tbm_surf;

   if (!im) return;
   Native *n = im->native.data;
   if (!n) return;
   if (n->ns.type != EVAS_NATIVE_SURFACE_TBM)
     return;

   tbm_surf = n->ns.data.tbm.buffer;
   if (sym_tbm_surface_map(tbm_surf, TBM_SURF_OPTION_READ|TBM_SURF_OPTION_WRITE, &info))
     return;

   im->image.data = (DATA32 *)info.planes[0].ptr;
}

static void
_native_unbind_cb(void *image)
{
   RGBA_Image *im = image;
   tbm_surface_h tbm_surf;

   if (!im) return;
   Native *n = im->native.data;
   if (!n) return;
   if (n->ns.type != EVAS_NATIVE_SURFACE_TBM)
     return;

   tbm_surf = n->ns.data.tbm.buffer;
   sym_tbm_surface_unmap(tbm_surf);
}

static void
_native_free_cb(void *image)
{
   RGBA_Image *im = image;

   if (!im) return;
   Native *n = im->native.data;

   im->native.data        = NULL;
   im->native.func.bind   = NULL;
   im->native.func.unbind = NULL;
   im->native.func.free   = NULL;

   free(n);

   _evas_native_tbm_shutdown();
}

EAPI int
_evas_native_tbm_surface_stride_get(void *data EINA_UNUSED, void *native)
{
   Evas_Native_Surface *ns = native;
   tbm_surface_info_s info;
   int stride;

   if (!ns)
     return -1;

   if (!_evas_native_tbm_init())
     {
        ERR("Could not initialize TBM!");
        return -1;
     }

   if (sym_tbm_surface_get_info(ns->data.tbm.buffer, &info))
     return -1;

   stride = info.planes[0].stride;
   return stride;
 }

EAPI void *
_evas_native_tbm_surface_image_set(void *data EINA_UNUSED, void *image, void *native)
{
   Evas_Native_Surface *ns = native;
   RGBA_Image *im = image;
   tbm_surface_h tbm_surf;

   if (!im) return NULL;

   if (ns)
     {
        void *pixels_data;
        int h, stride;
        tbm_format format;
        tbm_surface_info_s info;
        Native *n;

        if (ns->type != EVAS_NATIVE_SURFACE_TBM)
          return NULL;

        tbm_surf = ns->data.tbm.buffer;
/*
        if (!_evas_native_tbm_init())
          {
             ERR("Could not initialize TBM!");
             return NULL;
          }
*/
        n = calloc(1, sizeof(Native));
        if (!n) return NULL;

        if (sym_tbm_surface_map(tbm_surf, TBM_SURF_OPTION_READ|TBM_SURF_OPTION_WRITE, &info))
          {
             free(n);
             return im;
          }

        h = info.height;
        stride = info.planes[0].stride;
        format = info.format;
        pixels_data = info.planes[0].ptr;
        im->cache_entry.w = stride;
        im->cache_entry.h = h;

        // Handle all possible format here :"(
        switch (format)
          {
           case TBM_FORMAT_RGBA8888:
           case TBM_FORMAT_RGBX8888:
           case TBM_FORMAT_BGRA8888:
           case TBM_FORMAT_ARGB8888:
           case TBM_FORMAT_ABGR8888:
              im->cache_entry.w = stride / 4;
              evas_cache_image_colorspace(&im->cache_entry, EVAS_COLORSPACE_ARGB8888);
              im->cache_entry.flags.alpha = (format == TBM_FORMAT_RGBX8888 ? 0 : 1);
              im->image.data = pixels_data;
              im->image.no_free = 1;
              break;
              /* borrowing code from emotion here */
           case TBM_FORMAT_YVU420: /* EVAS_COLORSPACE_YCBCR422P601_PL */
              evas_cache_image_colorspace(&im->cache_entry, EVAS_COLORSPACE_YCBCR422P601_PL);
              _evas_video_yv12(im->cs.data, pixels_data, stride, h, h);
              evas_common_image_colorspace_dirty(im);
              break;
           case TBM_FORMAT_YUV420: /* EVAS_COLORSPACE_YCBCR422P601_PL */
              evas_cache_image_colorspace(&im->cache_entry, EVAS_COLORSPACE_YCBCR422P601_PL);
              _evas_video_i420(im->cs.data, pixels_data, stride, h, h);
              evas_common_image_colorspace_dirty(im);
              break;
           case TBM_FORMAT_NV12: /* EVAS_COLORSPACE_YCBCR420NV12601_PL */
              evas_cache_image_colorspace(&im->cache_entry, EVAS_COLORSPACE_YCBCR420NV12601_PL);
              _evas_video_nv12(im->cs.data, pixels_data, stride, h, h);
              evas_common_image_colorspace_dirty(im);
              break;
              /* Not planning to handle those in software */
           default:
              sym_tbm_surface_unmap(ns->data.tbm.buffer);
              free(n);
              return im;
          }

        memcpy(n, ns, sizeof(Evas_Native_Surface));
        im->native.data = n;
        im->native.func.bind   = _native_bind_cb;
        im->native.func.unbind = _native_unbind_cb;
        im->native.func.free   = _native_free_cb;

        sym_tbm_surface_unmap(tbm_surf);
     }
   return im;
}
