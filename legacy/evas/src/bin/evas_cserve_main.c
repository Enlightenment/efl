#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <signal.h>
#include <sys/time.h>
#include <time.h>
#ifdef _WIN32
# include <windows.h>
#endif

#ifdef BUILD_PTHREAD
#include <pthread.h>
#endif

#include "Evas.h"
#include "evas_cs.h"

#define D(...) EINA_LOG_DOM_DBG(_evas_cserve_bin_log_dom, __VA_ARGS__)
#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_cserve_bin_log_dom, __VA_ARGS__)
#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_cserve_bin_log_dom, __VA_ARGS__)
#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_cserve_bin_log_dom, __VA_ARGS__)
  
#ifdef CSERVE_BIN_DEFAULT_COLOR
#undef CSERVE_BIN_DEFAULT_COLOR
#endif
#define CSERVE_BIN_DEFAULT_COLOR EINA_COLOR_BLUE
// fixme:'s
// 
// preload - make it work (both)

// 
// pants!

typedef struct _Img Img;
typedef struct _Lopt Lopt;
typedef struct _Load_Inf Load_Inf;

struct _Lopt
{
   int    scale_down_by; // if > 1 then use this
   double dpi; // if > 0.0 use this
   int    w, h; // if > 0 use this
   struct {
      int x, y, w, h;
   } region;
};

struct _Img
{
   Image_Entry ie;
   int ref;
   int dref;
   int usage;
   Mem *mem;
   const char *key;
   time_t cached;
   struct {
      const char *file;
      const char *key;
      time_t modtime;
      time_t last_stat;
   } file;
   struct {
      int load1saved, load2saved;
      double load1, load2;
   } stats;
   Lopt load_opts;
   struct {
      int w, h;
      void *data;
      Eina_Bool alpha : 1;
   } image;
   int incache;
   LK(lock);
   Eina_Bool dead : 1;
   Eina_Bool active : 1;
   Eina_Bool useless : 1;
   Eina_Bool killme : 1;
};

struct _Load_Inf
{
   Img *img;
   Client *c;
};

// config
static int stat_res_interval = 2;

static time_t t_now = 0;

static int server_id = 0;

LK(strshr_freeme_lock);
static int strshr_freeme_count = 0;
static int strshr_freeme_alloc = 0;
static const char **strshr_freeme = NULL;

static int cache_cleanme = 0;
static Evas_Cache_Image *cache = NULL;

static Eina_Hash *active_images = NULL;
LK(cache_lock);
static Eina_List *cache_images = NULL;
static int cache_usage = 0;
static int cache_max_usage = 1 * 1024;
static int cache_max_adjust = 0;
static int cache_item_timeout = -1;
static int cache_item_timeout_check = -1;
static Mem *stat_mem = NULL;
static int _evas_cserve_bin_log_dom = -1;
static Eina_List *stat_mems = NULL;

static void cache_clean(void);

#ifndef _WIN32
static double
get_time(void)
{
   struct timeval      timev;
   
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}
#else
static double
get_time(void)
{
   return (double)GetTickCount()/1000.0;
}
#endif

static int mem_total = 0;
static int mem_free = 0;
static int mem_buffers = 0;
static int mem_cached = 0;

static void
meminfo_check(void)
{
   FILE *f;
   char buf[1024];
   int v;
   
   f = fopen("/proc/meminfo", "r");
   if (!f) return;
   if (!fgets(buf, sizeof(buf), f)) goto done;
   v = 0; if (sscanf(buf, "%*s %i %*s", &v) != 1) goto done;
   mem_total = v;
   if (!fgets(buf, sizeof(buf), f)) goto done;
   v = 0; if (sscanf(buf, "%*s %i %*s", &v) != 1) goto done;
   mem_free = v;
   if (!fgets(buf, sizeof(buf), f)) goto done;
   v = 0; if (sscanf(buf, "%*s %i %*s", &v) != 1) goto done;
   mem_buffers = v;
   if (!fgets(buf, sizeof(buf), f)) goto done;
   v = 0; if (sscanf(buf, "%*s %i %*s", &v) != 1) goto done;
   mem_cached = v;
   done:
   fclose(f);
}

static int stats_dirty = 0;
static int saved_loads = 0;
static double saved_load_time = 0;
static double saved_load_lifetime = 0;

static int saved_loaddatas = 0;
static double saved_loaddata_time = 0;
static double saved_loaddata_lifetime = 0;

static int saved_memory = 0;
static int saved_memory_peak = 0;
static int alloced_memory = 0;
static int alloced_memory_peak = 0;
static int real_memory = 0;
static int real_memory_peak = 0;

static Eina_Bool
stats_hash_image_cb(const Eina_Hash *hash __UNUSED__, 
                   const void *key __UNUSED__,
                   void *data, void *fdata __UNUSED__)
{
   Img *img = data;
   

   saved_load_time += img->stats.load1 * img->stats.load1saved;
   saved_loaddata_time += img->stats.load2 * img->stats.load2saved;
   if (img->ref > 1)
     saved_memory += img->image.w * img->image.h * sizeof(DATA32) * (img->ref - 1);
   if (img->mem)
     {
        alloced_memory += img->image.w * img->image.h * sizeof(DATA32);
        real_memory += (((img->image.w * img->image.h * sizeof(DATA32)) + 4095) >> 12) << 12;
     }
   return 1;
}

static void
stats_calc(void)
{
   Img *img;
   Eina_List *l;
   
   if (!stats_dirty) return;
   stats_dirty = 0;
   saved_loads = 0;
   saved_load_time = 0;
   saved_loaddatas = 0;
   saved_loaddata_time = 0;
   saved_memory = 0;
   alloced_memory = 0;
   real_memory = 0;
   
   if (active_images)
     eina_hash_foreach(active_images, stats_hash_image_cb, NULL);
   LKL(cache_lock);
   EINA_LIST_FOREACH(cache_images, l, img)
     {
        saved_loads += img->stats.load1saved;
        saved_load_time += img->stats.load1 * img->stats.load1saved;
        saved_loaddatas += img->stats.load2saved;
        saved_loaddata_time += img->stats.load2 * img->stats.load2saved;
        if (img->mem)
          {
             alloced_memory += img->image.w * img->image.h * sizeof(DATA32);
             real_memory += (((img->image.w * img->image.h * sizeof(DATA32)) + 4095) >> 12) << 12;
          }
     }
   LKU(cache_lock);
   if (saved_memory > saved_memory_peak)
     saved_memory_peak = saved_memory;
   if (real_memory > real_memory_peak)
     real_memory_peak = real_memory;
   if (alloced_memory > alloced_memory_peak)
     alloced_memory_peak = alloced_memory;
}

static void
stats_update(void)
{
   stats_dirty = 1;
}

static void
stats_lifetime_update(Img *img)
{
   saved_load_lifetime += img->stats.load1 * img->stats.load1saved;
   saved_loaddata_lifetime += img->stats.load2 * img->stats.load2saved;
}

static void
stat_clean(Mem *m)
{
   int *ints;
   int size, pid, *ids, count, i;
   
   ints = (int *)m->data;
   size = ints[0];
   if (!evas_cserve_mem_resize(m, size)) return;
   ints = (int *)m->data;
   pid =  ints[1];
   count =  (size - (2 * sizeof(int))) / sizeof(int);
   ids = ints + 2;
   for (i = 0; i < count; i++)
     evas_cserve_mem_del(pid, ids[i]);
}

static int
stat_init(Mem *m)
{
   int *ints;

   ints = (int *)m->data;
   
   if (!evas_cserve_mem_resize(m, 2 * sizeof(int))) return 0;
   ints[0] = 2 * sizeof(int);
   ints[1] = getpid();
   msync(m->data, 2 * sizeof(int), MS_SYNC | MS_INVALIDATE);
   return 1;
}

static int
stat_update(Mem *m)
{
   Eina_List *l;
   Mem *m2;
   int *ints, *ids, i;
   
   ints = (int *)m->data;
   ints[0] = (2 * sizeof(int)) + (eina_list_count(stat_mems) * sizeof(int));
   if (!evas_cserve_mem_resize(m, ints[0])) return 0;
   ints = (int *)m->data;
   ids = ints + 2;
   i = 0;
   EINA_LIST_FOREACH(stat_mems, l, m2)
     {
        ids[i] = m2->id;
        i++;
     }
   msync(m->data, ints[0], MS_SYNC | MS_INVALIDATE);
   return 1;
}

static Image_Entry *
_img_alloc(void)
{
   Img *img;
   
   img = calloc(1, sizeof(Img));
   LKI(img->lock);
   return (Image_Entry *)img;
}

static void
_img_dealloc(Image_Entry *ie)
{
   Img *img = (Img *)ie;
   LKD(img->lock);
   free(img);
}

static int
_img_surface_alloc(Image_Entry *ie, int w, int h)
{
   Img *img = (Img *)ie;

   img->mem = evas_cserve_mem_new(w * h * sizeof(DATA32), NULL);
   if (!img->mem) return -1;
   img->image.data = img->mem->data + img->mem->offset;
   
   stat_mems = eina_list_append(stat_mems, img->mem);
   stat_update(stat_mem);
   return 0;
}

static void
_img_surface_delete(Image_Entry *ie)
{
   Img *img = (Img *)ie;

   if (!img->mem) return;
   
   stat_mems = eina_list_remove(stat_mems, img->mem);
   stat_update(stat_mem);
   
   evas_cserve_mem_free(img->mem);
   img->mem = NULL;
   img->image.data = NULL;
}

static DATA32 *
_img_surface_pixels(Image_Entry *ie)
{
   Img *img = (Img *)ie;
   
   return img->image.data;
}

static int
_img_load(Image_Entry *ie)
{
   return evas_common_load_rgba_image_module_from_file(ie);
}

static void
_img_unload(Image_Entry *ie)
{
}

static void
_img_dirty_region(Image_Entry *ie, int x, int y, int w, int h)
{
}

static int
_img_dirty(Image_Entry *dst, const Image_Entry *src)
{
   return 0;
}

static int
_img_size_set(Image_Entry *dst, const Image_Entry *src, int w, int h)
{
   return 0;
}

static int
_img_copied_data(Image_Entry *ie, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   return 0;
}

static int
_img_data(Image_Entry *ie, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   return 0;
}

static int
_img_color_space(Image_Entry *ie, int cspace)
{
   return 0;
}

static int
_img_load_data(Image_Entry *ie)
{
   return evas_common_load_rgba_image_data_from_file(ie);
}

static int
_img_mem_size_get(Image_Entry *ie)
{
   return 1;
}

static void
img_init(void)
{
   const Evas_Cache_Image_Func cache_funcs = 
     {
        _img_alloc,//Image_Entry *(*alloc)(void);
          _img_dealloc,//void         (*dealloc)(Image_Entry *im);
        /* The cache provide some helpers for surface manipulation. */
          _img_surface_alloc,//int          (*surface_alloc)(Image_Entry *im, int w, int h);
          _img_surface_delete,//void         (*surface_delete)(Image_Entry *im);
          _img_surface_pixels,//DATA32      *(*surface_pixels)(Image_Entry *im);
        /* The cache is doing the allocation and deallocation, you must just do the rest. */
          _img_load,//int          (*constructor)(Image_Entry *im);
          _img_unload,//void         (*destructor)(Image_Entry *im);
          _img_dirty_region,//void         (*dirty_region)(Image_Entry *im, int x, int y, int w, int h);
        /* Only called when references > 0. Need to provide a fresh copie of im. */
        /* The destination surface does have a surface, but no allocated pixel data. */
          _img_dirty,//int          (*dirty)(Image_Entry *dst, const Image_Entry *src);
        /* Only called when references == 1. We will call drop on im'. */
        /* The destination surface does not have any surface. */
          _img_size_set,//int          (*size_set)(Image_Entry *dst, const Image_Entry *src, int w, int h);
        /* The destination surface does not have any surface. */
          _img_copied_data,//int          (*copied_data)(Image_Entry *dst, int w, int h, DATA32 *image_data, int alpha, int cspace);
        /* The destination surface does not have any surface. */
          _img_data,//int          (*data)(Image_Entry *dst, int w, int h, DATA32 *image_data, int alpha, int cspace);
          _img_color_space,//int          (*color_space)(Image_Entry *dst, int cspace);
        /* This function need to update im->w and im->h. */
          _img_load_data,//int          (*load)(Image_Entry *im);
          _img_mem_size_get,//int          (*mem_size_get)(Image_Entry *im);
          NULL,//void         (*debug)(const char *context, Image_Entry *im);
     };
   
   active_images = eina_hash_string_superfast_new(NULL);
   cache = evas_cache_image_init(&cache_funcs);
   LKI(cache_lock);
   LKI(strshr_freeme_lock);
}

static void
img_shutdown(void)
{
   evas_cache_image_shutdown(cache);
   cache = NULL;
   // FIXME: shutdown properly
   LKD(strshr_freeme_lock);
   LKI(cache_lock);
}

static Img *
img_new(const char *file, const char *key, RGBA_Image_Loadopts *load_opts, const char *bufkey)
{
   Img *img;
   struct stat st;
   int ret;
   Image_Entry *ie;
   int err = 0;
   double t;
   
   DBG("... stat %s", file);
   ret = stat(file, &st);
   if (ret < 0) return NULL;
   DBG("... load header");
   t = get_time();
   ie = evas_cache_image_request(cache, file, key, load_opts, &err);
   t = get_time() - t;
   DBG("... header done");
   if (!ie) return NULL;
   DBG("... ie->cache = %p", ie->cache);
   img = (Img *)ie;
   img->stats.load1 = t;
   img->key = eina_stringshare_add(bufkey);
   img->file.modtime = st.st_mtime;
   img->file.last_stat = t_now;
   img->file.file = eina_stringshare_add(file);
   if (key) img->file.key = eina_stringshare_add(key);
   img->load_opts.scale_down_by = load_opts->scale_down_by;
   img->load_opts.dpi = load_opts->dpi;
   img->load_opts.w = load_opts->w;
   img->load_opts.h = load_opts->h;
   img->image.w = ie->w;
   img->image.h = ie->h;
   img->image.alpha = ie->flags.alpha;
   img->ref = 1;
   img->active = 1;
   img->usage = sizeof(Img) + strlen(img->key) + 1 + 
     strlen(img->file.file) + 1;
   if (img->file.key) img->usage += strlen(img->file.key) + 1;
   eina_hash_direct_add(active_images, img->key, img);
   return img;
}

static void
img_loaddata(Img *img)
{
   double t;
   
   if (img->mem) return;
   t = get_time();
   LKL(cache_lock);
   evas_cache_image_load_data((Image_Entry *)img);
   t = get_time() - t;
   img->stats.load2 = t;
   if (img->image.data)
     msync(img->image.data, img->image.w * img->image.h * sizeof(DATA32), MS_SYNC | MS_INVALIDATE);
   if (!img->active) cache_usage -= img->usage;
   img->usage += 
     (4096 * (((img->image.w * img->image.h * sizeof(DATA32)) + 4095) / 4096)) +
     sizeof(Mem);
   if (!img->active) cache_usage += img->usage;
   LKU(cache_lock);
   cache_clean();
}

static void
img_free(Img *img)
{
   if (img->incache > 0)
     {
        ERR("EEEEEEEEEEEEEEEEK!");
        ERR("EEEEEEEEEEEEEEEEK! %p '%s' still in cache",
               img, img->file.file);
        ERR("EEEEEEEEEEEEEEEEK!");
        return;
     }
   stats_lifetime_update(img);
   stats_update();
   
   LKL(strshr_freeme_lock);
   strshr_freeme_count +=  3;
   if (strshr_freeme_count > strshr_freeme_alloc)
     {
        strshr_freeme_alloc += 32;
        strshr_freeme = realloc(strshr_freeme, strshr_freeme_alloc * sizeof(const char **));
     }
   strshr_freeme[strshr_freeme_count - 3] = img->key;
   strshr_freeme[strshr_freeme_count - 2] = img->file.file;
   strshr_freeme[strshr_freeme_count - 1] = img->file.key;
   LKU(strshr_freeme_lock);
   
   evas_cache_image_drop((Image_Entry *)img);
}

static void
cache_clean(void)
{
   DBG("... cache clean!!! do");
   LKL(cache_lock);
   while ((cache_usage > ((cache_max_usage + cache_max_adjust) * 1024)) && 
          (cache_images))
     {
        Img *img;
        Eina_List *l;

        DBG("... clean loop %i > %i", cache_usage, (cache_max_usage + cache_max_adjust) * 1024);
        l = eina_list_last(cache_images); // THREAD: called from thread. happens to be safe as it uses no unlocked shared resources
        if (!l) break;
        img = l->data;
        if (!img) break;
        LKL(img->lock);
        DBG("...   REMOVE %p '%s'", img, img->file.file);
#ifdef BUILD_PTHREAD
        img->killme = 1;
        img->useless = 1;
        img->dead = 1;
        cache_cleanme++;
        LKU(img->lock);
#else        
        cache_images = eina_list_remove_list(cache_images, l); // FIXME: called from thread
        img->incache--;
        cache_usage -= img->usage;
        DBG("...   IMG FREE %p", img);
        img_free(img);
#endif        
     }
   LKU(cache_lock);
}

static void
cache_timeout(time_t t)
{
   Eina_List *l, *l_next;
   Img *img;
   
   if (cache_item_timeout < 0) return;
   LKL(cache_lock);
   EINA_LIST_FOREACH_SAFE(cache_images, l, l_next, img)
     {
        LKL(img->lock);
        if ((t - img->cached) > cache_item_timeout)
          {
             cache_images = eina_list_remove_list(cache_images, l);
             img->incache--;
             cache_usage -= img->usage;
             img_free(img);
          }
        else
          LKU(img->lock);
     }
   LKU(cache_lock);
}

static void
mem_cache_adjust(void)
{
   int pval = cache_max_adjust;
   int max = 0;
   int mem_used;

   if (mem_total <= 0) return;
   mem_used = mem_total - mem_free - mem_cached - mem_buffers;
#if 0 // this lets the image cache to grow to fill all real free ram, if
      // there is any (ie ram unused by disk cache)
   if (mem_free < mem_total)
     {
        cache_max_adjust = mem_free;
        return;
     }
#endif

   max = ((mem_free + mem_cached + mem_buffers) / 8) - cache_max_usage;
   if (max < 0) max = 0;
   if (max > cache_max_usage) max = cache_max_usage;
   cache_max_adjust = max - cache_max_usage;
   
   if (cache_max_adjust < -cache_max_usage) 
     cache_max_adjust = -cache_max_usage;
   DBG("... cache_max_adjust = %i", cache_max_adjust);
   if (pval != cache_max_adjust)
     {
        DBG("... cache clean");
        // FIXME lock problem
        cache_clean();
     }
}

static void
img_cache(Img *img)
{
   eina_hash_del(active_images, img->key, img);
   if (img->dead)
     {
        DBG("... img %p '%s' dead", img , img->file.file);
        img_free(img);
        return;
     }
   if ((cache_usage + img->usage) > ((cache_max_usage + cache_max_adjust) * 1024))
     {
        DBG("... img %p '%s' too big for cache", img , img->file.file);
        img_free(img);
        return;
     }
   DBG("... img %p '%s' cached += %i", img , img->file.file, img->usage);
   if (img->incache > 0)
     {
        ERR("EEEEEEEEEEEEEEEEK!");
        ERR("EEEEEEEEEEEEEEEEK! %p '%s' already in cache",
               img, img->file.file);
        ERR("EEEEEEEEEEEEEEEEK!");
        return;
     }
   LKL(cache_lock);
   cache_images = eina_list_prepend(cache_images, img);
   LKU(cache_lock);
   img->incache++;
   cache_usage += img->usage;
   img->active = 0;
   img->cached = t_now;
   // FIXME: lock problem
   if (cache_usage > ((cache_max_usage + cache_max_adjust) * 1024))
     cache_clean();
}

static void
img_dead(Img *img)
{
   if (img->active) return;
   LKL(cache_lock);
   cache_images = eina_list_remove(cache_images, img);
   LKU(cache_lock);
   img->incache--;
   cache_usage -= img->usage;
   img_free(img);
}

static Eina_Bool
img_ok(Img *img)
{
   struct stat st;
   int ret;
   
   if (img->dead) return 0;
   if ((t_now > img->file.last_stat) &&
       ((t_now - img->file.last_stat) < stat_res_interval)) return 1;
   img->file.last_stat = t_now;
   ret = stat(img->file.file, &st);
   img->file.last_stat = t_now;
   if (ret < 0)
     {
        img->dead = 1;
        img_dead(img);
        return 0;
     }
   if (st.st_mtime != img->file.modtime)
     {
        img->dead = 1;
        img_dead(img);
        return 0;
     }
   return 1;
}

static Img *
img_load(const char *file, const char *key, RGBA_Image_Loadopts *load_opts)
{
   Img *img;
   char buf[8192];
   Eina_List *l, *l_next;
   
   if (!file) return NULL;
   DBG("... img_load '%s'", file);
   if (key) DBG("... ... key '%s'", key);
   if (key)
     snprintf(buf, sizeof(buf), "%s///::/%s/\001/%i/%1.8f/%ix%i",
              file, key, 
              load_opts->scale_down_by, 
              load_opts->dpi,
              load_opts->w, load_opts->h);
   else
     snprintf(buf, sizeof(buf), "%s///\001/%i/%1.8f/%ix%i",
              file, 
              load_opts->scale_down_by, 
              load_opts->dpi,
              load_opts->w, load_opts->h);
   DBG("... find '%s'", buf);
   img = eina_hash_find(active_images, buf);
   if ((img) && (img_ok(img)))
     {
        DBG("... found!");
        img->stats.load1saved++;
        img->ref++;
        DBG("... stats update");
        stats_update();
        DBG("... return %p", img);
        return img;
     }
   
   // FIXME: keep hash of cached images too 
   LKL(cache_lock);
   EINA_LIST_FOREACH_SAFE(cache_images, l, l_next, img)
    {
        if (!strcmp(img->key, buf))
          {
             LKL(img->lock);
             if (img_ok(img))
               {
                  DBG("... found cached");
                  cache_images = eina_list_remove_list(cache_images, l);
                  img->incache--;
                  cache_usage -= img->usage;
                  img->active = 1;
                  img->stats.load1saved++;
                  img->ref++;
                  eina_hash_direct_add(active_images, img->key, img);
                  DBG("... sats update");
                  stats_update();
                  DBG("... return %p", img);
                  LKU(img->lock);
                  LKU(cache_lock);
                  return img;
               }
             LKU(img->lock);
          }
     }
   LKU(cache_lock);
   DBG("... ned new img");
   return img_new(file, key, load_opts, buf);
}

static void
img_unload(Img *img)
{
   if (img->ref == 0)
     {
        ERR("EEEEEEEEEEEEEEEEK!");
        ERR("EEEEEEEEEEEEEEEEK! %p '%s' already @ ref 0",
               img, img->file.file);
        ERR("EEEEEEEEEEEEEEEEK!");
        return;
     }
   img->ref--;
   DBG("... img ref-- = %i", img->ref);
   if (img->ref == 0)
     {
        DBG("... img cache %p '%s'", img, img->file.file);
        img_cache(img);
     }
}

static void
img_unloaddata(Img *img)
{
   DBG("img_unloaddata() %p '%s'", img, img->file.file);
   if ((img->dref <= 0) && (img->useless) && (img->mem))
     {
        Image_Entry *ie = (Image_Entry *)img;
        
        DBG("... really do forced unload");
        if (!img->active) cache_usage -= img->usage;
        img->usage -= 
          (4096 * (((img->image.w * img->image.h * sizeof(DATA32)) + 4095) / 4096)) +
          sizeof(Mem);
        if (!img->active) cache_usage += img->usage;
        evas_cserve_mem_free(img->mem);
        stat_mems = eina_list_remove(stat_mems, img->mem);
        img->mem = NULL;
        img->image.data = NULL;
        img->dref = 0;
        DBG("... done");
        
        ie->flags.loaded = 0;
        ie->allocated.w = 0;
        ie->allocated.h = 0;
     }
}

static void
img_useless(Img *img)
{
   DBG("img_useless() %p", img);
   img->useless = 1;
   if (img->dref <= 0) img_unloaddata(img);
}

static void
img_forcedunload(Img *img)
{
   DBG("img_forcedunload() %p", img);
   img->dead = 1;
   img_unload(img);
}

static void
img_preload(Img *img)
{
   DBG("img_preload() %p", img);
}

static void
client_del(void *data, Client *c)
{
   Eina_List *images;
   Img *img;
   
   images = data;
   DBG("... CLIENT DEL %i", c->pid);
   EINA_LIST_FREE(images, img)
     {
        DBG("... unloaddata img %p", img);
        img_unloaddata(img);
        DBG("... unload img %p", img);
        img_unload(img);
     }
}

static Eina_Bool
getinfo_hash_image_cb(const Eina_Hash *hash __UNUSED__, 
                      const void *key __UNUSED__,
                      void *data, void *fdata __UNUSED__)
{
   Img *img = data;
   Eina_List **list = fdata;

   *list = eina_list_append(*list, img);
   return 1;
}

#ifdef BUILD_PTHREAD
static void *
load_data_thread(void *data)
{
   Load_Inf *li = data;
   Img *img = li->img;
   Client *c = li->c;
   Op_Loaddata_Reply msg;

   free(li);
   LKL(img->lock);
   if (img->mem)
     {
        memset(&msg, 0, sizeof(msg));
        msg.mem.id = img->mem->id;
        msg.mem.offset = img->mem->offset;
        msg.mem.size = img->mem->size;
        DBG("... reply");
        evas_cserve_client_send(c, OP_LOADDATA, sizeof(msg), (unsigned char *)(&msg));
        LKU(c->lock);
        return NULL;
     }
   img_loaddata(img);
   memset(&msg, 0, sizeof(msg));
   if (img->mem)
     {
        msg.mem.id = img->mem->id;
        msg.mem.offset = img->mem->offset;
        msg.mem.size = img->mem->size;
     }
   else
     msg.mem.id = msg.mem.offset = msg.mem.size = 0;
   LKU(img->lock);
   DBG("... reply");
   evas_cserve_client_send(c, OP_LOADDATA, sizeof(msg), (unsigned char *)(&msg));
   LKU(c->lock);
   return NULL;
}
#endif

static int
message(void *fdata, Server *s, Client *c, int opcode, int size, unsigned char *data)
{
   t_now = time(NULL);
   DBG("message @ %i...", (int)t_now);
   switch (opcode)
     {
     case OP_INIT:
          {
             Op_Init *rep;
             Op_Init msg;
             
             memset(&msg, 0, sizeof(msg));
             msg.pid = getpid();
             msg.server_id = server_id;
             msg.handle = c;
             rep = (Op_Init *)data;
             c->pid = rep->pid;
             if (rep->server_id == 1) // 2nd channel conn
               {
                  c->client_main = rep->handle;
               }
             c->func = client_del;
             c->data = NULL;
             DBG("OP_INIT %i", c->pid);
             DBG("... reply");
             evas_cserve_client_send(c, OP_INIT, sizeof(msg), (unsigned char *)(&msg));
          }
        break;
     case OP_LOAD:
          {
             Op_Load *rep;
             Op_Load_Reply msg;
             Img *img;
             RGBA_Image_Loadopts lopt = {0, 0.0, 0, 0, {0, 0, 0, 0}};
             char *file = NULL, *key = NULL;
             
             DBG("OP_LOAD %i", c->pid);
             rep = (Op_Load *)data;
             file = data + sizeof(Op_Load);
             key = file + strlen(file) + 1;
             if (key[0] == 0) key = NULL;
             lopt.scale_down_by = rep->lopt.scale_down_by;
             lopt.dpi = rep->lopt.dpi;
             lopt.w = rep->lopt.w;
             lopt.h = rep->lopt.h;
             lopt.region.x = rep->lopt.region.x;
             lopt.region.y = rep->lopt.region.y;
             lopt.region.w = rep->lopt.region.w;
             lopt.region.h = rep->lopt.region.h;
             DBG("... img_load '%s'", file);
             if (key) DBG("'%s'", (char *)key);
             else DBG("   '%s'", "");
             DBG("   lopt { %i %1.1f %i %i { %i %i %i %i}}",
               lopt.scale_down_by, lopt.dpi, lopt.w, lopt.h, 
               lopt.region.x, lopt.region.y, lopt.region.w, lopt.region.h);
             img = img_load(file, key, &lopt);
             DBG("... img_load = %p", img);
             if (img)
               {
                  DBG("... add image to client list");
                  if (c->client_main)
                    c->client_main->data = eina_list_append(c->client_main->data, img);
                  else
                    c->data = eina_list_append(c->data, img);
               }
             memset(&msg, 0, sizeof(msg));
             msg.handle = img;
             if ((img) && (img->mem))
               {
                  msg.mem.id = img->mem->id;
                  msg.mem.offset = img->mem->offset;
                  msg.mem.size = img->mem->size;
                  img->stats.load2saved++;
                  stats_update();
               }
             else
               msg.mem.id = msg.mem.offset = msg.mem.size = 0;
             if (img)
               {
                  msg.image.w = img->image.w;
                  msg.image.h = img->image.h;
                  msg.image.alpha = img->image.alpha;
               }
             DBG("... reply");
             evas_cserve_client_send(c, OP_LOAD, sizeof(msg), (unsigned char *)(&msg)); 
         } 
        break;
     case OP_UNLOAD:
          {
             Op_Unload *rep;
             Img *img;
             
             DBG("OP_UNLOAD %i", c->pid);
             rep = (Op_Unload *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  Eina_Bool doflush = 0;
                  
                  DBG("... remove %p from list", img);
                  if (c->client_main)
                    c->client_main->data = eina_list_remove(c->client_main->data, img);
                  else
                    c->data = eina_list_remove(c->data, img);
                  DBG("... unload %p", img);
                  LKL(img->lock);
                  img->ref++;
                  img_unload(img);
                  img->ref--;
                  if (img->ref == 0) doflush = 1;
                  LKU(img->lock);
                  if (doflush)
                    img_cache(img);
                  cache_clean();
               }
          } 
        break;
     case OP_LOADDATA:
          {
             Op_Loaddata *rep;
             Op_Loaddata_Reply msg;
             Img *img;
             
             DBG("OP_LOADDATA %i", c->pid);
             rep = (Op_Loaddata *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  if (img->mem)
                    {
                       DBG("... load saved - cached %p", img);
                       img->stats.load2saved++;
                       stats_update();
                       memset(&msg, 0, sizeof(msg));
                       if (img->mem)
                         {
                            msg.mem.id = img->mem->id;
                            msg.mem.offset = img->mem->offset;
                            msg.mem.size = img->mem->size;
                         }
                       else
                         msg.mem.id = msg.mem.offset = msg.mem.size = 0;
                       DBG("... reply");
                       evas_cserve_client_send(c, OP_LOADDATA, sizeof(msg), (unsigned char *)(&msg));
                    }
                  else
                    {
#ifdef BUILD_PTHREAD
                       pthread_t tid;
                       pthread_attr_t attr;
                       Load_Inf *li;
                       
                       DBG("... load data %p", img);
                       pthread_attr_init(&attr);
                       li = calloc(1, sizeof(Load_Inf));
                       if (li)
                         {
                            li->img= img;
                            li->c = c;
                            LKL(c->lock);
                            if (pthread_create(&tid, &attr, load_data_thread, li))
                              {
                                 perror("pthread_create()");
                              }
                            else
                              pthread_detach(tid);
                         }
                       pthread_attr_destroy(&attr);
#else
                       img_loaddata(img);
                       memset(&msg, 0, sizeof(msg));
                       if (img->mem)
                         {
                            msg.mem.id = img->mem->id;
                            msg.mem.offset = img->mem->offset;
                            msg.mem.size = img->mem->size;
                         }
                       else
                         msg.mem.id = msg.mem.offset = msg.mem.size = 0;
                       DBG("... reply");
                       evas_cserve_client_send(c, OP_LOADDATA, sizeof(msg), (unsigned char *)(&msg));
#endif
                    }
               }
             else
               {
                  msg.mem.id = msg.mem.offset = msg.mem.size = 0;
                  evas_cserve_client_send(c, OP_LOADDATA, sizeof(msg), (unsigned char *)(&msg));
               }
          }
        break;
     case OP_UNLOADDATA:
          {
             Op_Unloaddata *rep;
             Img *img;
             
             DBG("OP_UNLOADDATA %i", c->pid);
             rep = (Op_Unloaddata *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  DBG("... dref--");
                  LKL(img->lock);
                  img->dref--;
                  if (img->dref < 0) img->dref = 0;
                  DBG("... unload data %p '%s'", img, img->file.file);
                  img_unloaddata(img);
                  LKU(img->lock);
               }
          } 
        break;
     case OP_USELESSDATA:
          {
             Op_Unloaddata *rep;
             Img *img;
             
             DBG("OP_USELESSDATA %i", c->pid);
             rep = (Op_Unloaddata *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  DBG("... dref--");
                  LKL(img->lock);
                  img->dref--;
                  if (img->dref < 0) img->dref = 0;
                  DBG("... useless %p", img);
                  img_useless(img);
                  LKU(img->lock);
               }
          } 
        break;
     case OP_PRELOAD:
          {
             Op_Preload *rep;
             Img *img;
             
             DBG("OP_PRELOAD %i", c->pid);
             rep = (Op_Preload *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  LKL(img->lock);
                  if (c->client_main)
                    c->client_main->data = eina_list_remove(c->client_main->data, img);
                  else
                    c->data = eina_list_remove(c->data, img);
                  // FIXME: preload doesnt work async
                  img_preload(img);
                  LKU(img->lock);
               }
          }
     case OP_FORCEDUNLOAD:
          {
             Op_Forcedunload *rep;
             Img *img;
             
             DBG("OP_FORCEDUNLOAD %i", c->pid);
             rep = (Op_Forcedunload *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  Eina_Bool doflush = 0;
                  
                  LKL(img->lock);
                  DBG("remove %p from list", img);
                  if (c->client_main)
                    c->client_main->data = eina_list_remove(c->client_main->data, img);
                  else
                    c->data = eina_list_remove(c->data, img);
                  DBG("... forced unload now");
                  img->ref++;
                  img_forcedunload(img);
                  img->ref--;
                  if (img->ref == 0) doflush = 1;
                  LKU(img->lock);
                  if (doflush)
                    img_cache(img);
                  cache_clean();
               }
          } 
        break;
     case OP_GETCONFIG:
          {
             Op_Getconfig_Reply msg;
             
             DBG("OP_GETCONFIG %i", c->pid);
             msg.cache_max_usage = cache_max_usage;
             msg.cache_item_timeout = cache_item_timeout;
             msg.cache_item_timeout_check = cache_item_timeout_check;
             DBG("... reply");
             evas_cserve_client_send(c, OP_GETCONFIG, sizeof(msg), (unsigned char *)(&msg));
          } 
        break;
     case OP_SETCONFIG:
          {
             Op_Setconfig *rep;
             
             DBG("OP_SETCONFIG %i", c->pid);
             rep = (Op_Setconfig *)data;
             cache_max_usage = rep->cache_max_usage;
             cache_item_timeout = rep->cache_item_timeout;
             cache_item_timeout_check = rep->cache_item_timeout_check;
             DBG("... cache timeout");
             cache_timeout(t_now);
             DBG("... cache clean");
             cache_clean();
          } 
        break;
     case OP_GETSTATS:
          {
             Op_Getstats_Reply msg;

             DBG("OP_GETSTATS %i", c->pid);
             stats_calc();
             msg.saved_memory = saved_memory;
             msg.wasted_memory = (real_memory - alloced_memory);
             msg.saved_memory_peak = saved_memory_peak;
             msg.wasted_memory_peak = (real_memory_peak - alloced_memory_peak);
             msg.saved_time_image_header_load = saved_load_lifetime + saved_load_time;
             msg.saved_time_image_data_load = saved_loaddata_lifetime + saved_loaddata_time;
             DBG("... reply");
             evas_cserve_client_send(c, OP_GETSTATS, sizeof(msg), (unsigned char *)(&msg));
          } 
        break;
     case OP_GETINFO:
          {
             Op_Getinfo_Reply *msg;
             int len;
             Eina_List *imgs = NULL, *l;
             Img *img;
             
             DBG("OP_GETINFO %i", c->pid);
             len = sizeof(Op_Getinfo_Reply);
             DBG("... foreach");
             if (active_images)
               eina_hash_foreach(active_images, getinfo_hash_image_cb, &imgs);
             DBG("... walk foreach list output");
             LKL(cache_lock);
             EINA_LIST_FOREACH(cache_images, l, img)
               {
                  imgs = eina_list_append(imgs, img);
               }
             LKU(cache_lock);
             DBG("... walk image cache");
             EINA_LIST_FOREACH(imgs, l, img)
               {
                  len += sizeof(Op_Getinfo_Item);
                  if (img->file.file) len += strlen(img->file.file);
                  len++;
                  if (img->file.key) len += strlen(img->file.key);
                  len++;
               }
             DBG("... malloc msg");
             msg = malloc(len);
             if (msg)
               {
                  unsigned char *p;
                  
                  DBG("...   init msg");
                  memset(msg, 0, len);
                  p = (unsigned char *)msg;
                  msg->active.mem_total = 0;
                  msg->active.count = 0;
                  msg->cached.mem_total = 0;
                  msg->cached.count = 0;
                  p += sizeof(Op_Getinfo_Reply);
                  DBG("...   walk all imgs");
                  EINA_LIST_FOREACH(imgs, l, img)
                    {
                       Op_Getinfo_Item *itt, it;

                       LKL(img->lock); 
                       DBG("...   img %p", img);
                       memset(&it, 0, sizeof(Op_Getinfo_Item));
                       itt = (Op_Getinfo_Item *)p;
                       it.file_key_size = 0;
                       if (img->file.file)
                         {
                            strcpy(p + sizeof(Op_Getinfo_Item) + it.file_key_size, img->file.file);
                            it.file_key_size += strlen(img->file.file);
                         }
                       p[sizeof(Op_Getinfo_Item) + it.file_key_size] = 0;
                       it.file_key_size += 1;
                       if (img->file.key)
                         {
                            strcpy(p + sizeof(Op_Getinfo_Item) + it.file_key_size, img->file.key);
                            it.file_key_size += strlen(img->file.key);
                         }
                       p[sizeof(Op_Getinfo_Item) + it.file_key_size] = 0;
                       it.file_key_size += 1;
                       
                       it.w = img->image.w;
                       it.h = img->image.h;
                       it.file_mod_time = img->file.modtime;
                       it.file_checked_time = img->file.last_stat;
                       if (!img->active)
                         it.cached_time = img->cached;
                       else
                         it.cached_time = 0;
                       it.refcount = img->ref;
                       it.data_refcount = img->dref;
                       it.memory_footprint = img->usage;
                       it.head_load_time = img->stats.load1;
                       it.data_load_time = img->stats.load2;
                       it.alpha = img->image.alpha;
                       if (img->image.data)
                         it.data_loaded = 1;
                       else
                         it.data_loaded = 0;
                       it.active = img->active;
                       if (it.active)
                         {
                            msg->active.count++;
                            msg->active.mem_total += img->usage;
                         }
                       else
                         {
                            msg->cached.count++;
                            msg->cached.mem_total += img->usage;
                         }
                       it.dead = img->dead;
                       it.useless = img->useless;
                       DBG("...   memcpy %p %p %i ", 
                         itt, &it, sizeof(Op_Getinfo_Item));
                       memcpy(itt, &it, sizeof(Op_Getinfo_Item));
                       DBG("...   memcpy done %p", img);
                       p += sizeof(Op_Getinfo_Item) + it.file_key_size;
                       LKU(img->lock); 
                    }
                  DBG("...   walk all imgs done");
                  msg->active.mem_total = 
                    (msg->active.mem_total + 1023) / 1024;
                  msg->cached.mem_total = 
                    (msg->cached.mem_total + 1023) / 1024;
                  DBG("... reply");
                  evas_cserve_client_send(c, OP_GETINFO, len, (unsigned char *)msg);
                  free(msg);
               }
             else
               {
                  DBG("... reply empty");
                  evas_cserve_client_send(c, OP_GETINFO, 0, NULL);
               }
               DBG("... free imgs list");
           if (imgs) eina_list_free(imgs);
          } 
        break;
     default:
        DBG("OP_... UNKNOWN??? %i opcode: %i", c->pid, opcode);
        break;
     }
   return 0;
}

static void
parse_args(int argc, char **argv)
{
   int i;
   
   for (i = 1; i < argc; i++)
     {
        if ((!strcmp(argv[i], "-h")) ||
            (!strcmp(argv[i], "-help")) ||
            (!strcmp(argv[i], "--help")))
          {
             printf("Options:\n"
                    "\t-h          This help\n"
                    "\t-csize      Size of speculative cache (Kb)\n"
                    "\t-ctime      Maximum life of a cached image (seconds)\n"
                    "\t-ctimecheck Time between checking the cache for timeouts (seconds)\n"
                    "\t-debug      Enable debug logging\n"
                    "\n");
             exit(0);
          }
        else if ((!strcmp(argv[i], "-csize")) && (i < (argc - 1)))
          {
             i++;
             cache_max_usage = atoi(argv[i]);
          }
        else if ((!strcmp(argv[i], "-ctime")) && (i < (argc - 1)))
          {
             i++;
             cache_item_timeout = atoi(argv[i]);
          }
        else if ((!strcmp(argv[i], "-ctimecheck")) && (i < (argc - 1)))
          {
             i++;
             cache_item_timeout_check = atoi(argv[i]);
          }
        else if (!strcmp(argv[i], "-debug"))
          {
	     eina_log_level_set(EINA_LOG_LEVEL_DBG);
          }
     }
}

static int exit_flag = 0;

static void
exit_handler(int x, siginfo_t *info, void *data)
{
   exit_flag = 1;
}

static void
pipe_handler(int x, siginfo_t *info, void *data)
{
}

static void
signal_init(void)
{
   struct sigaction action, old_action;
   
   action.sa_handler = NULL;
   action.sa_sigaction = exit_handler;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGINT, &action, &old_action);

   action.sa_handler = NULL;
   action.sa_sigaction = exit_handler;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGTERM, &action, &old_action);

   action.sa_handler = NULL;
   action.sa_sigaction = exit_handler;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGQUIT, &action, &old_action);

   action.sa_handler = NULL;
   action.sa_sigaction = pipe_handler;
   action.sa_flags = SA_RESTART | SA_SIGINFO;
   sigemptyset(&action.sa_mask);
   sigaction(SIGPIPE, &action, &old_action);

   // SIGUSR1
   // SIGUSR2
   // SIGHUP

   // SIGCHLD
   
   // SIGSEGV
   // SIGILL
   // SIGBUS
   // SIGFPE
   // SIGABRT
}

static void
signal_shutdown(void)
{
}

int
main(int argc, char **argv)
{
   Server *s;
   time_t last_check, t, t_next;
   pid_t pid;
   
   t = time(NULL);
   pid = getpid();
   t ^= (pid << 24) | (pid << 16) | (pid << 8) | (pid);
   srand(t);
   server_id = rand();
   
   parse_args(argc, argv);
   
   unsetenv("EVAS_CSERVE");

   DBG("eina init...");
   eina_init();
   _evas_cserve_bin_log_dom = eina_log_domain_register("Evas_cserve_bin", CSERVE_BIN_DEFAULT_COLOR);
   if(_evas_cserve_bin_log_dom < 0) {
     DBG("Problem with eina_log : impossible to create a log domain");
     eina_shutdown();
     exit(1);
   }

   DBG("evas init...");
   evas_init();
   DBG("img init...");
   img_init();
   DBG("signal init...");
   signal_init();
   DBG("cserve add...");
   s = evas_cserve_server_add();
   if (!s)
     {
        ERR("ERROR: server socket init fail. abort.");
        goto error;
     }
   DBG("mem open (status)...");
   stat_mem = evas_cserve_mem_open(0, 0, "status", sizeof(int), 0);
   if (stat_mem)
     {
        WRN("WARNING: previous evas_cserve left garbage. cleaning up.");
        stat_clean(stat_mem);
        evas_cserve_mem_close(stat_mem);
        stat_mem = NULL;
     }
   DBG("mem new (status)...");
   stat_mem = evas_cserve_mem_new(sizeof(int), "status");
   if (!stat_mem)
     {
        ERR("ERROR: cannot create status shmseg. abort.");
        goto error;
     }
   DBG("init status...");
   if (!stat_init(stat_mem))
     {
        ERR("cannot init status shmseg. abort.");
        evas_cserve_mem_free(stat_mem);
        stat_mem = NULL;
        goto error;
     }
   
   DBG("cset server message handler...");
   evas_cserve_server_message_handler_set(s, message, NULL);
   last_check = time(NULL);
   t_next = 0; 
   if (cache_item_timeout_check > 0) t_next = cache_item_timeout_check;
   DBG("LOOP!!! ...");
   for (;;)
     {
        /* fixme: timeout 0 only her - future use timeouts for timed
         * housekeping */
        if (exit_flag) break;
        DBG("wait for messages...");
        evas_cserve_server_wait(s, t_next * 1000000);
        if (exit_flag) break;
        t = time(NULL);
        t_next = t - last_check;
        if ((cache_item_timeout_check > 0) &&
            ((t_next) >= cache_item_timeout_check))
          {
             DBG("check timeout of items...");
             t_next = cache_item_timeout_check;
             
             last_check = t;
             DBG("cache timeout...");
             cache_timeout(t);
             DBG("meminfo check...");
             meminfo_check();
             DBG("mem cache adjust...");
             mem_cache_adjust();
          }
        if ((t_next <= 0) && (cache_item_timeout_check > 0))
          t_next = 1;
        DBG("sleep for %isec...", (int)t_next);
        
        LKL(strshr_freeme_lock);
        if (strshr_freeme_count > 0)
          {
             int i;
             
             for (i = 0; i < strshr_freeme_count; i++)
               eina_stringshare_del(strshr_freeme[i]);
             strshr_freeme_count = 0;
          }
        LKU(strshr_freeme_lock);
        
        LKL(cache_lock);
        if (cache_cleanme)
          {
             Eina_List *l;
             Img *img;
             Eina_List *kills = NULL;
            
             EINA_LIST_FOREACH(cache_images, l, img)
               {
                  LKL(img->lock);
                  if (img->killme)
                    kills = eina_list_append(kills, img);
                  LKU(img->lock);
               }
             while (kills)
               {
                  img = kills->data;
                  kills = eina_list_remove_list(kills, kills);
                  LKL(img->lock);
                  cache_images = eina_list_remove(cache_images, img);
                  img->incache--;
                  cache_usage -= img->usage;
                  DBG("...   IMG FREE %p", img);
                  img_free(img);
               }
             cache_cleanme = 0;
          }
        LKU(cache_lock);
     }
   DBG("end loop...");
   error:
   DBG("cleanup...");
   if (stat_mem)
     {
        DBG("clean mem stat...");
        stat_clean(stat_mem);
     }
   DBG("signal shutdown...");
   signal_shutdown();
   DBG("img shutdown...");
   img_shutdown();
   if (stat_mem)
     {
        DBG("free stat mem...");
        evas_cserve_mem_free(stat_mem);
        stat_mem = NULL;
     }
   if (s)
     {
        DBG("del server...");
        evas_cserve_server_del(s);
     }
   DBG("evas shutdown...");
   evas_shutdown();
   eina_log_domain_unregister(_evas_cserve_bin_log_dom);
   DBG("eina shutdown...");
   eina_shutdown();
   DBG("exit..");
   return 0;
}
