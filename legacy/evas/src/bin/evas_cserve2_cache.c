#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#ifdef DEBUG_LOAD_TIME
   #include <sys/time.h>
#endif

#include "evas_cserve2.h"
#include "evas_cs2_utils.h"

typedef struct _Request_Funcs Request_Funcs;
typedef struct _Request Request;

typedef struct _Entry Entry;
typedef struct _Reference Reference;
typedef struct _Waiter Waiter;
typedef struct _File_Data File_Data;
typedef struct _Image_Data Image_Data;
typedef struct _File_Watch File_Watch;

typedef struct _Font_Source Font_Source;
typedef struct _Font_Entry Font_Entry;
typedef struct _Font_Cache Font_Cache;

typedef void *(*Request_Msg_Create)(Entry *e, int *size);
typedef void (*Request_Response)(Entry *e, void *resp);
typedef void (*Request_Error)(Entry *e, Error_Type error);

struct _Request_Funcs {
   Request_Msg_Create msg_create;
   Request_Response response;
   Request_Error error;
};

struct _Request {
   Entry *entry;
   Eina_List *waiters;
   Eina_Bool processing;
   Request_Funcs *funcs;
};

typedef enum {
   CSERVE2_IMAGE_FILE,
   CSERVE2_IMAGE_DATA,
   CSERVE2_FONT_ENTRY
} Entry_Type;

struct _Entry {
   unsigned int id;
   Eina_List *references;
   Request *request;
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
   char *path;
   char *key;
   int w, h;
   int frame_count;
   int loop_count;
   int loop_hint;
   const char *loader_data;
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
   struct {
      double dpi; // dpi < -1
      int w, h; // w and h < -1
      int scale_down; // scale_down < -1
      int rx, ry, rw, rh; // rx, ry, rw, rh < -1
      Eina_Bool orientation; // orientation == 0
   } opts;
   Shm_Handle *shm;
   Eina_Bool alpha_sparse : 1;
   Eina_Bool unused : 1;
   Eina_Bool doload : 1;
};

struct _Font_Source {
   const char *key;
   const char *name;
   const char *file;
   int references;
   void *ft;
};

struct _Font_Entry {
   Entry base;
   Font_Request *request;
   unsigned int rend_flags;
   unsigned int size;
   unsigned int dpi;
   Font_Source *src;
   void *ft;
   Fash_Glyph2 *glyphs;
   Eina_Inlist *caches;
   Font_Cache *last_cache;
   Eina_Bool unused : 1;
#ifdef DEBUG_LOAD_TIME
   struct timeval load_start;
   struct timeval load_finish;
   int gl_load_time;
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

struct _Waiter {
   Reference *ref;
   unsigned int rid;
   Message_Type type;
};

struct _File_Watch {
   const char *path;
   Eina_List *entries;
};

static Eina_List *open_requests = NULL;
static Eina_List *load_requests = NULL;
static Eina_List *spload_requests = NULL; // speculative preload requests

static unsigned int _file_id = 0; // id unique number
static unsigned int _image_id = 0; // id unique number
static Eina_Hash *file_ids = NULL; // maps path + key --> file_id
static Eina_Hash *file_entries = NULL; // maps file_id --> entry

static Eina_Hash *image_ids = NULL; // maps file id + load opts --> image id
static Eina_Hash *image_entries = NULL; // maps image_id --> entry

static Eina_Hash *font_sources = NULL; // font path --> font source
static Eina_Hash *font_entries = NULL; // maps font path + options --> entry

static Eina_Hash *file_watch = NULL;

static Eina_List *image_entries_lru = NULL;

static Eina_List *font_shm_lru = NULL;

static int max_unused_mem_usage = 5 * 1024; /* in kbytes */
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

static void
_image_opened_send(Client *client, File_Data *entry, unsigned int rid)
{
    int size;
    Msg_Opened msg;

    DBG("Sending OPENED reply for entry: %d and RID: %d.", entry->base.id, rid);
    // clear the struct with possible paddings, since it is not aligned.
    memset(&msg, 0, sizeof(msg));
    msg.base.rid = rid;
    msg.base.type = CSERVE2_OPENED;
    msg.image.w = entry->w;
    msg.image.h = entry->h;
    msg.image.frame_count = entry->frame_count;
    msg.image.loop_count = entry->loop_count;
    msg.image.loop_hint = entry->loop_hint;
    msg.image.alpha = entry->alpha;

    size = sizeof(msg);
    cserve2_client_send(client, &size, sizeof(size));
    cserve2_client_send(client, &msg, sizeof(msg));
    // _cserve2_cache_load_requests_process();
}

static void
_image_loaded_send(Client *client, Image_Data *entry, unsigned int rid)
{
   int size;
   const char *shmpath = cserve2_shm_name_get(entry->shm);
   Msg_Loaded msg;
   int path_len;
   char *buf;

   DBG("Sending LOADED reply for entry %d and RID: %d.", entry->base.id, rid);
   path_len = strlen(shmpath) + 1;

   memset(&msg, 0, sizeof(msg));
   msg.base.rid = rid;
   msg.base.type = CSERVE2_LOADED;

   msg.shm.mmap_offset = cserve2_shm_map_offset_get(entry->shm);
   msg.shm.use_offset = cserve2_shm_offset_get(entry->shm);
   msg.shm.mmap_size = cserve2_shm_map_size_get(entry->shm);
   msg.shm.image_size = cserve2_shm_size_get(entry->shm);
   msg.alpha_sparse = entry->alpha_sparse;

   buf = malloc(sizeof(msg) + path_len);

   memcpy(buf, &msg, sizeof(msg));
   memcpy(buf + sizeof(msg), shmpath, path_len);

   size = sizeof(msg) + path_len;

   cserve2_client_send(client, &size, sizeof(size));
   cserve2_client_send(client, buf, size);

   free(buf);
}

static void
_image_preloaded_send(Client *client, unsigned int rid)
{
   int size;
   Msg_Preloaded msg;

   DBG("Sending PRELOADED reply for RID: %d.", rid);
   memset(&msg, 0, sizeof(msg));
   msg.base.rid = rid;
   msg.base.type = CSERVE2_PRELOADED;

   size = sizeof(msg);
   cserve2_client_send(client, &size, sizeof(size));
   cserve2_client_send(client, &msg, size);
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
_open_request_build(File_Data *f, int *bufsize)
{
   char *buf;
   int size, pathlen, keylen;
   Slave_Msg_Image_Open msg;

   pathlen = strlen(f->path) + 1;
   keylen = strlen(f->key) + 1;

   size = sizeof(msg) + pathlen + keylen;
   buf = malloc(size);
   if (!buf) return NULL;

   memset(&msg, 0, sizeof(msg));
   memcpy(buf, &msg, sizeof(msg));
   memcpy(buf + sizeof(msg), f->path, pathlen);
   memcpy(buf + sizeof(msg) + pathlen, f->key, keylen);

   *bufsize = size;

   _entry_load_start(&f->base);

   return buf;
}

static void
_request_failed(Entry *e, Error_Type type)
{
   Waiter *w;
   Eina_List *l;
   Reference *ref;

   DBG("Request for entry %p failed with error %d", e, type);
   EINA_LIST_FREE(e->request->waiters, w)
     {
        cserve2_client_error_send(w->ref->client, w->rid, type);

        w->ref->count--;
        free(w);
     }

   EINA_LIST_FOREACH(e->references, l, ref)
     {
        Eina_Hash *hash = NULL;
        if (e->type == CSERVE2_IMAGE_FILE)
          hash = ref->client->files.referencing;
        else if (e->type == CSERVE2_IMAGE_DATA)
          hash = ref->client->images.referencing;
        else
          continue;

        eina_hash_del_by_key(hash, &(ref->client_entry_id));
     }
}

static void
_open_request_response(File_Data *e, Slave_Msg_Image_Opened *resp)
{
   Waiter *w;

   _entry_load_finish(&e->base);
   e->w = resp->w;
   e->h = resp->h;
   e->frame_count = resp->frame_count;
   e->loop_count = resp->loop_count;
   e->loop_hint = resp->loop_hint;
   e->alpha = resp->alpha;
   if (resp->has_loader_data)
     {
        const char *ldata = (const char *)resp +
                                           sizeof(Slave_Msg_Image_Opened);
        e->loader_data = eina_stringshare_add(ldata);
     }

   DBG("Finished opening file %d. Notifying %d waiters.", e->base.id,
       e->base.request->waiters ? eina_list_count(e->base.request->waiters) : 0);
   EINA_LIST_FREE(e->base.request->waiters, w)
     {
        _image_opened_send(w->ref->client, e, w->rid);
        free(w);
     }
}

static Request_Funcs _open_funcs = {
   .msg_create = (Request_Msg_Create)_open_request_build,
   .response = (Request_Response)_open_request_response,
   .error = (Request_Error)_request_failed
};

static void *
_load_request_build(Image_Data *i, int *bufsize)
{
   char *buf, *ptr;
   const char *shmpath;
   int size;
   int shmlen, filelen, keylen, loaderlen;
   Slave_Msg_Image_Load msg;

   // opening shm for this file
   i->shm = cserve2_shm_request(i->file->w * i->file->h * 4);

   shmpath = cserve2_shm_name_get(i->shm);

   shmlen = strlen(shmpath) + 1;
   filelen = strlen(i->file->path) + 1;
   keylen = strlen(i->file->key) + 1;
   if (i->file->loader_data)
     loaderlen = strlen(i->file->loader_data) + 1;
   else
     loaderlen = 0;

   size = sizeof(msg) + shmlen + filelen + keylen + loaderlen;
   buf = malloc(size);
   if (!buf) return NULL;

   memset(&msg, 0, sizeof(msg));
   msg.w = i->file->w;
   msg.h = i->file->h;
   msg.alpha = i->file->alpha;
   msg.opts.w = i->opts.w;
   msg.opts.h = i->opts.h;
   msg.opts.rx = i->opts.rx;
   msg.opts.ry = i->opts.ry;
   msg.opts.rw = i->opts.rw;
   msg.opts.rh = i->opts.rh;
   msg.opts.scale_down_by = i->opts.scale_down;
   msg.opts.dpi = i->opts.dpi;
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
   memcpy(ptr, i->file->path, filelen);
   ptr += filelen;
   memcpy(ptr, i->file->key, keylen);
   ptr += keylen;
   memcpy(ptr, i->file->loader_data, loaderlen);

   *bufsize = size;

   _entry_load_start(&i->base);

   return buf;
}

static void
_load_request_response(Image_Data *e, Slave_Msg_Image_Loaded *resp)
{
   Waiter *w;

   _entry_load_finish(&e->base);

   e->alpha_sparse = resp->alpha_sparse;
   if (!e->doload)
     DBG("Entry %d loaded by speculative preload.", e->base.id);

   DBG("Finished loading image %d. Notifying %d waiters.", e->base.id,
       e->base.request->waiters ? eina_list_count(e->base.request->waiters) : 0);
   EINA_LIST_FREE(e->base.request->waiters, w)
     {
        if (w->type == CSERVE2_LOAD)
          _image_loaded_send(w->ref->client, e, w->rid);
        else if (w->type == CSERVE2_PRELOAD)
          _image_preloaded_send(w->ref->client, w->rid);
        // else w->type == CSERVE2_SETOPTS --> do nothing

        free(w);
     }
}

static Request_Funcs _load_funcs = {
   .msg_create = (Request_Msg_Create)_load_request_build,
   .response = (Request_Response)_load_request_response,
   .error = (Request_Error)_request_failed
};

static unsigned int
_img_opts_id_get(Image_Data *im, char *buf, int size)
{
   uintptr_t image_id;

   snprintf(buf, size, "%u:%0.3f:%dx%d:%d:%d,%d+%dx%d:%d",
            im->file_id, im->opts.dpi, im->opts.w, im->opts.h,
            im->opts.scale_down, im->opts.rx, im->opts.ry,
            im->opts.rw, im->opts.rh, im->opts.orientation);

   image_id = (uintptr_t)eina_hash_find(image_ids, buf);

   return image_id;
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
_file_id_free(File_Data *entry)
{
   char buf[4096];

   DBG("Removing entry file id: %d, file: \"%s:%s\"",
       entry->base.id, entry->path, entry->key);
   snprintf(buf, sizeof(buf), "%s:%s", entry->path, entry->key);
   eina_hash_del_by_key(file_ids, buf);
}

static void
_image_id_free(Image_Data *entry)
{
   char buf[4096];

   DBG("Removing entry image id: %d", entry->base.id);

   _img_opts_id_get(entry, buf, sizeof(buf));
   eina_hash_del_by_key(image_ids, buf);
}

static void
_image_entry_free(Image_Data *entry)
{
   File_Data *fentry = entry->file;

   if (entry->base.request)
     {
        if (entry->base.request->processing)
          entry->base.request->entry = NULL;
        else if (!entry->base.request->waiters)
          {
             if (entry->doload)
               load_requests = eina_list_remove(load_requests,
                                                entry->base.request);
             else
               spload_requests = eina_list_remove(spload_requests,
                                                  entry->base.request);
          }
     }

   if (entry->unused)
     {
        image_entries_lru = eina_list_remove(image_entries_lru, entry);
        unused_mem_usage -= _image_entry_size_get(entry);
     }

   if (fentry)
     {
        fentry->images = eina_list_remove(fentry->images, entry);
        if (!fentry->images && !fentry->base.references)
          eina_hash_del_by_key(file_entries, &fentry->base.id);
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
_file_entry_free(File_Data *entry)
{
   File_Watch *fw;

   // Should we call free for each of the images too?
   // If everything goes fine, it's not necessary.
   if (entry->images)
     {
        ERR("Freeing file %d (\"%s:%s\") image data still referenced.",
            entry->base.id, entry->path, entry->key);
        eina_list_free(entry->images);
     }

   if (entry->base.request)
     {
        if (entry->base.request->processing)
          entry->base.request->entry = NULL;
        else if (!entry->base.request->waiters)
          {
             open_requests = eina_list_remove(open_requests,
                                              entry->base.request);
             free(entry->base.request);
          }
     }

   if ((fw = entry->watcher))
     {
        fw->entries = eina_list_remove(fw->entries, entry);
        if (!fw->entries)
          eina_hash_del_by_key(file_watch, fw->path);
     }

   free(entry->key);
   free(entry->path);
   eina_stringshare_del(entry->loader_data);
   free(entry);
}

static void
_hash_file_entry_free(void *data)
{
   File_Data *entry = data;
   // TODO: Add some checks to make sure that we are freeing an
   // unused entry.

   _file_id_free(entry);
   _file_entry_free(entry);
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
_font_entry_cmp(const Font_Entry *k1, int k1_length __UNUSED__, const Font_Entry *k2, int k2_length __UNUSED__)
{
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
   return strcmp(k1->src->key, k2->src->key);
}

static int
_font_entry_key_hash(const Font_Entry *key, int key_length __UNUSED__)
{
   int hash;
   hash = eina_hash_djb2(key->src->key, eina_stringshare_strlen(key->src->key) + 1);
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
     eina_hash_del_by_key(font_sources, fe->src->key);
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
   eina_stringshare_del(fs->key);
   eina_stringshare_del(fs->name);
   eina_stringshare_del(fs->file);
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

static void
_request_answer_del(Eina_List **requests, Request *req, Client *client, Error_Type err)
{
   Eina_List *l, *l_next;
   Waiter *it;

   DBG("Removing answer requests from entry: %d, client: %d",
       req->entry->id, client->id);

   EINA_LIST_FOREACH_SAFE(req->waiters, l, l_next, it)
     {
        if (it->ref->client->id == client->id)
          {
             cserve2_client_error_send(client, it->rid, err);
             req->waiters = eina_list_remove_list(req->waiters, l);
             free(it);
          }
     }

   // FIXME: Should this be really here? I guess that it should be in the
   // entry_free_cb function, or entry_reference_del, when there are no more
   // references
   if (!req->entry && !req->waiters)
     {
        *requests = eina_list_remove(*requests, req);
        free(req);
     }
}

static void
_request_answer_all_del(Eina_List **requests, Request *req, Error_Type err)
{
   Waiter *it;

   DBG("Removing all answer requests from entry: %d", req->entry->id);

   EINA_LIST_FREE(req->waiters, it)
     {
        cserve2_client_error_send(it->ref->client, it->rid, err);
        free(it);
     }

   *requests = eina_list_remove(*requests, req);
   free(req);
}

static void
_request_answer_add(Request *req, Reference *ref, unsigned int rid, Message_Type type)
{
   Waiter *w = malloc(sizeof(*w));

   w->ref = ref;
   w->rid = rid;
   w->type = type;

   DBG("Add answer request for entry id: %d, client: %d, rid: %d",
       req->entry->id, ref->client->id, rid);
   req->waiters = eina_list_append(req->waiters, w);
}

static void
_request_add(Eina_List **requests, Entry *entry, Reference *ref, unsigned int rid, Message_Type type)
{
   Request *req;

   // add the request if it doesn't exist yet
   if (!entry->request)
     {
        req = malloc(sizeof(*req));
        req->entry = entry;
        req->waiters = NULL;
        req->processing = EINA_FALSE;
        entry->request = req;
        if (type == CSERVE2_OPEN)
          req->funcs = &_open_funcs;
        else
          req->funcs = &_load_funcs;
        *requests = eina_list_append(*requests, req);
        DBG("Add request for entry id: %d, client: %d, rid: %d",
            req->entry->id, ref->client->id, rid);
     }
   else
     req = entry->request;

   if (type != CSERVE2_SETOPTS)
     _request_answer_add(req, ref, rid, type);
   else
     DBG("Adding entry for speculative preload: id=%d", req->entry->id);
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

static int
_cserve2_cache_open_requests_process(int nloaders)
{
   Request *req;
   char *slave_cmd_data;
   int slave_cmd_size;

   while ((nloaders > 0) && (open_requests))
     {
        // remove the first element from the list and process this element
        req = eina_list_data_get(open_requests);
        open_requests = eina_list_remove_list(open_requests, open_requests);

        DBG("Processing OPEN request for file entry: %d", req->entry->id);

        slave_cmd_data = req->funcs->msg_create(req->entry, &slave_cmd_size);

        cserve2_slave_cmd_dispatch(req, IMAGE_OPEN, slave_cmd_data,
                                   slave_cmd_size);

        free(slave_cmd_data);

        req->processing = EINA_TRUE;
        nloaders--;
     }

   return nloaders;
}

static int
_cserve2_cache_load_requests_list_process(Eina_List **queue, int nloaders)
{
   Eina_List *skipped = NULL;
   Request *req;

   while ((nloaders > 0) && (*queue))
     {
        Image_Data *ientry;
        char *buf;
        int size;

        // remove the first element from the list and process this element
        req = eina_list_data_get(*queue);
        *queue = eina_list_remove_list(*queue, *queue);

        ientry = (Image_Data *)req->entry;
        if (!ientry->file)
          {
             ERR("File entry doesn't exist for entry id %d", req->entry->id);
             _request_failed(req->entry, CSERVE2_INVALID_CACHE);
             continue;
          }

        if (ientry->file->base.request)
          {
             /* OPEN still pending, skip this request */
             skipped = eina_list_append(skipped, req);
             continue;
          }

        DBG("Processing LOAD request for image entry: %d", req->entry->id);

        buf = req->funcs->msg_create(req->entry, &size);

        cserve2_slave_cmd_dispatch(req, IMAGE_LOAD, buf, size);

        free(buf);

        req->processing = EINA_TRUE;

        nloaders--;
     }

   EINA_LIST_FREE(skipped, req)
      *queue = eina_list_append(*queue, req);

   return nloaders;
}

static void
_cserve2_cache_load_requests_process(int nloaders)
{
   nloaders = _cserve2_cache_load_requests_list_process(&load_requests,
                                                        nloaders);
   _cserve2_cache_load_requests_list_process(&spload_requests, nloaders - 1);
}


void
cserve2_cache_requests_process(void)
{
   int avail_loaders;

   avail_loaders = cserve2_slave_available_get();
   avail_loaders = _cserve2_cache_open_requests_process(avail_loaders);
   _cserve2_cache_load_requests_process(avail_loaders);
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
   while (size > (max_unused_mem_usage - unused_mem_usage))
     {
        Entry *ie = eina_list_data_get(eina_list_last(image_entries_lru));
        eina_hash_del_by_key(image_entries, &ie->id);
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
        File_Data *fentry = (File_Data *)entry;

        if (fentry->invalid)
          _file_entry_free(fentry);
        else if (!fentry->images)
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

   if (entry->request && !entry->request->processing)
     {
        if (entry->type == CSERVE2_IMAGE_FILE)
          _request_answer_del(&open_requests, entry->request, ref->client,
                              CSERVE2_REQUEST_CANCEL);
        else if (entry->type == CSERVE2_IMAGE_DATA)
          {
             if (((Image_Data *)entry)->doload)
               _request_answer_del(&load_requests, entry->request,
                                   ref->client, CSERVE2_REQUEST_CANCEL);
             else
               _request_answer_del(&spload_requests, entry->request,
                                   ref->client, CSERVE2_REQUEST_CANCEL);
          }
     }

   _entry_reference_del(entry, ref);
}

static void
_font_entry_reference_del(Client *client, Font_Entry *fe)
{
   Eina_List *l;
   Reference *ref;

   EINA_LIST_FOREACH(client->fonts.referencing, l, ref)
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
_image_msg_new(Client *client, Msg_Setopts *msg)
{
   Reference *ref;
   Image_Data *im_entry;

   ref = eina_hash_find(client->files.referencing, &msg->file_id);
   if (!ref)
     {
        ERR("Couldn't find file id: %d, for image id: %d",
            msg->file_id, msg->image_id);
        cserve2_client_error_send(client, msg->base.rid,
                                  CSERVE2_INVALID_CACHE);
        return NULL;
     }
   if (((File_Data *)ref->entry)->invalid)
     {
        cserve2_client_error_send(client, msg->base.rid,
                                  CSERVE2_FILE_CHANGED);
        return NULL;
     }

   im_entry = calloc(1, sizeof(*im_entry));
   im_entry->base.type = CSERVE2_IMAGE_DATA;
   im_entry->file_id = ref->entry->id;
   im_entry->file = (File_Data *)ref->entry;
   im_entry->opts.dpi = msg->opts.dpi;
   im_entry->opts.w = msg->opts.w;
   im_entry->opts.h = msg->opts.h;
   im_entry->opts.scale_down = msg->opts.scale_down;
   im_entry->opts.rx = msg->opts.rx;
   im_entry->opts.ry = msg->opts.ry;
   im_entry->opts.rw = msg->opts.rw;
   im_entry->opts.rh = msg->opts.rh;
   im_entry->opts.orientation = msg->opts.orientation;

   return im_entry;
}

static void
_file_changed_cb(const char *path __UNUSED__, Eina_Bool deleted __UNUSED__, void *data)
{
   File_Watch *fw = data;
   File_Data *e;
   Eina_List *l;

   EINA_LIST_FOREACH(fw->entries, l, e)
     {
        Eina_List *ll;
        Image_Data *ie;

        e->invalid = EINA_TRUE;
        e->watcher = NULL;

        EINA_LIST_FOREACH(e->images, ll, ie)
          {
             _image_id_free(ie);
             eina_hash_set(image_entries, &ie->base.id, NULL);
             if (ie->base.request && !ie->base.request->processing)
               {
                  if (ie->doload)
                    _request_answer_all_del(&load_requests, ie->base.request,
                                            CSERVE2_FILE_CHANGED);
                  else
                    _request_answer_all_del(&spload_requests, ie->base.request,
                                            CSERVE2_FILE_CHANGED);
               }
             ie->base.request = NULL;
             if (ie->unused)
               _image_entry_free(ie);
          }

        _file_id_free(e);
        eina_hash_set(file_entries, &e->base.id, NULL);
        if (e->base.request && !e->base.request->processing)
          _request_answer_all_del(&open_requests, e->base.request,
                                  CSERVE2_FILE_CHANGED);
        e->base.request = NULL;
        if (!e->images && !e->base.references)
          _file_entry_free(e);
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
_cserve2_font_entry_find(const char *name, unsigned int namelen, unsigned int size, unsigned int rend_flags, unsigned int dpi)
{
   Font_Entry tmp_fe;
   Font_Source tmp_fs;
   Font_Entry *fe;

   tmp_fs.key = eina_stringshare_add_length(name, namelen);
   tmp_fe.src = &tmp_fs;
   tmp_fe.size = size;
   tmp_fe.rend_flags = rend_flags;
   tmp_fe.dpi = dpi;

   fe = eina_hash_find(font_entries, &tmp_fe);
   eina_stringshare_del(tmp_fs.key);

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
   msg->name = fe->src->name;
   msg->file = fe->src->file;

   *size = 0;

   _entry_load_start(&fe->base);

   return msg;
}

static void
_font_load_request_free(void *msg, void *data __UNUSED__)
{
   free(msg);
}

static void
_font_load_request_response(Client *client __UNUSED__, void *data, void *resp, unsigned int rid __UNUSED__)
{
   Slave_Msg_Font_Loaded *msg = resp;
   Font_Entry *fe = data;

   DBG("request %d answered.", rid);

   if (!fe->src->ft)
     fe->src->ft = msg->ftdata1;

   if (!fe->ft)
     {
        fe->ft = msg->ftdata2;
        _entry_load_finish(&fe->base);
     }

   if (fe->request) fe->request = NULL;

   _font_loaded_send(client, rid);
}

static void
_font_load_request_failed(Client *client __UNUSED__, void *data __UNUSED__, Error_Type error __UNUSED__, unsigned int rid __UNUSED__)
{
   Font_Entry *fe = data;
   DBG("request %d error answered.", rid);

   cserve2_client_error_send(client, rid, error);

   if (fe->request) fe->request = NULL;

   _font_entry_reference_del(client, fe);
}

static Font_Request_Funcs _font_load_funcs = {
   .msg_create = (Font_Request_Msg_Create)_font_load_request_build,
   .msg_free = (Font_Request_Msg_Free)_font_load_request_free,
   .response = (Font_Request_Response)_font_load_request_response,
   .error = (Font_Request_Error)_font_load_request_failed
};

static Eina_Bool
_glyphs_request_check(Glyphs_Request *req)
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

static void
_glyphs_loaded_send(Glyphs_Request *req, unsigned int rid)
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
   msg.base.rid = rid;
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
_glyphs_request_create(Client *client, const char *source, unsigned int sourcelen, const char *name, unsigned int namelen, unsigned int hint, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int *glyphs, unsigned int nglyphs)
{
   char *fullname;
   Glyphs_Request *req = calloc(1, sizeof(*req));

   if (sourcelen == 0)
     source = NULL;
   if (namelen == 0)
     name = NULL;

   fullname = _file_path_join(source, name);
   req->fe = _cserve2_font_entry_find(fullname, strlen(fullname) + 1, size,
                                      rend_flags, dpi);
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
             ge->fc->inuse++;
          }
        else
          req->render[req->nrender++] = req->glyphs[i];
     }
}

static void *
_glyphs_load_request_build(void *data, int *size __UNUSED__)
{
   Glyphs_Request *req = data;
   Slave_Msg_Font_Glyphs_Load *msg = NULL;
   Font_Entry *fe = req->fe;
   Font_Cache *fc;

   _glyphs_load_request_prepare(req);

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

#ifdef DEBUG_LOAD_TIME
   gettimeofday(&fe->load_start, NULL);
#endif

   return msg;
}

static void
_glyphs_load_request_free(void *msg, void *data)
{
   _glyphs_request_free(data);
   free(msg);
}

static void
_glyphs_load_request_response(Client *client __UNUSED__, void *data, void *resp, unsigned int rid)
{
   Glyphs_Request *req = data;
   Slave_Msg_Font_Glyphs_Loaded *msg = resp;
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
             fc = malloc(sizeof(*fc));
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
             Glyph_Entry *gl = malloc(sizeof(*gl));
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
             fash_gl_add(fe->glyphs, gl->index, gl);
             req->answer[req->nanswer++] = gl;
             gl->fc->inuse++;
          }

        free(c); // FIXME: We are freeing this here because we only do a
                 // simple free on the response message. Later we need to
                 // setup a free callback for the slave response.
        fc = NULL;
     }

#ifdef DEBUG_LOAD_TIME
   int load_time;
   gettimeofday(&fe->load_finish, NULL);
   load_time = _timeval_sub(&fe->load_finish, &fe->load_start);
   fe->gl_load_time += load_time;
#endif

   _glyphs_loaded_send(req, rid);
   _font_shm_lru_flush();
}

static void
_glyphs_load_request_failed(Client *client __UNUSED__, void *data __UNUSED__, Error_Type error __UNUSED__, unsigned int rid __UNUSED__)
{
}

static Font_Request_Funcs _glyphs_load_funcs = {
   .msg_create = (Font_Request_Msg_Create)_glyphs_load_request_build,
   .msg_free = (Font_Request_Msg_Free)_glyphs_load_request_free,
   .response = (Font_Request_Response)_glyphs_load_request_response,
   .error = (Font_Request_Error)_glyphs_load_request_failed
};

static Eina_Bool
_font_entry_stats_cb(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata)
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
#endif

   return EINA_TRUE;
}

static Eina_Bool
_image_file_entry_stats_cb(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata)
{
   Msg_Stats *msg = fdata;
   File_Data *fd = data;

   // accounting numbers
   msg->images.files_loaded++;

   // accounting size
   msg->images.files_size += sizeof(File_Data) +
      eina_list_count(fd->images) * sizeof(Eina_List *) +
      eina_list_count(fd->base.references) *
         (sizeof(Request) + sizeof(Eina_List *));

#ifdef DEBUG_LOAD_TIME
   // accounting file entries load time
   msg->images.files_load_time += fd->base.load_time;
   msg->images.files_saved_time += fd->base.saved_time;
#endif

   return EINA_TRUE;
}

static Eina_Bool
_image_data_entry_stats_cb(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata)
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
_font_entry_debug_size_cb(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata)
{
   struct _debug_info *di = fdata;
   unsigned int size = di->size;
   Font_Entry *fe = data;
   Font_Cache *fc;
   unsigned int intsize = sizeof(unsigned int);

   // filelen
   size += intsize;

   // file
   if (fe->src->file)
     size += strlen(fe->src->file) + 1;

   // namelen
   size += intsize;

   // name
   if (fe->src->name)
     size += strlen(fe->src->name) + 1;

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
_font_entry_debug_cb(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata)
{
   char **pos = fdata;
   char *buf = *pos;
   Font_Entry *fe = data;
   Font_Cache *fc;
   unsigned int len;
   unsigned int unused;
   unsigned int ncaches;
   unsigned int intsize = sizeof(unsigned int);

   // filelen + file
   len = 0;
   if (fe->src->file)
     len = strlen(fe->src->file) + 1;
   memcpy(buf, &len, intsize);
   buf += intsize;
   memcpy(buf, fe->src->file, len);
   buf += len;

   // namelen + name
   len = 0;
   if (fe->src->name)
     len = strlen(fe->src->name) + 1;
   memcpy(buf, &len, intsize);
   buf += intsize;
   memcpy(buf, fe->src->name, len);
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
cserve2_cache_file_open(Client *client, unsigned int client_file_id, const char *path, const char *key, unsigned int rid)
{
   unsigned int file_id;
   File_Data *entry;
   Reference *ref;
   File_Watch *fw;
   char buf[4906];

   // look for this file on client references
   ref = eina_hash_find(client->files.referencing, &client_file_id);
   if (ref)
     {
        entry = (File_Data *)ref->entry;
        _entry_load_reused(ref->entry);

        if (entry->invalid)
          {
             cserve2_client_error_send(client, rid, CSERVE2_FILE_CHANGED);
             return -1;
          }

        DBG("found client file id: %d", client_file_id);
        ref->count++;

        // File already being loaded, just add the request to be replied
        if (entry->base.request)
          _request_answer_add(entry->base.request, ref, rid, CSERVE2_OPEN);
        else
          _image_opened_send(client, entry, rid);
        return 0;
     }

   // search whether the file is already opened by another client
   snprintf(buf, sizeof(buf), "%s:%s", path, key);
   file_id = (unsigned int)eina_hash_find(file_ids, buf);
   if (file_id)
     {
        DBG("found file_id %u for client file id %d",
                file_id, client_file_id);
        entry = eina_hash_find(file_entries, &file_id);
        if (!entry)
          {
             ERR("file \"%s\" is in file_ids hash but not in entries hash.",
                 buf);
             cserve2_client_error_send(client, rid, CSERVE2_INVALID_CACHE);
             return -1;
          }
        ref = _entry_reference_add((Entry *)entry, client, client_file_id);
        _entry_load_reused(ref->entry);
        eina_hash_add(client->files.referencing, &client_file_id, ref);
        if (entry->base.request)
          _request_answer_add(entry->base.request, ref, rid, CSERVE2_OPEN);
        else // File already loaded, otherwise there would be a request
          _image_opened_send(client, entry, rid);
        return 0;
     }

   file_id = _file_id++;
   while ((file_id == 0) || (eina_hash_find(file_entries, &file_id)))
     file_id = _file_id++;

   DBG("Creating new entry with file_id: %u for file \"%s:%s\"",
       file_id, path, key);
   entry = calloc(1, sizeof(*entry));
   entry->base.type = CSERVE2_IMAGE_FILE;
   entry->path = strdup(path);
   entry->key = strdup(key);
   entry->base.id = file_id;
   eina_hash_add(file_entries, &file_id, entry);
   eina_hash_add(file_ids, buf, (void *)file_id);
   ref = _entry_reference_add((Entry *)entry, client, client_file_id);
   eina_hash_add(client->files.referencing, &client_file_id, ref);

   fw = eina_hash_find(file_watch, entry->path);
   if (!fw)
     {
        fw = calloc(1, sizeof(File_Watch));
        fw->path = eina_stringshare_add(entry->path);
        cserve2_file_change_watch_add(fw->path, _file_changed_cb, fw);
        eina_hash_direct_add(file_watch, fw->path, fw);
     }
   fw->entries = eina_list_append(fw->entries, entry);
   entry->watcher = fw;

   _request_add(&open_requests, (Entry *)entry, ref, rid, CSERVE2_OPEN);

   // _open_image_default_set(entry);

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

int
cserve2_cache_image_opts_set(Client *client, Msg_Setopts *msg)
{
   Image_Data *entry;
   File_Data *fentry = NULL;
   Reference *ref, *oldref;
   unsigned int image_id;
   char buf[4096];

   oldref = eina_hash_find(client->images.referencing, &msg->image_id);

   // search whether the image is already loaded by another client
   entry = _image_msg_new(client, msg);
   if (!entry)
     return -1;
   image_id = _img_opts_id_get(entry, buf, sizeof(buf));
   if (image_id)
     {  // if so, just update the references
        free(entry);
        DBG("found image_id %d for client image id %d",
            image_id, msg->image_id);
        entry = eina_hash_find(image_entries, &image_id);
        if (!entry)
          {
             ERR("image id %d is in file_ids hash, but not in entries hash"
                 "with entry id %d.", msg->image_id, image_id);
             cserve2_client_error_send(client, msg->base.rid,
                                       CSERVE2_INVALID_CACHE);
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

        ref = _entry_reference_add((Entry *)entry, client, msg->image_id);

        if (oldref)
          eina_hash_del_by_key(client->images.referencing, &msg->image_id);

        eina_hash_add(client->images.referencing, &msg->image_id, ref);

        return 0;
     }

   image_id = _image_id++;
   while ((image_id == 0) || (eina_hash_find(image_entries, &image_id)))
     image_id = _image_id++;

   entry->base.id = image_id;
   eina_hash_add(image_entries, &image_id, entry);
   eina_hash_add(image_ids, buf, (void *)image_id);
   ref = _entry_reference_add((Entry *)entry, client, msg->image_id);

   if (oldref)
     eina_hash_del_by_key(client->images.referencing, &msg->image_id);
   eina_hash_add(client->images.referencing, &msg->image_id, ref);

   fentry = entry->file;
   fentry->images = eina_list_append(fentry->images, entry);

   _request_add(&spload_requests, (Entry *)entry, ref, msg->base.rid,
                CSERVE2_SETOPTS);
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
        _request_answer_add(entry->base.request, ref, rid, CSERVE2_LOAD);
        if ((!entry->base.request->processing) && (!entry->doload))
          {
             DBG("Removing entry %d from speculative preload and adding "
                 "to normal load queue.", entry->base.id);
             spload_requests = eina_list_remove(spload_requests,
                                                entry->base.request);
             load_requests = eina_list_append(load_requests,
                                              entry->base.request);
          }
     }
   else if (entry->shm)
     _image_loaded_send(client, entry, rid);
   else
     _request_add(&load_requests, (Entry *)entry, ref, rid, CSERVE2_LOAD);

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
        _request_answer_add(entry->base.request, ref, rid, CSERVE2_PRELOAD);
        if ((!entry->base.request->processing) && (!entry->doload))
          {
             DBG("Removing entry %d from speculative preload and adding "
                 "to normal (pre)load queue.", entry->base.id);
             spload_requests = eina_list_remove(spload_requests,
                                                entry->base.request);
             load_requests = eina_list_append(load_requests,
                                              entry->base.request);
          }
     }
   else if (entry->shm)
     _image_preloaded_send(client, rid);
   else
     _request_add(&load_requests, (Entry *)entry, ref, rid, CSERVE2_PRELOAD);

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
cserve2_cache_font_load(Client *client, const char *source, unsigned int sourcelen, const char *name, unsigned int namelen, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int rid)
{
   Reference *ref;
   Font_Source *fs;
   Font_Entry *fe;
   char *fullname;

   if (sourcelen == 0)
     source = NULL;
   if (namelen == 0)
     name = NULL;

   fullname = _file_path_join(source, name);
   fe = _cserve2_font_entry_find(fullname, strlen(fullname) + 1, size,
                                 rend_flags, dpi);
   if (fe)
     {
        DBG("found font entry %s, rendflags: %d, size: %d, dpi: %d",
            name, rend_flags, size, dpi);

        ref = _entry_reference_add((Entry *)fe, client, 0);
        client->fonts.referencing = eina_list_append(
           client->fonts.referencing, ref);

        _entry_load_reused(&fe->base);
        fe->unused = EINA_FALSE;

        if (fe->request)
          cserve2_request_waiter_add(fe->request, rid, client);
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
        fs = calloc(1, sizeof(*fs));
        if (source)
          {
             fs->key = eina_stringshare_add(fullname);
             fs->name = eina_stringshare_add_length(name, namelen);
             fs->file = eina_stringshare_add_length(source, sourcelen);
          }
        else
          {
             fs->file = eina_stringshare_add_length(name, namelen);
             fs->key = eina_stringshare_ref(fs->file);
          }
        eina_hash_direct_add(font_sources, fs->key, fs);
     }

   fe->src = fs;
   fs->references++;
   DBG("adding FONT_LOAD '%s' request.", fs->name);
   fe->request = cserve2_request_add(CSERVE2_REQ_FONT_LOAD, rid,
                                     client, &_font_load_funcs, fe);

   eina_hash_direct_add(font_entries, fe, fe);

   free(fullname);

   return 0;
}

int
cserve2_cache_font_unload(Client *client, const char *source, unsigned int sourcelen, const char *name, unsigned int namelen, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int rid __UNUSED__)
{
   Font_Entry *fe;
   char *fullname;

   if (sourcelen == 0)
     source = NULL;
   if (namelen == 0)
     name = NULL;

   fullname = _file_path_join(source, name);
   fe = _cserve2_font_entry_find(fullname, strlen(fullname) + 1, size,
                                 rend_flags, dpi);
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
cserve2_cache_font_glyphs_load(Client *client, const char *source, unsigned int sourcelen, const char *name, unsigned int namelen, unsigned int hint, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int *glyphs, unsigned int nglyphs, unsigned int rid)
{
   Glyphs_Request *req;

   req = _glyphs_request_create(client, source, sourcelen, name, namelen,
                                hint, rend_flags, size, dpi, glyphs, nglyphs);
   if (!req)
     {
        free(glyphs);
        return -1;
     }

   if (_glyphs_request_check(req))
     {
        INF("Glyphs already loaded. Sending answer.");
        _glyphs_loaded_send(req, rid);
        _glyphs_request_free(req);
     }
   else
     {
        cserve2_request_add(CSERVE2_REQ_FONT_GLYPHS_LOAD, rid,
                            client, &_glyphs_load_funcs, req);
     }
   return 0;
}

int
cserve2_cache_font_glyphs_used(Client *client, const char *source, unsigned int sourcelen, const char *name, unsigned int namelen, unsigned int hint, unsigned int rend_flags, unsigned int size, unsigned int dpi, unsigned int *glyphs, unsigned int nglyphs, unsigned int rid __UNUSED__)
{
   Glyphs_Group *gg;
   Eina_List *groups;
   Glyphs_Request *req;

   DBG("Received report of used glyphs from client %d", client->id);
   req = _glyphs_request_create(client, source, sourcelen, name, namelen,
                                hint, rend_flags, size, dpi, glyphs, nglyphs);
   if (!req)
     {
        free(glyphs);
        return 0;
     }

   _glyphs_request_check(req);
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
cserve2_cache_requests_response(Slave_Command type, void *msg, void *data)
{
   Request *req = data;

   if (!req->entry)
     {
        Waiter *w;
        DBG("Request finished but it has no entry anymore.");
        EINA_LIST_FREE(req->waiters, w)
          {
             cserve2_client_error_send(w->ref->client, w->rid,
                                       CSERVE2_REQUEST_CANCEL);

             w->ref->count--;
             free(w);
          }
     }
   else if (type == ERROR)
     {
        Error_Type *error = msg;
        req->funcs->error(req->entry, *error);
     }
   else
     req->funcs->response(req->entry, msg);

   if (req->entry)
     req->entry->request = NULL;
   free(req);
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
