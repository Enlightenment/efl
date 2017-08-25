#ifndef EVAS_ENGINE_H
#define EVAS_ENGINE_H

#define _EVAS_ENGINE_SDL_H

#include "config.h"
#include <SDL2/SDL.h>
#ifdef GL_GLES
# include <SDL2/SDL_opengles.h>
# ifdef HAVE_SDL_FLAG_OPENGLES
#  define EVAS_SDL_GL_FLAG SDL_OPENGLES
# else
#  define EVAS_SDL_GL_FLAG SDL_OPENGL /* This probably won't work? */
# endif
#else
# include <SDL2/SDL_opengl.h>
# define EVAS_SDL_GL_FLAG SDL_OPENGL
#endif
#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_gl_common.h"
#include "Evas.h"
#include "Evas_Engine_GL_SDL.h"

#include "../gl_generic/Evas_Engine_GL_Generic.h"

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

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_GL_SDL_log_dom, __VA_ARGS__)

typedef struct _Render_Engine Render_Engine;

struct _Outbuf
{
   Evas_Engine_Info_GL_SDL *info;
   SDL_Window *window;
   SDL_GLContext *context;

   Evas_Engine_GL_Context *gl_context;
   struct {
      int              redraw : 1;
      int              drew : 1;
      int              x1, y1, x2, y2;
   } draw;
#ifdef GL_GLES
   EGLContext       egl_context[1];
   EGLSurface       egl_surface[1];
   EGLConfig        egl_config;
   EGLDisplay       egl_disp;
#endif

   int w, h;
};

struct _Render_Engine
{
   Render_Output_GL_Generic generic;
};

#endif
