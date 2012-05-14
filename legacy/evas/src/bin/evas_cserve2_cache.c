#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <string.h>

#include "evas_cserve2.h"

typedef struct _Request Request;
typedef struct _Entry Entry;
typedef struct _Reference Reference;
typedef struct _Waiter Waiter;
typedef struct _File_Data File_Data;
typedef struct _Image_Data Image_Data;
typedef struct _File_Watch File_Watch;

typedef enum {
   CSERVE2_IMAGE_FILE,
   CSERVE2_IMAGE_DATA
} Entry_Type;

struct _Request {
   Entry *entry;
   Eina_List *waiters;
   Eina_Bool processing;
};

struct _File_Data {
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
   unsigned int file_id;
   Entry *file;
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

struct _Entry {
   unsigned int id;
   Eina_List *references;
   Request *request;
   Entry_Type type;
   union {
      File_Data file;
      Image_Data image;
   };
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

static unsigned int _file_id = 0; // id unique number
static unsigned int _image_id = 0; // id unique number
static Eina_Hash *file_ids = NULL; // maps path + key --> file_id
static Eina_Hash *file_entries = NULL; // maps file_id --> entry
static Eina_List *open_requests = NULL;

static Eina_Hash *image_ids = NULL; // maps file id + load opts --> image id
static Eina_Hash *image_entries = NULL; // maps image_id --> entry
static Eina_List *load_requests = NULL;
static Eina_List *spload_requests = NULL; // speculative preload requests

static Eina_Hash *file_watch = NULL;

static Eina_List *image_entries_lru = NULL;

static int max_unused_mem_usage = 5 * 1024; /* in kbytes */
static int unused_mem_usage = 0;

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
_image_entry_size_get(Entry *e)
{
   int size = sizeof(Entry);
   /* XXX: get the overhead of the shm handler too */
   if (e->image.shm)
     size += cserve2_shm_size_get(e->image.shm);
   return size / 1024;
}

static void
_file_id_free(Entry *entry)
{
   char buf[4096];

   DBG("Removing entry file id: %d, file: \"%s:%s\"",
       entry->id, entry->file.path, entry->file.key);
   snprintf(buf, sizeof(buf), "%s:%s", entry->file.path, entry->file.key);
   eina_hash_del_by_key(file_ids, buf);
}

static void
_image_id_free(Entry *entry)
{
   char buf[4096];

   DBG("Removing entry image id: %d", entry->id);

   _img_opts_id_get(&entry->image, buf, sizeof(buf));
   eina_hash_del_by_key(image_ids, buf);
}

static void
_image_entry_free(Entry *entry)
{
   Entry *fentry = entry->image.file;

   if (entry->request)
     {
        if (entry->request->processing)
          entry->request->entry = NULL;
        else if (!entry->request->waiters)
          {
             if (entry->image.doload)
               load_requests = eina_list_remove(load_requests, entry->request);
             else
               spload_requests = eina_list_remove(spload_requests,
                                                  entry->request);
          }
     }

   if (entry->image.unused)
     {
        image_entries_lru = eina_list_remove(image_entries_lru, entry);
        unused_mem_usage -= _image_entry_size_get(entry);
     }

   if (fentry)
     fentry->file.images = eina_list_remove(fentry->file.images, entry);
   if (entry->image.shm)
     cserve2_shm_unref(entry->image.shm);
   free(entry);
}

static void
_hash_image_entry_free(void *data)
{
   Entry *entry = data;

   _image_id_free(entry);
   _image_entry_free(entry);
}

static void
_file_entry_free(Entry *entry)
{
   File_Watch *fw;

   // Should we call free for each of the images too?
   // If everything goes fine, it's not necessary.
   if (entry->file.images)
     {
        ERR("Freeing file %d (\"%s:%s\") image data still referenced.",
            entry->id, entry->file.path, entry->file.key);
        eina_list_free(entry->file.images);
     }

   if (entry->request)
     {
        if (entry->request->processing)
          entry->request->entry = NULL;
        else if (!entry->request->waiters)
          {
             open_requests = eina_list_remove(open_requests, entry->request);
             free(entry->request);
          }
     }

   if ((fw = entry->file.watcher))
     {
        fw->entries = eina_list_remove(fw->entries, entry);
        if (!fw->entries)
          eina_hash_del_by_key(file_watch, fw->path);
     }

   free(entry->file.key);
   free(entry->file.path);
   eina_stringshare_del(entry->file.loader_data);
   free(entry);
}

static void
_hash_file_entry_free(void *data)
{
   Entry *entry = data;
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

void
cserve2_cache_init(void)
{
   file_ids = eina_hash_string_superfast_new(NULL);
   file_entries = eina_hash_int32_new(_hash_file_entry_free);
   image_ids = eina_hash_string_superfast_new(NULL);
   image_entries = eina_hash_string_superfast_new(_hash_image_entry_free);
   file_watch = eina_hash_string_superfast_new(_file_watch_free);
}

void
cserve2_cache_shutdown(void)
{
   eina_hash_free(image_entries);
   eina_hash_free(image_ids);
   eina_hash_free(file_entries);
   eina_hash_free(file_ids);
   eina_hash_free(file_watch);
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
   if (!req->waiters && !req->entry)
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

/*
static void
_open_request_del(Request *req)
{
   Waiter *it;

   EINA_LIST_FREE(req->waiters, it)
      free(it);

   req->entry->request = NULL;

   open_requests = eina_list_remove(open_requests, req);
   free(req);
}
*/

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
   Slave_Msg_Image_Open msg;
   Entry *entry;
   char slave_cmd_data[4096];
   int slave_cmd_size;
   int path_len, key_len;

   while ((nloaders > 0) && (open_requests))
     {

        // remove the first element from the list and process this element
        req = eina_list_data_get(open_requests);
        open_requests = eina_list_remove_list(open_requests, open_requests);

        entry = req->entry;
        DBG("Processing OPEN request for file entry: %d", entry->id);

        memset(&msg, 0, sizeof(msg));
        memcpy(slave_cmd_data, &msg, sizeof(msg));

        path_len = strlen(entry->file.path) + 1;
        key_len = strlen(entry->file.key) + 1;
        slave_cmd_size = sizeof(msg) + path_len + key_len;
        memcpy(slave_cmd_data + sizeof(msg), entry->file.path, path_len);
        memcpy(slave_cmd_data + sizeof(msg) + path_len, entry->file.key,
               key_len);
        cserve2_slave_cmd_dispatch(req, IMAGE_OPEN, slave_cmd_data,
                                   slave_cmd_size);

        req->processing = EINA_TRUE;
        nloaders--;
     }

   return nloaders;
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
_image_loaded_send(Client *client, Entry *entry, unsigned int rid)
{
   int size;
   const char *shmpath = cserve2_shm_name_get(entry->image.shm);
   Msg_Loaded msg;
   int path_len;
   char *buf;

   DBG("Sending LOADED reply for entry %d and RID: %d.", entry->id, rid);
   path_len = strlen(shmpath) + 1;

   memset(&msg, 0, sizeof(msg));
   msg.base.rid = rid;
   msg.base.type = CSERVE2_LOADED;

   msg.shm.mmap_offset = cserve2_shm_map_offset_get(entry->image.shm);
   msg.shm.use_offset = cserve2_shm_offset_get(entry->image.shm);
   msg.shm.mmap_size = cserve2_shm_map_size_get(entry->image.shm);
   msg.shm.image_size = cserve2_shm_size_get(entry->image.shm);
   msg.alpha_sparse = entry->image.alpha_sparse;

   buf = malloc(sizeof(msg) + path_len);

   memcpy(buf, &msg, sizeof(msg));
   memcpy(buf + sizeof(msg), shmpath, path_len);

   size = sizeof(msg) + path_len;

   cserve2_client_send(client, &size, sizeof(size));
   cserve2_client_send(client, buf, size);

   free(buf);
}

static void
_cserve2_cache_load_request_run(Request *req)
{
   Entry *fentry;
   Shm_Handle *shm;
   const char *shmpath, *file, *key, *loader;
   int shmlen, filelen, keylen, loaderlen;
   Slave_Msg_Image_Load msg;
   char *buf, *cur;
   Entry *ientry;
   size_t size;

   ientry = req->entry;

   fentry = ientry->image.file;
   // opening shm for this file
   shm = cserve2_shm_request(fentry->file.w * fentry->file.h * 4);
   shmpath = cserve2_shm_name_get(shm);
   shmlen = strlen(shmpath) + 1;

   file = fentry->file.path;
   filelen = strlen(file) + 1;

   key = fentry->file.key;
   keylen = strlen(key) + 1;

   loader = fentry->file.loader_data;
   if (!loader)
     loaderlen = 0;
   else
     loaderlen = strlen(loader) + 1;

   memset(&msg, 0, sizeof(msg));
   msg.w = ientry->image.file->file.w;
   msg.h = ientry->image.file->file.h;
   msg.alpha = ientry->image.file->file.alpha;
   msg.opts.w = ientry->image.opts.w;
   msg.opts.h = ientry->image.opts.h;
   msg.opts.rx = ientry->image.opts.rx;
   msg.opts.ry = ientry->image.opts.ry;
   msg.opts.rw = ientry->image.opts.rw;
   msg.opts.rh = ientry->image.opts.rh;
   msg.opts.scale_down_by = ientry->image.opts.scale_down;
   msg.opts.dpi = ientry->image.opts.dpi;
   msg.opts.orientation = ientry->image.opts.orientation;

   msg.shm.mmap_offset = cserve2_shm_map_offset_get(shm);
   msg.shm.image_offset = cserve2_shm_offset_get(shm);
   msg.shm.mmap_size = cserve2_shm_map_size_get(shm);
   msg.shm.image_size = cserve2_shm_size_get(shm);

   msg.has_loader_data = !!loaderlen;

   size = sizeof(msg) + shmlen + filelen + keylen + loaderlen;

   buf = calloc(1, size);

   memcpy(buf, &msg, sizeof(msg));
   cur = buf + sizeof(msg);
   memcpy(cur, shmpath, shmlen);

   cur += shmlen;
   memcpy(cur, file, filelen);

   cur += filelen;
   memcpy(cur, key, keylen);

   cur += keylen;
   memcpy(cur, loader, loaderlen);

   ientry->image.shm = shm;

   cserve2_slave_cmd_dispatch(req, IMAGE_LOAD, buf, size);

   free(buf);
}

static int
_cserve2_cache_load_requests_list_process(Eina_List **queue, int nloaders)
{
   Eina_List *skipped = NULL;
   Request *req;

   while ((nloaders > 0) && (*queue))
     {
        // remove the first element from the list and process this element
        req = eina_list_data_get(*queue);
        *queue = eina_list_remove_list(*queue, *queue);

        if (!req->entry->image.file)
          {
             ERR("File entry doesn't exist for entry id %d", req->entry->id);
             cserve2_cache_request_failed(req, CSERVE2_INVALID_CACHE);
             continue;
          }

        if (req->entry->image.file->request)
          {
             /* OPEN still pending, skip this request */
             skipped = eina_list_append(skipped, req);
             continue;
          }

        DBG("Processing LOAD request for image entry: %d", req->entry->id);

        _cserve2_cache_load_request_run(req);

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
_entry_unused_push(Entry *e)
{
   int size = _image_entry_size_get(e);

   if ((size > max_unused_mem_usage) || !(e->image.doload))
     {
        eina_hash_del_by_key(image_entries, &e->id);
        return;
     }
   while (size > (max_unused_mem_usage - unused_mem_usage))
     {
        Entry *ie = eina_list_data_get(eina_list_last(image_entries_lru));
        eina_hash_del_by_key(image_entries, &ie->id);
     }
   image_entries_lru = eina_list_append(image_entries_lru, e);
   e->image.unused = EINA_TRUE;
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
        if (entry->file.invalid)
          _file_entry_free(entry);
        else
          {
             if (entry->file.images)
               {
                  Entry *ie;
                  EINA_LIST_FREE(entry->file.images, ie)
                     ie->image.file = NULL;
                  entry->file.images = NULL;
               }
             eina_hash_del_by_key(file_entries, &entry->id);
          }
     }
   else if (entry->type == CSERVE2_IMAGE_DATA)
     {
        if (!entry->image.file)
          eina_hash_del_by_key(image_entries, &entry->id);
        else if (entry->image.file->file.invalid)
          _image_entry_free(entry);
        else
          _entry_unused_push(entry);
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
             if (entry->image.doload)
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

void
_image_opened_send(Client *client, Entry *entry, unsigned int rid)
{
    int size;
    Msg_Opened msg;

    DBG("Sending OPENED reply for entry: %d and RID: %d.", entry->id, rid);
    // clear the struct with possible paddings, since it is not aligned.
    memset(&msg, 0, sizeof(msg));
    msg.base.rid = rid;
    msg.base.type = CSERVE2_OPENED;
    msg.image.w = entry->file.w;
    msg.image.h = entry->file.h;
    msg.image.frame_count = entry->file.frame_count;
    msg.image.loop_count = entry->file.loop_count;
    msg.image.loop_hint = entry->file.loop_hint;
    msg.image.alpha = entry->file.alpha;

    size = sizeof(msg);
    cserve2_client_send(client, &size, sizeof(size));
    cserve2_client_send(client, &msg, sizeof(msg));
    // _cserve2_cache_load_requests_process();
}

static Entry *
_image_msg_new(Client *client, Msg_Setopts *msg)
{
   Reference *ref;
   Entry *im_entry;

   ref = eina_hash_find(client->files.referencing, &msg->file_id);
   if (!ref)
     {
        ERR("Couldn't find file id: %d, for image id: %d",
            msg->file_id, msg->image_id);
        cserve2_client_error_send(client, msg->base.rid,
                                  CSERVE2_INVALID_CACHE);
        return NULL;
     }
   if (ref->entry->file.invalid)
     {
        cserve2_client_error_send(client, msg->base.rid,
                                  CSERVE2_FILE_CHANGED);
        return NULL;
     }

   im_entry = calloc(1, sizeof(*im_entry));
   im_entry->type = CSERVE2_IMAGE_DATA;
   im_entry->image.file_id = ref->entry->id;
   im_entry->image.file = ref->entry;
   im_entry->image.opts.dpi = msg->opts.dpi;
   im_entry->image.opts.w = msg->opts.w;
   im_entry->image.opts.h = msg->opts.h;
   im_entry->image.opts.scale_down = msg->opts.scale_down;
   im_entry->image.opts.rx = msg->opts.rx;
   im_entry->image.opts.ry = msg->opts.ry;
   im_entry->image.opts.rw = msg->opts.rw;
   im_entry->image.opts.rh = msg->opts.rh;
   im_entry->image.opts.orientation = msg->opts.orientation;

   return im_entry;
}

/*
static Entry *
_image_default_new(Entry *file_entry)
{
   Entry *im_entry = calloc(1, sizeof(*im_entry));
   im_entry->image.file_id = file_entry->id;
   im_entry->image.opts.dpi = -1;
   im_entry->image.opts.w = -1;
   im_entry->image.opts.h = -1;
   im_entry->image.opts.scale_down = -1;
   im_entry->image.opts.rx = -1;
   im_entry->image.opts.ry = -1;
   im_entry->image.opts.rw = -1;
   im_entry->image.opts.rh = -1;
   im_entry->image.opts.orientation = 0;

   return im_entry;
}
*/

static void
_file_changed_cb(const char *path __UNUSED__, Eina_Bool deleted __UNUSED__, void *data)
{
   File_Watch *fw = data;
   Entry *e;
   Eina_List *l;

   EINA_LIST_FOREACH(fw->entries, l, e)
     {
        Eina_List *ll;
        Entry *ie;

        e->file.invalid = EINA_TRUE;
        e->file.watcher = NULL;

        EINA_LIST_FOREACH(e->file.images, ll, ie)
          {
             _image_id_free(ie);
             eina_hash_set(image_entries, &ie->id, NULL);
             if (ie->request && !ie->request->processing)
               {
                  if (ie->image.doload)
                    _request_answer_all_del(&load_requests, ie->request,
                                            CSERVE2_FILE_CHANGED);
                  else
                    _request_answer_all_del(&spload_requests, ie->request,
                                            CSERVE2_FILE_CHANGED);
               }
             ie->request = NULL;
             if (ie->image.unused)
               _image_entry_free(ie);
          }

        _file_id_free(e);
        eina_hash_set(file_entries, &e->id, NULL);
        if (e->request && !e->request->processing)
          _request_answer_all_del(&open_requests, ie->request,
                                  CSERVE2_FILE_CHANGED);
        e->request = NULL;
        if (!e->file.images && !e->references)
          _file_entry_free(e);
     }

   eina_hash_del_by_key(file_watch, fw->path);
}

int
cserve2_cache_file_open(Client *client, unsigned int client_file_id, const char *path, const char *key, unsigned int rid)
{
   unsigned int file_id;
   Entry *entry;
   Reference *ref;
   File_Watch *fw;
   char buf[4906];

   // look for this file on client references
   ref = eina_hash_find(client->files.referencing, &client_file_id);
   if (ref)
     {
        entry = ref->entry;

        if (entry->file.invalid)
          {
             cserve2_client_error_send(client, rid, CSERVE2_FILE_CHANGED);
             return -1;
          }

        DBG("found client file id: %d", client_file_id);
        ref->count++;

        // File already being loaded, just add the request to be replied
        if (entry->request)
          _request_answer_add(entry->request, ref, rid, CSERVE2_OPEN);
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
        ref = _entry_reference_add(entry, client, client_file_id);
        eina_hash_add(client->files.referencing, &client_file_id, ref);
        if (entry->request)
          _request_answer_add(entry->request, ref, rid, CSERVE2_OPEN);
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
   entry->type = CSERVE2_IMAGE_FILE;
   entry->file.path = strdup(path);
   entry->file.key = strdup(key);
   entry->id = file_id;
   eina_hash_add(file_entries, &file_id, entry);
   eina_hash_add(file_ids, buf, (void *)file_id);
   ref = _entry_reference_add(entry, client, client_file_id);
   eina_hash_add(client->files.referencing, &client_file_id, ref);

   fw = eina_hash_find(file_watch, entry->file.path);
   if (!fw)
     {
        fw = calloc(1, sizeof(File_Watch));
        fw->path = eina_stringshare_add(entry->file.path);
        cserve2_file_change_watch_add(fw->path, _file_changed_cb, fw);
        eina_hash_direct_add(file_watch, fw->path, fw);
     }
   fw->entries = eina_list_append(fw->entries, entry);
   entry->file.watcher = fw;

   _request_add(&open_requests, entry, ref, rid, CSERVE2_OPEN);

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
   Entry *entry;
   Entry *fentry = NULL;
   Reference *ref, *oldref;
   unsigned int image_id;
   char buf[4096];

   oldref = eina_hash_find(client->images.referencing, &msg->image_id);

   // search whether the image is already loaded by another client
   entry = _image_msg_new(client, msg);
   if (!entry)
     return -1;
   image_id = _img_opts_id_get(&entry->image, buf, sizeof(buf));
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

        if (entry->image.unused)
          {
             DBG("Re-using old image entry (id: %d) from the LRU list.",
                 entry->id);
             entry->image.unused = EINA_FALSE;
             image_entries_lru = eina_list_remove(image_entries_lru, entry);
             unused_mem_usage -= _image_entry_size_get(entry);
          }

        if (oldref && (oldref->entry->id == image_id))
          return 0;

        ref = _entry_reference_add(entry, client, msg->image_id);

        if (oldref)
          eina_hash_del_by_key(client->images.referencing, &msg->image_id);

        eina_hash_add(client->images.referencing, &msg->image_id, ref);

        return 0;
     }

   image_id = _image_id++;
   while ((image_id == 0) || (eina_hash_find(image_entries, &image_id)))
     image_id = _image_id++;

   entry->id = image_id;
   eina_hash_add(image_entries, &image_id, entry);
   eina_hash_add(image_ids, buf, (void *)image_id);
   ref = _entry_reference_add(entry, client, msg->image_id);

   if (oldref)
     eina_hash_del_by_key(client->images.referencing, &msg->image_id);
   eina_hash_add(client->images.referencing, &msg->image_id, ref);

   fentry = entry->image.file;
   fentry->file.images = eina_list_append(fentry->file.images, entry);

   _request_add(&spload_requests, entry, ref, msg->base.rid, CSERVE2_SETOPTS);
   return 0;
}

void
cserve2_cache_image_load(Client *client, unsigned int client_image_id, unsigned int rid)
{
   Entry *entry;
   Reference *ref;

   ref = eina_hash_find(client->images.referencing, &client_image_id);
   if (!ref)
     {
        ERR("Can't load: client %d has no image id %d",
            client->id, client_image_id);
        return;
     }
   if (ref->entry->image.file->file.invalid)
     {
        cserve2_client_error_send(client, rid, CSERVE2_FILE_CHANGED);
        return;
     }

   DBG("Loading image id: %d", ref->entry->id);

   entry = ref->entry;

   // File already being loaded, just add the request to be replied
   if (entry->request)
     {
        _request_answer_add(entry->request, ref, rid, CSERVE2_LOAD);
        if ((!entry->request->processing) && (!entry->image.doload))
          {
             DBG("Removing entry %d from speculative preload and adding "
                 "to normal load queue.", entry->id);
             spload_requests = eina_list_remove(spload_requests,
                                                entry->request);
             load_requests = eina_list_append(load_requests, entry->request);
          }
     }
   else if (entry->image.shm)
     _image_loaded_send(client, entry, rid);
   else
     _request_add(&load_requests, entry, ref, rid, CSERVE2_LOAD);

   entry->image.doload = EINA_TRUE;
}

void
cserve2_cache_image_preload(Client *client, unsigned int client_image_id, unsigned int rid)
{
   Entry *entry;
   Reference *ref;

   ref = eina_hash_find(client->images.referencing, &client_image_id);
   if (!ref)
     {
        ERR("Can't load: client %d has no image id %d",
            client->id, client_image_id);
        return;
     }
   if (ref->entry->image.file->file.invalid)
     {
        cserve2_client_error_send(client, rid, CSERVE2_FILE_CHANGED);
        return;
     }

   DBG("Loading image id: %d", ref->entry->id);

   entry = ref->entry;

   // File already being loaded, just add the request to be replied
   if (entry->request)
     {
        _request_answer_add(entry->request, ref, rid, CSERVE2_PRELOAD);
        if ((!entry->request->processing) && (!entry->image.doload))
          {
             DBG("Removing entry %d from speculative preload and adding "
                 "to normal (pre)load queue.", entry->id);
             spload_requests = eina_list_remove(spload_requests,
                                                entry->request);
             load_requests = eina_list_append(load_requests, entry->request);
          }
     }
   else if (entry->image.shm)
     _image_preloaded_send(client, rid);
   else
     _request_add(&load_requests, entry, ref, rid, CSERVE2_PRELOAD);

   entry->image.doload = EINA_TRUE;
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
cserve2_cache_request_opened(Slave_Msg_Image_Opened *resp, void *data)
{
   Waiter *w;
   Request *req = data;
   Entry *entry;

   entry = req->entry;
   if (!entry)
     {
        Error_Type err = entry->file.invalid
           ? CSERVE2_FILE_CHANGED
           : CSERVE2_REQUEST_CANCEL;
        DBG("File entry for request doesn't exist anymore.");
        cserve2_cache_request_failed(req, err);
        entry->request = NULL;
        return;
     }

   entry->request = NULL;

   entry->file.w = resp->w;
   entry->file.h = resp->h;
   entry->file.frame_count = resp->frame_count;
   entry->file.loop_count = resp->loop_count;
   entry->file.loop_hint = resp->loop_hint;
   entry->file.alpha = resp->alpha;
   if (resp->has_loader_data)
     {
        const char *ldata = (const char *)resp +
                                           sizeof(Slave_Msg_Image_Opened);
        entry->file.loader_data = eina_stringshare_add(ldata);
     }

   DBG("Finished opening file %d. Notifying %d waiters.", entry->id,
       req->waiters ? eina_list_count(req->waiters) : 0);
   EINA_LIST_FREE(req->waiters, w)
     {
        _image_opened_send(w->ref->client, entry, w->rid);
        free(w);
     }

   free(req);
}

void
cserve2_cache_request_loaded(Slave_Msg_Image_Loaded *resp, void *data)
{
   Waiter *w;
   Request *req = data;
   Entry *entry;

   entry = req->entry;
   if (!entry)
     {
        // FIXME: Wouldn't we keep the entry alive when the file changed
        // until we send the errors needed and just then delete it? Right now
        // the check below just makes no sense.
        // Error_Type err = entry->image.file->file.invalid
        //    ? CSERVE2_FILE_CHANGED
        //    : CSERVE2_REQUEST_CANCEL;
        DBG("Image entry for request doesn't exist anymore.");
        cserve2_cache_request_failed(req, CSERVE2_REQUEST_CANCEL);
        entry->request = NULL;
        return;
     }

   entry->request = NULL;
   entry->image.alpha_sparse = resp->alpha_sparse;
   if (!entry->image.doload)
     DBG("Entry %d loaded by speculative preload.", entry->id);

   DBG("Finished loading image %d. Notifying %d waiters.", entry->id,
       req->waiters ? eina_list_count(req->waiters) : 0);
   EINA_LIST_FREE(req->waiters, w)
     {
        if (w->type == CSERVE2_LOAD)
          _image_loaded_send(w->ref->client, entry, w->rid);
        else if (w->type == CSERVE2_PRELOAD)
          _image_preloaded_send(w->ref->client, w->rid);
        // else w->type == CSERVE2_SETOPTS --> do nothing

        free(w);
     }

   free(req);
}

void
cserve2_cache_request_failed(void *data, Error_Type type)
{
   Waiter *w;
   Request *req = data;
   Entry *entry;
   Eina_List *l;
   Reference *ref;

   DBG("Request for entry %p failed with error %d", req->entry, type);
   EINA_LIST_FREE(req->waiters, w)
     {
        cserve2_client_error_send(w->ref->client, w->rid, type);

        w->ref->count--;
        free(w);
     }

   entry = req->entry;
   if (!entry)
     goto free_req;

   EINA_LIST_FOREACH(entry->references, l, ref)
     {
        Eina_Hash *hash = NULL;
        if (entry->type == CSERVE2_IMAGE_FILE)
          hash = ref->client->files.referencing;
        else if (entry->type == CSERVE2_IMAGE_DATA)
          hash = ref->client->images.referencing;

        eina_hash_del_by_key(hash, &(ref->client_entry_id));
     }

free_req:
   free(req);
}
