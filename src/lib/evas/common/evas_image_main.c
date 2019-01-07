#ifdef HAVE_CONFIG_H
# include "config.h"  /* so that EAPI in Eet.h is correctly defined */
#endif

#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif

#ifdef BUILD_LOADER_EET
# include <Eet.h>
#endif

#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_image_private.h"
#include "evas_convert_yuv.h"
//#include "evas_cs.h"

#ifdef HAVE_VALGRIND
# include <valgrind.h>
# include <memcheck.h>
#endif

//#define SURFDBG 1

static Evas_Cache_Image *eci = NULL;
static int reference = 0;
static int evas_image_no_mmap = -1;

/* static RGBA_Image *evas_rgba_line_buffer = NULL; */

#define EVAS_RGBA_LINE_BUFFER_MIN_LEN 256
#define EVAS_RGBA_LINE_BUFFER_MAX_LEN 2048

/* static RGBA_Image *evas_alpha_line_buffer = NULL; */

#define EVAS_ALPHA_LINE_BUFFER_MIN_LEN 256
#define EVAS_ALPHA_LINE_BUFFER_MAX_LEN 2048

static Image_Entry *_evas_common_rgba_image_new(void);
static void _evas_common_rgba_image_delete(Image_Entry *ie);
static int _evas_common_rgba_image_surface_alloc(Image_Entry *ie,
                                                 unsigned int w,
                                                 unsigned int h);
static void _evas_common_rgba_image_surface_delete(Image_Entry *ie);
static DATA32 *_evas_common_rgba_image_surface_pixels(Image_Entry *ie);
static void _evas_common_rgba_image_unload(Image_Entry *im);
static void _evas_common_rgba_image_dirty_region(Image_Entry *im,
                                                 unsigned int x,
                                                 unsigned int y,
                                                 unsigned int w,
                                                 unsigned int h);
static int _evas_common_rgba_image_ram_usage(Image_Entry *ie);

/* Only called when references > 0. Need to provide a fresh copie of im. */
/* The destination surface does have a surface, but no allocated pixel data. */
static int _evas_common_rgba_image_dirty(Image_Entry* dst,
                                         const Image_Entry* src);

static const Evas_Cache_Image_Func _evas_common_image_func =
{
  _evas_common_rgba_image_new,
  _evas_common_rgba_image_delete,
  _evas_common_rgba_image_surface_alloc,
  _evas_common_rgba_image_surface_delete,
  _evas_common_rgba_image_surface_pixels,
  evas_common_load_rgba_image_module_from_file,
  _evas_common_rgba_image_unload,
  _evas_common_rgba_image_dirty_region,
  _evas_common_rgba_image_dirty,
  evas_common_rgba_image_size_set,
  evas_common_rgba_image_from_copied_data,
  evas_common_rgba_image_from_data,
  evas_common_rgba_image_colorspace_set,
  evas_common_load_rgba_image_data_from_file,
  _evas_common_rgba_image_ram_usage,
  NULL // _evas_common_rgba_image_debug
};

EAPI int
_evas_common_rgba_image_surface_size(unsigned int w, unsigned int h,
                                     Evas_Colorspace cspace,
                                     /*inout*/int *l, int *r, int *t, int *b)
{
#ifndef PAGE_SIZE
# define PAGE_SIZE (4 * 1024)
#endif
#define HUGE_PAGE_SIZE (2 * 1024 * 1024)
#if defined (HAVE_SYS_MMAN_H) && (!defined (_WIN32))
# define ALIGN_TO_PAGE(Siz) \
   (((Siz / PAGE_SIZE) + (Siz % PAGE_SIZE ? 1 : 0)) * PAGE_SIZE)
#else
# define ALIGN_TO_PAGE(Siz) Siz
#endif
   int siz, block_size = 8;
   Eina_Bool reset_borders = EINA_TRUE;

#ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) evas_image_no_mmap = 1;
#endif

   if (EINA_UNLIKELY(evas_image_no_mmap == -1))
     {
        const char *s = getenv("EVAS_IMAGE_NO_MMAP");
        evas_image_no_mmap = s && (atoi(s));
        if (evas_image_no_mmap)
          WRN("EVAS_IMAGE_NO_MMAP is set, use this only for debugging!");
     }

   switch (cspace)
     {
      case EVAS_COLORSPACE_GRY8: siz = w * h * sizeof(DATA8); break;
      case EVAS_COLORSPACE_AGRY88: siz = w * h * sizeof(DATA16); break;
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT2:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT3:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT4:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT5:
      case EVAS_COLORSPACE_ETC1_ALPHA:
        block_size = 16;
        // fallthrough
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
      case EVAS_COLORSPACE_RGB_S3TC_DXT1:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT1:
        reset_borders = EINA_FALSE;
        if (l && r && t && b)
          {
             w += *l + *r;
             h += *t + *b;
          }
        EINA_SAFETY_ON_FALSE_RETURN_VAL(!(w & 0x3) && !(h & 0x3), 0);
        siz = (w >> 2) * (h >> 2) * block_size;
        break;
      default:
      case EVAS_COLORSPACE_ARGB8888: siz = w * h * sizeof(DATA32); break;
     }

   if (reset_borders)
     {
        if (l) *l = 0;
        if (r) *r = 0;
        if (t) *t = 0;
        if (b) *b = 0;
     }

   if ((siz < PAGE_SIZE) || evas_image_no_mmap) return siz;

   return ALIGN_TO_PAGE(siz);
#undef ALIGN_TO_PAGE
}

EAPI Eina_Bool
_evas_common_rgba_image_plane_get(const RGBA_Image *im, int plane,
                                  Eina_Slice *slice)
{
   unsigned char **csdata = NULL;
   Evas_Colorspace cs;
   size_t w, h;

   if (!im || !slice) return EINA_FALSE;
   cs = im->cache_entry.space;
   w = im->cache_entry.w;
   h = im->cache_entry.h;

   switch (cs)
     {
    case EVAS_COLORSPACE_YCBCR422P601_PL:
    case EVAS_COLORSPACE_YCBCR422P709_PL:
    case EVAS_COLORSPACE_YCBCR422601_PL:
    case EVAS_COLORSPACE_YCBCR420NV12601_PL:
    case EVAS_COLORSPACE_YCBCR420TM12601_PL:
        if (!im->cs.data) return EINA_FALSE;
        csdata = im->cs.data;
        break;

      default:
        if (!im->image.data) return EINA_FALSE;
        break;
     }

   switch (cs)
     {
      case EVAS_COLORSPACE_ARGB8888:
        if (plane != 0) return EINA_FALSE;
        slice->len = w * h * 4;
        slice->mem = im->image.data;
        return EINA_TRUE;

      case EVAS_COLORSPACE_AGRY88:
        if (plane != 0) return EINA_FALSE;
        slice->len = w * h * 2;
        slice->mem = im->image.data;
        return EINA_TRUE;

      case EVAS_COLORSPACE_GRY8:
        if (plane != 0) return EINA_FALSE;
        slice->len = w * h;
        slice->mem = im->image.data;
        return EINA_TRUE;

      case EVAS_COLORSPACE_RGB565_A5P:
        if (plane == 0)
          {
             slice->mem = im->image.data;
             slice->len = w * h * 2;
             return EINA_TRUE;
          }
        else if (plane == 1)
          {
             slice->mem = im->image.data8 + (w * h * 2);
             slice->len = w * h;
             return EINA_TRUE;
          }
        return EINA_FALSE;

        // YUV, assume contiguous memory within a plane - padding ok
        // 1 plane
      case EVAS_COLORSPACE_YCBCR422601_PL:
        if (plane != 0) return EINA_FALSE;
        slice->mem = csdata[0];
        slice->len = (h > 1) ? ((csdata[1] - csdata[0]) * h * 2) : (w * 2);
        return EINA_TRUE;

        // 2 planes
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
        if (plane == 0)
          {
             slice->mem = csdata[0];
             slice->len = (h > 1) ? ((csdata[1] - csdata[0]) * h) : w;
             return EINA_TRUE;
          }
        else if (plane == 1)
          {
             slice->mem = csdata[h];
             slice->len = (h > 1) ? ((csdata[h+1] - csdata[h]) * h / 2) : w / 2;
             return EINA_TRUE;
          }
        return EINA_FALSE;

        // 3 planes
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
        if (plane == 0)
          {
             slice->mem = csdata[0];
             slice->len = (h > 1) ? ((csdata[1] - csdata[0]) * h) : w;
             return EINA_TRUE;
          }
        else if (plane == 1)
          {
             slice->mem = csdata[h];
             slice->len = (h > 1) ? ((csdata[h+1] - csdata[h]) * h / 2) : w / 2;
             return EINA_TRUE;
          }
        else if (plane == 2)
          {
             slice->mem = csdata[h + h / 2];
             slice->len = (h > 1) ? ((csdata[h+h/2+1] - csdata[h+h/2]) * h / 2) : w / 2;
             return EINA_TRUE;
          }
        return EINA_FALSE;

        // ETC1/2 RGB, S3TC RGB
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
      case EVAS_COLORSPACE_RGB_S3TC_DXT1:
        if (plane != 0) return EINA_FALSE;
        slice->mem = im->image.data;
        slice->len = (w * h * 8) / 16;
        return EINA_TRUE;

        // ETC2 ARGB, S3TC ARGB
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT1:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT2:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT3:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT4:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT5:
        if (plane != 0) return EINA_FALSE;
        slice->mem = im->image.data;
        slice->len = (w * h * 16) / 16;
        return EINA_TRUE;

        // ETC1+Alpha
      case EVAS_COLORSPACE_ETC1_ALPHA:
        if (plane == 0)
          {
             slice->mem = im->image.data;
             slice->len = (w * h * 8) / 16;
             return EINA_TRUE;
          }
        else if (plane == 1)
          {
             slice->mem = im->image.data8 + (w * h * 8) / 16;
             slice->len = (w * h * 8) / 16;
             return EINA_TRUE;
          }
        return EINA_FALSE;

      default:
        return EINA_FALSE;
     }
}

EAPI int
_evas_common_rgba_image_data_offset(int rx, int ry, int rw, int rh,
                                    int plane, const RGBA_Image *im)
{
   // note: no stride support
   EINA_SAFETY_ON_NULL_RETURN_VAL(im, -1);

   const Image_Entry *ie = &im->cache_entry;

   if ((rx < 0) || (ry < 0) || (rw < 0) || (rh < 0)) return -1;

   if (((rx + rw) > (int) ie->w) || ((ry + rh) > (int) ie->h)) return -1;

   switch (ie->space)
     {
      case EVAS_COLORSPACE_ARGB8888:
        return (ry * ie->w + rx) * 4;
      case EVAS_COLORSPACE_AGRY88:
        return (ry * ie->w + rx) * 2;
      case EVAS_COLORSPACE_GRY8:
        return ry * ie->w + rx;
      case EVAS_COLORSPACE_RGB565_A5P:
        if (plane == 0) return (ry * ie->w + rx) * 2;
        else if (plane == 1) return ry * ie->w + rx + (ie->w * ie->h) * 2;
        else return -1;

        // YUV
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
        if ((rx & 1) || (rw & 1)) return -1;
        if (plane == 0) return ry * ie->w + rx;
        else if (plane == 1) return (ry * ie->w) / 2 + rx + ie->w * ie->h;
        else return -1;

      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
        if ((rx & 1) || (ry & 1) || (rw & 1) || (rh & 1)) return -1;
        if (plane == 0) return ry * ie->w + rx;
        else if (plane == 1) return (ry * ie->w + rx) / 2 + ie->w * ie->h;
        else return -1;

        // ETC1/2 RGB, S3TC RGB
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
      case EVAS_COLORSPACE_RGB_S3TC_DXT1:
        if ((rx & 3) || (ry & 3) || (rw & 3) || (rh & 3)) return -1;
        return (ry * ie->w + rx) * 8 / 16;

        // ETC2 ARGB, S3TC ARGB
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT1:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT2:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT3:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT4:
      case EVAS_COLORSPACE_RGBA_S3TC_DXT5:
        if ((rx & 3) || (ry & 3) || (rw & 3) || (rh & 3)) return -1;
        return (ry * ie->w + rx) * 16 / 16;

        // ETC1+Alpha
      case EVAS_COLORSPACE_ETC1_ALPHA:
        if ((rx & 3) || (ry & 3) || (rw & 3) || (rh & 3)) return -1;
        if (plane == 0) return (ry * ie->w + rx) * 8 / 16;
        else if (plane == 1)
          return (ry * ie->w + rx) * 8 / 16 + (ie->w * ie->h) * 8 / 16;
        else return -1;

      default:
        CRI("unknown colorspace %d", ie->space);
        return EINA_FALSE;
     }
}

static void *
_evas_common_rgba_image_surface_mmap(Image_Entry *ie,
                                     unsigned int w, unsigned int h,
                                     /*inout*/int *pl, int *pr,
                                     int *pt, int *pb)
{
   int siz;
#if defined (HAVE_SYS_MMAN_H) && (!defined (_WIN32))
   void *r = MAP_FAILED;
#endif

   siz = _evas_common_rgba_image_surface_size(w, h, ie->space, pl, pr, pt, pb);

#if defined (HAVE_SYS_MMAN_H) && (!defined (_WIN32))
#ifndef MAP_HUGETLB
# define MAP_HUGETLB 0
#endif
   if (siz < 0) return NULL;

   if ((siz < PAGE_SIZE) || evas_image_no_mmap) return malloc(siz);

   if (siz > ((HUGE_PAGE_SIZE * 75) / 100))
     r = mmap(NULL, siz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_HUGETLB, -1, 0);
   if (r == MAP_FAILED)
     r = mmap(NULL, siz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
   if (r == MAP_FAILED)
     r = NULL;

   return r;
#else
   return malloc(siz);
#endif
}

void
evas_common_rgba_image_surface_munmap(void *data, unsigned int w, unsigned int h, Evas_Colorspace cspace)
{
   if (!data) return;
#if defined (HAVE_SYS_MMAN_H) && (!defined (_WIN32))
   size_t siz;

   siz = _evas_common_rgba_image_surface_size(w, h, cspace,
                                              NULL, NULL, NULL, NULL);
   if ((siz < PAGE_SIZE) || evas_image_no_mmap) free(data);
   else munmap(data, siz);
#else
   (void)w;
   (void)h;
   (void)cspace;
   free(data);
#endif
}

EAPI void
evas_common_image_init(void)
{
   if (!eci) eci = evas_cache_image_init(&_evas_common_image_func);
   reference++;

   evas_common_scalecache_init();
}

EAPI void
evas_common_image_shutdown(void)
{
   if (--reference == 0)
     {
////	printf("REF--=%i\n", reference);
// DISABLE for now - something wrong with cache shutdown freeing things
// still in use - rage_thumb segv's now.
//
// actually - i think i see it. cache ref goes to 0 (and thus gets freed)
// because in eng_setup() when a buffer changes size it is FIRST freed
// THEN allocated again - thus brignhjing ref to 0 then back to 1 immediately
// where it should stay at 1. - see evas_engine.c in the buffer enigne for
// example. eng_output_free() is called BEFORE _output_setup(). although this
// is only a SIGNE of the problem. we can patch this up with either freeing
// after the setup (so we just pt a ref of 2 then back to 1), or just
// evas_common_image_init() at the start and evas_common_image_shutdown()
// after it all. really ref 0 should only be reached when no more canvases
// with no more objects exist anywhere.

// ENABLE IT AGAIN, hope it is fixed. Gustavo @ January 22nd, 2009.
       evas_cache_image_shutdown(eci);
       eci = NULL;
     }
   evas_common_scalecache_shutdown();
}

EAPI void
evas_common_image_image_all_unload(void)
{
   evas_common_rgba_image_scalecache_dump();
   evas_cache_image_unload_all(eci);
}

static Image_Entry *
_evas_common_rgba_image_new(void)
{
   RGBA_Image *im;

   im = calloc(1, sizeof(RGBA_Image));
   if (!im) return NULL;
   im->flags = RGBA_IMAGE_NOTHING;

   evas_common_rgba_image_scalecache_init(&im->cache_entry);
   return &im->cache_entry;
}

static void
_evas_common_rgba_image_delete(Image_Entry *ie)
{
   RGBA_Image *im = (RGBA_Image *)ie;

   evas_common_rgba_pending_unloads_remove(ie);
#ifdef BUILD_PIPE_RENDER
   evas_common_pipe_free(im);
#endif
   if (ie->loader_data)
     {
        Evas_Image_Load_Func *evas_image_load_func = NULL;

        evas_image_load_func = ie->info.loader;
        if (evas_image_load_func)
          evas_image_load_func->file_close(ie->loader_data);
        ie->loader_data = NULL;
     }
   evas_common_rgba_image_scalecache_shutdown(&im->cache_entry);
   if (ie->info.module) evas_module_unref((Evas_Module *)ie->info.module);

   if (ie->animated.frames)
     {
        Image_Entry_Frame *frame;

        EINA_LIST_FREE(ie->animated.frames, frame)
          {
             if (frame->data) free(frame->data);
             if (frame->info) free(frame->info);
             free(frame);
          }
     }
   if (ie->f && !ie->flags.given_mmap) eina_file_close(ie->f);
   eina_freeq_ptr_add(eina_freeq_main_get(), im, free, sizeof(*im));
}

static void
evas_common_rgba_image_unload_real(Image_Entry *ie)
{
   RGBA_Image   *im = (RGBA_Image *) ie;

   ie->flags.loaded = 0;

   if ((im->cs.data) && (im->image.data))
     {
        if (im->cs.data != im->image.data)
          {
             if (!im->cs.no_free) free(im->cs.data);
          }
     }
   else if (im->cs.data)
     {
        if (!im->cs.no_free) free(im->cs.data);
     }
   im->cs.data = NULL;

   if (im->image.data && !im->image.no_free)
     {
        evas_common_rgba_image_surface_munmap(im->image.data,
                                              ie->allocated.w, ie->allocated.h,
                                              ie->space);
#ifdef SURFDBG
        surfs = eina_list_remove(surfs, ie);
#endif
     }
   im->image.data = NULL;
   ie->allocated.w = 0;
   ie->allocated.h = 0;
   ie->flags.loaded = 0;
   ie->flags.preload_done = 0;
   ie->need_unload = 0;
#ifdef SURFDBG
   surf_debug();
#endif
}

static Eina_List *pending_unloads = NULL;

EAPI void
evas_common_rgba_pending_unloads_cleanup(void)
{
   Image_Entry *ie;
   Eina_List *l;
   Eina_List *l_next;

   EINA_LIST_FOREACH_SAFE(pending_unloads, l, l_next, ie)
     {
        if ((ie->need_unload) && (!ie->preload))
          {
             evas_common_rgba_image_unload_real(ie);
             pending_unloads = eina_list_remove_list(pending_unloads, l);
          }
     }
}

EAPI void
evas_common_rgba_pending_unloads_remove(Image_Entry *ie)
{
   if (!ie->need_unload) return;
   ie->need_unload = 0;
   pending_unloads = eina_list_remove(pending_unloads, ie);
}

EAPI void
evas_common_rgba_image_free(Image_Entry *ie)
{
   if (ie->references > 0) return;
   evas_common_rgba_pending_unloads_remove(ie);
   _evas_common_rgba_image_surface_delete(ie);
   _evas_common_rgba_image_delete(ie);
}

#ifdef SURFDBG
static Eina_List *surfs = NULL;

static void
surf_debug(void)
{
   Eina_List *l;
   Image_Entry *ie;
   RGBA_Image *im;
   int i = 0;

   printf("----SURFS----\n");
   EINA_LIST_FOREACH(surfs, l, ie)
     {
        im = ie;
        printf("%i - %p - %ix%i  [%s][%s]\n",
               i, im->image.data, ie->allocated.w, ie->allocated.h,
               ie->file, ie->key
              );
        i++;
     }
}
#endif

EAPI void
evas_common_rgba_image_unload(Image_Entry *ie)
{
   if (!ie->flags.loaded) return;
   if ((!ie->info.module) && (!ie->data1)) return;
   if (!ie->file && !ie->f) return;
   if ((evas_cache_async_frozen_get() == 0) &&
       (ie->references > 0))
     {
        if (!ie->need_unload)
          {
             pending_unloads = eina_list_append(pending_unloads, ie);
             ie->need_unload = 1;
          }
        return;
     }
   if (!ie->need_unload) evas_common_rgba_image_unload_real(ie);
}

void
_evas_common_rgba_image_post_surface(Image_Entry *ie)
{
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_IMAGE
   RGBA_Image *im = (RGBA_Image *)ie;
   int w, h;

   if (!im->image.data) return;
   if (im->pixman.im) pixman_image_unref(im->pixman.im);
   w = ie->allocated.w;
   h = ie->allocated.h;
   if ((w <= 0) || (h <= 0))
     {
        w = im->cache_entry.w;
        h = im->cache_entry.h;
     }
   if (im->cache_entry.flags.alpha)
     {
        im->pixman.im = pixman_image_create_bits
          (
#ifdef WORDS_BIGENDIAN
           PIXMAN_b8g8r8a8,
#else
           PIXMAN_a8r8g8b8,
#endif
           w, h, im->image.data, w * 4);
     }
   else
     {
        im->pixman.im = pixman_image_create_bits
          (
#ifdef WORDS_BIGENDIAN
           PIXMAN_b8g8r8x8,
#else
           PIXMAN_x8r8g8b8,
#endif
           w, h, im->image.data, w * 4);
     }
# else
   (void)ie;
# endif
#else
   (void)ie;
#endif
}

static int
_evas_common_rgba_image_surface_alloc(Image_Entry *ie,
                                      unsigned int w, unsigned int h)
{
   RGBA_Image *im = (RGBA_Image *) ie;
   int l = 0, r = 0, t = 0, b = 0;

   if (im->image.no_free) return 0;

   if (im->image.data)
     {
        evas_common_rgba_image_surface_munmap(im->image.data,
                                              ie->allocated.w, ie->allocated.h,
                                              ie->space);
        im->image.data = NULL;
#ifdef SURFDBG
        surfs = eina_list_remove(surfs, ie);
#endif
     }

   l = ie->borders.l;
   r = ie->borders.r;
   t = ie->borders.t;
   b = ie->borders.b;
   im->image.data = _evas_common_rgba_image_surface_mmap(ie, w, h,
                                                         &l, &r, &t, &b);
   if (!im->image.data) return -1;
   ie->borders.l = l;
   ie->borders.r = r;
   ie->borders.t = t;
   ie->borders.b = b;
   ie->allocated.w = w + l + r;
   ie->allocated.h = h + t + b;
#ifdef SURFDBG
   surfs = eina_list_append(surfs, ie);
#endif
#ifdef HAVE_VALGRIND
   int        siz = 0;
   siz = _evas_common_rgba_image_surface_size(w, h, ie->space, &l, &r, &t, &b);
# ifdef VALGRIND_MAKE_READABLE
   if (siz > 0) VALGRIND_MAKE_READABLE(im->image.data, siz);
# else
#  ifdef VALGRIND_MAKE_MEM_DEFINED
   if (siz > 0) VALGRIND_MAKE_MEM_DEFINED(im->image.data, siz);
#  endif
# endif
#endif
   _evas_common_rgba_image_post_surface(ie);
#ifdef SURFDBG
   surf_debug();
#endif
   return 0;
}

static void
_evas_common_rgba_image_surface_delete(Image_Entry *ie)
{
   RGBA_Image   *im = (RGBA_Image *) ie;

#ifdef HAVE_PIXMAN
# ifdef PIXMAN_IMAGE
   if (im->pixman.im)
     {
        pixman_image_unref(im->pixman.im);
        im->pixman.im = NULL;
     }
# endif
#endif
   if (ie->file) DBG("unload: [%p] %s %s", ie, ie->file, ie->key);
   if ((im->cs.data) && (im->image.data))
     {
        if (im->cs.data != im->image.data)
          {
             if (!im->cs.no_free) free(im->cs.data);
          }
     }
   else if (im->cs.data)
     {
        if (!im->cs.no_free) free(im->cs.data);
     }
   im->cs.data = NULL;

   if (im->image.data && !im->image.no_free)
     {
        evas_common_rgba_image_surface_munmap(im->image.data,
                                              ie->allocated.w, ie->allocated.h,
                                              ie->space);
#ifdef SURFDBG
        surfs = eina_list_remove(surfs, ie);
#endif
     }

   im->image.data = NULL;
   ie->allocated.w = 0;
   ie->allocated.h = 0;
   ie->flags.preload_done = 0;
   ie->flags.loaded = 0;
   evas_common_rgba_image_scalecache_dirty(&im->cache_entry);
#ifdef SURFDBG
   surf_debug();
#endif
}

static void
_evas_common_rgba_image_unload(Image_Entry *im)
{
   evas_common_rgba_image_scalecache_dirty(im);
   evas_common_rgba_image_unload(im);
}

static void
_evas_common_rgba_image_dirty_region(Image_Entry* ie,
                                     unsigned int x EINA_UNUSED,
                                     unsigned int y EINA_UNUSED,
                                     unsigned int w EINA_UNUSED,
                                     unsigned int h EINA_UNUSED)
{
   RGBA_Image   *im = (RGBA_Image *) ie;

   im->flags |= RGBA_IMAGE_IS_DIRTY;
   evas_common_rgba_image_scalecache_dirty(&im->cache_entry);
}

/* Only called when references > 0. Need to provide a fresh copie of im. */
static int
_evas_common_rgba_image_dirty(Image_Entry *ie_dst, const Image_Entry *ie_src)
{
   RGBA_Image   *dst = (RGBA_Image *) ie_dst;
   RGBA_Image   *src = (RGBA_Image *) ie_src;

   evas_common_rgba_image_scalecache_dirty((Image_Entry *)ie_src);
   evas_common_rgba_image_scalecache_dirty(ie_dst);
   evas_cache_image_load_data(&src->cache_entry);
   if (!evas_cache_image_pixels(ie_dst))
     {
        if (_evas_common_rgba_image_surface_alloc(&dst->cache_entry,
                                                  src->cache_entry.w,
                                                  src->cache_entry.h))
          {
             return 1;
          }
     }
   evas_common_image_colorspace_normalize(src);
   evas_common_image_colorspace_normalize(dst);
   return 0;
}

static int
_evas_common_rgba_image_ram_usage(Image_Entry *ie)
{
   RGBA_Image *im = (RGBA_Image *)ie;
   int size = sizeof(struct _RGBA_Image);

   if (ie->cache_key) size += strlen(ie->cache_key);
   if (ie->file) size += strlen(ie->file);
   if (ie->key) size += strlen(ie->key);
   if (ie->f && eina_file_virtual(ie->f)) size += eina_file_size_get(ie->f);

   if (im->image.data)
     {
        if ((!im->image.no_free))
          size += im->cache_entry.w * im->cache_entry.h * sizeof(DATA32);
     }
   size += evas_common_rgba_image_scalecache_usage_get(&im->cache_entry);
   return size;
}

static DATA32 *
_evas_common_rgba_image_surface_pixels(Image_Entry *ie)
{
   RGBA_Image *im = (RGBA_Image *)ie;
   return im->image.data;
}

static RGBA_Image *
evas_common_image_create(unsigned int w, unsigned int h)
{
   RGBA_Image *im;

   im = (RGBA_Image *) _evas_common_rgba_image_new();
   if (!im) return NULL;
   im->cache_entry.w = w;
   im->cache_entry.h = h;
   if (_evas_common_rgba_image_surface_alloc(&im->cache_entry, w, h))
     {
        _evas_common_rgba_image_delete(&im->cache_entry);
        return NULL;
     }
   im->cache_entry.flags.cached = 0;
   return im;
}

EAPI RGBA_Image *
evas_common_image_alpha_create(unsigned int w, unsigned int h)
{
   RGBA_Image *im;

   im = (RGBA_Image *) _evas_common_rgba_image_new();
   if (!im) return NULL;
   im->cache_entry.w = w;
   im->cache_entry.h = h;
   im->cache_entry.flags.alpha = 1;
   if (_evas_common_rgba_image_surface_alloc(&im->cache_entry, w, h))
     {
        _evas_common_rgba_image_delete(&im->cache_entry);
        return NULL;
     }
   im->cache_entry.flags.cached = 0;
   return im;
}

EAPI RGBA_Image *
evas_common_image_new(unsigned int w, unsigned int h, unsigned int alpha)
{
   if (alpha) return evas_common_image_alpha_create(w, h);
   return evas_common_image_create(w, h);
}

void
evas_common_image_colorspace_normalize(RGBA_Image *im)
{
   if ((!im->cs.data) ||
       ((!im->cs.dirty) && (!(im->flags & RGBA_IMAGE_IS_DIRTY)))) return;
   switch (im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
      case EVAS_COLORSPACE_GRY8:
      case EVAS_COLORSPACE_AGRY88:
        if (im->image.data != im->cs.data)
          {
             if (!im->image.no_free)
               {
                  evas_common_rgba_image_surface_munmap
                    (im->image.data,
                     im->cache_entry.allocated.w,
                     im->cache_entry.allocated.h,
                     im->cache_entry.space);
#ifdef SURFDBG
                  surfs = eina_list_remove(surfs, im);
#endif
                  ((Image_Entry *)im)->allocated.w = 0;
                  ((Image_Entry *)im)->allocated.h = 0;
               }
             im->image.data = im->cs.data;
             im->cs.no_free = im->image.no_free;
          }
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
        if ((im->image.data) && (*((unsigned char **)im->cs.data)))
          evas_common_convert_yuv_422p_601_rgba(im->cs.data,
                                                (DATA8 *)im->image.data,
                                                im->cache_entry.w,
                                                im->cache_entry.h);
        break;
      case EVAS_COLORSPACE_YCBCR422601_PL:
        if ((im->image.data) && (*((unsigned char **)im->cs.data)))
          evas_common_convert_yuv_422_601_rgba(im->cs.data,
                                               (DATA8 *)im->image.data,
                                               im->cache_entry.w,
                                               im->cache_entry.h);
        break;
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
        if ((im->image.data) && (*((unsigned char **)im->cs.data)))
          evas_common_convert_yuv_420_601_rgba(im->cs.data,
                                               (DATA8 *)im->image.data,
                                               im->cache_entry.w,
                                               im->cache_entry.h);
        break;
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
        if ((im->image.data) && (*((unsigned char **)im->cs.data)))
          evas_common_convert_yuv_420T_601_rgba(im->cs.data,
                                                (DATA8 *)im->image.data,
                                                im->cache_entry.w,
                                                im->cache_entry.h);
         break;
      case EMILE_COLORSPACE_YCBCR422P709_PL:
        if ((im->image.data) && (*((unsigned char **)im->cs.data)))
          evas_common_convert_yuv_422p_709_rgba(im->cs.data,
                                                (DATA8 *)im->image.data,
                                                im->cache_entry.w,
                                                im->cache_entry.h);
        break;
      default:
        break;
     }
   im->cs.dirty = 0;
#ifdef SURFDBG
   surf_debug();
#endif
}

EAPI void
evas_common_image_colorspace_dirty(RGBA_Image *im)
{
   im->cs.dirty = 1;
   evas_common_rgba_image_scalecache_dirty(&im->cache_entry);
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_IMAGE
   if (im->pixman.im)
     {
        pixman_image_unref(im->pixman.im);
        im->pixman.im = NULL;
     }
   _evas_common_rgba_image_post_surface((Image_Entry *)im);
# endif
#endif
}

EAPI void
evas_common_image_set_cache(unsigned int size)
{
   if (eci)
     evas_cache_image_set(eci, size);
}

EAPI int
evas_common_image_get_cache(void)
{
   return evas_cache_image_get(eci);
}

EAPI RGBA_Image *
evas_common_load_image_from_mmap(Eina_File *f, const char *key,
                                 Evas_Image_Load_Opts *lo, int *error)
{
   if (!f)
     {
        *error = EVAS_LOAD_ERROR_GENERIC;
        return NULL;
     }
   return (RGBA_Image *) evas_cache_image_mmap_request(eci, f, key, lo, error);
}

EAPI void
evas_common_image_cache_free(void)
{
   evas_common_image_set_cache(0);
}

EAPI Evas_Cache_Image*
evas_common_image_cache_get(void)
{
   return eci;
}

EAPI RGBA_Image *
evas_common_image_line_buffer_obtain(int len)
{
   if (len < 1) return NULL;
   if (len < EVAS_RGBA_LINE_BUFFER_MIN_LEN)
     len = EVAS_RGBA_LINE_BUFFER_MIN_LEN;
   return evas_common_image_create(len, 1);
}

EAPI void
evas_common_image_line_buffer_release(RGBA_Image *im)
{
   _evas_common_rgba_image_delete(&im->cache_entry);
}

EAPI void
evas_common_image_line_buffer_free(RGBA_Image *im)
{
   _evas_common_rgba_image_delete(&im->cache_entry);
}

EAPI RGBA_Image *
evas_common_image_alpha_line_buffer_obtain(int len)
{
   if (len < 1) return NULL;
   if (len < EVAS_ALPHA_LINE_BUFFER_MIN_LEN)
     len = EVAS_ALPHA_LINE_BUFFER_MIN_LEN;
   return evas_common_image_alpha_create(len, 1);
}

EAPI void
evas_common_image_alpha_line_buffer_release(RGBA_Image *im)
{
   _evas_common_rgba_image_delete(&im->cache_entry);
}

EAPI void
evas_common_image_premul(Image_Entry *ie)
{
   DATA32 nas = 0;

   if (!ie) return;
   if (!evas_cache_image_pixels(ie)) return;
   if (!ie->flags.alpha) return;

   switch (ie->space)
     {
      case EVAS_COLORSPACE_ARGB8888:
        nas = evas_common_convert_argb_premul
          (evas_cache_image_pixels(ie), ie->w * ie->h);
        break;
      case EVAS_COLORSPACE_AGRY88:
        nas = evas_common_convert_ag_premul
          ((void *)evas_cache_image_pixels(ie), ie->w * ie->h);
      default: return;
     }
   if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (ie->w * ie->h))
     ie->flags.alpha_sparse = 1;
}

EAPI void
evas_common_image_set_alpha_sparse(Image_Entry *ie)
{
   DATA32 *s, *se;
   DATA32 nas = 0;

   if (!ie) return;
   if (!evas_cache_image_pixels(ie)) return;
   if (!ie->flags.alpha) return;

   s = evas_cache_image_pixels(ie);
   se = s + (ie->w * ie->h);
   while (s < se)
     {
        DATA32  p = *s & 0xff000000;

        if (!p || (p == 0xff000000)) nas++;
        s++;
     }
   if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (ie->w * ie->h))
     ie->flags.alpha_sparse = 1;
}
