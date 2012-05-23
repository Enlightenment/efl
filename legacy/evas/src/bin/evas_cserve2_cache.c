#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include "evas_cserve2.h"

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
typedef struct _Glyph_Entry Glyph_Entry;

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
   CSERVE2_IMAGE_DATA
} Entry_Type;

struct _Entry {
   unsigned int id;
   Eina_List *references;
   Request *request;
   Entry_Type type;
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
   const char *name;
   const char *file;
   int references;
};

struct _Font_Entry {
   unsigned int rend_flags;
   unsigned int hint;
   unsigned int size;
   unsigned int dpi;
   Font_Source *src;
};

struct _Font_Cache {
   Font_Entry *fe;
   struct {
      const char *name;
      void *data;
      unsigned int size;
      unsigned int usage;
   } shm;
   Eina_Inlist *glyphs;
};

struct _Glyph_Entry {
   EINA_INLIST;
   Font_Entry *fe;
   Font_Cache *fi;
   unsigned int index;
   unsigned int offset;
};

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

static int max_unused_mem_usage = 5 * 1024; /* in kbytes */
static int unused_mem_usage = 0;

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

        eina_hash_del_by_key(hash, &(ref->client_entry_id));
     }
}

static void
_open_request_response(File_Data *e, Slave_Msg_Image_Opened *resp)
{
   Waiter *w;

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
   return buf;
}

static void
_load_request_response(Image_Data *e, Slave_Msg_Image_Loaded *resp)
{
   Waiter *w;

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
     fentry->images = eina_list_remove(fentry->images, entry);
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
   if (k1->src->name == k2->src->name)
     {
        if (k1->size == k2->size)
          {
             if (k1->rend_flags == k2->rend_flags)
               {
                  if (k1->hint == k2->hint)
                    return k1->dpi - k2->dpi;
                  return k1->hint - k2->hint;
               }
             return k1->rend_flags - k2->rend_flags;
          }
        return k1->size - k2->size;
     }
   return strcmp(k1->src->name, k2->src->name);
}

static int
_font_entry_key_hash(const Font_Entry *key, int key_length __UNUSED__)
{
   int hash;
   hash = eina_hash_djb2(key->src->name, eina_stringshare_strlen(key->src->name) + 1);
   hash ^= eina_hash_int32(&key->rend_flags, sizeof(int));
   hash ^= eina_hash_int32(&key->size, sizeof(int));
   hash ^= eina_hash_int32(&key->dpi, sizeof(int));

   return hash;
}

static void
_font_entry_free(Font_Entry *fe)
{
   free(fe);
}

static void
_font_source_free(Font_Source *fs)
{
   if (fs->name) eina_stringshare_del(fs->name);
   if (fs->file) eina_stringshare_del(fs->file);

   free(fs);
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
        else
          {
             Image_Data *ie;
             EINA_LIST_FREE(fentry->images, ie)
                ie->file = NULL;
             eina_hash_del_by_key(file_entries, &entry->id);
          }
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
        else
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

void
cserve2_cache_client_new(Client *client)
{
   client->files.referencing = eina_hash_int32_new(_entry_free_cb);
   client->images.referencing = eina_hash_int32_new(_entry_free_cb);
}

void
cserve2_cache_client_del(Client *client)
{
   // will call _entry_free_cb() for every entry
   eina_hash_free(client->images.referencing);
   // will call _entry_free_cb() for every entry
   eina_hash_free(client->files.referencing);
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
