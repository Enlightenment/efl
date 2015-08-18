#define GL_ERRORS_NODEF 1
#include "evas_gl_core_private.h"
#include "evas_gl_api_ext.h"
#include <dlfcn.h>

#define EVGL_FUNC_BEGIN() if (UNLIKELY(_need_context_restore)) _context_restore()

#define EVGLD_FUNC_BEGIN() \
{ \
   _func_begin_debug(__FUNCTION__); \
}

#define EVGLD_FUNC_END() GLERRV(__FUNCTION__)
#define _EVGL_INT_INIT_VALUE -3

static void *_gles3_handle = NULL;
static Evas_GL_API _gles3_api;
//---------------------------------------//
// API Debug Error Checking Code
static
void _make_current_check(const char* api)
{
   EVGL_Context *ctx = NULL;

   ctx = evas_gl_common_current_context_get();

   if (!ctx)
     CRI("\e[1;33m%s\e[m: Current Context NOT SET: GL Call Should NOT Be Called without MakeCurrent!!!", api);
   else if ((ctx->version != EVAS_GL_GLES_2_X) && (ctx->version != EVAS_GL_GLES_3_X))
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
     {
        CRI("\e[1;33m%s\e[m: This API is being called outside Pixel Get Callback Function.", api);
     }
}

static
void _func_begin_debug(const char *api)
{
   _make_current_check(api);
   _direct_rendering_check(api);
}

//-------------------------------------------------------------//
// GL to GLES Compatibility Functions
//-------------------------------------------------------------//
void
_evgl_glBindFramebuffer(GLenum target, GLuint framebuffer)
{
   EVGL_Context *ctx = NULL;
   EVGL_Resource *rsc;

   rsc = _evgl_tls_resource_get();
   ctx = evas_gl_common_current_context_get();

   if (!ctx)
     {
        ERR("No current context set.");
        return;
     }
   if (!rsc)
     {
        ERR("No current TLS resource.");
        return;
     }

   // Take care of BindFramebuffer 0 issue
   if (framebuffer==0)
     {
        if (_evgl_direct_enabled())
          {
             glBindFramebuffer(target, 0);

             if (rsc->direct.partial.enabled)
               {
                  if (!ctx->partial_render)
                    {
                       evgl_direct_partial_render_start();
                       ctx->partial_render = 1;
                    }
               }
          }
        else
          {
             glBindFramebuffer(target, ctx->surface_fbo);
          }
        ctx->current_fbo = 0;
     }
   else
     {
        if (_evgl_direct_enabled())
          {
             if (ctx->current_fbo == 0)
               {
                  if (rsc->direct.partial.enabled)
                     evgl_direct_partial_render_end();
               }
          }

        glBindFramebuffer(target, framebuffer);

        // Save this for restore when doing make current
        ctx->current_fbo = framebuffer;
     }
}

void
_evgl_glClearDepthf(GLclampf depth)
{
#ifdef GL_GLES
   glClearDepthf(depth);
#else
   glClearDepth(depth);
#endif
}

void
_evgl_glDepthRangef(GLclampf zNear, GLclampf zFar)
{
#ifdef GL_GLES
   glDepthRangef(zNear, zFar);
#else
   glDepthRange(zNear, zFar);
#endif
}

void
_evgl_glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
#ifdef GL_GLES
   glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
#else
   if (range)
     {
        range[0] = -126; // floor(log2(FLT_MIN))
        range[1] = 127; // floor(log2(FLT_MAX))
     }
   if (precision)
     {
        precision[0] = 24; // floor(-log2((1.0/16777218.0)));
     }
   return;
   if (shadertype) shadertype = precisiontype = 0;
#endif
}

void
_evgl_glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length)
{
#ifdef GL_GLES
   glShaderBinary(n, shaders, binaryformat, binary, length);
#else
   // FIXME: need to dlsym/getprocaddress for this
   ERR("Binary Shader is not supported here yet.");
   (void)n;
   (void)shaders;
   (void)binaryformat;
   (void)binary;
   (void)length;
#endif
}

void
_evgl_glReleaseShaderCompiler(void)
{
#ifdef GL_GLES
   glReleaseShaderCompiler();
#else
#endif
}


//-------------------------------------------------------------//
// Calls related to Evas GL Direct Rendering
//-------------------------------------------------------------//
// Transform from Evas Coordinat to GL Coordinate
// returns: imgc[4] (oc[4]) original image object dimension in gl coord
// returns: objc[4] (nc[4]) tranformed  (x, y, width, heigth) in gl coord
// returns: cc[4] cliped coordinate in original coordinate
void
compute_gl_coordinates(int win_w, int win_h, int rot, int clip_image,
                       int x, int y, int width, int height,
                       int img_x, int img_y, int img_w, int img_h,
                       int clip_x, int clip_y, int clip_w, int clip_h,
                       int imgc[4], int objc[4], int cc[4])
{
   if (rot == 0)
     {
        // oringinal image object coordinate in gl coordinate
        imgc[0] = img_x;
        imgc[1] = win_h - img_y - img_h;
        imgc[2] = imgc[0] + img_w;
        imgc[3] = imgc[1] + img_h;

        // clip coordinates in gl coordinate
        cc[0] = clip_x;
        cc[1] = win_h - clip_y - clip_h;
        cc[2] = cc[0] + clip_w;
        cc[3] = cc[1] + clip_h;

        // transformed (x,y,width,height) in gl coordinate
        objc[0] = imgc[0] + x;
        objc[1] = imgc[1] + y;
        objc[2] = objc[0] + width;
        objc[3] = objc[1] + height;
     }
   else if (rot == 180)
     {
        // oringinal image object coordinate in gl coordinate
        imgc[0] = win_w - img_x - img_w;
        imgc[1] = img_y;
        imgc[2] = imgc[0] + img_w;
        imgc[3] = imgc[1] + img_h;

        // clip coordinates in gl coordinate
        cc[0] = win_w - clip_x - clip_w;
        cc[1] = clip_y;
        cc[2] = cc[0] + clip_w;
        cc[3] = cc[1] + clip_h;

        // transformed (x,y,width,height) in gl coordinate
        objc[0] = imgc[0] + img_w - x - width;
        objc[1] = imgc[1] + img_h - y - height;
        objc[2] = objc[0] + width;
        objc[3] = objc[1] + height;

     }
   else if (rot == 90)
     {
        // oringinal image object coordinate in gl coordinate
        imgc[0] = img_y;
        imgc[1] = img_x;
        imgc[2] = imgc[0] + img_h;
        imgc[3] = imgc[1] + img_w;

        // clip coordinates in gl coordinate
        cc[0] = clip_y;
        cc[1] = clip_x;
        cc[2] = cc[0] + clip_h;
        cc[3] = cc[1] + clip_w;

        // transformed (x,y,width,height) in gl coordinate
        objc[0] = imgc[0] + img_h - y - height;
        objc[1] = imgc[1] + x;
        objc[2] = objc[0] + height;
        objc[3] = objc[1] + width;
     }
   else if (rot == 270)
     {
        // oringinal image object coordinate in gl coordinate
        imgc[0] = win_h - img_y - img_h;
        imgc[1] = win_w - img_x - img_w;
        imgc[2] = imgc[0] + img_h;
        imgc[3] = imgc[1] + img_w;

        // clip coordinates in gl coordinate
        cc[0] = win_h - clip_y - clip_h;
        cc[1] = win_w - clip_x - clip_w;
        cc[2] = cc[0] + clip_h;
        cc[3] = cc[1] + clip_w;

        // transformed (x,y,width,height) in gl coordinate
        objc[0] = imgc[0] + y;
        objc[1] = imgc[1] + img_w - x - width;
        objc[2] = objc[0] + height;
        objc[3] = objc[1] + width;
     }
   else
     {
        ERR("Invalid rotation angle %d.", rot);
        return;
     }

   if (clip_image)
     {
        // Clip against original image object
        if (objc[0] < imgc[0]) objc[0] = imgc[0];
        if (objc[0] > imgc[2]) objc[0] = imgc[2];

        if (objc[1] < imgc[1]) objc[1] = imgc[1];
        if (objc[1] > imgc[3]) objc[1] = imgc[3];

        if (objc[2] < imgc[0]) objc[2] = imgc[0];
        if (objc[2] > imgc[2]) objc[2] = imgc[2];

        if (objc[3] < imgc[1]) objc[3] = imgc[1];
        if (objc[3] > imgc[3]) objc[3] = imgc[3];
     }

   imgc[2] = imgc[2]-imgc[0];     // width
   imgc[3] = imgc[3]-imgc[1];     // height

   objc[2] = objc[2]-objc[0];     // width
   objc[3] = objc[3]-objc[1];     // height

   cc[2] = cc[2]-cc[0]; // width
   cc[3] = cc[3]-cc[1]; // height

   //DBG( "\e[1;32m     Img[%d %d %d %d] Original [%d %d %d %d]  Transformed[%d %d %d %d]  Clip[%d %d %d %d] Clipped[%d %d %d %d] \e[m", img_x, img_y, img_w, img_h, imgc[0], imgc[1], imgc[2], imgc[3], objc[0], objc[1], objc[2], objc[3], clip[0], clip[1], clip[2], clip[3], cc[0], cc[1], cc[2], cc[3]);
}

static void
_evgl_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   EVGL_Resource *rsc;

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
   glClearColor(red, green, blue, alpha);
}

static void
_evgl_glClear(GLbitfield mask)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};
   int cc[4] = {0,0,0,0};

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

   if (_evgl_direct_enabled())
     {
        if (!(rsc->current_ctx->current_fbo))
          {
             /* Skip glClear() if clearing with transparent color
              * Note: There will be side effects if the object itself is not
              * marked as having an alpha channel!
              * COPY mode forces the normal behaviour of glClear().
              */
             if (ctx->current_sfc->alpha && !rsc->direct.render_op_copy &&
                 (mask & GL_COLOR_BUFFER_BIT))
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
                  glEnable(GL_SCISSOR_TEST);
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
                  glScissor(nc[0], nc[1], nc[2], nc[3]);
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
                  glScissor(cc[0], cc[1], cc[2], cc[3]);
               }

             glClear(mask);

             // TODO/FIXME: Restore previous client-side scissors.
          }
        else
          {
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  glDisable(GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             glClear(mask);
          }
     }
   else
     {
        if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
          {
             glDisable(GL_SCISSOR_TEST);
             ctx->direct_scissor = 0;
          }

        glClear(mask);
     }
}

static void
_evgl_glEnable(GLenum cap)
{
   EVGL_Context *ctx;

   ctx = evas_gl_common_current_context_get();

   if (ctx && (cap == GL_SCISSOR_TEST))
     {
        ctx->scissor_enabled = 1;

        if (_evgl_direct_enabled())
          {
             EVGL_Resource *rsc = _evgl_tls_resource_get();
             int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0}, cc[4] = {0,0,0,0};

             if (rsc)
               {
                  if (!ctx->current_fbo)
                    {
                       // Direct rendering to canvas
                       if (!ctx->scissor_updated)
                         {
                            compute_gl_coordinates(rsc->direct.win_w, rsc->direct.win_h,
                                                   rsc->direct.rot, 0,
                                                   0, 0, 0, 0,
                                                   rsc->direct.img.x, rsc->direct.img.y,
                                                   rsc->direct.img.w, rsc->direct.img.h,
                                                   rsc->direct.clip.x, rsc->direct.clip.y,
                                                   rsc->direct.clip.w, rsc->direct.clip.h,
                                                   oc, nc, cc);
                            glScissor(cc[0], cc[1], cc[2], cc[3]);
                         }
                       else
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
                            glScissor(nc[0], nc[1], nc[2], nc[3]);
                         }
                       ctx->direct_scissor = 1;
                    }
               }
             else
               {
                  // Bound to an FBO, reset scissors to user data
                  if (ctx->scissor_updated)
                    {
                       glScissor(ctx->scissor_coord[0], ctx->scissor_coord[1],
                                 ctx->scissor_coord[2], ctx->scissor_coord[3]);
                    }
                  else if (ctx->direct_scissor)
                    {
                       // Back to the default scissors (here: max texture size)
                       glScissor(0, 0, evgl_engine->caps.max_w, evgl_engine->caps.max_h);
                    }
                  ctx->direct_scissor = 0;
               }

             glEnable(GL_SCISSOR_TEST);
             return;
          }
     }

   glEnable(cap);
}

static void
_evgl_glDisable(GLenum cap)
{
   EVGL_Context *ctx;

   ctx = evas_gl_common_current_context_get();

   if (ctx && (cap == GL_SCISSOR_TEST))
     {
        ctx->scissor_enabled = 0;

        if (_evgl_direct_enabled())
          {
             if (!ctx->current_fbo)
               {
                  // Restore default scissors for direct rendering
                  int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0}, cc[4] = {0,0,0,0};
                  EVGL_Resource *rsc = _evgl_tls_resource_get();

                  if (rsc)
                    {
                       compute_gl_coordinates(rsc->direct.win_w, rsc->direct.win_h,
                                              rsc->direct.rot, 1,
                                              0, 0, rsc->direct.img.w, rsc->direct.img.h,
                                              rsc->direct.img.x, rsc->direct.img.y,
                                              rsc->direct.img.w, rsc->direct.img.h,
                                              rsc->direct.clip.x, rsc->direct.clip.y,
                                              rsc->direct.clip.w, rsc->direct.clip.h,
                                              oc, nc, cc);

                       RECTS_CLIP_TO_RECT(nc[0], nc[1], nc[2], nc[3], cc[0], cc[1], cc[2], cc[3]);
                       glScissor(nc[0], nc[1], nc[2], nc[3]);

                       ctx->direct_scissor = 1;
                       glEnable(GL_SCISSOR_TEST);
                    }
               }
             else
               {
                  // Bound to an FBO, disable scissors for real
                  ctx->direct_scissor = 0;
                  glDisable(GL_SCISSOR_TEST);
               }
             return;
          }
     }

   glDisable(cap);
}

void
_evgl_glGetIntegerv(GLenum pname, GLint* params)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;

   if (_evgl_direct_enabled())
     {
        if (!params)
          {
             ERR("Invalid Parameter");
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

        // Only need to handle it if it's directly rendering to the window
        if (!(rsc->current_ctx->current_fbo))
          {
             if (pname == GL_SCISSOR_BOX)
               {
                  if (ctx->scissor_updated)
                    {
                       memcpy(params, ctx->scissor_coord, sizeof(int)*4);
                       return;
                    }
               }
             else if (pname == GL_VIEWPORT)
               {
                  if (ctx->viewport_updated)
                    {
                       memcpy(params, ctx->viewport_coord, sizeof(int)*4);
                       return;
                    }
               }

             // If it hasn't been initialized yet, return img object size
             if ((pname == GL_SCISSOR_BOX) || (pname == GL_VIEWPORT))
               {
                  params[0] = 0;
                  params[1] = 0;
                  params[2] = (GLint)rsc->direct.img.w;
                  params[3] = (GLint)rsc->direct.img.h;
                  return;
               }
          }
     }
   else
     {
        if (pname == GL_FRAMEBUFFER_BINDING)
          {
             rsc = _evgl_tls_resource_get();
             ctx = rsc ? rsc->current_ctx : NULL;
             if (ctx)
               {
                  *params = ctx->current_fbo;
                  return;
               }
          }
     }

   glGetIntegerv(pname, params);
}

static const GLubyte *
_evgl_glGetString(GLenum name)
{
   static char _version[128] = {0};
   static char _glsl[128] = {0};
   EVGL_Resource *rsc;
   const GLubyte *ret;

   /* We wrap two values here:
    *
    * VERSION: Since OpenGL ES 3 is not supported yet, we return OpenGL ES 2.0
    *   The string is not modified on desktop GL (eg. 4.4.0 NVIDIA 343.22)
    *   GLES 3 support is not exposed because apps can't use GLES 3 core
    *   functions yet.
    *
    * EXTENSIONS: This should return only the list of GL extensions supported
    *   by Evas GL. This means as many extensions as possible should be
    *   added to the whitelist.
    */

   /*
    * Note from Khronos: "If an error is generated, glGetString returns 0."
    * I decided not to call glGetString if there is no context as this is
    * known to cause crashes on certain GL drivers (eg. Nvidia binary blob).
    * --> crash moved to app side if they blindly call strstr()
    */

   if ((!(rsc = _evgl_tls_resource_get())) || !rsc->current_ctx)
     {
        ERR("Current context is NULL, not calling glGetString");
        // This sets evas_gl_error_get instead of glGetError...
        evas_gl_common_error_set(NULL, EVAS_GL_BAD_CONTEXT);
        return NULL;
     }

   switch (name)
     {
      case GL_VENDOR:
      case GL_RENDERER:
        // Keep these as-is.
        break;

      case GL_SHADING_LANGUAGE_VERSION:
        ret = glGetString(GL_SHADING_LANGUAGE_VERSION);
        if (!ret) return NULL;
#ifdef GL_GLES
        if (ret[18] != (GLubyte) '1')
          {
             // We try not to remove the vendor fluff
             snprintf(_glsl, sizeof(_glsl), "OpenGL ES GLSL ES 1.00 Evas GL (%s)", ((char *) ret) + 18);
             _glsl[sizeof(_glsl) - 1] = '\0';
             return (const GLubyte *) _glsl;
          }
        return ret;
#else
        // Desktop GL, we still keep the official name
        snprintf(_glsl, sizeof(_glsl), "OpenGL ES GLSL ES 1.00 Evas GL (%s)", (char *) ret);
        _version[sizeof(_glsl) - 1] = '\0';
        return (const GLubyte *) _glsl;
#endif

      case GL_VERSION:
        ret = glGetString(GL_VERSION);
        if (!ret) return NULL;
#ifdef GL_GLES
        if ((ret[10] != (GLubyte) '2') && (ret[10] != (GLubyte) '3'))
          {
             // We try not to remove the vendor fluff
             snprintf(_version, sizeof(_version), "OpenGL ES 2.0 Evas GL (%s)", ((char *) ret) + 10);
             _version[sizeof(_version) - 1] = '\0';
             return (const GLubyte *) _version;
          }
        return ret;
#else
        // Desktop GL, we still keep the official name
        snprintf(_version, sizeof(_version), "OpenGL ES 2.0 Evas GL (%s)", (char *) ret);
        _version[sizeof(_version) - 1] = '\0';
        return (const GLubyte *) _version;
#endif

      case GL_EXTENSIONS:
        // Passing the version -  GLESv2/GLESv3.
        return (GLubyte *) evgl_api_ext_string_get(EINA_TRUE, rsc->current_ctx->version);

      default:
        // GL_INVALID_ENUM is generated if name is not an accepted value.
        WRN("Unknown string requested: %x", (unsigned int) name);
        break;
     }

   return glGetString(name);
}

static void
_evgl_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};
   int cc[4] = {0,0,0,0};


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

   if (_evgl_direct_enabled())
     {

        if (!(rsc->current_ctx->current_fbo))
          {
             compute_gl_coordinates(rsc->direct.win_w, rsc->direct.win_h,
                                    rsc->direct.rot, 1,
                                    x, y, width, height,
                                    rsc->direct.img.x, rsc->direct.img.y,
                                    rsc->direct.img.w, rsc->direct.img.h,
                                    rsc->direct.clip.x, rsc->direct.clip.y,
                                    rsc->direct.clip.w, rsc->direct.clip.h,
                                    oc, nc, cc);
             glReadPixels(nc[0], nc[1], nc[2], nc[3], format, type, pixels);
          }
        else
          {
             glReadPixels(x, y, width, height, format, type, pixels);
          }
     }
   else
     {
        glReadPixels(x, y, width, height, format, type, pixels);
     }
}

static void
_evgl_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};
   int cc[4] = {0,0,0,0};

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

   if (_evgl_direct_enabled())
     {
        if (!(rsc->current_ctx->current_fbo))
          {
             // Direct rendering to canvas
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  glDisable(GL_SCISSOR_TEST);
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
             glScissor(nc[0], nc[1], nc[2], nc[3]);

             ctx->direct_scissor = 0;

             // Mark user scissor_coord as valid
             ctx->scissor_updated = 1;
          }
        else
          {
             // Bound to an FBO, use these new scissors
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  glDisable(GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             glScissor(x, y, width, height);

             // Why did we set this flag to 0???
             //ctx->scissor_updated = 0;
          }
     }
   else
     {
        if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
          {
             glDisable(GL_SCISSOR_TEST);
             ctx->direct_scissor = 0;
          }

        glScissor(x, y, width, height);
     }
}

static void
_evgl_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};
   int cc[4] = {0,0,0,0};

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

   if (_evgl_direct_enabled())
     {
        if (!(rsc->current_ctx->current_fbo))
          {
             if ((!ctx->direct_scissor))
               {
                  glEnable(GL_SCISSOR_TEST);
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
                  glScissor(nc[0], nc[1], nc[2], nc[3]);

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
                  glViewport(nc[0], nc[1], nc[2], nc[3]);
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
                  glScissor(cc[0], cc[1], cc[2], cc[3]);

                  glViewport(nc[0], nc[1], nc[2], nc[3]);
               }

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
                  glDisable(GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             glViewport(x, y, width, height);
          }
     }
   else
     {
        if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
          {
             glDisable(GL_SCISSOR_TEST);
             ctx->direct_scissor = 0;
          }

        glViewport(x, y, width, height);
     }
}
//-------------------------------------------------------------//

// Open GLES 2.0 APIs
#define _EVASGL_FUNCTION_PRIVATE_BEGIN(ret, name, param1, param2) \
   static ret evgl_##name param1 { EVGL_FUNC_BEGIN(); return _evgl_##name param2; }
#define _EVASGL_FUNCTION_BEGIN(ret, name, param1, param2) \
   static ret evgl_##name param1 { EVGL_FUNC_BEGIN(); return name param2; }

#include "evas_gl_api_def.h"

#undef _EVASGL_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_FUNCTION_BEGIN

//-------------------------------------------------------------//
// Open GLES 3.0 APIs
//#define _CHECK_NULL(ret, name) if (!_gles3_api.##name) return (ret)0
#define _EVASGL_FUNCTION_BEGIN(ret, name, param1, param2) \
   static ret evgl_gles3_##name param1 {\
    if (!_gles3_api.name) return (ret)0;\
    return _gles3_api.name param2; }

#include "evas_gl_api_gles3_def.h"

#undef _EVASGL_FUNCTION_BEGIN

//-------------------------------------------------------------//
// Debug Evas GL APIs
//  - GL APIs Overriden for debugging purposes
//-------------------------------------------------------------//

void
_evgld_glActiveTexture(GLenum texture)
{
   EVGLD_FUNC_BEGIN();
   evgl_glActiveTexture(texture);
   EVGLD_FUNC_END();
}

void
_evgld_glAttachShader(GLuint program, GLuint shader)
{
   EVGLD_FUNC_BEGIN();
   evgl_glAttachShader(program, shader);
   EVGLD_FUNC_END();
}

void
_evgld_glBindAttribLocation(GLuint program, GLuint idx, const char* name)
{
   EVGLD_FUNC_BEGIN();
   evgl_glBindAttribLocation(program, idx, name);
   EVGLD_FUNC_END();
}

void
_evgld_glBindBuffer(GLenum target, GLuint buffer)
{
   EVGLD_FUNC_BEGIN();
   evgl_glBindBuffer(target, buffer);
   EVGLD_FUNC_END();
}

void
_evgld_glBindFramebuffer(GLenum target, GLuint framebuffer)
{
   EVGLD_FUNC_BEGIN();
   _evgl_glBindFramebuffer(target, framebuffer);
   EVGLD_FUNC_END();
}

void
_evgld_glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
   EVGLD_FUNC_BEGIN();
   evgl_glBindRenderbuffer(target, renderbuffer);
   EVGLD_FUNC_END();
}

void
_evgld_glBindTexture(GLenum target, GLuint texture)
{
   EVGLD_FUNC_BEGIN();
   evgl_glBindTexture(target, texture);
   EVGLD_FUNC_END();
}

void
_evgld_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   EVGLD_FUNC_BEGIN();
   evgl_glBlendColor(red, green, blue, alpha);
   EVGLD_FUNC_END();
}

void
_evgld_glBlendEquation(GLenum mode)
{
   EVGLD_FUNC_BEGIN();
   evgl_glBlendEquation(mode);
   EVGLD_FUNC_END();
}

void
_evgld_glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
   EVGLD_FUNC_BEGIN();
   evgl_glBlendEquationSeparate(modeRGB, modeAlpha);
   EVGLD_FUNC_END();
}

void
_evgld_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
   EVGLD_FUNC_BEGIN();
   evgl_glBlendFunc(sfactor, dfactor);
   EVGLD_FUNC_END();
}

void
_evgld_glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
   EVGLD_FUNC_BEGIN();
   evgl_glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
   EVGLD_FUNC_END();
}

void
_evgld_glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
   EVGLD_FUNC_BEGIN();
   evgl_glBufferData(target, size, data, usage);
   EVGLD_FUNC_END();
}

void
_evgld_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
   EVGLD_FUNC_BEGIN();
   evgl_glBufferSubData(target, offset, size, data);
   EVGLD_FUNC_END();
}

GLenum
_evgld_glCheckFramebufferStatus(GLenum target)
{
   GLenum ret = GL_NONE;

   EVGLD_FUNC_BEGIN();
   ret = glCheckFramebufferStatus(target);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   EVGLD_FUNC_BEGIN();
   _evgl_glClearColor(red, green, blue, alpha);
   EVGLD_FUNC_END();
}

void
_evgld_glClearDepthf(GLclampf depth)
{
   EVGLD_FUNC_BEGIN();
   _evgl_glClearDepthf(depth);
   EVGLD_FUNC_END();
}

void
_evgld_glClearStencil(GLint s)
{
   EVGLD_FUNC_BEGIN();
   evgl_glClearStencil(s);
   EVGLD_FUNC_END();
}

void
_evgld_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
   EVGLD_FUNC_BEGIN();
   evgl_glColorMask(red, green, blue, alpha);
   EVGLD_FUNC_END();
}

void
_evgld_glCompileShader(GLuint shader)
{
   EVGLD_FUNC_BEGIN();
   evgl_glCompileShader(shader);
   EVGLD_FUNC_END();
}

void
_evgld_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
   EVGLD_FUNC_BEGIN();
   evgl_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
   EVGLD_FUNC_END();
}

void
_evgld_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
   EVGLD_FUNC_BEGIN();
   evgl_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
   EVGLD_FUNC_END();
}

void
_evgld_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
   EVGLD_FUNC_BEGIN();
   evgl_glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
   EVGLD_FUNC_END();
}

void
_evgld_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGLD_FUNC_BEGIN();
   evgl_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
   EVGLD_FUNC_END();
}

GLuint
_evgld_glCreateProgram(void)
{
   GLuint ret = _EVGL_INT_INIT_VALUE;

   EVGLD_FUNC_BEGIN();
   ret = evgl_glCreateProgram();
   EVGLD_FUNC_END();
   return ret;
}

GLuint
_evgld_glCreateShader(GLenum type)
{
   GLuint ret = _EVGL_INT_INIT_VALUE;
   EVGLD_FUNC_BEGIN();
   ret = evgl_glCreateShader(type);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glCullFace(GLenum mode)
{
   EVGLD_FUNC_BEGIN();
   evgl_glCullFace(mode);
   EVGLD_FUNC_END();
}

void
_evgld_glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDeleteBuffers(n, buffers);
   EVGLD_FUNC_END();
}

void
_evgld_glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDeleteFramebuffers(n, framebuffers);
   EVGLD_FUNC_END();
}

void
_evgld_glDeleteProgram(GLuint program)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDeleteProgram(program);
   EVGLD_FUNC_END();
}

void
_evgld_glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDeleteRenderbuffers(n, renderbuffers);
   EVGLD_FUNC_END();
}

void
_evgld_glDeleteShader(GLuint shader)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDeleteShader(shader);
   EVGLD_FUNC_END();
}

void
_evgld_glDeleteTextures(GLsizei n, const GLuint* textures)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDeleteTextures(n, textures);
   EVGLD_FUNC_END();
}

void
_evgld_glDepthFunc(GLenum func)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDepthFunc(func);
   EVGLD_FUNC_END();
}

void
_evgld_glDepthMask(GLboolean flag)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDepthMask(flag);
   EVGLD_FUNC_END();
}

void
_evgld_glDepthRangef(GLclampf zNear, GLclampf zFar)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDepthRangef(zNear, zFar);
   EVGLD_FUNC_END();
}

void
_evgld_glDetachShader(GLuint program, GLuint shader)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDetachShader(program, shader);
   EVGLD_FUNC_END();
}

void
_evgld_glDisableVertexAttribArray(GLuint idx)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDisableVertexAttribArray(idx);
   EVGLD_FUNC_END();
}

void
_evgld_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDrawArrays(mode, first, count);
   EVGLD_FUNC_END();
}

void
_evgld_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDrawElements(mode, count, type, indices);
   EVGLD_FUNC_END();
}

void
_evgld_glEnableVertexAttribArray(GLuint idx)
{
   EVGLD_FUNC_BEGIN();
   evgl_glEnableVertexAttribArray(idx);
   EVGLD_FUNC_END();
}

void
_evgld_glFinish(void)
{
   EVGLD_FUNC_BEGIN();
   evgl_glFinish();
   EVGLD_FUNC_END();
}

void
_evgld_glFlush(void)
{
   EVGLD_FUNC_BEGIN();
   evgl_glFlush();
   EVGLD_FUNC_END();
}

void
_evgld_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
   EVGLD_FUNC_BEGIN();
   evgl_glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
   EVGLD_FUNC_END();
}

void
_evgld_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
   EVGLD_FUNC_BEGIN();
   evgl_glFramebufferTexture2D(target, attachment, textarget, texture, level);
   EVGLD_FUNC_END();
}

void
_evgld_glFrontFace(GLenum mode)
{
   EVGLD_FUNC_BEGIN();
   evgl_glFrontFace(mode);
   EVGLD_FUNC_END();
}

void
_evgld_glGetVertexAttribfv(GLuint idx, GLenum pname, GLfloat* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetVertexAttribfv(idx, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetVertexAttribiv(GLuint idx, GLenum pname, GLint* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetVertexAttribiv(idx, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetVertexAttribPointerv(GLuint idx, GLenum pname, void** pointer)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetVertexAttribPointerv(idx, pname, pointer);
   EVGLD_FUNC_END();
}

void
_evgld_glHint(GLenum target, GLenum mode)
{
   EVGLD_FUNC_BEGIN();
   evgl_glHint(target, mode);
   EVGLD_FUNC_END();
}

void
_evgld_glGenBuffers(GLsizei n, GLuint* buffers)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGenBuffers(n, buffers);
   EVGLD_FUNC_END();
}

void
_evgld_glGenerateMipmap(GLenum target)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGenerateMipmap(target);
   EVGLD_FUNC_END();
}

void
_evgld_glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGenFramebuffers(n, framebuffers);
   EVGLD_FUNC_END();
}

void
_evgld_glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGenRenderbuffers(n, renderbuffers);
   EVGLD_FUNC_END();
}

void
_evgld_glGenTextures(GLsizei n, GLuint* textures)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGenTextures(n, textures);
   EVGLD_FUNC_END();
}

void
_evgld_glGetActiveAttrib(GLuint program, GLuint idx, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetActiveAttrib(program, idx, bufsize, length, size, type, name);
   EVGLD_FUNC_END();
}

void
_evgld_glGetActiveUniform(GLuint program, GLuint idx, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetActiveUniform(program, idx, bufsize, length, size, type, name);
   EVGLD_FUNC_END();
}

void
_evgld_glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetAttachedShaders(program, maxcount, count, shaders);
   EVGLD_FUNC_END();
}

int
_evgld_glGetAttribLocation(GLuint program, const char* name)
{
   int ret = _EVGL_INT_INIT_VALUE;
   EVGLD_FUNC_BEGIN();
   ret = evgl_glGetAttribLocation(program, name);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glGetBooleanv(GLenum pname, GLboolean* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetBooleanv(pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetBufferParameteriv(target, pname, params);
   EVGLD_FUNC_END();
}

GLenum
_evgld_glGetError(void)
{
   GLenum ret = GL_NONE;

   EVGLD_FUNC_BEGIN();
   ret = evgl_glGetError();
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glGetFloatv(GLenum pname, GLfloat* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetFloatv(pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetProgramiv(program, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, char* infolog)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetProgramInfoLog(program, bufsize, length, infolog);
   EVGLD_FUNC_END();
}

void
_evgld_glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetRenderbufferParameteriv(target, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetShaderiv(shader, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetShaderInfoLog(shader, bufsize, length, infolog);
   EVGLD_FUNC_END();
}

void
_evgld_glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
   EVGLD_FUNC_END();
}

void
_evgld_glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, char* source)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetShaderSource(shader, bufsize, length, source);
   EVGLD_FUNC_END();
}

const GLubyte *
_evgld_glGetString(GLenum name)
{
   const GLubyte *ret = NULL;

   EVGLD_FUNC_BEGIN();
   ret = evgl_glGetString(name);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetTexParameterfv(target, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetTexParameteriv(target, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetUniformfv(program, location, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetUniformiv(GLuint program, GLint location, GLint* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetUniformiv(program, location, params);
   EVGLD_FUNC_END();
}
int
_evgld_glGetUniformLocation(GLuint program, const char* name)
{
   int ret = _EVGL_INT_INIT_VALUE;

   EVGLD_FUNC_BEGIN();
   ret = evgl_glGetUniformLocation(program, name);
   EVGLD_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsBuffer(GLuint buffer)
{
   GLboolean ret = GL_FALSE;

   EVGLD_FUNC_BEGIN();
   ret = evgl_glIsBuffer(buffer);
   EVGLD_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsEnabled(GLenum cap)
{
   GLboolean ret = GL_FALSE;

   EVGLD_FUNC_BEGIN();
   ret = evgl_glIsEnabled(cap);
   EVGLD_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsFramebuffer(GLuint framebuffer)
{
   GLboolean ret = GL_FALSE;

   EVGLD_FUNC_BEGIN();
   ret = evgl_glIsFramebuffer(framebuffer);
   EVGLD_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsProgram(GLuint program)
{
   GLboolean ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_glIsProgram(program);
   EVGLD_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsRenderbuffer(GLuint renderbuffer)
{
   GLboolean ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_glIsRenderbuffer(renderbuffer);
   EVGLD_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsShader(GLuint shader)
{
   GLboolean ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_glIsShader(shader);
   EVGLD_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsTexture(GLuint texture)
{
   GLboolean ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_glIsTexture(texture);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glLineWidth(GLfloat width)
{
   EVGLD_FUNC_BEGIN();
   evgl_glLineWidth(width);
   EVGLD_FUNC_END();
}

void
_evgld_glLinkProgram(GLuint program)
{
   EVGLD_FUNC_BEGIN();
   evgl_glLinkProgram(program);
   EVGLD_FUNC_END();
}

void
_evgld_glPixelStorei(GLenum pname, GLint param)
{
   EVGLD_FUNC_BEGIN();
   evgl_glPixelStorei(pname, param);
   EVGLD_FUNC_END();
}

void
_evgld_glPolygonOffset(GLfloat factor, GLfloat units)
{
   EVGLD_FUNC_BEGIN();
   evgl_glPolygonOffset(factor, units);
   EVGLD_FUNC_END();
}

void
_evgld_glReleaseShaderCompiler(void)
{
   EVGLD_FUNC_BEGIN();
   evgl_glReleaseShaderCompiler();
   EVGLD_FUNC_END();
}

void
_evgld_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
   EVGLD_FUNC_BEGIN();
   evgl_glRenderbufferStorage(target, internalformat, width, height);
   EVGLD_FUNC_END();
}

void
_evgld_glSampleCoverage(GLclampf value, GLboolean invert)
{
   EVGLD_FUNC_BEGIN();
   evgl_glSampleCoverage(value, invert);
   EVGLD_FUNC_END();
}

void
_evgld_glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length)
{
   EVGLD_FUNC_BEGIN();
   evgl_glShaderBinary(n, shaders, binaryformat, binary, length);
   EVGLD_FUNC_END();
}

void
_evgld_glShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
   EVGLD_FUNC_BEGIN();
   evgl_glShaderSource(shader, count, (const GLchar* const*) string, length);
   EVGLD_FUNC_END();
}

void
_evgld_glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
   EVGLD_FUNC_BEGIN();
   evgl_glStencilFunc(func, ref, mask);
   EVGLD_FUNC_END();
}

void
_evgld_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
   EVGLD_FUNC_BEGIN();
   evgl_glStencilFuncSeparate(face, func, ref, mask);
   EVGLD_FUNC_END();
}

void
_evgld_glStencilMask(GLuint mask)
{
   EVGLD_FUNC_BEGIN();
   evgl_glStencilMask(mask);
   EVGLD_FUNC_END();
}

void
_evgld_glStencilMaskSeparate(GLenum face, GLuint mask)
{
   EVGLD_FUNC_BEGIN();
   evgl_glStencilMaskSeparate(face, mask);
   EVGLD_FUNC_END();
}

void
_evgld_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
   EVGLD_FUNC_BEGIN();
   evgl_glStencilOp(fail, zfail, zpass);
   EVGLD_FUNC_END();
}

void
_evgld_glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
   EVGLD_FUNC_BEGIN();
   evgl_glStencilOpSeparate(face, fail, zfail, zpass);
   EVGLD_FUNC_END();
}

void
_evgld_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
   EVGLD_FUNC_BEGIN();
   evgl_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
   EVGLD_FUNC_END();
}

void
_evgld_glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
   EVGLD_FUNC_BEGIN();
   evgl_glTexParameterf(target, pname, param);
   EVGLD_FUNC_END();
}

void
_evgld_glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glTexParameterfv(target, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glTexParameteri(GLenum target, GLenum pname, GLint param)
{
   EVGLD_FUNC_BEGIN();
   evgl_glTexParameteri(target, pname, param);
   EVGLD_FUNC_END();
}

void
_evgld_glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glTexParameteriv(target, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
   EVGLD_FUNC_BEGIN();
   evgl_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform1f(GLint location, GLfloat x)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform1f(location, x);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform1fv(location, count, v);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform1i(GLint location, GLint x)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform1i(location, x);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform1iv(GLint location, GLsizei count, const GLint* v)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform1iv(location, count, v);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform2f(GLint location, GLfloat x, GLfloat y)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform2f(location, x, y);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform2fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform2fv(location, count, v);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform2i(GLint location, GLint x, GLint y)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform2i(location, x, y);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform2iv(GLint location, GLsizei count, const GLint* v)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform2iv(location, count, v);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform3f(location, x, y, z);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform3fv(location, count, v);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform3i(GLint location, GLint x, GLint y, GLint z)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform3i(location, x, y, z);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform3iv(GLint location, GLsizei count, const GLint* v)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform3iv(location, count, v);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform4f(location, x, y, z, w);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform4fv(location, count, v);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform4i(location, x, y, z, w);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform4iv(GLint location, GLsizei count, const GLint* v)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniform4iv(location, count, v);
   EVGLD_FUNC_END();
}

void
_evgld_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniformMatrix2fv(location, count, transpose, value);
   EVGLD_FUNC_END();
}

void
_evgld_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniformMatrix3fv(location, count, transpose, value);
   EVGLD_FUNC_END();
}

void
_evgld_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUniformMatrix4fv(location, count, transpose, value);
   EVGLD_FUNC_END();
}

void
_evgld_glUseProgram(GLuint program)
{
   EVGLD_FUNC_BEGIN();
   evgl_glUseProgram(program);
   EVGLD_FUNC_END();
}

void
_evgld_glValidateProgram(GLuint program)
{
   EVGLD_FUNC_BEGIN();
   evgl_glValidateProgram(program);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttrib1f(GLuint indx, GLfloat x)
{
   EVGLD_FUNC_BEGIN();
   evgl_glVertexAttrib1f(indx, x);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttrib1fv(GLuint indx, const GLfloat* values)
{
   EVGLD_FUNC_BEGIN();
   evgl_glVertexAttrib1fv(indx, values);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)
{
   EVGLD_FUNC_BEGIN();
   evgl_glVertexAttrib2f(indx, x, y);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttrib2fv(GLuint indx, const GLfloat* values)
{
   EVGLD_FUNC_BEGIN();
   evgl_glVertexAttrib2fv(indx, values);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
   EVGLD_FUNC_BEGIN();
   evgl_glVertexAttrib3f(indx, x, y, z);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttrib3fv(GLuint indx, const GLfloat* values)
{
   EVGLD_FUNC_BEGIN();
   evgl_glVertexAttrib3fv(indx, values);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   EVGLD_FUNC_BEGIN();
   evgl_glVertexAttrib4f(indx, x, y, z, w);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttrib4fv(GLuint indx, const GLfloat* values)
{
   EVGLD_FUNC_BEGIN();
   evgl_glVertexAttrib4fv(indx, values);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr)
{
   EVGLD_FUNC_BEGIN();
   evgl_glVertexAttribPointer(indx, size, type, normalized, stride, ptr);
   EVGLD_FUNC_END();
}

//-------------------------------------------------------------//
// Open GLES 3.0 APIs

void
_evgld_glReadBuffer(GLenum mode)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glReadBuffer(mode);
   EVGLD_FUNC_END();
}

void
_evgld_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glDrawRangeElements(mode, start, end, count, type, indices);
   EVGLD_FUNC_END();
}

void
_evgld_glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
   EVGLD_FUNC_END();
}

void
_evgld_glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
   EVGLD_FUNC_END();
}

void
_evgld_glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
   EVGLD_FUNC_END();
}

void
_evgld_glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
   EVGLD_FUNC_END();
}

void
_evgld_glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
   EVGLD_FUNC_END();
}

void
_evgld_glGenQueries(GLsizei n, GLuint *ids)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGenQueries(n, ids);
   EVGLD_FUNC_END();
}

void
_evgld_glDeleteQueries(GLsizei n, const GLuint *ids)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glDeleteQueries(n, ids);
   EVGLD_FUNC_END();
}

GLboolean
 _evgld_glIsQuery(GLuint id)
{
   GLboolean ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glIsQuery(id);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glBeginQuery(GLenum target, GLuint id)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glBeginQuery(target, id);
   EVGLD_FUNC_END();
}

void
_evgld_glEndQuery(GLenum target)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glEndQuery(target);
   EVGLD_FUNC_END();
}

void
_evgld_glGetQueryiv(GLenum target, GLenum pname, GLint *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetQueryiv(target, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetQueryObjectuiv(id, pname, params);
   EVGLD_FUNC_END();
}

GLboolean
_evgld_glUnmapBuffer(GLenum target)
{
   GLboolean ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glUnmapBuffer(target);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glGetBufferPointerv(GLenum target, GLenum pname, GLvoid **params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetBufferPointerv(target, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glDrawBuffers(GLsizei n, const GLenum *bufs)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glDrawBuffers(n, bufs);
   EVGLD_FUNC_END();
}

void
_evgld_glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniformMatrix2x3fv(location, count, transpose, value);
   EVGLD_FUNC_END();
}

void
_evgld_glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniformMatrix3x2fv(location, count, transpose, value);
   EVGLD_FUNC_END();
}

void
_evgld_glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniformMatrix2x4fv(location, count, transpose, value);
   EVGLD_FUNC_END();
}

void
_evgld_glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniformMatrix4x2fv(location, count, transpose, value);
   EVGLD_FUNC_END();
}

void
_evgld_glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniformMatrix3x4fv(location, count, transpose, value);
   EVGLD_FUNC_END();
}

void
_evgld_glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniformMatrix4x3fv(location, count, transpose, value);
   EVGLD_FUNC_END();
}

void
_evgld_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
   EVGLD_FUNC_END();
}

void
_evgld_glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
   EVGLD_FUNC_END();
}

void
_evgld_glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glFramebufferTextureLayer(target, attachment, texture, level, layer);
   EVGLD_FUNC_END();
}

void *
_evgld_glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
   void* ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glMapBufferRange(target, offset, length, access);
   EVGLD_FUNC_END();
   return ret;
}

GLsync
_evgld_glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
   GLsync ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glFlushMappedBufferRange(target, offset, length);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glBindVertexArray(GLuint array)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glBindVertexArray(array);
   EVGLD_FUNC_END();
}

void
_evgld_glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glDeleteVertexArrays(n, arrays);
   EVGLD_FUNC_END();
}

void
_evgld_glGenVertexArrays(GLsizei n, GLuint *arrays)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGenVertexArrays(n, arrays);
   EVGLD_FUNC_END();
}

GLboolean
_evgld_glIsVertexArray(GLuint array)
{
   GLboolean ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glIsVertexArray(array);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glGetIntegeri_v(GLenum target, GLuint index, GLint *data)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetIntegeri_v(target, index, data);
   EVGLD_FUNC_END();
}

void
_evgld_glBeginTransformFeedback(GLenum primitiveMode)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glBeginTransformFeedback(primitiveMode);
   EVGLD_FUNC_END();
}

void
_evgld_glEndTransformFeedback(void)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glEndTransformFeedback();
   EVGLD_FUNC_END();
}

void
_evgld_glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glBindBufferRange(target, index, buffer, offset, size);
   EVGLD_FUNC_END();
}

void
_evgld_glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glBindBufferBase(target, index, buffer);
   EVGLD_FUNC_END();
}

void
_evgld_glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glTransformFeedbackVaryings(program, count, varyings, bufferMode);
   EVGLD_FUNC_END();
}

void
_evgld_glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glVertexAttribIPointer(index, size, type, stride, pointer);
   EVGLD_FUNC_END();
}

void
_evgld_glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetVertexAttribIiv(index, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetVertexAttribIuiv(index, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glVertexAttribI4i(index, x, y, z,  w);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glVertexAttribI4ui(index, x, y, z, w);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttribI4iv(GLuint index, const GLint *v)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glVertexAttribI4iv(index, v);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttribI4uiv(GLuint index, const GLuint *v)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glVertexAttribI4uiv(index, v);
   EVGLD_FUNC_END();
}

void
_evgld_glGetUniformuiv(GLuint program, GLint location, GLuint *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetUniformuiv(program, location, params);
   EVGLD_FUNC_END();
}

GLint
_evgld_glGetFragDataLocation(GLuint program, const GLchar *name)
{
   GLint ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glGetFragDataLocation(program, name);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glUniform1ui(GLint location, GLuint v0)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniform1ui(location, v0);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniform2ui(location, v0, v1);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniform3ui(location, v0, v1, v2);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniform4ui(location, v0, v1, v2, v3);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniform1uiv(location, count, value);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniform2uiv(location, count, value);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniform3uiv(location, count, value);
   EVGLD_FUNC_END();
}

void
_evgld_glUniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniform4uiv(location, count, value);
   EVGLD_FUNC_END();
}

void
_evgld_glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glClearBufferiv(buffer, drawbuffer, value);
   EVGLD_FUNC_END();
}

void
_evgld_glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glClearBufferuiv(buffer, drawbuffer, value);
   EVGLD_FUNC_END();
}

void
_evgld_glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glClearBufferfv(buffer, drawbuffer, value);
   EVGLD_FUNC_END();
}

void
_evgld_glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glClearBufferfi(buffer, drawbuffer, depth, stencil);
   EVGLD_FUNC_END();
}

const GLubyte *
 _evgld_glGetStringi(GLenum name, GLuint index)
{
   const GLubyte *ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glGetStringi(name, index);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
   EVGLD_FUNC_END();
}

void
_evgld_glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetUniformIndices(program, uniformCount, uniformNames,uniformIndices);
   EVGLD_FUNC_END();
}

void
_evgld_glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
   EVGLD_FUNC_END();
}

GLuint
_evgld_glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName)
{
   GLuint ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glGetUniformBlockIndex(program, uniformBlockName);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
   EVGLD_FUNC_END();
}

void
_evgld_glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
   EVGLD_FUNC_END();
}

void
_evgld_glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glDrawArraysInstanced(mode, first, count, instancecount);
   EVGLD_FUNC_END();
}

void
_evgld_glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glDrawElementsInstanced(mode, count, type, indices, instancecount);
   EVGLD_FUNC_END();
}

GLsync
_evgld_glFenceSync(GLenum condition, GLbitfield flags)
{
   GLsync ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glFenceSync(condition, flags);
   EVGLD_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsSync(GLsync sync)
{
   GLboolean ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glIsSync(sync);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glDeleteSync(GLsync sync)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glDeleteSync(sync);
   EVGLD_FUNC_END();
}

GLenum
_evgld_glClientWaitSync(GLsync sync, GLbitfield flags, EvasGLuint64 timeout)
{
   GLenum ret;
   EVGLD_FUNC_BEGIN();
   ret =  evgl_gles3_glClientWaitSync(sync, flags, timeout);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glWaitSync(GLsync sync, GLbitfield flags, EvasGLuint64 timeout)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glWaitSync(sync, flags, timeout);
   EVGLD_FUNC_END();
}

void
_evgld_glGetInteger64v(GLenum pname, EvasGLint64 *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetInteger64v(pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetSynciv(sync, pname, bufSize, length, values);
   EVGLD_FUNC_END();
}

void
_evgld_glGetInteger64i_v(GLenum target, GLuint index, EvasGLint64 *data)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetInteger64i_v(target, index, data);
   EVGLD_FUNC_END();
}

void
_evgld_glGetBufferParameteri64v(GLenum target, GLenum pname, EvasGLint64 *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetBufferParameteri64v(target, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGenSamplers(GLsizei count, GLuint *samplers)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGenSamplers(count, samplers);
   EVGLD_FUNC_END();
}

void
_evgld_glDeleteSamplers(GLsizei count, const GLuint *samplers)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glDeleteSamplers(count, samplers);
   EVGLD_FUNC_END();
}

GLboolean
_evgld_glIsSampler(GLuint sampler)
{
   GLboolean ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glIsSampler(sampler);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glBindSampler(GLuint unit, GLuint sampler)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glBindSampler(unit, sampler);
   EVGLD_FUNC_END();
}

void
_evgld_glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glSamplerParameteri(sampler, pname, param);
   EVGLD_FUNC_END();
}

void
_evgld_glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glSamplerParameteriv(sampler, pname, param);
   EVGLD_FUNC_END();
}

void
_evgld_glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glSamplerParameterf(sampler, pname, param);
   EVGLD_FUNC_END();
}

void
_evgld_glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glSamplerParameterfv(sampler, pname, param);
   EVGLD_FUNC_END();
}

void
_evgld_glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetSamplerParameteriv(sampler, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetSamplerParameterfv(sampler, pname, params);
   EVGLD_FUNC_END();
}

void
_evgld_glVertexAttribDivisor(GLuint index, GLuint divisor)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glVertexAttribDivisor(index, divisor);
   EVGLD_FUNC_END();
}

void
_evgld_glBindTransformFeedback(GLenum target, GLuint id)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glBindTransformFeedback(target, id);
   EVGLD_FUNC_END();
}

void
_evgld_glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glDeleteTransformFeedbacks(n, ids);
   EVGLD_FUNC_END();
}

void
_evgld_glGenTransformFeedbacks(GLsizei n, GLuint *ids)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGenTransformFeedbacks(n, ids);
   EVGLD_FUNC_END();
}

GLboolean
_evgld_glIsTransformFeedback(GLuint id)
{
   GLboolean ret;
   EVGLD_FUNC_BEGIN();
   ret = evgl_gles3_glIsTransformFeedback(id);
   EVGLD_FUNC_END();
   return ret;
}

void
_evgld_glPauseTransformFeedback(void)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glPauseTransformFeedback();
   EVGLD_FUNC_END();
}

void
_evgld_glResumeTransformFeedback(void)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glResumeTransformFeedback();
   EVGLD_FUNC_END();
}

void
_evgld_glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
   EVGLD_FUNC_END();
}

void
_evgld_glProgramBinary(GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glProgramBinary(program, binaryFormat, binary, length);
   EVGLD_FUNC_END();
}

void
_evgld_glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glProgramParameteri(program, pname, value);
   EVGLD_FUNC_END();
}

void
_evgld_glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glInvalidateFramebuffer(target, numAttachments, attachments);
   EVGLD_FUNC_END();
}

void
_evgld_glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
   EVGLD_FUNC_END();
}

void
_evgld_glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glTexStorage2D(target, levels, internalformat, width, height);
   EVGLD_FUNC_END();
}

void
_evgld_glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glTexStorage3D(target, levels, internalformat, width, height, depth);
   EVGLD_FUNC_END();
}

void
_evgld_glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params)
{
   EVGLD_FUNC_BEGIN();
   evgl_gles3_glGetInternalformativ(target, internalformat, pname, bufSize, params);
   EVGLD_FUNC_END();
}

//-------------------------------------------------------------//
// Calls for stripping precision string in the shader
#if 0

static const char *
opengl_strtok(const char *s, int *n, char **saveptr, char *prevbuf)
{
   char *start;
   char *ret;
   char *p;
   int retlen;
   static const char *delim = " \t\n\r/";

   if (prevbuf)
      free(prevbuf);

   if (s)
     {
        *saveptr = s;
     }
   else
     {
        if (!(*saveptr) || !(*n))
           return NULL;
        s = *saveptr;
     }

   for (; *n && strchr(delim, *s); s++, (*n)--)
     {
        if (*s == '/' && *n > 1)
          {
             if (s[1] == '/')
               {
                  do {
                       s++, (*n)--;
                  } while (*n > 1 && s[1] != '\n' && s[1] != '\r');
               }
             else if (s[1] == '*')
               {
                  do {
                       s++, (*n)--;
                  } while (*n > 2 && (s[1] != '*' || s[2] != '/'));
                  s++, (*n)--;
                  s++, (*n)--;
                  if (*n == 0)
                    {
                       break;
                    }
               }
             else
               {
                  break;
               }
          }
     }

   start = s;
   for (; *n && *s && !strchr(delim, *s); s++, (*n)--);
   if (*n > 0)
      s++, (*n)--;

   *saveptr = s;

   retlen = s - start;
   ret = malloc(retlen + 1);
   p = ret;

   if (retlen == 0)
     {
        *p = 0;
        return;
     }

   while (retlen > 0)
     {
        if (*start == '/' && retlen > 1)
          {
             if (start[1] == '/')
               {
                  do {
                       start++, retlen--;
                  } while (retlen > 1 && start[1] != '\n' && start[1] != '\r');
                  start++, retlen--;
                  continue;
               } else if (start[1] == '*')
                 {
                    do {
                         start++, retlen--;
                    } while (retlen > 2 && (start[1] != '*' || start[2] != '/'));
                    start += 3, retlen -= 3;
                    continue;
                 }
          }
        *(p++) = *(start++), retlen--;
     }

   *p = 0;
   return ret;
}

static char *
do_eglShaderPatch(const char *source, int length, int *patched_len)
{
   char *saveptr = NULL;
   char *sp;
   char *p = NULL;

   if (!length) length = strlen(source);

   *patched_len = 0;
   int patched_size = length;
   char *patched = malloc(patched_size + 1);

   if (!patched) return NULL;

   p = opengl_strtok(source, &length, &saveptr, NULL);

   for (; p; p = opengl_strtok(0, &length, &saveptr, p))
     {
        if (!strncmp(p, "lowp", 4) || !strncmp(p, "mediump", 7) || !strncmp(p, "highp", 5))
          {
             continue;
          }
        else if (!strncmp(p, "precision", 9))
          {
             while ((p = opengl_strtok(0, &length, &saveptr, p)) && !strchr(p, ';'));
          }
        else
          {
             if (!strncmp(p, "gl_MaxVertexUniformVectors", 26))
               {
                  free(p);
                  p = strdup("(gl_MaxVertexUniformComponents / 4)");
               }
             else if (!strncmp(p, "gl_MaxFragmentUniformVectors", 28))
               {
                  free(p);
                  p = strdup("(gl_MaxFragmentUniformComponents / 4)");
               }
             else if (!strncmp(p, "gl_MaxVaryingVectors", 20))
               {
                  free(p);
                  p = strdup("(gl_MaxVaryingFloats / 4)");
               }

             int new_len = strlen(p);
             if (*patched_len + new_len > patched_size)
               {
                  char *tmp;

                  patched_size *= 2;
                  tmp = realloc(patched, patched_size + 1);
                  if (!tmp)
                    {
                       free(patched);
                       free(p);
                       return NULL;
                    }
                  patched = tmp;
               }

             memcpy(patched + *patched_len, p, new_len);
             *patched_len += new_len;
          }
     }

   patched[*patched_len] = 0;
   /* check that we don't leave dummy preprocessor lines */
   for (sp = patched; *sp;)
     {
        for (; *sp == ' ' || *sp == '\t'; sp++);
        if (!strncmp(sp, "#define", 7))
          {
             for (p = sp + 7; *p == ' ' || *p == '\t'; p++);
             if (*p == '\n' || *p == '\r' || *p == '/')
               {
                  memset(sp, 0x20, 7);
               }
          }
        for (; *sp && *sp != '\n' && *sp != '\r'; sp++);
        for (; *sp == '\n' || *sp == '\r'; sp++);
     }
   return patched;
}

static int
shadersrc_gles_to_gl(GLsizei count, const char** string, char **s, const GLint* length, GLint *l)
{
   int i;

   for(i = 0; i < count; ++i) {
        GLint len;
        if(length) {
             len = length[i];
             if (len < 0)
                len = string[i] ? strlen(string[i]) : 0;
        } else
           len = string[i] ? strlen(string[i]) : 0;

        if(string[i]) {
             s[i] = do_eglShaderPatch(string[i], len, &l[i]);
             if(!s[i]) {
                  while(i)
                     free(s[--i]);

                  free(l);
                  free(s);
                  return -1;
             }
        } else {
             s[i] = NULL;
             l[i] = 0;
        }
   }

   return 0;
}


void
_evgld_glShaderSource(GLuint shader, GLsizei count, const char* const* string, const GLint* length)
{
   EVGLD_FUNC_BEGIN();

#ifdef GL_GLES
   glShaderSource(shader, count, string, length);
   goto finish;
#else
   //GET_EXT_PTR(void, glShaderSource, (int, int, char **, void *));
   int size = count;
   int i;
   int acc_length = 0;
   GLchar **tab_prog = malloc(size * sizeof(GLchar *));
   int *tab_length = (int *) length;

   char **tab_prog_new;
   GLint *tab_length_new;

   tab_prog_new = malloc(count* sizeof(char*));
   tab_length_new = malloc(count* sizeof(GLint));

   memset(tab_prog_new, 0, count * sizeof(char*));
   memset(tab_length_new, 0, count * sizeof(GLint));

   for (i = 0; i < size; i++) {
        tab_prog[i] = ((GLchar *) string) + acc_length;
        acc_length += tab_length[i];
   }

   shadersrc_gles_to_gl(count, tab_prog, tab_prog_new, tab_length, tab_length_new);

   if (!tab_prog_new || !tab_length_new)
      ERR("Error allocating memory for shader string manipulation.");

   glShaderSource(shader, count, tab_prog_new, tab_length_new);

   for (i = 0; i < count; i++)
      free(tab_prog_new[i]);
   free(tab_prog_new);
   free(tab_length_new);

   free(tab_prog);
#endif

finish:
   EVGLD_FUNC_END();
}
#endif

//-------------------------------------------------------------//


//-------------------------------------------------------------//
// Calls related to Evas GL Direct Rendering
//-------------------------------------------------------------//
static void
_evgld_glClear(GLbitfield mask)
{
   EVGLD_FUNC_BEGIN();
   evgl_glClear(mask);
   EVGLD_FUNC_END();
}

static void
_evgld_glEnable(GLenum cap)
{
   EVGLD_FUNC_BEGIN();
   evgl_glEnable(cap);
   EVGLD_FUNC_END();
}

static void
_evgld_glDisable(GLenum cap)
{
   EVGLD_FUNC_BEGIN();
   evgl_glDisable(cap);
   EVGLD_FUNC_END();
}

void
_evgld_glGetIntegerv(GLenum pname, GLint* params)
{
   EVGLD_FUNC_BEGIN();
   evgl_glGetIntegerv(pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
   EVGLD_FUNC_BEGIN();
   evgl_glReadPixels(x, y, width, height, format, type, pixels);
   EVGLD_FUNC_END();
}

static void
_evgld_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGLD_FUNC_BEGIN();
   evgl_glScissor(x, y, width, height);
   EVGLD_FUNC_END();
}

static void
_evgld_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGLD_FUNC_BEGIN();
   evgl_glViewport(x, y, width, height);
   EVGLD_FUNC_END();
}
//-------------------------------------------------------------//

static void
_normal_gles2_api_get(Evas_GL_API *funcs)
{
   funcs->version = EVAS_GL_API_VERSION;

#define ORD(f) EVAS_API_OVERRIDE(f, funcs, evgl_)
   // GLES 2.0
   ORD(glActiveTexture);
   ORD(glAttachShader);
   ORD(glBindAttribLocation);
   ORD(glBindBuffer);
   ORD(glBindTexture);
   ORD(glBlendColor);
   ORD(glBlendEquation);
   ORD(glBlendEquationSeparate);
   ORD(glBlendFunc);
   ORD(glBlendFuncSeparate);
   ORD(glBufferData);
   ORD(glBufferSubData);
   ORD(glCheckFramebufferStatus);
   ORD(glClear);
   ORD(glClearColor);
   ORD(glClearDepthf);
   ORD(glClearStencil);
   ORD(glColorMask);
   ORD(glCompileShader);
   ORD(glCompressedTexImage2D);
   ORD(glCompressedTexSubImage2D);
   ORD(glCopyTexImage2D);
   ORD(glCopyTexSubImage2D);
   ORD(glCreateProgram);
   ORD(glCreateShader);
   ORD(glCullFace);
   ORD(glDeleteBuffers);
   ORD(glDeleteFramebuffers);
   ORD(glDeleteProgram);
   ORD(glDeleteRenderbuffers);
   ORD(glDeleteShader);
   ORD(glDeleteTextures);
   ORD(glDepthFunc);
   ORD(glDepthMask);
   ORD(glDepthRangef);
   ORD(glDetachShader);
   ORD(glDisable);
   ORD(glDisableVertexAttribArray);
   ORD(glDrawArrays);
   ORD(glDrawElements);
   ORD(glEnable);
   ORD(glEnableVertexAttribArray);
   ORD(glFinish);
   ORD(glFlush);
   ORD(glFramebufferRenderbuffer);
   ORD(glFramebufferTexture2D);
   ORD(glFrontFace);
   ORD(glGenBuffers);
   ORD(glGenerateMipmap);
   ORD(glGenFramebuffers);
   ORD(glGenRenderbuffers);
   ORD(glGenTextures);
   ORD(glGetActiveAttrib);
   ORD(glGetActiveUniform);
   ORD(glGetAttachedShaders);
   ORD(glGetAttribLocation);
   ORD(glGetBooleanv);
   ORD(glGetBufferParameteriv);
   ORD(glGetError);
   ORD(glGetFloatv);
   ORD(glGetFramebufferAttachmentParameteriv);
   ORD(glGetIntegerv);
   ORD(glGetProgramiv);
   ORD(glGetProgramInfoLog);
   ORD(glGetRenderbufferParameteriv);
   ORD(glGetShaderiv);
   ORD(glGetShaderInfoLog);
   ORD(glGetShaderPrecisionFormat);
   ORD(glGetShaderSource);
   ORD(glGetString);
   ORD(glGetTexParameterfv);
   ORD(glGetTexParameteriv);
   ORD(glGetUniformfv);
   ORD(glGetUniformiv);
   ORD(glGetUniformLocation);
   ORD(glGetVertexAttribfv);
   ORD(glGetVertexAttribiv);
   ORD(glGetVertexAttribPointerv);
   ORD(glHint);
   ORD(glIsBuffer);
   ORD(glIsEnabled);
   ORD(glIsFramebuffer);
   ORD(glIsProgram);
   ORD(glIsRenderbuffer);
   ORD(glIsShader);
   ORD(glIsTexture);
   ORD(glLineWidth);
   ORD(glLinkProgram);
   ORD(glPixelStorei);
   ORD(glPolygonOffset);
   ORD(glReadPixels);
   ORD(glReleaseShaderCompiler);
   ORD(glRenderbufferStorage);
   ORD(glSampleCoverage);
   ORD(glScissor);
   ORD(glShaderBinary);
   ORD(glShaderSource);
   ORD(glStencilFunc);
   ORD(glStencilFuncSeparate);
   ORD(glStencilMask);
   ORD(glStencilMaskSeparate);
   ORD(glStencilOp);
   ORD(glStencilOpSeparate);
   ORD(glTexImage2D);
   ORD(glTexParameterf);
   ORD(glTexParameterfv);
   ORD(glTexParameteri);
   ORD(glTexParameteriv);
   ORD(glTexSubImage2D);
   ORD(glUniform1f);
   ORD(glUniform1fv);
   ORD(glUniform1i);
   ORD(glUniform1iv);
   ORD(glUniform2f);
   ORD(glUniform2fv);
   ORD(glUniform2i);
   ORD(glUniform2iv);
   ORD(glUniform3f);
   ORD(glUniform3fv);
   ORD(glUniform3i);
   ORD(glUniform3iv);
   ORD(glUniform4f);
   ORD(glUniform4fv);
   ORD(glUniform4i);
   ORD(glUniform4iv);
   ORD(glUniformMatrix2fv);
   ORD(glUniformMatrix3fv);
   ORD(glUniformMatrix4fv);
   ORD(glUseProgram);
   ORD(glValidateProgram);
   ORD(glVertexAttrib1f);
   ORD(glVertexAttrib1fv);
   ORD(glVertexAttrib2f);
   ORD(glVertexAttrib2fv);
   ORD(glVertexAttrib3f);
   ORD(glVertexAttrib3fv);
   ORD(glVertexAttrib4f);
   ORD(glVertexAttrib4fv);
   ORD(glVertexAttribPointer);
   ORD(glViewport);

   ORD(glBindFramebuffer);
   ORD(glBindRenderbuffer);
#undef ORD
}

static void
_direct_scissor_off_api_get(Evas_GL_API *funcs)
{

#define ORD(f) EVAS_API_OVERRIDE(f, funcs,)
   // For Direct Rendering
   ORD(glClear);
   ORD(glClearColor);
   ORD(glDisable);
   ORD(glEnable);
   ORD(glGetIntegerv);
   ORD(glReadPixels);
   ORD(glScissor);
   ORD(glViewport);
#undef ORD
}


static void
_debug_gles2_api_get(Evas_GL_API *funcs)
{
   funcs->version = EVAS_GL_API_VERSION;

#define ORD(f) EVAS_API_OVERRIDE(f, funcs, _evgld_)
   // GLES 2.0
   ORD(glActiveTexture);
   ORD(glAttachShader);
   ORD(glBindAttribLocation);
   ORD(glBindBuffer);
   ORD(glBindTexture);
   ORD(glBlendColor);
   ORD(glBlendEquation);
   ORD(glBlendEquationSeparate);
   ORD(glBlendFunc);
   ORD(glBlendFuncSeparate);
   ORD(glBufferData);
   ORD(glBufferSubData);
   ORD(glCheckFramebufferStatus);
   ORD(glClear);
   ORD(glClearColor);
   ORD(glClearDepthf);
   ORD(glClearStencil);
   ORD(glColorMask);
   ORD(glCompileShader);
   ORD(glCompressedTexImage2D);
   ORD(glCompressedTexSubImage2D);
   ORD(glCopyTexImage2D);
   ORD(glCopyTexSubImage2D);
   ORD(glCreateProgram);
   ORD(glCreateShader);
   ORD(glCullFace);
   ORD(glDeleteBuffers);
   ORD(glDeleteFramebuffers);
   ORD(glDeleteProgram);
   ORD(glDeleteRenderbuffers);
   ORD(glDeleteShader);
   ORD(glDeleteTextures);
   ORD(glDepthFunc);
   ORD(glDepthMask);
   ORD(glDepthRangef);
   ORD(glDetachShader);
   ORD(glDisable);
   ORD(glDisableVertexAttribArray);
   ORD(glDrawArrays);
   ORD(glDrawElements);
   ORD(glEnable);
   ORD(glEnableVertexAttribArray);
   ORD(glFinish);
   ORD(glFlush);
   ORD(glFramebufferRenderbuffer);
   ORD(glFramebufferTexture2D);
   ORD(glFrontFace);
   ORD(glGenBuffers);
   ORD(glGenerateMipmap);
   ORD(glGenFramebuffers);
   ORD(glGenRenderbuffers);
   ORD(glGenTextures);
   ORD(glGetActiveAttrib);
   ORD(glGetActiveUniform);
   ORD(glGetAttachedShaders);
   ORD(glGetAttribLocation);
   ORD(glGetBooleanv);
   ORD(glGetBufferParameteriv);
   ORD(glGetError);
   ORD(glGetFloatv);
   ORD(glGetFramebufferAttachmentParameteriv);
   ORD(glGetIntegerv);
   ORD(glGetProgramiv);
   ORD(glGetProgramInfoLog);
   ORD(glGetRenderbufferParameteriv);
   ORD(glGetShaderiv);
   ORD(glGetShaderInfoLog);
   ORD(glGetShaderPrecisionFormat);
   ORD(glGetShaderSource);
   ORD(glGetString);
   ORD(glGetTexParameterfv);
   ORD(glGetTexParameteriv);
   ORD(glGetUniformfv);
   ORD(glGetUniformiv);
   ORD(glGetUniformLocation);
   ORD(glGetVertexAttribfv);
   ORD(glGetVertexAttribiv);
   ORD(glGetVertexAttribPointerv);
   ORD(glHint);
   ORD(glIsBuffer);
   ORD(glIsEnabled);
   ORD(glIsFramebuffer);
   ORD(glIsProgram);
   ORD(glIsRenderbuffer);
   ORD(glIsShader);
   ORD(glIsTexture);
   ORD(glLineWidth);
   ORD(glLinkProgram);
   ORD(glPixelStorei);
   ORD(glPolygonOffset);
   ORD(glReadPixels);
   ORD(glReleaseShaderCompiler);
   ORD(glRenderbufferStorage);
   ORD(glSampleCoverage);
   ORD(glScissor);
   ORD(glShaderBinary);
   ORD(glShaderSource);
   ORD(glStencilFunc);
   ORD(glStencilFuncSeparate);
   ORD(glStencilMask);
   ORD(glStencilMaskSeparate);
   ORD(glStencilOp);
   ORD(glStencilOpSeparate);
   ORD(glTexImage2D);
   ORD(glTexParameterf);
   ORD(glTexParameterfv);
   ORD(glTexParameteri);
   ORD(glTexParameteriv);
   ORD(glTexSubImage2D);
   ORD(glUniform1f);
   ORD(glUniform1fv);
   ORD(glUniform1i);
   ORD(glUniform1iv);
   ORD(glUniform2f);
   ORD(glUniform2fv);
   ORD(glUniform2i);
   ORD(glUniform2iv);
   ORD(glUniform3f);
   ORD(glUniform3fv);
   ORD(glUniform3i);
   ORD(glUniform3iv);
   ORD(glUniform4f);
   ORD(glUniform4fv);
   ORD(glUniform4i);
   ORD(glUniform4iv);
   ORD(glUniformMatrix2fv);
   ORD(glUniformMatrix3fv);
   ORD(glUniformMatrix4fv);
   ORD(glUseProgram);
   ORD(glValidateProgram);
   ORD(glVertexAttrib1f);
   ORD(glVertexAttrib1fv);
   ORD(glVertexAttrib2f);
   ORD(glVertexAttrib2fv);
   ORD(glVertexAttrib3f);
   ORD(glVertexAttrib3fv);
   ORD(glVertexAttrib4f);
   ORD(glVertexAttrib4fv);
   ORD(glVertexAttribPointer);
   ORD(glViewport);

   ORD(glBindFramebuffer);
   ORD(glBindRenderbuffer);
#undef ORD
}

void
_evgl_api_gles2_get(Evas_GL_API *funcs, Eina_Bool debug)
{
   if (debug)
     _debug_gles2_api_get(funcs);
   else
     _normal_gles2_api_get(funcs);

   if (evgl_engine->direct_scissor_off)
     _direct_scissor_off_api_get(funcs);
}

static void
_normal_gles3_api_get(Evas_GL_API *funcs)
{
#define ORD(f) EVAS_API_OVERRIDE(f, funcs, evgl_)
   // GLES 3.0 APIs that are same as GLES 2.0
   ORD(glActiveTexture);
   ORD(glAttachShader);
   ORD(glBindAttribLocation);
   ORD(glBindBuffer);
   ORD(glBindTexture);
   ORD(glBlendColor);
   ORD(glBlendEquation);
   ORD(glBlendEquationSeparate);
   ORD(glBlendFunc);
   ORD(glBlendFuncSeparate);
   ORD(glBufferData);
   ORD(glBufferSubData);
   ORD(glCheckFramebufferStatus);
   ORD(glClear);
   ORD(glClearColor);
   ORD(glClearDepthf);
   ORD(glClearStencil);
   ORD(glColorMask);
   ORD(glCompileShader);
   ORD(glCompressedTexImage2D);
   ORD(glCompressedTexSubImage2D);
   ORD(glCopyTexImage2D);
   ORD(glCopyTexSubImage2D);
   ORD(glCreateProgram);
   ORD(glCreateShader);
   ORD(glCullFace);
   ORD(glDeleteBuffers);
   ORD(glDeleteFramebuffers);
   ORD(glDeleteProgram);
   ORD(glDeleteRenderbuffers);
   ORD(glDeleteShader);
   ORD(glDeleteTextures);
   ORD(glDepthFunc);
   ORD(glDepthMask);
   ORD(glDepthRangef);
   ORD(glDetachShader);
   ORD(glDisable);
   ORD(glDisableVertexAttribArray);
   ORD(glDrawArrays);
   ORD(glDrawElements);
   ORD(glEnable);
   ORD(glEnableVertexAttribArray);
   ORD(glFinish);
   ORD(glFlush);
   ORD(glFramebufferRenderbuffer);
   ORD(glFramebufferTexture2D);
   ORD(glFrontFace);
   ORD(glGenBuffers);
   ORD(glGenerateMipmap);
   ORD(glGenFramebuffers);
   ORD(glGenRenderbuffers);
   ORD(glGenTextures);
   ORD(glGetActiveAttrib);
   ORD(glGetActiveUniform);
   ORD(glGetAttachedShaders);
   ORD(glGetAttribLocation);
   ORD(glGetBooleanv);
   ORD(glGetBufferParameteriv);
   ORD(glGetError);
   ORD(glGetFloatv);
   ORD(glGetFramebufferAttachmentParameteriv);
   ORD(glGetIntegerv);
   ORD(glGetProgramiv);
   ORD(glGetProgramInfoLog);
   ORD(glGetRenderbufferParameteriv);
   ORD(glGetShaderiv);
   ORD(glGetShaderInfoLog);
   ORD(glGetShaderPrecisionFormat);
   ORD(glGetShaderSource);
   ORD(glGetString);
   ORD(glGetTexParameterfv);
   ORD(glGetTexParameteriv);
   ORD(glGetUniformfv);
   ORD(glGetUniformiv);
   ORD(glGetUniformLocation);
   ORD(glGetVertexAttribfv);
   ORD(glGetVertexAttribiv);
   ORD(glGetVertexAttribPointerv);
   ORD(glHint);
   ORD(glIsBuffer);
   ORD(glIsEnabled);
   ORD(glIsFramebuffer);
   ORD(glIsProgram);
   ORD(glIsRenderbuffer);
   ORD(glIsShader);
   ORD(glIsTexture);
   ORD(glLineWidth);
   ORD(glLinkProgram);
   ORD(glPixelStorei);
   ORD(glPolygonOffset);
   ORD(glReadPixels);
   ORD(glReleaseShaderCompiler);
   ORD(glRenderbufferStorage);
   ORD(glSampleCoverage);
   ORD(glScissor);
   ORD(glShaderBinary);
   ORD(glShaderSource);
   ORD(glStencilFunc);
   ORD(glStencilFuncSeparate);
   ORD(glStencilMask);
   ORD(glStencilMaskSeparate);
   ORD(glStencilOp);
   ORD(glStencilOpSeparate);
   ORD(glTexImage2D);
   ORD(glTexParameterf);
   ORD(glTexParameterfv);
   ORD(glTexParameteri);
   ORD(glTexParameteriv);
   ORD(glTexSubImage2D);
   ORD(glUniform1f);
   ORD(glUniform1fv);
   ORD(glUniform1i);
   ORD(glUniform1iv);
   ORD(glUniform2f);
   ORD(glUniform2fv);
   ORD(glUniform2i);
   ORD(glUniform2iv);
   ORD(glUniform3f);
   ORD(glUniform3fv);
   ORD(glUniform3i);
   ORD(glUniform3iv);
   ORD(glUniform4f);
   ORD(glUniform4fv);
   ORD(glUniform4i);
   ORD(glUniform4iv);
   ORD(glUniformMatrix2fv);
   ORD(glUniformMatrix3fv);
   ORD(glUniformMatrix4fv);
   ORD(glUseProgram);
   ORD(glValidateProgram);
   ORD(glVertexAttrib1f);
   ORD(glVertexAttrib1fv);
   ORD(glVertexAttrib2f);
   ORD(glVertexAttrib2fv);
   ORD(glVertexAttrib3f);
   ORD(glVertexAttrib3fv);
   ORD(glVertexAttrib4f);
   ORD(glVertexAttrib4fv);
   ORD(glVertexAttribPointer);
   ORD(glViewport);

   ORD(glBindFramebuffer);
   ORD(glBindRenderbuffer);
#undef ORD

// GLES 3.0 NEW APIs
#define ORD(name) EVAS_API_OVERRIDE(name, funcs, evgl_gles3_)
   ORD(glBeginQuery);
   ORD(glBeginTransformFeedback);
   ORD(glBindBufferBase);
   ORD(glBindBufferRange);
   ORD(glBindSampler);
   ORD(glBindTransformFeedback);
   ORD(glBindVertexArray);
   ORD(glBlitFramebuffer);
   ORD(glClearBufferfi);
   ORD(glClearBufferfv);
   ORD(glClearBufferiv);
   ORD(glClearBufferuiv);
   ORD(glClientWaitSync);
   ORD(glCompressedTexImage3D);
   ORD(glCompressedTexSubImage3D);
   ORD(glCopyBufferSubData);
   ORD(glCopyTexSubImage3D);
   ORD(glDeleteQueries);
   ORD(glDeleteSamplers);
   ORD(glDeleteSync);
   ORD(glDeleteTransformFeedbacks);
   ORD(glDeleteVertexArrays);
   ORD(glDrawArraysInstanced);
   ORD(glDrawBuffers);
   ORD(glDrawElementsInstanced);
   ORD(glDrawRangeElements);
   ORD(glEndQuery);
   ORD(glEndTransformFeedback);
   ORD(glFenceSync);
   ORD(glFlushMappedBufferRange);
   ORD(glFramebufferTextureLayer);
   ORD(glGenQueries);
   ORD(glGenSamplers);
   ORD(glGenTransformFeedbacks);
   ORD(glGenVertexArrays);
   ORD(glGetActiveUniformBlockiv);
   ORD(glGetActiveUniformBlockName);
   ORD(glGetActiveUniformsiv);
   ORD(glGetBufferParameteri64v);
   ORD(glGetBufferPointerv);
   ORD(glGetFragDataLocation);
   ORD(glGetInteger64i_v);
   ORD(glGetInteger64v);
   ORD(glGetIntegeri_v);
   ORD(glGetInternalformativ);
   ORD(glGetProgramBinary);
   ORD(glGetQueryiv);
   ORD(glGetQueryObjectuiv);
   ORD(glGetSamplerParameterfv);
   ORD(glGetSamplerParameteriv);
   ORD(glGetStringi);
   ORD(glGetSynciv);
   ORD(glGetTransformFeedbackVarying);
   ORD(glGetUniformBlockIndex);
   ORD(glGetUniformIndices);
   ORD(glGetUniformuiv);
   ORD(glGetVertexAttribIiv);
   ORD(glGetVertexAttribIuiv);
   ORD(glInvalidateFramebuffer);
   ORD(glInvalidateSubFramebuffer);
   ORD(glIsQuery);
   ORD(glIsSampler);
   ORD(glIsSync);
   ORD(glIsTransformFeedback);
   ORD(glIsVertexArray);
   ORD(glMapBufferRange);
   ORD(glPauseTransformFeedback);
   ORD(glProgramBinary);
   ORD(glProgramParameteri);
   ORD(glReadBuffer);
   ORD(glRenderbufferStorageMultisample);
   ORD(glResumeTransformFeedback);
   ORD(glSamplerParameterf);
   ORD(glSamplerParameterfv);
   ORD(glSamplerParameteri);
   ORD(glSamplerParameteriv);
   ORD(glTexImage3D);
   ORD(glTexStorage2D);
   ORD(glTexStorage3D);
   ORD(glTexSubImage3D);
   ORD(glTransformFeedbackVaryings);
   ORD(glUniform1ui);
   ORD(glUniform1uiv);
   ORD(glUniform2ui);
   ORD(glUniform2uiv);
   ORD(glUniform3ui);
   ORD(glUniform3uiv);
   ORD(glUniform4ui);
   ORD(glUniform4uiv);
   ORD(glUniformBlockBinding);
   ORD(glUniformMatrix2x3fv);
   ORD(glUniformMatrix3x2fv);
   ORD(glUniformMatrix2x4fv);
   ORD(glUniformMatrix4x2fv);
   ORD(glUniformMatrix3x4fv);
   ORD(glUniformMatrix4x3fv);
   ORD(glUnmapBuffer);
   ORD(glVertexAttribDivisor);
   ORD(glVertexAttribI4i);
   ORD(glVertexAttribI4iv);
   ORD(glVertexAttribI4ui);
   ORD(glVertexAttribI4uiv);
   ORD(glVertexAttribIPointer);
   ORD(glWaitSync);

#undef ORD
}

static void
_debug_gles3_api_get(Evas_GL_API *funcs)
{

#define ORD(f) EVAS_API_OVERRIDE(f, funcs, _evgld_)
   // GLES 3.0 APIs that are same as GLES 2.0
   ORD(glActiveTexture);
   ORD(glAttachShader);
   ORD(glBindAttribLocation);
   ORD(glBindBuffer);
   ORD(glBindTexture);
   ORD(glBlendColor);
   ORD(glBlendEquation);
   ORD(glBlendEquationSeparate);
   ORD(glBlendFunc);
   ORD(glBlendFuncSeparate);
   ORD(glBufferData);
   ORD(glBufferSubData);
   ORD(glCheckFramebufferStatus);
   ORD(glClear);
   ORD(glClearColor);
   ORD(glClearDepthf);
   ORD(glClearStencil);
   ORD(glColorMask);
   ORD(glCompileShader);
   ORD(glCompressedTexImage2D);
   ORD(glCompressedTexSubImage2D);
   ORD(glCopyTexImage2D);
   ORD(glCopyTexSubImage2D);
   ORD(glCreateProgram);
   ORD(glCreateShader);
   ORD(glCullFace);
   ORD(glDeleteBuffers);
   ORD(glDeleteFramebuffers);
   ORD(glDeleteProgram);
   ORD(glDeleteRenderbuffers);
   ORD(glDeleteShader);
   ORD(glDeleteTextures);
   ORD(glDepthFunc);
   ORD(glDepthMask);
   ORD(glDepthRangef);
   ORD(glDetachShader);
   ORD(glDisable);
   ORD(glDisableVertexAttribArray);
   ORD(glDrawArrays);
   ORD(glDrawElements);
   ORD(glEnable);
   ORD(glEnableVertexAttribArray);
   ORD(glFinish);
   ORD(glFlush);
   ORD(glFramebufferRenderbuffer);
   ORD(glFramebufferTexture2D);
   ORD(glFrontFace);
   ORD(glGenBuffers);
   ORD(glGenerateMipmap);
   ORD(glGenFramebuffers);
   ORD(glGenRenderbuffers);
   ORD(glGenTextures);
   ORD(glGetActiveAttrib);
   ORD(glGetActiveUniform);
   ORD(glGetAttachedShaders);
   ORD(glGetAttribLocation);
   ORD(glGetBooleanv);
   ORD(glGetBufferParameteriv);
   ORD(glGetError);
   ORD(glGetFloatv);
   ORD(glGetFramebufferAttachmentParameteriv);
   ORD(glGetIntegerv);
   ORD(glGetProgramiv);
   ORD(glGetProgramInfoLog);
   ORD(glGetRenderbufferParameteriv);
   ORD(glGetShaderiv);
   ORD(glGetShaderInfoLog);
   ORD(glGetShaderPrecisionFormat);
   ORD(glGetShaderSource);
   ORD(glGetString);
   ORD(glGetTexParameterfv);
   ORD(glGetTexParameteriv);
   ORD(glGetUniformfv);
   ORD(glGetUniformiv);
   ORD(glGetUniformLocation);
   ORD(glGetVertexAttribfv);
   ORD(glGetVertexAttribiv);
   ORD(glGetVertexAttribPointerv);
   ORD(glHint);
   ORD(glIsBuffer);
   ORD(glIsEnabled);
   ORD(glIsFramebuffer);
   ORD(glIsProgram);
   ORD(glIsRenderbuffer);
   ORD(glIsShader);
   ORD(glIsTexture);
   ORD(glLineWidth);
   ORD(glLinkProgram);
   ORD(glPixelStorei);
   ORD(glPolygonOffset);
   ORD(glReadPixels);
   ORD(glReleaseShaderCompiler);
   ORD(glRenderbufferStorage);
   ORD(glSampleCoverage);
   ORD(glScissor);
   ORD(glShaderBinary);
   ORD(glShaderSource);
   ORD(glStencilFunc);
   ORD(glStencilFuncSeparate);
   ORD(glStencilMask);
   ORD(glStencilMaskSeparate);
   ORD(glStencilOp);
   ORD(glStencilOpSeparate);
   ORD(glTexImage2D);
   ORD(glTexParameterf);
   ORD(glTexParameterfv);
   ORD(glTexParameteri);
   ORD(glTexParameteriv);
   ORD(glTexSubImage2D);
   ORD(glUniform1f);
   ORD(glUniform1fv);
   ORD(glUniform1i);
   ORD(glUniform1iv);
   ORD(glUniform2f);
   ORD(glUniform2fv);
   ORD(glUniform2i);
   ORD(glUniform2iv);
   ORD(glUniform3f);
   ORD(glUniform3fv);
   ORD(glUniform3i);
   ORD(glUniform3iv);
   ORD(glUniform4f);
   ORD(glUniform4fv);
   ORD(glUniform4i);
   ORD(glUniform4iv);
   ORD(glUniformMatrix2fv);
   ORD(glUniformMatrix3fv);
   ORD(glUniformMatrix4fv);
   ORD(glUseProgram);
   ORD(glValidateProgram);
   ORD(glVertexAttrib1f);
   ORD(glVertexAttrib1fv);
   ORD(glVertexAttrib2f);
   ORD(glVertexAttrib2fv);
   ORD(glVertexAttrib3f);
   ORD(glVertexAttrib3fv);
   ORD(glVertexAttrib4f);
   ORD(glVertexAttrib4fv);
   ORD(glVertexAttribPointer);
   ORD(glViewport);

   ORD(glBindFramebuffer);
   ORD(glBindRenderbuffer);

   // GLES 3.0 new APIs
   ORD(glBeginQuery);
   ORD(glBeginTransformFeedback);
   ORD(glBindBufferBase);
   ORD(glBindBufferRange);
   ORD(glBindSampler);
   ORD(glBindTransformFeedback);
   ORD(glBindVertexArray);
   ORD(glBlitFramebuffer);
   ORD(glClearBufferfi);
   ORD(glClearBufferfv);
   ORD(glClearBufferiv);
   ORD(glClearBufferuiv);
   ORD(glClientWaitSync);
   ORD(glCompressedTexImage3D);
   ORD(glCompressedTexSubImage3D);
   ORD(glCopyBufferSubData);
   ORD(glCopyTexSubImage3D);
   ORD(glDeleteQueries);
   ORD(glDeleteSamplers);
   ORD(glDeleteSync);
   ORD(glDeleteTransformFeedbacks);
   ORD(glDeleteVertexArrays);
   ORD(glDrawArraysInstanced);
   ORD(glDrawBuffers);
   ORD(glDrawElementsInstanced);
   ORD(glDrawRangeElements);
   ORD(glEndQuery);
   ORD(glEndTransformFeedback);
   ORD(glFenceSync);
   ORD(glFlushMappedBufferRange);
   ORD(glFramebufferTextureLayer);
   ORD(glGenQueries);
   ORD(glGenSamplers);
   ORD(glGenTransformFeedbacks);
   ORD(glGenVertexArrays);
   ORD(glGetActiveUniformBlockiv);
   ORD(glGetActiveUniformBlockName);
   ORD(glGetActiveUniformsiv);
   ORD(glGetBufferParameteri64v);
   ORD(glGetBufferPointerv);
   ORD(glGetFragDataLocation);
   ORD(glGetInteger64i_v);
   ORD(glGetInteger64v);
   ORD(glGetIntegeri_v);
   ORD(glGetInternalformativ);
   ORD(glGetProgramBinary);
   ORD(glGetQueryiv);
   ORD(glGetQueryObjectuiv);
   ORD(glGetSamplerParameterfv);
   ORD(glGetSamplerParameteriv);
   ORD(glGetStringi);
   ORD(glGetSynciv);
   ORD(glGetTransformFeedbackVarying);
   ORD(glGetUniformBlockIndex);
   ORD(glGetUniformIndices);
   ORD(glGetUniformuiv);
   ORD(glGetVertexAttribIiv);
   ORD(glGetVertexAttribIuiv);
   ORD(glInvalidateFramebuffer);
   ORD(glInvalidateSubFramebuffer);
   ORD(glIsQuery);
   ORD(glIsSampler);
   ORD(glIsSync);
   ORD(glIsTransformFeedback);
   ORD(glIsVertexArray);
   ORD(glMapBufferRange);
   ORD(glPauseTransformFeedback);
   ORD(glProgramBinary);
   ORD(glProgramParameteri);
   ORD(glReadBuffer);
   ORD(glRenderbufferStorageMultisample);
   ORD(glResumeTransformFeedback);
   ORD(glSamplerParameterf);
   ORD(glSamplerParameterfv);
   ORD(glSamplerParameteri);
   ORD(glSamplerParameteriv);
   ORD(glTexImage3D);
   ORD(glTexStorage2D);
   ORD(glTexStorage3D);
   ORD(glTexSubImage3D);
   ORD(glTransformFeedbackVaryings);
   ORD(glUniform1ui);
   ORD(glUniform1uiv);
   ORD(glUniform2ui);
   ORD(glUniform2uiv);
   ORD(glUniform3ui);
   ORD(glUniform3uiv);
   ORD(glUniform4ui);
   ORD(glUniform4uiv);
   ORD(glUniformBlockBinding);
   ORD(glUniformMatrix2x3fv);
   ORD(glUniformMatrix3x2fv);
   ORD(glUniformMatrix2x4fv);
   ORD(glUniformMatrix4x2fv);
   ORD(glUniformMatrix3x4fv);
   ORD(glUniformMatrix4x3fv);
   ORD(glUnmapBuffer);
   ORD(glVertexAttribDivisor);
   ORD(glVertexAttribI4i);
   ORD(glVertexAttribI4iv);
   ORD(glVertexAttribI4ui);
   ORD(glVertexAttribI4uiv);
   ORD(glVertexAttribIPointer);
   ORD(glWaitSync);
#undef ORD
}


static Eina_Bool
_evgl_load_gles3_apis(void *dl_handle, Evas_GL_API *funcs)
{
   if (!dl_handle) return EINA_FALSE;

#define ORD(name) \
   funcs->name = dlsym(dl_handle, #name); \
   if (!funcs->name) \
     { \
        WRN("%s symbol not found", #name); \
        return EINA_FALSE; \
     }

   // Used to update extensions
   ORD(glGetString);

   // GLES 3.0 new APIs
   ORD(glBeginQuery);
   ORD(glBeginTransformFeedback);
   ORD(glBindBufferBase);
   ORD(glBindBufferRange);
   ORD(glBindSampler);
   ORD(glBindTransformFeedback);
   ORD(glBindVertexArray);
   ORD(glBlitFramebuffer);
   ORD(glClearBufferfi);
   ORD(glClearBufferfv);
   ORD(glClearBufferiv);
   ORD(glClearBufferuiv);
   ORD(glClientWaitSync);
   ORD(glCompressedTexImage3D);
   ORD(glCompressedTexSubImage3D);
   ORD(glCopyBufferSubData);
   ORD(glCopyTexSubImage3D);
   ORD(glDeleteQueries);
   ORD(glDeleteSamplers);
   ORD(glDeleteSync);
   ORD(glDeleteTransformFeedbacks);
   ORD(glDeleteVertexArrays);
   ORD(glDrawArraysInstanced);
   ORD(glDrawBuffers);
   ORD(glDrawElementsInstanced);
   ORD(glDrawRangeElements);
   ORD(glEndQuery);
   ORD(glEndTransformFeedback);
   ORD(glFenceSync);
   ORD(glFlushMappedBufferRange);
   ORD(glFramebufferTextureLayer);
   ORD(glGenQueries);
   ORD(glGenSamplers);
   ORD(glGenTransformFeedbacks);
   ORD(glGenVertexArrays);
   ORD(glGetActiveUniformBlockiv);
   ORD(glGetActiveUniformBlockName);
   ORD(glGetActiveUniformsiv);
   ORD(glGetBufferParameteri64v);
   ORD(glGetBufferPointerv);
   ORD(glGetFragDataLocation);
   ORD(glGetInteger64i_v);
   ORD(glGetInteger64v);
   ORD(glGetIntegeri_v);
   ORD(glGetInternalformativ);
   ORD(glGetProgramBinary);
   ORD(glGetQueryiv);
   ORD(glGetQueryObjectuiv);
   ORD(glGetSamplerParameterfv);
   ORD(glGetSamplerParameteriv);
   ORD(glGetStringi);
   ORD(glGetSynciv);
   ORD(glGetTransformFeedbackVarying);
   ORD(glGetUniformBlockIndex);
   ORD(glGetUniformIndices);
   ORD(glGetUniformuiv);
   ORD(glGetVertexAttribIiv);
   ORD(glGetVertexAttribIuiv);
   ORD(glInvalidateFramebuffer);
   ORD(glInvalidateSubFramebuffer);
   ORD(glIsQuery);
   ORD(glIsSampler);
   ORD(glIsSync);
   ORD(glIsTransformFeedback);
   ORD(glIsVertexArray);
   ORD(glMapBufferRange);
   ORD(glPauseTransformFeedback);
   ORD(glProgramBinary);
   ORD(glProgramParameteri);
   ORD(glReadBuffer);
   ORD(glRenderbufferStorageMultisample);
   ORD(glResumeTransformFeedback);
   ORD(glSamplerParameterf);
   ORD(glSamplerParameterfv);
   ORD(glSamplerParameteri);
   ORD(glSamplerParameteriv);
   ORD(glTexImage3D);
   ORD(glTexStorage2D);
   ORD(glTexStorage3D);
   ORD(glTexSubImage3D);
   ORD(glTransformFeedbackVaryings);
   ORD(glUniform1ui);
   ORD(glUniform1uiv);
   ORD(glUniform2ui);
   ORD(glUniform2uiv);
   ORD(glUniform3ui);
   ORD(glUniform3uiv);
   ORD(glUniform4ui);
   ORD(glUniform4uiv);
   ORD(glUniformBlockBinding);
   ORD(glUniformMatrix2x3fv);
   ORD(glUniformMatrix3x2fv);
   ORD(glUniformMatrix2x4fv);
   ORD(glUniformMatrix4x2fv);
   ORD(glUniformMatrix3x4fv);
   ORD(glUniformMatrix4x3fv);
   ORD(glUnmapBuffer);
   ORD(glVertexAttribDivisor);
   ORD(glVertexAttribI4i);
   ORD(glVertexAttribI4iv);
   ORD(glVertexAttribI4ui);
   ORD(glVertexAttribI4uiv);
   ORD(glVertexAttribIPointer);
   ORD(glWaitSync);
#undef ORD
   return EINA_TRUE;
}



static Eina_Bool
_evgl_gles3_api_init(void)
{
   static Eina_Bool _initialized = EINA_FALSE;
   if (_initialized) return EINA_TRUE;

   memset(&_gles3_api, 0, sizeof(_gles3_api));

#ifdef GL_GLES
   _gles3_handle = dlopen("libGLESv2.so", RTLD_NOW);
   if (!_gles3_handle) _gles3_handle = dlopen("libGLESv2.so.2.0", RTLD_NOW);
   if (!_gles3_handle) _gles3_handle = dlopen("libGLESv2.so.2", RTLD_NOW);
#else
   _gles3_handle = dlopen("libGL.so", RTLD_NOW);
   if (!_gles3_handle) _gles3_handle = dlopen("libGL.so.4", RTLD_NOW);
   if (!_gles3_handle) _gles3_handle = dlopen("libGL.so.3", RTLD_NOW);
   if (!_gles3_handle) _gles3_handle = dlopen("libGL.so.2", RTLD_NOW);
   if (!_gles3_handle) _gles3_handle = dlopen("libGL.so.1", RTLD_NOW);
   if (!_gles3_handle) _gles3_handle = dlopen("libGL.so.0", RTLD_NOW);
#endif

   if (!_gles3_handle)
     {
        WRN("OpenGL ES 3 was not found on this system. Evas GL will not support GLES 3 contexts.");
        return EINA_FALSE;
     }

   if (!dlsym(_gles3_handle, "glBeginQuery"))
     {
        WRN("OpenGL ES 3 was not found on this system. Evas GL will not support GLES 3 contexts.");
        return EINA_FALSE;
     }

   if (!_evgl_load_gles3_apis(_gles3_handle, &_gles3_api))
     {
        return EINA_FALSE;
     }

   _initialized = EINA_TRUE;
   return EINA_TRUE;
}


void
_evgl_api_gles3_get(Evas_GL_API *funcs, Eina_Bool debug)
{
   if (!_evgl_gles3_api_init())
      return;

   if (debug)
     _debug_gles3_api_get(funcs);
   else
     _normal_gles3_api_get(funcs);

   if (evgl_engine->direct_scissor_off)
     _direct_scissor_off_api_get(funcs);

   return;
}

Evas_GL_API *
_evgl_api_gles3_internal_get(void)
{
   return &_gles3_api;
}

