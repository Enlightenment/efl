#include "evas_gl_common.h"

#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "evas_common.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_GL_X11.h"

static Evas_GL_X11_Window *_evas_gl_x11_window = NULL;

/* FIXME: this will only work for 1 display connection... :( */
static GLXContext context = 0;

int _evas_gl_x11_configuration[9] =
{
   GLX_DOUBLEBUFFER,
     GLX_RGBA,
     GLX_RED_SIZE,   1,
     GLX_GREEN_SIZE, 1,
     GLX_BLUE_SIZE,  1,
     None
};

XVisualInfo *_evas_gl_x11_vi = NULL;
Colormap     _evas_gl_x11_cmap = 0;

Evas_GL_X11_Window *
eng_window_new(Display *disp,
	       Window   win,
	       int      screen,
	       Visual  *vis,
	       Colormap cmap,
	       int      depth,
	       int      w,
	       int      h)
{
   Evas_GL_X11_Window *gw;

   if (!_evas_gl_x11_vi) return NULL;
   gw = calloc(1, sizeof(Evas_GL_X11_Window));
   if (!gw) return NULL;
   gw->disp = disp;
   gw->win = win;
   gw->screen = screen;
   gw->visual = vis;
   gw->colormap = cmap;
   gw->depth = depth;

   gw->visualinfo = _evas_gl_x11_vi;
   if (!context)
     context = glXCreateContext(disp, gw->visualinfo, NULL, GL_TRUE);
   gw->context = context;
   glXMakeCurrent(gw->disp, gw->win, gw->context);
   gw->gl_context = evas_gl_common_context_new();
   if (!gw->gl_context)
     {
	glXDestroyContext(gw->disp, gw->context);
	free(gw);
	return NULL;
     }
   evas_gl_common_context_resize(gw->gl_context, w, h);
   return gw;
}

void
eng_window_free(Evas_GL_X11_Window *gw)
{
   if (gw == _evas_gl_x11_window) _evas_gl_x11_window = NULL;
   evas_gl_common_context_free(gw->gl_context);
//   glXDestroyContext(gw->disp, gw->context);
   free(gw);
}

void
eng_window_use(Evas_GL_X11_Window *gw)
{
   if (_evas_gl_x11_window != gw)
     {
	_evas_gl_x11_window = gw;
	glXMakeCurrent(gw->disp, gw->win, gw->context);
     }
   evas_gl_common_context_use(gw->gl_context);
}
