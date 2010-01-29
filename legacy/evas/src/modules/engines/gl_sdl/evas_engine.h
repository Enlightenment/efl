#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#include "config.h"
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
# if defined(GLES_VARIETY_S3C6410)
#  include <EGL/egl.h>
#  include <GLES2/gl2.h>
# elif defined(GLES_VARIETY_SGX)
#  include <EGL/egl.h>
#  include <GLES2/gl2.h>
#  include <GLES2/gl2ext.h>
# endif
# ifdef HAVE_SDL_FLAG_OPENGLES
#  define EVAS_SDL_GL_FLAG SDL_OPENGLES
# else
#  define EVAS_SDL_GL_FLAG SDL_OPENGL /* This probably won't work? */
# endif
#else
# define EVAS_SDL_GL_FLAG SDL_OPENGL
#endif
#include <SDL/SDL.h>
#include <SDL/SDL_opengl.h>
#include <GL/gl.h>
#include "evas_common.h"
#include "evas_private.h"
#include "evas_gl_common.h"
#include "Evas.h"
#include "Evas_Engine_GL_SDL.h"

extern int _evas_engine_GL_SDL_log_dom ;
#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_GL_SDL_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_GL_SDL_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_engine_GL_SDL_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_GL_SDL_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_evas_engine_GL_SDL_log_dom, __VA_ARGS__)

typedef struct _Render_Engine Render_Engine;
struct _Render_Engine
{
   Evas_Engine_Info_GL_SDL	*info;
   int			w, h;

   Evas_GL_Context *gl_context;
   struct {
      int              redraw : 1;
      int              drew : 1;
      int              x1, y1, x2, y2;
   } draw;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
   EGLContext       egl_context[1];
   EGLSurface       egl_surface[1];
   EGLConfig        egl_config;
   EGLDisplay       egl_disp;
#endif

};

#endif
