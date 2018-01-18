#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <drm_fourcc.h>
#include <intel_bufmgr.h>
#include <i915_drm.h>
#include <vc4_drm.h>
#include <exynos_drm.h>
#include <exynos_drmif.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "linux-dmabuf-unstable-v1-client-protocol.h"

#define SYM(lib, xx)               \
   do {                            \
      sym_## xx = dlsym(lib, #xx); \
      if (!(sym_ ## xx)) {         \
         fail = EINA_TRUE;         \
      }                            \
   } while (0)

static int drm_fd = -1;

typedef struct _Ecore_Wl2_Buffer Ecore_Wl2_Buffer;
typedef struct _Buffer_Handle Buffer_Handle;
typedef struct _Buffer_Manager Buffer_Manager;
struct _Buffer_Manager
{
   Buffer_Handle *(*alloc)(Buffer_Manager *self, const char *name, int w, int h, unsigned long *stride, int32_t *fd);
   struct wl_buffer *(*to_buffer)(Ecore_Wl2_Display *ewd, Ecore_Wl2_Buffer *db);
   void *(*map)(Ecore_Wl2_Buffer *buf);
   void (*unmap)(Ecore_Wl2_Buffer *buf);
   void (*discard)(Ecore_Wl2_Buffer *buf);
   void (*manager_destroy)(void);
   void *priv;
   void *dl_handle;
   int refcount;
   Eina_Bool destroyed;
};

static Buffer_Manager *buffer_manager = NULL;

static drm_intel_bufmgr *(*sym_drm_intel_bufmgr_gem_init)(int fd, int batch_size) = NULL;
static int (*sym_drm_intel_bo_unmap)(drm_intel_bo *bo) = NULL;
static int (*sym_drm_intel_bo_map)(drm_intel_bo *bo) = NULL;
static drm_intel_bo *(*sym_drm_intel_bo_alloc_tiled)(drm_intel_bufmgr *mgr, const char *name, int x, int y, int cpp, uint32_t *tile, unsigned long *pitch, unsigned long flags) = NULL;
static void (*sym_drm_intel_bo_unreference)(drm_intel_bo *bo) = NULL;
static int (*sym_drmPrimeHandleToFD)(int fd, uint32_t handle, uint32_t flags, int *prime_fd) = NULL;
static void (*sym_drm_intel_bufmgr_destroy)(drm_intel_bufmgr *) = NULL;

static struct exynos_device *(*sym_exynos_device_create)(int fd) = NULL;
static struct exynos_bo *(*sym_exynos_bo_create)(struct exynos_device *dev, size_t size, uint32_t flags) = NULL;
static void *(*sym_exynos_bo_map)(struct exynos_bo *bo) = NULL;
static void (*sym_exynos_bo_destroy)(struct exynos_bo *bo) = NULL;
static void (*sym_exynos_device_destroy)(struct exynos_device *) = NULL;

static void
buffer_release(void *data, struct wl_buffer *buffer EINA_UNUSED)
{
   Ecore_Wl2_Buffer *b = data;

   b->busy = EINA_FALSE;
   if (b->orphaned) ecore_wl2_buffer_destroy(b);
}

static const struct wl_buffer_listener buffer_listener =
{
   buffer_release
};

static struct wl_buffer *
_evas_dmabuf_wl_buffer_from_dmabuf(Ecore_Wl2_Display *ewd, Ecore_Wl2_Buffer *db)
{
   struct wl_buffer *buf;
   struct zwp_linux_dmabuf_v1 *dmabuf;
   struct zwp_linux_buffer_params_v1 *dp;
   uint32_t flags = 0;
   uint32_t format;

   if (db->alpha)
     format = DRM_FORMAT_ARGB8888;
   else
     format = DRM_FORMAT_XRGB8888;

   dmabuf = ecore_wl2_display_dmabuf_get(ewd);
   dp = zwp_linux_dmabuf_v1_create_params(dmabuf);
   zwp_linux_buffer_params_v1_add(dp, db->fd, 0, 0, db->stride, 0, 0);
   buf = zwp_linux_buffer_params_v1_create_immed(dp, db->w, db->h,
                                                 format, flags);
   wl_buffer_add_listener(buf, &buffer_listener, db);
   zwp_linux_buffer_params_v1_destroy(dp);

   return buf;
}

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
_intel_map(Ecore_Wl2_Buffer *buf)
{
   drm_intel_bo *bo;

   bo = (drm_intel_bo *)buf->bh;
   if (sym_drm_intel_bo_map(bo) != 0) return NULL;
   return bo->virtual;
}

static void
_intel_unmap(Ecore_Wl2_Buffer *buf)
{
   drm_intel_bo *bo;

   bo = (drm_intel_bo *)buf->bh;
   sym_drm_intel_bo_unmap(bo);
}

static void
_intel_discard(Ecore_Wl2_Buffer *buf)
{
   drm_intel_bo *bo;

   bo = (drm_intel_bo *)buf->bh;
   sym_drm_intel_bo_unreference(bo);
}

static void
_intel_manager_destroy()
{
   sym_drm_intel_bufmgr_destroy(buffer_manager->priv);
}

static Eina_Bool
_intel_buffer_manager_setup(int fd)
{
   Eina_Bool fail = EINA_FALSE;
   void *drm_intel_lib;

   drm_intel_lib = dlopen("libdrm_intel.so", RTLD_LAZY | RTLD_GLOBAL);
   if (!drm_intel_lib) return EINA_FALSE;

   SYM(drm_intel_lib, drm_intel_bufmgr_gem_init);
   SYM(drm_intel_lib, drm_intel_bo_unmap);
   SYM(drm_intel_lib, drm_intel_bo_map);
   SYM(drm_intel_lib, drm_intel_bo_alloc_tiled);
   SYM(drm_intel_lib, drm_intel_bo_unreference);
   SYM(drm_intel_lib, drm_intel_bufmgr_destroy);
   SYM(drm_intel_lib, drmPrimeHandleToFD);

   if (fail) goto err;

   buffer_manager->priv = sym_drm_intel_bufmgr_gem_init(fd, 32);
   if (!buffer_manager->priv) goto err;

   buffer_manager->alloc = _intel_alloc;
   buffer_manager->to_buffer = _evas_dmabuf_wl_buffer_from_dmabuf;
   buffer_manager->map = _intel_map;
   buffer_manager->unmap = _intel_unmap;
   buffer_manager->discard = _intel_discard;
   buffer_manager->manager_destroy = _intel_manager_destroy;
   buffer_manager->dl_handle = drm_intel_lib;

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
_exynos_map(Ecore_Wl2_Buffer *buf)
{
   struct exynos_bo *bo;
   void *ptr;

   bo = (struct exynos_bo *)buf->bh;
   ptr = mmap(0, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED, buf->fd, 0);
   if (ptr == MAP_FAILED) return NULL;
   return ptr;
}

static void
_exynos_unmap(Ecore_Wl2_Buffer *buf)
{
   struct exynos_bo *bo;

   bo = (struct exynos_bo *)buf->bh;
   munmap(buf->mapping, bo->size);
}

static void
_exynos_discard(Ecore_Wl2_Buffer *buf)
{
   struct exynos_bo *bo;

   bo = (struct exynos_bo *)buf->bh;
   sym_exynos_bo_destroy(bo);
}

static void
_exynos_manager_destroy()
{
   sym_exynos_device_destroy(buffer_manager->priv);
}

static Eina_Bool
_exynos_buffer_manager_setup(int fd)
{
   Eina_Bool fail = EINA_FALSE;
   void *drm_exynos_lib;
   struct exynos_bo *bo;

   drm_exynos_lib = dlopen("libdrm_exynos.so", RTLD_LAZY | RTLD_GLOBAL);
   if (!drm_exynos_lib) return EINA_FALSE;

   SYM(drm_exynos_lib, exynos_device_create);
   SYM(drm_exynos_lib, exynos_bo_create);
   SYM(drm_exynos_lib, exynos_bo_map);
   SYM(drm_exynos_lib, exynos_bo_destroy);
   SYM(drm_exynos_lib, exynos_device_destroy);
   SYM(drm_exynos_lib, drmPrimeHandleToFD);

   if (fail) goto err;

   buffer_manager->priv = sym_exynos_device_create(fd);
   if (!buffer_manager->priv) goto err;

   /* _device_create succeeds on any arch, test harder */
   bo = sym_exynos_bo_create(buffer_manager->priv, 32, 0);
   if (!bo) goto err;

   sym_exynos_bo_destroy(bo);

   buffer_manager->alloc = _exynos_alloc;
   buffer_manager->to_buffer = _evas_dmabuf_wl_buffer_from_dmabuf;
   buffer_manager->map = _exynos_map;
   buffer_manager->unmap = _exynos_unmap;
   buffer_manager->discard = _exynos_discard;
   buffer_manager->manager_destroy = _exynos_manager_destroy;
   buffer_manager->dl_handle = drm_exynos_lib;
   return EINA_TRUE;

err:
   dlclose(drm_exynos_lib);
   return EINA_FALSE;
}

static Buffer_Handle *
_wl_shm_alloc(Buffer_Manager *self EINA_UNUSED, const char *name EINA_UNUSED, int w, int h, unsigned long *stride, int32_t *fd)
{
   Efl_Vpath_File *file_obj;
   Eina_Tmpstr *fullname;
   size_t size = w * h * 4;
   void *out = NULL;

   file_obj = efl_vpath_manager_fetch(EFL_VPATH_MANAGER_CLASS,
                                      "(:run:)/evas-wayland_shm-XXXXXX");
   *fd = eina_file_mkstemp(efl_vpath_file_result_get(file_obj), &fullname);
   efl_del(file_obj);
   if (*fd < 0) return NULL;

   unlink(fullname);
   eina_tmpstr_del(fullname);

   *stride = w * 4;
   if (ftruncate(*fd, size) < 0) goto err;

   out = mmap(NULL, size, (PROT_READ | PROT_WRITE), MAP_SHARED, *fd, 0);
   if (out == MAP_FAILED) goto err;

   return out;

err:
   close(*fd);
   return NULL;
}

static void *
_wl_shm_map(Ecore_Wl2_Buffer *buf)
{
   return buf->bh;
}

static void
_wl_shm_unmap(Ecore_Wl2_Buffer *buf EINA_UNUSED)
{
   /* wl_shm is mapped for its lifetime */
}

static void
_wl_shm_discard(Ecore_Wl2_Buffer *buf)
{
   munmap(buf->bh, buf->size);
}

static void
_wl_shm_manager_destroy()
{
   /* Nop. */
}

static struct wl_buffer *
_wl_shm_to_buffer(Ecore_Wl2_Display *ewd, Ecore_Wl2_Buffer *db)
{
   struct wl_buffer *buf;
   struct wl_shm_pool *pool;
   struct wl_shm *shm;
   uint32_t format;

   if (db->alpha)
     format = WL_SHM_FORMAT_ARGB8888;
   else
     format = WL_SHM_FORMAT_XRGB8888;

   shm = ecore_wl2_display_shm_get(ewd);
   pool = wl_shm_create_pool(shm, db->fd, db->size);
   buf = wl_shm_pool_create_buffer(pool, 0, db->w, db->h, db->stride, format);
   wl_shm_pool_destroy(pool);
   close(db->fd);
   db->fd = -1;
   wl_buffer_add_listener(buf, &buffer_listener, db);
   return buf;
}

static Eina_Bool
_wl_shm_buffer_manager_setup(int fd EINA_UNUSED)
{
   buffer_manager->alloc = _wl_shm_alloc;
   buffer_manager->to_buffer = _wl_shm_to_buffer;
   buffer_manager->map = _wl_shm_map;
   buffer_manager->unmap = _wl_shm_unmap;
   buffer_manager->discard = _wl_shm_discard;
   buffer_manager->manager_destroy = _wl_shm_manager_destroy;
   return EINA_TRUE;
}

struct internal_vc4_bo
{
   __u32 handle;
   int size;
   int fd;
};

static int
align(int v, int a)
{
   return (v + a - 1) & ~((uint64_t)a - 1);
}

static Buffer_Handle *
_vc4_alloc(Buffer_Manager *self EINA_UNUSED, const char *name EINA_UNUSED, int w, int h, unsigned long *stride, int32_t *fd)
{
   struct drm_vc4_create_bo bo;
   struct internal_vc4_bo *obo;
   struct drm_gem_close cl;
   size_t size;
   int ret;

   obo = malloc(sizeof(struct internal_vc4_bo));
   if (!obo) return NULL;

   *stride = align(w * 4, 16);
   size = *stride * h;
   memset(&bo, 0, sizeof(bo));
   bo.size = size;
   ret = ioctl(drm_fd, DRM_IOCTL_VC4_CREATE_BO, &bo);
   if (ret)
     {
        free(obo);
        return NULL;
     }

   obo->handle = bo.handle;
   obo->size = size;
   /* First try to allocate an mmapable buffer with O_RDWR,
    * if that fails retry unmappable - if the compositor is
    * using GL it won't need to mmap the buffer and this can
    * work - otherwise it'll reject this buffer and we'll
    * have to fall back to shm rendering.
    */
   if (sym_drmPrimeHandleToFD(drm_fd, bo.handle,
                              DRM_CLOEXEC | O_RDWR, fd) != 0)
     if (sym_drmPrimeHandleToFD(drm_fd, bo.handle,
                                DRM_CLOEXEC, fd) != 0) goto err;

   obo->fd = *fd;
   return (Buffer_Handle *)obo;

err:
   memset(&cl, 0, sizeof(cl));
   cl.handle = bo.handle;
   ioctl(drm_fd, DRM_IOCTL_GEM_CLOSE, &cl);
   free(obo);
   return NULL;
}

static void *
_vc4_map(Ecore_Wl2_Buffer *buf)
{
   struct drm_vc4_mmap_bo map;
   struct internal_vc4_bo *bo;
   void *ptr;
   int ret;

   bo = (struct internal_vc4_bo *)buf->bh;

   memset(&map, 0, sizeof(map));
   map.handle = bo->handle;
   ret = ioctl(drm_fd, DRM_IOCTL_VC4_MMAP_BO, &map);
   if (ret) return NULL;

   ptr = mmap(NULL, bo->size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd,
              map.offset);
   if (ptr == MAP_FAILED) return NULL;

   return ptr;
}

static void
_vc4_unmap(Ecore_Wl2_Buffer *buf)
{
   struct internal_vc4_bo *bo;

   bo = (struct internal_vc4_bo *)buf->bh;
   munmap(buf->mapping, bo->size);
}

static void
_vc4_discard(Ecore_Wl2_Buffer *buf)
{
   struct drm_gem_close cl;
   struct internal_vc4_bo *bo;

   bo = (struct internal_vc4_bo *)buf->bh;

   memset(&cl, 0, sizeof(cl));
   cl.handle = bo->handle;
   ioctl(drm_fd, DRM_IOCTL_GEM_CLOSE, &cl);
}

static Eina_Bool
_vc4_buffer_manager_setup(int fd)
{
   struct drm_gem_close cl;
   struct drm_vc4_create_bo bo;
   Eina_Bool fail = EINA_FALSE;
   void *drm_lib;

   memset(&bo, 0, sizeof(bo));
   bo.size = 32;
   if (ioctl(fd, DRM_IOCTL_VC4_CREATE_BO, &bo)) return EINA_FALSE;

   memset(&cl, 0, sizeof(cl));
   cl.handle = bo.handle;
   ioctl(fd, DRM_IOCTL_GEM_CLOSE, &cl);

   drm_lib = dlopen("libdrm.so", RTLD_LAZY | RTLD_GLOBAL);
   if (!drm_lib) return EINA_FALSE;

   SYM(drm_lib, drmPrimeHandleToFD);

   if (fail) goto err;

   buffer_manager->alloc = _vc4_alloc;
   buffer_manager->to_buffer = _evas_dmabuf_wl_buffer_from_dmabuf;
   buffer_manager->map = _vc4_map;
   buffer_manager->unmap = _vc4_unmap;
   buffer_manager->discard = _vc4_discard;
   buffer_manager->manager_destroy = NULL;
   buffer_manager->dl_handle = drm_lib;
   return EINA_TRUE;
err:
   dlclose(drm_lib);
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_wl2_buffer_init(Ecore_Wl2_Display *ewd, Ecore_Wl2_Buffer_Type types)
{
   int fd = -1;
   Eina_Bool dmabuf = ewd->wl.dmabuf && (types & ECORE_WL2_BUFFER_DMABUF);
   Eina_Bool shm = ewd->wl.shm && (types & ECORE_WL2_BUFFER_SHM);
   Eina_Bool success = EINA_FALSE;

   if (buffer_manager)
     {
        buffer_manager->refcount++;
        return EINA_TRUE;
     }

   buffer_manager = calloc(1, sizeof(Buffer_Manager));
   if (!buffer_manager) goto err_alloc;

   if (!getenv("EVAS_WAYLAND_SHM_DISABLE_DMABUF") && dmabuf)
     {
        fd = open("/dev/dri/renderD128", O_RDWR | O_CLOEXEC);
        if (fd < 0) goto err_drm;

        success = _intel_buffer_manager_setup(fd);
        if (!success) success = _exynos_buffer_manager_setup(fd);
        if (!success) success = _vc4_buffer_manager_setup(fd);
     }
   if (!success) success = shm && _wl_shm_buffer_manager_setup(0);
   if (!success) goto err_bm;

   drm_fd = fd;
   buffer_manager->refcount = 1;
   return EINA_TRUE;

err_bm:
   if (fd >= 0) close(fd);
err_drm:
   free(buffer_manager);
   buffer_manager = NULL;
err_alloc:
   return EINA_FALSE;
}

static void
_buffer_manager_ref(void)
{
   buffer_manager->refcount++;
}

static void
_buffer_manager_deref(void)
{
   buffer_manager->refcount--;
   if (buffer_manager->refcount || !buffer_manager->destroyed) return;

   if (buffer_manager->manager_destroy) buffer_manager->manager_destroy();
   free(buffer_manager);
   buffer_manager = NULL;
   if (drm_fd >=0) close(drm_fd);
}

static void
_buffer_manager_destroy(void)
{
   if (buffer_manager->destroyed) return;
   buffer_manager->destroyed = EINA_TRUE;
   _buffer_manager_deref();
}

static Buffer_Handle *
_buffer_manager_alloc(const char *name, int w, int h, unsigned long *stride, int32_t *fd)
{
   Buffer_Handle *out;

   _buffer_manager_ref();
   out = buffer_manager->alloc(buffer_manager, name, w, h, stride, fd);
   if (!out) _buffer_manager_deref();
   return out;
}

EAPI struct wl_buffer *
ecore_wl2_buffer_wl_buffer_get(Ecore_Wl2_Display *ewd, Ecore_Wl2_Buffer *buf)
{
   return buffer_manager->to_buffer(ewd, buf);
}

EAPI void *
ecore_wl2_buffer_map(Ecore_Wl2_Buffer *buf)
{
   void *out;

   _buffer_manager_ref();
   out = buffer_manager->map(buf);
   if (!out) _buffer_manager_deref();
   return out;
}

EAPI void
ecore_wl2_buffer_unmap(Ecore_Wl2_Buffer *buf)
{
   buffer_manager->unmap(buf);
   _buffer_manager_deref();
}

EAPI void
ecore_wl2_buffer_discard(Ecore_Wl2_Buffer *buf)
{
   buffer_manager->discard(buf);
   _buffer_manager_deref();
}

EAPI void
ecore_wl2_buffer_unlock(Ecore_Wl2_Buffer *b)
{
   ecore_wl2_buffer_unmap(b);
   b->mapping = NULL;
   b->locked = EINA_FALSE;
}

EAPI void
ecore_wl2_buffer_destroy(Ecore_Wl2_Buffer *b)
{
   if (!b) return;

   if (b->locked || b->busy)
     {
        b->orphaned = EINA_TRUE;
        return;
     }
   if (b->fd != -1) close(b->fd);
   if (b->mapping) ecore_wl2_buffer_unmap(b);
   ecore_wl2_buffer_discard(b);
   if (b->wl_buffer) wl_buffer_destroy(b->wl_buffer);
   b->wl_buffer = NULL;
   free(b);
}

static Ecore_Wl2_Buffer *
_ecore_wl2_buffer_partial_create(int w, int h, Eina_Bool alpha)
{
   Ecore_Wl2_Buffer *out;

   out = calloc(1, sizeof(Ecore_Wl2_Buffer));
   if (!out) return NULL;

   out->fd = -1;
   out->alpha = alpha;
   out->bh = _buffer_manager_alloc("name", w, h, &out->stride, &out->fd);
   if (!out->bh)
     {
        free(out);
        return NULL;
     }
   out->w = w;
   out->h = h;
   out->size = out->stride * h;

   return out;
}

EAPI Ecore_Wl2_Buffer *
ecore_wl2_buffer_create(Ecore_Wl2_Display *ewd, int w, int h, Eina_Bool alpha)
{
   Ecore_Wl2_Buffer *out;

   out = _ecore_wl2_buffer_partial_create(w, h, alpha);
   if (!out) return NULL;

   out->wl_buffer = ecore_wl2_buffer_wl_buffer_get(ewd, out);

   return out;
}

static void
_create_succeeded(void *data EINA_UNUSED,
                 struct zwp_linux_buffer_params_v1 *params,
                 struct wl_buffer *new_buffer)
{
   wl_buffer_destroy(new_buffer);
   zwp_linux_buffer_params_v1_destroy(params);
}

static void
_create_failed(void *data, struct zwp_linux_buffer_params_v1 *params)
{
   Ecore_Wl2_Display *ewd = data;

   zwp_linux_buffer_params_v1_destroy(params);
   _buffer_manager_destroy();
   ewd->wl.dmabuf = NULL;
}

static const struct zwp_linux_buffer_params_v1_listener params_listener =
{
   _create_succeeded,
   _create_failed
};

void
_ecore_wl2_buffer_test(Ecore_Wl2_Display *ewd)
{
   struct zwp_linux_buffer_params_v1 *dp;
   Ecore_Wl2_Buffer *buf;

   if (!ecore_wl2_buffer_init(ewd, ECORE_WL2_BUFFER_DMABUF)) return;

   buf = _ecore_wl2_buffer_partial_create(1, 1, EINA_TRUE);
   if (!buf) goto fail;

   dp = zwp_linux_dmabuf_v1_create_params(ewd->wl.dmabuf);
   zwp_linux_buffer_params_v1_add(dp, buf->fd, 0, 0, buf->stride, 0, 0);
   zwp_linux_buffer_params_v1_add_listener(dp, &params_listener, ewd);
   zwp_linux_buffer_params_v1_create(dp, buf->w, buf->h,
                                     DRM_FORMAT_ARGB8888, 0);

   ecore_wl2_buffer_destroy(buf);

   return;

fail:
  _buffer_manager_destroy();
  ewd->wl.dmabuf = NULL;
}
