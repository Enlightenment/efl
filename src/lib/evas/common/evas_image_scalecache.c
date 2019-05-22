#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef _WIN32
# include <windows.h>
#endif

#include <assert.h>

#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_image_private.h"

#define SCALECACHE 1

#define MAX_SCALEITEMS 32
#define MIN_SCALE_USES 3
//#define MIN_SCALE_AGE_GAP 5000
#define MAX_SCALECACHE_DIM 3200
#define FLOP_ADD 4
#define MAX_FLOP_COUNT 16
#define FLOP_DEL 1
#define SCALE_CACHE_SIZE 4 * 1024 * 1024
//#define SCALE_CACHE_SIZE 0

typedef struct _ScaleitemKey ScaleitemKey;
typedef struct _Scaleitem Scaleitem;

struct _ScaleitemKey
{
   int src_x, src_y;
   unsigned int src_w, src_h;
   unsigned int dst_w, dst_h;
   Eina_Bool smooth : 1;
};

struct _Scaleitem
{
   EINA_INLIST;
   unsigned long long usage;
   unsigned long long usage_count;
   RGBA_Image *im, *parent_im;
   Eina_List *item;
   unsigned int flop;
   unsigned int size_adjust;

   ScaleitemKey key;

   Eina_Bool forced_unload : 1;
   Eina_Bool populate_me : 1;
};

#ifdef SCALECACHE
static unsigned long long use_counter = 0;

static SLK(cache_lock);
static Eina_Inlist *cache_list = NULL;
static unsigned int cache_size = 0;
static int init = 0;

static unsigned int max_cache_size = SCALE_CACHE_SIZE;
static unsigned int max_dimension = MAX_SCALECACHE_DIM;
static unsigned int max_flop_count = MAX_FLOP_COUNT;
static unsigned int max_scale_items = MAX_SCALEITEMS;
static unsigned int min_scale_uses = MIN_SCALE_USES;
#endif

static int
_evas_common_scalecache_key_hash(const void *key, int key_length EINA_UNUSED)
{
   const ScaleitemKey *skey;
   unsigned int tohash;
   int r;

   skey = key;

   tohash = (skey->src_x ^ skey->src_y) ^ ((skey->src_w ^ skey->src_h) ^ (skey->dst_w ^ skey->dst_h));
   r = eina_hash_int32(&tohash, sizeof (int));

   return r;
}

static unsigned int
_evas_common_scalecache_key_length(const void *key EINA_UNUSED)
{
   return sizeof (ScaleitemKey);
}

static int
_evas_common_scalecache_key_cmp(const void *key1, int key1_length EINA_UNUSED,
                                const void *key2, int key2_length EINA_UNUSED)
{
   const ScaleitemKey *sk1 = key1;
   const ScaleitemKey *sk2 = key2;

#define CMP_SKEY(Sk1, Sk2, Name)                 \
   if (Sk1->Name != Sk2->Name)                   \
     return Sk1->Name - Sk2->Name;

   CMP_SKEY(sk1, sk2, src_w);
   CMP_SKEY(sk1, sk2, src_h);
   CMP_SKEY(sk1, sk2, dst_w);
   CMP_SKEY(sk1, sk2, dst_h);
   CMP_SKEY(sk1, sk2, src_x);
   CMP_SKEY(sk1, sk2, src_y);
   CMP_SKEY(sk1, sk2, smooth);

   return 0;
}

void
evas_common_scalecache_init(void)
{
#ifdef SCALECACHE
   const char *s;

   init++;
   if (init > 1) return;
   use_counter = 0;
   SLKI(cache_lock);
   s = getenv("EVAS_SCALECACHE_SIZE");
   if (s) max_cache_size = atoi(s) * 1024;
   s = getenv("EVAS_SCALECACHE_MAX_DIMENSION");
   if (s) max_dimension = atoi(s);
   s = getenv("EVAS_SCALECACHE_MAX_FLOP_COUNT");
   if (s) max_flop_count = atoi(s);
   s = getenv("EVAS_SCALECACHE_MAX_ITEMS");
   if (s) max_scale_items = atoi(s);
   s = getenv("EVAS_SCALECACHE_MIN_USES");
   if (s) min_scale_uses = atoi(s);
#endif
}

void
evas_common_scalecache_shutdown(void)
{
#ifdef SCALECACHE
   init--;
   if (init ==0)
      SLKD(cache_lock);
#endif
}

void
evas_common_rgba_image_scalecache_init(Image_Entry *ie)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   // NOTE: this conflicts with evas image cache init and del of lock
   SLKI(im->cache.lock);
#endif
}

void
evas_common_rgba_image_scalecache_shutdown(Image_Entry *ie)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   evas_common_rgba_image_scalecache_dirty(ie);
   // NOTE: this conflicts with evas image cache init and del of lock
   SLKD(im->cache.lock);
#endif
}

void
evas_common_rgba_image_scalecache_dirty(Image_Entry *ie)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;

   SLKL(im->cache.lock);
   while (im->cache.list)
     {
        Scaleitem *sci = im->cache.list->data;

        im->cache.list = eina_list_remove(im->cache.list, sci);
        if ((sci->im) && (sci->im->cache_entry.references == 0))
          {
             SLKL(cache_lock);

             evas_common_rgba_image_free(&sci->im->cache_entry);
             sci->im = NULL;

             if (!sci->forced_unload)
               cache_size -= sci->key.dst_w * sci->key.dst_h * 4;
             else
               cache_size -= sci->size_adjust;
             cache_list = eina_inlist_remove(cache_list, (Eina_Inlist *)sci);

             SLKU(cache_lock);
          }

        if (!sci->im)
          free(sci);
     }
   eina_hash_free(im->cache.hash);
   im->cache.hash = NULL;
   SLKU(im->cache.lock);
#endif
}

void
evas_common_rgba_image_scalecache_orig_use(Image_Entry *ie)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   SLKL(im->cache.lock);
   use_counter++;
   // FIXME: if orig not loaded, reload
   // FIXME: mark orig with current used counter
   im->cache.orig_usage++;
   im->cache.usage_count = use_counter;
   SLKU(im->cache.lock);
#endif
}

int
evas_common_rgba_image_scalecache_usage_get(Image_Entry *ie)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   int size = 0;
   Eina_List *l;
   Scaleitem *sci;
   SLKL(im->cache.lock);
   EINA_LIST_FOREACH(im->cache.list, l, sci)
     {
        if (sci->im) size += sci->key.dst_w * sci->key.dst_h * 4;
     }
   SLKU(im->cache.lock);
   return size;
#else
   return 0;
#endif
}

/* receives original Image_Entry */
void
evas_common_rgba_image_scalecache_items_ref(Image_Entry *ie, Eina_Array *ret)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   Eina_List *l;
   Scaleitem *sci;

   SLKL(im->cache.lock);
   EINA_LIST_FOREACH(im->cache.list, l, sci)
     {
        if (sci->im)
          {
             Image_Entry *scie = (Image_Entry *)sci->im;
             assert(scie->references >= 0);
             scie->references++;
             eina_array_push(ret, scie);
          }
     }
   SLKU(im->cache.lock);
#endif
}

/* receives scaled Image_Entry */
void
evas_common_rgba_image_scalecache_item_unref(Image_Entry *scie)
{
#ifdef SCALECACHE
   scie->references--;
   assert(scie->references >= 0);
#endif
}

#ifdef SCALECACHE
static void
_sci_fix_newest(RGBA_Image *im)
{
   Eina_List *l;
   Scaleitem *sci;

   im->cache.newest_usage = 0;
   im->cache.newest_usage_count = 0;
   EINA_LIST_FOREACH(im->cache.list, l, sci)
     {
        if (sci->usage > im->cache.newest_usage)
          im->cache.newest_usage = sci->usage;
        if (sci->usage_count > im->cache.newest_usage_count)
          im->cache.newest_usage_count = sci->usage_count;
     }
//   INF("_sci_fix_newest! -> %i", im->cache.newest_usage);
}

static Scaleitem *
_sci_find(RGBA_Image *im,
          RGBA_Draw_Context *dc EINA_UNUSED, int smooth,
          int src_x, int src_y,
          unsigned int src_w, unsigned int src_h,
          unsigned int dst_w, unsigned int dst_h)
{
   Eina_List *l;
   Scaleitem *sci;
   ScaleitemKey key;

#define SET_SKEY(S, Name)                \
   S.Name = Name

   if (im->cache.hash)
     {
        SET_SKEY(key, src_w);
        SET_SKEY(key, src_h);
        SET_SKEY(key, dst_w);
        SET_SKEY(key, dst_h);
        SET_SKEY(key, src_x);
        SET_SKEY(key, src_y);
        SET_SKEY(key, smooth);

        sci = eina_hash_find(im->cache.hash, &key);
        if (sci)
          {
             im->cache.list = eina_list_promote_list(im->cache.list, sci->item);
             return sci;
          }
     }

   if (eina_list_count(im->cache.list) > max_scale_items)
     {
        l = eina_list_last(im->cache.list);
        sci = eina_list_data_get(l);

        eina_hash_del(im->cache.hash, &sci->key, sci);
        im->cache.list = eina_list_remove_list(im->cache.list, l);
        if ((sci->usage == im->cache.newest_usage) ||
            (sci->usage_count == im->cache.newest_usage_count))
          _sci_fix_newest(im);

        if (sci->im)
          {
             if (sci->im->cache_entry.references > 0) goto try_alloc;

             evas_common_rgba_image_free(&sci->im->cache_entry);
             if (!sci->forced_unload)
               cache_size -= sci->key.dst_w * sci->key.dst_h * 4;
             else
               cache_size -= sci->size_adjust;
//             INF(" 1- %i", sci->dst_w * sci->dst_h * 4);
             cache_list = eina_inlist_remove(cache_list, (Eina_Inlist *)sci);
          }
        if (max_scale_items < 1) return NULL;
     }
   else
     {
try_alloc:
        if (max_scale_items < 1) return NULL;
        if (eina_list_count(im->cache.list) > (max_scale_items - 1))
          return NULL;

        sci = calloc(1, sizeof(Scaleitem));
        sci->parent_im = im;
     }
   sci->usage = 0;
   sci->usage_count = 0;
   sci->populate_me = 0;
   sci->key.smooth = smooth;
   sci->forced_unload = 0;
   sci->flop = 0;
   sci->im = NULL;
   sci->key.src_x = src_x;
   sci->key.src_y = src_y;
   sci->key.src_w = src_w;
   sci->key.src_h = src_h;
   sci->key.dst_w = dst_w;
   sci->key.dst_h = dst_h;
   if (!im->cache.hash)
     im->cache.hash = eina_hash_new(_evas_common_scalecache_key_length,
                                    _evas_common_scalecache_key_cmp,
                                    _evas_common_scalecache_key_hash,
                                    NULL,
                                    3);
   eina_hash_direct_add(im->cache.hash, &sci->key, sci);
   im->cache.list = eina_list_prepend(im->cache.list, sci);
   sci->item = im->cache.list;
   return sci;
}

static void
_cache_prune(Scaleitem *notsci, Eina_Bool copies_only)
{
   Eina_Inlist *next;

   if (!cache_list) return;

   next = cache_list;
   while ((next) && (cache_size > max_cache_size))
     {
        Scaleitem *sci;
        Image_Entry *scie;

        sci = EINA_INLIST_CONTAINER_GET(next, Scaleitem);

        if (copies_only)
          {
             while (!sci->parent_im->image.data)
               {
                  next = EINA_INLIST_GET(sci)->next;
                  if (!next) return;
                  sci = EINA_INLIST_CONTAINER_GET(next, Scaleitem);
               }
          }

        scie = (Image_Entry *)sci->im;
        next = EINA_INLIST_GET(sci)->next;

        if (sci == notsci) continue;
        if ((!scie) || (scie->references > 0)) continue;

        evas_common_rgba_image_free(scie);
        sci->im = NULL;
        sci->usage = 0;
        sci->usage_count = 0;
        sci->flop += FLOP_ADD;

        if (!sci->forced_unload)
          cache_size -= sci->key.dst_w * sci->key.dst_h * 4;
        else
          cache_size -= sci->size_adjust;

        cache_list = eina_inlist_remove(cache_list, EINA_INLIST_GET(sci));
        memset(sci, 0, sizeof(Eina_Inlist));
     }
}
#endif

EAPI void
evas_common_rgba_image_scalecache_size_set(unsigned int size)
{
#ifdef SCALECACHE
   SLKL(cache_lock);
   if (size != max_cache_size)
     {
        max_cache_size = size;
        _cache_prune(NULL, 1);
     }
   SLKU(cache_lock);
#endif   
}

EAPI unsigned int
evas_common_rgba_image_scalecache_size_get(void)
{
#ifdef SCALECACHE
   int t;
   SLKL(cache_lock);
   t = max_cache_size;
   SLKU(cache_lock);
   return t;
#else
   return 0;
#endif   
}

EAPI void
evas_common_rgba_image_scalecache_prune(void)
{
#ifdef SCALECACHE
   SLKL(cache_lock);
   _cache_prune(NULL, 0);
   SLKU(cache_lock);
#endif
}

EAPI void
evas_common_rgba_image_scalecache_dump(void)
{
#ifdef SCALECACHE
   int t;
   SLKL(cache_lock);
   t = max_cache_size;
   max_cache_size = 0;
   _cache_prune(NULL, 0);
   max_cache_size = t;
   SLKU(cache_lock);
#endif   
}

EAPI void
evas_common_rgba_image_scalecache_flush(void)
{
#ifdef SCALECACHE
   int t;
   SLKL(cache_lock);
   t = max_cache_size;
   max_cache_size = 0;
   _cache_prune(NULL, 1);
   max_cache_size = t;
   SLKU(cache_lock);
#endif   
}

EAPI Eina_Bool
evas_common_rgba_image_scalecache_prepare(Image_Entry *ie, RGBA_Image *dst EINA_UNUSED,
                                          RGBA_Draw_Context *dc, int smooth,
                                          int src_region_x, int src_region_y,
                                          int src_region_w, int src_region_h,
                                          int dst_region_x EINA_UNUSED, int dst_region_y EINA_UNUSED,
                                          int dst_region_w, int dst_region_h)
{
#ifdef SCALECACHE
   int locked = 0;
   Eina_Lock_Result ret;
   RGBA_Image *im = (RGBA_Image *)ie;
   Scaleitem *sci;

   if ((dst_region_w == 0) || (dst_region_h == 0) ||
       (src_region_w == 0) || (src_region_h == 0)) return EINA_TRUE;
   // was having major lock issues here - SLKL was deadlocking. what was
   // going on? it may have been an eina treads badness but this will stay here
   // for now for debug
#if 1
   ret = SLKT(im->cache.lock);
   if (ret == EINA_FALSE) /* can't get image lock */
     {
        useconds_t slp = 1, slpt = 0;
        
        while (slpt < 500000)
          {
#ifdef _WIN32
             Sleep(slp / 1000);
#else
             usleep(slp);
#endif
             slpt += slp;
             slp++;
             ret = SLKT(im->cache.lock);
             if (ret == EINA_LOCK_DEADLOCK)
               {
                  printf("WARNING: DEADLOCK on image %p (%s)\n", im, ie->file);
               }
             else
               {
                  locked = 1;
                  break;
               }
          }
        if (ret == EINA_FALSE)
          {
             printf("WARNING: lock still there after %i usec\n", slpt);
             printf("WARNING: stucklock on image %p (%s)\n", im, ie->file);
             /* SLKDBG(im->cache.lock); */
          }
     }
   else if (ret == EINA_LOCK_DEADLOCK)
     {
        printf("WARNING: DEADLOCK on image %p (%s)\n", im, ie->file);
     }
   else locked = 1;
#endif   
   if (!locked) { SLKL(im->cache.lock); locked = 1; }
   use_counter++;
   if ((src_region_w == dst_region_w) && (src_region_h == dst_region_h))
     {
        // 1:1 scale.
        im->cache.orig_usage++;
        im->cache.usage_count = use_counter;
        if (locked) SLKU(im->cache.lock);
        return EINA_FALSE;
     }
   if ((!im->cache_entry.flags.alpha) && (!smooth))
     {
        // solid nearest scaling - it's actually the same speed cached or not,
        // or in some cases faster not cached
        im->cache.orig_usage++;
        im->cache.usage_count = use_counter;
        if (locked) SLKU(im->cache.lock);
        return EINA_FALSE;
     }
   SLKL(cache_lock);
   sci = _sci_find(im, dc, smooth, 
                   src_region_x, src_region_y, src_region_w, src_region_h, 
                   dst_region_w, dst_region_h);
   if (!sci)
     {
        SLKU(cache_lock);
        if (locked) SLKU(im->cache.lock);
        return EINA_FALSE;
     }
//   INF("%10i | %4i %4i %4ix%4i -> %4i %4i %4ix%4i | %i",
//          (int)use_counter,
//          src_region_x, src_region_y, src_region_w, src_region_h,
//          dst_region_x, dst_region_y, dst_region_w, dst_region_h,
//          smooth);
   if ((sci->usage >= min_scale_uses)
       && (ie->scale_hint != EVAS_IMAGE_SCALE_HINT_DYNAMIC)
//       && (sci->usage_count > (use_counter - MIN_SCALE_AGE_GAP))
       )
     {
        if (!sci->im)
          {
             if ((sci->key.dst_w < max_dimension) && 
                 (sci->key.dst_h < max_dimension))
               {
                  if (sci->flop <= max_flop_count)
                    {
                       sci->populate_me = 1;
                       im->cache.populate_count++;
                    }
               }
          }
     }
   sci->usage++;
   sci->usage_count = use_counter;
   SLKU(cache_lock);
   if (sci->usage > im->cache.newest_usage) 
     im->cache.newest_usage = sci->usage;
//   INF("newset? %p %i > %i", im, 
//          (int)sci->usage, 
//          (int)im->cache.newest_usage);
   if (sci->usage_count > im->cache.newest_usage_count) 
     im->cache.newest_usage_count = sci->usage_count;
//   INF("  -------------- used %8i#, %8i@", (int)sci->usage, (int)sci->usage_count);
   if (locked) SLKU(im->cache.lock);
#endif
   if ((!im->image.data) && (sci->populate_me)) return EINA_FALSE;
   return EINA_TRUE;
}

#ifdef SCALECACHE
//static int pops = 0;
//static int hits = 0;
//static int misses = 0;
//static int noscales = 0;
#endif

EAPI Eina_Bool
evas_common_rgba_image_scalecache_do_cbs(Image_Entry *ie, RGBA_Image *dst,
                                         RGBA_Draw_Context *dc, int smooth,
                                         int src_region_x, int src_region_y,
                                         int src_region_w, int src_region_h,
                                         int dst_region_x, int dst_region_y,
                                         int dst_region_w, int dst_region_h,
                                         Evas_Common_Scale_In_To_Out_Clip_Cb cb_sample,
                                         Evas_Common_Scale_In_To_Out_Clip_Cb cb_smooth)
{
#ifdef SCALECACHE
   RGBA_Image *im = (RGBA_Image *)ie;
   Scaleitem *sci;
   int didpop = 0;
   int dounload = 0;
   Eina_Bool ret = EINA_FALSE;
/*
   static int i = 0;

   i++;
   if (i > 2000)
     {
        INF("p: %6i, h: %6i, m: %6i, n: %6i",
               pops, hits, misses, noscales);
        i = 0;
     }
 */
   if ((dst_region_w == 0) || (dst_region_h == 0) ||
       (src_region_w == 0) || (src_region_h == 0)) return EINA_FALSE;
   if ((src_region_w == dst_region_w) && (src_region_h == dst_region_h))
     {
        if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
          evas_cache_image_load_data(&im->cache_entry);
	evas_common_image_colorspace_normalize(im);

//        noscales++;
        if (im->image.data)
          {
             return cb_sample(im, dst, dc,
                              src_region_x, src_region_y,
                              src_region_w, src_region_h,
                              dst_region_x, dst_region_y,
                              dst_region_w, dst_region_h);
          }
        return EINA_FALSE;
     }
   SLKL(cache_lock);
   sci = _sci_find(im, dc, smooth,
                   src_region_x, src_region_y, src_region_w, src_region_h,
                   dst_region_w, dst_region_h);
   SLKU(cache_lock);
   if (!sci)
     {
        if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
          evas_cache_image_load_data(&im->cache_entry);
	evas_common_image_colorspace_normalize(im);

//        misses++;
        if (im->image.data)
          {
             if (smooth)
               return cb_smooth(im, dst, dc,
                                src_region_x, src_region_y,
                                src_region_w, src_region_h,
                                dst_region_x, dst_region_y,
                                dst_region_w, dst_region_h);
             else
               return cb_sample(im, dst, dc,
                                src_region_x, src_region_y,
                                src_region_w, src_region_h,
                                dst_region_x, dst_region_y,
                                dst_region_w, dst_region_h);
          }
        return EINA_FALSE;
     }
   SLKL(im->cache.lock);
   if (sci->populate_me)
     {
        int size, osize, used;

        size = dst_region_w * dst_region_h;
        if (((((dst_region_w > 640) || (dst_region_h > 640)) &&
             (size > (480 * 480))) ||
             (ie->scale_hint == EVAS_IMAGE_SCALE_HINT_STATIC)) &&
            (ie->scale_hint != EVAS_IMAGE_SCALE_HINT_DYNAMIC))
          {
             Eina_List *l;
             Scaleitem *sci2;

             dounload = 1;
             osize = sci->parent_im->cache_entry.w * sci->parent_im->cache_entry.h;
             used = 0;
             EINA_LIST_FOREACH(im->cache.list, l, sci2)
               {
                  if (sci2->im) used += sci2->key.dst_w * sci2->key.dst_h;
               }
             if ((size < osize) && (used == 0))
               sci->size_adjust = 0;
             else
               {
                  osize -= used;
                  if (osize < 0) osize = 0;
                  size -= osize;
                  sci->size_adjust = size * 4;
               }
          }
        else
          {
             size *= sizeof(DATA32);
             if ((cache_size + size) > max_cache_size)
               {
                  sci->populate_me = 0;
                  im->cache.populate_count--;
               }
          }
     }

   if (sci->populate_me)
     {
//        INF("##! populate!");
        sci->im = evas_common_image_new
          (dst_region_w, dst_region_h, im->cache_entry.flags.alpha);
        if (sci->im)
          {
             static RGBA_Draw_Context *ct = NULL;

             SLKL(cache_lock);
             im->cache.orig_usage++;
             im->cache.usage_count = use_counter;
             im->cache.populate_count--;
//             pops++;
             if (!ct)
               {
                  // FIXME: static ct - never can free on shutdown? not a leak
                  // or real harm - just annoying valgrind bitch
                  ct = evas_common_draw_context_new();
                  evas_common_draw_context_set_render_op(ct, _EVAS_RENDER_COPY);
               }
             SLKU(cache_lock);
             SLKU(im->cache.lock);

             SLKL(cache_lock);
             if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
               evas_cache_image_load_data(&im->cache_entry);
             SLKU(cache_lock);

             SLKL(im->cache.lock);
             SLKL(cache_lock);
             evas_common_image_colorspace_normalize(im);
             if (im->image.data)
               {
                  if (smooth)
                    ret = cb_smooth(im, sci->im, ct,
                                    src_region_x, src_region_y,
                                    src_region_w, src_region_h,
                                    0, 0,
                                    dst_region_w, dst_region_h);
                  else
                    ret = cb_sample(im, sci->im, ct,
                                    src_region_x, src_region_y,
                                    src_region_w, src_region_h,
                                    0, 0,
                                    dst_region_w, dst_region_h);
                  sci->populate_me = 0;
#if 0 // visual debug of cached images
                    {
                       int xx, yy;
                       DATA32 *pp;

                       pp = sci->im->image.data;
                       for (yy = 0; yy < dst_region_h; yy++)
                         {

                            for (xx = 0; xx < dst_region_w; xx++)
                              {
                                 if (yy & 0x1)
                                   {
                                      if (xx & 0x1) *pp = 0x882288ff;
                                   }
                                 else
                                   {
                                      if (!(xx & 0x1)) *pp = 0x882288ff;
                                   }
                                 pp++;
                              }
                         }
                    }
#endif
               }
             if (dounload)
               {
                  sci->forced_unload = 1;
                  cache_size += sci->size_adjust;
               }
             else
               {
                  cache_size += sci->key.dst_w * sci->key.dst_h * 4;
               }
//             INF(" + %i @ flop: %i (%ix%i)",
//                    sci->dst_w * sci->dst_h * 4, sci->flop,
//                    sci->dst_w, sci->dst_h);
             cache_list = eina_inlist_append(cache_list, (Eina_Inlist *)sci);
             SLKU(cache_lock);
             didpop = 1;
          }
     }
   if (sci->im && !ie->animated.animated)
     {
        if (!didpop)
          {
	     SLKL(cache_lock);
             cache_list = eina_inlist_remove(cache_list, (Eina_Inlist *)sci);
             cache_list = eina_inlist_append(cache_list, (Eina_Inlist *)sci);
	     SLKU(cache_lock);
          }
        else
          {
             if (sci->flop >= FLOP_DEL) sci->flop -= FLOP_DEL;
          }
//        INF("use cached!");
        SLKU(im->cache.lock);
        ret |= cb_sample(sci->im, dst, dc,
                         0, 0,
                         dst_region_w, dst_region_h,
                         dst_region_x, dst_region_y,
                         dst_region_w, dst_region_h);
//        hits++;
//        INF("check %p %i < %i",
//               im,
//               (int)im->cache.orig_usage,
//               (int)im->cache.newest_usage);
        /* while framequeuing is applied,
         * original image data is loaded by the main thread
         * just before enqueuing the rendering op into the pipe.
         * so unloading the original image data here
         * causes only speed-down side-effect and no memory usage gain;
         * it will be loaded again for the very next rendering for this image.
         */
        if (ie->scale_hint != EVAS_IMAGE_SCALE_HINT_DYNAMIC)
          {
             if ((dounload) ||
                 ((im->cache_entry.flags.loaded) &&
                     ((!im->cs.no_free))  &&
                     (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)))
               {
                  if ((dounload) || (im->cache.orig_usage <
                                     (im->cache.newest_usage / 20)))
                    {
                       //FIXME: imagedataunload - inform owners
                       /* FIXME: must be the _cache version, no? */
                       evas_common_rgba_image_unload(&im->cache_entry);
                    }
               }
          }
     }
   else
     {
        SLKU(im->cache.lock);
        if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
          evas_cache_image_load_data(&im->cache_entry);
	evas_common_image_colorspace_normalize(im);
//        misses++;
        if (im->image.data)
          {
             if (smooth)
               ret |= cb_smooth(im, dst, dc,
                                src_region_x, src_region_y,
                                src_region_w, src_region_h,
                                dst_region_x, dst_region_y,
                                dst_region_w, dst_region_h);
             else
               ret |= cb_sample(im, dst, dc,
                               src_region_x, src_region_y,
                               src_region_w, src_region_h,
                               dst_region_x, dst_region_y,
                               dst_region_w, dst_region_h);
          }
     }

   return ret;

#else

   RGBA_Image *im = (RGBA_Image *)ie;

   if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
     evas_cache_image_load_data(&im->cache_entry);
   evas_common_image_colorspace_normalize(im);
   if (im->image.data)
     {
        if (smooth)
          return cb_smooth(im, dst, dc,
                           src_region_x, src_region_y,
                           src_region_w, src_region_h,
                           dst_region_x, dst_region_y,
                           dst_region_w, dst_region_h);
        else
          return cb_sample(im, dst, dc,
                           src_region_x, src_region_y,
                           src_region_w, src_region_h,
                           dst_region_x, dst_region_y,
                           dst_region_w, dst_region_h);
     }

   return EINA_FALSE;
#endif
}


EAPI void
evas_common_rgba_image_scalecache_do(Image_Entry *ie, RGBA_Image *dst,
                                     RGBA_Draw_Context *dc, int smooth,
                                     int src_region_x, int src_region_y,
                                     int src_region_w, int src_region_h,
                                     int dst_region_x, int dst_region_y,
                                     int dst_region_w, int dst_region_h)
{
   evas_common_rgba_image_scalecache_do_cbs(
     ie, dst, dc, smooth,
     src_region_x, src_region_y, src_region_w, src_region_h,
     dst_region_x, dst_region_y, dst_region_w, dst_region_h,
     evas_common_scale_rgba_in_to_out_clip_sample,
     evas_common_scale_rgba_in_to_out_clip_smooth);
   evas_common_rgba_image_scalecache_prune();
}
