#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_engine.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <drm_fourcc.h>
#include <intel_bufmgr.h>
#include <i915_drm.h>

#include <exynos_drm.h>
#include <exynos_drmif.h>
#include <sys/mman.h>

#include "linux-dmabuf-unstable-v1-client-protocol.h"

#define SYM(lib, xx)               \
   do {                            \
      sym_## xx = dlsym(lib, #xx); \
      if (!(sym_ ## xx)) {         \
         fail = EINA_TRUE;         \
      }                            \
   } while (0)

static Eina_Bool dmabuf_totally_hosed;

static int drm_fd = -1;

typedef struct _Dmabuf_Surface Dmabuf_Surface;

typedef struct _Dmabuf_Buffer Dmabuf_Buffer;
typedef struct _Buffer_Handle Buffer_Handle;
typedef struct _Buffer_Manager Buffer_Manager;
struct _Buffer_Manager
{
   Buffer_Handle *(*alloc)(Buffer_Manager *self, const char *name, int w, int h, unsigned long *stride, int32_t *fd);
   void *(*map)(Dmabuf_Buffer *buf);
   void (*unmap)(Dmabuf_Buffer *buf);
   void (*discard)(Dmabuf_Buffer *buf);
   void *priv;
};

Buffer_Manager *buffer_manager = NULL;

struct _Dmabuf_Buffer
{
   Buffer_Manager *bm;
   Dmabuf_Surface *surface;
   struct wl_buffer *wl_buffer;
   int w, h;
   int age;
   unsigned long stride;
   Buffer_Handle *bh;
   int fd;
   void *mapping;

   int index;
   Eina_Bool locked : 1;
   Eina_Bool busy : 1;
   Eina_Bool used : 1;
   Eina_Bool pending : 1;
   Eina_Bool orphaned : 1;
};

struct _Dmabuf_Surface
{
   Surface *surface;
   struct wl_display *wl_display;
   struct zwp_linux_dmabuf_v1 *dmabuf;
   struct wl_surface *wl_surface;
   int compositor_version;

   Dmabuf_Buffer *current;
   Dmabuf_Buffer *pre;
   Dmabuf_Buffer **buffer;
   int nbuf;

   Eina_Bool alpha : 1;
};

static void _internal_evas_dmabuf_surface_destroy(Dmabuf_Surface *surface);
static void _evas_dmabuf_surface_destroy(Surface *s);
static Dmabuf_Buffer *_evas_dmabuf_buffer_init(Dmabuf_Surface *s, int w, int h);
static void _evas_dmabuf_buffer_destroy(Dmabuf_Buffer *b);

drm_intel_bufmgr *(*sym_drm_intel_bufmgr_gem_init)(int fd, int batch_size) = NULL;
int (*sym_drm_intel_gem_bo_unmap_gtt)(drm_intel_bo *bo) = NULL;
int (*sym_drm_intel_gem_bo_map_gtt)(drm_intel_bo *bo) = NULL;
drm_intel_bo *(*sym_drm_intel_bo_alloc_tiled)(drm_intel_bufmgr *mgr, const char *name, int x, int y, int cpp, uint32_t *tile, unsigned long *pitch, unsigned long flags) = NULL;
void (*sym_drm_intel_bo_unreference)(drm_intel_bo *bo) = NULL;
int (*sym_drmPrimeHandleToFD)(int fd, uint32_t handle, uint32_t flags, int *prime_fd);

struct exynos_device *(*sym_exynos_device_create)(int fd) = NULL;
struct exynos_bo *(*sym_exynos_bo_create)(struct exynos_device *dev, size_t size, uint32_t flags) = NULL;
void *(*sym_exynos_bo_map)(struct exynos_bo *bo) = NULL;
void (*sym_exynos_bo_destroy)(struct exynos_bo *bo) = NULL;

static Buffer_Handle *
_intel_alloc(Buffer_Manager *self, const char *name, int w, int h, unsigned long *stride, int32_t *fd)
{
   uint32_t tile = I915_TILING_NONE;
   drm_intel_bo *out;

   out = sym_drm_intel_bo_alloc_tiled(self->priv, name, w, h, 4, &tile,
                                       stride, 0);

   if (!out) return NULL;

   if (tile != I915_TILING_NONE) goto err;
   /* First try to allocate an mmapable buffer with O_RDWR,
    * if that fails retry unmappable - if the compositor is
    * using GL it won't need to mmap the buffer and this can
    * work - otherwise it'll reject this buffer and we'll
    * have to fall back to shm rendering.
    */
   if (sym_drmPrimeHandleToFD(drm_fd, out->handle,
                              DRM_CLOEXEC | O_RDWR, fd) != 0)
     if (sym_drmPrimeHandleToFD(drm_fd, out->handle,
                                DRM_CLOEXEC, fd) != 0) goto err;

   return (Buffer_Handle *)out;

err:
   sym_drm_intel_bo_unreference(out);
   return NULL;
}

static void *
_intel_map(Dmabuf_Buffer *buf)
{
   drm_intel_bo *bo;

   bo = (drm_intel_bo *)buf->bh;
   if (sym_drm_intel_gem_bo_map_gtt(bo) != 0) return NULL;
   return bo->virtual;
}

static void
_intel_unmap(Dmabuf_Buffer *buf)
{
   drm_intel_bo *bo;

   bo = (drm_intel_bo *)buf->bh;
   sym_drm_intel_gem_bo_unmap_gtt(bo);
}

static void
_intel_discard(Dmabuf_Buffer *buf)
{
   drm_intel_bo *bo;

   bo = (drm_intel_bo *)buf->bh;
   sym_drm_intel_bo_unreference(bo);
}

static Eina_Bool
_intel_buffer_manager_setup(int fd)
{
   Eina_Bool fail = EINA_FALSE;
   void *drm_intel_lib;

   drm_intel_lib = dlopen("libdrm_intel.so", RTLD_LAZY | RTLD_GLOBAL);
   if (!drm_intel_lib) return EINA_FALSE;

   SYM(drm_intel_lib, drm_intel_bufmgr_gem_init);
   SYM(drm_intel_lib, drm_intel_gem_bo_unmap_gtt);
   SYM(drm_intel_lib, drm_intel_gem_bo_map_gtt);
   SYM(drm_intel_lib, drm_intel_bo_alloc_tiled);
   SYM(drm_intel_lib, drm_intel_bo_unreference);
   SYM(drm_intel_lib, drmPrimeHandleToFD);

   if (fail) goto err;

   buffer_manager->priv = sym_drm_intel_bufmgr_gem_init(fd, 32);
   if (!buffer_manager->priv) goto err;

   buffer_manager->alloc = _intel_alloc;
   buffer_manager->map = _intel_map;
   buffer_manager->unmap = _intel_unmap;
   buffer_manager->discard = _intel_discard;

   return EINA_TRUE;

err:
   dlclose(drm_intel_lib);
   return EINA_FALSE;
}

static Buffer_Handle *
_exynos_alloc(Buffer_Manager *self, const char *name EINA_UNUSED, int w, int h, unsigned long *stride, int32_t *fd)
{
   size_t size = w * h * 4;
   struct exynos_bo *out;

   *stride = w * 4;
   out = sym_exynos_bo_create(self->priv, size, 0);
   if (!out) return NULL;
   /* First try to allocate an mmapable buffer with O_RDWR,
    * if that fails retry unmappable - if the compositor is
    * using GL it won't need to mmap the buffer and this can
    * work - otherwise it'll reject this buffer and we'll
    * have to fall back to shm rendering.
    */
   if (sym_drmPrimeHandleToFD(drm_fd, out->handle,
                              DRM_CLOEXEC | O_RDWR, fd) != 0)
     if (sym_drmPrimeHandleToFD(drm_fd, out->handle,
                                DRM_CLOEXEC, fd) != 0) goto err;

   return (Buffer_Handle *)out;

err:
   sym_exynos_bo_destroy(out);
   return NULL;
}

static void *
_exynos_map(Dmabuf_Buffer *buf)
{
   struct exynos_bo *bo;
   void *ptr;

   bo = (struct exynos_bo *)buf->bh;
   ptr = mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED, buf->fd, 0);
   if (ptr == MAP_FAILED) return NULL;
   return ptr;
}

static void
_exynos_unmap(Dmabuf_Buffer *buf)
{
   struct exynos_bo *bo;

   bo = (struct exynos_bo *)buf->bh;
   munmap(buf->mapping, bo->size);
}

static void
_exynos_discard(Dmabuf_Buffer *buf)
{
   struct exynos_bo *bo;

   bo = (struct exynos_bo *)buf->bh;
   sym_exynos_bo_destroy(bo);
}

static Eina_Bool
_exynos_buffer_manager_setup(int fd)
{
   Eina_Bool fail = EINA_FALSE;
   void *drm_exynos_lib;

   drm_exynos_lib = dlopen("libdrm_exynos.so", RTLD_LAZY | RTLD_GLOBAL);
   if (!drm_exynos_lib) return EINA_FALSE;

   SYM(drm_exynos_lib, exynos_device_create);
   SYM(drm_exynos_lib, exynos_bo_create);
   SYM(drm_exynos_lib, exynos_bo_map);
   SYM(drm_exynos_lib, exynos_bo_destroy);
   SYM(drm_exynos_lib, drmPrimeHandleToFD);

   if (fail) goto err;

   buffer_manager->priv = sym_exynos_device_create(fd);
   if (!buffer_manager->priv) goto err;

   buffer_manager->alloc = _exynos_alloc;
   buffer_manager->map = _exynos_map;
   buffer_manager->unmap = _exynos_unmap;
   buffer_manager->discard = _exynos_discard;

   return EINA_TRUE;

err:
   dlclose(drm_exynos_lib);
   return EINA_FALSE;
}

static Buffer_Manager *
_buffer_manager_get(void)
{
   int fd;
   Eina_Bool success = EINA_FALSE;

   if (buffer_manager) return buffer_manager;

   buffer_manager = calloc(1, sizeof(Buffer_Manager));
   if (!buffer_manager) goto err_alloc;

   fd = open("/dev/dri/renderD128", O_RDWR);
   if (fd < 0) goto err_drm;

   success = _intel_buffer_manager_setup(fd);
   if (!success) success = _exynos_buffer_manager_setup(fd);
   if (!success) goto err_bm;

   drm_fd = fd;
   return buffer_manager;

err_bm:
   close(fd);
err_drm:
   free(buffer_manager);
err_alloc:
   dmabuf_totally_hosed = EINA_TRUE;
   return NULL;
}

static void
buffer_release(void *data, struct wl_buffer *buffer EINA_UNUSED)
{
   Dmabuf_Buffer *b = data;

   b->busy = EINA_FALSE;
   if (b->orphaned) _evas_dmabuf_buffer_destroy(b);
}

static const struct wl_buffer_listener buffer_listener =
{
   buffer_release
};

static void
_fallback(Dmabuf_Surface *s, int w, int h)
{
   Eina_Bool recovered;

   dmabuf_totally_hosed = EINA_TRUE;

   /* Depending when things broke we may need this commit to get
    * the frame callback to fire and keep the animator running
    */
   wl_surface_commit(s->wl_surface);

   _evas_dmabuf_surface_destroy(s->surface);
   recovered = _evas_surface_init(s->surface, w, h, s->nbuf);
   if (recovered) return;

   ERR("Fallback from dmabuf to shm attempted and failed.");
   abort();
}

static void
_allocation_complete(Dmabuf_Buffer *b)
{
   b->pending = EINA_FALSE;
   if (!dmabuf_totally_hosed) return;

   if (!b->surface) return;

   /* Something went wrong, better try to fall back to a different
    * buffer type...
    */
   _fallback(b->surface, b->w, b->h);
}

static void
_create_succeeded(void *data,
                 struct zwp_linux_buffer_params_v1 *params,
                 struct wl_buffer *new_buffer)
{
   Dmabuf_Buffer *b = data;

   b->wl_buffer = new_buffer;
   wl_buffer_add_listener(b->wl_buffer, &buffer_listener, b);
   zwp_linux_buffer_params_v1_destroy(params);

   if (b->orphaned)
     {
        _allocation_complete(b);
        _evas_dmabuf_buffer_destroy(b);
        return;
     }

   _allocation_complete(b);
   if (dmabuf_totally_hosed) return;

   if (!b->busy) return;
   if (b != b->surface->pre) return;

   /* This buffer was drawn into before it had a handle */
   wl_surface_attach(b->surface->wl_surface, b->wl_buffer, 0, 0);
   _evas_surface_damage(b->surface->wl_surface, b->surface->compositor_version,
                        b->w, b->h, NULL, 0);
   wl_surface_commit(b->surface->wl_surface);
   b->surface->pre = NULL;
   b->busy = EINA_FALSE;
}

static void
_create_failed(void *data, struct zwp_linux_buffer_params_v1 *params)
{
   Dmabuf_Buffer *b = data;
   Eina_Bool orphaned;

   zwp_linux_buffer_params_v1_destroy(params);

   dmabuf_totally_hosed = EINA_TRUE;
   orphaned = b->orphaned;
   _allocation_complete(b);
   if (orphaned) _evas_dmabuf_buffer_destroy(b);
}

static const struct zwp_linux_buffer_params_v1_listener params_listener =
{
   _create_succeeded,
   _create_failed
};

static void
_evas_dmabuf_buffer_unlock(Dmabuf_Buffer *b)
{
   b->bm->unmap(b);
   b->mapping = NULL;
   b->locked = EINA_FALSE;
}

static void
_evas_dmabuf_buffer_destroy(Dmabuf_Buffer *b)
{
   if (!b) return;

   if (b->locked || b->busy || b->pending)
     {
        b->orphaned = EINA_TRUE;
        b->surface = NULL;
        return;
     }
   b->bm->discard(b);
   if (b->wl_buffer) wl_buffer_destroy(b->wl_buffer);
   b->wl_buffer = NULL;
   free(b);
}

static void
_evas_dmabuf_surface_reconfigure(Surface *s, int w, int h, uint32_t flags EINA_UNUSED)
{
   Dmabuf_Buffer *buf;
   Dmabuf_Surface *surface;
   int i;

   surface = s->surf.dmabuf;
   for (i = 0; i < surface->nbuf; i++)
     {
        Dmabuf_Buffer *b = surface->buffer[i];
        int stride = b->stride;

        /* If stride is a little bigger than width we still fit */
        if ((w >= b->w) && (w <= stride / 4) && (h == b->h))
          {
             b->w = w;
             continue;
          }

        _evas_dmabuf_buffer_destroy(b);
        buf = _evas_dmabuf_buffer_init(surface, w, h);
        surface->buffer[i] = buf;
        if (!buf)
           {
              _fallback(surface, w, h);
              return;
           }
     }
}

static void *
_evas_dmabuf_surface_data_get(Surface *s, int *w, int *h)
{
   Dmabuf_Surface *surface;
   Dmabuf_Buffer *b;
   void *ptr;

   surface = s->surf.dmabuf;
   b = surface->current;
   if (!b) return NULL;

   /* We return stride/bpp because it may not match the allocated
    * width.  evas will figure out the clipping
    */
   if (w) *w = b->stride / 4;
   if (h) *h = b->h;
   if (b->locked) return b->mapping;

   ptr = b->bm->map(b);
   if (!ptr)
     return NULL;

   b->mapping = ptr;
   b->locked = EINA_TRUE;
   return b->mapping;
}

static Dmabuf_Buffer *
_evas_dmabuf_surface_wait(Dmabuf_Surface *s)
{
   int iterations = 0, i;

   while (iterations++ < 10)
     {
        for (i = 0; i < s->nbuf; i++)
          if (!s->buffer[i]->locked &&
              !s->buffer[i]->busy &&
              !s->buffer[i]->pending)
            return s->buffer[i];

        wl_display_dispatch_pending(s->wl_display);
     }

   /* May be we have a possible render target that just hasn't been
    * given a wl_buffer yet - draw there and let the success handler
    * figure it out.
    */
   for (i = 0; i < s->nbuf; i++)
     if (!s->buffer[i]->locked && !s->buffer[i]->busy)
       return s->buffer[i];

   return NULL;
}

static int
_evas_dmabuf_surface_assign(Surface *s)
{
   Dmabuf_Surface *surface;
   int i;

   surface = s->surf.dmabuf;
   surface->current = _evas_dmabuf_surface_wait(surface);
   if (!surface->current)
     {
        WRN("No free DMAbuf buffers, dropping a frame");
        for (i = 0; i < surface->nbuf; i++)
          surface->buffer[i]->age = 0;
        return 0;
     }
   for (i = 0; i < surface->nbuf; i++)
     if (surface->buffer[i]->used) surface->buffer[i]->age++;

   return surface->current->age;
}

static void
_evas_dmabuf_surface_post(Surface *s, Eina_Rectangle *rects, unsigned int count)
{
   Dmabuf_Surface *surface;
   Dmabuf_Buffer *b;

   surface = s->surf.dmabuf;
   b = surface->current;
   if (!b) return;

   _evas_dmabuf_buffer_unlock(b);

   surface->current = NULL;
   b->busy = EINA_TRUE;
   b->used = EINA_TRUE;
   b->age = 0;

   /* If we don't yet have a buffer assignement we need to track the
    * most recently filled unassigned buffer and make sure it gets
    * displayed.
    */
   if (!b->wl_buffer)
     {
        surface->pre = b;
        return;
     }
   surface->pre = NULL;
   wl_surface_attach(surface->wl_surface, b->wl_buffer, 0, 0);
   _evas_surface_damage(surface->wl_surface, surface->compositor_version,
                        b->w, b->h, rects, count);
   wl_surface_commit(surface->wl_surface);
}

static Dmabuf_Buffer *
_evas_dmabuf_buffer_init(Dmabuf_Surface *s, int w, int h)
{
   Dmabuf_Buffer *out;
   struct zwp_linux_buffer_params_v1 *dp;
   Buffer_Manager *bm = _buffer_manager_get();
   uint32_t flags = ZWP_LINUX_BUFFER_PARAMS_V1_FLAGS_Y_INVERT;

   if (!bm) return NULL;

   out = calloc(1, sizeof(Dmabuf_Buffer));
   if (!out) return NULL;

   out->bm = bm;

   out->surface = s;
   out->bh = bm->alloc(bm, "name", w, h, &out->stride, &out->fd);
   if (!out->bh)
   {
      free(out);
      _fallback(s, w, h);
      return NULL;
   }
   out->w = w;
   out->h = h;

   out->pending = EINA_TRUE;
   dp = zwp_linux_dmabuf_v1_create_params(out->surface->dmabuf);
   zwp_linux_buffer_params_v1_add(dp, out->fd, 0, 0, out->stride, 0, 0);
   zwp_linux_buffer_params_v1_add_listener(dp, &params_listener, out);
   zwp_linux_buffer_params_v1_create(dp, out->w, out->h,
                                     DRM_FORMAT_ARGB8888, flags);
   return out;
}

static void
_internal_evas_dmabuf_surface_destroy(Dmabuf_Surface *surface)
{
   int i;

   for (i = 0; i < surface->nbuf; i++)
      _evas_dmabuf_buffer_destroy(surface->buffer[i]);

   free(surface->buffer);
}

static void
_evas_dmabuf_surface_destroy(Surface *s)
{
   if (!s) return;

   _internal_evas_dmabuf_surface_destroy(s->surf.dmabuf);
}

Eina_Bool
_evas_dmabuf_surface_create(Surface *s, int w, int h, int num_buff)
{
   Dmabuf_Surface *surf = NULL;
   int i = 0;

   if (dmabuf_totally_hosed) return EINA_FALSE;

   if (!s->info->info.wl_dmabuf) return EINA_FALSE;
   if (!_buffer_manager_get()) return EINA_FALSE;

   if (!(s->surf.dmabuf = calloc(1, sizeof(Dmabuf_Surface)))) goto err;
   surf = s->surf.dmabuf;

   surf->surface = s;
   surf->wl_display = s->info->info.wl_disp;
   surf->dmabuf = s->info->info.wl_dmabuf;
   surf->wl_surface = s->info->info.wl_surface;
   surf->alpha = s->info->info.destination_alpha;
   surf->compositor_version = s->info->info.compositor_version;

   /* create surface buffers */
   surf->nbuf = num_buff;
   surf->buffer = calloc(surf->nbuf, sizeof(Dmabuf_Buffer *));
   if (!surf->buffer) goto err;

   for (i = 0; i < num_buff; i++)
     {
        surf->buffer[i] = _evas_dmabuf_buffer_init(surf, w, h);
        if (!surf->buffer[i])
          {
             ERR("Could not create buffers");
             goto err;
          }
     }

   s->type = SURFACE_DMABUF;
   s->funcs.destroy = _evas_dmabuf_surface_destroy;
   s->funcs.reconfigure = _evas_dmabuf_surface_reconfigure;
   s->funcs.data_get = _evas_dmabuf_surface_data_get;
   s->funcs.assign = _evas_dmabuf_surface_assign;
   s->funcs.post = _evas_dmabuf_surface_post;

   return EINA_TRUE;

err:
   if (surf) _fallback(surf, w, h);
   return EINA_FALSE;
}
