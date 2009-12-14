#include "evas_engine.h"

#ifdef HAVE_GL_GLEW_H
# include <GL/wglew.h>
#endif

static Evas_GL_Glew_Window *_evas_gl_glew_window = NULL;

Evas_GL_Glew_Window *
eng_window_new(HWND  window,
               int   depth,
               int   width,
               int   height)
{
   PIXELFORMATDESCRIPTOR pfd;
   Evas_GL_Glew_Window  *gw;
   int                   format;

   gw = calloc(1, sizeof(Evas_GL_Glew_Window));
   if (!gw) return NULL;

   gw->window = window;
   gw->depth = depth;

   gw->dc = GetDC(window);
   if (!gw->dc)
    goto free_window;

   ZeroMemory(&pfd, sizeof (pfd));
   pfd.nSize = sizeof (pfd);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = 24;
   pfd.cDepthBits = 32;
   pfd.iLayerType = PFD_MAIN_PLANE;

   format = ChoosePixelFormat(gw->dc, &pfd);
   if (!format)
     goto release_dc;

   SetPixelFormat(gw->dc, format, &pfd);

   if (pfd.iPixelType != PFD_TYPE_RGBA)
     goto release_dc;

   gw->context = wglCreateContext(gw->dc);
   if (!gw->context)
     goto release_dc;

   wglMakeCurrent(gw->dc, gw->context);

   if (glewInit() != GLEW_OK)
     goto delete_context;

   if (!GLEW_VERSION_2_0)
     {
        fprintf(stderr, "\nERROR: OpenGL 2.0 not supported. Exiting...\n\n");
        goto delete_context;
     }

   _evas_gl_glew_window = gw;

   gw->gl_context = evas_gl_common_context_new();
   if (!gw->gl_context)
     goto delete_context;
   evas_gl_common_context_resize(gw->gl_context, width, height);

   return gw;

 delete_context:
   wglMakeCurrent(NULL, NULL);
   wglDeleteContext(gw->context);
 release_dc:
   ReleaseDC(window, gw->dc);
 free_window:
   free(gw);

   return NULL;
}

void
eng_window_free(Evas_GL_Glew_Window *gw)
{
   if (!gw)
     return;
   if (gw == _evas_gl_glew_window) _evas_gl_glew_window = NULL;
   evas_gl_common_context_free(gw->gl_context);
   wglMakeCurrent(NULL, NULL);
   wglDeleteContext(gw->context);
   ReleaseDC(gw->window, gw->dc);
   free(gw);
}

void
eng_window_use(Evas_GL_Glew_Window *gw)
{
   if (_evas_gl_glew_window != gw)
     {
        if (_evas_gl_glew_window)
          evas_gl_common_context_flush(_evas_gl_glew_window->gl_context);
        _evas_gl_glew_window = gw;
        wglMakeCurrent(gw->dc, gw->context);
     }
   evas_gl_common_context_use(gw->gl_context);
}

void
eng_window_swap_buffers(Evas_GL_Glew_Window *gw)
{
   SwapBuffers(gw->dc);
}

void
eng_window_vsync_set(int on)
{
#if 1 /* Using Glew */
   wglSwapIntervalEXT(on);
#else /* Using plain OpenGL */
   const char *extensions = glGetString(GL_EXTENSIONS);

   /* check if WGL_EXT_swap_control extension is supported */
   if (strstr(extensions, "WGL_EXT_swap_control") == 0)
     return;
   else
     {
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALFARPROC)wglGetProcAddress("wglSwapIntervalEXT");

        if (wglSwapIntervalEXT)
          wglSwapIntervalEXT(on);
     }
#endif
}
