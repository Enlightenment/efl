/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "config.h"
#include <GL/glew.h>
#include "ecore_win32_private.h"
#include "Ecore_Win32.h"
#include <stdio.h>


static int _ecore_win32_glew_init = 0;

EAPI int
ecore_win32_glew_init(Ecore_Win32_Window *window)
{
#ifdef HAVE_OPENGL_GLEW
   PIXELFORMATDESCRIPTOR pfd;
   int                   format;

   RECT                        rect;
   struct _Ecore_Win32_Window *w;
   DWORD                       flag;

   if (!window)
     return 0;

   w = (struct _Ecore_Win32_Window *)window;

   printf ("ecore_win32_glew_init debut : %p (%d %d) (%d %d)\n",
           w,
           w->min_width,
           w->min_height,
           w->max_width,
           w->max_height);

   w->glew.dc = GetDC (w->window);
   if (!w->glew.dc)
     goto no_dc;

  ZeroMemory (&pfd, sizeof (pfd));
  pfd.nSize = sizeof (pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 32;
  pfd.iLayerType = PFD_MAIN_PLANE;

  format = ChoosePixelFormat (w->glew.dc, &pfd);
  if (!format)
    goto no_format;

  SetPixelFormat (w->glew.dc, format, &pfd);

  w->glew.depth = 32;

   printf ("ecore_win32_glew_init fin : %p (%d %d) (%d %d)\n",
           w,
           w->min_width,
           w->min_height,
           w->max_width,
           w->max_height);

   if (!_ecore_win32_glew_init)
     {
/*         if (glewInit() != GLEW_OK) */
/*           { */
/*              printf ("can not init Glew\n"); */
/*              goto no_glew_init; */
/*           } */
        if (GLEW_VERSION_2_0)
          {
             printf ("2.0\n");
             goto no_glew_2_0;
          }
        else {
          printf ("pas 2.0 !!\n");
        }
        _ecore_win32_glew_init = 1;
     }

   w->backend = ECORE_WIN32_BACKEND_GLEW;

   return 1;

 no_glew_2_0:
 no_glew_init:
 no_format:
   ReleaseDC (w->window, w->glew.dc);
 no_dc:
#endif /* HAVE_OPENGL_GLEW */

   return 0;
}

EAPI void
ecore_win32_glew_shutdown(Ecore_Win32_Window *window)
{
#ifdef HAVE_OPENGL_GLEW
   struct _Ecore_Win32_Window *w;

   if (!window)
     return;

   w = (struct _Ecore_Win32_Window *)window;

   if (w->glew.dc)
     ReleaseDC (w->window, w->glew.dc);

   w->backend = ECORE_WIN32_BACKEND_NONE;
#endif /* HAVE_OPENGL_GLEW */
}

EAPI Ecore_Win32_Glew_DC *
ecore_win32_glew_dc_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_OPENGL_GLEW
   return ((struct _Ecore_Win32_Window *)window)->glew.dc;
#else
   return NULL;
#endif /* HAVE_OPENGL_GLEW */
}

EAPI int
ecore_win32_glew_depth_get(Ecore_Win32_Window *window)
{
#ifdef HAVE_DIRECT3D
   return ((struct _Ecore_Win32_Window *)window)->glew.depth;
#else
   return 0;
#endif /* HAVE_DIRECT3D */
}
