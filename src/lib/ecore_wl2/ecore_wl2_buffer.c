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
#include <inttypes.h>

#if defined(__linux__)
#include <linux/dma-buf.h>
#elif defined(__FreeBSD__)
/* begin/end dma-buf functions used for userspace mmap. */
struct dma_buf_sync {
        __u64 flags;
};

#define DMA_BUF_SYNC_READ      (1 << 0)
#define DMA_BUF_SYNC_WRITE     (2 << 0)
#define DMA_BUF_SYNC_RW        (DMA_BUF_SYNC_READ | DMA_BUF_SYNC_WRITE)
#define DMA_BUF_SYNC_START     (0 << 2)
#define DMA_BUF_SYNC_END       (1 << 2)
#define DMA_BUF_SYNC_VALID_FLAGS_MASK \
        (DMA_BUF_SYNC_RW | DMA_BUF_SYNC_END)

#define DMA_BUF_BASE            'b'
#define DMA_BUF_IOCTL_SYNC      _IOW(DMA_BUF_BASE, 0, struct dma_buf_sync)
#endif

#include "linux-dmabuf-unstable-v1-client-protocol.h"

#define SYM(lib, xx)               \
   do {                            \
      sym_## xx = dlsym(lib, #xx); \
      if (!(sym_ ## xx)) {         \
         fail = EINA_TRUE;         \
      }                            \
   } while (0)

#ifndef DRM_FORMAT_MOD_INVALID
# define DRM_FORMAT_MOD_INVALID ((1ULL << 56) - 1)
#endif
#ifndef DRM_FORMAT_MOD_LINEAR
# define DRM_FORMAT_MOD_LINEAR 0ULL
#endif

/* Minimal GBM ABI.  We dlopen libgbm rather than linking it so that
 * ecore_wl2 keeps building and running on systems without Mesa, which is
 * also why we declare the few bits we need here instead of including
 * <gbm.h> and picking up a hard build dependency. */
struct gbm_device;
struct gbm_bo;

#define ECORE_GBM_BO_USE_RENDERING     (1 << 2)
#define ECORE_GBM_BO_USE_LINEAR        (1 << 4)
#define ECORE_GBM_BO_TRANSFER_READ     (1 << 0)
#define ECORE_GBM_BO_TRANSFER_WRITE    (1 << 1)
#define ECORE_GBM_BO_TRANSFER_RW \
   (ECORE_GBM_BO_TRANSFER_READ | ECORE_GBM_BO_TRANSFER_WRITE)

static int drm_fd = -1;

typedef struct _Ecore_Wl2_Buffer Ecore_Wl2_Buffer;
typedef struct _Buffer_Handle Buffer_Handle;
typedef struct _Buffer_Manager Buffer_Manager;
struct _Buffer_Manager
{
   Buffer_Handle *(*alloc)(Buffer_Manager *self, Ecore_Wl2_Display *ewd, const char *name, int w, int h, unsigned long *stride, uint64_t *modifier, int32_t *fd);
   struct wl_buffer *(*to_buffer)(Ecore_Wl2_Display *ewd, Ecore_Wl2_Buffer *db);
   void *(*map)(Ecore_Wl2_Buffer *buf);
   void (*unmap)(Ecore_Wl2_Buffer *buf);
   void (*discard)(Ecore_Wl2_Buffer *buf);
   void (*lock)(Ecore_Wl2_Buffer *buf);
   void (*unlock)(Ecore_Wl2_Buffer *buf);
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

static struct gbm_device *(*sym_gbm_create_device)(int fd) = NULL;
static void (*sym_gbm_device_destroy)(struct gbm_device *gbm) = NULL;
static struct gbm_bo *(*sym_gbm_bo_create)(struct gbm_device *gbm, uint32_t w, uint32_t h, uint32_t format, uint32_t flags) = NULL;
static void (*sym_gbm_bo_destroy)(struct gbm_bo *bo) = NULL;
static int (*sym_gbm_bo_get_fd)(struct gbm_bo *bo) = NULL;
static uint32_t (*sym_gbm_bo_get_stride)(struct gbm_bo *bo) = NULL;
static void *(*sym_gbm_bo_map)(struct gbm_bo *bo, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t flags, uint32_t *stride, void **map_data) = NULL;
static void (*sym_gbm_bo_unmap)(struct gbm_bo *bo, void *map_data) = NULL;
/* Optional - absent on pre-17.1 gbm.  Without it we simply report an
 * unknown modifier and let the compositor negotiate implicitly. */
static uint64_t (*sym_gbm_bo_get_modifier)(struct gbm_bo *bo) = NULL;

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

/* Read the driver name out of the kernel so we never aim a driver-specific
 * ioctl at a driver that never implemented it.  DRM command numbers are
 * per-driver: DRM_IOCTL_VC4_CREATE_BO is DRM_COMMAND_BASE + 0x03, which on
 * panfrost is DRM_PANFROST_MMAP_BO - same size, same direction, so the
 * kernel happily dispatches it and we get whatever that driver does with a
 * garbage handle. */
static Eina_Bool
_drm_driver_is(int fd, const char *want)
{
   drm_version_t v;
   char name[64];
   Eina_Bool ret;

   memset(&v, 0, sizeof(v));
   v.name = name;
   v.name_len = sizeof(name) - 1;

   if (ioctl(fd, DRM_IOCTL_VERSION, &v)) return EINA_FALSE;
   if (v.name_len >= (__typeof__(v.name_len))sizeof(name)) return EINA_FALSE;

   name[v.name_len] = '\0';
   ret = !strcmp(name, want);

   return ret;
}

static struct wl_buffer *
_evas_dmabuf_wl_buffer_from_dmabuf(Ecore_Wl2_Display *ewd, Ecore_Wl2_Buffer *db)
{
   struct wl_buffer *buf;
   struct zwp_linux_dmabuf_v1 *dmabuf;
   struct zwp_linux_buffer_params_v1 *dp;
   uint32_t flags = 0;
   uint32_t format;
   uint64_t modifier;

   if (db->alpha)
     format = DRM_FORMAT_ARGB8888;
   else
     format = DRM_FORMAT_XRGB8888;

   /* Tell the compositor the layout we actually allocated.  Sending a
    * concrete modifier is only meaningful if it had a chance to tell us
    * which ones it accepts (v3+); at v2 the field is ignored and treated
    * as implicit by spec-conforming compositors, so send INVALID rather
    * than claiming a layout we never negotiated. */
   modifier = db->modifier;
   if ((ewd->wl.dmabuf_version < 3) || (ewd->dmabuf_fmts.count == 0))
     modifier = DRM_FORMAT_MOD_INVALID;
   else if ((modifier != DRM_FORMAT_MOD_INVALID) &&
            (!_ecore_wl2_dmabuf_modifier_supported(ewd, format, modifier)))
     {
        /* We allocated something the compositor never advertised.  Fall
         * back to implicit negotiation instead of forcing a create that
         * would just fail. */
        DBG("Compositor does not advertise modifier %#" PRIx64 " for format "
            "%#x, falling back to implicit modifier", modifier, format);
        modifier = DRM_FORMAT_MOD_INVALID;
     }

   dmabuf = ecore_wl2_display_dmabuf_get(ewd);
   dp = zwp_linux_dmabuf_v1_create_params(dmabuf);
   zwp_linux_buffer_params_v1_add(dp, db->fd, 0, 0, db->stride,
                                  (uint32_t)(modifier >> 32),
                                  (uint32_t)(modifier & 0xFFFFFFFF));
   buf = zwp_linux_buffer_params_v1_create_immed(dp, db->w, db->h,
                                                 format, flags);
   wl_buffer_add_listener(buf, &buffer_listener, db);
   zwp_linux_buffer_params_v1_destroy(dp);

   return buf;
}

static void
_dmabuf_lock(Ecore_Wl2_Buffer *b)
{
   int ret;
   struct dma_buf_sync s;

   s.flags = DMA_BUF_SYNC_START | DMA_BUF_SYNC_RW;
   do
     {
        ret = ioctl(b->fd, DMA_BUF_IOCTL_SYNC, &s);
     } while (ret && ((errno == EAGAIN) || (errno == EINTR)));

   if (ret) WRN("Failed to lock dmabuf");
}

static void
_dmabuf_unlock(Ecore_Wl2_Buffer *b)
{
   int ret;
   struct dma_buf_sync s;

   s.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_RW;
   do
     {
        ret = ioctl(b->fd, DMA_BUF_IOCTL_SYNC, &s);
     } while (ret && ((errno == EAGAIN) || (errno == EINTR)));

   if (ret) WRN("Failed to unlock dmabuf");
}

static Buffer_Handle *
_intel_alloc(Buffer_Manager *self, Ecore_Wl2_Display *ewd EINA_UNUSED, const char *name, int w, int h, unsigned long *stride, uint64_t *modifier, int32_t *fd)
{
   uint32_t tile = I915_TILING_NONE;
   drm_intel_bo *out;

   /* we reject anything tiled below, so what we hand out is always linear */
   *modifier = DRM_FORMAT_MOD_LINEAR;
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

   if (!_drm_driver_is(fd, "i915")) return EINA_FALSE;

   drm_intel_lib = dlopen("libdrm_intel.so.1", RTLD_LAZY | RTLD_GLOBAL);
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
   buffer_manager->lock = _dmabuf_lock;
   buffer_manager->unlock = _dmabuf_unlock;
   buffer_manager->manager_destroy = _intel_manager_destroy;
   buffer_manager->dl_handle = drm_intel_lib;

   return EINA_TRUE;

err:
   dlclose(drm_intel_lib);
   return EINA_FALSE;
}

static Buffer_Handle *
_exynos_alloc(Buffer_Manager *self, Ecore_Wl2_Display *ewd EINA_UNUSED, const char *name EINA_UNUSED, int w, int h, unsigned long *stride, uint64_t *modifier, int32_t *fd)
{
   size_t size = w * h * 4;
   struct exynos_bo *out;

   *modifier = DRM_FORMAT_MOD_LINEAR;
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

   if (!_drm_driver_is(fd, "exynos")) return EINA_FALSE;

   drm_exynos_lib = dlopen("libdrm_exynos.so.1", RTLD_LAZY | RTLD_GLOBAL);
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
   buffer_manager->lock = _dmabuf_lock;
   buffer_manager->unlock = _dmabuf_unlock;
   buffer_manager->manager_destroy = _exynos_manager_destroy;
   buffer_manager->dl_handle = drm_exynos_lib;
   return EINA_TRUE;

err:
   dlclose(drm_exynos_lib);
   return EINA_FALSE;
}

/* Generic allocator that works on any driver with a Mesa gbm backend
 * (panfrost, amdgpu, radeon, nouveau, iris, lima, v3d, ...).  The
 * driver-specific managers above only cover i915, exynos and vc4, so
 * without this everything else silently falls back to wl_shm.
 *
 * The only consumer of these buffers is the software Evas engine, which
 * renders into them with the CPU, so they must be linear and mappable.
 * That is also why we do not use gbm_bo_create_with_modifiers(): letting
 * the driver pick a tiled layout here would be actively wrong. */
static Buffer_Handle *
_gbm_alloc(Buffer_Manager *self, Ecore_Wl2_Display *ewd EINA_UNUSED, const char *name EINA_UNUSED, int w, int h, unsigned long *stride, uint64_t *modifier, int32_t *fd)
{
   struct gbm_bo *bo;

   bo = sym_gbm_bo_create(self->priv, w, h, DRM_FORMAT_ARGB8888,
                          ECORE_GBM_BO_USE_LINEAR |
                          ECORE_GBM_BO_USE_RENDERING);
   if (!bo) return NULL;

   *fd = sym_gbm_bo_get_fd(bo);
   if (*fd < 0)
     {
        sym_gbm_bo_destroy(bo);
        return NULL;
     }

   *stride = sym_gbm_bo_get_stride(bo);
   if (sym_gbm_bo_get_modifier)
     *modifier = sym_gbm_bo_get_modifier(bo);
   else
     *modifier = DRM_FORMAT_MOD_INVALID;

   return (Buffer_Handle *)bo;
}

static void *
_gbm_map(Ecore_Wl2_Buffer *buf)
{
   struct gbm_bo *bo;
   uint32_t map_stride = 0;
   void *ptr;

   bo = (struct gbm_bo *)buf->bh;

   /* NB: mapping the exported dmabuf fd is not an option - Mesa exports it
    * read-only on several drivers (panfrost among them), so a writable
    * mmap() comes back EACCES.  gbm_bo_map() goes through the GEM mapping
    * instead and can hand us a writable pointer. */
   buf->map_data = NULL;
   ptr = sym_gbm_bo_map(bo, 0, 0, buf->w, buf->h, ECORE_GBM_BO_TRANSFER_RW,
                        &map_stride, &buf->map_data);
   if (!ptr) return NULL;

   /* Evas will render at buf->stride, which is what we handed to the
    * compositor when the wl_buffer was created.  If the map disagrees we
    * would scribble past the end of each scanline, so refuse instead. */
   if (map_stride != (uint32_t)buf->stride)
     {
        ERR("gbm map stride %u does not match buffer stride %lu",
            map_stride, buf->stride);
        sym_gbm_bo_unmap(bo, buf->map_data);
        buf->map_data = NULL;
        return NULL;
     }

   return ptr;
}

static void
_gbm_unmap(Ecore_Wl2_Buffer *buf)
{
   struct gbm_bo *bo;

   bo = (struct gbm_bo *)buf->bh;
   if (!buf->map_data) return;
   sym_gbm_bo_unmap(bo, buf->map_data);
   buf->map_data = NULL;
}

static void
_gbm_discard(Ecore_Wl2_Buffer *buf)
{
   struct gbm_bo *bo;

   bo = (struct gbm_bo *)buf->bh;
   sym_gbm_bo_destroy(bo);
}

static void
_gbm_manager_destroy(void)
{
   sym_gbm_device_destroy(buffer_manager->priv);
}

static Eina_Bool
_gbm_buffer_manager_setup(int fd)
{
   Eina_Bool fail = EINA_FALSE;
   void *gbm_lib;

   /* soname, not the -dev symlink: libgbm.so is frequently absent on
    * runtime-only installs */
   gbm_lib = dlopen("libgbm.so.1", RTLD_LAZY | RTLD_GLOBAL);
   if (!gbm_lib) return EINA_FALSE;

   SYM(gbm_lib, gbm_create_device);
   SYM(gbm_lib, gbm_device_destroy);
   SYM(gbm_lib, gbm_bo_create);
   SYM(gbm_lib, gbm_bo_destroy);
   SYM(gbm_lib, gbm_bo_get_fd);
   SYM(gbm_lib, gbm_bo_get_stride);
   SYM(gbm_lib, gbm_bo_map);
   SYM(gbm_lib, gbm_bo_unmap);

   if (fail) goto err;

   /* optional, older gbm does not have it */
   sym_gbm_bo_get_modifier = dlsym(gbm_lib, "gbm_bo_get_modifier");

   buffer_manager->priv = sym_gbm_create_device(fd);
   if (!buffer_manager->priv) goto err;

   buffer_manager->alloc = _gbm_alloc;
   buffer_manager->to_buffer = _evas_dmabuf_wl_buffer_from_dmabuf;
   buffer_manager->map = _gbm_map;
   buffer_manager->unmap = _gbm_unmap;
   buffer_manager->discard = _gbm_discard;
   /* NB: not redundant with gbm_bo_map(). ecore_wl2_buffer_map() caches the
    * mapping for the buffer's lifetime, so the map happens once; lock and
    * unlock are what bracket each frame's CPU writes, and every other
    * manager wires them up for exactly that reason. */
   buffer_manager->lock = _dmabuf_lock;
   buffer_manager->unlock = _dmabuf_unlock;
   buffer_manager->manager_destroy = _gbm_manager_destroy;
   buffer_manager->dl_handle = gbm_lib;

   return EINA_TRUE;

err:
   dlclose(gbm_lib);
   return EINA_FALSE;
}

static Buffer_Handle *
_wl_shm_alloc(Buffer_Manager *self EINA_UNUSED, Ecore_Wl2_Display *ewd EINA_UNUSED, const char *name EINA_UNUSED, int w, int h, unsigned long *stride, uint64_t *modifier, int32_t *fd)
{
   Eina_Tmpstr *fullname;
   size_t size = w * h * 4;
   void *out = NULL;
   char *tmp;

   /* shm buffers never travel over the dmabuf protocol */
   *modifier = DRM_FORMAT_MOD_INVALID;

   // XXX try memfd, then shm open then the below...
   tmp = eina_vpath_resolve("(:usr.run:)/evas-wayland_shm-XXXXXX");
   *fd = eina_file_mkstemp(tmp, &fullname);
   free(tmp);

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
_vc4_alloc(Buffer_Manager *self EINA_UNUSED, Ecore_Wl2_Display *ewd EINA_UNUSED, const char *name EINA_UNUSED, int w, int h, unsigned long *stride, uint64_t *modifier, int32_t *fd)
{
   struct drm_vc4_create_bo bo;
   struct internal_vc4_bo *obo;
   struct drm_gem_close cl;
   size_t size;
   int ret;

   *modifier = DRM_FORMAT_MOD_LINEAR;
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

   if (!_drm_driver_is(fd, "vc4")) return EINA_FALSE;

   memset(&bo, 0, sizeof(bo));
   bo.size = 32;
   if (ioctl(fd, DRM_IOCTL_VC4_CREATE_BO, &bo)) return EINA_FALSE;

   memset(&cl, 0, sizeof(cl));
   cl.handle = bo.handle;
   ioctl(fd, DRM_IOCTL_GEM_CLOSE, &cl);

   drm_lib = dlopen("libdrm.so.2", RTLD_LAZY | RTLD_GLOBAL);
   if (!drm_lib) return EINA_FALSE;

   SYM(drm_lib, drmPrimeHandleToFD);

   if (fail) goto err;

   buffer_manager->alloc = _vc4_alloc;
   buffer_manager->to_buffer = _evas_dmabuf_wl_buffer_from_dmabuf;
   buffer_manager->map = _vc4_map;
   buffer_manager->unmap = _vc4_unmap;
   buffer_manager->discard = _vc4_discard;
   buffer_manager->lock = _dmabuf_lock;
   buffer_manager->unlock = _dmabuf_unlock;
   buffer_manager->manager_destroy = NULL;
   buffer_manager->dl_handle = drm_lib;
   return EINA_TRUE;
err:
   dlclose(drm_lib);
   return EINA_FALSE;
}

/* renderD128 is only the first render node the kernel hands out; on a box
 * with a discrete card, or where the display and render devices are
 * separate (rockchip + panfrost, sun4i + lima, ...), the one we want may
 * well be a higher number. */
static int
_render_node_open(void)
{
   const char *env;
   char path[64];
   int i, fd;

   env = getenv("ECORE_WL2_RENDER_NODE");
   if (env)
     {
        fd = open(env, O_RDWR | O_CLOEXEC);
        if (fd >= 0) return fd;
        ERR("ECORE_WL2_RENDER_NODE is set to %s, which could not be opened",
            env);
        return -1;
     }

   /* 128..191 is the render node minor range */
   for (i = 128; i < 192; i++)
     {
        snprintf(path, sizeof(path), "/dev/dri/renderD%d", i);
        fd = open(path, O_RDWR | O_CLOEXEC);
        if (fd >= 0) return fd;
     }

   return -1;
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
        fd = _render_node_open();
        if (fd < 0)
          {
             ERR("Tried to use dmabufs, but found no usable render node in "
                 "/dev/dri. Falling back to regular SHM");
             goto fallback_shm;
          }

        /* gbm first: it is the only one of these that covers drivers we
         * have not been taught about by name.  The three below it stay for
         * setups where gbm is unavailable, and each now checks the kernel
         * driver name before touching a driver-specific ioctl. */
        success = _gbm_buffer_manager_setup(fd);
        if (!success) success = _intel_buffer_manager_setup(fd);
        if (!success) success = _exynos_buffer_manager_setup(fd);
        if (!success) success = _vc4_buffer_manager_setup(fd);

        if (!success)
          WRN("No dmabuf allocator available for this device, falling back "
              "to regular SHM");
     }
fallback_shm:
   if (!success) success = shm && _wl_shm_buffer_manager_setup(0);
   if (!success) goto err_bm;

   drm_fd = fd;
   buffer_manager->refcount = 1;
   return EINA_TRUE;

err_bm:
   if (fd >= 0) close(fd);
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
_buffer_manager_alloc(Ecore_Wl2_Display *ewd, const char *name, int w, int h, unsigned long *stride, uint64_t *modifier, int32_t *fd)
{
   Buffer_Handle *out;

   _buffer_manager_ref();
   out = buffer_manager->alloc(buffer_manager, ewd, name, w, h, stride,
                               modifier, fd);
   if (!out) _buffer_manager_deref();
   return out;
}

EAPI struct wl_buffer *
ecore_wl2_buffer_wl_buffer_get(Ecore_Wl2_Buffer *buf)
{
   return buf->wl_buffer;
}

EAPI void *
ecore_wl2_buffer_map(Ecore_Wl2_Buffer *buf, int *w, int *h, int *stride)
{
   void *out;

   EINA_SAFETY_ON_NULL_RETURN_VAL(buf, NULL);

   if (buf->mapping)
     {
        out = buf->mapping;
     }
   else
     {
        _buffer_manager_ref();
        out = buffer_manager->map(buf);
        if (!out)
          {
             _buffer_manager_deref();
             return NULL;
          }
        buf->locked = EINA_TRUE;
        buf->mapping = out;
     }
   if (w) *w = buf->w;
   if (h) *h = buf->h;
   if (stride) *stride = (int)buf->stride;

   if (!buf->locked) ecore_wl2_buffer_lock(buf);

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
ecore_wl2_buffer_lock(Ecore_Wl2_Buffer *b)
{
   if (b->locked) ERR("Buffer already locked\n");
   if (buffer_manager->lock) buffer_manager->lock(b);
   b->locked = EINA_TRUE;
}

EAPI void
ecore_wl2_buffer_unlock(Ecore_Wl2_Buffer *b)
{
   if (!b->locked) ERR("Buffer already unlocked\n");
   if (buffer_manager->unlock) buffer_manager->unlock(b);
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

EAPI Eina_Bool
ecore_wl2_buffer_busy_get(Ecore_Wl2_Buffer *buffer)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(buffer, EINA_FALSE);

   return (buffer->locked) || (buffer->busy);
}

EAPI void
ecore_wl2_buffer_busy_set(Ecore_Wl2_Buffer *buffer)
{
   EINA_SAFETY_ON_NULL_RETURN(buffer);

   buffer->busy = EINA_TRUE;
}

EAPI int
ecore_wl2_buffer_age_get(Ecore_Wl2_Buffer *buffer)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(buffer, 0);

   return buffer->age;
}

EAPI void ecore_wl2_buffer_age_set(Ecore_Wl2_Buffer *buffer, int age)
{
   EINA_SAFETY_ON_NULL_RETURN(buffer);

   buffer->age = age;
}

EAPI void
ecore_wl2_buffer_age_inc(Ecore_Wl2_Buffer *buffer)
{
   EINA_SAFETY_ON_NULL_RETURN(buffer);

   buffer->age++;
}

/* The only user of this function has been removed, but it
 * will likely come back later.  The problem is that
 * a dmabuf buffer needs to be resized on the compositor
 * even if the allocation still fits.  Doing the resize
 * properly isn't something that will be fixed in the 1.21
 * timeframe, so the optimization has been (temporarily)
 * removed.
 *
 * This is currently beta api - don't move it out of beta
 * with no users...
 */
EAPI Eina_Bool
ecore_wl2_buffer_fit(Ecore_Wl2_Buffer *b, int w, int h)
{
   int stride;

   EINA_SAFETY_ON_NULL_RETURN_VAL(b, EINA_FALSE);

   stride = b->stride;
   if ((w >= b->w) && (w <= stride / 4) && (h == b->h))
     {
        b->w = w;
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Ecore_Wl2_Buffer *
_ecore_wl2_buffer_partial_create(Ecore_Wl2_Display *ewd, int w, int h, Eina_Bool alpha)
{
   Ecore_Wl2_Buffer *out;

   out = calloc(1, sizeof(Ecore_Wl2_Buffer));
   if (!out) return NULL;

   out->fd = -1;
   out->alpha = alpha;
   out->modifier = DRM_FORMAT_MOD_INVALID;
   out->bh = _buffer_manager_alloc(ewd, "name", w, h, &out->stride,
                                   &out->modifier, &out->fd);
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

   out = _ecore_wl2_buffer_partial_create(ewd, w, h, alpha);
   if (!out) return NULL;

   out->wl_buffer = buffer_manager->to_buffer(ewd, out);

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

   /* ecore_wl2_buffer_init() hands back the pre-existing manager if one is
    * already up.  If that manager is the shm one its "fd" is a memfd, not a
    * dmabuf, and offering it over the dmabuf protocol would be nonsense.
    * NB: drop our reference and give up on dmabuf for this display only -
    * buffer_manager is process wide, so tearing it down here would poison
    * it for every other Ecore_Wl2_Display still using it. */
   if (buffer_manager->to_buffer != _evas_dmabuf_wl_buffer_from_dmabuf)
     {
        _buffer_manager_deref();
        ewd->wl.dmabuf = NULL;
        return;
     }

   buf = _ecore_wl2_buffer_partial_create(ewd, 1, 1, EINA_TRUE);
   if (!buf) goto fail;

   /* No modifier events can have arrived yet - we are still inside the
    * registry global handler - so probe the implicit path. */
   dp = zwp_linux_dmabuf_v1_create_params(ewd->wl.dmabuf);
   zwp_linux_buffer_params_v1_add(dp, buf->fd, 0, 0, buf->stride,
                                  (uint32_t)(DRM_FORMAT_MOD_INVALID >> 32),
                                  (uint32_t)(DRM_FORMAT_MOD_INVALID & 0xFFFFFFFF));
   zwp_linux_buffer_params_v1_add_listener(dp, &params_listener, ewd);
   zwp_linux_buffer_params_v1_create(dp, buf->w, buf->h,
                                     DRM_FORMAT_ARGB8888, 0);

   ecore_wl2_buffer_destroy(buf);

   return;

fail:
  _buffer_manager_destroy();
  ewd->wl.dmabuf = NULL;
}
