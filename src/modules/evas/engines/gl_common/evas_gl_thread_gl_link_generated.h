/*
 * This is an automatically generated file using a python script.
 * ($EFL_HOME/src/utils/evas/generate_gl_thread_api.py)
 * Recommend that you modify data files ($EFL_HOME/src/utils/evas/gl_api_def.txt)
 * and make use of scripts if you need to fix them.
 */
extern GLenum (*evas_glGetError_th)(void);
extern void (*evas_glVertexAttribPointer_th)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
extern void (*evas_glEnableVertexAttribArray_th)(GLuint index);
extern void (*evas_glDisableVertexAttribArray_th)(GLuint index);
extern void (*evas_glDrawArrays_th)(GLenum mode, GLint first, GLsizei count);
extern void (*evas_glDrawElements_th)(GLenum mode, GLsizei count, GLenum type, const void *indices);
extern void (*evas_glGenBuffers_th)(GLsizei n, GLuint *buffers);
extern void (*evas_glDeleteBuffers_th)(GLsizei n, const GLuint *buffers);
extern void (*evas_glBindBuffer_th)(GLenum target, GLuint buffer);
extern void (*evas_glBufferData_th)(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
extern void (*glMapBuffer_orig_evas_set)(void *func);
extern void *(*glMapBuffer_orig_evas_get)(void);
extern void * (*evas_glMapBuffer_th)(GLenum target, GLenum access);
extern void (*glUnmapBuffer_orig_evas_set)(void *func);
extern void *(*glUnmapBuffer_orig_evas_get)(void);
extern GLboolean (*evas_glUnmapBuffer_th)(GLenum target);
extern GLuint (*evas_glCreateShader_th)(GLenum type);
extern void (*evas_glShaderSource_th)(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
extern void (*evas_glCompileShader_th)(GLuint shader);
extern void (*glReleaseShaderCompiler_orig_evas_set)(void *func);
extern void *(*glReleaseShaderCompiler_orig_evas_get)(void);
extern void (*evas_glReleaseShaderCompiler_th)(void);
extern void (*evas_glDeleteShader_th)(GLuint shader);
extern GLuint (*evas_glCreateProgram_th)(void);
extern void (*evas_glAttachShader_th)(GLuint program, GLuint shader);
extern void (*evas_glDetachShader_th)(GLuint program, GLuint shader);
extern void (*evas_glLinkProgram_th)(GLuint program);
extern void (*evas_glUseProgram_th)(GLuint program);
extern void (*glProgramParameteri_orig_evas_set)(void *func);
extern void *(*glProgramParameteri_orig_evas_get)(void);
extern void (*evas_glProgramParameteri_th)(GLuint program, GLenum pname, GLint value);
extern void (*evas_glDeleteProgram_th)(GLuint program);
extern void (*glGetProgramBinary_orig_evas_set)(void *func);
extern void *(*glGetProgramBinary_orig_evas_get)(void);
extern void (*evas_glGetProgramBinary_th)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
extern void (*glProgramBinary_orig_evas_set)(void *func);
extern void *(*glProgramBinary_orig_evas_get)(void);
extern void (*evas_glProgramBinary_th)(GLuint program, GLenum binaryFormat, const void *binary, GLint length);
extern void (*evas_glGetActiveAttrib_th)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern GLint (*evas_glGetAttribLocation_th)(GLuint program, const GLchar *name);
extern void (*evas_glBindAttribLocation_th)(GLuint program, GLuint index, const GLchar *name);
extern GLint (*evas_glGetUniformLocation_th)(GLuint program, const GLchar *name);
extern void (*evas_glUniform1f_th)(GLint location, GLfloat v0);
extern void (*evas_glUniform1i_th)(GLint location, GLint v0);
extern void (*evas_glUniform2f_th)(GLint location, GLfloat v0, GLfloat v1);
extern void (*evas_glUniform2i_th)(GLint location, GLint v0, GLint v1);
extern void (*evas_glUniform3f_th)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void (*evas_glUniform3i_th)(GLint location, GLint v0, GLint v1, GLint v2);
extern void (*evas_glUniform4f_th)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void (*evas_glUniform4i_th)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
extern void (*evas_glUniform1fv_th)(GLint location, GLsizei count, const GLfloat *value);
extern void (*evas_glUniform1iv_th)(GLint location, GLsizei count, const GLint *value);
extern void (*evas_glUniform2fv_th)(GLint location, GLsizei count, const GLfloat *value);
extern void (*evas_glUniform2iv_th)(GLint location, GLsizei count, const GLint *value);
extern void (*evas_glUniform3fv_th)(GLint location, GLsizei count, const GLfloat *value);
extern void (*evas_glUniform3iv_th)(GLint location, GLsizei count, const GLint *value);
extern void (*evas_glUniform4fv_th)(GLint location, GLsizei count, const GLfloat *value);
extern void (*evas_glUniform4iv_th)(GLint location, GLsizei count, const GLint *value);
extern void (*evas_glUniformMatrix2fv_th)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*evas_glUniformMatrix3fv_th)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*evas_glUniformMatrix4fv_th)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void (*evas_glViewport_th)(GLint x, GLint y, GLsizei width, GLsizei height);
extern void (*evas_glEnable_th)(GLenum cap);
extern void (*evas_glDisable_th)(GLenum cap);
extern void (*evas_glLineWidth_th)(GLfloat width);
extern void (*evas_glPolygonOffset_th)(GLfloat factor, GLfloat units);
extern void (*evas_glPixelStorei_th)(GLenum pname, GLint param);
extern void (*evas_glActiveTexture_th)(GLenum texture);
extern void (*evas_glGenTextures_th)(GLsizei n, GLuint *textures);
extern void (*evas_glBindTexture_th)(GLenum target, GLuint texture);
extern void (*evas_glDeleteTextures_th)(GLsizei n, const GLuint *textures);
extern void (*evas_glTexImage2D_th)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
extern void (*evas_glTexSubImage2D_th)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
extern void (*evas_glCompressedTexImage2D_th)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
extern void (*evas_glCompressedTexSubImage2D_th)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
extern void (*evas_glTexParameterf_th)(GLenum target, GLenum pname, GLfloat param);
extern void (*evas_glTexParameterfv_th)(GLenum target, GLenum pname, const GLfloat *params);
extern void (*evas_glTexParameteri_th)(GLenum target, GLenum pname, GLint param);
extern void (*evas_glTexParameteriv_th)(GLenum target, GLenum pname, const GLint *params);
extern void (*evas_glScissor_th)(GLint x, GLint y, GLsizei width, GLsizei height);
extern void (*evas_glBlendFunc_th)(GLenum sfactor, GLenum dfactor);
extern void (*evas_glBlendColor_th)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void (*evas_glDepthMask_th)(GLboolean flag);
extern void (*evas_glClear_th)(GLbitfield mask);
extern void (*evas_glClearColor_th)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void (*evas_glReadPixels_th)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
extern void (*glGenFramebuffers_orig_evas_set)(void *func);
extern void *(*glGenFramebuffers_orig_evas_get)(void);
extern void (*evas_glGenFramebuffers_th)(GLsizei n, GLuint *framebuffers);
extern void (*glBindFramebuffer_orig_evas_set)(void *func);
extern void *(*glBindFramebuffer_orig_evas_get)(void);
extern void (*evas_glBindFramebuffer_th)(GLenum target, GLuint framebuffer);
extern void (*glDeleteFramebuffers_orig_evas_set)(void *func);
extern void *(*glDeleteFramebuffers_orig_evas_get)(void);
extern void (*evas_glDeleteFramebuffers_th)(GLsizei n, const GLuint *framebuffers);
extern void (*evas_glGenRenderbuffers_th)(GLsizei n, GLuint *renderbuffers);
extern void (*evas_glBindRenderbuffer_th)(GLenum target, GLuint renderbuffer);
extern void (*evas_glDeleteRenderbuffers_th)(GLsizei n, const GLuint *renderbuffers);
extern void (*evas_glRenderbufferStorage_th)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern void (*evas_glFramebufferRenderbuffer_th)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern void (*glFramebufferTexture2D_orig_evas_set)(void *func);
extern void *(*glFramebufferTexture2D_orig_evas_get)(void);
extern void (*evas_glFramebufferTexture2D_th)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void (*glFramebufferTexture2DMultisample_orig_evas_set)(void *func);
extern void *(*glFramebufferTexture2DMultisample_orig_evas_get)(void);
extern void (*evas_glFramebufferTexture2DMultisample_th)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);
extern GLenum (*evas_glCheckFramebufferStatus_th)(GLenum target);
extern void (*evas_glFlush_th)(void);
extern void (*evas_glFinish_th)(void);
extern void (*evas_glHint_th)(GLenum target, GLenum mode);
extern const GLubyte * (*evas_glGetString_th)(GLenum name);
extern void (*evas_glGetBooleanv_th)(GLenum pname, GLboolean *data);
extern void (*evas_glGetFloatv_th)(GLenum pname, GLfloat *data);
extern void (*evas_glGetIntegerv_th)(GLenum pname, GLint *data);
extern GLboolean (*evas_glIsBuffer_th)(GLint buffer);
extern void (*evas_glGetBufferParameteriv_th)(GLenum target, GLenum pname, GLint *params);
extern GLboolean (*evas_glIsShader_th)(GLuint shader);
extern void (*evas_glGetShaderiv_th)(GLuint shader, GLenum pname, GLint *params);
extern void (*evas_glGetAttachedShaders_th)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
extern void (*evas_glGetShaderInfoLog_th)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void (*evas_glGetShaderSource_th)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
extern void (*evas_glGetShaderPrecisionFormat_th)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
extern void (*evas_glGetVertexAttribfv_th)(GLuint index, GLenum pname, GLfloat *params);
extern void (*evas_glGetVertexAttribiv_th)(GLuint index, GLenum pname, GLint *params);
extern GLboolean (*evas_glIsProgram_th)(GLuint program);
extern void (*evas_glGetProgramInfoLog_th)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void (*evas_glGetProgramiv_th)(GLuint program, GLenum pname, GLint *params);
extern GLboolean (*evas_glIsFramebuffer_th)(GLint framebuffer);
extern void (*glGetFramebufferParameteriv_orig_evas_set)(void *func);
extern void *(*glGetFramebufferParameteriv_orig_evas_get)(void);
extern void (*evas_glGetFramebufferParameteriv_th)(GLenum target, GLenum pname, GLint *params);
extern GLboolean (*evas_glIsRenderbuffer_th)(GLint renderbuffer);
extern void (*evas_glGetRenderbufferParameteriv_th)(GLenum target, GLenum pname, GLint *params);
extern GLboolean (*evas_glIsTexture_th)(GLint texture);
extern void (*glStartTiling_orig_evas_set)(void *func);
extern void *(*glStartTiling_orig_evas_get)(void);
extern void (*evas_glStartTiling_th)(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e);
extern void (*glEndTiling_orig_evas_set)(void *func);
extern void *(*glEndTiling_orig_evas_get)(void);
extern void (*evas_glEndTiling_th)(GLuint a);
extern void (*glActivateTile_orig_evas_set)(void *func);
extern void *(*glActivateTile_orig_evas_get)(void);
extern void (*evas_glActivateTile_th)(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e);
extern void (*glEGLImageTargetTexture2DOES_orig_evas_set)(void *func);
extern void *(*glEGLImageTargetTexture2DOES_orig_evas_get)(void);
extern void (*evas_glEGLImageTargetTexture2DOES_th)(GLenum target, void *image);

#ifndef GL_GLES
extern void (*evas_glAlphaFunc_th)(GLenum func, GLclampf ref);
#endif

#ifndef GL_GLES
extern void (*evas_glGetTexLevelParameteriv_th)(GLenum target, GLint level, GLenum pname, GLint *params);
#endif

#ifndef GL_GLES
extern void (*glRenderbufferStorageMultisample_orig_evas_set)(void *func);
extern void *(*glRenderbufferStorageMultisample_orig_evas_get)(void);
extern void (*evas_glRenderbufferStorageMultisample_th)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#endif
extern void (*glGetStringi_orig_evas_set)(void *func);
extern void *(*glGetStringi_orig_evas_get)(void);
extern const GLubyte * (*evas_glGetStringi_th)(GLenum name, GLuint index);


extern void _gl_thread_link_gl_generated_init();
