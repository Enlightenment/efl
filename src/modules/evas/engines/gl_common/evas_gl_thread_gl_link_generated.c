/*
 * This is an automatically generated file using a python script.
 * ($EFL_HOME/src/utils/evas/generate_gl_thread_api.py)
 * Recommend that you modify data files ($EFL_HOME/src/utils/evas/gl_api_def.txt)
 * and make use of scripts if you need to fix them.
 */
GLenum (*evas_glGetError_th)(void) = NULL;
void (*evas_glVertexAttribPointer_th)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer) = NULL;
void (*evas_glEnableVertexAttribArray_th)(GLuint index) = NULL;
void (*evas_glDisableVertexAttribArray_th)(GLuint index) = NULL;
void (*evas_glDrawArrays_th)(GLenum mode, GLint first, GLsizei count) = NULL;
void (*evas_glDrawElements_th)(GLenum mode, GLsizei count, GLenum type, const void *indices) = NULL;
void (*evas_glGenBuffers_th)(GLsizei n, GLuint *buffers) = NULL;
void (*evas_glDeleteBuffers_th)(GLsizei n, const GLuint *buffers) = NULL;
void (*evas_glBindBuffer_th)(GLenum target, GLuint buffer) = NULL;
void (*evas_glBufferData_th)(GLenum target, GLsizeiptr size, const void *data, GLenum usage) = NULL;
void (*glMapBuffer_orig_evas_set)(void *func) = NULL;
void *(*glMapBuffer_orig_evas_get)(void) = NULL;
void * (*evas_glMapBuffer_th)(GLenum target, GLenum access) = NULL;
void (*glUnmapBuffer_orig_evas_set)(void *func) = NULL;
void *(*glUnmapBuffer_orig_evas_get)(void) = NULL;
GLboolean (*evas_glUnmapBuffer_th)(GLenum target) = NULL;
GLuint (*evas_glCreateShader_th)(GLenum type) = NULL;
void (*evas_glShaderSource_th)(GLuint shader, GLsizei count, const GLchar **string, const GLint *length) = NULL;
void (*evas_glCompileShader_th)(GLuint shader) = NULL;
void (*glReleaseShaderCompiler_orig_evas_set)(void *func) = NULL;
void *(*glReleaseShaderCompiler_orig_evas_get)(void) = NULL;
void (*evas_glReleaseShaderCompiler_th)(void) = NULL;
void (*evas_glDeleteShader_th)(GLuint shader) = NULL;
GLuint (*evas_glCreateProgram_th)(void) = NULL;
void (*evas_glAttachShader_th)(GLuint program, GLuint shader) = NULL;
void (*evas_glDetachShader_th)(GLuint program, GLuint shader) = NULL;
void (*evas_glLinkProgram_th)(GLuint program) = NULL;
void (*evas_glUseProgram_th)(GLuint program) = NULL;
void (*glProgramParameteri_orig_evas_set)(void *func) = NULL;
void *(*glProgramParameteri_orig_evas_get)(void) = NULL;
void (*evas_glProgramParameteri_th)(GLuint program, GLenum pname, GLint value) = NULL;
void (*evas_glDeleteProgram_th)(GLuint program) = NULL;
void (*glGetProgramBinary_orig_evas_set)(void *func) = NULL;
void *(*glGetProgramBinary_orig_evas_get)(void) = NULL;
void (*evas_glGetProgramBinary_th)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary) = NULL;
void (*glProgramBinary_orig_evas_set)(void *func) = NULL;
void *(*glProgramBinary_orig_evas_get)(void) = NULL;
void (*evas_glProgramBinary_th)(GLuint program, GLenum binaryFormat, const void *binary, GLint length) = NULL;
void (*evas_glGetActiveAttrib_th)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name) = NULL;
GLint (*evas_glGetAttribLocation_th)(GLuint program, const GLchar *name) = NULL;
void (*evas_glBindAttribLocation_th)(GLuint program, GLuint index, const GLchar *name) = NULL;
GLint (*evas_glGetUniformLocation_th)(GLuint program, const GLchar *name) = NULL;
void (*evas_glUniform1f_th)(GLint location, GLfloat v0) = NULL;
void (*evas_glUniform1i_th)(GLint location, GLint v0) = NULL;
void (*evas_glUniform2f_th)(GLint location, GLfloat v0, GLfloat v1) = NULL;
void (*evas_glUniform2i_th)(GLint location, GLint v0, GLint v1) = NULL;
void (*evas_glUniform3f_th)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) = NULL;
void (*evas_glUniform3i_th)(GLint location, GLint v0, GLint v1, GLint v2) = NULL;
void (*evas_glUniform4f_th)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) = NULL;
void (*evas_glUniform4i_th)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) = NULL;
void (*evas_glUniform1fv_th)(GLint location, GLsizei count, const GLfloat *value) = NULL;
void (*evas_glUniform1iv_th)(GLint location, GLsizei count, const GLint *value) = NULL;
void (*evas_glUniform2fv_th)(GLint location, GLsizei count, const GLfloat *value) = NULL;
void (*evas_glUniform2iv_th)(GLint location, GLsizei count, const GLint *value) = NULL;
void (*evas_glUniform3fv_th)(GLint location, GLsizei count, const GLfloat *value) = NULL;
void (*evas_glUniform3iv_th)(GLint location, GLsizei count, const GLint *value) = NULL;
void (*evas_glUniform4fv_th)(GLint location, GLsizei count, const GLfloat *value) = NULL;
void (*evas_glUniform4iv_th)(GLint location, GLsizei count, const GLint *value) = NULL;
void (*evas_glUniformMatrix2fv_th)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
void (*evas_glUniformMatrix3fv_th)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
void (*evas_glUniformMatrix4fv_th)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = NULL;
void (*evas_glViewport_th)(GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
void (*evas_glEnable_th)(GLenum cap) = NULL;
void (*evas_glDisable_th)(GLenum cap) = NULL;
void (*evas_glLineWidth_th)(GLfloat width) = NULL;
void (*evas_glPolygonOffset_th)(GLfloat factor, GLfloat units) = NULL;
void (*evas_glPixelStorei_th)(GLenum pname, GLint param) = NULL;
void (*evas_glActiveTexture_th)(GLenum texture) = NULL;
void (*evas_glGenTextures_th)(GLsizei n, GLuint *textures) = NULL;
void (*evas_glBindTexture_th)(GLenum target, GLuint texture) = NULL;
void (*evas_glDeleteTextures_th)(GLsizei n, const GLuint *textures) = NULL;
void (*evas_glTexImage2D_th)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels) = NULL;
void (*evas_glTexSubImage2D_th)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels) = NULL;
void (*evas_glCompressedTexImage2D_th)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data) = NULL;
void (*evas_glCompressedTexSubImage2D_th)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data) = NULL;
void (*evas_glTexParameterf_th)(GLenum target, GLenum pname, GLfloat param) = NULL;
void (*evas_glTexParameterfv_th)(GLenum target, GLenum pname, const GLfloat *params) = NULL;
void (*evas_glTexParameteri_th)(GLenum target, GLenum pname, GLint param) = NULL;
void (*evas_glTexParameteriv_th)(GLenum target, GLenum pname, const GLint *params) = NULL;
void (*evas_glScissor_th)(GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
void (*evas_glBlendFunc_th)(GLenum sfactor, GLenum dfactor) = NULL;
void (*evas_glBlendColor_th)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) = NULL;
void (*evas_glDepthMask_th)(GLboolean flag) = NULL;
void (*evas_glClear_th)(GLbitfield mask) = NULL;
void (*evas_glClearColor_th)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) = NULL;
void (*evas_glReadPixels_th)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels) = NULL;
void (*glGenFramebuffers_orig_evas_set)(void *func) = NULL;
void *(*glGenFramebuffers_orig_evas_get)(void) = NULL;
void (*evas_glGenFramebuffers_th)(GLsizei n, GLuint *framebuffers) = NULL;
void (*glBindFramebuffer_orig_evas_set)(void *func) = NULL;
void *(*glBindFramebuffer_orig_evas_get)(void) = NULL;
void (*evas_glBindFramebuffer_th)(GLenum target, GLuint framebuffer) = NULL;
void (*glDeleteFramebuffers_orig_evas_set)(void *func) = NULL;
void *(*glDeleteFramebuffers_orig_evas_get)(void) = NULL;
void (*evas_glDeleteFramebuffers_th)(GLsizei n, const GLuint *framebuffers) = NULL;
void (*evas_glGenRenderbuffers_th)(GLsizei n, GLuint *renderbuffers) = NULL;
void (*evas_glBindRenderbuffer_th)(GLenum target, GLuint renderbuffer) = NULL;
void (*evas_glDeleteRenderbuffers_th)(GLsizei n, const GLuint *renderbuffers) = NULL;
void (*evas_glRenderbufferStorage_th)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
void (*evas_glFramebufferRenderbuffer_th)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = NULL;
void (*glFramebufferTexture2D_orig_evas_set)(void *func) = NULL;
void *(*glFramebufferTexture2D_orig_evas_get)(void) = NULL;
void (*evas_glFramebufferTexture2D_th)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = NULL;
void (*glFramebufferTexture2DMultisample_orig_evas_set)(void *func) = NULL;
void *(*glFramebufferTexture2DMultisample_orig_evas_get)(void) = NULL;
void (*evas_glFramebufferTexture2DMultisample_th)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples) = NULL;
GLenum (*evas_glCheckFramebufferStatus_th)(GLenum target) = NULL;
void (*evas_glFlush_th)(void) = NULL;
void (*evas_glFinish_th)(void) = NULL;
void (*evas_glHint_th)(GLenum target, GLenum mode) = NULL;
const GLubyte * (*evas_glGetString_th)(GLenum name) = NULL;
void (*evas_glGetBooleanv_th)(GLenum pname, GLboolean *data) = NULL;
void (*evas_glGetFloatv_th)(GLenum pname, GLfloat *data) = NULL;
void (*evas_glGetIntegerv_th)(GLenum pname, GLint *data) = NULL;
GLboolean (*evas_glIsBuffer_th)(GLint buffer) = NULL;
void (*evas_glGetBufferParameteriv_th)(GLenum target, GLenum pname, GLint *params) = NULL;
GLboolean (*evas_glIsShader_th)(GLuint shader) = NULL;
void (*evas_glGetShaderiv_th)(GLuint shader, GLenum pname, GLint *params) = NULL;
void (*evas_glGetAttachedShaders_th)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders) = NULL;
void (*evas_glGetShaderInfoLog_th)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) = NULL;
void (*evas_glGetShaderSource_th)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source) = NULL;
void (*evas_glGetShaderPrecisionFormat_th)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision) = NULL;
void (*evas_glGetVertexAttribfv_th)(GLuint index, GLenum pname, GLfloat *params) = NULL;
void (*evas_glGetVertexAttribiv_th)(GLuint index, GLenum pname, GLint *params) = NULL;
GLboolean (*evas_glIsProgram_th)(GLuint program) = NULL;
void (*evas_glGetProgramInfoLog_th)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog) = NULL;
void (*evas_glGetProgramiv_th)(GLuint program, GLenum pname, GLint *params) = NULL;
GLboolean (*evas_glIsFramebuffer_th)(GLint framebuffer) = NULL;
void (*glGetFramebufferParameteriv_orig_evas_set)(void *func) = NULL;
void *(*glGetFramebufferParameteriv_orig_evas_get)(void) = NULL;
void (*evas_glGetFramebufferParameteriv_th)(GLenum target, GLenum pname, GLint *params) = NULL;
GLboolean (*evas_glIsRenderbuffer_th)(GLint renderbuffer) = NULL;
void (*evas_glGetRenderbufferParameteriv_th)(GLenum target, GLenum pname, GLint *params) = NULL;
GLboolean (*evas_glIsTexture_th)(GLint texture) = NULL;
void (*glStartTiling_orig_evas_set)(void *func) = NULL;
void *(*glStartTiling_orig_evas_get)(void) = NULL;
void (*evas_glStartTiling_th)(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e) = NULL;
void (*glEndTiling_orig_evas_set)(void *func) = NULL;
void *(*glEndTiling_orig_evas_get)(void) = NULL;
void (*evas_glEndTiling_th)(GLuint a) = NULL;
void (*glActivateTile_orig_evas_set)(void *func) = NULL;
void *(*glActivateTile_orig_evas_get)(void) = NULL;
void (*evas_glActivateTile_th)(GLuint a, GLuint b, GLuint c, GLuint d, GLuint e) = NULL;
void (*glEGLImageTargetTexture2DOES_orig_evas_set)(void *func) = NULL;
void *(*glEGLImageTargetTexture2DOES_orig_evas_get)(void) = NULL;
void (*evas_glEGLImageTargetTexture2DOES_th)(GLenum target, void *image) = NULL;

#ifndef GL_GLES
void (*evas_glAlphaFunc_th)(GLenum func, GLclampf ref) = NULL;
#endif

#ifndef GL_GLES
void (*evas_glGetTexLevelParameteriv_th)(GLenum target, GLint level, GLenum pname, GLint *params) = NULL;
#endif

#ifndef GL_GLES
void (*glRenderbufferStorageMultisample_orig_evas_set)(void *func) = NULL;
void *(*glRenderbufferStorageMultisample_orig_evas_get)(void) = NULL;
void (*evas_glRenderbufferStorageMultisample_th)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
#endif
void (*glGetStringi_orig_evas_set)(void *func) = NULL;
void *(*glGetStringi_orig_evas_get)(void) = NULL;
const GLubyte * (*evas_glGetStringi_th)(GLenum name, GLuint index) = NULL;


void
_gl_thread_link_gl_generated_init()
{
#define LINK2GENERIC(sym) \
   sym = dlsym(RTLD_DEFAULT, #sym); \
   if (!sym) ERR("Could not find function '%s'", #sym);

   LINK2GENERIC(evas_glGetError_th);
   LINK2GENERIC(evas_glVertexAttribPointer_th);
   LINK2GENERIC(evas_glEnableVertexAttribArray_th);
   LINK2GENERIC(evas_glDisableVertexAttribArray_th);
   LINK2GENERIC(evas_glDrawArrays_th);
   LINK2GENERIC(evas_glDrawElements_th);
   LINK2GENERIC(evas_glGenBuffers_th);
   LINK2GENERIC(evas_glDeleteBuffers_th);
   LINK2GENERIC(evas_glBindBuffer_th);
   LINK2GENERIC(evas_glBufferData_th);
   LINK2GENERIC(glMapBuffer_orig_evas_set);
   LINK2GENERIC(glMapBuffer_orig_evas_get);
   LINK2GENERIC(evas_glMapBuffer_th);
   LINK2GENERIC(glUnmapBuffer_orig_evas_set);
   LINK2GENERIC(glUnmapBuffer_orig_evas_get);
   LINK2GENERIC(evas_glUnmapBuffer_th);
   LINK2GENERIC(evas_glCreateShader_th);
   LINK2GENERIC(evas_glShaderSource_th);
   LINK2GENERIC(evas_glCompileShader_th);
   LINK2GENERIC(glReleaseShaderCompiler_orig_evas_set);
   LINK2GENERIC(glReleaseShaderCompiler_orig_evas_get);
   LINK2GENERIC(evas_glReleaseShaderCompiler_th);
   LINK2GENERIC(evas_glDeleteShader_th);
   LINK2GENERIC(evas_glCreateProgram_th);
   LINK2GENERIC(evas_glAttachShader_th);
   LINK2GENERIC(evas_glDetachShader_th);
   LINK2GENERIC(evas_glLinkProgram_th);
   LINK2GENERIC(evas_glUseProgram_th);
   LINK2GENERIC(glProgramParameteri_orig_evas_set);
   LINK2GENERIC(glProgramParameteri_orig_evas_get);
   LINK2GENERIC(evas_glProgramParameteri_th);
   LINK2GENERIC(evas_glDeleteProgram_th);
   LINK2GENERIC(glGetProgramBinary_orig_evas_set);
   LINK2GENERIC(glGetProgramBinary_orig_evas_get);
   LINK2GENERIC(evas_glGetProgramBinary_th);
   LINK2GENERIC(glProgramBinary_orig_evas_set);
   LINK2GENERIC(glProgramBinary_orig_evas_get);
   LINK2GENERIC(evas_glProgramBinary_th);
   LINK2GENERIC(evas_glGetActiveAttrib_th);
   LINK2GENERIC(evas_glGetAttribLocation_th);
   LINK2GENERIC(evas_glBindAttribLocation_th);
   LINK2GENERIC(evas_glGetUniformLocation_th);
   LINK2GENERIC(evas_glUniform1f_th);
   LINK2GENERIC(evas_glUniform1i_th);
   LINK2GENERIC(evas_glUniform2f_th);
   LINK2GENERIC(evas_glUniform2i_th);
   LINK2GENERIC(evas_glUniform3f_th);
   LINK2GENERIC(evas_glUniform3i_th);
   LINK2GENERIC(evas_glUniform4f_th);
   LINK2GENERIC(evas_glUniform4i_th);
   LINK2GENERIC(evas_glUniform1fv_th);
   LINK2GENERIC(evas_glUniform1iv_th);
   LINK2GENERIC(evas_glUniform2fv_th);
   LINK2GENERIC(evas_glUniform2iv_th);
   LINK2GENERIC(evas_glUniform3fv_th);
   LINK2GENERIC(evas_glUniform3iv_th);
   LINK2GENERIC(evas_glUniform4fv_th);
   LINK2GENERIC(evas_glUniform4iv_th);
   LINK2GENERIC(evas_glUniformMatrix2fv_th);
   LINK2GENERIC(evas_glUniformMatrix3fv_th);
   LINK2GENERIC(evas_glUniformMatrix4fv_th);
   LINK2GENERIC(evas_glViewport_th);
   LINK2GENERIC(evas_glEnable_th);
   LINK2GENERIC(evas_glDisable_th);
   LINK2GENERIC(evas_glLineWidth_th);
   LINK2GENERIC(evas_glPolygonOffset_th);
   LINK2GENERIC(evas_glPixelStorei_th);
   LINK2GENERIC(evas_glActiveTexture_th);
   LINK2GENERIC(evas_glGenTextures_th);
   LINK2GENERIC(evas_glBindTexture_th);
   LINK2GENERIC(evas_glDeleteTextures_th);
   LINK2GENERIC(evas_glTexImage2D_th);
   LINK2GENERIC(evas_glTexSubImage2D_th);
   LINK2GENERIC(evas_glCompressedTexImage2D_th);
   LINK2GENERIC(evas_glCompressedTexSubImage2D_th);
   LINK2GENERIC(evas_glTexParameterf_th);
   LINK2GENERIC(evas_glTexParameterfv_th);
   LINK2GENERIC(evas_glTexParameteri_th);
   LINK2GENERIC(evas_glTexParameteriv_th);
   LINK2GENERIC(evas_glScissor_th);
   LINK2GENERIC(evas_glBlendFunc_th);
   LINK2GENERIC(evas_glBlendColor_th);
   LINK2GENERIC(evas_glDepthMask_th);
   LINK2GENERIC(evas_glClear_th);
   LINK2GENERIC(evas_glClearColor_th);
   LINK2GENERIC(evas_glReadPixels_th);
   LINK2GENERIC(glGenFramebuffers_orig_evas_set);
   LINK2GENERIC(glGenFramebuffers_orig_evas_get);
   LINK2GENERIC(evas_glGenFramebuffers_th);
   LINK2GENERIC(glBindFramebuffer_orig_evas_set);
   LINK2GENERIC(glBindFramebuffer_orig_evas_get);
   LINK2GENERIC(evas_glBindFramebuffer_th);
   LINK2GENERIC(glDeleteFramebuffers_orig_evas_set);
   LINK2GENERIC(glDeleteFramebuffers_orig_evas_get);
   LINK2GENERIC(evas_glDeleteFramebuffers_th);
   LINK2GENERIC(evas_glGenRenderbuffers_th);
   LINK2GENERIC(evas_glBindRenderbuffer_th);
   LINK2GENERIC(evas_glDeleteRenderbuffers_th);
   LINK2GENERIC(evas_glRenderbufferStorage_th);
   LINK2GENERIC(evas_glFramebufferRenderbuffer_th);
   LINK2GENERIC(glFramebufferTexture2D_orig_evas_set);
   LINK2GENERIC(glFramebufferTexture2D_orig_evas_get);
   LINK2GENERIC(evas_glFramebufferTexture2D_th);
   LINK2GENERIC(glFramebufferTexture2DMultisample_orig_evas_set);
   LINK2GENERIC(glFramebufferTexture2DMultisample_orig_evas_get);
   LINK2GENERIC(evas_glFramebufferTexture2DMultisample_th);
   LINK2GENERIC(evas_glCheckFramebufferStatus_th);
   LINK2GENERIC(evas_glFlush_th);
   LINK2GENERIC(evas_glFinish_th);
   LINK2GENERIC(evas_glHint_th);
   LINK2GENERIC(evas_glGetString_th);
   LINK2GENERIC(evas_glGetBooleanv_th);
   LINK2GENERIC(evas_glGetFloatv_th);
   LINK2GENERIC(evas_glGetIntegerv_th);
   LINK2GENERIC(evas_glIsBuffer_th);
   LINK2GENERIC(evas_glGetBufferParameteriv_th);
   LINK2GENERIC(evas_glIsShader_th);
   LINK2GENERIC(evas_glGetShaderiv_th);
   LINK2GENERIC(evas_glGetAttachedShaders_th);
   LINK2GENERIC(evas_glGetShaderInfoLog_th);
   LINK2GENERIC(evas_glGetShaderSource_th);
   LINK2GENERIC(evas_glGetShaderPrecisionFormat_th);
   LINK2GENERIC(evas_glGetVertexAttribfv_th);
   LINK2GENERIC(evas_glGetVertexAttribiv_th);
   LINK2GENERIC(evas_glIsProgram_th);
   LINK2GENERIC(evas_glGetProgramInfoLog_th);
   LINK2GENERIC(evas_glGetProgramiv_th);
   LINK2GENERIC(evas_glIsFramebuffer_th);
   LINK2GENERIC(glGetFramebufferParameteriv_orig_evas_set);
   LINK2GENERIC(glGetFramebufferParameteriv_orig_evas_get);
   LINK2GENERIC(evas_glGetFramebufferParameteriv_th);
   LINK2GENERIC(evas_glIsRenderbuffer_th);
   LINK2GENERIC(evas_glGetRenderbufferParameteriv_th);
   LINK2GENERIC(evas_glIsTexture_th);
   LINK2GENERIC(glStartTiling_orig_evas_set);
   LINK2GENERIC(glStartTiling_orig_evas_get);
   LINK2GENERIC(evas_glStartTiling_th);
   LINK2GENERIC(glEndTiling_orig_evas_set);
   LINK2GENERIC(glEndTiling_orig_evas_get);
   LINK2GENERIC(evas_glEndTiling_th);
   LINK2GENERIC(glActivateTile_orig_evas_set);
   LINK2GENERIC(glActivateTile_orig_evas_get);
   LINK2GENERIC(evas_glActivateTile_th);
   LINK2GENERIC(glEGLImageTargetTexture2DOES_orig_evas_set);
   LINK2GENERIC(glEGLImageTargetTexture2DOES_orig_evas_get);
   LINK2GENERIC(evas_glEGLImageTargetTexture2DOES_th);

#ifndef GL_GLES
   LINK2GENERIC(evas_glAlphaFunc_th);
#endif


#ifndef GL_GLES
   LINK2GENERIC(evas_glGetTexLevelParameteriv_th);
#endif


#ifndef GL_GLES
   LINK2GENERIC(glRenderbufferStorageMultisample_orig_evas_set);
   LINK2GENERIC(glRenderbufferStorageMultisample_orig_evas_get);
   LINK2GENERIC(evas_glRenderbufferStorageMultisample_th);
#endif

   LINK2GENERIC(glGetStringi_orig_evas_set);
   LINK2GENERIC(glGetStringi_orig_evas_get);
   LINK2GENERIC(evas_glGetStringi_th);
}
