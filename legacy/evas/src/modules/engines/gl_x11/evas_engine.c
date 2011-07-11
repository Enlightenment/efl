#include "evas_common.h" /* Also includes international specific stuff */
#include "evas_engine.h"

#include <dlfcn.h>      /* dlopen,dlclose,etc */
#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
// EGL / GLES
# if defined(GLES_VARIETY_S3C6410)
# elif defined(GLES_VARIETY_SGX)
# endif
#else
// GLX
#endif

typedef struct _Render_Engine               Render_Engine;
typedef struct _Render_Engine_GL_Surface    Render_Engine_GL_Surface;
typedef struct _Render_Engine_GL_Context    Render_Engine_GL_Context;

struct _Render_Engine
{
   Evas_GL_X11_Window      *win;
   Evas_Engine_Info_GL_X11 *info;
   Evas                    *evas;
   int                      end;

   XrmDatabase   xrdb; // xres - dpi
   struct { // xres - dpi
      int        dpi; // xres - dpi
   } xr; // xres - dpi

   int w, h;
   int vsync;
};

struct _Render_Engine_GL_Surface
{
   int     initialized;
   int     fbo_attached;
   int     w, h;
   int     depth_bits;
   int     stencil_bits;

   // Render target texture/buffers
   GLuint  rt_tex;
   GLint   rt_internal_fmt;
   GLenum  rt_fmt;
   GLuint  rb_depth;
   GLenum  rb_depth_fmt;
   GLuint  rb_stencil;
   GLenum  rb_stencil_fmt;

   Render_Engine_GL_Context   *current_ctx;
};

struct _Render_Engine_GL_Context
{
   int         initialized;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   EGLContext  context;
#else
   GLXContext  context;
#endif
   GLuint      fbo;

   Render_Engine_GL_Surface   *current_sfc;
};

static int initted = 0;
static int gl_wins = 0;

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)

#ifndef EGL_NATIVE_PIXMAP_KHR
# define EGL_NATIVE_PIXMAP_KHR 0x30b0
#endif
typedef void (*_eng_fn) (void);

typedef _eng_fn (*glsym_func_eng_fn) ();
typedef void    (*glsym_func_void) ();
typedef void   *(*glsym_func_void_ptr) ();

_eng_fn  (*glsym_eglGetProcAddress)            (const char *a) = NULL;
void     (*glsym_eglBindTexImage)              (EGLDisplay a, EGLSurface b, int c) = NULL;
void     (*glsym_eglReleaseTexImage)           (EGLDisplay a, EGLSurface b, int c) = NULL;
void    *(*glsym_eglCreateImage)               (EGLDisplay a, EGLContext b, EGLenum c, EGLClientBuffer d, const int *e) = NULL;
void     (*glsym_eglDestroyImage)              (EGLDisplay a, void *b) = NULL;
void     (*glsym_glEGLImageTargetTexture2DOES) (int a, void *b)  = NULL;
#else
typedef void (*_eng_fn) (void);

typedef _eng_fn (*glsym_func_eng_fn) ();
typedef void    (*glsym_func_void) ();
typedef int     (*glsym_func_int) ();
typedef XID     (*glsym_func_xid) ();
typedef unsigned int     (*glsym_func_uint) ();
typedef unsigned char    (*glsym_func_uchar) ();
typedef unsigned char   *(*glsym_func_uchar_ptr) ();

_eng_fn  (*glsym_glXGetProcAddress)  (const char *a) = NULL;
void     (*glsym_glXBindTexImage)    (Display *a, GLXDrawable b, int c, int *d) = NULL;
void     (*glsym_glXReleaseTexImage) (Display *a, GLXDrawable b, int c) = NULL;
int      (*glsym_glXGetVideoSync)    (unsigned int *a) = NULL;
int      (*glsym_glXWaitVideoSync)   (int a, int b, unsigned int *c) = NULL;
XID      (*glsym_glXCreatePixmap)    (Display *a, void *b, Pixmap c, const int *d) = NULL;
void     (*glsym_glXDestroyPixmap)   (Display *a, XID b) = NULL;
void     (*glsym_glXQueryDrawable)   (Display *a, XID b, int c, unsigned int *d) = NULL;
int      (*glsym_glxSwapIntervalSGI) (int a) = NULL;
void     (*glsym_glxSwapIntervalEXT) (Display *s, GLXDrawable b, int c) = NULL;
#endif

static void
_sym_init(void)
{
   static int done = 0;

   if (done) return;

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#define FINDSYM(dst, sym, typ) \
   if ((!dst) && (glsym_eglGetProcAddress)) dst = (typ)glsym_eglGetProcAddress(sym); \
   if (!dst) dst = (typ)dlsym(RTLD_DEFAULT, sym)

   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddress", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressEXT", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressARB", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressKHR", glsym_func_eng_fn);

   FINDSYM(glsym_eglBindTexImage, "eglBindTexImage", glsym_func_void);
   FINDSYM(glsym_eglBindTexImage, "eglBindTexImageEXT", glsym_func_void);
   FINDSYM(glsym_eglBindTexImage, "eglBindTexImageARB", glsym_func_void);
   FINDSYM(glsym_eglBindTexImage, "eglBindTexImageKHR", glsym_func_void);

   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImage", glsym_func_void);
   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImageEXT", glsym_func_void);
   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImageARB", glsym_func_void);
   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImageKHR", glsym_func_void);

   FINDSYM(glsym_eglCreateImage, "eglCreateImage", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageEXT", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageARB", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageKHR", glsym_func_void_ptr);

   FINDSYM(glsym_eglDestroyImage, "eglDestroyImage", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageEXT", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageARB", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageKHR", glsym_func_void);

   FINDSYM(glsym_glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES", glsym_func_void);
#else
#define FINDSYM(dst, sym, typ) \
   if ((!dst) && (glsym_glXGetProcAddress)) dst = (typ)glsym_glXGetProcAddress(sym); \
   if (!dst) dst = (typ)dlsym(RTLD_DEFAULT, sym)

   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddress", glsym_func_eng_fn);
   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddressEXT", glsym_func_eng_fn);
   FINDSYM(glsym_glXGetProcAddress, "glXGetProcAddressARB", glsym_func_eng_fn);

   FINDSYM(glsym_glXBindTexImage, "glXBindTexImage", glsym_func_void);
   FINDSYM(glsym_glXBindTexImage, "glXBindTexImageEXT", glsym_func_void);
   FINDSYM(glsym_glXBindTexImage, "glXBindTexImageARB", glsym_func_void);

   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImage", glsym_func_void);
   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImageEXT", glsym_func_void);
   FINDSYM(glsym_glXReleaseTexImage, "glXReleaseTexImageARB", glsym_func_void);

   FINDSYM(glsym_glXGetVideoSync, "glXGetVideoSyncSGI", glsym_func_int);

   FINDSYM(glsym_glXWaitVideoSync, "glXWaitVideoSyncSGI", glsym_func_int);

   FINDSYM(glsym_glXCreatePixmap, "glXCreatePixmap", glsym_func_xid);
   FINDSYM(glsym_glXCreatePixmap, "glXCreatePixmapEXT", glsym_func_xid);
   FINDSYM(glsym_glXCreatePixmap, "glXCreatePixmapARB", glsym_func_xid);

   FINDSYM(glsym_glXDestroyPixmap, "glXDestroyPixmap", glsym_func_void);
   FINDSYM(glsym_glXDestroyPixmap, "glXDestroyPixmapEXT", glsym_func_void);
   FINDSYM(glsym_glXDestroyPixmap, "glXDestroyPixmapARB", glsym_func_void);

   FINDSYM(glsym_glXQueryDrawable, "glXQueryDrawable", glsym_func_void);
   FINDSYM(glsym_glXQueryDrawable, "glXQueryDrawableEXT", glsym_func_void);
   FINDSYM(glsym_glXQueryDrawable, "glXQueryDrawableARB", glsym_func_void);

   FINDSYM(glsym_glxSwapIntervalSGI, "glXSwapIntervalMESA", glsym_func_int);
   FINDSYM(glsym_glxSwapIntervalSGI, "glXSwapIntervalSGI", glsym_func_int);

   FINDSYM(glsym_glxSwapIntervalEXT, "glXSwapIntervalEXT", glsym_func_void);
#endif
}

int _evas_engine_GL_X11_log_dom = -1;
/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* Function table for GL APIs */
static Evas_GL_API gl_funcs;

struct xrdb_user
{
   time_t last_stat;
   time_t last_mtime;
   XrmDatabase db;
};
static struct xrdb_user xrdb_user = {0, 0, NULL};

static Eina_Bool
xrdb_user_query(const char *name, const char *cls, char **type, XrmValue *val)
{
   time_t last = xrdb_user.last_stat, now = time(NULL);

   xrdb_user.last_stat = now;
   if (last != now) /* don't stat() more than once every second */
     {
	struct stat st;
	const char *home = getenv("HOME");
	char tmp[PATH_MAX];

	if (!home) goto failed;
	snprintf(tmp, sizeof(tmp), "%s/.Xdefaults", home);
	if (stat(tmp, &st) != 0) goto failed;
	if (xrdb_user.last_mtime != st.st_mtime)
	  {
	     if (xrdb_user.db) XrmDestroyDatabase(xrdb_user.db);
	     xrdb_user.db = XrmGetFileDatabase(tmp);
	     if (!xrdb_user.db) goto failed;
	     xrdb_user.last_mtime = st.st_mtime;
	  }
     }

   if (!xrdb_user.db) return EINA_FALSE;
   return XrmGetResource(xrdb_user.db, name, cls, type, val);

 failed:
   if (xrdb_user.db)
     {
	XrmDestroyDatabase(xrdb_user.db);
	xrdb_user.db = NULL;
     }
   xrdb_user.last_mtime = 0;
   return EINA_FALSE;
}

static void *
eng_info(Evas *e)
{
   Evas_Engine_Info_GL_X11 *info;

   info = calloc(1, sizeof(Evas_Engine_Info_GL_X11));
   info->magic.magic = rand();
   info->func.best_visual_get = eng_best_visual_get;
   info->func.best_colormap_get = eng_best_colormap_get;
   info->func.best_depth_get = eng_best_depth_get;
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;
   return info;
   e = NULL;
}

static void
eng_info_free(Evas *e __UNUSED__, void *info)
{
   Evas_Engine_Info_GL_X11 *in;
// dont free! why bother? its not worth it
//   eina_log_domain_unregister(_evas_engine_GL_X11_log_dom);
   in = (Evas_Engine_Info_GL_X11 *)info;
   free(in);
}

static int
_re_wincheck(Render_Engine *re)
{
   if (re->win->surf) return 1;
   eng_window_resurf(re->win);
   if (!re->win->surf)
     {
        ERR("GL engine can't re-create window surface!");
     }
   return 0;
}

static void
_re_winfree(Render_Engine *re)
{
   if (!re->win->surf) return;
   eng_window_unsurf(re->win);
}

static int
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_GL_X11 *info;

   info = (Evas_Engine_Info_GL_X11 *)in;
   if (!e->engine.data.output)
     {
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#else
        int eb, evb;

	if (!glXQueryExtension(info->info.display, &eb, &evb)) return 0;
#endif
	re = calloc(1, sizeof(Render_Engine));
	if (!re) return 0;
        re->info = info;
        re->evas = e;
	e->engine.data.output = re;
        re->w = e->output.w;
        re->h = e->output.h;
	re->win = eng_window_new(re->info->info.display,
				 re->info->info.drawable,
                                 re->info->info.screen,
				 re->info->info.visual,
				 re->info->info.colormap,
				 re->info->info.depth,
                                 re->w,
                                 re->h,
                                 re->info->indirect,
                                 re->info->info.destination_alpha,
                                 re->info->info.rotation);
	if (!re->win)
	  {
	     free(re);
	     e->engine.data.output = NULL;
	     return 0;
	  }
        gl_wins++;

          {
             int status;
             char *type = NULL;
             XrmValue val;

             re->xr.dpi = 75000; // dpy * 1000

	     status = xrdb_user_query("Xft.dpi", "Xft.Dpi", &type, &val);
	     if ((!status) || (!type))
	       {
		  if (!re->xrdb) re->xrdb = XrmGetDatabase(re->info->info.display);
		  if (re->xrdb)
		    status = XrmGetResource(re->xrdb,
					    "Xft.dpi", "Xft.Dpi", &type, &val);
	       }

             if ((status) && (type))
               {
                  if (!strcmp(type, "String"))
                    {
                       const char *str, *dp;

                       str = val.addr;
                       dp = strchr(str, '.');
                       if (!dp) dp = strchr(str, ',');

                       if (dp)
                         {
                            int subdpi, len, i;
                            char *buf;

                            buf = alloca(dp - str + 1);
                            strncpy(buf, str, dp - str);
                            buf[dp - str] = 0;
                            len = strlen(dp + 1);
                            subdpi = atoi(dp + 1);

                            if (len < 3)
                              {
                                 for (i = len; i < 3; i++) subdpi *= 10;
                              }
                            else if (len > 3)
                              {
                                 for (i = len; i > 3; i--) subdpi /= 10;
                              }
                            re->xr.dpi = atoi(buf) * 1000;
                         }
                       else
                         re->xr.dpi = atoi(str) * 1000;
                       evas_common_font_dpi_set(re->xr.dpi / 1000);
                    }
               }
          }

        if (!initted)
          {
             evas_common_cpu_init();

             evas_common_blend_init();
             evas_common_image_init();
             evas_common_convert_init();
             evas_common_scale_init();
             evas_common_rectangle_init();
             evas_common_polygon_init();
             evas_common_line_init();
             evas_common_font_init();
             evas_common_draw_init();
             evas_common_tilebuf_init();
             initted = 1;
          }
     }
   else
     {
	re = e->engine.data.output;
        if (_re_wincheck(re))
          {
             if ((re->info->info.display != re->win->disp) ||
                 (re->info->info.drawable != re->win->win) ||
                 (re->info->info.screen != re->win->screen) ||
                 (re->info->info.visual != re->win->visual) ||
                 (re->info->info.colormap != re->win->colormap) ||
                 (re->info->info.depth != re->win->depth) ||
                 (re->info->info.destination_alpha != re->win->alpha) ||
                 (re->info->info.rotation != re->win->rot))
               {
                  int inc = 0;

                  if (re->win)
                    {
                       re->win->gl_context->references++;
                       eng_window_free(re->win);
                       inc = 1;
                       gl_wins--;
                    }
                  re->w = e->output.w;
                  re->h = e->output.h;
                  re->win = eng_window_new(re->info->info.display,
                                           re->info->info.drawable,
                                           re->info->info.screen,
                                           re->info->info.visual,
                                           re->info->info.colormap,
                                           re->info->info.depth,
                                           re->w,
                                           re->h,
                                           re->info->indirect,
                                           re->info->info.destination_alpha,
                                           re->info->info.rotation);
                  eng_window_use(re->win);
                  if (re->win) gl_wins++;
                  if ((re->win) && (inc))
                     re->win->gl_context->references--;
               }
             else if ((re->win->w != e->output.w) ||
                      (re->win->h != e->output.h))
               {
                  re->w = e->output.w;
                  re->h = e->output.h;
                  re->win->w = e->output.w;
                  re->win->h = e->output.h;
                  eng_window_use(re->win);
                  evas_gl_common_context_resize(re->win->gl_context, re->win->w, re->win->h, re->win->rot);
               }
          }
     }
   if (!re->win)
     {
        free(re);
        return 0;
     }

   if (!e->engine.data.output)
     {
        if (re->win)
          {
             eng_window_free(re->win);
             gl_wins--;
          }
        free(re);
        return 0;
     }
   if (!e->engine.data.context)
     e->engine.data.context =
     e->engine.func->context_new(e->engine.data.output);
   eng_window_use(re->win);

   re->vsync = 0;
/* we don't need this actually as evas_render does it  
   if (re->win->alpha)
     {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
     }
 */
   _sym_init();

   return 1;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   if (re)
     {
// NOTE: XrmGetDatabase() result is shared per connection, do not free it.
//   if (re->xrdb) XrmDestroyDatabase(re->xrdb);

        if (re->win)
          {
             eng_window_free(re->win);
             gl_wins--;
          }
        free(re);
     }
   if ((initted == 1) && (gl_wins == 0))
     {
        evas_common_image_shutdown();
        evas_common_font_shutdown();
        initted = 0;
     }
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->w = w;
   re->win->h = h;
   eng_window_use(re->win);
   evas_gl_common_context_resize(re->win->gl_context, w, h, re->win->rot);
}

static void
eng_output_tile_size_set(void *data __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_gl_common_context_resize(re->win->gl_context, re->win->w, re->win->h, re->win->rot);
   /* smple bounding box */
   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, re->win->w, re->win->h);
   if ((w <= 0) || (h <= 0)) return;
   if (!re->win->draw.redraw)
     {
#if 0
	re->win->draw.x1 = x;
	re->win->draw.y1 = y;
	re->win->draw.x2 = x + w - 1;
	re->win->draw.y2 = y + h - 1;
#else
	re->win->draw.x1 = 0;
	re->win->draw.y1 = 0;
	re->win->draw.x2 = re->win->w - 1;
	re->win->draw.y2 = re->win->h - 1;
#endif
     }
   else
     {
	if (x < re->win->draw.x1) re->win->draw.x1 = x;
	if (y < re->win->draw.y1) re->win->draw.y1 = y;
	if ((x + w - 1) > re->win->draw.x2) re->win->draw.x2 = x + w - 1;
	if ((y + h - 1) > re->win->draw.y2) re->win->draw.y2 = y + h - 1;
     }
   re->win->draw.redraw = 1;
}

static void
eng_output_redraws_rect_del(void *data __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->draw.redraw = 0;
//   INF("GL: finish update cycle!");
}

/* vsync games - not for now though */
#define VSYNC_TO_SCREEN 1

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   /* get the upate rect surface - return engine data as dummy */
   if (!re->win->draw.redraw) return NULL;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   // dont need to for egl - eng_window_use() can check for other ctxt's
#else
   eng_window_use(NULL);
#endif
   eng_window_use(re->win);
   if (!_re_wincheck(re)) return NULL;
   evas_gl_common_context_flush(re->win->gl_context);
   evas_gl_common_context_newframe(re->win->gl_context);
   if (x) *x = re->win->draw.x1;
   if (y) *y = re->win->draw.y1;
   if (w) *w = re->win->draw.x2 - re->win->draw.x1 + 1;
   if (h) *h = re->win->draw.y2 - re->win->draw.y1 + 1;
   if (cx) *cx = re->win->draw.x1;
   if (cy) *cy = re->win->draw.y1;
   if (cw) *cw = re->win->draw.x2 - re->win->draw.x1 + 1;
   if (ch) *ch = re->win->draw.y2 - re->win->draw.y1 + 1;

/* we don't need this actually as evas_render does it  
   if (re->win->alpha)
     {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
     }
 */
   return re->win->gl_context->def_surface;
}

//#define FRAMECOUNT 1

#ifdef FRAMECOUNT
double
get_time(void)
{
   struct timeval      timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}
#endif

static int safe_native = -1;

static void
eng_output_redraws_next_update_push(void *data, void *surface __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   Render_Engine *re;
#ifdef FRAMECOUNT
   static double pt = 0.0;
   double ta, tb;
#endif

   re = (Render_Engine *)data;
   /* put back update surface.. in this case just unflag redraw */
   if (!_re_wincheck(re)) return;
   re->win->draw.redraw = 0;
   re->win->draw.drew = 1;
   evas_gl_common_context_flush(re->win->gl_context);
   if (safe_native == -1)
     {
        const char *s = getenv("EVAS_GL_SAFE_NATIVE");
        safe_native = 0;
        if (s) safe_native = atoi(s);
        else
          {
             s = (const char *)glGetString(GL_RENDERER);
             if (s)
               {
                  if (strstr(s, "PowerVR SGX 540") ||
		      strstr(s, "Mali-400 MP"))
                     safe_native = 1;
               }
          }
     }
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   // this is needed to make sure all previous rendering is flushed to
   // buffers/surfaces
#ifdef FRAMECOUNT
   double t0 = get_time();
   ta = t0 - pt;
   pt = t0;
#endif
   // previous rendering should be done and swapped
   if (!safe_native) eglWaitNative(EGL_CORE_NATIVE_ENGINE);
#ifdef FRAMECOUNT
   double t1 = get_time();
   tb = t1 - t0;
   printf("... %1.5f -> %1.5f | ", ta, tb);
#endif
//   if (eglGetError() != EGL_SUCCESS)
//     {
//        printf("Error:  eglWaitNative(EGL_CORE_NATIVE_ENGINE) fail.\n");
//     }
#else
   // previous rendering should be done and swapped
   if (!safe_native) glXWaitX();
#endif
//x//   printf("frame -> push\n");
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!_re_wincheck(re)) return;
   if (!re->win->draw.drew) return;
//x//   printf("frame -> flush\n");
   re->win->draw.drew = 0;
   eng_window_use(re->win);

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#ifdef FRAMECOUNT
   double t0 = get_time();
#endif
   if (!re->vsync)
     {
        if (re->info->vsync) eglSwapInterval(re->win->egl_disp, 1);
        else eglSwapInterval(re->win->egl_disp, 0);
        re->vsync = 1;
     }
   if (re->info->callback.pre_swap)
     {
        re->info->callback.pre_swap(re->info->callback.data, re->evas);
     }
   eglSwapBuffers(re->win->egl_disp, re->win->egl_surface[0]);
   if (!safe_native) eglWaitGL();
   if (re->info->callback.post_swap)
     {
        re->info->callback.post_swap(re->info->callback.data, re->evas);
     }
#ifdef FRAMECOUNT
   double t1 = get_time();
   printf("%1.5f\n", t1 - t0);
#endif
//   if (eglGetError() != EGL_SUCCESS)
//     {
//        printf("Error:  eglSwapBuffers() fail.\n");
//     }
#else
#ifdef VSYNC_TO_SCREEN
   if ((re->info->vsync)/* || (1)*/)
     {
        if (glsym_glxSwapIntervalEXT)
          {
             if (!re->vsync)
               {
                  if (re->info->vsync) glsym_glxSwapIntervalEXT(re->win->disp, re->win->win, 1);
                  else glsym_glxSwapIntervalEXT(re->win->disp, re->win->win, 0);
                  re->vsync = 1;
               }
          }
        if (glsym_glxSwapIntervalSGI)
          {
             if (!re->vsync)
               {
                  if (re->info->vsync) glsym_glxSwapIntervalSGI(1);
                  else glsym_glxSwapIntervalSGI(0);
                  re->vsync = 1;
               }
          }
        else
          {
             if ((glsym_glXGetVideoSync) && (glsym_glXWaitVideoSync))
               {
                  unsigned int rc;

                  glsym_glXGetVideoSync(&rc);
                  glsym_glXWaitVideoSync(1, 0, &rc);
               }
          }
     }
# endif
   if (re->info->callback.pre_swap)
     {
        re->info->callback.pre_swap(re->info->callback.data, re->evas);
     }
/*
   if ((1)
//       (re->win->draw.x1 == 0) &&
//       (re->win->draw.y1 == 0) &&
//       (re->win->draw.x2 == (re->win->w - 1)) &&
//       (re->win->draw.y2 == (re->win->h - 1))
       )
 */
     {
        glXSwapBuffers(re->win->disp, re->win->win);
        if (!safe_native) glXWaitGL();
     }
/*
   else
     {
// FIXME: this doesn't work.. why oh why?
        int sx, sy, sw, sh;

        // fimxe - reset when done
//        glEnable(GL_SCISSOR_TEST);
        glDrawBuffer(GL_FRONT);

        sx = re->win->draw.x1;
        sy = re->win->draw.y1;
        sw = (re->win->draw.x2 - re->win->draw.x1) + 1;
        sh = (re->win->draw.y2 - re->win->draw.y1) + 1;
        sy = re->win->h - sy - sh;

//        glScissor(sx, sy, sw, sh);
        glRasterPos2i(sx, re->win->h - sy);
        glCopyPixels(sx, sy, sw, sh, GL_COLOR);
        glRasterPos2i(0, 0);

//        glDisable(GL_SCISSOR_TEST);
        glDrawBuffer(GL_BACK);
        glFlush();
     }
 */
   if (re->info->callback.post_swap)
     {
        re->info->callback.post_swap(re->info->callback.data, re->evas);
     }
#endif
}

static void
eng_output_idle_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
}

static void
eng_output_dump(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
   evas_gl_common_image_all_unload(re->win->gl_context);
   _re_winfree(re);
}

static void
eng_context_cutout_add(void *data __UNUSED__, void *context, int x, int y, int w, int h)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
//   re->win->gl_context->dc = context;
   evas_common_draw_context_add_cutout(context, x, y, w, h);
}

static void
eng_context_cutout_clear(void *data __UNUSED__, void *context)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
//   re->win->gl_context->dc = context;
   evas_common_draw_context_clear_cutouts(context);
}

static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_rect_draw(re->win->gl_context, x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_line_draw(re->win->gl_context, x1, y1, x2, y2);
}

static void *
eng_polygon_point_add(void *data, void *context __UNUSED__, void *polygon, int x, int y)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_poly_point_add(polygon, x, y);
}

static void *
eng_polygon_points_clear(void *data, void *context __UNUSED__, void *polygon)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_poly_points_clear(polygon);
}

static void
eng_polygon_draw(void *data, void *context, void *surface __UNUSED__, void *polygon, int x, int y)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_poly_draw(re->win->gl_context, polygon, x, y);
}

static int
eng_image_alpha_get(void *data __UNUSED__, void *image)
{
//   Render_Engine *re;
   Evas_GL_Image *im;

//   re = (Render_Engine *)data;
   if (!image) return 1;
   im = image;
   return im->alpha;
}

static int
eng_image_colorspace_get(void *data __UNUSED__, void *image)
{
//   Render_Engine *re;
   Evas_GL_Image *im;

//   re = (Render_Engine *)data;
   if (!image) return EVAS_COLORSPACE_ARGB8888;
   im = image;
   return im->cs.space;
}

static void
eng_image_mask_create(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *im;

   if (!image) return;
   im = image;
   if (!im->im->image.data)
      evas_cache_image_load_data(&im->im->cache_entry);
   if (!im->tex)
      im->tex = evas_gl_common_texture_new(im->gc, im->im);
}


static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   im = image;
   if (im->alpha == has_alpha) return image;
   if (im->native.data)
     {
        im->alpha = has_alpha;
        return image;
     }
   eng_window_use(re->win);
   if ((im->tex) && (im->tex->pt->dyn.img))
     {
        im->alpha = has_alpha;
        im->tex->alpha = im->alpha;
        return image;
     }
   /* FIXME: can move to gl_common */
   if (im->cs.space != EVAS_COLORSPACE_ARGB8888) return im;
   if ((has_alpha) && (im->im->cache_entry.flags.alpha)) return image;
   else if ((!has_alpha) && (!im->im->cache_entry.flags.alpha)) return image;
   if (im->references > 1)
     {
        Evas_GL_Image *im_new;

        im_new = evas_gl_common_image_new_from_copied_data
           (im->gc, im->im->cache_entry.w, im->im->cache_entry.h,
               im->im->image.data,
               eng_image_alpha_get(data, image),
               eng_image_colorspace_get(data, image));
        if (!im_new) return im;
        evas_gl_common_image_free(im);
        im = im_new;
     }
   else
     evas_gl_common_image_dirty(im, 0, 0, 0, 0);
   return evas_gl_common_image_alpha_set(im, has_alpha ? 1 : 0);
//   im->im->cache_entry.flags.alpha = has_alpha ? 1 : 0;
//   return image;
}

static void *
eng_image_border_set(void *data __UNUSED__, void *image, int l __UNUSED__, int r __UNUSED__, int t __UNUSED__, int b __UNUSED__)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
   return image;
}

static void
eng_image_border_get(void *data __UNUSED__, void *image __UNUSED__, int *l __UNUSED__, int *r __UNUSED__, int *t __UNUSED__, int *b __UNUSED__)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
}

static char *
eng_image_comment_get(void *data __UNUSED__, void *image, char *key __UNUSED__)
{
//   Render_Engine *re;
   Evas_GL_Image *im;

//   re = (Render_Engine *)data;
   if (!image) return NULL;
   im = image;
   if (!im->im) return NULL;
   return im->im->info.comment;
}

static char *
eng_image_format_get(void *data __UNUSED__, void *image)
{
//   Render_Engine *re;
   Evas_GL_Image *im;

//   re = (Render_Engine *)data;
   im = image;
   return NULL;
}

static void
eng_image_colorspace_set(void *data, void *image, int cspace)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image) return;
   im = image;
   if (im->native.data) return;
   /* FIXME: can move to gl_common */
   if (im->cs.space == cspace) return;
   eng_window_use(re->win);
   evas_cache_image_colorspace(&im->im->cache_entry, cspace);
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (im->cs.data)
	  {
	     if (!im->cs.no_free) free(im->cs.data);
	     im->cs.data = NULL;
	     im->cs.no_free = 0;
	  }
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
        if (im->tex) evas_gl_common_texture_free(im->tex);
        im->tex = NULL;
	if (im->cs.data)
	  {
	     if (!im->cs.no_free) free(im->cs.data);
	  }
        if (im->im->cache_entry.h > 0)
          im->cs.data =
          calloc(1, im->im->cache_entry.h * sizeof(unsigned char *) * 2);
        else
          im->cs.data = NULL;
	im->cs.no_free = 0;
	break;
      default:
	abort();
	break;
     }
   im->cs.space = cspace;
}

/////////////////////////////////////////////////////////////////////////
//
//
typedef struct _Native Native;

struct _Native
{
   Evas_Native_Surface ns;
   Pixmap     pixmap;
   Visual    *visual;

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   void      *egl_surface;
#else
   void  *fbc;
   XID    glx_pixmap;
#endif
};

// FIXME: this is enabled so updates happen - but its SLOOOOOOOOOOOOOOOW
// (i am sure this is the reason)  not to mention seemingly superfluous. but
// i need to enable it for it to work on fglrx at least. havent tried nvidia.
//
// why is this the case? does anyone know? has anyone tried it on other gfx
// drivers?
//
//#define GLX_TEX_PIXMAP_RECREATE 1

static void
_native_bind_cb(void *data, void *image)
{
   Evas_GL_Image *im = image;
   Native *n = im->native.data;

  if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
    {
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
      if (n->egl_surface)
        {
          if (glsym_glEGLImageTargetTexture2DOES)
            {
              glsym_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, n->egl_surface);
              if (eglGetError() != EGL_SUCCESS)
                ERR("glEGLImageTargetTexture2DOES() failed.");
            }
          else
            ERR("Try glEGLImageTargetTexture2DOES on EGL with no support");
        }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
      Render_Engine *re = data;

      if (glsym_glXBindTexImage)
        {
          glsym_glXBindTexImage(re->win->disp, n->glx_pixmap,
                                GLX_FRONT_LEFT_EXT, NULL);
          GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        }
      else
        ERR("Try glXBindTexImage on GLX with no support");
# endif
#endif
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
    {
      glBindTexture(GL_TEXTURE_2D, n->ns.data.opengl.texture_id);
      GLERR(__FUNCTION__, __FILE__, __LINE__, "");
    }
   return;
   data = NULL;
}

static void
_native_unbind_cb(void *data, void *image)
{
  Evas_GL_Image *im = image;
  Native *n = im->native.data;

  if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
    {
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
      // nothing
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
      Render_Engine *re = data;

      if (glsym_glXReleaseTexImage)
        {
          glsym_glXReleaseTexImage(re->win->disp, n->glx_pixmap,
                                   GLX_FRONT_LEFT_EXT);
          GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        }
      else
        ERR("Try glXReleaseTexImage on GLX with no support");
# endif
#endif
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
    {
      glBindTexture(GL_TEXTURE_2D, 0);
      GLERR(__FUNCTION__, __FILE__, __LINE__, "");
    }
   return;
   data = NULL;
}

static void
_native_free_cb(void *data, void *image)
{
  Render_Engine *re = data;
  Evas_GL_Image *im = image;
  Native *n = im->native.data;
  uint32_t pmid, texid;

  if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
    {
      pmid = n->pixmap;
      eina_hash_del(re->win->gl_context->shared->native_pm_hash, &pmid, im);
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
      if (n->egl_surface)
        {
          if (glsym_eglDestroyImage)
            {
              glsym_eglDestroyImage(re->win->egl_disp,
                                    n->egl_surface);
              if (eglGetError() != EGL_SUCCESS)
                ERR("eglDestroyImage() failed.");
            }
          else
            ERR("Try eglDestroyImage on EGL with no support");
        }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
      if (n->glx_pixmap)
        {
          if (im->native.loose)
            {
              if (glsym_glXReleaseTexImage)
                {
                  glsym_glXReleaseTexImage(re->win->disp, n->glx_pixmap,
                                           GLX_FRONT_LEFT_EXT);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                }
              else
                ERR("Try glXReleaseTexImage on GLX with no support");
            }
          if (glsym_glXDestroyPixmap)
            {
              glsym_glXDestroyPixmap(re->win->disp, n->glx_pixmap);
              GLERR(__FUNCTION__, __FILE__, __LINE__, "");
            }
          else
            ERR("Try glXDestroyPixmap on GLX with no support");
          n->glx_pixmap = 0;
        }
# endif
#endif
    }
  else if (n->ns.type == EVAS_NATIVE_SURFACE_OPENGL)
    {
      texid = n->ns.data.opengl.texture_id;
      eina_hash_del(re->win->gl_context->shared->native_tex_hash, &texid, im);
    }
  im->native.data        = NULL;
  im->native.func.data   = NULL;
  im->native.func.bind   = NULL;
  im->native.func.unbind = NULL;
  im->native.func.free   = NULL;
  free(n);
}

static void *
eng_image_native_set(void *data, void *image, void *native)
{
  Render_Engine *re = (Render_Engine *)data;
  Evas_Native_Surface *ns = native;
  Evas_GL_Image *im = image, *im2 = NULL;
  Visual *vis = NULL;
  Pixmap pm = 0;
  Native *n = NULL;
  uint32_t pmid, texid;
  unsigned int tex = 0;
  unsigned int fbo = 0;

  if (!im)
    {
       if ((!ns) && (ns->type == EVAS_NATIVE_SURFACE_OPENGL))
         {
            im = evas_gl_common_image_new_from_data(re->win->gl_context,
                                                    ns->data.opengl.w,
                                                    ns->data.opengl.h,
                                                    NULL, 1,
                                                    EVAS_COLORSPACE_ARGB8888);
         }
       else
           return NULL;
    }

  if (ns)
    {
      if (ns->type == EVAS_NATIVE_SURFACE_X11)
        {
          vis = ns->data.x11.visual;
          pm = ns->data.x11.pixmap;
          if (im->native.data)
            {
              Evas_Native_Surface *ens = im->native.data;
              if ((ens->data.x11.visual == vis) &&
                  (ens->data.x11.pixmap == pm))
                return im;
            }
        }
      else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
        {
          tex = ns->data.opengl.texture_id;
          fbo = ns->data.opengl.framebuffer_id;
          if (im->native.data)
            {
              Evas_Native_Surface *ens = im->native.data;
              if ((ens->data.opengl.texture_id == tex) &&
                  (ens->data.opengl.framebuffer_id == fbo))
                return im;
            }
        }
    }
  if ((!ns) && (!im->native.data)) return im;

  eng_window_use(re->win);

  if (im->native.data)
    {
      if (im->native.func.free)
        im->native.func.free(im->native.func.data, im);
      evas_gl_common_image_native_disable(im);
    }

  if (!ns) return im;

  if (ns->type == EVAS_NATIVE_SURFACE_X11)
    {
      pmid = pm;
      im2 = eina_hash_find(re->win->gl_context->shared->native_pm_hash, &pmid);
      if (im2 == im) return im;
      if (im2)
        {
           n = im2->native.data;
           if (n)
             {
                evas_gl_common_image_ref(im2);
                evas_gl_common_image_free(im);
                return im2;
             }
        }
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
       texid = tex;
       im2 = eina_hash_find(re->win->gl_context->shared->native_tex_hash, &texid);
       if (im2 == im) return im;
       if (im2)
         {
            n = im2->native.data;
            if (n)
              {
                 evas_gl_common_image_ref(im2);
                 evas_gl_common_image_free(im);
                 return im2;
              }
         }

    }
  im2 = evas_gl_common_image_new_from_data(re->win->gl_context,
                                           im->w, im->h, NULL, im->alpha,
                                           EVAS_COLORSPACE_ARGB8888);
  evas_gl_common_image_free(im);
  im = im2;
  if (ns->type == EVAS_NATIVE_SURFACE_X11)
    {
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
      if (native)
        {
          n = calloc(1, sizeof(Native));
          if (n)
            {
              EGLConfig egl_config;
              int config_attrs[20];
              int num_config, i = 0;

              eina_hash_add(re->win->gl_context->shared->native_pm_hash, &pmid, im);

              config_attrs[i++] = EGL_RED_SIZE;
              config_attrs[i++] = 8;
              config_attrs[i++] = EGL_GREEN_SIZE;
              config_attrs[i++] = 8;
              config_attrs[i++] = EGL_BLUE_SIZE;
              config_attrs[i++] = 8;
              config_attrs[i++] = EGL_ALPHA_SIZE;
              config_attrs[i++] = 8;
              config_attrs[i++] = EGL_DEPTH_SIZE;
              config_attrs[i++] = 0;
              config_attrs[i++] = EGL_STENCIL_SIZE;
              config_attrs[i++] = 0;
              config_attrs[i++] = EGL_RENDERABLE_TYPE;
              config_attrs[i++] = EGL_OPENGL_ES2_BIT;
              config_attrs[i++] = EGL_SURFACE_TYPE;
              config_attrs[i++] = EGL_PIXMAP_BIT;
              config_attrs[i++] = EGL_NONE;

              if (!eglChooseConfig(re->win->egl_disp, config_attrs,
                                   &egl_config, 1, &num_config))
                ERR("eglChooseConfig() failed for pixmap 0x%x, num_config = %i", (unsigned int)pm, num_config);
              memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
              n->pixmap = pm;
              n->visual = vis;
              if (glsym_eglCreateImage)
                n->egl_surface = glsym_eglCreateImage(re->win->egl_disp,
                                                      EGL_NO_CONTEXT,
                                                      EGL_NATIVE_PIXMAP_KHR,
                                                      (void *)pm,
                                                      NULL);
              else
                ERR("Try eglCreateImage on EGL with no support");
              if (!n->egl_surface)
                ERR("eglCreatePixmapSurface() for 0x%x failed", (unsigned int)pm);
              im->native.yinvert     = 1;
              im->native.loose       = 0;
              im->native.data        = n;
              im->native.func.data   = re;
              im->native.func.bind   = _native_bind_cb;
              im->native.func.unbind = _native_unbind_cb;
              im->native.func.free   = _native_free_cb;
              im->native.target      = GL_TEXTURE_2D;
              im->native.mipmap      = 0;
              evas_gl_common_image_native_enable(im);
            }
        }
#else
# ifdef GLX_BIND_TO_TEXTURE_TARGETS_EXT
      if (native)
        {
          int dummy;
          unsigned int w, h, depth = 32, border;
          Window wdummy;

          // fixme: round trip :(
          XGetGeometry(re->win->disp, pm, &wdummy, &dummy, &dummy,
                       &w, &h, &border, &depth);
          n = calloc(1, sizeof(Native));
          if (n)
            {
              int pixmap_att[20];
              unsigned int target = 0;
              unsigned int i = 0;

              eina_hash_add(re->win->gl_context->shared->native_pm_hash, &pmid, im);
              if ((re->win->depth_cfg[depth].tex_target &
                   GLX_TEXTURE_2D_BIT_EXT)
                  //                 && (1) // we assume npo2 for now
                  // size is pow2 || mnpo2 supported
                 )
                target = GLX_TEXTURE_2D_EXT;
              else if ((re->win->depth_cfg[depth].tex_target &
                        GLX_TEXTURE_RECTANGLE_BIT_EXT))
                {
                  ERR("rect!!! (not handled)");
                  target = GLX_TEXTURE_RECTANGLE_EXT;
                }
              if (!target)
                {
                  ERR("broken text-from-pixmap");
                  if (!(re->win->depth_cfg[depth].tex_target &
                        GLX_TEXTURE_2D_BIT_EXT))
                    target = GLX_TEXTURE_RECTANGLE_EXT;
                  else if (!(re->win->depth_cfg[depth].tex_target &
                             GLX_TEXTURE_RECTANGLE_BIT_EXT))
                    target = GLX_TEXTURE_2D_EXT;
                }


              pixmap_att[i++] = GLX_TEXTURE_FORMAT_EXT;
              pixmap_att[i++] = re->win->depth_cfg[depth].tex_format;
              pixmap_att[i++] = GLX_MIPMAP_TEXTURE_EXT;
              pixmap_att[i++] = re->win->depth_cfg[depth].mipmap;

              if (target)
                {
                  pixmap_att[i++] = GLX_TEXTURE_TARGET_EXT;
                  pixmap_att[i++] = target;
                }

              pixmap_att[i++] = 0;

              memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));
              n->pixmap = pm;
              n->visual = vis;
              n->fbc = re->win->depth_cfg[depth].fbc;
              if (glsym_glXCreatePixmap)
                n->glx_pixmap = glsym_glXCreatePixmap(re->win->disp,
                                                      n->fbc,
                                                      n->pixmap,
                                                      pixmap_att);
              else
                ERR("Try glXCreatePixmap on GLX with no support");
              if (n->glx_pixmap)
                {
//                  printf("%p: new native texture for %x | %4i x %4i @ %2i = %p\n",
//                         n, pm, w, h, depth, n->glx_pixmap);
                  if (!target)
                    {
                      ERR("no target :(");
                      if (glsym_glXQueryDrawable)
                        glsym_glXQueryDrawable(re->win->disp,
                                               n->pixmap,
                                               GLX_TEXTURE_TARGET_EXT,
                                               &target);
                    }
                  if (target == GLX_TEXTURE_2D_EXT)
                    {
                      im->native.target = GL_TEXTURE_2D;
                      im->native.mipmap = re->win->depth_cfg[depth].mipmap;
                    }
#  ifdef GL_TEXTURE_RECTANGLE_ARB
                  else if (target == GLX_TEXTURE_RECTANGLE_EXT)
                    {
                      im->native.target = GL_TEXTURE_RECTANGLE_ARB;
                      im->native.mipmap = 0;
                    }
#  endif
                  else
                    {
                      im->native.target = GL_TEXTURE_2D;
                      im->native.mipmap = 0;
                      ERR("still unknown target");
                    }
                }
              else
                ERR("GLX Pixmap create fail");
              im->native.yinvert     = re->win->depth_cfg[depth].yinvert;
              im->native.loose       = re->win->detected.loose_binding;
              im->native.data        = n;
              im->native.func.data   = re;
              im->native.func.bind   = _native_bind_cb;
              im->native.func.unbind = _native_unbind_cb;
              im->native.func.free   = _native_free_cb;

              evas_gl_common_image_native_enable(im);
            }
        }
# endif
#endif
    }
  else if (ns->type == EVAS_NATIVE_SURFACE_OPENGL)
    {
      if (native)
        {
          n = calloc(1, sizeof(Native));
          if (n)
            {
              memcpy(&(n->ns), ns, sizeof(Evas_Native_Surface));

              eina_hash_add(re->win->gl_context->shared->native_tex_hash, &texid, im);

              n->pixmap = 0;
              n->visual = 0;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
              n->egl_surface = 0;
#else
              n->fbc = 0;
              n->glx_pixmap = 0;
#endif

              im->native.yinvert     = 0;
              im->native.loose       = 0;
              im->native.data        = n;
              im->native.func.data   = re;
              im->native.func.bind   = _native_bind_cb;
              im->native.func.unbind = _native_unbind_cb;
              im->native.func.free   = _native_free_cb;
              im->native.target      = GL_TEXTURE_2D;
              im->native.mipmap      = 0;

              // FIXME: need to implement mapping sub texture regions
              // x, y, w, h for possible texture atlasing

              evas_gl_common_image_native_enable(im);
            }
        }

    }
   return im;
}

static void *
eng_image_native_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *im = image;
   Native *n;
   if (!im) return NULL;
   n = im->native.data;
   if (!n) return NULL;
   return &(n->ns);
}

#if 0 // filtering disabled
static void
eng_image_draw_filtered(void *data, void *context, void *surface,
                        void *image, Evas_Filter_Info *filter)
{
   Render_Engine *re = data;

   if (!image) return;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;

   evas_gl_common_filter_draw(re->win->gl_context, image, filter);
}

static Filtered_Image *
eng_image_filtered_get(void *im, uint8_t *key, size_t keylen)
{
   return evas_gl_common_image_filtered_get(im, key, keylen);
}

static Filtered_Image *
eng_image_filtered_save(void *im, void *fim, uint8_t *key, size_t keylen)
{
   return evas_gl_common_image_filtered_save(im, fim, key, keylen);
}

static void
eng_image_filtered_free(void *im, Filtered_Image *fim)
{
   evas_gl_common_image_filtered_free(im, fim);
}
#endif


//
//
/////////////////////////////////////////////////////////////////////////

static void *
eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   *error = EVAS_LOAD_ERROR_NONE;
   eng_window_use(re->win);
   return evas_gl_common_image_load(re->win->gl_context, file, key, lo, error);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_data(re->win->gl_context, w, h, image_data, alpha, cspace);
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_copied_data(re->win->gl_context, w, h, image_data, alpha, cspace);
}

static void
eng_image_free(void *data, void *image)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return;
   eng_window_use(re->win);
   evas_gl_common_image_free(image);
}

static void
eng_image_size_get(void *data __UNUSED__, void *image, int *w, int *h)
{
   if (!image)
     {
	*w = 0;
	*h = 0;
	return;
     }
   if (w) *w = ((Evas_GL_Image *)image)->w;
   if (h) *h = ((Evas_GL_Image *)image)->h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;
   Evas_GL_Image *im = image;
   Evas_GL_Image *im_old;

   re = (Render_Engine *)data;
   if (!im) return NULL;
   if (im->native.data)
     {
        im->w = w;
        im->h = h;
        return image;
     }
   eng_window_use(re->win);
   if ((im->tex) && (im->tex->pt->dyn.img))
     {
        evas_gl_common_texture_free(im->tex);
        im->tex = NULL;
        im->w = w;
        im->h = h;
        im->tex = evas_gl_common_texture_dynamic_new(im->gc, im);
        return image;
     }
   im_old = image;
   if ((eng_image_colorspace_get(data, image) == EVAS_COLORSPACE_YCBCR422P601_PL) ||
       (eng_image_colorspace_get(data, image) == EVAS_COLORSPACE_YCBCR422P709_PL))
     w &= ~0x1;
   if ((im_old) &&
       ((int)im_old->im->cache_entry.w == w) &&
       ((int)im_old->im->cache_entry.h == h))
     return image;
   if (im_old)
     {
	im = evas_gl_common_image_new(re->win->gl_context, w, h,
				      eng_image_alpha_get(data, image),
				      eng_image_colorspace_get(data, image));
/*
	evas_common_load_image_data_from_file(im_old->im);
	if (im_old->im->image->data)
	  {
	     evas_common_blit_rectangle(im_old->im, im->im, 0, 0, w, h, 0, 0);
	     evas_common_cpu_end_opt();
	  }
 */
        evas_gl_common_image_free(im_old);
     }
   else
     im = evas_gl_common_image_new(re->win->gl_context, w, h, 1, EVAS_COLORSPACE_ARGB8888);
   return im;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;
   Evas_GL_Image *im = image;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   if (im->native.data) return image;
   eng_window_use(re->win);
   evas_gl_common_image_dirty(image, x, y, w, h);
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data, int *err)
{
   Render_Engine *re;
   Evas_GL_Image *im;
   int error;

   re = (Render_Engine *)data;
   if (!image)
     {
	*image_data = NULL;
        if (err) *err = EVAS_LOAD_ERROR_GENERIC;
	return NULL;
     }
   im = image;
   if (im->native.data)
     {
        *image_data = NULL;
        if (err) *err = EVAS_LOAD_ERROR_NONE;
        return im;
     }
   if ((im->tex) && (im->tex->pt) && (im->tex->pt->dyn.data))
     {
        *image_data = im->tex->pt->dyn.data;
        if (err) *err = EVAS_LOAD_ERROR_NONE;
        return im;
     }
   eng_window_use(re->win);
   error = evas_cache_image_load_data(&im->im->cache_entry);
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (to_write)
	  {
	     if (im->references > 1)
	       {
		  Evas_GL_Image *im_new;

		  im_new = evas_gl_common_image_new_from_copied_data
                     (im->gc, im->im->cache_entry.w, im->im->cache_entry.h,
                         im->im->image.data,
                         eng_image_alpha_get(data, image),
                         eng_image_colorspace_get(data, image));
		  if (!im_new)
		    {
		       *image_data = NULL;
                       if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
		       return im;
		    }
		  evas_gl_common_image_free(im);
		  im = im_new;
	       }
	     else
	       evas_gl_common_image_dirty(im, 0, 0, 0, 0);
	  }
	*image_data = im->im->image.data;
	break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
	*image_data = im->cs.data;
	break;
      default:
	abort();
	break;
     }
   if (err) *err = error;
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im, *im2;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   im = image;
   if (im->native.data) return image;
   eng_window_use(re->win);
   if ((im->tex) && (im->tex->pt) && (im->tex->pt->dyn.data))
     {
        if (im->tex->pt->dyn.data == image_data)
          {
             return image;
          }
        else
          {
	     int w, h;

	     w = im->im->cache_entry.w;
	     h = im->im->cache_entry.h;
	     im2 = eng_image_new_from_data(data, w, h, image_data,
					   eng_image_alpha_get(data, image),
					   eng_image_colorspace_get(data, image));
	     if (!im2) return im;
	     evas_gl_common_image_free(im);
	     im = im2;
             evas_gl_common_image_dirty(im, 0, 0, 0, 0);
             return im;
          }
     }
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (image_data != im->im->image.data)
	  {
	     int w, h;

	     w = im->im->cache_entry.w;
	     h = im->im->cache_entry.h;
	     im2 = eng_image_new_from_data(data, w, h, image_data,
					   eng_image_alpha_get(data, image),
					   eng_image_colorspace_get(data, image));
	     if (!im2) return im;
	     evas_gl_common_image_free(im);
	     im = im2;
	  }
        break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
        if (image_data != im->cs.data)
	  {
	     if (im->cs.data)
	       {
		  if (!im->cs.no_free) free(im->cs.data);
	       }
	     im->cs.data = image_data;
	  }
	break;
      default:
	abort();
	break;
     }
   /* hmmm - but if we wrote... why bother? */
   evas_gl_common_image_dirty(im, 0, 0, 0, 0);
   return im;
}

static void
eng_image_data_preload_request(void *data __UNUSED__, void *image, const void *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;
   evas_cache_image_preload_data(&im->cache_entry, target);
}

static void
eng_image_data_preload_cancel(void *data __UNUSED__, void *image, const void *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;
   evas_cache_image_preload_cancel(&im->cache_entry, target);
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_image_draw(re->win->gl_context, image,
                             src_x, src_y, src_w, src_h,
                             dst_x, dst_y, dst_w, dst_h,
                             smooth);
}

static void
eng_image_scale_hint_set(void *data __UNUSED__, void *image, int hint)
{
   if (image) evas_gl_common_image_scale_hint_set(image, hint);
}

static int
eng_image_scale_hint_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *gim = image;
   if (!gim) return EVAS_IMAGE_SCALE_HINT_NONE;
   return gim->scale_hint;
}

static void
eng_image_map_draw(void *data, void *context, void *surface, void *image, int npoints, RGBA_Map_Point *p, int smooth, int level)
{
   Evas_GL_Image *gim = image;
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   if (npoints != 4)
     {
        // FIXME: nash - you didn't fix this
        abort();
     }
   if ((p[0].x == p[3].x) &&
       (p[1].x == p[2].x) &&
       (p[0].y == p[1].y) &&
       (p[3].y == p[2].y) &&
       (p[0].x <= p[1].x) &&
       (p[0].y <= p[2].y) &&
       (p[0].u == 0) &&
       (p[0].v == 0) &&
       (p[1].u == (gim->w << FP)) &&
       (p[1].v == 0) &&
       (p[2].u == (gim->w << FP)) &&
       (p[2].v == (gim->h << FP)) &&
       (p[3].u == 0) &&
       (p[3].v == (gim->h << FP)) &&
       (p[0].col == 0xffffffff) &&
       (p[1].col == 0xffffffff) &&
       (p[2].col == 0xffffffff) &&
       (p[3].col == 0xffffffff))
     {
        int dx, dy, dw, dh;

        dx = p[0].x >> FP;
        dy = p[0].y >> FP;
        dw = (p[2].x >> FP) - dx;
        dh = (p[2].y >> FP) - dy;
        eng_image_draw(data, context, surface, image,
                       0, 0, gim->w, gim->h, dx, dy, dw, dh, smooth);
     }
   else
     {
        evas_gl_common_image_map_draw(re->win->gl_context, image, npoints, p,
                                      smooth, level);
     }
}

static void *
eng_image_map_surface_new(void *data, int w, int h, int alpha)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_image_surface_new(re->win->gl_context, w, h, alpha);
}

static void
eng_image_map_surface_free(void *data __UNUSED__, void *surface)
{
   evas_gl_common_image_free(surface);
}

static void
eng_image_content_hint_set(void *data __UNUSED__, void *image, int hint)
{
   if (image) evas_gl_common_image_content_hint_set(image, hint);
}

static int
eng_image_content_hint_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *gim = image;
   if (!gim) return EVAS_IMAGE_CONTENT_HINT_NONE;
   return gim->content_hint;
}

static void
eng_image_cache_flush(void *data)
{
   Render_Engine *re;
   int tmp_size;

   re = (Render_Engine *)data;

   tmp_size = evas_common_image_get_cache();
   evas_common_image_set_cache(0);
   evas_common_rgba_image_scalecache_flush();
   evas_gl_common_image_cache_flush(re->win->gl_context);
   evas_common_image_set_cache(tmp_size);
}

static void
eng_image_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_image_set_cache(bytes);
   evas_common_rgba_image_scalecache_size_set(bytes);
   evas_gl_common_image_cache_flush(re->win->gl_context);
}

static int
eng_image_cache_get(void *data __UNUSED__)
{
   return evas_common_image_get_cache();
}

static void
eng_image_stride_get(void *data __UNUSED__, void *image, int *stride)
{
   Evas_GL_Image *im = image;

   *stride = im->w * 4;
   if ((im->tex) && (im->tex->pt->dyn.img))
     {
        *stride = im->tex->pt->dyn.w * 4;
        // FIXME: for other image formats (yuv etc.) different stride needed
     }
}

static void
eng_font_draw(void *data, void *context, void *surface, Evas_Font_Set *font, int x, int y, int w __UNUSED__, int h __UNUSED__, int ow __UNUSED__, int oh __UNUSED__, const Evas_Text_Props *intl_props)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
     {
        // FIXME: put im into context so we can free it
	static RGBA_Image *im = NULL;

        if (!im)
          im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        im->cache_entry.w = re->win->w;
        im->cache_entry.h = re->win->h;
        evas_common_draw_context_font_ext_set(context,
					      re->win->gl_context,
					      evas_gl_font_texture_new,
					      evas_gl_font_texture_free,
					      evas_gl_font_texture_draw);
	evas_common_font_draw(im, context, (RGBA_Font *) font, x, y,
                              intl_props);
	evas_common_draw_context_font_ext_set(context,
					      NULL,
					      NULL,
					      NULL,
					      NULL);
     }
}

static Eina_Bool
eng_canvas_alpha_get(void *data, void *info __UNUSED__)
{
   Render_Engine *re = (Render_Engine *)data;
   return re->win->alpha;
}

static int
_set_internal_config(Render_Engine_GL_Surface *sfc, Evas_GL_Config *cfg)
{
   // Also initialize pixel format here as well...
   switch(cfg->color_format)
     {
      case EVAS_GL_RGB_8:
         sfc->rt_fmt          = GL_RGB;
         sfc->rt_internal_fmt = GL_RGB;
         break;
      case EVAS_GL_RGBA_8:
         sfc->rt_fmt          = GL_RGBA;
         sfc->rt_internal_fmt = GL_RGBA;
         break;
      case EVAS_GL_RGB_32:
         // Only supported on some hw
         // Fill it in later...
      case EVAS_GL_RGBA_32:
         // Only supported on some hw
         // Fill it in later...
      default:
         ERR("Invalid Color Format!");
         return 0;
     }

   switch(cfg->depth_bits)
     {
      case EVAS_GL_DEPTH_NONE:
         break;
      case EVAS_GL_DEPTH_BIT_8:
      case EVAS_GL_DEPTH_BIT_16:
      case EVAS_GL_DEPTH_BIT_24:
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
         // 24 bit doesn't work... just cover it with 16 for now..
         sfc->rb_depth_fmt = GL_DEPTH_COMPONENT16;
#else
         sfc->rb_depth_fmt = GL_DEPTH_COMPONENT;
#endif
         break;
      case EVAS_GL_DEPTH_BIT_32:
      default:
         ERR("Unsupported Depth Bits Format!");
         return 0;
     }

   switch(cfg->stencil_bits)
     {
      case EVAS_GL_STENCIL_NONE:
         break;
      case EVAS_GL_STENCIL_BIT_1:
      case EVAS_GL_STENCIL_BIT_2:
      case EVAS_GL_STENCIL_BIT_4:
      case EVAS_GL_STENCIL_BIT_8:
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
         sfc->rb_stencil_fmt = GL_STENCIL_INDEX8;
#else
         sfc->rb_stencil_fmt = GL_STENCIL_INDEX;
#endif
         break;
      case EVAS_GL_STENCIL_BIT_16:
      default:
         ERR("Unsupported Stencil Bits Format!");
         return 0;
     }

   // Do Packed Depth24_Stencil8 Later...

   return 1;
}

static int
_create_rt_buffers(Render_Engine *data __UNUSED__,
                   Render_Engine_GL_Surface *sfc)
{
   // Render Target texture
   glGenTextures(1, &sfc->rt_tex );
   glBindTexture(GL_TEXTURE_2D, sfc->rt_tex );
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sfc->w, sfc->h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   glBindTexture(GL_TEXTURE_2D, 0);

   // Depth RenderBuffer - Create storage here...
   if (sfc->depth_bits != EVAS_GL_DEPTH_NONE)
     {
        glGenRenderbuffers(1, &sfc->rb_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, sfc->rb_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, sfc->rb_depth_fmt,
                              sfc->w, sfc->h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
     }

   // Stencil RenderBuffer - Create Storage here...
   if (sfc->stencil_bits != EVAS_GL_STENCIL_NONE)
     {
        glGenRenderbuffers(1, &sfc->rb_stencil);
        glBindRenderbuffer(GL_RENDERBUFFER, sfc->rb_stencil);
        glRenderbufferStorage(GL_RENDERBUFFER, sfc->rb_stencil_fmt,
                              sfc->w, sfc->h);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
     }

   return 1;
}

static int
_attach_fbo_surface(Render_Engine *data __UNUSED__,
                    Render_Engine_GL_Surface *sfc,
                    Render_Engine_GL_Context *ctx)
{
   int fb_status;

   // FBO
   glBindFramebuffer(GL_FRAMEBUFFER, ctx->fbo);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                          GL_TEXTURE_2D, sfc->rt_tex, 0);

   // Depth RenderBuffer - Attach it to FBO
   if (sfc->depth_bits != EVAS_GL_DEPTH_NONE)
     {
        glBindRenderbuffer(GL_RENDERBUFFER, sfc->rb_depth);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, sfc->rb_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
     }

   // Stencil RenderBuffer - Attach it to FBO
   if (sfc->stencil_bits != EVAS_GL_STENCIL_NONE)
     {
        glBindRenderbuffer(GL_RENDERBUFFER, sfc->rb_stencil);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, sfc->rb_stencil);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
     }

   // Check FBO for completeness
   fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (fb_status != GL_FRAMEBUFFER_COMPLETE)
     {
        ERR("FBO not complete!");
        return 0;
     }

   return 1;
}

static void *
eng_gl_surface_create(void *data, void *config, int w, int h)
{
   Render_Engine *re;
   Render_Engine_GL_Surface *sfc;
   Evas_GL_Config *cfg;
   int ret;

   sfc = calloc(1, sizeof(Render_Engine_GL_Surface));

   if (!sfc) return NULL;

   re  = (Render_Engine *)data;
   cfg = (Evas_GL_Config *)config;

   sfc->initialized  = 0;
   sfc->fbo_attached = 0;
   sfc->w            = w;
   sfc->h            = h;
   sfc->depth_bits   = cfg->depth_bits;
   sfc->stencil_bits = cfg->stencil_bits;
   sfc->rt_tex       = 0;
   sfc->rb_depth     = 0;
   sfc->rb_stencil   = 0;

   // Set the internal format based on the config
   if (!_set_internal_config(sfc, cfg))
     {
        ERR("Unsupported Format!");
        free(sfc);
        return NULL;
     }

   // Create Render Target Texture/Buffers if not initialized
   if (!sfc->initialized)
     {
        // I'm using evas's original context to create the render target texture
        // This is to prevent awkwardness in using native_surface_get() function
        // If the rt texture creation is deferred till the context is created and
        // make_current called, the user can't call native_surface_get() right
        // after the surface is created. hence this is done here using evas' context.
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
        ret = eglMakeCurrent(re->win->egl_disp, re->win->egl_surface[0], re->win->egl_surface[0], re->win->egl_context[0]);
#else
        ret = glXMakeCurrent(re->info->info.display, re->win->win, re->win->context);
#endif
        if (!ret)
          {
             ERR("xxxMakeCurrent() failed!");
             free(sfc);
             return NULL;
          }

        // Create Render texture
        if (!_create_rt_buffers(re, sfc))
          {
             ERR("_create_rt_buffers() failed.");
             free(sfc);
             return NULL;
          }

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
        ret = eglMakeCurrent(re->win->egl_disp, EGL_NO_SURFACE,
                             EGL_NO_SURFACE, EGL_NO_CONTEXT);
#else
        ret = glXMakeCurrent(re->info->info.display, None, NULL);
#endif
        if (!ret)
          {
             ERR("xxxMakeCurrent() failed!");
             free(sfc);
             return 0;
          }
        sfc->initialized = 1;
     }

   return sfc;
}

static int
eng_gl_surface_destroy(void *data, void *surface)
{
   Render_Engine *re;
   Render_Engine_GL_Surface *sfc;
   int ret;

   re  = (Render_Engine *)data;
   sfc = (Render_Engine_GL_Surface*)surface;

   // I'm using evas's original context to delete the created fbo and texture
   // This is because the fbo/texture was created in the user created context
   // but the context can be destroyed already...
   // I don't think this is the best way but at least for now this is A WAY.
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   ret = eglMakeCurrent(re->win->egl_disp, re->win->egl_surface[0], re->win->egl_surface[0], re->win->egl_context[0]);
#else
   ret = glXMakeCurrent(re->info->info.display, re->win->win, re->win->context);
#endif
   if (!ret)
     {
        ERR("xxxMakeCurrent() failed!");
        return 0;
     }

   // Delete FBO/RBO and Texture here
   if (glIsTexture(sfc->rt_tex))
     glDeleteTextures(1, &sfc->rt_tex);

   if (glIsBuffer(sfc->rb_depth))
     glDeleteRenderbuffers(1, &sfc->rb_depth);

   if (glIsBuffer(sfc->rb_stencil))
     glDeleteRenderbuffers(1, &sfc->rb_stencil);


#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   ret = eglMakeCurrent(re->win->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
#else
   ret = glXMakeCurrent(re->info->info.display, None, NULL);
#endif
   if (!ret)
     {
        ERR("xxxMakeCurrent() failed!");
        return 0;
     }

   free(sfc);
   surface = NULL;

   return 1;
}

static void *
eng_gl_context_create(void *data, void *share_context)
{
   Render_Engine *re;
   Render_Engine_GL_Context *ctx;
   Render_Engine_GL_Context *share_ctx;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   int context_attrs[3];
#endif

   ctx = calloc(1, sizeof(Render_Engine_GL_Context));

   if (!ctx) return NULL;

   re = (Render_Engine *)data;
   share_ctx = (Render_Engine_GL_Context *)share_context;

   // Set the share context to Evas' GL context if share_context is NULL.
   // Otherwise set it to the given share_context.
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   // EGL
   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;

   if (share_ctx)
     {
        ctx->context = eglCreateContext(re->win->egl_disp,
                                        re->win->egl_config,
                                        share_ctx->context,      // Share Context
                                        context_attrs);
     }
   else
     {
        ctx->context = eglCreateContext(re->win->egl_disp,
                                        re->win->egl_config,
                                        re->win->egl_context[0], // Evas' GL Context
                                        context_attrs);
     }

   if (!ctx->context)
     {
        ERR("eglCreateContext() fail. code=%#x", eglGetError());
        return NULL;
     }
#else
   // GLX
   if (share_context)
     {
        ctx->context = glXCreateContext(re->info->info.display,
                                        re->win->visualinfo,
                                        share_ctx->context,    // Share Context
                                        1);
     }
   else
     {
        ctx->context = glXCreateContext(re->info->info.display,
                                        re->win->visualinfo,
                                        re->win->context,      // Evas' GL Context
                                        1);
     }

   if (!ctx->context)
     {
        ERR("glXCreateContext() fail.");
        return NULL;
     }
#endif

   ctx->initialized = 0;
   ctx->fbo = 0;
   ctx->current_sfc = NULL;

   return ctx;
}

static int
eng_gl_context_destroy(void *data, void *context)
{
   Render_Engine *re;
   Render_Engine_GL_Context *ctx;
   int ret;

   re  = (Render_Engine *)data;
   ctx = (Render_Engine_GL_Context*)context;

   // 1. Do a make current with the given context
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   ret = eglMakeCurrent(re->win->egl_disp, re->win->egl_surface[0],
                        re->win->egl_surface[0], ctx->context);
#else
   ret = glXMakeCurrent(re->info->info.display, re->win->win,
                        ctx->context);
#endif
   if (!ret)
     {
        ERR("xxxMakeCurrent() failed!");
        return 0;
     }

   // 2. Delete the FBO
   if (glIsBuffer(ctx->fbo))
     glDeleteBuffers(1, &ctx->fbo);

   // 3. Destroy the Context
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   eglDestroyContext(re->win->egl_disp, ctx->context);

   ctx->context = EGL_NO_CONTEXT;

   ret = eglMakeCurrent(re->win->egl_disp, EGL_NO_SURFACE,
                        EGL_NO_SURFACE, EGL_NO_CONTEXT);
#else
   glXDestroyContext(re->info->info.display, ctx->context);

   ctx->context = 0;

   ret = glXMakeCurrent(re->info->info.display, None, NULL);
#endif
   if (!ret)
     {
        ERR("xxxMakeCurrent() failed!");
        return 0;
     }

   free(ctx);
   context = NULL;

   return 1;
}

static int
eng_gl_make_current(void *data, void *surface, void *context)
{
   Render_Engine *re;
   Render_Engine_GL_Surface *sfc;
   Render_Engine_GL_Context *ctx;
   int ret = 0;

   re  = (Render_Engine *)data;
   sfc = (Render_Engine_GL_Surface*)surface;
   ctx = (Render_Engine_GL_Context*)context;

   // Flush remainder of what's in Evas' pipeline
   if (re->win)
     {
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
        if ((eglGetCurrentContext() == re->win->egl_context[0]) ||
            (eglGetCurrentSurface(EGL_READ) == re->win->egl_surface[0]) ||
            (eglGetCurrentSurface(EGL_DRAW) == re->win->egl_surface[0]))
          {
             evas_gl_common_context_use(re->win->gl_context);
             evas_gl_common_context_flush(re->win->gl_context);
          }
#else
        if (glXGetCurrentContext() == re->win->context)
          {
             evas_gl_common_context_use(re->win->gl_context);
             evas_gl_common_context_flush(re->win->gl_context);
          }
#endif
        eng_window_use(NULL);
        evas_gl_common_context_use(NULL);
     }

   // Unset surface/context
   if ((!sfc) || (!ctx))
     {
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
        ret = eglMakeCurrent(re->win->egl_disp, EGL_NO_SURFACE,
                             EGL_NO_SURFACE, EGL_NO_CONTEXT);
#else
        ret = glXMakeCurrent(re->info->info.display, None, NULL);
#endif
        if (!ret)
          {
             ERR("xxxMakeCurrent() failed!");
             return 0;
          }
        return ret;
     }

   // Don't do a make current if it's already current
   ret = 1;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   if ((eglGetCurrentContext() != ctx->context))
      ret = eglMakeCurrent(re->win->egl_disp, re->win->egl_surface[0],
                           re->win->egl_surface[0], ctx->context);
#else
   if (glXGetCurrentContext() != ctx->context)
      ret = glXMakeCurrent(re->info->info.display, re->win->win, ctx->context);
#endif
   if (!ret)
     {
        ERR("xxxMakeCurrent() failed!");
        return 0;
     }

   // Create FBO if not already created
   if (!ctx->initialized)
     {
        glGenFramebuffers(1, &ctx->fbo);
        ctx->initialized = 1;
     }

   // Attach FBO if it hasn't been attached or if surface changed
   if ((!sfc->fbo_attached) || (ctx != sfc->current_ctx))
     {
        if (!_attach_fbo_surface(re, sfc, ctx))
          {
             ERR("_attach_fbo_surface() failed.");
             return 0;
          }
        sfc->fbo_attached = 1;
     }

   // Set the current surface/context
   ctx->current_sfc = sfc;
   sfc->current_ctx = ctx;

   // Bind FBO
   glBindFramebuffer(GL_FRAMEBUFFER, ctx->fbo);
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#else
   //glDrawBuffer(GL_COLOR_ATTACHMENT0);
#endif

   return 1;
}

static void *
eng_gl_proc_address_get(void *data __UNUSED__, const char *name)
{
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   if (glsym_eglGetProcAddress) return glsym_eglGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
#else
   if (glsym_glXGetProcAddress) return glsym_glXGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
#endif
}

static int
eng_gl_native_surface_get(void *data, void *surface, void *native_surface)
{
   Render_Engine *re;
   Render_Engine_GL_Surface *sfc;
   Evas_Native_Surface *ns;

   re  = (Render_Engine *)data;
   sfc = (Render_Engine_GL_Surface*)surface;
   ns  = (Evas_Native_Surface*)native_surface;

   ns->type = EVAS_NATIVE_SURFACE_OPENGL;
   ns->version = EVAS_NATIVE_SURFACE_VERSION;
   ns->data.opengl.texture_id = sfc->rt_tex;
   //ns->data.opengl.framebuffer_id = sfc->fbo;
   //ns->data.opengl.framebuffer_id = ctx->fbo;
   ns->data.opengl.x = 0;
   ns->data.opengl.y = 0;
   ns->data.opengl.w = sfc->w;
   ns->data.opengl.h = sfc->h;

   return 1;
}

#if 1
static void
evgl_glBindFramebuffer(GLenum target, GLuint framebuffer)
{
   // Add logic to take care when framebuffer=0
   glBindFramebuffer(target, framebuffer);
}

static void
evgl_glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
   // Add logic to take care when renderbuffer=0
   glBindRenderbuffer(target, renderbuffer);
}

static void
evgl_glClearDepthf(GLclampf depth)
{
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   glClearDepthf(depth);
#else
   glClearDepth(depth);
#endif
}

static void
evgl_glDepthRangef(GLclampf zNear, GLclampf zFar)
{
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   glDepthRangef(zNear, zFar);
#else
   glDepthRange(zNear, zFar);
#endif
}

static void
evgl_glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
#else
   if (range)
     {
        range[0] = -126; // floor(log2(FLT_MIN))
        range[1] = 127; // floor(log2(FLT_MAX))
     }
   if (precision)
     {
        precision[0] = 24; // floor(-log2((1.0/16777218.0)));
     }
   return;
   shadertype = precisiontype = 0;
#endif
}

static void
evgl_glReleaseShaderCompiler(void)
{
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   glReleaseShaderCompiler();
#else
#endif
}

static void
evgl_glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length)
{
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   glShaderBinary(n, shaders, binaryformat, binary, length);
#else
// FIXME: need to dlsym/getprocaddress for this
   return;
   n = binaryformat = length = 0;
   shaders = binary = 0;
#endif
}

#endif

static void *
eng_gl_api_get(void *data)
{
   Render_Engine *re;

   re  = (Render_Engine *)data;

   gl_funcs.version = EVAS_GL_API_VERSION;
#if 1
#define ORD(f) EVAS_API_OVERRIDE(f, &gl_funcs, )
   ORD(glActiveTexture);
   ORD(glAttachShader);
   ORD(glBindAttribLocation);
   ORD(glBindBuffer);
   ORD(glBindTexture);
   ORD(glBlendColor);
   ORD(glBlendEquation);
   ORD(glBlendEquationSeparate);
   ORD(glBlendFunc);
   ORD(glBlendFuncSeparate);
   ORD(glBufferData);
   ORD(glBufferSubData);
   ORD(glCheckFramebufferStatus);
   ORD(glClear);
   ORD(glClearColor);
//   ORD(glClearDepthf);
   ORD(glClearStencil);
   ORD(glColorMask);
   ORD(glCompileShader);
   ORD(glCompressedTexImage2D);
   ORD(glCompressedTexSubImage2D);
   ORD(glCopyTexImage2D);
   ORD(glCopyTexSubImage2D);
   ORD(glCreateProgram);
   ORD(glCreateShader);
   ORD(glCullFace);
   ORD(glDeleteBuffers);
   ORD(glDeleteFramebuffers);
   ORD(glDeleteProgram);
   ORD(glDeleteRenderbuffers);
   ORD(glDeleteShader);
   ORD(glDeleteTextures);
   ORD(glDepthFunc);
   ORD(glDepthMask);
//   ORD(glDepthRangef);
   ORD(glDetachShader);
   ORD(glDisable);
   ORD(glDisableVertexAttribArray);
   ORD(glDrawArrays);
   ORD(glDrawElements);
   ORD(glEnable);
   ORD(glEnableVertexAttribArray);
   ORD(glFinish);
   ORD(glFlush);
   ORD(glFramebufferRenderbuffer);
   ORD(glFramebufferTexture2D);
   ORD(glFrontFace);
   ORD(glGenBuffers);
   ORD(glGenerateMipmap);
   ORD(glGenFramebuffers);
   ORD(glGenRenderbuffers);
   ORD(glGenTextures);
   ORD(glGetActiveAttrib);
   ORD(glGetActiveUniform);
   ORD(glGetAttachedShaders);
   ORD(glGetAttribLocation);
   ORD(glGetBooleanv);
   ORD(glGetBufferParameteriv);
   ORD(glGetError);
   ORD(glGetFloatv);
   ORD(glGetFramebufferAttachmentParameteriv);
   ORD(glGetIntegerv);
   ORD(glGetProgramiv);
   ORD(glGetProgramInfoLog);
   ORD(glGetRenderbufferParameteriv);
   ORD(glGetShaderiv);
   ORD(glGetShaderInfoLog);
//   ORD(glGetShaderPrecisionFormat);
   ORD(glGetShaderSource);
   ORD(glGetString);
   ORD(glGetTexParameterfv);
   ORD(glGetTexParameteriv);
   ORD(glGetUniformfv);
   ORD(glGetUniformiv);
   ORD(glGetUniformLocation);
   ORD(glGetVertexAttribfv);
   ORD(glGetVertexAttribiv);
   ORD(glGetVertexAttribPointerv);
   ORD(glHint);
   ORD(glIsBuffer);
   ORD(glIsEnabled);
   ORD(glIsFramebuffer);
   ORD(glIsProgram);
   ORD(glIsRenderbuffer);
   ORD(glIsShader);
   ORD(glIsTexture);
   ORD(glLineWidth);
   ORD(glLinkProgram);
   ORD(glPixelStorei);
   ORD(glPolygonOffset);
   ORD(glReadPixels);
//   ORD(glReleaseShaderCompiler);
   ORD(glRenderbufferStorage);
   ORD(glSampleCoverage);
   ORD(glScissor);
//   ORD(glShaderBinary);
   ORD(glShaderSource);
   ORD(glStencilFunc);
   ORD(glStencilFuncSeparate);
   ORD(glStencilMask);
   ORD(glStencilMaskSeparate);
   ORD(glStencilOp);
   ORD(glStencilOpSeparate);
   ORD(glTexImage2D);
   ORD(glTexParameterf);
   ORD(glTexParameterfv);
   ORD(glTexParameteri);
   ORD(glTexParameteriv);
   ORD(glTexSubImage2D);
   ORD(glUniform1f);
   ORD(glUniform1fv);
   ORD(glUniform1i);
   ORD(glUniform1iv);
   ORD(glUniform2f);
   ORD(glUniform2fv);
   ORD(glUniform2i);
   ORD(glUniform2iv);
   ORD(glUniform3f);
   ORD(glUniform3fv);
   ORD(glUniform3i);
   ORD(glUniform3iv);
   ORD(glUniform4f);
   ORD(glUniform4fv);
   ORD(glUniform4i);
   ORD(glUniform4iv);
   ORD(glUniformMatrix2fv);
   ORD(glUniformMatrix3fv);
   ORD(glUniformMatrix4fv);
   ORD(glUseProgram);
   ORD(glValidateProgram);
   ORD(glVertexAttrib1f);
   ORD(glVertexAttrib1fv);
   ORD(glVertexAttrib2f);
   ORD(glVertexAttrib2fv);
   ORD(glVertexAttrib3f);
   ORD(glVertexAttrib3fv);
   ORD(glVertexAttrib4f);
   ORD(glVertexAttrib4fv);
   ORD(glVertexAttribPointer);
   ORD(glViewport);
#undef ORD

// Override functions wrapped by Evas_GL
#define ORD(f) EVAS_API_OVERRIDE(f, &gl_funcs, evgl_)
   ORD(glBindFramebuffer);
   ORD(glBindRenderbuffer);

// GLES2.0 API compat on top of desktop gl
   ORD(glClearDepthf);
   ORD(glDepthRangef);
   ORD(glGetShaderPrecisionFormat);
   ORD(glReleaseShaderCompiler);
   ORD(glShaderBinary);
#undef ORD

#endif

   return &gl_funcs;
}

static int
eng_image_load_error_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *im;

   if (!image) return EVAS_LOAD_ERROR_NONE;
   im = image;
   return im->im->cache_entry.load_error;
}

static int
module_open(Evas_Module *em)
{
   static Eina_Bool xrm_inited = EINA_FALSE;
   if (!xrm_inited)
     {
	xrm_inited = EINA_TRUE;
	XrmInitialize();
     }

   if (!em) return 0;
   if (!evas_gl_common_module_open()) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   if (_evas_engine_GL_X11_log_dom < 0)
     _evas_engine_GL_X11_log_dom = eina_log_domain_register
       ("evas-gl_x11", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_GL_X11_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
   #define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(canvas_alpha_get);
   ORD(output_free);
   ORD(output_resize);
   ORD(output_tile_size_set);
   ORD(output_redraws_rect_add);
   ORD(output_redraws_rect_del);
   ORD(output_redraws_clear);
   ORD(output_redraws_next_update_get);
   ORD(output_redraws_next_update_push);
   ORD(context_cutout_add);
   ORD(context_cutout_clear);
   ORD(output_flush);
   ORD(output_idle_flush);
   ORD(output_dump);
   ORD(rectangle_draw);
   ORD(line_draw);
   ORD(polygon_point_add);
   ORD(polygon_points_clear);
   ORD(polygon_draw);

   ORD(image_load);
   ORD(image_new_from_data);
   ORD(image_new_from_copied_data);
   ORD(image_free);
   ORD(image_size_get);
   ORD(image_size_set);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_data_preload_request);
   ORD(image_data_preload_cancel);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_border_set);
   ORD(image_border_get);
   ORD(image_draw);
   ORD(image_comment_get);
   ORD(image_format_get);
   ORD(image_colorspace_set);
   ORD(image_colorspace_get);
   ORD(image_mask_create);
   ORD(image_native_set);
   ORD(image_native_get);
#if 0 // filtering disabled
   ORD(image_draw_filtered);
   ORD(image_filtered_get);
   ORD(image_filtered_save);
   ORD(image_filtered_free);
#endif

   ORD(font_draw);

   ORD(image_scale_hint_set);
   ORD(image_scale_hint_get);
   ORD(image_stride_get);

   ORD(image_map_draw);
   ORD(image_map_surface_new);
   ORD(image_map_surface_free);

   ORD(image_content_hint_set);
   ORD(image_content_hint_get);

   ORD(image_cache_flush);
   ORD(image_cache_set);
   ORD(image_cache_get);

   ORD(gl_surface_create);
   ORD(gl_surface_destroy);
   ORD(gl_context_create);
   ORD(gl_context_destroy);
   ORD(gl_make_current);
   ORD(gl_proc_address_get);
   ORD(gl_native_surface_get);

   ORD(gl_api_get);

   ORD(image_load_error_get);

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
{
    eina_log_domain_unregister(_evas_engine_GL_X11_log_dom);
    if (xrdb_user.db)
      {
	 XrmDestroyDatabase(xrdb_user.db);
	 xrdb_user.last_stat = 0;
	 xrdb_user.last_mtime = 0;
	 xrdb_user.db = NULL;
      }
    evas_gl_common_module_close();
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "gl_x11",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, gl_x11);

#ifndef EVAS_STATIC_BUILD_GL_XLIB
EVAS_EINA_MODULE_DEFINE(engine, gl_x11);
#endif

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
