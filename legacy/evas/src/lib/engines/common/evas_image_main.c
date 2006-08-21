#include "evas_common.h"
#include "evas_private.h"

#ifdef BUILD_LOADER_EET
#include <Eet.h>
#endif

#ifdef HAVE_VALGRIND
#include <memcheck.h>
#endif

static Evas_Hash        * images = NULL;
static Evas_Object_List * cache = NULL;
static int                cache_size = 0;
static int                cache_usage = 0;

static RGBA_Image *evas_rgba_line_buffer = NULL;

#define  EVAS_RGBA_LINE_BUFFER_MIN_LEN  256
#define  EVAS_RGBA_LINE_BUFFER_MAX_LEN  1024

static RGBA_Image *evas_alpha_line_buffer = NULL;

#define  EVAS_ALPHA_LINE_BUFFER_MIN_LEN  256
#define  EVAS_ALPHA_LINE_BUFFER_MAX_LEN  1024

#if 0
int
image_debug_hash_cb(Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   RGBA_Image *im;

   im = data;
   printf("  [%i] %3ix%3i %6i %6i [%2x %2x] %i %s\n",
	  im->references,
	  im->image->w, im->image->h,
	  im->image->w * im->image->h * 4,
	  evas_common_image_ram_usage(im),
	  im->flags & RGBA_IMAGE_IS_DIRTY,
	  im->flags & RGBA_IMAGE_INDEXED,
	  im->info.format,
	  im->info.file);
}

static void
image_debug(void)
{
   Evas_Object_List *l;

   printf("active images:\n");
   evas_hash_foreach(images, image_debug_hash_cb, NULL);
   printf("cache size: %i\n", cache_size);
   printf("cache usage: %i\n", cache_usage);
   printf("cached images:\n");
   for (l = cache; l; l = l->next)
     {
	RGBA_Image *im;

	im = l;
	printf("  [%i] %3ix%3i %6i %6i [%2x %2x] %i %s\n",
	       im->references,
	       im->image->w, im->image->h,
	       im->image->w * im->image->h * 4,
	       evas_common_image_ram_usage(im),
	       im->flags & RGBA_IMAGE_IS_DIRTY,
	       im->flags & RGBA_IMAGE_INDEXED,
	       im->info.format,
	       im->info.file);
     }
}
#endif

void
evas_common_image_init(void)
{
#ifdef BUILD_LOADER_EET
   eet_init();
#endif
}

void
evas_common_image_shutdown(void)
{
#ifdef BUILD_LOADER_EET
   eet_shutdown();
#endif
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

RGBA_Surface *
evas_common_image_surface_new(RGBA_Image *im)
{
   RGBA_Surface *is;

   is = calloc(1, sizeof(RGBA_Surface));
   is->im = im;
   return is;
}

void
evas_common_image_surface_free(RGBA_Surface *is)
{
   evas_common_image_surface_dealloc(is);
   free(is);
}

void
evas_common_image_surface_alloc(RGBA_Surface *is)
{
   size_t siz = 0;

   if (is->im->flags & RGBA_IMAGE_ALPHA_ONLY)
     siz = is->w * is->h * sizeof(DATA8);
   else
     siz = is->w * is->h * sizeof(DATA32);

   is->data = malloc(siz);

#ifdef HAVE_VALGRIND
   VALGRIND_MAKE_READABLE(is->data, siz);
#endif
}

void
evas_common_image_surface_dealloc(RGBA_Surface *is)
{
   if ((is->data) && (!is->no_free))
     {
	free(is->data);
	is->data = NULL;
     }
}

RGBA_Image *
evas_common_image_create(int w, int h)
{
   RGBA_Image *im;

   im = evas_common_image_new();
   if (!im) return NULL;
   im->image = evas_common_image_surface_new(im);
   if (!im->image)
     {
	evas_common_image_free(im);
	return NULL;
     }
   im->image->w = w;
   im->image->h = h;
   evas_common_image_surface_alloc(im->image);
   if (!im->image->data)
     {
	evas_common_image_free(im);
	return NULL;
     }
   im->flags = RGBA_IMAGE_IS_DIRTY;
   im->references = 1;
   return im;
}

RGBA_Image *
evas_common_image_alpha_create(int w, int h)
{
   RGBA_Image *im;

   im = evas_common_image_new();
   if (!im) return NULL;
   im->image = evas_common_image_surface_new(im);
   if (!im->image)
     {
	evas_common_image_free(im);
	return NULL;
     }
   im->image->w = w;
   im->image->h = h;
   im->flags |= RGBA_IMAGE_ALPHA_ONLY;
   evas_common_image_surface_alloc(im->image);
   if (!im->image->data)
     {
	evas_common_image_free(im);
	return NULL;
     }
   im->flags = RGBA_IMAGE_IS_DIRTY;
   im->references = 1;
   return im;
}

RGBA_Image *
evas_common_image_new(void)
{
   RGBA_Image *im;

   im = calloc(1, sizeof(RGBA_Image));
   if (!im) return NULL;
   im->flags = RGBA_IMAGE_NOTHING;
   return im;
}

void
evas_common_image_free(RGBA_Image *im)
{
   if (im->image) evas_common_image_surface_free(im->image);
   if (im->info.file) evas_stringshare_del(im->info.file);
//   if (im->info.real_file) evas_stringshare_del(im->info.real_file);
   if (im->info.key) evas_stringshare_del(im->info.key);
//   if (im->info.comment) evas_stringshare_del(im->info.comment);
   free(im);
}

void
evas_common_image_ref(RGBA_Image *im)
{
   im->references++;
   if (im->references == 1) /* we were in cache - take us out */
     {
	evas_common_image_uncache(im);
	evas_common_image_store(im);
     }
}

void
evas_common_image_unref(RGBA_Image *im)
{
   im->references--;
   if (im->references <= 0) /* we were are now in cache - put us in */
     {
	evas_common_image_unstore(im);
	if ((cache_size > 0) &&
	    (!(im->flags & RGBA_IMAGE_IS_DIRTY)))
	  {
	     evas_common_image_cache(im);
	     evas_common_image_flush_cache();
	  }
	else
	  {
	     evas_common_image_free(im);
	  }
     }
}

void
evas_common_image_cache(RGBA_Image *im)
{
   int ram;

   if (im->flags & RGBA_IMAGE_INDEXED) return;
   im->flags |= RGBA_IMAGE_INDEXED;
   cache = evas_object_list_prepend(cache, im);
   ram = evas_common_image_ram_usage(im);
   cache_usage += ram;
   evas_common_image_flush_cache();
}

void
evas_common_image_uncache(RGBA_Image *im)
{
   int ram;

   if (!(im->flags & RGBA_IMAGE_INDEXED)) return;
   im->flags &= ~RGBA_IMAGE_INDEXED;
   cache = evas_object_list_remove(cache, im);
   ram = evas_common_image_ram_usage(im);
   cache_usage -= ram;
}

void
evas_common_image_flush_cache(void)
{
   Evas_Object_List *l, *l_next;

   if (!cache) return;
   if (cache_usage < cache_size) return;

   for (l = cache->last; l;)
     {
	RGBA_Image *im;

	l_next = l->prev;
	im = (RGBA_Image *)l;
	evas_common_image_uncache(im);
	evas_common_image_free(im);
	if (cache_usage <= cache_size) return;
	l = l_next;
     }
}

void
evas_common_image_set_cache(int size)
{
   cache_size = size;
   evas_common_image_flush_cache();
}

int
evas_common_image_get_cache(void)
{
   return cache_size;
}

void
evas_common_image_store(RGBA_Image *im)
{
   char buf[4096 + 1204];

   if (im->flags & RGBA_IMAGE_IS_DIRTY) return;
   if (im->flags & RGBA_IMAGE_INDEXED) return;
   if ((!im->info.file) && (!im->info.key)) return;
   if ((im->load_opts.scale_down_by == 0) &&
       (im->load_opts.dpi == 0.0) &&
       ((im->load_opts.w == 0) || (im->load_opts.h == 0)))
     {
	if (im->info.key)
	  snprintf(buf, sizeof(buf), "%s//://%s", im->info.file, im->info.key);
	else
	  snprintf(buf, sizeof(buf), "%s", im->info.file);
     }
   else
     {
	if (im->info.key)
	  snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//%s//://%s", im->load_opts.scale_down_by, im->load_opts.dpi, im->load_opts.w, im->load_opts.h, im->info.file, im->info.key);
	else
	  snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//%s", im->load_opts.scale_down_by, im->load_opts.dpi, im->load_opts.w, im->load_opts.h, im->info.file);
     }
   images = evas_hash_add(images, buf, im);
   im->flags |= RGBA_IMAGE_INDEXED;
}

void
evas_common_image_unstore(RGBA_Image *im)
{
   char buf[4096 + 1024];

   if (!(im->flags & RGBA_IMAGE_INDEXED)) return;
   if ((!im->info.file) && (!im->info.key)) return;
   if ((im->load_opts.scale_down_by == 0) &&
       (im->load_opts.dpi == 0.0) &&
       ((im->load_opts.w == 0) || (im->load_opts.h == 0)))
     {
	if (im->info.key)
	  snprintf(buf, sizeof(buf), "%s//://%s", im->info.file, im->info.key);
	else
	  snprintf(buf, sizeof(buf), "%s", im->info.file);
     }
   else
     {
	if (im->info.key)
	  snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//%s//://%s", im->load_opts.scale_down_by, im->load_opts.dpi, im->load_opts.w, im->load_opts.h, im->info.file, im->info.key);
	else
	  snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//%s", im->load_opts.scale_down_by, im->load_opts.dpi, im->load_opts.w, im->load_opts.h, im->info.file);
     }
   images = evas_hash_del(images, buf, im);
   im->flags &= ~RGBA_IMAGE_INDEXED;
}


RGBA_Image *
evas_common_image_find(const char *file, const char *key, DATA64 timestamp, RGBA_Image_Loadopts *lo)
{
   RGBA_Image *im;
   char buf[4096 + 1024];

   if ((!file) && (!key)) return NULL;
   if (!file) return NULL;
   if (!lo)
     {
	if (key)
	  snprintf(buf, sizeof(buf), "%s//://%s", file, key);
	else
	  snprintf(buf, sizeof(buf), "%s", file);
     }
   else
     {
	if (key)
	  snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//%s//://%s", lo->scale_down_by, lo->dpi, lo->w, lo->h, file, key);
	else
	  snprintf(buf, sizeof(buf), "//@/%i/%1.5f/%ix%i//%s", lo->scale_down_by, lo->dpi, lo->w, lo->h, file);
     }
   im = evas_hash_find(images, buf);
   if (im) return im;
/*
   for (l = cache; l; l = l->next)
     {
	int ok;

	im = (RGBA_Image *)l;
	ok = 0;
	if ((file) && (im->info.file) &&
	    (!strcmp(file, im->info.file)))
	  ok++;
	if ((!file) && (!im->info.file))
	  ok++;
	if ((key) && (im->info.key) &&
	    (!strcmp(key, im->info.key)))
	  ok++;
	if ((!key) && (!im->info.key))
	  ok++;
//	if (im->timestamp == timestamp)
//	  ok++;
	if (ok >= 2) return im;
     }
 */
   return NULL;
}

int
evas_common_image_ram_usage(RGBA_Image *im)
{
   int ram = 0;

   ram += sizeof(struct _RGBA_Image);
   if (im->info.file) ram += strlen(im->info.file);
//   if (im->info.real_file) ram += strlen(im->info.real_file);
   if (im->info.key) ram += strlen(im->info.key);
//   if (im->info.comment) ram += strlen(im->info.comment);
   if ((im->image) && (im->image->data) && (!im->image->no_free))
     ram += im->image->w * im->image->h * sizeof(DATA32);
   return ram;
}

void
evas_common_image_dirty(RGBA_Image *im)
{
   evas_common_image_unstore(im);
   im->flags |= RGBA_IMAGE_IS_DIRTY;
}

void
evas_common_image_cache_free(void)
{
   evas_common_image_set_cache(0);
}

RGBA_Image *
evas_common_image_line_buffer_obtain(int len)
{
   if (len < 1) return NULL;
   if (len < EVAS_RGBA_LINE_BUFFER_MIN_LEN)
	len = EVAS_RGBA_LINE_BUFFER_MIN_LEN;
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
}

void
evas_common_image_line_buffer_release(void)
{
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
}

void
evas_common_image_line_buffer_free(void)
{
   if (!evas_rgba_line_buffer) return;
   evas_common_image_free(evas_rgba_line_buffer);
   evas_rgba_line_buffer = NULL;
}

RGBA_Image *
evas_common_image_alpha_line_buffer_obtain(int len)
{
   if (len < 1) return NULL;
   if (len < EVAS_ALPHA_LINE_BUFFER_MIN_LEN)
	len = EVAS_ALPHA_LINE_BUFFER_MIN_LEN;
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
}

void
evas_common_image_alpha_line_buffer_release(void)
{
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
}

void
evas_common_image_alpha_line_buffer_free(void)
{
   if (!evas_alpha_line_buffer) return;
   evas_common_image_free(evas_alpha_line_buffer);
   evas_alpha_line_buffer = NULL;
}
