#include <Eina.h>
#include <Ecore.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*-------------------------------------------------------------------------*/

#define SYM(lib, xx)                         \
   do {                                      \
      sym_ ## xx = dlsym(lib, #xx);          \
      if (!(sym_ ## xx)) {                   \
         fail = 1;                           \
      }                                      \
   } while (0)

#define GLX_DRAWABLE_TYPE 0x8010
#define GLX_WINDOW_BIT 0x00000001
#define GLX_RENDER_TYPE 0x8011
#define GLX_RGBA_BIT 0x00000001
#define GLX_DOUBLEBUFFER 5
#define GLX_RED_SIZE 8
#define GLX_GREEN_SIZE 9
#define GLX_BLUE_SIZE 10
#define GLX_RGBA_TYPE 0x8014
typedef struct __GLXFBConfigRec *GLXFBConfig;

typedef struct
{
   void *visual;
   long visualid;
   int screen;
   int depth;
   int c_class;
   unsigned long red_mask;
   unsigned long green_mask;
   unsigned long blue_mask;
   int colormap_size;
   int bits_per_rgb;
} XVisualInfo;

typedef struct
{
   long background_pixmap;
   unsigned long background_pixel;
   long border_pixmap;
   unsigned long border_pixel;
   int bit_gravity;
   int win_gravity;
   int backing_store;
   unsigned long backing_planes;
   unsigned long backing_pixel;
   int save_under;
   long event_mask;
   long do_not_propagate_mask;
   int override_redirect;
   long colormap;
   long cursor;
} XSetWindowAttributes;

typedef struct {
   void *ext_data;
   void *display;
   long root;
   int width, height;
   int mwidth, mheight;
   int ndepths;
   void *depths;
   int root_depth;
   void *root_visual;
   long default_gc;
   long cmap;
   unsigned long white_pixel;
   unsigned long black_pixel;
   int max_maps, min_maps;
   int backing_store;
   int save_unders;
   long root_input_mask;
} Screen;

typedef struct
{
   void *ext_data;
   void *private1;
   int fd;
   int private2;
   int proto_major_version;
   int proto_minor_version;
   char *vendor;
   long private3;
   long private4;
   long private5;
   int private6;
   long (*resource_alloc)(void *);
   int byte_order;
   int bitmap_unit;
   int bitmap_pad;
   int bitmap_bit_order;
   int nformats;
   void *pixmap_format;
   int private8;
   int release;
   void *private9, *private10;
   int qlen;
   unsigned long last_request_read;
   unsigned long request;
   char *private11;
   char *private12;
   char *private13;
   char *private14;
   unsigned max_request_size;
   void *db;
   int (*private15)(void *);
   char *display_name;
   int default_screen;
   int nscreens;
   Screen *screens;
   unsigned long motion_buffer;
   unsigned long private16;
   int min_keycode;
   int max_keycode;
   char *private17;
   char *private18;
   int private19;
   char *xdefaults;
} *_XPrivDisplay;

#define InputOutput 1
#define RootWindow(dpy, scr) (ScreenOfDisplay(dpy,scr)->root)
#define ScreenOfDisplay(dpy, scr) (&((_XPrivDisplay)dpy)->screens[scr])
#define CWBorderPixel (1L<<3)
#define CWColormap (1L<<13)
#define CWOverrideRedirect (1L<<9)
#define AllocNone 0

/*-------------------------------------------------------------------------*/

static const char *lib_x11_files[] =
{
   "libX11.so.6",
   "libX11.so.5",
   "libX11.so.4",
   "libX11.so",
   NULL
};
static void *lib_x11 = NULL;
static void * (*sym_XOpenDisplay) (char *name) = NULL;
static long (*sym_XCreateColormap) (void *d, long w, void *vis, int alloc) = NULL;
static long (*sym_XCreateWindow) (void *d, long par, int x, int y, unsigned int w, unsigned int h, unsigned int bd, int depth, unsigned int clas, void *vis, unsigned long valmask, void *attr) = NULL;
static int (*sym_XCloseDisplay) (void *d) = NULL;
//static void * (*sym_) () = NULL;

static const char *lib_gl_files[] =
{
   "libGL.so.9",
   "libGL.so.8",
   "libGL.so.7",
   "libGL.so.6",
   "libGL.so.5",
   "libGL.so.4",
   "libGL.so.3",
   "libGL.so.2",
   "libGL.so.1",
   "libGL.so",
   NULL
};
static void *lib_gl = NULL;
static GLXFBConfig * (*sym_glXChooseFBConfig) (void *d, int sc, const int *att, int *n) = NULL;
static void * (*sym_glXGetVisualFromFBConfig) (void *d, GLXFBConfig fbconfig) = NULL;
static void *(*sym_glXCreateNewContext) (void *d, GLXFBConfig config, int rtype, void *shr, int direct) = NULL;
static long (*sym_glXCreateWindow) (void *d, GLXFBConfig config, long win, const int *attr) = NULL;
static int (*sym_glXMakeContextCurrent) (void *d, long draw, long draw2, void *context) = NULL;
static void (*sym_glXSwapBuffers) (void *d, long draw) = NULL;
static const char * (*sym_glXQueryExtensionsString) (void *d, int ival) = NULL;
static void * (*sym_glXGetProcAddressARB) (const char *sym) = NULL;
static int (*sym_glXGetVideoSyncSGI) (unsigned int *cnt) = NULL;
static int (*sym_glXWaitVideoSyncSGI) (int divisor, int remainder, unsigned int *cnt) = NULL;
//static void * (*sym_) () = NULL;

/*-------------------------------------------------------------------------*/

static void *disp = NULL;
static long gwin = 0;
static void *context = NULL;

static int _vsync_init_glx2(void);

static void
_vsync_shutdown_glx(void)
{
   if (disp) sym_XCloseDisplay(disp);
   disp = NULL;
}

static void *
lib_load(const char *files[])
{
   int i;
   void *lib = NULL;

   for (i = 0; files[i]; i++)
     {
        lib = dlopen(files[i], RTLD_LOCAL | RTLD_LAZY);
        if (lib) return lib;
     }
   return NULL;
}

int
_vsync_init_glx(void)
{
   int fail = 0;

   /*---------------------------*/
   lib_x11 = lib_load(lib_x11_files);
   if (!lib_x11) goto err;

   SYM(lib_x11, XOpenDisplay);
   SYM(lib_x11, XCreateColormap);
   SYM(lib_x11, XCreateWindow);
   SYM(lib_x11, XCloseDisplay);
   if (fail) goto err;

   /*---------------------------*/
   lib_gl = lib_load(lib_gl_files);
   if (!lib_gl)
     {
        dlclose(lib_x11);
        return 0;
     }

   SYM(lib_gl, glXChooseFBConfig);
   SYM(lib_gl, glXGetVisualFromFBConfig);
   SYM(lib_gl, glXCreateNewContext);
   SYM(lib_gl, glXCreateWindow);
   SYM(lib_gl, glXMakeContextCurrent);
   SYM(lib_gl, glXSwapBuffers);
   SYM(lib_gl, glXQueryExtensionsString);
   SYM(lib_gl, glXGetProcAddressARB);
   if (fail) goto err;

   /*---------------------------*/
   if (!_vsync_init_glx2()) goto err;
   return 1;

err:
   if (disp) sym_XCloseDisplay(disp);
   if (lib_gl) dlclose(lib_gl);
   if (lib_x11) dlclose(lib_x11);
   return 0;
}

static int
_vsync_init_glx2(void)
{
   GLXFBConfig *fbconfigs;
   int num = 0;
   int attr[] =
     {
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER,  1,
        GLX_RED_SIZE,      1,
        GLX_GREEN_SIZE,    1,
        GLX_BLUE_SIZE,     1,
        0
     };
   XVisualInfo *vi;
   XSetWindowAttributes wa;
   long win;
   const char *extns;

   disp = sym_XOpenDisplay(NULL);
   if (!disp) return 0;
   fbconfigs = sym_glXChooseFBConfig(disp, 0, attr, &num);
   if (!fbconfigs) return 0;
   vi = sym_glXGetVisualFromFBConfig(disp, fbconfigs[0]);
   if (!vi) return 0;
   wa.override_redirect = 1;
   wa.border_pixel = 0;
   wa.colormap = sym_XCreateColormap(disp, RootWindow(disp, vi->screen),
                                     vi->visual, AllocNone);
   if (!wa.colormap) return 0;
   win = sym_XCreateWindow(disp, RootWindow(disp, vi->screen),
                           -77, -777, 1, 1, 0, vi->depth, InputOutput,
                           vi->visual,
                           CWBorderPixel | CWColormap | CWOverrideRedirect, &wa);
   if (!win) return 0;
   context = sym_glXCreateNewContext(disp, fbconfigs[0], GLX_RGBA_TYPE,
                                     NULL, 1);
   if (!context) return 0;
   gwin = sym_glXCreateWindow(disp, fbconfigs[0], win, NULL);
   if (!gwin) return 0;
   extns = sym_glXQueryExtensionsString(disp, 0);
   if (!extns) return 0;
   if (!strstr(extns, "GLX_SGI_swap_control")) return 0;
   sym_glXGetVideoSyncSGI = sym_glXGetProcAddressARB("glXGetVideoSyncSGI");
   sym_glXWaitVideoSyncSGI = sym_glXGetProcAddressARB("glXWaitVideoSyncSGI");
   if ((!sym_glXGetVideoSyncSGI) || (!sym_glXWaitVideoSyncSGI)) return 0;

   return 1;
}

double
_vsync_wait_glx(void)
{
   double t;
   unsigned int rc = 0;
   static unsigned int prc = 0;

again:
   sym_glXMakeContextCurrent(disp, gwin, gwin, context);
   sym_glXGetVideoSyncSGI(&rc);
   sym_glXWaitVideoSyncSGI(1, 0, &rc);
   if (prc == rc)
     {
        _vsync_shutdown_glx();
        usleep(200000);
        if (!_vsync_init_glx2()) exit(1);
        goto again;
     }
   prc = rc;
   t = ecore_time_get();
   return t;
}
