#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_common_private.h"
#include "evas_xlib_dri_image.h"
#include "../software_generic/evas_native_common.h"

# include <dlfcn.h> /* dlopen,dlclose,etc */
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>

static Eina_Bool tried = EINA_FALSE;
////////////////////////////////////
//libdrm.so.2
static void *drm_lib = NULL;

typedef unsigned int drm_magic_t;
static int (*sym_drmGetMagic)(int fd, drm_magic_t *magic) = NULL;

////////////////////////////////////
// libtbm.so.1
#define TBM_DEVICE_CPU   1
#define TBM_OPTION_READ  (1 << 0)
#define TBM_OPTION_WRITE (1 << 1)
static void *lib_tbm = NULL;

static tbm_bo (*sym_tbm_bo_import)(tbm_bufmgr bufmgr, unsigned int key) = NULL;
static tbm_bo_handle (*sym_tbm_bo_map)(tbm_bo bo, int device, int opt) = NULL;
static int (*sym_tbm_bo_unmap)(tbm_bo bo) = NULL;
static void (*sym_tbm_bo_unref)(tbm_bo bo) = NULL;
static tbm_bufmgr (*sym_tbm_bufmgr_init)(int fd) = NULL;
static void (*sym_tbm_bufmgr_deinit)(tbm_bufmgr bufmgr) = NULL;

// legacy compatibility
static void *(*sym_drm_slp_bo_map)(tbm_bo bo, int device, int opt) = NULL;
static int (*sym_drm_slp_bo_unmap)(tbm_bo bo, int device) = NULL;
static tbm_bufmgr (*sym_drm_slp_bufmgr_init)(int fd, void *arg) = NULL;

////////////////////////////////////
// libdri2.so.0
#define DRI2BufferFrontLeft 0
static void *dri_lib = NULL;

typedef unsigned long long CD64;

static DRI2Buffer *(*sym_DRI2GetBuffers)(Display * display, XID drawable, int *width, int *height, unsigned int *attachments, int count, int *outCount) = NULL;
static Bool (*sym_DRI2QueryExtension)(Display *display, int *eventBase, int *errorBase) = NULL;
static Bool (*sym_DRI2QueryVersion)(Display *display, int *major, int *minor) = NULL;
static Bool (*sym_DRI2Connect)(Display *display, XID window, char **driverName, char **deviceName) = NULL;
static Bool (*sym_DRI2Authenticate)(Display *display, XID window, unsigned int magic) = NULL;
static void (*sym_DRI2CreateDrawable)(Display *display, XID drawable) = NULL;
static void (*sym_DRI2DestroyDrawable)(Display *display, XID handle) = NULL;

////////////////////////////////////
// libXfixes.so.3
static void *xfixes_lib = NULL;

static Bool (*sym_XFixesQueryExtension)(Display *display, int *event_base_return, int *error_base_return) = NULL;
static Status (*sym_XFixesQueryVersion)(Display *display, int *major_version_return, int *minor_version_return) = NULL;
static XID (*sym_XFixesCreateRegion)(Display *display, XRectangle *rectangles, int nrectangles) = NULL;
static void (*sym_XFixesDestroyRegion)(Display *dpy, XID region) = NULL;

static int inits = 0;
static int xfixes_ev_base = 0, xfixes_err_base = 0;
static int xfixes_major = 0, xfixes_minor = 0;
static int dri2_ev_base = 0, dri2_err_base = 0;
static int dri2_major = 0, dri2_minor = 0;
static int drm_fd = -1;
static tbm_bufmgr bufmgr = NULL;
static int exim_debug = -1;
static Eina_Bool use_cache = EINA_TRUE;
static Eina_Bool slp_mode = EINA_FALSE;

static Eina_Bool
_drm_init(Display *disp, int scr)
{
   char *drv_name = NULL, *dev_name = NULL;
   drm_magic_t magic = 0;

   if (xfixes_lib) return EINA_TRUE;
   if ((tried) && (!xfixes_lib)) return EINA_FALSE;
   if (tried) return EINA_FALSE;
   tried = EINA_TRUE;
   drm_lib = dlopen("libdrm.so.2", RTLD_NOW | RTLD_LOCAL);
   if (!drm_lib)
     {
        ERR("Can't load libdrm.so.2");
        goto err;
     }
   slp_mode = EINA_FALSE;
   lib_tbm = dlopen("libtbm.so.1", RTLD_NOW | RTLD_LOCAL);
   if (!lib_tbm)
     {
        ERR("Can't load libtbm.so.1");
        lib_tbm = dlopen("libdrm_slp.so.1", RTLD_NOW | RTLD_LOCAL);
        if (lib_tbm) slp_mode = EINA_TRUE;
        else goto err;
     }
   dri_lib = dlopen("libdri2.so.0", RTLD_NOW | RTLD_LOCAL);
   if (!dri_lib)
     {
        ERR("Can't load libdri2.so.0");
        goto err;
     }

   xfixes_lib = dlopen("libXfixes.so.3", RTLD_NOW | RTLD_LOCAL);
   if (!xfixes_lib)
     {
        ERR("Can't load libXfixes.so.3");
        goto err;
     }

#define SYM(l, x)                         \
  do { sym_ ## x = dlsym(l, #x);          \
       if (!sym_ ## x) {                  \
            ERR("Can't load symbol " #x); \
            goto err;                     \
         }                                \
    } while (0)

   SYM(drm_lib, drmGetMagic);

   if (!slp_mode)
     {
        SYM(lib_tbm, tbm_bo_import);
        SYM(lib_tbm, tbm_bo_map);
        SYM(lib_tbm, tbm_bo_unmap);
        SYM(lib_tbm, tbm_bo_unref);
        SYM(lib_tbm, tbm_bufmgr_init);
        SYM(lib_tbm, tbm_bufmgr_deinit);
     }
   else
     {
        // Looking up the legacy DRM SLP symbols. I don't believe this will
        // ever happen, this code is here "just in case".
        sym_tbm_bo_import = dlsym(lib_tbm, "drm_slp_bo_import");
        sym_drm_slp_bo_map = dlsym(lib_tbm, "drm_slp_bo_map");
        sym_drm_slp_bo_unmap = dlsym(lib_tbm, "drm_slp_bo_unmap");
        sym_tbm_bo_unref = dlsym(lib_tbm, "drm_slp_bo_unref");
        sym_drm_slp_bufmgr_init = dlsym(lib_tbm, "drm_slp_bufmgr_init");
        sym_tbm_bufmgr_deinit = dlsym(lib_tbm, "drm_slp_bufmgr_destroy");
        if (!sym_tbm_bo_import || !sym_drm_slp_bo_map || !sym_drm_slp_bo_unmap ||
            !sym_tbm_bo_unref || !sym_drm_slp_bufmgr_init || !sym_tbm_bufmgr_deinit)
          {
             ERR("Can't load symbols from libdrm_slp.so.1");
             goto err;
          }
     }

   SYM(dri_lib, DRI2GetBuffers);
   SYM(dri_lib, DRI2QueryExtension);
   SYM(dri_lib, DRI2QueryVersion);
   SYM(dri_lib, DRI2Connect);
   SYM(dri_lib, DRI2Authenticate);
   SYM(dri_lib, DRI2CreateDrawable);
   SYM(dri_lib, DRI2DestroyDrawable);

   SYM(xfixes_lib, XFixesQueryExtension);
   SYM(xfixes_lib, XFixesQueryVersion);
   SYM(xfixes_lib, XFixesCreateRegion);
   SYM(xfixes_lib, XFixesDestroyRegion);
   if (!sym_XFixesQueryExtension(disp, &xfixes_ev_base, &xfixes_err_base))
     {
        if (exim_debug) ERR("XFixes extension not in xserver");
        goto err;
     }
   sym_XFixesQueryVersion(disp, &xfixes_major, &xfixes_minor);

   if (!sym_DRI2QueryExtension(disp, &dri2_ev_base, &dri2_err_base))
     {
        if (exim_debug) ERR("DRI2 extension not in xserver");
        goto err;
     }
   if (!sym_DRI2QueryVersion(disp, &dri2_major, &dri2_minor))
     {
        if (exim_debug) ERR("DRI2 query version failed");
        goto err;
     }
   if (dri2_minor < 99)
     {
        if (exim_debug)
          ERR("Not supported by DRI2 version(%i.%i)",
              dri2_major, dri2_minor);
        goto err;
     }
   if (!sym_DRI2Connect(disp, RootWindow(disp, scr), &drv_name, &dev_name))
     {
        if (exim_debug) ERR("DRI2 connect failed on screen %i", scr);
        goto err;
     }
   if (!dev_name)
     {
        if (exim_debug) ERR("DRI2 connect - cannot find dev name");
        goto err;
     }
   drm_fd = open(dev_name, O_RDWR);
   if (drm_fd < 0)
     {
        if (exim_debug) ERR("DRM FD open of '%s' failed", dev_name);
        goto err;
     }
   if (sym_drmGetMagic(drm_fd, &magic))
     {
        if (exim_debug) ERR("DRM get magic failed");
        goto err;
     }
   if (!sym_DRI2Authenticate(disp, RootWindow(disp, scr),
                             (unsigned int)magic))
     {
        if (exim_debug) ERR("DRI2 authenticate failed with magic 0x%x on screen %i", (unsigned int)magic, scr);
        goto err;
     }
   if (!slp_mode)
     bufmgr = sym_tbm_bufmgr_init(drm_fd);
   else
     bufmgr = sym_drm_slp_bufmgr_init(drm_fd, NULL);
   if (!bufmgr)
     {
        if (exim_debug) ERR("DRM bufmgr init failed");
        goto err;
     }
   if (drv_name)
     {
        XFree(drv_name);
     }
   if (dev_name)
     {
        XFree(dev_name);
     }
   return EINA_TRUE;
err:
   if (drm_fd >= 0)
     {
        close(drm_fd);
        drm_fd = -1;
     }
   if (drm_lib)
     {
        dlclose(drm_lib);
        drm_lib = NULL;
     }
   if (lib_tbm)
     {
        dlclose(lib_tbm);
        lib_tbm = NULL;
     }
   if (dri_lib)
     {
        dlclose(dri_lib);
        dri_lib = NULL;
     }
   if (xfixes_lib)
     {
        dlclose(xfixes_lib);
        xfixes_lib = NULL;
     }
   if (drv_name)
     {
        XFree(drv_name);
     }
   if (dev_name)
     {
        XFree(dev_name);
     }
   return EINA_FALSE;
}

static void
_drm_shutdown(void)
{
   if (bufmgr)
     {
        sym_tbm_bufmgr_deinit(bufmgr);
        bufmgr = NULL;
     }
   if (drm_fd >= 0) close(drm_fd);
   tried = EINA_FALSE;
   drm_fd = -1;
   dlclose(lib_tbm);
   lib_tbm = NULL;
   dlclose(dri_lib);
   dri_lib = NULL;
   dlclose(xfixes_lib);
   xfixes_lib = NULL;
}

static Eina_Bool
_drm_setup(Display *disp, Evas_DRI_Image *exim)
{
   sym_DRI2CreateDrawable(disp, exim->draw);
   return EINA_TRUE;
}

static void
_drm_cleanup(Evas_DRI_Image *exim)
{
   sym_DRI2DestroyDrawable(exim->dis, exim->draw);
}

Eina_Bool
evas_xlib_image_dri_init(Evas_DRI_Image *exim,
                         Display *display)
{
   exim->dis = display;
   if (inits <= 0)
     {
        if (!_drm_init(display, 0)) return EINA_FALSE;
     }
   inits++;

   if (!_drm_setup(display, exim))
     {
        inits--;
        if (inits == 0) _drm_shutdown();
        free(exim);
        return EINA_FALSE;
     }

   if (getenv("EVAS_NO_DRI2_CACHE"))
     {
        use_cache = EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
evas_xlib_image_dri_used()
{
   if (inits > 0) return EINA_TRUE;
   return EINA_FALSE;
}

void
evas_xlib_image_buffer_unmap(Evas_DRI_Image *exim)
{
   if (!slp_mode)
     sym_tbm_bo_unmap(exim->buf_bo);
   else
     sym_drm_slp_bo_unmap(exim->buf_bo, TBM_DEVICE_CPU);
   if (exim_debug) DBG("Unmap buffer name %i\n", exim->buf->name);
   free(exim->buf);
   exim->buf = NULL;
   exim->buf_data = NULL;
}

Eina_Bool
_evas_xlib_image_cache_import(Evas_DRI_Image *exim)
{
   DRI2BufferFlags *flags;
   exim->buf_bo = NULL;
   flags = (DRI2BufferFlags *)(&(exim->buf->flags));
   if (!flags->data.is_reused)
     {
        if (exim_debug) DBG("Buffer cache not reused - clear cache\n");
        if (exim->buf_cache)
          {
             sym_tbm_bo_unref(exim->buf_cache->buf_bo);
             free(exim->buf_cache);
          }
     }
   else
     {
        if (exim->buf_cache && exim->buf_cache->name == exim->buf->name)
          {
             if (exim_debug) DBG("Cached buf name %i found\n", exim->buf_cache->name);
             exim->buf_bo = exim->buf_cache->buf_bo;
          }
        else
          {
             if (exim->buf_cache)
               {
                  sym_tbm_bo_unref(exim->buf_cache->buf_bo);
                  free(exim->buf_cache);
               }
          }
     }

   if (!exim->buf_bo)
     {
        exim->buf_bo = sym_tbm_bo_import(bufmgr, exim->buf->name);
        if (!exim->buf_bo) return EINA_FALSE;
        // cache the buf entry
        exim->buf_cache = calloc(1, sizeof(Buffer));
        if (!exim->buf_cache) return EINA_FALSE;
        exim->buf_cache->name = exim->buf->name;
        exim->buf_cache->buf_bo = exim->buf_bo;
        if (exim_debug) DBG("Buffer cache added name %i\n", exim->buf_cache->name);
     }
   return EINA_TRUE;
}

Eina_Bool
_evas_xlib_image_no_cache_import(Evas_DRI_Image *exim)
{
   if (exim->buf_bo) sym_tbm_bo_unref(exim->buf_bo);
   exim->buf_bo = sym_tbm_bo_import(bufmgr, exim->buf->name);
   if (!exim->buf_bo) return EINA_FALSE;
   return EINA_TRUE;
}

Eina_Bool
_evas_xlib_image_x_free(Display *d)
{
   XUngrabServer(d);
   XSync(d, 0);
   return EINA_FALSE;
}

Eina_Bool
evas_xlib_image_get_buffers(RGBA_Image *im)
{
   Native *n = NULL;
   Display *d;
   Evas_DRI_Image *exim;

   if (im->native.data)
     n = im->native.data;
   if (!n) return EINA_FALSE;

   exim = n->ns_data.x11.exim;
   d = n->ns_data.x11.display;

   if (!exim) return EINA_FALSE;

   unsigned int attach = DRI2BufferFrontLeft;
   int num;
   tbm_bo_handle bo_handle;

   XGrabServer(d);
   exim->buf = sym_DRI2GetBuffers(d, exim->draw,
                                  &(exim->buf_w), &(exim->buf_h),
                                  &attach, 1, &num);

   if (!exim->buf) return _evas_xlib_image_x_free(d);
   if (!exim->buf->name) return _evas_xlib_image_x_free(d);

   if (use_cache)
     {
        if (!_evas_xlib_image_cache_import(exim)) return _evas_xlib_image_x_free(d);
     }
   else
     {
        if (!_evas_xlib_image_no_cache_import(exim)) return _evas_xlib_image_x_free(d);
     }

   if (!slp_mode)
     {
        bo_handle = sym_tbm_bo_map(exim->buf_bo, TBM_DEVICE_CPU, TBM_OPTION_READ | TBM_OPTION_WRITE);
        if (bo_handle.ptr == NULL) return _evas_xlib_image_x_free(d);
        exim->buf_data = bo_handle.ptr;
     }
   else
     {
        exim->buf_data = sym_drm_slp_bo_map(exim->buf_bo, TBM_DEVICE_CPU, TBM_OPTION_READ | TBM_OPTION_WRITE);
     }
   if (!exim->buf_data)
     {
        ERR("Buffer map name %i failed", exim->buf->name);
        return _evas_xlib_image_x_free(d);
     }

   XUngrabServer(d);
   XSync(d, 0);

   im->image.data = exim->buf_data;
   im->cache_entry.w = exim->buf->pitch / 4;

   evas_xlib_image_buffer_unmap(exim);

   return EINA_TRUE;
}

void
evas_xlib_image_dri_free(Evas_DRI_Image *exim)
{
   if (use_cache)
     {
        if (exim->buf_cache)
          {
             if (exim_debug) DBG("Cached buf name %i freed\n", exim->buf_cache->name);
             sym_tbm_bo_unref(exim->buf_cache->buf_bo);
             free(exim->buf_cache);
          }
     }
   else
     {
        if (exim->buf_bo) sym_tbm_bo_unref(exim->buf_bo);
     }

   _drm_cleanup(exim);
   free(exim);
   inits--;
   if (inits == 0) _drm_shutdown();
}

Evas_DRI_Image *
evas_xlib_image_dri_new(int w, int h, Visual *vis, int depth)
{
   Evas_DRI_Image *exim;

   exim = calloc(1, sizeof(Evas_DRI_Image));
   if (!exim)
     return NULL;

   exim->w = w;
   exim->h = h;
   exim->visual = vis;
   exim->depth = depth;
   return exim;
}

static void
_native_bind_cb(void *image, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   RGBA_Image *im = image;
   Native *n = im->native.data;

   if ((n) && (n->ns.type == EVAS_NATIVE_SURFACE_X11))
     {
        if (evas_xlib_image_get_buffers(im))
          {
             evas_common_image_colorspace_dirty(im);
          }
     }
}

static void
_native_free_cb(void *image)
{
   RGBA_Image *im = image;
   Native *n = im->native.data;
   if (!n) return;
   if (n->ns_data.x11.exim)
     {
        evas_xlib_image_dri_free(n->ns_data.x11.exim);
        n->ns_data.x11.exim = NULL;
     }
   n->ns_data.x11.visual = NULL;
   n->ns_data.x11.display = NULL;

   im->native.data = NULL;
   im->native.func.bind = NULL;
   im->native.func.free = NULL;
   im->image.data = NULL;
   free(n);
}

void *
evas_xlib_image_dri_native_set(void *data, void *image, void *native)
{
   Display *d = NULL;
   Visual *vis = NULL;
   Pixmap pm = 0;
   Native *n = NULL;
   RGBA_Image *im = image;
   int w, h;
   Evas_DRI_Image *exim;
   Evas_Native_Surface *ns = native;
   Outbuf *ob = (Outbuf *)data;

   Window wdum;
   int idum;
   unsigned int uidum, depth = 0;

   if (!ns || ns->type != EVAS_NATIVE_SURFACE_X11)
     return NULL;

   d = ob->priv.x11.xlib.disp;
   vis = ns->data.x11.visual;
   pm = ns->data.x11.pixmap;
   if (!pm) return NULL;

   XGetGeometry(d, pm, &wdum, &idum, &idum, &uidum, &uidum, &uidum, &depth);

   w = im->cache_entry.w;
   h = im->cache_entry.h;

   exim = evas_xlib_image_dri_new(w, h, vis, depth);

   if (!exim)
     {
        ERR("evas_xlib_image_dri_new failed.");
        return NULL;
     }

   exim->draw = (Drawable)ns->data.x11.pixmap;

   n = calloc(1, sizeof(Native));
   if (!n)
     {
        evas_xlib_image_dri_free(exim);
        return NULL;
     }

   memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
   n->ns_data.x11.pixmap = pm;
   n->ns_data.x11.visual = vis;
   n->ns_data.x11.display = d;
   n->ns_data.x11.exim = exim;
   im->native.data = n;
   im->native.func.bind = _native_bind_cb;
   im->native.func.free = _native_free_cb;

   if (evas_xlib_image_dri_init(exim, d)) evas_xlib_image_get_buffers(im);
   else return NULL;
   return im;
}

