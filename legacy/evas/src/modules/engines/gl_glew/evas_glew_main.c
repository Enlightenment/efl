#include "evas_common.h"
#include "evas_engine.h"
#include "Evas_Engine_GL_Glew.h"


static Evas_GL_Glew_Window *_evas_gl_glew_window = NULL;

static HGLRC context = NULL;
static int   glew_is_init = 0;

Evas_GL_Glew_Window *
eng_window_new(HDC  dc,
               HWND window,
               int  depth,
               int  width,
               int  height)
{
   Evas_GL_Glew_Window *gw;

   gw = calloc(1, sizeof(Evas_GL_Glew_Window));
   if (!gw) return NULL;

   gw->dc = dc;
   gw->window = window;
   gw->depth = depth;

   if (!context)
     context = wglCreateContext(dc);
   if (!context)
     {
	free(gw);
        return NULL;
     }
   gw->context = context;
   wglMakeCurrent(dc, context);
   if (!glew_is_init)
     if (glewInit() != GLEW_OK)
       {
          wglMakeCurrent(NULL, NULL);
          wglDeleteContext(context);
          free(gw);
          return NULL;
       }
   gw->gl_context = evas_gl_common_context_new();
   if (!gw->gl_context)
     {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(context);
	free(gw);
	return NULL;
     }
   evas_gl_common_context_resize(gw->gl_context, width, height);

   return gw;
}

void
eng_window_free(Evas_GL_Glew_Window *gw)
{
   if (gw == _evas_gl_glew_window) _evas_gl_glew_window = NULL;
   evas_gl_common_context_free(gw->gl_context);
   /* wglDeleteContext(gw->context); */
   free(gw);
}

void
eng_window_use(Evas_GL_Glew_Window *gw)
{
   if (_evas_gl_glew_window != gw)
     {
	_evas_gl_glew_window = gw;
	wglMakeCurrent(gw->dc, gw->context);
     }
   evas_gl_common_context_use(gw->gl_context);
}
