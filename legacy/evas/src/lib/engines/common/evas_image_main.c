#include "evas_common.h"
#include "evas_private.h"

#ifdef BUILD_LOADER_EET
#include <Eet.h>
#endif

#ifdef HAVE_VALGRIND
#include <memcheck.h>
#endif

static Evas_Cache_Image * eci = NULL;
static int                reference = 0;

/* static RGBA_Image *evas_rgba_line_buffer = NULL; */

#define  EVAS_RGBA_LINE_BUFFER_MIN_LEN  256
#define  EVAS_RGBA_LINE_BUFFER_MAX_LEN  2048

/* static RGBA_Image *evas_alpha_line_buffer = NULL; */

#define  EVAS_ALPHA_LINE_BUFFER_MIN_LEN  256
#define  EVAS_ALPHA_LINE_BUFFER_MAX_LEN  2048

/* The cache is doing the allocation and deallocation, you must just do the rest. */
static void         _evas_common_image_unload(RGBA_Image* im);

static void         _evas_common_image_dirty_region(RGBA_Image* im, int x, int y, int w, int h);

/* Only called when references > 0. Need to provide a fresh copie of im. */
/* The destination surface does have a surface, but no allocated pixel data. */
static int          _evas_common_image_dirty(RGBA_Image* dst, const RGBA_Image* src);

static const Evas_Cache_Image_Func      _evas_common_image_func =
{
   evas_common_load_image_module_from_file,
   _evas_common_image_unload,
   _evas_common_image_dirty_region,
   _evas_common_image_dirty,
   evas_common_image_size_set,
   evas_common_image_from_copied_data,
   evas_common_image_from_data,
   evas_common_image_colorspace_set,
   evas_common_load_image_data_from_file,
   evas_common_image_ram_usage,
   NULL
};

EAPI void
evas_common_image_init(void)
{
   if (!eci)
     eci = evas_cache_image_init(&_evas_common_image_func);
   reference++;
////   printf("REF++=%i\n", reference);

#ifdef BUILD_LOADER_EET
   eet_init();
#endif
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
//        evas_cache_image_shutdown(eci);
//        eci = NULL;
     }

#ifdef BUILD_LOADER_EET
   eet_shutdown();
#endif
}

static void
_evas_common_image_unload(RGBA_Image* im)
{
}

static void
_evas_common_image_dirty_region(RGBA_Image* im, int x, int y, int w, int h)
{
}

void
evas_common_image_delete(RGBA_Image* im)
{
   if ((im->cs.data) && (im->image->data))
     {
	if (im->cs.data != im->image->data)
	  {
	     if (!im->cs.no_free) free(im->cs.data);
	  }
     }
   else if (im->cs.data)
     {
	if (!im->cs.no_free) free(im->cs.data);
     }

   im->cs.data = NULL;
   evas_common_pipe_free(im);
   if (im->image) evas_common_image_surface_free(im->image);
   if (im->info.file) evas_stringshare_del(im->info.file);
   if (im->info.key) evas_stringshare_del(im->info.key);
   if (im->info.module) evas_module_unref((Evas_Module *)im->info.module);
   /* memset the image to 0x99 because i recently saw a segv where an
    * seemed to be used BUT its contents were wrong - it looks like it was
    * overwritten by something from efreet - as there was an execute command
    * for a command there and some other signs - but to make sure, I am
    * going to empty this struct out in case this happens again so i know
    * that something else is overwritign this struct - or not */
   memset(im, 0x99, sizeof(im));
   free(im);
}

/* Only called when references > 0. Need to provide a fresh copie of im. */
static int
_evas_common_image_dirty(RGBA_Image* dst, const RGBA_Image* src)
{
   evas_common_load_image_data_from_file(src);
   evas_common_image_colorspace_normalize(src);
   evas_common_image_colorspace_normalize(dst);
   evas_common_image_surface_alloc(dst->image);
   evas_common_blit_rectangle(src, dst, 0, 0, src->image->w, src->image->h, 0, 0);
   evas_common_cpu_end_opt();

   return 0;
}

#if 0
void
evas_common_image_surface_alpha_tiles_calc(RGBA_Surface *is, int tsize)
{
   int x, y;
   DATA32 *ptr;

   if (is->spans) return;
   if (!(is->im->flags & RGBA_IMAGE_HAS_ALPHA)) return;
   /* FIXME: dont handle alpha only images yet */
   if ((is->im->flags & RGBA_IMAGE_ALPHA_ONLY)) return;
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
		       is->spans[y] = evas_object_list_append(is->spans[y], sp);
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
	     is->spans[y] = evas_object_list_append(is->spans[y], sp);
	     sp = NULL;
	  }
     }
}
#endif

EAPI RGBA_Surface *
evas_common_image_surface_new(RGBA_Image *im)
{
   RGBA_Surface *is;

   is = calloc(1, sizeof(RGBA_Surface));
   is->im = im;
   return is;
}

EAPI void
evas_common_image_surface_free(RGBA_Surface *is)
{
   evas_common_image_surface_dealloc(is);
   free(is);
}

EAPI void
evas_common_image_surface_alloc(RGBA_Surface *is)
{
   size_t siz = 0;

   if (is->data) return ;

   if (is->im->flags & RGBA_IMAGE_ALPHA_ONLY)
     siz = is->w * is->h * sizeof(DATA8);
   else
     siz = is->w * is->h * sizeof(DATA32);

   is->data = malloc(siz);

#ifdef HAVE_VALGRIND
   VALGRIND_MAKE_READABLE(is->data, siz);
#endif
}

EAPI void
evas_common_image_surface_dealloc(RGBA_Surface *is)
{
   if ((is->data) && (!is->no_free))
     {
	free(is->data);
	is->data = NULL;
     }
}

EAPI RGBA_Image *
evas_common_image_create(int w, int h)
{
   RGBA_Image *im;

   im = evas_common_image_new();
   if (!im) return NULL;
   im->image = evas_common_image_surface_new(im);
   if (!im->image)
     {
	evas_common_image_delete(im);
	return NULL;
     }
   im->image->w = w;
   im->image->h = h;
   evas_common_image_surface_alloc(im->image);
   if (!im->image->data)
     {
	evas_common_image_delete(im);
	return NULL;
     }
   im->flags = RGBA_IMAGE_IS_DIRTY;
   im->references = 1;
   return im;
}

EAPI RGBA_Image *
evas_common_image_alpha_create(int w, int h)
{
   RGBA_Image *im;

   im = evas_common_image_new();
   if (!im) return NULL;
   im->image = evas_common_image_surface_new(im);
   if (!im->image)
     {
	evas_common_image_delete(im);
	return NULL;
     }
   im->image->w = w;
   im->image->h = h;
   im->flags |= RGBA_IMAGE_ALPHA_ONLY;
   evas_common_image_surface_alloc(im->image);
   if (!im->image->data)
     {
	evas_common_image_delete(im);
	return NULL;
     }
   im->flags = RGBA_IMAGE_IS_DIRTY;
   im->references = 1;
   return im;
}

EAPI RGBA_Image *
evas_common_image_new(void)
{
   RGBA_Image *im;

   im = calloc(1, sizeof(RGBA_Image));
   if (!im) return NULL;
   im->flags = RGBA_IMAGE_NOTHING;
   im->ref = 1;
   im->cs.space = EVAS_COLORSPACE_ARGB8888;
   return im;
}

EAPI void
evas_common_image_colorspace_normalize(RGBA_Image *im)
{
   if ((!im->cs.data) || 
       ((!im->cs.dirty) && (!(im->flags & RGBA_IMAGE_IS_DIRTY)))) return;
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (im->image->data != im->cs.data)
	  {
	     if (!im->image->no_free) free(im->image->data);
	     im->image->data = im->cs.data;
	     im->cs.no_free = im->image->no_free;
	  }
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
#ifdef BUILD_CONVERT_YUV
	if ((im->image->data) && (*((unsigned char **)im->cs.data)))
	  evas_common_convert_yuv_420p_601_rgba(im->cs.data, (DATA8*) im->image->data,
						im->image->w, im->image->h);
#endif
	break;
      default:
	break;
     }
   im->cs.dirty = 0;
}

EAPI void
evas_common_image_colorspace_dirty(RGBA_Image *im)
{
   im->cs.dirty = 1;
}

EAPI void
evas_common_image_set_cache(int size)
{
   if (eci != NULL)
     evas_cache_image_set(eci, size);
}
EAPI int
evas_common_image_get_cache(void)
{
   return evas_cache_image_get(eci);
}

#define STAT_GAP 2

EAPI RGBA_Image *
evas_common_load_image_from_file(const char *file, const char *key, RGBA_Image_Loadopts *lo)
{
   int error;

   if (file == NULL) return NULL;
   return evas_cache_image_request(eci, file, key, lo, &error);
}

EAPI int
evas_common_image_ram_usage(RGBA_Image *im)
{
   int ram = 0;

//   ram += sizeof(struct _RGBA_Image);
//   if (im->info.file) ram += strlen(im->info.file);
//   if (im->info.real_file) ram += strlen(im->info.real_file);
//   if (im->info.key) ram += strlen(im->info.key);
//   if (im->info.comment) ram += strlen(im->info.comment);
   if ((im->image) && (im->image->data) && (!im->image->no_free))
     ram += im->image->w * im->image->h * sizeof(DATA32);
   return ram;
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
   evas_common_image_delete(im);
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
   evas_common_image_delete(im);
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
   evas_common_image_delete(im);
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
evas_common_image_premul(RGBA_Image *im)
{
   DATA32  *s, *se;
   DATA32  nas = 0;

   if (!im || !im->image || !im->image->data) return;
   if (!(im->flags & RGBA_IMAGE_HAS_ALPHA)) return;

   s = im->image->data;
   se = s + (im->image->w * im->image->h);
   while (s < se)
     {
	DATA32  a = 1 + (*s >> 24);

	*s = (*s & 0xff000000) + (((((*s) >> 8) & 0xff) * a) & 0xff00) + 
			 (((((*s) & 0x00ff00ff) * a) >> 8) & 0x00ff00ff);
	s++;
	if ((a == 1) || (a == 256))
	   nas++;
     }
   if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (im->image->w * im->image->h))
	im->flags |= RGBA_IMAGE_ALPHA_SPARSE;
}

EAPI void
evas_common_image_set_alpha_sparse(RGBA_Image *im)
{
   DATA32  *s, *se;
   DATA32  nas = 0;

   if (!im || !im->image || !im->image->data) return;
   if (!(im->flags & RGBA_IMAGE_HAS_ALPHA)) return;

   s = im->image->data;
   se = s + (im->image->w * im->image->h);
   while (s < se)
     {
	DATA32  p = *s & 0xff000000;

	if (!p || (p == 0xff000000))
	   nas++;
	s++;
     }
   if ((ALPHA_SPARSE_INV_FRACTION * nas) >= (im->image->w * im->image->h))
	im->flags |= RGBA_IMAGE_ALPHA_SPARSE;
}
