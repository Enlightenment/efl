#ifndef _EVAS_GL_API_EXT_H
#define _EVAS_GL_API_EXT_H

#include "evas_gl_core_private.h"

#ifdef GL_GLES
#include <EGL/egl.h>
#include <EGL/eglext.h>
#else
# ifdef BUILD_ENGINE_GL_COCOA
#  include <OpenGL/gl.h>
#  include <OpenGL/glext.h>
# else
#  include <GL/glext.h>
#  include <GL/glx.h>
# endif
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Extension HEADER
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_BEGIN(name)
#define _EVASGL_EXT_END()
#define _EVASGL_EXT_DRVNAME(name)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param) extern ret (*glextsym_##name) param;
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXT_FUNC(fname) glextsym_##fname


/////////////////////////////////////////////////////////////////////////////////////////////////////
// Extension HEADER
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_BEGIN(name) extern int _gl_ext_support_##name;
#define _EVASGL_EXT_END()
#define _EVASGL_EXT_DRVNAME(name)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param)
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXTENSION_SUPPORT(name) (_gl_ext_support_##name == 1)

extern void evgl_api_ext_init(void *getproc, const char *glueexts);
extern void evgl_api_ext_get(Evas_GL_API *gl_funcs);
extern const char *evgl_api_ext_string_get();

#endif //_EVAS_GL_API_EXT_H

