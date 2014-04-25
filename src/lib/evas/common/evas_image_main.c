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
# include <memcheck.h>
#endif

//#define SURFDBG 1

static Evas_Cache_Image * eci = NULL;
#ifdef EVAS_CSERVE2
#define EVAS_CSERVE2_SCALE_CACHE_SIZE (4 * 1024 * 1024)
static Evas_Cache2      * eci2 = NULL;
#endif
static int                reference = 0;

/* static RGBA_Image *evas_rgba_line_buffer = NULL; */

#define  EVAS_RGBA_LINE_BUFFER_MIN_LEN  256
#define  EVAS_RGBA_LINE_BUFFER_MAX_LEN  2048

/* static RGBA_Image *evas_alpha_line_buffer = NULL; */

#define  EVAS_ALPHA_LINE_BUFFER_MIN_LEN  256
#define  EVAS_ALPHA_LINE_BUFFER_MAX_LEN  2048


static Image_Entry      *_evas_common_rgba_image_new(void);
static void              _evas_common_rgba_image_delete(Image_Entry *ie);

static int               _evas_common_rgba_image_surface_alloc(Image_Entry *ie, unsigned int w, unsigned int h);
static void              _evas_common_rgba_image_surface_delete(Image_Entry *ie);
static DATA32           *_evas_common_rgba_image_surface_pixels(Image_Entry *ie);

static void              _evas_common_rgba_image_unload(Image_Entry *im);

static void              _evas_common_rgba_image_dirty_region(Image_Entry *im, unsigned int x, unsigned int y, unsigned int w, unsigned int h);

static int               _evas_common_rgba_image_ram_usage(Image_Entry *ie);

/* Only called when references > 0. Need to provide a fresh copie of im. */
/* The destination surface does have a surface, but no allocated pixel data. */
static int               _evas_common_rgba_image_dirty(Image_Entry* dst, const Image_Entry* src);

#if 0
static void
_evas_common_rgba_image_debug(const char* context, Image_Entry *eim)
{
  DBG("%p = [%s] {%s,%s} %i [%i|%i]", eim, context, eim->file, eim->key, eim->references, eim->w, eim->h);
}
#endif

static const Evas_Cache_Image_Func      _evas_common_image_func =
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
/*   _evas_common_rgba_image_debug */
  NULL
};

#ifdef EVAS_CSERVE2
static const Evas_Cache2_Image_Func      _evas_common_image_func2 =
{
  // _evas_common_rgba_image_new,
  // _evas_common_rgba_image_delete,
  _evas_common_rgba_image_surface_alloc,
  _evas_common_rgba_image_surface_delete,
  _evas_common_rgba_image_surface_pixels,
  // evas_common_load_rgba_image_module_from_file,
  // _evas_common_rgba_image_unload,
  NULL, // _evas_common_rgba_image_dirty_region,
  NULL, // _evas_common_rgba_image_dirty,
  evas_common_rgba_image_size_set,
  evas_common_rgba_image_from_copied_data,
  evas_common_rgba_image_from_data,
  NULL, // evas_common_rgba_image_colorspace_set,
  // evas_common_load_rgba_image_data_from_file,
  _evas_common_rgba_image_ram_usage,
/*   _evas_common_rgba_image_debug */
  NULL
};
#endif

static inline int
_evas_common_rgba_image_surface_size(unsigned int w, unsigned int h, Evas_Colorspace cspace)
{
#define PAGE_SIZE (4 * 1024)
#define HUGE_PAGE_SIZE (2 * 1024 * 1024)
#if defined (HAVE_SYS_MMAN_H) && (!defined (_WIN32))
# define ALIGN_TO_PAGE(Siz) (((Siz / PAGE_SIZE) + (Siz % PAGE_SIZE ? 1 : 0)) * PAGE_SIZE)
#else
# define ALIGN_TO_PAGE(Siz) Siz
#endif
   int siz;

   switch (cspace)
     {
      case EVAS_COLORSPACE_GRY8: siz = w * h * sizeof(DATA8); break;
      case EVAS_COLORSPACE_AGRY88: siz = w * h * sizeof(DATA16); break;
      case EVAS_COLORSPACE_ETC1:
      case EVAS_COLORSPACE_RGB8_ETC2:
         // Need to round width and height independently
         w += 2; h += 2; // We do duplicate border in ETC1 to have better rendering on GPU.
         siz = (w / 4 + (w % 4 ? 1 : 0)) * (h / 4 + (h % 4 ? 1 : 0)) * 8;
         break;
      case EVAS_COLORSPACE_RGBA8_ETC2_EAC:
         w += 2; h += 2;
         siz = (w / 4 + (w % 4 ? 1 : 0)) * (h / 4 + (h % 4 ? 1 : 0)) * 16;
         break;
      default:
      case EVAS_COLORSPACE_ARGB8888: siz = w * h * sizeof(DATA32); break;
     }

   if (siz < PAGE_SIZE) return siz;

   return ALIGN_TO_PAGE(siz);

#undef ALIGN_TO_PAGE
}

static void *
_evas_common_rgba_image_surface_mmap(unsigned int w, unsigned int h, Evas_Colorspace cspace)
{
   int siz;
#if defined (HAVE_SYS_MMAN_H) && (!defined (_WIN32))
   void *r = MAP_FAILED;
#endif

   siz = _evas_common_rgba_image_surface_size(w, h, cspace);

#if defined (HAVE_SYS_MMAN_H) && (!defined (_WIN32))
#ifndef MAP_HUGETLB
# define MAP_HUGETLB 0
#endif
   if (siz < 0)
     return NULL;

   if (siz < PAGE_SIZE)
     return malloc(siz);

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

static void
_evas_common_rgba_image_surface_munmap(void *data, unsigned int w, unsigned int h, Evas_Colorspace cspace)
{
   if (!data) return ;
#if defined (HAVE_SYS_MMAN_H) && (!defined (_WIN32))
   size_t siz;

   siz = _evas_common_rgba_image_surface_size(w, h, cspace);
   if (siz < PAGE_SIZE)
     free(data);
   else
     munmap(data, siz);
#else
   free(data);
#endif
}

EAPI void
evas_common_image_init(void)
{
   if (!eci)
     eci = evas_cache_image_init(&_evas_common_image_func);
#ifdef EVAS_CSERVE2
   if (!eci2)
     eci2 = evas_cache2_init(&_evas_common_image_func2);
#endif
   reference++;
////   ERR("REF++=%i", reference);

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
#ifdef EVAS_CSERVE2
       evas_cache2_shutdown(eci2);
       eci2 = NULL;
#endif
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
   /* memset the image to 0x99 because i recently saw a segv where an
    * seemed to be used BUT its contents were wrong - it looks like it was
    * overwritten by something from efreet - as there was an execute command
    * for a command there and some other signs - but to make sure, I am
    * going to empty this struct out in case this happens again so i know
    * that something else is overwritign this struct - or not */
//   memset(im, 0x99, sizeof(im));
#ifdef EVAS_CSERVE2
   if (ie->data1)
       ERR("Shouldn't reach this point since we are using cache2: '%s' '%s'",
           ie->file, ie->key);
   // if (ie->data1) evas_cserve2_image_free(ie);
#endif

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
   free(im);
}

EAPI void
evas_common_rgba_image_free(Image_Entry *ie)
{
   if (ie->references > 0) return;

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
   RGBA_Image   *im = (RGBA_Image *) ie;

   if (!ie->flags.loaded) return;
   if ((!ie->info.module) && (!ie->data1)) return;
   if (!ie->file && !ie->f) return;
   if ((evas_cache_async_frozen_get() == 0) &&
       (ie->references > 0)) return;

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

#ifdef EVAS_CSERVE2
   if (ie->data1)
     {
        ERR("Shouldn't reach this point since we are using cache2.");
//         evas_cserve2_image_unload(ie);
//         im->image.data = NULL;
//         ie->allocated.w = 0;
//         ie->allocated.h = 0;
//         ie->flags.loaded = 0;
        ie->flags.preload_done = 0;
        return;
     }
#endif

   if (im->image.data && !im->image.no_free)
     {
        _evas_common_rgba_image_surface_munmap(im->image.data,
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
#ifdef SURFDBG
   surf_debug();
#endif   
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
// FIXME: endianess determines this
            PIXMAN_a8r8g8b8,
//            PIXMAN_b8g8r8a8,
            w, h, im->image.data, w * 4);
     }
   else
     {
        im->pixman.im = pixman_image_create_bits
        (
// FIXME: endianess determines this
            PIXMAN_x8r8g8b8,
//            PIXMAN_b8g8r8x8,
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
_evas_common_rgba_image_surface_alloc(Image_Entry *ie, unsigned int w, unsigned int h)
{
   RGBA_Image   *im = (RGBA_Image *) ie;

#ifdef EVAS_CSERVE2
   if (ie->data1) return 0;
#endif
   if (im->image.no_free) return 0;

   if (im->image.data)
     {
        _evas_common_rgba_image_surface_munmap(im->image.data,
                                               ie->allocated.w, ie->allocated.h,
                                               ie->space);
	im->image.data = NULL;
#ifdef SURFDBG
        surfs = eina_list_remove(surfs, ie);
#endif
     }

   im->image.data = _evas_common_rgba_image_surface_mmap(w, h, ie->space);
   if (!im->image.data) return -1;
   ie->allocated.w = w;
   ie->allocated.h = h;
#ifdef SURFDBG
   surfs = eina_list_append(surfs, ie);
#endif
#ifdef HAVE_VALGRIND
   int        siz = 0;
   siz = _evas_common_rgba_image_surface_size(w, h, ie->space);
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
   if (ie->file)
     DBG("unload: [%p] %s %s", ie, ie->file, ie->key);
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
        _evas_common_rgba_image_surface_munmap(im->image.data,
                                               ie->allocated.w, ie->allocated.h,
                                               ie->space);
#ifdef SURFDBG
        surfs = eina_list_remove(surfs, ie);
#endif
     }
// #ifdef EVAS_CSERVE2
//    else if (ie->data1)
//      ERR("Shouldn't reach this point since we are using cache2.");
//      // evas_cserve2_image_free(ie);
// #endif

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
//   DBG("unload: [%p] %s %s", im, im->file, im->key);
   evas_common_rgba_image_scalecache_dirty(im);
   evas_common_rgba_image_unload(im);
}

static void
_evas_common_rgba_image_dirty_region(Image_Entry* ie, unsigned int x EINA_UNUSED, unsigned int y EINA_UNUSED, unsigned int w EINA_UNUSED, unsigned int h EINA_UNUSED)
{
   RGBA_Image   *im = (RGBA_Image *) ie;

#ifdef EVAS_CSERVE2
   // if (ie->data1) evas_cserve2_image_free(ie);
   if (ie->data1) ERR("Shouldn't reach this point since we are using cache2.");
#endif
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
                                                  src->cache_entry.w, src->cache_entry.h))
          {
#ifdef EVAS_CSERVE2
             // if (ie_src->data1) evas_cserve2_image_free((Image_Entry*) ie_src);
             if (ie_src->data1) ERR("Shouldn't reach this point since we are using cache2.");
#endif
             return 1;
          }
     }
#ifdef EVAS_CSERVE2
   // if (ie_src->data1) evas_cserve2_image_free((Image_Entry*) ie_src);
   if (ie_src->data1) ERR("Shouldn't reach this point since we are using cache2.");
#endif
   evas_common_image_colorspace_normalize(src);
   evas_common_image_colorspace_normalize(dst);
/*    evas_common_blit_rectangle(src, dst, 0, 0, src->cache_entry.w, src->cache_entry.h, 0, 0); */
/*    evas_common_cpu_end_opt(); */

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

   if (im->image.data)
     {
#ifdef EVAS_CSERVE2
        if ((!im->image.no_free) || (ie->data1))
#else
        if ((!im->image.no_free))
#endif
          size += im->cache_entry.w * im->cache_entry.h * sizeof(DATA32);
     }
   size += evas_common_rgba_image_scalecache_usage_get(&im->cache_entry);
   return size;
}

static DATA32 *
_evas_common_rgba_image_surface_pixels(Image_Entry *ie)
{
   RGBA_Image *im = (RGBA_Image *) ie;

   return im->image.data;
}

#if 0
void
evas_common_image_surface_alpha_tiles_calc(RGBA_Surface *is, int tsize)
{
   int x, y;
   DATA32 *ptr;

   if (is->spans) return;
   if (!is->im->cache_entry.flags.alpha) return;
   /* FIXME: dont handle alpha only images yet */
   if (is->im->space != EVAS_COLORSPACE_GRY8) return;
   if (tsize < 0) tsize = 0;
   is->spans = calloc(1, sizeof(RGBA_Image_Span *) * is->h);
   if (!is->spans) return;
   ptr = is->data;
   for (y = 0; y < is->h; y++)
     {
	RGBA_Image_Span *sp;

	sp = NULL;
	for (x = 0; x < is->w; x++)
	  {
	     DATA8 a;

	     a = A_VAL(ptr);
	     if (sp)
	       {
		  if (a == 0)
		    {
		       is->spans[y] = eina_inlist_append(is->spans[y], sp);
		       sp = NULL;
		    }
		  else
		    {
		       sp->w++;
		       if ((sp->v == 2) && (a != 255)) sp->v = 1;
		    }
	       }
	     else
	       {
		  if (a == 255)
		    {
		       sp = calloc(1, sizeof(RGBA_Image_Span));
		       sp->x = x;
		       sp->w = 1;
		       sp->v = 2;
		    }
		  else if (a > 0)
		    {
		       sp = calloc(1, sizeof(RGBA_Image_Span));
		       sp->x = x;
		       sp->w = 1;
		       sp->v = 1;
		    }
	       }
	     ptr++;
	  }
	if (sp)
	  {
	     is->spans[y] = eina_inlist_append(is->spans[y], sp);
	     sp = NULL;
	  }
     }
}
#endif

/* EAPI void */
/* evas_common_image_surface_dealloc(RGBA_Surface *is) */
/* { */
/*    if ((is->data) && (!is->no_free)) */
/*      { */
/*	free(is->data); */
/*	is->data = NULL; */
/*      } */
/* } */

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
   RGBA_Image   *im;

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
   if (alpha)
     return evas_common_image_alpha_create(w, h);
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
#ifdef EVAS_CSERVE2
             // if (((Image_Entry *)im)->data1) evas_cserve2_image_free(&im->cache_entry);
             if (((Image_Entry *)im)->data1) ERR("Shouldn't reach this point since we are using cache2.");
#endif
	     if (!im->image.no_free)
               {
                  _evas_common_rgba_image_surface_munmap(im->image.data,
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
	  evas_common_convert_yuv_420p_601_rgba(im->cs.data, (DATA8*) im->image.data,
						im->cache_entry.w, im->cache_entry.h);
	break;
      case EVAS_COLORSPACE_YCBCR422601_PL:
        if ((im->image.data) && (*((unsigned char **)im->cs.data)))
          evas_common_convert_yuv_422_601_rgba(im->cs.data, (DATA8*) im->image.data,
                                               im->cache_entry.w, im->cache_entry.h);
        break;
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
        if ((im->image.data) && (*((unsigned char **)im->cs.data)))
          evas_common_convert_yuv_420_601_rgba(im->cs.data, (DATA8*) im->image.data,
                                               im->cache_entry.w, im->cache_entry.h);
        break;
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
        if ((im->image.data) && (*((unsigned char **)im->cs.data)))
          evas_common_convert_yuv_420T_601_rgba(im->cs.data, (DATA8*) im->image.data,
                                                im->cache_entry.w, im->cache_entry.h);
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
#ifdef EVAS_CSERVE2
   if (eci2)
     evas_cache2_limit_set(eci2, size + EVAS_CSERVE2_SCALE_CACHE_SIZE);
#endif
}

EAPI int
evas_common_image_get_cache(void)
{
   return evas_cache_image_get(eci);
}

EAPI RGBA_Image *
evas_common_load_image_from_file(const char *file, const char *key, Evas_Image_Load_Opts *lo, int *error)
{
   if (!file)
     {
	*error = EVAS_LOAD_ERROR_GENERIC;
	return NULL;
     }
   return (RGBA_Image *) evas_cache_image_request(eci, file, key, lo, error);
}

EAPI RGBA_Image *
evas_common_load_image_from_mmap(Eina_File *f, const char *key, Evas_Image_Load_Opts *lo, int *error)
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

#ifdef EVAS_CSERVE2
EAPI Evas_Cache2*
evas_common_image_cache2_get(void)
{
   return eci2;
}
#endif

EAPI RGBA_Image *
evas_common_image_line_buffer_obtain(int len)
{
   if (len < 1) return NULL;
   if (len < EVAS_RGBA_LINE_BUFFER_MIN_LEN)
	len = EVAS_RGBA_LINE_BUFFER_MIN_LEN;
   return evas_common_image_create(len, 1);
/*
   if (evas_rgba_line_buffer)
     {
	if (evas_rgba_line_buffer->image->w >= len)
	   return evas_rgba_line_buffer;
	evas_rgba_line_buffer->image->data = (DATA32 *)realloc(evas_rgba_line_buffer->image->data, len * sizeof(DATA32));
	if (!evas_rgba_line_buffer->image->data)
	  {
	   evas_common_image_free(evas_rgba_line_buffer);
	   evas_rgba_line_buffer = NULL;
	   return NULL;
	  }
	evas_rgba_line_buffer->image->w = len;
	return evas_rgba_line_buffer;
     }
   evas_rgba_line_buffer = evas_common_image_create(len, 1);
   if (!evas_rgba_line_buffer) return NULL;
   return evas_rgba_line_buffer;
 */
}

EAPI void
evas_common_image_line_buffer_release(RGBA_Image *im)
{
    _evas_common_rgba_image_delete(&im->cache_entry);
/*
   if (!evas_rgba_line_buffer) return;
   if (EVAS_RGBA_LINE_BUFFER_MAX_LEN < evas_rgba_line_buffer->image->w)
     {
	evas_rgba_line_buffer->image->w = EVAS_RGBA_LINE_BUFFER_MAX_LEN;
	evas_rgba_line_buffer->image->data = (DATA32 *)realloc(evas_rgba_line_buffer->image->data,
	                         evas_rgba_line_buffer->image->w * sizeof(DATA32));
	if (!evas_rgba_line_buffer->image->data)
	  {
	   evas_common_image_free(evas_rgba_line_buffer);
	   evas_rgba_line_buffer = NULL;
	  }
     }
 */
}

EAPI void
evas_common_image_line_buffer_free(RGBA_Image *im)
{
    _evas_common_rgba_image_delete(&im->cache_entry);
/*
   if (!evas_rgba_line_buffer) return;
   evas_common_image_free(evas_rgba_line_buffer);
   evas_rgba_line_buffer = NULL;
 */
}

EAPI RGBA_Image *
evas_common_image_alpha_line_buffer_obtain(int len)
{
   if (len < 1) return NULL;
   if (len < EVAS_ALPHA_LINE_BUFFER_MIN_LEN)
	len = EVAS_ALPHA_LINE_BUFFER_MIN_LEN;
   return evas_common_image_alpha_create(len, 1);
/*
   if (evas_alpha_line_buffer)
     {
	if (evas_alpha_line_buffer->image->w >= len)
	   return evas_alpha_line_buffer;
	evas_alpha_line_buffer->image->data = realloc(evas_alpha_line_buffer->image->data, len * sizeof(DATA8));
	if (!evas_alpha_line_buffer->image->data)
	  {
	   evas_common_image_free(evas_alpha_line_buffer);
	   evas_alpha_line_buffer = NULL;
	   return NULL;
	  }
	evas_alpha_line_buffer->image->w = len;
	return evas_alpha_line_buffer;
     }
   evas_alpha_line_buffer = evas_common_image_alpha_create(len, 1);
   return evas_alpha_line_buffer;
 */
}

EAPI void
evas_common_image_alpha_line_buffer_release(RGBA_Image *im)
{
    _evas_common_rgba_image_delete(&im->cache_entry);
/*
   if (!evas_alpha_line_buffer) return;
   if (EVAS_ALPHA_LINE_BUFFER_MAX_LEN < evas_alpha_line_buffer->image->w)
     {
	evas_alpha_line_buffer->image->w = EVAS_ALPHA_LINE_BUFFER_MAX_LEN;
	evas_alpha_line_buffer->image->data = realloc(evas_alpha_line_buffer->image->data,
	                         evas_alpha_line_buffer->image->w * sizeof(DATA8));
	if (!evas_alpha_line_buffer->image->data)
	  {
	   evas_common_image_free(evas_alpha_line_buffer);
	   evas_alpha_line_buffer = NULL;
	  }
     }
 */
}

EAPI void
evas_common_image_premul(Image_Entry *ie)
{
   DATA32  nas = 0;

   if (!ie) return;
   if (!evas_cache_image_pixels(ie)) return;
   if (!ie->flags.alpha) return;

   switch (ie->space)
     {
      case EVAS_COLORSPACE_ARGB8888:
         nas = evas_common_convert_argb_premul(evas_cache_image_pixels(ie), ie->w * ie->h);
         break;
      case EVAS_COLORSPACE_AGRY88:
         nas = evas_common_convert_ag_premul((void*) evas_cache_image_pixels(ie), ie->w * ie->h);
      default: return;
     }
   if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (ie->w * ie->h))
     ie->flags.alpha_sparse = 1;
}

EAPI void
evas_common_image_set_alpha_sparse(Image_Entry *ie)
{
   DATA32  *s, *se;
   DATA32  nas = 0;

   if (!ie) return;
   if (!evas_cache_image_pixels(ie)) return;
   if (!ie->flags.alpha) return;

   s = evas_cache_image_pixels(ie);
   se = s + (ie->w * ie->h);
   while (s < se)
     {
	DATA32  p = *s & 0xff000000;

	if (!p || (p == 0xff000000))
	   nas++;
	s++;
     }
   if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (ie->w * ie->h))
     ie->flags.alpha_sparse = 1;
}
