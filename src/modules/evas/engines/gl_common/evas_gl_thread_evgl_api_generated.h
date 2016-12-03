/*
 * This is an automatically generated file using a python script.
 * ($EFL_HOME/src/utils/evas/generate_gl_thread_api.py)
 * Recommend that you modify data files ($EFL_HOME/src/utils/evas/gl_api_def.txt)
 * and make use of scripts if you need to fix them.
 */

#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */


extern void (*orig_evgl_api_glActiveTexture)(GLenum texture);
EAPI void evas_glActiveTexture_evgl_api_th(GLenum texture);

extern void (*orig_evgl_api_glAttachShader)(GLuint program, GLuint shader);
EAPI void evas_glAttachShader_evgl_api_th(GLuint program, GLuint shader);

extern void (*orig_evgl_api_glBindAttribLocation)(GLuint program, GLuint index, const char* name);
EAPI void evas_glBindAttribLocation_evgl_api_th(GLuint program, GLuint index, const char* name);

extern void (*orig_evgl_api_glBindBuffer)(GLenum target, GLuint buffer);
EAPI void evas_glBindBuffer_evgl_api_th(GLenum target, GLuint buffer);

extern void (*orig_evgl_api_glBindFramebuffer)(GLenum target, GLuint framebuffer);
EAPI void evas_glBindFramebuffer_evgl_api_th(GLenum target, GLuint framebuffer);

extern void (*orig_evgl_api_glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
EAPI void evas_glBindRenderbuffer_evgl_api_th(GLenum target, GLuint renderbuffer);

extern void (*orig_evgl_api_glBindTexture)(GLenum target, GLuint texture);
EAPI void evas_glBindTexture_evgl_api_th(GLenum target, GLuint texture);

extern void (*orig_evgl_api_glBlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
EAPI void evas_glBlendColor_evgl_api_th(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

extern void (*orig_evgl_api_glBlendEquation)(GLenum mode);
EAPI void evas_glBlendEquation_evgl_api_th(GLenum mode);

extern void (*orig_evgl_api_glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
EAPI void evas_glBlendEquationSeparate_evgl_api_th(GLenum modeRGB, GLenum modeAlpha);

extern void (*orig_evgl_api_glBlendFunc)(GLenum sfactor, GLenum dfactor);
EAPI void evas_glBlendFunc_evgl_api_th(GLenum sfactor, GLenum dfactor);

extern void (*orig_evgl_api_glBlendFuncSeparate)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
EAPI void evas_glBlendFuncSeparate_evgl_api_th(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

extern void (*orig_evgl_api_glBufferData)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
EAPI void evas_glBufferData_evgl_api_th(GLenum target, GLsizeiptr size, const void* data, GLenum usage);

extern void (*orig_evgl_api_glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
EAPI void evas_glBufferSubData_evgl_api_th(GLenum target, GLintptr offset, GLsizeiptr size, const void* data);

extern GLenum (*orig_evgl_api_glCheckFramebufferStatus)(GLenum target);
EAPI GLenum evas_glCheckFramebufferStatus_evgl_api_th(GLenum target);

extern void (*orig_evgl_api_glClear)(GLbitfield mask);
EAPI void evas_glClear_evgl_api_th(GLbitfield mask);

extern void (*orig_evgl_api_glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
EAPI void evas_glClearColor_evgl_api_th(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);

extern void (*orig_evgl_api_glClearDepthf)(GLclampf depth);
EAPI void evas_glClearDepthf_evgl_api_th(GLclampf depth);

extern void (*orig_evgl_api_glClearStencil)(GLint s);
EAPI void evas_glClearStencil_evgl_api_th(GLint s);

extern void (*orig_evgl_api_glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
EAPI void evas_glColorMask_evgl_api_th(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

extern void (*orig_evgl_api_glCompileShader)(GLuint shader);
EAPI void evas_glCompileShader_evgl_api_th(GLuint shader);

extern void (*orig_evgl_api_glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data);
EAPI void evas_glCompressedTexImage2D_evgl_api_th(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data);

extern void (*orig_evgl_api_glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);
EAPI void evas_glCompressedTexSubImage2D_evgl_api_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);

extern void (*orig_evgl_api_glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
EAPI void evas_glCopyTexImage2D_evgl_api_th(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);

extern void (*orig_evgl_api_glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
EAPI void evas_glCopyTexSubImage2D_evgl_api_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);

extern GLuint (*orig_evgl_api_glCreateProgram)(void);
EAPI GLuint evas_glCreateProgram_evgl_api_th(void);

extern GLuint (*orig_evgl_api_glCreateShader)(GLenum type);
EAPI GLuint evas_glCreateShader_evgl_api_th(GLenum type);

extern void (*orig_evgl_api_glCullFace)(GLenum mode);
EAPI void evas_glCullFace_evgl_api_th(GLenum mode);

extern void (*orig_evgl_api_glDeleteBuffers)(GLsizei n, const GLuint* buffers);
EAPI void evas_glDeleteBuffers_evgl_api_th(GLsizei n, const GLuint* buffers);

extern void (*orig_evgl_api_glDeleteFramebuffers)(GLsizei n, const GLuint* framebuffers);
EAPI void evas_glDeleteFramebuffers_evgl_api_th(GLsizei n, const GLuint* framebuffers);

extern void (*orig_evgl_api_glDeleteProgram)(GLuint program);
EAPI void evas_glDeleteProgram_evgl_api_th(GLuint program);

extern void (*orig_evgl_api_glDeleteRenderbuffers)(GLsizei n, const GLuint* renderbuffers);
EAPI void evas_glDeleteRenderbuffers_evgl_api_th(GLsizei n, const GLuint* renderbuffers);

extern void (*orig_evgl_api_glDeleteShader)(GLuint shader);
EAPI void evas_glDeleteShader_evgl_api_th(GLuint shader);

extern void (*orig_evgl_api_glDeleteTextures)(GLsizei n, const GLuint* textures);
EAPI void evas_glDeleteTextures_evgl_api_th(GLsizei n, const GLuint* textures);

extern void (*orig_evgl_api_glDepthFunc)(GLenum func);
EAPI void evas_glDepthFunc_evgl_api_th(GLenum func);

extern void (*orig_evgl_api_glDepthMask)(GLboolean flag);
EAPI void evas_glDepthMask_evgl_api_th(GLboolean flag);

extern void (*orig_evgl_api_glDepthRangef)(GLclampf zNear, GLclampf zFar);
EAPI void evas_glDepthRangef_evgl_api_th(GLclampf zNear, GLclampf zFar);

extern void (*orig_evgl_api_glDetachShader)(GLuint program, GLuint shader);
EAPI void evas_glDetachShader_evgl_api_th(GLuint program, GLuint shader);

extern void (*orig_evgl_api_glDisable)(GLenum cap);
EAPI void evas_glDisable_evgl_api_th(GLenum cap);

extern void (*orig_evgl_api_glDisableVertexAttribArray)(GLuint index);
EAPI void evas_glDisableVertexAttribArray_evgl_api_th(GLuint index);

extern void (*orig_evgl_api_glDrawArrays)(GLenum mode, GLint first, GLsizei count);
EAPI void evas_glDrawArrays_evgl_api_th(GLenum mode, GLint first, GLsizei count);

extern void (*orig_evgl_api_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const void* indices);
EAPI void evas_glDrawElements_evgl_api_th(GLenum mode, GLsizei count, GLenum type, const void* indices);

extern void (*orig_evgl_api_glEnable)(GLenum cap);
EAPI void evas_glEnable_evgl_api_th(GLenum cap);

extern void (*orig_evgl_api_glEnableVertexAttribArray)(GLuint index);
EAPI void evas_glEnableVertexAttribArray_evgl_api_th(GLuint index);

extern void (*orig_evgl_api_glFinish)(void);
EAPI void evas_glFinish_evgl_api_th(void);

extern void (*orig_evgl_api_glFlush)(void);
EAPI void evas_glFlush_evgl_api_th(void);

extern void (*orig_evgl_api_glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
EAPI void evas_glFramebufferRenderbuffer_evgl_api_th(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

extern void (*orig_evgl_api_glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
EAPI void evas_glFramebufferTexture2D_evgl_api_th(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

extern void (*orig_evgl_api_glFrontFace)(GLenum mode);
EAPI void evas_glFrontFace_evgl_api_th(GLenum mode);

extern void (*orig_evgl_api_glGenBuffers)(GLsizei n, GLuint* buffers);
EAPI void evas_glGenBuffers_evgl_api_th(GLsizei n, GLuint* buffers);

extern void (*orig_evgl_api_glGenerateMipmap)(GLenum target);
EAPI void evas_glGenerateMipmap_evgl_api_th(GLenum target);

extern void (*orig_evgl_api_glGenFramebuffers)(GLsizei n, GLuint* framebuffers);
EAPI void evas_glGenFramebuffers_evgl_api_th(GLsizei n, GLuint* framebuffers);

extern void (*orig_evgl_api_glGenRenderbuffers)(GLsizei n, GLuint* renderbuffers);
EAPI void evas_glGenRenderbuffers_evgl_api_th(GLsizei n, GLuint* renderbuffers);

extern void (*orig_evgl_api_glGenTextures)(GLsizei n, GLuint* textures);
EAPI void evas_glGenTextures_evgl_api_th(GLsizei n, GLuint* textures);

extern void (*orig_evgl_api_glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name);
EAPI void evas_glGetActiveAttrib_evgl_api_th(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name);

extern void (*orig_evgl_api_glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name);
EAPI void evas_glGetActiveUniform_evgl_api_th(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name);

extern void (*orig_evgl_api_glGetAttachedShaders)(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
EAPI void evas_glGetAttachedShaders_evgl_api_th(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);

extern int (*orig_evgl_api_glGetAttribLocation)(GLuint program, const char* name);
EAPI int evas_glGetAttribLocation_evgl_api_th(GLuint program, const char* name);

extern void (*orig_evgl_api_glGetBooleanv)(GLenum pname, GLboolean* params);
EAPI void evas_glGetBooleanv_evgl_api_th(GLenum pname, GLboolean* params);

extern void (*orig_evgl_api_glGetBufferParameteriv)(GLenum target, GLenum pname, GLint* params);
EAPI void evas_glGetBufferParameteriv_evgl_api_th(GLenum target, GLenum pname, GLint* params);

extern GLenum (*orig_evgl_api_glGetError)(void);
EAPI GLenum evas_glGetError_evgl_api_th(void);

extern void (*orig_evgl_api_glGetFloatv)(GLenum pname, GLfloat* params);
EAPI void evas_glGetFloatv_evgl_api_th(GLenum pname, GLfloat* params);

extern void (*orig_evgl_api_glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint* params);
EAPI void evas_glGetFramebufferAttachmentParameteriv_evgl_api_th(GLenum target, GLenum attachment, GLenum pname, GLint* params);

extern void (*orig_evgl_api_glGetIntegerv)(GLenum pname, GLint* params);
EAPI void evas_glGetIntegerv_evgl_api_th(GLenum pname, GLint* params);

extern void (*orig_evgl_api_glGetProgramiv)(GLuint program, GLenum pname, GLint* params);
EAPI void evas_glGetProgramiv_evgl_api_th(GLuint program, GLenum pname, GLint* params);

extern void (*orig_evgl_api_glGetProgramInfoLog)(GLuint program, GLsizei bufsize, GLsizei* length, char* infolog);
EAPI void evas_glGetProgramInfoLog_evgl_api_th(GLuint program, GLsizei bufsize, GLsizei* length, char* infolog);

extern void (*orig_evgl_api_glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params);
EAPI void evas_glGetRenderbufferParameteriv_evgl_api_th(GLenum target, GLenum pname, GLint* params);

extern void (*orig_evgl_api_glGetShaderiv)(GLuint shader, GLenum pname, GLint* params);
EAPI void evas_glGetShaderiv_evgl_api_th(GLuint shader, GLenum pname, GLint* params);

extern void (*orig_evgl_api_glGetShaderInfoLog)(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog);
EAPI void evas_glGetShaderInfoLog_evgl_api_th(GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog);

extern void (*orig_evgl_api_glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
EAPI void evas_glGetShaderPrecisionFormat_evgl_api_th(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);

extern void (*orig_evgl_api_glGetShaderSource)(GLuint shader, GLsizei bufsize, GLsizei* length, char* source);
EAPI void evas_glGetShaderSource_evgl_api_th(GLuint shader, GLsizei bufsize, GLsizei* length, char* source);

extern const GLubyte* (*orig_evgl_api_glGetString)(GLenum name);
EAPI const GLubyte* evas_glGetString_evgl_api_th(GLenum name);

extern void (*orig_evgl_api_glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat* params);
EAPI void evas_glGetTexParameterfv_evgl_api_th(GLenum target, GLenum pname, GLfloat* params);

extern void (*orig_evgl_api_glGetTexParameteriv)(GLenum target, GLenum pname, GLint* params);
EAPI void evas_glGetTexParameteriv_evgl_api_th(GLenum target, GLenum pname, GLint* params);

extern void (*orig_evgl_api_glGetUniformfv)(GLuint program, GLint location, GLfloat* params);
EAPI void evas_glGetUniformfv_evgl_api_th(GLuint program, GLint location, GLfloat* params);

extern void (*orig_evgl_api_glGetUniformiv)(GLuint program, GLint location, GLint* params);
EAPI void evas_glGetUniformiv_evgl_api_th(GLuint program, GLint location, GLint* params);

extern int (*orig_evgl_api_glGetUniformLocation)(GLuint program, const char* name);
EAPI int evas_glGetUniformLocation_evgl_api_th(GLuint program, const char* name);

extern void (*orig_evgl_api_glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat* params);
EAPI void evas_glGetVertexAttribfv_evgl_api_th(GLuint index, GLenum pname, GLfloat* params);

extern void (*orig_evgl_api_glGetVertexAttribiv)(GLuint index, GLenum pname, GLint* params);
EAPI void evas_glGetVertexAttribiv_evgl_api_th(GLuint index, GLenum pname, GLint* params);

extern void (*orig_evgl_api_glGetVertexAttribPointerv)(GLuint index, GLenum pname, void** pointer);
EAPI void evas_glGetVertexAttribPointerv_evgl_api_th(GLuint index, GLenum pname, void** pointer);

extern void (*orig_evgl_api_glHint)(GLenum target, GLenum mode);
EAPI void evas_glHint_evgl_api_th(GLenum target, GLenum mode);

extern GLboolean (*orig_evgl_api_glIsBuffer)(GLuint buffer);
EAPI GLboolean evas_glIsBuffer_evgl_api_th(GLuint buffer);

extern GLboolean (*orig_evgl_api_glIsEnabled)(GLenum cap);
EAPI GLboolean evas_glIsEnabled_evgl_api_th(GLenum cap);

extern GLboolean (*orig_evgl_api_glIsFramebuffer)(GLuint framebuffer);
EAPI GLboolean evas_glIsFramebuffer_evgl_api_th(GLuint framebuffer);

extern GLboolean (*orig_evgl_api_glIsProgram)(GLuint program);
EAPI GLboolean evas_glIsProgram_evgl_api_th(GLuint program);

extern GLboolean (*orig_evgl_api_glIsRenderbuffer)(GLuint renderbuffer);
EAPI GLboolean evas_glIsRenderbuffer_evgl_api_th(GLuint renderbuffer);

extern GLboolean (*orig_evgl_api_glIsShader)(GLuint shader);
EAPI GLboolean evas_glIsShader_evgl_api_th(GLuint shader);

extern GLboolean (*orig_evgl_api_glIsTexture)(GLuint texture);
EAPI GLboolean evas_glIsTexture_evgl_api_th(GLuint texture);

extern void (*orig_evgl_api_glLineWidth)(GLfloat width);
EAPI void evas_glLineWidth_evgl_api_th(GLfloat width);

extern void (*orig_evgl_api_glLinkProgram)(GLuint program);
EAPI void evas_glLinkProgram_evgl_api_th(GLuint program);

extern void (*orig_evgl_api_glPixelStorei)(GLenum pname, GLint param);
EAPI void evas_glPixelStorei_evgl_api_th(GLenum pname, GLint param);

extern void (*orig_evgl_api_glPolygonOffset)(GLfloat factor, GLfloat units);
EAPI void evas_glPolygonOffset_evgl_api_th(GLfloat factor, GLfloat units);

extern void (*orig_evgl_api_glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
EAPI void evas_glReadPixels_evgl_api_th(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);

extern void (*orig_evgl_api_glReleaseShaderCompiler)(void);
EAPI void evas_glReleaseShaderCompiler_evgl_api_th(void);

extern void (*orig_evgl_api_glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
EAPI void evas_glRenderbufferStorage_evgl_api_th(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

extern void (*orig_evgl_api_glSampleCoverage)(GLclampf value, GLboolean invert);
EAPI void evas_glSampleCoverage_evgl_api_th(GLclampf value, GLboolean invert);

extern void (*orig_evgl_api_glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
EAPI void evas_glScissor_evgl_api_th(GLint x, GLint y, GLsizei width, GLsizei height);

extern void (*orig_evgl_api_glShaderBinary)(GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length);
EAPI void evas_glShaderBinary_evgl_api_th(GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length);

extern void (*orig_evgl_api_glShaderSource)(GLuint shader, GLsizei count, const char* const * string, const GLint* length);
EAPI void evas_glShaderSource_evgl_api_th(GLuint shader, GLsizei count, const char* const * string, const GLint* length);

extern void (*orig_evgl_api_glStencilFunc)(GLenum func, GLint ref, GLuint mask);
EAPI void evas_glStencilFunc_evgl_api_th(GLenum func, GLint ref, GLuint mask);

extern void (*orig_evgl_api_glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
EAPI void evas_glStencilFuncSeparate_evgl_api_th(GLenum face, GLenum func, GLint ref, GLuint mask);

extern void (*orig_evgl_api_glStencilMask)(GLuint mask);
EAPI void evas_glStencilMask_evgl_api_th(GLuint mask);

extern void (*orig_evgl_api_glStencilMaskSeparate)(GLenum face, GLuint mask);
EAPI void evas_glStencilMaskSeparate_evgl_api_th(GLenum face, GLuint mask);

extern void (*orig_evgl_api_glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
EAPI void evas_glStencilOp_evgl_api_th(GLenum fail, GLenum zfail, GLenum zpass);

extern void (*orig_evgl_api_glStencilOpSeparate)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
EAPI void evas_glStencilOpSeparate_evgl_api_th(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);

extern void (*orig_evgl_api_glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
EAPI void evas_glTexImage2D_evgl_api_th(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);

extern void (*orig_evgl_api_glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
EAPI void evas_glTexParameterf_evgl_api_th(GLenum target, GLenum pname, GLfloat param);

extern void (*orig_evgl_api_glTexParameterfv)(GLenum target, GLenum pname, const GLfloat* params);
EAPI void evas_glTexParameterfv_evgl_api_th(GLenum target, GLenum pname, const GLfloat* params);

extern void (*orig_evgl_api_glTexParameteri)(GLenum target, GLenum pname, GLint param);
EAPI void evas_glTexParameteri_evgl_api_th(GLenum target, GLenum pname, GLint param);

extern void (*orig_evgl_api_glTexParameteriv)(GLenum target, GLenum pname, const GLint* params);
EAPI void evas_glTexParameteriv_evgl_api_th(GLenum target, GLenum pname, const GLint* params);

extern void (*orig_evgl_api_glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
EAPI void evas_glTexSubImage2D_evgl_api_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);

extern void (*orig_evgl_api_glUniform1f)(GLint location, GLfloat x);
EAPI void evas_glUniform1f_evgl_api_th(GLint location, GLfloat x);

extern void (*orig_evgl_api_glUniform1fv)(GLint location, GLsizei count, const GLfloat* v);
EAPI void evas_glUniform1fv_evgl_api_th(GLint location, GLsizei count, const GLfloat* v);

extern void (*orig_evgl_api_glUniform1i)(GLint location, GLint x);
EAPI void evas_glUniform1i_evgl_api_th(GLint location, GLint x);

extern void (*orig_evgl_api_glUniform1iv)(GLint location, GLsizei count, const GLint* v);
EAPI void evas_glUniform1iv_evgl_api_th(GLint location, GLsizei count, const GLint* v);

extern void (*orig_evgl_api_glUniform2f)(GLint location, GLfloat x, GLfloat y);
EAPI void evas_glUniform2f_evgl_api_th(GLint location, GLfloat x, GLfloat y);

extern void (*orig_evgl_api_glUniform2fv)(GLint location, GLsizei count, const GLfloat* v);
EAPI void evas_glUniform2fv_evgl_api_th(GLint location, GLsizei count, const GLfloat* v);

extern void (*orig_evgl_api_glUniform2i)(GLint location, GLint x, GLint y);
EAPI void evas_glUniform2i_evgl_api_th(GLint location, GLint x, GLint y);

extern void (*orig_evgl_api_glUniform2iv)(GLint location, GLsizei count, const GLint* v);
EAPI void evas_glUniform2iv_evgl_api_th(GLint location, GLsizei count, const GLint* v);

extern void (*orig_evgl_api_glUniform3f)(GLint location, GLfloat x, GLfloat y, GLfloat z);
EAPI void evas_glUniform3f_evgl_api_th(GLint location, GLfloat x, GLfloat y, GLfloat z);

extern void (*orig_evgl_api_glUniform3fv)(GLint location, GLsizei count, const GLfloat* v);
EAPI void evas_glUniform3fv_evgl_api_th(GLint location, GLsizei count, const GLfloat* v);

extern void (*orig_evgl_api_glUniform3i)(GLint location, GLint x, GLint y, GLint z);
EAPI void evas_glUniform3i_evgl_api_th(GLint location, GLint x, GLint y, GLint z);

extern void (*orig_evgl_api_glUniform3iv)(GLint location, GLsizei count, const GLint* v);
EAPI void evas_glUniform3iv_evgl_api_th(GLint location, GLsizei count, const GLint* v);

extern void (*orig_evgl_api_glUniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
EAPI void evas_glUniform4f_evgl_api_th(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

extern void (*orig_evgl_api_glUniform4fv)(GLint location, GLsizei count, const GLfloat* v);
EAPI void evas_glUniform4fv_evgl_api_th(GLint location, GLsizei count, const GLfloat* v);

extern void (*orig_evgl_api_glUniform4i)(GLint location, GLint x, GLint y, GLint z, GLint w);
EAPI void evas_glUniform4i_evgl_api_th(GLint location, GLint x, GLint y, GLint z, GLint w);

extern void (*orig_evgl_api_glUniform4iv)(GLint location, GLsizei count, const GLint* v);
EAPI void evas_glUniform4iv_evgl_api_th(GLint location, GLsizei count, const GLint* v);

extern void (*orig_evgl_api_glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
EAPI void evas_glUniformMatrix2fv_evgl_api_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

extern void (*orig_evgl_api_glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
EAPI void evas_glUniformMatrix3fv_evgl_api_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

extern void (*orig_evgl_api_glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
EAPI void evas_glUniformMatrix4fv_evgl_api_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);

extern void (*orig_evgl_api_glUseProgram)(GLuint program);
EAPI void evas_glUseProgram_evgl_api_th(GLuint program);

extern void (*orig_evgl_api_glValidateProgram)(GLuint program);
EAPI void evas_glValidateProgram_evgl_api_th(GLuint program);

extern void (*orig_evgl_api_glVertexAttrib1f)(GLuint indx, GLfloat x);
EAPI void evas_glVertexAttrib1f_evgl_api_th(GLuint indx, GLfloat x);

extern void (*orig_evgl_api_glVertexAttrib1fv)(GLuint indx, const GLfloat* values);
EAPI void evas_glVertexAttrib1fv_evgl_api_th(GLuint indx, const GLfloat* values);

extern void (*orig_evgl_api_glVertexAttrib2f)(GLuint indx, GLfloat x, GLfloat y);
EAPI void evas_glVertexAttrib2f_evgl_api_th(GLuint indx, GLfloat x, GLfloat y);

extern void (*orig_evgl_api_glVertexAttrib2fv)(GLuint indx, const GLfloat* values);
EAPI void evas_glVertexAttrib2fv_evgl_api_th(GLuint indx, const GLfloat* values);

extern void (*orig_evgl_api_glVertexAttrib3f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z);
EAPI void evas_glVertexAttrib3f_evgl_api_th(GLuint indx, GLfloat x, GLfloat y, GLfloat z);

extern void (*orig_evgl_api_glVertexAttrib3fv)(GLuint indx, const GLfloat* values);
EAPI void evas_glVertexAttrib3fv_evgl_api_th(GLuint indx, const GLfloat* values);

extern void (*orig_evgl_api_glVertexAttrib4f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
EAPI void evas_glVertexAttrib4f_evgl_api_th(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);

extern void (*orig_evgl_api_glVertexAttrib4fv)(GLuint indx, const GLfloat* values);
EAPI void evas_glVertexAttrib4fv_evgl_api_th(GLuint indx, const GLfloat* values);

extern void (*orig_evgl_api_glVertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr);
EAPI void evas_glVertexAttribPointer_evgl_api_th(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr);

extern void (*orig_evgl_api_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
EAPI void evas_glViewport_evgl_api_th(GLint x, GLint y, GLsizei width, GLsizei height);

extern void (*orig_evgl_api_glEvasGLImageTargetTexture2DOES)(GLenum target, EvasGLImage image);
EAPI void evas_glEvasGLImageTargetTexture2DOES_evgl_api_th(GLenum target, EvasGLImage image);

extern void (*orig_evgl_api_glEvasGLImageTargetRenderbufferStorageOES)(GLenum target, EvasGLImage image);
EAPI void evas_glEvasGLImageTargetRenderbufferStorageOES_evgl_api_th(GLenum target, EvasGLImage image);

extern void (*orig_evgl_api_glGetProgramBinaryOES)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
EAPI void evas_glGetProgramBinaryOES_evgl_api_th(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);

extern void (*orig_evgl_api_glProgramBinaryOES)(GLuint program, GLenum binaryFormat, const void *binary, GLint length);
EAPI void evas_glProgramBinaryOES_evgl_api_th(GLuint program, GLenum binaryFormat, const void *binary, GLint length);

extern void * (*orig_evgl_api_glMapBufferOES)(GLenum target, GLenum access);
EAPI void * evas_glMapBufferOES_evgl_api_th(GLenum target, GLenum access);

extern GLboolean (*orig_evgl_api_glUnmapBufferOES)(GLenum target);
EAPI GLboolean evas_glUnmapBufferOES_evgl_api_th(GLenum target);

extern void (*orig_evgl_api_glGetBufferPointervOES)(GLenum target, GLenum pname, void** params);
EAPI void evas_glGetBufferPointervOES_evgl_api_th(GLenum target, GLenum pname, void** params);

extern void (*orig_evgl_api_glTexImage3DOES)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels);
EAPI void evas_glTexImage3DOES_evgl_api_th(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels);

extern void (*orig_evgl_api_glTexSubImage3DOES)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
EAPI void evas_glTexSubImage3DOES_evgl_api_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);

extern void (*orig_evgl_api_glCopyTexSubImage3DOES)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
EAPI void evas_glCopyTexSubImage3DOES_evgl_api_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

extern void (*orig_evgl_api_glCompressedTexImage3DOES)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data);
EAPI void evas_glCompressedTexImage3DOES_evgl_api_th(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data);

extern void (*orig_evgl_api_glCompressedTexSubImage3DOES)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data);
EAPI void evas_glCompressedTexSubImage3DOES_evgl_api_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data);

extern void (*orig_evgl_api_glFramebufferTexture3DOES)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
EAPI void evas_glFramebufferTexture3DOES_evgl_api_th(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);

extern void (*orig_evgl_api_glGetPerfMonitorGroupsAMD)(GLint* numGroups, GLsizei groupsSize, GLuint* groups);
EAPI void evas_glGetPerfMonitorGroupsAMD_evgl_api_th(GLint* numGroups, GLsizei groupsSize, GLuint* groups);

extern void (*orig_evgl_api_glGetPerfMonitorCountersAMD)(GLuint group, GLint* numCounters, GLint* maxActiveCounters, GLsizei counterSize, GLuint* counters);
EAPI void evas_glGetPerfMonitorCountersAMD_evgl_api_th(GLuint group, GLint* numCounters, GLint* maxActiveCounters, GLsizei counterSize, GLuint* counters);

extern void (*orig_evgl_api_glGetPerfMonitorGroupStringAMD)(GLuint group, GLsizei bufSize, GLsizei* length, char* groupString);
EAPI void evas_glGetPerfMonitorGroupStringAMD_evgl_api_th(GLuint group, GLsizei bufSize, GLsizei* length, char* groupString);

extern void (*orig_evgl_api_glGetPerfMonitorCounterStringAMD)(GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, char* counterString);
EAPI void evas_glGetPerfMonitorCounterStringAMD_evgl_api_th(GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, char* counterString);

extern void (*orig_evgl_api_glGetPerfMonitorCounterInfoAMD)(GLuint group, GLuint counter, GLenum pname, void* data);
EAPI void evas_glGetPerfMonitorCounterInfoAMD_evgl_api_th(GLuint group, GLuint counter, GLenum pname, void* data);

extern void (*orig_evgl_api_glGenPerfMonitorsAMD)(GLsizei n, GLuint* monitors);
EAPI void evas_glGenPerfMonitorsAMD_evgl_api_th(GLsizei n, GLuint* monitors);

extern void (*orig_evgl_api_glDeletePerfMonitorsAMD)(GLsizei n, GLuint* monitors);
EAPI void evas_glDeletePerfMonitorsAMD_evgl_api_th(GLsizei n, GLuint* monitors);

extern void (*orig_evgl_api_glSelectPerfMonitorCountersAMD)(GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* countersList);
EAPI void evas_glSelectPerfMonitorCountersAMD_evgl_api_th(GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* countersList);

extern void (*orig_evgl_api_glBeginPerfMonitorAMD)(GLuint monitor);
EAPI void evas_glBeginPerfMonitorAMD_evgl_api_th(GLuint monitor);

extern void (*orig_evgl_api_glEndPerfMonitorAMD)(GLuint monitor);
EAPI void evas_glEndPerfMonitorAMD_evgl_api_th(GLuint monitor);

extern void (*orig_evgl_api_glGetPerfMonitorCounterDataAMD)(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint* bytesWritten);
EAPI void evas_glGetPerfMonitorCounterDataAMD_evgl_api_th(GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint* bytesWritten);

extern void (*orig_evgl_api_glDiscardFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum* attachments);
EAPI void evas_glDiscardFramebuffer_evgl_api_th(GLenum target, GLsizei numAttachments, const GLenum* attachments);

extern void (*orig_evgl_api_glDiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum* attachments);
EAPI void evas_glDiscardFramebufferEXT_evgl_api_th(GLenum target, GLsizei numAttachments, const GLenum* attachments);

extern void (*orig_evgl_api_glMultiDrawArrays)(GLenum mode, GLint* first, GLsizei* count, GLsizei primcount);
EAPI void evas_glMultiDrawArrays_evgl_api_th(GLenum mode, GLint* first, GLsizei* count, GLsizei primcount);

extern void (*orig_evgl_api_glMultiDrawArraysEXT)(GLenum mode, GLint* first, GLsizei* count, GLsizei primcount);
EAPI void evas_glMultiDrawArraysEXT_evgl_api_th(GLenum mode, GLint* first, GLsizei* count, GLsizei primcount);

extern void (*orig_evgl_api_glMultiDrawElements)(GLenum mode, const GLsizei* count, GLenum type, const GLvoid** indices, GLsizei primcount);
EAPI void evas_glMultiDrawElements_evgl_api_th(GLenum mode, const GLsizei* count, GLenum type, const GLvoid** indices, GLsizei primcount);

extern void (*orig_evgl_api_glMultiDrawElementsEXT)(GLenum mode, const GLsizei* count, GLenum type, const GLvoid** indices, GLsizei primcount);
EAPI void evas_glMultiDrawElementsEXT_evgl_api_th(GLenum mode, const GLsizei* count, GLenum type, const GLvoid** indices, GLsizei primcount);

extern void (*orig_evgl_api_glDeleteFencesNV)(GLsizei n, const GLuint* fences);
EAPI void evas_glDeleteFencesNV_evgl_api_th(GLsizei n, const GLuint* fences);

extern void (*orig_evgl_api_glGenFencesNV)(GLsizei n, GLuint* fences);
EAPI void evas_glGenFencesNV_evgl_api_th(GLsizei n, GLuint* fences);

extern GLboolean (*orig_evgl_api_glIsFenceNV)(GLuint fence);
EAPI GLboolean evas_glIsFenceNV_evgl_api_th(GLuint fence);

extern GLboolean (*orig_evgl_api_glTestFenceNV)(GLuint fence);
EAPI GLboolean evas_glTestFenceNV_evgl_api_th(GLuint fence);

extern void (*orig_evgl_api_glGetFenceivNV)(GLuint fence, GLenum pname, GLint* params);
EAPI void evas_glGetFenceivNV_evgl_api_th(GLuint fence, GLenum pname, GLint* params);

extern void (*orig_evgl_api_glFinishFenceNV)(GLuint fence);
EAPI void evas_glFinishFenceNV_evgl_api_th(GLuint fence);

extern void (*orig_evgl_api_glSetFenceNV)(GLuint a, GLenum b);
EAPI void evas_glSetFenceNV_evgl_api_th(GLuint a, GLenum b);

extern void (*orig_evgl_api_glGetDriverControlsQCOM)(GLint* num, GLsizei size, GLuint* driverControls);
EAPI void evas_glGetDriverControlsQCOM_evgl_api_th(GLint* num, GLsizei size, GLuint* driverControls);

extern void (*orig_evgl_api_glGetDriverControlStringQCOM)(GLuint driverControl, GLsizei bufSize, GLsizei* length, char* driverControlString);
EAPI void evas_glGetDriverControlStringQCOM_evgl_api_th(GLuint driverControl, GLsizei bufSize, GLsizei* length, char* driverControlString);

extern void (*orig_evgl_api_glEnableDriverControlQCOM)(GLuint driverControl);
EAPI void evas_glEnableDriverControlQCOM_evgl_api_th(GLuint driverControl);

extern void (*orig_evgl_api_glDisableDriverControlQCOM)(GLuint driverControl);
EAPI void evas_glDisableDriverControlQCOM_evgl_api_th(GLuint driverControl);

extern void (*orig_evgl_api_glExtGetTexturesQCOM)(GLuint* textures, GLint maxTextures, GLint* numTextures);
EAPI void evas_glExtGetTexturesQCOM_evgl_api_th(GLuint* textures, GLint maxTextures, GLint* numTextures);

extern void (*orig_evgl_api_glExtGetBuffersQCOM)(GLuint* buffers, GLint maxBuffers, GLint* numBuffers);
EAPI void evas_glExtGetBuffersQCOM_evgl_api_th(GLuint* buffers, GLint maxBuffers, GLint* numBuffers);

extern void (*orig_evgl_api_glExtGetRenderbuffersQCOM)(GLuint* renderbuffers, GLint maxRenderbuffers, GLint* numRenderbuffers);
EAPI void evas_glExtGetRenderbuffersQCOM_evgl_api_th(GLuint* renderbuffers, GLint maxRenderbuffers, GLint* numRenderbuffers);

extern void (*orig_evgl_api_glExtGetFramebuffersQCOM)(GLuint* framebuffers, GLint maxFramebuffers, GLint* numFramebuffers);
EAPI void evas_glExtGetFramebuffersQCOM_evgl_api_th(GLuint* framebuffers, GLint maxFramebuffers, GLint* numFramebuffers);

extern void (*orig_evgl_api_glExtGetTexLevelParameterivQCOM)(GLuint texture, GLenum face, GLint level, GLenum pname, GLint* params);
EAPI void evas_glExtGetTexLevelParameterivQCOM_evgl_api_th(GLuint texture, GLenum face, GLint level, GLenum pname, GLint* params);

extern void (*orig_evgl_api_glExtTexObjectStateOverrideiQCOM)(GLenum target, GLenum pname, GLint param);
EAPI void evas_glExtTexObjectStateOverrideiQCOM_evgl_api_th(GLenum target, GLenum pname, GLint param);

extern void (*orig_evgl_api_glExtGetTexSubImageQCOM)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, void* texels);
EAPI void evas_glExtGetTexSubImageQCOM_evgl_api_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, void* texels);

extern void (*orig_evgl_api_glExtGetBufferPointervQCOM)(GLenum target, void** params);
EAPI void evas_glExtGetBufferPointervQCOM_evgl_api_th(GLenum target, void** params);

extern void (*orig_evgl_api_glExtGetShadersQCOM)(GLuint* shaders, GLint maxShaders, GLint* numShaders);
EAPI void evas_glExtGetShadersQCOM_evgl_api_th(GLuint* shaders, GLint maxShaders, GLint* numShaders);

extern void (*orig_evgl_api_glExtGetProgramsQCOM)(GLuint* programs, GLint maxPrograms, GLint* numPrograms);
EAPI void evas_glExtGetProgramsQCOM_evgl_api_th(GLuint* programs, GLint maxPrograms, GLint* numPrograms);

extern GLboolean (*orig_evgl_api_glExtIsProgramBinaryQCOM)(GLuint program);
EAPI GLboolean evas_glExtIsProgramBinaryQCOM_evgl_api_th(GLuint program);

extern void (*orig_evgl_api_glExtGetProgramBinarySourceQCOM)(GLuint program, GLenum shadertype, char* source, GLint* length);
EAPI void evas_glExtGetProgramBinarySourceQCOM_evgl_api_th(GLuint program, GLenum shadertype, char* source, GLint* length);

extern void (*orig_evgl_api_glAlphaFunc)(GLenum func, GLclampf ref);
EAPI void evas_glAlphaFunc_evgl_api_th(GLenum func, GLclampf ref);

extern void (*orig_evgl_api_glClipPlanef)(GLenum plane, const GLfloat *equation);
EAPI void evas_glClipPlanef_evgl_api_th(GLenum plane, const GLfloat *equation);

extern void (*orig_evgl_api_glColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
EAPI void evas_glColor4f_evgl_api_th(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

extern void (*orig_evgl_api_glFogf)(GLenum pname, GLfloat param);
EAPI void evas_glFogf_evgl_api_th(GLenum pname, GLfloat param);

extern void (*orig_evgl_api_glFogfv)(GLenum pname, const GLfloat *params);
EAPI void evas_glFogfv_evgl_api_th(GLenum pname, const GLfloat *params);

extern void (*orig_evgl_api_glFrustumf)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
EAPI void evas_glFrustumf_evgl_api_th(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);

extern void (*orig_evgl_api_glGetClipPlanef)(GLenum pname, GLfloat eqn[4]);
EAPI void evas_glGetClipPlanef_evgl_api_th(GLenum pname, GLfloat eqn[4]);

extern void (*orig_evgl_api_glGetLightfv)(GLenum light, GLenum pname, GLfloat *params);
EAPI void evas_glGetLightfv_evgl_api_th(GLenum light, GLenum pname, GLfloat *params);

extern void (*orig_evgl_api_glGetMaterialfv)(GLenum face, GLenum pname, GLfloat *params);
EAPI void evas_glGetMaterialfv_evgl_api_th(GLenum face, GLenum pname, GLfloat *params);

extern void (*orig_evgl_api_glGetTexEnvfv)(GLenum env, GLenum pname, GLfloat *params);
EAPI void evas_glGetTexEnvfv_evgl_api_th(GLenum env, GLenum pname, GLfloat *params);

extern void (*orig_evgl_api_glLightModelf)(GLenum pname, GLfloat param);
EAPI void evas_glLightModelf_evgl_api_th(GLenum pname, GLfloat param);

extern void (*orig_evgl_api_glLightModelfv)(GLenum pname, const GLfloat *params);
EAPI void evas_glLightModelfv_evgl_api_th(GLenum pname, const GLfloat *params);

extern void (*orig_evgl_api_glLightf)(GLenum light, GLenum pname, GLfloat param);
EAPI void evas_glLightf_evgl_api_th(GLenum light, GLenum pname, GLfloat param);

extern void (*orig_evgl_api_glLightfv)(GLenum light, GLenum pname, const GLfloat *params);
EAPI void evas_glLightfv_evgl_api_th(GLenum light, GLenum pname, const GLfloat *params);

extern void (*orig_evgl_api_glLoadMatrixf)(const GLfloat *m);
EAPI void evas_glLoadMatrixf_evgl_api_th(const GLfloat *m);

extern void (*orig_evgl_api_glMaterialf)(GLenum face, GLenum pname, GLfloat param);
EAPI void evas_glMaterialf_evgl_api_th(GLenum face, GLenum pname, GLfloat param);

extern void (*orig_evgl_api_glMaterialfv)(GLenum face, GLenum pname, const GLfloat *params);
EAPI void evas_glMaterialfv_evgl_api_th(GLenum face, GLenum pname, const GLfloat *params);

extern void (*orig_evgl_api_glMultMatrixf)(const GLfloat *m);
EAPI void evas_glMultMatrixf_evgl_api_th(const GLfloat *m);

extern void (*orig_evgl_api_glMultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
EAPI void evas_glMultiTexCoord4f_evgl_api_th(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);

extern void (*orig_evgl_api_glNormal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
EAPI void evas_glNormal3f_evgl_api_th(GLfloat nx, GLfloat ny, GLfloat nz);

extern void (*orig_evgl_api_glOrthof)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
EAPI void evas_glOrthof_evgl_api_th(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);

extern void (*orig_evgl_api_glPointParameterf)(GLenum pname, GLfloat param);
EAPI void evas_glPointParameterf_evgl_api_th(GLenum pname, GLfloat param);

extern void (*orig_evgl_api_glPointParameterfv)(GLenum pname, const GLfloat *params);
EAPI void evas_glPointParameterfv_evgl_api_th(GLenum pname, const GLfloat *params);

extern void (*orig_evgl_api_glPointSize)(GLfloat size);
EAPI void evas_glPointSize_evgl_api_th(GLfloat size);

extern void (*orig_evgl_api_glPointSizePointerOES)(GLenum type, GLsizei stride, const GLvoid * pointer);
EAPI void evas_glPointSizePointerOES_evgl_api_th(GLenum type, GLsizei stride, const GLvoid * pointer);

extern void (*orig_evgl_api_glRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
EAPI void evas_glRotatef_evgl_api_th(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

extern void (*orig_evgl_api_glScalef)(GLfloat x, GLfloat y, GLfloat z);
EAPI void evas_glScalef_evgl_api_th(GLfloat x, GLfloat y, GLfloat z);

extern void (*orig_evgl_api_glTexEnvf)(GLenum target, GLenum pname, GLfloat param);
EAPI void evas_glTexEnvf_evgl_api_th(GLenum target, GLenum pname, GLfloat param);

extern void (*orig_evgl_api_glTexEnvfv)(GLenum target, GLenum pname, const GLfloat *params);
EAPI void evas_glTexEnvfv_evgl_api_th(GLenum target, GLenum pname, const GLfloat *params);

extern void (*orig_evgl_api_glTranslatef)(GLfloat x, GLfloat y, GLfloat z);
EAPI void evas_glTranslatef_evgl_api_th(GLfloat x, GLfloat y, GLfloat z);

extern void (*orig_evgl_api_glAlphaFuncx)(GLenum func, GLclampx ref);
EAPI void evas_glAlphaFuncx_evgl_api_th(GLenum func, GLclampx ref);

extern void (*orig_evgl_api_glClearColorx)(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);
EAPI void evas_glClearColorx_evgl_api_th(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);

extern void (*orig_evgl_api_glClearDepthx)(GLclampx depth);
EAPI void evas_glClearDepthx_evgl_api_th(GLclampx depth);

extern void (*orig_evgl_api_glClientActiveTexture)(GLenum texture);
EAPI void evas_glClientActiveTexture_evgl_api_th(GLenum texture);

extern void (*orig_evgl_api_glClipPlanex)(GLenum plane, const GLfixed *equation);
EAPI void evas_glClipPlanex_evgl_api_th(GLenum plane, const GLfixed *equation);

extern void (*orig_evgl_api_glColor4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
EAPI void evas_glColor4ub_evgl_api_th(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);

extern void (*orig_evgl_api_glColor4x)(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
EAPI void evas_glColor4x_evgl_api_th(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);

extern void (*orig_evgl_api_glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
EAPI void evas_glColorPointer_evgl_api_th(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

extern void (*orig_evgl_api_glDepthRangex)(GLclampx zNear, GLclampx zFar);
EAPI void evas_glDepthRangex_evgl_api_th(GLclampx zNear, GLclampx zFar);

extern void (*orig_evgl_api_glDisableClientState)(GLenum array);
EAPI void evas_glDisableClientState_evgl_api_th(GLenum array);

extern void (*orig_evgl_api_glEnableClientState)(GLenum array);
EAPI void evas_glEnableClientState_evgl_api_th(GLenum array);

extern void (*orig_evgl_api_glFogx)(GLenum pname, GLfixed param);
EAPI void evas_glFogx_evgl_api_th(GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glFogxv)(GLenum pname, const GLfixed *params);
EAPI void evas_glFogxv_evgl_api_th(GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glFrustumx)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
EAPI void evas_glFrustumx_evgl_api_th(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);

extern void (*orig_evgl_api_glGetClipPlanex)(GLenum pname, GLfixed eqn[4]);
EAPI void evas_glGetClipPlanex_evgl_api_th(GLenum pname, GLfixed eqn[4]);

extern void (*orig_evgl_api_glGetFixedv)(GLenum pname, GLfixed *params);
EAPI void evas_glGetFixedv_evgl_api_th(GLenum pname, GLfixed *params);

extern void (*orig_evgl_api_glGetLightxv)(GLenum light, GLenum pname, GLfixed *params);
EAPI void evas_glGetLightxv_evgl_api_th(GLenum light, GLenum pname, GLfixed *params);

extern void (*orig_evgl_api_glGetMaterialxv)(GLenum face, GLenum pname, GLfixed *params);
EAPI void evas_glGetMaterialxv_evgl_api_th(GLenum face, GLenum pname, GLfixed *params);

extern void (*orig_evgl_api_glGetPointerv)(GLenum pname, GLvoid **params);
EAPI void evas_glGetPointerv_evgl_api_th(GLenum pname, GLvoid **params);

extern void (*orig_evgl_api_glGetTexEnviv)(GLenum env, GLenum pname, GLint *params);
EAPI void evas_glGetTexEnviv_evgl_api_th(GLenum env, GLenum pname, GLint *params);

extern void (*orig_evgl_api_glGetTexEnvxv)(GLenum env, GLenum pname, GLfixed *params);
EAPI void evas_glGetTexEnvxv_evgl_api_th(GLenum env, GLenum pname, GLfixed *params);

extern void (*orig_evgl_api_glGetTexParameterxv)(GLenum target, GLenum pname, GLfixed *params);
EAPI void evas_glGetTexParameterxv_evgl_api_th(GLenum target, GLenum pname, GLfixed *params);

extern void (*orig_evgl_api_glLightModelx)(GLenum pname, GLfixed param);
EAPI void evas_glLightModelx_evgl_api_th(GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glLightModelxv)(GLenum pname, const GLfixed *params);
EAPI void evas_glLightModelxv_evgl_api_th(GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glLightx)(GLenum light, GLenum pname, GLfixed param);
EAPI void evas_glLightx_evgl_api_th(GLenum light, GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glLightxv)(GLenum light, GLenum pname, const GLfixed *params);
EAPI void evas_glLightxv_evgl_api_th(GLenum light, GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glLineWidthx)(GLfixed width);
EAPI void evas_glLineWidthx_evgl_api_th(GLfixed width);

extern void (*orig_evgl_api_glLoadIdentity)(void);
EAPI void evas_glLoadIdentity_evgl_api_th(void);

extern void (*orig_evgl_api_glLoadMatrixx)(const GLfixed *m);
EAPI void evas_glLoadMatrixx_evgl_api_th(const GLfixed *m);

extern void (*orig_evgl_api_glLogicOp)(GLenum opcode);
EAPI void evas_glLogicOp_evgl_api_th(GLenum opcode);

extern void (*orig_evgl_api_glMaterialx)(GLenum face, GLenum pname, GLfixed param);
EAPI void evas_glMaterialx_evgl_api_th(GLenum face, GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glMaterialxv)(GLenum face, GLenum pname, const GLfixed *params);
EAPI void evas_glMaterialxv_evgl_api_th(GLenum face, GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glMatrixMode)(GLenum mode);
EAPI void evas_glMatrixMode_evgl_api_th(GLenum mode);

extern void (*orig_evgl_api_glMultMatrixx)(const GLfixed *m);
EAPI void evas_glMultMatrixx_evgl_api_th(const GLfixed *m);

extern void (*orig_evgl_api_glMultiTexCoord4x)(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
EAPI void evas_glMultiTexCoord4x_evgl_api_th(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);

extern void (*orig_evgl_api_glNormal3x)(GLfixed nx, GLfixed ny, GLfixed nz);
EAPI void evas_glNormal3x_evgl_api_th(GLfixed nx, GLfixed ny, GLfixed nz);

extern void (*orig_evgl_api_glNormalPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
EAPI void evas_glNormalPointer_evgl_api_th(GLenum type, GLsizei stride, const GLvoid *pointer);

extern void (*orig_evgl_api_glOrthox)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
EAPI void evas_glOrthox_evgl_api_th(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);

extern void (*orig_evgl_api_glPointParameterx)(GLenum pname, GLfixed param);
EAPI void evas_glPointParameterx_evgl_api_th(GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glPointParameterxv)(GLenum pname, const GLfixed *params);
EAPI void evas_glPointParameterxv_evgl_api_th(GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glPointSizex)(GLfixed size);
EAPI void evas_glPointSizex_evgl_api_th(GLfixed size);

extern void (*orig_evgl_api_glPolygonOffsetx)(GLfixed factor, GLfixed units);
EAPI void evas_glPolygonOffsetx_evgl_api_th(GLfixed factor, GLfixed units);

extern void (*orig_evgl_api_glPopMatrix)(void);
EAPI void evas_glPopMatrix_evgl_api_th(void);

extern void (*orig_evgl_api_glPushMatrix)(void);
EAPI void evas_glPushMatrix_evgl_api_th(void);

extern void (*orig_evgl_api_glRotatex)(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
EAPI void evas_glRotatex_evgl_api_th(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);

extern void (*orig_evgl_api_glSampleCoveragex)(GLclampx value, GLboolean invert);
EAPI void evas_glSampleCoveragex_evgl_api_th(GLclampx value, GLboolean invert);

extern void (*orig_evgl_api_glScalex)(GLfixed x, GLfixed y, GLfixed z);
EAPI void evas_glScalex_evgl_api_th(GLfixed x, GLfixed y, GLfixed z);

extern void (*orig_evgl_api_glShadeModel)(GLenum mode);
EAPI void evas_glShadeModel_evgl_api_th(GLenum mode);

extern void (*orig_evgl_api_glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
EAPI void evas_glTexCoordPointer_evgl_api_th(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

extern void (*orig_evgl_api_glTexEnvi)(GLenum target, GLenum pname, GLint param);
EAPI void evas_glTexEnvi_evgl_api_th(GLenum target, GLenum pname, GLint param);

extern void (*orig_evgl_api_glTexEnvx)(GLenum target, GLenum pname, GLfixed param);
EAPI void evas_glTexEnvx_evgl_api_th(GLenum target, GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glTexEnviv)(GLenum target, GLenum pname, const GLint *params);
EAPI void evas_glTexEnviv_evgl_api_th(GLenum target, GLenum pname, const GLint *params);

extern void (*orig_evgl_api_glTexEnvxv)(GLenum target, GLenum pname, const GLfixed *params);
EAPI void evas_glTexEnvxv_evgl_api_th(GLenum target, GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glTexParameterx)(GLenum target, GLenum pname, GLfixed param);
EAPI void evas_glTexParameterx_evgl_api_th(GLenum target, GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glTexParameterxv)(GLenum target, GLenum pname, const GLfixed *params);
EAPI void evas_glTexParameterxv_evgl_api_th(GLenum target, GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glTranslatex)(GLfixed x, GLfixed y, GLfixed z);
EAPI void evas_glTranslatex_evgl_api_th(GLfixed x, GLfixed y, GLfixed z);

extern void (*orig_evgl_api_glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
EAPI void evas_glVertexPointer_evgl_api_th(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

extern void (*orig_evgl_api_glBlendEquationSeparateOES)(GLenum modeRGB, GLenum modeAlpha);
EAPI void evas_glBlendEquationSeparateOES_evgl_api_th(GLenum modeRGB, GLenum modeAlpha);

extern void (*orig_evgl_api_glBlendFuncSeparateOES)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
EAPI void evas_glBlendFuncSeparateOES_evgl_api_th(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

extern void (*orig_evgl_api_glBlendEquationOES)(GLenum mode);
EAPI void evas_glBlendEquationOES_evgl_api_th(GLenum mode);

extern void (*orig_evgl_api_glDrawTexsOES)(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
EAPI void evas_glDrawTexsOES_evgl_api_th(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);

extern void (*orig_evgl_api_glDrawTexiOES)(GLint x, GLint y, GLint z, GLint width, GLint height);
EAPI void evas_glDrawTexiOES_evgl_api_th(GLint x, GLint y, GLint z, GLint width, GLint height);

extern void (*orig_evgl_api_glDrawTexxOES)(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
EAPI void evas_glDrawTexxOES_evgl_api_th(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);

extern void (*orig_evgl_api_glDrawTexsvOES)(const GLshort *coords);
EAPI void evas_glDrawTexsvOES_evgl_api_th(const GLshort *coords);

extern void (*orig_evgl_api_glDrawTexivOES)(const GLint *coords);
EAPI void evas_glDrawTexivOES_evgl_api_th(const GLint *coords);

extern void (*orig_evgl_api_glDrawTexxvOES)(const GLfixed *coords);
EAPI void evas_glDrawTexxvOES_evgl_api_th(const GLfixed *coords);

extern void (*orig_evgl_api_glDrawTexfOES)(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
EAPI void evas_glDrawTexfOES_evgl_api_th(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);

extern void (*orig_evgl_api_glDrawTexfvOES)(const GLfloat *coords);
EAPI void evas_glDrawTexfvOES_evgl_api_th(const GLfloat *coords);

extern void (*orig_evgl_api_glAlphaFuncxOES)(GLenum func, GLclampx ref);
EAPI void evas_glAlphaFuncxOES_evgl_api_th(GLenum func, GLclampx ref);

extern void (*orig_evgl_api_glClearColorxOES)(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);
EAPI void evas_glClearColorxOES_evgl_api_th(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);

extern void (*orig_evgl_api_glClearDepthxOES)(GLclampx depth);
EAPI void evas_glClearDepthxOES_evgl_api_th(GLclampx depth);

extern void (*orig_evgl_api_glClipPlanexOES)(GLenum plane, const GLfixed *equation);
EAPI void evas_glClipPlanexOES_evgl_api_th(GLenum plane, const GLfixed *equation);

extern void (*orig_evgl_api_glColor4xOES)(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
EAPI void evas_glColor4xOES_evgl_api_th(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);

extern void (*orig_evgl_api_glDepthRangexOES)(GLclampx zNear, GLclampx zFar);
EAPI void evas_glDepthRangexOES_evgl_api_th(GLclampx zNear, GLclampx zFar);

extern void (*orig_evgl_api_glFogxOES)(GLenum pname, GLfixed param);
EAPI void evas_glFogxOES_evgl_api_th(GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glFogxvOES)(GLenum pname, const GLfixed *params);
EAPI void evas_glFogxvOES_evgl_api_th(GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glFrustumxOES)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
EAPI void evas_glFrustumxOES_evgl_api_th(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);

extern void (*orig_evgl_api_glGetClipPlanexOES)(GLenum pname, GLfixed eqn[4]);
EAPI void evas_glGetClipPlanexOES_evgl_api_th(GLenum pname, GLfixed eqn[4]);

extern void (*orig_evgl_api_glGetFixedvOES)(GLenum pname, GLfixed *params);
EAPI void evas_glGetFixedvOES_evgl_api_th(GLenum pname, GLfixed *params);

extern void (*orig_evgl_api_glGetLightxvOES)(GLenum light, GLenum pname, GLfixed *params);
EAPI void evas_glGetLightxvOES_evgl_api_th(GLenum light, GLenum pname, GLfixed *params);

extern void (*orig_evgl_api_glGetMaterialxvOES)(GLenum face, GLenum pname, GLfixed *params);
EAPI void evas_glGetMaterialxvOES_evgl_api_th(GLenum face, GLenum pname, GLfixed *params);

extern void (*orig_evgl_api_glGetTexEnvxvOES)(GLenum env, GLenum pname, GLfixed *params);
EAPI void evas_glGetTexEnvxvOES_evgl_api_th(GLenum env, GLenum pname, GLfixed *params);

extern void (*orig_evgl_api_glGetTexParameterxvOES)(GLenum target, GLenum pname, GLfixed *params);
EAPI void evas_glGetTexParameterxvOES_evgl_api_th(GLenum target, GLenum pname, GLfixed *params);

extern void (*orig_evgl_api_glLightModelxOES)(GLenum pname, GLfixed param);
EAPI void evas_glLightModelxOES_evgl_api_th(GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glLightModelxvOES)(GLenum pname, const GLfixed *params);
EAPI void evas_glLightModelxvOES_evgl_api_th(GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glLightxOES)(GLenum light, GLenum pname, GLfixed param);
EAPI void evas_glLightxOES_evgl_api_th(GLenum light, GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glLightxvOES)(GLenum light, GLenum pname, const GLfixed *params);
EAPI void evas_glLightxvOES_evgl_api_th(GLenum light, GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glLineWidthxOES)(GLfixed width);
EAPI void evas_glLineWidthxOES_evgl_api_th(GLfixed width);

extern void (*orig_evgl_api_glLoadMatrixxOES)(const GLfixed *m);
EAPI void evas_glLoadMatrixxOES_evgl_api_th(const GLfixed *m);

extern void (*orig_evgl_api_glMaterialxOES)(GLenum face, GLenum pname, GLfixed param);
EAPI void evas_glMaterialxOES_evgl_api_th(GLenum face, GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glMaterialxvOES)(GLenum face, GLenum pname, const GLfixed *params);
EAPI void evas_glMaterialxvOES_evgl_api_th(GLenum face, GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glMultMatrixxOES)(const GLfixed *m);
EAPI void evas_glMultMatrixxOES_evgl_api_th(const GLfixed *m);

extern void (*orig_evgl_api_glMultiTexCoord4xOES)(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
EAPI void evas_glMultiTexCoord4xOES_evgl_api_th(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);

extern void (*orig_evgl_api_glNormal3xOES)(GLfixed nx, GLfixed ny, GLfixed nz);
EAPI void evas_glNormal3xOES_evgl_api_th(GLfixed nx, GLfixed ny, GLfixed nz);

extern void (*orig_evgl_api_glOrthoxOES)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
EAPI void evas_glOrthoxOES_evgl_api_th(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);

extern void (*orig_evgl_api_glPointParameterxOES)(GLenum pname, GLfixed param);
EAPI void evas_glPointParameterxOES_evgl_api_th(GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glPointParameterxvOES)(GLenum pname, const GLfixed *params);
EAPI void evas_glPointParameterxvOES_evgl_api_th(GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glPointSizexOES)(GLfixed size);
EAPI void evas_glPointSizexOES_evgl_api_th(GLfixed size);

extern void (*orig_evgl_api_glPolygonOffsetxOES)(GLfixed factor, GLfixed units);
EAPI void evas_glPolygonOffsetxOES_evgl_api_th(GLfixed factor, GLfixed units);

extern void (*orig_evgl_api_glRotatexOES)(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
EAPI void evas_glRotatexOES_evgl_api_th(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);

extern void (*orig_evgl_api_glSampleCoveragexOES)(GLclampx value, GLboolean invert);
EAPI void evas_glSampleCoveragexOES_evgl_api_th(GLclampx value, GLboolean invert);

extern void (*orig_evgl_api_glScalexOES)(GLfixed x, GLfixed y, GLfixed z);
EAPI void evas_glScalexOES_evgl_api_th(GLfixed x, GLfixed y, GLfixed z);

extern void (*orig_evgl_api_glTexEnvxOES)(GLenum target, GLenum pname, GLfixed param);
EAPI void evas_glTexEnvxOES_evgl_api_th(GLenum target, GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glTexEnvxvOES)(GLenum target, GLenum pname, const GLfixed *params);
EAPI void evas_glTexEnvxvOES_evgl_api_th(GLenum target, GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glTexParameterxOES)(GLenum target, GLenum pname, GLfixed param);
EAPI void evas_glTexParameterxOES_evgl_api_th(GLenum target, GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glTexParameterxvOES)(GLenum target, GLenum pname, const GLfixed *params);
EAPI void evas_glTexParameterxvOES_evgl_api_th(GLenum target, GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glTranslatexOES)(GLfixed x, GLfixed y, GLfixed z);
EAPI void evas_glTranslatexOES_evgl_api_th(GLfixed x, GLfixed y, GLfixed z);

extern GLboolean (*orig_evgl_api_glIsRenderbufferOES)(GLuint renderbuffer);
EAPI GLboolean evas_glIsRenderbufferOES_evgl_api_th(GLuint renderbuffer);

extern void (*orig_evgl_api_glBindRenderbufferOES)(GLenum target, GLuint renderbuffer);
EAPI void evas_glBindRenderbufferOES_evgl_api_th(GLenum target, GLuint renderbuffer);

extern void (*orig_evgl_api_glDeleteRenderbuffersOES)(GLsizei n, const GLuint* renderbuffers);
EAPI void evas_glDeleteRenderbuffersOES_evgl_api_th(GLsizei n, const GLuint* renderbuffers);

extern void (*orig_evgl_api_glGenRenderbuffersOES)(GLsizei n, GLuint* renderbuffers);
EAPI void evas_glGenRenderbuffersOES_evgl_api_th(GLsizei n, GLuint* renderbuffers);

extern void (*orig_evgl_api_glRenderbufferStorageOES)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
EAPI void evas_glRenderbufferStorageOES_evgl_api_th(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);

extern void (*orig_evgl_api_glGetRenderbufferParameterivOES)(GLenum target, GLenum pname, GLint* params);
EAPI void evas_glGetRenderbufferParameterivOES_evgl_api_th(GLenum target, GLenum pname, GLint* params);

extern GLboolean (*orig_evgl_api_glIsFramebufferOES)(GLuint framebuffer);
EAPI GLboolean evas_glIsFramebufferOES_evgl_api_th(GLuint framebuffer);

extern void (*orig_evgl_api_glBindFramebufferOES)(GLenum target, GLuint framebuffer);
EAPI void evas_glBindFramebufferOES_evgl_api_th(GLenum target, GLuint framebuffer);

extern void (*orig_evgl_api_glDeleteFramebuffersOES)(GLsizei n, const GLuint* framebuffers);
EAPI void evas_glDeleteFramebuffersOES_evgl_api_th(GLsizei n, const GLuint* framebuffers);

extern void (*orig_evgl_api_glGenFramebuffersOES)(GLsizei n, GLuint* framebuffers);
EAPI void evas_glGenFramebuffersOES_evgl_api_th(GLsizei n, GLuint* framebuffers);

extern GLenum (*orig_evgl_api_glCheckFramebufferStatusOES)(GLenum target);
EAPI GLenum evas_glCheckFramebufferStatusOES_evgl_api_th(GLenum target);

extern void (*orig_evgl_api_glFramebufferRenderbufferOES)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
EAPI void evas_glFramebufferRenderbufferOES_evgl_api_th(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

extern void (*orig_evgl_api_glFramebufferTexture2DOES)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
EAPI void evas_glFramebufferTexture2DOES_evgl_api_th(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

extern void (*orig_evgl_api_glGetFramebufferAttachmentParameterivOES)(GLenum target, GLenum attachment, GLenum pname, GLint* params);
EAPI void evas_glGetFramebufferAttachmentParameterivOES_evgl_api_th(GLenum target, GLenum attachment, GLenum pname, GLint* params);

extern void (*orig_evgl_api_glGenerateMipmapOES)(GLenum target);
EAPI void evas_glGenerateMipmapOES_evgl_api_th(GLenum target);

extern void (*orig_evgl_api_glCurrentPaletteMatrixOES)(GLuint matrixpaletteindex);
EAPI void evas_glCurrentPaletteMatrixOES_evgl_api_th(GLuint matrixpaletteindex);

extern void (*orig_evgl_api_glLoadPaletteFromModelViewMatrixOES)(void);
EAPI void evas_glLoadPaletteFromModelViewMatrixOES_evgl_api_th(void);

extern void (*orig_evgl_api_glMatrixIndexPointerOES)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
EAPI void evas_glMatrixIndexPointerOES_evgl_api_th(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

extern void (*orig_evgl_api_glWeightPointerOES)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
EAPI void evas_glWeightPointerOES_evgl_api_th(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

extern GLbitfield (*orig_evgl_api_glQueryMatrixxOES)(GLfixed mantissa[16], GLint exponent[16]);
EAPI GLbitfield evas_glQueryMatrixxOES_evgl_api_th(GLfixed mantissa[16], GLint exponent[16]);

extern void (*orig_evgl_api_glDepthRangefOES)(GLclampf zNear, GLclampf zFar);
EAPI void evas_glDepthRangefOES_evgl_api_th(GLclampf zNear, GLclampf zFar);

extern void (*orig_evgl_api_glFrustumfOES)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
EAPI void evas_glFrustumfOES_evgl_api_th(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);

extern void (*orig_evgl_api_glOrthofOES)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
EAPI void evas_glOrthofOES_evgl_api_th(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);

extern void (*orig_evgl_api_glClipPlanefOES)(GLenum plane, const GLfloat *equation);
EAPI void evas_glClipPlanefOES_evgl_api_th(GLenum plane, const GLfloat *equation);

extern void (*orig_evgl_api_glGetClipPlanefOES)(GLenum pname, GLfloat eqn[4]);
EAPI void evas_glGetClipPlanefOES_evgl_api_th(GLenum pname, GLfloat eqn[4]);

extern void (*orig_evgl_api_glClearDepthfOES)(GLclampf depth);
EAPI void evas_glClearDepthfOES_evgl_api_th(GLclampf depth);

extern void (*orig_evgl_api_glTexGenfOES)(GLenum coord, GLenum pname, GLfloat param);
EAPI void evas_glTexGenfOES_evgl_api_th(GLenum coord, GLenum pname, GLfloat param);

extern void (*orig_evgl_api_glTexGenfvOES)(GLenum coord, GLenum pname, const GLfloat *params);
EAPI void evas_glTexGenfvOES_evgl_api_th(GLenum coord, GLenum pname, const GLfloat *params);

extern void (*orig_evgl_api_glTexGeniOES)(GLenum coord, GLenum pname, GLint param);
EAPI void evas_glTexGeniOES_evgl_api_th(GLenum coord, GLenum pname, GLint param);

extern void (*orig_evgl_api_glTexGenivOES)(GLenum coord, GLenum pname, const GLint *params);
EAPI void evas_glTexGenivOES_evgl_api_th(GLenum coord, GLenum pname, const GLint *params);

extern void (*orig_evgl_api_glTexGenxOES)(GLenum coord, GLenum pname, GLfixed param);
EAPI void evas_glTexGenxOES_evgl_api_th(GLenum coord, GLenum pname, GLfixed param);

extern void (*orig_evgl_api_glTexGenxvOES)(GLenum coord, GLenum pname, const GLfixed *params);
EAPI void evas_glTexGenxvOES_evgl_api_th(GLenum coord, GLenum pname, const GLfixed *params);

extern void (*orig_evgl_api_glGetTexGenfvOES)(GLenum coord, GLenum pname, GLfloat *params);
EAPI void evas_glGetTexGenfvOES_evgl_api_th(GLenum coord, GLenum pname, GLfloat *params);

extern void (*orig_evgl_api_glGetTexGenivOES)(GLenum coord, GLenum pname, GLint *params);
EAPI void evas_glGetTexGenivOES_evgl_api_th(GLenum coord, GLenum pname, GLint *params);

extern void (*orig_evgl_api_glGetTexGenxvOES)(GLenum coord, GLenum pname, GLfixed *params);
EAPI void evas_glGetTexGenxvOES_evgl_api_th(GLenum coord, GLenum pname, GLfixed *params);

extern void (*orig_evgl_api_glBindVertexArrayOES)(GLuint array);
EAPI void evas_glBindVertexArrayOES_evgl_api_th(GLuint array);

extern void (*orig_evgl_api_glDeleteVertexArraysOES)(GLsizei n, const GLuint *arrays);
EAPI void evas_glDeleteVertexArraysOES_evgl_api_th(GLsizei n, const GLuint *arrays);

extern void (*orig_evgl_api_glGenVertexArraysOES)(GLsizei n, GLuint *arrays);
EAPI void evas_glGenVertexArraysOES_evgl_api_th(GLsizei n, GLuint *arrays);

extern GLboolean (*orig_evgl_api_glIsVertexArrayOES)(GLuint array);
EAPI GLboolean evas_glIsVertexArrayOES_evgl_api_th(GLuint array);

extern void (*orig_evgl_api_glCopyTextureLevelsAPPLE)(GLuint destinationTexture, GLuint sourceTexture, GLint sourceBaseLevel, GLsizei sourceLevelCount);
EAPI void evas_glCopyTextureLevelsAPPLE_evgl_api_th(GLuint destinationTexture, GLuint sourceTexture, GLint sourceBaseLevel, GLsizei sourceLevelCount);

extern void (*orig_evgl_api_glRenderbufferStorageMultisampleAPPLE)(GLenum a, GLsizei b, GLenum c, GLsizei d, GLsizei e);
EAPI void evas_glRenderbufferStorageMultisampleAPPLE_evgl_api_th(GLenum a, GLsizei b, GLenum c, GLsizei d, GLsizei e);

extern void (*orig_evgl_api_glResolveMultisampleFramebufferAPPLE)(void);
EAPI void evas_glResolveMultisampleFramebufferAPPLE_evgl_api_th(void);

extern GLsync (*orig_evgl_api_glFenceSyncAPPLE)(GLenum condition, GLbitfield flags);
EAPI GLsync evas_glFenceSyncAPPLE_evgl_api_th(GLenum condition, GLbitfield flags);

extern GLboolean (*orig_evgl_api_glIsSyncAPPLE)(GLsync sync);
EAPI GLboolean evas_glIsSyncAPPLE_evgl_api_th(GLsync sync);

extern void (*orig_evgl_api_glDeleteSyncAPPLE)(GLsync sync);
EAPI void evas_glDeleteSyncAPPLE_evgl_api_th(GLsync sync);

extern GLenum (*orig_evgl_api_glClientWaitSyncAPPLE)(GLsync sync, GLbitfield flags, EvasGLuint64 timeout);
EAPI GLenum evas_glClientWaitSyncAPPLE_evgl_api_th(GLsync sync, GLbitfield flags, EvasGLuint64 timeout);

extern void (*orig_evgl_api_glWaitSyncAPPLE)(GLsync sync, GLbitfield flags, EvasGLuint64 timeout);
EAPI void evas_glWaitSyncAPPLE_evgl_api_th(GLsync sync, GLbitfield flags, EvasGLuint64 timeout);

extern void (*orig_evgl_api_glGetInteger64vAPPLE)(GLenum pname, EvasGLint64 *params);
EAPI void evas_glGetInteger64vAPPLE_evgl_api_th(GLenum pname, EvasGLint64 *params);

extern void (*orig_evgl_api_glGetSyncivAPPLE)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
EAPI void evas_glGetSyncivAPPLE_evgl_api_th(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);

extern void * (*orig_evgl_api_glMapBufferRangeEXT)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
EAPI void * evas_glMapBufferRangeEXT_evgl_api_th(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);

extern void (*orig_evgl_api_glFlushMappedBufferRangeEXT)(GLenum target, GLintptr offset, GLsizeiptr length);
EAPI void evas_glFlushMappedBufferRangeEXT_evgl_api_th(GLenum target, GLintptr offset, GLsizeiptr length);

extern void (*orig_evgl_api_glRenderbufferStorageMultisampleEXT)(GLenum a, GLsizei b, GLenum c, GLsizei d, GLsizei e);
EAPI void evas_glRenderbufferStorageMultisampleEXT_evgl_api_th(GLenum a, GLsizei b, GLenum c, GLsizei d, GLsizei e);

extern void (*orig_evgl_api_glFramebufferTexture2DMultisample)(GLenum a, GLenum b, GLenum c, GLuint d, GLint e, GLsizei f);
EAPI void evas_glFramebufferTexture2DMultisample_evgl_api_th(GLenum a, GLenum b, GLenum c, GLuint d, GLint e, GLsizei f);

extern void (*orig_evgl_api_glFramebufferTexture2DMultisampleEXT)(GLenum a, GLenum b, GLenum c, GLuint d, GLint e, GLsizei f);
EAPI void evas_glFramebufferTexture2DMultisampleEXT_evgl_api_th(GLenum a, GLenum b, GLenum c, GLuint d, GLint e, GLsizei f);

extern GLenum (*orig_evgl_api_glGetGraphicsResetStatus)(void);
EAPI GLenum evas_glGetGraphicsResetStatus_evgl_api_th(void);

extern GLenum (*orig_evgl_api_glGetGraphicsResetStatusEXT)(void);
EAPI GLenum evas_glGetGraphicsResetStatusEXT_evgl_api_th(void);

extern void (*orig_evgl_api_glReadnPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
EAPI void evas_glReadnPixels_evgl_api_th(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);

extern void (*orig_evgl_api_glReadnPixelsEXT)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
EAPI void evas_glReadnPixelsEXT_evgl_api_th(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);

extern void (*orig_evgl_api_glGetnUniformfv)(GLuint program, GLint location, GLsizei bufSize, float *params);
EAPI void evas_glGetnUniformfv_evgl_api_th(GLuint program, GLint location, GLsizei bufSize, float *params);

extern void (*orig_evgl_api_glGetnUniformfvEXT)(GLuint program, GLint location, GLsizei bufSize, float *params);
EAPI void evas_glGetnUniformfvEXT_evgl_api_th(GLuint program, GLint location, GLsizei bufSize, float *params);

extern void (*orig_evgl_api_glGetnUniformiv)(GLuint program, GLint location, GLsizei bufSize, GLint *params);
EAPI void evas_glGetnUniformiv_evgl_api_th(GLuint program, GLint location, GLsizei bufSize, GLint *params);

extern void (*orig_evgl_api_glGetnUniformivEXT)(GLuint program, GLint location, GLsizei bufSize, GLint *params);
EAPI void evas_glGetnUniformivEXT_evgl_api_th(GLuint program, GLint location, GLsizei bufSize, GLint *params);

extern void (*orig_evgl_api_glTexStorage1DEXT)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
EAPI void evas_glTexStorage1DEXT_evgl_api_th(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);

extern void (*orig_evgl_api_glTexStorage2DEXT)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
EAPI void evas_glTexStorage2DEXT_evgl_api_th(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);

extern void (*orig_evgl_api_glTexStorage3DEXT)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
EAPI void evas_glTexStorage3DEXT_evgl_api_th(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);

extern void (*orig_evgl_api_glTextureStorage1DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
EAPI void evas_glTextureStorage1DEXT_evgl_api_th(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);

extern void (*orig_evgl_api_glTextureStorage2DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
EAPI void evas_glTextureStorage2DEXT_evgl_api_th(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);

extern void (*orig_evgl_api_glTextureStorage3DEXT)(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
EAPI void evas_glTextureStorage3DEXT_evgl_api_th(GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);

extern void (*orig_evgl_api_glClipPlanefIMG)(GLenum a, const GLfloat * b);
EAPI void evas_glClipPlanefIMG_evgl_api_th(GLenum a, const GLfloat * b);

extern void (*orig_evgl_api_glClipPlanexIMG)(GLenum a, const GLfixed * b);
EAPI void evas_glClipPlanexIMG_evgl_api_th(GLenum a, const GLfixed * b);

extern void (*orig_evgl_api_glRenderbufferStorageMultisampleIMG)(GLenum a, GLsizei b, GLenum c, GLsizei d, GLsizei e);
EAPI void evas_glRenderbufferStorageMultisampleIMG_evgl_api_th(GLenum a, GLsizei b, GLenum c, GLsizei d, GLsizei e);

extern void (*orig_evgl_api_glFramebufferTexture2DMultisampleIMG)(GLenum a, GLenum b, GLenum c, GLuint d, GLint e, GLsizei f);
EAPI void evas_glFramebufferTexture2DMultisampleIMG_evgl_api_th(GLenum a, GLenum b, GLenum c, GLuint d, GLint e, GLsizei f);

extern void (*orig_evgl_api_glStartTilingQCOM)(GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask);
EAPI void evas_glStartTilingQCOM_evgl_api_th(GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask);

extern void (*orig_evgl_api_glEndTilingQCOM)(GLbitfield preserveMask);
EAPI void evas_glEndTilingQCOM_evgl_api_th(GLbitfield preserveMask);

extern void (*orig_evgl_api_glBeginQuery)(GLenum target, GLuint id);
EAPI void evas_glBeginQuery_evgl_api_th(GLenum target, GLuint id);

extern void (*orig_evgl_api_glBeginTransformFeedback)(GLenum primitiveMode);
EAPI void evas_glBeginTransformFeedback_evgl_api_th(GLenum primitiveMode);

extern void (*orig_evgl_api_glBindBufferBase)(GLenum target, GLuint index, GLuint buffer);
EAPI void evas_glBindBufferBase_evgl_api_th(GLenum target, GLuint index, GLuint buffer);

extern void (*orig_evgl_api_glBindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
EAPI void evas_glBindBufferRange_evgl_api_th(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);

extern void (*orig_evgl_api_glBindSampler)(GLuint unit, GLuint sampler);
EAPI void evas_glBindSampler_evgl_api_th(GLuint unit, GLuint sampler);

extern void (*orig_evgl_api_glBindTransformFeedback)(GLenum target, GLuint id);
EAPI void evas_glBindTransformFeedback_evgl_api_th(GLenum target, GLuint id);

extern void (*orig_evgl_api_glBindVertexArray)(GLuint array);
EAPI void evas_glBindVertexArray_evgl_api_th(GLuint array);

extern void (*orig_evgl_api_glBlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
EAPI void evas_glBlitFramebuffer_evgl_api_th(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);

extern void (*orig_evgl_api_glClearBufferfi)(GLenum buffer, GLint drawBuffer, GLfloat depth, GLint stencil);
EAPI void evas_glClearBufferfi_evgl_api_th(GLenum buffer, GLint drawBuffer, GLfloat depth, GLint stencil);

extern void (*orig_evgl_api_glClearBufferfv)(GLenum buffer, GLint drawBuffer, const GLfloat * value);
EAPI void evas_glClearBufferfv_evgl_api_th(GLenum buffer, GLint drawBuffer, const GLfloat * value);

extern void (*orig_evgl_api_glClearBufferiv)(GLenum buffer, GLint drawBuffer, const GLint * value);
EAPI void evas_glClearBufferiv_evgl_api_th(GLenum buffer, GLint drawBuffer, const GLint * value);

extern void (*orig_evgl_api_glClearBufferuiv)(GLenum buffer, GLint drawBuffer, const GLuint * value);
EAPI void evas_glClearBufferuiv_evgl_api_th(GLenum buffer, GLint drawBuffer, const GLuint * value);

extern GLenum (*orig_evgl_api_glClientWaitSync)(GLsync sync, GLbitfield flags, EvasGLuint64 timeout);
EAPI GLenum evas_glClientWaitSync_evgl_api_th(GLsync sync, GLbitfield flags, EvasGLuint64 timeout);

extern void (*orig_evgl_api_glCompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data);
EAPI void evas_glCompressedTexImage3D_evgl_api_th(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data);

extern void (*orig_evgl_api_glCompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data);
EAPI void evas_glCompressedTexSubImage3D_evgl_api_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data);

extern void (*orig_evgl_api_glCopyBufferSubData)(GLenum readtarget, GLenum writetarget, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size);
EAPI void evas_glCopyBufferSubData_evgl_api_th(GLenum readtarget, GLenum writetarget, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size);

extern void (*orig_evgl_api_glCopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
EAPI void evas_glCopyTexSubImage3D_evgl_api_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);

extern void (*orig_evgl_api_glDeleteQueries)(GLsizei n, const GLuint * ids);
EAPI void evas_glDeleteQueries_evgl_api_th(GLsizei n, const GLuint * ids);

extern void (*orig_evgl_api_glDeleteSamplers)(GLsizei n, const GLuint * samplers);
EAPI void evas_glDeleteSamplers_evgl_api_th(GLsizei n, const GLuint * samplers);

extern void (*orig_evgl_api_glDeleteSync)(GLsync sync);
EAPI void evas_glDeleteSync_evgl_api_th(GLsync sync);

extern void (*orig_evgl_api_glDeleteTransformFeedbacks)(GLsizei n, const GLuint *ids);
EAPI void evas_glDeleteTransformFeedbacks_evgl_api_th(GLsizei n, const GLuint *ids);

extern void (*orig_evgl_api_glDeleteVertexArrays)(GLsizei n, const GLuint *arrays);
EAPI void evas_glDeleteVertexArrays_evgl_api_th(GLsizei n, const GLuint *arrays);

extern void (*orig_evgl_api_glDrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei primcount);
EAPI void evas_glDrawArraysInstanced_evgl_api_th(GLenum mode, GLint first, GLsizei count, GLsizei primcount);

extern void (*orig_evgl_api_glDrawBuffers)(GLsizei n, const GLenum *bufs);
EAPI void evas_glDrawBuffers_evgl_api_th(GLsizei n, const GLenum *bufs);

extern void (*orig_evgl_api_glDrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei primcount);
EAPI void evas_glDrawElementsInstanced_evgl_api_th(GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei primcount);

extern void (*orig_evgl_api_glDrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices);
EAPI void evas_glDrawRangeElements_evgl_api_th(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices);

extern void (*orig_evgl_api_glEndQuery)(GLenum target);
EAPI void evas_glEndQuery_evgl_api_th(GLenum target);

extern void (*orig_evgl_api_glEndTransformFeedback)(void);
EAPI void evas_glEndTransformFeedback_evgl_api_th(void);

extern GLsync (*orig_evgl_api_glFenceSync)(GLenum condition, GLbitfield flags);
EAPI GLsync evas_glFenceSync_evgl_api_th(GLenum condition, GLbitfield flags);

extern GLsync (*orig_evgl_api_glFlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);
EAPI GLsync evas_glFlushMappedBufferRange_evgl_api_th(GLenum target, GLintptr offset, GLsizeiptr length);

extern void (*orig_evgl_api_glFramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
EAPI void evas_glFramebufferTextureLayer_evgl_api_th(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);

extern void (*orig_evgl_api_glGenQueries)(GLsizei n, GLuint * ids);
EAPI void evas_glGenQueries_evgl_api_th(GLsizei n, GLuint * ids);

extern void (*orig_evgl_api_glGenSamplers)(GLsizei n, GLuint *samplers);
EAPI void evas_glGenSamplers_evgl_api_th(GLsizei n, GLuint *samplers);

extern void (*orig_evgl_api_glGenTransformFeedbacks)(GLsizei n, GLuint *ids);
EAPI void evas_glGenTransformFeedbacks_evgl_api_th(GLsizei n, GLuint *ids);

extern void (*orig_evgl_api_glGenVertexArrays)(GLsizei n, GLuint *arrays);
EAPI void evas_glGenVertexArrays_evgl_api_th(GLsizei n, GLuint *arrays);

extern void (*orig_evgl_api_glGetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
EAPI void evas_glGetActiveUniformBlockiv_evgl_api_th(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);

extern void (*orig_evgl_api_glGetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
EAPI void evas_glGetActiveUniformBlockName_evgl_api_th(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);

extern void (*orig_evgl_api_glGetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
EAPI void evas_glGetActiveUniformsiv_evgl_api_th(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);

extern void (*orig_evgl_api_glGetBufferParameteri64v)(GLenum target, GLenum value, EvasGLint64 * data);
EAPI void evas_glGetBufferParameteri64v_evgl_api_th(GLenum target, GLenum value, EvasGLint64 * data);

extern void (*orig_evgl_api_glGetBufferPointerv)(GLenum target, GLenum pname, GLvoid ** params);
EAPI void evas_glGetBufferPointerv_evgl_api_th(GLenum target, GLenum pname, GLvoid ** params);

extern GLint (*orig_evgl_api_glGetFragDataLocation)(GLuint program, const char * name);
EAPI GLint evas_glGetFragDataLocation_evgl_api_th(GLuint program, const char * name);

extern void (*orig_evgl_api_glGetInteger64i_v)(GLenum target, GLuint index, EvasGLint64 * data);
EAPI void evas_glGetInteger64i_v_evgl_api_th(GLenum target, GLuint index, EvasGLint64 * data);

extern void (*orig_evgl_api_glGetInteger64v)(GLenum pname, EvasGLint64 * data);
EAPI void evas_glGetInteger64v_evgl_api_th(GLenum pname, EvasGLint64 * data);

extern void (*orig_evgl_api_glGetIntegeri_v)(GLenum target, GLuint index, GLint * data);
EAPI void evas_glGetIntegeri_v_evgl_api_th(GLenum target, GLuint index, GLint * data);

extern void (*orig_evgl_api_glGetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
EAPI void evas_glGetInternalformativ_evgl_api_th(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);

extern void (*orig_evgl_api_glGetProgramBinary)(GLuint program, GLsizei bufsize, GLsizei *length, GLenum *binaryFormat, void *binary);
EAPI void evas_glGetProgramBinary_evgl_api_th(GLuint program, GLsizei bufsize, GLsizei *length, GLenum *binaryFormat, void *binary);

extern void (*orig_evgl_api_glGetQueryiv)(GLenum target, GLenum pname, GLint * params);
EAPI void evas_glGetQueryiv_evgl_api_th(GLenum target, GLenum pname, GLint * params);

extern void (*orig_evgl_api_glGetQueryObjectuiv)(GLuint id, GLenum pname, GLuint * params);
EAPI void evas_glGetQueryObjectuiv_evgl_api_th(GLuint id, GLenum pname, GLuint * params);

extern void (*orig_evgl_api_glGetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat * params);
EAPI void evas_glGetSamplerParameterfv_evgl_api_th(GLuint sampler, GLenum pname, GLfloat * params);

extern void (*orig_evgl_api_glGetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint * params);
EAPI void evas_glGetSamplerParameteriv_evgl_api_th(GLuint sampler, GLenum pname, GLint * params);

extern const GLubyte * (*orig_evgl_api_glGetStringi)(GLenum name, GLuint index);
EAPI const GLubyte * evas_glGetStringi_evgl_api_th(GLenum name, GLuint index);

extern void (*orig_evgl_api_glGetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
EAPI void evas_glGetSynciv_evgl_api_th(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);

extern void (*orig_evgl_api_glGetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, char * name);
EAPI void evas_glGetTransformFeedbackVarying_evgl_api_th(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, char * name);

extern GLuint (*orig_evgl_api_glGetUniformBlockIndex)(GLuint program, const GLchar *uniformBlockName);
EAPI GLuint evas_glGetUniformBlockIndex_evgl_api_th(GLuint program, const GLchar *uniformBlockName);

extern void (*orig_evgl_api_glGetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);
EAPI void evas_glGetUniformIndices_evgl_api_th(GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices);

extern void (*orig_evgl_api_glGetUniformuiv)(GLuint program, GLint location, GLuint* params);
EAPI void evas_glGetUniformuiv_evgl_api_th(GLuint program, GLint location, GLuint* params);

extern void (*orig_evgl_api_glGetVertexAttribIiv)(GLuint index, GLenum pname, GLint *params);
EAPI void evas_glGetVertexAttribIiv_evgl_api_th(GLuint index, GLenum pname, GLint *params);

extern void (*orig_evgl_api_glGetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint *params);
EAPI void evas_glGetVertexAttribIuiv_evgl_api_th(GLuint index, GLenum pname, GLuint *params);

extern void (*orig_evgl_api_glInvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments);
EAPI void evas_glInvalidateFramebuffer_evgl_api_th(GLenum target, GLsizei numAttachments, const GLenum *attachments);

extern void (*orig_evgl_api_glInvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
EAPI void evas_glInvalidateSubFramebuffer_evgl_api_th(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);

extern GLboolean (*orig_evgl_api_glIsQuery)(GLuint id);
EAPI GLboolean evas_glIsQuery_evgl_api_th(GLuint id);

extern GLboolean (*orig_evgl_api_glIsSampler)(GLuint id);
EAPI GLboolean evas_glIsSampler_evgl_api_th(GLuint id);

extern GLboolean (*orig_evgl_api_glIsSync)(GLsync sync);
EAPI GLboolean evas_glIsSync_evgl_api_th(GLsync sync);

extern GLboolean (*orig_evgl_api_glIsTransformFeedback)(GLuint id);
EAPI GLboolean evas_glIsTransformFeedback_evgl_api_th(GLuint id);

extern GLboolean (*orig_evgl_api_glIsVertexArray)(GLuint array);
EAPI GLboolean evas_glIsVertexArray_evgl_api_th(GLuint array);

extern void * (*orig_evgl_api_glMapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
EAPI void * evas_glMapBufferRange_evgl_api_th(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);

extern void (*orig_evgl_api_glPauseTransformFeedback)(void);
EAPI void evas_glPauseTransformFeedback_evgl_api_th(void);

extern void (*orig_evgl_api_glProgramBinary)(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
EAPI void evas_glProgramBinary_evgl_api_th(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);

extern void (*orig_evgl_api_glProgramParameteri)(GLuint program, GLenum pname, GLint value);
EAPI void evas_glProgramParameteri_evgl_api_th(GLuint program, GLenum pname, GLint value);

extern void (*orig_evgl_api_glReadBuffer)(GLenum src);
EAPI void evas_glReadBuffer_evgl_api_th(GLenum src);

extern void (*orig_evgl_api_glRenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
EAPI void evas_glRenderbufferStorageMultisample_evgl_api_th(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);

extern void (*orig_evgl_api_glResumeTransformFeedback)(void);
EAPI void evas_glResumeTransformFeedback_evgl_api_th(void);

extern void (*orig_evgl_api_glSamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param);
EAPI void evas_glSamplerParameterf_evgl_api_th(GLuint sampler, GLenum pname, GLfloat param);

extern void (*orig_evgl_api_glSamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat * params);
EAPI void evas_glSamplerParameterfv_evgl_api_th(GLuint sampler, GLenum pname, const GLfloat * params);

extern void (*orig_evgl_api_glSamplerParameteri)(GLuint sampler, GLenum pname, GLint param);
EAPI void evas_glSamplerParameteri_evgl_api_th(GLuint sampler, GLenum pname, GLint param);

extern void (*orig_evgl_api_glSamplerParameteriv)(GLuint sampler, GLenum pname, const GLint * params);
EAPI void evas_glSamplerParameteriv_evgl_api_th(GLuint sampler, GLenum pname, const GLint * params);

extern void (*orig_evgl_api_glTexImage3D)(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * data);
EAPI void evas_glTexImage3D_evgl_api_th(GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * data);

extern void (*orig_evgl_api_glTexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
EAPI void evas_glTexStorage2D_evgl_api_th(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);

extern void (*orig_evgl_api_glTexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
EAPI void evas_glTexStorage3D_evgl_api_th(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);

extern void (*orig_evgl_api_glTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * data);
EAPI void evas_glTexSubImage3D_evgl_api_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * data);

extern void (*orig_evgl_api_glTransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar *const* varyings, GLenum bufferMode);
EAPI void evas_glTransformFeedbackVaryings_evgl_api_th(GLuint program, GLsizei count, const GLchar *const* varyings, GLenum bufferMode);

extern void (*orig_evgl_api_glUniform1ui)(GLint location, GLuint v0);
EAPI void evas_glUniform1ui_evgl_api_th(GLint location, GLuint v0);

extern void (*orig_evgl_api_glUniform1uiv)(GLint location, GLsizei count, const GLuint *value);
EAPI void evas_glUniform1uiv_evgl_api_th(GLint location, GLsizei count, const GLuint *value);

extern void (*orig_evgl_api_glUniform2ui)(GLint location, GLuint v0, GLuint v1);
EAPI void evas_glUniform2ui_evgl_api_th(GLint location, GLuint v0, GLuint v1);

extern void (*orig_evgl_api_glUniform2uiv)(GLint location, GLsizei count, const GLuint *value);
EAPI void evas_glUniform2uiv_evgl_api_th(GLint location, GLsizei count, const GLuint *value);

extern void (*orig_evgl_api_glUniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2);
EAPI void evas_glUniform3ui_evgl_api_th(GLint location, GLuint v0, GLuint v1, GLuint v2);

extern void (*orig_evgl_api_glUniform3uiv)(GLint location, GLsizei count, const GLuint *value);
EAPI void evas_glUniform3uiv_evgl_api_th(GLint location, GLsizei count, const GLuint *value);

extern void (*orig_evgl_api_glUniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
EAPI void evas_glUniform4ui_evgl_api_th(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

extern void (*orig_evgl_api_glUniform4uiv)(GLint location, GLsizei count, const GLuint *value);
EAPI void evas_glUniform4uiv_evgl_api_th(GLint location, GLsizei count, const GLuint *value);

extern void (*orig_evgl_api_glUniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
EAPI void evas_glUniformBlockBinding_evgl_api_th(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);

extern void (*orig_evgl_api_glUniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glUniformMatrix2x3fv_evgl_api_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glUniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glUniformMatrix3x2fv_evgl_api_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glUniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glUniformMatrix2x4fv_evgl_api_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glUniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glUniformMatrix4x2fv_evgl_api_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glUniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glUniformMatrix3x4fv_evgl_api_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glUniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glUniformMatrix4x3fv_evgl_api_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern GLboolean (*orig_evgl_api_glUnmapBuffer)(GLenum target);
EAPI GLboolean evas_glUnmapBuffer_evgl_api_th(GLenum target);

extern void (*orig_evgl_api_glVertexAttribDivisor)(GLuint index, GLuint divisor);
EAPI void evas_glVertexAttribDivisor_evgl_api_th(GLuint index, GLuint divisor);

extern void (*orig_evgl_api_glVertexAttribI4i)(GLuint index, GLint v0, GLint v1, GLint v2, GLint v3);
EAPI void evas_glVertexAttribI4i_evgl_api_th(GLuint index, GLint v0, GLint v1, GLint v2, GLint v3);

extern void (*orig_evgl_api_glVertexAttribI4iv)(GLuint index, const GLint *v);
EAPI void evas_glVertexAttribI4iv_evgl_api_th(GLuint index, const GLint *v);

extern void (*orig_evgl_api_glVertexAttribI4ui)(GLuint index, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
EAPI void evas_glVertexAttribI4ui_evgl_api_th(GLuint index, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

extern void (*orig_evgl_api_glVertexAttribI4uiv)(GLuint index, const GLuint *v);
EAPI void evas_glVertexAttribI4uiv_evgl_api_th(GLuint index, const GLuint *v);

extern void (*orig_evgl_api_glVertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
EAPI void evas_glVertexAttribIPointer_evgl_api_th(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

extern void (*orig_evgl_api_glWaitSync)(GLsync sync, GLbitfield flags, EvasGLuint64 timeout);
EAPI void evas_glWaitSync_evgl_api_th(GLsync sync, GLbitfield flags, EvasGLuint64 timeout);

extern void (*orig_evgl_api_glDispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
EAPI void evas_glDispatchCompute_evgl_api_th(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);

extern void (*orig_evgl_api_glDispatchComputeIndirect)(GLintptr indirect);
EAPI void evas_glDispatchComputeIndirect_evgl_api_th(GLintptr indirect);

extern void (*orig_evgl_api_glDrawArraysIndirect)(GLenum mode, const void *indirect);
EAPI void evas_glDrawArraysIndirect_evgl_api_th(GLenum mode, const void *indirect);

extern void (*orig_evgl_api_glDrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect);
EAPI void evas_glDrawElementsIndirect_evgl_api_th(GLenum mode, GLenum type, const void *indirect);

extern void (*orig_evgl_api_glFramebufferParameteri)(GLenum target, GLenum pname, GLint param);
EAPI void evas_glFramebufferParameteri_evgl_api_th(GLenum target, GLenum pname, GLint param);

extern void (*orig_evgl_api_glGetFramebufferParameteriv)(GLenum target, GLenum pname, GLint *params);
EAPI void evas_glGetFramebufferParameteriv_evgl_api_th(GLenum target, GLenum pname, GLint *params);

extern void (*orig_evgl_api_glGetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint *params);
EAPI void evas_glGetProgramInterfaceiv_evgl_api_th(GLuint program, GLenum programInterface, GLenum pname, GLint *params);

extern GLuint (*orig_evgl_api_glGetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar *name);
EAPI GLuint evas_glGetProgramResourceIndex_evgl_api_th(GLuint program, GLenum programInterface, const GLchar *name);

extern void (*orig_evgl_api_glGetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
EAPI void evas_glGetProgramResourceName_evgl_api_th(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);

extern void (*orig_evgl_api_glGetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
EAPI void evas_glGetProgramResourceiv_evgl_api_th(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);

extern GLint (*orig_evgl_api_glGetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar *name);
EAPI GLint evas_glGetProgramResourceLocation_evgl_api_th(GLuint program, GLenum programInterface, const GLchar *name);

extern void (*orig_evgl_api_glUseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program);
EAPI void evas_glUseProgramStages_evgl_api_th(GLuint pipeline, GLbitfield stages, GLuint program);

extern void (*orig_evgl_api_glActiveShaderProgram)(GLuint pipeline, GLuint program);
EAPI void evas_glActiveShaderProgram_evgl_api_th(GLuint pipeline, GLuint program);

extern GLuint (*orig_evgl_api_glCreateShaderProgramv)(GLenum type, GLsizei count, const GLchar *const*strings);
EAPI GLuint evas_glCreateShaderProgramv_evgl_api_th(GLenum type, GLsizei count, const GLchar *const*strings);

extern void (*orig_evgl_api_glBindProgramPipeline)(GLuint pipeline);
EAPI void evas_glBindProgramPipeline_evgl_api_th(GLuint pipeline);

extern void (*orig_evgl_api_glDeleteProgramPipelines)(GLsizei n, const GLuint *pipelines);
EAPI void evas_glDeleteProgramPipelines_evgl_api_th(GLsizei n, const GLuint *pipelines);

extern void (*orig_evgl_api_glGenProgramPipelines)(GLsizei n, GLuint *pipelines);
EAPI void evas_glGenProgramPipelines_evgl_api_th(GLsizei n, GLuint *pipelines);

extern GLboolean (*orig_evgl_api_glIsProgramPipeline)(GLuint pipeline);
EAPI GLboolean evas_glIsProgramPipeline_evgl_api_th(GLuint pipeline);

extern void (*orig_evgl_api_glGetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint *params);
EAPI void evas_glGetProgramPipelineiv_evgl_api_th(GLuint pipeline, GLenum pname, GLint *params);

extern void (*orig_evgl_api_glProgramUniform1i)(GLuint program, GLint location, GLint v0);
EAPI void evas_glProgramUniform1i_evgl_api_th(GLuint program, GLint location, GLint v0);

extern void (*orig_evgl_api_glProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1);
EAPI void evas_glProgramUniform2i_evgl_api_th(GLuint program, GLint location, GLint v0, GLint v1);

extern void (*orig_evgl_api_glProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
EAPI void evas_glProgramUniform3i_evgl_api_th(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);

extern void (*orig_evgl_api_glProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
EAPI void evas_glProgramUniform4i_evgl_api_th(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);

extern void (*orig_evgl_api_glProgramUniform1ui)(GLuint program, GLint location, GLuint v0);
EAPI void evas_glProgramUniform1ui_evgl_api_th(GLuint program, GLint location, GLuint v0);

extern void (*orig_evgl_api_glProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1);
EAPI void evas_glProgramUniform2ui_evgl_api_th(GLuint program, GLint location, GLuint v0, GLuint v1);

extern void (*orig_evgl_api_glProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
EAPI void evas_glProgramUniform3ui_evgl_api_th(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);

extern void (*orig_evgl_api_glProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
EAPI void evas_glProgramUniform4ui_evgl_api_th(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);

extern void (*orig_evgl_api_glProgramUniform1f)(GLuint program, GLint location, GLfloat v0);
EAPI void evas_glProgramUniform1f_evgl_api_th(GLuint program, GLint location, GLfloat v0);

extern void (*orig_evgl_api_glProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
EAPI void evas_glProgramUniform2f_evgl_api_th(GLuint program, GLint location, GLfloat v0, GLfloat v1);

extern void (*orig_evgl_api_glProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
EAPI void evas_glProgramUniform3f_evgl_api_th(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);

extern void (*orig_evgl_api_glProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
EAPI void evas_glProgramUniform4f_evgl_api_th(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

extern void (*orig_evgl_api_glProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
EAPI void evas_glProgramUniform1iv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLint *value);

extern void (*orig_evgl_api_glProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
EAPI void evas_glProgramUniform2iv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLint *value);

extern void (*orig_evgl_api_glProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
EAPI void evas_glProgramUniform3iv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLint *value);

extern void (*orig_evgl_api_glProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
EAPI void evas_glProgramUniform4iv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLint *value);

extern void (*orig_evgl_api_glProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
EAPI void evas_glProgramUniform1uiv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLuint *value);

extern void (*orig_evgl_api_glProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
EAPI void evas_glProgramUniform2uiv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLuint *value);

extern void (*orig_evgl_api_glProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
EAPI void evas_glProgramUniform3uiv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLuint *value);

extern void (*orig_evgl_api_glProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
EAPI void evas_glProgramUniform4uiv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLuint *value);

extern void (*orig_evgl_api_glProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glProgramUniform1fv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glProgramUniform2fv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glProgramUniform3fv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glProgramUniform4fv_evgl_api_th(GLuint program, GLint location, GLsizei count, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glProgramUniformMatrix2fv_evgl_api_th(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glProgramUniformMatrix3fv_evgl_api_th(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glProgramUniformMatrix4fv_evgl_api_th(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glProgramUniformMatrix2x3fv_evgl_api_th(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glProgramUniformMatrix3x2fv_evgl_api_th(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glProgramUniformMatrix2x4fv_evgl_api_th(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glProgramUniformMatrix4x2fv_evgl_api_th(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glProgramUniformMatrix3x4fv_evgl_api_th(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glProgramUniformMatrix4x3fv_evgl_api_th(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern void (*orig_evgl_api_glValidateProgramPipeline)(GLuint pipeline);
EAPI void evas_glValidateProgramPipeline_evgl_api_th(GLuint pipeline);

extern void (*orig_evgl_api_glGetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
EAPI void evas_glGetProgramPipelineInfoLog_evgl_api_th(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

extern void (*orig_evgl_api_glBindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
EAPI void evas_glBindImageTexture_evgl_api_th(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);

extern void (*orig_evgl_api_glGetBooleani_v)(GLenum target, GLuint index, GLboolean *data);
EAPI void evas_glGetBooleani_v_evgl_api_th(GLenum target, GLuint index, GLboolean *data);

extern void (*orig_evgl_api_glMemoryBarrier)(GLbitfield barriers);
EAPI void evas_glMemoryBarrier_evgl_api_th(GLbitfield barriers);

extern void (*orig_evgl_api_glMemoryBarrierByRegion)(GLbitfield barriers);
EAPI void evas_glMemoryBarrierByRegion_evgl_api_th(GLbitfield barriers);

extern void (*orig_evgl_api_glTexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
EAPI void evas_glTexStorage2DMultisample_evgl_api_th(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);

extern void (*orig_evgl_api_glGetMultisamplefv)(GLenum pname, GLuint index, GLfloat *val);
EAPI void evas_glGetMultisamplefv_evgl_api_th(GLenum pname, GLuint index, GLfloat *val);

extern void (*orig_evgl_api_glSampleMaski)(GLuint maskNumber, GLbitfield mask);
EAPI void evas_glSampleMaski_evgl_api_th(GLuint maskNumber, GLbitfield mask);

extern void (*orig_evgl_api_glGetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params);
EAPI void evas_glGetTexLevelParameteriv_evgl_api_th(GLenum target, GLint level, GLenum pname, GLint *params);

extern void (*orig_evgl_api_glGetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params);
EAPI void evas_glGetTexLevelParameterfv_evgl_api_th(GLenum target, GLint level, GLenum pname, GLfloat *params);

extern void (*orig_evgl_api_glBindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
EAPI void evas_glBindVertexBuffer_evgl_api_th(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);

extern void (*orig_evgl_api_glVertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
EAPI void evas_glVertexAttribFormat_evgl_api_th(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);

extern void (*orig_evgl_api_glVertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
EAPI void evas_glVertexAttribIFormat_evgl_api_th(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);

extern void (*orig_evgl_api_glVertexAttribBinding)(GLuint attribindex, GLuint bindingindex);
EAPI void evas_glVertexAttribBinding_evgl_api_th(GLuint attribindex, GLuint bindingindex);

extern void (*orig_evgl_api_glVertexBindingDivisor)(GLuint bindingindex, GLuint divisor);
EAPI void evas_glVertexBindingDivisor_evgl_api_th(GLuint bindingindex, GLuint divisor);

extern void (*orig_evgl_api_glEGLImageTargetTexture2DOES)(GLenum target, void *image);
EAPI void evas_glEGLImageTargetTexture2DOES_evgl_api_th(GLenum target, void *image);

extern void (*orig_evgl_api_glEGLImageTargetRenderbufferStorageOES)(GLenum target, void *image);
EAPI void evas_glEGLImageTargetRenderbufferStorageOES_evgl_api_th(GLenum target, void *image);

extern void (*orig_evgl_api__evgl_glDiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum* attachments);
EAPI void evas__evgl_glDiscardFramebufferEXT_evgl_api_th(GLenum target, GLsizei numAttachments, const GLenum* attachments);

extern void (*orig_evgl_api__evgl_glEvasGLImageTargetTexture2D)(GLenum target, EvasGLImage image);
EAPI void evas__evgl_glEvasGLImageTargetTexture2D_evgl_api_th(GLenum target, EvasGLImage image);

extern void (*orig_evgl_api__evgl_glEvasGLImageTargetRenderbufferStorage)(GLenum target, EvasGLImage image);
EAPI void evas__evgl_glEvasGLImageTargetRenderbufferStorage_evgl_api_th(GLenum target, EvasGLImage image);

extern EvasGLImage (*orig_evgl_api__evgl_evasglCreateImage)(int target, void* buffer, const int *attrib_list);
EAPI EvasGLImage evas__evgl_evasglCreateImage_evgl_api_th(int target, void* buffer, const int *attrib_list);

extern void (*orig_evgl_api__evgl_evasglDestroyImage)(EvasGLImage image);
EAPI void evas__evgl_evasglDestroyImage_evgl_api_th(EvasGLImage image);

extern EvasGLImage (*orig_evgl_api__evgl_evasglCreateImageForContext)(Evas_GL *evas_gl, Evas_GL_Context *ctx, int target, void* buffer, const int *attrib_list);
EAPI EvasGLImage evas__evgl_evasglCreateImageForContext_evgl_api_th(Evas_GL *evas_gl, Evas_GL_Context *ctx, int target, void* buffer, const int *attrib_list);

extern EvasGLSync (*orig_evgl_api__evgl_evasglCreateSync)(Evas_GL *evas_gl, unsigned int type, const int *attrib_list);
EAPI EvasGLSync evas__evgl_evasglCreateSync_evgl_api_th(Evas_GL *evas_gl, unsigned int type, const int *attrib_list);

extern Eina_Bool (*orig_evgl_api__evgl_evasglDestroySync)(Evas_GL *evas_gl, EvasGLSync sync);
EAPI Eina_Bool evas__evgl_evasglDestroySync_evgl_api_th(Evas_GL *evas_gl, EvasGLSync sync);

extern int (*orig_evgl_api__evgl_evasglClientWaitSync)(Evas_GL *evas_gl, EvasGLSync sync);
EAPI int evas__evgl_evasglClientWaitSync_evgl_api_th(Evas_GL *evas_gl, EvasGLSync sync);

extern Eina_Bool (*orig_evgl_api__evgl_evasglGetSyncAttrib)(Evas_GL *evas_gl, EvasGLSync sync, int attribute, int *value);
EAPI Eina_Bool evas__evgl_evasglGetSyncAttrib_evgl_api_th(Evas_GL *evas_gl, EvasGLSync sync, int attribute, int *value);

extern Eina_Bool (*orig_evgl_api__evgl_evasglSignalSync)(Evas_GL *evas_gl, EvasGLSync sync, unsigned mode);
EAPI Eina_Bool evas__evgl_evasglSignalSync_evgl_api_th(Evas_GL *evas_gl, EvasGLSync sync, unsigned mode);

extern int (*orig_evgl_api__evgl_evasglWaitSync)(Evas_GL *evas_gl, EvasGLSync sync, int flags);
EAPI int evas__evgl_evasglWaitSync_evgl_api_th(Evas_GL *evas_gl, EvasGLSync sync, int flags);

extern Eina_Bool (*orig_evgl_api__evgl_evasglBindWaylandDisplay)(Evas_GL *evas_gl, void *wl_display);
EAPI Eina_Bool evas__evgl_evasglBindWaylandDisplay_evgl_api_th(Evas_GL *evas_gl, void *wl_display);

extern Eina_Bool (*orig_evgl_api__evgl_evasglUnbindWaylandDisplay)(Evas_GL *evas_gl, void *wl_display);
EAPI Eina_Bool evas__evgl_evasglUnbindWaylandDisplay_evgl_api_th(Evas_GL *evas_gl, void *wl_display);

extern Eina_Bool (*orig_evgl_api__evgl_evasglQueryWaylandBuffer)(Evas_GL *evas_gl, void *buffer, int attribute, int *value);
EAPI Eina_Bool evas__evgl_evasglQueryWaylandBuffer_evgl_api_th(Evas_GL *evas_gl, void *buffer, int attribute, int *value);
