#include "evas_common.h" /* Also includes international specific stuff */
#include "evas_engine.h"

#ifdef HAVE_DLSYM
# include <dlfcn.h>      /* dlopen,dlclose,etc */
#else
# error gl_x11 should not get compiled if dlsym is not found on the system!
#endif

#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

typedef struct _Render_Engine               Render_Engine;
typedef struct _Render_Engine_GL_Surface    Render_Engine_GL_Surface;
typedef struct _Render_Engine_GL_Context    Render_Engine_GL_Context;
typedef struct _Render_Engine_GL_Resource   Render_Engine_GL_Resource;
typedef struct _Extension_Entry             Extension_Entry;

struct _Render_Engine
{
   Evas_GL_Wl_Window      *win;
   Evas_Engine_Info_Wayland_Egl *info;
   Evas                    *evas;
   Tilebuf                 *tb;
   int                      end;
   int w, h;
   int vsync;
};

struct _Render_Engine_GL_Surface
{
   int      initialized;
   int      fbo_attached;
   int      w, h;
   int      depth_bits;
   int      stencil_bits;

   // Render target texture/buffers
   GLuint   rt_tex;
   GLint    rt_internal_fmt;
   GLenum   rt_fmt;
   GLuint   rb_depth;
   GLenum   rb_depth_fmt;
   GLuint   rb_stencil;
   GLenum   rb_stencil_fmt;

   Render_Engine_GL_Context   *current_ctx;
};

struct _Render_Engine_GL_Context
{
   int         initialized;
   EGLContext  context;
   GLuint      context_fbo;
   GLuint      current_fbo;

   Render_Engine_GL_Surface   *current_sfc;
};

// Resources used per thread
struct _Render_Engine_GL_Resource
{
   // Resource context/surface per Thread in TLS for evasgl use 
   EGLContext context;
   EGLSurface surface;
};

// Extension Handling
struct _Extension_Entry
{
   const char *name;
   const char *real_name;
   int supported;
};

static int initted = 0;
static int gl_wins = 0;
static Render_Engine_GL_Context *current_evgl_ctx;
static Render_Engine *current_engine;

static char _gl_ext_string[1024];
static char _evasgl_ext_string[1024];

// Resource context/surface per Thread in TLS for evasgl use 
static Eina_TLS   resource_key;
static Eina_List *resource_list;
LK(resource_lock);

typedef void            (*_eng_fn) (void);
typedef _eng_fn         (*glsym_func_eng_fn) ();
typedef void            (*glsym_func_void) ();
typedef void           *(*glsym_func_void_ptr) ();
typedef int             (*glsym_func_int) ();
typedef unsigned int    (*glsym_func_uint) ();
typedef unsigned char   (*glsym_func_uchar) ();
typedef unsigned char  *(*glsym_func_uchar_ptr) ();
typedef const char     *(*glsym_func_const_char_ptr) ();

#ifndef EGL_NATIVE_PIXMAP_KHR
# define EGL_NATIVE_PIXMAP_KHR 0x30b0
#endif
_eng_fn  (*glsym_eglGetProcAddress)            (const char *a) = NULL;
void     (*glsym_eglBindTexImage)              (EGLDisplay a, EGLSurface b, int c) = NULL;
void     (*glsym_eglReleaseTexImage)           (EGLDisplay a, EGLSurface b, int c) = NULL;
void    *(*glsym_eglCreateImage)               (EGLDisplay a, EGLContext b, EGLenum c, EGLClientBuffer d, const int *e) = NULL;
void     (*glsym_eglDestroyImage)              (EGLDisplay a, void *b) = NULL;
void     (*glsym_glEGLImageTargetTexture2DOES) (int a, void *b)  = NULL;
void     (*glsym_glEGLImageTargetRenderbufferStorageOES) (int a, void *b)  = NULL;
void          *(*glsym_eglMapImageSEC)         (void *a, void *b) = NULL;
unsigned int   (*glsym_eglUnmapImageSEC)       (void *a, void *b) = NULL;
const char    *(*glsym_eglQueryString)         (EGLDisplay a, int name) = NULL;

unsigned int   (*glsym_eglLockSurface)          (EGLDisplay a, EGLSurface b, const int *attrib_list) = NULL;
unsigned int   (*glsym_eglUnlockSurface)        (EGLDisplay a, EGLSurface b) = NULL;

// GLES2 Extensions
void 	(*glsym_glGetProgramBinaryOES) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary) = NULL;
void 	(*glsym_glProgramBinaryOES) (GLuint program, GLenum binaryFormat, const void *binary, GLint length) = NULL;
void* 	(*glsym_glMapBufferOES) (GLenum target, GLenum access) = NULL;
unsigned char 	(*glsym_glUnmapBufferOES) (GLenum target) = NULL;
void 	(*glsym_glGetBufferPointervOES) (GLenum target, GLenum pname, void** params) = NULL;
void 	(*glsym_glTexImage3DOES) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels) = NULL;
void 	(*glsym_glTexSubImage3DOES) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels) = NULL;
void 	(*glsym_glCopyTexSubImage3DOES) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
void 	(*glsym_glCompressedTexImage3DOES) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data) = NULL;
void 	(*glsym_glCompressedTexSubImage3DOES) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data) = NULL;
void 	(*glsym_glFramebufferTexture3DOES) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) = NULL;
void 	(*glsym_glGetPerfMonitorGroupsAMD) (GLint* numGroups, GLsizei groupsSize, GLuint* groups) = NULL;
void 	(*glsym_glGetPerfMonitorCountersAMD) (GLuint group, GLint* numCounters, GLint* maxActiveCounters, GLsizei counterSize, GLuint* counters) = NULL;
void 	(*glsym_glGetPerfMonitorGroupStringAMD) (GLuint group, GLsizei bufSize, GLsizei* length, char* groupString) = NULL;
void 	(*glsym_glGetPerfMonitorCounterStringAMD) (GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, char* counterString) = NULL;
void 	(*glsym_glGetPerfMonitorCounterInfoAMD) (GLuint group, GLuint counter, GLenum pname, void* data) = NULL;
void 	(*glsym_glGenPerfMonitorsAMD) (GLsizei n, GLuint* monitors) = NULL;
void 	(*glsym_glDeletePerfMonitorsAMD) (GLsizei n, GLuint* monitors) = NULL;
void 	(*glsym_glSelectPerfMonitorCountersAMD) (GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* countersList) = NULL;
void 	(*glsym_glBeginPerfMonitorAMD) (GLuint monitor) = NULL;
void 	(*glsym_glEndPerfMonitorAMD) (GLuint monitor) = NULL;
void 	(*glsym_glGetPerfMonitorCounterDataAMD) (GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint* bytesWritten) = NULL;
void 	(*glsym_glDiscardFramebufferEXT) (GLenum target, GLsizei numAttachments, const GLenum* attachments) = NULL;
void 	(*glsym_glMultiDrawArraysEXT) (GLenum mode, GLint* first, GLsizei* count, GLsizei primcount) = NULL;
void 	(*glsym_glMultiDrawElementsEXT) (GLenum mode, const GLsizei* count, GLenum type, const GLvoid** indices, GLsizei primcount) = NULL;
void 	(*glsym_glDeleteFencesNV) (GLsizei n, const GLuint* fences) = NULL;
void 	(*glsym_glGenFencesNV) (GLsizei n, GLuint* fences) = NULL;
unsigned char 	(*glsym_glIsFenceNV) (GLuint fence) = NULL;
unsigned char 	(*glsym_glTestFenceNV) (GLuint fence) = NULL;
void 	(*glsym_glGetFenceivNV) (GLuint fence, GLenum pname, GLint* params) = NULL;
void 	(*glsym_glFinishFenceNV) (GLuint fence) = NULL;
void 	(*glsym_glSetFenceNV) (GLuint, GLenum) = NULL;
void 	(*glsym_glGetDriverControlsQCOM) (GLint* num, GLsizei size, GLuint* driverControls) = NULL;
void 	(*glsym_glGetDriverControlStringQCOM) (GLuint driverControl, GLsizei bufSize, GLsizei* length, char* driverControlString) = NULL;
void 	(*glsym_glEnableDriverControlQCOM) (GLuint driverControl) = NULL;
void 	(*glsym_glDisableDriverControlQCOM) (GLuint driverControl) = NULL;
void 	(*glsym_glExtGetTexturesQCOM) (GLuint* textures, GLint maxTextures, GLint* numTextures) = NULL;
void 	(*glsym_glExtGetBuffersQCOM) (GLuint* buffers, GLint maxBuffers, GLint* numBuffers) = NULL;
void 	(*glsym_glExtGetRenderbuffersQCOM) (GLuint* renderbuffers, GLint maxRenderbuffers, GLint* numRenderbuffers) = NULL;
void 	(*glsym_glExtGetFramebuffersQCOM) (GLuint* framebuffers, GLint maxFramebuffers, GLint* numFramebuffers) = NULL;
void 	(*glsym_glExtGetTexLevelParameterivQCOM) (GLuint texture, GLenum face, GLint level, GLenum pname, GLint* params) = NULL;
void 	(*glsym_glExtTexObjectStateOverrideiQCOM) (GLenum target, GLenum pname, GLint param) = NULL;
void 	(*glsym_glExtGetTexSubImageQCOM) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, void* texels) = NULL;
void 	(*glsym_glExtGetBufferPointervQCOM) (GLenum target, void** params) = NULL;
void 	(*glsym_glExtGetShadersQCOM) (GLuint* shaders, GLint maxShaders, GLint* numShaders) = NULL;
void 	(*glsym_glExtGetProgramsQCOM) (GLuint* programs, GLint maxPrograms, GLint* numPrograms) = NULL;
unsigned char 	(*glsym_glExtIsProgramBinaryQCOM) (GLuint program) = NULL;
void 	(*glsym_glExtGetProgramBinarySourceQCOM) (GLuint program, GLenum shadertype, char* source, GLint* length) = NULL;


//------ GLES 2.0 Extensions supported in EvasGL -----//
static Extension_Entry _gl_ext_entries[] = {
       //--- Function Extensions ---//
       { "GL_OES_get_program_binary", "get_program_binary", 0 },
       { "GL_OES_mapbuffer", "mapbuffer", 0 },
       { "GL_OES_texture_3D", "texture_3D", 0 },
       { "AMD_performance_monitor", "AMD_performance_monitor", 0 },
       { "GL_EXT_discard_framebuffer", "discard_framebuffer", 0 },
       { "GL_EXT_multi_draw_arrays", "multi_draw_arrays", 0 },
       { "GL_NV_fence", "NV_fence", 0 },
       { "GL_QCOM_driver_control", "QCOM_driver_control", 0 },
       { "GL_QCOM_extended_get", "QCOM_extended_get", 0 },
       { "GL_QCOM_extended_get2", "QCOM_extended_get2", 0 },

       //--- Define Extensions ---//
       { "GL_OES_compressed_ETC1_RGB8_texture", "compressed_ETC1_RGB8_texture", 0 },
       { "GL_OES_compressed_paletted_texture", "compressed_paletted_texture", 0 },
       { "GL_OES_depth24", "depth24", 0 },
       { "GL_OES_depth32", "depth32", 0 },
       { "GL_OES_EvasGL_image", "EGL_image", 0 },
       { "GL_OES_packed_depth_stencil", "packed_depth_stencil", 0 },
       { "GL_OES_rgb8_rgba8", "rgb8_rgba8", 0 },
       { "GL_OES_standard_derivatives", "standard_derivatives", 0 },
       { "GL_OES_stencil1", "stencil1", 0 },
       { "GL_OES_stencil4", "stencil4", 0 },
       { "GL_OES_texture_float", "texture_float", 0 },
       { "GL_OES_texture_half_float", "texture_half_float", 0 },
       { "GL_OES_texture_half_float_linear", "texture_half_float_linear", 0 },
       { "GL_OES_texture_npot", "texture_npot", 0 },
       { "GL_OES_vertex_half_float", "vertex_half_float", 0 },
       { "GL_OES_vertex_type_10_10_10_2", "vertex_type_10_10_10_2", 0 },
       { "GL_AMD_compressed_3DC_texture", "compressed_3DC_texture", 0 },
       { "GL_AMD_compressed_ATC_texture", "compressed_ATC_texture", 0 },
       { "GL_AMD_program_binary_Z400", "program_binary_Z400", 0 },
       { "GL_EXT_blend_minmax", "blend_minmax", 0 },
       { "GL_EXT_read_format_bgra", "read_format_bgra", 0 },
       { "GL_EXT_texture_filter_anisotropic", "texture_filter_anisotrophic", 0 },
       { "GL_EXT_texture_format_BGRA8888", "texture_format_BGRA8888", 0 },
       { "GL_EXT_texture_type_2_10_10_10_REV", "texture_type_2_10_10_10_rev", 0 },
       { "GL_IMG_program_binary", "IMG_program_binary", 0 },
       { "GL_IMG_read_format", "IMG_read_format", 0 },
       { "GL_IMG_shader_binary", "IMG_shader_binary", 0 },
       { "GL_IMG_texture_compression_pvrtc", "IMG_texture_compression_pvrtc", 0 },
       { "GL_QCOM_perfmon_global_mode", "QCOM_perfmon_global_mode", 0 },
       { "GL_QCOM_writeonly_rendering", "QCOM_writeonly_rendering", 0 },
       { NULL, NULL, 0}
};

//------ Extensions supported in EvasGL -----//
static Extension_Entry _evasgl_ext_entries[] = {
       { "EvasGL_KHR_image", "EGL_KHR_image", 0 },
       { "EvasGL_KHR_vg_parent_image", "EGL_KHR_vg_parent_image", 0 },
       { "EvasGL_KHR_gl_texture_2D_image", "EGL_KHR_gl_texture_2D_image", 0 },
       { "EvasGL_KHR_gl_texture_cubemap_image", "EGL_KHR_gl_texture_cubemap_image", 0 },
       { "EvasGL_KHR_gl_texture_3D_image", "EGL_KHR_gl_texture_3D_image", 0 },
       { "EvasGL_KHR_gl_renderbuffer_image", "EGL_KHR_gl_renderbuffer_image", 0 },
       { NULL, NULL, 0 }
};

static void
_sym_init(void)
{
   static int done = 0;

   if (done) return;

#define FINDSYM(dst, sym, typ) \
   if ((!dst) && (glsym_eglGetProcAddress)) dst = (typ)glsym_eglGetProcAddress(sym); \
   if (!dst) dst = (typ)dlsym(RTLD_DEFAULT, sym)

   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddress", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressEXT", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressARB", glsym_func_eng_fn);
   FINDSYM(glsym_eglGetProcAddress, "eglGetProcAddressKHR", glsym_func_eng_fn);

   FINDSYM(glsym_eglBindTexImage, "eglBindTexImage", glsym_func_void);
   FINDSYM(glsym_eglBindTexImage, "eglBindTexImageEXT", glsym_func_void);
   FINDSYM(glsym_eglBindTexImage, "eglBindTexImageARB", glsym_func_void);
   FINDSYM(glsym_eglBindTexImage, "eglBindTexImageKHR", glsym_func_void);

   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImage", glsym_func_void);
   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImageEXT", glsym_func_void);
   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImageARB", glsym_func_void);
   FINDSYM(glsym_eglReleaseTexImage, "eglReleaseTexImageKHR", glsym_func_void);

   FINDSYM(glsym_eglCreateImage, "eglCreateImage", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageEXT", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageARB", glsym_func_void_ptr);
   FINDSYM(glsym_eglCreateImage, "eglCreateImageKHR", glsym_func_void_ptr);

   FINDSYM(glsym_eglDestroyImage, "eglDestroyImage", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageEXT", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageARB", glsym_func_void);
   FINDSYM(glsym_eglDestroyImage, "eglDestroyImageKHR", glsym_func_void);

   FINDSYM(glsym_glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES", glsym_func_void);

   FINDSYM(glsym_glEGLImageTargetRenderbufferStorageOES, "glEGLImageTargetRenderbufferStorageOES", glsym_func_void);

   FINDSYM(glsym_eglMapImageSEC, "eglMapImageSEC", glsym_func_void_ptr);
   FINDSYM(glsym_eglUnmapImageSEC, "eglUnmapImageSEC", glsym_func_uint);

   FINDSYM(glsym_eglQueryString, "eglQueryString", glsym_func_const_char_ptr);

   FINDSYM(glsym_eglLockSurface, "eglLockSurface", glsym_func_uint);
   FINDSYM(glsym_eglLockSurface, "eglLockSurfaceEXT", glsym_func_uint);
   FINDSYM(glsym_eglLockSurface, "eglLockSurfaceARB", glsym_func_uint);
   FINDSYM(glsym_eglLockSurface, "eglLockSurfaceKHR", glsym_func_uint);

   FINDSYM(glsym_eglUnlockSurface, "eglUnlockSurface", glsym_func_uint);
   FINDSYM(glsym_eglUnlockSurface, "eglUnlockSurfaceEXT", glsym_func_uint);
   FINDSYM(glsym_eglUnlockSurface, "eglUnlockSurfaceARB", glsym_func_uint);
   FINDSYM(glsym_eglUnlockSurface, "eglUnlockSurfaceKHR", glsym_func_uint);

   //----------- GLES 2.0 Extensions ------------//
   // If the symbol's not found, they get set to NULL
   // If one of the functions in the extension exists, the extension in supported
   /* GL_OES_get_program_binary */
   FINDSYM(glsym_glGetProgramBinaryOES, "glGetProgramBinary", glsym_func_void);
   FINDSYM(glsym_glGetProgramBinaryOES, "glGetProgramBinaryEXT", glsym_func_void);
   FINDSYM(glsym_glGetProgramBinaryOES, "glGetProgramBinaryARB", glsym_func_void);
   FINDSYM(glsym_glGetProgramBinaryOES, "glGetProgramBinaryOES", glsym_func_void);

   FINDSYM(glsym_glProgramBinaryOES, "glProgramBinary", glsym_func_void);
   FINDSYM(glsym_glProgramBinaryOES, "glProgramBinaryEXT", glsym_func_void);
   FINDSYM(glsym_glProgramBinaryOES, "glProgramBinaryARB", glsym_func_void);
   FINDSYM(glsym_glProgramBinaryOES, "glProgramBinaryOES", glsym_func_void);

   // Check the first function to see if the extension is supported...
   if (glsym_glGetProgramBinaryOES) _gl_ext_entries[0].supported = 1;

   /* GL_OES_mapbuffer */
   FINDSYM(glsym_glMapBufferOES, "glMapBuffer", glsym_func_void_ptr);
   FINDSYM(glsym_glMapBufferOES, "glMapBufferEXT", glsym_func_void_ptr);
   FINDSYM(glsym_glMapBufferOES, "glMapBufferARB", glsym_func_void_ptr);
   FINDSYM(glsym_glMapBufferOES, "glMapBufferOES", glsym_func_void_ptr);

   FINDSYM(glsym_glUnmapBufferOES, "glUnmapBuffer", glsym_func_uchar);
   FINDSYM(glsym_glUnmapBufferOES, "glUnmapBufferEXT", glsym_func_uchar);
   FINDSYM(glsym_glUnmapBufferOES, "glUnmapBufferARB", glsym_func_uchar);
   FINDSYM(glsym_glUnmapBufferOES, "glUnmapBufferOES", glsym_func_uchar);

   FINDSYM(glsym_glGetBufferPointervOES, "glGetBufferPointerv", glsym_func_void);
   FINDSYM(glsym_glGetBufferPointervOES, "glGetBufferPointervEXT", glsym_func_void);
   FINDSYM(glsym_glGetBufferPointervOES, "glGetBufferPointervARB", glsym_func_void);
   FINDSYM(glsym_glGetBufferPointervOES, "glGetBufferPointervOES", glsym_func_void);

   if (glsym_glMapBufferOES) _gl_ext_entries[1].supported = 1;

   /* GL_OES_texture_3D */
   FINDSYM(glsym_glTexImage3DOES, "glTexImage3D", glsym_func_void);
   FINDSYM(glsym_glTexImage3DOES, "glTexImage3DEXT", glsym_func_void);
   FINDSYM(glsym_glTexImage3DOES, "glTexImage3DARB", glsym_func_void);
   FINDSYM(glsym_glTexImage3DOES, "glTexImage3DOES", glsym_func_void);

   FINDSYM(glsym_glTexSubImage3DOES, "glTexSubImage3D", glsym_func_void);
   FINDSYM(glsym_glTexSubImage3DOES, "glTexSubImage3DEXT", glsym_func_void);
   FINDSYM(glsym_glTexSubImage3DOES, "glTexSubImage3DARB", glsym_func_void);
   FINDSYM(glsym_glTexSubImage3DOES, "glTexSubImage3DOES", glsym_func_void);

   FINDSYM(glsym_glCopyTexSubImage3DOES, "glCopyTexSubImage3D", glsym_func_void);
   FINDSYM(glsym_glCopyTexSubImage3DOES, "glCopyTexSubImage3DARB", glsym_func_void);
   FINDSYM(glsym_glCopyTexSubImage3DOES, "glCopyTexSubImage3DEXT", glsym_func_void);
   FINDSYM(glsym_glCopyTexSubImage3DOES, "glCopyTexSubImage3DOES", glsym_func_void);

   FINDSYM(glsym_glCompressedTexImage3DOES, "glCompressedTexImage3D", glsym_func_void);
   FINDSYM(glsym_glCompressedTexImage3DOES, "glCompressedTexImage3DARB", glsym_func_void);
   FINDSYM(glsym_glCompressedTexImage3DOES, "glCompressedTexImage3DEXT", glsym_func_void);
   FINDSYM(glsym_glCompressedTexImage3DOES, "glCompressedTexImage3DOES", glsym_func_void);

   FINDSYM(glsym_glCompressedTexSubImage3DOES, "glCompressedTexSubImage3D", glsym_func_void);
   FINDSYM(glsym_glCompressedTexSubImage3DOES, "glCompressedTexSubImage3DARB", glsym_func_void);
   FINDSYM(glsym_glCompressedTexSubImage3DOES, "glCompressedTexSubImage3DEXT", glsym_func_void);
   FINDSYM(glsym_glCompressedTexSubImage3DOES, "glCompressedTexSubImage3DOES", glsym_func_void);

   FINDSYM(glsym_glFramebufferTexture3DOES, "glFramebufferTexture3D", glsym_func_void);
   FINDSYM(glsym_glFramebufferTexture3DOES, "glFramebufferTexture3DARB", glsym_func_void);
   FINDSYM(glsym_glFramebufferTexture3DOES, "glFramebufferTexture3DEXT", glsym_func_void);
   FINDSYM(glsym_glFramebufferTexture3DOES, "glFramebufferTexture3DOES", glsym_func_void);

   if (glsym_glTexSubImage3DOES) _gl_ext_entries[2].supported = 1;

   /* AMD_performance_monitor */
   FINDSYM(glsym_glGetPerfMonitorGroupsAMD, "glGetPerfMonitorGroupsAMD", glsym_func_void);
   FINDSYM(glsym_glGetPerfMonitorCountersAMD, "glGetPerfMonitorCountersAMD", glsym_func_void);
   FINDSYM(glsym_glGetPerfMonitorGroupStringAMD, "glGetPerfMonitorGroupStringAMD", glsym_func_void);
   FINDSYM(glsym_glGetPerfMonitorCounterStringAMD, "glGetPerfMonitorCounterStringAMD", glsym_func_void);
   FINDSYM(glsym_glGetPerfMonitorCounterInfoAMD, "glGetPerfMonitorCounterInfoAMD", glsym_func_void);
   FINDSYM(glsym_glGenPerfMonitorsAMD, "glGenPerfMonitorsAMD", glsym_func_void);
   FINDSYM(glsym_glDeletePerfMonitorsAMD, "glDeletePerfMonitorsAMD", glsym_func_void);
   FINDSYM(glsym_glSelectPerfMonitorCountersAMD, "glSelectPerfMonitorCountersAMD", glsym_func_void);
   FINDSYM(glsym_glBeginPerfMonitorAMD, "glBeginPerfMonitorAMD", glsym_func_void);
   FINDSYM(glsym_glEndPerfMonitorAMD, "glEndPerfMonitorAMD", glsym_func_void);
   FINDSYM(glsym_glGetPerfMonitorCounterDataAMD, "glGetPerfMonitorCounterDataAMD", glsym_func_void);

   if (glsym_glGetPerfMonitorGroupsAMD) _gl_ext_entries[3].supported = 1;

   /* GL_EXT_discard_framebuffer */
   FINDSYM(glsym_glDiscardFramebufferEXT, "glDiscardFramebuffer", glsym_func_void);
   FINDSYM(glsym_glDiscardFramebufferEXT, "glDiscardFramebufferARB", glsym_func_void);
   FINDSYM(glsym_glDiscardFramebufferEXT, "glDiscardFramebufferEXT", glsym_func_void);

   if (glsym_glDiscardFramebufferEXT) _gl_ext_entries[4].supported = 1;

   /* GL_EXT_multi_draw_arrays */
   FINDSYM(glsym_glMultiDrawArraysEXT, "glMultiDrawArrays", glsym_func_void);
   FINDSYM(glsym_glMultiDrawArraysEXT, "glMultiDrawArraysARB", glsym_func_void);
   FINDSYM(glsym_glMultiDrawArraysEXT, "glMultiDrawArraysEXT", glsym_func_void);

   FINDSYM(glsym_glMultiDrawElementsEXT, "glMultiDrawElements", glsym_func_void);
   FINDSYM(glsym_glMultiDrawElementsEXT, "glMultiDrawElementsARB", glsym_func_void);
   FINDSYM(glsym_glMultiDrawElementsEXT, "glMultiDrawElementsEXT", glsym_func_void);

   if (glsym_glMultiDrawArraysEXT) _gl_ext_entries[5].supported = 1;

   /* GL_NV_fence */
   FINDSYM(glsym_glDeleteFencesNV, "glDeleteFencesNV", glsym_func_void);
   FINDSYM(glsym_glGenFencesNV, "glGenFencesNV", glsym_func_void);
   FINDSYM(glsym_glIsFenceNV, "glIsFenceNV", glsym_func_uchar);
   FINDSYM(glsym_glTestFenceNV, "glTestFenceNV", glsym_func_uchar);
   FINDSYM(glsym_glGetFenceivNV, "glGetFenceivNV", glsym_func_void);
   FINDSYM(glsym_glFinishFenceNV, "glFinishFenceNV", glsym_func_void);
   FINDSYM(glsym_glSetFenceNV, "glSetFenceNV", glsym_func_void);

   if (glsym_glDeleteFencesNV) _gl_ext_entries[6].supported = 1;

   /* GL_QCOM_driver_control */
   FINDSYM(glsym_glGetDriverControlsQCOM, "glGetDriverControlsQCOM", glsym_func_void);
   FINDSYM(glsym_glGetDriverControlStringQCOM, "glGetDriverControlStringQCOM", glsym_func_void);
   FINDSYM(glsym_glEnableDriverControlQCOM, "glEnableDriverControlQCOM", glsym_func_void);
   FINDSYM(glsym_glDisableDriverControlQCOM, "glDisableDriverControlQCOM", glsym_func_void);

   if (glsym_glGetDriverControlsQCOM) _gl_ext_entries[7].supported = 1;

   /* GL_QCOM_extended_get */
   FINDSYM(glsym_glExtGetTexturesQCOM, "glExtGetTexturesQCOM", glsym_func_void);
   FINDSYM(glsym_glExtGetBuffersQCOM, "glExtGetBuffersQCOM", glsym_func_void);
   FINDSYM(glsym_glExtGetRenderbuffersQCOM, "glExtGetRenderbuffersQCOM", glsym_func_void);
   FINDSYM(glsym_glExtGetFramebuffersQCOM, "glExtGetFramebuffersQCOM", glsym_func_void);
   FINDSYM(glsym_glExtGetTexLevelParameterivQCOM, "glExtGetTexLevelParameterivQCOM", glsym_func_void);
   FINDSYM(glsym_glExtTexObjectStateOverrideiQCOM, "glExtTexObjectStateOverrideiQCOM", glsym_func_void);
   FINDSYM(glsym_glExtGetTexSubImageQCOM, "glExtGetTexSubImageQCOM", glsym_func_void);
   FINDSYM(glsym_glExtGetBufferPointervQCOM, "glExtGetBufferPointervQCOM", glsym_func_void);

   if (glsym_glExtGetTexturesQCOM) _gl_ext_entries[8].supported = 1;

   /* GL_QCOM_extended_get2 */
   FINDSYM(glsym_glExtGetShadersQCOM, "glExtGetShadersQCOM", glsym_func_void);
   FINDSYM(glsym_glExtGetProgramsQCOM, "glExtGetProgramsQCOM", glsym_func_void);
   FINDSYM(glsym_glExtIsProgramBinaryQCOM, "glExtIsProgramBinaryQCOM", glsym_func_uchar);
   FINDSYM(glsym_glExtGetProgramBinarySourceQCOM, "glExtGetProgramBinarySourceQCOM", glsym_func_void);

   if (glsym_glExtGetShadersQCOM) _gl_ext_entries[9].supported = 1;
}

static void
_extensions_init(Render_Engine *re)
{
   int i;
   const char *glexts, *evasglexts;

   memset(_gl_ext_string, 0, 1024);
   memset(_evasgl_ext_string, 0, 1024);

   // GLES 2.0 Extensions
   glexts = (const char*)glGetString(GL_EXTENSIONS);

   DBG("--------GLES 2.0 Extensions--------");
   for (i = 0; _gl_ext_entries[i].name != NULL; i++)
     {
        if ( (strstr(glexts, _gl_ext_entries[i].name) != NULL) ||
             (strstr(glexts, _gl_ext_entries[i].real_name) != NULL) )
          {
             _gl_ext_entries[i].supported = 1;
             strcat(_gl_ext_string, _gl_ext_entries[i].name);
             strcat(_gl_ext_string, " ");
             DBG("\t%s", _gl_ext_entries[i].name);
          }

     }
   DBG(" ");

   // EGL Extensions
   evasglexts = glsym_eglQueryString(re->win->egl_disp, EGL_EXTENSIONS);

   DBG("--------EvasGL Extensions----------");
   for (i = 0; _evasgl_ext_entries[i].name != NULL; i++)
     {
        if ( (strstr(evasglexts, _evasgl_ext_entries[i].name) != NULL) ||
             (strstr(evasglexts, _evasgl_ext_entries[i].real_name) != NULL) )
          {
             _evasgl_ext_entries[i].supported = 1;
             strcat(_evasgl_ext_string, _evasgl_ext_entries[i].name);
             strcat(_evasgl_ext_string, " ");
             DBG("\t%s", _evasgl_ext_entries[i].name);
          }
     }
   DBG(" ");
}

int _evas_engine_wl_egl_log_dom = -1;

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* Function table for GL APIs */
static Evas_GL_API gl_funcs;

static void *
eng_info(Evas *e __UNUSED__)
{
   Evas_Engine_Info_Wayland_Egl *info;

   info = calloc(1, sizeof(Evas_Engine_Info_Wayland_Egl));
   info->magic.magic = rand();
   info->func.best_depth_get = eng_best_depth_get;
   info->render_mode = EVAS_RENDER_MODE_BLOCKING;

   return info;
}

static void
eng_info_free(Evas *e __UNUSED__, void *info)
{
   Evas_Engine_Info_Wayland_Egl *in;
// dont free! why bother? its not worth it
//   eina_log_domain_unregister(_evas_engine_GL_X11_log_dom);
   in = (Evas_Engine_Info_Wayland_Egl *)info;
   free(in);
}

static int
_re_wincheck(Render_Engine *re)
{
   if (!re) return 0;
   if (re->win->surf) return 1;
   eng_window_resurf(re->win);
   if (!re->win->surf)
     {
        ERR("GL engine can't re-create window surface!");
     }
   return 0;
}

static void
_re_winfree(Render_Engine *re)
{
   if (!re->win->surf) return;
   eng_window_unsurf(re->win);
}

static Render_Engine_GL_Resource *
_create_internal_glue_resources(void *data)
{
   Render_Engine *re;
   Render_Engine_GL_Resource *rsc;

   if (!(re = (Render_Engine *)data)) return NULL;

   rsc = calloc(1, sizeof(Render_Engine_GL_Resource));
   if (!rsc) return NULL;

   // EGL
   int context_attrs[3];
   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;

   // Create resource surface for EGL
   rsc->surface = 
     eglCreateWindowSurface(re->win->egl_disp, re->win->egl_config,
                            (EGLNativeWindowType)re->win->win, NULL);
   if (!rsc->surface)
     {
        ERR("Creating internal resource surface failed.");
        free(rsc);
        return NULL;
     }

   // Create a resource context for EGL
   rsc->context = 
     eglCreateContext(re->win->egl_disp, re->win->egl_config,
                      re->win->egl_context[0], context_attrs);
   if (!rsc->context)
     {
        ERR("Internal Resource Context Creations Failed.");
        free(rsc);
        return NULL;
     }

   // Add to the resource resource list for cleanup
   LKL(resource_lock);
   resource_list = eina_list_prepend(resource_list, rsc);
   LKU(resource_lock);

   // Set the resource in TLS
   if (eina_tls_set(resource_key, (void*)rsc) == EINA_FALSE)
     {
        ERR("Failed setting TLS Resource");
        free(rsc);
        return NULL;
     }

   return rsc;
}

static int
_destroy_internal_glue_resources(void *data)
{
   Render_Engine *re = (Render_Engine *)data;
   Eina_List *l;
   Render_Engine_GL_Resource *rsc;

   // EGL
   // Delete the Resources
   LKL(resource_lock);
   EINA_LIST_FOREACH(resource_list, l, rsc)
     {
        if (rsc->surface) eglDestroySurface(re->win->egl_disp, rsc->surface);
        if (rsc->context) eglDestroyContext(re->win->egl_disp, rsc->context);
        free(rsc);
     }
   eina_list_free(resource_list);
   resource_list = NULL;
   LKU(resource_lock);

   // Destroy TLS
   eina_tls_free(resource_key);

   return 1;
}

static int
eng_setup(Evas *e, void *in)
{
   Render_Engine *re;
   Evas_Engine_Info_Wayland_Egl *info;
   Evas_GL_Wl_Window *new_win = NULL;

   info = (Evas_Engine_Info_Wayland_Egl *)in;
   if (!e->engine.data.output)
     {
        re = calloc(1, sizeof(Render_Engine));
        if (!re) return 0;
        re->info = info;
        re->evas = e;
        e->engine.data.output = re;
        re->w = e->output.w;
        re->h = e->output.h;

        re->win = eng_window_new(re->info->info.display, 
                                 re->info->info.surface, 
                                 re->info->info.screen,
                                 re->info->info.depth, re->w, re->h,
                                 re->info->indirect,
                                 re->info->info.destination_alpha,
                                 re->info->info.rotation);
        if (!re->win)
          {
             free(re);
             e->engine.data.output = NULL;
             return 0;
          }

        gl_wins++;
        if (!initted)
          {
             evas_common_cpu_init();
             evas_common_blend_init();
             evas_common_image_init();
             evas_common_convert_init();
             evas_common_scale_init();
             evas_common_rectangle_init();
             evas_common_polygon_init();
             evas_common_line_init();
             evas_common_font_init();
             evas_common_draw_init();
             evas_common_tilebuf_init();

             // Initialize TLS
             if (eina_tls_new(&resource_key) == EINA_FALSE)
               ERR("Error creating tls key");

             DBG("TLS KEY create... %d", resource_key);

             initted = 1;
          }
     }
   else
     {
        if (!(re = e->engine.data.output)) return 0;
        if (_re_wincheck(re))
          {
             if ((re->info->info.display != re->win->disp) ||
                 (re->info->info.surface != re->win->surface) ||
                 (re->info->info.screen != re->win->screen) ||
                 (re->info->info.depth != re->win->depth) ||
                 (re->info->info.destination_alpha != re->win->alpha) ||
                 (re->info->info.rotation != re->win->rot))
               {
                  int inc = 0;

                  /* if we already have a window surface, check for NULL input surface.
                   * this will mean we are hiding the window and should destroy 
                   * things properly */
                  if ((re->win->surface) && (re->info->info.surface == NULL))
                    {
                       if (re->win)
                         {
                            eng_window_free(re->win);
                            gl_wins--;
                            re->win = NULL;
                         }
                       return 1;
                    }

                  new_win = eng_window_new(re->info->info.display,
                                           re->info->info.surface,
                                           re->info->info.screen,
                                           re->info->info.depth,
                                           e->output.w, e->output.h,
                                           re->info->indirect,
                                           re->info->info.destination_alpha,
                                           re->info->info.rotation);

                  if (new_win) 
                    {
                       // free old win
                       if (re->win)
                         {
                            re->win->gl_context->references++;
                            eng_window_free(re->win);
                            inc = 1;
                            gl_wins--;
                         }

                       re->win = new_win;
                       re->w = e->output.w;
                       re->h = e->output.h;

                       eng_window_use(re->win);
                       if (re->win) gl_wins++;
                       if ((re->win) && (inc))
                         re->win->gl_context->references--;
                    }
							 }
             else if ((re->win->w != e->output.w) ||
                      (re->win->h != e->output.h))
               {
                  re->w = e->output.w;
                  re->h = e->output.h;
                  re->win->w = e->output.w;
                  re->win->h = e->output.h;
                  eng_window_use(re->win);
                  evas_gl_common_context_resize(re->win->gl_context, 
                                                re->win->w, re->win->h, 
                                                re->win->rot);
               }
          }
     }

   if (!re->win)
     {
        free(re);
        e->engine.data.output = NULL;
        return 0;
     }

   if (!e->engine.data.output)
     {
        if (re->win)
          {
             eng_window_free(re->win);
             gl_wins--;
          }
        free(re);
        e->engine.data.output = NULL;
        return 0;
     }
   re->tb = evas_common_tilebuf_new(re->win->w, re->win->h);
   if (!re->tb)
     {
        if (re->win)
          {
             eng_window_free(re->win);
             gl_wins--;
          }
        free(re);
        e->engine.data.output = NULL;
        return 0;
     }
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   if (!e->engine.data.context)
     {
        e->engine.data.context =
          e->engine.func->context_new(e->engine.data.output);
     }

   eng_window_use(re->win);

   re->vsync = 0;
   _sym_init();
   _extensions_init(re);

   // This is used in extensions.  Not pretty but can't get display otherwise.
   current_engine = re;

   return 1;
}

static void
eng_output_free(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;

   if (re)
     {
#if 0
        // Destroy the resource surface
        // Only required for EGL case
        if (re->surface)
           eglDestroySurface(re->win->egl_disp, re->surface);

        // Destroy the resource context
        _destroy_internal_context(re, context);
#endif
        if (re->win)
          {
             if ((initted == 1) && (gl_wins == 1))
               _destroy_internal_glue_resources(re);
             eng_window_free(re->win);
             gl_wins--;
          }
        evas_common_tilebuf_free(re->tb);
        free(re);
     }
   if ((initted == 1) && (gl_wins == 0))
     {
        evas_common_image_shutdown();
        evas_common_font_shutdown();
        initted = 0;
     }
}

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   re->win->w = w;
   re->win->h = h;
   eng_window_use(re->win);

   if (re->win->win)
     {
        int aw, ah, dx, dy;

        wl_egl_window_get_attached_size(re->win->win, &aw, &ah);

        if (re->info->info.edges & 4) // resize from left
          dx = aw - w;
        else
          dx = 0;
        if (re->info->info.edges & 1) // resize from top
          dy = ah - h;
        else
          dy = 0;

        wl_egl_window_resize(re->win->win, w, h, dx, dy);
     }

   evas_gl_common_context_resize(re->win->gl_context, w, h, re->win->rot);
   evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb)
     evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
}

static void
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if ((!re) || (!re->win)) return;

   eng_window_use(re->win);
   evas_gl_common_context_resize(re->win->gl_context, re->win->w, re->win->h, re->win->rot);
   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);

   RECTS_CLIP_TO_RECT(x, y, w, h, 0, 0, re->win->w, re->win->h);
   if ((w <= 0) || (h <= 0)) return;
   if (!re->win->draw.redraw)
     {
#if 1
	re->win->draw.x1 = x;
	re->win->draw.y1 = y;
	re->win->draw.x2 = x + w - 1;
	re->win->draw.y2 = y + h - 1;
#else
	re->win->draw.x1 = 0;
	re->win->draw.y1 = 0;
	re->win->draw.x2 = re->win->w - 1;
	re->win->draw.y2 = re->win->h - 1;
#endif
     }
   else
     {
	if (x < re->win->draw.x1) re->win->draw.x1 = x;
	if (y < re->win->draw.y1) re->win->draw.y1 = y;
	if ((x + w - 1) > re->win->draw.x2) re->win->draw.x2 = x + w - 1;
	if ((y + h - 1) > re->win->draw.y2) re->win->draw.y2 = y + h - 1;
     }
   re->win->draw.redraw = 1;
}

static void
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_tilebuf_clear(re->tb);
/*   re->win->draw.redraw = 0;*/
//   INF("GL: finish update cycle!");
}

/* vsync games - not for now though */
#define VSYNC_TO_SCREEN 1

static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;
   Tilebuf_Rect *rects;

   re = (Render_Engine *)data;
   /* get the upate rect surface - return engine data as dummy */
   rects = evas_common_tilebuf_get_render_rects(re->tb);
   if (rects)
     {
        evas_common_tilebuf_free_render_rects(rects);
        evas_common_tilebuf_clear(re->tb);
        eng_window_use(re->win);
        if (!_re_wincheck(re)) return NULL;
        evas_gl_common_context_flush(re->win->gl_context);
        evas_gl_common_context_newframe(re->win->gl_context);
        if (x) *x = 0;
        if (y) *y = 0;
        if (w) *w = re->win->w;
        if (h) *h = re->win->h;
        if (cx) *cx = 0;
        if (cy) *cy = 0;
        if (cw) *cw = re->win->w;
        if (ch) *ch = re->win->h;
        return re->win->gl_context->def_surface;
     }
   return NULL;
}

//#define FRAMECOUNT 1

#ifdef FRAMECOUNT
static double
get_time(void)
{
   struct timeval timev;

   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000);
}
#endif

static int safe_native = -1;

static void
eng_output_redraws_next_update_push(void *data, void *surface __UNUSED__, int x __UNUSED__, int y __UNUSED__, int w __UNUSED__, int h __UNUSED__)
{
   Render_Engine *re;
#ifdef FRAMECOUNT
   static double pt = 0.0;
   double ta, tb;
#endif

   re = (Render_Engine *)data;
   /* put back update surface.. in this case just unflag redraw */
   if (!_re_wincheck(re)) return;
   re->win->draw.redraw = 0;
   re->win->draw.drew = 1;
   evas_gl_common_context_flush(re->win->gl_context);
   if (safe_native == -1)
     {
        const char *s;

        s = getenv("EVAS_GL_SAFE_NATIVE");
        safe_native = 0;
        if (s) 
          safe_native = atoi(s);
        else
          {
             s = (const char *)glGetString(GL_RENDERER);
             if (s)
               {
                  if (strstr(s, "PowerVR SGX 540") || strstr(s, "Mali-400 MP"))
                    safe_native = 1;
               }
          }
     }
   // this is needed to make sure all previous rendering is flushed to
   // buffers/surfaces
# ifdef FRAMECOUNT
   double t0 = get_time();
   ta = t0 - pt;
   pt = t0;
# endif
   // previous rendering should be done and swapped
   if (!safe_native) eglWaitNative(EGL_CORE_NATIVE_ENGINE);
# ifdef FRAMECOUNT
   double t1 = get_time();
   tb = t1 - t0;
   printf("... %1.5f -> %1.5f | ", ta, tb);
# endif
   if (eglGetError() != EGL_SUCCESS)
     {
        printf("Error:  eglWaitNative(EGL_CORE_NATIVE_ENGINE) fail.\n");
     }
}

static void
eng_output_flush(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!_re_wincheck(re)) return;
   if (!re->win->draw.drew) return;
//x//   printf("frame -> flush\n");
   re->win->draw.drew = 0;
   eng_window_use(re->win);

# ifdef FRAMECOUNT
   double t0 = get_time();
# endif

   if (!re->vsync)
     {
        if (re->info->vsync) eglSwapInterval(re->win->egl_disp, 1);
        else eglSwapInterval(re->win->egl_disp, 0);
        re->vsync = 1;
     }

   if (re->info->callback.pre_swap)
     re->info->callback.pre_swap(re->info->callback.data, re->evas);

   eglSwapBuffers(re->win->egl_disp, re->win->egl_surface[0]);
   if (!safe_native) eglWaitGL();

   if (re->info->callback.post_swap)
     re->info->callback.post_swap(re->info->callback.data, re->evas);

   if (eglGetError() != EGL_SUCCESS)
     printf("Error:  eglSwapBuffers() fail.\n");

# ifdef FRAMECOUNT
   double t1 = get_time();
   printf("%1.5f\n", t1 - t0);
# endif
}

static void
eng_output_idle_flush(void *data __UNUSED__)
{
   /* Render_Engine *re; */

   /* re = (Render_Engine *)data; */
}

static void
eng_output_dump(void *data)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
   evas_gl_common_image_all_unload(re->win->gl_context);
   _re_winfree(re);
}

static void
eng_context_cutout_add(void *data __UNUSED__, void *context, int x, int y, int w, int h)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
//   re->win->gl_context->dc = context;
   evas_common_draw_context_add_cutout(context, x, y, w, h);
}

static void
eng_context_cutout_clear(void *data __UNUSED__, void *context)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
//   re->win->gl_context->dc = context;
   evas_common_draw_context_clear_cutouts(context);
}

static void
eng_rectangle_draw(void *data, void *context, void *surface, int x, int y, int w, int h)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_rect_draw(re->win->gl_context, x, y, w, h);
}

static void
eng_line_draw(void *data, void *context, void *surface, int x1, int y1, int x2, int y2)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_line_draw(re->win->gl_context, x1, y1, x2, y2);
}

static void *
eng_polygon_point_add(void *data __UNUSED__, void *context __UNUSED__, void *polygon, int x, int y)
{
   /* Render_Engine *re; */

   /* re = (Render_Engine *)data; */
   return evas_gl_common_poly_point_add(polygon, x, y);
}

static void *
eng_polygon_points_clear(void *data __UNUSED__, void *context __UNUSED__, void *polygon)
{
   /* Render_Engine *re; */

   /* re = (Render_Engine *)data; */
   return evas_gl_common_poly_points_clear(polygon);
}

static void
eng_polygon_draw(void *data, void *context, void *surface __UNUSED__, void *polygon, int x, int y)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_poly_draw(re->win->gl_context, polygon, x, y);
}

static int
eng_image_alpha_get(void *data __UNUSED__, void *image)
{
//   Render_Engine *re;
   Evas_GL_Image *im;

//   re = (Render_Engine *)data;
   if (!image) return 1;
   im = image;
   return im->alpha;
}

static int
eng_image_colorspace_get(void *data __UNUSED__, void *image)
{
//   Render_Engine *re;
   Evas_GL_Image *im;

//   re = (Render_Engine *)data;
   if (!image) return EVAS_COLORSPACE_ARGB8888;
   im = image;
   return im->cs.space;
}

static void
eng_image_mask_create(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *im;

   if (!image) return;
   im = image;
   if (!im->im->image.data)
      evas_cache_image_load_data(&im->im->cache_entry);
   if (!im->tex)
      im->tex = evas_gl_common_texture_new(im->gc, im->im);
}


static void *
eng_image_alpha_set(void *data, void *image, int has_alpha)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   im = image;
   if (im->alpha == has_alpha) return image;
   if (im->native.data)
     {
        im->alpha = has_alpha;
        return image;
     }
   eng_window_use(re->win);
   if ((im->tex) && (im->tex->pt->dyn.img))
     {
        im->alpha = has_alpha;
        im->tex->alpha = im->alpha;
        return image;
     }
   /* FIXME: can move to gl_common */
   if (im->cs.space != EVAS_COLORSPACE_ARGB8888) return im;
   if ((has_alpha) && (im->im->cache_entry.flags.alpha)) return image;
   else if ((!has_alpha) && (!im->im->cache_entry.flags.alpha)) return image;
   if (im->references > 1)
     {
        Evas_GL_Image *im_new;

        im_new = evas_gl_common_image_new_from_copied_data
           (im->gc, im->im->cache_entry.w, im->im->cache_entry.h,
               im->im->image.data,
               eng_image_alpha_get(data, image),
               eng_image_colorspace_get(data, image));
        if (!im_new) return im;
        evas_gl_common_image_free(im);
        im = im_new;
     }
   else
     evas_gl_common_image_dirty(im, 0, 0, 0, 0);
   return evas_gl_common_image_alpha_set(im, has_alpha ? 1 : 0);
//   im->im->cache_entry.flags.alpha = has_alpha ? 1 : 0;
//   return image;
}

static void *
eng_image_border_set(void *data __UNUSED__, void *image, int l __UNUSED__, int r __UNUSED__, int t __UNUSED__, int b __UNUSED__)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
   return image;
}

static void
eng_image_border_get(void *data __UNUSED__, void *image __UNUSED__, int *l __UNUSED__, int *r __UNUSED__, int *t __UNUSED__, int *b __UNUSED__)
{
//   Render_Engine *re;
//
//   re = (Render_Engine *)data;
}

static char *
eng_image_comment_get(void *data __UNUSED__, void *image, char *key __UNUSED__)
{
//   Render_Engine *re;
   Evas_GL_Image *im;

//   re = (Render_Engine *)data;
   if (!image) return NULL;
   im = image;
   if (!im->im) return NULL;
   return im->im->info.comment;
}

static char *
eng_image_format_get(void *data __UNUSED__, void *image __UNUSED__)
{
//   Render_Engine *re;
   /* Evas_GL_Image *im; */

//   re = (Render_Engine *)data;
   /* im = image; */
   return NULL;
}

static void
eng_image_colorspace_set(void *data, void *image, int cspace)
{
   Render_Engine *re;
   Evas_GL_Image *im;

   re = (Render_Engine *)data;
   if (!image) return;
   im = image;
   if (im->native.data) return;
   /* FIXME: can move to gl_common */
   if (im->cs.space == cspace) return;
   eng_window_use(re->win);
   evas_cache_image_colorspace(&im->im->cache_entry, cspace);
   switch (cspace)
     {
      case EVAS_COLORSPACE_ARGB8888:
         if (im->cs.data)
           {
              if (!im->cs.no_free) free(im->cs.data);
              im->cs.data = NULL;
              im->cs.no_free = 0;
           }
         break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         if (im->tex) evas_gl_common_texture_free(im->tex);
         im->tex = NULL;
         if (im->cs.data)
           {
              if (!im->cs.no_free) free(im->cs.data);
           }
         if (im->im->cache_entry.h > 0)
           im->cs.data =
              calloc(1, im->im->cache_entry.h * sizeof(unsigned char *) * 2);
         else
           im->cs.data = NULL;
         im->cs.no_free = 0;
         break;
      default:
         abort();
         break;
     }
   im->cs.space = cspace;
}

/////////////////////////////////////////////////////////////////////////
//
//
typedef struct _Native Native;

struct _Native
{
   Evas_Native_Surface ns;
   struct wl_egl_pixmap *pixmap;

   void *egl_surface;
};

// FIXME: this is enabled so updates happen - but its SLOOOOOOOOOOOOOOOW
// (i am sure this is the reason)  not to mention seemingly superfluous. but
// i need to enable it for it to work on fglrx at least. havent tried nvidia.
//
// why is this the case? does anyone know? has anyone tried it on other gfx
// drivers?
//
//#define GLX_TEX_PIXMAP_RECREATE 1

static void *
eng_image_native_set(void *data, void *image, void *native)
{
   Render_Engine *re = (Render_Engine *)data;
   Evas_Native_Surface *ns = native;
   Evas_GL_Image *im = image, *im2 = NULL;
   Native *n = NULL;

  if (!im)
    {
       if ((!ns) && (ns->type == EVAS_NATIVE_SURFACE_OPENGL))
         {
            im = evas_gl_common_image_new_from_data(re->win->gl_context,
                                                    ns->data.opengl.w,
                                                    ns->data.opengl.h,
                                                    NULL, 1,
                                                    EVAS_COLORSPACE_ARGB8888);
         }
       else
           return NULL;
    }

  if (ns)
    {
       if (im->native.data) return im;
    }
  if ((!ns) && (!im->native.data)) return im;

  eng_window_use(re->win);

  if (im->native.data)
    {
      if (im->native.func.free)
        im->native.func.free(im->native.func.data, im);
      evas_gl_common_image_native_disable(im);
    }

  if (!ns) return im;

   if (im2 == im) return im;
   if (im2)
     {
        n = im2->native.data;
        if (n)
          {
             evas_gl_common_image_ref(im2);
             evas_gl_common_image_free(im);
             return im2;
          }
     }

   im2 = evas_gl_common_image_new_from_data(re->win->gl_context,
                                            im->w, im->h, NULL, im->alpha,
                                            EVAS_COLORSPACE_ARGB8888);
   evas_gl_common_image_free(im);
   im = im2;

   return im;
}

static void *
eng_image_native_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *im;
   Native *n;

   if (!(im = image)) return NULL;
   if (!(n = im->native.data)) return NULL;
   return &(n->ns);
}

#if 0 // filtering disabled
static void
eng_image_draw_filtered(void *data, void *context, void *surface,
                        void *image, Evas_Filter_Info *filter)
{
   Render_Engine *re = data;

   if (!image) return;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;

   evas_gl_common_filter_draw(re->win->gl_context, image, filter);
}

static Filtered_Image *
eng_image_filtered_get(void *im, uint8_t *key, size_t keylen)
{
   return evas_gl_common_image_filtered_get(im, key, keylen);
}

static Filtered_Image *
eng_image_filtered_save(void *im, void *fim, uint8_t *key, size_t keylen)
{
   return evas_gl_common_image_filtered_save(im, fim, key, keylen);
}

static void
eng_image_filtered_free(void *im, Filtered_Image *fim)
{
   evas_gl_common_image_filtered_free(im, fim);
}
#endif

static void *
eng_image_load(void *data, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   *error = EVAS_LOAD_ERROR_NONE;
   eng_window_use(re->win);
   return evas_gl_common_image_load(re->win->gl_context, file, key, lo, error);
}

static void *
eng_image_new_from_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_data(re->win->gl_context, w, h, image_data, alpha, cspace);
}

static void *
eng_image_new_from_copied_data(void *data, int w, int h, DATA32 *image_data, int alpha, int cspace)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   return evas_gl_common_image_new_from_copied_data(re->win->gl_context, w, h, image_data, alpha, cspace);
}

static void
eng_image_free(void *data, void *image)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return;
   eng_window_use(re->win);
}

static void
eng_image_size_get(void *data __UNUSED__, void *image, int *w, int *h)
{
   if (!image)
     {
        *w = 0;
        *h = 0;
        return;
     }
   if (w) *w = ((Evas_GL_Image *)image)->w;
   if (h) *h = ((Evas_GL_Image *)image)->h;
}

static void *
eng_image_size_set(void *data, void *image, int w, int h)
{
   Render_Engine *re;
   Evas_GL_Image *im = image;
   Evas_GL_Image *im_old;

   re = (Render_Engine *)data;
   if (!im) return NULL;
   if (im->native.data)
     {
        im->w = w;
        im->h = h;
        return image;
     }
   eng_window_use(re->win);
   if ((im->tex) && (im->tex->pt->dyn.img))
     {
        evas_gl_common_texture_free(im->tex);
        im->tex = NULL;
        im->w = w;
        im->h = h;
        im->tex = evas_gl_common_texture_dynamic_new(im->gc, im);
        return image;
     }
   im_old = image;

   switch (eng_image_colorspace_get(data, image))
     {
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         w &= ~0x1;
         break;
     }

   if ((im_old) &&
       ((int)im_old->im->cache_entry.w == w) &&
       ((int)im_old->im->cache_entry.h == h))
     return image;
   if (im_old)
     {
        im = evas_gl_common_image_new(re->win->gl_context, w, h,
                                      eng_image_alpha_get(data, image),
                                      eng_image_colorspace_get(data, image));
        /*
	evas_common_load_image_data_from_file(im_old->im);
	if (im_old->im->image->data)
	  {
	     evas_common_blit_rectangle(im_old->im, im->im, 0, 0, w, h, 0, 0);
	     evas_common_cpu_end_opt();
	  }
         */
        evas_gl_common_image_free(im_old);
     }
   else
     im = evas_gl_common_image_new(re->win->gl_context, w, h, 1, EVAS_COLORSPACE_ARGB8888);
   return im;
}

static void *
eng_image_dirty_region(void *data, void *image, int x, int y, int w, int h)
{
   Render_Engine *re;
   Evas_GL_Image *im = image;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   if (im->native.data) return image;
   eng_window_use(re->win);
   evas_gl_common_image_dirty(image, x, y, w, h);
   return image;
}

static void *
eng_image_data_get(void *data, void *image, int to_write, DATA32 **image_data, int *err)
{
   Render_Engine *re;
   Evas_GL_Image *im;
   int error;

   re = (Render_Engine *)data;
   if (!image)
     {
        *image_data = NULL;
        if (err) *err = EVAS_LOAD_ERROR_GENERIC;
        return NULL;
     }
   im = image;
   if (im->native.data)
     {
        *image_data = NULL;
        if (err) *err = EVAS_LOAD_ERROR_NONE;
        return im;
     }

   eng_window_use(re->win);

   if ((im->tex) && (im->tex->pt) && (im->tex->pt->dyn.img) && (im->cs.space == EVAS_COLORSPACE_ARGB8888))
     {
        if (im->tex->pt->dyn.checked_out > 0)
          {
             im->tex->pt->dyn.checked_out++;
             *image_data = im->tex->pt->dyn.data;
             if (err) *err = EVAS_LOAD_ERROR_NONE;
             return im;
          }
        *image_data = im->tex->pt->dyn.data = glsym_eglMapImageSEC(re->win->egl_disp, im->tex->pt->dyn.img);

        if (!im->tex->pt->dyn.data)
          {
             if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             return im;
          }
        im->tex->pt->dyn.checked_out++;

        if (err) *err = EVAS_LOAD_ERROR_NONE;
        return im;
     }

   /* Engine can fail to create texture after cache drop like eng_image_content_hint_set function,
    so it is need to add code which check im->im's NULL value*/

   if (!im->im)
    {
       *image_data = NULL;
       if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
       return NULL;
    }

   error = evas_cache_image_load_data(&im->im->cache_entry);
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
         if (to_write)
           {
              if (im->references > 1)
                {
                   Evas_GL_Image *im_new;

                   im_new = evas_gl_common_image_new_from_copied_data
                      (im->gc, im->im->cache_entry.w, im->im->cache_entry.h,
                       im->im->image.data,
                       eng_image_alpha_get(data, image),
                       eng_image_colorspace_get(data, image));
                   if (!im_new)
                     {
                        *image_data = NULL;
                        if (err) *err = EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED;
                        return NULL;
                     }
                   evas_gl_common_image_free(im);
                   im = im_new;
                }
              else
                evas_gl_common_image_dirty(im, 0, 0, 0, 0);
           }
         *image_data = im->im->image.data;
         break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         *image_data = im->cs.data;
         break;
      default:
         abort();
         break;
     }
   if (err) *err = error;
   return im;
}

static void *
eng_image_data_put(void *data, void *image, DATA32 *image_data)
{
   Render_Engine *re;
   Evas_GL_Image *im, *im2;

   re = (Render_Engine *)data;
   if (!image) return NULL;
   im = image;
   if (im->native.data) return image;
   eng_window_use(re->win);
   if ((im->tex) && (im->tex->pt)
       && (im->tex->pt->dyn.data)
       && (im->cs.space == EVAS_COLORSPACE_ARGB8888))
     {
        int w, h;

        if (im->tex->pt->dyn.data == image_data)
          {
             im->tex->pt->dyn.checked_out--;
             if (im->tex->pt->dyn.checked_out == 0)
               glsym_eglUnmapImageSEC(re->win->egl_disp, im->tex->pt->dyn.img);
             return image;
          }

        w = im->im->cache_entry.w;
        h = im->im->cache_entry.h;
        im2 = eng_image_new_from_data(data, w, h, image_data,
                                      eng_image_alpha_get(data, image),
                                      eng_image_colorspace_get(data, image));
        if (!im2) return im;
        evas_gl_common_image_free(im);
        im = im2;
        evas_gl_common_image_dirty(im, 0, 0, 0, 0);
        return im;
     }
   switch (im->cs.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
         if (image_data != im->im->image.data)
           {
              int w, h;

              w = im->im->cache_entry.w;
              h = im->im->cache_entry.h;
              im2 = eng_image_new_from_data(data, w, h, image_data,
                                            eng_image_alpha_get(data, image),
                                            eng_image_colorspace_get(data, image));
              if (!im2) return im;
              evas_gl_common_image_free(im);
              im = im2;
           }
         break;
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
      case EVAS_COLORSPACE_YCBCR422601_PL:
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         if (image_data != im->cs.data)
           {
              if (im->cs.data)
                {
                   if (!im->cs.no_free) free(im->cs.data);
                }
              im->cs.data = image_data;
           }
         evas_gl_common_image_dirty(im, 0, 0, 0, 0);
         break;
      default:
         abort();
         break;
     }
   return im;
}

static void
eng_image_data_preload_request(void *data __UNUSED__, void *image, const void *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;
   evas_cache_image_preload_data(&im->cache_entry, target);
}

static void
eng_image_data_preload_cancel(void *data __UNUSED__, void *image, const void *target)
{
   Evas_GL_Image *gim = image;
   RGBA_Image *im;

   if (!gim) return;
   if (gim->native.data) return;
   im = (RGBA_Image *)gim->im;
   if (!im) return;
   evas_cache_image_preload_cancel(&im->cache_entry, target);
}

static void
eng_image_draw(void *data, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   evas_gl_common_image_draw(re->win->gl_context, image,
                             src_x, src_y, src_w, src_h,
                             dst_x, dst_y, dst_w, dst_h,
                             smooth);
}

static void
eng_image_scale_hint_set(void *data __UNUSED__, void *image, int hint)
{
   if (image) evas_gl_common_image_scale_hint_set(image, hint);
}

static int
eng_image_scale_hint_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *gim = image;
   if (!gim) return EVAS_IMAGE_SCALE_HINT_NONE;
   return gim->scale_hint;
}

static void
eng_image_map_draw(void *data, void *context, void *surface, void *image, RGBA_Map *m, int smooth, int level)
{
   Evas_GL_Image *gim = image;
   Render_Engine *re;

   re = (Render_Engine *)data;
   if (!image) return;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
   if (m->count != 4)
     {
        // FIXME: nash - you didn't fix this
        abort();
     }
   if ((m->pts[0].x == m->pts[3].x) &&
       (m->pts[1].x == m->pts[2].x) &&
       (m->pts[0].y == m->pts[1].y) &&
       (m->pts[3].y == m->pts[2].y) &&
       (m->pts[0].x <= m->pts[1].x) &&
       (m->pts[0].y <= m->pts[2].y) &&
       (m->pts[0].u == 0) &&
       (m->pts[0].v == 0) &&
       (m->pts[1].u == (gim->w << FP)) &&
       (m->pts[1].v == 0) &&
       (m->pts[2].u == (gim->w << FP)) &&
       (m->pts[2].v == (gim->h << FP)) &&
       (m->pts[3].u == 0) &&
       (m->pts[3].v == (gim->h << FP)) &&
       (m->pts[0].col == 0xffffffff) &&
       (m->pts[1].col == 0xffffffff) &&
       (m->pts[2].col == 0xffffffff) &&
       (m->pts[3].col == 0xffffffff))
     {
        int dx, dy, dw, dh;

        dx = m->pts[0].x >> FP;
        dy = m->pts[0].y >> FP;
        dw = (m->pts[2].x >> FP) - dx;
        dh = (m->pts[2].y >> FP) - dy;
        eng_image_draw(data, context, surface, image,
                       0, 0, gim->w, gim->h, dx, dy, dw, dh, smooth);
     }
   else
     {
        evas_gl_common_image_map_draw(re->win->gl_context, image, m->count, 
                                      &m->pts[0], smooth, level);
     }
}

static void *
eng_image_map_surface_new(void *data, int w, int h, int alpha)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   return evas_gl_common_image_surface_new(re->win->gl_context, w, h, alpha);
}

static void
eng_image_map_surface_free(void *data __UNUSED__, void *surface)
{
   evas_gl_common_image_free(surface);
}

static void
eng_image_content_hint_set(void *data __UNUSED__, void *image, int hint)
{
   if (image) evas_gl_common_image_content_hint_set(image, hint);
}

static int
eng_image_content_hint_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *gim = image;
   if (!gim) return EVAS_IMAGE_CONTENT_HINT_NONE;
   return gim->content_hint;
}

static void
eng_image_cache_flush(void *data)
{
   Render_Engine *re;
   int tmp_size;

   re = (Render_Engine *)data;

   tmp_size = evas_common_image_get_cache();
   evas_common_image_set_cache(0);
   evas_common_rgba_image_scalecache_flush();

   if ((re) && (re->win) && (re->win->gl_context))
     evas_gl_common_image_cache_flush(re->win->gl_context);
   evas_common_image_set_cache(tmp_size);
}

static void
eng_image_cache_set(void *data, int bytes)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   evas_common_image_set_cache(bytes);
   evas_common_rgba_image_scalecache_size_set(bytes);
   evas_gl_common_image_cache_flush(re->win->gl_context);
}

static int
eng_image_cache_get(void *data __UNUSED__)
{
   return evas_common_image_get_cache();
}

static void
eng_image_stride_get(void *data __UNUSED__, void *image, int *stride)
{
   Evas_GL_Image *im = image;

   if ((im->tex) && (im->tex->pt->dyn.img))
     *stride = im->tex->pt->dyn.stride;
   else
     *stride = im->w * 4;
}

static void
eng_font_draw(void *data, void *context, void *surface, Evas_Font_Set *font __UNUSED__, int x, int y, int w __UNUSED__, int h __UNUSED__, int ow __UNUSED__, int oh __UNUSED__, Evas_Text_Props *intl_props)
{
   Render_Engine *re;

   re = (Render_Engine *)data;
   eng_window_use(re->win);
   evas_gl_common_context_target_surface_set(re->win->gl_context, surface);
   re->win->gl_context->dc = context;
     {
        // FIXME: put im into context so we can free it
        static RGBA_Image *im = NULL;

        if (!im)
          im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        im->cache_entry.w = re->win->w;
        im->cache_entry.h = re->win->h;
        evas_common_draw_context_font_ext_set(context,
                                              re->win->gl_context,
                                              evas_gl_font_texture_new,
                                              evas_gl_font_texture_free,
                                              evas_gl_font_texture_draw);
	evas_common_font_draw_prepare(intl_props);
        evas_common_font_draw(im, context, x, y, intl_props);
        evas_common_draw_context_font_ext_set(context, NULL, NULL, NULL, NULL);
     }
}

static Eina_Bool
eng_canvas_alpha_get(void *data, void *info __UNUSED__)
{
   Render_Engine *re = (Render_Engine *)data;

   if ((!re) || (!re->win)) return EINA_FALSE;
   return re->win->alpha;
}

static int
_set_internal_config(Render_Engine_GL_Surface *sfc, Evas_GL_Config *cfg)
{
   // Also initialize pixel format here as well...
   switch(cfg->color_format)
     {
      case EVAS_GL_RGB_888:
         sfc->rt_fmt          = GL_RGB;
         sfc->rt_internal_fmt = GL_RGB;
         break;
      case EVAS_GL_RGBA_8888:
         sfc->rt_fmt          = GL_RGBA;
         sfc->rt_internal_fmt = GL_RGBA;
         break;
      default:
         ERR("Invalid Color Format!");
         return 0;
     }

   switch(cfg->depth_bits)
     {
      case EVAS_GL_DEPTH_NONE:
         break;
      case EVAS_GL_DEPTH_BIT_8:
      case EVAS_GL_DEPTH_BIT_16:
      case EVAS_GL_DEPTH_BIT_24:
         // 24 bit doesn't work... just cover it with 16 for now..
         sfc->rb_depth_fmt = GL_DEPTH_COMPONENT16;
         break;
      case EVAS_GL_DEPTH_BIT_32:
      default:
         ERR("Unsupported Depth Bits Format!");
         return 0;
     }

   switch(cfg->stencil_bits)
     {
      case EVAS_GL_STENCIL_NONE:
         break;
      case EVAS_GL_STENCIL_BIT_1:
      case EVAS_GL_STENCIL_BIT_2:
      case EVAS_GL_STENCIL_BIT_4:
      case EVAS_GL_STENCIL_BIT_8:
         sfc->rb_stencil_fmt = GL_STENCIL_INDEX8;
         break;
      case EVAS_GL_STENCIL_BIT_16:
      default:
         ERR("Unsupported Stencil Bits Format!");
         return 0;
     }

   // Do Packed Depth24_Stencil8 Later...

   return 1;
}

static int
_create_rt_buffers(Render_Engine *data __UNUSED__,
                   Render_Engine_GL_Surface *sfc)
{
   // Render Target texture
   glGenTextures(1, &sfc->rt_tex );

   // Depth RenderBuffer - Create storage here...
   if (sfc->depth_bits != EVAS_GL_DEPTH_NONE)
      glGenRenderbuffers(1, &sfc->rb_depth);

   // Stencil RenderBuffer - Create Storage here...
   if (sfc->stencil_bits != EVAS_GL_STENCIL_NONE)
      glGenRenderbuffers(1, &sfc->rb_stencil);

   return 1;
}

static int
_attach_fbo_surface(Render_Engine *data __UNUSED__,
                    Render_Engine_GL_Surface *sfc,
                    Render_Engine_GL_Context *ctx)
{
   int fb_status;

   // Initialize Texture
   glBindTexture(GL_TEXTURE_2D, sfc->rt_tex );
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sfc->w, sfc->h, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   glBindTexture(GL_TEXTURE_2D, 0);


   // Attach texture to FBO
   glBindFramebuffer(GL_FRAMEBUFFER, ctx->context_fbo);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                          GL_TEXTURE_2D, sfc->rt_tex, 0);

   // Depth RenderBuffer - Attach it to FBO
   if (sfc->depth_bits != EVAS_GL_DEPTH_NONE)
     {
        glBindRenderbuffer(GL_RENDERBUFFER, sfc->rb_depth);
        glRenderbufferStorage(GL_RENDERBUFFER, sfc->rb_depth_fmt,
                              sfc->w, sfc->h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, sfc->rb_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
     }

   // Stencil RenderBuffer - Attach it to FBO
   if (sfc->stencil_bits != EVAS_GL_STENCIL_NONE)
     {
        glBindRenderbuffer(GL_RENDERBUFFER, sfc->rb_stencil);
        glRenderbufferStorage(GL_RENDERBUFFER, sfc->rb_stencil_fmt,
                              sfc->w, sfc->h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, sfc->rb_stencil);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
     }

   // Check FBO for completeness
   fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
   if (fb_status != GL_FRAMEBUFFER_COMPLETE)
     {
        ERR("FBO not complete!");
        return 0;
     }

   return 1;
}


static void *
eng_gl_surface_create(void *data, void *config, int w, int h)
{
   Render_Engine *re;
   Render_Engine_GL_Surface *sfc;
   Render_Engine_GL_Resource *rsc;
   Evas_GL_Config *cfg;
   int ret;

   sfc = calloc(1, sizeof(Render_Engine_GL_Surface));
   if (!sfc) return NULL;

   re  = (Render_Engine *)data;
   cfg = (Evas_GL_Config *)config;

   sfc->initialized  = 0;
   sfc->fbo_attached = 0;
   sfc->w            = w;
   sfc->h            = h;
   sfc->depth_bits   = cfg->depth_bits;
   sfc->stencil_bits = cfg->stencil_bits;
   sfc->rt_tex       = 0;
   sfc->rb_depth     = 0;
   sfc->rb_stencil   = 0;

   // Set the internal format based on the config
   if (!_set_internal_config(sfc, cfg))
     {
        ERR("Unsupported Format!");
        free(sfc);
        return NULL;
     }

   // Create internal resource context if it hasn't been created already
   if ((rsc = eina_tls_get(resource_key)) == NULL)
     {
        if ((rsc = _create_internal_glue_resources(re)) == NULL)
          {
             ERR("Error creating internal resources.");
             free(sfc);
             return NULL;
          }
     }

   // I'm using evas's original context to create the render target texture
   // This is to prevent awkwardness in using native_surface_get() function
   // If the rt texture creation is deferred till the context is created and
   // make_current called, the user can't call native_surface_get() right
   // after the surface is created. hence this is done here using evas' context.
   ret = eglMakeCurrent(re->win->egl_disp, rsc->surface, rsc->surface, rsc->context);
   if (!ret)
     {
        ERR("xxxMakeCurrent() failed!");
        free(sfc);
        return NULL;
     }

   // Create Render texture
   if (!_create_rt_buffers(re, sfc))
     {
        ERR("_create_rt_buffers() failed.");
        free(sfc);
        return NULL;
     }

   ret = eglMakeCurrent(re->win->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   if (!ret)
     {
        ERR("xxxMakeCurrent() failed!");
        free(sfc);
        return NULL;
     }

   return sfc;
}

static int
eng_gl_surface_destroy(void *data, void *surface)
{
   Render_Engine *re;
   Render_Engine_GL_Surface *sfc;
   Render_Engine_GL_Resource *rsc;
   int ret;

   re  = (Render_Engine *)data;
   sfc = (Render_Engine_GL_Surface*)surface;

   if (!sfc) return 0;

   if ((rsc = eina_tls_get(resource_key)) == EINA_FALSE) return 0;

   ret = eglMakeCurrent(re->win->egl_disp, rsc->surface, rsc->surface, rsc->context);
   if (!ret)
     {
        ERR("xxxMakeCurrent() failed!");
        return 0;
     }

   // Delete FBO/RBO and Texture here
   if (sfc->rt_tex)
      glDeleteTextures(1, &sfc->rt_tex);

   if (sfc->rb_depth)
      glDeleteRenderbuffers(1, &sfc->rb_depth);

   if (sfc->rb_stencil)
      glDeleteRenderbuffers(1, &sfc->rb_stencil);

   ret = eglMakeCurrent(re->win->egl_disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   if (!ret)
     {
        ERR("xxxMakeCurrent() failed!");
        free(sfc);
        return 0;
     }

   free(sfc);
   surface = NULL;

   return 1;
}

static void *
eng_gl_context_create(void *data, void *share_context)
{
   Render_Engine *re;
   Render_Engine_GL_Context *ctx;
   Render_Engine_GL_Context *share_ctx;
   int context_attrs[3];

   ctx = calloc(1, sizeof(Render_Engine_GL_Context));

   if (!ctx) return NULL;

   re = (Render_Engine *)data;
   share_ctx = (Render_Engine_GL_Context *)share_context;

   // Set the share context to Evas' GL context if share_context is NULL.
   // Otherwise set it to the given share_context.

   // EGL
   context_attrs[0] = EGL_CONTEXT_CLIENT_VERSION;
   context_attrs[1] = 2;
   context_attrs[2] = EGL_NONE;

   if (share_ctx)
     {
        ctx->context = eglCreateContext(re->win->egl_disp,
                                        re->win->egl_config,
                                        share_ctx->context,      // Share Context
                                        context_attrs);
     }
   else
     {
        ctx->context = eglCreateContext(re->win->egl_disp,
                                        re->win->egl_config,
                                        re->win->egl_context[0], // Evas' GL Context
                                        context_attrs);
     }

   if (!ctx->context)
     {
        ERR("eglCreateContext() fail. code=%#x", eglGetError());
        return NULL;
     }

   ctx->initialized = 0;
   ctx->context_fbo = 0;
   ctx->current_sfc = NULL;

   return ctx;
}

static int
eng_gl_context_destroy(void *data, void *context)
{
   Render_Engine *re;
   Render_Engine_GL_Context *ctx;
   Render_Engine_GL_Resource *rsc;
   int ret;

   re  = (Render_Engine *)data;
   ctx = (Render_Engine_GL_Context*)context;

   if (!ctx) return 0;

   if ((rsc = eina_tls_get(resource_key)) == EINA_FALSE) return 0;

   // 1. Do a make current with the given context
   ret = eglMakeCurrent(re->win->egl_disp, rsc->surface,
                        rsc->surface, ctx->context);
   if (!ret)
     {
        ERR("xxxMakeCurrent() failed!");
        return 0;
     }

   // 2. Delete the FBO
   if (ctx->context_fbo)
     glDeleteFramebuffers(1, &ctx->context_fbo);

   // 3. Destroy the Context
   eglDestroyContext(re->win->egl_disp, ctx->context);

   ctx->context = EGL_NO_CONTEXT;

   ret = eglMakeCurrent(re->win->egl_disp, EGL_NO_SURFACE,
                        EGL_NO_SURFACE, EGL_NO_CONTEXT);
   if (!ret)
     {
        ERR("xxxMakeCurrent() failed!");
        return 0;
     }

   if (current_evgl_ctx == ctx)
      current_evgl_ctx = NULL;

   free(ctx);
   context = NULL;

   return 1;
}

static int
eng_gl_make_current(void *data __UNUSED__, void *surface, void *context)
{
   Render_Engine *re;
   Render_Engine_GL_Surface *sfc;
   Render_Engine_GL_Context *ctx;
   int ret = 0;
   Render_Engine_GL_Resource *rsc;

   re  = (Render_Engine *)data;
   sfc = (Render_Engine_GL_Surface*)surface;
   ctx = (Render_Engine_GL_Context*)context;

   // Unset surface/context
   if ((!sfc) || (!ctx))
     {
        ret = eglMakeCurrent(re->win->egl_disp, EGL_NO_SURFACE,
                             EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (!ret)
          {
             ERR("xxxMakeCurrent() failed!");
             return 0;
          }

        if (ctx) ctx->current_sfc = NULL;
        if (sfc) sfc->current_ctx = NULL;
        current_evgl_ctx = NULL;
        return 1;
     }

   // Do a make current only if it's not already current
   if ((rsc = eina_tls_get(resource_key)) == EINA_FALSE) return 0;

   if ((eglGetCurrentContext() != ctx->context) ||
       (eglGetCurrentSurface(EGL_READ) != rsc->surface) ||
       (eglGetCurrentSurface(EGL_DRAW) != rsc->surface) )
     {
        // Flush remainder of what's in Evas' pipeline
        if (re->win) eng_window_use(NULL);

        // Do a make current
        ret = eglMakeCurrent(re->win->egl_disp, rsc->surface,
                             rsc->surface, ctx->context);
        if (!ret)
          {
             ERR("xxxMakeCurrent() failed!");
             return 0;
          }
     }

   // Create FBO if not already created
   if (!ctx->initialized)
     {
        glGenFramebuffers(1, &ctx->context_fbo);
        ctx->initialized = 1;
     }

   // Attach FBO if it hasn't been attached or if surface changed
   if ((!sfc->fbo_attached) || (ctx->current_sfc != sfc))
     {
        if (!_attach_fbo_surface(re, sfc, ctx))
          {
             ERR("_attach_fbo_surface() failed.");
             return 0;
          }

        if (ctx->current_fbo)
           // Bind to the previously bound buffer
           glBindFramebuffer(GL_FRAMEBUFFER, ctx->current_fbo);
        else
           // Bind FBO
           glBindFramebuffer(GL_FRAMEBUFFER, ctx->context_fbo);

        sfc->fbo_attached = 1;
     }

   // Set the current surface/context
   ctx->current_sfc = sfc;
   sfc->current_ctx = ctx;
   current_evgl_ctx = ctx;

   return 1;
}

static void *
eng_gl_string_query(void *data __UNUSED__, int name)
{
   switch(name)
     {
      case EVAS_GL_EXTENSIONS:
         return (void*)_evasgl_ext_string;
      default:
         return NULL;
     };
}

static void *
eng_gl_proc_address_get(void *data __UNUSED__, const char *name)
{
   if (glsym_eglGetProcAddress) return glsym_eglGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
}

static int
eng_gl_native_surface_get(void *data __UNUSED__, void *surface, void *native_surface)
{
   /* Render_Engine *re; */
   Render_Engine_GL_Surface *sfc;
   Evas_Native_Surface *ns;

   /* re  = (Render_Engine *)data; */
   sfc = (Render_Engine_GL_Surface*)surface;
   ns  = (Evas_Native_Surface*)native_surface;

   ns->type = EVAS_NATIVE_SURFACE_OPENGL;
   ns->version = EVAS_NATIVE_SURFACE_VERSION;
   ns->data.opengl.texture_id = sfc->rt_tex;
   ns->data.opengl.x = 0;
   ns->data.opengl.y = 0;
   ns->data.opengl.w = sfc->w;
   ns->data.opengl.h = sfc->h;

   return 1;
}


static const GLubyte *
evgl_glGetString(GLenum name)
{
   if (name == GL_EXTENSIONS)
     return (GLubyte *)_gl_ext_string; //glGetString(GL_EXTENSIONS); 
   else
     return glGetString(name);
}

static void
evgl_glBindFramebuffer(GLenum target, GLuint framebuffer)
{
   Render_Engine_GL_Context *ctx = current_evgl_ctx;

   // Take care of BindFramebuffer 0 issue
   if (framebuffer==0)
     {
        if (ctx)
          {
             glBindFramebuffer(target, ctx->context_fbo);
             ctx->current_fbo = 0;
          }
     }
   else
     {
        glBindFramebuffer(target, framebuffer);

        // Save this for restore when doing make current
        if (ctx)
           ctx->current_fbo = framebuffer;
     }
}

static void
evgl_glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
   // Add logic to take care when renderbuffer=0
   // On a second thought we don't need this
   glBindRenderbuffer(target, renderbuffer);
}

static void
evgl_glClearDepthf(GLclampf depth)
{
   glClearDepthf(depth);
}

static void
evgl_glDepthRangef(GLclampf zNear, GLclampf zFar)
{
   glDepthRangef(zNear, zFar);
}

static void
evgl_glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
   glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}

static void
evgl_glReleaseShaderCompiler(void)
{
   glReleaseShaderCompiler();
}

static void
evgl_glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length)
{
   glShaderBinary(n, shaders, binaryformat, binary, length);
}

//--------------------------------//
//#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
// EGL Extensions
static void *
evgl_evasglCreateImage(int target, void* buffer, int *attrib_list)
{
   if (current_engine)
     {
        return glsym_eglCreateImage(current_engine->win->egl_disp,
                                    EGL_NO_CONTEXT,
                                    target,
                                    buffer,
                                    attrib_list);
     }
   else
     {
        ERR("Invalid Engine... (Can't acccess EGL Display)\n");
        return NULL;
     }
}

static void
evgl_evasglDestroyImage(EvasGLImage image)
{
   if (current_engine)
     glsym_eglDestroyImage(current_engine->win->egl_disp, image);
   else
     ERR("Invalid Engine... (Can't acccess EGL Display)\n");
}

static void
evgl_glEvasGLImageTargetTexture2DOES(GLenum target, EvasGLImage image)
{
   glsym_glEGLImageTargetTexture2DOES(target, image);
}

static void
evgl_glEvasGLImageTargetRenderbufferStorageOES(GLenum target, EvasGLImage image)
{
   glsym_glEGLImageTargetTexture2DOES(target, image);
}

//--------------------------------//


static void *
eng_gl_api_get(void *data __UNUSED__)
{
   /* Render_Engine *re; */

   /* re  = (Render_Engine *)data; */

   gl_funcs.version = EVAS_GL_API_VERSION;

#define ORD(f) EVAS_API_OVERRIDE(f, &gl_funcs, )
   // GLES 2.0
   ORD(glActiveTexture);
   ORD(glAttachShader);
   ORD(glBindAttribLocation);
   ORD(glBindBuffer);
   ORD(glBindTexture);
   ORD(glBlendColor);
   ORD(glBlendEquation);
   ORD(glBlendEquationSeparate);
   ORD(glBlendFunc);
   ORD(glBlendFuncSeparate);
   ORD(glBufferData);
   ORD(glBufferSubData);
   ORD(glCheckFramebufferStatus);
   ORD(glClear);
   ORD(glClearColor);
//   ORD(glClearDepthf);
   ORD(glClearStencil);
   ORD(glColorMask);
   ORD(glCompileShader);
   ORD(glCompressedTexImage2D);
   ORD(glCompressedTexSubImage2D);
   ORD(glCopyTexImage2D);
   ORD(glCopyTexSubImage2D);
   ORD(glCreateProgram);
   ORD(glCreateShader);
   ORD(glCullFace);
   ORD(glDeleteBuffers);
   ORD(glDeleteFramebuffers);
   ORD(glDeleteProgram);
   ORD(glDeleteRenderbuffers);
   ORD(glDeleteShader);
   ORD(glDeleteTextures);
   ORD(glDepthFunc);
   ORD(glDepthMask);
//   ORD(glDepthRangef);
   ORD(glDetachShader);
   ORD(glDisable);
   ORD(glDisableVertexAttribArray);
   ORD(glDrawArrays);
   ORD(glDrawElements);
   ORD(glEnable);
   ORD(glEnableVertexAttribArray);
   ORD(glFinish);
   ORD(glFlush);
   ORD(glFramebufferRenderbuffer);
   ORD(glFramebufferTexture2D);
   ORD(glFrontFace);
   ORD(glGenBuffers);
   ORD(glGenerateMipmap);
   ORD(glGenFramebuffers);
   ORD(glGenRenderbuffers);
   ORD(glGenTextures);
   ORD(glGetActiveAttrib);
   ORD(glGetActiveUniform);
   ORD(glGetAttachedShaders);
   ORD(glGetAttribLocation);
   ORD(glGetBooleanv);
   ORD(glGetBufferParameteriv);
   ORD(glGetError);
   ORD(glGetFloatv);
   ORD(glGetFramebufferAttachmentParameteriv);
   ORD(glGetIntegerv);
   ORD(glGetProgramiv);
   ORD(glGetProgramInfoLog);
   ORD(glGetRenderbufferParameteriv);
   ORD(glGetShaderiv);
   ORD(glGetShaderInfoLog);
//   ORD(glGetShaderPrecisionFormat);
   ORD(glGetShaderSource);
//   ORD(glGetString);
   ORD(glGetTexParameterfv);
   ORD(glGetTexParameteriv);
   ORD(glGetUniformfv);
   ORD(glGetUniformiv);
   ORD(glGetUniformLocation);
   ORD(glGetVertexAttribfv);
   ORD(glGetVertexAttribiv);
   ORD(glGetVertexAttribPointerv);
   ORD(glHint);
   ORD(glIsBuffer);
   ORD(glIsEnabled);
   ORD(glIsFramebuffer);
   ORD(glIsProgram);
   ORD(glIsRenderbuffer);
   ORD(glIsShader);
   ORD(glIsTexture);
   ORD(glLineWidth);
   ORD(glLinkProgram);
   ORD(glPixelStorei);
   ORD(glPolygonOffset);
   ORD(glReadPixels);
//   ORD(glReleaseShaderCompiler);
   ORD(glRenderbufferStorage);
   ORD(glSampleCoverage);
   ORD(glScissor);
//   ORD(glShaderBinary);
   ORD(glShaderSource);
   ORD(glStencilFunc);
   ORD(glStencilFuncSeparate);
   ORD(glStencilMask);
   ORD(glStencilMaskSeparate);
   ORD(glStencilOp);
   ORD(glStencilOpSeparate);
   ORD(glTexImage2D);
   ORD(glTexParameterf);
   ORD(glTexParameterfv);
   ORD(glTexParameteri);
   ORD(glTexParameteriv);
   ORD(glTexSubImage2D);
   ORD(glUniform1f);
   ORD(glUniform1fv);
   ORD(glUniform1i);
   ORD(glUniform1iv);
   ORD(glUniform2f);
   ORD(glUniform2fv);
   ORD(glUniform2i);
   ORD(glUniform2iv);
   ORD(glUniform3f);
   ORD(glUniform3fv);
   ORD(glUniform3i);
   ORD(glUniform3iv);
   ORD(glUniform4f);
   ORD(glUniform4fv);
   ORD(glUniform4i);
   ORD(glUniform4iv);
   ORD(glUniformMatrix2fv);
   ORD(glUniformMatrix3fv);
   ORD(glUniformMatrix4fv);
   ORD(glUseProgram);
   ORD(glValidateProgram);
   ORD(glVertexAttrib1f);
   ORD(glVertexAttrib1fv);
   ORD(glVertexAttrib2f);
   ORD(glVertexAttrib2fv);
   ORD(glVertexAttrib3f);
   ORD(glVertexAttrib3fv);
   ORD(glVertexAttrib4f);
   ORD(glVertexAttrib4fv);
   ORD(glVertexAttribPointer);
   ORD(glViewport);
#undef ORD

#define ORD(f) EVAS_API_OVERRIDE(f, &gl_funcs, glsym_)
   // Extensions
   ORD(glGetProgramBinaryOES);
   ORD(glProgramBinaryOES);
   ORD(glMapBufferOES);
   ORD(glUnmapBufferOES);
   ORD(glGetBufferPointervOES);
   ORD(glTexImage3DOES);
   ORD(glTexSubImage3DOES);
   ORD(glCopyTexSubImage3DOES);
   ORD(glCompressedTexImage3DOES);
   ORD(glCompressedTexSubImage3DOES);
   ORD(glFramebufferTexture3DOES);
   ORD(glGetPerfMonitorGroupsAMD);
   ORD(glGetPerfMonitorCountersAMD);
   ORD(glGetPerfMonitorGroupStringAMD);
   ORD(glGetPerfMonitorCounterStringAMD);
   ORD(glGetPerfMonitorCounterInfoAMD);
   ORD(glGenPerfMonitorsAMD);
   ORD(glDeletePerfMonitorsAMD);
   ORD(glSelectPerfMonitorCountersAMD);
   ORD(glBeginPerfMonitorAMD);
   ORD(glEndPerfMonitorAMD);
   ORD(glGetPerfMonitorCounterDataAMD);
   ORD(glDiscardFramebufferEXT);
   ORD(glMultiDrawArraysEXT);
   ORD(glMultiDrawElementsEXT);
   ORD(glDeleteFencesNV);
   ORD(glGenFencesNV);
   ORD(glIsFenceNV);
   ORD(glTestFenceNV);
   ORD(glGetFenceivNV);
   ORD(glFinishFenceNV);
   ORD(glSetFenceNV);
   ORD(glGetDriverControlsQCOM);
   ORD(glGetDriverControlStringQCOM);
   ORD(glEnableDriverControlQCOM);
   ORD(glDisableDriverControlQCOM);
   ORD(glExtGetTexturesQCOM);
   ORD(glExtGetBuffersQCOM);
   ORD(glExtGetRenderbuffersQCOM);
   ORD(glExtGetFramebuffersQCOM);
   ORD(glExtGetTexLevelParameterivQCOM);
   ORD(glExtTexObjectStateOverrideiQCOM);
   ORD(glExtGetTexSubImageQCOM);
   ORD(glExtGetBufferPointervQCOM);
   ORD(glExtGetShadersQCOM);
   ORD(glExtGetProgramsQCOM);
   ORD(glExtIsProgramBinaryQCOM);
   ORD(glExtGetProgramBinarySourceQCOM);
#undef ORD

// Override functions wrapped by Evas_GL
#define ORD(f) EVAS_API_OVERRIDE(f, &gl_funcs, evgl_)
   ORD(glBindFramebuffer);
   ORD(glBindRenderbuffer);

   // GLES2.0 API compat on top of desktop gl
   ORD(glClearDepthf);
   ORD(glDepthRangef);
   ORD(glGetShaderPrecisionFormat);
   ORD(glReleaseShaderCompiler);
   ORD(glShaderBinary);

   ORD(glGetString);

   // GLES 2.0 Extensions that needs wrapping
   ORD(evasglCreateImage);
   ORD(evasglDestroyImage);
   ORD(glEvasGLImageTargetTexture2DOES);
   ORD(glEvasGLImageTargetRenderbufferStorageOES);

#undef ORD

   return &gl_funcs;
}

static int
eng_image_load_error_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *im;

   if (!image) return EVAS_LOAD_ERROR_NONE;
   im = image;
   return im->im->cache_entry.load_error;
}

static Eina_Bool
eng_image_animated_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return EINA_FALSE;
   im = (Image_Entry *)gim->im;
   if (!im) return EINA_FALSE;

   return im->flags.animated;
}

static int
eng_image_animated_frame_count_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return -1;
   im = (Image_Entry *)gim->im;
   if (!im) return -1;

   if (!im->flags.animated) return -1;
   return im->frame_count;
}

static Evas_Image_Animated_Loop_Hint
eng_image_animated_loop_type_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return EVAS_IMAGE_ANIMATED_HINT_NONE;
   im = (Image_Entry *)gim->im;
   if (!im) return EVAS_IMAGE_ANIMATED_HINT_NONE;

   if (!im->flags.animated) return EVAS_IMAGE_ANIMATED_HINT_NONE;
   return im->loop_hint;
}

static int
eng_image_animated_loop_count_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return -1;
   im = (Image_Entry *)gim->im;
   if (!im) return -1;

   if (!im->flags.animated) return -1;
   return im->loop_count;
}

static double
eng_image_animated_frame_duration_get(void *data __UNUSED__, void *image, int start_frame, int frame_num)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return -1;
   im = (Image_Entry *)gim->im;
   if (!im) return -1;

   if (!im->flags.animated) return -1;
   return evas_common_load_rgba_image_frame_duration_from_file(im, start_frame, frame_num);
}

static Eina_Bool
eng_image_animated_frame_set(void *data __UNUSED__, void *image, int frame_index)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;

   if (!gim) return EINA_FALSE;
   im = (Image_Entry *)gim->im;
   if (!im) return EINA_FALSE;

   if (!im->flags.animated) return EINA_FALSE;
   if (im->cur_frame == frame_index) return EINA_FALSE;

   im->cur_frame = frame_index;
   return EINA_TRUE;
}

static Eina_Bool
eng_image_can_region_get(void *data __UNUSED__, void *image)
{
   Evas_GL_Image *gim = image;
   Image_Entry *im;
   if (!gim) return EINA_FALSE;
   im = (Image_Entry *)gim->im;
   if (!im) return EINA_FALSE;
   return ((Evas_Image_Load_Func*) im->info.loader)->do_region;
}


static void
eng_image_max_size_get(void *data, int *maxw, int *maxh)
{
   Render_Engine *re = (Render_Engine *)data;
   if (maxw) *maxw = re->win->gl_context->shared->info.max_texture_size;
   if (maxh) *maxh = re->win->gl_context->shared->info.max_texture_size;
}

static int
module_open(Evas_Module *em)
{
   static Eina_Bool xrm_inited = EINA_FALSE;
   if (!xrm_inited)
     {
        xrm_inited = EINA_TRUE;
        XrmInitialize();
     }

   if (!em) return 0;
   if (!evas_gl_common_module_open()) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;
   if (_evas_engine_wl_egl_log_dom < 0)
     {
        _evas_engine_wl_egl_log_dom = 
          eina_log_domain_register("evas-wayland_egl", EVAS_DEFAULT_LOG_COLOR);
     }

   if (_evas_engine_wl_egl_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }
   /* store it for later use */
   func = pfunc;
   /* now to override methods */
   #define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(info);
   ORD(info_free);
   ORD(setup);
   ORD(canvas_alpha_get);
   ORD(output_free);
   ORD(output_resize);
   ORD(output_tile_size_set);
   ORD(output_redraws_rect_add);
   ORD(output_redraws_rect_del);
   ORD(output_redraws_clear);
   ORD(output_redraws_next_update_get);
   ORD(output_redraws_next_update_push);
   ORD(context_cutout_add);
   ORD(context_cutout_clear);
   ORD(output_flush);
   ORD(output_idle_flush);
   ORD(output_dump);
   ORD(rectangle_draw);
   ORD(line_draw);
   ORD(polygon_point_add);
   ORD(polygon_points_clear);
   ORD(polygon_draw);

   ORD(image_load);
   ORD(image_new_from_data);
   ORD(image_new_from_copied_data);
   ORD(image_free);
   ORD(image_size_get);
   ORD(image_size_set);
   ORD(image_dirty_region);
   ORD(image_data_get);
   ORD(image_data_put);
   ORD(image_data_preload_request);
   ORD(image_data_preload_cancel);
   ORD(image_alpha_set);
   ORD(image_alpha_get);
   ORD(image_border_set);
   ORD(image_border_get);
   ORD(image_draw);
   ORD(image_comment_get);
   ORD(image_format_get);
   ORD(image_colorspace_set);
   ORD(image_colorspace_get);
   ORD(image_can_region_get);
   ORD(image_mask_create);
   ORD(image_native_set);
   ORD(image_native_get);
#if 0 // filtering disabled
   ORD(image_draw_filtered);
   ORD(image_filtered_get);
   ORD(image_filtered_save);
   ORD(image_filtered_free);
#endif

   ORD(font_draw);

   ORD(image_scale_hint_set);
   ORD(image_scale_hint_get);
   ORD(image_stride_get);

   ORD(image_map_draw);
   ORD(image_map_surface_new);
   ORD(image_map_surface_free);

   ORD(image_content_hint_set);
   ORD(image_content_hint_get);

   ORD(image_cache_flush);
   ORD(image_cache_set);
   ORD(image_cache_get);

   ORD(gl_surface_create);
   ORD(gl_surface_destroy);
   ORD(gl_context_create);
   ORD(gl_context_destroy);
   ORD(gl_make_current);
   ORD(gl_string_query);
   ORD(gl_proc_address_get);
   ORD(gl_native_surface_get);
   ORD(gl_api_get);

   ORD(image_load_error_get);

   /* now advertise out own api */
   ORD(image_animated_get);
   ORD(image_animated_frame_count_get);
   ORD(image_animated_loop_type_get);
   ORD(image_animated_loop_count_get);
   ORD(image_animated_frame_duration_get);
   ORD(image_animated_frame_set);

   ORD(image_max_size_get);

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em __UNUSED__)
{
    eina_log_domain_unregister(_evas_engine_wl_egl_log_dom);
    evas_gl_common_module_close();
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION, "wayland_egl", "none", {module_open, module_close}
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, wayland_egl);

#ifndef EVAS_STATIC_BUILD_WAYLAND_EGL
EVAS_EINA_MODULE_DEFINE(engine, wayland_egl);
#endif

/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
