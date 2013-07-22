#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>
#include <sys/mman.h>

#ifdef DEBUG_LOAD_TIME
   #include <sys/time.h>
#endif

#include "evas_cserve2.h"
#include "evas_cs2_utils.h"

#include <Evas_Loader.h>

// For testing purposes only.
#define EXPERIMENTAL_SHARED_INDEX 0
#if EXPERIMENTAL_SHARED_INDEX
typedef int string_t;
#else
#define cserve2_shared_string_get(str) (str)
#define cserve2_shared_string_del(str) (eina_stringshare_del(str))
#define cserve2_shared_string_add(str) (eina_stringshare_add(str))
#define cserve2_shared_string_ref(str) (eina_stringshare_ref(str))
typedef Eina_Stringshare* string_t;
#endif

typedef struct _Entry Entry;
typedef struct _Reference Reference;
typedef struct _File_Data File_Data;
typedef struct _Image_Data Image_Data;
typedef struct _File_Watch File_Watch;

typedef struct _Font_Source Font_Source;
typedef struct _Font_Entry Font_Entry;
typedef struct _Font_Cache Font_Cache;

typedef enum {
   CSERVE2_IMAGE_FILE,
   CSERVE2_IMAGE_DATA,
   CSERVE2_FONT_ENTRY
} Entry_Type;

struct _Entry {
   unsigned int id;
   Eina_List *references;
   Slave_Request *request;
   Entry_Type type;
#ifdef DEBUG_LOAD_TIME
   struct timeval load_start;
   struct timeval load_finish;
   int load_time;
   int saved_time;
#endif
};

struct _File_Data {
   Entry base;
   string_t path;
   string_t key;
   string_t loader_data;
   int w, h;
   int frame_count;
   int loop_count;
   int loop_hint;
   File_Watch *watcher;
   Eina_List *images;
   Eina_Bool alpha : 1;
   Eina_Bool invalid : 1;
};

// Default values for load options commented below
struct _Image_Data {
   Entry base;
   unsigned int file_id;
   File_Data *file;
   Evas_Image_Load_Opts opts;
   Shm_Handle *shm;
   Eina_Bool alpha_sparse : 1;
   Eina_Bool unused : 1;
   Eina_Bool doload : 1;
};

struct _Font_Source {
   string_t key;
   string_t name;
   string_t file;
   int references;
   void *ft;
};

struct _Font_Entry {
   Entry base;
   unsigned int rend_flags;
   unsigned int size;
   unsigned int dpi;
   Font_Source *src;
   void *ft;
   Fash_Glyph2 *glyphs;
   unsigned int nglyphs;
   Eina_Inlist *caches;
   Font_Cache *last_cache;
   Eina_Bool unused : 1;
#ifdef DEBUG_LOAD_TIME
   struct timeval rstart; // start of the glyphs load request
   struct timeval rfinish; // finish of the glyphs load request
   int gl_request_time; // time spent to process glyph requests
   int gl_load_time;
   int gl_render_time;
   int gl_slave_time;
   int gl_saved_time;
#endif
};

struct _Font_Cache {
   EINA_INLIST;
   Font_Entry *fe;
   Shm_Handle *shm;
   unsigned int usage;
   int inuse;
   Eina_Inlist *glyphs;
   unsigned int nglyphs;
};

struct _Glyph_Entry {
   EINA_INLIST;
   Font_Entry *fe;
   Font_Cache *fc;
   unsigned int index;
   unsigned int offset;
   unsigned int size;
   unsigned int rows;
   unsigned int width;
   unsigned int pitch;
   unsigned int num_grays;
   unsigned int pixel_mode;
};

struct _Glyphs_Request {
   Client *client;
   Font_Entry *fe;
   unsigned int current;
   unsigned int nglyphs;
   unsigned int *glyphs;
   unsigned int nrender;
   unsigned int *render;
   unsigned int nanswer;
   Glyph_Entry **answer;
   unsigned int hint;
};

typedef struct _Glyphs_Request Glyphs_Request;

struct _Glyphs_Group {
   Font_Cache *fc;
   Eina_List *glyphs;
};

typedef struct _Glyphs_Group Glyphs_Group;

struct _Reference {
   Client *client;
   Entry *entry;
   unsigned int client_entry_id; // for reverse lookup
   int count;
};

struct _File_Watch {
   const char *path;
   Eina_List *entries;
};

static unsigned int _entry_id = 0;
static Eina_Hash *file_ids = NULL; // maps path + key --> file_id
static Eina_Hash *file_entries = NULL; // maps file_id --> entry

static Eina_Hash *image_ids = NULL; // maps file id + load opts --> image id
static Eina_Hash *image_entries = NULL; // maps image_id --> entry

static Eina_Hash *font_sources = NULL; // font path --> font source
static Eina_Hash *font_entries = NULL; // maps font path + options --> entry

static Eina_Hash *file_watch = NULL;

static Eina_List *image_entries_lru = NULL;

static Eina_List *font_shm_lru = NULL;

static int max_unused_mem_usage = 5 * 4 * 1024; /* in kbytes */
static int unused_mem_usage = 0;
static int max_font_usage = 10 * 4 * 1024; /* in kbytes */
static int font_mem_usage = 0;

#ifdef DEBUG_LOAD_TIME
static int
_timeval_sub(const struct timeval *tv2, const struct timeval *tv1)
{
    int t1, t2;

    t1 = tv1->tv_usec + tv1->tv_sec * 1000000;
    t2 = tv2->tv_usec + tv2->tv_sec * 1000000;

    // Make sure that we don't add negative values. Some images may have
    // been not loaded yet, so it would mess with the stats.
    if (t2 > t1)
      return t2 - t1;

    return 0;
}
#endif

static inline void
_entry_load_start(Entry *e)
{
#ifdef DEBUG_LOAD_TIME
   gettimeofday(&e->load_start, NULL);
#endif
}

static inline void
_entry_load_finish(Entry *e)
{
#ifdef DEBUG_LOAD_TIME
   gettimeofday(&e->load_finish, NULL);
   e->load_time = _timeval_sub(&e->load_finish, &e->load_start);
#endif
}

static inline void
_entry_load_reused(Entry *e)
{
#ifdef DEBUG_LOAD_TIME
   e->saved_time += e->load_time;
#endif
}

static Msg_Opened *
_image_opened_msg_create(File_Data *fe, int *size)
{
   Msg_Opened *msg;

   msg = calloc(1, sizeof(*msg));
   msg->base.type = CSERVE2_OPENED;
   msg->image.w = fe->w;
   msg->image.h = fe->h;
   msg->image.frame_count = fe->frame_count;
   msg->image.loop_count = fe->loop_count;
   msg->image.loop_hint = fe->loop_hint;
   msg->image.alpha = fe->alpha;

   *size = sizeof(*msg);

   return msg;
}

static void
_image_opened_send(Client *client, File_Data *fe, unsigned int rid)
{
    int size;
    Msg_Opened *msg;

    DBG("Sending OPENED reply for entry: %d and RID: %d.", fe->base.id, rid);
    // clear the struct with possible paddings, since it is not aligned.

    msg = _image_opened_msg_create(fe, &size);
    msg->base.rid = rid;

    cserve2_client_send(client, &size, sizeof(size));
    cserve2_client_send(client, msg, size);

    free(msg);
}

static Msg_Loaded *
_image_loaded_msg_create(Image_Data *entry, int *size)
{
   Msg_Loaded *msg;
   const char *shmpath = cserve2_shm_name_get(entry->shm);
   int path_len;
   char *buf;

   path_len = strlen(shmpath) + 1;

   *size = sizeof(*msg) + path_len;
   msg = calloc(1, *size);
   msg->base.type = CSERVE2_LOADED;

   msg->shm.mmap_offset = cserve2_shm_map_offset_get(entry->shm);
   msg->shm.use_offset = cserve2_shm_offset_get(entry->shm);
   msg->shm.mmap_size = cserve2_shm_map_size_get(entry->shm);
   msg->shm.image_size = cserve2_shm_size_get(entry->shm);
   msg->alpha_sparse = entry->alpha_sparse;

   buf = (char *)msg + sizeof(*msg);
   memcpy(buf, shmpath, path_len);

   return msg;
}

static void
_image_loaded_send(Client *client, Image_Data *entry, unsigned int rid)
{
   int size;
   Msg_Loaded *msg;

   DBG("Sending LOADED reply for entry %d and RID: %d.", entry->base.id, rid);

   msg = _image_loaded_msg_create(entry, &size);
   msg->base.rid = rid;

   cserve2_client_send(client, &size, sizeof(size));
   cserve2_client_send(client, msg, size);

   free(msg);
}

static void
_font_loaded_send(Client *client, unsigned int rid)
{
   int size;
   Msg_Font_Loaded msg;

   DBG("Sending FONT_LOADED reply for RID: %d.", rid);

   size = sizeof(msg);
   memset(&msg, 0, size);
   msg.base.rid = rid;
   msg.base.type = CSERVE2_FONT_LOADED;

   size = sizeof(msg);
   cserve2_client_send(client, &size, sizeof(size));
   cserve2_client_send(client, &msg, size);
}

static void *
_open_request_build(File_Data *fe, int *bufsize)
{
   const char *loader_data;
   char *buf;
   int size, pathlen, keylen, loaderlen;
   Slave_Msg_Image_Open msg;

   pathlen = strlen(cserve2_shared_string_get(fe->path)) + 1;
   keylen = strlen(cserve2_shared_string_get(fe->key)) + 1;

   memset(&msg, 0, sizeof(msg));
   loader_data = cserve2_shared_string_get(fe->loader_data);
   msg.has_loader_data = !!loader_data;
   loaderlen = msg.has_loader_data ? (strlen(loader_data) + 1) : 0;

   size = sizeof(msg) + pathlen + keylen + loaderlen;
   buf = malloc(size);
   if (!buf) return NULL;

   memcpy(buf, &msg, sizeof(msg));
   memcpy(buf + sizeof(msg), cserve2_shared_string_get(fe->path), pathlen);
   memcpy(buf + sizeof(msg) + pathlen, cserve2_shared_string_get(fe->key), keylen);
   if (msg.has_loader_data)
     memcpy(buf + sizeof(msg) + pathlen + keylen, loader_data, loaderlen);

   *bufsize = size;

   _entry_load_start(&fe->base);

   return buf;
}

static void
_request_free(void *msg, void *data EINA_UNUSED)
{
   free(msg);
}

static Msg_Opened *
_open_request_response(File_Data *fe, Slave_Msg_Image_Opened *resp, int *size)
{
   _entry_load_finish(&fe->base);

   fe->base.request = NULL;

   fe->w = resp->w;
   fe->h = resp->h;
   fe->frame_count = resp->frame_count;
   fe->loop_count = resp->loop_count;
   fe->loop_hint = resp->loop_hint;
   fe->alpha = resp->alpha;
   if (resp->has_loader_data)
     {
        const char *ldata =
              (const char *)resp + sizeof(Slave_Msg_Image_Opened);
        fe->loader_data = cserve2_shared_string_add(ldata);
     }

   return _image_opened_msg_create(fe, size);
}

static void
_request_failed(Entry *e, Error_Type type EINA_UNUSED)
{
   Eina_List *l, *l_next;
   Reference *ref;

   e->request = NULL;

   EINA_LIST_FOREACH_SAFE(e->references, l, l_next, ref)
     {
        Eina_Hash *hash = NULL;
        if (e->type == CSERVE2_IMAGE_FILE)
          hash = ref->client->files.referencing;
        else if (e->type == CSERVE2_IMAGE_DATA)
          hash = ref->client->images.referencing;
        else
          continue;

        if (type != CSERVE2_REQUEST_CANCEL)
          {
             DBG("removing entry %u from hash", ref->client_entry_id);
             eina_hash_del_by_key(hash, &(ref->client_entry_id));
          }
     }
}

static Slave_Request_Funcs _open_funcs = {
   .msg_create = (Slave_Request_Msg_Create)_open_request_build,
   .msg_free = _request_free,
   .response = (Slave_Request_Response)_open_request_response,
   .error = (Slave_Request_Error)_request_failed
};

static void *
_load_request_build(Image_Data *i, int *bufsize)
{
   char *buf, *ptr;
   const char *shmpath, *loader_data;
   int size;
   int shmlen, filelen, keylen, loaderlen;
   Slave_Msg_Image_Load msg;

   // opening shm for this file
   i->shm = cserve2_shm_request("img", i->file->w * i->file->h * 4);
   if (!i->shm)
     return NULL;

   shmpath = cserve2_shm_name_get(i->shm);

   shmlen = strlen(shmpath) + 1;
   filelen = strlen(cserve2_shared_string_get(i->file->path)) + 1;
   keylen = strlen(cserve2_shared_string_get(i->file->key)) + 1;
   loader_data = cserve2_shared_string_get(i->file->loader_data);
   if (loader_data)
     loaderlen = strlen(loader_data) + 1;
   else
     loaderlen = 0;

   size = sizeof(msg) + shmlen + filelen + keylen + loaderlen;
   buf = malloc(size);
   if (!buf) return NULL;

   memset(&msg, 0, sizeof(msg));
   msg.w = i->file->w;
   msg.h = i->file->h;
   msg.alpha = i->file->alpha;

   // NOTE: Not passing scale_load options
   msg.opts.w = i->opts.w;
   msg.opts.h = i->opts.h;
   msg.opts.region = i->opts.region;
   msg.opts.scale_down_by = i->opts.scale_down_by;
   msg.opts.dpi = i->opts.dpi;
   msg.opts.degree = i->opts.degree;
   msg.opts.orientation = i->opts.orientation;

   msg.shm.mmap_offset = cserve2_shm_map_offset_get(i->shm);
   msg.shm.image_offset = cserve2_shm_offset_get(i->shm);
   msg.shm.mmap_size = cserve2_shm_map_size_get(i->shm);
   msg.shm.image_size = cserve2_shm_size_get(i->shm);

   msg.has_loader_data = !!loaderlen;

   memcpy(buf, &msg, sizeof(msg));
   ptr = buf + sizeof(msg);

   memcpy(ptr, shmpath, shmlen);
   ptr += shmlen;
   memcpy(ptr, cserve2_shared_string_get(i->file->path), filelen);
   ptr += filelen;
   memcpy(ptr, cserve2_shared_string_get(i->file->key), keylen);
   ptr += keylen;
   if (loaderlen > 0) memcpy(ptr, cserve2_shared_string_get(i->file->loader_data), loaderlen);

   *bufsize = size;

   _entry_load_start(&i->base);

   return buf;
}

static inline Eina_Bool
_scaling_needed(Image_Data *entry, Slave_Msg_Image_Loaded *resp)
{
   return (((entry->opts.scale_load.dst_w) && (entry->opts.scale_load.dst_h)) &&
           ((entry->opts.scale_load.dst_w != resp->w) ||
            (entry->opts.scale_load.dst_h != resp->h)));
}

static int
_scaling_do(Shm_Handle *scale_shm, Image_Data *entry, Image_Data *original)
{
   char *scale_map, *orig_map;
   void *src_data, *dst_data;

   scale_map = cserve2_shm_map(scale_shm);
   if (scale_map == MAP_FAILED)
     {
        ERR("Failed to memory map file for scale image.");
        return -1;
     }

   orig_map = cserve2_shm_map(original->shm);
   if (orig_map == MAP_FAILED)
     {
        ERR("Failed to memory map file for original image.");

        cserve2_shm_unmap(scale_shm);
        return -1;
     }

   src_data = orig_map + cserve2_shm_map_offset_get(original->shm);
   dst_data = scale_map + cserve2_shm_map_offset_get(scale_shm);

   DBG("Scaling image ([%d,%d:%dx%d] --> [%d,%d:%dx%d])",
       entry->opts.scale_load.src_x, entry->opts.scale_load.src_y,
       entry->opts.scale_load.src_w, entry->opts.scale_load.src_h,
       0, 0,
       entry->opts.scale_load.dst_w, entry->opts.scale_load.dst_h);

   cserve2_rgba_image_scale_do(src_data, dst_data,
                               entry->opts.scale_load.src_x, entry->opts.scale_load.src_y,
                               entry->opts.scale_load.src_w, entry->opts.scale_load.src_h,
                               0, 0,
                               entry->opts.scale_load.dst_w, entry->opts.scale_load.dst_h,
                               entry->file->alpha, entry->opts.scale_load.smooth);

   cserve2_shm_unmap(original->shm);
   cserve2_shm_unmap(scale_shm);

   return 0;
}

static int
_scaling_prepare_and_do(Image_Data *orig)
{
   Shm_Handle *scale_shm;

   DBG("Original image's shm path %s", cserve2_shm_name_get(orig->shm));

   scale_shm = cserve2_shm_request(
            "img",
            orig->opts.scale_load.dst_w * orig->opts.scale_load.dst_h * 4);

   DBG("Scale image's shm path %s", cserve2_shm_name_get(scale_shm));

   if (_scaling_do(scale_shm, orig, orig)) return -1;

   cserve2_shm_unref(orig->shm); /* unreference old shm */
   orig->shm = scale_shm; /* update shm */

   return 0;
}

static Msg_Loaded *
_load_request_response(Image_Data *e, Slave_Msg_Image_Loaded *resp, int *size)
{
   const char *path = cserve2_shared_string_get(e->file->path);
   const char *key = cserve2_shared_string_get(e->file->key);

   _entry_load_finish(&e->base);

   e->base.request = NULL;

   e->alpha_sparse = resp->alpha_sparse;
   if (!e->doload)
     DBG("Entry %d loaded by speculative preload.", e->base.id);

   if (_scaling_needed(e, resp))
     {

        DBG("About to scale down image '%s%s'", path, key);

        if (!_scaling_prepare_and_do(e))
          DBG("Image '%s:%s' has been scaled down.",
              path, key);
        else
          ERR("Failed to scale down image '%s%s'",
              path, key);
     }
   else
     DBG("No scaling needed for image '%s%s'", path, key);

   return _image_loaded_msg_create(e, size);
}

static Slave_Request_Funcs _load_funcs = {
   .msg_create = (Slave_Request_Msg_Create)_load_request_build,
   .msg_free = _request_free,
   .response = (Slave_Request_Response)_load_request_response,
   .error = (Slave_Request_Error)_request_failed
};

static unsigned int
_img_opts_id_get(unsigned int file_id, Evas_Image_Load_Opts *opts,
                 char *buf, int size)
{
   uintptr_t image_id;

   snprintf(buf, size,
            "%u:%0.3f:%dx%d:%d:%d,%d+%dx%d:!([%d,%d:%dx%d]-[%dx%d:%d]):%d:%d",
            file_id, opts->dpi, opts->w, opts->h,
            opts->scale_down_by, opts->region.x, opts->region.y,
            opts->region.w, opts->region.h,
            opts->scale_load.src_x, opts->scale_load.src_y,
            opts->scale_load.src_w, opts->scale_load.src_h,
            opts->scale_load.dst_w, opts->scale_load.dst_h,
            opts->scale_load.smooth, opts->degree,
            opts->orientation);

   image_id = (uintptr_t)eina_hash_find(image_ids, buf);

   return image_id;
}

static unsigned int
_image_data_opts_id_get(Image_Data *im, char *buf, int size)
{
   return _img_opts_id_get(im->file_id, &im->opts, buf, size);
}

static int
_image_entry_size_get(Image_Data *e)
{
   int size = sizeof(Image_Data);
   /* XXX: get the overhead of the shm handler too */
   if (e->shm)
     size += cserve2_shm_size_get(e->shm);
   return size / 1024;
}

static void
_file_id_free(File_Data *fe)
{
   char buf[4096];

   DBG("Removing entry file id: %d, file: \"%s:%s\"",
       fe->base.id, cserve2_shared_string_get(fe->path), cserve2_shared_string_get(fe->key));
   snprintf(buf, sizeof(buf), "%s:%s",
            cserve2_shared_string_get(fe->path), cserve2_shared_string_get(fe->key));
   eina_hash_del_by_key(file_ids, buf);
}

static void
_image_id_free(Image_Data *entry)
{
   char buf[4096];

   DBG("Removing entry image id: %d", entry->base.id);

   _image_data_opts_id_get(entry, buf, sizeof(buf));
   eina_hash_del_by_key(image_ids, buf);
}

static void
_image_entry_free(Image_Data *entry)
{
   File_Data *fe = entry->file;

   if (entry->base.request)
     cserve2_request_cancel_all(entry->base.request, CSERVE2_REQUEST_CANCEL);

   if (entry->unused)
     {
        image_entries_lru = eina_list_remove(image_entries_lru, entry);
        unused_mem_usage -= _image_entry_size_get(entry);
     }

   if (fe)
     {
        fe->images = eina_list_remove(fe->images, entry);
        if (!fe->images && !fe->base.references)
          eina_hash_del_by_key(file_entries, &fe->base.id);
     }
   if (entry->shm)
     cserve2_shm_unref(entry->shm);
   free(entry);
}

static void
_hash_image_entry_free(void *data)
{
   Image_Data *entry = data;

   _image_id_free(entry);
   _image_entry_free(entry);
}

static void
_file_entry_free(File_Data *fe)
{
   File_Watch *fw;

   // Should we call free for each of the images too?
   // If everything goes fine, it's not necessary.
   if (fe->images)
     {
        ERR("Freeing file %d (\"%s:%s\") image data still referenced.",
            fe->base.id, cserve2_shared_string_get(fe->path), cserve2_shared_string_get(fe->key));
        eina_list_free(fe->images);
     }

   if (fe->base.request)
     cserve2_request_cancel_all(fe->base.request, CSERVE2_REQUEST_CANCEL);

   if ((fw = fe->watcher))
     {
        fw->entries = eina_list_remove(fw->entries, fe);
        if (!fw->entries)
          eina_hash_del_by_key(file_watch, fw->path);
     }

   cserve2_shared_string_del(fe->key);
   cserve2_shared_string_del(fe->path);
   cserve2_shared_string_del(fe->loader_data);
   free(fe);
}

static void
_hash_file_entry_free(void *data)
{
   File_Data *fe = data;
   // TODO: Add some checks to make sure that we are freeing an
   // unused entry.

   _file_id_free(fe);
   _file_entry_free(fe);
}

static void
_file_watch_free(void *data)
{
   File_Watch *fw = data;
   cserve2_file_change_watch_del(fw->path);
   eina_stringshare_del(fw->path);
   eina_list_free(fw->entries);
   free(fw);
}

static int
_font_entry_cmp(const Font_Entry *k1, int k1_length EINA_UNUSED,
                const Font_Entry *k2, int k2_length EINA_UNUSED)
{
   const char *key1, *key2;
   if (k1->src->key == k2->src->key)
     {
        if (k1->size == k2->size)
          {
             if (k1->rend_flags == k2->rend_flags)
               return k1->dpi - k2->dpi;
             return k1->rend_flags - k2->rend_flags;
          }
        return k1->size - k2->size;
     }
   key1 = cserve2_shared_string_get(k1->src->key);
   key2 = cserve2_shared_string_get(k2->src->key);
   return strcmp(key1, key2);
}

static int
_font_entry_key_hash(const Font_Entry *key, int key_length EINA_UNUSED)
{
   const char *keystr;
   int hash;   

   keystr = cserve2_shared_string_get(key->src->key);
   hash = eina_hash_djb2(keystr, strlen(keystr));
   hash ^= eina_hash_int32(&key->rend_flags, sizeof(int));
   hash ^= eina_hash_int32(&key->size, sizeof(int));
   hash ^= eina_hash_int32(&key->dpi, sizeof(int));

   return hash;
}

static void
_font_entry_free(Font_Entry *fe)
{
   fash_gl_free(fe->glyphs);
   fe->src->references--;
   if (fe->ft) cserve2_font_ft_free(fe->ft);
   if (fe->src->references <= 0)
     {
        const char *key = cserve2_shared_string_get(fe->src->key);
        eina_hash_del_by_key(font_sources, key);
     }
   free(fe);
}

static void
_glyph_free_cb(void *data)
{
   Glyph_Entry *gl = data;
   free(gl);
}

static void
_font_source_free(Font_Source *fs)
{
   cserve2_shared_string_del(fs->key);
   cserve2_shared_string_del(fs->name);
   cserve2_shared_string_del(fs->file);
   if (fs->ft) cserve2_font_source_ft_free(fs->ft);

   free(fs);
}

static void
_font_shm_promote(Font_Cache *fc)
{
   Eina_List *l;
   l = eina_list_data_find_list(font_shm_lru, fc);
   font_shm_lru = eina_list_demote_list(font_shm_lru, l);
}

static int
_font_shm_size_get(Font_Cache *fc)
{
   int size;

   size = sizeof(*fc) + cserve2_shm_size_get(fc->shm);

   return size;
}

static void
_font_shm_free(Font_Cache *fc)
{
   Font_Entry *fe = fc->fe;
   fe->caches = eina_inlist_remove(fe->caches, EINA_INLIST_GET(fc));
   if (fc == fe->last_cache)
     fe->last_cache = NULL;

   while (fc->glyphs)
     {
        Glyph_Entry *gl = EINA_INLIST_CONTAINER_GET(fc->glyphs, Glyph_Entry);
        fc->glyphs = eina_inlist_remove(fc->glyphs, fc->glyphs);
        fash_gl_del(fe->glyphs, gl->index);
     }

   cserve2_shm_unref(fc->shm);
   free(fc);

   if (!fe->caches)
     eina_hash_del_by_key(font_entries, fe);
}

static void
_font_shm_lru_flush(void)
{
   Eina_List *l, *l_next;

   l = font_shm_lru;
   l_next = eina_list_next(l);

   while (l && font_mem_usage > max_font_usage)
     {
        int size;
        Font_Cache *fc;

        fc = eina_list_data_get(l);
        if (fc->fe->unused && fc->inuse == 0)
          {
             font_shm_lru = eina_list_remove_list(font_shm_lru, l);
             size = _font_shm_size_get(fc);
             size += fc->nglyphs * sizeof(Glyph_Entry);
             _font_shm_free(fc);
             font_mem_usage -= size;
          }

        l = l_next;
        l_next = eina_list_next(l);
     }
}

void
cserve2_cache_init(void)
{
   file_ids = eina_hash_string_superfast_new(NULL);
   file_entries = eina_hash_int32_new(_hash_file_entry_free);
   image_ids = eina_hash_string_superfast_new(NULL);
   image_entries = eina_hash_string_superfast_new(_hash_image_entry_free);
   file_watch = eina_hash_string_superfast_new(_file_watch_free);

   font_sources = eina_hash_string_small_new(EINA_FREE_CB(_font_source_free));
   font_entries = eina_hash_new(NULL,
                                EINA_KEY_CMP(_font_entry_cmp),
                                EINA_KEY_HASH(_font_entry_key_hash),
                                EINA_FREE_CB(_font_entry_free),
                                5);
}

void
cserve2_cache_shutdown(void)
{
   Font_Cache *fc;

   EINA_LIST_FREE(font_shm_lru, fc)
     _font_shm_free(fc);

   eina_hash_free(image_entries);
   eina_hash_free(image_ids);
   eina_hash_free(file_entries);
   eina_hash_free(file_ids);
   eina_hash_free(file_watch);

   eina_hash_free(font_entries);
   eina_hash_free(font_sources);
}

static Reference *
_entry_reference_add(Entry *entry, Client *client, unsigned int client_entry_id)
{
   Reference *ref;

   // increase reference for this file
   ref = malloc(sizeof(*ref));
   ref->client = client;
   ref->entry = entry;
   ref->client_entry_id = client_entry_id;
   ref->count = 1;
   entry->references = eina_list_append(entry->references, ref);

   return ref;
}

static void
_entry_unused_push(Image_Data *e)
{
   int size = _image_entry_size_get(e);

   if ((size > max_unused_mem_usage) || !(e->doload))
     {
        eina_hash_del_by_key(image_entries, &e->base.id);
        return;
     }
   while (image_entries_lru &&
          (size > (max_unused_mem_usage - unused_mem_usage)))
     {
        Entry *ie = eina_list_data_get(eina_list_last(image_entries_lru));
        Eina_Bool ok = eina_hash_del_by_key(image_entries, &ie->id);
        if (!ok)
          {
             DBG("Image %d was not found in the hash table!", ie->id);
             image_entries_lru = eina_list_remove(image_entries_lru, ie);
             _image_entry_free((Image_Data*) ie);
          }
     }
   if (!image_entries_lru && (unused_mem_usage != 0))
     {
        DBG("Invalid accounting of LRU size (was empty but size: %d)",
            unused_mem_usage);
        unused_mem_usage = 0;
     }
   image_entries_lru = eina_list_append(image_entries_lru, e);
   e->unused = EINA_TRUE;
   unused_mem_usage += size;
}

static void
_entry_reference_del(Entry *entry, Reference *ref)
{
   entry->references = eina_list_remove(entry->references, ref);

   if (entry->references)
     goto free_ref;

   if (entry->type == CSERVE2_IMAGE_FILE)
     {
        File_Data *fe = (File_Data *)entry;

        if (fe->invalid)
          _file_entry_free(fe);
        else if (!fe->images)
          eina_hash_del_by_key(file_entries, &entry->id);
        /* don't free file entries that have images attached to it, they will
         * be freed when the last unused image is freed */
     }
   else if (entry->type == CSERVE2_IMAGE_DATA)
     {
        Image_Data *ientry = (Image_Data *)entry;

        if (!ientry->file)
          eina_hash_del_by_key(image_entries, &entry->id);
        else if (ientry->file->invalid)
          _image_entry_free(ientry);
        else
          _entry_unused_push(ientry);
     }
   else if (entry->type == CSERVE2_FONT_ENTRY)
     {
        Font_Entry *fe = (Font_Entry *)entry;
        fe->unused = EINA_TRUE;
        if (!fe->caches)
          eina_hash_del_by_key(font_entries, fe);
     }
   else
     ERR("Wrong type of entry.");

free_ref:
   free(ref);
}

static void
_entry_free_cb(void *data)
{
   Reference *ref = data;
   Entry *entry;

   DBG("Removing client reference for entry id: %d, client: %d",
       ref->entry->id, ref->client->id);

   entry = ref->entry;

   /* XXX: handle the case of requests being processed gracefully */
   if (entry->request /*&& !entry->request->processing*/)
     {
        if (entry->type == CSERVE2_IMAGE_FILE)
          cserve2_request_cancel(entry->request, ref->client,
                                 CSERVE2_REQUEST_CANCEL);
        else if (entry->type == CSERVE2_IMAGE_DATA)
          cserve2_request_cancel(entry->request, ref->client,
                                 CSERVE2_REQUEST_CANCEL);
     }

   _entry_reference_del(entry, ref);
}

static void
_font_entry_reference_del(Client *client, Font_Entry *fe)
{
   Eina_List *l, *l_next;
   Reference *ref;

   EINA_LIST_FOREACH_SAFE(client->fonts.referencing, l, l_next, ref)
     {
        if (ref->entry == (Entry *)fe)
          {
             ref->count--;
             if (ref->count > 0)
               break;

             client->fonts.referencing = eina_list_remove_list(
                 client->fonts.referencing, l);
             _entry_reference_del(&fe->base, ref);
             return;
          }
     }
}

void
cserve2_cache_client_new(Client *client)
{
   client->files.referencing = eina_hash_int32_new(_entry_free_cb);
   client->images.referencing = eina_hash_int32_new(_entry_free_cb);
   client->fonts.referencing = NULL;
}

void
cserve2_cache_client_del(Client *client)
{
   Reference *ref;

   // will call _entry_free_cb() for every entry
   eina_hash_free(client->images.referencing);
   // will call _entry_free_cb() for every entry
   eina_hash_free(client->files.referencing);

   EINA_LIST_FREE(client->fonts.referencing, ref)
     {
        _entry_reference_del(ref->entry, ref);
     }
}

static Image_Data *
_image_entry_new(Client *client, int rid,
                 unsigned int file_id, unsigned int image_id,
                 Evas_Image_Load_Opts *opts)
{
   Reference *ref;
   Image_Data *im_entry;

   ref = eina_hash_find(client->files.referencing, &file_id);
   if (!ref)
     {
        ERR("Couldn't find file id: %d, for image id: %d",
            file_id, image_id);
        cserve2_client_error_send(client, rid,
                                  CSERVE2_INVALID_CACHE);
        return NULL;
     }
   if (((File_Data *)ref->entry)->invalid)
     {
        cserve2_client_error_send(client, rid,
                                  CSERVE2_FILE_CHANGED);
        return NULL;
     }

   im_entry = calloc(1, sizeof(*im_entry));
   im_entry->base.type = CSERVE2_IMAGE_DATA;
   im_entry->file_id = ref->entry->id;
   im_entry->file = (File_Data *)ref->entry;
   if (opts)
     {
        im_entry->opts.dpi = opts->dpi;
        im_entry->opts.w = opts->w;
        im_entry->opts.h = opts->h;
        im_entry->opts.scale_down_by = opts->scale_down_by;
        im_entry->opts.region.x = opts->region.x;
        im_entry->opts.region.y = opts->region.y;
        im_entry->opts.region.w = opts->region.w;
        im_entry->opts.region.h = opts->region.h;
        im_entry->opts.scale_load.src_x = opts->scale_load.src_x;
        im_entry->opts.scale_load.src_y = opts->scale_load.src_y;
        im_entry->opts.scale_load.src_w = opts->scale_load.src_w;
        im_entry->opts.scale_load.src_h = opts->scale_load.src_h;
        im_entry->opts.scale_load.dst_w = opts->scale_load.dst_w;
        im_entry->opts.scale_load.dst_h = opts->scale_load.dst_h;
        im_entry->opts.scale_load.smooth = opts->scale_load.smooth;
        im_entry->opts.scale_load.scale_hint = opts->scale_load.scale_hint;
        im_entry->opts.degree = opts->degree;
        im_entry->opts.orientation = opts->orientation;
     }

   return im_entry;
}

static void
_file_changed_cb(const char *path EINA_UNUSED, Eina_Bool deleted EINA_UNUSED, void *data)
{
   File_Watch *fw = data;
   File_Data *fe;
   Eina_List *l;

   EINA_LIST_FOREACH(fw->entries, l, fe)
     {
        Eina_List *ll;
        Image_Data *ie;

        fe->invalid = EINA_TRUE;
        fe->watcher = NULL;

        EINA_LIST_FOREACH(fe->images, ll, ie)
          {
             _image_id_free(ie);
             eina_hash_set(image_entries, &ie->base.id, NULL);
             if (ie->base.request /*&& !ie->base.request->processing*/)
               cserve2_request_cancel_all(ie->base.request,
                                          CSERVE2_FILE_CHANGED);
             ie->base.request = NULL;
             if (ie->unused)
               _image_entry_free(ie);
          }

        _file_id_free(fe);
        eina_hash_set(file_entries, &fe->base.id, NULL);
        if (fe->base.request /*&& !fe->base.request->processing*/)
          cserve2_request_cancel_all(fe->base.request, CSERVE2_FILE_CHANGED);
        fe->base.request = NULL;
        if (!fe->images && !fe->base.references)
          _file_entry_free(fe);
     }

   eina_hash_del_by_key(file_watch, fw->path);
}

static Font_Source *
_cserve2_font_source_find(const char *name)
{
   Font_Source *fs;

   fs = eina_hash_find(font_sources, name);

   return fs;
}

static Font_Entry *
_cserve2_font_entry_find(const char *name, unsigned int size, unsigned int rend_flags, unsigned int dpi)
{
   Font_Entry tmp_fe;
   Font_Source tmp_fs;
   Font_Entry *fe;

   tmp_fs.key = cserve2_shared_string_add(name);
   tmp_fe.src = &tmp_fs;
   tmp_fe.size = size;
   tmp_fe.rend_flags = rend_flags;
   tmp_fe.dpi = dpi;

   fe = eina_hash_find(font_entries, &tmp_fe);
   cserve2_shared_string_del(tmp_fs.key);

   return fe;
}

static void *
_font_load_request_build(void *data, int *size)
{
   Font_Entry *fe = data;
   Slave_Msg_Font_Load *msg = calloc(1, sizeof(*msg));

   msg->ftdata1 = fe->src->ft;
   msg->ftdata2 = fe->ft;
   msg->rend_flags = fe->rend_flags;
   msg->size = fe->size;
   msg->dpi = fe->dpi;

#warning TODO Use shared index on client side
   msg->name = cserve2_shared_string_get(fe->src->name);
   msg->file = cserve2_shared_string_get(fe->src->file);

   *size = 0;

   _entry_load_start(&fe->base);

   return msg;
}

static void
_font_load_request_free(void *msg, void *data EINA_UNUSED)
{
   free(msg);
}

static Msg_Font_Loaded *
_font_load_request_response(Font_Entry *fe, Slave_Msg_Font_Loaded *msg, int *size)
{
   Msg_Font_Loaded *resp;

   if (!fe->src->ft)
     fe->src->ft = msg->ftdata1;

   if (!fe->ft)
     {
        fe->ft = msg->ftdata2;
        _entry_load_finish(&fe->base);
     }

   if (fe->base.request) fe->base.request = NULL;

   /* could be a function, but it's too basic and only used here */
   resp = calloc(1, sizeof(*resp));
   resp->base.type = CSERVE2_FONT_LOADED;
   *size = sizeof(*resp);

   return resp;
}

static void
_font_load_request_failed(Font_Entry *fe, Error_Type error EINA_UNUSED)
{
   Eina_List *l, *l_next;
   Reference *ref;

   if (fe->base.request) fe->base.request = NULL;

   EINA_LIST_FOREACH_SAFE(fe->base.references, l, l_next, ref)
     _font_entry_reference_del(ref->client, fe);
}

static Slave_Request_Funcs _font_load_funcs = {
   .msg_create = (Slave_Request_Msg_Create)_font_load_request_build,
   .msg_free = (Slave_Request_Msg_Free)_font_load_request_free,
   .response = (Slave_Request_Response)_font_load_request_response,
   .error = (Slave_Request_Error)_font_load_request_failed
};

static Eina_Bool
_glyphs_request_check(Glyphs_Request *req, Eina_Bool report_load)
{
   unsigned int i;
   Font_Entry *fe = req->fe;

   req->answer = malloc(sizeof(*req->answer) * req->nglyphs);
   req->nanswer = 0;

   for (i = req->current; i < req->nglyphs; i++)
     {
        Glyph_Entry *ge;
        ge = fash_gl_find(fe->glyphs, req->glyphs[i]);
        if (ge)
          {
             req->answer[req->nanswer++] = ge;
#ifdef DEBUG_LOAD_TIME
             // calculate average time saved when loading glyphs
             if (report_load)
               fe->gl_saved_time +=
                  (fe->gl_load_time / fe->nglyphs);
#endif
             ge->fc->inuse++;
          }
        else
          break;
     }

   req->current = i;

   // No glyphs need to be rendered.
   return (req->nanswer == req->nglyphs);
}

/* organize answer (cache1{gl1, gl2,}, cache2{gl3,gl4,gl5}, cache3{gl6})
 */
static Eina_List *
_glyphs_group_create(Glyphs_Request *req)
{
   Eina_List *groups = NULL;
   unsigned int i;

   for (i = 0; i < req->nanswer; i++)
     {
        Eina_List *l;
        Glyphs_Group *iter, *gg = NULL;
        Font_Cache *fc = req->answer[i]->fc;

        EINA_LIST_FOREACH(groups, l, iter)
          {
             if (iter->fc == fc)
               {
                  gg = iter;
                  break;
               }
          }

        if (!gg)
          {
             gg = calloc(1, sizeof(*gg));
             gg->fc = fc;
             groups = eina_list_append(groups, gg);
          }
        gg->glyphs = eina_list_append(gg->glyphs, req->answer[i]);
     }

   return groups;
}

static Msg_Font_Glyphs_Loaded *
_glyphs_loaded_msg_create(Glyphs_Request *req, int *resp_size)
{
   Msg_Font_Glyphs_Loaded msg;
   unsigned int size;
   Eina_List *ll, *answers = NULL;
   const char *shmname;
   unsigned int shmsize;
   unsigned int intsize;
   char *resp, *buf;
   Glyphs_Group *iter;

   memset(&msg, 0, sizeof(msg));
   msg.base.type = CSERVE2_FONT_GLYPHS_LOADED;

   answers = _glyphs_group_create(req);
   msg.ncaches = eina_list_count(answers);
   size = sizeof(msg);

   // calculate size of message
   // ncaches * sizeof(cache) + nglyphs1 * sizeof(glyph) +
   //   nglyphs2 * sizeof(glyph)...

   intsize = sizeof(unsigned int);

   EINA_LIST_FOREACH(answers, ll, iter)
     {
        shmname = cserve2_shm_name_get(iter->fc->shm);
        shmsize = eina_stringshare_strlen(shmname) + 1;
        // shm namelen + name
        size += intsize + shmsize;

        // nglyphs
        size += intsize;
        // nglyphs * (index + offset + size + rows + width + pitch +
        //            num_grays + pixel_mode)
        size += eina_list_count(iter->glyphs) * 8 * intsize;
     }

   resp = malloc(size);
   memcpy(resp, &msg, sizeof(msg));
   buf = resp + sizeof(msg);

   EINA_LIST_FREE(answers, iter)
     {
        Glyph_Entry *gl;
        unsigned int nglyphs;

        shmname = cserve2_shm_name_get(iter->fc->shm);
        shmsize = eina_stringshare_strlen(shmname) + 1;
        memcpy(buf, &shmsize, intsize);
        buf += intsize;
        memcpy(buf, shmname, shmsize);
        buf += shmsize;

        nglyphs = eina_list_count(iter->glyphs);
        memcpy(buf, &nglyphs, intsize);
        buf += intsize;

        iter->fc->inuse -= eina_list_count(iter->glyphs);

        EINA_LIST_FREE(iter->glyphs, gl)
          {
             memcpy(buf, &gl->index, intsize);
             buf += intsize;
             memcpy(buf, &gl->offset, intsize);
             buf += intsize;
             memcpy(buf, &gl->size, intsize);
             buf += intsize;
             memcpy(buf, &gl->rows, intsize);
             buf += intsize;
             memcpy(buf, &gl->width, intsize);
             buf += intsize;
             memcpy(buf, &gl->pitch, intsize);
             buf += intsize;
             memcpy(buf, &gl->num_grays, intsize);
             buf += intsize;
             memcpy(buf, &gl->pixel_mode, intsize);
             buf += intsize;
          }

        /* We are removing SHMs from the beginning of the list, so this
         * gives a higher priority to them */
        _font_shm_promote(iter->fc);
        eina_list_free(iter->glyphs);
        free(iter);
     }

   *resp_size = size;

   return (Msg_Font_Glyphs_Loaded *)resp;
}

static void
_glyphs_loaded_send(Glyphs_Request *req, unsigned int rid)
{
   Msg_Font_Glyphs_Loaded *resp;
   int size;

   resp = _glyphs_loaded_msg_create(req, &size);
   resp->base.rid = rid;

   cserve2_client_send(req->client, &size, sizeof(size));
   cserve2_client_send(req->client, resp, size);

   free(resp);
}

/*
 * taken from evas_path.c. It would be good to clean up those utils to
 * have cserve link against them easily without dragging unneeded dependencies
 */
#ifdef _WIN32
# define EVAS_PATH_SEPARATOR "\\"
#else
# define EVAS_PATH_SEPARATOR "/"
#endif

static char *
_file_path_join(const char *path, const char *end)
{
   char *res = NULL;
   size_t len;

   if ((!path) && (!end)) return NULL;
   if (!path) return strdup(end);
   if (!end) return strdup(path);
   len = strlen(path);
   len += strlen(end);
   len += strlen(EVAS_PATH_SEPARATOR);
   res = malloc(len + 1);
   if (!res) return NULL;
   strcpy(res, path);
   strcat(res, EVAS_PATH_SEPARATOR);
   strcat(res, end);
   return res;
}

static Glyphs_Request *
_glyphs_request_create(Client *client, const char *source, const char *name, unsigned int hint, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int *glyphs, unsigned int nglyphs)
{
   char *fullname;
   Glyphs_Request *req = calloc(1, sizeof(*req));

   if (source && !*source)
     source = NULL;
   if (name && !*name)
     name = NULL;

   fullname = _file_path_join(source, name);
   req->fe = _cserve2_font_entry_find(fullname, size, rend_flags, dpi);
   free(fullname);
   if (!req->fe)
     {
        ERR("No font entry found: source %s, name %s, rendflags: %d, hint: %d,"
            " size: %d, dpi: %d", source, name, rend_flags, hint, size, dpi);
        free(req);
        return NULL;
     }

   req->client = client;

   req->nglyphs = nglyphs;
   req->current = 0;
   req->glyphs = glyphs;
   req->hint = hint;

   return req;
}

static void
_glyphs_request_free(Glyphs_Request *req)
{
   free(req->glyphs);
   free(req->render);
   free(req->answer);
   free(req);
}

/* add glyphs that are already in cache to the "answers" array, and the ones
 * that are not cached to the "render" array.
 */
static void
_glyphs_load_request_prepare(Glyphs_Request *req)
{
   unsigned int i, max;
   req->nrender = 0;
   Font_Entry *fe = req->fe;

   if (!fe)
     {
        ERR("No font entry for this request.");
        return;
     }

   // Won't render more than this number of glyphs
   max = req->nglyphs - req->nanswer;
   req->render = malloc(sizeof(*req->render) * max);

   for (i = req->current; i < req->nglyphs; i++)
     {
        Glyph_Entry *ge;
        ge = fash_gl_find(fe->glyphs, req->glyphs[i]);
        if (ge)
          {
             req->answer[req->nanswer++] = ge;

#ifdef DEBUG_LOAD_TIME
             // calculate average time saved when loading glyphs
             fe->gl_saved_time +=
                (fe->gl_load_time / fe->nglyphs);
#endif
             ge->fc->inuse++;
          }
        else
          req->render[req->nrender++] = req->glyphs[i];
     }
}

static void *
_glyphs_load_request_build(void *data, int *size EINA_UNUSED)
{
   Glyphs_Request *req = data;
   Slave_Msg_Font_Glyphs_Load *msg = NULL;
   Font_Entry *fe = req->fe;
   Font_Cache *fc;

#ifdef DEBUG_LOAD_TIME
   gettimeofday(&fe->rstart, NULL);
   _glyphs_load_request_prepare(req);
#endif

   msg = calloc(1, sizeof(*msg));

   msg->font.ftdata1 = fe->src->ft;
   msg->font.ftdata2 = fe->ft;
   msg->font.hint = req->hint;
   msg->font.rend_flags = fe->rend_flags;
   msg->glyphs.nglyphs = req->nrender;
   msg->glyphs.glyphs = req->render;

   // Trying to reuse last filled cache.
   fc = fe->last_cache;
   if (fc)
     {
        msg->cache.shm = fc->shm;
        msg->cache.usage = fc->usage;
        msg->cache.nglyphs = fc->nglyphs;
     }

   return msg;
}

static void
_glyphs_load_request_free(void *msg, void *data)
{
   _glyphs_request_free(data);
   free(msg);
}

static Msg_Font_Glyphs_Loaded *
_glyphs_load_request_response(Glyphs_Request *req, Slave_Msg_Font_Glyphs_Loaded *msg, int *size)
{
   Font_Entry *fe = req->fe;
   Font_Cache *fc = NULL;
   unsigned int i = 0;

   if (fe->last_cache && fe->last_cache->shm == msg->caches[0]->shm)
     fc = fe->last_cache;

   while (i < msg->ncaches)
     {
        unsigned int j;
        Slave_Msg_Font_Cache *c = msg->caches[i++];

        if (!fc)
          {
             fc = calloc(1, sizeof(*fc));
             fe->caches = eina_inlist_append(fe->caches, EINA_INLIST_GET(fc));
             fe->last_cache = fc;
             fc->fe = fe;
             fc->shm = c->shm;
             fc->glyphs = NULL;
             fc->nglyphs = 0;
             fc->inuse = 0;
             font_shm_lru = eina_list_append(font_shm_lru, fc);
             font_mem_usage += _font_shm_size_get(fc);
          }
        fc->usage = c->usage;
        for (j = 0; j < c->nglyphs; j++)
          {
             Glyph_Entry *gl;

             gl = fash_gl_find(fe->glyphs, c->glyphs[j].index);
             if (!gl)
               {
                  gl = calloc(1, sizeof(*gl));
                  gl->fe = fe;
                  gl->fc = fc;
                  gl->index = c->glyphs[j].index;
                  gl->offset = c->glyphs[j].offset;
                  gl->size = c->glyphs[j].size;
                  gl->rows = c->glyphs[j].rows;
                  gl->width = c->glyphs[j].width;
                  gl->pitch = c->glyphs[j].pitch;
                  gl->num_grays = c->glyphs[j].num_grays;
                  gl->pixel_mode = c->glyphs[j].pixel_mode;
                  font_mem_usage += sizeof(*gl);
                  fc->glyphs = eina_inlist_append(fc->glyphs, EINA_INLIST_GET(gl));
                  fc->nglyphs++;
                  fe->nglyphs++;
                  fash_gl_add(fe->glyphs, gl->index, gl);
               }
             req->answer[req->nanswer++] = gl;
             gl->fc->inuse++;
          }

        free(c); // FIXME: We are freeing this here because we only do a
                 // simple free on the response message. Later we need to
                 // setup a free callback for the slave response.
        fc = NULL;
     }

#ifdef DEBUG_LOAD_TIME
   gettimeofday(&fe->rfinish, NULL);
   fe->gl_request_time += _timeval_sub(&fe->rfinish, &fe->rstart);
   fe->gl_load_time += msg->gl_load_time;
   fe->gl_render_time += msg->gl_render_time;
   fe->gl_slave_time += msg->gl_slave_time;
#endif

   _font_shm_lru_flush();

   return _glyphs_loaded_msg_create(req, size);
}

static void
_glyphs_load_request_failed(void *data EINA_UNUSED, Error_Type error EINA_UNUSED)
{
}

static Slave_Request_Funcs _glyphs_load_funcs = {
   .msg_create = (Slave_Request_Msg_Create)_glyphs_load_request_build,
   .msg_free = (Slave_Request_Msg_Free)_glyphs_load_request_free,
   .response = (Slave_Request_Response)_glyphs_load_request_response,
   .error = (Slave_Request_Error)_glyphs_load_request_failed
};

static Eina_Bool
_font_entry_stats_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Font_Entry *fe = data;
   Msg_Stats *msg = fdata;
   Font_Cache *fc;
   int nrefs = eina_list_count(fe->base.references);

   msg->fonts.fonts_loaded++;
   if (fe->unused) msg->fonts.fonts_unused++;

   // accounting size
   EINA_INLIST_FOREACH(fe->caches, fc)
     {
        unsigned int fc_usage, shmsize;
        /* This is not real requested usage, but an approximation. We don't
         * know how many times each glyph would be used by each client, but
         * assume that a similar set of glyphs from a given font would be used
         * by each client, thus counting them one time per client referencing
         * them.
         */
        fc_usage = fc->usage * nrefs;
        shmsize = cserve2_shm_size_get(fc->shm);

        msg->fonts.requested_size += fc_usage;
        msg->fonts.real_size += shmsize;
        if (fe->unused) msg->fonts.unused_size += shmsize;
     }

#ifdef DEBUG_LOAD_TIME
   // accounting fonts load time
   msg->fonts.fonts_load_time += fe->base.load_time;
   if (fe->caches)
     {
        msg->fonts.fonts_used_load_time += fe->base.load_time;
        msg->fonts.fonts_used_saved_time += fe->base.saved_time;
     }

   // accounting glyphs load time
   msg->fonts.glyphs_load_time += fe->gl_load_time;
   msg->fonts.glyphs_render_time += fe->gl_render_time;
   msg->fonts.glyphs_saved_time += fe->gl_saved_time;
   msg->fonts.glyphs_request_time += fe->gl_request_time;
   msg->fonts.glyphs_slave_time += fe->gl_slave_time;
#endif

   return EINA_TRUE;
}

static Eina_Bool
_image_file_entry_stats_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Msg_Stats *msg = fdata;
   File_Data *fe = data;

   // accounting numbers
   msg->images.files_loaded++;

   // accounting size
   msg->images.files_size += sizeof(File_Data) +
      eina_list_count(fe->images) * sizeof(Eina_List *) +
      eina_list_count(fe->base.references) *
         (sizeof(Slave_Request *) + sizeof(Eina_List *));

#ifdef DEBUG_LOAD_TIME
   // accounting file entries load time
   msg->images.files_load_time += fe->base.load_time;
   msg->images.files_saved_time += fe->base.saved_time;
#endif

   return EINA_TRUE;
}

static Eina_Bool
_image_data_entry_stats_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Msg_Stats *msg = fdata;
   Image_Data *id = data;
   unsigned int image_size;

   // accounting numbers
   msg->images.images_loaded++;
   if (id->unused) msg->images.images_unused++;

   // accounting size
   msg->images.images_size += _image_entry_size_get(id) * 1024;
   if (id->unused) msg->images.unused_size += _image_entry_size_get(id) * 1024;

   image_size = id->file->w * id->file->h * 4;
   msg->images.requested_size +=
      (image_size * eina_list_count(id->base.references));

#ifdef DEBUG_LOAD_TIME
   // accounting image entries load time
   msg->images.images_load_time += id->base.load_time;
   msg->images.images_saved_time += id->base.saved_time;
#endif

   return EINA_TRUE;
}

static void
_cserve2_cache_image_stats_get(Msg_Stats *msg)
{
   eina_hash_foreach(file_entries, _image_file_entry_stats_cb, msg);
   eina_hash_foreach(image_entries, _image_data_entry_stats_cb, msg);
}

static void
_cserve2_cache_font_stats_get(Msg_Stats *msg)
{
   eina_hash_foreach(font_entries, _font_entry_stats_cb, msg);
}

struct _debug_info
{
   unsigned int size;
   unsigned int nfonts;
};

static Eina_Bool
_font_entry_debug_size_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   struct _debug_info *di = fdata;
   unsigned int size = di->size;
   Font_Entry *fe = data;
   Font_Cache *fc;
   unsigned int intsize = sizeof(unsigned int);
   const char *str;

   // filelen
   size += intsize;

   // file
   if (fe->src->file)
     {
        str = cserve2_shared_string_get(fe->src->file);
        size += strlen(str) + 1;
     }

   // namelen
   size += intsize;

   // name
   if (fe->src->name)
     {
        str = cserve2_shared_string_get(fe->src->file);
        size += strlen(str) + 1;
     }

   // rend_flags, size, dpi
   size += 3 * intsize;

   // unused
   size += intsize;

   // ncaches
   size += intsize;

   EINA_INLIST_FOREACH(fe->caches, fc)
     {
        Glyph_Entry *gl;

        // shmnamelen + shmname
        size += intsize;
        size += strlen(cserve2_shm_name_get(fc->shm)) + 1;

        // size + usage
        size += 2 * intsize;

        // nglyphs
        size += intsize;

        EINA_INLIST_FOREACH(fc->glyphs, gl)
          {
             // index, offset, size
             size += 3 * intsize;

             // rows, width, pitch
             size += 3 * intsize;

             // num_grays, pixel_mode
             size += 2 * intsize;
          }
     }

   di->size = size;
   di->nfonts++;

   return EINA_TRUE;
}

static Eina_Bool
_font_entry_debug_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   char **pos = fdata;
   char *buf = *pos;
   Font_Entry *fe = data;
   Font_Cache *fc;
   unsigned int len;
   unsigned int unused;
   unsigned int ncaches;
   unsigned int intsize = sizeof(unsigned int);
   const char *str;

   // filelen + file
   len = 0;
   if (fe->src->file)
     {
        str = cserve2_shared_string_get(fe->src->file);
        len = strlen(str) + 1;
     }
   memcpy(buf, &len, intsize);
   buf += intsize;
   if (len) memcpy(buf, cserve2_shared_string_get(fe->src->file), len);
   buf += len;

   // namelen + name
   len = 0;
   if (fe->src->name)
     {
        str = cserve2_shared_string_get(fe->src->name);
        len = strlen(str) + 1;
     }
   memcpy(buf, &len, intsize);
   buf += intsize;
   if (len) memcpy(buf, cserve2_shared_string_get(fe->src->name), len);
   buf += len;

   // rend_flags, size, dpi
   memcpy(buf, &fe->rend_flags, intsize);
   buf += intsize;
   memcpy(buf, &fe->size, intsize);
   buf += intsize;
   memcpy(buf, &fe->dpi, intsize);
   buf += intsize;

   // unused
   unused = fe->unused;
   memcpy(buf, &unused, intsize);
   buf += intsize;

   // ncaches
   ncaches = eina_inlist_count(fe->caches);
   memcpy(buf, &ncaches, intsize);
   buf += intsize;

   EINA_INLIST_FOREACH(fe->caches, fc)
     {
        Glyph_Entry *gl;
        const char *shmname;
        unsigned int shmsize;

        // shmnamelen + shmname
        shmname = cserve2_shm_name_get(fc->shm);
        len = strlen(shmname) + 1;
        memcpy(buf, &len, intsize);
        buf += intsize;
        memcpy(buf, shmname, len);
        buf += len;

        // size, usage, nglyphs
        shmsize = cserve2_shm_size_get(fc->shm);
        memcpy(buf, &shmsize, intsize);
        buf += intsize;
        memcpy(buf, &fc->usage, intsize);
        buf += intsize;
        memcpy(buf, &fc->nglyphs, intsize);
        buf += intsize;

        EINA_INLIST_FOREACH(fc->glyphs, gl)
          {
             // index, offset, size
             memcpy(buf, &gl->index, intsize);
             buf += intsize;
             memcpy(buf, &gl->offset, intsize);
             buf += intsize;
             memcpy(buf, &gl->size, intsize);
             buf += intsize;

             // rows, width, pitch
             memcpy(buf, &gl->rows, intsize);
             buf += intsize;
             memcpy(buf, &gl->width, intsize);
             buf += intsize;
             memcpy(buf, &gl->pitch, intsize);
             buf += intsize;

             // num_grays, pixel_mode
             memcpy(buf, &gl->num_grays, intsize);
             buf += intsize;
             memcpy(buf, &gl->pixel_mode, intsize);
             buf += intsize;
          }
     }

   *pos = buf;
   return EINA_TRUE;
}

static void *
_cserve2_cache_font_debug(unsigned int rid, unsigned int *size)
{
   Msg_Font_Debug msg;
   char *buf, *pos;
   struct _debug_info di;
   di.size = sizeof(msg);
   di.nfonts = 0;

   memset(&msg, 0, sizeof(msg));

   msg.base.type = CSERVE2_FONT_DEBUG;
   msg.base.rid = rid;

   // First calculate how much size is needed for this message:

   // nfonts
   di.size += sizeof(unsigned int);

   // size needed for each font entry
   eina_hash_foreach(font_entries, _font_entry_debug_size_cb, &di);

   // Now really create the message
   buf = malloc(di.size);
   pos = buf;

   // msg base
   memcpy(buf, &msg, sizeof(msg));
   pos += sizeof(msg);

   // nfonts
   memcpy(pos, &di.nfonts, sizeof(unsigned int));
   pos += sizeof(unsigned int);

   eina_hash_foreach(font_entries, _font_entry_debug_cb, &pos);

   *size = di.size;
   return buf;
}

int
cserve2_cache_file_open(Client *client, unsigned int client_file_id,
                        const char *path, const char *key, unsigned int rid)
{
   unsigned int file_id;
   File_Data *fe;
   Reference *ref;
   File_Watch *fw;
   char buf[4906];

   // look for this file on client references
   ref = eina_hash_find(client->files.referencing, &client_file_id);
   if (ref)
     {
        fe = (File_Data *)ref->entry;
        _entry_load_reused(ref->entry);

        if (fe->invalid)
          {
             cserve2_client_error_send(client, rid, CSERVE2_FILE_CHANGED);
             return -1;
          }

        DBG("found client file id: %d", client_file_id);
        ref->count++;

        // File already being loaded, just add the request to be replied
        if (fe->base.request)
          cserve2_request_waiter_add(fe->base.request, rid, client);
        else
          _image_opened_send(client, fe, rid);
        return 0;
     }

   // search whether the file is already opened by another client
   snprintf(buf, sizeof(buf), "%s:%s", path, key);
   file_id = (unsigned int)(uintptr_t)eina_hash_find(file_ids, buf);
   if (file_id)
     {
        DBG("found file_id %u for client file id %d",
            file_id, client_file_id);
        fe = eina_hash_find(file_entries, &file_id);
        if (!fe)
          {
             ERR("file \"%s\" is in file_ids hash but not in entries hash.",
                 buf);
             cserve2_client_error_send(client, rid, CSERVE2_INVALID_CACHE);
             // FIXME: Maybe we should remove the entry from file_ids then?
             return -1;
          }
        ref = _entry_reference_add((Entry *)fe, client, client_file_id);
        _entry_load_reused(ref->entry);
        eina_hash_add(client->files.referencing, &client_file_id, ref);
        if (fe->base.request)
          cserve2_request_waiter_add(fe->base.request, rid, client);
        else // File already loaded, otherwise there would be a request
           _image_opened_send(client, fe, rid);
        return 0;
     }

   file_id = ++_entry_id;
   while ((file_id == 0) || (eina_hash_find(file_entries, &file_id)))
     file_id = ++_entry_id;

   DBG("Creating new entry with file_id: %u for file \"%s:%s\"",
       file_id, path, key);
   fe = calloc(1, sizeof(*fe));
   fe->base.type = CSERVE2_IMAGE_FILE;
   fe->path = cserve2_shared_string_add(path);
   fe->key = cserve2_shared_string_add(key);
   fe->base.id = file_id;
   eina_hash_add(file_entries, &file_id, fe);
   eina_hash_add(file_ids, buf, (void*)(intptr_t)file_id);
   ref = _entry_reference_add((Entry *)fe, client, client_file_id);
   eina_hash_add(client->files.referencing, &client_file_id, ref);

   fw = eina_hash_find(file_watch, cserve2_shared_string_get(fe->path));
   if (!fw)
     {
        fw = calloc(1, sizeof(File_Watch));
        fw->path = eina_stringshare_add(cserve2_shared_string_get(fe->path));
        cserve2_file_change_watch_add(fw->path, _file_changed_cb, fw);
        eina_hash_direct_add(file_watch, fw->path, fw);
     }
   fw->entries = eina_list_append(fw->entries, fe);
   fe->watcher = fw;

   fe->base.request = cserve2_request_add(CSERVE2_REQ_IMAGE_OPEN,
                                          rid, client, NULL, &_open_funcs,
                                          fe);

   // _open_image_default_set(fe);

   return 0;
}

void
cserve2_cache_file_close(Client *client, unsigned int client_file_id)
{
   Reference *ref = eina_hash_find(client->files.referencing,
                                        &client_file_id);
   if (!ref)
     {
        ERR("Couldn't find file %d in client hash.", client_file_id);
        return;
     }

   ref->count--;
   if (ref->count <= 0)
     // will call _entry_free_cb() for this entry
     eina_hash_del_by_key(client->files.referencing, &client_file_id);
}

static int
_cserve2_cache_fast_scaling_check(Client *client, Image_Data *entry)
{
   Eina_Iterator *iter;
   Image_Data *i;
   Image_Data *original = NULL;
   Evas_Image_Load_Opts unscaled;
   char buf[4096];
   unsigned int image_id;
   int scaled_count = 0;
   int dst_w, dst_h;
   Eina_Bool first_attempt = EINA_TRUE;

   if (!entry) return -1;
   if (!entry->file) return -1;

   dst_w = entry->opts.scale_load.dst_w;
   dst_h = entry->opts.scale_load.dst_h;

   // Copy opts w/o scaling
   memset(&unscaled, 0, sizeof(unscaled));
   unscaled.dpi = entry->opts.dpi;
   //unscaled.w = entry->opts.w;
   //unscaled.h = entry->opts.h;
   //unscaled.scale_down_by = entry->opts.scale_down_by;
   //unscaled.region.x = entry->opts.region.x;
   //unscaled.region.y = entry->opts.region.y;
   //unscaled.region.w = entry->opts.region.w;
   //unscaled.region.h = entry->opts.region.h;
   unscaled.scale_load.scale_hint = 0;
   unscaled.degree = entry->opts.degree;
   unscaled.orientation = entry->opts.orientation;
   unscaled.scale_load.smooth = entry->opts.scale_load.smooth;

try_again:
   image_id = _img_opts_id_get(entry->file_id, &unscaled, buf, sizeof(buf));
   if (image_id)
     {
        original = eina_hash_find(image_entries, &image_id);
        if (!original) return -1; // Should not happen
        DBG("Found original image in hash: %d,%d:%dx%d -> %dx%d shm %p",
                original->opts.scale_load.src_x, original->opts.scale_load.src_y,
                original->opts.scale_load.src_w, original->opts.scale_load.src_h,
                original->opts.scale_load.dst_w, original->opts.scale_load.dst_h,
                original->shm);
        goto do_scaling;
     }

   if (first_attempt)
     {
        first_attempt = EINA_FALSE;
        memset(&unscaled, 0, sizeof(unscaled));
        goto try_again;
     }

   iter = eina_list_iterator_new(entry->file->images);
   EINA_ITERATOR_FOREACH(iter, i)
     {
        if (i == entry) continue;
        if (i->opts.w && i->opts.h &&
            (!i->opts.scale_load.dst_w && !i->opts.scale_load.dst_h))
          {
             DBG("Found image in list: %d,%d:%dx%d -> %dx%d shm %p",
                     i->opts.scale_load.src_x, i->opts.scale_load.src_y,
                     i->opts.scale_load.src_w, i->opts.scale_load.src_h,
                     i->opts.scale_load.dst_w, i->opts.scale_load.dst_h,
                     i->shm);
             if (i->base.request || !i->shm) continue; // Not loaded yet
             original = i;
             break;
          }
        scaled_count++;
     }
   eina_iterator_free(iter);

   if (!original)
     {
        DBG("Found %d scaled images for %s:%s but none matches",
            scaled_count, cserve2_shared_string_get(entry->file->path),
            cserve2_shared_string_get(entry->file->key));

        if (scaled_count >= 4)
          {
             DBG("Forcing load of original image now!");

             File_Data *fe;

             original = _image_entry_new(client, 0, entry->file_id,
                                         0, &unscaled);
             if (!original) return -1;

             image_id = ++_entry_id;
             while ((image_id == 0) || (eina_hash_find(image_entries, &image_id)))
               image_id = ++_entry_id;
             DBG("Creating new image_id: %d", image_id);

             original->base.id = image_id;
             eina_hash_add(image_entries, &image_id, original);
             eina_hash_add(image_ids, buf, (void *)(intptr_t)image_id);
             _entry_unused_push(original);

             fe = original->file;
             fe->images = eina_list_append(fe->images, original);
          }
        else
          return -1;
     }

do_scaling:
   if (!original) return -1;
   if (!original->shm && !original->base.request)
     {
        if (original->base.id != image_id) abort();
        original->base.request = cserve2_request_add(
                 CSERVE2_REQ_IMAGE_LOAD,
                 0, NULL, 0, &_load_funcs, original);
     }
   if (original->base.request || !original->shm)
     return -1; // Not loaded yet

   if (entry->shm)
     cserve2_shm_unref(entry->shm);

   entry->shm = cserve2_shm_request("img", dst_w * dst_h * 4);
   if (!entry->shm) return -1;

   if (_scaling_do(entry->shm, entry, original) != 0)
     {
        cserve2_shm_unref(entry->shm);
        entry->shm = NULL;
        return -1;
     }

   if (original->unused)
     {
        image_entries_lru = eina_list_remove(image_entries_lru, original);
        image_entries_lru = eina_list_prepend(image_entries_lru, original);
     }
   return 0;
}

int
cserve2_cache_image_entry_create(Client *client, int rid,
                                 unsigned int file_id,
                                 unsigned int client_image_id,
                                 Evas_Image_Load_Opts *opts)
{
   Image_Data *entry;
   File_Data *fe = NULL;
   Reference *ref, *oldref;
   unsigned int image_id;
   char buf[4096];

   oldref = eina_hash_find(client->images.referencing, &client_image_id);

   // search whether the image is already loaded by another client
   entry = _image_entry_new(client, rid, file_id, client_image_id, opts);
   if (!entry)
     return -1;
   image_id = _image_data_opts_id_get(entry, buf, sizeof(buf));
   if (image_id)
     {  // if so, just update the references
        free(entry);
        DBG("found image_id %d for client image id %d",
            image_id, client_image_id);
        entry = eina_hash_find(image_entries, &image_id);
        if (!entry)
          {
             ERR("image id %d is in file_ids hash, but not in entries hash"
                 "with entry id %d.", client_image_id, image_id);
             cserve2_client_error_send(client, rid, CSERVE2_INVALID_CACHE);
             return -1;
          }

        if (entry->unused)
          {
             DBG("Re-using old image entry (id: %d) from the LRU list.",
                 entry->base.id);
             entry->unused = EINA_FALSE;
             image_entries_lru = eina_list_remove(image_entries_lru, entry);
             unused_mem_usage -= _image_entry_size_get(entry);
          }
        _entry_load_reused(&entry->base);

        if (oldref && (oldref->entry->id == image_id))
          return 0;

        ref = _entry_reference_add((Entry *)entry, client, client_image_id);

        if (oldref)
          eina_hash_del_by_key(client->images.referencing, &client_image_id);

        eina_hash_add(client->images.referencing, &client_image_id, ref);
        return 0;
     }

   image_id = ++_entry_id;
   while ((image_id == 0) || (eina_hash_find(image_entries, &image_id)))
     image_id = ++_entry_id;

   entry->base.id = image_id;
   eina_hash_add(image_entries, &image_id, entry);
   eina_hash_add(image_ids, buf, (void *)(intptr_t)image_id);
   ref = _entry_reference_add((Entry *)entry, client, client_image_id);

   if (oldref)
     eina_hash_del_by_key(client->images.referencing, &client_image_id);
   eina_hash_add(client->images.referencing, &client_image_id, ref);

   fe = entry->file;
   fe->images = eina_list_append(fe->images, entry);

   if (opts && opts->scale_load.dst_w && opts->scale_load.dst_h)
     {
        if (!_cserve2_cache_fast_scaling_check(client, entry))
          return 0;
     }

   entry->base.request = cserve2_request_add(CSERVE2_REQ_IMAGE_SPEC_LOAD,
                                             0, NULL, fe->base.request,
                                             &_load_funcs, entry);
   return 0;
}

void
cserve2_cache_image_load(Client *client, unsigned int client_image_id, unsigned int rid)
{
   Image_Data *entry;
   Reference *ref;

   ref = eina_hash_find(client->images.referencing, &client_image_id);
   if (!ref)
     {
        ERR("Can't load: client %d has no image id %d",
            client->id, client_image_id);
        return;
     }

   entry = (Image_Data *)ref->entry;

   if (entry->file->invalid)
     {
        cserve2_client_error_send(client, rid, CSERVE2_FILE_CHANGED);
        return;
     }

   DBG("Loading image id: %d", ref->entry->id);

   // File already being loaded, just add the request to be replied
   if (entry->base.request)
     {
        cserve2_request_waiter_add(entry->base.request, rid, client);
        if (!entry->doload)
          cserve2_request_type_set(entry->base.request, CSERVE2_REQ_IMAGE_LOAD);
     }
   else if (entry->shm)
     _image_loaded_send(client, entry, rid);
   else
     entry->base.request = cserve2_request_add(CSERVE2_REQ_IMAGE_LOAD,
                                               rid, client,
                                               entry->file->base.request,
                                               &_load_funcs,
                                               entry);

   entry->doload = EINA_TRUE;
}

void
cserve2_cache_image_preload(Client *client, unsigned int client_image_id, unsigned int rid)
{
   Image_Data *entry;
   Reference *ref;

   ref = eina_hash_find(client->images.referencing, &client_image_id);
   if (!ref)
     {
        ERR("Can't load: client %d has no image id %d",
            client->id, client_image_id);
        return;
     }

   entry = (Image_Data *)ref->entry;

   if (entry->file->invalid)
     {
        cserve2_client_error_send(client, rid, CSERVE2_FILE_CHANGED);
        return;
     }

   DBG("Loading image id: %d", ref->entry->id);

   // File already being loaded, just add the request to be replied
   if (entry->base.request)
     {
        cserve2_request_waiter_add(entry->base.request, rid, client);
        if (!entry->doload)
          cserve2_request_type_set(entry->base.request, CSERVE2_REQ_IMAGE_LOAD);
     }
   else if (entry->shm)
     _image_loaded_send(client, entry, rid);
   else
     entry->base.request = cserve2_request_add(CSERVE2_REQ_IMAGE_LOAD,
                                               rid, client,
                                               entry->file->base.request,
                                               &_load_funcs,
                                               entry);

   entry->doload = EINA_TRUE;
}

void
cserve2_cache_image_unload(Client *client, unsigned int client_image_id)
{
   Reference *ref = eina_hash_find(client->images.referencing,
                                   &client_image_id);
   if (!ref)
     {
        ERR("Couldn't find file %d in client hash.", client_image_id);
        return;
     }

   ref->count--;
   if (ref->count <= 0)
     // will call _entry_free_cb() for this entry
     eina_hash_del_by_key(client->images.referencing, &client_image_id);
}

int
cserve2_cache_font_load(Client *client, const char *source, const char *name, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int rid)
{
   Reference *ref;
   Font_Source *fs;
   Font_Entry *fe;
   char *fullname;

   if (source && !*source)
     source = NULL;
   if (name && !*name)
     name = NULL;

   fullname = _file_path_join(source, name);
   fe = _cserve2_font_entry_find(fullname, size, rend_flags, dpi);
   if (fe)
     {
        DBG("found font entry %s, rendflags: %d, size: %d, dpi: %d",
            name, rend_flags, size, dpi);

        ref = _entry_reference_add((Entry *)fe, client, 0);
        client->fonts.referencing = eina_list_append(
           client->fonts.referencing, ref);

        _entry_load_reused(&fe->base);
        fe->unused = EINA_FALSE;

        if (fe->base.request)
          cserve2_request_waiter_add(fe->base.request, rid, client);
        else
          _font_loaded_send(client, rid);
        free(fullname);
        return 0;
     }

   fe = calloc(1, sizeof(*fe));
   fe->rend_flags = rend_flags;
   fe->size = size;
   fe->dpi = dpi;
   fe->base.type = CSERVE2_FONT_ENTRY;
   fe->glyphs = fash_gl_new(_glyph_free_cb);
   ref = _entry_reference_add((Entry *)fe, client, 0);
   client->fonts.referencing = eina_list_append(
      client->fonts.referencing, ref);
   fe->unused = EINA_FALSE;

   fs = _cserve2_font_source_find(fullname);
   if (!fs)
     {
        const char *key;
        fs = calloc(1, sizeof(*fs));
        if (source)
          {
             fs->key = cserve2_shared_string_add(fullname);
             fs->name = cserve2_shared_string_add(name);
             fs->file = cserve2_shared_string_add(source);
             key = fullname;
          }
        else
          {
             fs->file = cserve2_shared_string_add(name);
             fs->key = cserve2_shared_string_ref(fs->file);
             key = name;
          }
        eina_hash_direct_add(font_sources, key, fs);
     }

   fe->src = fs;
   fs->references++;
   DBG("adding FONT_LOAD '%s' request.", name);
   fe->base.request = cserve2_request_add(CSERVE2_REQ_FONT_LOAD, rid, client,
                                          NULL, &_font_load_funcs, fe);

   eina_hash_direct_add(font_entries, fe, fe);

   free(fullname);

   return 0;
}

int
cserve2_cache_font_unload(Client *client, const char *source, const char *name, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int rid EINA_UNUSED)
{
   Font_Entry *fe;
   char *fullname;

   if (source && !*source)
     source = NULL;
   if (name && !*name)
     name = NULL;

   fullname = _file_path_join(source, name);
   fe = _cserve2_font_entry_find(fullname, size, rend_flags, dpi);
   free(fullname);

   if (!fe)
     {
        ERR("Unreferencing font not found: '%s:%s'.", source, name);
        return -1;
     }

   _font_entry_reference_del(client, fe);

   return 0;
}

int
cserve2_cache_font_glyphs_load(Client *client, const char *source, const char *name, unsigned int hint, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int *glyphs, unsigned int nglyphs, unsigned int rid)
{
   Glyphs_Request *req;

   req = _glyphs_request_create(client, source, name,
                                hint, rend_flags, size, dpi, glyphs, nglyphs);
   if (!req)
     {
        free(glyphs);
        return -1;
     }

   if (_glyphs_request_check(req, EINA_TRUE))
     {
        INF("Glyphs already loaded. Sending answer.");
        _glyphs_loaded_send(req, rid);
        _glyphs_request_free(req);
     }
   else
     {
        cserve2_request_add(CSERVE2_REQ_FONT_GLYPHS_LOAD, rid, client,
                            req->fe->base.request, &_glyphs_load_funcs, req);
     }
   return 0;
}

int
cserve2_cache_font_glyphs_used(Client *client, const char *source, const char *name, unsigned int hint, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int *glyphs, unsigned int nglyphs, unsigned int rid EINA_UNUSED)
{
   Glyphs_Group *gg;
   Eina_List *groups;
   Glyphs_Request *req;

   DBG("Received report of used glyphs from client %d", client->id);
   req = _glyphs_request_create(client, source, name,
                                hint, rend_flags, size, dpi, glyphs, nglyphs);
   if (!req)
     {
        free(glyphs);
        return 0;
     }

   _glyphs_request_check(req, EINA_FALSE);
   groups = _glyphs_group_create(req);

   // Promote SHMs which are still cached and in use
   // TODO: We can use later the information from request_prepare to preload
   // glyphs which are not cached anymore, but are in use on the client.
   EINA_LIST_FREE(groups, gg)
     {
        _font_shm_promote(gg->fc);
        eina_list_free(gg->glyphs);
        free(gg);
     }

   _glyphs_request_free(req);
   return 0;
}

void
cserve2_cache_stats_get(Client *client, unsigned int rid)
{
   Msg_Stats msg;
   int size;

   memset(&msg, 0, sizeof(msg));

   msg.base.type = CSERVE2_STATS;
   msg.base.rid = rid;

   _cserve2_cache_image_stats_get(&msg);
   _cserve2_cache_font_stats_get(&msg);

   size = sizeof(msg);
   cserve2_client_send(client, &size, sizeof(size));
   cserve2_client_send(client, &msg, size);
}

void
cserve2_cache_font_debug(Client *client, unsigned int rid)
{
   void *msg;
   unsigned int size;

   msg = _cserve2_cache_font_debug(rid, &size);

   cserve2_client_send(client, &size, sizeof(size));
   cserve2_client_send(client, msg, size);

   free(msg);
}
