/*
 * This is an automatically generated file using a python script.
 * ($EFL_HOME/src/utils/evas/generate_gl_thread_api.py)
 * Recommend that you modify data files ($EFL_HOME/src/utils/evas/gl_api_def.txt)
 * and make use of scripts if you need to fix them.
 */

EAPI GLenum evas_glGetError_th(void);
EAPI void evas_glVertexAttribPointer_th(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
EAPI void evas_glEnableVertexAttribArray_th(GLuint index);
EAPI void evas_glDisableVertexAttribArray_th(GLuint index);
EAPI void evas_glDrawArrays_th(GLenum mode, GLint first, GLsizei count);
EAPI void evas_glDrawElements_th(GLenum mode, GLsizei count, GLenum type, const void *indices);
EAPI void evas_glGenBuffers_th(GLsizei n, GLuint *buffers);
EAPI void evas_glDeleteBuffers_th(GLsizei n, const GLuint *buffers);
EAPI void evas_glBindBuffer_th(GLenum target, GLuint buffer);
EAPI void evas_glBufferData_th(GLenum target, GLsizeiptr size, const void *data, GLenum usage);

EAPI void glMapBuffer_orig_evas_set(void *func);
EAPI void *glMapBuffer_orig_evas_get(void);
EAPI void * evas_glMapBuffer_th(GLenum target, GLenum access);

EAPI void glUnmapBuffer_orig_evas_set(void *func);
EAPI void *glUnmapBuffer_orig_evas_get(void);
EAPI GLboolean evas_glUnmapBuffer_th(GLenum target);
EAPI GLuint evas_glCreateShader_th(GLenum type);
EAPI void evas_glShaderSource_th(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
EAPI void evas_glCompileShader_th(GLuint shader);

EAPI void glReleaseShaderCompiler_orig_evas_set(void *func);
EAPI void *glReleaseShaderCompiler_orig_evas_get(void);
EAPI void evas_glReleaseShaderCompiler_th(void);
EAPI void evas_glDeleteShader_th(GLuint shader);
EAPI GLuint evas_glCreateProgram_th(void);
EAPI void evas_glAttachShader_th(GLuint program, GLuint shader);
EAPI void evas_glDetachShader_th(GLuint program, GLuint shader);
EAPI void evas_glLinkProgram_th(GLuint program);
EAPI void evas_glUseProgram_th(GLuint program);

EAPI void glProgramParameteri_orig_evas_set(void *func);
EAPI void *glProgramParameteri_orig_evas_get(void);
EAPI void evas_glProgramParameteri_th(GLuint program, GLenum pname, GLint value);
EAPI void evas_glDeleteProgram_th(GLuint program);

EAPI void glGetProgramBinary_orig_evas_set(void *func);
EAPI void *glGetProgramBinary_orig_evas_get(void);
EAPI void evas_glGetProgramBinary_th(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);

EAPI void glProgramBinary_orig_evas_set(void *func);
EAPI void *glProgramBinary_orig_evas_get(void);
EAPI void evas_glProgramBinary_th(GLuint program, GLenum binaryFormat, const void *binary, GLint length);
EAPI void evas_glGetActiveAttrib_th(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
EAPI GLint evas_glGetAttribLocation_th(GLuint program, const GLchar *name);
EAPI void evas_glBindAttribLocation_th(GLuint program, GLuint index, const GLchar *name);
EAPI GLint evas_glGetUniformLocation_th(GLuint program, const GLchar *name);
EAPI void evas_glUniform1f_th(GLint location, GLfloat v0);
EAPI void evas_glUniform1i_th(GLint location, GLint v0);
EAPI void evas_glUniform2f_th(GLint location, GLfloat v0, GLfloat v1);
EAPI void evas_glUniform2i_th(GLint location, GLint v0, GLint v1);
EAPI void evas_glUniform3f_th(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
EAPI void evas_glUniform3i_th(GLint location, GLint v0, GLint v1, GLint v2);
EAPI void evas_glUniform4f_th(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
EAPI void evas_glUniform4i_th(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
EAPI void evas_glUniform1fv_th(GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glUniform1iv_th(GLint location, GLsizei count, const GLint *value);
EAPI void evas_glUniform2fv_th(GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glUniform2iv_th(GLint location, GLsizei count, const GLint *value);
EAPI void evas_glUniform3fv_th(GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glUniform3iv_th(GLint location, GLsizei count, const GLint *value);
EAPI void evas_glUniform4fv_th(GLint location, GLsizei count, const GLfloat *value);
EAPI void evas_glUniform4iv_th(GLint location, GLsizei count, const GLint *value);
EAPI void evas_glUniformMatrix2fv_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glUniformMatrix3fv_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glUniformMatrix4fv_th(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
EAPI void evas_glViewport_th(GLint x, GLint y, GLsizei width, GLsizei height);
EAPI void evas_glEnable_th(GLenum cap);
EAPI void evas_glDisable_th(GLenum cap);
EAPI void evas_glLineWidth_th(GLfloat width);
EAPI void evas_glPolygonOffset_th(GLfloat factor, GLfloat units);
EAPI void evas_glPixelStorei_th(GLenum pname, GLint param);
EAPI void evas_glActiveTexture_th(GLenum texture);
EAPI void evas_glGenTextures_th(GLsizei n, GLuint *textures);
EAPI void evas_glBindTexture_th(GLenum target, GLuint texture);
EAPI void evas_glDeleteTextures_th(GLsizei n, const GLuint *textures);
EAPI void evas_glTexImage2D_th(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
EAPI void evas_glTexSubImage2D_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
EAPI void evas_glCompressedTexImage2D_th(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
EAPI void evas_glCompressedTexSubImage2D_th(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
EAPI void evas_glTexParameterf_th(GLenum target, GLenum pname, GLfloat param);
EAPI void evas_glTexParameterfv_th(GLenum target, GLenum pname, const GLfloat *params);
EAPI void evas_glTexParameteri_th(GLenum target, GLenum pname, GLint param);
EAPI void evas_glTexParameteriv_th(GLenum target, GLenum pname, const GLint *params);
EAPI void evas_glScissor_th(GLint x, GLint y, GLsizei width, GLsizei height);
EAPI void evas_glBlendFunc_th(GLenum sfactor, GLenum dfactor);
EAPI void evas_glBlendColor_th(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
EAPI void evas_glDepthMask_th(GLboolean flag);
EAPI void evas_glClear_th(GLbitfield mask);
EAPI void evas_glClearColor_th(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
EAPI void evas_glReadPixels_th(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);

EAPI void glGenFramebuffers_orig_evas_set(void *func);
EAPI void *glGenFramebuffers_orig_evas_get(void);
EAPI void evas_glGenFramebuffers_th(GLsizei n, GLuint *framebuffers);

EAPI void glBindFramebuffer_orig_evas_set(void *func);
EAPI void *glBindFramebuffer_orig_evas_get(void);
EAPI void evas_glBindFramebuffer_th(GLenum target, GLuint framebuffer);

EAPI void glDeleteFramebuffers_orig_evas_set(void *func);
EAPI void *glDeleteFramebuffers_orig_evas_get(void);
EAPI void evas_glDeleteFramebuffers_th(GLsizei n, const GLuint *framebuffers);
EAPI void evas_glGenRenderbuffers_th(GLsizei n, GLuint *renderbuffers);
EAPI void evas_glBindRenderbuffer_th(GLenum target, GLuint renderbuffer);
EAPI void evas_glDeleteRenderbuffers_th(GLsizei n, const GLuint *renderbuffers);
EAPI void evas_glRenderbufferStorage_th(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
EAPI void evas_glFramebufferRenderbuffer_th(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);

EAPI void glFramebufferTexture2D_orig_evas_set(void *func);
EAPI void *glFramebufferTexture2D_orig_evas_get(void);
EAPI void evas_glFramebufferTexture2D_th(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);

EAPI void glFramebufferTexture2DMultisample_orig_evas_set(void *func);
EAPI void *glFramebufferTexture2DMultisample_orig_evas_get(void);
EAPI void evas_glFramebufferTexture2DMultisample_th(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
EAPI GLenum evas_glCheckFramebufferStatus_th(GLenum target);
EAPI void evas_glFlush_th(void);
EAPI void evas_glFinish_th(void);
EAPI void evas_glHint_th(GLenum target, GLenum mode);
EAPI const GLubyte * evas_glGetString_th(GLenum name);
EAPI void evas_glGetBooleanv_th(GLenum pname, GLboolean *data);
EAPI void evas_glGetFloatv_th(GLenum pname, GLfloat *data);
EAPI void evas_glGetIntegerv_th(GLenum pname, GLint *data);
EAPI GLboolean evas_glIsBuffer_th(GLint buffer);
EAPI void evas_glGetBufferParameteriv_th(GLenum target, GLenum pname, GLint *params);
EAPI GLboolean evas_glIsShader_th(GLuint shader);
EAPI void evas_glGetShaderiv_th(GLuint shader, GLenum pname, GLint *params);
EAPI void evas_glGetAttachedShaders_th(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
EAPI void evas_glGetShaderInfoLog_th(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
EAPI void evas_glGetShaderSource_th(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
EAPI void evas_glGetShaderPrecisionFormat_th(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
EAPI void evas_glGetVertexAttribfv_th(GLuint index, GLenum pname, GLfloat *params);
EAPI void evas_glGetVertexAttribiv_th(GLuint index, GLenum pname, GLint *params);
EAPI GLboolean evas_glIsProgram_th(GLuint program);
EAPI void evas_glGetProgramInfoLog_th(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
EAPI void evas_glGetProgramiv_th(GLuint program, GLenum pname, GLint *params);
EAPI GLboolean evas_glIsFramebuffer_th(GLint framebuffer);

EAPI void glGetFramebufferParameteriv_orig_evas_set(void *func);
EAPI void *glGetFramebufferParameteriv_orig_evas_get(void);
EAPI void evas_glGetFramebufferParameteriv_th(GLenum target, GLenum pname, GLint *params);
EAPI GLboolean evas_glIsRenderbuffer_th(GLint renderbuffer);
EAPI void evas_glGetRenderbufferParameteriv_th(GLenum target, GLenum pname, GLint *params);
EAPI GLboolean evas_glIsTexture_th(GLint texture);

EAPI void glStartTiling_orig_evas_set(void *func);
EAPI void *glStartTiling_orig_evas_get(void);
EAPI void evas_glStartTiling_th(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e);

EAPI void glEndTiling_orig_evas_set(void *func);
EAPI void *glEndTiling_orig_evas_get(void);
EAPI void evas_glEndTiling_th(GLuint a);

EAPI void glActivateTile_orig_evas_set(void *func);
EAPI void *glActivateTile_orig_evas_get(void);
EAPI void evas_glActivateTile_th(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e);

EAPI void glEGLImageTargetTexture2DOES_orig_evas_set(void *func);
EAPI void *glEGLImageTargetTexture2DOES_orig_evas_get(void);
EAPI void evas_glEGLImageTargetTexture2DOES_th(GLenum target, void *image);
#ifndef GL_GLES

EAPI void evas_glAlphaFunc_th(GLenum func, GLclampf ref);
#endif

#ifndef GL_GLES

EAPI void evas_glGetTexLevelParameteriv_th(GLenum target, GLint level, GLenum pname, GLint *params);
#endif

#ifndef GL_GLES


EAPI void glRenderbufferStorageMultisample_orig_evas_set(void *func);
EAPI void *glRenderbufferStorageMultisample_orig_evas_get(void);
EAPI void evas_glRenderbufferStorageMultisample_th(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#endif


EAPI void glGetStringi_orig_evas_set(void *func);
EAPI void *glGetStringi_orig_evas_get(void);
EAPI const GLubyte * evas_glGetStringi_th(GLenum name, GLuint index);
