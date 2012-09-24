#include "evas_gl_core_private.h"
#include "evas_gl_api_ext.h"

#define EVGL_FUNC_BEGIN()
#define EVGL_FUNC_END()
#define _EVGL_INT_INIT_VALUE -3

void
evgl_glActiveTexture(GLenum texture)
{
   EVGL_FUNC_BEGIN();
   glActiveTexture(texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glAttachShader(GLuint program, GLuint shader)
{
   EVGL_FUNC_BEGIN();
   glAttachShader(program, shader);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBindAttribLocation(GLuint program, GLuint index, const char* name)
{
   EVGL_FUNC_BEGIN();
   glBindAttribLocation(program, index, name);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBindBuffer(GLenum target, GLuint buffer)
{
   EVGL_FUNC_BEGIN();
   glBindBuffer(target, buffer);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBindFramebuffer(GLenum target, GLuint framebuffer)
{
   EVGL_Context *ctx = NULL;

   EVGL_FUNC_BEGIN();

   ctx = _evgl_current_context_get();

   if (!ctx)
     {
        ERR("No current context set.");
        return;
     }

   // Take care of BindFramebuffer 0 issue
   if (framebuffer==0)
     {
        if (evgl_direct_enabled(evgl_engine))
           glBindFramebuffer(target, 0);
        else
           glBindFramebuffer(target, ctx->surface_fbo);
        ctx->current_fbo = 0;
     }
   else
     {
        glBindFramebuffer(target, framebuffer);

        // Save this for restore when doing make current
        ctx->current_fbo = framebuffer;
     }

   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
   EVGL_FUNC_BEGIN();
   glBindRenderbuffer(target, renderbuffer);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBindTexture(GLenum target, GLuint texture)
{
   EVGL_FUNC_BEGIN();
   glBindTexture(target, texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   EVGL_FUNC_BEGIN();
   glBlendColor(red, green, blue, alpha);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBlendEquation(GLenum mode)
{
   EVGL_FUNC_BEGIN();
   glBlendEquation(mode);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
   EVGL_FUNC_BEGIN();
   glBlendEquationSeparate(modeRGB, modeAlpha);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
   EVGL_FUNC_BEGIN();
   glBlendFunc(sfactor, dfactor);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
   EVGL_FUNC_BEGIN();
   glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBufferData(GLenum target, GLsizeiptr size, const void* data, GLenum usage)
{
   EVGL_FUNC_BEGIN();
   glBufferData(target, size, data, usage);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void* data)
{
   EVGL_FUNC_BEGIN();
   glBufferSubData(target, offset, size, data);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

GLenum
evgl_glCheckFramebufferStatus(GLenum target)
{
   GLenum ret = GL_NONE;

   EVGL_FUNC_BEGIN();
   ret = glCheckFramebufferStatus(target);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

#ifdef NO_DIRECT_RENDERING
void
evgl_glClear(GLbitfield mask)
{
   EVGL_FUNC_BEGIN();
   glClear(mask);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}
#endif

void
evgl_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   EVGL_FUNC_BEGIN();
   glClearColor(red, green, blue, alpha);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glClearDepthf(GLclampf depth)
{
   EVGL_FUNC_BEGIN();

#ifdef GL_GLES
   glClearDepthf(depth);
#else
   glClearDepth(depth);
#endif

   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glClearStencil(GLint s)
{
   EVGL_FUNC_BEGIN();
   glClearStencil(s);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
   EVGL_FUNC_BEGIN();
   glColorMask(red, green, blue, alpha);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glCompileShader(GLuint shader)
{
   EVGL_FUNC_BEGIN();
   glCompileShader(shader);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data)
{
   EVGL_FUNC_BEGIN();
   glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data)
{
   EVGL_FUNC_BEGIN();
   glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
   EVGL_FUNC_BEGIN();
   glCopyTexImage2D(target, level, internalformat, x, y, width, height, border);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_FUNC_BEGIN();
   glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

GLuint
evgl_glCreateProgram(void)
{
   GLuint ret = _EVGL_INT_INIT_VALUE;

   EVGL_FUNC_BEGIN();
   ret = glCreateProgram();
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

GLuint
evgl_glCreateShader(GLenum type)
{
   GLuint ret = _EVGL_INT_INIT_VALUE;
   EVGL_FUNC_BEGIN();
   ret = glCreateShader(type);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

void
evgl_glCullFace(GLenum mode)
{
   EVGL_FUNC_BEGIN();
   glCullFace(mode);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDeleteBuffers(GLsizei n, const GLuint* buffers)
{
   EVGL_FUNC_BEGIN();
   glDeleteBuffers(n, buffers);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
   EVGL_FUNC_BEGIN();
   glDeleteFramebuffers(n, framebuffers);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDeleteProgram(GLuint program)
{
   EVGL_FUNC_BEGIN();
   glDeleteProgram(program);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
   EVGL_FUNC_BEGIN();
   glDeleteRenderbuffers(n, renderbuffers);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDeleteShader(GLuint shader)
{
   EVGL_FUNC_BEGIN();
   glDeleteShader(shader);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDeleteTextures(GLsizei n, const GLuint* textures)
{
   EVGL_FUNC_BEGIN();
   glDeleteTextures(n, textures);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDepthFunc(GLenum func)
{
   EVGL_FUNC_BEGIN();
   glDepthFunc(func);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDepthMask(GLboolean flag)
{
   EVGL_FUNC_BEGIN();
   glDepthMask(flag);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDepthRangef(GLclampf zNear, GLclampf zFar)
{
   EVGL_FUNC_BEGIN();

#ifdef GL_GLES
   glDepthRangef(zNear, zFar);
#else
   glDepthRange(zNear, zFar);
#endif

   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDetachShader(GLuint program, GLuint shader)
{
   EVGL_FUNC_BEGIN();
   glDetachShader(program, shader);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

#ifdef NO_DIRECT_RENDERING
void
evgl_glDisable(GLenum cap)
{
   EVGL_FUNC_BEGIN();
   glDisable(cap);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}
#endif

void
evgl_glDisableVertexAttribArray(GLuint index)
{
   EVGL_FUNC_BEGIN();
   glDisableVertexAttribArray(index);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
   EVGL_FUNC_BEGIN();
   glDrawArrays(mode, first, count);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices)
{
   EVGL_FUNC_BEGIN();
   glDrawElements(mode, count, type, indices);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

#ifdef NO_DIRECT_RENDERING
void
evgl_glEnable(GLenum cap)
{
   EVGL_FUNC_BEGIN();
   glEnable(cap);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}
#endif

void
evgl_glEnableVertexAttribArray(GLuint index)
{
   EVGL_FUNC_BEGIN();
   glEnableVertexAttribArray(index);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glFinish(void)
{
   EVGL_FUNC_BEGIN();
   glFinish();
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glFlush(void)
{
   EVGL_FUNC_BEGIN();
   glFlush();
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
   EVGL_FUNC_BEGIN();
   glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
   EVGL_FUNC_BEGIN();
   glFramebufferTexture2D(target, attachment, textarget, texture, level);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glFrontFace(GLenum mode)
{
   EVGL_FUNC_BEGIN();
   glFrontFace(mode);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
   EVGL_FUNC_BEGIN();
   glGetVertexAttribfv(index, pname, params);

   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetVertexAttribiv(index, pname, params);

   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetVertexAttribPointerv(GLuint index, GLenum pname, void** pointer)
{
   EVGL_FUNC_BEGIN();
   glGetVertexAttribPointerv(index, pname, pointer);

   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glHint(GLenum target, GLenum mode)
{
   EVGL_FUNC_BEGIN();
   glHint(target, mode);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGenBuffers(GLsizei n, GLuint* buffers)
{
   EVGL_FUNC_BEGIN();
   glGenBuffers(n, buffers);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGenerateMipmap(GLenum target)
{
   EVGL_FUNC_BEGIN();
   glGenerateMipmap(target);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGenFramebuffers(GLsizei n, GLuint* framebuffers)
{
   EVGL_FUNC_BEGIN();
   glGenFramebuffers(n, framebuffers);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
   EVGL_FUNC_BEGIN();
   glGenRenderbuffers(n, renderbuffers);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGenTextures(GLsizei n, GLuint* textures)
{
   EVGL_FUNC_BEGIN();
   glGenTextures(n, textures);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)
{
   EVGL_FUNC_BEGIN();
   glGetActiveAttrib(program, index, bufsize, length, size, type, name);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name)
{
   EVGL_FUNC_BEGIN();
   glGetActiveUniform(program, index, bufsize, length, size, type, name);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
   EVGL_FUNC_BEGIN();
   glGetAttachedShaders(program, maxcount, count, shaders);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

int
evgl_glGetAttribLocation(GLuint program, const char* name)
{
   int ret = _EVGL_INT_INIT_VALUE;
   EVGL_FUNC_BEGIN();
   ret = glGetAttribLocation(program, name);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

void
evgl_glGetBooleanv(GLenum pname, GLboolean* params)
{
   EVGL_FUNC_BEGIN();
   glGetBooleanv(pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetBufferParameteriv(target, pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

GLenum
evgl_glGetError(void)
{
   GLenum ret = GL_NONE;

   EVGL_FUNC_BEGIN();
   ret = glGetError();
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

void
evgl_glGetFloatv(GLenum pname, GLfloat* params)
{
   EVGL_FUNC_BEGIN();
   glGetFloatv(pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetIntegerv(GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetIntegerv(pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetProgramiv(GLuint program, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetProgramiv(program, pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, char* infolog)
{
   EVGL_FUNC_BEGIN();
   glGetProgramInfoLog(program, bufsize, length, infolog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetRenderbufferParameteriv(target, pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetShaderiv(shader, pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog)
{
   EVGL_FUNC_BEGIN();
   glGetShaderInfoLog(shader, bufsize, length, infolog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
   EVGL_FUNC_BEGIN();

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
   shadertype = precisiontype = 0;
#endif

   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, char* source)
{
   EVGL_FUNC_BEGIN();
   glGetShaderSource(shader, bufsize, length, source);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

const GLubyte *
evgl_glGetString(GLenum name)
{
   const GLubyte *ret = NULL;

   EVGL_FUNC_BEGIN();
#if 0
   if (name == GL_EXTENSIONS)
      return (GLubyte *)_gl_ext_string; //glGetString(GL_EXTENSIONS);
   else
      return glGetString(name);
#endif
   ret = glGetString(name);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

void
evgl_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
   EVGL_FUNC_BEGIN();
   glGetTexParameterfv(target, pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetTexParameteriv(target, pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetUniformfv(GLuint program, GLint location, GLfloat* params)
{
   EVGL_FUNC_BEGIN();
   glGetUniformfv(program, location, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glGetUniformiv(GLuint program, GLint location, GLint* params)
{
   EVGL_FUNC_BEGIN();
   glGetUniformiv(program, location, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}
int
evgl_glGetUniformLocation(GLuint program, const char* name)
{
   int ret = _EVGL_INT_INIT_VALUE;

   EVGL_FUNC_BEGIN();
   ret = glGetUniformLocation(program, name);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

GLboolean
evgl_glIsBuffer(GLuint buffer)
{
   GLboolean ret = GL_FALSE;

   EVGL_FUNC_BEGIN();
   ret = glIsBuffer(buffer);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

GLboolean
evgl_glIsEnabled(GLenum cap)
{
   GLboolean ret = GL_FALSE;

   EVGL_FUNC_BEGIN();
   ret = glIsEnabled(cap);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

GLboolean
evgl_glIsFramebuffer(GLuint framebuffer)
{
   GLboolean ret = GL_FALSE;

   EVGL_FUNC_BEGIN();
   ret = glIsFramebuffer(framebuffer);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

GLboolean
evgl_glIsProgram(GLuint program)
{
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = glIsProgram(program);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

GLboolean
evgl_glIsRenderbuffer(GLuint renderbuffer)
{
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = glIsRenderbuffer(renderbuffer);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

GLboolean
evgl_glIsShader(GLuint shader)
{
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = glIsShader(shader);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

GLboolean
evgl_glIsTexture(GLuint texture)
{
   GLboolean ret;
   EVGL_FUNC_BEGIN();
   ret = glIsTexture(texture);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
   return ret;
}

void
evgl_glLineWidth(GLfloat width)
{
   EVGL_FUNC_BEGIN();
   glLineWidth(width);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glLinkProgram(GLuint program)
{
   EVGL_FUNC_BEGIN();
   glLinkProgram(program);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glPixelStorei(GLenum pname, GLint param)
{
   EVGL_FUNC_BEGIN();
   glPixelStorei(pname, param);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glPolygonOffset(GLfloat factor, GLfloat units)
{
   EVGL_FUNC_BEGIN();
   glPolygonOffset(factor, units);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

#ifdef NO_DIRECT_RENDERING
void
evgl_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
   EVGL_FUNC_BEGIN();
   glReadPixels(x, y, width, height, format, type, pixels);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}
#endif

void
evgl_glReleaseShaderCompiler(void)
{
   EVGL_FUNC_BEGIN();

#ifdef GL_GLES
   glReleaseShaderCompiler();
#else
#endif

   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
   EVGL_FUNC_BEGIN();
   glRenderbufferStorage(target, internalformat, width, height);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glSampleCoverage(GLclampf value, GLboolean invert)
{
   EVGL_FUNC_BEGIN();
   glSampleCoverage(value, invert);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

#ifdef NO_DIRECT_RENDERING
void
evgl_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_FUNC_BEGIN();
   glScissor(x, y, width, height);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}
#endif

void
evgl_glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length)
{
   EVGL_FUNC_BEGIN();

#ifdef GL_GLES
   glShaderBinary(n, shaders, binaryformat, binary, length);
#else
   // FIXME: need to dlsym/getprocaddress for this
   ERR("Binary Shader is not supported here yet.");
   return;
   n = binaryformat = length = 0;
   shaders = binary = 0;
#endif

   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glShaderSource(GLuint shader, GLsizei count, const char** string, const GLint* length)
{
   EVGL_FUNC_BEGIN();
   glShaderSource(shader, count, string, length);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
   EVGL_FUNC_BEGIN();
   glStencilFunc(func, ref, mask);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
   EVGL_FUNC_BEGIN();
   glStencilFuncSeparate(face, func, ref, mask);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glStencilMask(GLuint mask)
{
   EVGL_FUNC_BEGIN();
   glStencilMask(mask);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glStencilMaskSeparate(GLenum face, GLuint mask)
{
   EVGL_FUNC_BEGIN();
   glStencilMaskSeparate(face, mask);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
   EVGL_FUNC_BEGIN();
   glStencilOp(fail, zfail, zpass);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
   EVGL_FUNC_BEGIN();
   glStencilOpSeparate(face, fail, zfail, zpass);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
   EVGL_FUNC_BEGIN();
   glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
   EVGL_FUNC_BEGIN();
   glTexParameterf(target, pname, param);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
   EVGL_FUNC_BEGIN();
   glTexParameterfv(target, pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glTexParameteri(GLenum target, GLenum pname, GLint param)
{
   EVGL_FUNC_BEGIN();
   glTexParameteri(target, pname, param);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
   EVGL_FUNC_BEGIN();
   glTexParameteriv(target, pname, params);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels)
{
   EVGL_FUNC_BEGIN();
   glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform1f(GLint location, GLfloat x)
{
   EVGL_FUNC_BEGIN();
   glUniform1f(location, x);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGL_FUNC_BEGIN();
   glUniform1fv(location, count, v);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform1i(GLint location, GLint x)
{
   EVGL_FUNC_BEGIN();
   glUniform1i(location, x);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform1iv(GLint location, GLsizei count, const GLint* v)
{
   EVGL_FUNC_BEGIN();
   glUniform1iv(location, count, v);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform2f(GLint location, GLfloat x, GLfloat y)
{
   EVGL_FUNC_BEGIN();
   glUniform2f(location, x, y);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform2fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGL_FUNC_BEGIN();
   glUniform2fv(location, count, v);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform2i(GLint location, GLint x, GLint y)
{
   EVGL_FUNC_BEGIN();
   glUniform2i(location, x, y);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform2iv(GLint location, GLsizei count, const GLint* v)
{
   EVGL_FUNC_BEGIN();
   glUniform2iv(location, count, v);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
   EVGL_FUNC_BEGIN();
   glUniform3f(location, x, y, z);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGL_FUNC_BEGIN();
   glUniform3fv(location, count, v);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform3i(GLint location, GLint x, GLint y, GLint z)
{
   EVGL_FUNC_BEGIN();
   glUniform3i(location, x, y, z);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform3iv(GLint location, GLsizei count, const GLint* v)
{
   EVGL_FUNC_BEGIN();
   glUniform3iv(location, count, v);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   EVGL_FUNC_BEGIN();
   glUniform4f(location, x, y, z, w);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
   EVGL_FUNC_BEGIN();
   glUniform4fv(location, count, v);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
   EVGL_FUNC_BEGIN();
   glUniform4i(location, x, y, z, w);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniform4iv(GLint location, GLsizei count, const GLint* v)
{
   EVGL_FUNC_BEGIN();
   glUniform4iv(location, count, v);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
   EVGL_FUNC_BEGIN();
   glUniformMatrix2fv(location, count, transpose, value);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
   EVGL_FUNC_BEGIN();
   glUniformMatrix3fv(location, count, transpose, value);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
   EVGL_FUNC_BEGIN();
   glUniformMatrix4fv(location, count, transpose, value);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glUseProgram(GLuint program)
{
   EVGL_FUNC_BEGIN();
   glUseProgram(program);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glValidateProgram(GLuint program)
{
   EVGL_FUNC_BEGIN();
   glValidateProgram(program);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glVertexAttrib1f(GLuint indx, GLfloat x)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib1f(indx, x);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glVertexAttrib1fv(GLuint indx, const GLfloat* values)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib1fv(indx, values);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib2f(indx, x, y);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glVertexAttrib2fv(GLuint indx, const GLfloat* values)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib2fv(indx, values);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib3f(indx, x, y, z);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glVertexAttrib3fv(GLuint indx, const GLfloat* values)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib3fv(indx, values);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib4f(indx, x, y, z, w);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glVertexAttrib4fv(GLuint indx, const GLfloat* values)
{
   EVGL_FUNC_BEGIN();
   glVertexAttrib4fv(indx, values);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

void
evgl_glVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr)
{
   EVGL_FUNC_BEGIN();
   glVertexAttribPointer(indx, size, type, normalized, stride, ptr);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

#ifdef NO_DIRECT_RENDERING
void
evgl_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_FUNC_BEGIN();
   glViewport(x, y, width, height);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}
#endif

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
                  p = "(gl_MaxVertexUniformComponents / 4)";
               }
             else if (!strncmp(p, "gl_MaxFragmentUniformVectors", 28))
               {
                  p = "(gl_MaxFragmentUniformComponents / 4)";
               }
             else if (!strncmp(p, "gl_MaxVaryingVectors", 20))
               {
                  p = "(gl_MaxVaryingFloats / 4)";
               }

             int new_len = strlen(p);
             if (*patched_len + new_len > patched_size)
               {
                  patched_size *= 2;
                  patched = realloc(patched, patched_size + 1);

                  if (!patched) return NULL;
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
evgl_glShaderSource(GLuint shader, GLsizei count, const char** string, const GLint* length)
{
   EVGL_FUNC_BEGIN();

#ifdef GL_GLES
   glShaderSource(shader, count, string, length);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
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
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

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

// Transform from Evas Coordinat to GL Coordinate
// returns: oc[4] original image object dimension in gl coord
// returns: nc[4] tranformed  (x, y, width, heigth) in gl coord
static void
compute_gl_coordinates(Evas_Object *obj, int rot, int clip,
                       int x, int y, int width, int height,
                       int imgc[4], int objc[4])
{
   if (rot == 0)
     {
        // oringinal image object coordinate in gl coordinate
        imgc[0] = obj->cur.geometry.x;
        imgc[1] = obj->layer->evas->output.h - obj->cur.geometry.y - obj->cur.geometry.h;
        imgc[2] = imgc[0] + obj->cur.geometry.w;
        imgc[3] = imgc[1] + obj->cur.geometry.h;

        // transformed (x,y,width,height) in gl coordinate
        objc[0] = imgc[0] + x;
        objc[1] = imgc[1] + y;
        objc[2] = objc[0] + width;
        objc[3] = objc[1] + height;
     }
   else if (rot == 180)
     {
        // oringinal image object coordinate in gl coordinate
        imgc[0] = obj->layer->evas->output.w - obj->cur.geometry.x - obj->cur.geometry.w;
        imgc[1] = obj->cur.geometry.y;
        imgc[2] = imgc[0] + obj->cur.geometry.w;
        imgc[3] = imgc[1] + obj->cur.geometry.h;

        // transformed (x,y,width,height) in gl coordinate
        objc[0] = imgc[0] + obj->cur.geometry.w - x - width;
        objc[1] = imgc[1] + obj->cur.geometry.h - y - height;
        objc[2] = objc[0] + width;
        objc[3] = objc[1] + height;

     }
   else if (rot == 90)
     {
        // oringinal image object coordinate in gl coordinate
        imgc[0] = obj->cur.geometry.y;
        imgc[1] = obj->cur.geometry.x;
        imgc[2] = imgc[0] + obj->cur.geometry.h;
        imgc[3] = imgc[1] + obj->cur.geometry.w;

        // transformed (x,y,width,height) in gl coordinate
        objc[0] = imgc[0] + obj->cur.geometry.h - y - height;
        objc[1] = imgc[1] + x;
        objc[2] = objc[0] + height;
        objc[3] = objc[1] + width;
     }
   else if (rot == 270)
     {
        // oringinal image object coordinate in gl coordinate
        imgc[0] = obj->layer->evas->output.h - obj->cur.geometry.y - obj->cur.geometry.h;
        imgc[1] = obj->layer->evas->output.w - obj->cur.geometry.x - obj->cur.geometry.w;
        imgc[2] = imgc[0] + obj->cur.geometry.h;
        imgc[3] = imgc[1] + obj->cur.geometry.w;

        // transformed (x,y,width,height) in gl coordinate
        objc[0] = imgc[0] + y;
        objc[1] = imgc[1] + obj->cur.geometry.w - x - width;
        objc[2] = objc[0] + height;
        objc[3] = objc[1] + width;
     }
   else
     {
        ERR("Invalid rotation angle %d.", rot);
        return;
     }

   if (clip)
     {
        // Clip against original image object
        if (objc[0] < imgc[0]) objc[0] = imgc[0];
        if (objc[0] > imgc[2]) objc[0] = 0;

        if (objc[1] < imgc[1]) objc[1] = imgc[1];
        if (objc[1] > imgc[3]) objc[1] = 0;

        if (objc[2] < imgc[0]) objc[0] = 0;
        if (objc[2] > imgc[2]) objc[2] = imgc[2];

        if (objc[3] < imgc[1]) objc[1] = 0;
        if (objc[3] > imgc[3]) objc[3] = imgc[3];
     }

   imgc[2] = imgc[2]-imgc[0];     // width
   imgc[3] = imgc[3]-imgc[1];     // height

   objc[2] = objc[2]-objc[0];     // width
   objc[3] = objc[3]-objc[1];     // height
}

static void
evgl_glClear(GLbitfield mask)
{
   EVGL_Engine *ee = evgl_engine;
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   Evas_Object *img;
   int rot = 0;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};

   EVGL_FUNC_BEGIN();

   if (!(rsc=_evgl_tls_resource_get(ee)))
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

   if (evgl_direct_enabled(evgl_engine))
     {
        if (!(rsc->current_ctx->current_fbo))
          {
             if ((!ctx->direct_scissor))
               {
                  glEnable(GL_SCISSOR_TEST);
                  ctx->direct_scissor = 1;
               }

             img = rsc->direct_img_obj;
             rot = ee->funcs->rotation_angle_get(ee->engine_data);

             compute_gl_coordinates(img, rot, 0, 0, 0, 0, 0, oc, nc);


             if ((ctx->scissor_upated) && (ctx->scissor_enabled))
               {
                  glScissor(ctx->scissor_coord[0], ctx->scissor_coord[1],
                            ctx->scissor_coord[2], ctx->scissor_coord[3]);
                  ctx->direct_scissor = 0;
               }
             else
                glScissor(oc[0], oc[1], oc[2], oc[3]);

             GLERR(__FUNCTION__, __FILE__, __LINE__, "");

             glClear(mask);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        else
          {
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  glDisable(GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             glClear(mask);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
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
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   goto finish;

finish:
   EVGL_FUNC_END();
}

static void
evgl_glEnable(GLenum cap)
{
   EVGL_Context *ctx;

   EVGL_FUNC_BEGIN();

   ctx = _evgl_current_context_get();

   if (cap == GL_SCISSOR_TEST)
      if (ctx) ctx->scissor_enabled = 1;
   glEnable(cap);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}

static void
evgl_glDisable(GLenum cap)
{
   EVGL_Context *ctx;

   EVGL_FUNC_BEGIN();

   ctx = _evgl_current_context_get();

   if (cap == GL_SCISSOR_TEST)
      if (ctx) ctx->scissor_enabled = 0;
   glDisable(cap);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   goto finish;

finish:
   EVGL_FUNC_END();
}


static void
evgl_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels)
{
   EVGL_Engine *ee = evgl_engine;
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   Evas_Object *img;
   int rot = 0;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};

   EVGL_FUNC_BEGIN();

   if (!(rsc=_evgl_tls_resource_get(ee)))
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

   if (evgl_direct_enabled(evgl_engine))
     {

        if (!(rsc->current_ctx->current_fbo))
          {
             img = rsc->direct_img_obj;
             rot = ee->funcs->rotation_angle_get(ee->engine_data);

             compute_gl_coordinates(img, rot, 1, x, y, width, height, oc, nc);
             glReadPixels(nc[0], nc[1], nc[2], nc[3], format, type, pixels);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        else
          {
             glReadPixels(x, y, width, height, format, type, pixels);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
     }
   else
     {
        glReadPixels(x, y, width, height, format, type, pixels);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   goto finish;

finish:
   EVGL_FUNC_END();
}

static void
evgl_glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_Engine *ee = evgl_engine;
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   Evas_Object *img;
   int rot = 0;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};

   EVGL_FUNC_BEGIN();

   if (!(rsc=_evgl_tls_resource_get(ee)))
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

   if (evgl_direct_enabled(evgl_engine))
     {
        if (!(rsc->current_ctx->current_fbo))
          {
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  glDisable(GL_SCISSOR_TEST);
               }

             img = rsc->direct_img_obj;
             rot = ee->funcs->rotation_angle_get(ee->engine_data);
             
             compute_gl_coordinates(img, rot, 1, x, y, width, height, oc, nc);
             glScissor(nc[0], nc[1], nc[2], nc[3]);

             // Update coordinates
             ctx->scissor_coord[0] = nc[0];
             ctx->scissor_coord[1] = nc[1];
             ctx->scissor_coord[2] = nc[2];
             ctx->scissor_coord[3] = nc[3];

             ctx->direct_scissor = 0;

             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             // Check....!!!!
             ctx->scissor_upated = 1;
          }
        else
          {
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  glDisable(GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             glScissor(x, y, width, height);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");

             ctx->scissor_upated = 0;
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
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   goto finish;

finish:
   EVGL_FUNC_END();
}

static void
evgl_glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
   EVGL_Engine *ee = evgl_engine;
   EVGL_Resource *rsc;
   EVGL_Context *ctx;
   Evas_Object *img;
   int rot = 0;
   int oc[4] = {0,0,0,0}, nc[4] = {0,0,0,0};

   if (!(rsc=_evgl_tls_resource_get(ee)))
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

   if (evgl_direct_enabled(evgl_engine))
     {
        if (!(rsc->current_ctx->current_fbo))
          {
             if ((!ctx->direct_scissor))
               {
                  glEnable(GL_SCISSOR_TEST);
                  ctx->direct_scissor = 1;
               }

             img = rsc->direct_img_obj;
             rot = ee->funcs->rotation_angle_get(ee->engine_data);

             compute_gl_coordinates(img, rot, 0, x, y, width, height, oc, nc);

             if ((ctx->scissor_upated) && (ctx->scissor_enabled))
               {
                  glScissor(ctx->scissor_coord[0], ctx->scissor_coord[1],
                            ctx->scissor_coord[2], ctx->scissor_coord[3]);
                  ctx->direct_scissor = 0;
               }
             else
                glScissor(oc[0], oc[1], oc[2], oc[3]);

             glViewport(nc[0], nc[1], nc[2], nc[3]);
          }
        else
          {
             if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
               {
                  glDisable(GL_SCISSOR_TEST);
                  ctx->direct_scissor = 0;
               }

             glViewport(x, y, width, height);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
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
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   goto finish;

finish:
   EVGL_FUNC_END();
}
//-------------------------------------------------------------//



void
_evgl_api_get(Evas_GL_API *funcs)
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

   evgl_api_ext_get(funcs);
}

