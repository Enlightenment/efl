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
#define _EVASGL_EXT_DRVNAME_PRIVATE(name)
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param1, param2) \
   extern ret (*egl_ext_sym_##name) param1; \
   extern ret (*gl_ext_sym_##name) param1; \
   extern ret (*gles1_ext_sym_##name) param1; \
   extern ret (*gles3_ext_sym_##name) param1;
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name)

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXT_FUNC_EGL(fname) egl_ext_sym_##fname
#define EXT_FUNC(fname) gl_ext_sym_##fname
#define EXT_FUNC_GLES1(fname) gles1_ext_sym_##fname
#define EXT_FUNC_GLES3(fname) gles3_ext_sym_##fname

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Extension HEADER
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define _EVASGL_EXT_CHECK_SUPPORT(name)
#define _EVASGL_EXT_DISCARD_SUPPORT()
#define _EVASGL_EXT_BEGIN(name) \
   extern int _egl_ext_support_##name; \
   extern int _gl_ext_support_##name; \
   extern int _gles1_ext_support_##name; \
   extern int _gles3_ext_support_##name;
#define _EVASGL_EXT_END()
#define _EVASGL_EXT_DRVNAME(name)
#define _EVASGL_EXT_DRVNAME_PRIVATE(name)
#define _EVASGL_EXT_DRVNAME_DESKTOP(deskname)
#define _EVASGL_EXT_FUNCTION_BEGIN(ret, name, param1, param2)
#define _EVASGL_EXT_FUNCTION_END()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN()
#define _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END()
#define _EVASGL_EXT_FUNCTION_DRVFUNC(name)
#define _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR(name)

#include "evas_gl_api_ext_def.h"

#undef _EVASGL_EXT_CHECK_SUPPORT
#undef _EVASGL_EXT_DISCARD_SUPPORT
#undef _EVASGL_EXT_BEGIN
#undef _EVASGL_EXT_END
#undef _EVASGL_EXT_DRVNAME
#undef _EVASGL_EXT_DRVNAME_PRIVATE
#undef _EVASGL_EXT_DRVNAME_DESKTOP
#undef _EVASGL_EXT_FUNCTION_BEGIN
#undef _EVASGL_EXT_FUNCTION_END
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_BEGIN
#undef _EVASGL_EXT_FUNCTION_DISABLE_FOR_GLES1_END
#undef _EVASGL_EXT_FUNCTION_DRVFUNC
#undef _EVASGL_EXT_FUNCTION_DRVFUNC_PROCADDR
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXTENSION_SUPPORT_EGL(name) (_egl_ext_support_##name == 1)
#define EXTENSION_SUPPORT(name) (_gl_ext_support_##name == 1)
#define EXTENSION_SUPPORT_GLES1(name) (_gles1_ext_support_##name == 1)
#define EXTENSION_SUPPORT_GLES3(name) (_gles3_ext_support_##name == 1)

#ifdef GL_GLES
extern Eina_Bool evgl_api_egl_ext_init(void *getproc, const char *glueexts);
#endif
extern void evgl_api_gles2_ext_get(Evas_GL_API *gl_funcs, void *getproc, const char *glueexts);
extern void evgl_api_gles1_ext_get(Evas_GL_API *gl_funcs, void *getproc, const char *glueexts);
extern void evgl_api_gles3_ext_get(Evas_GL_API *gl_funcs, void *getproc, const char *glueexts);
extern const char *evgl_api_ext_egl_string_get(void);
extern const char *evgl_api_ext_string_get(Eina_Bool official, int version);
extern const char *evgl_api_ext_stringi_get(GLuint index, int version);
extern GLuint evgl_api_ext_num_extensions_get(int version);

#endif //_EVAS_GL_API_EXT_H

