_EVASGL_FUNCTION_PRIVATE_BEGIN_VOID( glDrawBuffers, (GLsizei n, const GLenum *bufs), (n, bufs))
_EVASGL_FUNCTION_PRIVATE_BEGIN(const GLubyte *, glGetStringi, (GLenum name, GLuint index), (name, index))
_EVASGL_FUNCTION_PRIVATE_BEGIN_VOID( glReadBuffer, (GLenum src), (src))
//GLES 3.1
_EVASGL_FUNCTION_PRIVATE_BEGIN_VOID( glFramebufferParameteri, (GLenum target, GLenum pname, GLint param), (target, pname, param))
_EVASGL_FUNCTION_PRIVATE_BEGIN_VOID( glGetFramebufferParameteriv, (GLenum target, GLenum pname, GLint *params), (target, pname, params))
//GLES 3.2
_EVASGL_FUNCTION_PRIVATE_BEGIN_VOID( glFramebufferTexture, (GLenum target, GLenum attachment, GLuint texture, GLint level), (target, attachment, texture, level))

_EVASGL_FUNCTION_BEGIN_VOID( glBeginQuery, (GLenum target, GLuint id), (target, id))
_EVASGL_FUNCTION_BEGIN_VOID( glBeginTransformFeedback, (GLenum primitiveMode), (primitiveMode))
_EVASGL_FUNCTION_BEGIN_VOID( glBindBufferBase, (GLenum target, GLuint index, GLuint buffer), (target, index, buffer))
_EVASGL_FUNCTION_BEGIN_VOID( glBindBufferRange, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size), (target, index, buffer, offset, size))
_EVASGL_FUNCTION_BEGIN_VOID( glBindSampler, (GLuint unit, GLuint sampler), (unit, sampler))
_EVASGL_FUNCTION_BEGIN_VOID( glBindTransformFeedback, (GLenum target, GLuint id), (target, id))
_EVASGL_FUNCTION_BEGIN_VOID( glBindVertexArray, (GLuint array), (array))
_EVASGL_FUNCTION_BEGIN_VOID( glBlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter), (srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter))
_EVASGL_FUNCTION_BEGIN_VOID( glClearBufferfi, (GLenum buffer, GLint drawBuffer, GLfloat depth, GLint stencil), (buffer, drawBuffer, depth, stencil))
_EVASGL_FUNCTION_BEGIN_VOID( glClearBufferfv, (GLenum buffer, GLint drawBuffer, const GLfloat * value), (buffer, drawBuffer, value))
_EVASGL_FUNCTION_BEGIN_VOID( glClearBufferiv, (GLenum buffer, GLint drawBuffer, const GLint * value), (buffer, drawBuffer, value))
_EVASGL_FUNCTION_BEGIN_VOID( glClearBufferuiv, (GLenum buffer, GLint drawBuffer, const GLuint * value), (buffer, drawBuffer, value))
_EVASGL_FUNCTION_BEGIN(GLenum, glClientWaitSync, (GLsync sync, GLbitfield flags, uint64_t timeout), (sync, flags, timeout))
_EVASGL_FUNCTION_BEGIN_VOID( glCompressedTexImage3D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data), (target, level, internalformat, width, height, depth, border, imageSize, data))
_EVASGL_FUNCTION_BEGIN_VOID( glCompressedTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data), (target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data))
_EVASGL_FUNCTION_BEGIN_VOID( glCopyBufferSubData, (GLenum readtarget, GLenum writetarget, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size), (readtarget, writetarget, readoffset, writeoffset, size))
_EVASGL_FUNCTION_BEGIN_VOID( glCopyTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height), (target, level, xoffset, yoffset, zoffset, x, y, width, height))
_EVASGL_FUNCTION_BEGIN_VOID( glDeleteQueries, (GLsizei n, const GLuint * ids), (n, ids))
_EVASGL_FUNCTION_BEGIN_VOID( glDeleteSamplers, (GLsizei n, const GLuint * samplers), (n, samplers))
_EVASGL_FUNCTION_BEGIN_VOID( glDeleteSync, (GLsync sync), (sync))
_EVASGL_FUNCTION_BEGIN_VOID( glDeleteTransformFeedbacks, (GLsizei n, const GLuint *ids), (n, ids))
_EVASGL_FUNCTION_BEGIN_VOID( glDeleteVertexArrays, (GLsizei n, const GLuint *arrays), (n, arrays))
_EVASGL_FUNCTION_BEGIN_VOID( glDrawArraysInstanced, (GLenum mode, GLint first, GLsizei count, GLsizei primcount), (mode, first, count, primcount))
_EVASGL_FUNCTION_BEGIN_VOID( glDrawElementsInstanced, (GLenum mode, GLsizei count, GLenum type, const void * indices, GLsizei primcount), (mode, count, type, indices, primcount))
_EVASGL_FUNCTION_BEGIN_VOID( glDrawRangeElements, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices), (mode, start, end, count, type, indices))
_EVASGL_FUNCTION_BEGIN_VOID( glEndQuery, (GLenum target), (target))
_EVASGL_FUNCTION_BEGIN_VOID( glEndTransformFeedback, (void), ())
_EVASGL_FUNCTION_BEGIN(GLsync, glFenceSync, (GLenum condition, GLbitfield flags), (condition, flags))
_EVASGL_FUNCTION_BEGIN(GLsync, glFlushMappedBufferRange, (GLenum target, GLintptr offset, GLsizeiptr length), (target, offset, length))
_EVASGL_FUNCTION_BEGIN_VOID( glFramebufferTextureLayer, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer), (target, attachment, texture, level, layer))
_EVASGL_FUNCTION_BEGIN_VOID( glGenQueries, (GLsizei n, GLuint * ids), (n, ids))
_EVASGL_FUNCTION_BEGIN_VOID( glGenSamplers, (GLsizei n, GLuint *samplers), (n, samplers))
_EVASGL_FUNCTION_BEGIN_VOID( glGenTransformFeedbacks, (GLsizei n, GLuint *ids), (n, ids))
_EVASGL_FUNCTION_BEGIN_VOID( glGenVertexArrays, (GLsizei n, GLuint *arrays), (n, arrays))
_EVASGL_FUNCTION_BEGIN_VOID( glGetActiveUniformBlockiv, (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params), (program, uniformBlockIndex, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetActiveUniformBlockName, (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName), (program, uniformBlockIndex, bufSize, length, uniformBlockName))
_EVASGL_FUNCTION_BEGIN_VOID( glGetActiveUniformsiv, (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params), (program, uniformCount, uniformIndices, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetBufferParameteri64v, (GLenum target, GLenum value, int64_t * data), (target, value, data))
_EVASGL_FUNCTION_BEGIN_VOID( glGetBufferPointerv, (GLenum target, GLenum pname, GLvoid ** params), (target, pname, params))
_EVASGL_FUNCTION_BEGIN(GLint, glGetFragDataLocation, (GLuint program, const char * name), (program,  name))
_EVASGL_FUNCTION_BEGIN_VOID( glGetInteger64i_v, (GLenum target, GLuint index, int64_t * data), (target, index, data))
_EVASGL_FUNCTION_BEGIN_VOID( glGetInteger64v, (GLenum pname, int64_t * data), (pname, data))
_EVASGL_FUNCTION_BEGIN_VOID( glGetIntegeri_v, (GLenum target, GLuint index, GLint * data), (target, index, data))
_EVASGL_FUNCTION_BEGIN_VOID( glGetInternalformativ, (GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params), (target, internalformat, pname, bufSize, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetProgramBinary, (GLuint program, GLsizei bufsize, GLsizei *length, GLenum *binaryFormat, void *binary), (program, bufsize, length, binaryFormat, binary))
_EVASGL_FUNCTION_BEGIN_VOID( glGetQueryiv, (GLenum target, GLenum pname, GLint * params), (target, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetQueryObjectuiv, (GLuint id, GLenum pname, GLuint * params), (id, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetSamplerParameterfv, (GLuint sampler, GLenum pname, GLfloat * params), (sampler, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetSamplerParameteriv, (GLuint sampler, GLenum pname, GLint * params), (sampler, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetSynciv, (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values), (sync, pname, bufSize, length, values))
_EVASGL_FUNCTION_BEGIN_VOID( glGetTransformFeedbackVarying, (GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLsizei * size, GLenum * type, char * name), (program, index, bufSize, length, size, type, name))
_EVASGL_FUNCTION_BEGIN(GLuint, glGetUniformBlockIndex, (GLuint program, const GLchar *uniformBlockName), (program, uniformBlockName))
_EVASGL_FUNCTION_BEGIN_VOID( glGetUniformIndices, (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices), (program, uniformCount, uniformNames, uniformIndices))
_EVASGL_FUNCTION_BEGIN_VOID( glGetUniformuiv, (GLuint program, GLint location, GLuint* params), (program, location, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetVertexAttribIiv, (GLuint index, GLenum pname, GLint *params), (index, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetVertexAttribIuiv, (GLuint index, GLenum pname, GLuint *params), (index, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glInvalidateFramebuffer, (GLenum target, GLsizei numAttachments, const GLenum *attachments), (target, numAttachments, attachments))
_EVASGL_FUNCTION_BEGIN_VOID( glInvalidateSubFramebuffer, (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height), (target, numAttachments, attachments, x, y, width, height))
_EVASGL_FUNCTION_BEGIN(GLboolean, glIsQuery, (GLuint id), (id))
_EVASGL_FUNCTION_BEGIN(GLboolean, glIsSampler, (GLuint id), (id))
_EVASGL_FUNCTION_BEGIN(GLboolean, glIsSync, (GLsync sync), (sync))
_EVASGL_FUNCTION_BEGIN(GLboolean, glIsTransformFeedback, (GLuint id), (id))
_EVASGL_FUNCTION_BEGIN(GLboolean, glIsVertexArray, (GLuint array), (array))
_EVASGL_FUNCTION_BEGIN(void *, glMapBufferRange, (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access), (target, offset, length, access))
_EVASGL_FUNCTION_BEGIN_VOID( glPauseTransformFeedback, (void), ())
_EVASGL_FUNCTION_BEGIN_VOID( glProgramBinary, (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length), (program, binaryFormat, binary, length))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramParameteri, (GLuint program, GLenum pname, GLint value), (program, pname, value))
_EVASGL_FUNCTION_BEGIN_VOID( glRenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height), (target, samples, internalformat, width, height))
_EVASGL_FUNCTION_BEGIN_VOID( glResumeTransformFeedback, (void), ())
_EVASGL_FUNCTION_BEGIN_VOID( glSamplerParameterf, (GLuint sampler, GLenum pname, GLfloat param), (sampler, pname, param))
_EVASGL_FUNCTION_BEGIN_VOID( glSamplerParameterfv, (GLuint sampler, GLenum pname, const GLfloat * params), (sampler, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glSamplerParameteri, (GLuint sampler, GLenum pname, GLint param), (sampler, pname, param))
_EVASGL_FUNCTION_BEGIN_VOID( glSamplerParameteriv, (GLuint sampler, GLenum pname, const GLint * params), (sampler, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glTexImage3D, (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * data), (target, level, internalFormat, width, height, depth, border, format, type, data))
_EVASGL_FUNCTION_BEGIN_VOID( glTexStorage2D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height), (target, levels, internalformat, width, height))
_EVASGL_FUNCTION_BEGIN_VOID( glTexStorage3D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth), (target, levels, internalformat, width, height, depth))
_EVASGL_FUNCTION_BEGIN_VOID( glTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * data), (target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data))
_EVASGL_FUNCTION_BEGIN_VOID( glTransformFeedbackVaryings, (GLuint program, GLsizei count, const GLchar *const* varyings, GLenum bufferMode), (program, count, varyings, bufferMode))
_EVASGL_FUNCTION_BEGIN_VOID( glUniform1ui, (GLint location, GLuint v0), (location, v0))
_EVASGL_FUNCTION_BEGIN_VOID( glUniform1uiv, (GLint location, GLsizei count, const GLuint *value), (location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glUniform2ui, (GLint location, GLuint v0, GLuint v1), (location, v0, v1))
_EVASGL_FUNCTION_BEGIN_VOID( glUniform2uiv, (GLint location, GLsizei count, const GLuint *value), (location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glUniform3ui, (GLint location, GLuint v0, GLuint v1, GLuint v2), (location, v0, v1, v2))
_EVASGL_FUNCTION_BEGIN_VOID( glUniform3uiv, (GLint location, GLsizei count, const GLuint *value), (location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glUniform4ui, (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3), (location, v0, v1, v2, v3))
_EVASGL_FUNCTION_BEGIN_VOID( glUniform4uiv, (GLint location, GLsizei count, const GLuint *value), (location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glUniformBlockBinding, (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding), (program, uniformBlockIndex, uniformBlockBinding))
_EVASGL_FUNCTION_BEGIN_VOID( glUniformMatrix2x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glUniformMatrix3x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glUniformMatrix2x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glUniformMatrix4x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glUniformMatrix3x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glUniformMatrix4x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN(GLboolean, glUnmapBuffer, (GLenum target), (target))
_EVASGL_FUNCTION_BEGIN_VOID( glVertexAttribDivisor, (GLuint index, GLuint divisor), (index, divisor))
_EVASGL_FUNCTION_BEGIN_VOID( glVertexAttribI4i, (GLuint index, GLint v0, GLint v1, GLint v2, GLint v3), (index, v0, v1, v2, v3))
_EVASGL_FUNCTION_BEGIN_VOID( glVertexAttribI4iv, (GLuint index, const GLint *v), (index, v))
_EVASGL_FUNCTION_BEGIN_VOID( glVertexAttribI4ui, (GLuint index, GLuint v0, GLuint v1, GLuint v2, GLuint v3), (index, v0, v1, v2, v3))
_EVASGL_FUNCTION_BEGIN_VOID( glVertexAttribI4uiv, (GLuint index, const GLuint *v), (index, v))
_EVASGL_FUNCTION_BEGIN_VOID( glVertexAttribIPointer, (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer), (index, size, type, stride, pointer))
_EVASGL_FUNCTION_BEGIN_VOID( glWaitSync, (GLsync sync, GLbitfield flags, uint64_t timeout), (sync, flags, timeout))


//GLES 3.1
_EVASGL_FUNCTION_BEGIN_VOID( glDispatchCompute, (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z), (num_groups_x, num_groups_y, num_groups_z))
_EVASGL_FUNCTION_BEGIN_VOID( glDispatchComputeIndirect, (GLintptr indirect), (indirect))
_EVASGL_FUNCTION_BEGIN_VOID( glDrawArraysIndirect, (GLenum mode, const void *indirect), (mode, indirect))
_EVASGL_FUNCTION_BEGIN_VOID( glDrawElementsIndirect, (GLenum mode, GLenum type, const void *indirect), (mode, type, indirect))
_EVASGL_FUNCTION_BEGIN_VOID( glGetProgramInterfaceiv, (GLuint program, GLenum programInterface, GLenum pname, GLint *params), (program, programInterface, pname, params))
_EVASGL_FUNCTION_BEGIN(GLuint, glGetProgramResourceIndex, (GLuint program, GLenum programInterface, const GLchar *name), (program, programInterface, name))
_EVASGL_FUNCTION_BEGIN_VOID( glGetProgramResourceName, (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name), (program, programInterface, index, bufSize, length, name))
_EVASGL_FUNCTION_BEGIN_VOID( glGetProgramResourceiv, (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params), (program, programInterface, index, propCount, props, bufSize, length, params))
_EVASGL_FUNCTION_BEGIN(GLint, glGetProgramResourceLocation, (GLuint program, GLenum programInterface, const GLchar *name), (program, programInterface, name))
_EVASGL_FUNCTION_BEGIN_VOID( glUseProgramStages, (GLuint pipeline, GLbitfield stages, GLuint program), (pipeline, stages, program))
_EVASGL_FUNCTION_BEGIN_VOID( glActiveShaderProgram, (GLuint pipeline, GLuint program), (pipeline, program))
_EVASGL_FUNCTION_BEGIN(GLuint, glCreateShaderProgramv, (GLenum type, GLsizei count, const GLchar *const*strings), (type, count, strings))
_EVASGL_FUNCTION_BEGIN_VOID( glBindProgramPipeline, (GLuint pipeline), (pipeline))
_EVASGL_FUNCTION_BEGIN_VOID( glDeleteProgramPipelines, (GLsizei n, const GLuint *pipelines), (n, pipelines))
_EVASGL_FUNCTION_BEGIN_VOID( glGenProgramPipelines, (GLsizei n, GLuint *pipelines), (n, pipelines))
_EVASGL_FUNCTION_BEGIN(GLboolean, glIsProgramPipeline, (GLuint pipeline), (pipeline))
_EVASGL_FUNCTION_BEGIN_VOID( glGetProgramPipelineiv, (GLuint pipeline, GLenum pname, GLint *params), (pipeline, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform1i, (GLuint program, GLint location, GLint v0), (program, location, v0))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform2i, (GLuint program, GLint location, GLint v0, GLint v1), (program, location, v0, v1))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform3i, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2), (program, location, v0, v1, v2))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform4i, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3), (program, location, v0, v1, v2, v3))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform1ui, (GLuint program, GLint location, GLuint v0), (program, location, v0))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform2ui, (GLuint program, GLint location, GLuint v0, GLuint v1), (program, location, v0, v1))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform3ui, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2), (program, location, v0, v1, v2))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform4ui, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3), (program, location, v0, v1, v2, v3))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform1f, (GLuint program, GLint location, GLfloat v0), (program, location, v0))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform2f, (GLuint program, GLint location, GLfloat v0, GLfloat v1) ,(program, location, v0, v1))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform3f, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2), (program, location, v0, v1, v2))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform4f, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3), (program, location, v0, v1, v2, v3))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform1iv, (GLuint program, GLint location, GLsizei count, const GLint *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform2iv, (GLuint program, GLint location, GLsizei count, const GLint *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform3iv, (GLuint program, GLint location, GLsizei count, const GLint *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform4iv, (GLuint program, GLint location, GLsizei count, const GLint *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform1uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform2uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform3uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform4uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform1fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform2fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform3fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniform4fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value), (program, location, count, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniformMatrix2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (program, location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniformMatrix3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (program, location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniformMatrix4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (program, location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniformMatrix2x3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (program, location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniformMatrix3x2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (program, location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniformMatrix2x4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (program, location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniformMatrix4x2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (program, location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniformMatrix3x4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (program, location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glProgramUniformMatrix4x3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value), (program, location, count, transpose, value))
_EVASGL_FUNCTION_BEGIN_VOID( glValidateProgramPipeline, (GLuint pipeline), (pipeline))
_EVASGL_FUNCTION_BEGIN_VOID( glGetProgramPipelineInfoLog, (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog), (pipeline, bufSize, length, infoLog))
_EVASGL_FUNCTION_BEGIN_VOID( glBindImageTexture, (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format), (unit, texture, level, layered, layer, access, format))
_EVASGL_FUNCTION_BEGIN_VOID( glGetBooleani_v, (GLenum target, GLuint index, GLboolean *data), (target, index, data))
_EVASGL_FUNCTION_BEGIN_VOID( glMemoryBarrier, (GLbitfield barriers), (barriers))
_EVASGL_FUNCTION_BEGIN_VOID( glMemoryBarrierByRegion, (GLbitfield barriers), (barriers))
_EVASGL_FUNCTION_BEGIN_VOID( glTexStorage2DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations), (target, samples, internalformat, width, height, fixedsamplelocations))
_EVASGL_FUNCTION_BEGIN_VOID( glGetMultisamplefv, (GLenum pname, GLuint index, GLfloat *val), (pname, index, val))
_EVASGL_FUNCTION_BEGIN_VOID( glSampleMaski, (GLuint maskNumber, GLbitfield mask), (maskNumber, mask))
_EVASGL_FUNCTION_BEGIN_VOID( glGetTexLevelParameteriv, (GLenum target, GLint level, GLenum pname, GLint *params), (target, level, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetTexLevelParameterfv, (GLenum target, GLint level, GLenum pname, GLfloat *params), (target, level, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glBindVertexBuffer, (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride), (bindingindex, buffer, offset, stride))
_EVASGL_FUNCTION_BEGIN_VOID( glVertexAttribFormat, (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset), (attribindex, size, type, normalized, relativeoffset))
_EVASGL_FUNCTION_BEGIN_VOID( glVertexAttribIFormat, (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset), (attribindex, size, type, relativeoffset))
_EVASGL_FUNCTION_BEGIN_VOID( glVertexAttribBinding, (GLuint attribindex, GLuint bindingindex), (attribindex, bindingindex))
_EVASGL_FUNCTION_BEGIN_VOID( glVertexBindingDivisor, (GLuint bindingindex, GLuint divisor), (bindingindex, divisor))

//GLES 3.2
_EVASGL_FUNCTION_BEGIN_VOID( glBlendBarrier, (void), ())
_EVASGL_FUNCTION_BEGIN_VOID( glCopyImageSubData, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth), (srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth))
_EVASGL_FUNCTION_BEGIN_VOID( glDebugMessageControl, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled), (source, type, severity, count, ids, enabled))
_EVASGL_FUNCTION_BEGIN_VOID( glDebugMessageInsert, (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf), (source, type, id, severity, length, buf))
_EVASGL_FUNCTION_BEGIN_VOID( glDebugMessageCallback, (void (*callback)(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam), const void *userParam), (callback, userParam))
_EVASGL_FUNCTION_BEGIN (GLuint, glGetDebugMessageLog, (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog), (count, bufSize, sources, types, ids, severities, lengths, messageLog))
_EVASGL_FUNCTION_BEGIN_VOID( glPushDebugGroup, (GLenum source, GLuint id, GLsizei length, const GLchar *message),(source, id, length, message))
_EVASGL_FUNCTION_BEGIN_VOID( glPopDebugGroup, (void), ())
_EVASGL_FUNCTION_BEGIN_VOID( glObjectLabel, (GLenum identifier, GLuint name, GLsizei length, const GLchar *label),(identifier, name, length, label))
_EVASGL_FUNCTION_BEGIN_VOID( glGetObjectLabel, (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label), (identifier, name, bufSize, length, label))
_EVASGL_FUNCTION_BEGIN_VOID( glObjectPtrLabel, (const void *ptr, GLsizei length, const GLchar *label), (ptr, length, label))
_EVASGL_FUNCTION_BEGIN_VOID( glGetObjectPtrLabel, (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label), (ptr, bufSize, length, label))
_EVASGL_FUNCTION_BEGIN_VOID( glGetPointerv, (GLenum pname, void **params), (pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glEnablei, (GLenum target, GLuint index), (target, index))
_EVASGL_FUNCTION_BEGIN_VOID( glDisablei, (GLenum target, GLuint index), (target, index))
_EVASGL_FUNCTION_BEGIN_VOID( glBlendEquationi, (GLuint buf, GLenum mode), (buf, mode))
_EVASGL_FUNCTION_BEGIN_VOID( glBlendEquationSeparatei, (GLuint buf, GLenum modeRGB, GLenum modeAlpha), (buf, modeRGB, modeAlpha))
_EVASGL_FUNCTION_BEGIN_VOID( glBlendFunci, (GLuint buf, GLenum src, GLenum dst), (buf, src, dst))
_EVASGL_FUNCTION_BEGIN_VOID( glBlendFuncSeparatei, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha), (buf, srcRGB, dstRGB, srcAlpha, dstAlpha))
_EVASGL_FUNCTION_BEGIN_VOID( glColorMaski, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a), (index, r, g, b, a))
_EVASGL_FUNCTION_BEGIN(GLboolean, glIsEnabledi, (GLenum target, GLuint index), (target, index))
_EVASGL_FUNCTION_BEGIN_VOID( glDrawElementsBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex), (mode, count, type, indices, basevertex))
_EVASGL_FUNCTION_BEGIN_VOID( glDrawRangeElementsBaseVertex, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex), (mode, start, end, count, type, indices, basevertex))
_EVASGL_FUNCTION_BEGIN_VOID( glDrawElementsInstancedBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex), (mode, count, type, indices, instancecount, basevertex))
_EVASGL_FUNCTION_BEGIN_VOID( glPrimitiveBoundingBox, (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW), (minX, minY, minZ, minW, maxX, maxY, maxZ, maxW))
_EVASGL_FUNCTION_BEGIN(GLenum, glGetGraphicsResetStatus, (void), ())
_EVASGL_FUNCTION_BEGIN_VOID( glReadnPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data), (x, y, width, height, format, type, bufSize, data))
_EVASGL_FUNCTION_BEGIN_VOID( glGetnUniformfv, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params), (program, location, bufSize, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetnUniformiv, (GLuint program, GLint location, GLsizei bufSize, GLint *params), (program, location, bufSize, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetnUniformuiv, (GLuint program, GLint location, GLsizei bufSize, GLuint *params), (program, location, bufSize, params))
_EVASGL_FUNCTION_BEGIN_VOID( glMinSampleShading, (GLfloat value), (value))
_EVASGL_FUNCTION_BEGIN_VOID( glPatchParameteri, (GLenum pname, GLint value), (pname, value))
_EVASGL_FUNCTION_BEGIN_VOID( glTexParameterIiv, (GLenum target, GLenum pname, const GLint *params), (target, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glTexParameterIuiv, (GLenum target, GLenum pname, const GLuint *params), (target, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetTexParameterIiv, (GLenum target, GLenum pname, GLint *params), (target, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetTexParameterIuiv, (GLenum target, GLenum pname, GLuint *params), (target, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glSamplerParameterIiv, (GLuint sampler, GLenum pname, const GLint *param), (sampler, pname, param))
_EVASGL_FUNCTION_BEGIN_VOID( glSamplerParameterIuiv, (GLuint sampler, GLenum pname, const GLuint *param), (sampler, pname, param))
_EVASGL_FUNCTION_BEGIN_VOID( glGetSamplerParameterIiv, (GLuint sampler, GLenum pname, GLint *params), (sampler, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glGetSamplerParameterIuiv, (GLuint sampler, GLenum pname, GLuint *params), (sampler, pname, params))
_EVASGL_FUNCTION_BEGIN_VOID( glTexBuffer, (GLenum target, GLenum internalformat, GLuint buffer), (target, internalformat, buffer))
_EVASGL_FUNCTION_BEGIN_VOID( glTexBufferRange, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size), (target, internalformat, buffer, offset, size))
_EVASGL_FUNCTION_BEGIN_VOID( glTexStorage3DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations), (target, samples, internalformat, width, height, depth, fixedsamplelocations))
