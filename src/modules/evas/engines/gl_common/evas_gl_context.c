#define _EVAS_GL_CONTEXT_C
#include "evas_gl_private.h"

#if defined HAVE_DLSYM && ! defined _WIN32
# include <dlfcn.h>      /* dlopen,dlclose,etc */
#else
# error gl_common should not get compiled if dlsym is not found on the system!
#endif

#define PRG_INVALID NULL
#define GLPIPES 1

static int tbm_sym_done = 0;
int _evas_engine_GL_common_log_dom = -1;
Cutout_Rects *_evas_gl_common_cutout_rects = NULL;

typedef void       (*glsym_func_void) ();
typedef void      *(*glsym_func_void_ptr) ();
typedef GLboolean  (*glsym_func_boolean) ();
typedef const char *(*glsym_func_const_char_ptr) ();

void       (*glsym_glGenFramebuffers)      (GLsizei a, GLuint *b) = NULL;
void       (*glsym_glBindFramebuffer)      (GLenum a, GLuint b) = NULL;
void       (*glsym_glFramebufferTexture2D) (GLenum a, GLenum b, GLenum c, GLuint d, GLint e) = NULL;
void       (*glsym_glDeleteFramebuffers)   (GLsizei a, const GLuint *b) = NULL;
void       (*glsym_glGetProgramBinary)     (GLuint a, GLsizei b, GLsizei *c, GLenum *d, void *e) = NULL;
void       (*glsym_glProgramBinary)        (GLuint a, GLenum b, const void *c, GLint d) = NULL;
void       (*glsym_glProgramParameteri)    (GLuint a, GLuint b, GLint d) = NULL;
void       (*glsym_glReleaseShaderCompiler)(void) = NULL;
void      *(*glsym_glMapBuffer)            (GLenum a, GLenum b) = NULL;
GLboolean  (*glsym_glUnmapBuffer)          (GLenum a) = NULL;
void       (*glsym_glStartTiling)          (GLuint a, GLuint b, GLuint c, GLuint d, GLuint e) = NULL;
void       (*glsym_glEndTiling)            (GLuint a) = NULL;
void       (*glsym_glRenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) = NULL;

const char *(*glsym_glGetStringi) (GLenum name, GLuint index) = NULL;

#ifdef GL_GLES

# ifndef GL_LINE_SMOOTH
#  define GL_LINE_SMOOTH 0x0B20
# endif

// just used for finding symbols :)
typedef void (*_eng_fn) (void);

typedef _eng_fn (*glsym_func_eng_fn) ();
typedef int  (*secsym_func_int) ();
typedef unsigned int  (*secsym_func_uint) ();
typedef void         *(*secsym_func_void_ptr) ();

unsigned int   (*eglsym_eglDestroyImage)              (void *a, void *b) = NULL;
void           (*secsym_glEGLImageTargetTexture2DOES) (int a, void *b) = NULL;
void          *(*secsym_eglMapImageSEC)               (void *a, void *b, int c, int d) = NULL;
unsigned int   (*secsym_eglUnmapImageSEC)             (void *a, void *b, int c) = NULL;
unsigned int   (*secsym_eglGetImageAttribSEC)         (void *a, void *b, int c, int *d) = NULL;

/* This one is now a local wrapper to avoid type mixups */
void *        evas_gl_common_eglCreateImage           (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list);
static void * (*eglsym_eglCreateImage)                (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list) = NULL;
static void * (*eglsym_eglCreateImageKHR)             (EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *e) = NULL;

////////////////////////////////////
//libtbm.so.1
static void *tbm_lib_handle;

void *(*secsym_tbm_surface_create) (int width, int height, unsigned int format) = NULL;
int  (*secsym_tbm_surface_destroy) (void *surface) = NULL;
int  (*secsym_tbm_surface_map) (void *surface, int opt, void *info) = NULL;
int  (*secsym_tbm_surface_unmap) (void *surface) = NULL;
int  (*secsym_tbm_surface_get_info) (void *surface, void *info) = NULL;
////////////////////////////////////
#else
typedef void (*_eng_fn) (void);

typedef _eng_fn (*glsym_func_eng_fn) ();
#endif

static int dbgflushnum = -1;

static void
sym_missing(void)
{
   ERR("GL symbols missing!");
}

/* This check is based heavily on the check from libepoxy.
 * Previously we used strstr(), however there are some extensions
 * whose names are subsets of others.
 */
EAPI Eina_Bool
evas_gl_extension_string_check(const char *exts, const char *ext)
{
   const char *ptr;
   int len;

   if (!exts || !ext) return EINA_FALSE;
   ptr = exts;

   if (*ptr == '\0')
     return EINA_FALSE;

   len = strlen(ext);

   while (1)
     {
        ptr = strstr(ptr, ext);
        if (!ptr)
          return EINA_FALSE;

        if (ptr[len] == ' ' || ptr[len] == '\0')
          return EINA_TRUE;

        ptr += len;
     }
}

/* Totally gross, but I didn't want to reindent all the
 * strstr() callers :(
 */
static Evas_Gl_Extension_String_Check _ckext = evas_gl_extension_string_check;

static int
_has_ext(const char *ext, const char **pexts, int *pnum)
{
   if (!ext) return EINA_FALSE;

   if (glsym_glGetStringi)
     {
        GLint num = *pnum, k;
        if (!num)
          {
             glGetIntegerv(GL_NUM_EXTENSIONS, &num);
             *pnum = num;
          }
        for (k = 0; k < num; k++)
          {
             const char *support = glsym_glGetStringi(GL_EXTENSIONS, k);
             if (support && !strcmp(support, ext))
               return EINA_TRUE;
          }
        return EINA_FALSE;
     }
   else
     {
        const char *exts = *pexts;
        if (!exts)
          {
             exts = (const char *) glGetString(GL_EXTENSIONS);
             if (!exts) return EINA_FALSE;
             *pexts = exts;
          }
        return _ckext(exts, ext);
     }
}

#ifdef GL_GLES

EAPI void *
evas_gl_common_eglCreateImage(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list)
{
   if (eglsym_eglCreateImageKHR)
     {
        int count, i;
        EGLint *ints = NULL;

        if (attrib_list)
          {
             for (count = 0; attrib_list[count] != EGL_NONE; count += 2);
             count++;
             ints = alloca(count * sizeof(EGLint));
             for (i = 0; i < count; i++)
               ints[i] = attrib_list[i];
          }
        return eglsym_eglCreateImageKHR(dpy, ctx, target, buffer, ints);
     }
   if (eglsym_eglCreateImage)
     return eglsym_eglCreateImage(dpy, ctx, target, buffer, attrib_list);
   return NULL;
}

EAPI int
evas_gl_common_eglDestroyImage(EGLDisplay dpy, void *im)
{
   if (eglsym_eglDestroyImage)
     return eglsym_eglDestroyImage(dpy, im);
   return EGL_FALSE;
}

#endif

/* FIXME: return error if a required symbol was not found */
EAPI void
evas_gl_symbols(void *(*GetProcAddress)(const char *name), const char *extsn)
{
   int failed = 0, num = 0;
   const char *exts = NULL;

   static int done = 0;
   if (done) return;

   if (!extsn) ERR("GL extension string was empty");

   /* For all extension functions, we need to match with the extension itself
    * since GetProcAddress() can return a non-NULL value even when the function
    * does not exist. Drivers can do a runtime mapping depending on the
    * context. So, we only trust the return value of GetProcAddress() when
    * we know for sure that the extension exists.
    *
    * Thus, if a symbol exists we will always prefer it rather than relying
    * on GetProcAddress().
    *
    * -- jpeg, 2016/08/04
    */

   glsym_glGetStringi = dlsym(RTLD_DEFAULT, "glGetStringi");

#define FINDSYM(dst, sym, ext, typ) do { \
   if (!dst) { \
      if (_has_ext(ext, &exts, &num) && GetProcAddress) \
        dst = (typ) GetProcAddress(sym); \
      if (!dst) \
        dst = (typ) dlsym(RTLD_DEFAULT, sym); \
   }} while (0)
#define FALLBAK(dst, typ) do { \
   if (!dst) { \
      ERR("Symbol '%s' could not be found!", (#dst) + 6); \
      dst = (typ) sym_missing; \
      failed = EINA_TRUE; \
   }} while (0)

#ifdef GL_GLES

   FINDSYM(glsym_glGenFramebuffers, "glGenFramebuffers", NULL, glsym_func_void);
   FALLBAK(glsym_glGenFramebuffers, glsym_func_void);

   FINDSYM(glsym_glBindFramebuffer, "glBindFramebuffer", NULL, glsym_func_void);
   FALLBAK(glsym_glBindFramebuffer, glsym_func_void);

#else

   /*
   Note about FBO APIs (from ARB_framebuffer_object):

   Framebuffer objects created with the commands defined by the
   GL_EXT_framebuffer_object extension are defined to be shared, while
   FBOs created with commands defined by the OpenGL core or
   GL_ARB_framebuffer_object extension are defined *not* to be shared.

   [...]

   Since the above pairs are aliases, the functions of a pair are
   equivalent.  Note that the functions BindFramebuffer and
   BindFramebufferEXT are not aliases and neither are the functions
   BindRenderbuffer and BindRenderbufferEXT.  Because object creation
   occurs when the framebuffer object is bound for the first time, a
   framebuffer object can be shared across contexts only if it was first
   bound with BindFramebufferEXT.  Framebuffers first bound with
   BindFramebuffer may not be shared across contexts.  Framebuffer
   objects created with BindFramebufferEXT may subsequently be bound
   using BindFramebuffer.  Framebuffer objects created with
   BindFramebuffer may be bound with BindFramebufferEXT provided they are
   bound to the same context they were created on.

   Undefined behavior results when using FBOs created by EXT commands
   through non-EXT interfaces, or vice-versa.

   Thus, I believe core should come first, then ARB and use EXT as fallback.
   -- jpeg, 2016/08/04

   Old note:
   nvidia tegra3 drivers seem to not expose via getprocaddress, but dlsym finds it:
   glGenFramebuffers, glBindFramebuffer, glFramebufferTexture2D, glDeleteFramebuffers
   */

   FINDSYM(glsym_glGenFramebuffers, "glGenFramebuffers", NULL, glsym_func_void);
   FINDSYM(glsym_glGenFramebuffers, "glGenFramebuffersARB", "GL_ARB_framebuffer_object", glsym_func_void);
   FINDSYM(glsym_glGenFramebuffers, "glGenFramebuffersEXT", "GL_EXT_framebuffer_object", glsym_func_void);
   FALLBAK(glsym_glGenFramebuffers, glsym_func_void);

   FINDSYM(glsym_glBindFramebuffer, "glBindFramebuffer", NULL, glsym_func_void);
   FINDSYM(glsym_glBindFramebuffer, "glBindFramebufferARB", "GL_ARB_framebuffer_object", glsym_func_void);
   FINDSYM(glsym_glBindFramebuffer, "glBindFramebufferEXT", "GL_EXT_framebuffer_object", glsym_func_void);
   FALLBAK(glsym_glBindFramebuffer, glsym_func_void);

#endif

   FINDSYM(glsym_glFramebufferTexture2D, "glFramebufferTexture2D", NULL, glsym_func_void);
   FINDSYM(glsym_glFramebufferTexture2D, "glFramebufferTexture2DARB", "GL_ARB_framebuffer_object", glsym_func_void);
   FINDSYM(glsym_glFramebufferTexture2D, "glFramebufferTexture2DEXT", "GL_EXT_framebuffer_object", glsym_func_void);
   FALLBAK(glsym_glFramebufferTexture2D, glsym_func_void);

   FINDSYM(glsym_glDeleteFramebuffers, "glDeleteFramebuffers", NULL, glsym_func_void);
   FINDSYM(glsym_glDeleteFramebuffers, "glDeleteFramebuffersARB", "GL_ARB_framebuffer_object", glsym_func_void);
   FINDSYM(glsym_glDeleteFramebuffers, "glDeleteFramebuffersEXT", "GL_EXT_framebuffer_object", glsym_func_void);
   FALLBAK(glsym_glDeleteFramebuffers, glsym_func_void);

   // Not sure there's an EXT variant
   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinary", NULL, glsym_func_void);
   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinaryOES", "GL_OES_get_program_binary", glsym_func_void);
   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinaryEXT", "GL_EXT_get_program_binary", glsym_func_void);

   // Not sure there's an EXT variant
   FINDSYM(glsym_glProgramBinary, "glProgramBinary", NULL, glsym_func_void);
   FINDSYM(glsym_glProgramBinary, "glProgramBinaryOES", "GL_OES_get_program_binary", glsym_func_void);
   FINDSYM(glsym_glProgramBinary, "glProgramBinaryEXT", "GL_EXT_get_program_binary", glsym_func_void);

   FINDSYM(glsym_glProgramParameteri, "glProgramParameteri", NULL, glsym_func_void);
   FINDSYM(glsym_glProgramParameteri, "glProgramParameteriEXT", "GL_EXT_separate_shader_objects", glsym_func_void);
   FINDSYM(glsym_glProgramParameteri, "glProgramParameteriARB", "GL_ARB_geometry_shader4", glsym_func_void);

   FINDSYM(glsym_glReleaseShaderCompiler, "glReleaseShaderCompiler", NULL, glsym_func_void);
#ifndef GL_GLES
   FINDSYM(glsym_glReleaseShaderCompiler, "glReleaseShaderCompiler", "GL_ARB_ES2_compatibility", glsym_func_void);
#endif

   // Not sure there's a core variant, glActivateTileQCOM is strange as well
   FINDSYM(glsym_glStartTiling, "glStartTilingQCOM", "GL_QCOM_tiled_rendering", glsym_func_void);
   FINDSYM(glsym_glStartTiling, "glStartTiling", NULL, glsym_func_void);
   FINDSYM(glsym_glStartTiling, "glActivateTileQCOM", NULL, glsym_func_void);
   FINDSYM(glsym_glEndTiling, "glEndTilingQCOM", "GL_QCOM_tiled_rendering", glsym_func_void);
   FINDSYM(glsym_glEndTiling, "glEndTiling", NULL, glsym_func_void);
   
   if (!getenv("EVAS_GL_MAPBUFFER_DISABLE"))
     {
        // Not sure there's an EXT variant. (probably no KHR variant)
        FINDSYM(glsym_glMapBuffer, "glMapBuffer", NULL, glsym_func_void_ptr);
        FINDSYM(glsym_glMapBuffer, "glMapBufferOES", "GL_OES_mapbuffer", glsym_func_void_ptr);
        FINDSYM(glsym_glMapBuffer, "glMapBufferARB", "GL_ARB_vertex_buffer_object", glsym_func_void_ptr);
        FINDSYM(glsym_glMapBuffer, "glMapBufferARB", "GLX_ARB_vertex_buffer_object", glsym_func_void_ptr);
        FINDSYM(glsym_glMapBuffer, "glMapBufferEXT", NULL, glsym_func_void_ptr);

        FINDSYM(glsym_glUnmapBuffer, "glUnmapBuffer", NULL, glsym_func_boolean);
        FINDSYM(glsym_glUnmapBuffer, "glUnmapBufferOES", "GL_OES_mapbuffer", glsym_func_boolean);
        FINDSYM(glsym_glUnmapBuffer, "glUnmapBufferARB", "GL_ARB_vertex_buffer_object", glsym_func_boolean);
        FINDSYM(glsym_glUnmapBuffer, "glUnmapBufferARB", "GLX_ARB_vertex_buffer_object", glsym_func_boolean);
        FINDSYM(glsym_glUnmapBuffer, "glUnmapBufferEXT", NULL, glsym_func_boolean);
     }

   FINDSYM(glsym_glRenderbufferStorageMultisample, "glRenderbufferStorageMultisample", NULL, glsym_func_void);

#ifdef GL_GLES
#define FINDSYMN(dst, sym, ext, typ) do { \
   if (!dst) { \
      if (_ckext(extsn, ext) && GetProcAddress) \
        dst = (typ) GetProcAddress(sym); \
      if (!dst) \
        dst = (typ) dlsym(RTLD_DEFAULT, sym); \
   }} while (0)

// yes - gl core looking for egl stuff. i know it's odd. a reverse-layer thing
// but it will work as the egl/glx layer calls gl core common stuff and thus
// these symbols will work. making the glx/egl + x11 layer do this kind-of is
// wrong as this is not x11 (output) layer specific like the native surface
// stuff. this is generic zero-copy textures for gl

   {
      const char *egl_version = eglQueryString(eglGetCurrentDisplay(), EGL_VERSION);
      int vmin = 1, vmaj = 0;

      if (!egl_version || (sscanf(egl_version, "%d.%d", &vmaj, &vmin) != 2))
        vmaj = 0;

      // Verify that EGL is >= 1.5 before looking up core function
      if ((vmaj > 1) || (vmaj == 1 && vmin >= 5))
        {
           eglsym_eglCreateImage = dlsym(RTLD_DEFAULT, "eglCreateImage");
           eglsym_eglDestroyImage = dlsym(RTLD_DEFAULT, "eglDestroyImage");
        }

      // For EGL <= 1.4 only the KHR extension exists: "EGL_KHR_image_base"
      if (!eglsym_eglCreateImage || !eglsym_eglDestroyImage)
        {
           eglsym_eglCreateImage = NULL;
           eglsym_eglDestroyImage = NULL;
           FINDSYMN(eglsym_eglCreateImageKHR, "eglCreateImageKHR", "EGL_KHR_image_base", secsym_func_void_ptr);
           FINDSYMN(eglsym_eglDestroyImage, "eglDestroyImageKHR", "EGL_KHR_image_base", secsym_func_uint);
        }
   }

   FINDSYM(glsym_glProgramParameteri, "glProgramParameteri", NULL, glsym_func_void);
   FINDSYM(glsym_glProgramParameteri, "glProgramParameteriEXT", "GL_EXT_geometry_shader4", glsym_func_void);
   FINDSYM(glsym_glProgramParameteri, "glProgramParameteriARB", "GL_ARB_geometry_shader4", glsym_func_void);

   FINDSYMN(secsym_glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES", "GL_OES_EGL_image_external", glsym_func_void);

   // Old SEC extensions
   FINDSYMN(secsym_eglMapImageSEC, "eglMapImageSEC", NULL, secsym_func_void_ptr);
   FINDSYMN(secsym_eglUnmapImageSEC, "eglUnmapImageSEC", NULL, secsym_func_uint);
   FINDSYMN(secsym_eglGetImageAttribSEC, "eglGetImageAttribSEC", NULL, secsym_func_uint);

#undef FINDSYMN

#endif

#undef FINDSYM
#undef FALLBAK

   if (failed)
     {
        ERR("Some core GL symbols could not be found, the GL engine will not "
            "work properly.");
     }

   done = 1;
}

static void
tbm_symbols(void)
{
   if (tbm_sym_done) return;
   tbm_sym_done = 1;

#ifdef GL_GLES
   tbm_lib_handle = dlopen("libtbm.so.1", RTLD_NOW);
   if (!tbm_lib_handle)
     {
        DBG("Unable to open libtbm:  %s", dlerror());
        return;
     }

#define FINDSYM(dst, sym, typ) \
   if (!dst) dst = (typ)dlsym(tbm_lib_handle, sym); \
   if (!dst)  \
     { \
        ERR("Symbol not found %s\n", sym); \
        return; \
     }

   FINDSYM(secsym_tbm_surface_create, "tbm_surface_create", secsym_func_void_ptr);
   FINDSYM(secsym_tbm_surface_destroy, "tbm_surface_destroy", secsym_func_int);
   FINDSYM(secsym_tbm_surface_map, "tbm_surface_map", secsym_func_int);
   FINDSYM(secsym_tbm_surface_unmap, "tbm_surface_unmap", secsym_func_int);
   FINDSYM(secsym_tbm_surface_get_info, "tbm_surface_get_info", secsym_func_int);

#undef FINDSYM
#endif
}

static void shader_array_flush(Evas_Engine_GL_Context *gc);

static Evas_Engine_GL_Context *_evas_gl_common_context = NULL;
static Evas_GL_Shared *shared = NULL;

EAPI void
__evas_gl_err(int err, const char *file, const char *func, int line, const char *op)
{
   const char *errmsg;
   char buf[32];

   switch (err)
     {
     case GL_INVALID_ENUM:
        errmsg = "GL_INVALID_ENUM";
        break;
     case GL_INVALID_VALUE:
        errmsg = "GL_INVALID_VALUE";
        break;
     case GL_INVALID_OPERATION:
        errmsg = "GL_INVALID_OPERATION";
        break;
     case GL_OUT_OF_MEMORY:
        errmsg = "GL_OUT_OF_MEMORY";
        break;
#ifdef GL_INVALID_FRAMEBUFFER_OPERATION
     case GL_INVALID_FRAMEBUFFER_OPERATION:
        {
           GLenum e = glCheckFramebufferStatus(GL_FRAMEBUFFER);
           switch (e)
             {
#ifdef GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
              case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                errmsg = "GL_INVALID_FRAMEBUFFER_OPERATION: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
                break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS
              case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
                errmsg = "GL_INVALID_FRAMEBUFFER_OPERATION: GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
                break;
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
              case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                errmsg = "GL_INVALID_FRAMEBUFFER_OPERATION: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
                break;
#endif
#ifdef GL_FRAMEBUFFER_UNSUPPORTED
              case GL_FRAMEBUFFER_UNSUPPORTED:
                errmsg = "GL_INVALID_FRAMEBUFFER_OPERATION: GL_FRAMEBUFFER_UNSUPPORTED";
                break;
#endif
              default:
                errmsg = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
             }
           break;
        }
#endif
     default:
        snprintf(buf, sizeof(buf), "%#x", err);
        errmsg = buf;
     }

   eina_log_print(_evas_engine_GL_common_log_dom, EINA_LOG_LEVEL_ERR,
                  file, func, line, "%s: %s", op, errmsg);
}

static void
matrix_ortho(GLfloat *m,
             GLfloat l, GLfloat r,
             GLfloat t, GLfloat b,
             GLfloat near_, GLfloat far_,
             int rot, int vw, int vh,
             int foc, GLfloat orth)
{
   GLfloat rotf;
   GLfloat cosv, sinv;
   GLfloat tx, ty;

   rotf = (((rot / 90) & 0x3) * M_PI) / 2.0;

   tx = -0.5 * (1.0 - orth);
   ty = -0.5 * (1.0 - orth);

   if (rot == 90)
     {
        tx += -(vw * 1.0);
        ty += -(vh * 0.0);
     }
   if (rot == 180)
     {
        tx += -(vw * 1.0);
        ty += -(vh * 1.0);
     }
   if (rot == 270)
     {
        tx += -(vw * 0.0);
        ty += -(vh * 1.0);
     }

   cosv = cos(rotf);
   sinv = sin(rotf);

   m[0] = (2.0 / (r - l)) * ( cosv);
   m[1] = (2.0 / (r - l)) * ( sinv);
   m[2] = 0.0;
   m[3] = 0.0;

   m[4] = (2.0 / (t - b)) * (-sinv);
   m[5] = (2.0 / (t - b)) * ( cosv);
   m[6] = 0.0;
   m[7] = 0.0;

   m[8] = 0.0;
   m[9] = 0.0;
   m[10] = -(2.0 / (far_ - near_));
   m[11] = 1.0 / (GLfloat)foc;

   m[12] = (m[0] * tx) + (m[4] * ty) - ((r + l) / (r - l));
   m[13] = (m[1] * tx) + (m[5] * ty) - ((t + b) / (t - b));
   m[14] = (m[2] * tx) + (m[6] * ty) - ((near_ + far_) / (far_ - near_));
   m[15] = (m[3] * tx) + (m[7] * ty) + orth;
}

int
evas_gl_common_version_check(int *minor_version)
{
   char *version;
   char *tmp;
   char *tmp2;
   int major = 0;
   int minor = 0;

  /*
   * glGetString returns a string describing the current GL connection.
   * GL_VERSION is used to get the version of the connection
   */

   if (minor_version) *minor_version = 0;

   version = (char *)glGetString(GL_VERSION);
   if (!version)
     {
        /* Something is wrong! */
        return 0;
     }

  /*
   * OpengL ES
   *
   * 1.* : The form is:
   *
   * OpenGL ES-<profile> <major>.<minor>
   *
   * where <profile> is either "CM" or "CL". The minor can be followed by the vendor
   * specific information
   *
   * 2.0 : The form is:
   *
   * OpenGL<space>ES<space><version number><space><vendor-specific information>
   */

   /* OpenGL ES 1.* ? */

   if (strstr(version, "OpenGL ES-CM ") || strstr(version, "OpenGL ES-CL "))
     {
        /* Not supported */
        return 0;
     }

   /* OpenGL ES 3.*  */

   if (strstr(version, "OpenGL ES 3"))
     {
        /* Supported */
        if (minor_version)
          {
             if ((version[11] == '.') && isdigit(version[12]))
               *minor_version = atoi(&version[12]);
             else *minor_version = 0;
          }
        return 3;
     }

   /* OpenGL ES 2.* ? */

   if (strstr(version, "OpenGL ES "))
     {
        /* Supported */
        if (minor_version)
          {
             if ((version[10] == '2') &&
                 (version[11] == '.') && isdigit(version[12]))
               *minor_version = atoi(&version[12]);
             else *minor_version = 0;
          }
        return 2;
     }

  /*
   * OpenGL
   *
   * The GL_VERSION and GL_SHADING_LANGUAGE_VERSION strings begin with a
   * version number. The version number uses one of these forms:
   *
   * major_number.minor_number
   * major_number.minor_number.release_number
   *
   * Vendor-specific information may follow the version number. Its format
   * depends on the implementation, but a space always separates the
   * version number and the vendor-specific information.
   */

   /* glGetString() returns a static string, and we are going to */
   /* modify it, so we get a copy first */
   version = strdup(version);
   if (!version)
     return 0;

   tmp = strchr(version, '.');
   if (!tmp) goto fail;
   /* the first '.' always exists */
   *tmp = '\0';
   major = atoi(version);
   /* FIXME: maybe we can assume that minor in only a cipher */
   tmp2 = ++tmp;
   while ((*tmp != '.') && (*tmp != ' ') && (*tmp != '\0'))
     tmp++;
   /* *tmp is '\0' : version is major_number.minor_number */
   /* *tmp is '.'  : version is major_number.minor_number.release_number */
   /* *tmp is ' '  : version is major_number.minor_number followed by vendor */
   *tmp = '\0';
   minor = atoi(tmp2);

 fail:
   free(version);

   // OpenGL 4.5 is supposed to be a superset of GLES 3.1
   if ((major == 4) && (minor >= 5))
     {
        if (minor_version) *minor_version = 1;
        return 3;
     }

   // OpenGL 4.3 is supposed to be a superset of GLES 3.0
   if ((major == 4) && (minor >= 3))
     return 3;

   // Extension GL_ARB_ES3_compatibility means OpenGL is a superset of GLES 3.0
   if ((major > 3) || ((major == 3) && (minor >= 3)))
     {
        const char *exts = NULL;
        int num = 0;

        if (_has_ext("GL_ARB_ES3_compatibility", &exts, &num))
          return 3;
     }

   // OpenGL >= 1.4 is a superset of the features of GLES 2 (albeit not an
   // exact function match)
   if (((major == 1) && (minor >= 4)) || (major >= 2))
     return 2; /* emulated support */

   return 0;
}

static void
_evas_gl_common_viewport_set(Evas_Engine_GL_Context *gc)
{
   int w = 1, h = 1, m = 1, rot = 1, foc = 0;
   int offx = 0, offy = 0;
   Evas_GL_Program *prog;
   Eina_Iterator *it;

   EINA_SAFETY_ON_NULL_RETURN(gc);
   foc = gc->foc;
   // surface in pipe 0 will be the same as all pipes
   if ((gc->pipe[0].shader.surface == gc->def_surface) ||
       (!gc->pipe[0].shader.surface))
     {
        w = gc->w;
        h = gc->h;
        rot = gc->rot;
     }
   else
     {
        w = gc->pipe[0].shader.surface->w;
        h = gc->pipe[0].shader.surface->h;
        rot = 0;
        m = -1;
        offx = gc->pipe[0].shader.surface->tex->x;
        offy = gc->pipe[0].shader.surface->tex->y;
     }

#ifdef GL_GLES
   if (gc->shared->eglctxt == gc->eglctxt)
#endif
     {
        if (((offx == gc->shared->offx) && (offy == gc->shared->offy)) &&
            ((!gc->change.size) ||
             (
                (gc->shared->w == w) && (gc->shared->h == h) &&
                (gc->shared->rot == rot) && (gc->shared->foc == gc->foc) &&
                (gc->shared->mflip == m)
             )
            )
           )
          return;
     }
#ifdef GL_GLES
   gc->shared->eglctxt = gc->eglctxt;
#endif

   gc->shared->w = w;
   gc->shared->h = h;
   gc->shared->rot = rot;
   gc->shared->mflip = m;
   gc->shared->foc = foc;
   gc->shared->z0 = gc->z0;
   gc->shared->px = gc->px;
   gc->shared->py = gc->py;
   gc->change.size = 0;
   gc->shared->offx = offx;
   gc->shared->offy = offy;

   if (foc == 0)
     {
        if ((rot == 0) || (rot == 180))
           glViewport(offx, offy, w, h);
        else
           glViewport(offx, offy, h, w);
        // std matrix
        if (m == 1)
           matrix_ortho(gc->shared->proj,
                        0, w, 0, h,
                        -1000000.0, 1000000.0,
                        rot, w, h,
                        1, 1.0);
        // v flipped matrix for render-to-texture
        else
           matrix_ortho(gc->shared->proj,
                        0, w, h, 0,
                        -1000000.0, 1000000.0,
                        rot, w, h,
                        1, 1.0);
     }
   else
     {
        int px, py, vx, vy, vw = 0, vh = 0, ax = 0, ay = 0, ppx = 0, ppy = 0;

        px = gc->shared->px;
        py = gc->shared->py;

        if      ((rot == 0  ) || (rot == 90 )) ppx = px;
        else if ((rot == 180) || (rot == 270)) ppx = w - px;
        if      ((rot == 0  ) || (rot == 270)) ppy = py;
        else if ((rot == 90 ) || (rot == 180)) ppy = h - py;

        vx = ((w / 2) - ppx);
        if (vx >= 0)
          {
             vw = w + (2 * vx);
             if      ((rot == 0  ) || (rot == 90 )) ax = 2 * vx;
             else if ((rot == 180) || (rot == 270)) ax = 0;
          }
        else
          {
             vw = w - (2 * vx);
             if      ((rot == 0  ) || (rot == 90 )) ax = 0;
             else if ((rot == 180) || (rot == 270)) ax = ppx - px;
             vx = 0;
          }

        vy = ((h / 2) - ppy);
        if (vy < 0)
          {
             vh = h - (2 * vy);
             if      (rot == 0) ay = 0;
             else if ((rot == 90 ) || (rot == 180) || (rot == 270)) ay = ppy - py;
             vy = -vy;
          }
        else
          {
             vh = h + (2 * vy);
             if      ((rot == 0  ) || (rot == 270)) ay = 2 * vy;
             else if ((rot == 90 ) || (rot == 180)) ay = 0;
             vy = 0;
          }

        if (m == -1) ay = vy * 2;

        if ((rot == 0) || (rot == 180))
           glViewport(offx + (-2 * vx), offy + (-2 * vy), vw, vh);
        else
           glViewport(offx + (-2 * vy), offy + (-2 * vx), vh, vw);
        if (m == 1)
           matrix_ortho(gc->shared->proj,
                        0, vw, 0, vh,
                        -1000000.0, 1000000.0,
                        rot, vw, vh,
                        foc, 0.0);
        else
           matrix_ortho(gc->shared->proj,
                        0, vw, vh, 0,
                        -1000000.0, 1000000.0,
                        rot, vw, vh,
                        foc, 0.0);
        gc->shared->ax = ax;
        gc->shared->ay = ay;
     }

   // FIXME: Is this heavy work?
   it = eina_hash_iterator_data_new(gc->shared->shaders_hash);
   EINA_ITERATOR_FOREACH(it, prog)
     prog->reset = EINA_TRUE;
   eina_iterator_free(it);

   if (gc->state.current.prog != PRG_INVALID)
     {
        prog = gc->state.current.prog;
        glUseProgram(prog->prog);
        glUniform1i(prog->uniform.rotation_id, gc->rot / 90);
        glUniformMatrix4fv(prog->uniform.mvp, 1, GL_FALSE, gc->shared->proj);
     }
}

EAPI Evas_Engine_GL_Context *
evas_gl_common_context_new(void)
{
   Evas_Engine_GL_Context *gc;
   const char *s;
   int i, gles_version;

#if 1
   if (_evas_gl_common_context)
     {
        _evas_gl_common_context->references++;
        return _evas_gl_common_context;
     }
#endif

   if (!glsym_glGetStringi)
     glsym_glGetStringi = dlsym(RTLD_DEFAULT, "glGetStringi");

   gles_version = evas_gl_common_version_check(NULL);
   if (!gles_version) return NULL;

   gc = calloc(1, sizeof(Evas_Engine_GL_Context));
   if (!gc) return NULL;

   tbm_symbols();
   gc->gles_version = gles_version;

   gc->references = 1;

   _evas_gl_common_context = gc;

   for (i = 0; i < MAX_PIPES; i++)
     {
        gc->pipe[i].shader.render_op = EVAS_RENDER_BLEND;
        if (glsym_glMapBuffer && glsym_glUnmapBuffer)
          {
             glGenBuffers(1, &gc->pipe[i].array.buffer);
             gc->pipe[i].array.buffer_alloc = 0;
             gc->pipe[i].array.buffer_use = 0;
          }
     }

   gc->state.current.tex_target = GL_TEXTURE_2D;

   if (!shared)
     {
        const char *ext;

        shared = calloc(1, sizeof(Evas_GL_Shared));
        ext = (const char *) glGetString(GL_EXTENSIONS);
        if (ext)
          {
             if (getenv("EVAS_GL_INFO"))
                fprintf(stderr, "EXT:\n%s\n", ext);
             if ((_ckext(ext, "GL_ARB_texture_non_power_of_two")) ||
                 (_ckext(ext, "OES_texture_npot")) ||
                 (_ckext(ext, "GL_IMG_texture_npot")))
               shared->info.tex_npo2 = 1;
             if ((_ckext(ext, "GL_NV_texture_rectangle")) ||
                 (_ckext(ext, "GL_EXT_texture_rectangle")) ||
                 (_ckext(ext, "GL_ARB_texture_rectangle")))
               shared->info.tex_rect = 1;
             if ((_ckext(ext, "GL_ARB_get_program_binary")) ||
                 (_ckext(ext, "GL_OES_get_program_binary")))
               shared->info.bin_program = 1;
             else
               glsym_glGetProgramBinary = NULL;
#ifdef GL_UNPACK_ROW_LENGTH
             shared->info.unpack_row_length = 1;
# ifdef GL_GLES
             if (!_ckext(ext, "_unpack_subimage"))
               shared->info.unpack_row_length = 0;
# endif
#endif

#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
             if ((_ckext(ext, "GL_EXT_texture_filter_anisotropic")))
               glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,
                           &(shared->info.anisotropic));
#endif
#ifdef GL_BGRA
             if ((_ckext(ext, "GL_EXT_bgra")) ||
                 (_ckext(ext, "GL_EXT_texture_format_BGRA8888")))
               shared->info.bgra = 1;
#endif
             if (_ckext(ext, "OES_compressed_ETC1_RGB8_texture"))
               shared->info.etc1 = 1;
             if (_ckext(ext, "GL_EXT_texture_compression_s3tc") ||
                 _ckext(ext, "GL_S3_s3tc"))
               shared->info.s3tc = 1;
#ifdef GL_GLES
             // FIXME: there should be an extension name/string to check for
             // not just symbols in the lib
             i = 0;
             s = getenv("EVAS_GL_NO_MAP_IMAGE_SEC");
             if (s) i = atoi(s);
             if (!i)
               {
                  // test for all needed symbols - be "conservative" and
                  // need all of it
                  if ((eglsym_eglDestroyImage) &&
                      (secsym_glEGLImageTargetTexture2DOES) &&
                      (secsym_eglMapImageSEC) &&
                      (secsym_eglUnmapImageSEC) &&
                      (secsym_eglGetImageAttribSEC))
                     shared->info.sec_image_map = 1;
               }
             i = 0;

             if ((secsym_tbm_surface_create) &&
                  (secsym_tbm_surface_destroy) &&
                  (secsym_tbm_surface_map) &&
                  (secsym_tbm_surface_unmap) &&
                  (secsym_tbm_surface_get_info))
                  shared->info.sec_tbm_surface = 1;
#endif
             if (!_ckext(ext, "GL_QCOM_tiled_rendering"))
               {
                  glsym_glStartTiling = NULL;
                  glsym_glEndTiling = NULL;
               }
          }
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,
                      &(shared->info.max_texture_units));
        glGetIntegerv(GL_MAX_TEXTURE_SIZE,
                      &(shared->info.max_texture_size));
        shared->info.max_vertex_elements = 6 * 100000;
#ifdef GL_MAX_ELEMENTS_VERTICES
/* only applies to glDrawRangeElements. don't really need to get it.
        glGetIntegerv(GL_MAX_ELEMENTS_VERTICES,
                      &(shared->info.max_vertex_elements));
 */
#endif
        s = getenv("EVAS_GL_VERTEX_MAX");
        if (s) shared->info.max_vertex_elements = atoi(s);
        if (shared->info.max_vertex_elements < 6)
           shared->info.max_vertex_elements = 6;

        // magic numbers that are a result of imperical testing and getting
        // "best case" performance across a range of systems
        shared->info.tune.cutout.max                 = DEF_CUTOUT;
        shared->info.tune.pipes.max                  = DEF_PIPES;
        shared->info.tune.atlas.max_alloc_size       = DEF_ATLAS_ALLOC;
        shared->info.tune.atlas.max_alloc_alpha_size = DEF_ATLAS_ALLOC_ALPHA;
        shared->info.tune.atlas.max_w                = DEF_ATLAS_W;
        shared->info.tune.atlas.max_h                = DEF_ATLAS_H;

        // per gpu hacks. based on impirical measurement of some known gpu's
        s = (const char *)glGetString(GL_RENDERER);
        if (s)
          {
             if      (strstr(s, "PowerVR SGX 540"))
                shared->info.tune.pipes.max = DEF_PIPES_SGX_540;
             else if (strstr(s, "NVIDIA Tegra 3"))
                shared->info.tune.pipes.max = DEF_PIPES_TEGRA_3;
             else if (strstr(s, "NVIDIA Tegra"))
                shared->info.tune.pipes.max = DEF_PIPES_TEGRA_2;
          }
        if (!getenv("EVAS_GL_MAPBUFFER"))
          {
             glsym_glMapBuffer = NULL;
             glsym_glUnmapBuffer= NULL;
          }

#define GETENVOPT(name, tune_param, min, max) \
        do { \
           const char *__v = getenv(name); \
           if (__v) { \
              shared->info.tune.tune_param = atoi(__v); \
              if (shared->info.tune.tune_param > max) \
                 shared->info.tune.tune_param = max; \
              else if (shared->info.tune.tune_param < min) \
                 shared->info.tune.tune_param = min; \
           } \
        } while (0)

        GETENVOPT("EVAS_GL_CUTOUT_MAX", cutout.max, -1, 0x7fffffff);
        GETENVOPT("EVAS_GL_PIPES_MAX", pipes.max, 1, MAX_PIPES);
        GETENVOPT("EVAS_GL_ATLAS_ALLOC_SIZE", atlas.max_alloc_size, MIN_ATLAS_ALLOC, MAX_ATLAS_ALLOC);
        GETENVOPT("EVAS_GL_ATLAS_ALLOC_ALPHA_SIZE", atlas.max_alloc_alpha_size, MIN_ATLAS_ALLOC_ALPHA, MAX_ATLAS_ALLOC_ALPHA);
        GETENVOPT("EVAS_GL_ATLAS_MAX_W", atlas.max_w, 0, MAX_ATLAS_W);
        GETENVOPT("EVAS_GL_ATLAS_MAX_H", atlas.max_h, 0, MAX_ATLAS_H);
        s = (const char *)getenv("EVAS_GL_GET_PROGRAM_BINARY");
        if (s)
          {
             if (atoi(s) == 0) shared->info.bin_program = 0;
          }

#ifdef GL_GLES
        // Detect ECT2 support. We need both RGB and RGBA formats.
          {
             GLint texFormatCnt = 0;
             glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &texFormatCnt);
             if (texFormatCnt > 0)
               {
                  GLenum *texFormats = malloc(texFormatCnt * sizeof(GLenum));
                  if (texFormats)
                    {
                       int k, cnt = 0;
                       glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, (GLint *) texFormats);
                       for (k = 0; k < texFormatCnt && cnt < 2; k++)
                         {
                            if (texFormats[k] == GL_COMPRESSED_RGB8_ETC2)
                              cnt++;
                            else if (texFormats[k] == GL_COMPRESSED_RGBA8_ETC2_EAC)
                              cnt++;
                         }
                       shared->info.etc2 = (cnt == 2);
                       free(texFormats);

                       // Note: If we support ETC2 we'll try to always use ETC2 even when the
                       // image has colorspace ETC1 (backwards compatibility).

                       if (_ckext(ext, "GL_EXT_compressed_ETC1_RGB8_sub_texture"))
                         shared->info.etc1_subimage = 1;
                       else
                         shared->info.etc1_subimage = shared->info.etc2;

                       // FIXME: My NVIDIA driver advertises ETC2 texture formats
                       // but does not support them. Driver bug? Logic bug?
                       // This is in #ifdef GL_GLES because Khronos recommends
                       // use of GL_COMPRESSED_TEXTURE_FORMATS but OpenGL 4.x
                       // does not.
                    }
               }
          }
#endif

        if (getenv("EVAS_GL_INFO"))
           fprintf(stderr,
                   "max tex size %ix%i\n"
                   "max units %i\n"
                   "non-power-2 tex %i\n"
                   "rect tex %i\n"
                   "bgra : %i\n"
                   "etc1 : %i\n"
                   "etc2 : %i%s\n"
                   "s3tc : %i\n"
                   "max ansiotropic filtering: %3.3f\n"
                   "egl sec map image: %i\n"
                   "max vertex count: %i\n"
                   "\n"
                   "(can set EVAS_GL_VERTEX_MAX  EVAS_GL_NO_MAP_IMAGE_SEC  EVAS_GL_INFO  EVAS_GL_MEMINFO )\n"
                   "\n"
                   "EVAS_GL_GET_PROGRAM_BINARY: %i\n"
                   "EVAS_GL_CUTOUT_MAX: %i\n"
                   "EVAS_GL_PIPES_MAX: %i\n"
                   "EVAS_GL_ATLAS_ALLOC_SIZE: %i\n"
                   "EVAS_GL_ATLAS_ALLOC_ALPHA_SIZE: %i\n"
                   "EVAS_GL_ATLAS_MAX_W x EVAS_GL_ATLAS_MAX_H: %i x %i\n"
                   ,
                   (int)shared->info.max_texture_size, (int)shared->info.max_texture_size,
                   (int)shared->info.max_texture_units,
                   (int)shared->info.tex_npo2,
                   (int)shared->info.tex_rect,
                   (int)shared->info.bgra,
                   (int)shared->info.etc1,
                   (int)shared->info.etc2, shared->info.etc2 ? " (GL_COMPRESSED_RGB8_ETC2, GL_COMPRESSED_RGBA8_ETC2_EAC)" : "",
                   (int)shared->info.s3tc,
                   (double)shared->info.anisotropic,
                   (int)shared->info.sec_image_map,
                   (int)shared->info.max_vertex_elements,

                   (int)shared->info.bin_program,
                   (int)shared->info.tune.cutout.max,
                   (int)shared->info.tune.pipes.max,
                   (int)shared->info.tune.atlas.max_alloc_size,
                   (int)shared->info.tune.atlas.max_alloc_alpha_size,
                   (int)shared->info.tune.atlas.max_w, (int)shared->info.tune.atlas.max_h
                  );

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_DITHER);
        glDisable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        // no dest alpha
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // dest alpha
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // ???
        glDepthMask(GL_FALSE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
        if (shared->info.anisotropic > 0.0)
          glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
#endif

        glEnableVertexAttribArray(SHAD_VERTEX);
        glEnableVertexAttribArray(SHAD_COLOR);

        if (!evas_gl_common_shader_program_init(shared))
          goto error;

        if (gc->state.current.prog)
          glUseProgram(gc->state.current.prog->prog);

        // in shader:
        // uniform sampler2D tex[8];
        //
        // in code:
        // GLuint texes[8];
        // GLint loc = glGetUniformLocation(prog, "tex");
        // glUniform1iv(loc, 8, texes);

        shared->native_pm_hash  = eina_hash_int32_new(NULL);
        shared->native_tex_hash = eina_hash_int32_new(NULL);
        shared->native_wl_hash = eina_hash_pointer_new(NULL);
        shared->native_tbm_hash = eina_hash_pointer_new(NULL);
        shared->native_evasgl_hash = eina_hash_pointer_new(NULL);
     }
   gc->shared = shared;
   gc->shared->references++;
   _evas_gl_common_viewport_set(gc);

   gc->def_surface = evas_gl_common_image_surface_new(gc, 1, 1, 1, EINA_FALSE);

   return gc;

error:
   evas_gl_common_context_free(gc);
   return NULL;
}

#define VERTEX_CNT 3
#define COLOR_CNT  4
#define TEX_CNT    2
#define SAM_CNT    2
#define MASK_CNT   4

#define PUSH_VERTEX(n, x, y, z) do { \
   gc->pipe[n].array.vertex[nv++] = x; \
   gc->pipe[n].array.vertex[nv++] = y; \
   gc->pipe[n].array.vertex[nv++] = z; } while(0)
#define PUSH_COLOR(n, r, g, b, a) do { \
   gc->pipe[n].array.color[nc++] = r; \
   gc->pipe[n].array.color[nc++] = g; \
   gc->pipe[n].array.color[nc++] = b; \
   gc->pipe[n].array.color[nc++] = a; } while(0)
#define PUSH_TEXUV(n, u, v) do { \
   gc->pipe[n].array.texuv[nu++] = u; \
   gc->pipe[n].array.texuv[nu++] = v; } while(0)
#define PUSH_TEXUV2(n, u, v) do { \
   gc->pipe[n].array.texuv2[nu2++] = u; \
   gc->pipe[n].array.texuv2[nu2++] = v; } while(0)
#define PUSH_TEXUV3(n, u, v) do { \
   gc->pipe[n].array.texuv3[nu3++] = u; \
   gc->pipe[n].array.texuv3[nu3++] = v; } while(0)
#define PUSH_TEXA(n, u, v) do { \
   gc->pipe[n].array.texa[na++] = u; \
   gc->pipe[n].array.texa[na++] = v; } while(0)
#define PUSH_TEXM(n, u, v, w, z) do { \
   gc->pipe[n].array.mask[nm++] = u; \
   gc->pipe[n].array.mask[nm++] = v; \
   gc->pipe[n].array.mask[nm++] = w; \
   gc->pipe[n].array.mask[nm++] = z; } while(0)
#define PUSH_TEXSAM(n, x, y) do { \
   gc->pipe[n].array.texsam[ns++] = x; \
   gc->pipe[n].array.texsam[ns++] = y; } while(0)

#define PUSH_6_VERTICES(pn, x, y, w, h) do { \
   PUSH_VERTEX(pn, x    , y    , 0); PUSH_VERTEX(pn, x + w, y    , 0); \
   PUSH_VERTEX(pn, x    , y + h, 0); PUSH_VERTEX(pn, x + w, y    , 0); \
   PUSH_VERTEX(pn, x + w, y + h, 0); PUSH_VERTEX(pn, x    , y + h, 0); \
   } while (0)
#define PUSH_6_QUAD(pn, x1, y1, x2, y2, x3, y3, x4, y4)                 \
  PUSH_TEXUV(pn, x1, y1); PUSH_TEXUV(pn, x2, y2); PUSH_TEXUV(pn, x4, y4);\
  PUSH_TEXUV(pn, x2, y2); PUSH_TEXUV(pn, x3, y3); PUSH_TEXUV(pn, x4, y4);

#define PUSH_6_TEXUV(pn, x1, y1, x2, y2)                \
  PUSH_6_QUAD(pn, x1, y1, x2, y1, x2, y2, x1, y2);

#define PUSH_6_TEXUV2(pn, x1, y1, x2, y2) do { \
   PUSH_TEXUV2(pn, x1, y1); PUSH_TEXUV2(pn, x2, y1); PUSH_TEXUV2(pn, x1, y2); \
   PUSH_TEXUV2(pn, x2, y1); PUSH_TEXUV2(pn, x2, y2); PUSH_TEXUV2(pn, x1, y2); \
   } while (0)
#define PUSH_6_TEXUV3(pn, x1, y1, x2, y2) do { \
   PUSH_TEXUV3(pn, x1, y1); PUSH_TEXUV3(pn, x2, y1); PUSH_TEXUV3(pn, x1, y2); \
   PUSH_TEXUV3(pn, x2, y1); PUSH_TEXUV3(pn, x2, y2); PUSH_TEXUV3(pn, x1, y2); \
   } while (0)
#define PUSH_6_TEXA(pn, x1, y1, x2, y2) do { \
   PUSH_TEXA(pn, x1, y1); PUSH_TEXA(pn, x2, y1); PUSH_TEXA(pn, x1, y2); \
   PUSH_TEXA(pn, x2, y1); PUSH_TEXA(pn, x2, y2); PUSH_TEXA(pn, x1, y2); \
   } while (0)
#define PUSH_SAMPLES(pn, dx, dy) do { \
   PUSH_TEXSAM(pn, dx, dy); PUSH_TEXSAM(pn, dx, dy); PUSH_TEXSAM(pn, dx, dy); \
   PUSH_TEXSAM(pn, dx, dy); PUSH_TEXSAM(pn, dx, dy); PUSH_TEXSAM(pn, dx, dy); \
   } while (0)
#define PUSH_MASKSAM(pn, x, y, cnt) do { int _i; for (_i = 0; _i < cnt; _i++) { \
   gc->pipe[pn].array.masksam[nms++] = x; gc->pipe[pn].array.masksam[nms++] = y; \
   } } while (0)
#define PUSH_6_COLORS(pn, r, g, b, a) \
   do { int i; for (i = 0; i < 6; i++) PUSH_COLOR(pn, r, g, b, a); } while(0)

#define PIPE_GROW(gc, pn, inc) \
   int nv = gc->pipe[pn].array.num * VERTEX_CNT; (void) nv; \
   int nc = gc->pipe[pn].array.num * COLOR_CNT; (void) nc; \
   int nu = gc->pipe[pn].array.num * TEX_CNT; (void) nu; \
   int nu2 = gc->pipe[pn].array.num * TEX_CNT; (void) nu2; \
   int nu3 = gc->pipe[pn].array.num * TEX_CNT; (void) nu3; \
   int na = gc->pipe[pn].array.num * TEX_CNT; (void) na; \
   int ns = gc->pipe[pn].array.num * SAM_CNT; (void) ns; \
   int nm = gc->pipe[pn].array.num * MASK_CNT; (void) nm; \
   int nms = gc->pipe[pn].array.num * SAM_CNT; (void) nms; \
   gc->pipe[pn].array.num += inc; \
   array_alloc(gc, pn);

#define PIPE_FREE(x) \
   do { _pipebuf_free(x); (x) = NULL; } while (0)

#define FREE(x) \
   do { free(x); (x) = NULL; } while (0)

typedef struct _Pipebuf
{
   int skipped, alloc;
} Pipebuf;

static int        _pipe_bufs_max = 0;
static int        _pipe_bufs_skipped = 0;
static Eina_List *_pipe_bufs = NULL;

/*
static int _used = 0, _alloced = 0, _realloced = 0;
static int _searches = 0, _looks = 0;
*/

static void *
_pipebuf_resize(void *pb, int size)
{
   Pipebuf *buf, *buf2;
   Eina_List *l, *ll;

   if (size > _pipe_bufs_max) _pipe_bufs_max = size;

   if (!pb)
     {
        if (_pipe_bufs)
          {
//             _searches++;
             EINA_LIST_FOREACH(_pipe_bufs, l, buf)
               {
//                  _looks++;
                  if (buf->alloc >= size) break;
                  buf->skipped++;
                  _pipe_bufs_skipped++;
               }
             if (l)
               {
//                  _used++;
                  _pipe_bufs = eina_list_remove_list(_pipe_bufs, l);
                  _pipe_bufs_skipped -= buf->skipped;
                  buf->skipped = 0;
                  goto done;
               }
          }
        buf = malloc(size + sizeof(Pipebuf));
        if (!buf) return NULL;
        buf->skipped = 0;
        buf->alloc = size;
//        _alloced++;
        goto done;
     }
   buf = (Pipebuf *)(((unsigned char *)pb) - sizeof(Pipebuf));
   if (buf->alloc < size)
     {
        buf2 = realloc(buf, size + sizeof(Pipebuf));
        if (!buf2) return NULL;
//        _realloced++;
        buf = buf2;
        buf->alloc = size;
     }
done:
   if (_pipe_bufs_skipped > 100)
     {
        EINA_LIST_REVERSE_FOREACH_SAFE(_pipe_bufs, l, ll, buf2)
          {
             if (buf2->skipped > 5)
               {
                  _pipe_bufs = eina_list_remove_list(_pipe_bufs, l);
                  _pipe_bufs_skipped -= buf2->skipped;
                  free(buf2);
                  if (_pipe_bufs_skipped == 0) break;
               }
          }
     }
//   if ((_used + _alloced + _realloced) % 1000 == 0)
//     printf("MAX=%i/%i skipped=%i searching=%i/%i    -   %i | %i | %i\n",
//            _pipe_bufs_max, eina_list_count(_pipe_bufs), _pipe_bufs_skipped,
//            _looks, _searches,
//            _used, _alloced, _realloced);
   return ((unsigned char *)buf) +  sizeof(Pipebuf);
}

static void
_pipebuf_free(void *pb)
{
   Pipebuf *buf;

   if (!pb) return;
   buf = (Pipebuf *)(((unsigned char *)pb) - sizeof(Pipebuf));
   _pipe_bufs_max = (_pipe_bufs_max * 19) / 20;
   if (buf->alloc > (_pipe_bufs_max * 4))
     {
        free(buf);
        return;
     }
   if ((!_pipe_bufs) || (eina_list_count(_pipe_bufs) < 20))
     {
        _pipe_bufs = eina_list_prepend(_pipe_bufs, buf);
        return;
     }
   free(buf);
}

static void
_pipebuf_clear(void)
{
   Pipebuf *buf;

   _pipe_bufs_max = 0;
   EINA_LIST_FREE(_pipe_bufs, buf)
     {
        free(buf);
     }
}

static void
array_alloc(Evas_Engine_GL_Context *gc, int n)
{
   gc->havestuff = EINA_TRUE;
   if (gc->pipe[n].array.num <= gc->pipe[n].array.alloc) return;

   gc->pipe[n].array.alloc += 6 * 256;

#define RALOC(field, type, size) \
   if (gc->pipe[n].array.use_##field) \
      gc->pipe[n].array.field = _pipebuf_resize(gc->pipe[n].array.field, \
                                                gc->pipe[n].array.alloc * sizeof(type) * size)

   RALOC(vertex, GLshort, VERTEX_CNT);
   RALOC(color,  GLubyte, COLOR_CNT);
   RALOC(texuv,  GLfloat, TEX_CNT);
   RALOC(texa,   GLfloat, TEX_CNT);
   RALOC(texuv2, GLfloat, TEX_CNT);
   RALOC(texuv3, GLfloat, TEX_CNT);
   RALOC(texsam, GLfloat, SAM_CNT);
   RALOC(mask,   GLfloat, MASK_CNT);
   RALOC(masksam, GLfloat, SAM_CNT);

#undef ALOC
#undef RALOC
}

EAPI void
evas_gl_common_context_free(Evas_Engine_GL_Context *gc)
{
   int i, j;
   Eina_List *l;

   gc->references--;
   if (gc->references > 0) return;
   if (gc->shared) gc->shared->references--;

   if (gc->def_surface) evas_gl_common_image_free(gc->def_surface);

   if (gc->font_surface)
     evas_cache_image_drop(&gc->font_surface->cache_entry);

   if (glsym_glMapBuffer && glsym_glUnmapBuffer)
     {
        for (i = 0; i < MAX_PIPES; i++)
          glDeleteBuffers(1, &gc->pipe[i].array.buffer);
     }

   if (gc->shared)
     {
        for (i = 0; i < gc->shared->info.tune.pipes.max; i++)
          {
             PIPE_FREE(gc->pipe[i].array.vertex);
             PIPE_FREE(gc->pipe[i].array.color);
             PIPE_FREE(gc->pipe[i].array.texuv);
             PIPE_FREE(gc->pipe[i].array.texuv2);
             PIPE_FREE(gc->pipe[i].array.texuv3);
             PIPE_FREE(gc->pipe[i].array.texa);
             PIPE_FREE(gc->pipe[i].array.texsam);
             PIPE_FREE(gc->pipe[i].array.mask);
             PIPE_FREE(gc->pipe[i].array.masksam);
             FREE(gc->pipe[i].array.filter_data);
          }
     }

   while (gc->font_glyph_textures)
     evas_gl_common_texture_free(gc->font_glyph_textures->data, EINA_TRUE);

   while (gc->font_glyph_images)
     evas_gl_common_image_free(gc->font_glyph_images->data);

   if ((gc->shared) && (gc->shared->references == 0))
     {
        Evas_GL_Texture_Pool *pt;

        evas_gl_common_shader_program_shutdown(gc->shared);

        while (gc->shared->images)
          {
             evas_gl_common_image_free(gc->shared->images->data);
          }

        for (j = 0; j < ATLAS_FORMATS_COUNT; j++)
          {
              EINA_LIST_FOREACH(gc->shared->tex.atlas[j], l, pt)
                 evas_gl_texture_pool_empty(pt);
              eina_list_free(gc->shared->tex.atlas[j]);
          }
        EINA_LIST_FOREACH(gc->shared->tex.whole, l, pt)
           evas_gl_texture_pool_empty(pt);
        eina_list_free(gc->shared->info.cspaces);
        eina_list_free(gc->shared->tex.whole);
        eina_hash_free(gc->shared->native_pm_hash);
        eina_hash_free(gc->shared->native_tex_hash);
        eina_hash_free(gc->shared->native_wl_hash);
        eina_hash_free(gc->shared->native_tbm_hash);
        eina_hash_free(gc->shared->native_evasgl_hash);
        eina_stringshare_del(gc->shared->shaders_checksum);
        free(gc->shared);
        shared = NULL;
     }
   if (gc == _evas_gl_common_context)
     {
        _pipebuf_clear();
        _evas_gl_common_context = NULL;
     }
   free(gc);
   if (_evas_gl_common_cutout_rects)
     {
        evas_common_draw_context_apply_clear_cutouts(_evas_gl_common_cutout_rects);
        _evas_gl_common_cutout_rects = NULL;
     }
}

EAPI void
evas_gl_common_context_use(Evas_Engine_GL_Context *gc)
{
   if (_evas_gl_common_context == gc) return;
   _evas_gl_common_context = gc;
   if (gc) _evas_gl_common_viewport_set(gc);
}

EAPI void
evas_gl_common_context_newframe(Evas_Engine_GL_Context *gc)
{
   int i;

   if (_evas_gl_common_cutout_rects)
     {
        evas_common_draw_context_apply_clear_cutouts(_evas_gl_common_cutout_rects);
        _evas_gl_common_cutout_rects = NULL;
     }
   if (dbgflushnum < 0)
     {
        dbgflushnum = 0;
        if (getenv("EVAS_GL_DBG")) dbgflushnum = 1;
     }
   if (dbgflushnum) printf("----prev-flushnum: %i -----------------------------------\n", gc->flushnum);
//   fprintf(stderr, "------------------------\n");

   gc->flushnum = 0;
   gc->state.current.prog = NULL;
   gc->state.current.cur_tex = 0;
   gc->state.current.cur_texu = 0;
   gc->state.current.cur_texv = 0;
   gc->state.current.cur_texa = 0;
   gc->state.current.cur_texm = 0;
   gc->state.current.tex_target = GL_TEXTURE_2D;
   gc->state.current.render_op = EVAS_RENDER_COPY;
   gc->state.current.smooth = 0;
   gc->state.current.blend = 0;
   gc->state.current.clip = 0;
   gc->state.current.cx = 0;
   gc->state.current.cy = 0;
   gc->state.current.cw = 0;
   gc->state.current.ch = 0;

   for (i = 0; i < gc->shared->info.tune.pipes.max; i++)
     {
        gc->pipe[i].region.x = 0;
        gc->pipe[i].region.y = 0;
        gc->pipe[i].region.w = 0;
        gc->pipe[i].region.h = 0;
        gc->pipe[i].region.type = 0;
        //gc->pipe[i].shader.surface = NULL;
        gc->pipe[i].shader.prog = NULL;
        gc->pipe[i].shader.cur_tex = 0;
        gc->pipe[i].shader.cur_texu = 0;
        gc->pipe[i].shader.cur_texv = 0;
        gc->pipe[i].shader.cur_texa = 0;
        gc->pipe[i].shader.cur_texm = 0;
        gc->pipe[i].shader.tex_target = GL_TEXTURE_2D;
        gc->pipe[i].shader.render_op = EVAS_RENDER_COPY;
        gc->pipe[i].shader.smooth = 0;
        gc->pipe[i].shader.blend = 0;
        gc->pipe[i].shader.clip = 0;
        gc->pipe[i].shader.cx = 0;
        gc->pipe[i].shader.cy = 0;
        gc->pipe[i].shader.cw = 0;
        gc->pipe[i].shader.ch = 0;
     }
   gc->change.size = 1;

   glDisable(GL_SCISSOR_TEST);
   glScissor(0, 0, 0, 0);

   glDisable(GL_DEPTH_TEST);
   glEnable(GL_DITHER);
   glDisable(GL_BLEND);
   glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
   // no dest alpha
//   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // dest alpha
//   glBlendFunc(GL_SRC_ALPHA, GL_ONE); // ???
   glDepthMask(GL_FALSE);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
   if (shared->info.anisotropic > 0.0)
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
#endif

   glEnableVertexAttribArray(SHAD_VERTEX);
   glEnableVertexAttribArray(SHAD_COLOR);
   if (gc->state.current.prog != PRG_INVALID)
     glUseProgram(gc->state.current.prog->prog);

   glActiveTexture(GL_TEXTURE0);
   glBindTexture(gc->pipe[0].shader.tex_target, gc->pipe[0].shader.cur_tex);

   _evas_gl_common_viewport_set(gc);
}

EAPI void
evas_gl_common_context_resize(Evas_Engine_GL_Context *gc, int w, int h, int rot)
{
   if ((gc->w == w) && (gc->h == h) && (gc->rot == rot)) return;
   evas_gl_common_context_flush(gc);
   gc->change.size = 1;
   gc->rot = rot;
   gc->w = w;
   gc->h = h;
   if (_evas_gl_common_context == gc) _evas_gl_common_viewport_set(gc);
}

void
evas_gl_common_tiling_start(Evas_Engine_GL_Context *gc EINA_UNUSED,
                            int rot, int gw, int gh,
                            int cx, int cy, int cw, int ch,
                            int bitmask)
{
   if (!glsym_glStartTiling) return;
   switch (rot)
     {
      case 0: // UP this way: ^
        glsym_glStartTiling(cx, cy, cw, ch, bitmask);
        break;
      case 90: // UP this way: <
        glsym_glStartTiling(gh - (cy + ch), cx, ch, cw, bitmask);
        break;
      case 180: // UP this way: v
        glsym_glStartTiling(gw - (cx + cw), gh - (cy + ch), cw, ch, bitmask);
        break;
      case 270: // UP this way: >
        glsym_glStartTiling(cy, gw - (cx + cw), ch, cw, bitmask);
        break;
      default: // assume up is up
        glsym_glStartTiling(cx, cy, cw, ch, bitmask);
        break;
     }
   GLERRV("glsym_glStartTiling");
}

void
evas_gl_common_tiling_done(Evas_Engine_GL_Context *gc EINA_UNUSED)
{
   if (glsym_glEndTiling)
     {
        glsym_glEndTiling(GL_COLOR_BUFFER_BIT0_QCOM);
        GLERRV("glsym_glEndTiling");
     }
}


EAPI void
evas_gl_common_context_done(Evas_Engine_GL_Context *gc)
{
   if (gc->master_clip.used)
     {
        if (glsym_glEndTiling)
          {
             glsym_glEndTiling(GL_COLOR_BUFFER_BIT0_QCOM);
             GLERRV("glsym_glEndTiling");
          }
        gc->master_clip.used = EINA_FALSE;
     }
}

void
evas_gl_common_context_target_surface_set(Evas_Engine_GL_Context *gc,
                                          Evas_GL_Image *surface)
{
   if (surface == gc->pipe[0].shader.surface) return;

   evas_gl_common_context_flush(gc);
   evas_gl_common_context_done(gc);

   gc->state.current.prog = NULL;
   gc->state.current.cur_tex = 0;
   gc->state.current.cur_texu = 0;
   gc->state.current.cur_texv = 0;
   gc->state.current.cur_texa = 0;
   gc->state.current.cur_texm = 0;
   gc->state.current.tex_target = GL_TEXTURE_2D;
   gc->state.current.render_op = -1;
   gc->state.current.smooth = -1;
   gc->state.current.blend = -1;
   gc->state.current.clip = -1;
   gc->state.current.cx = -1;
   gc->state.current.cy = -1;
   gc->state.current.cw = -1;
   gc->state.current.ch = -1;
   gc->state.current.anti_alias = -1;

   gc->pipe[0].shader.surface = surface;
   gc->change.size = 1;
#ifdef GL_GLES
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
# endif
#else
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
# endif
#endif
   if (gc->pipe[0].shader.surface == gc->def_surface)
     glsym_glBindFramebuffer(GL_FRAMEBUFFER, 0);
   else
      glsym_glBindFramebuffer(GL_FRAMEBUFFER, surface->tex->pt->fb);
   _evas_gl_common_viewport_set(gc);
}

static inline Eina_Bool
_push_mask(Evas_Engine_GL_Context *gc, const int pn, int nm, Evas_GL_Texture *mtex,
           int mx, int my, int mw, int mh, Shader_Sampling msam, int nms)
{
   double glmx, glmy, glmw, glmh, yinv = -1.f;
   double gw = gc->w, gh = gc->h;
   int i, cnt = 6;

   if (!((gc->pipe[0].shader.surface == gc->def_surface) ||
         (!gc->pipe[0].shader.surface)))
     {
        gw = gc->pipe[0].shader.surface->w;
        gh = gc->pipe[0].shader.surface->h;
        yinv = 1.f;
     }

   if (!gw || !gh || !mw || !mh || !mtex->pt->w || !mtex->pt->h)
     return EINA_FALSE;

   /* Vertex shader:
    *
    * INPUTS:
    *   vec4 mask_coord = vec4(glmx, glmy, glmw, glmh);
    *   int rotation_id = gc->rot / 90;
    *
    * CODE:
    *   vec4 mask_Position = mvp * vertex * vec4(0.5, sign(mask_coord.w) * 0.5, 0.5, 0.5) + vec4(0.5, 0.5, 0, 0);
    *   vec2 pos[4]; // no ctor-style init because of GLSL-ES (version 100)
    *   pos[0] = vec2(mask_Position.xy);
    *   pos[1] = vec2(1.0 - mask_Position.y, mask_Position.x);
    *   pos[2] = vec2(1.0 - mask_Position.xy);
    *   pos[3] = vec2(mask_Position.y, 1.0 - mask_Position.x);
    *   tex_m = pos[rotation_id].xy * abs(mask_coord.zw) + mask_coord.xy;
    */
   glmx = (double)((mtex->x * mw) - (mtex->w * mx)) / (double)(mw * mtex->pt->w);
   glmy = (double)((mtex->y * mh) - (mtex->h * my)) / (double)(mh * mtex->pt->h);
   glmw = (double)(gw * mtex->w) / (double)(mw * mtex->pt->w);
   glmh = (double)(gh * mtex->h) / (double)(mh * mtex->pt->h);
   glmh *= yinv;

   if (gc->pipe[pn].array.line)
     cnt = 2;

   for (i = 0; i < cnt; i++)
     PUSH_TEXM(pn, glmx, glmy, glmw, glmh);

   if (msam)
     {
        double samx = (double)(mtex->w) / (double)(mtex->pt->w * mw * 4);
        double samy = (double)(mtex->h) / (double)(mtex->pt->h * mh * 4);
        PUSH_MASKSAM(pn, samx, samy, cnt);
     }

   return EINA_TRUE;
}

#define PUSH_MASK(pn, mtex, mx, my, mw, mh, msam) if (mtex) do { \
   _push_mask(gc, pn, nm, mtex, mx, my, mw, mh, msam, nms); \
   } while(0)

#ifdef GLPIPES
static int
pipe_region_intersects(Evas_Engine_GL_Context *gc, int n,
                       int x, int y, int w, int h)
{
   int rx, ry, rw, rh, ii, end;
   const GLshort *v;
   
   rx = gc->pipe[n].region.x;
   ry = gc->pipe[n].region.y;
   rw = gc->pipe[n].region.w;
   rh = gc->pipe[n].region.h;
   if (!RECTS_INTERSECT(x, y, w, h, rx, ry, rw, rh)) return 0;

   // a hack for now. map pipes use their whole bounding box for intersects
   // which at worst case reduces to old pipeline flushes, but cheaper than
   // full quad region or triangle intersects right now
   if (gc->pipe[n].region.type == SHD_MAP) return 1;

   v = gc->pipe[n].array.vertex;
   end = gc->pipe[n].array.num * 3;
   for (ii = 0; ii < end; ii += (3 * 3 * 2))
     {  // tri 1...
        // 0, 1, 2 < top left
        // 3, 4, 5 < top right
        // 6, 7, 8 < bottom left
        rx = v[ii + 0];
        ry = v[ii + 1];
        rw = v[ii + 3] - rx;
        rh = v[ii + 7] - ry;
        if (RECTS_INTERSECT(x, y, w, h, rx, ry, rw, rh)) return 1;
     }
   return 0;
}
#endif

static void
pipe_region_expand(Evas_Engine_GL_Context *gc, int n,
                   int x, int y, int w, int h)
{
   int x1, y1, x2, y2;

   if (gc->pipe[n].region.w <= 0)
     {
        gc->pipe[n].region.x = x;
        gc->pipe[n].region.y = y;
        gc->pipe[n].region.w = w;
        gc->pipe[n].region.h = h;
        return;
     }
   x1 = gc->pipe[n].region.x;
   y1 = gc->pipe[n].region.y;
   x2 = gc->pipe[n].region.x + gc->pipe[n].region.w;
   y2 = gc->pipe[n].region.y + gc->pipe[n].region.h;
   if (x < x1) x1 = x;
   if (y < y1) y1 = y;
   if ((x + w) > x2) x2 = x + w;
   if ((y + h) > y2) y2 = y + h;
   gc->pipe[n].region.x = x1;
   gc->pipe[n].region.y = y1;
   gc->pipe[n].region.w = x2 - x1;
   gc->pipe[n].region.h = y2 - y1;
}

static Eina_Bool
vertex_array_size_check(Evas_Engine_GL_Context *gc EINA_UNUSED, int pn EINA_UNUSED, int n EINA_UNUSED)
{
   return 1;
// this fixup breaks for expedite test 32. why?
/* for reference
   if ((gc->pipe[pn].array.num + n) > gc->shared->info.max_vertex_elements)
     {
        shader_array_flush(gc);
        return 0;
     }
   return 1;
 */
}

static int
_evas_gl_common_context_push(Shader_Type rtype,
                             Evas_Engine_GL_Context *gc,
                             Evas_GL_Texture *tex,
                             Evas_GL_Texture *texm,
                             Evas_GL_Program *prog,
                             int x, int y, int w, int h,
                             Eina_Bool blend,
                             Eina_Bool smooth,
                             Eina_Bool clip,
                             int cx, int cy, int cw, int ch,
                             Eina_Bool mask_smooth)
{
   GLuint current_tex = 0;
   int pn = 0;

   if (tex)
     current_tex = tex->ptt ? tex->ptt->texture : tex->pt->texture;

#ifdef GLPIPES
 again:
#endif
   vertex_array_size_check(gc, gc->state.top_pipe, (rtype == SHD_LINE) ? 2 : 6);
   pn = gc->state.top_pipe;
#ifdef GLPIPES
   if (!((pn == 0) && (gc->pipe[pn].array.num == 0)))
     {
        int found = 0;
        int i;

        for (i = pn; i >= 0; i--)
          {
             if ((gc->pipe[i].region.type == rtype)
                 && (!tex || gc->pipe[i].shader.cur_tex == current_tex)
                 && (!texm || ((gc->pipe[i].shader.cur_texm == texm->pt->texture)
                               && (gc->pipe[i].shader.mask_smooth == mask_smooth)))
                 && (gc->pipe[i].shader.prog == prog)
                 && (gc->pipe[i].shader.smooth == smooth)
                 && (gc->pipe[i].shader.blend == blend)
                 && (gc->pipe[i].shader.render_op == gc->dc->render_op)
                 && (gc->pipe[i].shader.clip == clip)
                 && (!clip || ((gc->pipe[i].shader.cx == cx)
                               && (gc->pipe[i].shader.cy == cy)
                               && (gc->pipe[i].shader.cw == cw)
                               && (gc->pipe[i].shader.ch == ch))))
               {
                  found = 1;
                  pn = i;
                  break;
               }
             if (pipe_region_intersects(gc, i, x, y, w, h)) break;
          }
        if (!found)
          {
             pn = gc->state.top_pipe + 1;
             if (pn >= gc->shared->info.tune.pipes.max)
               {
                  shader_array_flush(gc);
                  goto again;
               }
             gc->state.top_pipe = pn;
          }
     }
   if ((tex) && (((tex->im) && (tex->im->native.data)) || tex->pt->dyn.img))
     {
        if (gc->pipe[pn].array.im != tex->im)
          {
             shader_array_flush(gc);
             pn = gc->state.top_pipe; // 0 after flush
             gc->pipe[pn].array.im = tex->im;
             goto again;
          }
     }
#else
   if (!((gc->pipe[pn].region.type == rtype)
         && (!tex || gc->pipe[pn].shader.cur_tex == current_tex)
         /* && (!texa || gc->pipe[pn].shader.cur_texa == current_texa) */
         && (!texm || ((gc->pipe[i].shader.cur_texm == texm->pt->texture)
                       && (gc->pipe[i].shader.mask_smooth == mask_smooth)))
         && (gc->pipe[pn].shader.prog == prog)
         && (gc->pipe[pn].shader.smooth == smooth)
         && (gc->pipe[pn].shader.blend == blend)
         && (gc->pipe[pn].shader.render_op == gc->dc->render_op)
         && (gc->pipe[pn].shader.clip == clip)
         && (!clip || ((gc->pipe[pn].shader.cx == cx)
                       && (gc->pipe[pn].shader.cy == cy)
                       && (gc->pipe[pn].shader.cw == cw)
                       && (gc->pipe[pn].shader.ch == ch)))))
     {
        shader_array_flush(gc);
     }
   if ((tex) && (((tex->im) && (tex->im->native.data)) || tex->pt->dyn.img))
     {
        if (gc->pipe[pn].array.im != tex->im)
          {
             shader_array_flush(gc);
             gc->pipe[pn].array.im = tex->im;
          }
     }
#endif

   return pn;
}

void
evas_gl_common_context_line_push(Evas_Engine_GL_Context *gc,
                                 int x1, int y1, int x2, int y2,
                                 int clip, int cx, int cy, int cw, int ch,
                                 Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                 Eina_Bool mask_smooth, Eina_Bool mask_color,
                                 int r, int g, int b, int a)
{
   Eina_Bool blend = EINA_FALSE;
   Evas_GL_Program *prog;
   int pn = 0, i;
   GLuint mtexid = mtex ? mtex->pt->texture : 0;
   Shader_Sampling masksam = SHD_SAM11;
   int x = MIN(x1, x2);
   int y = MIN(y1, y2);
   int w = abs(x2 - x1);
   int h = abs(y2 - y1);

   if (!(gc->dc->render_op == EVAS_RENDER_COPY) && ((a < 255) || (mtex)))
     blend = EINA_TRUE;

   prog = evas_gl_common_shader_program_get(gc, SHD_LINE, NULL, 0, r, g, b, a,
                                            0, 0, 0, 0, EINA_FALSE, NULL, EINA_FALSE,
                                            mtex, mask_smooth, mask_color, mw, mh,
                                            EINA_FALSE, NULL, NULL, &masksam);

   pn = _evas_gl_common_context_push(SHD_LINE,
                                     gc, NULL, mtex,
                                     prog,
                                     x, y, w, h,
                                     blend,
                                     EINA_FALSE,
                                     0, 0, 0, 0, 0,
                                     mask_smooth);

   gc->pipe[pn].region.type = SHD_LINE;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = 0;
   gc->pipe[pn].shader.cur_texm = mtexid;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.clip = clip;
   gc->pipe[pn].shader.cx = cx;
   gc->pipe[pn].shader.cy = cy;
   gc->pipe[pn].shader.cw = cw;
   gc->pipe[pn].shader.ch = ch;
   gc->pipe[pn].shader.mask_smooth = mask_smooth;

   gc->pipe[pn].array.line = 1;
   gc->pipe[pn].array.anti_alias = gc->dc->anti_alias;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = 1;
   gc->pipe[pn].array.use_texuv = 0;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;
   gc->pipe[pn].array.use_texa = 0;
   gc->pipe[pn].array.use_texsam = 0;
   gc->pipe[pn].array.use_masksam = (masksam != SHD_SAM11);
   gc->pipe[pn].array.use_mask = !!mtex;

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 2);
   PUSH_VERTEX(pn, x1, y1, 0);
   PUSH_VERTEX(pn, x2, y2, 0);
   PUSH_MASK(pn, mtex, mx, my, mw, mh, masksam);
   for (i = 0; i < 2; i++)
     PUSH_COLOR(pn, r, g, b, a);
}

void
evas_gl_common_context_rectangle_push(Evas_Engine_GL_Context *gc,
                                      int x, int y, int w, int h,
                                      int r, int g, int b, int a,
                                      Evas_GL_Texture *mtex,
                                      int mx, int my, int mw, int mh,
                                      Eina_Bool mask_smooth, Eina_Bool mask_color)
{
   Eina_Bool blend = EINA_FALSE;
   Shader_Sampling masksam = SHD_SAM11;
   Evas_GL_Program *prog;
   GLuint mtexid = mtex ? mtex->pt->texture : 0;
   int pn = 0;

   if (!(gc->dc->render_op == EVAS_RENDER_COPY) && ((a < 255) || mtex))
     blend = EINA_TRUE;

   prog = evas_gl_common_shader_program_get(gc, SHD_RECT, NULL, 0, r, g, b, a,
                                            0, 0, 0, 0, EINA_FALSE, NULL, EINA_FALSE,
                                            mtex, mask_smooth, mask_color, mw, mh,
                                            EINA_FALSE, NULL, NULL, &masksam);


   pn = _evas_gl_common_context_push(SHD_RECT,
                                     gc, NULL, mtex,
                                     prog,
                                     x, y, w, h,
                                     blend,
                                     EINA_FALSE,
                                     0, 0, 0, 0, 0,
                                     mask_smooth);

   gc->pipe[pn].region.type = SHD_RECT;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = 0;
   gc->pipe[pn].shader.cur_texm = mtexid;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.mask_smooth = mask_smooth;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = 1;
   gc->pipe[pn].array.use_texuv = 0;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;
   gc->pipe[pn].array.use_texa = 0;
   gc->pipe[pn].array.use_texsam = 0;
   gc->pipe[pn].array.use_masksam = (masksam != SHD_SAM11);
   gc->pipe[pn].array.use_mask = !!mtex;

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 6);
   PUSH_6_VERTICES(pn, x, y, w, h);
   PUSH_MASK(pn, mtex, mx, my, mw, mh, masksam);
   PUSH_6_COLORS(pn, r, g, b, a);
}

#define SWAP(a, b, tmp) \
   tmp = *a; \
   *a = *b; \
   *b = tmp;

// 1-2      4-1
// | |  =>  | |
// 4-3      3-2
static void
_rotate_90(double *x1, double *y1, double *x2, double *y2, double *x3, double *y3, double *x4, double *y4)
{
   double tmp;

   SWAP(x1, x4, tmp);
   SWAP(y1, y4, tmp);

   SWAP(x4, x3, tmp);
   SWAP(y4, y3, tmp);

   SWAP(x3, x2, tmp);
   SWAP(y3, y2, tmp);
}

// 1-2      3-4
// | |  =>  | |
// 4-3      2-1
static void
_rotate_180(double *x1, double *y1, double *x2, double *y2, double *x3, double *y3, double *x4, double *y4)
{
   double tmp;

   SWAP(x1, x3, tmp);
   SWAP(y1, y3, tmp);

   SWAP(x2, x4, tmp);
   SWAP(y2, y4, tmp);
}

// 1-2      2-3
// | |  =>  | |
// 4-3      1-4
static void
_rotate_270(double *x1, double *y1, double *x2, double *y2, double *x3, double *y3, double *x4, double *y4)
{
   double tmp;

   SWAP(x1, x2, tmp);
   SWAP(y1, y2, tmp);

   SWAP(x2, x3, tmp);
   SWAP(y2, y3, tmp);

   SWAP(x3, x4, tmp);
   SWAP(y3, y4, tmp);
}

// 1-2      2-1
// | |  =>  | |
// 4-3      3-4
static void
_flip_horizontal(double *x1, double *y1, double *x2, double *y2, double *x3, double *y3, double *x4, double *y4)
{
   double tmp;

   SWAP(x1, x2, tmp);
   SWAP(y1, y2, tmp);

   SWAP(x3, x4, tmp);
   SWAP(y3, y4, tmp);
}

// 1-2      4-3
// | |  =>  | |
// 4-3      1-2
static void
_flip_vertical(double *x1, double *y1, double *x2, double *y2, double *x3, double *y3, double *x4, double *y4)
{
   double tmp;

   SWAP(x1, x4, tmp);
   SWAP(y1, y4, tmp);

   SWAP(x2, x3, tmp);
   SWAP(y2, y3, tmp);
}

// 1-2      1-4
// | |  =>  | |
// 4-3      2-3
static void
_transpose(double *x1 EINA_UNUSED, double *y1 EINA_UNUSED, double *x2, double *y2,
           double *x3 EINA_UNUSED, double *y3 EINA_UNUSED, double *x4, double *y4)
{
   double tmp;

   SWAP(x2, x4, tmp);
   SWAP(y2, y4, tmp);
}

// 1-2      3-2
// | |  =>  | |
// 4-3      4-1
static void
_transverse(double *x1, double *y1, double *x2 EINA_UNUSED, double *y2 EINA_UNUSED,
            double *x3, double *y3, double *x4 EINA_UNUSED, double *y4 EINA_UNUSED)
{
   double tmp;

   SWAP(x1, x3, tmp);
   SWAP(y1, y3, tmp);
}

void
evas_gl_common_context_image_push(Evas_Engine_GL_Context *gc,
                                  Evas_GL_Texture *tex,
                                  double sx, double sy, double sw, double sh,
                                  int x, int y, int w, int h,
                                  Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                  Eina_Bool mask_smooth, Eina_Bool mask_color,
                                  int r, int g, int b, int a,
                                  Eina_Bool smooth, Eina_Bool tex_only,
                                  Eina_Bool alphaonly)
{

   Evas_GL_Texture_Pool *pt;
   double ox1, oy1, ox2, oy2, ox3, oy3, ox4, oy4;
   GLfloat tx1, ty1, tx2, ty2, tx3, ty3, tx4, ty4;
   GLfloat offsetx, offsety;
   double pw, ph;
   Eina_Bool blend = EINA_FALSE;
   Evas_GL_Program *prog;
   int pn = 0, render_op = gc->dc->render_op, nomul = 0;
   Shader_Sampling sam = 0, masksam = 0;
   int yinvert = 0;
   Shader_Type shd_in = SHD_IMAGE;
   int tex_target = GL_TEXTURE_2D;

   if (tex->im)
     {
        if (tex->im->native.data)
          shd_in = SHD_IMAGENATIVE;
        if (tex->im->native.target == GL_TEXTURE_EXTERNAL_OES)
          tex_target = GL_TEXTURE_EXTERNAL_OES;
     }

   if (!!mtex)
     {
        // masking forces BLEND mode (mask with COPY does not make sense)
        blend = EINA_TRUE;
        render_op = EVAS_RENDER_BLEND;
     }
   else if (!(render_op == EVAS_RENDER_COPY) && ((a < 255) || (tex->alpha)))
     blend = EINA_TRUE;

   prog = evas_gl_common_shader_program_get(gc, shd_in, NULL, 0, r, g, b, a,
                                            sw, sh, w, h, smooth, tex, tex_only,
                                            mtex, mask_smooth, mask_color, mw, mh,
                                            alphaonly, &sam, &nomul, &masksam);

   if (tex->ptt)
     {
        pt = tex->ptt;
        offsetx = tex->tx;
        offsety = tex->ty;
        smooth = EINA_TRUE;

        // Adjusting sx, sy, sw and sh to real size of tiny texture
        sx = sx * (EVAS_GL_TILE_SIZE - 2) / tex->w;
        sw = sw * (EVAS_GL_TILE_SIZE - 2) / tex->w;
        sy = sy * (EVAS_GL_TILE_SIZE - 1) / tex->h;
        sh = sh * (EVAS_GL_TILE_SIZE - 1) / tex->h;
     }
   else
     {
        pt = tex->pt;
        offsetx = tex->x;
        offsety = tex->y;
     }

   pn = _evas_gl_common_context_push(SHD_IMAGE,
                                     gc, tex, mtex,
                                     prog,
                                     x, y, w, h,
                                     blend,
                                     smooth,
                                     0, 0, 0, 0, 0,
                                     mask_smooth);

   gc->pipe[pn].region.type = SHD_IMAGE;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = pt->texture;
   gc->pipe[pn].shader.cur_texm = mtex ? mtex->pt->texture : 0;
   gc->pipe[pn].shader.tex_target = tex_target;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.mask_smooth = mask_smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = render_op;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = !nomul;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;
   gc->pipe[pn].array.use_texsam = (sam != SHD_SAM11);
   gc->pipe[pn].array.use_mask = !!mtex;
   gc->pipe[pn].array.use_masksam = (masksam != SHD_SAM11);

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 6);

   pw = pt->w;
   ph = pt->h;

   if (tex->im &&
       (tex->im->orient == EVAS_IMAGE_ORIENT_90))
     {
        double tmp;
        
        SWAP(&sw, &sh, tmp);
        SWAP(&sx, &sy, tmp);

        sy = tex->im->h - sh - sy;
     }

   if (tex->im &&
       (tex->im->orient == EVAS_IMAGE_ORIENT_180))
     {
        sx = tex->im->w - sw - sx;
        sy = tex->im->h - sh - sy;
     }

   if (tex->im &&
       (tex->im->orient == EVAS_IMAGE_ORIENT_270))
     {
        double tmp;

        SWAP(&sw, &sh, tmp);
        SWAP(&sx, &sy, tmp);

        sx = tex->im->w - sw - sx;
     }

   if (tex->im &&
       (tex->im->orient == EVAS_IMAGE_FLIP_HORIZONTAL))
     {
        sx = tex->im->w - sw - sx;
     }

   if (tex->im &&
       (tex->im->orient == EVAS_IMAGE_FLIP_VERTICAL))
     {
        sy = tex->im->h - sh - sy;
     }

   if (tex->im &&
       (tex->im->orient == EVAS_IMAGE_FLIP_TRANSVERSE))
     {
        double tmp;

        SWAP(&sw, &sh, tmp);
        SWAP(&sx, &sy, tmp);
        
        sx = tex->im->w - sw - sx;
        sy = tex->im->h - sh - sy;
     }

   if (tex->im &&
       (tex->im->orient == EVAS_IMAGE_FLIP_TRANSPOSE))
     {
        double tmp;

        SWAP(&sw, &sh, tmp);
        SWAP(&sx, &sy, tmp);
     }

   ox1 = sx;
   oy1 = sy;
   ox2 = sx + sw;
   oy2 = sy;
   ox3 = sx + sw;
   oy3 = sy + sh;
   ox4 = sx;
   oy4 = sy + sh;

   if ((tex->im) && (tex->im->native.data))
     {
        if (tex->im->native.func.yinvert)
          yinvert = tex->im->native.func.yinvert(tex->im);
        else
          yinvert = tex->im->native.yinvert;
     }

   if ((tex->im) && (!yinvert))
     {
        // FIXME: What if yinvert is true? How to test that?
        switch (tex->im->orient)
          {
           case EVAS_IMAGE_ORIENT_NONE:
              break;
           case EVAS_IMAGE_ORIENT_90:
              _rotate_90(&ox1, &oy1, &ox2, &oy2, &ox3, &oy3, &ox4, &oy4);
              break;
           case EVAS_IMAGE_ORIENT_180:
              _rotate_180(&ox1, &oy1, &ox2, &oy2, &ox3, &oy3, &ox4, &oy4);
              break;
           case EVAS_IMAGE_ORIENT_270:
              _rotate_270(&ox1, &oy1, &ox2, &oy2, &ox3, &oy3, &ox4, &oy4);
              break;
           case EVAS_IMAGE_FLIP_HORIZONTAL:
              _flip_horizontal(&ox1, &oy1, &ox2, &oy2, &ox3, &oy3, &ox4, &oy4);
              break;
           case EVAS_IMAGE_FLIP_VERTICAL:
              _flip_vertical(&ox1, &oy1, &ox2, &oy2, &ox3, &oy3, &ox4, &oy4);
              break;
           case EVAS_IMAGE_FLIP_TRANSVERSE:
              _transverse(&ox1, &oy1, &ox2, &oy2, &ox3, &oy3, &ox4, &oy4);
              break;
           case EVAS_IMAGE_FLIP_TRANSPOSE:
              _transpose(&ox1, &oy1, &ox2, &oy2, &ox3, &oy3, &ox4, &oy4);
              break;
           default:
              ERR("Wrong orientation ! %i", tex->im->orient);
          }
     }

   tx1 = ((double)(offsetx) + ox1) / pw;
   ty1 = ((double)(offsety) + oy1) / ph;
   tx2 = ((double)(offsetx) + ox2) / pw;
   ty2 = ((double)(offsety) + oy2) / ph;
   tx3 = ((double)(offsetx) + ox3) / pw;
   ty3 = ((double)(offsety) + oy3) / ph;
   tx4 = ((double)(offsetx) + ox4) / pw;
   ty4 = ((double)(offsety) + oy4) / ph;
   if ((tex->im) && (tex->im->native.data) && (!tex->im->native.yinvert))
     {
        ty1 = 1.0 - ty1;
        ty2 = 1.0 - ty2;
        ty3 = 1.0 - ty3;
        ty4 = 1.0 - ty4;
     }

   PUSH_6_VERTICES(pn, x, y, w, h);
   PUSH_6_QUAD(pn, tx1, ty1, tx2, ty2, tx3, ty3, tx4, ty4);

   if (sam)
     {
        double samx = (double)(sw) / (double)(tex->pt->w * w * 4);
        double samy = (double)(sh) / (double)(tex->pt->h * h * 4);
        PUSH_SAMPLES(pn, samx, samy);
     }

   PUSH_MASK(pn, mtex, mx, my, mw, mh, masksam);
   
   if (!nomul)
     PUSH_6_COLORS(pn, r, g, b, a);
}

void
evas_gl_common_context_font_push(Evas_Engine_GL_Context *gc,
                                 Evas_GL_Texture *tex,
                                 double sx, double sy, double sw, double sh,
                                 int x, int y, int w, int h,
                                 Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                 Eina_Bool mask_smooth, Eina_Bool mask_color,
                                 int r, int g, int b, int a)
{
   GLfloat tx1, tx2, ty1, ty2;
   Shader_Sampling masksam = SHD_SAM11;
   Evas_GL_Program *prog;
   int pn = 0;

   prog = evas_gl_common_shader_program_get(gc, SHD_FONT, NULL, 0, r, g, b, a,
                                            sw, sh, w, h, EINA_FALSE, tex, EINA_FALSE,
                                            mtex, mask_smooth, mask_color, mw, mh,
                                            EINA_FALSE, NULL, NULL, &masksam);

   pn = _evas_gl_common_context_push(SHD_FONT,
                                     gc, tex, mtex,
                                     prog,
                                     x, y, w, h,
                                     1,
                                     0,
                                     0, 0, 0, 0, 0,
                                     mask_smooth);

   gc->pipe[pn].region.type = SHD_FONT;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_texm = mtex ? mtex->pt->texture : 0;
   gc->pipe[pn].shader.smooth = 0;
   gc->pipe[pn].shader.blend = 1;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.mask_smooth = mask_smooth;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = 1;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;
   gc->pipe[pn].array.use_texsam = 0;
   gc->pipe[pn].array.use_mask = !!mtex;
   gc->pipe[pn].array.use_masksam = (masksam != SHD_SAM11);

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 6);

   if (sw == 0.0)
     {
        tx1 = tex->sx1;
        ty1 = tex->sy1;
        tx2 = tex->sx2;
        ty2 = tex->sy2;
     }
   else
     {
        tx1 = ((double)(tex->x) + sx) / (double)tex->pt->w;
        ty1 = ((double)(tex->y) + sy) / (double)tex->pt->h;
        tx2 = ((double)(tex->x) + sx + sw) / (double)tex->pt->w;
        ty2 = ((double)(tex->y) + sy + sh) / (double)tex->pt->h;
     }

   PUSH_6_VERTICES(pn, x, y, w, h);
   PUSH_6_TEXUV(pn, tx1, ty1, tx2, ty2);
   PUSH_MASK(pn, mtex, mx, my, mw, mh, masksam);
   PUSH_6_COLORS(pn, r, g, b, a);
}

void
evas_gl_common_context_yuv_push(Evas_Engine_GL_Context *gc,
                                Evas_GL_Texture *tex,
                                double sx, double sy, double sw, double sh,
                                int x, int y, int w, int h,
                                Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                Eina_Bool mask_smooth, Eina_Bool mask_color,
                                int r, int g, int b, int a,
                                Eina_Bool smooth)
{
   GLfloat tx1, tx2, ty1, ty2, t2x1, t2x2, t2y1, t2y2;
   Shader_Sampling masksam = SHD_SAM11;
   Eina_Bool blend = 0;
   Evas_GL_Program *prog;
   int pn = 0, nomul = 0;

   if ((a < 255) || (!!mtex))
     blend = 1;

   prog = evas_gl_common_shader_program_get(gc, SHD_YUV, NULL, 0, r, g, b, a,
                                            w, h, w, h, smooth, tex, 0,
                                            mtex, mask_smooth, mask_color, mw, mh,
                                            EINA_FALSE, NULL, &nomul, &masksam);

   pn = _evas_gl_common_context_push(SHD_YUV,
                                     gc, tex, mtex,
                                     prog,
                                     x, y, w, h,
                                     blend,
                                     smooth,
                                     0, 0, 0, 0, 0,
                                     mask_smooth);

   gc->pipe[pn].region.type = SHD_YUV;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_texu = tex->ptu->texture;
   gc->pipe[pn].shader.cur_texv = tex->ptv->texture;
   gc->pipe[pn].shader.cur_texm = mtex ? mtex->pt->texture : 0;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.mask_smooth = mask_smooth;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = !nomul;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 1;
   gc->pipe[pn].array.use_texuv3 = 1;
   gc->pipe[pn].array.use_mask = !!mtex;
   gc->pipe[pn].array.use_texsam = 0;
   gc->pipe[pn].array.use_masksam = (masksam != SHD_SAM11);

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 6);

   tx1 = (sx) / (double)tex->pt->w;
   ty1 = (sy) / (double)tex->pt->h;
   tx2 = (sx + sw) / (double)tex->pt->w;
   ty2 = (sy + sh) / (double)tex->pt->h;

   t2x1 = ((sx) / 2.0) / (double)tex->ptu->w;
   t2y1 = ((sy) / 2.0) / (double)tex->ptu->h;
   t2x2 = ((sx + sw) / 2.0) / (double)tex->ptu->w;
   t2y2 = ((sy + sh) / 2.0) / (double)tex->ptu->h;

   PUSH_6_VERTICES(pn, x, y, w, h);
   PUSH_6_TEXUV(pn, tx1, ty1, tx2, ty2);
   PUSH_6_TEXUV2(pn, t2x1, t2y1, t2x2, t2y2);
   PUSH_6_TEXUV3(pn, t2x1, t2y1, t2x2, t2y2);
   PUSH_MASK(pn, mtex, mx, my, mw, mh, masksam);
   if (!nomul)
     PUSH_6_COLORS(pn, r, g, b, a);
}

void
evas_gl_common_context_yuv_709_push(Evas_Engine_GL_Context *gc,
                                    Evas_GL_Texture *tex,
                                    double sx, double sy, double sw, double sh,
                                    int x, int y, int w, int h,
                                    Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                    Eina_Bool mask_smooth, Eina_Bool mask_color,
                                    int r, int g, int b, int a,
                                    Eina_Bool smooth)
{
   GLfloat tx1, tx2, ty1, ty2, t2x1, t2x2, t2y1, t2y2;
   Shader_Sampling masksam = SHD_SAM11;
   Eina_Bool blend = 0;
   Evas_GL_Program *prog;
   int pn = 0, nomul = 0;

   if ((a < 255) || (!!mtex))
     blend = 1;

   prog = evas_gl_common_shader_program_get(gc, SHD_YUV_709, NULL, 0, r, g, b, a,
                                            w, h, w, h, smooth, tex, 0,
                                            mtex, mask_smooth, mask_color, mw, mh,
                                            EINA_FALSE, NULL, &nomul, &masksam);

   pn = _evas_gl_common_context_push(SHD_YUV_709,
                                     gc, tex, mtex,
                                     prog,
                                     x, y, w, h,
                                     blend,
                                     smooth,
                                     0, 0, 0, 0, 0,
                                     mask_smooth);

   gc->pipe[pn].region.type = SHD_YUV_709;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_texu = tex->ptu->texture;
   gc->pipe[pn].shader.cur_texv = tex->ptv->texture;
   gc->pipe[pn].shader.cur_texm = mtex ? mtex->pt->texture : 0;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.mask_smooth = mask_smooth;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = !nomul;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 1;
   gc->pipe[pn].array.use_texuv3 = 1;
   gc->pipe[pn].array.use_mask = !!mtex;
   gc->pipe[pn].array.use_texsam = 0;
   gc->pipe[pn].array.use_masksam = (masksam != SHD_SAM11);

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 6);

   tx1 = (sx) / (double)tex->pt->w;
   ty1 = (sy) / (double)tex->pt->h;
   tx2 = (sx + sw) / (double)tex->pt->w;
   ty2 = (sy + sh) / (double)tex->pt->h;

   t2x1 = ((sx) / 2.0) / (double)tex->ptu->w;
   t2y1 = ((sy) / 2.0) / (double)tex->ptu->h;
   t2x2 = ((sx + sw) / 2.0) / (double)tex->ptu->w;
   t2y2 = ((sy + sh) / 2.0) / (double)tex->ptu->h;

   PUSH_6_VERTICES(pn, x, y, w, h);
   PUSH_6_TEXUV(pn, tx1, ty1, tx2, ty2);
   PUSH_6_TEXUV2(pn, t2x1, t2y1, t2x2, t2y2);
   PUSH_6_TEXUV3(pn, t2x1, t2y1, t2x2, t2y2);
   PUSH_MASK(pn, mtex, mx, my, mw, mh, masksam);
   if (!nomul)
     PUSH_6_COLORS(pn, r, g, b, a);
}

void
evas_gl_common_context_yuy2_push(Evas_Engine_GL_Context *gc,
                                 Evas_GL_Texture *tex,
                                 double sx, double sy, double sw, double sh,
                                 int x, int y, int w, int h,
                                 Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                 Eina_Bool mask_smooth, Eina_Bool mask_color,
                                 int r, int g, int b, int a,
                                 Eina_Bool smooth)
{
   GLfloat tx1, tx2, ty1, ty2, t2x1, t2x2, t2y1, t2y2;
   Shader_Sampling masksam = SHD_SAM11;
   Eina_Bool blend = 0;
   Evas_GL_Program *prog;
   int pn = 0, nomul = 0;

   if ((a < 255) || (!!mtex))
     blend = 1;

   prog = evas_gl_common_shader_program_get(gc, SHD_YUY2, NULL, 0, r, g, b, a,
                                            sw, sh, w, h, smooth, tex, 0,
                                            mtex, mask_smooth, mask_color, mw, mh,
                                            EINA_FALSE, NULL, &nomul, &masksam);

   pn = _evas_gl_common_context_push(SHD_YUY2,
                                     gc, tex, mtex,
                                     prog,
                                     x, y, w, h,
                                     blend,
                                     smooth,
                                     0, 0, 0, 0, 0,
                                     mask_smooth);

   gc->pipe[pn].region.type = SHD_YUY2;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_texu = tex->ptuv->texture;
   gc->pipe[pn].shader.cur_texm = mtex ? mtex->pt->texture : 0;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.mask_smooth = mask_smooth;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = !nomul;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 1;
   gc->pipe[pn].array.use_texuv3 = 0;
   gc->pipe[pn].array.use_mask = !!mtex;
   gc->pipe[pn].array.use_texsam = 0;
   gc->pipe[pn].array.use_masksam = (masksam != SHD_SAM11);

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 6);

   tx1 = (sx) / (double)tex->pt->w;
   ty1 = (sy) / (double)tex->pt->h;
   tx2 = (sx + sw) / (double)tex->pt->w;
   ty2 = (sy + sh) / (double)tex->pt->h;

   t2x1 = sx / (double)tex->ptuv->w;
   t2y1 = sy / (double)tex->ptuv->h;
   t2x2 = (sx + sw) / (double)tex->ptuv->w;
   t2y2 = (sy + sh) / (double)tex->ptuv->h;

   PUSH_6_VERTICES(pn, x, y, w, h);
   PUSH_6_TEXUV(pn, tx1, ty1, tx2, ty2);
   PUSH_6_TEXUV2(pn, t2x1, t2y1, t2x2, t2y2);
   PUSH_MASK(pn, mtex, mx, my, mw, mh, masksam);
   if (!nomul)
     PUSH_6_COLORS(pn, r, g, b, a);
}

void
evas_gl_common_context_nv12_push(Evas_Engine_GL_Context *gc,
                                 Evas_GL_Texture *tex,
                                 double sx, double sy, double sw, double sh,
                                 int x, int y, int w, int h,
                                 Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                 Eina_Bool mask_smooth, Eina_Bool mask_color,
                                 int r, int g, int b, int a,
                                 Eina_Bool smooth)
{
   GLfloat tx1, tx2, ty1, ty2, t2x1, t2x2, t2y1, t2y2;
   Shader_Sampling masksam = SHD_SAM11;
   Eina_Bool blend = 0;
   Evas_GL_Program *prog;
   int pn = 0, nomul = 0;

   if ((a < 255) || (!!mtex))
     blend = 1;

   prog = evas_gl_common_shader_program_get(gc, SHD_NV12, NULL, 0, r, g, b, a,
                                            sw, sh, w, h, smooth, tex, 0,
                                            mtex, mask_smooth, mask_color, mw, mh,
                                            EINA_FALSE, NULL, &nomul, &masksam);

   pn = _evas_gl_common_context_push(SHD_NV12,
                                     gc, tex, mtex,
                                     prog,
                                     x, y, w, h,
                                     blend,
                                     smooth,
                                     0, 0, 0, 0, 0,
                                     mask_smooth);

   gc->pipe[pn].region.type = SHD_NV12;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_tex_dyn = tex->pt->dyn.img;
   gc->pipe[pn].shader.cur_texu = tex->ptuv->texture;
   gc->pipe[pn].shader.cur_texu_dyn = tex->ptuv->dyn.img;
   gc->pipe[pn].shader.cur_texm = mtex ? mtex->pt->texture : 0;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.mask_smooth = mask_smooth;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = !nomul;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 1;
   gc->pipe[pn].array.use_texuv3 = 0;
   gc->pipe[pn].array.use_mask = !!mtex;
   gc->pipe[pn].array.use_texsam = 0;
   gc->pipe[pn].array.use_masksam = (masksam != SHD_SAM11);

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 6);

   tx1 = (sx) / (double)tex->pt->w;
   ty1 = (sy) / (double)tex->pt->h;
   tx2 = (sx + sw) / (double)tex->pt->w;
   ty2 = (sy + sh) / (double)tex->pt->h;

   t2x1 = sx / (double)tex->ptuv->w;
   t2y1 = sy / (double)tex->ptuv->h;
   t2x2 = (sx + sw) / (double)tex->ptuv->w;
   t2y2 = (sy + sh) / (double)tex->ptuv->h;

   PUSH_6_VERTICES(pn, x, y, w, h);
   PUSH_6_TEXUV(pn, tx1, ty1, tx2, ty2);
   PUSH_6_TEXUV2(pn, t2x1, t2y1, t2x2, t2y2);
   PUSH_MASK(pn, mtex, mx, my, mw, mh, masksam);
   if (!nomul)
     PUSH_6_COLORS(pn, r, g, b, a);
}

void
evas_gl_common_context_rgb_a_pair_push(Evas_Engine_GL_Context *gc,
                                       Evas_GL_Texture *tex,
                                       double sx, double sy,
                                       double sw, double sh,
                                       int x, int y, int w, int h,
                                       Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                       Eina_Bool mask_smooth, Eina_Bool mask_color,
                                       int r, int g, int b, int a,
                                       Eina_Bool smooth)

{
   /* This RGB+Alpha mode is used for ETC1+Alpha textures, where the shader
    * will multiply RGB by alpha. Two textures are created: tex->{pt,pta}.
    * Since the exact encoding doesn't matter here (decoding is transparent
    * from the shader point of view), this method could be used for other
    * colorspaces as well (eg. RGB565+Alpha4, ...).
    */

   GLfloat tx1, tx2, ty1, ty2, t2x1, t2x2, t2y1, t2y2;
   Shader_Sampling masksam = SHD_SAM11;
   Evas_GL_Program *prog;
   int pn, nomul = 0;

   prog = evas_gl_common_shader_program_get(gc, SHD_RGB_A_PAIR, NULL, 0, r, g, b, a,
                                            sw, sh, w, h, smooth, tex, 0,
                                            mtex, mask_smooth, mask_color, mw, mh,
                                            EINA_FALSE, NULL, &nomul, &masksam);

   pn = _evas_gl_common_context_push(SHD_RGB_A_PAIR,
                                     gc, tex, mtex,
                                     prog,
                                     x, y, w, h,
                                     EINA_TRUE,
                                     smooth,
                                     EINA_FALSE, 0, 0, 0, 0,
                                     mask_smooth);

   gc->pipe[pn].region.type = SHD_RGB_A_PAIR;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_texa = tex->pta->texture;
   gc->pipe[pn].shader.cur_texm = mtex ? mtex->pt->texture : 0;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = EINA_TRUE;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.mask_smooth = mask_smooth;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = EINA_TRUE;
   gc->pipe[pn].array.use_color = !nomul;
   gc->pipe[pn].array.use_texuv = EINA_TRUE;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;
   gc->pipe[pn].array.use_texa = EINA_TRUE;
   gc->pipe[pn].array.use_texsam = 0;
   gc->pipe[pn].array.use_mask = !!mtex;
   gc->pipe[pn].array.use_masksam = (masksam != SHD_SAM11);

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 6);

   // FIXME: pt and pta could have different x,y
   tx1 = (tex->x + sx) / (double)tex->pt->w;
   ty1 = (tex->y + sy) / (double)tex->pt->h;
   tx2 = (tex->x + sx + sw) / (double)tex->pt->w;
   ty2 = (tex->y + sy + sh) / (double)tex->pt->h;

   t2x1 = (tex->x + sx) / (double)tex->pta->w;
   t2y1 = (tex->y + sy) / (double)tex->pta->h;
   t2x2 = (tex->x + sx + sw) / (double)tex->pta->w;
   t2y2 = (tex->y + sy + sh) / (double)tex->pta->h;

   PUSH_6_VERTICES(pn, x, y, w, h);
   PUSH_6_TEXUV(pn, tx1, ty1, tx2, ty2);
   PUSH_6_TEXA(pn, t2x1, t2y1, t2x2, t2y2);
   PUSH_MASK(pn, mtex, mx, my, mw, mh, masksam);
   if (!nomul)
     PUSH_6_COLORS(pn, r, g, b, a);
}

void
evas_gl_common_context_image_map_push(Evas_Engine_GL_Context *gc,
                                      Evas_GL_Texture *tex,
                                      int npoints,
                                      RGBA_Map_Point *p,
                                      int clip, int cx, int cy, int cw, int ch,
                                      Evas_GL_Texture *mtex, int mx, int my, int mw, int mh,
                                      Eina_Bool mask_smooth, Eina_Bool mask_color,
                                      int r, int g, int b, int a,
                                      Eina_Bool smooth, Eina_Bool tex_only,
                                      Evas_Colorspace cspace)
{
   const int points[6] = { 0, 1, 2, 0, 2, 3 };
   int x = 0, y = 0, w = 0, h = 0, px = 0, py = 0;
   GLfloat tx[4], ty[4], t2x[4], t2y[4];
   DATA32 cmul;
   Shader_Sampling masksam = SHD_SAM11;
   Evas_GL_Program *prog;
   Eina_Bool utexture = EINA_FALSE;
   Eina_Bool uvtexture = EINA_FALSE;
   Eina_Bool use_texa = EINA_FALSE;
   Shader_Type type;
   int pn = 0, i;
   int nomul = 0, yinvert = 0;
   Eina_Bool flat = EINA_FALSE;
   Eina_Bool blend = EINA_FALSE;

   if (!(gc->dc->render_op == EVAS_RENDER_COPY) &&
       ((a < 255) || (tex->alpha) || (!!mtex))) blend = EINA_TRUE;

   if ((A_VAL(&(p[0].col)) < 0xff) || (A_VAL(&(p[1].col)) < 0xff) ||
       (A_VAL(&(p[2].col)) < 0xff) || (A_VAL(&(p[3].col)) < 0xff))
     blend = EINA_TRUE;

   if ((p[0].z == p[1].z) && (p[1].z == p[2].z) && (p[2].z == p[3].z))
      flat = EINA_TRUE;

   if (!clip) cx = cy = cw = ch = 0;

   if (!flat)
     {
        if (p[0].foc <= 0) flat = EINA_TRUE;
     }

   switch (cspace)
     {
      case EVAS_COLORSPACE_YCBCR422P601_PL:
        type = SHD_YUV;
        utexture = EINA_TRUE;
        break;
      case EVAS_COLORSPACE_YCBCR422P709_PL:
        type = SHD_YUV_709;
        utexture = EINA_TRUE;
        break;
      case EVAS_COLORSPACE_YCBCR422601_PL:
        type = SHD_YUY2;
        uvtexture = EINA_TRUE;
        break;
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
        type = SHD_NV12;
        uvtexture = EINA_TRUE;
        break;
      case EVAS_COLORSPACE_ETC1_ALPHA:
        type = SHD_RGB_A_PAIR;
        use_texa = EINA_TRUE;
        break;
      default:
        type = SHD_MAP;
        break;
     }
   prog = evas_gl_common_shader_program_get(gc, type, p, npoints, r, g, b, a,
                                            w, h, w, h, smooth, tex, tex_only,
                                            mtex, mask_smooth, mask_color, mw, mh,
                                            EINA_FALSE, NULL, &nomul, &masksam);

   x = w = (p[0].x >> FP);
   y = h = (p[0].y >> FP);
   for (i = 0; i < 4; i++)
     {
        tx[i] = ((double)(tex->x) + (((double)p[i].u) / FP1)) /
          (double)tex->pt->w;
        ty[i] = ((double)(tex->y) + (((double)p[i].v) / FP1)) /
          (double)tex->pt->h;
        px = (p[i].x >> FP);
        if      (px < x) x = px;
        else if (px > w) w = px;
        py = (p[i].y >> FP);
        if      (py < y) y = py;
        else if (py > h) h = py;
        if (utexture)
          {
             t2x[i] = ((((double)p[i].u / 2) / FP1)) / (double)tex->ptu->w;
             t2y[i] = ((((double)p[i].v / 2) / FP1)) / (double)tex->ptu->h;
          }
        else if (uvtexture)
          {
             t2x[i] = ((((double)p[i].u / 2) / FP1)) / (double)tex->ptuv->w;
             t2y[i] = ((((double)p[i].v / 2) / FP1)) / (double)tex->ptuv->h;
          }
     }
   w = w - x;
   h = h - y;

   if (!flat)
     {
        // FUZZZZ!
        x -= 3;
        y -= 3;
        w += 6;
        h += 6;
     }
   if (clip)
     {
        if (flat)
          {
             int nx = x, ny = y, nw = w, nh = h;

             RECTS_CLIP_TO_RECT(nx, ny, nw, nh, cx, cy, cw, ch);
             if ((nx == x) && (ny == y) && (nw == w) && (nh == h))
               {
                  clip = 0; cx = 0; cy = 0; cw = 0; ch = 0;
               }
             x = nx; y = ny; w = nw; h = nh;
          }
     }

   if (!flat)
     {
        shader_array_flush(gc);
        gc->foc = p[0].foc >> FP;
        gc->z0 = p[0].z0 >> FP;
        gc->px = p[0].px >> FP;
        gc->py = p[0].py >> FP;
        gc->change.size = 1;
        _evas_gl_common_viewport_set(gc);
     }

   pn = _evas_gl_common_context_push(SHD_MAP,
                                     gc, tex, mtex,
                                     prog,
                                     x, y, w, h,
                                     blend,
                                     smooth,
                                     clip, cx, cy, cw, ch,
                                     mask_smooth);
   gc->pipe[pn].region.type = SHD_MAP;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   if (utexture)
     {
       gc->pipe[pn].shader.cur_texu = tex->ptu->texture;
       gc->pipe[pn].shader.cur_texu_dyn = tex->ptu->dyn.img;
       gc->pipe[pn].shader.cur_texv = tex->ptv->texture;
       gc->pipe[pn].shader.cur_texv_dyn = tex->ptv->dyn.img;
     }
   else if (uvtexture)
     {
       gc->pipe[pn].shader.cur_texu = tex->ptuv->texture;
       gc->pipe[pn].shader.cur_texu_dyn = tex->ptuv->dyn.img;
     }
   gc->pipe[pn].shader.cur_texm = mtex ? mtex->pt->texture : 0;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.mask_smooth = mask_smooth;
   gc->pipe[pn].shader.clip = clip;
   gc->pipe[pn].shader.cx = cx;
   gc->pipe[pn].shader.cy = cy;
   gc->pipe[pn].shader.cw = cw;
   gc->pipe[pn].shader.ch = ch;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = !nomul;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = (utexture || uvtexture) ? 1 : 0;
   gc->pipe[pn].array.use_texuv3 = (utexture) ? 1 : 0;
   gc->pipe[pn].array.use_mask = !!mtex;
   gc->pipe[pn].array.use_texa = use_texa;
   gc->pipe[pn].array.use_texsam = 0;
   gc->pipe[pn].array.use_masksam = (masksam != SHD_SAM11);

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 6);

   if ((tex->im) && (tex->im->native.data))
     {
        if (tex->im->native.func.yinvert)
          yinvert = tex->im->native.func.yinvert(tex->im);
        else
          yinvert = tex->im->native.yinvert;
     }

   if ((tex->im) && (tex->im->native.data) && (!yinvert))
     {
        for (i = 0; i < 4; i++)
          {
             ty[i] = 1.0 - ty[i];
             if (utexture || uvtexture)
                t2y[i] = 1.0 - t2y[i];
          }
     }

   cmul = ARGB_JOIN(a, r, g, b);
   for (i = 0; i < 6; i++)
     {
        DATA32 cl = MUL4_SYM(cmul, p[points[i]].col);
        if (flat)
          {
             PUSH_VERTEX(pn,
                         (p[points[i]].x >> FP),
                         (p[points[i]].y >> FP),
                         0);
          }
        else
          {
             PUSH_VERTEX(pn,
                         (p[points[i]].fx) + gc->shared->ax,
                         (p[points[i]].fy) + gc->shared->ay,
                         (p[points[i]].fz)
                         + (gc->shared->foc - gc->shared->z0));
          }
        PUSH_TEXUV(pn,
                   tx[points[i]],
                   ty[points[i]]);
        if (utexture)
          {
             PUSH_TEXUV2(pn,
                         t2x[points[i]],
                         t2y[points[i]]);
             PUSH_TEXUV3(pn,
                         t2x[points[i]],
                         t2y[points[i]]);
          }
        else if (uvtexture)
          {
             PUSH_TEXUV2(pn,
                         t2x[points[i]],
                         t2y[points[i]]);
          }

        if (!nomul)
          {
             PUSH_COLOR(pn,
                        R_VAL(&cl),
                        G_VAL(&cl),
                        B_VAL(&cl),
                        A_VAL(&cl));
          }
     }

   PUSH_MASK(pn, mtex, mx, my, mw, mh, masksam);

   if (!flat)
     {
        shader_array_flush(gc);
        gc->foc = 0;
        gc->z0 = 0;
        gc->px = 0;
        gc->py = 0;
        gc->change.size = 1;
        _evas_gl_common_viewport_set(gc);
     }
}

// ----------------------------------------------------------------------------
// Gfx Filters

static inline void
_filter_data_flush(Evas_Engine_GL_Context *gc, Evas_GL_Program *prog)
{
   // filter_data is not using a vertex array, which means early flushes
   // are necessary. Vertex arrays crashed for whatever reason :(
   for (size_t k = 0; k < MAX_PIPES; k++)
     if ((gc->pipe[k].array.filter_data || gc->pipe[k].shader.filter.map_tex)
         && (gc->pipe[k].shader.prog == prog))
       {
          shader_array_flush(gc);
          break;
       }
}

static inline void
_filter_data_alloc(Evas_Engine_GL_Context *gc, int pn, int count)
{
   gc->pipe[pn].array.filter_data_count = count;
   if (count) gc->pipe[pn].array.filter_data = malloc(count * 2 * sizeof(GLfloat));
   else gc->pipe[pn].array.filter_data = NULL;
}

static inline void
_filter_data_prepare(Evas_Engine_GL_Context *gc EINA_UNUSED,
                     Evas_GL_Program *prog, int count)
{
   if (!prog->filter) prog->filter = calloc(1, sizeof(*prog->filter));
   if (!prog->filter->attribute.known_locations)
     {
        prog->filter->attribute.known_locations = EINA_TRUE;
        for (int k = 0; k < count; k++)
          {
             char name[32];

             sprintf(name, "filter_data_%d", k);
             prog->filter->attribute.loc[k] = glGetAttribLocation(prog->prog, name);
          }
     }
}

void
evas_gl_common_filter_displace_push(Evas_Engine_GL_Context *gc,
                                    Evas_GL_Texture *tex, Evas_GL_Texture *map_tex,
                                    int x, int y, int w, int h, double dx, double dy,
                                    Eina_Bool nearest)
{
   double sx, sy, sw, sh, pw, ph;
   double ox1, oy1, ox2, oy2, ox3, oy3, ox4, oy4;
   GLfloat tx1, ty1, tx2, ty2, tx3, ty3, tx4, ty4;
   Shader_Sampling sam = SHD_SAM11;
   GLfloat offsetx, offsety;
   GLfloat *filter_data;
   int r, g, b, a, nomul = 0, pn;
   Evas_GL_Program *prog;
   Eina_Bool blend = EINA_TRUE;
   Eina_Bool smooth = EINA_TRUE;

   r = R_VAL(&gc->dc->mul.col);
   g = G_VAL(&gc->dc->mul.col);
   b = B_VAL(&gc->dc->mul.col);
   a = A_VAL(&gc->dc->mul.col);
   if (gc->dc->render_op == EVAS_RENDER_COPY)
     blend = EINA_FALSE;

   prog = evas_gl_common_shader_program_get(gc, SHD_FILTER_DISPLACE, NULL, 0, r, g, b, a,
                                            w, h, w, h, smooth, tex, EINA_FALSE,
                                            NULL, EINA_FALSE, EINA_FALSE, 0, 0,
                                            EINA_FALSE, &sam, &nomul, NULL);
   _filter_data_flush(gc, prog);

   pn = _evas_gl_common_context_push(SHD_FILTER_DISPLACE, gc, tex, NULL, prog,
                                     x, y, w, h, blend, smooth,
                                     0, 0, 0, 0, 0, EINA_FALSE);

   gc->pipe[pn].region.type = SHD_FILTER_DISPLACE;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_texm = 0;
   gc->pipe[pn].shader.tex_target = GL_TEXTURE_2D;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.mask_smooth = 0;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = !nomul;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;
   gc->pipe[pn].array.use_texsam = (sam != SHD_SAM11);
   gc->pipe[pn].array.use_mask = 0;
   gc->pipe[pn].array.use_masksam = 0;

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 6);

   // displace properties
   gc->pipe[pn].shader.filter.map_tex = map_tex->pt->texture;
   gc->pipe[pn].shader.filter.map_nearest = nearest;
   _filter_data_prepare(gc, prog, 3);
   _filter_data_alloc(gc, pn, 3);

   sx = x;
   sy = y;
   sw = w;
   sh = h;

   pw = tex->pt->w;
   ph = tex->pt->h;

   ox1 = sx;
   oy1 = sy;
   ox2 = sx + sw;
   oy2 = sy;
   ox3 = sx + sw;
   oy3 = sy + sh;
   ox4 = sx;
   oy4 = sy + sh;

   offsetx = tex->x;
   offsety = tex->y;

   tx1 = ((double)(offsetx) + ox1) / pw;
   ty1 = ((double)(offsety) + oy1) / ph;
   tx2 = ((double)(offsetx) + ox2) / pw;
   ty2 = ((double)(offsety) + oy2) / ph;
   tx3 = ((double)(offsetx) + ox3) / pw;
   ty3 = ((double)(offsety) + oy3) / ph;
   tx4 = ((double)(offsetx) + ox4) / pw;
   ty4 = ((double)(offsety) + oy4) / ph;

   PUSH_6_VERTICES(pn, x, y, w, h);
   PUSH_6_QUAD(pn, tx1, ty1, tx2, ty2, tx3, ty3, tx4, ty4);

   if (sam)
     {
        double samx = (double)(sw) / (double)(tex->pt->w * w * 4);
        double samy = (double)(sh) / (double)(tex->pt->h * h * 4);
        PUSH_SAMPLES(pn, samx, samy);
     }

   filter_data = gc->pipe[pn].array.filter_data;
   filter_data[0] = (dx * tex->w / pw) / w;
   filter_data[1] = (dy * tex->h / ph) / h;
   filter_data[2] = tex->x / pw;
   filter_data[3] = tex->y / ph;
   filter_data[4] = (tex->x + tex->w) / pw;
   filter_data[5] = (tex->y + tex->h) / ph;

   if (!nomul)
     PUSH_6_COLORS(pn, r, g, b, a);
}

void
evas_gl_common_filter_curve_push(Evas_Engine_GL_Context *gc,
                                 Evas_GL_Texture *tex,
                                 int x, int y, int w, int h,
                                 const uint8_t *points, int channel)
{
   double sx, sy, sw, sh, pw, ph;
   double ox1, oy1, ox2, oy2, ox3, oy3, ox4, oy4;
   GLfloat tx1, ty1, tx2, ty2, tx3, ty3, tx4, ty4;
   Shader_Sampling sam = SHD_SAM11;
   GLfloat offsetx, offsety;
   int r, g, b, a, nomul = 0, pn, k;
   uint32_t values[256];
   Evas_GL_Program *prog;
   Eina_Bool blend = EINA_TRUE;
   Eina_Bool smooth = EINA_TRUE;
   GLuint map_tex;

   r = R_VAL(&gc->dc->mul.col);
   g = G_VAL(&gc->dc->mul.col);
   b = B_VAL(&gc->dc->mul.col);
   a = A_VAL(&gc->dc->mul.col);
   if (gc->dc->render_op == EVAS_RENDER_COPY)
     blend = EINA_FALSE;

   prog = evas_gl_common_shader_program_get(gc, SHD_FILTER_CURVE, NULL, 0, r, g, b, a,
                                            w, h, w, h, smooth, tex, EINA_FALSE,
                                            NULL, EINA_FALSE, EINA_FALSE, 0, 0,
                                            EINA_FALSE, &sam, &nomul, NULL);
   _filter_data_flush(gc, prog);

   pn = _evas_gl_common_context_push(SHD_FILTER_CURVE, gc, tex, NULL, prog,
                                     x, y, w, h, blend, smooth,
                                     0, 0, 0, 0, 0, EINA_FALSE);

   gc->pipe[pn].region.type = SHD_FILTER_CURVE;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_texm = 0;
   gc->pipe[pn].shader.tex_target = GL_TEXTURE_2D;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.mask_smooth = 0;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = !nomul;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;
   gc->pipe[pn].array.use_texsam = (sam != SHD_SAM11);
   gc->pipe[pn].array.use_mask = 0;
   gc->pipe[pn].array.use_masksam = 0;

   pipe_region_expand(gc, pn, x, y, w, h);
   PIPE_GROW(gc, pn, 6);

   // Build 256 colors map in RGBA
   switch (channel)
     {
      case 0: // EVAS_FILTER_CHANNEL_ALPHA
        for (k = 0; k < 256; k++)
          values[k] = ARGB_JOIN(points[k], k, k, k);
        break;
      case 1:  // EVAS_FILTER_CHANNEL_RED
        for (k = 0; k < 256; k++)
          values[k] = ARGB_JOIN(k, points[k], k, k);
        break;
      case 2:  // EVAS_FILTER_CHANNEL_GREEN
        for (k = 0; k < 256; k++)
          values[k] = ARGB_JOIN(k, k, points[k], k);
        break;
      case 3:  // EVAS_FILTER_CHANNEL_BLUE
        for (k = 0; k < 256; k++)
          values[k] = ARGB_JOIN(k, k, k, points[k]);
        break;
      case 4:  // EVAS_FILTER_CHANNEL_RGB
        for (k = 0; k < 256; k++)
          values[k] = ARGB_JOIN(k, points[k], points[k], points[k]);
        break;
      case 5: // ALPHA only
        for (k = 0; k < 256; k++)
          values[k] = ARGB_JOIN(points[k], points[k], points[k], points[k]);
        break;
      default: // INVALID: no curve applied
        for (k = 0; k < 256; k++)
          values[k] = ARGB_JOIN(k, k, k, k);
        break;
     }

   // Synchronous upload of 256x1 RGBA texture (FIXME: no reuse)
   glGenTextures(1, &map_tex);
   glBindTexture(GL_TEXTURE_2D, map_tex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, values);

   // Set curve properties (no need for filter_data)
   gc->pipe[pn].shader.filter.map_tex = map_tex;
   gc->pipe[pn].shader.filter.map_nearest = EINA_TRUE;
   gc->pipe[pn].shader.filter.map_delete = EINA_TRUE;
   gc->pipe[pn].array.filter_data_count = 0;

   sx = x;
   sy = y;
   sw = w;
   sh = h;

   pw = tex->pt->w;
   ph = tex->pt->h;

   ox1 = sx;
   oy1 = sy;
   ox2 = sx + sw;
   oy2 = sy;
   ox3 = sx + sw;
   oy3 = sy + sh;
   ox4 = sx;
   oy4 = sy + sh;

   offsetx = tex->x;
   offsety = tex->y;

   tx1 = ((double)(offsetx) + ox1) / pw;
   ty1 = ((double)(offsety) + oy1) / ph;
   tx2 = ((double)(offsetx) + ox2) / pw;
   ty2 = ((double)(offsety) + oy2) / ph;
   tx3 = ((double)(offsetx) + ox3) / pw;
   ty3 = ((double)(offsety) + oy3) / ph;
   tx4 = ((double)(offsetx) + ox4) / pw;
   ty4 = ((double)(offsety) + oy4) / ph;

   PUSH_6_VERTICES(pn, x, y, w, h);
   PUSH_6_QUAD(pn, tx1, ty1, tx2, ty2, tx3, ty3, tx4, ty4);

   if (sam)
     {
        double samx = (double)(sw) / (double)(tex->pt->w * w * 4);
        double samy = (double)(sh) / (double)(tex->pt->h * h * 4);
        PUSH_SAMPLES(pn, samx, samy);
     }

   if (!nomul)
     PUSH_6_COLORS(pn, r, g, b, a);
}

void
evas_gl_common_filter_blend_push(Evas_Engine_GL_Context *gc,
                                 Evas_GL_Texture *tex,
                                 double sx, double sy, double sw, double sh,
                                 double dx, double dy, double dw, double dh,
                                 Eina_Bool alphaonly)
{
   int r, g, b, a;

   r = R_VAL(&gc->dc->mul.col);
   g = G_VAL(&gc->dc->mul.col);
   b = B_VAL(&gc->dc->mul.col);
   a = A_VAL(&gc->dc->mul.col);
   if (alphaonly)
     r = g = b = a;

   evas_gl_common_context_image_push(gc, tex, sx, sy, sw, sh, dx, dy, dw, dh,
                                     NULL, 0, 0, 0, 0, EINA_FALSE, EINA_FALSE,
                                     r, g, b, a, EINA_TRUE, EINA_FALSE,
                                     alphaonly);
}

void
evas_gl_common_filter_blur_push(Evas_Engine_GL_Context *gc,
                                Evas_GL_Texture *tex,
                                double sx, double sy, double sw, double sh,
                                double dx, double dy, double dw, double dh,
                                const double * const weights,
                                const double * const offsets, int count,
                                double radius, Eina_Bool horiz, Eina_Bool alphaonly)
{
   double ox1, oy1, ox2, oy2, ox3, oy3, ox4, oy4, pw, ph, texlen;
   GLfloat tx1, ty1, tx2, ty2, tx3, ty3, tx4, ty4;
   GLfloat offsetx, offsety;
   int r, g, b, a, nomul = 0, pn;
   Evas_GL_Program *prog;
   Eina_Bool blend = EINA_TRUE;
   Eina_Bool smooth = EINA_TRUE;
   Shader_Type type = horiz ? SHD_FILTER_BLUR_X : SHD_FILTER_BLUR_Y;
   Eina_Bool update_uniforms = EINA_FALSE;
   GLuint *map_tex_data;
   double sum = 0.0;

   r = R_VAL(&gc->dc->mul.col);
   g = G_VAL(&gc->dc->mul.col);
   b = B_VAL(&gc->dc->mul.col);
   a = A_VAL(&gc->dc->mul.col);
   if (gc->dc->render_op == EVAS_RENDER_COPY)
     blend = EINA_FALSE;

   prog = evas_gl_common_shader_program_get(gc, type, NULL, 0, r, g, b, a,
                                            sw, sh, dw, dh, smooth, tex, EINA_FALSE,
                                            NULL, EINA_FALSE, EINA_FALSE, 0, 0,
                                            alphaonly, NULL, &nomul, NULL);

   _filter_data_flush(gc, prog);
   EINA_SAFETY_ON_NULL_RETURN(prog);

   pw = tex->pt->w;
   ph = tex->pt->h;
   texlen = horiz ? pw : ph;

   /* Convert double data to RGBA pixel data.
    *
    * We are not using GL_FLOAT or GL_DOUBLE because:
    * - It's not as portable (needs extensions),
    * - GL_DOUBLE didn't work during my tests (dunno why),
    * - GL_FLOAT didn't seem to carry the proper precision all the way to
    *   the fragment shader,
    * - Real data buffers are not available in GLES 2.0,
    * - GL_RGBA is 100% portable.
    */
   map_tex_data = alloca(2 * count * sizeof(*map_tex_data));
   for (int k = 0; k < count; k++)
     {
        GLuint val;

        if (k == 0) sum = weights[k];
        else sum += 2.0 * weights[k];

        // Weight is always > 0.0 and < 255.0 by maths
        val = (GLuint) (weights[k] * 256.0 * 256.0 * 256.0);
        map_tex_data[k] = val;

        // Offset is always in [0.0 , 1.0] by definition
        val = (GLuint) (offsets[k] * 256.0 * 256.0 * 256.0);
        map_tex_data[k + count] = val;
     }

   // Prepare attributes & uniforms
   _filter_data_prepare(gc, prog, 0);
   if (!prog->filter->uniform.known_locations)
     {
        prog->filter->uniform.known_locations = EINA_TRUE;
        prog->filter->uniform.blur_count_loc = glGetUniformLocation(prog->prog, "blur_count");
        prog->filter->uniform.blur_texlen_loc = glGetUniformLocation(prog->prog, "blur_texlen");
        prog->filter->uniform.blur_div_loc = glGetUniformLocation(prog->prog, "blur_div");
     }

   if ((prog->filter->uniform.blur_count_value != count - 1) ||
       (!EINA_FLT_EQ(prog->filter->uniform.blur_texlen_value, texlen)) ||
       (!EINA_FLT_EQ(prog->filter->uniform.blur_div_value, sum)))
     {
        update_uniforms = EINA_TRUE;
        shader_array_flush(gc);
     }

   pn = _evas_gl_common_context_push(type, gc, tex, NULL, prog,
                                     sx, sy, dw, dh, blend, smooth,
                                     0, 0, 0, 0, 0, EINA_FALSE);

   gc->pipe[pn].region.type = type;
   gc->pipe[pn].shader.prog = prog;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_texm = 0;
   gc->pipe[pn].shader.tex_target = GL_TEXTURE_2D;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.mask_smooth = 0;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = !nomul;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;
   gc->pipe[pn].array.use_texsam = 0;
   gc->pipe[pn].array.use_mask = 0;
   gc->pipe[pn].array.use_masksam = 0;

   pipe_region_expand(gc, pn, dx, dy, dw, dh);
   PIPE_GROW(gc, pn, 6);

   _filter_data_alloc(gc, pn, 0);

   // Synchronous upload of Nx2 RGBA texture
   if (!EINA_DBL_EQ(prog->filter->blur_radius, radius))
     {
        prog->filter->blur_radius = radius;

        if (!prog->filter->texture.tex_ids[0])
          glGenTextures(1, prog->filter->texture.tex_ids);

        glBindTexture(GL_TEXTURE_2D, prog->filter->texture.tex_ids[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        if (tex->gc->shared->info.unpack_row_length)
          glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, sizeof(*map_tex_data));
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, count, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, map_tex_data);
     }

   if (update_uniforms)
     {
        prog->filter->uniform.blur_count_value = count - 1;
        prog->filter->uniform.blur_texlen_value = texlen;
        prog->filter->uniform.blur_div_value = sum;
        glUseProgram(prog->prog);
        glUniform1i(prog->filter->uniform.blur_count_loc, prog->filter->uniform.blur_count_value);
        glUniform1f(prog->filter->uniform.blur_texlen_loc, prog->filter->uniform.blur_texlen_value);
        glUniform1f(prog->filter->uniform.blur_div_loc, prog->filter->uniform.blur_div_value);
     }

   // Set shader properties
   gc->pipe[pn].shader.filter.map_tex = prog->filter->texture.tex_ids[0];
   gc->pipe[pn].shader.filter.map_nearest = EINA_TRUE;
   gc->pipe[pn].shader.filter.map_delete = EINA_FALSE;

   ox1 = sx;
   oy1 = sy;
   ox2 = sx + sw;
   oy2 = sy;
   ox3 = sx + sw;
   oy3 = sy + sh;
   ox4 = sx;
   oy4 = sy + sh;

   offsetx = tex->x;
   offsety = tex->y;

   tx1 = ((double)(offsetx) + ox1) / pw;
   ty1 = ((double)(offsety) + oy1) / ph;
   tx2 = ((double)(offsetx) + ox2) / pw;
   ty2 = ((double)(offsety) + oy2) / ph;
   tx3 = ((double)(offsetx) + ox3) / pw;
   ty3 = ((double)(offsety) + oy3) / ph;
   tx4 = ((double)(offsetx) + ox4) / pw;
   ty4 = ((double)(offsety) + oy4) / ph;

   PUSH_6_VERTICES(pn, dx, dy, dw, dh);
   PUSH_6_QUAD(pn, tx1, ty1, tx2, ty2, tx3, ty3, tx4, ty4);

   if (!nomul)
     PUSH_6_COLORS(pn, r, g, b, a);
}

// ----------------------------------------------------------------------------

EAPI void
evas_gl_common_context_flush(Evas_Engine_GL_Context *gc)
{
   shader_array_flush(gc);
}

static void
scissor_rot(Evas_Engine_GL_Context *gc EINA_UNUSED,
            int rot, int gw, int gh, int cx, int cy, int cw, int ch)
{
   switch (rot)
     {
      case 0: // UP this way: ^
        glScissor(cx, cy, cw, ch);
        break;
      case 90: // UP this way: <
        glScissor(gh - (cy + ch), cx, ch, cw);
        break;
      case 180: // UP this way: v
        glScissor(gw - (cx + cw), gh - (cy + ch), cw, ch);
        break;
      case 270: // UP this way: >
        glScissor(cy, gw - (cx + cw), ch, cw);
        break;
      default: // assume up is up
        glScissor(cx, cy, cw, ch);
        break;
     }
}

static void
start_tiling(Evas_Engine_GL_Context *gc EINA_UNUSED,
             int rot, int gw, int gh, int cx, int cy, int cw, int ch,
             int bitmask)
{
   if (!glsym_glStartTiling) return;
   switch (rot)
     {
      case 0: // UP this way: ^
        glsym_glStartTiling(cx, cy, cw, ch, bitmask);
        break;
      case 90: // UP this way: <
        glsym_glStartTiling(gh - (cy + ch), cx, ch, cw, bitmask);
        break;
      case 180: // UP this way: v
        glsym_glStartTiling(gw - (cx + cw), gh - (cy + ch), cw, ch, bitmask);
        break;
      case 270: // UP this way: >
        glsym_glStartTiling(cy, gw - (cx + cw), ch, cw, bitmask);
        break;
      default: // assume up is up
        glsym_glStartTiling(cx, cy, cw, ch, bitmask);
        break;
     }
   GLERRV("glsym_glStartTiling");
}

static void
shader_array_flush(Evas_Engine_GL_Context *gc)
{
   int i, gw, gh, offx = 0, offy = 0;
   unsigned int pipe_done = 0;  //count pipe iteration for debugging
   Eina_Bool setclip;
   Eina_Bool fbo = EINA_FALSE;

   if (!gc->havestuff) return;
   gw = gc->w;
   gh = gc->h;
   if (!((gc->pipe[0].shader.surface == gc->def_surface) ||
         (!gc->pipe[0].shader.surface)))
     {
        gw = gc->pipe[0].shader.surface->w;
        gh = gc->pipe[0].shader.surface->h;
        fbo = EINA_TRUE;
        offx = gc->pipe[0].shader.surface->tex->x;
        offy = gc->pipe[0].shader.surface->tex->y;
     }
   for (i = 0; i < gc->shared->info.tune.pipes.max; i++)
     {
        Evas_GL_Program *prog;

        if (gc->pipe[i].array.num <= 0) break;

        prog = gc->pipe[i].shader.prog;
        setclip = EINA_FALSE;
        pipe_done++;
        gc->flushnum++;

        GLERRV("<flush err>");
        if (prog && (prog != gc->state.current.prog))
          {
             glUseProgram(prog->prog);
             if (prog->reset)
               {
                  glUniform1i(prog->uniform.rotation_id, fbo ? 0 : gc->rot / 90);
                  glUniformMatrix4fv(prog->uniform.mvp, 1, GL_FALSE, gc->shared->proj);
                  prog->reset = EINA_FALSE;
               }
          }

        if (gc->pipe[i].shader.cur_tex != gc->state.current.cur_tex)
          {
#if 0
             if (gc->pipe[i].shader.cur_tex)
               {
                  glEnable(GL_TEXTURE_2D);
               }
             else
               {
                  glDisable(GL_TEXTURE_2D);
               }
#endif
             glActiveTexture(GL_TEXTURE0);
             glBindTexture(gc->pipe[i].shader.tex_target, gc->pipe[i].shader.cur_tex);
          }
        if (gc->pipe[i].array.im)
          {
#ifdef GL_GLES
             if (gc->pipe[i].array.im->tex->pt->dyn.img)
               {
                  secsym_glEGLImageTargetTexture2DOES
                        (gc->pipe[i].array.im->tex->pt->dyn.target, gc->pipe[i].array.im->tex->pt->dyn.img);
               }
             else
#endif
               {
                  if (!gc->pipe[i].array.im->native.loose)
                    {
                       if (gc->pipe[i].array.im->native.func.bind)
                         gc->pipe[i].array.im->native.func.bind(gc->pipe[i].array.im);
                    }
               }
          }
        if ((gc->pipe[i].shader.render_op != gc->state.current.render_op) ||
            gc->state.current.anti_alias)
          {
             switch (gc->pipe[i].shader.render_op)
               {
                case EVAS_RENDER_BLEND: /**< default op: d = d*(1-sa) + s */
                  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                  break;
                case EVAS_RENDER_BLEND_REL: /**< d = d*(1 - sa) + s*da */
                  glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                  break;
                case EVAS_RENDER_COPY: /**< d = s */
                  gc->pipe[i].shader.blend = 0;
                  // just disable blend mode. no need to set blend func
                  //glBlendFunc(GL_ONE, GL_ZERO);
                  break;
                case EVAS_RENDER_COPY_REL: /**< d = s*da */
                  glBlendFunc(GL_DST_ALPHA, GL_ZERO);
                  break;
                case EVAS_RENDER_ADD: /**< d = d + s */
                  glBlendFunc(GL_ONE, GL_ONE);
                  break;
                case EVAS_RENDER_ADD_REL: /**< d = d + s*da */
                  glBlendFunc(GL_DST_ALPHA, GL_ONE);
                  break;
                case EVAS_RENDER_SUB: /**< d = d - s */
                  glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
                  break;
                case EVAS_RENDER_SUB_REL: /**< d = d - s*da */
                  glBlendFunc(GL_ZERO, GL_ONE_MINUS_DST_ALPHA);
                  break;
                case EVAS_RENDER_MASK: /**< d = d*sa */
                  glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
                  break;
                  // FIXME: fix blend funcs below!
                case EVAS_RENDER_TINT: /**< d = d*s + d*(1 - sa) + s*(1 - da) */
                case EVAS_RENDER_TINT_REL: /**< d = d*(1 - sa + s) */
                case EVAS_RENDER_MUL: /**< d = d*s */
                default:
                  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                  break;
               }
          }
        if (gc->pipe[i].shader.blend != gc->state.current.blend)
          {
             if (gc->pipe[i].shader.blend)
               glEnable(GL_BLEND);
             else
               glDisable(GL_BLEND);
          }
        if ((gc->pipe[i].shader.smooth != gc->state.current.smooth) ||
            (gc->pipe[i].shader.cur_tex != gc->state.current.cur_tex))
          {
             if (gc->pipe[i].shader.smooth)
               {
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
                  if (shared->info.anisotropic > 0.0)
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, shared->info.anisotropic);
#endif
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
               }
             else
               {
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
                  if (shared->info.anisotropic > 0.0)
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
#endif
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
               }
          }
        if (gc->pipe[i].shader.clip != gc->state.current.clip)
          {
             int cx, cy, cw, ch;

             cx = gc->pipe[i].shader.cx;
             cy = gc->pipe[i].shader.cy;
             cw = gc->pipe[i].shader.cw;
             ch = gc->pipe[i].shader.ch;
             if ((gc->master_clip.enabled) && (!fbo))
               {
                  if (gc->pipe[i].shader.clip)
                    {
                       RECTS_CLIP_TO_RECT(cx, cy, cw, ch,
                                          gc->master_clip.x, gc->master_clip.y,
                                          gc->master_clip.w, gc->master_clip.h);
                    }
                  else
                    {
                       cx = gc->master_clip.x;
                       cy = gc->master_clip.y;
                       cw = gc->master_clip.w;
                       ch = gc->master_clip.h;
                    }
               }
             if ((glsym_glStartTiling) && (glsym_glEndTiling) &&
                 (gc->master_clip.enabled) &&
                 (gc->master_clip.w > 0) && (gc->master_clip.h > 0))
               {
                  if (!gc->master_clip.used)
                    {
                       if (!fbo)
                         {
                            start_tiling(gc, gc->rot, gw, gh,
                                         gc->master_clip.x + offx,
                                         gh - gc->master_clip.y - offy - gc->master_clip.h,
                                         gc->master_clip.w, gc->master_clip.h,
                                         gc->preserve_bit);
                            if (!gc->preserve_bit)
                              gc->preserve_bit = GL_COLOR_BUFFER_BIT0_QCOM;
                         }
                       else
                         start_tiling(gc, 0, gw, gh,
                                      gc->master_clip.x + offx,
                                      gc->master_clip.y + offy,
                                      gc->master_clip.w, gc->master_clip.h, 0);
                       gc->master_clip.used = EINA_TRUE;
                    }
               }
             if ((gc->pipe[i].shader.clip) ||
                 ((gc->master_clip.enabled) && (!fbo)))
               {
                  glEnable(GL_SCISSOR_TEST);
                  if (!fbo)
                    scissor_rot(gc, gc->rot, gw, gh,
                                cx + offx,
                                gh - cy - offy - ch,
                                cw, ch);
                  else
                     glScissor(cx + offx, cy + offy, cw, ch);
                  setclip = EINA_TRUE;
                  gc->state.current.cx = cx;
                  gc->state.current.cy = cy;
                  gc->state.current.cw = cw;
                  gc->state.current.ch = ch;
               }
             else
               {
                  glDisable(GL_SCISSOR_TEST);
                  glScissor(0, 0, 0, 0);
                  gc->state.current.cx = 0;
                  gc->state.current.cy = 0;
                  gc->state.current.cw = 0;
                  gc->state.current.ch = 0;
               }
          }
        if (((gc->pipe[i].shader.clip) && (!setclip)) ||
            ((gc->master_clip.enabled) && (!fbo)))
          {
             int cx, cy, cw, ch;

             cx = gc->pipe[i].shader.cx;
             cy = gc->pipe[i].shader.cy;
             cw = gc->pipe[i].shader.cw;
             ch = gc->pipe[i].shader.ch;
             if ((gc->master_clip.enabled) && (!fbo))
               {
                  if (gc->pipe[i].shader.clip)
                    {
                       RECTS_CLIP_TO_RECT(cx, cy, cw, ch,
                                          gc->master_clip.x, gc->master_clip.y,
                                          gc->master_clip.w, gc->master_clip.h);
                    }
                  else
                    {
                       cx = gc->master_clip.x;
                       cy = gc->master_clip.y;
                       cw = gc->master_clip.w;
                       ch = gc->master_clip.h;
                    }
               }
             if ((cx != gc->state.current.cx) ||
                 (cy != gc->state.current.cy) ||
                 (cw != gc->state.current.cw) ||
                 (ch != gc->state.current.ch))
               {
                  if (!fbo)
                    scissor_rot(gc, gc->rot, gw, gh,
                                cx + offx,
                                gh - cy - offy - ch,
                                cw, ch);
                  else
                    glScissor(cx, cy, cw, ch);
                  gc->state.current.cx = cx;
                  gc->state.current.cy = cy;
                  gc->state.current.cw = cw;
                  gc->state.current.ch = ch;
               }
          }

        unsigned char *vertex_ptr = NULL;
        unsigned char *color_ptr = NULL;
        unsigned char *texuv_ptr = NULL;
        unsigned char *texuv2_ptr = NULL;
        unsigned char *texuv3_ptr = NULL;
        unsigned char *texa_ptr = NULL;
        unsigned char *texsam_ptr = NULL;
        unsigned char *mask_ptr = NULL;
        unsigned char *masksam_ptr = NULL;
        GLint ACTIVE_TEXTURE = GL_TEXTURE0;

        if (glsym_glMapBuffer && glsym_glUnmapBuffer)
          {
             unsigned char *x;

# define VERTEX_SIZE (gc->pipe[i].array.num * sizeof(GLshort) * VERTEX_CNT)
# define COLOR_SIZE (gc->pipe[i].array.num * sizeof(GLubyte) * COLOR_CNT)
# define TEX_SIZE (gc->pipe[i].array.num * sizeof(GLfloat) * TEX_CNT)
# define SAM_SIZE (gc->pipe[i].array.num * sizeof(GLfloat) * SAM_CNT)
# define MASK_SIZE (gc->pipe[i].array.num * sizeof(GLfloat) * MASK_CNT)
             vertex_ptr = NULL;
             color_ptr = vertex_ptr + VERTEX_SIZE;
             texuv_ptr = color_ptr + COLOR_SIZE;
             texuv2_ptr = texuv_ptr + TEX_SIZE;
             texuv3_ptr = texuv2_ptr + TEX_SIZE;
             texa_ptr = texuv3_ptr + TEX_SIZE;
             texsam_ptr = texa_ptr + TEX_SIZE;
             mask_ptr = texsam_ptr + SAM_SIZE;
             masksam_ptr = mask_ptr + MASK_SIZE;
# define END_POINTER (masksam_ptr + SAM_SIZE)

             glBindBuffer(GL_ARRAY_BUFFER, gc->pipe[i].array.buffer);
             if ((gc->pipe[i].array.buffer_alloc < (long)END_POINTER) ||
                 (gc->pipe[i].array.buffer_use >= (ARRAY_BUFFER_USE + ARRAY_BUFFER_USE_SHIFT * i)))
               {
                  glBufferData(GL_ARRAY_BUFFER, (long)END_POINTER, NULL, GL_STATIC_DRAW);
                  gc->pipe[i].array.buffer_alloc = (long)END_POINTER;
                  gc->pipe[i].array.buffer_use = 0;
               }
             gc->pipe[i].array.buffer_use++;

             x = glsym_glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
             if (x)
               {
                  if (gc->pipe[i].array.use_vertex)
                    memcpy(x + (unsigned long)vertex_ptr, gc->pipe[i].array.vertex, VERTEX_SIZE);
                  if (gc->pipe[i].array.use_color)
                    memcpy(x + (unsigned long)color_ptr, gc->pipe[i].array.color, COLOR_SIZE);
                  if (gc->pipe[i].array.use_texuv)
                    memcpy(x + (unsigned long)texuv_ptr, gc->pipe[i].array.texuv, TEX_SIZE);
                  if (gc->pipe[i].array.use_texuv2)
                    memcpy(x + (unsigned long)texuv2_ptr, gc->pipe[i].array.texuv2, TEX_SIZE);
                  if (gc->pipe[i].array.use_texuv3)
                    memcpy(x + (unsigned long)texuv3_ptr, gc->pipe[i].array.texuv3, TEX_SIZE);
                  if (gc->pipe[i].array.use_texa)
                    memcpy(x + (unsigned long)texa_ptr, gc->pipe[i].array.texa, TEX_SIZE);
                  if (gc->pipe[i].array.use_texsam)
                    memcpy(x + (unsigned long)texsam_ptr, gc->pipe[i].array.texsam, SAM_SIZE);
                  if (gc->pipe[i].array.use_mask)
                    {
                       memcpy(x + (unsigned long)mask_ptr, gc->pipe[i].array.mask, MASK_SIZE);
                       if (gc->pipe[i].array.use_masksam)
                         memcpy(x + (unsigned long)masksam_ptr, gc->pipe[i].array.masksam, SAM_SIZE);
                    }
                  /*
                  fprintf(stderr, "copy %i bytes [%i/%i slots] [%i + %i + %i + %i + %i + %i + %i] <%i %i %i %i %i %i %i>\n",
                          (int)((unsigned char *)END_POINTER),
                          gc->pipe[i].array.num,
                          gc->pipe[i].array.alloc,
                          VERTEX_SIZE, COLOR_SIZE, TEX_SIZE, TEX_SIZE, TEX_SIZE, TEX_SIZE, TEX_SIZE,
                          gc->pipe[i].array.use_vertex,
                          gc->pipe[i].array.use_color,
                          gc->pipe[i].array.use_texuv,
                          gc->pipe[i].array.use_texuv2,
                          gc->pipe[i].array.use_texuv3,
                          gc->pipe[i].array.use_texa,
                          gc->pipe[i].array.use_texsam,
                          gc->pipe[i].array.use_texm);
 */
                  glsym_glUnmapBuffer(GL_ARRAY_BUFFER);
               }
          }
        else
          {
             vertex_ptr = (unsigned char *)gc->pipe[i].array.vertex;
             color_ptr = (unsigned char *)gc->pipe[i].array.color;
             texuv_ptr = (unsigned char *)gc->pipe[i].array.texuv;
             texuv2_ptr = (unsigned char *)gc->pipe[i].array.texuv2;
             texuv3_ptr = (unsigned char *)gc->pipe[i].array.texuv3;
             texa_ptr = (unsigned char *)gc->pipe[i].array.texa;
             texsam_ptr = (unsigned char *)gc->pipe[i].array.texsam;
             mask_ptr = (unsigned char *)gc->pipe[i].array.mask;
             masksam_ptr = (unsigned char *)gc->pipe[i].array.masksam;
          }

        // use_vertex is always true
        glVertexAttribPointer(SHAD_VERTEX, VERTEX_CNT, GL_SHORT, GL_FALSE, 0, vertex_ptr);

        if (gc->pipe[i].array.use_color)
          {
             glEnableVertexAttribArray(SHAD_COLOR);
             glVertexAttribPointer(SHAD_COLOR, COLOR_CNT, GL_UNSIGNED_BYTE, GL_TRUE, 0, color_ptr);
          }
        else
          glDisableVertexAttribArray(SHAD_COLOR);

        if (gc->pipe[i].array.line)
          {
             if (gc->pipe[i].array.anti_alias)
               {
                  glEnable(GL_BLEND);
                  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                  glHint(GL_LINE_SMOOTH, GL_NICEST);
                  glEnable(GL_LINE_SMOOTH);
               }
             else
               {
                  glDisable(GL_LINE_SMOOTH);
               }

             glDisableVertexAttribArray(SHAD_TEXUV);
             glDisableVertexAttribArray(SHAD_TEXUV2);
             glDisableVertexAttribArray(SHAD_TEXUV3);
             glDisableVertexAttribArray(SHAD_TEXA);
             glDisableVertexAttribArray(SHAD_TEXSAM);

             /* kopi pasta from below */
             if (gc->pipe[i].array.use_mask)
               {
                  glEnableVertexAttribArray(SHAD_MASK);
                  glVertexAttribPointer(SHAD_MASK, MASK_CNT, GL_FLOAT, GL_FALSE, 0, mask_ptr);
                  glActiveTexture(ACTIVE_TEXTURE);
                  glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texm);
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
                  if (shared->info.anisotropic > 0.0)
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, shared->info.anisotropic);
#endif
                  if (gc->pipe[i].shader.mask_smooth)
                    {
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    }
                  else
                    {
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    }
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                  glActiveTexture(GL_TEXTURE0);

                  if (gc->pipe[i].array.use_masksam)
                    {
                       glEnableVertexAttribArray(SHAD_MASKSAM);
                       glVertexAttribPointer(SHAD_MASKSAM, SAM_CNT, GL_FLOAT, GL_FALSE, 0, masksam_ptr);
                    }
                  else glDisableVertexAttribArray(SHAD_MASKSAM);
               }
             else
               {
                  glDisableVertexAttribArray(SHAD_MASK);
                  glDisableVertexAttribArray(SHAD_MASKSAM);
               }

             glDrawArrays(GL_LINES, 0, gc->pipe[i].array.num);
          }
        else
          {
             if (gc->pipe[i].array.use_texuv)
               {
                  glEnableVertexAttribArray(SHAD_TEXUV);
                  glVertexAttribPointer(SHAD_TEXUV, TEX_CNT, GL_FLOAT, GL_FALSE, 0, texuv_ptr);

                  ACTIVE_TEXTURE += 1;
               }
             else
               {
                  glDisableVertexAttribArray(SHAD_TEXUV);
               }

             /* Alpha plane */
             if (gc->pipe[i].array.use_texa)
               {
                  glEnableVertexAttribArray(SHAD_TEXA);
                  glVertexAttribPointer(SHAD_TEXA, TEX_CNT, GL_FLOAT, GL_FALSE, 0, texa_ptr);
                  glActiveTexture(GL_TEXTURE1);
                  glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texa);
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
                  if (shared->info.anisotropic > 0.0)
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, shared->info.anisotropic);
#endif
                  if (gc->pipe[i].shader.smooth)
                    {
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    }
                  else
                    {
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    }
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                  glActiveTexture(GL_TEXTURE0);

                  ACTIVE_TEXTURE += 1;
               }
             else
               {
                  glDisableVertexAttribArray(SHAD_TEXA);
               }

             if (gc->pipe[i].array.use_texsam)
               {
                  glEnableVertexAttribArray(SHAD_TEXSAM);
                  glVertexAttribPointer(SHAD_TEXSAM, SAM_CNT, GL_FLOAT, GL_FALSE, 0, texsam_ptr);
               }
             else
               {
                  glDisableVertexAttribArray(SHAD_TEXSAM);
               }

             if ((gc->pipe[i].array.use_texuv2) && (gc->pipe[i].array.use_texuv3))
               {
                  glEnableVertexAttribArray(SHAD_TEXUV2);
                  glEnableVertexAttribArray(SHAD_TEXUV3);
                  glVertexAttribPointer(SHAD_TEXUV2, TEX_CNT, GL_FLOAT, GL_FALSE, 0, texuv2_ptr);
                  glVertexAttribPointer(SHAD_TEXUV3, TEX_CNT, GL_FLOAT, GL_FALSE, 0, texuv3_ptr);

                  glActiveTexture(GL_TEXTURE1);
                  glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texu);
#ifdef GL_GLES
                  if (gc->pipe[i].shader.cur_texu_dyn)
                    secsym_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texu_dyn);
#endif
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                  glActiveTexture(GL_TEXTURE2);
                  glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texv);
#ifdef GL_GLES
                  if (gc->pipe[i].shader.cur_texv_dyn)
                    secsym_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texv_dyn);
#endif
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                  glActiveTexture(GL_TEXTURE0);

                  ACTIVE_TEXTURE += 2;
               }
             else if (gc->pipe[i].array.use_texuv2)
               {
                  glEnableVertexAttribArray(SHAD_TEXUV2);
                  glVertexAttribPointer(SHAD_TEXUV2, TEX_CNT, GL_FLOAT, GL_FALSE, 0, texuv2_ptr);

                  glActiveTexture(GL_TEXTURE1);
                  glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texu);
#ifdef GL_GLES
                  if (gc->pipe[i].shader.cur_texu_dyn)
                    secsym_glEGLImageTargetTexture2DOES
                          (GL_TEXTURE_2D, gc->pipe[i].shader.cur_texu_dyn);
#endif
                  if (gc->pipe[i].shader.smooth)
                    {
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    }
                  else
                    {
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                    }
                  glActiveTexture(GL_TEXTURE0);

                  glDisableVertexAttribArray(SHAD_TEXUV3);
                  ACTIVE_TEXTURE += 1;
               }
             else
               {
                  glDisableVertexAttribArray(SHAD_TEXUV2);
                  glDisableVertexAttribArray(SHAD_TEXUV3);
               }

             /* Mask surface */
             if (gc->pipe[i].array.use_mask)
               {
                  glEnableVertexAttribArray(SHAD_MASK);
                  glVertexAttribPointer(SHAD_MASK, MASK_CNT, GL_FLOAT, GL_FALSE, 0, mask_ptr);
                  glActiveTexture(ACTIVE_TEXTURE);
                  glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texm);
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
                  if (shared->info.anisotropic > 0.0)
                    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, shared->info.anisotropic);
#endif
                  if (gc->pipe[i].shader.mask_smooth)
                    {
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    }
                  else
                    {
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    }
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                  glActiveTexture(GL_TEXTURE0);

                  if (gc->pipe[i].array.use_masksam)
                    {
                       glEnableVertexAttribArray(SHAD_MASKSAM);
                       glVertexAttribPointer(SHAD_MASKSAM, SAM_CNT, GL_FLOAT, GL_FALSE, 0, masksam_ptr);
                    }
                  else glDisableVertexAttribArray(SHAD_MASKSAM);
                  ACTIVE_TEXTURE += 1;
               }
             else
               {
                  glDisableVertexAttribArray(SHAD_MASK);
                  glDisableVertexAttribArray(SHAD_MASKSAM);
               }

             /* Gfx filters: data */
             if (gc->pipe[i].array.filter_data)
               {
                  for (int k = 0; k < gc->pipe[i].array.filter_data_count; k++)
                    glVertexAttrib2fv(prog->filter->attribute.loc[k], &gc->pipe[i].array.filter_data[k * 2]);
               }

             /* Gfx filters: texture (or data array as texture) */
             if (gc->pipe[i].shader.filter.map_tex)
               {
                  glActiveTexture(ACTIVE_TEXTURE);
                  glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.filter.map_tex);
                  if (gc->pipe[i].shader.filter.map_nearest)
                    {
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                    }
                  else
                    {
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    }
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                  glActiveTexture(GL_TEXTURE0);
                  ACTIVE_TEXTURE += 1;
               }

             if (dbgflushnum == 1)
               {
                  const char *types[] =
                  { "----", "RECT", "IMAG", "FONT", "YUV-", "YUY2", "NV12", "LINE", "PAIR", "EXTR", "MAP-", "FILTER_DISPLACE" };
                  printf("  DRAW#%3i %4i -> %p[%4ix%4i] @ %4ix%4i -{ tex %4i type %s }-\n",
                         i,
                         gc->pipe[i].array.num / 6,
                         gc->pipe[0].shader.surface,
                         gc->pipe[0].shader.surface->w,
                         gc->pipe[0].shader.surface->h,
                         gw, gh,
                         gc->pipe[i].shader.cur_tex,
                         types[gc->pipe[i].region.type]
                         );
               }
             glDrawArrays(GL_TRIANGLES, 0, gc->pipe[i].array.num);
          }
        if (gc->pipe[i].array.im)
          {
             if (!gc->pipe[i].array.im->native.loose)
               {
                  if (gc->pipe[i].array.im->native.func.unbind)
                    gc->pipe[i].array.im->native.func.unbind(gc->pipe[i].array.im);
               }
             gc->pipe[i].array.im = NULL;
          }

        gc->state.current.prog      = gc->pipe[i].shader.prog;
        gc->state.current.cur_tex   = gc->pipe[i].shader.cur_tex;
        gc->state.current.cur_texm  = gc->pipe[i].shader.cur_texm;
        gc->state.current.cur_texa  = gc->pipe[i].shader.cur_texa;
        gc->state.current.cur_texu  = gc->pipe[i].shader.cur_texu;
        gc->state.current.cur_texv  = gc->pipe[i].shader.cur_texv;
        gc->state.current.tex_target = gc->pipe[i].shader.tex_target;
        gc->state.current.render_op = gc->pipe[i].shader.render_op;
//        gc->state.current.cx        = gc->pipe[i].shader.cx;
//        gc->state.current.cy        = gc->pipe[i].shader.cy;
//        gc->state.current.cw        = gc->pipe[i].shader.cw;
//        gc->state.current.ch        = gc->pipe[i].shader.ch;
        gc->state.current.smooth    = gc->pipe[i].shader.smooth;
        gc->state.current.blend     = gc->pipe[i].shader.blend;
        gc->state.current.clip      = gc->pipe[i].shader.clip;
        gc->state.current.anti_alias = gc->pipe[i].array.anti_alias;

        if (gc->pipe[i].shader.filter.map_delete)
          {
             glDeleteTextures(1, &gc->pipe[i].shader.filter.map_tex);
             gc->pipe[i].shader.filter.map_delete = EINA_FALSE;
             gc->pipe[i].shader.filter.map_tex = 0;
          }

        gc->pipe[i].array.line = 0;
        //gc->pipe[i].array.use_vertex = 0;
        gc->pipe[i].array.use_color = 0;
        gc->pipe[i].array.use_texuv = 0;
        gc->pipe[i].array.use_texuv2 = 0;
        gc->pipe[i].array.use_texuv3 = 0;
        gc->pipe[i].array.use_texa = 0;
        gc->pipe[i].array.use_texsam = 0;
        gc->pipe[i].array.use_mask = 0;
        gc->pipe[i].array.use_masksam = 0;
        gc->pipe[i].array.anti_alias = 0;
        gc->pipe[i].array.filter_data_count = 0;

        PIPE_FREE(gc->pipe[i].array.vertex);
        PIPE_FREE(gc->pipe[i].array.color);
        PIPE_FREE(gc->pipe[i].array.texuv);
        PIPE_FREE(gc->pipe[i].array.texuv2);
        PIPE_FREE(gc->pipe[i].array.texuv3);
        PIPE_FREE(gc->pipe[i].array.texa);
        PIPE_FREE(gc->pipe[i].array.texsam);
        PIPE_FREE(gc->pipe[i].array.mask);
        PIPE_FREE(gc->pipe[i].array.masksam);
        FREE(gc->pipe[i].array.filter_data);

        gc->pipe[i].array.num = 0;
        gc->pipe[i].array.alloc = 0;

        if (glsym_glMapBuffer && glsym_glUnmapBuffer)
          {
             glBindBuffer(GL_ARRAY_BUFFER, 0);
          }

        gc->pipe[i].region.x = 0;
        gc->pipe[i].region.y = 0;
        gc->pipe[i].region.w = 0;
        gc->pipe[i].region.h = 0;
        gc->pipe[i].region.type = 0;
     }
   gc->state.top_pipe = 0;
   if (dbgflushnum == 1)
     {
        if (pipe_done > 0) printf("DONE (pipes): %i\n", pipe_done);
     }
   gc->havestuff = EINA_FALSE;
}

EAPI int
evas_gl_common_buffer_dump(Evas_Engine_GL_Context *gc, const char* dname, const char* buf_name, int frame, const char *suffix)
{
   RGBA_Image *im = NULL;
   DATA32 *data1, *data2;
   char fname[100];
   int ok = 0;

   if (suffix)
     snprintf(fname, sizeof(fname), "./%s/win_%s-fc_%03d_%s.png", dname, buf_name, frame, suffix);
   else
     snprintf(fname, sizeof(fname), "./%s/win_%s-fc_%03d.png", dname, buf_name, frame);
   fname[sizeof(fname) - 1] = '\0';

   data1 = (DATA32 *)malloc(gc->w * gc->h * sizeof(DATA32));
   data2 = (DATA32 *)malloc(gc->w * gc->h * sizeof(DATA32));

   if ((!data1) || (!data2)) goto finish;

   glReadPixels(0, 0, gc->w, gc->h, GL_RGBA,
                GL_UNSIGNED_BYTE, (unsigned char*)data1);

   // Flip the Y and change from RGBA TO BGRA
   int i, j;
   for (j = 0; j < gc->h; j++)
      for (i = 0; i < gc->w; i++)
        {
           DATA32 d;
           int idx1 = (j * gc->w) + i;
           int idx2 = ((gc->h - 1) - j) * gc->w + i;

           d = data1[idx1];
           data2[idx2] = ((d & 0x000000ff) << 16) +
              ((d & 0x00ff0000) >> 16)  +
              ((d & 0xff00ff00));
        }

   evas_common_convert_argb_premul(data2, gc->w * gc->h);

   im = (RGBA_Image*) evas_cache_image_data(evas_common_image_cache_get(),
                                            gc->w,
                                            gc->h,
                                            (DATA32 *)data2,
                                            1,
                                            EVAS_COLORSPACE_ARGB8888);
   if (im)
     {
        im->image.data = data2;
        if (im->image.data)
          {
             ok = evas_common_save_image_to_file(im, fname, NULL, 0, 0, NULL);

             if (!ok) ERR("Error Saving file.");
          }

        evas_cache_image_drop(&im->cache_entry);
     }

finish:
   if (data1) free(data1);
   if (data2) free(data2);
   if (im)  evas_cache_image_drop(&im->cache_entry);

   if (ok) return 1;
   else return 0;
}

Eina_Bool
evas_gl_common_module_open(void)
{
   if (_evas_engine_GL_common_log_dom < 0)
     _evas_engine_GL_common_log_dom = eina_log_domain_register
       ("evas-gl_common", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_GL_common_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

void
evas_gl_common_module_close(void)
{
   if (_evas_engine_GL_common_log_dom < 0) return;
   eina_log_domain_unregister(_evas_engine_GL_common_log_dom);
   _evas_engine_GL_common_log_dom = -1;
}
