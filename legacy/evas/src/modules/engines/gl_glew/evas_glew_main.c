#include "evas_common.h"
#include "evas_engine.h"
#include "Evas_Engine_GL_Glew.h"


static Evas_GL_Glew_Window *_evas_gl_glew_window = NULL;

int
evas_glew_init(HWND window, HDC *dc, HGLRC *context)
{
   PIXELFORMATDESCRIPTOR pfd;
   int                   format;

   *dc = GetDC(window);
   if (!*dc)
    goto no_dc;

   ZeroMemory(&pfd, sizeof (pfd));
   pfd.nSize = sizeof (pfd);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = 24;
   pfd.cDepthBits = 32;
   pfd.iLayerType = PFD_MAIN_PLANE;

   format = ChoosePixelFormat(*dc, &pfd);
   if (!format)
     goto no_format;

   SetPixelFormat(*dc, format, &pfd);

   if (pfd.iPixelType != PFD_TYPE_RGBA)
     goto no_format;

   *context = wglCreateContext(*dc);
   if (!*context)
     goto no_format;

   wglMakeCurrent(*dc, *context);

   if (glewInit() != GLEW_OK)
     goto glew_init_failed;

   if (GLEW_VERSION_2_0)
     {
       printf ("2.0\n");
     }

   return 1;

 glew_init_failed:
   wglMakeCurrent(NULL, NULL);
   wglDeleteContext(*context);
 no_format:
   ReleaseDC(window, *dc);
 no_dc:

  return 0;
}

void
evas_glew_shutdown(HWND  window,
                   HDC   dc,
                   HGLRC context)
{
   wglMakeCurrent(NULL, NULL);
   wglDeleteContext(context);
   ReleaseDC(window, dc);
}

Evas_GL_Glew_Window *
eng_window_new(HWND  window,
               HDC   dc,
               HGLRC context,
               int   depth,
               int   width,
               int   height)
{
   Evas_GL_Glew_Window *gw;

   gw = calloc(1, sizeof(Evas_GL_Glew_Window));
   if (!gw) return NULL;

   gw->window = window;
   gw->dc = dc;
   gw->context = context;
   gw->depth = depth;

   gw->gl_context = evas_gl_common_context_new();
   if (!gw->gl_context)
     {
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
