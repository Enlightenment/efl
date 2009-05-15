#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <signal.h>
#include <sys/time.h>
#include <time.h>
#ifdef _WIN32
# include <windows.h>
#endif

#include "Evas.h"
#include "evas_cs.h"

#define D(fmt, args...) EINA_ERROR_PDBG(fmt, ##args)

// fixme:'s
// 
// preload - make it work (both)

// 
// pants!

typedef struct _Img Img;
typedef struct _Lopt Lopt;

struct _Lopt
{
   int    scale_down_by; // if > 1 then use this
   double dpi; // if > 0.0 use this
   int    w, h; // if > 0 use this
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
   Eina_Bool dead : 1;
   Eina_Bool active : 1;
   Eina_Bool useless : 1;
};

// config
static int stat_res_interval = 2;

static time_t t_now = 0;

static int server_id = 0;

static Evas_Cache_Image *cache = NULL;

static Eina_Hash *active_images = NULL;
static Eina_List *cache_images = NULL;
static int cache_usage = 0;
static int cache_max_usage = 1 * 1024;
static int cache_max_adjust = 0;
static int cache_item_timeout = -1;
static int cache_item_timeout_check = -1;
static Mem *stat_mem = NULL;

static int stat_mem_num = 0;
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
   return (Image_Entry *)img;
}

static void
_img_dealloc(Image_Entry *ie)
{
   Img *img = (Img *)ie;
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
}

static void
img_shutdown(void)
{
   evas_cache_image_shutdown(cache);
   cache = NULL;
   // FIXME: shutdown properly
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
   
   D("... stat %s\n", file);
   ret = stat(file, &st);
   if (ret < 0) return NULL;
   D("... load header\n");
   t = get_time();
   ie = evas_cache_image_request(cache, file, key, load_opts, &err);
   t = get_time() - t;
   D("... header done\n");
   if (!ie) return NULL;
   D("... ie->cache = %p\n", ie->cache);
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
   cache_clean();
}

static void
img_free(Img *img)
{
   if (img->incache > 0)
     {
        printf("EEEEEEEEEEEEEEEEK!\n");
        printf("EEEEEEEEEEEEEEEEK! %p '%s' still in cache\n", 
               img, img->file.file);
        printf("EEEEEEEEEEEEEEEEK!\n");
        return;
     }
   stats_lifetime_update(img);
   stats_update();
   eina_stringshare_del(img->key);
   eina_stringshare_del(img->file.file);
   eina_stringshare_del(img->file.key);
   evas_cache_image_drop((Image_Entry *)img);
}

static void
cache_clean(void)
{
   D("... cache clean!!! do\n");
   while ((cache_usage > ((cache_max_usage + cache_max_adjust) * 1024)) && 
          (cache_images))
     {
        Img *img;
        Eina_List *l;
        
        D("... clean loop %i > %i\n", cache_usage, (cache_max_usage + cache_max_adjust) * 1024);
        l = eina_list_last(cache_images);
        if (!l) break;
        img = l->data;
        if (!img) break;
        D("...   REMOVE %p '%s'\n", img, img->file.file);
        cache_images = eina_list_remove_list(cache_images, l);
        img->incache--;
        cache_usage -= img->usage;
        D("...   IMG FREE %p\n", img);
        img_free(img);
     }
}

static void
cache_timeout(time_t t)
{
   Eina_List *l, *l_next;
   Img *img;
   
   if (cache_item_timeout < 0) return;
   EINA_LIST_FOREACH_SAFE(cache_images, l, l_next, img)
     {
        if ((t - img->cached) > cache_item_timeout)
          {
             cache_images = eina_list_remove_list(cache_images, l);
             img->incache--;
             cache_usage -= img->usage;
             img_free(img);
          }
     }
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
   D("... cache_max_adjust = %i\n", cache_max_adjust);
   if (pval != cache_max_adjust)
     {
        D("... cache clean\n");
        cache_clean();
     }
}

static void
img_cache(Img *img)
{
   eina_hash_del(active_images, img->key, img);
   if (img->dead)
     {
        D("... img %p '%s' dead\n", img , img->file.file);
        img_free(img);
        return;
     }
   if ((cache_usage + img->usage) > ((cache_max_usage + cache_max_adjust) * 1024))
     {
        D("... img %p '%s' too big for cache\n", img , img->file.file);
        img_free(img);
        return;
     }
   D("... img %p '%s' cached += %i\n", img , img->file.file, img->usage);
   if (img->incache > 0)
     {
        printf("EEEEEEEEEEEEEEEEK!\n");
        printf("EEEEEEEEEEEEEEEEK! %p '%s' already in cache\n", 
               img, img->file.file);
        printf("EEEEEEEEEEEEEEEEK!\n");
        return;
     }
   cache_images = eina_list_prepend(cache_images, img);
   img->incache++;
   cache_usage += img->usage;
   img->active = 0;
   img->cached = t_now;
   if (cache_usage > ((cache_max_usage + cache_max_adjust) * 1024))
     cache_clean();
}

static void
img_dead(Img *img)
{
   if (img->active) return;
   cache_images = eina_list_remove(cache_images, img);
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
   D("... img_load '%s'\n", file);
   if (key) D("... ... key '%s'\n", key);
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
   D("... find '%s'\n", buf);
   img = eina_hash_find(active_images, buf);
   if ((img) && (img_ok(img)))
     {
        D("... found!\n");
        img->stats.load1saved++;
        img->ref++;
        D("... sats update\n");
        stats_update();
        D("... return %p\n", img);
        return img;
     }
   
   // FIXME: keep hash of cached images too 
   EINA_LIST_FOREACH_SAFE(cache_images, l, l_next, img)
    {
        if (!strcmp(img->key, buf))
          {
             if (img_ok(img))
               {
                  D("... found cached\n");
                  cache_images = eina_list_remove_list(cache_images, l);
                  img->incache--;
                  cache_usage -= img->usage;
                  img->active = 1;
                  img->stats.load1saved++;
                  img->ref++;
                  eina_hash_direct_add(active_images, img->key, img);
                  D("... sats update\n");
                  stats_update();
                  D("... return %p\n", img);
                  return img;
               }
          }
     }
   D("... ned new img\n");
   return img_new(file, key, load_opts, buf);
}

static void
img_unload(Img *img)
{
   if (img->ref == 0)
     {
        printf("EEEEEEEEEEEEEEEEK!\n");
        printf("EEEEEEEEEEEEEEEEK! %p '%s' already @ ref 0\n", 
               img, img->file.file);
        printf("EEEEEEEEEEEEEEEEK!\n");
        return;
     }
   img->ref--;
   D("... img ref-- = %i\n", img->ref);
   if (img->ref == 0)
     {
        D("... img cache %p '%s'\n", img, img->file.file);
        img_cache(img);
     }
}

static void
img_unloaddata(Img *img)
{
   D("img_unloaddata() %p '%s'\n", img, img->file.file);
   if ((img->dref <= 0) && (img->useless) && (img->mem))
     {
        Image_Entry *ie = (Image_Entry *)img;
        
        D("... really do forced unload\n");
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
        D("... done\n");
        
        ie->flags.loaded = 0;
        ie->allocated.w = 0;
        ie->allocated.h = 0;
     }
}

static void
img_useless(Img *img)
{
   D("img_useless() %p\n", img);
   img->useless = 1;
   if (img->dref <= 0) img_unloaddata(img);
}

static void
img_forcedunload(Img *img)
{
   D("img_forcedunload() %p\n", img);
   img->dead = 1;
   img_unload(img);
}

static void
img_preload(Img *img)
{
   D("img_preload() %p\n", img);
   printf("preload '%s'\n", img->file.file);
}

static void
client_del(void *data, Client *c)
{
   Eina_List *images;
   Img *img;
   
   images = data;
   D("... CLIENT DEL %i\n", c->pid);
   EINA_LIST_FREE(images, img)
     {
        D("... unloaddata img %p\n", img);
        img_unloaddata(img);
        D("... unload img %p\n", img);
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

static int
message(void *fdata, Server *s, Client *c, int opcode, int size, unsigned char *data)
{
   t_now = time(NULL);
   D("message @ %i...\n", (int)t_now);
   switch (opcode)
     {
     case OP_INIT:
          {
             Op_Init *rep;
             Op_Init msg;
             
             memset(&msg, 0, sizeof(msg));
             msg.pid = getpid();
             msg.server_id = server_id;
             rep = (Op_Init *)data;
             c->pid = rep->pid;
             c->func = client_del;
             c->data = NULL;
             D("OP_INIT %i\n", c->pid);
             D("... reply\n");
             evas_cserve_client_send(c, OP_INIT, sizeof(msg), (unsigned char *)(&msg));
          }
        break;
     case OP_LOAD:
          {
             Op_Load *rep;
             Op_Load_Reply msg;
             Img *img;
             RGBA_Image_Loadopts lopt = {0, 0.0, 0, 0};
             char *file = NULL, *key = NULL;
             
             D("OP_LOAD %i\n", c->pid);
             rep = (Op_Load *)data;
             file = data + sizeof(Op_Load);
             key = file + strlen(file) + 1;
             if (key[0] == 0) key = NULL;
             lopt.scale_down_by = rep->lopt.scale_down_by;
             lopt.dpi = rep->lopt.dpi;
             lopt.w = rep->lopt.w;
             lopt.h = rep->lopt.h;
             D("... img_load '%s'\n", file);
             if (key) D("'%s'\n", key);
             else D("   '%s'\n", NULL);
             D("   lopt { %i %1.1f %i %i}\n", 
               lopt.scale_down_by, lopt.dpi, lopt.w, lopt.h);
             img = img_load(file, key, &lopt);
             D("... img_load = %p\n", img);
             if (img)
               {
                  D("... add image to client list\n");
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
             D("... reply\n");
             evas_cserve_client_send(c, OP_LOAD, sizeof(msg), (unsigned char *)(&msg)); 
         } 
        break;
     case OP_UNLOAD:
          {
             Op_Unload *rep;
             Img *img;
             
             D("OP_UNLOAD %i\n", c->pid);
             rep = (Op_Unload *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  D("... remove %p from list\n", img);
                  c->data = eina_list_remove(c->data, img);
                  D("... unload %p\n", img);
                  img_unload(img);
               }
          } 
        break;
     case OP_LOADDATA:
          {
             Op_Loaddata *rep;
             Op_Loaddata_Reply msg;
             Img *img;
             
             D("OP_LOADDATA %i\n", c->pid);
             rep = (Op_Loaddata *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  if (img->mem)
                    {
                       D("... load saved - cached %p\n", img);
                       img->stats.load2saved++;
                       stats_update();
                    }
                  else
                    {
                       D("... load data %p\n", img);
                       img_loaddata(img);
                    }
                  memset(&msg, 0, sizeof(msg));
                  if (img->mem)
                    {
                       msg.mem.id = img->mem->id;
                       msg.mem.offset = img->mem->offset;
                       msg.mem.size = img->mem->size;
                    }
                  else
                    msg.mem.id = msg.mem.offset = msg.mem.size = 0;
               }
             else
               msg.mem.id = msg.mem.offset = msg.mem.size = 0;
             D("... reply\n");
             evas_cserve_client_send(c, OP_LOADDATA, sizeof(msg), (unsigned char *)(&msg));
          }
        break;
     case OP_UNLOADDATA:
          {
             Op_Unloaddata *rep;
             Img *img;
             
             D("OP_UNLOADDATA %i\n", c->pid);
             rep = (Op_Unloaddata *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  D("... dref--\n");
                  img->dref--;
                  if (img->dref < 0) img->dref = 0;
                  D("... unload data %p '%s'\n", img, img->file.file);
                  img_unloaddata(img);
               }
          } 
        break;
     case OP_USELESSDATA:
          {
             Op_Unloaddata *rep;
             Img *img;
             
             D("OP_USELESSDATA %i\n", c->pid);
             rep = (Op_Unloaddata *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  D("... dref--\n");
                  img->dref--;
                  if (img->dref < 0) img->dref = 0;
                  D("... useless %p\n", img);
                  img_useless(img);
               }
          } 
        break;
     case OP_PRELOAD:
          {
             Op_Preload *rep;
             Img *img;
             
             D("OP_PRELOAD %i\n", c->pid);
             rep = (Op_Preload *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  c->data = eina_list_remove(c->data, img);
                  img_preload(img);
               }
          }
     case OP_FORCEDUNLOAD:
          {
             Op_Forcedunload *rep;
             Img *img;
             
             D("OP_FORCEDUNLOAD %i\n", c->pid);
             rep = (Op_Forcedunload *)data;
             img = rep->handle;
             if ((img) && (rep->server_id == server_id))
               {
                  D("remove %p from list\n", img);
                  c->data = eina_list_remove(c->data, img);
                  D("... forced unload now\n");
                  img_forcedunload(img);
               }
          } 
        break;
     case OP_GETCONFIG:
          {
             Op_Getconfig_Reply msg;
             
             D("OP_GETCONFIG %i\n", c->pid);
             msg.cache_max_usage = cache_max_usage;
             msg.cache_item_timeout = cache_item_timeout;
             msg.cache_item_timeout_check = cache_item_timeout_check;
             D("... reply\n");
             evas_cserve_client_send(c, OP_GETCONFIG, sizeof(msg), (unsigned char *)(&msg));
          } 
        break;
     case OP_SETCONFIG:
          {
             Op_Setconfig *rep;
             
             D("OP_SETCONFIG %i\n", c->pid);
             rep = (Op_Setconfig *)data;
             cache_max_usage = rep->cache_max_usage;
             cache_item_timeout = rep->cache_item_timeout;
             cache_item_timeout_check = rep->cache_item_timeout_check;
             D("... cache timeout\n");
             cache_timeout(t_now);
             D("... cache clean\n");
             cache_clean();
          } 
        break;
     case OP_GETSTATS:
          {
             Op_Getstats_Reply msg;

             D("OP_GETSTATS %i\n", c->pid);
             stats_calc();
             msg.saved_memory = saved_memory;
             msg.wasted_memory = (real_memory - alloced_memory);
             msg.saved_memory_peak = saved_memory_peak;
             msg.wasted_memory_peak = (real_memory_peak - alloced_memory_peak);
             msg.saved_time_image_header_load = saved_load_lifetime + saved_load_time;
             msg.saved_time_image_data_load = saved_loaddata_lifetime + saved_loaddata_time;
             D("... reply\n");
             evas_cserve_client_send(c, OP_GETSTATS, sizeof(msg), (unsigned char *)(&msg));
          } 
        break;
     case OP_GETINFO:
          {
             Op_Getinfo_Reply *msg;
             int len;
             Eina_List *imgs = NULL, *l;
             Img *img;
             
             D("OP_GETINFO %i\n", c->pid);
             len = sizeof(Op_Getinfo_Reply);
             D("... foreach\n");
             if (active_images)
               eina_hash_foreach(active_images, getinfo_hash_image_cb, &imgs);
             D("... walk foreach list output\n");
             EINA_LIST_FOREACH(cache_images, l, img)
               {
                  imgs = eina_list_append(imgs, img);
               }
             D("... walk image cache\n");
             EINA_LIST_FOREACH(imgs, l, img)
               {
                  len += sizeof(Op_Getinfo_Item);
                  if (img->file.file) len += strlen(img->file.file);
                  len++;
                  if (img->file.key) len += strlen(img->file.key);
                  len++;
               }
             D("... malloc msg\n");
             msg = malloc(len);
             if (msg)
               {
                  unsigned char *p;
                  
                  D("...   init msg\n");
                  memset(msg, 0, len);
                  p = (unsigned char *)msg;
                  msg->active.mem_total = 0;
                  msg->active.count = 0;
                  msg->cached.mem_total = 0;
                  msg->cached.count = 0;
                  p += sizeof(Op_Getinfo_Reply);
                  D("...   walk all imgs\n");
                  EINA_LIST_FOREACH(imgs, l, img)
                    {
                       Op_Getinfo_Item *itt, it;

                       D("...   img %p\n", img);
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
                       D("...   memcpy %p %p %i \n", 
                         itt, &it, sizeof(Op_Getinfo_Item));
                       memcpy(itt, &it, sizeof(Op_Getinfo_Item));
                       D("...   memcpy done n\n", img);
                       p += sizeof(Op_Getinfo_Item) + it.file_key_size;
                    }
                  D("...   walk all imgs done\n");
                  msg->active.mem_total = 
                    (msg->active.mem_total + 1023) / 1024;
                  msg->cached.mem_total = 
                    (msg->cached.mem_total + 1023) / 1024;
                  D("... reply\n");
                  evas_cserve_client_send(c, OP_GETINFO, len, msg);
                  free(msg);
               }
             else
               {
                  D("... reply empty\n");
                  evas_cserve_client_send(c, OP_GETINFO, 0, NULL);
               }
               D("... free imgs list\n");
           if (imgs) eina_list_free(imgs);
          } 
        break;
     default:
        D("OP_... UNKNOWN??? %i opcode: %i\n", c->pid, opcode);
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
             eina_error_log_level_set(EINA_ERROR_LEVEL_DBG);
          }
     }
}

static exit_flag = 0;

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

   D("eina init...\n");
   eina_init();
   D("evas init...\n");
   evas_init();

   D("img init...");
   img_init();
   D("signal init...\n");
   signal_init();
   D("cserve add...\n");
   s = evas_cserve_server_add();
   if (!s)
     {
        printf("ERROR: server socket init fail. abort.\n");
        goto error;
     }
   D("mem open (status)...\n");
   stat_mem = evas_cserve_mem_open(0, 0, "status", sizeof(int), 0);
   if (stat_mem)
     {
        printf("WARNING: previous evas_cserve left garbage. cleaning up.\n");
        stat_clean(stat_mem);
        evas_cserve_mem_close(stat_mem);
        stat_mem = NULL;
     }
   D("mem new (status)...\n");
   stat_mem = evas_cserve_mem_new(sizeof(int), "status");
   if (!stat_mem)
     {
        printf("ERROR: cannot create status shmseg. abort.\n");
        goto error;
     }
   D("init status...\n");
   if (!stat_init(stat_mem))
     {
        printf("ERROR: cannot init status shmseg. abort.\n");
        evas_cserve_mem_free(stat_mem);
        stat_mem = NULL;
        goto error;
     }
   
   D("cset server message handler...\n");
   evas_cserve_server_message_handler_set(s, message, NULL);
   last_check = time(NULL);
   t_next = 0; 
   if (cache_item_timeout_check > 0) t_next = cache_item_timeout_check;
   D("LOOP!!! ...\n");
   for (;;)
     {
        /* fixme: timeout 0 only her - future use timeouts for timed
         * housekeping */
        if (exit_flag) break;
        D("wait for messages...\n");
        evas_cserve_server_wait(s, t_next * 1000000);
        if (exit_flag) break;
        t = time(NULL);
        t_next = t - last_check;
        if ((cache_item_timeout_check > 0) &&
            ((t_next) >= cache_item_timeout_check))
          {
             D("check timeout of items...\n");
             t_next = cache_item_timeout_check;
             
             last_check = t;
             D("cache timeout...\n");
             cache_timeout(t);
             D("meminfo check...\n");
             meminfo_check();
             D("mem cache adjust...\n");
             mem_cache_adjust();
          }
        if ((t_next <= 0) && (cache_item_timeout_check > 0))
          t_next = 1;
        D("sleep for %isec...\n", t_next);
     }
   D("end loop...\n");
   error:
   D("cleanup...\n");
   if (stat_mem)
     {
        D("clean mem stat...\n");
        stat_clean(stat_mem);
     }
   D("signal shutdown...\n");
   signal_shutdown();
   D("img shutdown...\n");
   img_shutdown();
   if (stat_mem)
     {
        D("free stat mem...\n");
        evas_cserve_mem_free(stat_mem);
        stat_mem = NULL;
     }
   if (s)
     {
        D("del server...\n");
        evas_cserve_server_del(s);
     }
   D("evas shutdown...\n");
   evas_shutdown();
   D("eina shutdown...\n");
   eina_shutdown();
   D("exit..\n");
   return 0;
}
