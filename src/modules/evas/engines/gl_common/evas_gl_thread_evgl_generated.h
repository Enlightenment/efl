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

EAPI GLenum evas_glGetError_evgl_th(void);
EAPI void evas_glVertexAttribPointer_evgl_th(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
EAPI void evas_glEnableVertexAttribArray_evgl_th(GLuint index);
EAPI void evas_glDisableVertexAttribArray_evgl_th(GLuint index);
EAPI void evas_glDrawArrays_evgl_th(GLenum mode, GLint first, GLsizei count);
EAPI void evas_glDrawElements_evgl_th(GLenum mode, GLsizei count, GLenum type, const void *indices);
EAPI void evas_glGenBuffers_evgl_th(GLsizei n, GLuint *buffers);
EAPI void evas_glDeleteBuffers_evgl_th(GLsizei n, const GLuint *buffers);
EAPI void evas_glBindBuffer_evgl_th(GLenum target, GLuint buffer);
EAPI void evas_glBufferData_evgl_th(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
EAPI GLuint evas_glCreateShader_evgl_th(GLenum type);
EAPI void evas_glShaderSource_evgl_th(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
EAPI void evas_glCompileShader_evgl_th(GLuint shader);
EAPI void evas_glDeleteShader_evgl_th(GLuint shader);
EAPI GLuint evas_glCreateProgram_evgl_th(void);
EAPI void evas_glAttachShader_evgl_th(GLuint program, GLuint shader);
EAPI void evas_glDetachShader_evgl_th(GLuint program, GLuint shader);
EAPI void evas_glLinkProgram_evgl_th(GLuint program);
EAPI void evas_glUseProgram_evgl_th(GLuint program);
EAPI void evas_glDeleteProgram_evgl_th(GLuint program);

EAPI void glGetProgramBinary_orig_evgl_set(void *func);
EAPI void *glGetProgramBinary_orig_evgl_get(void);
EAPI void evas_glGetProgramBinary_evgl_th(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);

EAPI void glProgramBinary_orig_evgl_set(void *func);
EAPI void *glProgramBinary_orig_evgl_get(void);
EAPI void evas_glProgramBinary_evgl_th(GLuint program, GLenum binaryFormat, const void *binary, GLint length);
EAPI void evas_glGetActiveAttrib_evgl_th(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
EAPI GLint evas_glGetAttribLocation_evgl_th(GLuint program, const GLchar *name);
EAPI void evas_glBindAttribLocation_evgl_th(GLuint program, GLuint index, const GLchar *name);
EAPI GLint evas_glGetUniformLocation_evgl_th(GLuint program, const GLchar *name);
EAPI void evas_glUniform1f_evgl_th(GLint location, GLfloat v0);
EAPI void evas_glUniform1i_evgl_th(GLint location, GLint v0);
EAPI void evas_glUniform2f_evgl_th(GLint location, GLfloat v0, GLfloat v1);
EAPI void evas_glUniform2i_evgl_th(GLint location, GLint v0, GLint v1);
EAPI void evas_glUniform3f_evgl_th(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
EAPI void evas_glUniform3i_evgl_th(GLint location, GLint v0, GLint v1, GLint v2);
EAPI void evas_glUniform4f_evgl_th(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
EAPI void evas_glUniform4i_evgl_th(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
EAPI void evas_glUniform1fv_evgl_th(GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glUniform1iv_evgl_th(GLint location, GLsizei count, const GLint *value);
EAPI void evas_glUniform2fv_evgl_th(GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glUniform2iv_evgl_th(GLint location, GLsizei count, const GLint *value);
EAPI void evas_glUniform3fv_evgl_th(GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glUniform3iv_evgl_th(GLint location, GLsizei count, const GLint *value);
EAPI void evas_glUniform4fv_evgl_th(GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glUniform4iv_evgl_th(GLint location, GLsizei count, const GLint *value);
EAPI void evas_glUniformMatrix2fv_evgl_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glUniformMatrix3fv_evgl_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glUniformMatrix4fv_evgl_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glViewport_evgl_th(GLint x, GLint y, GLsizei width, GLsizei height);
EAPI void evas_glEnable_evgl_th(GLenum cap);
EAPI void evas_glDisable_evgl_th(GLenum cap);
EAPI void evas_glLineWidth_evgl_th(GLfloat width);
EAPI void evas_glPolygonOffset_evgl_th(GLfloat factor, GLfloat units);
EAPI void evas_glPixelStorei_evgl_th(GLenum pname, GLint param);
EAPI void evas_glActiveTexture_evgl_th(GLenum texture);
EAPI void evas_glGenTextures_evgl_th(GLsizei n, GLuint *textures);
EAPI void evas_glBindTexture_evgl_th(GLenum target, GLuint texture);
EAPI void evas_glDeleteTextures_evgl_th(GLsizei n, const GLuint *textures);
EAPI void evas_glTexImage2D_evgl_th(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
EAPI void evas_glTexSubImage2D_evgl_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
EAPI void evas_glCompressedTexImage2D_evgl_th(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
EAPI void evas_glCompressedTexSubImage2D_evgl_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
EAPI void evas_glTexParameterf_evgl_th(GLenum target, GLenum pname, GLfloat param);
EAPI void evas_glTexParameterfv_evgl_th(GLenum target, GLenum pname, const GLfloat *params);
EAPI void evas_glTexParameteri_evgl_th(GLenum target, GLenum pname, GLint param);
EAPI void evas_glTexParameteriv_evgl_th(GLenum target, GLenum pname, const GLint *params);
EAPI void evas_glScissor_evgl_th(GLint x, GLint y, GLsizei width, GLsizei height);
EAPI void evas_glBlendFunc_evgl_th(GLenum sfactor, GLenum dfactor);
EAPI void evas_glBlendColor_evgl_th(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
EAPI void evas_glDepthMask_evgl_th(GLboolean flag);
EAPI void evas_glClear_evgl_th(GLbitfield mask);
EAPI void evas_glClearColor_evgl_th(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
EAPI void evas_glReadPixels_evgl_th(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
EAPI void evas_glGenFramebuffers_evgl_th(GLsizei n, GLuint *framebuffers);
EAPI void evas_glBindFramebuffer_evgl_th(GLenum target, GLuint framebuffer);
EAPI void evas_glDeleteFramebuffers_evgl_th(GLsizei n, const GLuint *framebuffers);
EAPI void evas_glGenRenderbuffers_evgl_th(GLsizei n, GLuint *renderbuffers);
EAPI void evas_glBindRenderbuffer_evgl_th(GLenum target, GLuint renderbuffer);
EAPI void evas_glDeleteRenderbuffers_evgl_th(GLsizei n, const GLuint *renderbuffers);
EAPI void evas_glRenderbufferStorage_evgl_th(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
EAPI void evas_glFramebufferRenderbuffer_evgl_th(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
EAPI void evas_glFramebufferTexture2D_evgl_th(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
EAPI GLenum evas_glCheckFramebufferStatus_evgl_th(GLenum target);
EAPI void evas_glFlush_evgl_th(void);
EAPI void evas_glFinish_evgl_th(void);
EAPI void evas_glHint_evgl_th(GLenum target, GLenum mode);
EAPI const GLubyte * evas_glGetString_evgl_th(GLenum name);
EAPI void evas_glGetBooleanv_evgl_th(GLenum pname, GLboolean *data);
EAPI void evas_glGetFloatv_evgl_th(GLenum pname, GLfloat *data);
EAPI void evas_glGetIntegerv_evgl_th(GLenum pname, GLint *data);
EAPI GLboolean evas_glIsBuffer_evgl_th(GLint buffer);
EAPI void evas_glGetBufferParameteriv_evgl_th(GLenum target, GLenum pname, GLint *params);
EAPI GLboolean evas_glIsShader_evgl_th(GLuint shader);
EAPI void evas_glGetShaderiv_evgl_th(GLuint shader, GLenum pname, GLint *params);
EAPI void evas_glGetAttachedShaders_evgl_th(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
EAPI void evas_glGetShaderInfoLog_evgl_th(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
EAPI void evas_glGetShaderSource_evgl_th(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
EAPI void evas_glGetShaderPrecisionFormat_evgl_th(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
EAPI void evas_glGetVertexAttribfv_evgl_th(GLuint index, GLenum pname, GLfloat *params);
EAPI void evas_glGetVertexAttribiv_evgl_th(GLuint index, GLenum pname, GLint *params);
EAPI GLboolean evas_glIsProgram_evgl_th(GLuint program);
EAPI void evas_glGetProgramInfoLog_evgl_th(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
EAPI void evas_glGetProgramiv_evgl_th(GLuint program, GLenum pname, GLint *params);
EAPI GLboolean evas_glIsFramebuffer_evgl_th(GLint framebuffer);
EAPI GLboolean evas_glIsRenderbuffer_evgl_th(GLint renderbuffer);
EAPI void evas_glGetRenderbufferParameteriv_evgl_th(GLenum target, GLenum pname, GLint *params);
EAPI GLboolean evas_glIsTexture_evgl_th(GLint texture);
EAPI void evas_glClearDepthf_evgl_th(GLclampf depth);
EAPI void evas_glDepthRangef_evgl_th(GLclampf zNear, GLclampf zFar);
EAPI void evas_glClearDepth_evgl_th(GLclampf depth);
EAPI void evas_glDepthRange_evgl_th(GLclampf zNear, GLclampf zFar);
EAPI void evas_glGetFramebufferAttachmentParameteriv_evgl_th(GLenum target, GLenum attachment, GLenum pname, GLint* params);
