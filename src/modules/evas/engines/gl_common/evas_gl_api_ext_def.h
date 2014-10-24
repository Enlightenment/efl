
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

        /* GLES 1.x extension */
        _EVASGL_EXT_FUNCTION_BEGIN(void, glRenderbufferStorageMultisampleEXT, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glRenderbufferStorageMultisample"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glRenderbufferStorageMultisampleEXT"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glFramebufferTexture2DMultisampleEXT, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFramebufferTexture2DMultisample"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFramebufferTexture2DMultisampleEXT"))
        _EVASGL_EXT_FUNCTION_END()
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

_EVASGL_EXT_BEGIN(texture_filter_anisotropic)
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
// GLES 1.1 ONLY EXTENSIONS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

_EVASGL_EXT_BEGIN(blend_equation_separate)
        _EVASGL_EXT_DRVNAME(GL_OES_blend_equation_separate)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glBlendEquationSeparateOES, (GLenum modeRGB, GLenum modeAlpha))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBlendEquationSeparate"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBlendEquationSeparateOES"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(blend_func_separate)
        _EVASGL_EXT_DRVNAME(GL_OES_blend_func_separate)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glBlendFuncSeparateOES, (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBlendFuncSeparate"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBlendFuncSeparateOES"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(blend_subtract)
        _EVASGL_EXT_DRVNAME(GL_OES_blend_subtract)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glBlendEquationOES, (GLenum mode))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBlendEquation"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBlendEquationOES"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(byte_coordinates)
        _EVASGL_EXT_DRVNAME(GL_OES_byte_coordinates)
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(draw_texture)
        _EVASGL_EXT_DRVNAME(GL_OES_draw_texture)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glDrawTexsOES, (GLshort x, GLshort y, GLshort z, GLshort width, GLshort height))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexs"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexsOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDrawTexiOES, (GLint x, GLint y, GLint z, GLint width, GLint height))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexi"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexiOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDrawTexxOES, (GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDrawTexsvOES, (const GLshort *coords))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexsv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexsvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDrawTexivOES, (const GLint *coords))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexiv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexivOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDrawTexxvOES, (const GLfixed *coords))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDrawTexfOES, (GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexf"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexfOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDrawTexfvOES, (const GLfloat *coords))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexfv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDrawTexfvOES"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(extended_matrix_palette)
        _EVASGL_EXT_DRVNAME(GL_OES_extended_matrix_palette)
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(fixed_point)
        _EVASGL_EXT_DRVNAME(GL_OES_fixed_point)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glAlphaFuncxOES, (GLenum func, GLclampx ref))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glAlphaFuncx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glAlphaFuncxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glClearColorxOES, (GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClearColorx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClearColorxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glClearDepthxOES, (GLclampx depth))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClearDepthx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClearDepthxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glClipPlanexOES, (GLenum plane, const GLfixed *equation))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClipPlanex"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClipPlanexOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glColor4xOES, (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glColor4x"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glColor4xOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDepthRangexOES, (GLclampx zNear, GLclampx zFar))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDepthRangex"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDepthRangexOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glFogxOES, (GLenum pname, GLfixed param))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFogx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFogxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glFogxvOES, (GLenum pname, const GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFogxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFogxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glFrustumxOES, (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFrustumx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFrustumxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetClipPlanexOES, (GLenum pname, GLfixed eqn[4]))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetClipPlanex"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetClipPlanexOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetFixedvOES, (GLenum pname, GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetFixedv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetFixedvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetLightxvOES, (GLenum light, GLenum pname, GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetLightxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetLightxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetMaterialxvOES, (GLenum face, GLenum pname, GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetMaterialxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetMaterialxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetTexEnvxvOES, (GLenum env, GLenum pname, GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetTexEnvxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetTexEnvxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetTexParameterxvOES, (GLenum target, GLenum pname, GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetTexParameterxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetTexParameterxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glLightModelxOES, (GLenum pname, GLfixed param))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLightModelx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLightModelxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glLightModelxvOES, (GLenum pname, const GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLightModelxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLightModelxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glLightxOES, (GLenum light, GLenum pname, GLfixed param))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLightx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLightxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glLightxvOES, (GLenum light, GLenum pname, const GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLightxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLightxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glLineWidthxOES, (GLfixed width))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLineWidthx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLineWidthxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glLoadMatrixxOES, (const GLfixed *m))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLoadMatrixx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLoadMatrixxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glMaterialxOES, (GLenum face, GLenum pname, GLfixed param))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMaterialx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMaterialxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glMaterialxvOES, (GLenum face, GLenum pname, const GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMaterialxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMaterialxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glMultMatrixxOES, (const GLfixed *m))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMultMatrixx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMultMatrixxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glMultiTexCoord4xOES, (GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMultiTexCoord4x"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMultiTexCoord4xOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glNormal3xOES, (GLfixed nx, GLfixed ny, GLfixed nz))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glNormal3x"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glNormal3xOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glOrthoxOES, (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glOrthox"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glOrthoxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glPointParameterxOES, (GLenum pname, GLfixed param))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glPointParameterx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glPointParameterxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glPointParameterxvOES, (GLenum pname, const GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glPointParameterxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glPointParameterxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glPointSizexOES, (GLfixed size))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glPointSizex"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glPointSizexOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glPolygonOffsetxOES, (GLfixed factor, GLfixed units))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glPolygonOffsetx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glPolygonOffsetxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glRotatexOES, (GLfixed angle, GLfixed x, GLfixed y, GLfixed z))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glRotatex"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glRotatexOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glSampleCoveragexOES, (GLclampx value, GLboolean invert))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glSampleCoveragex"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glSampleCoveragexOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glScalexOES, (GLfixed x, GLfixed y, GLfixed z))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glScalex"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glScalexOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexEnvxOES, (GLenum target, GLenum pname, GLfixed param))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexEnvx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexEnvxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexEnvxvOES, (GLenum target, GLenum pname, const GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexEnvxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexEnvxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexParameterxOES, (GLenum target, GLenum pname, GLfixed param))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexParameterx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexParameterxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexParameterxvOES, (GLenum target, GLenum pname, const GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexParameterxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexParameterxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTranslatexOES, (GLfixed x, GLfixed y, GLfixed z))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTranslatex"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTranslatexOES"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(framebuffer_object)
        _EVASGL_EXT_DRVNAME(GL_OES_framebuffer_object)

        _EVASGL_EXT_FUNCTION_BEGIN(GLboolean, glIsRenderbufferOES, (GLuint renderbuffer))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glIsRenderbuffer"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glIsRenderbufferOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glBindRenderbufferOES, (GLenum target, GLuint renderbuffer))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBindRenderbuffer"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBindRenderbufferOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDeleteRenderbuffersOES, (GLsizei n, const GLuint* renderbuffers))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDeleteRenderbuffers"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDeleteRenderbuffersOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGenRenderbuffersOES, (GLsizei n, GLuint* renderbuffers))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGenRenderbuffers"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGenRenderbuffersOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glRenderbufferStorageOES, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glRenderbufferStorage"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glRenderbufferStorageOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetRenderbufferParameterivOES, (GLenum target, GLenum pname, GLint* params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetRenderbufferParameteriv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetRenderbufferParameterivOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(GLboolean, glIsFramebufferOES, (GLuint framebuffer))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glIsFramebuffer"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glIsFramebufferOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glBindFramebufferOES, (GLenum target, GLuint framebuffer))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBindFramebuffer"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBindFramebufferOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDeleteFramebuffersOES, (GLsizei n, const GLuint* framebuffers))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDeleteFramebuffers"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDeleteFramebuffersOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGenFramebuffersOES, (GLsizei n, GLuint* framebuffers))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGenFramebuffers"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGenFramebuffersOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(GLenum, glCheckFramebufferStatusOES, (GLenum target))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCheckFramebufferStatus"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCheckFramebufferStatusOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glFramebufferRenderbufferOES, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFramebufferRenderbuffer"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFramebufferRenderbufferOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glFramebufferTexture2DOES, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFramebufferTexture2D"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFramebufferTexture2DOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetFramebufferAttachmentParameterivOES, (GLenum target, GLenum attachment, GLenum pname, GLint* params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetFramebufferAttachmentParameteriv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetFramebufferAttachmentParameterivOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGenerateMipmapOES, (GLenum target))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGenerateMipmap"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGenerateMipmapOES"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(matrix_get)
        _EVASGL_EXT_DRVNAME(GL_OES_matrix_get)
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(matrix_palette)
        _EVASGL_EXT_DRVNAME(GL_OES_matrix_palette)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glCurrentPaletteMatrixOES, (GLuint matrixpaletteindex))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCurrentPaletteMatrix"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCurrentPaletteMatrixOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glLoadPaletteFromModelViewMatrixOES, (void))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLoadPaletteFromModelViewMatrix"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glLoadPaletteFromModelViewMatrixOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glMatrixIndexPointerOES, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMatrixIndexPointer"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMatrixIndexPointerOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glWeightPointerOES, (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glWeightPointer"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glWeightPointerOES"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(required_internalformat)
        _EVASGL_EXT_DRVNAME(GL_OES_required_internalformat)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(query_matrix)
        _EVASGL_EXT_DRVNAME(GL_OES_query_matrix)

        _EVASGL_EXT_FUNCTION_BEGIN(GLbitfield, glQueryMatrixxOES, (GLfixed mantissa[16], GLint exponent[16]))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glQueryMatrixx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glQueryMatrixxOES"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(single_precision)
        _EVASGL_EXT_DRVNAME(GL_OES_single_precision)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glDepthRangefOES, (GLclampf zNear, GLclampf zFar))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDepthRangef"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDepthRangefOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glFrustumfOES, (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFrustumf"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFrustumfOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glOrthofOES, (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glOrthof"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glOrthofOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glClipPlanefOES, (GLenum plane, const GLfloat *equation))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClipPlanef"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClipPlanefOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetClipPlanefOES, (GLenum pname, GLfloat eqn[4]))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetClipPlanef"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetClipPlanefOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glClearDepthfOES, (GLclampf depth))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClearDepthf"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClearDepthfOES"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(stencil8)
        _EVASGL_EXT_DRVNAME(GL_OES_stencil8)
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(stencil_wrap)
        _EVASGL_EXT_DRVNAME(GL_OES_stencil_wrap)
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(texture_cube_map)
        _EVASGL_EXT_DRVNAME(GL_OES_texture_cube_map)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexGenfOES, (GLenum coord, GLenum pname, GLfloat param))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGenf"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGenfOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexGenfvOES, (GLenum coord, GLenum pname, const GLfloat *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGenfv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGenfvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexGeniOES, (GLenum coord, GLenum pname, GLint param))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGeni"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGeniOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexGenivOES, (GLenum coord, GLenum pname, const GLint *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGeniv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGenivOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexGenxOES, (GLenum coord, GLenum pname, GLfixed param))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGenx"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGenxOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexGenxvOES, (GLenum coord, GLenum pname, const GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGenxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexGenxvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetTexGenfvOES, (GLenum coord, GLenum pname, GLfloat *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetTexGenfv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetTexGenfvOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetTexGenivOES, (GLenum coord, GLenum pname, GLint *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetTexGeniv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetTexGenivOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetTexGenxvOES, (GLenum coord, GLenum pname, GLfixed *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetTexGenxv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetTexGenxvOES"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(texture_env_crossbar)
        _EVASGL_EXT_DRVNAME(GL_OES_texture_env_crossbar)
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(texture_mirrored_repeat)
        _EVASGL_EXT_DRVNAME(GL_OES_texture_mirrored_repeat)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(vertex_array_object)
        _EVASGL_EXT_DRVNAME(GL_OES_vertex_array_object)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glBindVertexArrayOES, (GLuint array))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBindVertexArray"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glBindVertexArrayOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDeleteVertexArraysOES, (GLsizei n, const GLuint *arrays))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDeleteVertexArrays"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDeleteVertexArraysOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGenVertexArraysOES, (GLsizei n, GLuint *arrays))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGenVertexArrays"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGenVertexArraysOES"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(GLboolean, glIsVertexArrayOES, (GLuint array))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glIsVertexArray"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glIsVertexArrayOES"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()

/* APPLE extension functions */
_EVASGL_EXT_BEGIN(APPLE_copy_texture_levels)
        _EVASGL_EXT_DRVNAME(GL_APPLE_copy_texture_levels)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glCopyTextureLevelsAPPLE, (GLuint destinationTexture, GLuint sourceTexture, GLint sourceBaseLevel, GLsizei sourceLevelCount))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCopyTextureLevels"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glCopyTextureLevelsAPPLE"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(APPLE_framebuffer_multisample)
        _EVASGL_EXT_DRVNAME(GL_APPLE_framebuffer_multisample)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glRenderbufferStorageMultisampleAPPLE, (GLenum, GLsizei, GLenum, GLsizei, GLsizei))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glRenderbufferStorageMultisample"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glRenderbufferStorageMultisampleAPPLE"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glResolveMultisampleFramebufferAPPLE, (void))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glResolveMultisampleFramebuffer"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glResolveMultisampleFramebufferAPPLE"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(APPLE_sync)
        _EVASGL_EXT_DRVNAME(GL_APPLE_sync)

        _EVASGL_EXT_FUNCTION_BEGIN(GLsync, glFenceSyncAPPLE, (GLenum condition, GLbitfield flags))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFenceSyncAPPLE"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(GLboolean, glIsSyncAPPLE, (GLsync sync))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glIsSyncAPPLE"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glDeleteSyncAPPLE, (GLsync sync))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glDeleteSyncAPPLE"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(GLenum, glClientWaitSyncAPPLE, (GLsync sync, GLbitfield flags, EvasGLuint64 timeout))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClientWaitSyncAPPLE"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glWaitSyncAPPLE, (GLsync sync, GLbitfield flags, EvasGLuint64 timeout))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glWaitSyncAPPLE"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetInteger64vAPPLE, (GLenum pname, EvasGLint64 *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetInteger64vAPPLE"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetSyncivAPPLE, (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetSyncivAPPLE"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(APPLE_texture_2D_limited_npot)
        _EVASGL_EXT_DRVNAME(GL_APPLE_texture_2D_limited_npot)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(APPLE_texture_format_BGRA8888)
        _EVASGL_EXT_DRVNAME(GL_APPLE_texture_format_BGRA8888)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(APPLE_texture_max_level)
        _EVASGL_EXT_DRVNAME(GL_APPLE_texture_max_level)
_EVASGL_EXT_END()

/* ARM extension */
_EVASGL_EXT_BEGIN(ARM_rgba8)
        _EVASGL_EXT_DRVNAME(GL_ARM_rgba8)
_EVASGL_EXT_END()

/* EXT extension functions */
_EVASGL_EXT_BEGIN(map_buffer_range)
        _EVASGL_EXT_DRVNAME(GL_EXT_map_buffer_range)

        _EVASGL_EXT_FUNCTION_BEGIN(void *, glMapBufferRangeEXT, (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMapBufferRange"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glMapBufferRangeEXT"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glFlushMappedBufferRangeEXT, (GLenum target, GLintptr offset, GLsizeiptr length))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFlushMappedBufferRange"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glFlushMappedBufferRangeEXT"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(robustness)
        _EVASGL_EXT_DRVNAME(GL_EXT_robustness)

        _EVASGL_EXT_FUNCTION_BEGIN(GLenum, glGetGraphicsResetStatusEXT, (void))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetGraphicsResetStatus"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetGraphicsResetStatusEXT"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glReadnPixelsEXT, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glReadnPixels"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glReadnPixelsEXT"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetnUniformfvEXT, (GLuint program, GLint location, GLsizei bufSize, float *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetnUniformfv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetnUniformfvEXT"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glGetnUniformivEXT, (GLuint program, GLint location, GLsizei bufSize, GLint *params))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetnUniformiv"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glGetnUniformivEXT"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(sRGB)
        _EVASGL_EXT_DRVNAME(GL_EXT_sRGB)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_compression_dxt1)
        _EVASGL_EXT_DRVNAME(GL_EXT_texture_compression_dxt1)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_lod_bias)
        _EVASGL_EXT_DRVNAME(GL_EXT_texture_lod_bias)
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(texture_storage)
        _EVASGL_EXT_DRVNAME(GL_EXT_texture_storage)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexStorage1DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexStorage1D"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexStorage1DEXT"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexStorage2DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexStorage2D"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexStorage2DEXT"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTexStorage3DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexStorage3D"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTexStorage3DEXT"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTextureStorage1DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTextureStorage1D"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTextureStorage1DEXT"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTextureStorage2DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTextureStorage2D"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTextureStorage2DEXT"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glTextureStorage3DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTextureStorage3D"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glTextureStorage3DEXT"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(IMG_texture_env_enhanced_fixed_function)
        _EVASGL_EXT_DRVNAME(GL_IMG_texture_env_enhanced_fixed_function)
_EVASGL_EXT_END()


_EVASGL_EXT_BEGIN(IMG_user_clip_plane)
        _EVASGL_EXT_DRVNAME(GL_IMG_user_clip_plane)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glClipPlanefIMG, (GLenum, const GLfloat *))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClipPlanef"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClipPlanefIMG"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glClipPlanexIMG, (GLenum, const GLfixed *))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClipPlanex"))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glClipPlanexIMG"))
        _EVASGL_EXT_FUNCTION_END()
_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(QCOM_tiled_rendering)
        _EVASGL_EXT_DRVNAME(GL_QCOM_tiled_rendering)

        _EVASGL_EXT_FUNCTION_BEGIN(void, glStartTilingQCOM, (GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glStartTilingQCOM"))
        _EVASGL_EXT_FUNCTION_END()
        _EVASGL_EXT_FUNCTION_BEGIN(void, glEndTilingQCOM, (GLbitfield preserveMask))
                _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("glEndTilingQCOM"))
        _EVASGL_EXT_FUNCTION_END()
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

	_EVASGL_EXT_FUNCTION_BEGIN(EvasGLImage, evasglCreateImage, (int target, void* buffer, const int *attrib_list))
		_EVASGL_EXT_FUNCTION_DRVFUNC(evgl_evasglCreateImage)
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(void, evasglDestroyImage, (EvasGLImage image))
		_EVASGL_EXT_FUNCTION_DRVFUNC(evgl_evasglDestroyImage)
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(EvasGLImage, evasglCreateImageForContext, (Evas_GL *evas_gl, Evas_GL_Context *ctx, int target, void* buffer, const int *attrib_list))
		_EVASGL_EXT_FUNCTION_DRVFUNC(evgl_evasglCreateImageForContext)
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

_EVASGL_EXT_BEGIN(EGL_KHR_fence_sync)

	_EVASGL_EXT_DRVNAME(EGL_KHR_fence_sync)

	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(void *, eglCreateSyncKHR, (EGLDisplay dpy, EGLenum type, const EGLint *attrib_list))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("eglCreateSyncKHR"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(EGLBoolean, eglDestroySyncKHR, (EGLDisplay dpy, EGLSyncKHR sync))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("eglDestroySyncKHR"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(EGLint, eglClientWaitSyncKHR, (EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("eglClientWaitSyncKHR"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()
	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(EGLBoolean, eglGetSyncAttribKHR, (EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("eglGetSyncAttribKHR"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()

	_EVASGL_EXT_FUNCTION_BEGIN(EvasGLSync, evasglCreateSync, (Evas_GL *evas_gl, unsigned int type, const int *attrib_list))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(evgl_evasglCreateSync)
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(Eina_Bool, evasglDestroySync, (Evas_GL *evas_gl, EvasGLSync sync))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(evgl_evasglDestroySync)
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(int, evasglClientWaitSync, (Evas_GL *evas_gl, EvasGLSync sync, int flags, EvasGLTime timeout))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(evgl_evasglClientWaitSync)
	_EVASGL_EXT_FUNCTION_END()
	_EVASGL_EXT_FUNCTION_BEGIN(Eina_Bool, evasglGetSyncAttrib, (Evas_GL *evas_gl, EvasGLSync sync, int attribute, int *value))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(evgl_evasglGetSyncAttrib)
	_EVASGL_EXT_FUNCTION_END()

_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(EGL_KHR_reusable_sync)

	_EVASGL_EXT_DRVNAME(EGL_KHR_reusable_sync)

	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(EGLBoolean, eglSignalSyncKHR, (EGLDisplay dpy, EGLSyncKHR sync, EGLenum mode))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("eglSignalSyncKHR"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()

	_EVASGL_EXT_FUNCTION_BEGIN(Eina_Bool, evasglSignalSync, (Evas_GL *evas_gl, EvasGLSync sync, unsigned mode))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(evgl_evasglSignalSync)
	_EVASGL_EXT_FUNCTION_END()

_EVASGL_EXT_END()

_EVASGL_EXT_BEGIN(EGL_KHR_wait_sync)

	_EVASGL_EXT_DRVNAME(EGL_KHR_wait_sync)

	_EVASGL_EXT_FUNCTION_PRIVATE_BEGIN(EGLint, eglWaitSyncKHR, (EGLDisplay dpy, EGLSyncKHR sync, int flags))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(GETPROCADDR("eglWaitSyncKHR"))
	_EVASGL_EXT_FUNCTION_PRIVATE_END()

	_EVASGL_EXT_FUNCTION_BEGIN(int, evasglWaitSync, (Evas_GL *evas_gl, EvasGLSync sync, int flags))
	        _EVASGL_EXT_FUNCTION_DRVFUNC(evgl_evasglWaitSync)
	_EVASGL_EXT_FUNCTION_END()

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


