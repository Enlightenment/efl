/**
 * Software 16 image cache system.
 *
 * Images backed by files are registered in a hash table (_soft16_cache.hash)
 * for fast lookup of existing, thus avoiding duplication of data.
 *
 * Images considered live, or in use, are those with im->references >
 * 0, these are NOT accounted in the cache limits, but they're still
 * available in the hash table for lookup.
 *
 * As soon as some image is not in use anymore (im->references == 0),
 * it's put into cache and appended to a linked list
 * (_soft16_cache.lru) if it fit into limits, if required, older
 * elements will be removed to make room.
 *
 * Images that are tainted (pixels were modified), are removed from
 * cache.
 *
 * Just payload size (pixels, with alpha) are accounted in
 * calculation, shallow images (without real data), are accounted as
 * regular images.
 */

#include "evas_soft16.h"
#include <assert.h>

#ifndef DEBUG_CACHE
//#define DEBUG_CACHE
#endif

#ifndef DEBUG_COLOR
#define DEBUG_COLOR
#endif

#ifdef DEBUG_COLOR
#define COLOR_CLEAR    "\033[0m"
#define COLOR_RED      "\033[31m"
#define COLOR_GREEN    "\033[32m"
#define COLOR_YELLOW   "\033[33m"
#define COLOR_BLUE     "\033[34m"
#define COLOR_MAGENTA  "\033[35m"
#define COLOR_CYAN     "\033[36m"
#else
#define COLOR_CLEAR    ""
#define COLOR_RED      ""
#define COLOR_GREEN    ""
#define COLOR_YELLOW   ""
#define COLOR_BLUE     ""
#define COLOR_MAGENTA  ""
#define COLOR_CYAN     ""
#endif

#ifdef DEBUG_CACHE
static inline void
_dbg(const char *file, int line, const char *func, const char *color, const char *fmt, ...)
{
   char msg[4096];
   va_list args;

   va_start(args, fmt);
   vsnprintf(msg, sizeof(msg), fmt, args);
   va_end(args);

   fprintf(stderr, "DBG: %s%s:%d:%s"COLOR_CLEAR"   %s\n", color, file, line, func, msg);
}

#define dbg(fmt, ...)							   \
  _dbg(__FILE__, __LINE__, __FUNCTION__, COLOR_CLEAR, fmt, ##__VA_ARGS__)
#define dbg_color(col, fmt, ...)                                           \
  _dbg(__FILE__, __LINE__, __FUNCTION__, col, fmt, ##__VA_ARGS__)
#else
#define dbg(fmt, ...) do {} while (0)
#define dbg_color(col, fmt, ...) do {} while (0)
#endif


typedef struct _Soft16_Cache Soft16_Cache;
struct _Soft16_Cache {
  Evas_Hash *hash; /**< every known image */
  Evas_List *lru;  /**< unused images, from older to newer */
  int limit;       /**< maximum size in bytes to keep with unused images */
  int used;        /**< current amount in bytes used by unused images */
};
static Soft16_Cache _soft16_cache = {NULL, NULL, 0, 0};

static inline void
soft16_image_cache_destroy_image(Soft16_Image *im)
{
   _soft16_cache.hash = evas_hash_del(_soft16_cache.hash, im->cache_key, im);
   evas_stringshare_del(im->cache_key);
   soft16_image_destroy(im);
}

void
soft16_image_cache_flush(void)
{
   Evas_List *n;

   dbg_color(COLOR_RED, "cache flush");

   for (n = _soft16_cache.lru; n != NULL; n = n->next)
     soft16_image_cache_destroy_image(n->data);

   _soft16_cache.lru = evas_list_free(_soft16_cache.lru);
   _soft16_cache.used = 0;
}

static int
soft16_image_cache_image_size_get(Soft16_Image *im)
{
   return IMG_BYTE_SIZE(im->stride, im->h, im->have_alpha);
}

static void
soft16_image_cache_free(int bytes)
{
   Evas_List *n;

   dbg_color(COLOR_RED, "BEG cache free %d bytes, used %d of %d...",
	     bytes, _soft16_cache.used, _soft16_cache.limit);

   if (bytes >= _soft16_cache.used)
     {
	soft16_image_cache_flush();
	return;
     }

   n = _soft16_cache.lru;
   while (bytes > 0 && n != NULL)
     {
	Soft16_Image *im;
	int size;

	im = n->data;
	size = soft16_image_cache_image_size_get(im);
	bytes -= size;
	_soft16_cache.used -= size;

	soft16_image_cache_destroy_image(im);
	n = _soft16_cache.lru = evas_list_remove_list(_soft16_cache.lru, n);
     }

   assert(_soft16_cache.used >= 0);
   dbg_color(COLOR_RED, "END cache is now %d of %d",
	     _soft16_cache.used, _soft16_cache.limit);
}

int
soft16_image_cache_size_get(void)
{
   return _soft16_cache.limit;
}

void
soft16_image_cache_size_set(int limit)
{
   dbg("old: %d, new: %d, used: %d",
       _soft16_cache.limit, limit, _soft16_cache.used);

   if (limit < _soft16_cache.used && limit >= 0)
     soft16_image_cache_free(_soft16_cache.used - limit);

   _soft16_cache.limit = limit;
}

static void
soft16_image_cache_lru_del(Soft16_Image *im)
{
   Evas_List *n;

   dbg_color(COLOR_YELLOW, "im=%p\n", im);

   for (n = _soft16_cache.lru; n != NULL; n = n->next)
     if (n->data == im)
       break;

   if (!n)
     return;

   _soft16_cache.lru = evas_list_remove_list(_soft16_cache.lru, n);
   _soft16_cache.used -= soft16_image_cache_image_size_get(im);
   dbg_color(COLOR_YELLOW, "   found, remove from cache, stats %d of %d",
	     _soft16_cache.used, _soft16_cache.limit);
}

static void
soft16_image_cache_lru_add(Soft16_Image *im)
{
   _soft16_cache.lru = evas_list_append(_soft16_cache.lru, im);
   _soft16_cache.used += soft16_image_cache_image_size_get(im);
   dbg_color(COLOR_YELLOW, "   add to cache, stats %d of %d",
	     _soft16_cache.used, _soft16_cache.limit);
}

#define STAT_GAP 2

Soft16_Image *
soft16_image_cache_get(const char *cache_key)
{
   Soft16_Image *im;

   dbg_color(COLOR_GREEN, "cache_key=[%s]", cache_key);

   im = evas_hash_find(_soft16_cache.hash, cache_key);
   if (im)
     {
	time_t t;

	t = time(NULL);
	if ((t - im->laststat) > STAT_GAP)
	  {
	     struct stat st;

	     if (stat(im->file, &st) < 0) return NULL;
	     if (st.st_mtime != im->timestamp) return NULL;

	     im->laststat = t;
	  }

	dbg_color(COLOR_GREEN, "  found %p, [%s][%s] %dx%d, refernces=%d",
		  im, im->file, im->key, im->w, im->h, im->references);

	if (im->references == 0)
	  soft16_image_cache_lru_del(im);

	im->references++;
     }
   else
     dbg_color(COLOR_GREEN, "not found!");

   return im;
}

void
soft16_image_cache_put(Soft16_Image *im)
{
   int size;

   assert(im);
   assert(im->cache_key);
   assert(im->references == 0);

   dbg_color(COLOR_CYAN, "BEG im=%p, cache_key=%s", im, im->cache_key);

   size = soft16_image_cache_image_size_get(im);
   dbg("  sizes: im=%d, used=%d, limit=%d", size,
       _soft16_cache.used, _soft16_cache.limit);

   if (size + _soft16_cache.used > _soft16_cache.limit)
     {
	if (size > _soft16_cache.limit)
	  {
	     dbg_color(COLOR_CYAN, "    image doesn't fit in cache, destroy");
	     soft16_image_cache_del(im);
	     soft16_image_destroy(im);
	     return;
	  }
	soft16_image_cache_free(_soft16_cache.used - size);
	dbg("    freed cache space, now %d available",
	    _soft16_cache.limit - _soft16_cache.used);
     }

   soft16_image_cache_lru_add(im);
   dbg_color(COLOR_CYAN, "END cache stats: used=%d, limit=%d",
	     _soft16_cache.used, _soft16_cache.limit);
}

void
soft16_image_cache_add(Soft16_Image *im, const char *cache_key)
{
   assert(im);
   assert(cache_key);
   assert(im->cache_key == NULL);

   dbg_color(COLOR_MAGENTA, "im=%p, cache_key=%s", im, cache_key);

   im->cache_key = evas_stringshare_add(cache_key);
   _soft16_cache.hash = evas_hash_add(_soft16_cache.hash, im->cache_key, im);
}

void
soft16_image_cache_del(Soft16_Image *im)
{
   assert(im);
   assert(im->cache_key != NULL);

   dbg_color(COLOR_MAGENTA, "im=%p, cache_key=%s", im, im->cache_key);

   _soft16_cache.hash = evas_hash_del(_soft16_cache.hash, im->cache_key, im);
   soft16_image_cache_lru_del(im);

   evas_stringshare_del(im->cache_key);
   im->cache_key = NULL;
}
