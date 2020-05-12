#define GL_ERRORS_NODEF 1
#include "evas_gl_core_private.h"

#ifndef _WIN32
# include <dlfcn.h>
#endif

#define EVGL_FUNC_BEGIN() if (UNLIKELY(_need_context_restore)) _context_restore()

#define EVGLD_FUNC_BEGIN() \
{ \
   _func_begin_debug(__func__); \
}

#define EVGLD_FUNC_END() GLERRV(__func__)

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
_evgl_gles1_glAlphaFunc(GLenum func, GLclampf ref)
{
   if (!_gles1_api.glAlphaFunc)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glAlphaFunc(func, ref);
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
   _gles1_api.glClearColor(red, green, blue, alpha);
}

static void
_evgl_gles1_glClearDepthf(GLclampf depth)
{
   if (!_gles1_api.glClearDepthf)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glClearDepthf(depth);
}

static void
_evgl_gles1_glClipPlanef(GLenum plane, const GLfloat *equation)
{
   if (!_gles1_api.glClipPlanef)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glClipPlanef(plane, equation);
}

static void
_evgl_gles1_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
   if (!_gles1_api.glColor4f)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glColor4f(red, green, blue, alpha);
}

static void
_evgl_gles1_glDepthRangef(GLclampf zNear, GLclampf zFar)
{
   if (!_gles1_api.glDepthRangef)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glDepthRangef(zNear, zFar);
}

static void
_evgl_gles1_glFogf(GLenum pname, GLfloat param)
{
   if (!_gles1_api.glFogf)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glFogf(pname, param);
}

static void
_evgl_gles1_glFogfv(GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glFogfv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glFogfv(pname, params);
}

static void
_evgl_gles1_glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
   if (!_gles1_api.glFrustumf)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glFrustumf(left, right, bottom, top, zNear, zFar);
}

static void
_evgl_gles1_glGetClipPlanef(GLenum pname, GLfloat eqn[4])
{
   if (!_gles1_api.glGetClipPlanef)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetClipPlanef(pname, eqn);
}

static void
_evgl_gles1_glGetFloatv(GLenum pname, GLfloat *params)
{
   if (!_gles1_api.glGetFloatv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetFloatv(pname, params);
}

static void
_evgl_gles1_glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
   if (!_gles1_api.glGetLightfv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetLightfv(light, pname, params);
}

static void
_evgl_gles1_glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
   if (!_gles1_api.glGetMaterialfv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetMaterialfv(face, pname, params);
}

static void
_evgl_gles1_glGetTexEnvfv(GLenum env, GLenum pname, GLfloat *params)
{
   if (!_gles1_api.glGetTexEnvfv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetTexEnvfv(env, pname, params);
}

static void
_evgl_gles1_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
   if (!_gles1_api.glGetTexParameterfv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetTexParameterfv(target, pname, params);
}

static void
_evgl_gles1_glLightModelf(GLenum pname, GLfloat param)
{
   if (!_gles1_api.glLightModelf)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLightModelf(pname, param);
}

static void
_evgl_gles1_glLightModelfv(GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glLightModelfv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLightModelfv(pname, params);
}

static void
_evgl_gles1_glLightf(GLenum light, GLenum pname, GLfloat param)
{
   if (!_gles1_api.glLightf)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLightf(light, pname, param);
}

static void
_evgl_gles1_glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glLightfv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLightfv(light, pname, params);
}

static void
_evgl_gles1_glLineWidth(GLfloat width)
{
   if (!_gles1_api.glLineWidth)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLineWidth(width);
}

static void
_evgl_gles1_glLoadMatrixf(const GLfloat *m)
{
   if (!_gles1_api.glLoadMatrixf)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLoadMatrixf(m);
}

static void
_evgl_gles1_glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
   if (!_gles1_api.glMaterialf)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glMaterialf(face, pname, param);
}

static void
_evgl_gles1_glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glMaterialfv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glMaterialfv(face, pname, params);
}

static void
_evgl_gles1_glMultMatrixf(const GLfloat *m)
{
   if (!_gles1_api.glMultMatrixf)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glMultMatrixf(m);
}

static void
_evgl_gles1_glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
   if (!_gles1_api.glMultiTexCoord4f)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glMultiTexCoord4f(target, s, t, r, q);
}

static void
_evgl_gles1_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
   if (!_gles1_api.glNormal3f)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glNormal3f(nx, ny, nz);
}

static void
_evgl_gles1_glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
   if (!_gles1_api.glOrthof)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glOrthof(left, right, bottom, top, zNear, zFar);
}

static void
_evgl_gles1_glPointParameterf(GLenum pname, GLfloat param)
{
   if (!_gles1_api.glPointParameterf)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPointParameterf(pname, param);
}

static void
_evgl_gles1_glPointParameterfv(GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glPointParameterfv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPointParameterfv(pname, params);
}

static void
_evgl_gles1_glPointSize(GLfloat size)
{
   if (!_gles1_api.glPointSize)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPointSize(size);
}

static void
_evgl_gles1_glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles1_api.glPointSizePointerOES)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPointSizePointerOES(type, stride, pointer);
}

static void
_evgl_gles1_glPolygonOffset(GLfloat factor, GLfloat units)
{
   if (!_gles1_api.glPolygonOffset)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPolygonOffset(factor, units);
}

static void
_evgl_gles1_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   if (!_gles1_api.glRotatef)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glRotatef(angle, x, y, z);
}

static void
_evgl_gles1_glScalef(GLfloat x, GLfloat y, GLfloat z)
{
   if (!_gles1_api.glScalef)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glScalef(x, y, z);
}

static void
_evgl_gles1_glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
   if (!_gles1_api.glTexEnvf)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexEnvf(target, pname, param);
}

static void
_evgl_gles1_glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glTexEnvfv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexEnvfv(target, pname, params);
}

static void
_evgl_gles1_glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
   if (!_gles1_api.glTexParameterf)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexParameterf(target, pname, param);
}

static void
_evgl_gles1_glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glTexParameterfv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexParameterfv(target, pname, params);
}

static void
_evgl_gles1_glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
   if (!_gles1_api.glTranslatef)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTranslatef(x, y, z);
}

static void
_evgl_gles1_glActiveTexture(GLenum texture)
{
   if (!_gles1_api.glActiveTexture)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glActiveTexture(texture);
}

static void
_evgl_gles1_glAlphaFuncx(GLenum func, GLclampx ref)
{
   if (!_gles1_api.glAlphaFuncx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glAlphaFuncx(func, ref);
}

static void
_evgl_gles1_glBindBuffer(GLenum target, GLuint buffer)
{
   if (!_gles1_api.glBindBuffer)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glBindBuffer(target, buffer);
}

static void
_evgl_gles1_glBindTexture(GLenum target, GLuint texture)
{
   if (!_gles1_api.glBindTexture)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glBindTexture(target, texture);
}

static void
_evgl_gles1_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
   if (!_gles1_api.glBlendFunc)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glBlendFunc(sfactor, dfactor);
}

static void
_evgl_gles1_glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
   if (!_gles1_api.glBufferData)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glBufferData(target, size, data, usage);
}

static void
_evgl_gles1_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
   if (!_gles1_api.glBufferSubData)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glBufferSubData(target, offset, size, data);
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
        ERR("Unable to retrieve Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrieve Current Context");
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
                  if (EINA_DBL_EQ(rsc->clear_color.a, 0) &&
                      EINA_DBL_EQ(rsc->clear_color.r, 0) &&
                      EINA_DBL_EQ(rsc->clear_color.g, 0) &&
                      EINA_DBL_EQ(rsc->clear_color.b, 0))
                    {
                       // Skip clear color as we don't want to write black
                       mask &= ~GL_COLOR_BUFFER_BIT;
                    }
                  else if (EINA_DBL_EQ(rsc->clear_color.a, 1.0))
                    {
                       // TODO: Draw a rectangle? This will never be the perfect solution though.
                       WRN("glClear() used with a semi-transparent color and direct rendering. "
                           "This will erase the previous contents of the evas!");
                    }
                  if (!mask) return;
               }

             if ((!ctx->direct_scissor))
               {
                  _gles1_api.glEnable(GL_SCISSOR_TEST);
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
                  _gles1_api.glScissor(nc[0], nc[1], nc[2], nc[3]);
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

                  _gles1_api.glScissor(cc[0], cc[1], cc[2], cc[3]);
               }

             _gles1_api.glClear(mask);

             // TODO/FIXME: Restore previous client-side scissors.
          }
        else
          {
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  _gles1_api.glDisable(GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             _gles1_api.glClear(mask);
          }
     }
   else
     {
        if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
          {
             _gles1_api.glDisable(GL_SCISSOR_TEST);
             ctx->direct_scissor = 0;
          }

        _gles1_api.glClear(mask);
     }
}

static void
_evgl_gles1_glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha)
{
   if (!_gles1_api.glClearColorx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glClearColorx(red, green, blue, alpha);
}

static void
_evgl_gles1_glClearDepthx(GLclampx depth)
{
   if (!_gles1_api.glClearDepthx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glClearDepthx(depth);
}

static void
_evgl_gles1_glClearStencil(GLint s)
{
   if (!_gles1_api.glClearStencil)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glClearStencil(s);
}

static void
_evgl_gles1_glClientActiveTexture(GLenum texture)
{
   if (!_gles1_api.glClientActiveTexture)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glClientActiveTexture(texture);
}

static void
_evgl_gles1_glClipPlanex(GLenum plane, const GLfixed *equation)
{
   if (!_gles1_api.glClipPlanex)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glClipPlanex(plane, equation);
}

static void
_evgl_gles1_glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
   if (!_gles1_api.glColor4ub)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glColor4ub(red, green, blue, alpha);
}

static void
_evgl_gles1_glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
   if (!_gles1_api.glColor4x)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glColor4x(red, green, blue, alpha);
}

static void
_evgl_gles1_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
   if (!_gles1_api.glColorMask)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glColorMask(red, green, blue, alpha);
}

static void
_evgl_gles1_glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles1_api.glColorPointer)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glColorPointer(size, type, stride, pointer);
}

static void
_evgl_gles1_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
   if (!_gles1_api.glCompressedTexImage2D)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

static void
_evgl_gles1_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
   if (!_gles1_api.glCompressedTexSubImage2D)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

static void
_evgl_gles1_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
   if (!_gles1_api.glCopyTexImage2D)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

static void
_evgl_gles1_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
   if (!_gles1_api.glCopyTexSubImage2D)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

static void
_evgl_gles1_glCullFace(GLenum mode)
{
   if (!_gles1_api.glCullFace)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glCullFace(mode);
}

static void
_evgl_gles1_glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
   if (!_gles1_api.glDeleteBuffers)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glDeleteBuffers(n, buffers);
}

static void
_evgl_gles1_glDeleteTextures(GLsizei n, const GLuint *textures)
{
   if (!_gles1_api.glDeleteTextures)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glDeleteTextures(n, textures);
}

static void
_evgl_gles1_glDepthFunc(GLenum func)
{
   if (!_gles1_api.glDepthFunc)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glDepthFunc(func);
}

static void
_evgl_gles1_glDepthMask(GLboolean flag)
{
   if (!_gles1_api.glDepthMask)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glDepthMask(flag);
}

static void
_evgl_gles1_glDepthRangex(GLclampx zNear, GLclampx zFar)
{
   if (!_gles1_api.glDepthRangex)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glDepthRangex(zNear, zFar);
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
        ERR("Unable to retrieve Current Context");
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
   _gles1_api.glDisable(cap);
}

static void
_evgl_gles1_glDisableClientState(GLenum array)
{
   if (!_gles1_api.glDisableClientState)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glDisableClientState(array);
}

static void
_evgl_gles1_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   if (!_gles1_api.glDrawArrays)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glDrawArrays(mode, first, count);
}

static void
_evgl_gles1_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
   if (!_gles1_api.glDrawElements)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glDrawElements(mode, count, type, indices);
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
        ERR("Unable to retrieve Current Context");
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
   _gles1_api.glEnable(cap);
}

static void
_evgl_gles1_glEnableClientState(GLenum array)
{
   if (!_gles1_api.glEnableClientState)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glEnableClientState(array);
}

static void
_evgl_gles1_glFinish(void)
{
   if (!_gles1_api.glFinish)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glFinish();
}

static void
_evgl_gles1_glFlush(void)
{
   if (!_gles1_api.glFlush)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glFlush();
}

static void
_evgl_gles1_glFogx(GLenum pname, GLfixed param)
{
   if (!_gles1_api.glFogx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glFogx(pname, param);
}

static void
_evgl_gles1_glFogxv(GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glFogxv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glFogxv(pname, params);
}

static void
_evgl_gles1_glFrontFace(GLenum mode)
{
   if (!_gles1_api.glFrontFace)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glFrontFace(mode);
}

static void
_evgl_gles1_glFrustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
   if (!_gles1_api.glFrustumx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glFrustumx(left, right, bottom, top, zNear, zFar);
}

static void
_evgl_gles1_glGetBooleanv(GLenum pname, GLboolean *params)
{
   if (!_gles1_api.glGetBooleanv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetBooleanv(pname, params);
}

static void
_evgl_gles1_glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
   if (!_gles1_api.glGetBufferParameteriv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetBufferParameteriv(target, pname, params);
}

static void
_evgl_gles1_glGetClipPlanex(GLenum pname, GLfixed eqn[4])
{
   if (!_gles1_api.glGetClipPlanex)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetClipPlanex(pname, eqn);
}

static void
_evgl_gles1_glGenBuffers(GLsizei n, GLuint *buffers)
{
   if (!_gles1_api.glGenBuffers)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGenBuffers(n, buffers);
}

static void
_evgl_gles1_glGenTextures(GLsizei n, GLuint *textures)
{
   if (!_gles1_api.glGenTextures)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGenTextures(n, textures);
}

static GLenum
_evgl_gles1_glGetError(void)
{
   GLenum ret;
   if (!_gles1_api.glGetError)
     return EVAS_GL_NOT_INITIALIZED;
   EVGL_FUNC_BEGIN();
   ret = _gles1_api.glGetError();
   return ret;
}

static void
_evgl_gles1_glGetFixedv(GLenum pname, GLfixed *params)
{
   if (!_gles1_api.glGetFixedv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetFixedv(pname, params);
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
             ERR("Unable to retrieve Current Context");
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
   _gles1_api.glGetIntegerv(pname, params);
}

static void
_evgl_gles1_glGetLightxv(GLenum light, GLenum pname, GLfixed *params)
{
   if (!_gles1_api.glGetLightxv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetLightxv(light, pname, params);
}

static void
_evgl_gles1_glGetMaterialxv(GLenum face, GLenum pname, GLfixed *params)
{
   if (!_gles1_api.glGetMaterialxv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetMaterialxv(face, pname, params);
}

static void
_evgl_gles1_glGetPointerv(GLenum pname, GLvoid **params)
{
   if (!_gles1_api.glGetPointerv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetPointerv(pname, params);
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
        ret = glGetString(GL_VERSION);
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
   return _gles1_api.glGetString(name);
}

static void
_evgl_gles1_glGetTexEnviv(GLenum env, GLenum pname, GLint *params)
{
   if (!_gles1_api.glGetTexEnviv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetTexEnviv(env, pname, params);
}

static void
_evgl_gles1_glGetTexEnvxv(GLenum env, GLenum pname, GLfixed *params)
{
   if (!_gles1_api.glGetTexEnvxv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetTexEnvxv(env, pname, params);
}

static void
_evgl_gles1_glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
   if (!_gles1_api.glGetTexParameteriv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetTexParameteriv(target, pname, params);
}

static void
_evgl_gles1_glGetTexParameterxv(GLenum target, GLenum pname, GLfixed *params)
{
   if (!_gles1_api.glGetTexParameterxv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glGetTexParameterxv(target, pname, params);
}

static void
_evgl_gles1_glHint(GLenum target, GLenum mode)
{
   if (!_gles1_api.glHint)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glHint(target, mode);
}

static GLboolean
_evgl_gles1_glIsBuffer(GLuint buffer)
{
   GLboolean ret;
   if (!_gles1_api.glIsBuffer)
     return EINA_FALSE;
   EVGL_FUNC_BEGIN();
   ret = _gles1_api.glIsBuffer(buffer);
   return ret;
}

static GLboolean
_evgl_gles1_glIsEnabled(GLenum cap)
{
   GLboolean ret;
   if (!_gles1_api.glIsEnabled)
     return EINA_FALSE;
   EVGL_FUNC_BEGIN();
   ret = _gles1_api.glIsEnabled(cap);
   return ret;
}

static GLboolean
_evgl_gles1_glIsTexture(GLuint texture)
{
   GLboolean ret;
   if (!_gles1_api.glIsTexture)
     return EINA_FALSE;
   EVGL_FUNC_BEGIN();
   ret = _gles1_api.glIsTexture(texture);
   return ret;
}

static void
_evgl_gles1_glLightModelx(GLenum pname, GLfixed param)
{
   if (!_gles1_api.glLightModelx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLightModelx(pname, param);
}

static void
_evgl_gles1_glLightModelxv(GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glLightModelxv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLightModelxv(pname, params);
}

static void
_evgl_gles1_glLightx(GLenum light, GLenum pname, GLfixed param)
{
   if (!_gles1_api.glLightx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLightx(light, pname, param);
}

static void
_evgl_gles1_glLightxv(GLenum light, GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glLightxv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLightxv(light, pname, params);
}

static void
_evgl_gles1_glLineWidthx(GLfixed width)
{
   if (!_gles1_api.glLineWidthx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLineWidthx(width);
}

static void
_evgl_gles1_glLoadIdentity(void)
{
   if (!_gles1_api.glLoadIdentity)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLoadIdentity();
}

static void
_evgl_gles1_glLoadMatrixx(const GLfixed *m)
{
   if (!_gles1_api.glLoadMatrixx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLoadMatrixx(m);
}

static void
_evgl_gles1_glLogicOp(GLenum opcode)
{
   if (!_gles1_api.glLogicOp)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glLogicOp(opcode);
}

static void
_evgl_gles1_glMaterialx(GLenum face, GLenum pname, GLfixed param)
{
   if (!_gles1_api.glMaterialx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glMaterialx(face, pname, param);
}

static void
_evgl_gles1_glMaterialxv(GLenum face, GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glMaterialxv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glMaterialxv(face, pname, params);
}

static void
_evgl_gles1_glMatrixMode(GLenum mode)
{
   if (!_gles1_api.glMatrixMode)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glMatrixMode(mode);
}

static void
_evgl_gles1_glMultMatrixx(const GLfixed *m)
{
   if (!_gles1_api.glMultMatrixx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glMultMatrixx(m);
}

static void
_evgl_gles1_glMultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
   if (!_gles1_api.glMultiTexCoord4x)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glMultiTexCoord4x(target, s, t, r, q);
}

static void
_evgl_gles1_glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
   if (!_gles1_api.glNormal3x)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glNormal3x(nx, ny, nz);
}

static void
_evgl_gles1_glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles1_api.glNormalPointer)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glNormalPointer(type, stride, pointer);
}

static void
_evgl_gles1_glOrthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
   if (!_gles1_api.glOrthox)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glOrthox(left, right, bottom, top, zNear, zFar);
}

static void
_evgl_gles1_glPixelStorei(GLenum pname, GLint param)
{
   if (!_gles1_api.glPixelStorei)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPixelStorei(pname, param);
}

static void
_evgl_gles1_glPointParameterx(GLenum pname, GLfixed param)
{
   if (!_gles1_api.glPointParameterx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPointParameterx(pname, param);
}

static void
_evgl_gles1_glPointParameterxv(GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glPointParameterxv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPointParameterxv(pname, params);
}

static void
_evgl_gles1_glPointSizex(GLfixed size)
{
   if (!_gles1_api.glPointSizex)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPointSizex(size);
}

static void
_evgl_gles1_glPolygonOffsetx(GLfixed factor, GLfixed units)
{
   if (!_gles1_api.glPolygonOffsetx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPolygonOffsetx(factor, units);
}

static void
_evgl_gles1_glPopMatrix(void)
{
   if (!_gles1_api.glPopMatrix)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPopMatrix();
}

static void
_evgl_gles1_glPushMatrix(void)
{
   if (!_gles1_api.glPushMatrix)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glPushMatrix();
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
        ERR("Unable to retrieve Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrieve Current Context");
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
             _gles1_api.glReadPixels(nc[0], nc[1], nc[2], nc[3], format, type, pixels);
          }
        else
          {
             _gles1_api.glReadPixels(x, y, width, height, format, type, pixels);
          }
     }
   else
     {
        _gles1_api.glReadPixels(x, y, width, height, format, type, pixels);
     }
}

static void
_evgl_gles1_glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
   if (!_gles1_api.glRotatex)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glRotatex(angle, x, y, z);
}

static void
_evgl_gles1_glSampleCoverage(GLclampf value, GLboolean invert)
{
   if (!_gles1_api.glSampleCoverage)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glSampleCoverage(value, invert);
}

static void
_evgl_gles1_glSampleCoveragex(GLclampx value, GLboolean invert)
{
   if (!_gles1_api.glSampleCoveragex)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glSampleCoveragex(value, invert);
}

static void
_evgl_gles1_glScalex(GLfixed x, GLfixed y, GLfixed z)
{
   if (!_gles1_api.glScalex)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glScalex(x, y, z);
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
        ERR("Unable to retrieve Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrieve Current Context");
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
                  _gles1_api.glDisable(GL_SCISSOR_TEST);
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
             _gles1_api.glScissor(nc[0], nc[1], nc[2], nc[3]);

             ctx->direct_scissor = 0;

             // Check....!!!!
             ctx->scissor_updated = 1;
          }
        else
          {
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  _gles1_api.glDisable(GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             _gles1_api.glScissor(x, y, width, height);

             ctx->scissor_updated = 0;
          }
     }
   else
     {
        if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
          {
             _gles1_api.glDisable(GL_SCISSOR_TEST);
             ctx->direct_scissor = 0;
          }

        _gles1_api.glScissor(x, y, width, height);
     }
}

static void
_evgl_gles1_glShadeModel(GLenum mode)
{
   if (!_gles1_api.glShadeModel)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glShadeModel(mode);
}

static void
_evgl_gles1_glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
   if (!_gles1_api.glStencilFunc)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glStencilFunc(func, ref, mask);
}

static void
_evgl_gles1_glStencilMask(GLuint mask)
{
   if (!_gles1_api.glStencilMask)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glStencilMask(mask);
}

static void
_evgl_gles1_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
   if (!_gles1_api.glStencilOp)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glStencilOp(fail, zfail, zpass);
}

static void
_evgl_gles1_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles1_api.glTexCoordPointer)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexCoordPointer(size, type, stride, pointer);
}

static void
_evgl_gles1_glTexEnvi(GLenum target, GLenum pname, GLint param)
{
   if (!_gles1_api.glTexEnvi)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexEnvi(target, pname, param);
}

static void
_evgl_gles1_glTexEnvx(GLenum target, GLenum pname, GLfixed param)
{
   if (!_gles1_api.glTexEnvx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexEnvx(target, pname, param);
}

static void
_evgl_gles1_glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
   if (!_gles1_api.glTexEnviv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexEnviv(target, pname, params);
}

static void
_evgl_gles1_glTexEnvxv(GLenum target, GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glTexEnvxv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexEnvxv(target, pname, params);
}

static void
_evgl_gles1_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   if (!_gles1_api.glTexImage2D)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

static void
_evgl_gles1_glTexParameteri(GLenum target, GLenum pname, GLint param)
{
   if (!_gles1_api.glTexParameteri)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexParameteri(target, pname, param);
}

static void
_evgl_gles1_glTexParameterx(GLenum target, GLenum pname, GLfixed param)
{
   if (!_gles1_api.glTexParameterx)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexParameterx(target, pname, param);
}

static void
_evgl_gles1_glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
   if (!_gles1_api.glTexParameteriv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexParameteriv(target, pname, params);
}

static void
_evgl_gles1_glTexParameterxv(GLenum target, GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glTexParameterxv)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexParameterxv(target, pname, params);
}

static void
_evgl_gles1_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
   if (!_gles1_api.glTexSubImage2D)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

static void
_evgl_gles1_glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
   if (!_gles1_api.glTranslatex)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glTranslatex(x, y, z);
}

static void
_evgl_gles1_glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles1_api.glVertexPointer)
     return;
   EVGL_FUNC_BEGIN();
   _gles1_api.glVertexPointer(size, type, stride, pointer);
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
        ERR("Unable to retrieve Current Engine");
        return;
     }

   ctx = rsc->current_ctx;
   if (!ctx)
     {
        ERR("Unable to retrieve Current Context");
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
                  _gles1_api.glEnable(GL_SCISSOR_TEST);
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
                  _gles1_api.glScissor(nc[0], nc[1], nc[2], nc[3]);

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
                  _gles1_api.glViewport(nc[0], nc[1], nc[2], nc[3]);
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
                  _gles1_api.glScissor(cc[0], cc[1], cc[2], cc[3]);

                  _gles1_api.glViewport(nc[0], nc[1], nc[2], nc[3]);
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
                  _gles1_api.glDisable(GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             _gles1_api.glViewport(x, y, width, height);
          }
     }
   else
     {
        if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
          {
             _gles1_api.glDisable(GL_SCISSOR_TEST);
             ctx->direct_scissor = 0;
          }

        _gles1_api.glViewport(x, y, width, height);
     }
}

static void
_evgld_gles1_glAlphaFunc(GLenum func, GLclampf ref)
{
   if (!_gles1_api.glAlphaFunc)
     {
        ERR("Can not call glAlphaFunc() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glAlphaFunc(func, ref);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   if (!_gles1_api.glClearColor)
     {
        ERR("Can not call glClearColor() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glClearColor(red, green, blue, alpha);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glClearDepthf(GLclampf depth)
{
   if (!_gles1_api.glClearDepthf)
     {
        ERR("Can not call glClearDepthf() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glClearDepthf(depth);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glClipPlanef(GLenum plane, const GLfloat *equation)
{
   if (!_gles1_api.glClipPlanef)
     {
        ERR("Can not call glClipPlanef() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glClipPlanef(plane, equation);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
   if (!_gles1_api.glColor4f)
     {
        ERR("Can not call glColor4f() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glColor4f(red, green, blue, alpha);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glDepthRangef(GLclampf zNear, GLclampf zFar)
{
   if (!_gles1_api.glDepthRangef)
     {
        ERR("Can not call glDepthRangef() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glDepthRangef(zNear, zFar);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glFogf(GLenum pname, GLfloat param)
{
   if (!_gles1_api.glFogf)
     {
        ERR("Can not call glFogf() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glFogf(pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glFogfv(GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glFogfv)
     {
        ERR("Can not call glFogfv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glFogfv(pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
   if (!_gles1_api.glFrustumf)
     {
        ERR("Can not call glFrustumf() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glFrustumf(left, right, bottom, top, zNear, zFar);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetClipPlanef(GLenum pname, GLfloat eqn[4])
{
   if (!_gles1_api.glGetClipPlanef)
     {
        ERR("Can not call glGetClipPlanef() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetClipPlanef(pname, eqn);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetFloatv(GLenum pname, GLfloat *params)
{
   if (!_gles1_api.glGetFloatv)
     {
        ERR("Can not call glGetFloatv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetFloatv(pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
   if (!_gles1_api.glGetLightfv)
     {
        ERR("Can not call glGetLightfv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetLightfv(light, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
   if (!_gles1_api.glGetMaterialfv)
     {
        ERR("Can not call glGetMaterialfv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetMaterialfv(face, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetTexEnvfv(GLenum env, GLenum pname, GLfloat *params)
{
   if (!_gles1_api.glGetTexEnvfv)
     {
        ERR("Can not call glGetTexEnvfv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetTexEnvfv(env, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
   if (!_gles1_api.glGetTexParameterfv)
     {
        ERR("Can not call glGetTexParameterfv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetTexParameterfv(target, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLightModelf(GLenum pname, GLfloat param)
{
   if (!_gles1_api.glLightModelf)
     {
        ERR("Can not call glLightModelf() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLightModelf(pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLightModelfv(GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glLightModelfv)
     {
        ERR("Can not call glLightModelfv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLightModelfv(pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLightf(GLenum light, GLenum pname, GLfloat param)
{
   if (!_gles1_api.glLightf)
     {
        ERR("Can not call glLightf() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLightf(light, pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glLightfv)
     {
        ERR("Can not call glLightfv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLightfv(light, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLineWidth(GLfloat width)
{
   if (!_gles1_api.glLineWidth)
     {
        ERR("Can not call glLineWidth() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLineWidth(width);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLoadMatrixf(const GLfloat *m)
{
   if (!_gles1_api.glLoadMatrixf)
     {
        ERR("Can not call glLoadMatrixf() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLoadMatrixf(m);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
   if (!_gles1_api.glMaterialf)
     {
        ERR("Can not call glMaterialf() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glMaterialf(face, pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glMaterialfv)
     {
        ERR("Can not call glMaterialfv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glMaterialfv(face, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glMultMatrixf(const GLfloat *m)
{
   if (!_gles1_api.glMultMatrixf)
     {
        ERR("Can not call glMultMatrixf() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glMultMatrixf(m);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
   if (!_gles1_api.glMultiTexCoord4f)
     {
        ERR("Can not call glMultiTexCoord4f() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glMultiTexCoord4f(target, s, t, r, q);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
   if (!_gles1_api.glNormal3f)
     {
        ERR("Can not call glNormal3f() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glNormal3f(nx, ny, nz);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
   if (!_gles1_api.glOrthof)
     {
        ERR("Can not call glOrthof() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glOrthof(left, right, bottom, top, zNear, zFar);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPointParameterf(GLenum pname, GLfloat param)
{
   if (!_gles1_api.glPointParameterf)
     {
        ERR("Can not call glPointParameterf() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPointParameterf(pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPointParameterfv(GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glPointParameterfv)
     {
        ERR("Can not call glPointParameterfv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPointParameterfv(pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPointSize(GLfloat size)
{
   if (!_gles1_api.glPointSize)
     {
        ERR("Can not call glPointSize() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPointSize(size);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles1_api.glPointSizePointerOES)
     {
        ERR("Can not call glPointSizePointerOES() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPointSizePointerOES(type, stride, pointer);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPolygonOffset(GLfloat factor, GLfloat units)
{
   if (!_gles1_api.glPolygonOffset)
     {
        ERR("Can not call glPolygonOffset() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPolygonOffset(factor, units);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
   if (!_gles1_api.glRotatef)
     {
        ERR("Can not call glRotatef() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glRotatef(angle, x, y, z);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glScalef(GLfloat x, GLfloat y, GLfloat z)
{
   if (!_gles1_api.glScalef)
     {
        ERR("Can not call glScalef() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glScalef(x, y, z);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
   if (!_gles1_api.glTexEnvf)
     {
        ERR("Can not call glTexEnvf() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexEnvf(target, pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glTexEnvfv)
     {
        ERR("Can not call glTexEnvfv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexEnvfv(target, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
   if (!_gles1_api.glTexParameterf)
     {
        ERR("Can not call glTexParameterf() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexParameterf(target, pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
   if (!_gles1_api.glTexParameterfv)
     {
        ERR("Can not call glTexParameterfv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexParameterfv(target, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
   if (!_gles1_api.glTranslatef)
     {
        ERR("Can not call glTranslatef() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTranslatef(x, y, z);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glActiveTexture(GLenum texture)
{
   if (!_gles1_api.glActiveTexture)
     {
        ERR("Can not call glActiveTexture() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glActiveTexture(texture);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glAlphaFuncx(GLenum func, GLclampx ref)
{
   if (!_gles1_api.glAlphaFuncx)
     {
        ERR("Can not call glAlphaFuncx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glAlphaFuncx(func, ref);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glBindBuffer(GLenum target, GLuint buffer)
{
   if (!_gles1_api.glBindBuffer)
     {
        ERR("Can not call glBindBuffer() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glBindBuffer(target, buffer);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glBindTexture(GLenum target, GLuint texture)
{
   if (!_gles1_api.glBindTexture)
     {
        ERR("Can not call glBindTexture() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glBindTexture(target, texture);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
   if (!_gles1_api.glBlendFunc)
     {
        ERR("Can not call glBlendFunc() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glBlendFunc(sfactor, dfactor);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
   if (!_gles1_api.glBufferData)
     {
        ERR("Can not call glBufferData() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glBufferData(target, size, data, usage);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
   if (!_gles1_api.glBufferSubData)
     {
        ERR("Can not call glBufferSubData() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glBufferSubData(target, offset, size, data);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glClear(GLbitfield mask)
{
   if (!_gles1_api.glClear)
     {
        ERR("Can not call glClear() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glClear(mask);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha)
{
   if (!_gles1_api.glClearColorx)
     {
        ERR("Can not call glClearColorx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glClearColorx(red, green, blue, alpha);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glClearDepthx(GLclampx depth)
{
   if (!_gles1_api.glClearDepthx)
     {
        ERR("Can not call glClearDepthx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glClearDepthx(depth);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glClearStencil(GLint s)
{
   if (!_gles1_api.glClearStencil)
     {
        ERR("Can not call glClearStencil() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glClearStencil(s);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glClientActiveTexture(GLenum texture)
{
   if (!_gles1_api.glClientActiveTexture)
     {
        ERR("Can not call glClientActiveTexture() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glClientActiveTexture(texture);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glClipPlanex(GLenum plane, const GLfixed *equation)
{
   if (!_gles1_api.glClipPlanex)
     {
        ERR("Can not call glClipPlanex() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glClipPlanex(plane, equation);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
   if (!_gles1_api.glColor4ub)
     {
        ERR("Can not call glColor4ub() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glColor4ub(red, green, blue, alpha);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
   if (!_gles1_api.glColor4x)
     {
        ERR("Can not call glColor4x() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glColor4x(red, green, blue, alpha);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
   if (!_gles1_api.glColorMask)
     {
        ERR("Can not call glColorMask() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glColorMask(red, green, blue, alpha);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles1_api.glColorPointer)
     {
        ERR("Can not call glColorPointer() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glColorPointer(size, type, stride, pointer);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
   if (!_gles1_api.glCompressedTexImage2D)
     {
        ERR("Can not call glCompressedTexImage2D() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
   if (!_gles1_api.glCompressedTexSubImage2D)
     {
        ERR("Can not call glCompressedTexSubImage2D() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
   if (!_gles1_api.glCopyTexImage2D)
     {
        ERR("Can not call glCopyTexImage2D() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
   if (!_gles1_api.glCopyTexSubImage2D)
     {
        ERR("Can not call glCopyTexSubImage2D() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glCullFace(GLenum mode)
{
   if (!_gles1_api.glCullFace)
     {
        ERR("Can not call glCullFace() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glCullFace(mode);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
   if (!_gles1_api.glDeleteBuffers)
     {
        ERR("Can not call glDeleteBuffers() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glDeleteBuffers(n, buffers);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glDeleteTextures(GLsizei n, const GLuint *textures)
{
   if (!_gles1_api.glDeleteTextures)
     {
        ERR("Can not call glDeleteTextures() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glDeleteTextures(n, textures);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glDepthFunc(GLenum func)
{
   if (!_gles1_api.glDepthFunc)
     {
        ERR("Can not call glDepthFunc() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glDepthFunc(func);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glDepthMask(GLboolean flag)
{
   if (!_gles1_api.glDepthMask)
     {
        ERR("Can not call glDepthMask() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glDepthMask(flag);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glDepthRangex(GLclampx zNear, GLclampx zFar)
{
   if (!_gles1_api.glDepthRangex)
     {
        ERR("Can not call glDepthRangex() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glDepthRangex(zNear, zFar);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glDisable(GLenum cap)
{
   if (!_gles1_api.glDisable)
     {
        ERR("Can not call glDisable() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glDisable(cap);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glDisableClientState(GLenum array)
{
   if (!_gles1_api.glDisableClientState)
     {
        ERR("Can not call glDisableClientState() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glDisableClientState(array);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   if (!_gles1_api.glDrawArrays)
     {
        ERR("Can not call glDrawArrays() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glDrawArrays(mode, first, count);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
   if (!_gles1_api.glDrawElements)
     {
        ERR("Can not call glDrawElements() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glDrawElements(mode, count, type, indices);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glEnable(GLenum cap)
{
   if (!_gles1_api.glEnable)
     {
        ERR("Can not call glEnable() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glEnable(cap);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glEnableClientState(GLenum array)
{
   if (!_gles1_api.glEnableClientState)
     {
        ERR("Can not call glEnableClientState() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glEnableClientState(array);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glFinish(void)
{
   if (!_gles1_api.glFinish)
     {
        ERR("Can not call glFinish() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glFinish();
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glFlush(void)
{
   if (!_gles1_api.glFlush)
     {
        ERR("Can not call glFlush() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glFlush();
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glFogx(GLenum pname, GLfixed param)
{
   if (!_gles1_api.glFogx)
     {
        ERR("Can not call glFogx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glFogx(pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glFogxv(GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glFogxv)
     {
        ERR("Can not call glFogxv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glFogxv(pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glFrontFace(GLenum mode)
{
   if (!_gles1_api.glFrontFace)
     {
        ERR("Can not call glFrontFace() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glFrontFace(mode);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glFrustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
   if (!_gles1_api.glFrustumx)
     {
        ERR("Can not call glFrustumx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glFrustumx(left, right, bottom, top, zNear, zFar);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetBooleanv(GLenum pname, GLboolean *params)
{
   if (!_gles1_api.glGetBooleanv)
     {
        ERR("Can not call glGetBooleanv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetBooleanv(pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
   if (!_gles1_api.glGetBufferParameteriv)
     {
        ERR("Can not call glGetBufferParameteriv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetBufferParameteriv(target, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetClipPlanex(GLenum pname, GLfixed eqn[4])
{
   if (!_gles1_api.glGetClipPlanex)
     {
        ERR("Can not call glGetClipPlanex() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetClipPlanex(pname, eqn);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGenBuffers(GLsizei n, GLuint *buffers)
{
   if (!_gles1_api.glGenBuffers)
     {
        ERR("Can not call glGenBuffers() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGenBuffers(n, buffers);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGenTextures(GLsizei n, GLuint *textures)
{
   if (!_gles1_api.glGenTextures)
     {
        ERR("Can not call glGenTextures() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGenTextures(n, textures);
   EVGLD_FUNC_END();
}

static GLenum
_evgld_gles1_glGetError(void)
{
   GLenum ret;
   if (!_gles1_api.glGetError)
     {
        ERR("Can not call glGetError() in this context!");
        return EVAS_GL_NOT_INITIALIZED;
     }
   EVGLD_FUNC_BEGIN();
   ret = _evgl_gles1_glGetError();
   EVGLD_FUNC_END();
   return ret;
}

static void
_evgld_gles1_glGetFixedv(GLenum pname, GLfixed *params)
{
   if (!_gles1_api.glGetFixedv)
     {
        ERR("Can not call glGetFixedv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetFixedv(pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetIntegerv(GLenum pname, GLint *params)
{
   if (!_gles1_api.glGetIntegerv)
     {
        ERR("Can not call glGetIntegerv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetIntegerv(pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetLightxv(GLenum light, GLenum pname, GLfixed *params)
{
   if (!_gles1_api.glGetLightxv)
     {
        ERR("Can not call glGetLightxv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetLightxv(light, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetMaterialxv(GLenum face, GLenum pname, GLfixed *params)
{
   if (!_gles1_api.glGetMaterialxv)
     {
        ERR("Can not call glGetMaterialxv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetMaterialxv(face, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetPointerv(GLenum pname, GLvoid **params)
{
   if (!_gles1_api.glGetPointerv)
     {
        ERR("Can not call glGetPointerv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetPointerv(pname, params);
   EVGLD_FUNC_END();
}

static const GLubyte *
_evgld_gles1_glGetString(GLenum name)
{
   const GLubyte * ret;
   if (!_gles1_api.glGetString)
     {
        ERR("Can not call glGetString() in this context!");
        return NULL;
     }
   EVGLD_FUNC_BEGIN();
   ret = _evgl_gles1_glGetString(name);
   EVGLD_FUNC_END();
   return ret;
}

static void
_evgld_gles1_glGetTexEnviv(GLenum env, GLenum pname, GLint *params)
{
   if (!_gles1_api.glGetTexEnviv)
     {
        ERR("Can not call glGetTexEnviv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetTexEnviv(env, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetTexEnvxv(GLenum env, GLenum pname, GLfixed *params)
{
   if (!_gles1_api.glGetTexEnvxv)
     {
        ERR("Can not call glGetTexEnvxv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetTexEnvxv(env, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
   if (!_gles1_api.glGetTexParameteriv)
     {
        ERR("Can not call glGetTexParameteriv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetTexParameteriv(target, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glGetTexParameterxv(GLenum target, GLenum pname, GLfixed *params)
{
   if (!_gles1_api.glGetTexParameterxv)
     {
        ERR("Can not call glGetTexParameterxv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glGetTexParameterxv(target, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glHint(GLenum target, GLenum mode)
{
   if (!_gles1_api.glHint)
     {
        ERR("Can not call glHint() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glHint(target, mode);
   EVGLD_FUNC_END();
}

static GLboolean
_evgld_gles1_glIsBuffer(GLuint buffer)
{
   GLboolean ret;
   if (!_gles1_api.glIsBuffer)
     {
        ERR("Can not call glIsBuffer() in this context!");
        return EINA_FALSE;
     }
   EVGLD_FUNC_BEGIN();
   ret = _evgl_gles1_glIsBuffer(buffer);
   EVGLD_FUNC_END();
   return ret;
}

static GLboolean
_evgld_gles1_glIsEnabled(GLenum cap)
{
   GLboolean ret;
   if (!_gles1_api.glIsEnabled)
     {
        ERR("Can not call glIsEnabled() in this context!");
        return EINA_FALSE;
     }
   EVGLD_FUNC_BEGIN();
   ret = _evgl_gles1_glIsEnabled(cap);
   EVGLD_FUNC_END();
   return ret;
}

static GLboolean
_evgld_gles1_glIsTexture(GLuint texture)
{
   GLboolean ret;
   if (!_gles1_api.glIsTexture)
     {
        ERR("Can not call glIsTexture() in this context!");
        return EINA_FALSE;
     }
   EVGLD_FUNC_BEGIN();
   ret = _evgl_gles1_glIsTexture(texture);
   EVGLD_FUNC_END();
   return ret;
}

static void
_evgld_gles1_glLightModelx(GLenum pname, GLfixed param)
{
   if (!_gles1_api.glLightModelx)
     {
        ERR("Can not call glLightModelx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLightModelx(pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLightModelxv(GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glLightModelxv)
     {
        ERR("Can not call glLightModelxv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLightModelxv(pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLightx(GLenum light, GLenum pname, GLfixed param)
{
   if (!_gles1_api.glLightx)
     {
        ERR("Can not call glLightx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLightx(light, pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLightxv(GLenum light, GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glLightxv)
     {
        ERR("Can not call glLightxv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLightxv(light, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLineWidthx(GLfixed width)
{
   if (!_gles1_api.glLineWidthx)
     {
        ERR("Can not call glLineWidthx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLineWidthx(width);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLoadIdentity(void)
{
   if (!_gles1_api.glLoadIdentity)
     {
        ERR("Can not call glLoadIdentity() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLoadIdentity();
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLoadMatrixx(const GLfixed *m)
{
   if (!_gles1_api.glLoadMatrixx)
     {
        ERR("Can not call glLoadMatrixx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLoadMatrixx(m);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glLogicOp(GLenum opcode)
{
   if (!_gles1_api.glLogicOp)
     {
        ERR("Can not call glLogicOp() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glLogicOp(opcode);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glMaterialx(GLenum face, GLenum pname, GLfixed param)
{
   if (!_gles1_api.glMaterialx)
     {
        ERR("Can not call glMaterialx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glMaterialx(face, pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glMaterialxv(GLenum face, GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glMaterialxv)
     {
        ERR("Can not call glMaterialxv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glMaterialxv(face, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glMatrixMode(GLenum mode)
{
   if (!_gles1_api.glMatrixMode)
     {
        ERR("Can not call glMatrixMode() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glMatrixMode(mode);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glMultMatrixx(const GLfixed *m)
{
   if (!_gles1_api.glMultMatrixx)
     {
        ERR("Can not call glMultMatrixx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glMultMatrixx(m);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glMultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
   if (!_gles1_api.glMultiTexCoord4x)
     {
        ERR("Can not call glMultiTexCoord4x() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glMultiTexCoord4x(target, s, t, r, q);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
   if (!_gles1_api.glNormal3x)
     {
        ERR("Can not call glNormal3x() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glNormal3x(nx, ny, nz);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles1_api.glNormalPointer)
     {
        ERR("Can not call glNormalPointer() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glNormalPointer(type, stride, pointer);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glOrthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
   if (!_gles1_api.glOrthox)
     {
        ERR("Can not call glOrthox() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glOrthox(left, right, bottom, top, zNear, zFar);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPixelStorei(GLenum pname, GLint param)
{
   if (!_gles1_api.glPixelStorei)
     {
        ERR("Can not call glPixelStorei() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPixelStorei(pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPointParameterx(GLenum pname, GLfixed param)
{
   if (!_gles1_api.glPointParameterx)
     {
        ERR("Can not call glPointParameterx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPointParameterx(pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPointParameterxv(GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glPointParameterxv)
     {
        ERR("Can not call glPointParameterxv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPointParameterxv(pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPointSizex(GLfixed size)
{
   if (!_gles1_api.glPointSizex)
     {
        ERR("Can not call glPointSizex() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPointSizex(size);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPolygonOffsetx(GLfixed factor, GLfixed units)
{
   if (!_gles1_api.glPolygonOffsetx)
     {
        ERR("Can not call glPolygonOffsetx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPolygonOffsetx(factor, units);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPopMatrix(void)
{
   if (!_gles1_api.glPopMatrix)
     {
        ERR("Can not call glPopMatrix() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPopMatrix();
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glPushMatrix(void)
{
   if (!_gles1_api.glPushMatrix)
     {
        ERR("Can not call glPushMatrix() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glPushMatrix();
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
   if (!_gles1_api.glReadPixels)
     {
        ERR("Can not call glReadPixels() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glReadPixels(x, y, width, height, format, type, pixels);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
   if (!_gles1_api.glRotatex)
     {
        ERR("Can not call glRotatex() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glRotatex(angle, x, y, z);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glSampleCoverage(GLclampf value, GLboolean invert)
{
   if (!_gles1_api.glSampleCoverage)
     {
        ERR("Can not call glSampleCoverage() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glSampleCoverage(value, invert);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glSampleCoveragex(GLclampx value, GLboolean invert)
{
   if (!_gles1_api.glSampleCoveragex)
     {
        ERR("Can not call glSampleCoveragex() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glSampleCoveragex(value, invert);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glScalex(GLfixed x, GLfixed y, GLfixed z)
{
   if (!_gles1_api.glScalex)
     {
        ERR("Can not call glScalex() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glScalex(x, y, z);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
   if (!_gles1_api.glScissor)
     {
        ERR("Can not call glScissor() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glScissor(x, y, width, height);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glShadeModel(GLenum mode)
{
   if (!_gles1_api.glShadeModel)
     {
        ERR("Can not call glShadeModel() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glShadeModel(mode);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
   if (!_gles1_api.glStencilFunc)
     {
        ERR("Can not call glStencilFunc() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glStencilFunc(func, ref, mask);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glStencilMask(GLuint mask)
{
   if (!_gles1_api.glStencilMask)
     {
        ERR("Can not call glStencilMask() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glStencilMask(mask);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
   if (!_gles1_api.glStencilOp)
     {
        ERR("Can not call glStencilOp() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glStencilOp(fail, zfail, zpass);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles1_api.glTexCoordPointer)
     {
        ERR("Can not call glTexCoordPointer() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexCoordPointer(size, type, stride, pointer);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexEnvi(GLenum target, GLenum pname, GLint param)
{
   if (!_gles1_api.glTexEnvi)
     {
        ERR("Can not call glTexEnvi() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexEnvi(target, pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexEnvx(GLenum target, GLenum pname, GLfixed param)
{
   if (!_gles1_api.glTexEnvx)
     {
        ERR("Can not call glTexEnvx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexEnvx(target, pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
   if (!_gles1_api.glTexEnviv)
     {
        ERR("Can not call glTexEnviv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexEnviv(target, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexEnvxv(GLenum target, GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glTexEnvxv)
     {
        ERR("Can not call glTexEnvxv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexEnvxv(target, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   if (!_gles1_api.glTexImage2D)
     {
        ERR("Can not call glTexImage2D() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexParameteri(GLenum target, GLenum pname, GLint param)
{
   if (!_gles1_api.glTexParameteri)
     {
        ERR("Can not call glTexParameteri() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexParameteri(target, pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexParameterx(GLenum target, GLenum pname, GLfixed param)
{
   if (!_gles1_api.glTexParameterx)
     {
        ERR("Can not call glTexParameterx() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexParameterx(target, pname, param);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
   if (!_gles1_api.glTexParameteriv)
     {
        ERR("Can not call glTexParameteriv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexParameteriv(target, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexParameterxv(GLenum target, GLenum pname, const GLfixed *params)
{
   if (!_gles1_api.glTexParameterxv)
     {
        ERR("Can not call glTexParameterxv() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexParameterxv(target, pname, params);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
   if (!_gles1_api.glTexSubImage2D)
     {
        ERR("Can not call glTexSubImage2D() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
   if (!_gles1_api.glTranslatex)
     {
        ERR("Can not call glTranslatex() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glTranslatex(x, y, z);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   if (!_gles1_api.glVertexPointer)
     {
        ERR("Can not call glVertexPointer() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glVertexPointer(size, type, stride, pointer);
   EVGLD_FUNC_END();
}

static void
_evgld_gles1_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
   if (!_gles1_api.glViewport)
     {
        ERR("Can not call glViewport() in this context!");
        return;
     }
   EVGLD_FUNC_BEGIN();
   _evgl_gles1_glViewport(x, y, width, height);
   EVGLD_FUNC_END();
}



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

#define ORD(name) EVAS_API_OVERRIDE(name, funcs, _evgl_gles1_)
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
