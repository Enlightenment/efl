
//////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _EVASGL_EXT_BEGIN

#define _EVASGL_EXT_USE_DEFAULT_DEFINE

#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()

// Begin of the extension block (name : EVAS extension name)
#define _EVASGL_EXT_BEGIN(name)
// End of the extension block
#define _EVASGL_EXT_END()

// Driver extensions to wrap (name : SPEC extension name)
#define _EVASGL_EXT_DRVNAME(name)

// These functions will be exported to 'EVAS extension function'.
// The functions of this block must correspond with the functions list in Evas_GL.h.
// Begin of the extension function block (ret : return value, name : function name, param : parameters with bracket)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) ret (*name) param;
// End of the extension function block
#define _EVASGL_EXT_FUNCTION_END()

// These functions will not be exported. Only be used in engines privately.
// Begin of the extension function block (ret : return value, name : function name, param : parameters with bracket)
#define _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(ret, name, param)
// End of the extension function block
#define _EVASGL_EXT_FUNCTION_PRIVATE_END()

// Driver extension functions to wrap (name : SPEC extension function name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)

#endif
//////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN
#define _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(ret, name, param) _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN_DEFINED
#endif

#ifndef _EVASGL_EXT_FUNCTION_PRIVATE_END
#define _EVASGL_EXT_FUNCTION_PRIVATE_END() _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_PRIVATE_END_DEFINED
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GL/GLES EXTENSIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
_EVASGL_EXT_BEGIN(get_program_binary)
	_EVASGL_EXT_DRVNAME(GL_OES_get_program_binary)

	_EVASGL_EXT_FUNCTION_BEGIN(void, glGetProgramBinaryOES, (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetProgramBinary"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetProgramBinaryOES"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glProgramBinaryOES, (GLuint program, GLenum binaryFormat, const void *binary, GLint length))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glProgramBinary"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glProgramBinaryOES"))
	_EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(mapbuffer)
	_EVASGL_EXT_DRVNAME(GL_OES_mapbuffer)

	_EVASGL_EXT_FUNCTION_BEGIN(void *, glMapBufferOES, (GLenum target, GLenum access))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMapBuffer"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMapBufferOES"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(GLboolean, glUnmapBufferOES, (GLenum target))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glUnmapBuffer"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glUnmapBufferOES"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glGetBufferPointervOES, (GLenum target, GLenum pname, void** params))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetBufferPointerv"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetBufferPointervOES"))
	_EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_3D)
	_EVASGL_EXT_DRVNAME(GL_OES_texture_3D)

	_EVASGL_EXT_FUNCTION_BEGIN(void, glTexImage3DOES, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexImage3D"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexImage3DOES"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glTexSubImage3DOES, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexSubImage3D"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexSubImage3DOES"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glCopyTexSubImage3DOES, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCopyTexSubImage3D"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCopyTexSubImage3DOES"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glCompressedTexImage3DOES, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCompressedTexImage3D"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCompressedTexImage3DOES"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glCompressedTexSubImage3DOES, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCompressedTexSubImage3D"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCompressedTexSubImage3DOES"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glFramebufferTexture3DOES, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFramebufferTexture3D"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFramebufferTexture3DOES"))
	_EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(AMD_performance_monitor)
	_EVASGL_EXT_DRVNAME(AMD_performance_monitor)

	_EVASGL_EXT_FUNCTION_BEGIN(void, glGetPerfMonitorGroupsAMD, (GLint* numGroups, GLsizei groupsSize, GLuint* groups))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetPerfMonitorGroupsAMD"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glGetPerfMonitorCountersAMD, (GLuint group, GLint* numCounters, GLint* maxActiveCounters, GLsizei counterSize, GLuint* counters))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetPerfMonitorCountersAMD"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glGetPerfMonitorGroupStringAMD, (GLuint group, GLsizei bufSize, GLsizei* length, char* groupString))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetPerfMonitorGroupStringAMD"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glGetPerfMonitorCounterStringAMD, (GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, char* counterString))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetPerfMonitorCounterStringAMD"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glGetPerfMonitorCounterInfoAMD, (GLuint group, GLuint counter, GLenum pname, void* data))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetPerfMonitorCounterInfoAMD"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glGenPerfMonitorsAMD, (GLsizei n, GLuint* monitors))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGenPerfMonitorsAMD"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glDeletePerfMonitorsAMD, (GLsizei n, GLuint* monitors))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDeletePerfMonitorsAMD"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glSelectPerfMonitorCountersAMD, (GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* countersList))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glSelectPerfMonitorCountersAMD"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glBeginPerfMonitorAMD, (GLuint monitor))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBeginPerfMonitorAMD"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glEndPerfMonitorAMD, (GLuint monitor))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glEndPerfMonitorAMD"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glGetPerfMonitorCounterDataAMD, (GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint* bytesWritten))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetPerfMonitorCounterDataAMD"))
	_EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(discard_framebuffer)
	_EVASGL_EXT_DRVNAME(GL_EXT_discard_framebuffer)

	_EVASGL_EXT_FUNCTION_BEGIN(void, glDiscardFramebufferEXT, (GLenum target, GLsizei numAttachments, const GLenum* attachments))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDiscardFramebuffer"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDiscardFramebufferEXT"))
	_EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(multi_draw_arrays)
	_EVASGL_EXT_DRVNAME(GL_EXT_multi_draw_arrays)

	_EVASGL_EXT_FUNCTION_BEGIN(void, glMultiDrawArraysEXT, (GLenum mode, GLint* first, GLsizei* count, GLsizei primcount))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMultiDrawArrays"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMultiDrawArraysEXT"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glMultiDrawElementsEXT, (GLenum mode, const GLsizei* count, GLenum type, const GLvoid** indices, GLsizei primcount))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMultiDrawElements"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMultiDrawElementsEXT"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMultiDrawElementsARB"))
	_EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(NV_fence)
	_EVASGL_EXT_DRVNAME(GL_NV_fence)

	_EVASGL_EXT_FUNCTION_BEGIN(void, glDeleteFencesNV, (GLsizei n, const GLuint* fences))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDeleteFencesNV"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glGenFencesNV, (GLsizei n, GLuint* fences))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGenFencesNV"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(unsigned char, glIsFenceNV, (GLuint fence))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glIsFenceNV"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(unsigned char, glTestFenceNV, (GLuint fence))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTestFenceNV"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glGetFenceivNV, (GLuint fence, GLenum pname, GLint* params))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetFenceivNV"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glFinishFenceNV, (GLuint fence))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFinishFenceNV"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glSetFenceNV, (GLuint, GLenum))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glSetFenceNV"))
	_EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(QCOM_driver_control)
	_EVASGL_EXT_DRVNAME(GL_QCOM_driver_control)

	_EVASGL_EXT_FUNCTION_BEGIN(void, glGetDriverControlsQCOM, (GLint* num, GLsizei size, GLuint* driverControls))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetDriverControlsQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glGetDriverControlStringQCOM, (GLuint driverControl, GLsizei bufSize, GLsizei *length, GLchar *driverControlString))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetDriverControlStringQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glEnableDriverControlQCOM, (GLuint driverControl))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glEnableDriverControlQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glDisableDriverControlQCOM, (GLuint driverControl))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDisableDriverControlQCOM"))
	_EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(QCOM_extended_get)
	_EVASGL_EXT_DRVNAME(GL_QCOM_extended_get)

	_EVASGL_EXT_FUNCTION_BEGIN(void, glExtGetTexturesQCOM, (GLuint* textures, GLint maxTextures, GLint* numTextures))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtGetTexturesQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glExtGetBuffersQCOM, (GLuint* buffers, GLint maxBuffers, GLint* numBuffers))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtGetBuffersQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glExtGetRenderbuffersQCOM, (GLuint* renderbuffers, GLint maxRenderbuffers, GLint* numRenderbuffers))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtGetRenderbuffersQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glExtGetFramebuffersQCOM, (GLuint* framebuffers, GLint maxFramebuffers, GLint* numFramebuffers))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtGetFramebuffersQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glExtGetTexLevelParameterivQCOM, (GLuint texture, GLenum face, GLint level, GLenum pname, GLint* params))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtGetTexLevelParameterivQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glExtTexObjectStateOverrideiQCOM, (GLenum target, GLenum pname, GLint param))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtTexObjectStateOverrideiQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glExtGetTexSubImageQCOM, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, void* texels))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtGetTexSubImageQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glExtGetBufferPointervQCOM, (GLenum target, void** params))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtGetBufferPointervQCOM"))
	_EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(QCOM_extended_get2)
	_EVASGL_EXT_DRVNAME(GL_QCOM_extended_get2)

	_EVASGL_EXT_FUNCTION_BEGIN(void, glExtGetShadersQCOM, (GLuint* shaders, GLint maxShaders, GLint* numShaders))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtGetShadersQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glExtGetProgramsQCOM, (GLuint* programs, GLint maxPrograms, GLint* numPrograms))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtGetProgramsQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(unsigned char, glExtIsProgramBinaryQCOM, (GLuint program))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtIsProgramBinaryQCOM"))
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glExtGetProgramBinarySourceQCOM, (GLuint program, GLenum shadertype, char* source, GLint* length))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glExtGetProgramBinarySourceQCOM"))
	_EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(IMG_multisampled_render_to_texture)
	_EVASGL_EXT_DRVNAME(GL_IMG_multisampled_render_to_texture)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(EXT_multisampled_render_to_texture)
	_EVASGL_EXT_DRVNAME(GL_EXT_multisampled_render_to_texture)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(multisampled_render_to_texture)
	_EVASGL_EXT_DRVNAME(GL_IMG_multisampled_render_to_texture)
	_EVASGL_EXT_DRVNAME(GL_EXT_multisampled_render_to_texture)

	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(void, glRenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glRenderbufferStorageMultisample"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glRenderbufferStorageMultisampleIMG"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glRenderbufferStorageMultisampleEXT"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(void, glFramebufferTexture2DMultisample, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFramebufferTexture2DMultisample"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFramebufferTexture2DMultisampleIMG"))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFramebufferTexture2DMultisampleEXT"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(compressed_ETC1_RGB8_texture)
	_EVASGL_EXT_DRVNAME(GL_OES_compressed_ETC1_RGB8_texture)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(compressed_paletted_texture)
	_EVASGL_EXT_DRVNAME(GL_OES_compressed_paletted_texture)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(depth24)
	_EVASGL_EXT_DRVNAME(GL_OES_depth24)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(depth32)
	_EVASGL_EXT_DRVNAME(GL_OES_depth32)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(EGL_image)
	_EVASGL_EXT_DRVNAME(GL_OES_EvasGL_image)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(packed_depth_stencil)
	_EVASGL_EXT_DRVNAME(GL_OES_packed_depth_stencil)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(rgb8_rgba8)
	_EVASGL_EXT_DRVNAME(GL_OES_rgb8_rgba8)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(standard_derivatives)
	_EVASGL_EXT_DRVNAME(GL_OES_standard_derivatives)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(stencil1)
	_EVASGL_EXT_DRVNAME(GL_OES_stencil1)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(stencil4)
	_EVASGL_EXT_DRVNAME(GL_OES_stencil4)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_float)
	_EVASGL_EXT_DRVNAME(GL_OES_texture_float)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_half_float)
	_EVASGL_EXT_DRVNAME(GL_OES_texture_half_float)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_half_float_linear)
	_EVASGL_EXT_DRVNAME(GL_OES_texture_half_float_linear)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_npot)     // Desktop differs
	_EVASGL_EXT_DRVNAME(GL_OES_texture_npot)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_npot_DESKTOP)     // Desktop differs
	_EVASGL_EXT_DRVNAME(GL_OES_texture_npot)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(vertex_half_float)     // Desktop differs
	_EVASGL_EXT_DRVNAME(GL_OES_vertex_half_float)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(vertex_half_float_DESKTOP)     // Desktop differs
	_EVASGL_EXT_DRVNAME(GL_OES_vertex_half_float)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(vertex_type_10_10_10_2)
	_EVASGL_EXT_DRVNAME(GL_OES_vertex_type_10_10_10_2)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(compressed_3DC_texture)
	_EVASGL_EXT_DRVNAME(GL_AMD_compressed_3DC_texture)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(compressed_ATC_texture)
	_EVASGL_EXT_DRVNAME(GL_AMD_compressed_ATC_texture)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(program_binary_Z400)
	_EVASGL_EXT_DRVNAME(GL_AMD_program_binary_Z400)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(blend_minmax)
	_EVASGL_EXT_DRVNAME(GL_EXT_blend_minmax)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(read_format_bgra) // Desktop differs
	_EVASGL_EXT_DRVNAME(GL_EXT_read_format_bgra)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(read_format_bgra_DESKTOP) // Desktop differs
	_EVASGL_EXT_DRVNAME(GL_EXT_read_format_bgra)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_filter_anisotrophic)
	_EVASGL_EXT_DRVNAME(GL_EXT_texture_filter_anisotropic)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_format_BGRA8888) // Desktop differs
	_EVASGL_EXT_DRVNAME(GL_EXT_texture_format_BGRA8888)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_format_BGRA8888_DESKTOP) // Desktop differs
	_EVASGL_EXT_DRVNAME(GL_EXT_texture_format_BGRA8888)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_type_2_10_10_10_rev) // Desktop differs
	_EVASGL_EXT_DRVNAME(GL_EXT_texture_type_2_10_10_10_REV)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_type_2_10_10_10_rev_DESKTOP) // Desktop differs
	_EVASGL_EXT_DRVNAME(GL_EXT_texture_type_2_10_10_10_REV)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(IMG_program_binary)
	_EVASGL_EXT_DRVNAME(GL_IMG_program_binary)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(IMG_read_format)
	_EVASGL_EXT_DRVNAME(GL_IMG_read_format)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(IMG_shader_binary)
	_EVASGL_EXT_DRVNAME(GL_IMG_shader_binary)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(IMG_texture_compression_pvrtc)
	_EVASGL_EXT_DRVNAME(GL_IMG_texture_compression_pvrtc)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(QCOM_perfmon_global_mode)
	_EVASGL_EXT_DRVNAME(GL_QCOM_perfmon_global_mode)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(QCOM_writeonly_rendering)
	_EVASGL_EXT_DRVNAME(GL_QCOM_writeonly_rendering)
_EVASGL_EXT_END()
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// EGL EXTENSIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef GL_GLES

_EVASGL_EXT_BEGIN(EGL_KHR_image_base)

	_EVASGL_EXT_DRVNAME(EGL_KHR_image_base)

	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(void *, eglCreateImage, (EGLDisplay a, EGLContext b, EGLenum c, EGLClientBuffer d, const int *e))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("eglCreateImageKHR"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(void, eglDestroyImage, (EGLDisplay a, void *b))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("eglDestroyImageKHR"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()

	_EVASGL_EXT_FUNCTION_BEGIN(EvasGLImage, evasglCreateImage, (int target, void* buffer, int *attrib_list))
		_EVASGL_EXT_FUNCTION_DRVFUNC(evgl_evasglCreateImage)
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, evasglDestroyImage, (EvasGLImage image))
		_EVASGL_EXT_FUNCTION_DRVFUNC(evgl_evasglDestroyImage)
	_EVASGL_EXT_FUNCTION_END()

	#ifdef _EVASGL_EXT_VERIFY
	{
		// Add special function pointers
		evgl_evasglCreateImage_ptr = GETPROCADDR("eglCreateImageKHR");
		evgl_evasglDestroyImage_ptr = GETPROCADDR("eglDestroyImageKHR");
	}
	#endif

_EVASGL_EXT_END()



_EVASGL_EXT_BEGIN(GL_OES_EGL_image)

	_EVASGL_EXT_DRVNAME(GL_OES_EGL_image)

	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(void, glEGLImageTargetTexture2DOES, (GLenum target, GLeglImageOES image))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glEGLImageTargetTexture2DOES"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(void, glEGLImageTargetRenderbufferStorageOES, (GLenum target, GLeglImageOES image))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glEGLImageTargetRenderbufferStorageOES"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()

	_EVASGL_EXT_FUNCTION_BEGIN(void, glEvasGLImageTargetTexture2DOES, (GLenum target, EvasGLImage image))
		_EVASGL_EXT_FUNCTION_DRVFUNC(evgl_glEvasGLImageTargetTexture2D)
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, glEvasGLImageTargetRenderbufferStorageOES, (GLenum target, EvasGLImage image))
		_EVASGL_EXT_FUNCTION_DRVFUNC(evgl_glEvasGLImageTargetRenderbufferStorage)
	_EVASGL_EXT_FUNCTION_END()

	#ifdef _EVASGL_EXT_VERIFY
	{
		if (!_EVASGL_EXT_CHECK_SUPPORT("EGL_KHR_image_base")) _EVASGL_EXT_DISCARD_SUPPORT();
	}
	#endif

_EVASGL_EXT_END()



_EVASGL_EXT_BEGIN(EGL_KHR_image_pixmap)
	_EVASGL_EXT_DRVNAME(EGL_KHR_image_pixmap)
	#ifdef _EVASGL_EXT_VERIFY
	{
		if (!_EVASGL_EXT_CHECK_SUPPORT("EGL_KHR_image_base")) _EVASGL_EXT_DISCARD_SUPPORT();
	}
	#endif
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(EGL_KHR_image)
	_EVASGL_EXT_DRVNAME(EGL_KHR_image)
	#ifdef _EVASGL_EXT_VERIFY
	{
		if (!_EVASGL_EXT_CHECK_SUPPORT("EGL_KHR_image_base")) _EVASGL_EXT_DISCARD_SUPPORT();
	}
	#endif
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(EGL_KHR_vg_parent_image)
	_EVASGL_EXT_DRVNAME(EvasGL_KHR_vg_parent_image)
	#ifdef _EVASGL_EXT_VERIFY
	{
		if (!_EVASGL_EXT_CHECK_SUPPORT("EGL_KHR_image_base")) _EVASGL_EXT_DISCARD_SUPPORT();
	}
	#endif
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(EGL_KHR_gl_texture_2D_image)
	_EVASGL_EXT_DRVNAME(EGL_KHR_gl_texture_2D_image)
	#ifdef _EVASGL_EXT_VERIFY
	{
		if (!_EVASGL_EXT_CHECK_SUPPORT("EGL_KHR_image_base")) _EVASGL_EXT_DISCARD_SUPPORT();
	}
	#endif
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(EGL_KHR_gl_texture_cubemap_image)
	_EVASGL_EXT_DRVNAME(EGL_KHR_gl_texture_cubemap_image)
	#ifdef _EVASGL_EXT_VERIFY
	{
		if (!_EVASGL_EXT_CHECK_SUPPORT("EGL_KHR_image_base")) _EVASGL_EXT_DISCARD_SUPPORT();
	}
	#endif
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(EGL_KHR_gl_texture_3D_image)
	_EVASGL_EXT_DRVNAME(EGL_KHR_gl_texture_3D_image)
	#ifdef _EVASGL_EXT_VERIFY
	{
		if (!_EVASGL_EXT_CHECK_SUPPORT("EGL_KHR_image_base")) _EVASGL_EXT_DISCARD_SUPPORT();
	}
	#endif
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(EGL_KHR_gl_renderbuffer_image)
	_EVASGL_EXT_DRVNAME(EGL_KHR_gl_renderbuffer_image)
	#ifdef _EVASGL_EXT_VERIFY
	{
		if (!_EVASGL_EXT_CHECK_SUPPORT("EGL_KHR_image_base")) _EVASGL_EXT_DISCARD_SUPPORT();
	}
	#endif
_EVASGL_EXT_END()

#if 0
_EVASGL_EXT_BEGIN(EGL_SEC_map_image)
	_EVASGL_EXT_DRVNAME(EGL_SEC_map_image)

	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(void *, eglMapImageSEC, void *a, void *b, int c, int d))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("eglMapImageSEC"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(unsigned int, eglUnmapImageSEC, void *a, void *b, int c))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("eglUnmapImageSEC"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
_EVASGL_EXT_END()
#endif

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GLX EXTENSIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
#ifdef GL_GLES
#else

_EVASGL_EXT_BEGIN(GLX_EXT_swap_control)
	_EVASGL_EXT_DRVNAME(GLX_EXT_swap_control)

	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(void, glXSwapIntervalEXT, (Display *dpy, GLXDrawable drawable, int interval))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glXSwapIntervalEXT"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(GLX_SGI_swap_control)
	_EVASGL_EXT_DRVNAME(GLX_SGI_swap_control)

	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(int, glXSwapIntervalSGI, (int interval))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glXSwapIntervalSGI"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(GLX_SGI_video_sync)
	_EVASGL_EXT_DRVNAME(GLX_SGI_video_sync)

	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(int, glXGetVideoSyncSGI, (uint *count))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glXGetVideoSyncSGI"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(int, glXWaitVideoSyncSGI, (int divisor, int remainder, unsigned int *count))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glXWaitVideoSyncSGI"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()

_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(GLX_EXT_texture_from_pixmap)
	_EVASGL_EXT_DRVNAME(GLX_EXT_texture_from_pixmap)

	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(void, glXBindTexImageEXT, (Display *display, GLXDrawable drawable, int buffer, const int *attrib_list))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glXBindTexImageEXT"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(void, glXReleaseTexImageEXT, (Display *display, GLXDrawable drawable, int buffer))
		_EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glXReleaseTexImageEXT"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()

_EVASGL_EXT_END()

#endif
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifdef _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN_DEFINED
#undef _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN_DEFINED
#endif

#ifdef _EVASGL_EXT_FUNCTION_PRIVATE_END_DEFINED
#undef _EVASGL_EXT_FUNCTION_PRIVATE_END
#undef _EVASGL_EXT_FUNCTION_PRIVATE_END_DEFINED
#endif

#ifdef _EVASGL_EXT_USE_DEFAULT_DEFINE
#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_PRIVATE_BEGIN
#undef _EVASGL_EXT_FUNCTION_PRIVATE_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#endif


