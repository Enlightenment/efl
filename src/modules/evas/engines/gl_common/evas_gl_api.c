#define GL_ERRORS_NODEF 1
#include "evas_gl_core_private.h"
#include "evas_gl_api_ext.h"

#ifndef _WIN32
# include <dlfcn.h>
#endif

#define EVGL_FUNC_BEGIN() if (UNLIKELY(_need_context_restore)) _context_restore()

#define EVGLD_FUNC_BEGIN() \
{ \
   EVGL_FUNC_BEGIN(); \
   _func_begin_debug(__func__); \
}

#define EVGLD_FUNC_END() GLERRV(__func__)
#define _EVGL_INT_INIT_VALUE -3

#define SET_GL_ERROR(gl_error_type) \
    if (ctx->gl_error == GL_NO_ERROR) \
      { \
         ctx->gl_error = glGetError(); \
         if (ctx->gl_error == GL_NO_ERROR) ctx->gl_error = gl_error_type; \
      }

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
   if (ctx->version == EVAS_GL_GLES_2_X)
     {
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
   else if (ctx->version == EVAS_GL_GLES_3_X)
     {
        if (target == GL_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER)
          {
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
                  ctx->current_draw_fbo = 0;

                  if (target == GL_FRAMEBUFFER)
                    ctx->current_read_fbo = 0;
               }
             else
               {
                  if (_evgl_direct_enabled())
                    {
                       if (ctx->current_draw_fbo == 0)
                         {
                            if (rsc->direct.partial.enabled)
                               evgl_direct_partial_render_end();
                         }
                    }

                  glBindFramebuffer(target, framebuffer);

                  // Save this for restore when doing make current
                  ctx->current_draw_fbo = framebuffer;

                  if (target == GL_FRAMEBUFFER)
                    ctx->current_read_fbo = framebuffer;
               }
          }
        else if (target == GL_READ_FRAMEBUFFER)
          {
             if (framebuffer==0)
               {
                 if (_evgl_direct_enabled())
                   {
                      glBindFramebuffer(target, 0);
                   }
                 else
                   {
                      glBindFramebuffer(target, ctx->surface_fbo);
                   }
                 ctx->current_read_fbo = 0;
               }
             else
               {
                 glBindFramebuffer(target, framebuffer);

                 // Save this for restore when doing make current
                 ctx->current_read_fbo = framebuffer;
               }
          }
        else
          {
             glBindFramebuffer(target, framebuffer);
          }
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
_evgl_glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
   EVGL_Context *ctx;
   int i;

   ctx = evas_gl_common_current_context_get();
   if (!ctx)
     {
        ERR("Unable to retrieve Current Context");
        return;
     }

   if (!framebuffers)
     {
        glDeleteFramebuffers(n, framebuffers);
        return;
     }

   if (!_evgl_direct_enabled())
     {
        if (ctx->version == EVAS_GL_GLES_2_X)
          {
             for (i = 0; i < n; i++)
               {
                  if (framebuffers[i] == ctx->current_fbo)
                    {
                       glBindFramebuffer(GL_FRAMEBUFFER, ctx->surface_fbo);
                       ctx->current_fbo = 0;
                       break;
                    }
               }
          }
        else if (ctx->version == EVAS_GL_GLES_3_X)
          {
             for (i = 0; i < n; i++)
               {
                  if (framebuffers[i] == ctx->current_draw_fbo)
                    {
                       glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ctx->surface_fbo);
                       ctx->current_draw_fbo = 0;
                    }

                  if (framebuffers[i] == ctx->current_read_fbo)
                    {
                       glBindFramebuffer(GL_READ_FRAMEBUFFER, ctx->surface_fbo);
                       ctx->current_read_fbo = 0;
                    }
               }
          }
     }

   glDeleteFramebuffers(n, framebuffers);
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

GLenum
_evgl_glGetError(void)
{
   GLenum ret;
   EVGL_Context *ctx = evas_gl_common_current_context_get();

   if (!ctx)
     {
        ERR("No current context set.");
        return GL_NO_ERROR;
     }

   if (ctx->gl_error != GL_NO_ERROR)
     {
        ret = ctx->gl_error;

        //reset error state to GL_NO_ERROR. (EvasGL & Native GL)
        ctx->gl_error = GL_NO_ERROR;
        glGetError();

        return ret;
     }
   else
     {
        return glGetError();
     }
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
        ERR("Unable to retrieve Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrieve Current Context");
        return;
     }

   if (_evgl_direct_enabled())
     {
        if ((!(rsc->current_ctx->current_fbo) && rsc->current_ctx->version == EVAS_GL_GLES_2_X) ||
            (!(rsc->current_ctx->current_draw_fbo) && rsc->current_ctx->version == EVAS_GL_GLES_3_X))
          {
             /* Skip glClear() if clearing with transparent color
              * Note: There will be side effects if the object itself is not
              * marked as having an alpha channel!
              * COPY mode forces the normal behaviour of glClear().
              */
             if (ctx->current_sfc->alpha && !rsc->direct.render_op_copy &&
                 (mask & GL_COLOR_BUFFER_BIT))
               {
                  if (EINA_DBL_EQ(rsc->clear_color.a, 0) &&
                      EINA_DBL_EQ(rsc->clear_color.r, 0) &&
                      EINA_DBL_EQ(rsc->clear_color.g, 0) &&
                      EINA_DBL_EQ(rsc->clear_color.b, 0))
                    {
                       // Skip clear color as we don't want to write black
                       mask &= ~GL_COLOR_BUFFER_BIT;
                    }
                  else if (!EINA_DBL_EQ(rsc->clear_color.a, 1.0))
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
                  if ((!ctx->current_fbo && ctx->version == EVAS_GL_GLES_2_X) ||
                      (!ctx->current_draw_fbo && ctx->version == EVAS_GL_GLES_3_X))
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
             if ((!ctx->current_fbo && ctx->version == EVAS_GL_GLES_2_X) ||
                 (!ctx->current_draw_fbo && ctx->version == EVAS_GL_GLES_3_X))
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
_evgl_glFramebufferParameteri(GLenum target, GLenum pname, GLint param)
{
    EVGL_Resource *rsc;
    EVGL_Context *ctx;

    EINA_SAFETY_ON_NULL_RETURN(_gles3_api.glFramebufferParameteri);

    if (!(rsc=_evgl_tls_resource_get()))
      {
         ERR("Unable to execute GL command. Error retrieving tls");
         return;
      }

    if (!rsc->current_eng)
      {
         ERR("Unable to retrieve Current Engine");
         return;
      }

    ctx = rsc->current_ctx;
    if (!ctx)
      {
         ERR("Unable to retrieve Current Context");
         return;
      }

    if (!_evgl_direct_enabled())
      {
         if (target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER)
           {
              if (ctx->current_draw_fbo == 0)
                {
                   SET_GL_ERROR(GL_INVALID_OPERATION);
                   return;
                }
           }
         else if (target == GL_READ_FRAMEBUFFER)
           {
              if (ctx->current_read_fbo == 0)
                {
                   SET_GL_ERROR(GL_INVALID_OPERATION);
                   return;
                }
           }
      }

   _gles3_api.glFramebufferParameteri(target, pname, param);
}

static void
_evgl_glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    EVGL_Resource *rsc;
    EVGL_Context *ctx;

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

    if (!_evgl_direct_enabled())
      {
         if (ctx->version == EVAS_GL_GLES_2_X)
           {
              if (target == GL_FRAMEBUFFER && ctx->current_fbo == 0)
                {
                   SET_GL_ERROR(GL_INVALID_OPERATION);
                   return;
                }
           }
         else if (ctx->version == EVAS_GL_GLES_3_X)
           {
              if (target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER)
                {
                   if (ctx->current_draw_fbo == 0)
                     {
                        SET_GL_ERROR(GL_INVALID_OPERATION);
                        return;
                     }
                }
              else if (target == GL_READ_FRAMEBUFFER)
                {
                   if (ctx->current_read_fbo == 0)
                     {
                        SET_GL_ERROR(GL_INVALID_OPERATION);
                        return;
                     }
                }
           }
      }

   _gles3_api.glFramebufferTexture(target, attachment, texture, level);
}


static void
_evgl_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    EVGL_Resource *rsc;
    EVGL_Context *ctx;

    if (!(rsc=_evgl_tls_resource_get()))
      {
         ERR("Unable to execute GL command. Error retrieving tls");
         return;
      }

    if (!rsc->current_eng)
      {
         ERR("Unable to retrieve Current Engine");
         return;
      }

    ctx = rsc->current_ctx;
    if (!ctx)
      {
         ERR("Unable to retrieve Current Context");
         return;
      }

    if (!_evgl_direct_enabled())
      {
         if (ctx->version == EVAS_GL_GLES_2_X)
           {
              if (target == GL_FRAMEBUFFER && ctx->current_fbo == 0)
                {
                   SET_GL_ERROR(GL_INVALID_OPERATION);
                   return;
                }
           }
         else if (ctx->version == EVAS_GL_GLES_3_X)
           {
              if (target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER)
                {
                   if (ctx->current_draw_fbo == 0)
                     {
                        SET_GL_ERROR(GL_INVALID_OPERATION);
                        return;
                     }
                }
              else if (target == GL_READ_FRAMEBUFFER)
                {
                   if (ctx->current_read_fbo == 0)
                     {
                        SET_GL_ERROR(GL_INVALID_OPERATION);
                        return;
                     }
                }
           }
      }

   glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

static void
_evgl_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    EVGL_Resource *rsc;
    EVGL_Context *ctx;

    if (!(rsc=_evgl_tls_resource_get()))
      {
         ERR("Unable to execute GL command. Error retrieving tls");
         return;
      }

    if (!rsc->current_eng)
      {
         ERR("Unable to retrieve Current Engine");
         return;
      }

    ctx = rsc->current_ctx;
    if (!ctx)
      {
         ERR("Unable to retrieve Current Context");
         return;
      }

    if (!_evgl_direct_enabled())
      {
         if(ctx->version == EVAS_GL_GLES_2_X)
           {
              if (target == GL_FRAMEBUFFER && ctx->current_fbo == 0)
                {
                   SET_GL_ERROR(GL_INVALID_OPERATION);
                   return;
                }
           }
         else if(ctx->version == EVAS_GL_GLES_3_X)
           {
              if (target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER)
                {
                   if (ctx->current_draw_fbo == 0)
                     {
                        SET_GL_ERROR(GL_INVALID_OPERATION);
                        return;
                     }
                }
              else if (target == GL_READ_FRAMEBUFFER)
                {
                   if (ctx->current_read_fbo == 0)
                     {
                        SET_GL_ERROR(GL_INVALID_OPERATION);
                        return;
                     }
                }
           }
      }

   glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

void
_evgl_glGetFloatv(GLenum pname, GLfloat* params)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;

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
        ERR("Unable to retrieve Current Context");
        return;
     }

   if (_evgl_direct_enabled())
     {
        if (ctx->version == EVAS_GL_GLES_2_X)
          {
             // Only need to handle it if it's directly rendering to the window
             if (!(rsc->current_ctx->current_fbo))
               {
                  if (pname == GL_SCISSOR_BOX)
                    {
                       if (ctx->scissor_updated)
                         {
                            params[0] = (GLfloat)ctx->scissor_coord[0];
                            params[1] = (GLfloat)ctx->scissor_coord[1];
                            params[2] = (GLfloat)ctx->scissor_coord[2];
                            params[3] = (GLfloat)ctx->scissor_coord[3];
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
                       params[0] = (GLfloat)0.0;
                       params[1] = (GLfloat)0.0;
                       params[2] = (GLfloat)rsc->direct.img.w;
                       params[3] = (GLfloat)rsc->direct.img.h;
                       return;
                    }
               }
          }
        else if (ctx->version == EVAS_GL_GLES_3_X)
          {
             // Only need to handle it if it's directly rendering to the window
             if (!(rsc->current_ctx->current_draw_fbo))
               {
                  if (pname == GL_SCISSOR_BOX)
                    {
                       if (ctx->scissor_updated)
                         {
                            params[0] = (GLfloat)ctx->scissor_coord[0];
                            params[1] = (GLfloat)ctx->scissor_coord[1];
                            params[2] = (GLfloat)ctx->scissor_coord[2];
                            params[3] = (GLfloat)ctx->scissor_coord[3];
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
                  if (pname == GL_SCISSOR_BOX) //|| (pname == GL_VIEWPORT))
                    {
                       params[0] = (GLfloat)0.0;
                       params[1] = (GLfloat)0.0;
                       params[2] = (GLfloat)rsc->direct.img.w;
                       params[3] = (GLfloat)rsc->direct.img.h;
                       return;
                    }
               }

             if (pname == GL_NUM_EXTENSIONS)
               {
                  *params = (GLfloat)evgl_api_ext_num_extensions_get(ctx->version);
                  return;
               }
          }
     }
   else
     {
        if (ctx->version == EVAS_GL_GLES_2_X)
          {
             if (pname == GL_FRAMEBUFFER_BINDING)
               {
                  rsc = _evgl_tls_resource_get();
                  ctx = rsc ? rsc->current_ctx : NULL;
                  if (ctx)
                    {
                       *params = (GLfloat)ctx->current_fbo;
                       return;
                    }
               }
          }
        else if (ctx->version == EVAS_GL_GLES_3_X)
          {
             if (pname == GL_DRAW_FRAMEBUFFER_BINDING || pname == GL_FRAMEBUFFER_BINDING)
               {
                  *params = (GLfloat)ctx->current_draw_fbo;
                  return;
               }
             else if (pname == GL_READ_FRAMEBUFFER_BINDING)
               {
                  *params = (GLfloat)ctx->current_read_fbo;
                  return;
               }
             else if (pname == GL_READ_BUFFER)
               {
                  if (ctx->current_read_fbo == 0)
                    {
                       glGetFloatv(pname, params);
                       if (EINA_DBL_EQ(*params, GL_COLOR_ATTACHMENT0))
                         {
                            *params = (GLfloat)GL_BACK;
                            return;
                         }
                    }
               }
             else if (pname == GL_NUM_EXTENSIONS)
               {
                  *params = (GLfloat)evgl_api_ext_num_extensions_get(ctx->version);
                  return;
               }
          }
     }

   glGetFloatv(pname, params);
}

void
_evgl_glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
   EVGL_Context *ctx;

   ctx = evas_gl_common_current_context_get();

   if (!ctx)
     {
        ERR("Unable to retrieve Current Context");
        return;
     }

   if (!_evgl_direct_enabled())
     {
        if (ctx->version == EVAS_GL_GLES_2_X)
          {
             if (ctx->current_fbo == 0)
               {
                  SET_GL_ERROR(GL_INVALID_OPERATION);
                  return;
               }
          }
        else if (ctx->version == EVAS_GL_GLES_3_X)
          {
             if (target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER)
               {
                  if (ctx->current_draw_fbo == 0 && attachment == GL_BACK)
                    {
                       glGetFramebufferAttachmentParameteriv(target, GL_COLOR_ATTACHMENT0, pname, params);
                       return;
                    }
               }
             else if (target == GL_READ_FRAMEBUFFER)
               {
                  if (ctx->current_read_fbo == 0 && attachment == GL_BACK)
                    {
                       glGetFramebufferAttachmentParameteriv(target, GL_COLOR_ATTACHMENT0, pname, params);
                       return;
                    }
               }
          }
     }

   glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

void
_evgl_glGetFramebufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    EVGL_Resource *rsc;
    EVGL_Context *ctx;

    EINA_SAFETY_ON_NULL_RETURN(_gles3_api.glGetFramebufferParameteriv);

    if (!(rsc=_evgl_tls_resource_get()))
      {
         ERR("Unable to execute GL command. Error retrieving tls");
         return;
      }

    if (!rsc->current_eng)
      {
         ERR("Unable to retrieve Current Engine");
         return;
      }

    ctx = rsc->current_ctx;
    if (!ctx)
      {
         ERR("Unable to retrieve Current Context");
         return;
      }

    if (!_evgl_direct_enabled())
      {
         if (target == GL_DRAW_FRAMEBUFFER || target == GL_FRAMEBUFFER)
           {
              if (ctx->current_draw_fbo == 0)
                {
                   SET_GL_ERROR(GL_INVALID_OPERATION);
                   return;
                }
           }
         else if (target == GL_READ_FRAMEBUFFER)
           {
              if (ctx->current_read_fbo == 0)
                {
                   SET_GL_ERROR(GL_INVALID_OPERATION);
                   return;
                }
           }
      }

   _gles3_api.glGetFramebufferParameteriv(target, pname, params);
}
void
_evgl_glGetIntegerv(GLenum pname, GLint* params)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;

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
        ERR("Unable to retrieve Current Context");
        return;
     }

   if (_evgl_direct_enabled())
     {
        if (ctx->version == EVAS_GL_GLES_2_X)
          {
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
        else if (ctx->version == EVAS_GL_GLES_3_X)
          {
             // Only need to handle it if it's directly rendering to the window
             if (!(rsc->current_ctx->current_draw_fbo))
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
                  if (pname == GL_SCISSOR_BOX) //|| (pname == GL_VIEWPORT))
                    {
                       params[0] = 0;
                       params[1] = 0;
                       params[2] = (GLint)rsc->direct.img.w;
                       params[3] = (GLint)rsc->direct.img.h;
                       return;
                    }
               }

             if (pname == GL_NUM_EXTENSIONS)
               {
                  *params = evgl_api_ext_num_extensions_get(ctx->version);
                  return;
               }
          }
     }
   else
     {
        if (ctx->version == EVAS_GL_GLES_2_X)
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
        else if (ctx->version == EVAS_GL_GLES_3_X)
          {
             if (pname == GL_DRAW_FRAMEBUFFER_BINDING || pname == GL_FRAMEBUFFER_BINDING)
               {
                  *params = ctx->current_draw_fbo;
                  return;
               }
             else if (pname == GL_READ_FRAMEBUFFER_BINDING)
               {
                  *params = ctx->current_read_fbo;
                  return;
               }
             else if (pname == GL_READ_BUFFER)
               {
                  if (ctx->current_read_fbo == 0)
                    {
                       glGetIntegerv(pname, params);

                       if (*params == GL_COLOR_ATTACHMENT0)
                         {
                            *params = GL_BACK;
                            return;
                         }
                    }
               }
             else if (pname == GL_NUM_EXTENSIONS)
               {
                  *params = evgl_api_ext_num_extensions_get(ctx->version);
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
   const char *ret, *version_extra;
   EVGL_Resource *rsc;
   EVGL_Context *ctx;

   /* We wrap two values here:
    *
    * VERSION: Since OpenGL ES 3 is not supported yet*, we return OpenGL ES 2.0
    *   The string is not modified on desktop GL (eg. 4.4.0 NVIDIA 343.22)
    *   GLES 3 support is not exposed because apps can't use GLES 3 core
    *   functions yet.
    *
    * EXTENSIONS: This should return only the list of GL extensions supported
    *   by Evas GL. This means as many extensions as possible should be
    *   added to the whitelist.
    *
    * *: GLES 3.0/3.1 is not fully supported... we also have buggy drivers!
    */

   /*
    * Note from Khronos: "If an error is generated, glGetString returns 0."
    * I decided not to call glGetString if there is no context as this is
    * known to cause crashes on certain GL drivers (eg. Nvidia binary blob).
    * --> crash moved to app side if they blindly call strstr()
    */

   /* NOTE: Please modify software_generic/evas_engine.c as well if you change
    *       this function!
    */

   if ((!(rsc = _evgl_tls_resource_get())) || !rsc->current_ctx)
     {
        ERR("Current context is NULL, not calling glGetString");
        // This sets evas_gl_error_get instead of eglGetError...
        evas_gl_common_error_set(EVAS_GL_BAD_CONTEXT);
        return NULL;
     }

   ctx = rsc->current_ctx;
   switch (name)
     {
      case GL_VENDOR:
      case GL_RENDERER:
        // Keep these as-is.
        break;

      case GL_SHADING_LANGUAGE_VERSION:
        ret = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
        if (!ret) return NULL;
#ifdef GL_GLES
        // FIXME: We probably shouldn't wrap anything for EGL
        if (ret[18] != '1')
          {
             // We try not to remove the vendor fluff
             snprintf(_glsl, sizeof(_glsl), "OpenGL ES GLSL ES 1.00 Evas GL (%s)", ret + 18);
             _glsl[sizeof(_glsl) - 1] = '\0';
             return (const GLubyte *) _glsl;
          }
        return (const GLubyte *) ret;
#else
        // Desktop GL, we still keep the official name
        snprintf(_glsl, sizeof(_glsl), "OpenGL ES GLSL ES 1.00 Evas GL (%s)", (char *) ret);
        _version[sizeof(_glsl) - 1] = '\0';
        return (const GLubyte *) _glsl;
#endif

      case GL_VERSION:
        ret = (const char *) glGetString(GL_VERSION);
        if (!ret) return NULL;
#ifdef GL_GLES
        version_extra = ret + 10;
#else
        version_extra = ret;
#endif
        snprintf(_version, sizeof(_version), "OpenGL ES %d.%d Evas GL (%s)",
                 (int) ctx->version, ctx->version_minor, version_extra);
        _version[sizeof(_version) - 1] = '\0';
        return (const GLubyte *) _version;

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

static const GLubyte *
_evgl_glGetStringi(GLenum name, GLuint index)
{
   EVGL_Context *ctx;

   ctx = evas_gl_common_current_context_get();

   if (!ctx)
     {
        ERR("Unable to retrieve Current Context");
        return NULL;
     }

   switch (name)
     {
        case GL_EXTENSIONS:
           if (index < evgl_api_ext_num_extensions_get(ctx->version))
             {
                return (GLubyte *)evgl_api_ext_stringi_get(index, ctx->version);
             }
           else
             SET_GL_ERROR(GL_INVALID_VALUE);
           break;
        default:
           SET_GL_ERROR(GL_INVALID_ENUM);
           break;
     }

   return NULL;
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
        ERR("Unable to retrieve Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrieve Current Context");
        return;
     }

   if (_evgl_direct_enabled())
     {

        if ((!(rsc->current_ctx->current_fbo) && rsc->current_ctx->version == EVAS_GL_GLES_2_X) ||
            (!(rsc->current_ctx->current_read_fbo) && rsc->current_ctx->version == EVAS_GL_GLES_3_X))
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
        ERR("Unable to retrieve Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrieve Current Context");
        return;
     }

   if (_evgl_direct_enabled())
     {
        if ((!(rsc->current_ctx->current_fbo) && rsc->current_ctx->version == EVAS_GL_GLES_2_X) ||
            (!(rsc->current_ctx->current_draw_fbo) && rsc->current_ctx->version == EVAS_GL_GLES_3_X))
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
        ERR("Unable to retrieve Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrieve Current Context");
        return;
     }

   if (_evgl_direct_enabled())
     {
        if ((!(rsc->current_ctx->current_fbo) && rsc->current_ctx->version == EVAS_GL_GLES_2_X) ||
            (!(rsc->current_ctx->current_draw_fbo) && rsc->current_ctx->version == EVAS_GL_GLES_3_X))
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

static void
_evgl_glDrawBuffers(GLsizei n, const GLenum *bufs)
{
    EVGL_Context *ctx;
    Eina_Bool target_is_fbo = EINA_FALSE;
    unsigned int drawbuffer;

    if (!_gles3_api.glDrawBuffers) return;

    ctx = evas_gl_common_current_context_get();
    if (!ctx)
      {
         ERR("Unable to retrieve Current Context");
         return;
      }

    if (!bufs)
      {
         _gles3_api.glDrawBuffers(n, bufs);
         return;
      }

    if (!_evgl_direct_enabled())
      {
         if (ctx->current_draw_fbo == 0)
           target_is_fbo = EINA_TRUE;
      }

    if (target_is_fbo)
      {
         if (n==1)
           {
              if (*bufs == GL_BACK)
                {
                   drawbuffer = GL_COLOR_ATTACHMENT0;
                   _gles3_api.glDrawBuffers(n, &drawbuffer);
                }
              else if ((*bufs & GL_COLOR_ATTACHMENT0) == GL_COLOR_ATTACHMENT0)
                {
                   SET_GL_ERROR(GL_INVALID_OPERATION);
                }
              else
                {
                   _gles3_api.glDrawBuffers(n, bufs);
                }
           }
         else
           {
              SET_GL_ERROR(GL_INVALID_OPERATION);
           }
      }
    else
      {
         _gles3_api.glDrawBuffers(n, bufs);
      }
}

static void
_evgl_glReadBuffer(GLenum src)
{
    EVGL_Context *ctx;
    Eina_Bool target_is_fbo = EINA_FALSE;

    if (!_gles3_api.glReadBuffer) return;

    ctx = evas_gl_common_current_context_get();
    if (!ctx)
      {
         ERR("Unable to retrieve Current Context");
         return;
      }

    if (!_evgl_direct_enabled())
      {
         if (ctx->current_read_fbo == 0)
           target_is_fbo = EINA_TRUE;
      }

    if (target_is_fbo)
      {
         if (src == GL_BACK)
           {
              _gles3_api.glReadBuffer(GL_COLOR_ATTACHMENT0);
           }
         else if((src & GL_COLOR_ATTACHMENT0) == GL_COLOR_ATTACHMENT0)
           {
              SET_GL_ERROR(GL_INVALID_OPERATION);
           }
         else
           {
              _gles3_api.glReadBuffer(src);
           }
      }
    else
      {
         _gles3_api.glReadBuffer(src);
      }
}

//-------------------------------------------------------------//
// Open GLES 2.0 APIs
#define _EVASGL_FUNCTION_PRIVATE_BEGIN(ret, name, param1, param2) \
static ret evgl_##name param1 { \
   EVGL_FUNC_BEGIN(); \
   return _evgl_##name param2; \
}

#define _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID(name, param1, param2) \
static void evgl_##name param1 { \
   EVGL_FUNC_BEGIN(); \
   _evgl_##name param2; \
}

#define _EVASGL_FUNCTION_BEGIN(ret, name, param1, param2) \
static ret evgl_##name param1 { \
   EVGL_FUNC_BEGIN(); \
   return name param2; \
}

#define _EVASGL_FUNCTION_BEGIN_VOID(name, param1, param2) \
static void evgl_##name param1 { \
   EVGL_FUNC_BEGIN(); \
   name param2; \
}

#include "evas_gl_api_def.h"

#undef _EVASGL_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID
#undef _EVASGL_FUNCTION_BEGIN
#undef _EVASGL_FUNCTION_BEGIN_VOID


//-------------------------------------------------------------//
// Open GLES 2.0 APIs DEBUG
#define _EVASGL_FUNCTION_PRIVATE_BEGIN(ret, name, param1, param2) \
static ret _evgld_##name param1 { \
   EVGLD_FUNC_BEGIN(); \
   ret _a = _evgl_##name param2; \
   EVGLD_FUNC_END(); \
   return _a; \
}

#define _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID(name, param1, param2) \
static void _evgld_##name param1 { \
   EVGLD_FUNC_BEGIN(); \
   _evgl_##name param2; \
   EVGLD_FUNC_END(); \
}

#define _EVASGL_FUNCTION_BEGIN(ret, name, param1, param2) \
static ret _evgld_##name param1 { \
   EVGLD_FUNC_BEGIN(); \
   ret _a = name param2; \
   EVGLD_FUNC_END(); \
   return _a; \
}

#define _EVASGL_FUNCTION_BEGIN_VOID(name, param1, param2) \
static void _evgld_##name param1 { \
   EVGLD_FUNC_BEGIN(); \
   name param2; \
   EVGLD_FUNC_END(); \
}

#include "evas_gl_api_def.h"

#undef _EVASGL_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID
#undef _EVASGL_FUNCTION_BEGIN
#undef _EVASGL_FUNCTION_BEGIN_VOID


//-------------------------------------------------------------//
// Open GLES 3.0 APIs
#define _EVASGL_FUNCTION_PRIVATE_BEGIN(ret, name, param1, param2) \
static ret evgl_gles3_##name param1 { \
   EVGL_FUNC_BEGIN(); \
   if (!_gles3_api.name) return (ret)0; \
   return _evgl_##name param2; \
}

#define _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID(name, param1, param2) \
static void evgl_gles3_##name param1 { \
   EVGL_FUNC_BEGIN(); \
   if (!_gles3_api.name) return; \
   _evgl_##name param2; \
}

#define _EVASGL_FUNCTION_BEGIN(ret, name, param1, param2) \
static ret evgl_gles3_##name param1 { \
   EVGL_FUNC_BEGIN(); \
   if (!_gles3_api.name) return (ret)0; \
   return _gles3_api.name param2; \
}

#define _EVASGL_FUNCTION_BEGIN_VOID(name, param1, param2) \
static void evgl_gles3_##name param1 { \
   EVGL_FUNC_BEGIN(); \
   if (!_gles3_api.name) return; \
   _gles3_api.name param2; \
}

#include "evas_gl_api_gles3_def.h"

#undef _EVASGL_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID
#undef _EVASGL_FUNCTION_BEGIN
#undef _EVASGL_FUNCTION_BEGIN_VOID


//-------------------------------------------------------------//
// Open GLES 3.0 APIs DEBUG
#define _EVASGL_FUNCTION_PRIVATE_BEGIN(ret, name, param1, param2) \
static ret _evgld_##name param1 { \
   EVGLD_FUNC_BEGIN(); \
   if (!_gles3_api.name) return (ret)0; \
   ret _a = _evgl_##name param2; \
   EVGLD_FUNC_END(); \
   return _a; \
}

#define _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID(name, param1, param2) \
static void _evgld_##name param1 { \
   EVGLD_FUNC_BEGIN(); \
   if (!_gles3_api.name) return; \
   _evgl_##name param2; \
   EVGLD_FUNC_END(); \
}

#define _EVASGL_FUNCTION_BEGIN(ret, name, param1, param2) \
static ret _evgld_##name param1 { \
   EVGLD_FUNC_BEGIN(); \
   if (!_gles3_api.name) return (ret)0; \
   ret _a = _gles3_api.name param2; \
   EVGLD_FUNC_END(); \
   return _a; \
}

#define _EVASGL_FUNCTION_BEGIN_VOID(name, param1, param2) \
static void _evgld_##name param1 { \
   EVGLD_FUNC_BEGIN(); \
   if (!_gles3_api.name) return; \
   _gles3_api.name param2; \
   EVGLD_FUNC_END(); \
}

#include "evas_gl_api_gles3_def.h"

#undef _EVASGL_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_FUNCTION_PRIVATE_BEGIN_VOID
#undef _EVASGL_FUNCTION_BEGIN
#undef _EVASGL_FUNCTION_BEGIN_VOID

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
_normal_gles3_api_get(Evas_GL_API *funcs, int minor_version)
{
   if (!funcs) return;
   funcs->version = EVAS_GL_API_VERSION;

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
   switch (minor_version)
     {
      case 2:
        ORD(glBlendBarrier);
        ORD(glCopyImageSubData);
        ORD(glDebugMessageControl);
        ORD(glDebugMessageInsert);
        ORD(glDebugMessageCallback);
        ORD(glGetDebugMessageLog);
        ORD(glPushDebugGroup);
        ORD(glPopDebugGroup);
        ORD(glObjectLabel);
        ORD(glGetObjectLabel);
        ORD(glObjectPtrLabel);
        ORD(glGetObjectPtrLabel);
        ORD(glGetPointerv);
        ORD(glEnablei);
        ORD(glDisablei);
        ORD(glBlendEquationi);
        ORD(glBlendEquationSeparatei);
        ORD(glBlendFunci);
        ORD(glBlendFuncSeparatei);
        ORD(glColorMaski);
        ORD(glIsEnabledi);
        ORD(glDrawElementsBaseVertex);
        ORD(glDrawRangeElementsBaseVertex);
        ORD(glDrawElementsInstancedBaseVertex);
        ORD(glFramebufferTexture);
        ORD(glPrimitiveBoundingBox);
        ORD(glGetGraphicsResetStatus);
        ORD(glReadnPixels);
        ORD(glGetnUniformfv);
        ORD(glGetnUniformiv);
        ORD(glGetnUniformuiv);
        ORD(glMinSampleShading);
        ORD(glPatchParameteri);
        ORD(glTexParameterIiv);
        ORD(glTexParameterIuiv);
        ORD(glGetTexParameterIiv);
        ORD(glGetTexParameterIuiv);
        ORD(glSamplerParameterIiv);
        ORD(glSamplerParameterIuiv);
        ORD(glGetSamplerParameterIiv);
        ORD(glGetSamplerParameterIuiv);
        ORD(glTexBuffer);
        ORD(glTexBufferRange);
        ORD(glTexStorage3DMultisample);
        EINA_FALLTHROUGH;
      case 1:
        ORD(glDispatchCompute);
        ORD(glDispatchComputeIndirect);
        ORD(glDrawArraysIndirect);
        ORD(glDrawElementsIndirect);
        ORD(glFramebufferParameteri);
        ORD(glGetFramebufferParameteriv);
        ORD(glGetProgramInterfaceiv);
        ORD(glGetProgramResourceIndex);
        ORD(glGetProgramResourceName);
        ORD(glGetProgramResourceiv);
        ORD(glGetProgramResourceLocation);
        ORD(glUseProgramStages);
        ORD(glActiveShaderProgram);
        ORD(glCreateShaderProgramv);
        ORD(glBindProgramPipeline);
        ORD(glDeleteProgramPipelines);
        ORD(glGenProgramPipelines);
        ORD(glIsProgramPipeline);
        ORD(glGetProgramPipelineiv);
        ORD(glProgramUniform1i);
        ORD(glProgramUniform2i);
        ORD(glProgramUniform3i);
        ORD(glProgramUniform4i);
        ORD(glProgramUniform1ui);
        ORD(glProgramUniform2ui);
        ORD(glProgramUniform3ui);
        ORD(glProgramUniform4ui);
        ORD(glProgramUniform1f);
        ORD(glProgramUniform2f);
        ORD(glProgramUniform3f);
        ORD(glProgramUniform4f);
        ORD(glProgramUniform1iv);
        ORD(glProgramUniform2iv);
        ORD(glProgramUniform3iv);
        ORD(glProgramUniform4iv);
        ORD(glProgramUniform1uiv);
        ORD(glProgramUniform2uiv);
        ORD(glProgramUniform3uiv);
        ORD(glProgramUniform4uiv);
        ORD(glProgramUniform1fv);
        ORD(glProgramUniform2fv);
        ORD(glProgramUniform3fv);
        ORD(glProgramUniform4fv);
        ORD(glProgramUniformMatrix2fv);
        ORD(glProgramUniformMatrix3fv);
        ORD(glProgramUniformMatrix4fv);
        ORD(glProgramUniformMatrix2x3fv);
        ORD(glProgramUniformMatrix3x2fv);
        ORD(glProgramUniformMatrix2x4fv);
        ORD(glProgramUniformMatrix4x2fv);
        ORD(glProgramUniformMatrix3x4fv);
        ORD(glProgramUniformMatrix4x3fv);
        ORD(glValidateProgramPipeline);
        ORD(glGetProgramPipelineInfoLog);
        ORD(glBindImageTexture);
        ORD(glGetBooleani_v);
        ORD(glMemoryBarrier);
        ORD(glMemoryBarrierByRegion);
        ORD(glTexStorage2DMultisample);
        ORD(glGetMultisamplefv);
        ORD(glSampleMaski);
        ORD(glGetTexLevelParameteriv);
        ORD(glGetTexLevelParameterfv);
        ORD(glBindVertexBuffer);
        ORD(glVertexAttribFormat);
        ORD(glVertexAttribIFormat);
        ORD(glVertexAttribBinding);
        ORD(glVertexBindingDivisor);
        EINA_FALLTHROUGH;
      case 0:
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
     }

#undef ORD
}

static void
_debug_gles3_api_get(Evas_GL_API *funcs, int minor_version)
{
   if (!funcs) return;
   funcs->version = EVAS_GL_API_VERSION;

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

   switch (minor_version)
     {
      case 2:
        ORD(glBlendBarrier);
        ORD(glCopyImageSubData);
        ORD(glDebugMessageControl);
        ORD(glDebugMessageInsert);
        ORD(glDebugMessageCallback);
        ORD(glGetDebugMessageLog);
        ORD(glPushDebugGroup);
        ORD(glPopDebugGroup);
        ORD(glObjectLabel);
        ORD(glGetObjectLabel);
        ORD(glObjectPtrLabel);
        ORD(glGetObjectPtrLabel);
        ORD(glGetPointerv);
        ORD(glEnablei);
        ORD(glDisablei);
        ORD(glBlendEquationi);
        ORD(glBlendEquationSeparatei);
        ORD(glBlendFunci);
        ORD(glBlendFuncSeparatei);
        ORD(glColorMaski);
        ORD(glIsEnabledi);
        ORD(glDrawElementsBaseVertex);
        ORD(glDrawRangeElementsBaseVertex);
        ORD(glDrawElementsInstancedBaseVertex);
        ORD(glFramebufferTexture);
        ORD(glPrimitiveBoundingBox);
        ORD(glGetGraphicsResetStatus);
        ORD(glReadnPixels);
        ORD(glGetnUniformfv);
        ORD(glGetnUniformiv);
        ORD(glGetnUniformuiv);
        ORD(glMinSampleShading);
        ORD(glPatchParameteri);
        ORD(glTexParameterIiv);
        ORD(glTexParameterIuiv);
        ORD(glGetTexParameterIiv);
        ORD(glGetTexParameterIuiv);
        ORD(glSamplerParameterIiv);
        ORD(glSamplerParameterIuiv);
        ORD(glGetSamplerParameterIiv);
        ORD(glGetSamplerParameterIuiv);
        ORD(glTexBuffer);
        ORD(glTexBufferRange);
        ORD(glTexStorage3DMultisample);
        EINA_FALLTHROUGH;
      case 1:
        ORD(glDispatchCompute);
        ORD(glDispatchComputeIndirect);
        ORD(glDrawArraysIndirect);
        ORD(glDrawElementsIndirect);
        ORD(glFramebufferParameteri);
        ORD(glGetFramebufferParameteriv);
        ORD(glGetProgramInterfaceiv);
        ORD(glGetProgramResourceIndex);
        ORD(glGetProgramResourceName);
        ORD(glGetProgramResourceiv);
        ORD(glGetProgramResourceLocation);
        ORD(glUseProgramStages);
        ORD(glActiveShaderProgram);
        ORD(glCreateShaderProgramv);
        ORD(glBindProgramPipeline);
        ORD(glDeleteProgramPipelines);
        ORD(glGenProgramPipelines);
        ORD(glIsProgramPipeline);
        ORD(glGetProgramPipelineiv);
        ORD(glProgramUniform1i);
        ORD(glProgramUniform2i);
        ORD(glProgramUniform3i);
        ORD(glProgramUniform4i);
        ORD(glProgramUniform1ui);
        ORD(glProgramUniform2ui);
        ORD(glProgramUniform3ui);
        ORD(glProgramUniform4ui);
        ORD(glProgramUniform1f);
        ORD(glProgramUniform2f);
        ORD(glProgramUniform3f);
        ORD(glProgramUniform4f);
        ORD(glProgramUniform1iv);
        ORD(glProgramUniform2iv);
        ORD(glProgramUniform3iv);
        ORD(glProgramUniform4iv);
        ORD(glProgramUniform1uiv);
        ORD(glProgramUniform2uiv);
        ORD(glProgramUniform3uiv);
        ORD(glProgramUniform4uiv);
        ORD(glProgramUniform1fv);
        ORD(glProgramUniform2fv);
        ORD(glProgramUniform3fv);
        ORD(glProgramUniform4fv);
        ORD(glProgramUniformMatrix2fv);
        ORD(glProgramUniformMatrix3fv);
        ORD(glProgramUniformMatrix4fv);
        ORD(glProgramUniformMatrix2x3fv);
        ORD(glProgramUniformMatrix3x2fv);
        ORD(glProgramUniformMatrix2x4fv);
        ORD(glProgramUniformMatrix4x2fv);
        ORD(glProgramUniformMatrix3x4fv);
        ORD(glProgramUniformMatrix4x3fv);
        ORD(glValidateProgramPipeline);
        ORD(glGetProgramPipelineInfoLog);
        ORD(glBindImageTexture);
        ORD(glGetBooleani_v);
        ORD(glMemoryBarrier);
        ORD(glMemoryBarrierByRegion);
        ORD(glTexStorage2DMultisample);
        ORD(glGetMultisamplefv);
        ORD(glSampleMaski);
        ORD(glGetTexLevelParameteriv);
        ORD(glGetTexLevelParameterfv);
        ORD(glBindVertexBuffer);
        ORD(glVertexAttribFormat);
        ORD(glVertexAttribIFormat);
        ORD(glVertexAttribBinding);
        ORD(glVertexBindingDivisor);
        EINA_FALLTHROUGH;
      case 0:
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
     }

#undef ORD
}


static Eina_Bool
_evgl_load_gles3_apis(void *dl_handle, Evas_GL_API *funcs, int minor_version,
                      void *(*get_proc_address)(const char *))
{
   if (!dl_handle) return EINA_FALSE;
   Eina_Bool ret_value = EINA_FALSE;

#define ORD(name) do { \
   funcs->name = dlsym(dl_handle, #name); \
   if (!funcs->name && get_proc_address) \
     funcs->name = get_proc_address(#name); \
   if (!funcs->name) { \
        WRN("%s symbol not found", #name); \
        return ret_value; \
     } } while (0)

   // Used to update extensions
   ORD(glGetString);

   switch (minor_version)
     {
      case 2:
        ORD(glBlendBarrier);
        ORD(glCopyImageSubData);
        ORD(glDebugMessageControl);
        ORD(glDebugMessageInsert);
        ORD(glDebugMessageCallback);
        ORD(glGetDebugMessageLog);
        ORD(glPushDebugGroup);
        ORD(glPopDebugGroup);
        ORD(glObjectLabel);
        ORD(glGetObjectLabel);
        ORD(glObjectPtrLabel);
        ORD(glGetObjectPtrLabel);
        ORD(glGetPointerv);
        ORD(glEnablei);
        ORD(glDisablei);
        ORD(glBlendEquationi);
        ORD(glBlendEquationSeparatei);
        ORD(glBlendFunci);
        ORD(glBlendFuncSeparatei);
        ORD(glColorMaski);
        ORD(glIsEnabledi);
        ORD(glDrawElementsBaseVertex);
        ORD(glDrawRangeElementsBaseVertex);
        ORD(glDrawElementsInstancedBaseVertex);
        ORD(glFramebufferTexture);
        ORD(glPrimitiveBoundingBox);
        ORD(glGetGraphicsResetStatus);
        ORD(glReadnPixels);
        ORD(glGetnUniformfv);
        ORD(glGetnUniformiv);
        ORD(glGetnUniformuiv);
        ORD(glMinSampleShading);
        ORD(glPatchParameteri);
        ORD(glTexParameterIiv);
        ORD(glTexParameterIuiv);
        ORD(glGetTexParameterIiv);
        ORD(glGetTexParameterIuiv);
        ORD(glSamplerParameterIiv);
        ORD(glSamplerParameterIuiv);
        ORD(glGetSamplerParameterIiv);
        ORD(glGetSamplerParameterIuiv);
        ORD(glTexBuffer);
        ORD(glTexBufferRange);
        ORD(glTexStorage3DMultisample);
        EINA_FALLTHROUGH;
      case 1:
        ORD(glDispatchCompute);
        ORD(glDispatchComputeIndirect);
        ORD(glDrawArraysIndirect);
        ORD(glDrawElementsIndirect);
        ORD(glFramebufferParameteri);
        ORD(glGetFramebufferParameteriv);
        ORD(glGetProgramInterfaceiv);
        ORD(glGetProgramResourceIndex);
        ORD(glGetProgramResourceName);
        ORD(glGetProgramResourceiv);
        ORD(glGetProgramResourceLocation);
        ORD(glUseProgramStages);
        ORD(glActiveShaderProgram);
        ORD(glCreateShaderProgramv);
        ORD(glBindProgramPipeline);
        ORD(glDeleteProgramPipelines);
        ORD(glGenProgramPipelines);
        ORD(glIsProgramPipeline);
        ORD(glGetProgramPipelineiv);
        ORD(glProgramUniform1i);
        ORD(glProgramUniform2i);
        ORD(glProgramUniform3i);
        ORD(glProgramUniform4i);
        ORD(glProgramUniform1ui);
        ORD(glProgramUniform2ui);
        ORD(glProgramUniform3ui);
        ORD(glProgramUniform4ui);
        ORD(glProgramUniform1f);
        ORD(glProgramUniform2f);
        ORD(glProgramUniform3f);
        ORD(glProgramUniform4f);
        ORD(glProgramUniform1iv);
        ORD(glProgramUniform2iv);
        ORD(glProgramUniform3iv);
        ORD(glProgramUniform4iv);
        ORD(glProgramUniform1uiv);
        ORD(glProgramUniform2uiv);
        ORD(glProgramUniform3uiv);
        ORD(glProgramUniform4uiv);
        ORD(glProgramUniform1fv);
        ORD(glProgramUniform2fv);
        ORD(glProgramUniform3fv);
        ORD(glProgramUniform4fv);
        ORD(glProgramUniformMatrix2fv);
        ORD(glProgramUniformMatrix3fv);
        ORD(glProgramUniformMatrix4fv);
        ORD(glProgramUniformMatrix2x3fv);
        ORD(glProgramUniformMatrix3x2fv);
        ORD(glProgramUniformMatrix2x4fv);
        ORD(glProgramUniformMatrix4x2fv);
        ORD(glProgramUniformMatrix3x4fv);
        ORD(glProgramUniformMatrix4x3fv);
        ORD(glValidateProgramPipeline);
        ORD(glGetProgramPipelineInfoLog);
        ORD(glBindImageTexture);
        ORD(glGetBooleani_v);
        ORD(glMemoryBarrier);
        ORD(glMemoryBarrierByRegion);
        ORD(glTexStorage2DMultisample);
        ORD(glGetMultisamplefv);
        ORD(glSampleMaski);
        ORD(glGetTexLevelParameteriv);
        ORD(glGetTexLevelParameterfv);
        ORD(glBindVertexBuffer);
        ORD(glVertexAttribFormat);
        ORD(glVertexAttribIFormat);
        ORD(glVertexAttribBinding);
        ORD(glVertexBindingDivisor);
        EINA_FALLTHROUGH;
      case 0:
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
     }
#undef ORD

   return EINA_TRUE;
}



static Eina_Bool
_evgl_gles3_api_init(int minor_version, void *(*get_proc_address)(const char *))
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

   if (!_evgl_load_gles3_apis(_gles3_handle, &_gles3_api, minor_version, get_proc_address))
     {
        return EINA_FALSE;
     }

   _initialized = EINA_TRUE;
   return EINA_TRUE;
}

void
_evgl_api_gles3_get(Evas_GL_API *funcs, void *(*get_proc_address)(const char *),
                    Eina_Bool debug, int minor_version)
{
   int effective_minor = minor_version;

   if (!_evgl_gles3_api_init(minor_version, get_proc_address))
      return;

   // Hack for NVIDIA. See also evas_gl_core.c:_context_ext_check()
   if (!_gles3_api.glVertexBindingDivisor)
     effective_minor = 0;

   if (debug)
     _debug_gles3_api_get(funcs, effective_minor);
   else
     _normal_gles3_api_get(funcs, effective_minor);

   if (evgl_engine->direct_scissor_off)
     _direct_scissor_off_api_get(funcs);

   return;
}

Evas_GL_API *
_evgl_api_gles3_internal_get(void)
{
   return &_gles3_api;
}

