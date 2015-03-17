#include "evas_common_private.h"
#include "evas_private.h"
#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif
#include "evas_engine.h"
#include <sys/mman.h>

static void 
_evas_shm_surface_cb_frame(void *data, struct wl_callback *callback, uint32_t timestamp EINA_UNUSED)
{
   Shm_Surface *surf;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(surf = data)) return;
   if (callback != surf->frame_cb) return;

   wl_callback_destroy(callback);
   surf->frame_cb = NULL;
   surf->redraw = EINA_FALSE;
}

static const struct wl_callback_listener _frame_listener = 
{
   _evas_shm_surface_cb_frame
};

static struct wl_shm_pool *
_shm_pool_make(struct wl_shm *shm, int size, void **data)
{
   struct wl_shm_pool *pool;
   static const char tmp[] = "/evas-wayland_shm-XXXXXX";
   const char *path;
   char *name;
   int fd = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid wl_shm */
   if (!shm) return NULL;

   /* create tmp file name */
   if ((path = getenv("XDG_RUNTIME_DIR")))
     {
        if ((name = malloc(strlen(path) + sizeof(tmp))))
          strcpy(name, path);
     }
   else
     {
        if ((name = malloc(strlen("/tmp") + sizeof(tmp))))
          strcpy(name, "/tmp");
     }

   if (!name) return NULL;

   strcat(name, tmp);

   /* try to create tmp file */
   if ((fd = mkstemp(name)) < 0)
     {
        ERR("Could not create temporary file: %m");
        free(name);
        return NULL;
     }

   unlink(name);
   free(name);

   /* try to truncate file to size */
   if (ftruncate(fd, size) < 0)
     {
        ERR("Could not truncate temporary file: %m");
        goto fd_err;
     }

   /* try to mmap the file */
   *data = mmap(NULL, size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, 0);
   if (*data == MAP_FAILED)
     {
        ERR("Could not mmap temporary file: %m");
        goto fd_err;
     }

   /* create wl_shm_pool using fd */
   pool = wl_shm_create_pool(shm, fd, size);

   close(fd);

   return pool;

fd_err:
   close(fd);
   return NULL;
}

static Shm_Pool *
_shm_pool_create(struct wl_shm *shm, size_t size)
{
   Shm_Pool *pool;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(pool = malloc(sizeof(Shm_Pool)))) return NULL;

   pool->pool = _shm_pool_make(shm, size, &pool->data);
   if (!pool->pool) goto err;

   pool->size = size;
   pool->used = 0;

   return pool;

err:
   free(pool);
   return NULL;
}

static void 
_shm_pool_destroy(Shm_Pool *pool)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   munmap(pool->data, pool->size);
   wl_shm_pool_destroy(pool->pool);
   free(pool);
}

static void *
_shm_pool_allocate(Shm_Pool *pool, size_t size, int *offset)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if ((pool->used + size) > pool->size)
     return NULL;

   *offset = pool->used;
   pool->used += size;

   return (char *)pool->data + *offset;
}

static void 
_shm_pool_reset(Shm_Pool *pool)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   pool->used = 0;
}

static Shm_Data *
_shm_data_create_from_pool(Shm_Pool *pool, int w, int h, Eina_Bool alpha)
{
   Shm_Data *data;
   int len, offset;
   uint32_t wl_format = WL_SHM_FORMAT_XRGB8888;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* try to malloc space for data */
   if (!(data = malloc(sizeof(Shm_Data))))
     {
        ERR("Could not allocate space for data");
        return NULL;
     }

   len = (w * sizeof(int)) * h;
   data->pool = NULL;

   if (!(data->map = _shm_pool_allocate(pool, len, &offset)))
     {
        ERR("Could not map leaf data");
        free(data);
        return NULL;
     }

   if (alpha) 
     wl_format = WL_SHM_FORMAT_ARGB8888;

   data->buffer = 
     wl_shm_pool_create_buffer(pool->pool, offset, w, h, 
                               (w * sizeof(int)), wl_format);
   if (!data->buffer)
     {
        ERR("Could not create buffer from pool: %m");
        free(data);
        return NULL;
     }

   return data;
}

static void
_shm_data_create(Shm_Pool *alt_pool, Shm_Data **ret, Shm_Surface *surface, int w, int h)
{
   Shm_Pool *pool;
   Shm_Data *data;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (alt_pool)
     {
        _shm_pool_reset(alt_pool);
        if ((data = _shm_data_create_from_pool(alt_pool, w, h, surface->alpha)))
          goto out;
     }

   if (!(pool = _shm_pool_create(surface->shm, ((w * sizeof(int)) * h))))
     {
        ERR("Could not create shm pool");
        goto err;
     }

   if (!(data = _shm_data_create_from_pool(pool, w, h, surface->alpha)))
     {
        ERR("Could not create data from pool");
        _shm_pool_destroy(pool);
        goto err;
     }

   data->pool = pool;

out:
   if (ret) *ret = data;
   return;
err:
   if (ret) *ret = NULL;
}

static void 
_shm_data_destroy(Shm_Data *data)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (data->buffer) wl_buffer_destroy(data->buffer);
   if (data->pool) _shm_pool_destroy(data->pool);
   free(data);
}

static void 
_shm_leaf_release(Shm_Leaf *leaf)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (leaf->data) _shm_data_destroy(leaf->data);
   if (leaf->resize_pool) _shm_pool_destroy(leaf->resize_pool);
   memset(leaf, 0, sizeof(*leaf));
}

static void 
_shm_buffer_release(void *data, struct wl_buffer *buffer)
{
   Shm_Surface *surf;
   Shm_Leaf *leaf;
   int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   surf = data;

   for (; i < surf->num_buff; i++)
     {
        leaf = &surf->leaf[i];
        if ((leaf->data) && (leaf->data->buffer == buffer))
          {
             leaf->busy = 0;
             break;
          }
     }
}

static const struct wl_buffer_listener _shm_buffer_listener = 
{
   _shm_buffer_release
};

Shm_Surface *
_evas_shm_surface_create(struct wl_shm *shm, struct wl_surface *surface, int w, int h, Eina_Bool alpha)
{
   Shm_Surface *surf;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(surf = calloc(1, sizeof(Shm_Surface)))) return NULL;

   surf->dx = 0;
   surf->dy = 0;
   surf->w = w;
   surf->h = h;
   surf->shm = shm;
   surf->surface = surface;
   surf->alpha = alpha;
   surf->flags = 0;

   return surf;
}

void 
_evas_shm_surface_destroy(Shm_Surface *surface)
{
   int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   for (; i < surface->num_buff; i++)
     _shm_leaf_release(&surface->leaf[i]);

   free(surface);
}

void 
_evas_shm_surface_prepare(Shm_Surface *surface, int dx, int dy, int w, int h, int num_buff, uint32_t flags)
{
   Shm_Leaf *leaf = NULL;
   int i = 0, resize = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   resize = !!(flags & SURFACE_HINT_RESIZING);

   /* update surface properties */
   surface->w = w;
   surface->h = h;
   surface->dx = dx;
   surface->dy = dy;
   surface->num_buff = num_buff;

   for (; i < num_buff; i++)
     {
        if (surface->leaf[i].busy) continue;
        if ((!leaf) || (leaf->valid))
          {
             leaf = &surface->leaf[i];
             break;
          }
     }

   if (!leaf)
     {
        CRI("All buffers held by server");
        return;
     }

   if ((!resize) && (leaf->resize_pool))
     {
        _shm_data_destroy(leaf->data);
        leaf->data = NULL;

        _shm_pool_destroy(leaf->resize_pool);
        leaf->resize_pool = NULL;
     }

   if (leaf->valid)
     {
        if ((leaf->w == w) && (leaf->h == h)) goto out;
     }

   if (leaf->data) _shm_data_destroy(leaf->data);
   leaf->data = NULL;

   if ((resize) && (!leaf->resize_pool))
     {
        leaf->resize_pool = 
          _shm_pool_create(surface->shm, 6 * 1024 * 1024);
     }

   _shm_data_create(leaf->resize_pool, &leaf->data, surface, w, h);
   if (!leaf->data)
     {
        CRI("Failed to create leaf data");
        abort();
     }

   leaf->w = w;
   leaf->h = h;
   leaf->valid = EINA_TRUE;

   wl_buffer_add_listener(leaf->data->buffer, &_shm_buffer_listener, surface);

out:
   surface->current = leaf;
}

void 
_evas_shm_surface_swap(Shm_Surface *surface, Eina_Rectangle *rects, unsigned int count)
{
   Shm_Leaf *leaf;
   Eina_Rectangle *rect;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!(leaf = surface->current))
     {
        ERR("No Current Leaf");
        return;
     }

   if (!leaf->valid)
     {
        ERR("Leaf Not Valid");
        return;
     }

   rect = eina_rectangle_new(0, 0, 0, 0);
   if ((rects) && (count > 0))
     {
        unsigned int i = 0;

        for (; i < count; i++)
          eina_rectangle_union(rect, &rects[i]);
     }
   else
     {
        Eina_Rectangle r;

        r.x = 0; r.y = 0;
        r.w = leaf->w; r.h = leaf->h;

        eina_rectangle_union(rect, &r);
     }

   wl_surface_attach(surface->surface, leaf->data->buffer, 0, 0);
   wl_surface_damage(surface->surface, rect->x, rect->y, rect->w, rect->h);
   wl_surface_commit(surface->surface);

   eina_rectangle_free(rect);

   leaf->busy = 1;
   surface->dx = 0;
   surface->dy = 0;
   surface->redraw = EINA_TRUE;
}

void *
_evas_shm_surface_data_get(Shm_Surface *surface, int *bw, int *bh)
{
   Shm_Leaf *leaf;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (bw) *bw = 0;
   if (bh) *bh = 0;

   if (!(leaf = surface->current))
     {
        _evas_shm_surface_prepare(surface, 0, 0, surface->w, surface->h, 
                                  surface->num_buff, surface->flags);

        if (!(leaf = surface->current))
          {
             CRI("NO Current Surface");
             return NULL;
          }
     }

   if (bw) *bw = leaf->w;
   if (bh) *bh = leaf->h;

   return leaf->data->map;
}

void 
_evas_shm_surface_redraw(Shm_Surface *surface)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (surface->frame_cb)
     {
        if (!surface->redraw) return;
        wl_callback_destroy(surface->frame_cb);
     }

   if (!surface->surface) return;

   surface->frame_cb = wl_surface_frame(surface->surface);
   wl_callback_add_listener(surface->frame_cb, &_frame_listener, surface);
}
