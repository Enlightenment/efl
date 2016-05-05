#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_engine.h"

/* When other buffer managers are supported this will become
 * #ifdef HAVE_DRM_FOR_WAYLAND_SHM
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <drm_fourcc.h>
#include <intel_bufmgr.h>
#include <i915_drm.h>

#include "linux-dmabuf-unstable-v1-client-protocol.h"

#define SYM(lib, xx)               \
   do {                            \
      sym_## xx = dlsym(lib, #xx); \
      if (!(sym_ ## xx)) {         \
         fail = EINA_TRUE;         \
      }                            \
   } while (0)

static drm_intel_bufmgr *buffer_manager;

static Eina_Bool dmabuf_totally_hosed;

typedef struct _Dmabuf_Surface Dmabuf_Surface;

typedef struct _Dmabuf_Buffer Dmabuf_Buffer;
struct _Dmabuf_Buffer
{
   Dmabuf_Surface *surface;
   struct wl_buffer *wl_buffer;
   int w, h;
   int age;
   unsigned long stride;
   drm_intel_bo *bo;
   int fd;

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
int (*sym_drm_intel_bo_gem_export_to_prime)(drm_intel_bo *bo, int *fd) = NULL;
void (*sym_drm_intel_bo_unreference)(drm_intel_bo *bo) = NULL;

static drm_intel_bufmgr *
_get_buffer_manager(void)
{
   int fd;
   void *drm_intel_lib;
   Eina_Bool fail = EINA_FALSE;

   if (buffer_manager) return buffer_manager;

   drm_intel_lib = dlopen("libdrm_intel.so", RTLD_LAZY | RTLD_GLOBAL);
   if (!drm_intel_lib) goto err_dlopen;

   SYM(drm_intel_lib, drm_intel_bufmgr_gem_init);
   SYM(drm_intel_lib, drm_intel_gem_bo_unmap_gtt);
   SYM(drm_intel_lib, drm_intel_gem_bo_map_gtt);
   SYM(drm_intel_lib, drm_intel_bo_alloc_tiled);
   SYM(drm_intel_lib, drm_intel_bo_gem_export_to_prime);
   SYM(drm_intel_lib, drm_intel_bo_unreference);

   if (fail) goto err_dlsym;

   fd = open("/dev/dri/renderD128", O_RDWR);
   if (fd < 0) goto err_dlsym;

   buffer_manager = sym_drm_intel_bufmgr_gem_init(fd, 32);
   if (!buffer_manager) goto err_bufmgr;

   return buffer_manager;

err_bufmgr:
   close(fd);
err_dlsym:
   dlclose(drm_intel_lib);
err_dlopen:
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
_allocation_complete(Dmabuf_Buffer *b)
{
   Surface *s;
   int w, h, num_buf;
   Eina_Bool recovered;

   b->pending = EINA_FALSE;
   if (!dmabuf_totally_hosed) return;

   if (!b->surface) return;

   /* Something went wrong, better try to fall back to a different
    * buffer type...
    */
   s = b->surface->surface;
   w = b->w;
   h = b->h;
   num_buf = b->surface->nbuf;

   /* Depending when things broke we may need this commit to get
    * the frame callback to fire and keep the animator running
    */
   wl_surface_commit(b->surface->wl_surface);
   _evas_dmabuf_surface_destroy(b->surface->surface);
   recovered = _evas_surface_init(s, w, h, num_buf);
   if (recovered) return;

   ERR("Fallback from dmabuf to shm attempted and failed.");
   abort();
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
   sym_drm_intel_gem_bo_unmap_gtt(b->bo);
   b->locked = EINA_FALSE;
}

static void
_evas_dmabuf_buffer_destroy(Dmabuf_Buffer *b)
{
   if (b->locked || b->busy || b->pending)
     {
        b->orphaned = EINA_TRUE;
        b->surface = NULL;
        return;
     }
   sym_drm_intel_bo_unreference(b->bo);
   if (b->wl_buffer) wl_buffer_destroy(b->wl_buffer);
   b->wl_buffer = NULL;
   free(b);
}

static void
_evas_dmabuf_surface_reconfigure(Surface *s, int w, int h, uint32_t flags EINA_UNUSED)
{
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
        surface->buffer[i] = _evas_dmabuf_buffer_init(surface, w, h);
     }
}

static void *
_evas_dmabuf_surface_data_get(Surface *s, int *w, int *h)
{
   Dmabuf_Surface *surface;
   Dmabuf_Buffer *b;

   surface = s->surf.dmabuf;
   b = surface->current;
   if (!b) return NULL;

   /* We return stride/bpp because it may not match the allocated
    * width.  evas will figure out the clipping
    */
   if (w) *w = b->stride / 4;
   if (h) *h = b->h;
   if (b->locked) return b->bo->virtual;

   if (sym_drm_intel_gem_bo_map_gtt(b->bo) != 0)
     return NULL;

   b->locked = EINA_TRUE;
   return b->bo->virtual;
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
   drm_intel_bufmgr *mgr = _get_buffer_manager();
   uint32_t tile = I915_TILING_NONE;
   uint32_t flags = ZWP_LINUX_BUFFER_PARAMS_V1_FLAGS_Y_INVERT;

   if (!mgr) return NULL;

   out = calloc(1, sizeof(Dmabuf_Buffer));
   if (!out) return NULL;

   out->surface = s;
   out->bo = sym_drm_intel_bo_alloc_tiled(mgr, "name", w, h, 4, &tile,
                                          &out->stride, 0);
   out->w = w;
   out->h = h;
   if (tile != I915_TILING_NONE) goto err;
   if (sym_drm_intel_bo_gem_export_to_prime(out->bo, &out->fd) != 0) goto err;

   out->pending = EINA_TRUE;
   dp = zwp_linux_dmabuf_v1_create_params(out->surface->dmabuf);
   zwp_linux_buffer_params_v1_add(dp, out->fd, 0, 0, out->stride, 0, 0);
   zwp_linux_buffer_params_v1_add_listener(dp, &params_listener, out);
   zwp_linux_buffer_params_v1_create(dp, out->w, out->h,
                                     DRM_FORMAT_ARGB8888, flags);
   return out;
err:
   _evas_dmabuf_buffer_destroy(out);
   return NULL;
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
   Dmabuf_Surface *surf;
   int i = 0;

   if (dmabuf_totally_hosed) return EINA_FALSE;

   if (!s->info->info.wl_dmabuf) return EINA_FALSE;
   if (!_get_buffer_manager()) return EINA_FALSE;

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
   _evas_dmabuf_surface_destroy(s);
   return EINA_FALSE;
}
