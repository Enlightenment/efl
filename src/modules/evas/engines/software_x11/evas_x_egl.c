#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "evas_common_private.h"
#include "evas_macros.h"

#ifdef HAVE_DLSYM
# include <dlfcn.h> /* dlopen,dlclose,etc */
#else
# undef BUILD_ENGINE_SOFTWARE_XLIB
#endif

#ifdef BUILD_ENGINE_SOFTWARE_XLIB
#include "evas_x_egl.h"

#define EGL_SURFACE_TYPE                      0x3033
#define EGL_WINDOW_BIT                        0x0004
#define EGL_RENDERABLE_TYPE                   0x3040
#define EGL_ALPHA_SIZE                        0x3021
#define EGL_BLUE_SIZE                         0x3022
#define EGL_GREEN_SIZE                        0x3023
#define EGL_RED_SIZE                          0x3024
#define EGL_DEPTH_SIZE                        0x3025
#define EGL_STENCIL_SIZE                      0x3026
#define EGL_SURFACE_TYPE                      0x3033
#define EGL_NONE                              0x3038
#define EGL_FALSE                             0
#define EGL_TRUE                              1

#define EGL_LOCK_SURFACE_BIT_KHR              0x0080
#define EGL_OPTIMAL_FORMAT_BIT_KHR            0x0100
#define EGL_MATCH_FORMAT_KHR                  0x3043
#define EGL_FORMAT_RGB_565_EXACT_KHR          0x30C0
#define EGL_FORMAT_RGB_565_KHR                0x30C1
#define EGL_FORMAT_RGBA_8888_EXACT_KHR        0x30C2
#define EGL_FORMAT_RGBA_8888_KHR              0x30C3
#define EGL_MAP_PRESERVE_PIXELS_KHR           0x30C4
#define EGL_LOCK_USAGE_HINT_KHR               0x30C5
#define EGL_READ_SURFACE_BIT_KHR              0x0001
#define EGL_WRITE_SURFACE_BIT_KHR             0x0002
#define EGL_BITMAP_POINTER_KHR                0x30C6
#define EGL_BITMAP_PITCH_KHR                  0x30C7
#define EGL_BITMAP_ORIGIN_KHR                 0x30C8
#define EGL_BITMAP_PIXEL_RED_OFFSET_KHR       0x30C9
#define EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR     0x30CA
#define EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR      0x30CB
#define EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR     0x30CC
#define EGL_BITMAP_PIXEL_LUMINANCE_OFFSET_KHR 0x30CD
#define EGL_LOWER_LEFT_KHR                    0x30CE
#define EGL_UPPER_LEFT_KHR                    0x30CF

static int egl_found = -1;
static void *egl_lib = NULL;

static struct
{
   void       * (*GetProcAddress)(const char *name);
   void       * (*GetDisplay)(void *d);
   unsigned int (*Initialize)(void *ed, int *vmaj, int *vmin);
   unsigned int (*Terminate)(void *ed);
   const char * (*QueryString)(void *ed, int name);
   unsigned int (*ChooseConfig)(void *ed, int *attr, void **configs, int config_size, int *num_config);
   unsigned int (*GetConfigAttrib)(void *ed, void *config, int attr, int *val);
   unsigned int (*QuerySurface)(void *ed, void *surf, int attr, int *val);
   void       * (*CreateWindowSurface)(void *ed, void *config, Window win, int *attr);
   unsigned int (*DestroySurface)(void *ed, void *surf);
   unsigned int (*SwapBuffers)(void *ed, void *surf);
   unsigned int (*SwapInterval)(void *ed, int interval);

   unsigned int (*LockSurface)(void *ed, void *surf, int *attr);
   unsigned int (*UnlockSurface)(void *ed, void *surf);
} egl;

static int
_egl_find(void)
{
   if (egl_found == 0) return 0;
   if (!egl_lib) egl_lib = dlopen("libEGL.so.1", RTLD_NOW | RTLD_LOCAL);
   if (!egl_lib)
     {
        egl_found = 0;
        return 0;
     }
   if (!(egl.GetProcAddress = dlsym(egl_lib, "eglGetProcAddress"))) goto err;

#define SYM(x, y) if (!(egl.x = egl.GetProcAddress(y))) \
    goto err
// core syms used
   SYM(GetDisplay, "eglGetDisplay");
   SYM(Initialize, "eglInitialize");
   SYM(Terminate, "eglTerminate");
   SYM(QueryString, "eglQueryString");
   SYM(ChooseConfig, "eglChooseConfig");
   SYM(UnlockSurface, "eglGetConfigAttrib");
   SYM(QuerySurface, "eglQuerySurface");
   SYM(CreateWindowSurface, "eglCreateWindowSurface");
   SYM(DestroySurface, "eglDestroySurface");
   SYM(SwapBuffers, "eglSwapBuffers");
   SYM(SwapInterval, "eglSwapInterval");

#undef SYM
#define SYM(x, y) egl.x = egl.GetProcAddress(y)
// extns
   SYM(LockSurface, "eglLockSurface");
   if (!egl.LockSurface) SYM(LockSurface, "eglLockSurfaceKHR");
   SYM(UnlockSurface, "eglUnlockSurface");
   if (!egl.UnlockSurface) SYM(UnlockSurface, "eglUnlockSurfaceKHR");

   if (!egl.LockSurface) goto err;
   if (!egl.UnlockSurface) goto err;

   egl_found = 1;
   return 1;
err:
   if (egl_lib) dlclose(egl_lib);
   egl_lib = NULL;
   return 0;
}

#endif

void *
_egl_x_disp_get(void *d)
{
#ifdef BUILD_ENGINE_SOFTWARE_XLIB
   if (!_egl_find()) return NULL;
   return egl.GetDisplay(d);
#else
   return NULL;
   (void)d;
#endif
}

void
_egl_x_disp_terminate(void *ed)
{
#ifdef BUILD_ENGINE_SOFTWARE_XLIB
   if (!_egl_find()) return;
   egl.Terminate(ed);
#else
   (void)ed;
#endif
}

int
_egl_x_disp_init(void *ed)
{
#ifdef BUILD_ENGINE_SOFTWARE_XLIB
   int vmaj = 0, vmin = 0;
   if (!_egl_find()) return 0;
   if (!egl.Initialize(ed, &vmaj, &vmin)) return 0;
   return 1;
#else
   return 0;
   (void)ed;
#endif
}

void *
_egl_x_disp_choose_config(void *ed)
{
#ifdef BUILD_ENGINE_SOFTWARE_XLIB
   int config_attrs[40], n = 0, num_config = 0;
   void *eglconfig = NULL;

   if (!_egl_find()) return NULL;
   config_attrs[n++] = EGL_SURFACE_TYPE;
   config_attrs[n++] = EGL_WINDOW_BIT;
   config_attrs[n++] = EGL_RED_SIZE;
   config_attrs[n++] = 8;
   config_attrs[n++] = EGL_GREEN_SIZE;
   config_attrs[n++] = 8;
   config_attrs[n++] = EGL_BLUE_SIZE;
   config_attrs[n++] = 8;
   config_attrs[n++] = EGL_ALPHA_SIZE;
   config_attrs[n++] = 8;
   config_attrs[n++] = EGL_DEPTH_SIZE;
   config_attrs[n++] = 0;
   config_attrs[n++] = EGL_STENCIL_SIZE;
   config_attrs[n++] = 0;
   config_attrs[n++] = EGL_SURFACE_TYPE;
   config_attrs[n++] = EGL_LOCK_SURFACE_BIT_KHR;
   config_attrs[n++] = EGL_MATCH_FORMAT_KHR;
   config_attrs[n++] = EGL_FORMAT_RGBA_8888_KHR;

   config_attrs[n++] = EGL_NONE;

   if (!egl.ChooseConfig(ed, config_attrs, &eglconfig, 1, &num_config))
     return NULL;
   return eglconfig;
#else
   return NULL;
   (void)ed;
#endif
}

#ifdef BUILD_ENGINE_SOFTWARE_XLIB
void *
_egl_x_win_surf_new(void *ed, Window win, void *config)
{
   if (!_egl_find()) return NULL;
   return egl.CreateWindowSurface(ed, config, win, NULL);
}

#endif

void
_egl_x_win_surf_free(void *ed, void *surf)
{
#ifdef BUILD_ENGINE_SOFTWARE_XLIB
   if (!_egl_find()) return;
   egl.DestroySurface(ed, surf);
#else
   (void)ed;
   (void)surf;
#endif
}

void *
_egl_x_surf_map(void *ed, void *surf, int *stride)
{
#ifdef BUILD_ENGINE_SOFTWARE_XLIB
   int config_attrs[40], n = 0;
   void *ptr = NULL;
   int pitch = 0, origin = 0;
   int r_offset = 0, g_offset = 0, b_offset = 0;

   if (!_egl_find()) return NULL;

   config_attrs[n++] = EGL_MAP_PRESERVE_PIXELS_KHR;
   config_attrs[n++] = EGL_TRUE;
   config_attrs[n++] = EGL_LOCK_USAGE_HINT_KHR;
   config_attrs[n++] = EGL_READ_SURFACE_BIT_KHR | EGL_WRITE_SURFACE_BIT_KHR;
   config_attrs[n++] = EGL_NONE;

   if (!egl.LockSurface(ed, surf, config_attrs)) return NULL;
   if (!egl.QuerySurface(ed, surf, EGL_BITMAP_POINTER_KHR, (int *)&ptr)) goto err;
   if (!egl.QuerySurface(ed, surf, EGL_BITMAP_PITCH_KHR, &pitch)) goto err;
   if (!egl.QuerySurface(ed, surf, EGL_BITMAP_ORIGIN_KHR, &origin)) goto err;
   if (!egl.QuerySurface(ed, surf, EGL_BITMAP_PIXEL_RED_OFFSET_KHR, &r_offset)) goto err;
   if (!egl.QuerySurface(ed, surf, EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR, &g_offset)) goto err;
   if (!egl.QuerySurface(ed, surf, EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR, &b_offset)) goto err;

   if (!ptr) goto err;
   if (pitch <= 0) goto err;
   // must be top-left to bottom-right ordered
   if (origin != EGL_UPPER_LEFT_KHR) goto err;
   // must be xRGB
   if (!((b_offset == 0) && (g_offset == 8) && (r_offset == 16))) goto err;
   // return stride
   *stride = pitch; // pitch is in bytes
   return ptr;
err:
   egl.UnlockSurface(ed, surf);
   return NULL;
#else
   return NULL;
   (void)ed;
   (void)surf;
   (void)stride;
#endif
}

void
_egl_x_surf_unmap(void *ed, void *surf)
{
#ifdef BUILD_ENGINE_SOFTWARE_XLIB
   egl.UnlockSurface(ed, surf);
#else
   (void)ed;
   (void)surf;
#endif
}

void
_egl_x_surf_swap(void *ed, void *surf, int vsync)
{
#ifdef BUILD_ENGINE_SOFTWARE_XLIB
   if (vsync) egl.SwapInterval(ed, 1);
   else egl.SwapInterval(ed, 0);
   egl.SwapBuffers(ed, surf);
#else
   (void)ed;
   (void)surf;
   (void)vsync;
#endif
}

#ifdef BUILD_ENGINE_SOFTWARE_XLIB
Outbuf *
evas_software_egl_outbuf_setup_x(int w, int h, int rot, Outbuf_Depth depth,
                                 Display *disp, Drawable draw, Visual *vis,
                                 Colormap cmap, int x_depth,
                                 int grayscale, int max_colors, Pixmap mask,
                                 int shape_dither, int destination_alpha)
{
   (void)w;
   (void)h;
   (void)rot;
   (void)depth;
   (void)draw;
   (void)cmap;
   (void)x_depth;
   (void)grayscale;
   (void)max_colors;
   (void)mask;
   (void)shape_dither;
   (void)destination_alpha;
   (void)disp;
   (void)vis;
   return NULL;
}

#endif
