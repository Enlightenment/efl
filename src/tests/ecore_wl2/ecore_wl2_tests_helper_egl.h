#ifndef ECORE_WL2_TEST_HELPER_EGL_H
# define ECORE_WL2_TEST_HELPER_EGL_H

#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

static void
_init_egl(Test_Data *td)
{
   eglBindAPI(EGL_OPENGL_API);
   EGLint num_config;

   EGLint attributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_NONE
   };

   td->egl_display = eglGetDisplay((EGLNativeDisplayType)ecore_wl2_display_get(td->display));
   eglInitialize(td->egl_display, NULL, NULL);
   eglChooseConfig(td->egl_display, attributes, &td->egl_conf, 1, &num_config);
   td->egl_context = eglCreateContext(td->egl_display, td->egl_conf, EGL_NO_CONTEXT, NULL);

   td->egl_window = wl_egl_window_create(td->surface, td->width, td->height);
   td->egl_surface = eglCreateWindowSurface(td->egl_display,
                                            td->egl_conf, td->egl_window, NULL);

   eglMakeCurrent(td->egl_display, td->egl_surface, td->egl_surface, td->egl_context);
}

static void
_term_egl(Test_Data *td)
{
   eglDestroySurface(td->egl_display, td->egl_surface);
   wl_egl_window_destroy(td->egl_window);
   eglDestroyContext(td->egl_display, td->egl_context);
   eglTerminate(td->egl_display);
}

#endif
