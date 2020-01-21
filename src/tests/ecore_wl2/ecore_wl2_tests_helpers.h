#ifndef ECORE_WL2_TEST_HELPERS_H
# define ECORE_WL2_TEST_HELPERS_H

#include <wayland-egl.h>

#ifdef GL_GLES
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#endif

#define WIDTH 480
#define HEIGHT 360

typedef struct _Test_Data
{
   Ecore_Wl2_Display *display;
   Ecore_Wl2_Window *win;
   Ecore_Wl2_Frame_Cb_Handle *frame_callback_handler;
   Ecore_Event_Handler *handler;

   struct wl_surface *surface;
   struct wl_egl_window *egl_window;

   int width;
   int height;
   int frame_callback_count;

#ifdef GL_GLES
   EGLDisplay egl_display;
   EGLConfig egl_conf;
   EGLSurface egl_surface;
   EGLContext egl_context;
#endif
} Test_Data;

static Ecore_Wl2_Display *
_display_connect(void)
{
   return ecore_wl2_display_connect(NULL);
}

static Ecore_Wl2_Window *
_window_create(Ecore_Wl2_Display *disp)
{
   return ecore_wl2_window_new(disp, NULL, 100, 100, WIDTH, HEIGHT);
}

#endif
