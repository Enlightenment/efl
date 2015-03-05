#define GL_ERRORS_NODEF 1
#include "evas_gl_core_private.h"
#include "evas_gl_api_ext.h"
#include <dlfcn.h>

#define EVGL_FUNC_BEGIN() \
{ \
   _func_begin_debug(__FUNCTION__); \
}

#define EVGL_FUNC_END() GLERRV(__FUNCTION__)
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
        // Passing the verion -  GLESv2/GLESv3.
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


//-------------------------------------------------------------//
// Open GLES 3.0 APIs


static void
_evgl_glReadBuffer(GLenum mode)
{
   if (!_gles3_api.glReadBuffer)
     return;
   _gles3_api.glReadBuffer(mode);
}

static void
_evgl_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
   if (!_gles3_api.glDrawRangeElements)
     return;
   _gles3_api.glDrawRangeElements(mode, start, end, count, type, indices);
}

static void
_evgl_glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   if (!_gles3_api.glTexImage3D)
     return;
   _gles3_api.glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

static void
_evgl_glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
   if (!_gles3_api.glTexSubImage3D)
     return;
   _gles3_api.glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

static void
_evgl_glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
   if (!_gles3_api.glCopyTexSubImage3D)
     return;
   _gles3_api.glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

static void
_evgl_glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data)
{
   if (!_gles3_api.glCompressedTexImage3D)
     return;
   _gles3_api.glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}

static void
_evgl_glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)
{
   if (!_gles3_api.glCompressedTexSubImage3D)
     return;
   _gles3_api.glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

static void
_evgl_glGenQueries(GLsizei n, GLuint *ids)
{
   if (!_gles3_api.glGenQueries)
     return;
   _gles3_api.glGenQueries(n, ids);
}

static void
_evgl_glDeleteQueries(GLsizei n, const GLuint *ids)
{
   if (!_gles3_api.glDeleteQueries)
     return;
   _gles3_api.glDeleteQueries(n, ids);
}

static GLboolean
 _evgl_glIsQuery(GLuint id)
{
   GLboolean ret;
   if (!_gles3_api.glIsQuery)
     return EINA_FALSE;
   ret = _gles3_api.glIsQuery(id);
   return ret;
}

static void
_evgl_glBeginQuery(GLenum target, GLuint id)
{
   if (!_gles3_api.glBeginQuery)
     return;
   _gles3_api.glBeginQuery(target, id);
}

static void
_evgl_glEndQuery(GLenum target)
{
   if (!_gles3_api.glEndQuery)
     return;
   _gles3_api.glEndQuery(target);
}

static void
_evgl_glGetQueryiv(GLenum target, GLenum pname, GLint *params)
{
   if (!_gles3_api.glGetQueryiv)
     return;
   _gles3_api.glGetQueryiv(target, pname, params);
}

static void
_evgl_glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
   if (!_gles3_api.glGetQueryObjectuiv)
     return;
   _gles3_api.glGetQueryObjectuiv(id, pname, params);
}

static GLboolean
_evgl_glUnmapBuffer(GLenum target)
{
   GLboolean ret;
   if (!_gles3_api.glUnmapBuffer)
     return EINA_FALSE;
   ret = _gles3_api.glUnmapBuffer(target);
   return ret;
}

static void
_evgl_glGetBufferPointerv(GLenum target, GLenum pname, GLvoid **params)
{
   if (!_gles3_api.glGetBufferPointerv)
     return;
   _gles3_api.glGetBufferPointerv(target, pname, params);
}

static void
_evgl_glDrawBuffers(GLsizei n, const GLenum *bufs)
{
   if (!_gles3_api.glDrawBuffers)
     return;
   _gles3_api.glDrawBuffers(n, bufs);
}

static void
_evgl_glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix2x3fv)
     return;
   _gles3_api.glUniformMatrix2x3fv(location, count, transpose, value);
}

static void
_evgl_glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix3x2fv)
     return;
   _gles3_api.glUniformMatrix3x2fv(location, count, transpose, value);
}

static void
_evgl_glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix2x4fv)
     return;
   _gles3_api.glUniformMatrix2x4fv(location, count, transpose, value);
}

static void
_evgl_glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix4x2fv)
     return;
   _gles3_api.glUniformMatrix4x2fv(location, count, transpose, value);
}

static void
_evgl_glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix3x4fv)
     return;
   _gles3_api.glUniformMatrix3x4fv(location, count, transpose, value);
}

static void
_evgl_glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix4x3fv)
     return;
   _gles3_api.glUniformMatrix4x3fv(location, count, transpose, value);
}

static void
_evgl_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
   if (!_gles3_api.glBlitFramebuffer)
     return;
   _gles3_api.glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

static void
_evgl_glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
   if (!_gles3_api.glRenderbufferStorageMultisample)
     return;
   _gles3_api.glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

static void
_evgl_glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
   if (!_gles3_api.glFramebufferTextureLayer)
     return;
   _gles3_api.glFramebufferTextureLayer(target, attachment, texture, level, layer);
}

static void*
_evgl_glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
   void* ret;
   if (!_gles3_api.glMapBufferRange)
     return NULL;
   ret = _gles3_api.glMapBufferRange(target, offset, length, access);
   return ret;
}

static GLsync
_evgl_glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
   GLsync ret;
   if (!_gles3_api.glFlushMappedBufferRange)
     return 0;
   ret = _gles3_api.glFlushMappedBufferRange(target, offset, length);
   return ret;
}

static void
_evgl_glBindVertexArray(GLuint array)
{
   if (!_gles3_api.glBindVertexArray)
     return;
   _gles3_api.glBindVertexArray(array);
}

static void
_evgl_glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
   if (!_gles3_api.glDeleteVertexArrays)
     return;
   _gles3_api.glDeleteVertexArrays(n, arrays);
}

static void
_evgl_glGenVertexArrays(GLsizei n, GLuint *arrays)
{
   if (!_gles3_api.glGenVertexArrays)
     return;
   _gles3_api.glGenVertexArrays(n, arrays);
}

static GLboolean
_evgl_glIsVertexArray(GLuint array)
{
   GLboolean ret;
   if (!_gles3_api.glIsVertexArray)
     return EINA_FALSE;
   ret = _gles3_api.glIsVertexArray(array);
   return ret;
}

static void
_evgl_glGetIntegeri_v(GLenum target, GLuint index, GLint *data)
{
   if (!_gles3_api.glGetIntegeri_v)
     return;
   _gles3_api.glGetIntegeri_v(target, index, data);
}

static void
_evgl_glBeginTransformFeedback(GLenum primitiveMode)
{
   if (!_gles3_api.glBeginTransformFeedback)
     return;
   _gles3_api.glBeginTransformFeedback(primitiveMode);
}

static void
_evgl_glEndTransformFeedback(void)
{
   if (!_gles3_api.glEndTransformFeedback)
     return;
   _gles3_api.glEndTransformFeedback();
}

static void
_evgl_glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
   if (!_gles3_api.glBindBufferRange)
     return;
   _gles3_api.glBindBufferRange(target, index, buffer, offset, size);
}

static void
_evgl_glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
   if (!_gles3_api.glBindBufferBase)
     return;
   _gles3_api.glBindBufferBase(target, index, buffer);
}

static void
_evgl_glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode)
{
   if (!_gles3_api.glTransformFeedbackVaryings)
     return;
   _gles3_api.glTransformFeedbackVaryings(program, count, varyings, bufferMode);
}

static void
_evgl_glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
   if (!_gles3_api.glGetTransformFeedbackVarying)
     return;
   _gles3_api.glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}

static void
_evgl_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles3_api.glVertexAttribIPointer)
     return;
   _gles3_api.glVertexAttribIPointer(index, size, type, stride, pointer);
}

static void
_evgl_glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
   if (!_gles3_api.glGetVertexAttribIiv)
     return;
   _gles3_api.glGetVertexAttribIiv(index, pname, params);
}

static void
_evgl_glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
   if (!_gles3_api.glGetVertexAttribIuiv)
     return;
   _gles3_api.glGetVertexAttribIuiv(index, pname, params);
}

static void
_evgl_glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
   if (!_gles3_api.glVertexAttribI4i)
     return;
   _gles3_api.glVertexAttribI4i(index, x, y, z,  w);
}

static void
_evgl_glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
   if (!_gles3_api.glVertexAttribI4ui)
     return;
   _gles3_api.glVertexAttribI4ui(index, x, y, z, w);
}

static void
_evgl_glVertexAttribI4iv(GLuint index, const GLint *v)
{
   if (!_gles3_api.glVertexAttribI4iv)
     return;
   _gles3_api.glVertexAttribI4iv(index, v);
}

static void
_evgl_glVertexAttribI4uiv(GLuint index, const GLuint *v)
{
   if (!_gles3_api.glVertexAttribI4uiv)
     return;
   _gles3_api.glVertexAttribI4uiv(index, v);
}

static void
_evgl_glGetUniformuiv(GLuint program, GLint location, GLuint *params)
{
   if (!_gles3_api.glGetUniformuiv)
     return;
   _gles3_api.glGetUniformuiv(program, location, params);
}

static GLint
_evgl_glGetFragDataLocation(GLuint program, const GLchar *name)
{
   GLint ret;
   if (!_gles3_api.glGetFragDataLocation)
     return EVAS_GL_NOT_INITIALIZED;
   ret = _gles3_api.glGetFragDataLocation(program, name);
   return ret;
}

static void
_evgl_glUniform1ui(GLint location, GLuint v0)
{
   if (!_gles3_api.glUniform1ui)
     return;
   _gles3_api.glUniform1ui(location, v0);
}

static void
_evgl_glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
   if (!_gles3_api.glUniform2ui)
     return;
   _gles3_api.glUniform2ui(location, v0, v1);
}

static void
_evgl_glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
   if (!_gles3_api.glUniform3ui)
     return;
   _gles3_api.glUniform3ui(location, v0, v1, v2);
}

static void
_evgl_glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
   if (!_gles3_api.glUniform4ui)
     return;
   _gles3_api.glUniform4ui(location, v0, v1, v2, v3);
}

static void
_evgl_glUniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
   if (!_gles3_api.glUniform1uiv)
     return;
   _gles3_api.glUniform1uiv(location, count, value);
}

static void
_evgl_glUniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
   if (!_gles3_api.glUniform2uiv)
     return;
   _gles3_api.glUniform2uiv(location, count, value);
}

static void
_evgl_glUniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
   if (!_gles3_api.glUniform3uiv)
     return;
   _gles3_api.glUniform3uiv(location, count, value);
}

static void
_evgl_glUniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
   if (!_gles3_api.glUniform4uiv)
     return;
   _gles3_api.glUniform4uiv(location, count, value);
}

static void
_evgl_glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
   if (!_gles3_api.glClearBufferiv)
     return;
   _gles3_api.glClearBufferiv(buffer, drawbuffer, value);
}

static void
_evgl_glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
   if (!_gles3_api.glClearBufferuiv)
     return;
   _gles3_api.glClearBufferuiv(buffer, drawbuffer, value);
}

static void
_evgl_glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
   if (!_gles3_api.glClearBufferfv)
     return;
   _gles3_api.glClearBufferfv(buffer, drawbuffer, value);
}

static void
_evgl_glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
   if (!_gles3_api.glClearBufferfi)
     return;
   _gles3_api.glClearBufferfi(buffer, drawbuffer, depth, stencil);
}

static const GLubyte *
 _evgl_glGetStringi(GLenum name, GLuint index)
{
   const GLubyte *ret;
   if (!_gles3_api.glGetStringi)
     return NULL;
   ret = _gles3_api.glGetStringi(name, index);
   return ret;
}

static void
_evgl_glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
   if (!_gles3_api.glCopyBufferSubData)
     return;
   _gles3_api.glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}

static void
_evgl_glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices)
{
   if (!_gles3_api.glGetUniformIndices)
     return;
   _gles3_api.glGetUniformIndices(program, uniformCount, uniformNames,uniformIndices);
}

static void
_evgl_glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)
{
   if (!_gles3_api.glGetActiveUniformsiv)
     return;
   _gles3_api.glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
}

static GLuint
_evgl_glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName)
{
   GLuint ret;
   if (!_gles3_api.glGetUniformBlockIndex)
     return EVAS_GL_NOT_INITIALIZED;
   ret = _gles3_api.glGetUniformBlockIndex(program, uniformBlockName);
   return ret;
}

static void
_evgl_glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)
{
   if (!_gles3_api.glGetActiveUniformBlockiv)
     return;
   _gles3_api.glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}

static void
_evgl_glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)
{
   if (!_gles3_api.glGetActiveUniformBlockName)
     return;
   _gles3_api.glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

static void
_evgl_glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
   if (!_gles3_api.glUniformBlockBinding)
     return;
   _gles3_api.glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}

static void
_evgl_glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
   if (!_gles3_api.glDrawArraysInstanced)
     return;
   _gles3_api.glDrawArraysInstanced(mode, first, count, instancecount);
}

static void
_evgl_glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount)
{
   if (!_gles3_api.glDrawElementsInstanced)
     return;
   _gles3_api.glDrawElementsInstanced(mode, count, type, indices, instancecount);
}

static GLsync
_evgl_glFenceSync(GLenum condition, GLbitfield flags)
{
   GLsync ret;
   if (!_gles3_api.glFenceSync)
     return 0;
   ret = _gles3_api.glFenceSync(condition, flags);
   return ret;
}

static GLboolean
_evgl_glIsSync(GLsync sync)
{
   GLboolean ret;
   if (!_gles3_api.glIsSync)
     return EINA_FALSE;
   ret = _gles3_api.glIsSync(sync);
   return ret;
}

static void
_evgl_glDeleteSync(GLsync sync)
{
   if (!_gles3_api.glDeleteSync)
     return;
   _gles3_api.glDeleteSync(sync);
}

static GLenum
_evgl_glClientWaitSync(GLsync sync, GLbitfield flags, EvasGLuint64 timeout)
{
   GLenum ret;
   if (!_gles3_api.glClientWaitSync)
     return EVAS_GL_NOT_INITIALIZED;
   ret = _gles3_api.glClientWaitSync(sync, flags, timeout);
   return ret;
}

static void
_evgl_glWaitSync(GLsync sync, GLbitfield flags, EvasGLuint64 timeout)
{
   if (!_gles3_api.glWaitSync)
     return;
   _gles3_api.glWaitSync(sync, flags, timeout);
}

static void
_evgl_glGetInteger64v(GLenum pname, EvasGLint64 *params)
{
   if (!_gles3_api.glGetInteger64v)
     return;
   _gles3_api.glGetInteger64v(pname, params);
}

static void
_evgl_glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
   if (!_gles3_api.glGetSynciv)
     return;
   _gles3_api.glGetSynciv(sync, pname, bufSize, length, values);
}

static void
_evgl_glGetInteger64i_v(GLenum target, GLuint index, EvasGLint64 *data)
{
   if (!_gles3_api.glGetInteger64i_v)
     return;
   _gles3_api.glGetInteger64i_v(target, index, data);
}

static void
_evgl_glGetBufferParameteri64v(GLenum target, GLenum pname, EvasGLint64 *params)
{
   if (!_gles3_api.glGetBufferParameteri64v)
     return;
   _gles3_api.glGetBufferParameteri64v(target, pname, params);
}

static void
_evgl_glGenSamplers(GLsizei count, GLuint *samplers)
{
   if (!_gles3_api.glGenSamplers)
     return;
   _gles3_api.glGenSamplers(count, samplers);
}

static void
_evgl_glDeleteSamplers(GLsizei count, const GLuint *samplers)
{
   if (!_gles3_api.glDeleteSamplers)
     return;
   _gles3_api.glDeleteSamplers(count, samplers);
}

static GLboolean
_evgl_glIsSampler(GLuint sampler)
{
   GLboolean ret;
   if (!_gles3_api.glIsSampler)
     return EINA_FALSE;
   ret = _gles3_api.glIsSampler(sampler);
   return ret;
}

static void
_evgl_glBindSampler(GLuint unit, GLuint sampler)
{
   if (!_gles3_api.glBindSampler)
     return;
   _gles3_api.glBindSampler(unit, sampler);
}

static void
_evgl_glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
   if (!_gles3_api.glSamplerParameteri)
     return;
   _gles3_api.glSamplerParameteri(sampler, pname, param);
}

static void
_evgl_glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param)
{
   if (!_gles3_api.glSamplerParameteriv)
     return;
   _gles3_api.glSamplerParameteriv(sampler, pname, param);
}

static void
_evgl_glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
   if (!_gles3_api.glSamplerParameterf)
     return;
   _gles3_api.glSamplerParameterf(sampler, pname, param);
}

static void
_evgl_glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param)
{
   if (!_gles3_api.glSamplerParameterfv)
     return;
   _gles3_api.glSamplerParameterfv(sampler, pname, param);
}

static void
_evgl_glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params)
{
   if (!_gles3_api.glGetSamplerParameteriv)
     return;
   _gles3_api.glGetSamplerParameteriv(sampler, pname, params);
}

static void
_evgl_glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params)
{
   if (!_gles3_api.glGetSamplerParameterfv)
     return;
   _gles3_api.glGetSamplerParameterfv(sampler, pname, params);
}

static void
_evgl_glVertexAttribDivisor(GLuint index, GLuint divisor)
{
   if (!_gles3_api.glVertexAttribDivisor)
     return;
   _gles3_api.glVertexAttribDivisor(index, divisor);
}

static void
_evgl_glBindTransformFeedback(GLenum target, GLuint id)
{
   if (!_gles3_api.glBindTransformFeedback)
     return;
   _gles3_api.glBindTransformFeedback(target, id);
}

static void
_evgl_glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids)
{
   if (!_gles3_api.glDeleteTransformFeedbacks)
     return;
   _gles3_api.glDeleteTransformFeedbacks(n, ids);
}

static void
_evgl_glGenTransformFeedbacks(GLsizei n, GLuint *ids)
{
   if (!_gles3_api.glGenTransformFeedbacks)
     return;
   _gles3_api.glGenTransformFeedbacks(n, ids);
}

static GLboolean
_evgl_glIsTransformFeedback(GLuint id)
{
   GLboolean ret;
   if (!_gles3_api.glIsTransformFeedback)
     return EINA_FALSE;
   ret = _gles3_api.glIsTransformFeedback(id);
   return ret;
}

static void
_evgl_glPauseTransformFeedback(void)
{
   if (!_gles3_api.glPauseTransformFeedback)
     return;
   _gles3_api.glPauseTransformFeedback();
}

static void
_evgl_glResumeTransformFeedback(void)
{
   if (!_gles3_api.glResumeTransformFeedback)
     return;
   _gles3_api.glResumeTransformFeedback();
}

static void
_evgl_glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary)
{
   if (!_gles3_api.glGetProgramBinary)
     return;
   _gles3_api.glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
}

static void
_evgl_glProgramBinary(GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length)
{
   if (!_gles3_api.glProgramBinary)
     return;
   _gles3_api.glProgramBinary(program, binaryFormat, binary, length);
}

static void
_evgl_glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
   if (!_gles3_api.glProgramParameteri)
     return;
   _gles3_api.glProgramParameteri(program, pname, value);
}

static void
_evgl_glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
   if (!_gles3_api.glInvalidateFramebuffer)
     return;
   _gles3_api.glInvalidateFramebuffer(target, numAttachments, attachments);
}

static void
_evgl_glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
   if (!_gles3_api.glInvalidateSubFramebuffer)
     return;
   _gles3_api.glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
}

static void
_evgl_glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
   if (!_gles3_api.glTexStorage2D)
     return;
   _gles3_api.glTexStorage2D(target, levels, internalformat, width, height);
}

static void
_evgl_glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
   if (!_gles3_api.glTexStorage3D)
     return;
   _gles3_api.glTexStorage3D(target, levels, internalformat, width, height, depth);
}

static void
_evgl_glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params)
{
   if (!_gles3_api.glGetInternalformativ)
     return;
   _gles3_api.glGetInternalformativ(target, internalformat, pname, bufSize, params);
}

//-------------------------------------------------------------//
// Debug Evas GL APIs
//  - GL APIs Overriden for debugging purposes
//-------------------------------------------------------------//

void
_evgld_glActiveTexture(GLenum texture)
{
   EVGL_FUNC_BEGIN();
   glActiveTexture(texture);
   EVGL_FUNC_END();
}

void
_evgld_glAttachShader(GLuint program, GLuint shader)
{
   EVGL_FUNC_BEGIN();
   glAttachShader(program, shader);
   EVGL_FUNC_END();
}

void
_evgld_glBindAttribLocation(GLuint program, GLuint idx, const char* name)
{
   EVGL_FUNC_BEGIN();
   glBindAttribLocation(program, idx, name);
   EVGL_FUNC_END();
}

void
_evgld_glBindBuffer(GLenum target, GLuint buffer)
{
   EVGL_FUNC_BEGIN();
   glBindBuffer(target, buffer);
   EVGL_FUNC_END();
}

void
_evgld_glBindFramebuffer(GLenum target, GLuint framebuffer)
{
   EVGL_FUNC_BEGIN();

   _evgl_glBindFramebuffer(target, framebuffer);
   EVGL_FUNC_END();
}

void
_evgld_glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
   EVGL_FUNC_BEGIN();
   glBindRenderbuffer(target, renderbuffer);
   EVGL_FUNC_END();
}

void
_evgld_glBindTexture(GLenum target, GLuint texture)
{
   EVGL_FUNC_BEGIN();
   glBindTexture(target, texture);
   EVGL_FUNC_END();
}

void
_evgld_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   EVGL_FUNC_BEGIN();
   glBlendColor(red, green, blue, alpha);
   EVGL_FUNC_END();
}

void
_evgld_glBlendEquation(GLenum mode)
{
   EVGL_FUNC_BEGIN();
   glBlendEquation(mode);
   EVGL_FUNC_END();
}

void
_evgld_glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
   EVGL_FUNC_BEGIN();
   glBlendEquationSeparate(modeRGB, modeAlpha);
   EVGL_FUNC_END();
}

void
_evgld_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
   EVGL_FUNC_BEGIN();
   glBlendFunc(sfactor, dfactor);
   EVGL_FUNC_END();
}

void
_evgld_glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
   EVGL_FUNC_BEGIN();
   glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
   EVGL_FUNC_END();
}

void
_evgld_glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
   EVGL_FUNC_BEGIN();
   glBufferData(target, size, data, usage);
   EVGL_FUNC_END();
}

void
_evgld_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
   EVGL_FUNC_BEGIN();
   glBufferSubData(target, offset, size, data);
   EVGL_FUNC_END();
}

GLenum
_evgld_glCheckFramebufferStatus(GLenum target)
{
   GLenum ret = GL_NONE;

   EVGL_FUNC_BEGIN();
   ret = glCheckFramebufferStatus(target);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   EVGL_FUNC_BEGIN();
   _evgl_glClearColor(red, green, blue, alpha);
   EVGL_FUNC_END();
}

void
_evgld_glClearDepthf(GLclampf depth)
{
   EVGL_FUNC_BEGIN();

   _evgl_glClearDepthf(depth);

   EVGL_FUNC_END();
}

void
_evgld_glClearStencil(GLint s)
{
   EVGL_FUNC_BEGIN();
   glClearStencil(s);
   EVGL_FUNC_END();
}

void
_evgld_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
   EVGL_FUNC_BEGIN();
   glColorMask(red, green, blue, alpha);
   EVGL_FUNC_END();
}

void
_evgld_glCompileShader(GLuint shader)
{
   EVGL_FUNC_BEGIN();
   glCompileShader(shader);
   EVGL_FUNC_END();
}

void
_evgld_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
   EVGL_FUNC_BEGIN();
   glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
   EVGL_FUNC_END();
}

void
_evgld_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
   EVGL_FUNC_BEGIN();
   glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
   EVGL_FUNC_END();
}

void
_evgld_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
   EVGL_FUNC_BEGIN();
   glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
   EVGL_FUNC_END();
}

void
_evgld_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_FUNC_BEGIN();
   glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
   EVGL_FUNC_END();
}

GLuint
_evgld_glCreateProgram(void)
{
   GLuint ret = _EVGL_INT_INIT_VALUE;

   EVGL_FUNC_BEGIN();
   ret = glCreateProgram();
   EVGL_FUNC_END();
   return ret;
}

GLuint
_evgld_glCreateShader(GLenum type)
{
   GLuint ret = _EVGL_INT_INIT_VALUE;
   EVGL_FUNC_BEGIN();
   ret = glCreateShader(type);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glCullFace(GLenum mode)
{
   EVGL_FUNC_BEGIN();
   glCullFace(mode);
   EVGL_FUNC_END();
}

void
_evgld_glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
   EVGL_FUNC_BEGIN();
   glDeleteBuffers(n, buffers);
   EVGL_FUNC_END();
}

void
_evgld_glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
   EVGL_FUNC_BEGIN();
   glDeleteFramebuffers(n, framebuffers);
   EVGL_FUNC_END();
}

void
_evgld_glDeleteProgram(GLuint program)
{
   EVGL_FUNC_BEGIN();
   glDeleteProgram(program);
   EVGL_FUNC_END();
}

void
_evgld_glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
   EVGL_FUNC_BEGIN();
   glDeleteRenderbuffers(n, renderbuffers);
   EVGL_FUNC_END();
}

void
_evgld_glDeleteShader(GLuint shader)
{
   EVGL_FUNC_BEGIN();
   glDeleteShader(shader);
   EVGL_FUNC_END();
}

void
_evgld_glDeleteTextures(GLsizei n, const GLuint* textures)
{
   EVGL_FUNC_BEGIN();
   glDeleteTextures(n, textures);
   EVGL_FUNC_END();
}

void
_evgld_glDepthFunc(GLenum func)
{
   EVGL_FUNC_BEGIN();
   glDepthFunc(func);
   EVGL_FUNC_END();
}

void
_evgld_glDepthMask(GLboolean flag)
{
   EVGL_FUNC_BEGIN();
   glDepthMask(flag);
   EVGL_FUNC_END();
}

void
_evgld_glDepthRangef(GLclampf zNear, GLclampf zFar)
{
   EVGL_FUNC_BEGIN();

   _evgl_glDepthRangef(zNear, zFar);

   EVGL_FUNC_END();
}

void
_evgld_glDetachShader(GLuint program, GLuint shader)
{
   EVGL_FUNC_BEGIN();
   glDetachShader(program, shader);
   EVGL_FUNC_END();
}

void
_evgld_glDisableVertexAttribArray(GLuint idx)
{
   EVGL_FUNC_BEGIN();
   glDisableVertexAttribArray(idx);
   EVGL_FUNC_END();
}

void
_evgld_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   EVGL_FUNC_BEGIN();
   glDrawArrays(mode, first, count);
   EVGL_FUNC_END();
}

void
_evgld_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
   EVGL_FUNC_BEGIN();
   glDrawElements(mode, count, type, indices);
   EVGL_FUNC_END();
}

void
_evgld_glEnableVertexAttribArray(GLuint idx)
{
   EVGL_FUNC_BEGIN();
   glEnableVertexAttribArray(idx);
   EVGL_FUNC_END();
}

void
_evgld_glFinish(void)
{
   EVGL_FUNC_BEGIN();
   glFinish();
   EVGL_FUNC_END();
}

void
_evgld_glFlush(void)
{
   EVGL_FUNC_BEGIN();
   glFlush();
   EVGL_FUNC_END();
}

void
_evgld_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
   EVGL_FUNC_BEGIN();
   glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
   EVGL_FUNC_END();
}

void
_evgld_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
   EVGL_FUNC_BEGIN();
   glFramebufferTexture2D(target, attachment, textarget, texture, level);
   EVGL_FUNC_END();
}

void
_evgld_glFrontFace(GLenum mode)
{
   EVGL_FUNC_BEGIN();
   glFrontFace(mode);
   EVGL_FUNC_END();
}

void
_evgld_glGetVertexAttribfv(GLuint idx, GLenum pname, GLfloat* params)
{
   EVGL_FUNC_BEGIN();
   glGetVertexAttribfv(idx, pname, params);

   EVGL_FUNC_END();
}

void
_evgld_glGetVertexAttribiv(GLuint idx, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetVertexAttribiv(idx, pname, params);

   EVGL_FUNC_END();
}

void
_evgld_glGetVertexAttribPointerv(GLuint idx, GLenum pname, void** pointer)
{
   EVGL_FUNC_BEGIN();
   glGetVertexAttribPointerv(idx, pname, pointer);


   EVGL_FUNC_END();
}

void
_evgld_glHint(GLenum target, GLenum mode)
{
   EVGL_FUNC_BEGIN();
   glHint(target, mode);
   EVGL_FUNC_END();
}

void
_evgld_glGenBuffers(GLsizei n, GLuint* buffers)
{
   EVGL_FUNC_BEGIN();
   glGenBuffers(n, buffers);
   EVGL_FUNC_END();
}

void
_evgld_glGenerateMipmap(GLenum target)
{
   EVGL_FUNC_BEGIN();
   glGenerateMipmap(target);
   EVGL_FUNC_END();
}

void
_evgld_glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
   EVGL_FUNC_BEGIN();
   glGenFramebuffers(n, framebuffers);
   EVGL_FUNC_END();
}

void
_evgld_glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
   EVGL_FUNC_BEGIN();
   glGenRenderbuffers(n, renderbuffers);
   EVGL_FUNC_END();
}

void
_evgld_glGenTextures(GLsizei n, GLuint* textures)
{
   EVGL_FUNC_BEGIN();
   glGenTextures(n, textures);
   EVGL_FUNC_END();
}

void
_evgld_glGetActiveAttrib(GLuint program, GLuint idx, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)
{
   EVGL_FUNC_BEGIN();
   glGetActiveAttrib(program, idx, bufsize, length, size, type, name);
   EVGL_FUNC_END();
}

void
_evgld_glGetActiveUniform(GLuint program, GLuint idx, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)
{
   EVGL_FUNC_BEGIN();
   glGetActiveUniform(program, idx, bufsize, length, size, type, name);
   EVGL_FUNC_END();
}

void
_evgld_glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
   EVGL_FUNC_BEGIN();
   glGetAttachedShaders(program, maxcount, count, shaders);
   EVGL_FUNC_END();
}

int
_evgld_glGetAttribLocation(GLuint program, const char* name)
{
   int ret = _EVGL_INT_INIT_VALUE;
   EVGL_FUNC_BEGIN();
   ret = glGetAttribLocation(program, name);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glGetBooleanv(GLenum pname, GLboolean* params)
{
   EVGL_FUNC_BEGIN();
   glGetBooleanv(pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetBufferParameteriv(target, pname, params);
   EVGL_FUNC_END();
}

GLenum
_evgld_glGetError(void)
{
   GLenum ret = GL_NONE;

   EVGL_FUNC_BEGIN();
   ret = glGetError();
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glGetFloatv(GLenum pname, GLfloat* params)
{
   EVGL_FUNC_BEGIN();
   glGetFloatv(pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetProgramiv(program, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, char* infolog)
{
   EVGL_FUNC_BEGIN();
   glGetProgramInfoLog(program, bufsize, length, infolog);
   EVGL_FUNC_END();
}

void
_evgld_glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetRenderbufferParameteriv(target, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetShaderiv(shader, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog)
{
   EVGL_FUNC_BEGIN();
   glGetShaderInfoLog(shader, bufsize, length, infolog);
   EVGL_FUNC_END();
}

void
_evgld_glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
   EVGL_FUNC_BEGIN();

   _evgl_glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);

   EVGL_FUNC_END();
}

void
_evgld_glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, char* source)
{
   EVGL_FUNC_BEGIN();
   glGetShaderSource(shader, bufsize, length, source);
   EVGL_FUNC_END();
}

const GLubyte *
_evgld_glGetString(GLenum name)
{
   const GLubyte *ret = NULL;

   EVGL_FUNC_BEGIN();
   ret = _evgl_glGetString(name);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
   EVGL_FUNC_BEGIN();
   glGetTexParameterfv(target, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetTexParameteriv(target, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
   EVGL_FUNC_BEGIN();
   glGetUniformfv(program, location, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetUniformiv(GLuint program, GLint location, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetUniformiv(program, location, params);
   EVGL_FUNC_END();
}
int
_evgld_glGetUniformLocation(GLuint program, const char* name)
{
   int ret = _EVGL_INT_INIT_VALUE;

   EVGL_FUNC_BEGIN();
   ret = glGetUniformLocation(program, name);
   EVGL_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsBuffer(GLuint buffer)
{
   GLboolean ret = GL_FALSE;

   EVGL_FUNC_BEGIN();
   ret = glIsBuffer(buffer);
   EVGL_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsEnabled(GLenum cap)
{
   GLboolean ret = GL_FALSE;

   EVGL_FUNC_BEGIN();
   ret = glIsEnabled(cap);
   EVGL_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsFramebuffer(GLuint framebuffer)
{
   GLboolean ret = GL_FALSE;

   EVGL_FUNC_BEGIN();
   ret = glIsFramebuffer(framebuffer);
   EVGL_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsProgram(GLuint program)
{
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = glIsProgram(program);
   EVGL_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsRenderbuffer(GLuint renderbuffer)
{
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = glIsRenderbuffer(renderbuffer);
   EVGL_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsShader(GLuint shader)
{
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = glIsShader(shader);
   EVGL_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsTexture(GLuint texture)
{
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = glIsTexture(texture);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glLineWidth(GLfloat width)
{
   EVGL_FUNC_BEGIN();
   glLineWidth(width);
   EVGL_FUNC_END();
}

void
_evgld_glLinkProgram(GLuint program)
{
   EVGL_FUNC_BEGIN();
   glLinkProgram(program);
   EVGL_FUNC_END();
}

void
_evgld_glPixelStorei(GLenum pname, GLint param)
{
   EVGL_FUNC_BEGIN();
   glPixelStorei(pname, param);
   EVGL_FUNC_END();
}

void
_evgld_glPolygonOffset(GLfloat factor, GLfloat units)
{
   EVGL_FUNC_BEGIN();
   glPolygonOffset(factor, units);
   EVGL_FUNC_END();
}

void
_evgld_glReleaseShaderCompiler(void)
{
   EVGL_FUNC_BEGIN();

   _evgl_glReleaseShaderCompiler();

   EVGL_FUNC_END();
}

void
_evgld_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
   EVGL_FUNC_BEGIN();
   glRenderbufferStorage(target, internalformat, width, height);
   EVGL_FUNC_END();
}

void
_evgld_glSampleCoverage(GLclampf value, GLboolean invert)
{
   EVGL_FUNC_BEGIN();
   glSampleCoverage(value, invert);
   EVGL_FUNC_END();
}

void
_evgld_glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length)
{
   EVGL_FUNC_BEGIN();

   _evgl_glShaderBinary(n, shaders, binaryformat, binary, length);

   EVGL_FUNC_END();
}

void
_evgld_glShaderSource(GLuint shader, GLsizei count, const char* const * string, const GLint* length)
{
   EVGL_FUNC_BEGIN();
#ifdef GL_GLES
   glShaderSource(shader, count, (const GLchar * const *) string, length);
#else
   glShaderSource(shader, count, (const GLchar **) string, length);
#endif
   EVGL_FUNC_END();
}

void
_evgld_glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
   EVGL_FUNC_BEGIN();
   glStencilFunc(func, ref, mask);
   EVGL_FUNC_END();
}

void
_evgld_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
   EVGL_FUNC_BEGIN();
   glStencilFuncSeparate(face, func, ref, mask);
   EVGL_FUNC_END();
}

void
_evgld_glStencilMask(GLuint mask)
{
   EVGL_FUNC_BEGIN();
   glStencilMask(mask);
   EVGL_FUNC_END();
}

void
_evgld_glStencilMaskSeparate(GLenum face, GLuint mask)
{
   EVGL_FUNC_BEGIN();
   glStencilMaskSeparate(face, mask);
   EVGL_FUNC_END();
}

void
_evgld_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
   EVGL_FUNC_BEGIN();
   glStencilOp(fail, zfail, zpass);
   EVGL_FUNC_END();
}

void
_evgld_glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
   EVGL_FUNC_BEGIN();
   glStencilOpSeparate(face, fail, zfail, zpass);
   EVGL_FUNC_END();
}

void
_evgld_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
   EVGL_FUNC_BEGIN();
   glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
   EVGL_FUNC_END();
}

void
_evgld_glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
   EVGL_FUNC_BEGIN();
   glTexParameterf(target, pname, param);
   EVGL_FUNC_END();
}

void
_evgld_glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
   EVGL_FUNC_BEGIN();
   glTexParameterfv(target, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glTexParameteri(GLenum target, GLenum pname, GLint param)
{
   EVGL_FUNC_BEGIN();
   glTexParameteri(target, pname, param);
   EVGL_FUNC_END();
}

void
_evgld_glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
   EVGL_FUNC_BEGIN();
   glTexParameteriv(target, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
   EVGL_FUNC_BEGIN();
   glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
   EVGL_FUNC_END();
}

void
_evgld_glUniform1f(GLint location, GLfloat x)
{
   EVGL_FUNC_BEGIN();
   glUniform1f(location, x);
   EVGL_FUNC_END();
}

void
_evgld_glUniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGL_FUNC_BEGIN();
   glUniform1fv(location, count, v);
   EVGL_FUNC_END();
}

void
_evgld_glUniform1i(GLint location, GLint x)
{
   EVGL_FUNC_BEGIN();
   glUniform1i(location, x);
   EVGL_FUNC_END();
}

void
_evgld_glUniform1iv(GLint location, GLsizei count, const GLint* v)
{
   EVGL_FUNC_BEGIN();
   glUniform1iv(location, count, v);
   EVGL_FUNC_END();
}

void
_evgld_glUniform2f(GLint location, GLfloat x, GLfloat y)
{
   EVGL_FUNC_BEGIN();
   glUniform2f(location, x, y);
   EVGL_FUNC_END();
}

void
_evgld_glUniform2fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGL_FUNC_BEGIN();
   glUniform2fv(location, count, v);
   EVGL_FUNC_END();
}

void
_evgld_glUniform2i(GLint location, GLint x, GLint y)
{
   EVGL_FUNC_BEGIN();
   glUniform2i(location, x, y);
   EVGL_FUNC_END();
}

void
_evgld_glUniform2iv(GLint location, GLsizei count, const GLint* v)
{
   EVGL_FUNC_BEGIN();
   glUniform2iv(location, count, v);
   EVGL_FUNC_END();
}

void
_evgld_glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
   EVGL_FUNC_BEGIN();
   glUniform3f(location, x, y, z);
   EVGL_FUNC_END();
}

void
_evgld_glUniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGL_FUNC_BEGIN();
   glUniform3fv(location, count, v);
   EVGL_FUNC_END();
}

void
_evgld_glUniform3i(GLint location, GLint x, GLint y, GLint z)
{
   EVGL_FUNC_BEGIN();
   glUniform3i(location, x, y, z);
   EVGL_FUNC_END();
}

void
_evgld_glUniform3iv(GLint location, GLsizei count, const GLint* v)
{
   EVGL_FUNC_BEGIN();
   glUniform3iv(location, count, v);
   EVGL_FUNC_END();
}

void
_evgld_glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   EVGL_FUNC_BEGIN();
   glUniform4f(location, x, y, z, w);
   EVGL_FUNC_END();
}

void
_evgld_glUniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGL_FUNC_BEGIN();
   glUniform4fv(location, count, v);
   EVGL_FUNC_END();
}

void
_evgld_glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
   EVGL_FUNC_BEGIN();
   glUniform4i(location, x, y, z, w);
   EVGL_FUNC_END();
}

void
_evgld_glUniform4iv(GLint location, GLsizei count, const GLint* v)
{
   EVGL_FUNC_BEGIN();
   glUniform4iv(location, count, v);
   EVGL_FUNC_END();
}

void
_evgld_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
   EVGL_FUNC_BEGIN();
   glUniformMatrix2fv(location, count, transpose, value);
   EVGL_FUNC_END();
}

void
_evgld_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
   EVGL_FUNC_BEGIN();
   glUniformMatrix3fv(location, count, transpose, value);
   EVGL_FUNC_END();
}

void
_evgld_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
   EVGL_FUNC_BEGIN();
   glUniformMatrix4fv(location, count, transpose, value);
   EVGL_FUNC_END();
}

void
_evgld_glUseProgram(GLuint program)
{
   EVGL_FUNC_BEGIN();
   glUseProgram(program);
   EVGL_FUNC_END();
}

void
_evgld_glValidateProgram(GLuint program)
{
   EVGL_FUNC_BEGIN();
   glValidateProgram(program);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttrib1f(GLuint indx, GLfloat x)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib1f(indx, x);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttrib1fv(GLuint indx, const GLfloat* values)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib1fv(indx, values);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib2f(indx, x, y);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttrib2fv(GLuint indx, const GLfloat* values)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib2fv(indx, values);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib3f(indx, x, y, z);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttrib3fv(GLuint indx, const GLfloat* values)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib3fv(indx, values);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib4f(indx, x, y, z, w);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttrib4fv(GLuint indx, const GLfloat* values)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib4fv(indx, values);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr)
{
   EVGL_FUNC_BEGIN();
   glVertexAttribPointer(indx, size, type, normalized, stride, ptr);
   EVGL_FUNC_END();
}

//-------------------------------------------------------------//
// Open GLES 3.0 APIs

void
_evgld_glReadBuffer(GLenum mode)
{
   if (!_gles3_api.glReadBuffer)
     {
        ERR("Can not call glReadBuffer() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glReadBuffer(mode);
   EVGL_FUNC_END();
}

void
_evgld_glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
   if (!_gles3_api.glDrawRangeElements)
     {
        ERR("Can not call glDrawRangeElements() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glDrawRangeElements(mode, start, end, count, type, indices);
   EVGL_FUNC_END();
}

void
_evgld_glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   if (!_gles3_api.glTexImage3D)
     {
        ERR("Can not call glTexImage3D() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glTexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
    EVGL_FUNC_END();
}

void
_evgld_glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
   if (!_gles3_api.glTexSubImage3D)
     {
        ERR("Can not call glTexSubImage3D() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
   EVGL_FUNC_END();
}

void
_evgld_glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
   if (!_gles3_api.glCopyTexSubImage3D)
     {
        ERR("Can not call glCopyTexSubImage3D() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glCopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
   EVGL_FUNC_END();
}

void
_evgld_glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data)
{
   if (!_gles3_api.glCompressedTexImage3D)
     {
        ERR("Can not call glCompressedTexImage3D() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glCompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
   EVGL_FUNC_END();
}

void
_evgld_glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)
{
   if (!_gles3_api.glCompressedTexSubImage3D)
     {
        ERR("Can not call glCompressedTexSubImage3D() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glCompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
   EVGL_FUNC_END();
}

void
_evgld_glGenQueries(GLsizei n, GLuint *ids)
{
   if (!_gles3_api.glGenQueries)
     {
        ERR("Can not call glGenQueries() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGenQueries(n, ids);
   EVGL_FUNC_END();
}

void
_evgld_glDeleteQueries(GLsizei n, const GLuint *ids)
{
   if (!_gles3_api.glDeleteQueries)
     {
        ERR("Can not call glDeleteQueries() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glDeleteQueries(n, ids);
   EVGL_FUNC_END();
}

GLboolean
 _evgld_glIsQuery(GLuint id)
{
   if (!_gles3_api.glIsQuery)
     {
        ERR("Can not call glIsQuery() in this context!");
        return EINA_FALSE;
     }
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glIsQuery(id);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glBeginQuery(GLenum target, GLuint id)
{
   if (!_gles3_api.glBeginQuery)
     {
        ERR("Can not call glBeginQuery() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glBeginQuery(target, id);
   EVGL_FUNC_END();
}

void
_evgld_glEndQuery(GLenum target)
{
   if (!_gles3_api.glEndQuery)
     {
        ERR("Can not call glEndQuery() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glEndQuery(target);
   EVGL_FUNC_END();
}

void
_evgld_glGetQueryiv(GLenum target, GLenum pname, GLint *params)
{
   if (!_gles3_api.glGetQueryiv)
     {
        ERR("Can not call glGetQueryiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetQueryiv(target, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
   if (!_gles3_api.glGetQueryObjectuiv)
     {
        ERR("Can not call glGetQueryObjectuiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetQueryObjectuiv(id, pname, params);
   EVGL_FUNC_END();
}

GLboolean
_evgld_glUnmapBuffer(GLenum target)
{
   if (!_gles3_api.glUnmapBuffer)
     {
        ERR("Can not call glUnmapBuffer() in this context!");
        return EINA_FALSE;
     }
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glUnmapBuffer(target);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glGetBufferPointerv(GLenum target, GLenum pname, GLvoid **params)
{
   if (!_gles3_api.glGetBufferPointerv)
     {
        ERR("Can not call glGetBufferPointerv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetBufferPointerv(target, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glDrawBuffers(GLsizei n, const GLenum *bufs)
{
   if (!_gles3_api.glDrawBuffers)
     {
        ERR("Can not call glDrawBuffers() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glDrawBuffers(n, bufs);
   EVGL_FUNC_END();
}

void
_evgld_glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix2x3fv)
     {
        ERR("Can not call glUniformMatrix2x3fv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniformMatrix2x3fv(location, count, transpose, value);
   EVGL_FUNC_END();
}

void
_evgld_glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix3x2fv)
     {
        ERR("Can not call glUniformMatrix3x2fv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniformMatrix3x2fv(location, count, transpose, value);
   EVGL_FUNC_END();
}

void
_evgld_glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix2x4fv)
     {
        ERR("Can not call glUniformMatrix2x4fv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniformMatrix2x4fv(location, count, transpose, value);
   EVGL_FUNC_END();
}

void
_evgld_glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix4x2fv)
     {
        ERR("Can not call glUniformMatrix4x2fv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniformMatrix4x2fv(location, count, transpose, value);
   EVGL_FUNC_END();
}

void
_evgld_glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix3x4fv)
     {
        ERR("Can not call glUniformMatrix3x4fv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniformMatrix3x4fv(location, count, transpose, value);
   EVGL_FUNC_END();
}

void
_evgld_glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
   if (!_gles3_api.glUniformMatrix4x3fv)
     {
        ERR("Can not call glUniformMatrix4x3fv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniformMatrix4x3fv(location, count, transpose, value);
   EVGL_FUNC_END();
}

void
_evgld_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
   if (!_gles3_api.glBlitFramebuffer)
     {
        ERR("Can not call glBlitFramebuffer() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
   EVGL_FUNC_END();
}

void
_evgld_glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
   if (!_gles3_api.glRenderbufferStorageMultisample)
     {
        ERR("Can not call glRenderbufferStorageMultisample() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glRenderbufferStorageMultisample(target, samples, internalformat, width, height);
   EVGL_FUNC_END();
}

void
_evgld_glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
   if (!_gles3_api.glFramebufferTextureLayer)
     {
        ERR("Can not call glFramebufferTextureLayer() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glFramebufferTextureLayer(target, attachment, texture, level, layer);
   EVGL_FUNC_END();
}

void*
_evgld_glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
   if (!_gles3_api.glMapBufferRange)
     {
        ERR("Can not call glMapBufferRange() in this context!");
        return NULL;
     }
   void* ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glMapBufferRange(target, offset, length, access);
   EVGL_FUNC_END();
   return ret;
}

GLsync
_evgld_glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
   if (!_gles3_api.glFlushMappedBufferRange)
     {
        ERR("Can not call glFlushMappedBufferRange() in this context!");
        return 0;
     }
   GLsync ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glFlushMappedBufferRange(target, offset, length);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glBindVertexArray(GLuint array)
{
   if (!_gles3_api.glBindVertexArray)
     {
        ERR("Can not call glBindVertexArray() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glBindVertexArray(array);
   EVGL_FUNC_END();
}

void
_evgld_glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
   if (!_gles3_api.glDeleteVertexArrays)
     {
        ERR("Can not call glDeleteVertexArrays() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glDeleteVertexArrays(n, arrays);
   EVGL_FUNC_END();
}

void
_evgld_glGenVertexArrays(GLsizei n, GLuint *arrays)
{
   if (!_gles3_api.glGenVertexArrays)
     {
        ERR("Can not call glGenVertexArrays() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGenVertexArrays(n, arrays);
   EVGL_FUNC_END();
}

GLboolean
_evgld_glIsVertexArray(GLuint array)
{
   if (!_gles3_api.glIsVertexArray)
     {
        ERR("Can not call glIsVertexArray() in this context!");
        return EINA_FALSE;
     }
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glIsVertexArray(array);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glGetIntegeri_v(GLenum target, GLuint index, GLint *data)
{
   if (!_gles3_api.glGetIntegeri_v)
     {
        ERR("Can not call glGetIntegeri_v() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetIntegeri_v(target, index, data);
   EVGL_FUNC_END();
}

void
_evgld_glBeginTransformFeedback(GLenum primitiveMode)
{
   if (!_gles3_api.glBeginTransformFeedback)
     {
        ERR("Can not call glBeginTransformFeedback() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glBeginTransformFeedback(primitiveMode);
   EVGL_FUNC_END();
}

void
_evgld_glEndTransformFeedback(void)
{
   if (!_gles3_api.glEndTransformFeedback)
     {
        ERR("Can not call glEndTransformFeedback() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glEndTransformFeedback();
   EVGL_FUNC_END();
}

void
_evgld_glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
   if (!_gles3_api.glBindBufferRange)
     {
        ERR("Can not call glBindBufferRange() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glBindBufferRange(target, index, buffer, offset, size);
   EVGL_FUNC_END();
}

void
_evgld_glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
   if (!_gles3_api.glBindBufferBase)
     {
        ERR("Can not call glBindBufferBase() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glBindBufferBase(target, index, buffer);
   EVGL_FUNC_END();
}

void
_evgld_glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode)
{
   if (!_gles3_api.glTransformFeedbackVaryings)
     {
        ERR("Can not call glTransformFeedbackVaryings() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glTransformFeedbackVaryings(program, count, varyings, bufferMode);
   EVGL_FUNC_END();
}

void
_evgld_glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
   if (!_gles3_api.glGetTransformFeedbackVarying)
     {
        ERR("Can not call glGetTransformFeedbackVarying() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles3_api.glVertexAttribIPointer)
     {
        ERR("Can not call glVertexAttribIPointer() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glVertexAttribIPointer(index, size, type, stride, pointer);
   EVGL_FUNC_END();
}

void
_evgld_glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
   if (!_gles3_api.glGetVertexAttribIiv)
     {
        ERR("Can not call glGetVertexAttribIiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetVertexAttribIiv(index, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
   if (!_gles3_api.glGetVertexAttribIuiv)
     {
        ERR("Can not call glGetVertexAttribIuiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetVertexAttribIuiv(index, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
   if (!_gles3_api.glVertexAttribI4i)
     {
        ERR("Can not call glVertexAttribI4i() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glVertexAttribI4i(index, x, y, z,  w);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
   if (!_gles3_api.glVertexAttribI4ui)
     {
        ERR("Can not call glVertexAttribI4ui() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glVertexAttribI4ui(index, x, y, z, w);
    EVGL_FUNC_END();
}

void
_evgld_glVertexAttribI4iv(GLuint index, const GLint *v)
{
   if (!_gles3_api.glVertexAttribI4iv)
     {
        ERR("Can not call glVertexAttribI4iv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glVertexAttribI4iv(index, v);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttribI4uiv(GLuint index, const GLuint *v)
{
   if (!_gles3_api.glVertexAttribI4uiv)
     {
        ERR("Can not call glVertexAttribI4uiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glVertexAttribI4uiv(index, v);
   EVGL_FUNC_END();
}

void
_evgld_glGetUniformuiv(GLuint program, GLint location, GLuint *params)
{
   if (!_gles3_api.glGetUniformuiv)
     {
        ERR("Can not call glGetUniformuiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetUniformuiv(program, location, params);
   EVGL_FUNC_END();
}

GLint
_evgld_glGetFragDataLocation(GLuint program, const GLchar *name)
{
   if (!_gles3_api.glGetFragDataLocation)
     {
        ERR("Can not call glGetFragDataLocation() in this context!");
        return EVAS_GL_NOT_INITIALIZED;
     }
   GLint ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glGetFragDataLocation(program, name);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glUniform1ui(GLint location, GLuint v0)
{
   if (!_gles3_api.glUniform1ui)
     {
       ERR("Can not call glUniform1ui() in this context!");
       return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniform1ui(location, v0);
   EVGL_FUNC_END();
}

void
_evgld_glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
   if (!_gles3_api.glUniform2ui)
     {
        ERR("Can not call glUniform2ui() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniform2ui(location, v0, v1);
   EVGL_FUNC_END();
}

void
_evgld_glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
   if (!_gles3_api.glUniform3ui)
     {
        ERR("Can not call glUniform3ui() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniform3ui(location, v0, v1, v2);
   EVGL_FUNC_END();
}

void
_evgld_glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
   if (!_gles3_api.glUniform4ui)
     {
        ERR("Can not call glUniform4ui() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniform4ui(location, v0, v1, v2, v3);
   EVGL_FUNC_END();
}

void
_evgld_glUniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
   if (!_gles3_api.glUniform1uiv)
     {
        ERR("Can not call glUniform1uiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniform1uiv(location, count, value);
   EVGL_FUNC_END();
}

void
_evgld_glUniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
   if (!_gles3_api.glUniform2uiv)
     {
        ERR("Can not call glUniform2uiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniform2uiv(location, count, value);
   EVGL_FUNC_END();
}

void
_evgld_glUniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
   if (!_gles3_api.glUniform3uiv)
     {
        ERR("Can not call glUniform3uiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniform3uiv(location, count, value);
   EVGL_FUNC_END();
}

void
_evgld_glUniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
   if (!_gles3_api.glUniform4uiv)
     {
        ERR("Can not call glUniform4uiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniform4uiv(location, count, value);
   EVGL_FUNC_END();
}

void
_evgld_glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
   if (!_gles3_api.glClearBufferiv)
     {
        ERR("Can not call glClearBufferiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glClearBufferiv(buffer, drawbuffer, value);
   EVGL_FUNC_END();
}

void
_evgld_glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
   if (!_gles3_api.glClearBufferuiv)
     {
        ERR("Can not call glClearBufferuiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glClearBufferuiv(buffer, drawbuffer, value);
   EVGL_FUNC_END();
}

void
_evgld_glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
   if (!_gles3_api.glClearBufferfv)
     {
        ERR("Can not call glClearBufferfv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glClearBufferfv(buffer, drawbuffer, value);
   EVGL_FUNC_END();
}

void
_evgld_glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
   if (!_gles3_api.glClearBufferfi)
     {
        ERR("Can not call glClearBufferfi() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glClearBufferfi(buffer, drawbuffer, depth, stencil);
   EVGL_FUNC_END();
}

const GLubyte *
 _evgld_glGetStringi(GLenum name, GLuint index)
{
   if (!_gles3_api.glGetStringi)
     {
        ERR("Can not call glGetStringi() in this context!");
        return NULL;
     }
   const GLubyte *ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glGetStringi(name, index);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
   if (!_gles3_api.glCopyBufferSubData)
     {
        ERR("Can not call glCopyBufferSubData() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glCopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
   EVGL_FUNC_END();
}

void
_evgld_glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices)
{
   if (!_gles3_api.glGetUniformIndices)
     {
        ERR("Can not call glGetUniformIndices() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetUniformIndices(program, uniformCount, uniformNames,uniformIndices);
   EVGL_FUNC_END();
}

void
_evgld_glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)
{
   if (!_gles3_api.glGetActiveUniformsiv)
     {
        ERR("Can not call glGetActiveUniformsiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
   EVGL_FUNC_END();
}

GLuint
_evgld_glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName)
{
   if (!_gles3_api.glGetUniformBlockIndex)
     {
        ERR("Can not call glGetUniformBlockIndex() in this context!");
        return EVAS_GL_NOT_INITIALIZED;
     }
   GLuint ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glGetUniformBlockIndex(program, uniformBlockName);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)
{
   if (!_gles3_api.glGetActiveUniformBlockiv)
     {
        ERR("Can not call glGetActiveUniformBlockiv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)
{
   if (!_gles3_api.glGetActiveUniformBlockName)
     {
        ERR("Can not call glGetActiveUniformBlockName() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
   EVGL_FUNC_END();
}

void
_evgld_glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
   if (!_gles3_api.glUniformBlockBinding)
     {
        ERR("Can not call glUniformBlockBinding() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glUniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
   EVGL_FUNC_END();
}

void
_evgld_glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
   if (!_gles3_api.glDrawArraysInstanced)
     {
        ERR("Can not call glDrawArraysInstanced() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glDrawArraysInstanced(mode, first, count, instancecount);
   EVGL_FUNC_END();
}

void
_evgld_glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount)
{
   if (!_gles3_api.glDrawElementsInstanced)
     {
        ERR("Can not call glDrawElementsInstanced() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glDrawElementsInstanced(mode, count, type, indices, instancecount);
   EVGL_FUNC_END();
}

GLsync
_evgld_glFenceSync(GLenum condition, GLbitfield flags)
{
   if (!_gles3_api.glFenceSync)
     {
        ERR("Can not call glFenceSync() in this context!");
        return 0;
     }
   GLsync ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glFenceSync(condition, flags);
   EVGL_FUNC_END();
   return ret;
}

GLboolean
_evgld_glIsSync(GLsync sync)
{
   if (!_gles3_api.glIsSync)
     {
        ERR("Can not call glIsSync() in this context!");
        return EINA_FALSE;
     }
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glIsSync(sync);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glDeleteSync(GLsync sync)
{
   if (!_gles3_api.glDeleteSync)
     {
        ERR("Can not call glDeleteSync() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glDeleteSync(sync);
   EVGL_FUNC_END();
}

GLenum
_evgld_glClientWaitSync(GLsync sync, GLbitfield flags, EvasGLuint64 timeout)
{
   if (!_gles3_api.glClientWaitSync)
     {
        ERR("Can not call glClientWaitSync() in this context!");
        return EVAS_GL_NOT_INITIALIZED;
     }
   GLenum ret;
   EVGL_FUNC_BEGIN();
   ret =  _evgl_glClientWaitSync(sync, flags, timeout);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glWaitSync(GLsync sync, GLbitfield flags, EvasGLuint64 timeout)
{
   if (!_gles3_api.glWaitSync)
     {
        ERR("Can not call glWaitSync() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glWaitSync(sync, flags, timeout);
   EVGL_FUNC_END();
}

void
_evgld_glGetInteger64v(GLenum pname, EvasGLint64 *params)
{
   if (!_gles3_api.glGetInteger64v)
     {
        ERR("Can not call glGetInteger64v() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetInteger64v(pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
   if (!_gles3_api.glGetSynciv)
     {
        ERR("Can not call glGetSynciv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetSynciv(sync, pname, bufSize, length, values);
   EVGL_FUNC_END();
}

void
_evgld_glGetInteger64i_v(GLenum target, GLuint index, EvasGLint64 *data)
{
   if (!_gles3_api.glGetInteger64i_v)
     {
        ERR("Can not call glGetInteger64i_v() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetInteger64i_v(target, index, data);
   EVGL_FUNC_END();
}

void
_evgld_glGetBufferParameteri64v(GLenum target, GLenum pname, EvasGLint64 *params)
{
   if (!_gles3_api.glGetBufferParameteri64v)
     {
        ERR("Can not call glGetBufferParameteri64v() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetBufferParameteri64v(target, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGenSamplers(GLsizei count, GLuint *samplers)
{
   if (!_gles3_api.glGenSamplers)
     {
        ERR("Can not call glGenSamplers() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGenSamplers(count, samplers);
   EVGL_FUNC_END();
}

void
_evgld_glDeleteSamplers(GLsizei count, const GLuint *samplers)
{
   if (!_gles3_api.glDeleteSamplers)
     {
        ERR("Can not call glDeleteSamplers() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glDeleteSamplers(count, samplers);
   EVGL_FUNC_END();
}

GLboolean
_evgld_glIsSampler(GLuint sampler)
{
   if (!_gles3_api.glIsSampler)
     {
        ERR("Can not call glIsSampler() in this context!");
        return EINA_FALSE;
     }
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glIsSampler(sampler);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glBindSampler(GLuint unit, GLuint sampler)
{
   if (!_gles3_api.glBindSampler)
     {
        ERR("Can not call glBindSampler() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glBindSampler(unit, sampler);
   EVGL_FUNC_END();
}

void
_evgld_glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
   if (!_gles3_api.glSamplerParameteri)
     {
        ERR("Can not call glSamplerParameteri() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glSamplerParameteri(sampler, pname, param);
   EVGL_FUNC_END();
}

void
_evgld_glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param)
{
   if (!_gles3_api.glSamplerParameteriv)
     {
        ERR("Can not call glSamplerParameteriv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glSamplerParameteriv(sampler, pname, param);
   EVGL_FUNC_END();
}

void
_evgld_glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
   if (!_gles3_api.glSamplerParameterf)
     {
        ERR("Can not call glSamplerParameterf() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glSamplerParameterf(sampler, pname, param);
   EVGL_FUNC_END();
}

void
_evgld_glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param)
{
   if (!_gles3_api.glSamplerParameterfv)
     {
        ERR("Can not call glSamplerParameterfv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glSamplerParameterfv(sampler, pname, param);
   EVGL_FUNC_END();
}

void
_evgld_glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params)
{
   if (!_gles3_api.glGetSamplerParameteriv)
     {
        ERR("Can not call glGetSamplerParameteriv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetSamplerParameteriv(sampler, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params)
{
   if (!_gles3_api.glGetSamplerParameterfv)
     {
        ERR("Can not call glGetSamplerParameterfv() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetSamplerParameterfv(sampler, pname, params);
   EVGL_FUNC_END();
}

void
_evgld_glVertexAttribDivisor(GLuint index, GLuint divisor)
{
   if (!_gles3_api.glVertexAttribDivisor)
     {
        ERR("Can not call glVertexAttribDivisor() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glVertexAttribDivisor(index, divisor);
   EVGL_FUNC_END();
}

void
_evgld_glBindTransformFeedback(GLenum target, GLuint id)
{
   if (!_gles3_api.glBindTransformFeedback)
     {
        ERR("Can not call glBindTransformFeedback() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glBindTransformFeedback(target, id);
   EVGL_FUNC_END();
}

void
_evgld_glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids)
{
   if (!_gles3_api.glDeleteTransformFeedbacks)
     {
        ERR("Can not call glDeleteTransformFeedbacks() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glDeleteTransformFeedbacks(n, ids);
   EVGL_FUNC_END();
}

void
_evgld_glGenTransformFeedbacks(GLsizei n, GLuint *ids)
{
   if (!_gles3_api.glGenTransformFeedbacks)
     {
        ERR("Can not call glGenTransformFeedbacks() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGenTransformFeedbacks(n, ids);
   EVGL_FUNC_END();
}

GLboolean
_evgld_glIsTransformFeedback(GLuint id)
{
   if (!_gles3_api.glIsTransformFeedback)
     {
        ERR("Can not call glIsTransformFeedback() in this context!");
        return EINA_FALSE;
     }
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = _evgl_glIsTransformFeedback(id);
   EVGL_FUNC_END();
   return ret;
}

void
_evgld_glPauseTransformFeedback(void)
{
   if (!_gles3_api.glPauseTransformFeedback)
     {
        ERR("Can not call glPauseTransformFeedback() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glPauseTransformFeedback();
   EVGL_FUNC_END();
}

void
_evgld_glResumeTransformFeedback(void)
{
   if (!_gles3_api.glResumeTransformFeedback)
     {
        ERR("Can not call glResumeTransformFeedback() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glResumeTransformFeedback();
   EVGL_FUNC_END();
}

void
_evgld_glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary)
{
   if (!_gles3_api.glGetProgramBinary)
     {
        ERR("Can not call glGetProgramBinary() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
   EVGL_FUNC_END();
}

void
_evgld_glProgramBinary(GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length)
{
   if (!_gles3_api.glProgramBinary)
     {
        ERR("Can not call glProgramBinary() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glProgramBinary(program, binaryFormat, binary, length);
   EVGL_FUNC_END();
}

void
_evgld_glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
   if (!_gles3_api.glProgramParameteri)
     {
        ERR("Can not call glProgramParameteri() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glProgramParameteri(program, pname, value);
   EVGL_FUNC_END();
}

void
_evgld_glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
   if (!_gles3_api.glInvalidateFramebuffer)
     {
        ERR("Can not call glInvalidateFramebuffer() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glInvalidateFramebuffer(target, numAttachments, attachments);
   EVGL_FUNC_END();
}

void
_evgld_glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
   if (!_gles3_api.glInvalidateSubFramebuffer)
     {
        ERR("Can not call glInvalidateSubFramebuffer() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glInvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
   EVGL_FUNC_END();
}

void
_evgld_glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
   if (!_gles3_api.glTexStorage2D)
     {
        ERR("Can not call glTexStorage2D() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glTexStorage2D(target, levels, internalformat, width, height);
   EVGL_FUNC_END();
}

void
_evgld_glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
   if (!_gles3_api.glTexStorage3D)
     {
        ERR("Can not call glTexStorage3D() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glTexStorage3D(target, levels, internalformat, width, height, depth);
   EVGL_FUNC_END();
}

void
_evgld_glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params)
{
   if (!_gles3_api.glGetInternalformativ)
     {
        ERR("Can not call glGetInternalformativ() in this context!");
        return;
     }
   EVGL_FUNC_BEGIN();
   _evgl_glGetInternalformativ(target, internalformat, pname, bufSize, params);
   EVGL_FUNC_END();
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
   EVGL_FUNC_BEGIN();

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
   EVGL_FUNC_END();
}
#endif

//-------------------------------------------------------------//


//-------------------------------------------------------------//
// Calls related to Evas GL Direct Rendering
//-------------------------------------------------------------//
static void
_evgld_glClear(GLbitfield mask)
{
   EVGL_FUNC_BEGIN();

   _evgl_glClear(mask);
   EVGL_FUNC_END();
}

static void
_evgld_glEnable(GLenum cap)
{
   EVGL_FUNC_BEGIN();

   _evgl_glEnable(cap);
   EVGL_FUNC_END();
}

static void
_evgld_glDisable(GLenum cap)
{
   EVGL_FUNC_BEGIN();

   _evgl_glDisable(cap);
   EVGL_FUNC_END();
}

void
_evgld_glGetIntegerv(GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   _evgl_glGetIntegerv(pname, params);
   EVGL_FUNC_END();
}

static void
_evgld_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
   EVGL_FUNC_BEGIN();

   _evgl_glReadPixels(x, y, width, height, format, type, pixels);
   EVGL_FUNC_END();
}

static void
_evgld_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_FUNC_BEGIN();

   _evgl_glScissor(x, y, width, height);
   EVGL_FUNC_END();
}

static void
_evgld_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_FUNC_BEGIN();

   _evgl_glViewport(x, y, width, height);
   EVGL_FUNC_END();
}
//-------------------------------------------------------------//

static void
_normal_gl_api_get(Evas_GL_API *funcs)
{
   funcs->version = EVAS_GL_API_VERSION;

#define ORD(f) EVAS_API_OVERRIDE(f, funcs,)
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
//   ORD(glClear);
//   ORD(glClearColor);
//   ORD(glClearDepthf);
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
//   ORD(glDepthRangef);
   ORD(glDetachShader);
//   ORD(glDisable);
   ORD(glDisableVertexAttribArray);
   ORD(glDrawArrays);
   ORD(glDrawElements);
//   ORD(glEnable);
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
//   ORD(glGetIntegerv);
   ORD(glGetProgramiv);
   ORD(glGetProgramInfoLog);
   ORD(glGetRenderbufferParameteriv);
   ORD(glGetShaderiv);
   ORD(glGetShaderInfoLog);
//   ORD(glGetShaderPrecisionFormat);
   ORD(glGetShaderSource);
//   ORD(glGetString);
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
//   ORD(glReadPixels);
//   ORD(glReleaseShaderCompiler);
   ORD(glRenderbufferStorage);
   ORD(glSampleCoverage);
//   ORD(glScissor);
//   ORD(glShaderBinary);
// Deal with double glShaderSource signature
   funcs->glShaderSource = (void (*)(GLuint, GLsizei, const char * const *, const GLint *))glShaderSource;
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
//   ORD(glViewport);

//   ORD(glBindFramebuffer);
   ORD(glBindRenderbuffer);
#undef ORD


#define ORD(f) EVAS_API_OVERRIDE(f, funcs, _evgl_)
   // For Surface FBO
   ORD(glBindFramebuffer);

   // For Direct Rendering
   ORD(glClear);
   ORD(glClearColor);
   ORD(glDisable);
   ORD(glEnable);
   ORD(glGetIntegerv);
   ORD(glGetString);
   ORD(glReadPixels);
   ORD(glScissor);
   ORD(glViewport);

   // GLES 2 Compat for Desktop
   ORD(glClearDepthf);
   ORD(glDepthRangef);
   ORD(glGetShaderPrecisionFormat);
   ORD(glShaderBinary);
   ORD(glReleaseShaderCompiler);

#undef ORD

   evgl_api_ext_get(funcs);
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
_debug_gl_api_get(Evas_GL_API *funcs)
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

   evgl_api_ext_get(funcs);
}

void
_evgl_api_get(Evas_GL_API *funcs, int debug)
{
   if (debug)
      _debug_gl_api_get(funcs);
   else
      _normal_gl_api_get(funcs);

   if (evgl_engine->direct_scissor_off)
      _direct_scissor_off_api_get(funcs);
}

static void
_normal_gles3_api_get(Evas_GL_API *funcs)
{
#define ORD(f) EVAS_API_OVERRIDE(f, funcs,)
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
//   ORD(glClear);
//   ORD(glClearColor);
//   ORD(glClearDepthf);
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
//   ORD(glDepthRangef);
   ORD(glDetachShader);
//   ORD(glDisable);
   ORD(glDisableVertexAttribArray);
   ORD(glDrawArrays);
   ORD(glDrawElements);
//   ORD(glEnable);
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
//   ORD(glGetIntegerv);
   ORD(glGetProgramiv);
   ORD(glGetProgramInfoLog);
   ORD(glGetRenderbufferParameteriv);
   ORD(glGetShaderiv);
   ORD(glGetShaderInfoLog);
//   ORD(glGetShaderPrecisionFormat);
   ORD(glGetShaderSource);
//   ORD(glGetString);
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
//   ORD(glReadPixels);
//   ORD(glReleaseShaderCompiler);
   ORD(glRenderbufferStorage);
   ORD(glSampleCoverage);
//   ORD(glScissor);
//   ORD(glShaderBinary);
// Deal with double glShaderSource signature
   funcs->glShaderSource = (void (*)(GLuint, GLsizei, const char * const *, const GLint *))glShaderSource;
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
//   ORD(glViewport);

//   ORD(glBindFramebuffer);
   ORD(glBindRenderbuffer);
#undef ORD

// GLES 3.0 NEW APIs
#define ORD(name) EVAS_API_OVERRIDE(name, funcs, _evgl_)
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
   ORD(glVertexAttribI4i);
   ORD(glVertexAttribI4iv);
   ORD(glVertexAttribI4ui);
   ORD(glVertexAttribI4uiv);
   ORD(glWaitSync);

#undef ORD


#define ORD(f) EVAS_API_OVERRIDE(f, funcs, _evgl_)

   // General purpose wrapper
   ORD(glGetString);

   // For Surface FBO
   ORD(glBindFramebuffer);

   // For Direct Rendering
   ORD(glClear);
   ORD(glClearColor);
   ORD(glDisable);
   ORD(glEnable);
   ORD(glGetIntegerv);
   ORD(glReadPixels);
   ORD(glScissor);
   ORD(glViewport);

   // GLES 2 Compat for Desktop
   ORD(glClearDepthf);
   ORD(glDepthRangef);
   ORD(glGetShaderPrecisionFormat);
   ORD(glShaderBinary);
   ORD(glReleaseShaderCompiler);

#undef ORD

   evgl_api_gles3_ext_get(funcs);
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
   ORD(glVertexAttribI4i);
   ORD(glVertexAttribI4iv);
   ORD(glVertexAttribI4ui);
   ORD(glVertexAttribI4uiv);
   ORD(glWaitSync);
#undef ORD

   evgl_api_gles3_ext_get(funcs);
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
   ORD(glVertexAttribI4i);
   ORD(glVertexAttribI4iv);
   ORD(glVertexAttribI4ui);
   ORD(glVertexAttribI4uiv);
   ORD(glWaitSync);
#undef ORD
   return EINA_TRUE;
}



static Eina_Bool
_evgl_api_init(void)
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
/*  TODO
   if (!_evgl_api_gles3_ext_init())
     WRN("Could not initialize OpenGL ES 1 extensions yet.");
*/
   _initialized = EINA_TRUE;
   return EINA_TRUE;
}


Eina_Bool
_evgl_api_gles3_get(Evas_GL_API *funcs, Eina_Bool debug)
{
   if(!_evgl_api_init())
      return EINA_FALSE;

   if (debug)
     _debug_gles3_api_get(funcs);
   else
     _normal_gles3_api_get(funcs);

   if (evgl_engine->direct_scissor_off)
     _direct_scissor_off_api_get(funcs);

   return EINA_TRUE;
}

Evas_GL_API *
_evgl_api_gles3_internal_get(void)
{
   return &_gles3_api;
}
