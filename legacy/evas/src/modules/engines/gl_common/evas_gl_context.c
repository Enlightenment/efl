#include "evas_gl_private.h"

#define GLPIPES 1

static int sym_done = 0;
int _evas_engine_GL_common_log_dom = -1;

typedef void    (*glsym_func_void) ();

void (*glsym_glGenFramebuffers)      (GLsizei a, GLuint *b) = NULL;
void (*glsym_glBindFramebuffer)      (GLenum a, GLuint b) = NULL;
void (*glsym_glFramebufferTexture2D) (GLenum a, GLenum b, GLenum c, GLuint d, GLint e) = NULL;
void (*glsym_glDeleteFramebuffers)   (GLsizei a, const GLuint *b) = NULL;
void (*glsym_glGetProgramBinary)     (GLuint a, GLsizei b, GLsizei *c, GLenum *d, void *e) = NULL;
void (*glsym_glProgramBinary)        (GLuint a, GLenum b, const void *c, GLint d) = NULL;
void (*glsym_glProgramParameteri)    (GLuint a, GLuint b, GLint d) = NULL;

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
// just used for finding symbols :)
typedef void (*_eng_fn) (void);

typedef _eng_fn       (*secsym_func_eng_fn) ();
typedef unsigned int  (*secsym_func_uint) ();
typedef void         *(*secsym_func_void_ptr) ();

static _eng_fn  (*secsym_eglGetProcAddress)          (const char *a) = NULL;

void          *(*secsym_eglCreateImage)               (void *a, void *b, GLenum c, void *d, const int *e) = NULL;
unsigned int   (*secsym_eglDestroyImage)              (void *a, void *b) = NULL;
void           (*secsym_glEGLImageTargetTexture2DOES) (int a, void *b) = NULL;
void          *(*secsym_eglMapImageSEC)               (void *a, void *b) = NULL;
unsigned int   (*secsym_eglUnmapImageSEC)             (void *a, void *b) = NULL;
unsigned int   (*secsym_eglGetImageAttribSEC)         (void *a, void *b, int c, int *d) = NULL;
#endif

static int dbgflushnum = -1;

static void
sym_missing(void)
{
   ERR("GL symbols missing!");
}

static void
gl_symbols(void)
{
   if (sym_done) return;
   sym_done = 1;

#ifdef _EVAS_ENGINE_SDL_H
# define FINDSYM(dst, sym, typ) if (!dst) dst = (typ)SDL_GL_GetProcAddress(sym)
#else
# define FINDSYM(dst, sym, typ) if (!dst) dst = (typ)dlsym(RTLD_DEFAULT, sym)
#endif
#define FALLBAK(dst, typ) if (!dst) dst = (typ)sym_missing;

   FINDSYM(glsym_glGenFramebuffers, "glGenFramebuffers", glsym_func_void);
   FINDSYM(glsym_glGenFramebuffers, "glGenFramebuffersEXT", glsym_func_void);
   FINDSYM(glsym_glGenFramebuffers, "glGenFramebuffersARB", glsym_func_void);
   FALLBAK(glsym_glGenFramebuffers, glsym_func_void);

   FINDSYM(glsym_glBindFramebuffer, "glBindFramebuffer", glsym_func_void);
   FINDSYM(glsym_glBindFramebuffer, "glBindFramebufferEXT", glsym_func_void);
   FINDSYM(glsym_glBindFramebuffer, "glBindFramebufferARB", glsym_func_void);
   FALLBAK(glsym_glBindFramebuffer, glsym_func_void);

   FINDSYM(glsym_glFramebufferTexture2D, "glFramebufferTexture2D", glsym_func_void);
   FINDSYM(glsym_glFramebufferTexture2D, "glFramebufferTexture2DEXT", glsym_func_void);
   FINDSYM(glsym_glFramebufferTexture2D, "glFramebufferTexture2DARB", glsym_func_void);
   FALLBAK(glsym_glFramebufferTexture2D, glsym_func_void);

   FINDSYM(glsym_glDeleteFramebuffers, "glDeleteFramebuffers", glsym_func_void);
   FINDSYM(glsym_glDeleteFramebuffers, "glDeleteFramebuffersEXT", glsym_func_void);
   FINDSYM(glsym_glDeleteFramebuffers, "glDeleteFramebuffersARB", glsym_func_void);
   FALLBAK(glsym_glDeleteFramebuffers, glsym_func_void);

   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinary", glsym_func_void);
   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinaryEXT", glsym_func_void);
   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinaryARB", glsym_func_void);
   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinaryOES", glsym_func_void);

   FINDSYM(glsym_glProgramBinary, "glProgramBinary", glsym_func_void);
   FINDSYM(glsym_glProgramBinary, "glProgramBinaryEXT", glsym_func_void);
   FINDSYM(glsym_glProgramBinary, "glProgramBinaryARB", glsym_func_void);

   FINDSYM(glsym_glProgramParameteri, "glProgramParameteri", glsym_func_void);
   FINDSYM(glsym_glProgramParameteri, "glProgramParameteriEXT", glsym_func_void);
   FINDSYM(glsym_glProgramParameteri, "glProgramParameteriARB", glsym_func_void);

#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
#undef FINDSYM
#define FINDSYM(dst, sym, typ) \
   if ((!dst) && (secsym_eglGetProcAddress)) dst = (typ)secsym_eglGetProcAddress(sym); \
   if (!dst) dst = (typ)dlsym(RTLD_DEFAULT, sym)
// yes - gl core looking for egl stuff. i know it's odd. a reverse-layer thing
// but it will work as the egl/glx layer calls gl core common stuff and thus
// these symbols will work. making the glx/egl + x11 layer do this kind-of is
// wrong as this is not x11 (output) layer specific like the native surface
// stuff. this is generic zero-copy textures for gl

   FINDSYM(secsym_eglGetProcAddress, "eglGetProcAddress", secsym_func_eng_fn);
   FINDSYM(secsym_eglGetProcAddress, "eglGetProcAddressEXT", secsym_func_eng_fn);
   FINDSYM(secsym_eglGetProcAddress, "eglGetProcAddressARB", secsym_func_eng_fn);
   FINDSYM(secsym_eglGetProcAddress, "eglGetProcAddressKHR", secsym_func_eng_fn);

   FINDSYM(secsym_eglCreateImage, "eglCreateImage", secsym_func_void_ptr);
   FINDSYM(secsym_eglCreateImage, "eglCreateImageEXT", secsym_func_void_ptr);
   FINDSYM(secsym_eglCreateImage, "eglCreateImageARB", secsym_func_void_ptr);
   FINDSYM(secsym_eglCreateImage, "eglCreateImageKHR", secsym_func_void_ptr);

   FINDSYM(secsym_eglDestroyImage, "eglDestroyImage", secsym_func_uint);
   FINDSYM(secsym_eglDestroyImage, "eglDestroyImageEXT", secsym_func_uint);
   FINDSYM(secsym_eglDestroyImage, "eglDestroyImageARB", secsym_func_uint);
   FINDSYM(secsym_eglDestroyImage, "eglDestroyImageKHR", secsym_func_uint);

   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinary", glsym_func_void);
   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinaryEXT", glsym_func_void);
   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinaryARB", glsym_func_void);
   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinaryOES", glsym_func_void);
   FINDSYM(glsym_glGetProgramBinary, "glGetProgramBinaryKHR", glsym_func_void);

   FINDSYM(glsym_glProgramBinary, "glProgramBinary", glsym_func_void);
   FINDSYM(glsym_glProgramBinary, "glProgramBinaryEXT", glsym_func_void);
   FINDSYM(glsym_glProgramBinary, "glProgramBinaryARB", glsym_func_void);
   FINDSYM(glsym_glProgramBinary, "glProgramBinaryOES", glsym_func_void);
   FINDSYM(glsym_glProgramBinary, "glProgramBinaryKHR", glsym_func_void);

   FINDSYM(glsym_glProgramParameteri, "glProgramParameteri", glsym_func_void);
   FINDSYM(glsym_glProgramParameteri, "glProgramParameteriEXT", glsym_func_void);
   FINDSYM(glsym_glProgramParameteri, "glProgramParameteriARB", glsym_func_void);
   FINDSYM(glsym_glProgramParameteri, "glProgramParameteriOES", glsym_func_void);
   FINDSYM(glsym_glProgramParameteri, "glProgramParameteriKHR", glsym_func_void);

   FINDSYM(secsym_glEGLImageTargetTexture2DOES, "glEGLImageTargetTexture2DOES", glsym_func_void);

   FINDSYM(secsym_eglMapImageSEC, "eglMapImageSEC", secsym_func_void_ptr);

   FINDSYM(secsym_eglUnmapImageSEC, "eglUnmapImageSEC", secsym_func_uint);

   FINDSYM(secsym_eglGetImageAttribSEC, "eglGetImageAttribSEC", secsym_func_uint);
#endif
}

static void shader_array_flush(Evas_Engine_GL_Context *gc);

static Evas_Engine_GL_Context *_evas_gl_common_context = NULL;
static Evas_GL_Shared *shared = NULL;

void
glerr(int err, const char *file, const char *func, int line, const char *op)
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
             GLfloat near, GLfloat far,
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
   m[10] = -(2.0 / (far - near));
   m[11] = 1.0 / (GLfloat)foc;

   m[12] = (m[0] * tx) + (m[4] * ty) - ((r + l) / (r - l));
   m[13] = (m[1] * tx) + (m[5] * ty) - ((t + b) / (t - b));
   m[14] = (m[2] * tx) + (m[6] * ty) - ((near + far) / (far - near));
   m[15] = (m[3] * tx) + (m[7] * ty) + orth;
}

static int
_evas_gl_common_version_check()
{
   char *version;
   char *tmp;
   char *tmp2;
   int major;
   int minor;

  /*
   * glGetString returns a string describing the current GL connection.
   * GL_VERSION is used to get the version of the connection
   */

   version = (char *)glGetString(GL_VERSION);

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

   if ((tmp = strstr(version, "OpenGL ES-CM ")) || (tmp = strstr(version, "OpenGL ES-CL ")))
     {
        /* Not supported */
        return 0;
     }

   /* OpenGL ES 2.* ? */

   if ((tmp = strstr(version, "OpenGL ES ")))
     {
        /* Supported */
        return 1;
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
   free(version);

   if (((major == 1) && (minor >= 4)) || (major >= 2))
     return 1;

   return 0;
}

static void
_evas_gl_common_viewport_set(Evas_Engine_GL_Context *gc)
{
   GLfloat proj[16];
   unsigned int i;
   int w = 1, h = 1, m = 1, rot = 1, foc = 0;

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
     }

   if ((!gc->change.size) ||
       ((gc->shared->w == w) && (gc->shared->h == h) &&
           (gc->shared->rot == rot) && (gc->shared->foc == gc->foc) &&
           (gc->shared->mflip == m)))
      return;

   gc->shared->w = w;
   gc->shared->h = h;
   gc->shared->rot = rot;
   gc->shared->mflip = m;
   gc->shared->foc = foc;
   gc->shared->z0 = gc->z0;
   gc->shared->px = gc->px;
   gc->shared->py = gc->py;
   gc->change.size = 0;

   if (foc == 0)
     {
        if ((rot == 0) || (rot == 180))
           glViewport(0, 0, w, h);
        else
           glViewport(0, 0, h, w);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        // std matrix
        if (m == 1)
           matrix_ortho(proj,
                        0, w, 0, h,
                        -1000000.0, 1000000.0,
                        rot, w, h,
                        1, 1.0);
        // v flipped matrix for render-to-texture
        else
           matrix_ortho(proj,
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
             if      ((rot == 0  ))                                 ay = 0;
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
           glViewport(-2 * vx, -2 * vy, vw, vh);
        else
           glViewport(-2 * vy, -2 * vx, vh, vw);
        if (m == 1)
           matrix_ortho(proj, 0, vw, 0, vh,
                        -1000000.0, 1000000.0,
                        rot, vw, vh,
                        foc, 0.0);
        else
           matrix_ortho(proj, 0, vw, vh, 0,
                        -1000000.0, 1000000.0,
                        rot, vw, vh,
                        foc, 0.0);
        gc->shared->ax = ax;
        gc->shared->ay = ay;
     }

   for (i = 0; i < SHADER_LAST; ++i)
     {
        glUseProgram(gc->shared->shader[i].prog);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glUniformMatrix4fv(glGetUniformLocation(gc->shared->shader[i].prog, "mvp"), 1, GL_FALSE, proj);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }

   glUseProgram(gc->pipe[0].shader.cur_prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
}

Evas_Engine_GL_Context *
evas_gl_common_context_new(void)
{
   Evas_Engine_GL_Context *gc;
   const char *s;
   int i;

#if 1
   if (_evas_gl_common_context)
     {
        _evas_gl_common_context->references++;
        return _evas_gl_common_context;
     }
#endif
   if (!_evas_gl_common_version_check())
     return NULL;
   gc = calloc(1, sizeof(Evas_Engine_GL_Context));
   if (!gc) return NULL;

   gl_symbols();

   gc->references = 1;

   _evas_gl_common_context = gc;

   for (i = 0; i < MAX_PIPES; i++)
      gc->pipe[i].shader.render_op = EVAS_RENDER_BLEND;

   if (!shared)
     {
        const GLubyte *ext;

        shared = calloc(1, sizeof(Evas_GL_Shared));
        ext = glGetString(GL_EXTENSIONS);
        if (ext)
          {
             if (getenv("EVAS_GL_INFO"))
                fprintf(stderr, "EXT:\n%s\n", ext);
             if ((strstr((char *)ext, "GL_ARB_texture_non_power_of_two")) ||
                 (strstr((char *)ext, "OES_texture_npot")) ||
                 (strstr((char *)ext, "GL_IMG_texture_npot")))
               shared->info.tex_npo2 = 1;
             if ((strstr((char *)ext, "GL_NV_texture_rectangle")) ||
                 (strstr((char *)ext, "GL_EXT_texture_rectangle")) ||
                 (strstr((char *)ext, "GL_ARB_texture_rectangle")))
               shared->info.tex_rect = 1;
             if ((strstr((char *)ext, "GL_ARB_get_program_binary")) ||
                 (strstr((char *)ext, "GL_OES_get_program_binary")))
               shared->info.bin_program = 1;
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
             if ((strstr((char *)ext, "GL_EXT_texture_filter_anisotropic")))
               glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,
                           &(shared->info.anisotropic));
#endif
#ifdef GL_BGRA
             if ((strstr((char *)ext, "GL_EXT_bgra")) ||
                 (strstr((char *)ext, "GL_EXT_texture_format_BGRA8888")))
               shared->info.bgra = 1;
#endif
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
             // FIXME: there should be an extension name/string to check for
             // not just symbols in the lib
             i = 0;
             s = getenv("EVAS_GL_NO_MAP_IMAGE_SEC");
             if (s) i = atoi(s);
             if (!i)
               {
                  // test for all needed symbols - be "conservative" and
                  // need all of it
                  if ((secsym_eglCreateImage) &&
                      (secsym_eglDestroyImage) &&
                      (secsym_glEGLImageTargetTexture2DOES) &&
                      (secsym_eglMapImageSEC) &&
                      (secsym_eglUnmapImageSEC) &&
                      (secsym_eglGetImageAttribSEC))
                     shared->info.sec_image_map = 1;
               }
#endif
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
        shared->info.tune.atlas.slot_size            = DEF_ATLAS_SLOT;

        // per gpu hacks. based on impirical measurement of some known gpu's
        s = (const char *)glGetString(GL_RENDERER);
        if (s)
          {
             if      (strstr(s, "PowerVR SGX 540"))
                shared->info.tune.pipes.max = DEF_PIPES_SGX_540;
             else if (strstr(s, "NVIDIA Tegra"))
                shared->info.tune.pipes.max = DEF_PIPES_TEGRA_2;
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
        GETENVOPT("EVAS_GL_ATLAS_SLOT_SIZE", atlas.slot_size, MIN_ATLAS_SLOT, MAX_ATLAS_SLOT);
        s = (const char *)getenv("EVAS_GL_GET_PROGRAM_BINARY");
        if (s)
          {
             if (atoi(s) == 0) shared->info.bin_program = 0;
          }

        if (getenv("EVAS_GL_INFO"))
           fprintf(stderr,
                   "max tex size %ix%i\n"
                   "max units %i\n"
                   "non-power-2 tex %i\n"
                   "rect tex %i\n"
                   "bgra : %i\n"
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
                   "EVAS_GL_ATLAS_SLOT_SIZE: %i\n"
                   ,
                   (int)shared->info.max_texture_size, (int)shared->info.max_texture_size,
                   (int)shared->info.max_texture_units,
                   (int)shared->info.tex_npo2,
                   (int)shared->info.tex_rect,
                   (int)shared->info.bgra,
                   (double)shared->info.anisotropic,
                   (int)shared->info.sec_image_map,
                   (int)shared->info.max_vertex_elements,

                   (int)shared->info.bin_program,
                   (int)shared->info.tune.cutout.max,
                   (int)shared->info.tune.pipes.max,
                   (int)shared->info.tune.atlas.max_alloc_size,
                   (int)shared->info.tune.atlas.max_alloc_alpha_size,
                   (int)shared->info.tune.atlas.max_w, (int)shared->info.tune.atlas.max_h,
                   (int)shared->info.tune.atlas.slot_size
                  );

        glDisable(GL_DEPTH_TEST);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glEnable(GL_DITHER);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glDisable(GL_BLEND);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        // no dest alpha
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // dest alpha
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // ???
        glDepthMask(GL_FALSE);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
        if (shared->info.anisotropic > 0.0)
          {
             glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
#endif

        glEnableVertexAttribArray(SHAD_VERTEX);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glEnableVertexAttribArray(SHAD_COLOR);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");

	if (!evas_gl_common_shader_program_init(shared)) goto error;

#define SHADER_TEXTURE_ADD(Shared, Shader, Name)                        \
        glUseProgram(Shared->shader[SHADER_##Shader].prog);             \
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");                    \
        glUniform1i(glGetUniformLocation(Shared->shader[SHADER_##Shader].prog, #Name), Shared->shader[SHADER_##Shader].tex_count++); \
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");

        SHADER_TEXTURE_ADD(shared, YUV, tex);
        SHADER_TEXTURE_ADD(shared, YUV, texu);
        SHADER_TEXTURE_ADD(shared, YUV, texv);

        SHADER_TEXTURE_ADD(shared, YUY2, tex);
        SHADER_TEXTURE_ADD(shared, YUY2, texuv);

        SHADER_TEXTURE_ADD(shared, NV12, tex);
        SHADER_TEXTURE_ADD(shared, NV12, texuv);

        SHADER_TEXTURE_ADD(shared, YUV_NOMUL, tex);
        SHADER_TEXTURE_ADD(shared, YUV_NOMUL, texu);
        SHADER_TEXTURE_ADD(shared, YUV_NOMUL, texv);

        SHADER_TEXTURE_ADD(shared, YUY2_NOMUL, tex);
        SHADER_TEXTURE_ADD(shared, YUY2_NOMUL, texuv);

        SHADER_TEXTURE_ADD(shared, NV12_NOMUL, tex);
        SHADER_TEXTURE_ADD(shared, NV12_NOMUL, texuv);

        SHADER_TEXTURE_ADD(shared, IMG_MASK, tex);
        SHADER_TEXTURE_ADD(shared, IMG_MASK, texm);

        glUseProgram(gc->pipe[0].shader.cur_prog);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");

        evas_gl_common_shader_program_init_done();
        // in shader:
        // uniform sampler2D tex[8];
        //
        // in code:
        // GLuint texes[8];
        // GLint loc = glGetUniformLocation(prog, "tex");
        // glUniform1iv(loc, 8, texes);

        shared->native_pm_hash  = eina_hash_int32_new(NULL);
        shared->native_tex_hash = eina_hash_int32_new(NULL);
     }
   gc->shared = shared;
   gc->shared->references++;
   _evas_gl_common_viewport_set(gc);

   gc->def_surface = evas_gl_common_image_surface_new(gc, 1, 1, 1);

   return gc;
   error:
   evas_gl_common_context_free(gc);
   return NULL;
}

void
evas_gl_common_context_free(Evas_Engine_GL_Context *gc)
{
   int i, j;
   Eina_List *l;

   gc->references--;
   if (gc->references > 0) return;
   if (gc->shared) gc->shared->references--;

   if (gc->def_surface) evas_gl_common_image_free(gc->def_surface);

   if (gc->shared)
     {
        for (i = 0; i < gc->shared->info.tune.pipes.max; i++)
          {
             if (gc->pipe[i].array.vertex) free(gc->pipe[i].array.vertex);
             if (gc->pipe[i].array.color) free(gc->pipe[i].array.color);
             if (gc->pipe[i].array.texuv) free(gc->pipe[i].array.texuv);
             if (gc->pipe[i].array.texm) free(gc->pipe[i].array.texm);
             if (gc->pipe[i].array.texuv2) free(gc->pipe[i].array.texuv2);
             if (gc->pipe[i].array.texuv3) free(gc->pipe[i].array.texuv3);
          }
     }

   if ((gc->shared) && (gc->shared->references == 0))
     {
        Evas_GL_Texture_Pool *pt;

        for (i = 0; i < SHADER_LAST; ++i)
          evas_gl_common_shader_program_shutdown(&(gc->shared->shader[i]));

        while (gc->shared->images)
          {
             evas_gl_common_image_free(gc->shared->images->data);
          }

        EINA_LIST_FOREACH(gc->shared->tex.whole, l, pt)
           evas_gl_texture_pool_empty(pt);
        for (i = 0; i < 33; i++)
          {
             for (j = 0; j < 3; j++)
               {
                  EINA_LIST_FOREACH(gc->shared->tex.atlas[i][j], l, pt)
                     evas_gl_texture_pool_empty(pt);
               }
          }
        eina_hash_free(gc->shared->native_pm_hash);
        eina_hash_free(gc->shared->native_tex_hash);
        free(gc->shared);
        shared = NULL;
     }
   if (gc == _evas_gl_common_context) _evas_gl_common_context = NULL;
   free(gc);
}

void
evas_gl_common_context_use(Evas_Engine_GL_Context *gc)
{
   if (_evas_gl_common_context == gc) return;
   _evas_gl_common_context = gc;
   if (gc) _evas_gl_common_viewport_set(gc);
}

void
evas_gl_common_context_newframe(Evas_Engine_GL_Context *gc)
{
   int i;

   if (dbgflushnum < 0)
     {
        dbgflushnum = 0;
        if (getenv("EVAS_GL_DBG")) dbgflushnum = 1;
     }
   if (dbgflushnum) printf("----prev-flushnum: %i -----------------------------------\n", gc->flushnum);

   gc->flushnum = 0;
   gc->state.current.cur_prog = 0;
   gc->state.current.cur_tex = 0;
   gc->state.current.cur_texu = 0;
   gc->state.current.cur_texv = 0;
   gc->state.current.cur_texm = 0;
   gc->state.current.cur_texmu = 0;
   gc->state.current.cur_texmv = 0;
   gc->state.current.render_op = 0;
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
        gc->pipe[i].clip.active = 0;
        gc->pipe[i].clip.x = 0;
        gc->pipe[i].clip.y = 0;
        gc->pipe[i].clip.w = 0;
        gc->pipe[i].clip.h = 0;
        gc->pipe[i].shader.surface = NULL;
        gc->pipe[i].shader.cur_prog = 0;
        gc->pipe[i].shader.cur_tex = 0;
        gc->pipe[i].shader.cur_texu = 0;
        gc->pipe[i].shader.cur_texv = 0;
        gc->pipe[i].shader.cur_texm = 0;
        gc->pipe[i].shader.render_op = EVAS_RENDER_BLEND;
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
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glScissor(0, 0, 0, 0);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glDisable(GL_DEPTH_TEST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glEnable(GL_DITHER);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glDisable(GL_BLEND);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   // no dest alpha
//   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // dest alpha
//   glBlendFunc(GL_SRC_ALPHA, GL_ONE); // ???
   glDepthMask(GL_FALSE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
   if (shared->info.anisotropic > 0.0)
     {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
#endif

   glEnableVertexAttribArray(SHAD_VERTEX);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glEnableVertexAttribArray(SHAD_COLOR);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glUseProgram(gc->pipe[0].shader.cur_prog);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   glActiveTexture(GL_TEXTURE0);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");
   glBindTexture(GL_TEXTURE_2D, gc->pipe[0].shader.cur_tex);
   GLERR(__FUNCTION__, __FILE__, __LINE__, "");

   _evas_gl_common_viewport_set(gc);
}

void
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
evas_gl_common_context_target_surface_set(Evas_Engine_GL_Context *gc,
                                          Evas_GL_Image *surface)
{
   if (surface == gc->pipe[0].shader.surface) return;

   evas_gl_common_context_flush(gc);

   gc->state.current.cur_prog = -1;
   gc->state.current.cur_tex = -1;
   gc->state.current.cur_texu = -1;
   gc->state.current.cur_texv = -1;
   gc->state.current.render_op = -1;
   gc->state.current.smooth = -1;
   gc->state.current.blend = -1;
   gc->state.current.clip = -1;
   gc->state.current.cx = -1;
   gc->state.current.cy = -1;
   gc->state.current.cw = -1;
   gc->state.current.ch = -1;

   gc->pipe[0].shader.surface = surface;
   gc->change.size = 1;
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_OES
# endif
#else
# ifndef GL_FRAMEBUFFER
#  define GL_FRAMEBUFFER GL_FRAMEBUFFER_EXT
# endif
#endif
   if (gc->pipe[0].shader.surface == gc->def_surface)
     {
        glsym_glBindFramebuffer(GL_FRAMEBUFFER, 0);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   else
     {
        glsym_glBindFramebuffer(GL_FRAMEBUFFER, surface->tex->pt->fb);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
     }
   _evas_gl_common_viewport_set(gc);
}

#define PUSH_VERTEX(n, x, y, z) \
   gc->pipe[n].array.vertex[nv++] = x; \
   gc->pipe[n].array.vertex[nv++] = y; \
   gc->pipe[n].array.vertex[nv++] = z
#define PUSH_COLOR(n, r, g, b, a) \
   gc->pipe[n].array.color[nc++] = r; \
   gc->pipe[n].array.color[nc++] = g; \
   gc->pipe[n].array.color[nc++] = b; \
   gc->pipe[n].array.color[nc++] = a
#define PUSH_TEXUV(n, u, v) \
   gc->pipe[n].array.texuv[nu++] = u; \
   gc->pipe[n].array.texuv[nu++] = v
#define PUSH_TEXUV2(n, u, v) \
   gc->pipe[n].array.texuv2[nu2++] = u; \
   gc->pipe[n].array.texuv2[nu2++] = v
#define PUSH_TEXUV3(n, u, v) \
   gc->pipe[n].array.texuv3[nu3++] = u; \
   gc->pipe[n].array.texuv3[nu3++] = v
#define PUSH_TEXM(n, u, v) \
   gc->pipe[n].array.texm[nm++] = u; \
   gc->pipe[n].array.texm[nm++] = v


static inline void
array_alloc(Evas_Engine_GL_Context *gc, int n)
{
   gc->havestuff = EINA_TRUE;
   if (gc->pipe[n].array.num <= gc->pipe[n].array.alloc) return;
   gc->pipe[n].array.alloc += 6 * 1024;
   if (gc->pipe[n].array.use_vertex)
     gc->pipe[n].array.vertex = realloc(gc->pipe[n].array.vertex,
                                gc->pipe[n].array.alloc * sizeof(GLshort) * 3);
   if (gc->pipe[n].array.use_color)
     gc->pipe[n].array.color  = realloc(gc->pipe[n].array.color,
                                gc->pipe[n].array.alloc * sizeof(GLubyte) * 4);
   if (gc->pipe[n].array.use_texuv)
     gc->pipe[n].array.texuv  = realloc(gc->pipe[n].array.texuv,
                                gc->pipe[n].array.alloc * sizeof(GLfloat) * 2);
   if (gc->pipe[n].array.use_texm)
     gc->pipe[n].array.texm  = realloc(gc->pipe[n].array.texm,
                                gc->pipe[n].array.alloc * sizeof(GLfloat) * 2);
   if (gc->pipe[n].array.use_texuv2)
     gc->pipe[n].array.texuv2  = realloc(gc->pipe[n].array.texuv2,
                               gc->pipe[n].array.alloc * sizeof(GLfloat) * 2);
   if (gc->pipe[n].array.use_texuv3)
     gc->pipe[n].array.texuv3  = realloc(gc->pipe[n].array.texuv3,
                                 gc->pipe[n].array.alloc * sizeof(GLfloat) * 2);
}

#ifdef GLPIPES
static int
pipe_region_intersects(Evas_Engine_GL_Context *gc, int n,
                       int x, int y, int w, int h)
{
   int i, rx, ry, rw, rh, ii;

   rx = gc->pipe[n].region.x;
   ry = gc->pipe[n].region.y;
   rw = gc->pipe[n].region.w;
   rh = gc->pipe[n].region.h;
   if (!RECTS_INTERSECT(x, y, w, h, rx, ry, rw, rh))
      return 0;

   // a hack for now. map pipes use their whole bounding box for intersects
   // which at worst case reduces to old pipeline flushes, but cheaper than
   // full quad region or triangle intersects right now
   if (gc->pipe[n].region.type == RTYPE_MAP) return 1;

   for (i = 0,
        ii = 0;

        i < gc->pipe[n].array.num;

        i += (3 * 2),
        ii += (3 * 3 * 2))
     {  // tri 1...
        // 0, 1, 2 < top left
        // 3, 4, 5 < top right
        // 6. 7, 8 < bottom left
        rx = gc->pipe[n].array.vertex[ii + 0];
        ry = gc->pipe[n].array.vertex[ii + 1];
        rw = gc->pipe[n].array.vertex[ii + 3] - rx;
        rh = gc->pipe[n].array.vertex[ii + 7] - ry;
        if (RECTS_INTERSECT(x, y, w, h, rx, ry, rw, rh))
           return 1;
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
vertex_array_size_check(Evas_Engine_GL_Context *gc, int pn, int n)
{
   return 1;
// this fixup breaks for expedite test 32. why?
   if ((gc->pipe[pn].array.num + n) > gc->shared->info.max_vertex_elements)
     {
        shader_array_flush(gc);
        return 0;
     }
   return 1;
}

static inline Evas_GL_Shader
evas_gl_common_shader_choice(int npoints __UNUSED__,
			     RGBA_Map_Point *p,
			     int r, int g, int b, int a,
			     Evas_GL_Shader nomul,
			     Evas_GL_Shader mul)
{
  if ((a == 255) && (r == 255) && (g == 255) && (b == 255))
    {
       if (!p) return nomul;

       if ((p[0].col == 0xffffffff) && (p[1].col == 0xffffffff) &&
           (p[2].col == 0xffffffff) && (p[3].col == 0xffffffff))
         return nomul;
    }
  return mul;
}

static int
_evas_gl_common_context_push(int rtype,
                             Evas_Engine_GL_Context *gc,
                             Evas_GL_Texture *tex,
                             Evas_GL_Texture *texm,
                             GLuint prog,
                             int x, int y, int w, int h,
                             Eina_Bool blend,
                             Eina_Bool smooth,
                             Eina_Bool clip,
                             int cx, int cy, int cw, int ch)
{
   int pn = 0;

#ifdef GLPIPES
 again:
#endif
   vertex_array_size_check(gc, gc->state.top_pipe, 6);
   pn = gc->state.top_pipe;
#ifdef GLPIPES
   if (!((pn == 0) && (gc->pipe[pn].array.num == 0)))
     {
        int found = 0;
        int i;

        for (i = pn; i >= 0; i--)
          {
             if ((gc->pipe[i].region.type == rtype)
                 && (!tex || gc->pipe[i].shader.cur_tex == tex->pt->texture)
                 && (!texm || gc->pipe[i].shader.cur_texm == texm->pt->texture)
                 && (gc->pipe[i].shader.cur_prog == prog)
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
             pn = gc->state.top_pipe;
             gc->pipe[pn].array.im = tex->im;
             goto again;
          }
     }
#else
   if (!((gc->pipe[pn].region.type == rtype)
         && (!tex || gc->pipe[pn].shader.cur_tex == tex->pt->texture)
         && (!texm || gc->pipe[pn].shader.cur_texm == texm->pt->texture)
         && (gc->pipe[pn].shader.cur_prog == prog)
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
                                 int r, int g, int b, int a)
{
   int pnum, nv, nc, nu, nt, i;
   Eina_Bool blend = 0;
   GLuint prog = gc->shared->shader[SHADER_RECT].prog;
   int pn = 0;

   if (a < 255) blend = 1;
   if (gc->dc->render_op == EVAS_RENDER_COPY) blend = 0;

   shader_array_flush(gc);
   vertex_array_size_check(gc, gc->state.top_pipe, 2);
   pn = gc->state.top_pipe;
   gc->pipe[pn].shader.cur_tex = 0;
   gc->pipe[pn].shader.cur_prog = prog;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.clip = clip;
   gc->pipe[pn].shader.cx = cx;
   gc->pipe[pn].shader.cy = cy;
   gc->pipe[pn].shader.cw = cw;
   gc->pipe[pn].shader.ch = ch;

   gc->pipe[pn].array.line = 1;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = 1;
   gc->pipe[pn].array.use_texuv = 0;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;

   pnum = gc->pipe[pn].array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nt = pnum * 4;
   gc->pipe[pn].array.num += 2;
   array_alloc(gc, pn);

   PUSH_VERTEX(pn, x1, y1, 0);
   PUSH_VERTEX(pn, x2, y2, 0);

   for (i = 0; i < 2; i++)
     {
        PUSH_COLOR(pn, r, g, b, a);
     }

   shader_array_flush(gc);
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 0;
   gc->pipe[pn].array.use_color = 0;
   gc->pipe[pn].array.use_texuv = 0;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;
}

void
evas_gl_common_context_rectangle_push(Evas_Engine_GL_Context *gc,
                                      int x, int y, int w, int h,
                                      int r, int g, int b, int a)
{
   int pnum, nv, nc, nu, nt, i;
   Eina_Bool blend = 0;
   GLuint prog = gc->shared->shader[SHADER_RECT].prog;
   int pn = 0;

   if (gc->dc->mask.mask)
     {
	RGBA_Draw_Context *dc;
	dc = gc->dc;
	Evas_GL_Image *im;
	im = (void *)dc->mask.mask;
	evas_gl_common_context_font_push(gc, im->tex,
				x - dc->mask.x,y - dc->mask.y,
				dc->mask.w,dc->mask.h,
				x,y,w,h,r,g,b,a);
	return;
     }

   if (a < 255) blend = 1;
   if (gc->dc->render_op == EVAS_RENDER_COPY) blend = 0;

again:
   vertex_array_size_check(gc, gc->state.top_pipe, 6);
   pn = gc->state.top_pipe;
#ifdef GLPIPES
   if ((pn == 0) && (gc->pipe[pn].array.num == 0))
     {
        gc->pipe[pn].region.type = RTYPE_RECT;
        gc->pipe[pn].shader.cur_tex = 0;
        gc->pipe[pn].shader.cur_prog = prog;
        gc->pipe[pn].shader.blend = blend;
        gc->pipe[pn].shader.render_op = gc->dc->render_op;
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
     }
   else
     {
        int found = 0;

        for (i = pn; i >= 0; i--)
          {
             if ((gc->pipe[i].region.type == RTYPE_RECT)
                 && (gc->pipe[i].shader.cur_tex == 0)
                 && (gc->pipe[i].shader.cur_prog == prog)
                 && (gc->pipe[i].shader.blend == blend)
                 && (gc->pipe[i].shader.render_op == gc->dc->render_op)
                 && (gc->pipe[i].shader.clip == 0)
                )
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
             gc->pipe[pn].region.type = RTYPE_RECT;
             gc->pipe[pn].shader.cur_tex = 0;
             gc->pipe[pn].shader.cur_prog = prog;
             gc->pipe[pn].shader.blend = blend;
             gc->pipe[pn].shader.render_op = gc->dc->render_op;
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
         }
     }
#else
   if ((gc->pipe[pn].shader.cur_tex != 0)
       || (gc->pipe[pn].shader.cur_prog != prog)
       || (gc->pipe[pn].shader.blend != blend)
       || (gc->pipe[pn].shader.render_op != gc->dc->render_op)
       || (gc->pipe[pn].shader.clip != 0)
       )
     {
        shader_array_flush(gc);
        pn = gc->state.top_pipe;
        gc->pipe[pn].shader.cur_tex = 0;
        gc->pipe[pn].shader.cur_prog = prog;
        gc->pipe[pn].shader.blend = blend;
        gc->pipe[pn].shader.render_op = gc->dc->render_op;
        gc->pipe[pn].shader.clip = 0;
        gc->pipe[pn].shader.cx = 0;
        gc->pipe[pn].shader.cy = 0;
        gc->pipe[pn].shader.cw = 0;
        gc->pipe[pn].shader.ch = 0;
     }

   gc->pipe[pn].region.type = RTYPE_RECT;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = 1;
   gc->pipe[pn].array.use_texuv = 0;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;
#endif

   pipe_region_expand(gc, pn, x, y, w, h);

   pnum = gc->pipe[pn].array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nt = pnum * 4;
   gc->pipe[pn].array.num += 6;
   array_alloc(gc, pn);

   PUSH_VERTEX(pn, x    , y    , 0);
   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x + w, y + h, 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(pn, r, g, b, a);
     }
}

void
evas_gl_common_context_image_push(Evas_Engine_GL_Context *gc,
                                  Evas_GL_Texture *tex,
                                  double sx, double sy, double sw, double sh,
                                  int x, int y, int w, int h,
                                  int r, int g, int b, int a,
                                  Eina_Bool smooth, Eina_Bool tex_only)
{
   int pnum, nv, nc, nu, nu2, nt, i;
   GLfloat tx1, tx2, ty1, ty2;
   Eina_Bool blend = 1;
   GLuint prog = gc->shared->shader[SHADER_IMG].prog;
   int pn = 0;

   if (!tex->alpha) blend = 0;
   if (a < 255) blend = 1;

   if (gc->filter_prog)
     {
        prog = gc->filter_prog;
     }
   else if (tex_only)
     {
        if (tex->pt->dyn.img)
          {
             prog = gc->shared->shader[evas_gl_common_shader_choice(0, NULL, r, g, b, a,
                                                                    SHADER_IMG_NOMUL, SHADER_IMG)].prog;
          }
        else
          {
             prog = gc->shared->shader[evas_gl_common_shader_choice(0, NULL, r, g, b, a,
                                                                    SHADER_TEX_NOMUL, SHADER_TEX)].prog;
          }
     }
   else
     {
        if (tex->gc->shared->info.bgra)
          {
             prog = gc->shared->shader[evas_gl_common_shader_choice(0, NULL, r, g, b, a,
                                                                    SHADER_IMG_BGRA_NOMUL, SHADER_IMG_BGRA)].prog;
          }
        else
          {
             prog = gc->shared->shader[evas_gl_common_shader_choice(0, NULL, r, g, b, a,
                                                                    SHADER_IMG_NOMUL, SHADER_IMG)].prog;
          }
     }

   pn = _evas_gl_common_context_push(RTYPE_IMAGE,
                                     gc, tex, NULL,
                                     prog,
                                     x, y, w, h,
                                     blend,
                                     smooth,
                                     0, 0, 0, 0, 0);

   gc->pipe[pn].region.type = RTYPE_IMAGE;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_prog = prog;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   // if nomul... dont need this
   gc->pipe[pn].array.use_color = 1;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;

   pipe_region_expand(gc, pn, x, y, w, h);

   pnum = gc->pipe[pn].array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nu2 = pnum * 2;
   nt = pnum * 4;
   gc->pipe[pn].array.num += 6;
   array_alloc(gc, pn);

   if ((tex->im) && (tex->im->native.data) && (!tex->im->native.yinvert))
     {
        tx1 = ((double)(tex->x) + sx) / (double)tex->pt->w;
        ty1 = 1.0 - ((double)(tex->y) + sy) / (double)tex->pt->h;
        tx2 = ((double)(tex->x) + sx + sw) / (double)tex->pt->w;
        ty2 = 1.0 - ((double)(tex->y) + sy + sh) / (double)tex->pt->h;
     }
   else
     {
        tx1 = ((double)(tex->x) + sx) / (double)tex->pt->w;
        ty1 = ((double)(tex->y) + sy) / (double)tex->pt->h;
        tx2 = ((double)(tex->x) + sx + sw) / (double)tex->pt->w;
        ty2 = ((double)(tex->y) + sy + sh) / (double)tex->pt->h;
     }

   PUSH_VERTEX(pn, x    , y    , 0);
   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx1, ty1);
   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx1, ty2);

   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x + w, y + h, 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx2, ty2);
   PUSH_TEXUV(pn, tx1, ty2);

   // if nomul... dont need this
   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(pn, r, g, b, a);
     }
}

void
evas_gl_common_context_image_mask_push(Evas_Engine_GL_Context *gc,
                                  Evas_GL_Texture *tex,
                                  Evas_GL_Texture *texm,
                                  double sx, double sy, double sw, double sh,
                                  double sxm, double sym, double swm,double shm,
                                  int x, int y, int w, int h,
                                  int r, int g, int b, int a,
                                  Eina_Bool smooth)
{
   int pnum, nv, nc, nu, nm, nt, i;
   GLfloat tx1, tx2, ty1, ty2;
   GLfloat txm1, txm2, tym1, tym2;
   Eina_Bool blend = 1;
   GLuint prog = gc->shared->shader[SHADER_IMG_MASK].prog;
   int pn = 0;

#if 0
   if (tex->gc->shared->info.bgra)
   {
      prog = gc->shared->shader[SHADER_IMG_MASK].prog;
   }
   else
   {
#warning Nash: FIXME: Need two shaders?
	   printf("Not good: Need other texture\n");
	   prog = gc->shared->shader[SHADER_IMG].prog;
   }
#endif

   pn = _evas_gl_common_context_push(RTYPE_IMASK,
                                     gc, tex, texm,
                                     prog,
                                     x, y, w, h,
                                     blend,
                                     smooth,
                                     0, 0, 0, 0, 0);

   gc->pipe[pn].region.type = RTYPE_IMASK;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_texm = texm->pt->texture;
   gc->pipe[pn].shader.cur_prog = prog;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   // if nomul... dont need this
   gc->pipe[pn].array.use_color = 1;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 0;
   gc->pipe[pn].array.use_texuv3 = 0;
   gc->pipe[pn].array.use_texm = 1;

   pipe_region_expand(gc, pn, x, y, w, h);

   pnum = gc->pipe[pn].array.num;
   nv = pnum * 3; nc = pnum * 4; nm = pnum * 2; nu = pnum * 2;
   nt = pnum * 4;
   gc->pipe[pn].array.num += 6;
   array_alloc(gc, pn);

   if ((tex->im) && (tex->im->native.data) && (!tex->im->native.yinvert))
     {
        tx1 = ((double)(tex->x) + sx) / (double)tex->pt->w;
        ty1 = ((double)(tex->y) + sy + sh) / (double)tex->pt->h;
        tx2 = ((double)(tex->x) + sx + sw) / (double)tex->pt->w;
        ty2 = ((double)(tex->y) + sy) / (double)tex->pt->h;

	txm1 = ((double)(texm->x) + sxm) / (double)texm->pt->w;
        tym1 = ((double)(texm->y) + sym + shm) / (double)texm->pt->h;
        txm2 = ((double)(texm->x) + sxm + swm) / (double)texm->pt->w;
        tym2 = ((double)(texm->y) + sym) / (double)texm->pt->h;
     }
   else
     {
        tx1 = ((double)(tex->x) + sx) / (double)tex->pt->w;
        ty1 = ((double)(tex->y) + sy) / (double)tex->pt->h;
        tx2 = ((double)(tex->x) + sx + sw) / (double)tex->pt->w;
        ty2 = ((double)(tex->y) + sy + sh) / (double)tex->pt->h;

        txm1 = (texm->x + sxm) / (double)texm->pt->w;
        tym1 = (texm->y + sym) / (double)texm->pt->h;
        txm2 = (texm->x + sxm + swm) / (double)texm->pt->w;
        tym2 = (texm->y + sym + shm) / (double)texm->pt->h;
     }
 // printf(" %3.6lf %3.6lf %3.6lf %3.6lf\n",sx,sy,sw,sh);
 //  printf("m%3.6lf %3.6lf %3.6lf %3.6lf\n",sxm,sym,swm,shm);
 // printf(" %3f %3f %3f %3f\n",tx1,ty1,tx2,ty2);
 // printf("m%3f %3f %3f %3f\n",txm1,tym1,txm2,tym2);

   PUSH_VERTEX(pn, x    , y    , 0);
   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx1, ty1);
   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx1, ty2);

   PUSH_TEXM(pn, txm1, tym1);
   PUSH_TEXM(pn, txm2, tym1);
   PUSH_TEXM(pn, txm1, tym2);

   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x + w, y + h, 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx2, ty2);
   PUSH_TEXUV(pn, tx1, ty2);

   PUSH_TEXM(pn, txm2, tym1);
   PUSH_TEXM(pn, txm2, tym2);
   PUSH_TEXM(pn, txm1, tym2);

   // if nomul... dont need this
   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(pn, r, g, b, a);
     }
}


void
evas_gl_common_context_font_push(Evas_Engine_GL_Context *gc,
                                 Evas_GL_Texture *tex,
                                 double sx, double sy, double sw, double sh,
                                 int x, int y, int w, int h,
                                 int r, int g, int b, int a)
{
   int pnum, nv, nc, nu, nt, i;
   GLfloat tx1, tx2, ty1, ty2;
   GLuint prog = gc->shared->shader[SHADER_FONT].prog;
   int pn = 0;

   pn = _evas_gl_common_context_push(RTYPE_FONT,
				     gc, tex, NULL,
				     prog,
				     x, y, w, h,
				     1,
				     0,
				     0, 0, 0, 0, 0);

   gc->pipe[pn].region.type = RTYPE_FONT;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_prog = prog;
   gc->pipe[pn].shader.smooth = 0;
   gc->pipe[pn].shader.blend = 1;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
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

   pipe_region_expand(gc, pn, x, y, w, h);

   pnum = gc->pipe[pn].array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nt = pnum * 4;
   gc->pipe[pn].array.num += 6;
   array_alloc(gc, pn);

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

   PUSH_VERTEX(pn, x    , y    , 0);
   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx1, ty1);
   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx1, ty2);

   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x + w, y + h, 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx2, ty2);
   PUSH_TEXUV(pn, tx1, ty2);

   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(pn, r, g, b, a);
     }
}

void
evas_gl_common_context_yuv_push(Evas_Engine_GL_Context *gc,
                                Evas_GL_Texture *tex,
                                double sx, double sy, double sw, double sh,
                                int x, int y, int w, int h,
                                int r, int g, int b, int a,
                                Eina_Bool smooth)
{
   int pnum, nv, nc, nu, nu2, nu3, nt, i;
   GLfloat tx1, tx2, ty1, ty2, t2x1, t2x2, t2y1, t2y2;
   Eina_Bool blend = 0;
   GLuint prog;
   int pn = 0;

   if (a < 255) blend = 1;

   prog = gc->shared->shader[evas_gl_common_shader_choice(0, NULL, r, g, b, a,
                                                          SHADER_YUV_NOMUL, SHADER_YUV)].prog;

   pn = _evas_gl_common_context_push(RTYPE_YUV,
				     gc, tex, NULL,
				     prog,
				     x, y, w, h,
				     blend,
				     smooth,
				     0, 0, 0, 0, 0);

   gc->pipe[pn].region.type = RTYPE_YUV;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_texu = tex->ptu->texture;
   gc->pipe[pn].shader.cur_texv = tex->ptv->texture;
   gc->pipe[pn].shader.cur_prog = prog;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = 1;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 1;
   gc->pipe[pn].array.use_texuv3 = 1;

   pipe_region_expand(gc, pn, x, y, w, h);

   pnum = gc->pipe[pn].array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2;
   nu2 = pnum * 2; nu3 = pnum * 2; nt = pnum * 4;
   gc->pipe[pn].array.num += 6;
   array_alloc(gc, pn);

   tx1 = (sx) / (double)tex->pt->w;
   ty1 = (sy) / (double)tex->pt->h;
   tx2 = (sx + sw) / (double)tex->pt->w;
   ty2 = (sy + sh) / (double)tex->pt->h;

   t2x1 = ((sx) / 2) / (double)tex->ptu->w;
   t2y1 = ((sy) / 2) / (double)tex->ptu->h;
   t2x2 = ((sx + sw) / 2) / (double)tex->ptu->w;
   t2y2 = ((sy + sh) / 2) / (double)tex->ptu->h;

   PUSH_VERTEX(pn, x    , y    , 0);
   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx1, ty1);
   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx1, ty2);

   PUSH_TEXUV2(pn, t2x1, t2y1);
   PUSH_TEXUV2(pn, t2x2, t2y1);
   PUSH_TEXUV2(pn, t2x1, t2y2);

   PUSH_TEXUV3(pn, t2x1, t2y1);
   PUSH_TEXUV3(pn, t2x2, t2y1);
   PUSH_TEXUV3(pn, t2x1, t2y2);

   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x + w, y + h, 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx2, ty2);
   PUSH_TEXUV(pn, tx1, ty2);

   PUSH_TEXUV2(pn, t2x2, t2y1);
   PUSH_TEXUV2(pn, t2x2, t2y2);
   PUSH_TEXUV2(pn, t2x1, t2y2);

   PUSH_TEXUV3(pn, t2x2, t2y1);
   PUSH_TEXUV3(pn, t2x2, t2y2);
   PUSH_TEXUV3(pn, t2x1, t2y2);

   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(pn, r, g, b, a);
     }
}

void
evas_gl_common_context_yuy2_push(Evas_Engine_GL_Context *gc,
				 Evas_GL_Texture *tex,
				 double sx, double sy, double sw, double sh,
				 int x, int y, int w, int h,
				 int r, int g, int b, int a,
				 Eina_Bool smooth)
{
   int pnum, nv, nc, nu, nu2, nu3, nt, i;
   GLfloat tx1, tx2, ty1, ty2, t2x1, t2x2, t2y1, t2y2;
   Eina_Bool blend = 0;
   GLuint prog;
   int pn = 0;

   if (a < 255) blend = 1;

   prog = gc->shared->shader[evas_gl_common_shader_choice(0, NULL, r, g, b, a,
                                                          SHADER_YUY2_NOMUL, SHADER_YUY2)].prog;

   pn = _evas_gl_common_context_push(RTYPE_YUY2,
				     gc, tex, NULL,
				     prog,
				     x, y, w, h,
				     blend,
				     smooth,
				     0, 0, 0, 0, 0);

   gc->pipe[pn].region.type = RTYPE_YUY2;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_texu = tex->ptuv->texture;
   gc->pipe[pn].shader.cur_prog = prog;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = 1;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 1;
   gc->pipe[pn].array.use_texuv3 = 0;

   pipe_region_expand(gc, pn, x, y, w, h);

   pnum = gc->pipe[pn].array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2;
   nu2 = pnum * 2; nu3 = pnum * 2; nt = pnum * 4;
   gc->pipe[pn].array.num += 6;
   array_alloc(gc, pn);

   tx1 = (sx) / (double)tex->pt->w;
   ty1 = (sy) / (double)tex->pt->h;
   tx2 = (sx + sw) / (double)tex->pt->w;
   ty2 = (sy + sh) / (double)tex->pt->h;

   t2x1 = sx / (double)tex->ptuv->w;
   t2y1 = sy / (double)tex->ptuv->h;
   t2x2 = (sx + sw) / (double)tex->ptuv->w;
   t2y2 = (sy + sh) / (double)tex->ptuv->h;

   PUSH_VERTEX(pn, x    , y    , 0);
   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx1, ty1);
   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx1, ty2);

   PUSH_TEXUV2(pn, t2x1, t2y1);
   PUSH_TEXUV2(pn, t2x2, t2y1);
   PUSH_TEXUV2(pn, t2x1, t2y2);

   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x + w, y + h, 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx2, ty2);
   PUSH_TEXUV(pn, tx1, ty2);

   PUSH_TEXUV2(pn, t2x2, t2y1);
   PUSH_TEXUV2(pn, t2x2, t2y2);
   PUSH_TEXUV2(pn, t2x1, t2y2);

   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(pn, r, g, b, a);
     }
}

void
evas_gl_common_context_nv12_push(Evas_Engine_GL_Context *gc,
				 Evas_GL_Texture *tex,
				 double sx, double sy, double sw, double sh,
				 int x, int y, int w, int h,
				 int r, int g, int b, int a,
				 Eina_Bool smooth)
{
   int pnum, nv, nc, nu, nu2, nu3, nt, i;
   GLfloat tx1, tx2, ty1, ty2, t2x1, t2x2, t2y1, t2y2;
   Eina_Bool blend = 0;
   GLuint prog;
   int pn = 0;

   if (a < 255) blend = 1;

   prog = gc->shared->shader[evas_gl_common_shader_choice(0, NULL, r, g, b, a,
                                                          SHADER_NV12_NOMUL, SHADER_NV12)].prog;

   pn = _evas_gl_common_context_push(RTYPE_NV12,
				     gc, tex, NULL,
				     prog,
				     x, y, w, h,
				     blend,
				     smooth,
				     0, 0, 0, 0, 0);

   gc->pipe[pn].region.type = RTYPE_NV12;
   gc->pipe[pn].shader.cur_tex = tex->pt->texture;
   gc->pipe[pn].shader.cur_tex_dyn = tex->pt->dyn.img;
   gc->pipe[pn].shader.cur_texu = tex->ptuv->texture;
   gc->pipe[pn].shader.cur_texu_dyn = tex->ptuv->dyn.img;
   gc->pipe[pn].shader.cur_prog = prog;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.clip = 0;
   gc->pipe[pn].shader.cx = 0;
   gc->pipe[pn].shader.cy = 0;
   gc->pipe[pn].shader.cw = 0;
   gc->pipe[pn].shader.ch = 0;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = 1;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = 1;
   gc->pipe[pn].array.use_texuv3 = 0;

   pipe_region_expand(gc, pn, x, y, w, h);

   pnum = gc->pipe[pn].array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2;
   nu2 = pnum * 2; nu3 = pnum * 2; nt = pnum * 4;
   gc->pipe[pn].array.num += 6;
   array_alloc(gc, pn);

   tx1 = (sx) / (double)tex->pt->w;
   ty1 = (sy) / (double)tex->pt->h;
   tx2 = (sx + sw) / (double)tex->pt->w;
   ty2 = (sy + sh) / (double)tex->pt->h;

   t2x1 = sx / (double)tex->ptuv->w;
   t2y1 = sy / (double)tex->ptuv->h;
   t2x2 = (sx + sw) / (double)tex->ptuv->w;
   t2y2 = (sy + sh) / (double)tex->ptuv->h;

   PUSH_VERTEX(pn, x    , y    , 0);
   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx1, ty1);
   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx1, ty2);

   PUSH_TEXUV2(pn, t2x1, t2y1);
   PUSH_TEXUV2(pn, t2x2, t2y1);
   PUSH_TEXUV2(pn, t2x1, t2y2);

   PUSH_VERTEX(pn, x + w, y    , 0);
   PUSH_VERTEX(pn, x + w, y + h, 0);
   PUSH_VERTEX(pn, x    , y + h, 0);

   PUSH_TEXUV(pn, tx2, ty1);
   PUSH_TEXUV(pn, tx2, ty2);
   PUSH_TEXUV(pn, tx1, ty2);

   PUSH_TEXUV2(pn, t2x2, t2y1);
   PUSH_TEXUV2(pn, t2x2, t2y2);
   PUSH_TEXUV2(pn, t2x1, t2y2);

   for (i = 0; i < 6; i++)
     {
        PUSH_COLOR(pn, r, g, b, a);
     }
}

void
evas_gl_common_context_image_map_push(Evas_Engine_GL_Context *gc,
                                      Evas_GL_Texture *tex,
                                      int npoints,
                                      RGBA_Map_Point *p,
                                      int clip, int cx, int cy, int cw, int ch,
                                      int r, int g, int b, int a,
				      Eina_Bool smooth, Eina_Bool tex_only,
				      Evas_Colorspace cspace)
{
   int pnum, nv, nc, nu, nu2, nu3, nt, i;
   const int points[6] = { 0, 1, 2, 0, 2, 3 };
   int x = 0, y = 0, w = 0, h = 0, px = 0, py = 0;
   GLfloat tx[4], ty[4], t2x[4], t2y[4];
   Eina_Bool blend = 1;
   DATA32 cmul;
   GLuint prog = gc->shared->shader[SHADER_IMG].prog;
   Eina_Bool utexture = EINA_FALSE;
   Eina_Bool uvtexture = EINA_FALSE;
   int pn = 0;
   int flat = 0;

   if (!tex->alpha) blend = 0;
   if (a < 255) blend = 1;
   if (npoints != 4)
     {
        // FIXME: nash - you didn't fix this for n points. its still all
        // 4 point stuff!!! grrrr.
        abort();
     }
   if ((A_VAL(&(p[0].col)) < 0xff) || (A_VAL(&(p[1].col)) < 0xff) ||
       (A_VAL(&(p[2].col)) < 0xff) || (A_VAL(&(p[3].col)) < 0xff))
     blend = 1;

   if ((p[0].z == p[1].z) && (p[1].z == p[2].z) && (p[2].z == p[3].z))
      flat = 1;

   if (!clip) cx = cy = cw = ch = 0;

   if (!flat)
     {
        if (p[0].foc <= 0) flat = 1;
     }

   switch (cspace)
     {
      case EVAS_COLORSPACE_YCBCR422P601_PL:
      case EVAS_COLORSPACE_YCBCR422P709_PL:
         prog = gc->shared->shader[evas_gl_common_shader_choice(npoints, p, r, g, b, a,
                                                                SHADER_YUV_NOMUL, SHADER_YUV)].prog;
         utexture = EINA_TRUE;
         break;
      case EVAS_COLORSPACE_YCBCR422601_PL:
         prog = gc->shared->shader[evas_gl_common_shader_choice(npoints, p, r, g, b, a,
                                                                SHADER_YUY2_NOMUL, SHADER_YUY2)].prog;
         uvtexture = EINA_TRUE;
         break;
      case EVAS_COLORSPACE_YCBCR420NV12601_PL:
      case EVAS_COLORSPACE_YCBCR420TM12601_PL:
         prog = gc->shared->shader[evas_gl_common_shader_choice(npoints, p, r, g, b, a,
                                                                SHADER_NV12_NOMUL, SHADER_NV12)].prog;
         uvtexture = EINA_TRUE;
         break;

      default:
         if (tex_only)
           {
              if (tex->pt->dyn.img)
                {
                   prog = gc->shared->shader[evas_gl_common_shader_choice(npoints, p, r, g, b, a,
                                                                          SHADER_IMG_NOMUL, SHADER_IMG)].prog;
                }
              else
                {
                   prog = gc->shared->shader[evas_gl_common_shader_choice(npoints, p, r, g, b, a,
                                                                          SHADER_TEX_NOMUL, SHADER_TEX)].prog;
                }
           }
         else
           {
              if (tex->gc->shared->info.bgra)
                {
                   prog = gc->shared->shader[evas_gl_common_shader_choice(npoints, p, r, g, b, a,
                                                                          SHADER_IMG_BGRA_NOMUL,
                                                                          SHADER_IMG_BGRA)].prog;
                }
             else
               {
                  prog = gc->shared->shader[evas_gl_common_shader_choice(npoints, p, r, g, b, a,
                                                                         SHADER_IMG_NOMUL,
                                                                         SHADER_IMG)].prog;
               }
           }
     }

   x = w = (p[points[0]].x >> FP);
   y = h = (p[points[0]].y >> FP);
   for (i = 0; i < 4; i++)
     {
        tx[i] = ((double)(tex->x) + (((double)p[i].u) / FP1)) /
          (double)tex->pt->w;
        ty[i] = ((double)(tex->y) + (((double)p[i].v) / FP1)) /
          (double)tex->pt->h;
        px = (p[points[i]].x >> FP);
        if      (px < x) x = px;
        else if (px > w) w = px;
        py = (p[points[i]].y >> FP);
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

   if (clip)
     {
        int nx = x, ny = y, nw = w, nh = h;

        RECTS_CLIP_TO_RECT(nx, ny, nw, nh, cx, cy, cw, ch);
        if ((nx == x) && (ny == y) && (nw == w) && (nh == h))
          {
             clip = 0; cx = 0; cy = 0; cw = 0; ch = 0;
          }
        x = nx; y = nw; w = nw; h = nh;
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

   pn = _evas_gl_common_context_push(RTYPE_MAP,
				     gc, tex, NULL,
				     prog,
				     x, y, w, h,
				     blend,
				     smooth,
				     clip, cx, cy, cw, ch);

   gc->pipe[pn].region.type = RTYPE_MAP;
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
   gc->pipe[pn].shader.cur_prog = prog;
   gc->pipe[pn].shader.smooth = smooth;
   gc->pipe[pn].shader.blend = blend;
   gc->pipe[pn].shader.render_op = gc->dc->render_op;
   gc->pipe[pn].shader.clip = clip;
   gc->pipe[pn].shader.cx = cx;
   gc->pipe[pn].shader.cy = cy;
   gc->pipe[pn].shader.cw = cw;
   gc->pipe[pn].shader.ch = ch;
   gc->pipe[pn].array.line = 0;
   gc->pipe[pn].array.use_vertex = 1;
   gc->pipe[pn].array.use_color = 1;
   gc->pipe[pn].array.use_texuv = 1;
   gc->pipe[pn].array.use_texuv2 = (utexture || uvtexture) ? 1 : 0;
   gc->pipe[pn].array.use_texuv3 = (utexture) ? 1 : 0;

   pipe_region_expand(gc, pn, x, y, w, h);

   pnum = gc->pipe[pn].array.num;
   nv = pnum * 3; nc = pnum * 4; nu = pnum * 2; nu2 = pnum * 2;
   nu2 = pnum * 2; nu3 = pnum * 2; nt = pnum * 4;
   gc->pipe[pn].array.num += 6;
   array_alloc(gc, pn);

   if ((tex->im) && (tex->im->native.data) && (!tex->im->native.yinvert))
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

        PUSH_COLOR(pn,
                   R_VAL(&cl),
                   G_VAL(&cl),
                   B_VAL(&cl),
                   A_VAL(&cl));
     }
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

void
evas_gl_common_context_flush(Evas_Engine_GL_Context *gc)
{
   shader_array_flush(gc);
}

static void
scissor_rot(Evas_Engine_GL_Context *gc __UNUSED__,
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
shader_array_flush(Evas_Engine_GL_Context *gc)
{
   int i, gw, gh, setclip, cy, fbo = 0, done = 0;

   if (!gc->havestuff) return;
   gw = gc->w;
   gh = gc->h;
   if (!((gc->pipe[0].shader.surface == gc->def_surface) ||
         (!gc->pipe[0].shader.surface)))
     {
        gw = gc->pipe[0].shader.surface->w;
        gh = gc->pipe[0].shader.surface->h;
        fbo = 1;
     }
   for (i = 0; i < gc->shared->info.tune.pipes.max; i++)
     {
        if (gc->pipe[i].array.num <= 0) break;
        setclip = 0;
        done++;
        gc->flushnum++;
        GLERR(__FUNCTION__, __FILE__, __LINE__, "<flush err>");
        if (gc->pipe[i].shader.cur_prog != gc->state.current.cur_prog)
          {
             glUseProgram(gc->pipe[i].shader.cur_prog);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }

        if (gc->pipe[i].shader.cur_tex != gc->state.current.cur_tex)
          {
#if 0
             if (gc->pipe[i].shader.cur_tex)
               {
                  glEnable(GL_TEXTURE_2D);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
               }
             else
               {
                  glDisable(GL_TEXTURE_2D);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
               }
#endif
             glActiveTexture(GL_TEXTURE0);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.cur_tex);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        if (gc->pipe[i].array.im)
          {
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
             if (gc->pipe[i].array.im->tex->pt->dyn.img)
               {
                  secsym_glEGLImageTargetTexture2DOES
                     (GL_TEXTURE_2D, gc->pipe[i].array.im->tex->pt->dyn.img);
               }
             else
#endif
               {
                  if (!gc->pipe[i].array.im->native.loose)
                    {
                       if (gc->pipe[i].array.im->native.func.bind)
                          gc->pipe[i].array.im->native.func.bind(gc->pipe[i].array.im->native.func.data,
                                                                 gc->pipe[i].array.im);
                    }
               }
          }
        if (gc->pipe[i].shader.render_op != gc->state.current.render_op)
          {
             switch (gc->pipe[i].shader.render_op)
               {
               case EVAS_RENDER_BLEND: /**< default op: d = d*(1-sa) + s */
                  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  break;
               case EVAS_RENDER_COPY: /**< d = s */
                  gc->pipe[i].shader.blend = 0;
                  glBlendFunc(GL_ONE, GL_ONE);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  break;
                  // FIXME: fix blend funcs below!
               case EVAS_RENDER_BLEND_REL: /**< d = d*(1 - sa) + s*da */
               case EVAS_RENDER_COPY_REL: /**< d = s*da */
               case EVAS_RENDER_ADD: /**< d = d + s */
               case EVAS_RENDER_ADD_REL: /**< d = d + s*da */
               case EVAS_RENDER_SUB: /**< d = d - s */
               case EVAS_RENDER_SUB_REL: /**< d = d - s*da */
               case EVAS_RENDER_TINT: /**< d = d*s + d*(1 - sa) + s*(1 - da) */
               case EVAS_RENDER_TINT_REL: /**< d = d*(1 - sa + s) */
               case EVAS_RENDER_MASK: /**< d = d*sa */
               case EVAS_RENDER_MUL: /**< d = d*s */
               default:
                  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  break;
               }
          }
        if (gc->pipe[i].shader.blend != gc->state.current.blend)
          {
             if (gc->pipe[i].shader.blend)
               {
                  glEnable(GL_BLEND);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
               }
             else
               {
                  glDisable(GL_BLEND);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
               }
          }
        if ((gc->pipe[i].shader.smooth != gc->state.current.smooth) ||
            (gc->pipe[i].shader.cur_tex != gc->state.current.cur_tex))
          {
             if (gc->pipe[i].shader.smooth)
               {
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
                  if (shared->info.anisotropic > 0.0)
                    {
                       glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, shared->info.anisotropic);
                       GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                    }
#endif
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
               }
             else
               {
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
                  if (shared->info.anisotropic > 0.0)
                    {
                       glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0);
                       GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                    }
#endif
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
               }
          }
        if (gc->pipe[i].shader.clip != gc->state.current.clip)
          {

             if (gc->pipe[i].shader.clip)
               {
                  cy = gh - gc->pipe[i].shader.cy - gc->pipe[i].shader.ch;
                  if (fbo) cy = gc->pipe[i].shader.cy;
                  glEnable(GL_SCISSOR_TEST);
                  if (!fbo)
                     scissor_rot(gc, gc->rot, gw, gh,
                                 gc->pipe[i].shader.cx,
                                 cy,
                                 gc->pipe[i].shader.cw,
                                 gc->pipe[i].shader.ch);
                  else
                     glScissor(gc->pipe[i].shader.cx, cy,
                               gc->pipe[i].shader.cw, gc->pipe[i].shader.ch);
                  setclip = 1;
               }
             else
               {
                  glDisable(GL_SCISSOR_TEST);
                  glScissor(0, 0, 0, 0);
               }
          }
        if ((gc->pipe[i].shader.clip) && (!setclip))
          {
             if ((gc->pipe[i].shader.cx != gc->state.current.cx) ||
                 (gc->pipe[i].shader.cy != gc->state.current.cy) ||
                 (gc->pipe[i].shader.cw != gc->state.current.cw) ||
                 (gc->pipe[i].shader.ch != gc->state.current.ch))
               {
                  cy = gh - gc->pipe[i].shader.cy - gc->pipe[i].shader.ch;
                  if (fbo) cy = gc->pipe[i].shader.cy;
                  if (!fbo)
                     scissor_rot(gc, gc->rot, gw, gh,
                                 gc->pipe[i].shader.cx,
                                 cy,
                                 gc->pipe[i].shader.cw,
                                 gc->pipe[i].shader.ch);
                  else
                     glScissor(gc->pipe[i].shader.cx, cy,
                               gc->pipe[i].shader.cw, gc->pipe[i].shader.ch);
               }
          }

        glVertexAttribPointer(SHAD_VERTEX, 3, GL_SHORT, GL_FALSE, 0, gc->pipe[i].array.vertex);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        glVertexAttribPointer(SHAD_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, gc->pipe[i].array.color);
        GLERR(__FUNCTION__, __FILE__, __LINE__, "");
        if (gc->pipe[i].array.use_texuv)
          {
             glEnableVertexAttribArray(SHAD_TEXUV);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glVertexAttribPointer(SHAD_TEXUV, 2, GL_FLOAT, GL_FALSE, 0, gc->pipe[i].array.texuv);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        else
          {
             glDisableVertexAttribArray(SHAD_TEXUV);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }

        if (gc->pipe[i].array.line)
          {
             glDisableVertexAttribArray(SHAD_TEXUV);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glDisableVertexAttribArray(SHAD_TEXUV2);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glDisableVertexAttribArray(SHAD_TEXUV3);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
             glDrawArrays(GL_LINES, 0, gc->pipe[i].array.num);
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        else
          {
	     if (gc->pipe[i].array.use_texm)
	       {
		  glEnableVertexAttribArray(SHAD_TEXM);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glVertexAttribPointer(SHAD_TEXM, 2, GL_FLOAT, GL_FALSE, 0, gc->pipe[i].array.texm);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glActiveTexture(GL_TEXTURE1);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texm);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glActiveTexture(GL_TEXTURE0);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
	       }
	     else
	       {
                  glDisableVertexAttribArray(SHAD_TEXM);
	       }
             if ((gc->pipe[i].array.use_texuv2) && (gc->pipe[i].array.use_texuv3))
               {
                  glEnableVertexAttribArray(SHAD_TEXUV2);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glEnableVertexAttribArray(SHAD_TEXUV3);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glVertexAttribPointer(SHAD_TEXUV2, 2, GL_FLOAT, GL_FALSE, 0, gc->pipe[i].array.texuv2);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glVertexAttribPointer(SHAD_TEXUV3, 2, GL_FLOAT, GL_FALSE, 0, gc->pipe[i].array.texuv3);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");

                  glActiveTexture(GL_TEXTURE1);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texu);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
                  if (gc->pipe[i].shader.cur_texu_dyn)
		    secsym_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texu_dyn);
#endif
                  
                  glActiveTexture(GL_TEXTURE2);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texv);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
		  if (gc->pipe[i].shader.cur_texv_dyn)
		    secsym_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texv_dyn);
#endif
                  glActiveTexture(GL_TEXTURE0);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
               }
             else if (gc->pipe[i].array.use_texuv2)
               {
                  glEnableVertexAttribArray(SHAD_TEXUV2);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glVertexAttribPointer(SHAD_TEXUV2, 2, GL_FLOAT, GL_FALSE, 0, gc->pipe[i].array.texuv2);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");

                  glActiveTexture(GL_TEXTURE1);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glBindTexture(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texu);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
#if defined (GLES_VARIETY_S3C6410) || defined (GLES_VARIETY_SGX)
		  if (gc->pipe[i].shader.cur_texu_dyn)
		    secsym_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, gc->pipe[i].shader.cur_texu_dyn);
#endif
                  glActiveTexture(GL_TEXTURE0);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
               }
             else
               {
                  glDisableVertexAttribArray(SHAD_TEXUV2);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
                  glDisableVertexAttribArray(SHAD_TEXUV3);
                  GLERR(__FUNCTION__, __FILE__, __LINE__, "");
               }
             if (dbgflushnum)
               {
                  const char *types[6] =
                    {"----", "RECT", "IMAG", "FONT", "YUV-", "MAP"};
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
             GLERR(__FUNCTION__, __FILE__, __LINE__, "");
          }
        if (gc->pipe[i].array.im)
          {
             if (!gc->pipe[i].array.im->native.loose)
               {
                  if (gc->pipe[i].array.im->native.func.unbind)
                     gc->pipe[i].array.im->native.func.unbind(gc->pipe[i].array.im->native.func.data,
                                                              gc->pipe[i].array.im);
               }
             gc->pipe[i].array.im = NULL;
          }

        gc->state.current.cur_prog  = gc->pipe[i].shader.cur_prog;
        gc->state.current.cur_tex   = gc->pipe[i].shader.cur_tex;
        gc->state.current.blend     = gc->pipe[i].shader.blend;
        gc->state.current.smooth    = gc->pipe[i].shader.smooth;
        gc->state.current.render_op = gc->pipe[i].shader.render_op;
        gc->state.current.clip      = gc->pipe[i].shader.clip;
        gc->state.current.cx        = gc->pipe[i].shader.cx;
        gc->state.current.cy        = gc->pipe[i].shader.cy;
        gc->state.current.cw        = gc->pipe[i].shader.cw;
        gc->state.current.ch        = gc->pipe[i].shader.ch;

        if (gc->pipe[i].array.vertex) free(gc->pipe[i].array.vertex);
        if (gc->pipe[i].array.color) free(gc->pipe[i].array.color);
        if (gc->pipe[i].array.texuv) free(gc->pipe[i].array.texuv);
        if (gc->pipe[i].array.texm) free(gc->pipe[i].array.texm);
        if (gc->pipe[i].array.texuv2) free(gc->pipe[i].array.texuv2);
        if (gc->pipe[i].array.texuv3) free(gc->pipe[i].array.texuv3);

        gc->pipe[i].array.vertex = NULL;
        gc->pipe[i].array.color = NULL;
        gc->pipe[i].array.texuv = NULL;
        gc->pipe[i].array.texm = NULL;
        gc->pipe[i].array.texuv2 = NULL;
        gc->pipe[i].array.texuv3 = NULL;

        gc->pipe[i].array.num = 0;
        gc->pipe[i].array.alloc = 0;

        gc->pipe[i].region.x = 0;
        gc->pipe[i].region.y = 0;
        gc->pipe[i].region.w = 0;
        gc->pipe[i].region.h = 0;
        gc->pipe[i].region.type = 0;
     }
   gc->state.top_pipe = 0;
   if (dbgflushnum)
     {
        if (done > 0) printf("DONE (pipes): %i\n", done);
     }
   gc->havestuff = EINA_FALSE;
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
