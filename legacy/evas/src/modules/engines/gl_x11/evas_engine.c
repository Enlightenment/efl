#include "evas_common.h" /* Also includes international specific stuff */
#include "evas_engine.h"

#include <dlfcn.h>      /* dlopen,dlclose,etc */

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
// EGL / GLES
# if defined(GLES_VARIETY_S3C6410)
# elif defined(GLES_VARIETY_SGX)
# endif
#else
// GLX
#endif

typedef struct _Render_Engine Render_Engine;

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

   FINDSYM(glsym_glxSwapIntervalSGI, "glxSwapIntervalSGI", glsym_func_int);
   
   FINDSYM(glsym_glxSwapIntervalEXT, "glxSwapIntervalEXT", glsym_func_void);
#endif
}

int _evas_engine_GL_X11_log_dom = -1;
/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

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
   if (re->win->alpha)
     {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
     }
   
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
                  if (strstr(s, "PowerVR SGX 540"))
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
   eglSwapBuffers(re->win->egl_disp, re->win->egl_surface[0]);
   if (!safe_native) eglWaitGL();
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
   
   if (re->win->alpha)
     {
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
     }
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
      // FIXME: implement
    }
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
      // FIXME: implement
    }
}

static void
_native_free_cb(void *data, void *image)
{
  Render_Engine *re = data;
  Evas_GL_Image *im = image;
  Native *n = im->native.data;
  uint32_t pmid;

  if (n->ns.type == EVAS_NATIVE_SURFACE_X11)
    {
      pmid = n->pixmap;
      eina_hash_del(re->win->gl_context->shared->native_hash, &pmid, im);
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
      // FIXME: implement
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
  uint32_t pmid;
  
  if (!im) return NULL;
  
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
          // FIXME: implement
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
      im2 = eina_hash_find(re->win->gl_context->shared->native_hash, &pmid);
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
      // FIXME: implement
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
              
              eina_hash_add(re->win->gl_context->shared->native_hash, &pmid, im);
              
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
              
              eina_hash_add(re->win->gl_context->shared->native_hash, &pmid, im);
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
      // FIXME: implement
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
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image)
     {
	*image_data = NULL;
	return NULL;
     }
   im = image;
   if (im->native.data)
     {
        *image_data = NULL;
        return im;
     }
   if ((im->tex) && (im->tex->pt) && (im->tex->pt->dyn.data))
     {
        *image_data = im->tex->pt->dyn.data;
        return im;
     }
   eng_window_use(re->win);
   evas_cache_image_load_data(&im->im->cache_entry);
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
eng_image_map_draw(void *data __UNUSED__, void *context, void *surface, void *image, int npoints, RGBA_Map_Point *p, int smooth, int level)
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
        // FIXME: nash - you didnt fix this
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
eng_image_map_surface_new(void *data __UNUSED__, int w, int h, int alpha)
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
eng_image_cache_flush(void *data __UNUSED__)
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
eng_image_cache_set(void *data __UNUSED__, int bytes)
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
   Render_Engine *re;
   
   re = (Render_Engine *)data;
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
eng_font_draw(void *data, void *context, void *surface, void *font, int x, int y, int w __UNUSED__, int h __UNUSED__, int ow __UNUSED__, int oh __UNUSED__, const Eina_Unicode *text, const Evas_Text_Props *intl_props)
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
	evas_common_font_draw(im, context, font, x, y, text, intl_props);
	evas_common_draw_context_font_ext_set(context,
					      NULL,
					      NULL,
					      NULL,
					      NULL);
     }
}

static Eina_Bool
eng_canvas_alpha_get(void *data __UNUSED__, void *info __UNUSED__)
{
   // FIXME: support ARGB gl targets!!!
   return EINA_FALSE;
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
   ORD(image_native_set);
   ORD(image_native_get);
   
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

#ifndef EVAS_STATIC_BUILD_GL_X11
EVAS_EINA_MODULE_DEFINE(engine, gl_x11);
#endif
