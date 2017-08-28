#define GL_ERRORS_NODEF 1
#include "evas_gl_core_private.h"
#include "evas_gl_api_ext.h"

#ifndef _WIN32
# include <dlfcn.h>
#endif

#define EVGL_FUNC_BEGIN() if (UNLIKELY(_need_context_restore)) _context_restore()

#define EVGLD_FUNC_BEGIN() \
{ \
   _func_begin_debug(__FUNCTION__); \
}

#define EVGLD_FUNC_END() GLERRV(__FUNCTION__)

static void *_gles1_handle = NULL;
static Evas_GL_API _gles1_api;

void
compute_gl_coordinates(int win_w, int win_h, int rot, int clip_image,
                       int x, int y, int width, int height,
                       int img_x, int img_y, int img_w, int img_h,
                       int clip_x, int clip_y, int clip_w, int clip_h,
                       int imgc[4], int objc[4], int cc[4]);

//---------------------------------------//
// API Debug Error Checking Code

static
void _make_current_check(const char* api)
{
   EVGL_Context *ctx = NULL;

   ctx = evas_gl_common_current_context_get();

   if (!ctx)
     CRI("\e[1;33m%s\e[m: Current Context NOT SET: GL Call Should NOT Be Called without MakeCurrent!!!", api);
   else if (ctx->version != EVAS_GL_GLES_1_X)
     CRI("\e[1;33m%s\e[m: This API is being called with the wrong context (invalid version).", api);
}

static
void _direct_rendering_check(const char *api)
{
   EVGL_Context *ctx = NULL;

   ctx = evas_gl_common_current_context_get();
   if (!ctx)
     {
        ERR("Current Context Not Set");
        return;
     }

   if (_evgl_not_in_pixel_get())
     CRI("\e[1;33m%s\e[m: This API is being called outside Pixel Get Callback Function.", api);
   else if (ctx->version != EVAS_GL_GLES_1_X)
     CRI("\e[1;33m%s\e[m: This API is being called with the wrong context (invalid version).", api);
}

static
void _func_begin_debug(const char *api)
{
   _make_current_check(api);
   _direct_rendering_check(api);
}

static void
_evgl_gles1_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   EVGL_Resource *rsc;

   if (!_gles1_api.glClearColor)
     return;

   if (!(rsc=_evgl_tls_resource_get()))
     {
        ERR("Unable to execute GL command. Error retrieving tls");
        return;
     }

   if (_evgl_direct_enabled())
     {
        rsc->clear_color.a = alpha;
        rsc->clear_color.r = red;
        rsc->clear_color.g = green;
        rsc->clear_color.b = blue;
     }
   EVGL_FUNC_BEGIN();
   EVGL_TH_CALL(glClearColor, _gles1_api.glClearColor, red, green, blue, alpha);
}

static void
_evgl_gles1_glClear(GLbitfield mask)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};
   int cc[4] = {0,0,0,0};

   if (!_gles1_api.glClear)
     return;

   if (!(rsc=_evgl_tls_resource_get()))
     {
        ERR("Unable to execute GL command. Error retrieving tls");
        return;
     }

   if (!rsc->current_eng)
     {
        ERR("Unable to retrive Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrive Current Context");
        return;
     }

   if (ctx->version != EVAS_GL_GLES_1_X)
     {
        ERR("Invalid context version %d", (int) ctx->version);
        return;
     }

   EVGL_FUNC_BEGIN();
   if (_evgl_direct_enabled())
     {
        if (!(rsc->current_ctx->current_fbo))
          //|| rsc->current_ctx->map_tex)
          {
             /* Skip glClear() if clearing with transparent color
              * Note: There will be side effects if the object itself is not
              * marked as having an alpha channel!
              */
             if (ctx->current_sfc->alpha && (mask & GL_COLOR_BUFFER_BIT))
               {
                  if ((rsc->clear_color.a == 0) &&
                      (rsc->clear_color.r == 0) &&
                      (rsc->clear_color.g == 0) &&
                      (rsc->clear_color.b == 0))
                    {
                       // Skip clear color as we don't want to write black
                       mask &= ~GL_COLOR_BUFFER_BIT;
                    }
                  else if (rsc->clear_color.a != 1.0)
                    {
                       // TODO: Draw a rectangle? This will never be the perfect solution though.
                       WRN("glClear() used with a semi-transparent color and direct rendering. "
                           "This will erase the previous contents of the evas!");
                    }
                  if (!mask) return;
               }

             if ((!ctx->direct_scissor))
               {
                  EVGL_TH_CALL(glEnable, _gles1_api.glEnable, GL_SCISSOR_TEST);
                  ctx->direct_scissor = 1;
               }

             if ((ctx->scissor_updated) && (ctx->scissor_enabled))
               {
                  compute_gl_coordinates(rsc->direct.win_w, rsc->direct.win_h,
                                         rsc->direct.rot, 1,
                                         ctx->scissor_coord[0], ctx->scissor_coord[1],
                                         ctx->scissor_coord[2], ctx->scissor_coord[3],
                                         rsc->direct.img.x, rsc->direct.img.y,
                                         rsc->direct.img.w, rsc->direct.img.h,
                                         rsc->direct.clip.x, rsc->direct.clip.y,
                                         rsc->direct.clip.w, rsc->direct.clip.h,
                                         oc, nc, cc);

                  RECTS_CLIP_TO_RECT(nc[0], nc[1], nc[2], nc[3], cc[0], cc[1], cc[2], cc[3]);
                  EVGL_TH_CALL(glScissor, _gles1_api.glScissor, nc[0], nc[1], nc[2], nc[3]);
                  ctx->direct_scissor = 0;
               }
             else
               {
                  compute_gl_coordinates(rsc->direct.win_w, rsc->direct.win_h,
                                         rsc->direct.rot, 0,
                                         0, 0, 0, 0,
                                         rsc->direct.img.x, rsc->direct.img.y,
                                         rsc->direct.img.w, rsc->direct.img.h,
                                         rsc->direct.clip.x, rsc->direct.clip.y,
                                         rsc->direct.clip.w, rsc->direct.clip.h,
                                         oc, nc, cc);

                  EVGL_TH_CALL(glScissor, _gles1_api.glScissor, cc[0], cc[1], cc[2], cc[3]);
               }

             EVGL_TH_CALL(glClear, _gles1_api.glClear, mask);

             // TODO/FIXME: Restore previous client-side scissors.
          }
        else
          {
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  EVGL_TH_CALL(glDisable, _gles1_api.glDisable, GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             EVGL_TH_CALL(glClear, _gles1_api.glClear, mask);
          }
     }
   else
     {
        if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
          {
             EVGL_TH_CALL(glDisable, _gles1_api.glDisable, GL_SCISSOR_TEST);
             ctx->direct_scissor = 0;
          }

        EVGL_TH_CALL(glClear, _gles1_api.glClear, mask);
     }
}

static void
_evgl_gles1_glDisable(GLenum cap)
{
   EVGL_Context *ctx;

   if (!_gles1_api.glDisable)
     return;

   ctx = evas_gl_common_current_context_get();
   if (!ctx)
     {
        ERR("Unable to retrive Current Context");
        return;
     }

   if (ctx->version != EVAS_GL_GLES_1_X)
     {
        ERR("Invalid context version %d", (int) ctx->version);
        return;
     }

   if (cap == GL_SCISSOR_TEST)
      ctx->scissor_enabled = 0;
   EVGL_FUNC_BEGIN();
   EVGL_TH_CALL(glDisable, _gles1_api.glDisable, cap);
}

static void
_evgl_gles1_glEnable(GLenum cap)
{
   EVGL_Context *ctx;

   if (!_gles1_api.glEnable)
     return;

   ctx = evas_gl_common_current_context_get();
   if (!ctx)
     {
        ERR("Unable to retrive Current Context");
        return;
     }

   if (ctx->version != EVAS_GL_GLES_1_X)
     {
        ERR("Invalid context version %d", (int) ctx->version);
        return;
     }

   if (cap == GL_SCISSOR_TEST)
      ctx->scissor_enabled = 1;
   EVGL_FUNC_BEGIN();
   EVGL_TH_CALL(glEnable, _gles1_api.glEnable, cap);
}

static GLenum
_evgl_gles1_glGetError(void)
{
   GLenum ret;
   if (!_gles1_api.glGetError)
     return EVAS_GL_NOT_INITIALIZED;
   EVGL_FUNC_BEGIN();
   ret = EVGL_TH_CALL(glGetError, _gles1_api.glGetError);
   return ret;
}

static void
_evgl_gles1_glGetIntegerv(GLenum pname, GLint *params)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;

   if (!_gles1_api.glGetIntegerv)
     return;

   if (_evgl_direct_enabled())
     {
        if (!params)
          {
             ERR("Inavlid Parameter");
             return;
          }

        if (!(rsc=_evgl_tls_resource_get()))
          {
             ERR("Unable to execute GL command. Error retrieving tls");
             return;
          }

        ctx = rsc->current_ctx;
        if (!ctx)
          {
             ERR("Unable to retrive Current Context");
             return;
          }

        if (ctx->version != EVAS_GL_GLES_1_X)
          {
             ERR("Invalid context version %d", (int) ctx->version);
             return;
          }

        // Only need to handle it if it's directly rendering to the window
        if (!(rsc->current_ctx->current_fbo))
          //|| rsc->current_ctx->map_tex)
          {
             if (pname == GL_SCISSOR_BOX)
               {
                  if (ctx->scissor_updated)
                    {
                       memcpy(params, ctx->scissor_coord, sizeof(int)*4);
                       return;
                    }
               }
             /*!!! Temporary Fixes to avoid Webkit issue
             if (pname == GL_VIEWPORT)
               {
                  if (ctx->viewport_updated)
                    {
                       memcpy(params, ctx->viewport_coord, sizeof(int)*4);
                       return;
                    }
               }
               */

             // If it hasn't been initialized yet, return img object size
             if (pname == GL_SCISSOR_BOX) //|| (pname == GL_VIEWPORT))
               {
                  params[0] = 0;
                  params[1] = 0;
                  params[2] = (GLint)rsc->direct.img.w;
                  params[3] = (GLint)rsc->direct.img.h;
                  return;
               }
          }
     }

   EVGL_FUNC_BEGIN();
   EVGL_TH_CALL(glGetIntegerv, _gles1_api.glGetIntegerv, pname, params);
}

static const GLubyte *
_evgl_gles1_glGetString(GLenum name)
{
   static char _version[128] = {0};
   EVGL_Resource *rsc;
   const GLubyte *ret;

   if (!_gles1_api.glGetString)
     return NULL;

   if ((!(rsc = _evgl_tls_resource_get())) || !rsc->current_ctx)
     {
        ERR("Current context is NULL, not calling glGetString");
        // This sets evas_gl_error_get instead of glGetError...
        evas_gl_common_error_set(EVAS_GL_BAD_CONTEXT);
        return NULL;
     }

   if (rsc->current_ctx->version != EVAS_GL_GLES_1_X)
     {
        ERR("Invalid context version %d", (int) rsc->current_ctx->version);
        evas_gl_common_error_set(EVAS_GL_BAD_MATCH);
        return NULL;
     }

   switch (name)
     {
      case GL_VENDOR:
      case GL_RENDERER:
      case GL_SHADING_LANGUAGE_VERSION:
        break;
      case GL_VERSION:
        ret = EVGL_TH_CALL(glGetString, NULL, GL_VERSION);
        if (!ret) return NULL;
#ifdef GL_GLES
        if (ret[13] != (GLubyte) '1')
          {
             // We try not to remove the vendor fluff
             snprintf(_version, sizeof(_version), "OpenGL ES-CM 1.1 Evas GL (%s)", ((char *) ret) + 10);
             _version[sizeof(_version) - 1] = '\0';
             return (const GLubyte *) _version;
          }
        return ret;
#else
        // Desktop GL, we still keep the official name
        snprintf(_version, sizeof(_version), "OpenGL ES-CM 1.1 Evas GL (%s)", (char *) ret);
        _version[sizeof(_version) - 1] = '\0';
        return (const GLubyte *) _version;
#endif

      case GL_EXTENSIONS:
        return (GLubyte *) evgl_api_ext_string_get(EINA_TRUE, EVAS_GL_GLES_1_X);

      default:
        WRN("Unknown string requested: %x", (unsigned int) name);
        break;
     }

   EVGL_FUNC_BEGIN();
   return EVGL_TH_CALL(glGetString, _gles1_api.glGetString, name);
}

static void
_evgl_gles1_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};
   int cc[4] = {0,0,0,0};

   if (!_gles1_api.glReadPixels)
     return;

   if (!(rsc=_evgl_tls_resource_get()))
     {
        ERR("Unable to execute GL command. Error retrieving tls");
        return;
     }

   if (!rsc->current_eng)
     {
        ERR("Unable to retrive Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrive Current Context");
        return;
     }

   if (ctx->version != EVAS_GL_GLES_1_X)
     {
        ERR("Invalid context version %d", (int) ctx->version);
        return;
     }

   EVGL_FUNC_BEGIN();
   if (_evgl_direct_enabled())
     {
        if (!(rsc->current_ctx->current_fbo))
          //|| rsc->current_ctx->map_tex)
          {
             compute_gl_coordinates(rsc->direct.win_w, rsc->direct.win_h,
                                    rsc->direct.rot, 1,
                                    x, y, width, height,
                                    rsc->direct.img.x, rsc->direct.img.y,
                                    rsc->direct.img.w, rsc->direct.img.h,
                                    rsc->direct.clip.x, rsc->direct.clip.y,
                                    rsc->direct.clip.w, rsc->direct.clip.h,
                                    oc, nc, cc);
             EVGL_TH_CALL(glReadPixels, _gles1_api.glReadPixels, nc[0], nc[1], nc[2], nc[3], format, type, pixels);
          }
        else
          {
             EVGL_TH_CALL(glReadPixels, _gles1_api.glReadPixels, x, y, width, height, format, type, pixels);
          }
     }
   else
     {
        EVGL_TH_CALL(glReadPixels, _gles1_api.glReadPixels, x, y, width, height, format, type, pixels);
     }
}

static void
_evgl_gles1_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};
   int cc[4] = {0,0,0,0};

   if (!_gles1_api.glScissor)
     return;

   if (!(rsc=_evgl_tls_resource_get()))
     {
        ERR("Unable to execute GL command. Error retrieving tls");
        return;
     }

   if (!rsc->current_eng)
     {
        ERR("Unable to retrive Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrive Current Context");
        return;
     }

   if (ctx->version != EVAS_GL_GLES_1_X)
     {
        ERR("Invalid context version %d", (int) ctx->version);
        return;
     }

   if (_evgl_direct_enabled())
     {
        if (!(rsc->current_ctx->current_fbo))
          //|| rsc->current_ctx->map_tex)
          {
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  EVGL_TH_CALL(glDisable, _gles1_api.glDisable, GL_SCISSOR_TEST);
               }

             compute_gl_coordinates(rsc->direct.win_w, rsc->direct.win_h,
                                    rsc->direct.rot, 1,
                                    x, y, width, height,
                                    rsc->direct.img.x, rsc->direct.img.y,
                                    rsc->direct.img.w, rsc->direct.img.h,
                                    rsc->direct.clip.x, rsc->direct.clip.y,
                                    rsc->direct.clip.w, rsc->direct.clip.h,
                                    oc, nc, cc);

             // Keep a copy of the original coordinates
             ctx->scissor_coord[0] = x;
             ctx->scissor_coord[1] = y;
             ctx->scissor_coord[2] = width;
             ctx->scissor_coord[3] = height;

             RECTS_CLIP_TO_RECT(nc[0], nc[1], nc[2], nc[3], cc[0], cc[1], cc[2], cc[3]);
             EVGL_TH_CALL(glScissor, _gles1_api.glScissor, nc[0], nc[1], nc[2], nc[3]);

             ctx->direct_scissor = 0;

             // Check....!!!!
             ctx->scissor_updated = 1;
          }
        else
          {
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  EVGL_TH_CALL(glDisable, _gles1_api.glDisable, GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             EVGL_TH_CALL(glScissor, _gles1_api.glScissor, x, y, width, height);

             ctx->scissor_updated = 0;
          }
     }
   else
     {
        if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
          {
             EVGL_TH_CALL(glDisable, _gles1_api.glDisable, GL_SCISSOR_TEST);
             ctx->direct_scissor = 0;
          }

        EVGL_TH_CALL(glScissor, _gles1_api.glScissor, x, y, width, height);
     }
}

static void
_evgl_gles1_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};
   int cc[4] = {0,0,0,0};

   if (!_gles1_api.glViewport)
     return;

   if (!(rsc=_evgl_tls_resource_get()))
     {
        ERR("Unable to execute GL command. Error retrieving tls");
        return;
     }

   if (!rsc->current_eng)
     {
        ERR("Unable to retrive Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrive Current Context");
        return;
     }

   if (ctx->version != EVAS_GL_GLES_1_X)
     {
        ERR("Invalid context version %d", (int) ctx->version);
        return;
     }

   EVGL_FUNC_BEGIN();
   if (_evgl_direct_enabled())
     {
        if (!(rsc->current_ctx->current_fbo))
          //|| rsc->current_ctx->map_tex)
          {
             if ((!ctx->direct_scissor))
               {
                  EVGL_TH_CALL(glEnable, _gles1_api.glEnable, GL_SCISSOR_TEST);
                  ctx->direct_scissor = 1;
               }

             if ((ctx->scissor_updated) && (ctx->scissor_enabled))
               {
                  // Recompute the scissor coordinates
                  compute_gl_coordinates(rsc->direct.win_w, rsc->direct.win_h,
                                         rsc->direct.rot, 1,
                                         ctx->scissor_coord[0], ctx->scissor_coord[1],
                                         ctx->scissor_coord[2], ctx->scissor_coord[3],
                                         rsc->direct.img.x, rsc->direct.img.y,
                                         rsc->direct.img.w, rsc->direct.img.h,
                                         rsc->direct.clip.x, rsc->direct.clip.y,
                                         rsc->direct.clip.w, rsc->direct.clip.h,
                                         oc, nc, cc);

                  RECTS_CLIP_TO_RECT(nc[0], nc[1], nc[2], nc[3], cc[0], cc[1], cc[2], cc[3]);
                  EVGL_TH_CALL(glScissor, _gles1_api.glScissor, nc[0], nc[1], nc[2], nc[3]);

                  ctx->direct_scissor = 0;

                  // Compute the viewport coordinate
                  compute_gl_coordinates(rsc->direct.win_w, rsc->direct.win_h,
                                         rsc->direct.rot, 0,
                                         x, y, width, height,
                                         rsc->direct.img.x, rsc->direct.img.y,
                                         rsc->direct.img.w, rsc->direct.img.h,
                                         rsc->direct.clip.x, rsc->direct.clip.y,
                                         rsc->direct.clip.w, rsc->direct.clip.h,
                                         oc, nc, cc);
                  EVGL_TH_CALL(glViewport, _gles1_api.glViewport, nc[0], nc[1], nc[2], nc[3]);
               }
             else
               {

                  compute_gl_coordinates(rsc->direct.win_w, rsc->direct.win_h,
                                         rsc->direct.rot, 0,
                                         x, y, width, height,
                                         rsc->direct.img.x, rsc->direct.img.y,
                                         rsc->direct.img.w, rsc->direct.img.h,
                                         rsc->direct.clip.x, rsc->direct.clip.y,
                                         rsc->direct.clip.w, rsc->direct.clip.h,
                                         oc, nc, cc);
                  EVGL_TH_CALL(glScissor, _gles1_api.glScissor, cc[0], cc[1], cc[2], cc[3]);

                  EVGL_TH_CALL(glViewport, _gles1_api.glViewport, nc[0], nc[1], nc[2], nc[3]);
               }

             ctx->viewport_direct[0] = nc[0];
             ctx->viewport_direct[1] = nc[1];
             ctx->viewport_direct[2] = nc[2];
             ctx->viewport_direct[3] = nc[3];

             // Keep a copy of the original coordinates
             ctx->viewport_coord[0] = x;
             ctx->viewport_coord[1] = y;
             ctx->viewport_coord[2] = width;
             ctx->viewport_coord[3] = height;

             ctx->viewport_updated   = 1;
          }
        else
          {
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  EVGL_TH_CALL(glDisable, _gles1_api.glDisable, GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             EVGL_TH_CALL(glViewport, _gles1_api.glViewport, x, y, width, height);
          }
     }
   else
     {
        if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
          {
             EVGL_TH_CALL(glDisable, _gles1_api.glDisable, GL_SCISSOR_TEST);
             ctx->direct_scissor = 0;
          }

        EVGL_TH_CALL(glViewport, _gles1_api.glViewport, x, y, width, height);
     }
}

//-------------------------------------------------------------//
// Open GLES 1.0 APIs
#define _EVASGL_FUNCTION_PRIVATE_BEGIN(ret, name, ...) \
static ret evgl_gles1_##name (_EVASGL_PARAM_PROTO(__VA_ARGS__)) { \
   if (!_gles1_api.name) return (ret)0; \
   EVGL_FUNC_BEGIN(); \
   return _gles1_api.name (_EVASGL_PARAM_NAME(__VA_ARGS__)); \
}

#define _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID(name, ...) \
static void evgl_gles1_##name (_EVASGL_PARAM_PROTO(__VA_ARGS__)) { \
   if (!_gles1_api.name) return; \
   EVGL_FUNC_BEGIN(); \
   _gles1_api.name (_EVASGL_PARAM_NAME(__VA_ARGS__)); \
}

#define _EVASGL_FUNCTION_BEGIN(ret, name, ...) \
static ret evgl_gles1_##name (_EVASGL_PARAM_PROTO(__VA_ARGS__)) { \
   if (!_gles1_api.name) return (ret)0; \
   EVGL_FUNC_BEGIN(); \
   return EVGL_TH_CALL(name, _EVASGL_PARAM_NAME(void, _gles1_api.name, __VA_ARGS__)); \
}

#define _EVASGL_FUNCTION_BEGIN_VOID(name, ...) \
static void evgl_gles1_##name (_EVASGL_PARAM_PROTO(__VA_ARGS__)) { \
   if (!_gles1_api.name) return; \
   EVGL_FUNC_BEGIN(); \
   EVGL_TH_CALL(name, _EVASGL_PARAM_NAME(void, _gles1_api.name, __VA_ARGS__)); \
}

#include "evas_gl_api_gles1_def.h"

#undef _EVASGL_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID
#undef _EVASGL_FUNCTION_BEGIN
#undef _EVASGL_FUNCTION_BEGIN_VOID


//-------------------------------------------------------------//
// Open GLES 1.0 APIs DEBUG
#define _EVASGL_FUNCTION_PRIVATE_BEGIN(ret, name, ...) \
static ret _evgld_gles1_##name (_EVASGL_PARAM_PROTO(__VA_ARGS__)) { \
   EVGLD_FUNC_BEGIN(); \
   ret _a = _evgl_gles1_##name (_EVASGL_PARAM_NAME(__VA_ARGS__)); \
   EVGLD_FUNC_END(); \
   return _a; \
}

#define _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID(name, ...) \
static void _evgld_gles1_##name (_EVASGL_PARAM_PROTO(__VA_ARGS__)) { \
   EVGLD_FUNC_BEGIN(); \
   _evgl_gles1_##name (_EVASGL_PARAM_NAME(__VA_ARGS__)); \
   EVGLD_FUNC_END(); \
}

#define _EVASGL_FUNCTION_BEGIN(ret, name, ...) \
static ret _evgld_gles1_##name (_EVASGL_PARAM_PROTO(__VA_ARGS__)) { \
   EVGLD_FUNC_BEGIN(); \
   ret _a; \
   _a = EVGL_TH_CALL(name, _EVASGL_PARAM_NAME(void, _gles1_api.name, __VA_ARGS__)); \
   EVGLD_FUNC_END(); \
   return _a; \
}

#define _EVASGL_FUNCTION_BEGIN_VOID(name, ...) \
static void _evgld_gles1_##name (_EVASGL_PARAM_PROTO(__VA_ARGS__)) { \
   EVGLD_FUNC_BEGIN(); \
   EVGL_TH_CALL(name, _EVASGL_PARAM_NAME(void, _gles1_api.name, __VA_ARGS__)); \
   EVGLD_FUNC_END(); \
}

#include "evas_gl_api_gles1_def.h"

#undef _EVASGL_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID
#undef _EVASGL_FUNCTION_BEGIN
#undef _EVASGL_FUNCTION_BEGIN_VOID


static void
_evgl_load_gles1_apis(void *dl_handle, Evas_GL_API *funcs)
{
   if (!dl_handle) return;

#define ORD(name) funcs->name = dlsym(dl_handle, #name)
   /* Available only in Common profile */
   ORD(glAlphaFunc);
   ORD(glClearColor);
   ORD(glClearDepthf);
   ORD(glClipPlanef);
   ORD(glColor4f);
   ORD(glDepthRangef);
   ORD(glFogf);
   ORD(glFogfv);
   ORD(glFrustumf);
   ORD(glGetClipPlanef);
   ORD(glGetFloatv);
   ORD(glGetLightfv);
   ORD(glGetMaterialfv);
   ORD(glGetTexEnvfv);
   ORD(glGetTexParameterfv);
   ORD(glLightModelf);
   ORD(glLightModelfv);
   ORD(glLightf);
   ORD(glLightfv);
   ORD(glLineWidth);
   ORD(glLoadMatrixf);
   ORD(glMaterialf);
   ORD(glMaterialfv);
   ORD(glMultMatrixf);
   ORD(glMultiTexCoord4f);
   ORD(glNormal3f);
   ORD(glOrthof);
   ORD(glPointParameterf);
   ORD(glPointParameterfv);
   ORD(glPointSize);
   ORD(glPointSizePointerOES);
   ORD(glPolygonOffset);
   ORD(glRotatef);
   ORD(glScalef);
   ORD(glTexEnvf);
   ORD(glTexEnvfv);
   ORD(glTexParameterf);
   ORD(glTexParameterfv);
   ORD(glTranslatef);
   /* Available in both Common and Common-Lite profiles */
   ORD(glActiveTexture);
   ORD(glAlphaFuncx);
   ORD(glBindBuffer);
   ORD(glBindTexture);
   ORD(glBlendFunc);
   ORD(glBufferData);
   ORD(glBufferSubData);
   ORD(glClear);
   ORD(glClearColorx);
   ORD(glClearDepthx);
   ORD(glClearStencil);
   ORD(glClientActiveTexture);
   ORD(glClipPlanex);
   ORD(glColor4ub);
   ORD(glColor4x);
   ORD(glColorMask);
   ORD(glColorPointer);
   ORD(glCompressedTexImage2D);
   ORD(glCompressedTexSubImage2D);
   ORD(glCopyTexImage2D);
   ORD(glCopyTexSubImage2D);
   ORD(glCullFace);
   ORD(glDeleteBuffers);
   ORD(glDeleteTextures);
   ORD(glDepthFunc);
   ORD(glDepthMask);
   ORD(glDepthRangex);
   ORD(glDisable);
   ORD(glDisableClientState);
   ORD(glDrawArrays);
   ORD(glDrawElements);
   ORD(glEnable);
   ORD(glEnableClientState);
   ORD(glFinish);
   ORD(glFlush);
   ORD(glFogx);
   ORD(glFogxv);
   ORD(glFrontFace);
   ORD(glFrustumx);
   ORD(glGetBooleanv);
   ORD(glGetBufferParameteriv);
   ORD(glGetClipPlanex);
   ORD(glGenBuffers);
   ORD(glGenTextures);
   ORD(glGetError);
   ORD(glGetFixedv);
   ORD(glGetIntegerv);
   ORD(glGetLightxv);
   ORD(glGetMaterialxv);
   ORD(glGetPointerv);
   ORD(glGetString);
   ORD(glGetTexEnviv);
   ORD(glGetTexEnvxv);
   ORD(glGetTexParameteriv);
   ORD(glGetTexParameterxv);
   ORD(glHint);
   ORD(glIsBuffer);
   ORD(glIsEnabled);
   ORD(glIsTexture);
   ORD(glLightModelx);
   ORD(glLightModelxv);
   ORD(glLightx);
   ORD(glLightxv);
   ORD(glLineWidthx);
   ORD(glLoadIdentity);
   ORD(glLoadMatrixx);
   ORD(glLogicOp);
   ORD(glMaterialx);
   ORD(glMaterialxv);
   ORD(glMatrixMode);
   ORD(glMultMatrixx);
   ORD(glMultiTexCoord4x);
   ORD(glNormal3x);
   ORD(glNormalPointer);
   ORD(glOrthox);
   ORD(glPixelStorei);
   ORD(glPointParameterx);
   ORD(glPointParameterxv);
   ORD(glPointSizex);
   ORD(glPolygonOffsetx);
   ORD(glPopMatrix);
   ORD(glPushMatrix);
   ORD(glReadPixels);
   ORD(glRotatex);
   ORD(glSampleCoverage);
   ORD(glSampleCoveragex);
   ORD(glScalex);
   ORD(glScissor);
   ORD(glShadeModel);
   ORD(glStencilFunc);
   ORD(glStencilMask);
   ORD(glStencilOp);
   ORD(glTexCoordPointer);
   ORD(glTexEnvi);
   ORD(glTexEnvx);
   ORD(glTexEnviv);
   ORD(glTexEnvxv);
   ORD(glTexImage2D);
   ORD(glTexParameteri);
   ORD(glTexParameterx);
   ORD(glTexParameteriv);
   ORD(glTexParameterxv);
   ORD(glTexSubImage2D);
   ORD(glTranslatex);
   ORD(glVertexPointer);
   ORD(glViewport);
#undef ORD
}

static Eina_Bool
_evgl_gles1_api_init(void)
{
   static Eina_Bool _initialized = EINA_FALSE;
   if (_initialized) return EINA_TRUE;

   memset(&_gles1_api, 0, sizeof(_gles1_api));

#ifdef GL_GLES
   _gles1_handle = dlopen("libGLES_CM.so", RTLD_NOW);
   if (!_gles1_handle) _gles1_handle = dlopen("libGLES_CM.so.1", RTLD_NOW);
   if (!_gles1_handle) _gles1_handle = dlopen("libGLES_CM.so.1.1", RTLD_NOW);
   if (!_gles1_handle) _gles1_handle = dlopen("libGLESv1_CM.so", RTLD_NOW);
   if (!_gles1_handle) _gles1_handle = dlopen("libGLESv1_CM.so.1", RTLD_NOW);
#else
   _gles1_handle = dlopen("libGL.so", RTLD_NOW);
   if (!_gles1_handle) _gles1_handle = dlopen("libGL.so.4", RTLD_NOW);
   if (!_gles1_handle) _gles1_handle = dlopen("libGL.so.3", RTLD_NOW);
   if (!_gles1_handle) _gles1_handle = dlopen("libGL.so.2", RTLD_NOW);
   if (!_gles1_handle) _gles1_handle = dlopen("libGL.so.1", RTLD_NOW);
   if (!_gles1_handle) _gles1_handle = dlopen("libGL.so.0", RTLD_NOW);
#endif

   if (!_gles1_handle)
     {
        WRN("OpenGL ES 1 was not found on this system. Evas GL will not support GLES 1 contexts.");
        return EINA_FALSE;
     }

   _evgl_load_gles1_apis(_gles1_handle, &_gles1_api);

   _initialized = EINA_TRUE;
   return EINA_TRUE;
}

static void
_debug_gles1_api_get(Evas_GL_API *funcs)
{
   if (!funcs) return;
   funcs->version = EVAS_GL_API_VERSION;

#define ORD(name) EVAS_API_OVERRIDE(name, funcs, _evgld_gles1_)
   /* Available only in Common profile */
   ORD(glAlphaFunc);
   ORD(glClearColor);
   ORD(glClearDepthf);
   ORD(glClipPlanef);
   ORD(glColor4f);
   ORD(glDepthRangef);
   ORD(glFogf);
   ORD(glFogfv);
   ORD(glFrustumf);
   ORD(glGetClipPlanef);
   ORD(glGetFloatv);
   ORD(glGetLightfv);
   ORD(glGetMaterialfv);
   ORD(glGetTexEnvfv);
   ORD(glGetTexParameterfv);
   ORD(glLightModelf);
   ORD(glLightModelfv);
   ORD(glLightf);
   ORD(glLightfv);
   ORD(glLineWidth);
   ORD(glLoadMatrixf);
   ORD(glMaterialf);
   ORD(glMaterialfv);
   ORD(glMultMatrixf);
   ORD(glMultiTexCoord4f);
   ORD(glNormal3f);
   ORD(glOrthof);
   ORD(glPointParameterf);
   ORD(glPointParameterfv);
   ORD(glPointSize);
   ORD(glPointSizePointerOES);
   ORD(glPolygonOffset);
   ORD(glRotatef);
   ORD(glScalef);
   ORD(glTexEnvf);
   ORD(glTexEnvfv);
   ORD(glTexParameterf);
   ORD(glTexParameterfv);
   ORD(glTranslatef);
   /* Available in both Common and Common-Lite profiles */
   ORD(glActiveTexture);
   ORD(glAlphaFuncx);
   ORD(glBindBuffer);
   ORD(glBindTexture);
   ORD(glBlendFunc);
   ORD(glBufferData);
   ORD(glBufferSubData);
   ORD(glClear);
   ORD(glClearColorx);
   ORD(glClearDepthx);
   ORD(glClearStencil);
   ORD(glClientActiveTexture);
   ORD(glClipPlanex);
   ORD(glColor4ub);
   ORD(glColor4x);
   ORD(glColorMask);
   ORD(glColorPointer);
   ORD(glCompressedTexImage2D);
   ORD(glCompressedTexSubImage2D);
   ORD(glCopyTexImage2D);
   ORD(glCopyTexSubImage2D);
   ORD(glCullFace);
   ORD(glDeleteBuffers);
   ORD(glDeleteTextures);
   ORD(glDepthFunc);
   ORD(glDepthMask);
   ORD(glDepthRangex);
   ORD(glDisable);
   ORD(glDisableClientState);
   ORD(glDrawArrays);
   ORD(glDrawElements);
   ORD(glEnable);
   ORD(glEnableClientState);
   ORD(glFinish);
   ORD(glFlush);
   ORD(glFogx);
   ORD(glFogxv);
   ORD(glFrontFace);
   ORD(glFrustumx);
   ORD(glGetBooleanv);
   ORD(glGetBufferParameteriv);
   ORD(glGetClipPlanex);
   ORD(glGenBuffers);
   ORD(glGenTextures);
   ORD(glGetError);
   ORD(glGetFixedv);
   ORD(glGetIntegerv);
   ORD(glGetLightxv);
   ORD(glGetMaterialxv);
   ORD(glGetPointerv);
   ORD(glGetString);
   ORD(glGetTexEnviv);
   ORD(glGetTexEnvxv);
   ORD(glGetTexParameteriv);
   ORD(glGetTexParameterxv);
   ORD(glHint);
   ORD(glIsBuffer);
   ORD(glIsEnabled);
   ORD(glIsTexture);
   ORD(glLightModelx);
   ORD(glLightModelxv);
   ORD(glLightx);
   ORD(glLightxv);
   ORD(glLineWidthx);
   ORD(glLoadIdentity);
   ORD(glLoadMatrixx);
   ORD(glLogicOp);
   ORD(glMaterialx);
   ORD(glMaterialxv);
   ORD(glMatrixMode);
   ORD(glMultMatrixx);
   ORD(glMultiTexCoord4x);
   ORD(glNormal3x);
   ORD(glNormalPointer);
   ORD(glOrthox);
   ORD(glPixelStorei);
   ORD(glPointParameterx);
   ORD(glPointParameterxv);
   ORD(glPointSizex);
   ORD(glPolygonOffsetx);
   ORD(glPopMatrix);
   ORD(glPushMatrix);
   ORD(glReadPixels);
   ORD(glRotatex);
   ORD(glSampleCoverage);
   ORD(glSampleCoveragex);
   ORD(glScalex);
   ORD(glScissor);
   ORD(glShadeModel);
   ORD(glStencilFunc);
   ORD(glStencilMask);
   ORD(glStencilOp);
   ORD(glTexCoordPointer);
   ORD(glTexEnvi);
   ORD(glTexEnvx);
   ORD(glTexEnviv);
   ORD(glTexEnvxv);
   ORD(glTexImage2D);
   ORD(glTexParameteri);
   ORD(glTexParameterx);
   ORD(glTexParameteriv);
   ORD(glTexParameterxv);
   ORD(glTexSubImage2D);
   ORD(glTranslatex);
   ORD(glVertexPointer);
   ORD(glViewport);
#undef ORD
}

static void
_normal_gles1_api_get(Evas_GL_API *funcs)
{
   if (!funcs) return;
   funcs->version = EVAS_GL_API_VERSION;

#define ORD(name) EVAS_API_OVERRIDE(name, funcs, evgl_gles1_)
   /* Available only in Common profile */
   ORD(glAlphaFunc);
   ORD(glClearColor);
   ORD(glClearDepthf);
   ORD(glClipPlanef);
   ORD(glColor4f);
   ORD(glDepthRangef);
   ORD(glFogf);
   ORD(glFogfv);
   ORD(glFrustumf);
   ORD(glGetClipPlanef);
   ORD(glGetFloatv);
   ORD(glGetLightfv);
   ORD(glGetMaterialfv);
   ORD(glGetTexEnvfv);
   ORD(glGetTexParameterfv);
   ORD(glLightModelf);
   ORD(glLightModelfv);
   ORD(glLightf);
   ORD(glLightfv);
   ORD(glLineWidth);
   ORD(glLoadMatrixf);
   ORD(glMaterialf);
   ORD(glMaterialfv);
   ORD(glMultMatrixf);
   ORD(glMultiTexCoord4f);
   ORD(glNormal3f);
   ORD(glOrthof);
   ORD(glPointParameterf);
   ORD(glPointParameterfv);
   ORD(glPointSize);
   ORD(glPointSizePointerOES);
   ORD(glPolygonOffset);
   ORD(glRotatef);
   ORD(glScalef);
   ORD(glTexEnvf);
   ORD(glTexEnvfv);
   ORD(glTexParameterf);
   ORD(glTexParameterfv);
   ORD(glTranslatef);
   /* Available in both Common and Common-Lite profiles */
   ORD(glActiveTexture);
   ORD(glAlphaFuncx);
   ORD(glBindBuffer);
   ORD(glBindTexture);
   ORD(glBlendFunc);
   ORD(glBufferData);
   ORD(glBufferSubData);
   ORD(glClear);
   ORD(glClearColorx);
   ORD(glClearDepthx);
   ORD(glClearStencil);
   ORD(glClientActiveTexture);
   ORD(glClipPlanex);
   ORD(glColor4ub);
   ORD(glColor4x);
   ORD(glColorMask);
   ORD(glColorPointer);
   ORD(glCompressedTexImage2D);
   ORD(glCompressedTexSubImage2D);
   ORD(glCopyTexImage2D);
   ORD(glCopyTexSubImage2D);
   ORD(glCullFace);
   ORD(glDeleteBuffers);
   ORD(glDeleteTextures);
   ORD(glDepthFunc);
   ORD(glDepthMask);
   ORD(glDepthRangex);
   ORD(glDisable);
   ORD(glDisableClientState);
   ORD(glDrawArrays);
   ORD(glDrawElements);
   ORD(glEnable);
   ORD(glEnableClientState);
   ORD(glFinish);
   ORD(glFlush);
   ORD(glFogx);
   ORD(glFogxv);
   ORD(glFrontFace);
   ORD(glFrustumx);
   ORD(glGetBooleanv);
   ORD(glGetBufferParameteriv);
   ORD(glGetClipPlanex);
   ORD(glGenBuffers);
   ORD(glGenTextures);
   ORD(glGetError);
   ORD(glGetFixedv);
   ORD(glGetIntegerv);
   ORD(glGetLightxv);
   ORD(glGetMaterialxv);
   ORD(glGetPointerv);
   ORD(glGetString);
   ORD(glGetTexEnviv);
   ORD(glGetTexEnvxv);
   ORD(glGetTexParameteriv);
   ORD(glGetTexParameterxv);
   ORD(glHint);
   ORD(glIsBuffer);
   ORD(glIsEnabled);
   ORD(glIsTexture);
   ORD(glLightModelx);
   ORD(glLightModelxv);
   ORD(glLightx);
   ORD(glLightxv);
   ORD(glLineWidthx);
   ORD(glLoadIdentity);
   ORD(glLoadMatrixx);
   ORD(glLogicOp);
   ORD(glMaterialx);
   ORD(glMaterialxv);
   ORD(glMatrixMode);
   ORD(glMultMatrixx);
   ORD(glMultiTexCoord4x);
   ORD(glNormal3x);
   ORD(glNormalPointer);
   ORD(glOrthox);
   ORD(glPixelStorei);
   ORD(glPointParameterx);
   ORD(glPointParameterxv);
   ORD(glPointSizex);
   ORD(glPolygonOffsetx);
   ORD(glPopMatrix);
   ORD(glPushMatrix);
   ORD(glReadPixels);
   ORD(glRotatex);
   ORD(glSampleCoverage);
   ORD(glSampleCoveragex);
   ORD(glScalex);
   ORD(glScissor);
   ORD(glShadeModel);
   ORD(glStencilFunc);
   ORD(glStencilMask);
   ORD(glStencilOp);
   ORD(glTexCoordPointer);
   ORD(glTexEnvi);
   ORD(glTexEnvx);
   ORD(glTexEnviv);
   ORD(glTexEnvxv);
   ORD(glTexImage2D);
   ORD(glTexParameteri);
   ORD(glTexParameterx);
   ORD(glTexParameteriv);
   ORD(glTexParameterxv);
   ORD(glTexSubImage2D);
   ORD(glTranslatex);
   ORD(glVertexPointer);
   ORD(glViewport);
#undef ORD
}

void
_evgl_api_gles1_get(Evas_GL_API *funcs, Eina_Bool debug)
{
   if (!_evgl_gles1_api_init())
     return;

   if (debug)
     _debug_gles1_api_get(funcs);
   else
     _normal_gles1_api_get(funcs);

   // FIXME: This looks wrong, we should be calling the gles1 API
   // TODO: Implement these wrappers first
   //if (evgl_engine->direct_scissor_off)
     //_direct_scissor_off_api_get(funcs);
}

Evas_GL_API *
_evgl_api_gles1_internal_get(void)
{
   return &_gles1_api;
}
