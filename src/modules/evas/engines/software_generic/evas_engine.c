#include "evas_common_private.h" /* Also includes international specific stuff */
#include "evas_private.h"
#include "evas_blend_private.h"
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

#ifdef HAVE_DLSYM
# include <dlfcn.h>      /* dlopen,dlclose,etc */

# define EVAS_GL 1
# define EVAS_GL_NO_GL_H_CHECK 1
# include "Evas_GL.h"

#else
# warning software_generic will not be able to have Evas_GL API.
#endif

#include "Evas_Engine_Software_Generic.h"

#ifdef EVAS_GL
//----------------------------------//
// OSMesa...

#define OSMESA_MAJOR_VERSION 6
#define OSMESA_MINOR_VERSION 5
#define OSMESA_PATCH_VERSION 0

/*
 * Values for the format parameter of OSMesaCreateContext()
 * New in version 2.0.
 */
#define OSMESA_COLOR_INDEX	GL_COLOR_INDEX
#define OSMESA_RGBA		GL_RGBA
#define OSMESA_BGRA		0x1
#define OSMESA_ARGB		0x2
#define OSMESA_RGB		GL_RGB
#define OSMESA_BGR		0x4
#define OSMESA_RGB_565		0x5


/*
 * OSMesaPixelStore() parameters:
 * New in version 2.0.
 */
#define OSMESA_ROW_LENGTH	0x10
#define OSMESA_Y_UP		0x11


/*
 * Accepted by OSMesaGetIntegerv:
 */
#define OSMESA_WIDTH		0x20
#define OSMESA_HEIGHT		0x21
#define OSMESA_FORMAT		0x22
#define OSMESA_TYPE		0x23
#define OSMESA_MAX_WIDTH	0x24  /* new in 4.0 */
#define OSMESA_MAX_HEIGHT	0x25  /* new in 4.0 */


typedef void (*OSMESAproc)();
typedef struct osmesa_context *OSMesaContext;
#endif

typedef struct _Render_Engine_GL_Surface    Render_Engine_GL_Surface;
typedef struct _Render_Engine_GL_Context    Render_Engine_GL_Context;

struct _Render_Engine_GL_Surface
{
   int     initialized;
   int     w, h;

#ifdef EVAS_GL
   GLenum  internal_fmt;
#endif
   int     internal_cpp;   // Component per pixel.  ie. RGB = 3

   int     depth_bits;
   int     stencil_bits;

   // Data 
   void   *buffer;

   Render_Engine_GL_Context   *current_ctx;
};

#ifdef EVAS_GL
struct _Render_Engine_GL_Context
{
   int            initialized;

   OSMesaContext  context;

   Render_Engine_GL_Context   *share_ctx;

   Render_Engine_GL_Surface   *current_sfc;
};

//------------------------------------------------------//
typedef void                   (*_eng_fn) (void );
typedef _eng_fn                (*glsym_func_eng_fn) ();
typedef void                   (*glsym_func_void) ();
typedef unsigned int           (*glsym_func_uint) ();
typedef int                    (*glsym_func_int) ();
typedef unsigned char          (*glsym_func_uchar) ();
typedef unsigned char         *(*glsym_func_uchar_ptr) ();
typedef const unsigned char   *(*glsym_func_const_uchar_ptr) ();
typedef char const            *(*glsym_func_char_const_ptr) ();
typedef GLboolean              (*glsym_func_bool) ();
typedef OSMesaContext          (*glsym_func_osm_ctx) ();
//------------------------------------------------------//

/* Function table for GL APIs */
static Evas_GL_API gl_funcs;
static void *gl_lib_handle;
static int gl_lib_is_gles = 0;
static Evas_GL_API gl_funcs;

//------------------------------------------------------//
// OSMesa APIS...
static OSMesaContext (*_sym_OSMesaCreateContextExt)             (GLenum format, GLint depthBits, GLint stencilBits, GLint accumBits, OSMesaContext sharelist) = NULL;
static void          (*_sym_OSMesaDestroyContext)               (OSMesaContext ctx) = NULL;
static GLboolean     (*_sym_OSMesaMakeCurrent)                  (OSMesaContext ctx, void *buffer, GLenum type, GLsizei width, GLsizei height) = NULL;
static void          (*_sym_OSMesaPixelStore)                   (GLint pname, GLint value) = NULL;
static OSMESAproc    (*_sym_OSMesaGetProcAddress)               (const char *funcName);


//------------------------------------------------------//
// GLES 2.0 APIs...
static void       (*_sym_glActiveTexture)                       (GLenum texture) = NULL;
static void       (*_sym_glAttachShader)                        (GLuint program, GLuint shader) = NULL;
static void       (*_sym_glBindAttribLocation)                  (GLuint program, GLuint index, const char* name) = NULL;
static void       (*_sym_glBindBuffer)                          (GLenum target, GLuint buffer) = NULL;
static void       (*_sym_glBindFramebuffer)                     (GLenum target, GLuint framebuffer) = NULL;
static void       (*_sym_glBindRenderbuffer)                    (GLenum target, GLuint renderbuffer) = NULL;
static void       (*_sym_glBindTexture)                         (GLenum target, GLuint texture) = NULL;
static void       (*_sym_glBlendColor)                          (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) = NULL;
static void       (*_sym_glBlendEquation)                       (GLenum mode) = NULL;
static void       (*_sym_glBlendEquationSeparate)               (GLenum modeRGB, GLenum modeAlpha) = NULL;
static void       (*_sym_glBlendFunc)                           (GLenum sfactor, GLenum dfactor) = NULL;
static void       (*_sym_glBlendFuncSeparate)                   (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) = NULL;
static void       (*_sym_glBufferData)                          (GLenum target, GLsizeiptr size, const void* data, GLenum usage) = NULL;
static void       (*_sym_glBufferSubData)                       (GLenum target, GLintptr offset, GLsizeiptr size, const void* data) = NULL;
static GLenum     (*_sym_glCheckFramebufferStatus)              (GLenum target) = NULL;
static void       (*_sym_glClear)                               (GLbitfield mask) = NULL;
static void       (*_sym_glClearColor)                          (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) = NULL;
static void       (*_sym_glClearDepthf)                         (GLclampf depth) = NULL;
static void       (*_sym_glClearStencil)                        (GLint s) = NULL;
static void       (*_sym_glColorMask)                           (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) = NULL;
static void       (*_sym_glCompileShader)                       (GLuint shader) = NULL;
static void       (*_sym_glCompressedTexImage2D)                (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data) = NULL;
static void       (*_sym_glCompressedTexSubImage2D)             (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data) = NULL;
static void       (*_sym_glCopyTexImage2D)                      (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) = NULL;
static void       (*_sym_glCopyTexSubImage2D)                   (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
static GLuint     (*_sym_glCreateProgram)                       (void) = NULL;
static GLuint     (*_sym_glCreateShader)                        (GLenum type) = NULL;
static void       (*_sym_glCullFace)                            (GLenum mode) = NULL;
static void       (*_sym_glDeleteBuffers)                       (GLsizei n, const GLuint* buffers) = NULL;
static void       (*_sym_glDeleteFramebuffers)                  (GLsizei n, const GLuint* framebuffers) = NULL;
static void       (*_sym_glDeleteProgram)                       (GLuint program) = NULL;
static void       (*_sym_glDeleteRenderbuffers)                 (GLsizei n, const GLuint* renderbuffers) = NULL;
static void       (*_sym_glDeleteShader)                        (GLuint shader) = NULL;
static void       (*_sym_glDeleteTextures)                      (GLsizei n, const GLuint* textures) = NULL;
static void       (*_sym_glDepthFunc)                           (GLenum func) = NULL;
static void       (*_sym_glDepthMask)                           (GLboolean flag) = NULL;
static void       (*_sym_glDepthRangef)                         (GLclampf zNear, GLclampf zFar) = NULL;
static void       (*_sym_glDetachShader)                        (GLuint program, GLuint shader) = NULL;
static void       (*_sym_glDisable)                             (GLenum cap) = NULL;
static void       (*_sym_glDisableVertexAttribArray)            (GLuint index) = NULL;
static void       (*_sym_glDrawArrays)                          (GLenum mode, GLint first, GLsizei count) = NULL;
static void       (*_sym_glDrawElements)                        (GLenum mode, GLsizei count, GLenum type, const void* indices) = NULL;
static void       (*_sym_glEnable)                              (GLenum cap) = NULL;
static void       (*_sym_glEnableVertexAttribArray)             (GLuint index) = NULL;
static void       (*_sym_glFinish)                              (void) = NULL;
static void       (*_sym_glFlush)                               (void) = NULL;
static void       (*_sym_glFramebufferRenderbuffer)             (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) = NULL;
static void       (*_sym_glFramebufferTexture2D)                (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) = NULL;
static void       (*_sym_glFrontFace)                           (GLenum mode) = NULL;
static void       (*_sym_glGenBuffers)                          (GLsizei n, GLuint* buffers) = NULL;
static void       (*_sym_glGenerateMipmap)                      (GLenum target) = NULL;
static void       (*_sym_glGenFramebuffers)                     (GLsizei n, GLuint* framebuffers) = NULL;
static void       (*_sym_glGenRenderbuffers)                    (GLsizei n, GLuint* renderbuffers) = NULL;
static void       (*_sym_glGenTextures)                         (GLsizei n, GLuint* textures) = NULL;
static void       (*_sym_glGetActiveAttrib)                     (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name) = NULL;
static void       (*_sym_glGetActiveUniform)                    (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name) = NULL;
static void       (*_sym_glGetAttachedShaders)                  (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders) = NULL;
static int        (*_sym_glGetAttribLocation)                   (GLuint program, const char* name) = NULL;
static void       (*_sym_glGetBooleanv)                         (GLenum pname, GLboolean* params) = NULL;
static void       (*_sym_glGetBufferParameteriv)                (GLenum target, GLenum pname, GLint* params) = NULL;
static GLenum     (*_sym_glGetError)                            (void) = NULL;
static void       (*_sym_glGetFloatv)                           (GLenum pname, GLfloat* params) = NULL;
static void       (*_sym_glGetFramebufferAttachmentParameteriv) (GLenum target, GLenum attachment, GLenum pname, GLint* params) = NULL;
static void       (*_sym_glGetIntegerv)                         (GLenum pname, GLint* params) = NULL;
static void       (*_sym_glGetProgramiv)                        (GLuint program, GLenum pname, GLint* params) = NULL;
static void       (*_sym_glGetProgramInfoLog)                   (GLuint program, GLsizei bufsize, GLsizei* length, char* infolog) = NULL;
static void       (*_sym_glGetRenderbufferParameteriv)          (GLenum target, GLenum pname, GLint* params) = NULL;
static void       (*_sym_glGetShaderiv)                         (GLuint shader, GLenum pname, GLint* params) = NULL;
static void       (*_sym_glGetShaderInfoLog)                    (GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog) = NULL;
static void       (*_sym_glGetShaderPrecisionFormat)            (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) = NULL;
static void       (*_sym_glGetShaderSource)                     (GLuint shader, GLsizei bufsize, GLsizei* length, char* source) = NULL;
static const GLubyte *(*_sym_glGetString)                           (GLenum name) = NULL;
static void       (*_sym_glGetTexParameterfv)                   (GLenum target, GLenum pname, GLfloat* params) = NULL;
static void       (*_sym_glGetTexParameteriv)                   (GLenum target, GLenum pname, GLint* params) = NULL;
static void       (*_sym_glGetUniformfv)                        (GLuint program, GLint location, GLfloat* params) = NULL;
static void       (*_sym_glGetUniformiv)                        (GLuint program, GLint location, GLint* params) = NULL;
static int        (*_sym_glGetUniformLocation)                  (GLuint program, const char* name) = NULL;
static void       (*_sym_glGetVertexAttribfv)                   (GLuint index, GLenum pname, GLfloat* params) = NULL;
static void       (*_sym_glGetVertexAttribiv)                   (GLuint index, GLenum pname, GLint* params) = NULL;
static void       (*_sym_glGetVertexAttribPointerv)             (GLuint index, GLenum pname, void** pointer) = NULL;
static void       (*_sym_glHint)                                (GLenum target, GLenum mode) = NULL;
static GLboolean  (*_sym_glIsBuffer)                            (GLuint buffer) = NULL;
static GLboolean  (*_sym_glIsEnabled)                           (GLenum cap) = NULL;
static GLboolean  (*_sym_glIsFramebuffer)                       (GLuint framebuffer) = NULL;
static GLboolean  (*_sym_glIsProgram)                           (GLuint program) = NULL;
static GLboolean  (*_sym_glIsRenderbuffer)                      (GLuint renderbuffer) = NULL;
static GLboolean  (*_sym_glIsShader)                            (GLuint shader) = NULL;
static GLboolean  (*_sym_glIsTexture)                           (GLuint texture) = NULL;
static void       (*_sym_glLineWidth)                           (GLfloat width) = NULL;
static void       (*_sym_glLinkProgram)                         (GLuint program) = NULL;
static void       (*_sym_glPixelStorei)                         (GLenum pname, GLint param) = NULL;
static void       (*_sym_glPolygonOffset)                       (GLfloat factor, GLfloat units) = NULL;
static void       (*_sym_glReadPixels)                          (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels) = NULL;
static void       (*_sym_glReleaseShaderCompiler)               (void) = NULL;
static void       (*_sym_glRenderbufferStorage)                 (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) = NULL;
static void       (*_sym_glSampleCoverage)                      (GLclampf value, GLboolean invert) = NULL;
static void       (*_sym_glScissor)                             (GLint x, GLint y, GLsizei width, GLsizei height) = NULL;
static void       (*_sym_glShaderBinary)                        (GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length) = NULL;
static void       (*_sym_glShaderSource)                        (GLuint shader, GLsizei count, const char* const* string, const GLint* length) = NULL;
static void       (*_sym_glStencilFunc)                         (GLenum func, GLint ref, GLuint mask) = NULL;
static void       (*_sym_glStencilFuncSeparate)                 (GLenum face, GLenum func, GLint ref, GLuint mask) = NULL;
static void       (*_sym_glStencilMask)                         (GLuint mask) = NULL;
static void       (*_sym_glStencilMaskSeparate)                 (GLenum face, GLuint mask) = NULL;
static void       (*_sym_glStencilOp)                           (GLenum fail, GLenum zfail, GLenum zpass) = NULL;
static void       (*_sym_glStencilOpSeparate)                   (GLenum face, GLenum fail, GLenum zfail, GLenum zpass) = NULL;
static void       (*_sym_glTexImage2D)                          (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels) = NULL;
static void       (*_sym_glTexParameterf)                       (GLenum target, GLenum pname, GLfloat param) = NULL;
static void       (*_sym_glTexParameterfv)                      (GLenum target, GLenum pname, const GLfloat* params) = NULL;
static void       (*_sym_glTexParameteri)                       (GLenum target, GLenum pname, GLint param) = NULL;
static void       (*_sym_glTexParameteriv)                      (GLenum target, GLenum pname, const GLint* params) = NULL;
static void       (*_sym_glTexSubImage2D)                       (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels) = NULL;
static void       (*_sym_glUniform1f)                           (GLint location, GLfloat x) = NULL;
static void       (*_sym_glUniform1fv)                          (GLint location, GLsizei count, const GLfloat* v) = NULL;
static void       (*_sym_glUniform1i)                           (GLint location, GLint x) = NULL;
static void       (*_sym_glUniform1iv)                          (GLint location, GLsizei count, const GLint* v) = NULL;
static void       (*_sym_glUniform2f)                           (GLint location, GLfloat x, GLfloat y) = NULL;
static void       (*_sym_glUniform2fv)                          (GLint location, GLsizei count, const GLfloat* v) = NULL;
static void       (*_sym_glUniform2i)                           (GLint location, GLint x, GLint y) = NULL;
static void       (*_sym_glUniform2iv)                          (GLint location, GLsizei count, const GLint* v) = NULL;
static void       (*_sym_glUniform3f)                           (GLint location, GLfloat x, GLfloat y, GLfloat z) = NULL;
static void       (*_sym_glUniform3fv)                          (GLint location, GLsizei count, const GLfloat* v) = NULL;
static void       (*_sym_glUniform3i)                           (GLint location, GLint x, GLint y, GLint z) = NULL;
static void       (*_sym_glUniform3iv)                          (GLint location, GLsizei count, const GLint* v) = NULL;
static void       (*_sym_glUniform4f)                           (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) = NULL;
static void       (*_sym_glUniform4fv)                          (GLint location, GLsizei count, const GLfloat* v) = NULL;
static void       (*_sym_glUniform4i)                           (GLint location, GLint x, GLint y, GLint z, GLint w) = NULL;
static void       (*_sym_glUniform4iv)                          (GLint location, GLsizei count, const GLint* v) = NULL;
static void       (*_sym_glUniformMatrix2fv)                    (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = NULL;
static void       (*_sym_glUniformMatrix3fv)                    (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = NULL;
static void       (*_sym_glUniformMatrix4fv)                    (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) = NULL;
static void       (*_sym_glUseProgram)                          (GLuint program) = NULL;
static void       (*_sym_glValidateProgram)                     (GLuint program) = NULL;
static void       (*_sym_glVertexAttrib1f)                      (GLuint indx, GLfloat x) = NULL;
static void       (*_sym_glVertexAttrib1fv)                     (GLuint indx, const GLfloat* values) = NULL;
static void       (*_sym_glVertexAttrib2f)                      (GLuint indx, GLfloat x, GLfloat y) = NULL;
static void       (*_sym_glVertexAttrib2fv)                     (GLuint indx, const GLfloat* values) = NULL;
static void       (*_sym_glVertexAttrib3f)                      (GLuint indx, GLfloat x, GLfloat y, GLfloat z) = NULL;
static void       (*_sym_glVertexAttrib3fv)                     (GLuint indx, const GLfloat* values) = NULL;
static void       (*_sym_glVertexAttrib4f)                      (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w) = NULL;
static void       (*_sym_glVertexAttrib4fv)                     (GLuint indx, const GLfloat* values) = NULL;
static void       (*_sym_glVertexAttribPointer)                 (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr) = NULL;
static void       (*_sym_glViewport)                            (GLint x, GLint y, GLsizei width, GLsizei height) = NULL;

// GLES Extensions...
/* static void       (*_sym_glGetProgramBinary)                    (GLuint a, GLsizei b, GLsizei* c, GLenum* d, void* e) = NULL; */
/* static void       (*_sym_glProgramBinary)                       (GLuint a, GLenum b, const void* c, GLint d) = NULL; */
/* static void       (*_sym_glProgramParameteri)                   (GLuint a, GLuint b, GLint d) = NULL; */
#endif

// Threaded Render

typedef struct _Evas_Thread_Command_Rect Evas_Thread_Command_Rect;
typedef struct _Evas_Thread_Command_Line Evas_Thread_Command_Line;
typedef struct _Evas_Thread_Command_Polygon Evas_Thread_Command_Polygon;
typedef struct _Evas_Thread_Command_Image Evas_Thread_Command_Image;
typedef struct _Evas_Thread_Command_Font Evas_Thread_Command_Font;
typedef struct _Evas_Thread_Command_Map Evas_Thread_Command_Map;
typedef struct _Evas_Thread_Command_Multi_Font Evas_Thread_Command_Multi_Font;

struct _Evas_Thread_Command_Rect
{
   void *surface;
   DATA32 color;
   int render_op;
   int x, y, w, h;
};

struct _Evas_Thread_Command_Line
{
   void *surface;
   Eina_Rectangle clip;
   DATA32 color;
   int render_op;
   Eina_Bool anti_alias;
   int x1, y1;
   int x2, y2;
};

struct _Evas_Thread_Command_Polygon
{
   Eina_Rectangle ext;
   DATA32 col;
   int render_op;
   void *surface;
   RGBA_Polygon_Point *points;
   int x, y;
};

struct _Evas_Thread_Command_Image
{
   void *surface;
   void *image;
   Eina_Rectangle src, dst, clip;
   DATA32 mul_col;
   int render_op;
   int smooth;
};

struct _Evas_Thread_Command_Font
{
   RGBA_Image *dst;
   int x;
   int y;
   Evas_Glyph_Array *glyphs;
   RGBA_Gfx_Func func;
   void *gl_new;
   void *gl_free;
   void *gl_draw;
   void *font_ext_data;
   DATA32 col;
   Eina_Bool clip_use : 1;
   Eina_Rectangle clip_rect, ext;
   int im_w, im_h;
};

struct _Evas_Thread_Command_Map
{
   void *image;
   RGBA_Draw_Context image_ctx;
   void *surface;
   Eina_Rectangle clip;
   DATA32 mul_col;
   int render_op;
   RGBA_Map *map;
   int smooth, level, offset;
};

struct _Evas_Thread_Command_Multi_Font
{
   RGBA_Draw_Context context;
   void *surface;
   int x, y;
   Evas_Font_Array *texts;
};

Eina_Mempool *_mp_command_rect = NULL;
Eina_Mempool *_mp_command_line = NULL;
Eina_Mempool *_mp_command_polygon = NULL;
Eina_Mempool *_mp_command_image = NULL;
Eina_Mempool *_mp_command_font = NULL;
Eina_Mempool *_mp_command_map = NULL;
Eina_Mempool *_mp_command_multi_font = NULL;

/*
 *****
 **
 ** ENGINE ROUTINES
 **
 *****
 */
static int cpunum = 0;
static int _evas_soft_gen_log_dom = -1;

static void
eng_output_dump(void *data EINA_UNUSED)
{
   evas_common_image_image_all_unload();
   evas_common_font_font_all_unload();
}

static void *
eng_context_new(void *data EINA_UNUSED)
{
   return evas_common_draw_context_new();
}

static void
eng_context_free(void *data EINA_UNUSED, void *context)
{
   evas_common_draw_context_free(context);
}

static void
eng_context_clip_set(void *data EINA_UNUSED, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_set_clip(context, x, y, w, h);
}

static void
eng_context_clip_clip(void *data EINA_UNUSED, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_clip_clip(context, x, y, w, h);
}

static void
eng_context_clip_unset(void *data EINA_UNUSED, void *context)
{
   evas_common_draw_context_unset_clip(context);
}

static int
eng_context_clip_get(void *data EINA_UNUSED, void *context, int *x, int *y, int *w, int *h)
{
   *x = ((RGBA_Draw_Context *)context)->clip.x;
   *y = ((RGBA_Draw_Context *)context)->clip.y;
   *w = ((RGBA_Draw_Context *)context)->clip.w;
   *h = ((RGBA_Draw_Context *)context)->clip.h;
   return ((RGBA_Draw_Context *)context)->clip.use;
}

static void
eng_context_color_set(void *data EINA_UNUSED, void *context, int r, int g, int b, int a)
{
   evas_common_draw_context_set_color(context, r, g, b, a);
}

static int
eng_context_color_get(void *data EINA_UNUSED, void *context, int *r, int *g, int *b, int *a)
{
   *r = (int)(R_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *g = (int)(G_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *b = (int)(B_VAL(&((RGBA_Draw_Context *)context)->col.col));
   *a = (int)(A_VAL(&((RGBA_Draw_Context *)context)->col.col));
   return 1;
}

static void
eng_context_multiplier_set(void *data EINA_UNUSED, void *context, int r, int g, int b, int a)
{
   evas_common_draw_context_set_multiplier(context, r, g, b, a);
}

static void
eng_context_multiplier_unset(void *data EINA_UNUSED, void *context)
{
   evas_common_draw_context_unset_multiplier(context);
}

static int
eng_context_multiplier_get(void *data EINA_UNUSED, void *context, int *r, int *g, int *b, int *a)
{
   *r = (int)(R_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *g = (int)(G_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *b = (int)(B_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   *a = (int)(A_VAL(&((RGBA_Draw_Context *)context)->mul.col));
   return ((RGBA_Draw_Context *)context)->mul.use;
}

static void
eng_context_cutout_add(void *data EINA_UNUSED, void *context, int x, int y, int w, int h)
{
   evas_common_draw_context_add_cutout(context, x, y, w, h);
}

static void
eng_context_cutout_clear(void *data EINA_UNUSED, void *context)
{
   evas_common_draw_context_clear_cutouts(context);
}

static void
eng_context_anti_alias_set(void *data EINA_UNUSED, void *context, unsigned char aa)
{
   evas_common_draw_context_set_anti_alias(context, aa);
}

static unsigned char
eng_context_anti_alias_get(void *data EINA_UNUSED, void *context)
{
   return ((RGBA_Draw_Context *)context)->anti_alias;
}

static void
eng_context_color_interpolation_set(void *data EINA_UNUSED, void *context, int color_space)
{
   evas_common_draw_context_set_color_interpolation(context, color_space);
}

static int
eng_context_color_interpolation_get(void *data EINA_UNUSED, void *context)
{
   return ((RGBA_Draw_Context *)context)->interpolation.color_space;
}

static void
eng_context_render_op_set(void *data EINA_UNUSED, void *context, int op)
{
   evas_common_draw_context_set_render_op(context, op);
}

static int
eng_context_render_op_get(void *data EINA_UNUSED, void *context)
{
   return ((RGBA_Draw_Context *)context)->render_op;
}

static void
_draw_thread_rectangle_draw(void *data)
{
    Evas_Thread_Command_Rect *rect = data;

    evas_common_rectangle_rgba_draw(rect->surface,
                                    rect->color, rect->render_op,
                                    rect->x, rect->y, rect->w, rect->h);

    eina_mempool_free(_mp_command_rect, rect);
}

static void
_draw_rectangle_thread_cmd(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, int w, int h)
{
   Evas_Thread_Command_Rect *cr;

   RECTS_CLIP_TO_RECT(x, y, w, h, dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
   if ((w <= 0) || (h <= 0)) return;

   cr = eina_mempool_malloc(_mp_command_rect, sizeof (Evas_Thread_Command_Rect));
   if (!cr) return;

   cr->surface = dst;
   cr->color = dc->col.col;
   cr->render_op = dc->render_op;
   cr->x = x;
   cr->y = y;
   cr->w = w;
   cr->h = h;

   evas_thread_cmd_enqueue(_draw_thread_rectangle_draw, cr);
}

static void
eng_rectangle_draw(void *data EINA_UNUSED, void *context, void *surface, int x, int y, int w, int h, Eina_Bool do_async)
{
   if (do_async)
     evas_common_rectangle_draw_cb(surface, context, x, y, w, h,
                                   _draw_rectangle_thread_cmd);
#ifdef BUILD_PIPE_RENDER
   else if ((cpunum > 1))
     evas_common_pipe_rectangle_draw(surface, context, x, y, w, h);
#endif
   else
     {
        evas_common_rectangle_draw(surface, context, x, y, w, h);
        evas_common_cpu_end_opt();
     }
}

static void
_draw_thread_line_draw(void *data)
{
   Evas_Thread_Command_Line *line = data;
   int clip_x, clip_y, clip_w, clip_h;

   clip_x = line->clip.x;
   clip_y = line->clip.y;
   clip_w = line->clip.w;
   clip_h = line->clip.h;

   if ((line->x1 == line->x2) && (line->y1 == line->y2))
     {
        evas_common_line_point_draw(line->surface,
                                    clip_x, clip_y, clip_w, clip_h,
                                    line->color, line->render_op,
                                    line->x1, line->y1);
        return;
     }

   if (line->anti_alias)
     evas_common_line_draw_line_aa
       (line->surface,
        clip_x, clip_y, clip_w, clip_h,
        line->color, line->render_op,
        line->x1, line->y1,
        line->x2, line->y2);
   else
     evas_common_line_draw_line
       (line->surface,
        clip_x, clip_y, clip_w, clip_h,
        line->color, line->render_op,
        line->x1, line->y1,
        line->x2, line->y2);

   eina_mempool_free(_mp_command_line, line);
}

static void
_line_draw_thread_cmd(RGBA_Image *dst, RGBA_Draw_Context *dc, int x1, int y1, int x2, int y2)
{
   Evas_Thread_Command_Line *cl;
   int clx, cly, clw, clh;
   int cx, cy, cw, ch;
   int x, y, w, h;

   cl = eina_mempool_malloc(_mp_command_line, sizeof (Evas_Thread_Command_Line));
   if (!cl) return;

   cl->surface = dst;

   if ((x1 == x2) && (y1 == y2))
     {
        EINA_RECTANGLE_SET(&cl->clip,
                           dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
        goto done;
     }

   clx = cly = 0;
   clw = dst->cache_entry.w;
   clh = dst->cache_entry.h;

   cx = dc->clip.x;
   cy = dc->clip.y;
   cw = dc->clip.w;
   ch = dc->clip.h;

   if (dc->clip.use)
     {
	RECTS_CLIP_TO_RECT(clx, cly, clw, clh, cx, cy, cw, ch);
	if ((clw < 1) || (clh < 1))
          {
             eina_mempool_free(_mp_command_line, cl);
             return;
          }
     }

   x = MIN(x1, x2);
   y = MIN(y1, y2);
   w = MAX(x1, x2) - x + 1;
   h = MAX(y1, y2) - y + 1;

   RECTS_CLIP_TO_RECT(clx, cly, clw, clh, x, y, w, h);
   if ((clw < 1) || (clh < 1))
     {
        eina_mempool_free(_mp_command_line, cl);
        return;
     }

   EINA_RECTANGLE_SET(&cl->clip, clx, cly, clw, clh);

 done:
   cl->color = dc->col.col;
   cl->render_op = dc->render_op;
   cl->anti_alias = dc->anti_alias;
   cl->x1 = x1;
   cl->y1 = y1;
   cl->x2 = x2;
   cl->y2 = y2;

   evas_thread_cmd_enqueue(_draw_thread_line_draw, cl);
}

static void
eng_line_draw(void *data EINA_UNUSED, void *context, void *surface, int x1, int y1, int x2, int y2, Eina_Bool do_async)
{
   if (do_async) _line_draw_thread_cmd(surface, context, x1, y1, x2, y2);
#ifdef BUILD_PIPE_RENDER
   else if ((cpunum > 1))
     evas_common_pipe_line_draw(surface, context, x1, y1, x2, y2);
#endif
   else
     {
        evas_common_line_draw(surface, context, x1, y1, x2, y2);
        evas_common_cpu_end_opt();
     }
}

static void *
eng_polygon_point_add(void *data EINA_UNUSED, void *context EINA_UNUSED, void *polygon, int x, int y)
{
   return evas_common_polygon_point_add(polygon, x, y);
}

static void *
eng_polygon_points_clear(void *data EINA_UNUSED, void *context EINA_UNUSED, void *polygon)
{
   return evas_common_polygon_points_clear(polygon);
}

static void
_draw_thread_polygon_cleanup(Evas_Thread_Command_Polygon *poly)
{
   RGBA_Polygon_Point *points = poly->points;

   while (points)
     {
        RGBA_Polygon_Point *p;

        p = points;
        points =
          (RGBA_Polygon_Point *)eina_inlist_remove(EINA_INLIST_GET(points),
                                                   EINA_INLIST_GET(points));
        free(p);
     }

   poly->points = NULL;
}

static void
_draw_thread_polygon_draw(void *data)
{
   Evas_Thread_Command_Polygon *poly = data;

   evas_common_polygon_rgba_draw
     (poly->surface,
      poly->ext.x, poly->ext.y, poly->ext.w, poly->ext.h,
      poly->col, poly->render_op,
      poly->points, poly->x, poly->y);

   _draw_thread_polygon_cleanup(poly);
   eina_mempool_free(_mp_command_polygon, poly);
}

static void
_polygon_draw_thread_points_populate(Evas_Thread_Command_Polygon *cp, RGBA_Polygon_Point *points)
{
   RGBA_Polygon_Point *cur, *npoints = NULL;

   if (!points) return;

   EINA_INLIST_FOREACH(EINA_INLIST_GET(points), cur)
     {
        RGBA_Polygon_Point *point;

        point = malloc(sizeof *point);
        point->x = cur->x;
        point->y = cur->y;

        npoints =
          (RGBA_Polygon_Point *)eina_inlist_append(EINA_INLIST_GET(npoints),
                                                   EINA_INLIST_GET(point));
     }

   cp->points = npoints;
}

static void
_polygon_draw_thread_cmd(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points, int x, int y)
{
   int ext_x, ext_y, ext_w, ext_h;
   Evas_Thread_Command_Polygon *cp;

   ext_x = 0;
   ext_y = 0;
   ext_w = dst->cache_entry.w;
   ext_h = dst->cache_entry.h;

   if (dc->clip.use)
     {
	if (dc->clip.x > ext_x)
	  {
	     ext_w += ext_x - dc->clip.x;
	     ext_x = dc->clip.x;
	  }

	if ((ext_x + ext_w) > (dc->clip.x + dc->clip.w))
          ext_w = (dc->clip.x + dc->clip.w) - ext_x;

	if (dc->clip.y > ext_y)
	  {
	     ext_h += ext_y - dc->clip.y;
	     ext_y = dc->clip.y;
	  }

	if ((ext_y + ext_h) > (dc->clip.y + dc->clip.h))
          ext_h = (dc->clip.y + dc->clip.h) - ext_y;
     }

   cp = eina_mempool_malloc(_mp_command_polygon, sizeof (Evas_Thread_Command_Polygon));
   if (!cp) return;

   EINA_RECTANGLE_SET(&cp->ext, ext_x, ext_y, ext_w, ext_h);
   cp->col = dc->col.col;
   cp->render_op = dc->render_op;
   cp->surface = dst;

   _polygon_draw_thread_points_populate(cp, points);

   cp->x = x;
   cp->y = y;

   evas_thread_cmd_enqueue(_draw_thread_polygon_draw, cp);
}

static void
eng_polygon_draw(void *data EINA_UNUSED, void *context, void *surface, void *polygon, int x, int y, Eina_Bool do_async)
{
   if (do_async) _polygon_draw_thread_cmd(surface, context, polygon, x, y);
#ifdef BUILD_PIPE_RENDER
   else if ((cpunum > 1))
     evas_common_pipe_poly_draw(surface, context, polygon, x, y);
#endif
   else
     {
	evas_common_polygon_draw(surface, context, polygon, x, y);
	evas_common_cpu_end_opt();
     }
}

static int
eng_image_alpha_get(void *data EINA_UNUSED, void *image)
{
   Image_Entry *im;

   if (!image) return 1;
   im = image;
   switch (im->space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (im->flags.alpha) return 1;
      default:
	break;
     }
   return 0;
}

static Evas_Colorspace
eng_image_colorspace_get(void *data EINA_UNUSED, void *image)
{
   Image_Entry *im;

   if (!image) return EVAS_COLORSPACE_ARGB8888;
   im = image;
   return im->space;
}

static Eina_Bool
eng_image_can_region_get(void *data EINA_UNUSED, void *image)
{
   Image_Entry *im;
   if (!image) return EINA_FALSE;
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
     return EINA_FALSE;
#endif
   im = image;
   return ((Evas_Image_Load_Func*) im->info.loader)->do_region;
}

static void *
eng_image_alpha_set(void *data EINA_UNUSED, void *image, int has_alpha)
{
   RGBA_Image *im;

   if (!image) return NULL;
   im = image;
   if (im->cache_entry.space != EVAS_COLORSPACE_ARGB8888)
     {
	im->cache_entry.flags.alpha = 0;
	return im;
     }
   if (!im->image.data) evas_cache_image_load_data(&im->cache_entry);
   im = (RGBA_Image *) evas_cache_image_alone(&im->cache_entry);
   im->cache_entry.flags.alpha = has_alpha ? 1 : 0;
   evas_common_image_colorspace_dirty(im);
   return im;
}

static void *
eng_image_border_set(void *data EINA_UNUSED, void *image, int l EINA_UNUSED, int r EINA_UNUSED, int t EINA_UNUSED, int b EINA_UNUSED)
{
   RGBA_Image *im;

   im = image;
   return im;
}

static void
eng_image_border_get(void *data EINA_UNUSED, void *image EINA_UNUSED, int *l EINA_UNUSED, int *r EINA_UNUSED, int *t EINA_UNUSED, int *b EINA_UNUSED)
{
}

static char *
eng_image_comment_get(void *data EINA_UNUSED, void *image, char *key EINA_UNUSED)
{
   RGBA_Image *im;

   if (!image) return NULL;
   im = image;
   return im->info.comment;
}

static char *
eng_image_format_get(void *data EINA_UNUSED, void *image EINA_UNUSED)
{
   return NULL;
}

static void
eng_image_colorspace_set(void *data EINA_UNUSED, void *image, Evas_Colorspace cspace)
{
   Image_Entry *im;

   if (!image) return;
   im = image;
   evas_cache_image_colorspace(im, cspace);
}

static void *
eng_image_native_set(void *data EINA_UNUSED, void *image, void *native)
{
   //return image;
   Evas_Native_Surface *ns = native;
   Image_Entry *im = image, *im2 = NULL;

   if (!im || !ns) return im;

   if ((ns->type == EVAS_NATIVE_SURFACE_OPENGL) &&
       (ns->version == EVAS_NATIVE_SURFACE_VERSION))
     im2 = evas_cache_image_data(evas_common_image_cache_get(), 
                                 im->w, im->h, 
                                 ns->data.x11.visual, 1,
                                 EVAS_COLORSPACE_ARGB8888);
   else
     im2 = evas_cache_image_data(evas_common_image_cache_get(), 
                                 im->w, im->h, 
                                 NULL, 1,
                                 EVAS_COLORSPACE_ARGB8888);
   if (im->references > 1)
     ERR("Setting native with more than one references for im=%p", im);

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(im))
     evas_cache2_image_close(im);
   else
#endif
   evas_cache_image_drop(im);

   return im2;
}

static void *
eng_image_native_get(void *data EINA_UNUSED, void *image EINA_UNUSED)
{
   return NULL;
}

static void *
eng_image_load(void *data EINA_UNUSED, const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   *error = EVAS_LOAD_ERROR_NONE;
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
     {
        Image_Entry *ie;
        ie = evas_cache2_image_open(evas_common_image_cache2_get(),
                                    file, key, lo, error);
        if (ie)
          {
             *error = evas_cache2_image_open_wait(ie);
             if ((*error != EVAS_LOAD_ERROR_NONE) && ie->animated.animated)
               {
                  evas_cache2_image_close(ie);
                  goto use_local_cache;
               }
          }
        return ie;
     }
use_local_cache:
#endif

   return evas_common_load_image_from_file(file, key, lo, error);
}

static void *
eng_image_mmap(void *data EINA_UNUSED, Eina_File *f, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   *error = EVAS_LOAD_ERROR_NONE;
#ifdef EVAS_CSERVE2
   // FIXME: Need to pass fd to make that useful, so just get the filename for now.
   if (evas_cserve2_use_get() && !eina_file_virtual(f))
     {
        Image_Entry *ie;
        ie = evas_cache2_image_open(evas_common_image_cache2_get(),
                                    eina_file_filename_get(f), key, lo, error);
        if (ie)
          {
            *error = evas_cache2_image_open_wait(ie);
            if ((*error != EVAS_LOAD_ERROR_NONE) && ie->animated.animated)
              {
                 evas_cache2_image_close(ie);
                 goto use_local_cache;
              }
          }
        return ie;
     }
use_local_cache:
#endif

   return evas_common_load_image_from_mmap(f, key, lo, error);
}

static void *
eng_image_new_from_data(void *data EINA_UNUSED, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace)
{
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
     {
        Evas_Cache2 *cache = evas_common_image_cache2_get();
        return evas_cache2_image_data(cache, w, h, image_data, alpha, cspace);
     }
#endif
   return evas_cache_image_data(evas_common_image_cache_get(), w, h, image_data, alpha, cspace);
}

static void *
eng_image_new_from_copied_data(void *data EINA_UNUSED, int w, int h, DATA32 *image_data, int alpha, Evas_Colorspace cspace)
{
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
     {
        Evas_Cache2 *cache = evas_common_image_cache2_get();
        return evas_cache2_image_copied_data(cache, w, h, image_data, alpha,
                                             cspace);
     }
#endif
   return evas_cache_image_copied_data(evas_common_image_cache_get(), w, h, image_data, alpha, cspace);
}

static void
eng_image_free(void *data EINA_UNUSED, void *image)
{
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(image))
     {
        evas_cache2_image_close(image);
        return;
     }
#endif
   evas_cache_image_drop(image);
}

static void
eng_image_size_get(void *data EINA_UNUSED, void *image, int *w, int *h)
{
   Image_Entry *im;

   im = image;
   if (w) *w = im->w;
   if (h) *h = im->h;
}

static void *
eng_image_size_set(void *data EINA_UNUSED, void *image, int w, int h)
{
   Image_Entry *im = image;
   if (!im) return NULL;
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(im))
     return evas_cache2_image_size_set(im, w, h);
#endif
   return evas_cache_image_size_set(im, w, h);
}

static void *
eng_image_dirty_region(void *data EINA_UNUSED, void *image, int x, int y, int w, int h)
{
   Image_Entry *im = image;
   if (!im) return NULL;
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(im))
     return evas_cache2_image_dirty(im, x, y, w, h);
#endif
   return evas_cache_image_dirty(im, x, y, w, h);
}

static void *
eng_image_data_get(void *data EINA_UNUSED, void *image, int to_write, DATA32 **image_data, int *err)
{
   RGBA_Image *im;
   int error = EVAS_LOAD_ERROR_NONE;

   if (!image)
     {
	*image_data = NULL;
	return NULL;
     }
   im = image;

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(&im->cache_entry))
     {
        error = evas_cache2_image_load_data(&im->cache_entry);
        if (err) *err = error;

        if (to_write)
          {
             im = (RGBA_Image *)evas_cache2_image_writable(&im->cache_entry);
             if (!im)
               {
                  *image_data = NULL;
                  return NULL;
               }
          }
     }
   else
#endif
   error = evas_cache_image_load_data(&im->cache_entry);

   switch (im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (to_write)
          im = (RGBA_Image *)evas_cache_image_alone(&im->cache_entry);
	*image_data = im->image.data;
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
   RGBA_Image *im, *im2;

   if (!image) return NULL;
   im = image;
   switch (im->cache_entry.space)
     {
      case EVAS_COLORSPACE_ARGB8888:
	if (image_data != im->image.data)
	  {
	     int w, h;

	     w = im->cache_entry.w;
	     h = im->cache_entry.h;
	     im2 = eng_image_new_from_data(data, w, h, image_data,
					   eng_image_alpha_get(data, image),
					   eng_image_colorspace_get(data, image));
#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get())
               {
                  evas_cache2_image_close(&im->cache_entry);
                  im = im2;
                  break;
               }
#endif
             evas_cache_image_drop(&im->cache_entry);
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
        evas_common_image_colorspace_dirty(im);
        break;
      default:
	abort();
	break;
     }
   return im;
}

static void
eng_image_data_preload_request(void *data EINA_UNUSED, void *image, const Eo *target)
{
   RGBA_Image *im = image;
   if (!im) return;

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(&im->cache_entry))
     {
        evas_cache2_image_preload_data(&im->cache_entry, target);
        return;
     }
#endif
   evas_cache_image_preload_data(&im->cache_entry, target, NULL, NULL, NULL);
}

static void
eng_image_data_preload_cancel(void *data EINA_UNUSED, void *image, const Eo *target)
{
   RGBA_Image *im = image;

   if (!im) return;

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get() && evas_cache2_image_cached(&im->cache_entry))
     {
        evas_cache2_image_preload_cancel(&im->cache_entry, target);
        return;
     }
#endif

   evas_cache_image_preload_cancel(&im->cache_entry, target);
}

static void
_draw_thread_image_draw(void *data)
{
   Evas_Thread_Command_Image *image = data;

   if (image->smooth)
     evas_common_scale_rgba_smooth_draw
       (image->image, image->surface,
        image->clip.x, image->clip.y, image->clip.w, image->clip.h,
        image->mul_col, image->render_op,
        image->src.x, image->src.y, image->src.w, image->src.h,
        image->dst.x, image->dst.y, image->dst.w, image->dst.h);
   else
     evas_common_scale_rgba_sample_draw
       (image->image, image->surface,
        image->clip.x, image->clip.y, image->clip.w, image->clip.h,
        image->mul_col, image->render_op,
        image->src.x, image->src.y, image->src.w, image->src.h,
        image->dst.x, image->dst.y, image->dst.w, image->dst.h);

   eina_mempool_free(_mp_command_image, image);
}

static Eina_Bool
_image_draw_thread_cmd(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   Evas_Thread_Command_Image *cr;
   int clip_x, clip_y, clip_w, clip_h;

   if ((dst_w <= 0) || (dst_h <= 0)) return EINA_FALSE;
   if (!(RECTS_INTERSECT(dst_x, dst_y, dst_w, dst_h,
                         0, 0, dst->cache_entry.w, dst->cache_entry.h))) return EINA_FALSE;

   cr = eina_mempool_malloc(_mp_command_image, sizeof (Evas_Thread_Command_Image));
   if (!cr) return EINA_FALSE;

   cr->image = src;
   cr->surface = dst;
   EINA_RECTANGLE_SET(&cr->src, src_x, src_y, src_w, src_h);
   EINA_RECTANGLE_SET(&cr->dst, dst_x, dst_y, dst_w, dst_h);

   if (dc->clip.use)
     {
	clip_x = dc->clip.x;
	clip_y = dc->clip.y;
	clip_w = dc->clip.w;
	clip_h = dc->clip.h;
     }
   else
     {
	clip_x = 0;
	clip_y = 0;
	clip_w = dst->cache_entry.w;
	clip_h = dst->cache_entry.h;
     }

   EINA_RECTANGLE_SET(&cr->clip, clip_x, clip_y, clip_w, clip_h);

   cr->mul_col = dc->mul.use ? dc->mul.col : 0xffffffff;
   cr->render_op = dc->render_op;
   cr->smooth = smooth;

   evas_thread_cmd_enqueue(_draw_thread_image_draw, cr);

   return EINA_TRUE;
}

static Eina_Bool
_image_draw_thread_cmd_smooth(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h)
{
   return _image_draw_thread_cmd(src, dst, dc,
                                 src_x, src_y, src_w, src_h,
                                 dst_x, dst_y, dst_w, dst_h,
                                 1);
}

static Eina_Bool
_image_draw_thread_cmd_sample(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h)
{
   return _image_draw_thread_cmd(src, dst, dc,
                                 src_x, src_y, src_w, src_h,
                                 dst_x, dst_y, dst_w, dst_h,
                                 0);
}

static Eina_Bool
_image_thr_cb_smooth(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h)
{
   return evas_common_scale_rgba_in_to_out_clip_cb(src, dst, dc,
                                                   src_x, src_y, src_w, src_h,
                                                   dst_x, dst_y, dst_w, dst_h,
                                                   _image_draw_thread_cmd_smooth);
}

static Eina_Bool
_image_thr_cb_sample(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h)
{
   return evas_common_scale_rgba_in_to_out_clip_cb(src, dst, dc,
                                                   src_x, src_y, src_w, src_h,
                                                   dst_x, dst_y, dst_w, dst_h,
                                                   _image_draw_thread_cmd_sample);
}

static Eina_Bool
eng_image_draw(void *data EINA_UNUSED, void *context, void *surface, void *image, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth, Eina_Bool do_async)
{
   RGBA_Image *im;

   if (!image) return EINA_FALSE;
   im = image;

   if (do_async)
     {
        if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
          {
#if EVAS_CSERVE2
             if (evas_cserve2_use_get() && evas_cache2_image_cached(&im->cache_entry))
               evas_cache2_image_load_data(&im->cache_entry);
             else
#endif
               evas_cache_image_load_data(&im->cache_entry);

             if (!im->cache_entry.flags.loaded) return EINA_FALSE;
          }

        evas_common_rgba_image_scalecache_prepare(image, surface, context, smooth,
                                                  src_x, src_y, src_w, src_h,
                                                  dst_x, dst_y, dst_w, dst_h);

        return evas_common_rgba_image_scalecache_do_cbs(image, surface,
                                                        context, smooth,
                                                        src_x, src_y, src_w, src_h,
                                                        dst_x, dst_y, dst_w, dst_h,
                                                        _image_thr_cb_sample,
                                                        _image_thr_cb_smooth);
     }
#ifdef BUILD_PIPE_RENDER
   else if ((cpunum > 1))
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_load_data(&im->cache_entry);
#endif
        evas_common_rgba_image_scalecache_prepare((Image_Entry *)(im),
                                                  surface, context, smooth,
                                                  src_x, src_y, src_w, src_h,
                                                  dst_x, dst_y, dst_w, dst_h);

        evas_common_pipe_image_draw(im, surface, context, smooth,
                                    src_x, src_y, src_w, src_h,
                                    dst_x, dst_y, dst_w, dst_h);
     }
#endif
   else
     {
        evas_common_rgba_image_scalecache_prepare
          (&im->cache_entry, surface, context, smooth,
           src_x, src_y, src_w, src_h,
           dst_x, dst_y, dst_w, dst_h);
        evas_common_rgba_image_scalecache_do
          (&im->cache_entry, surface, context, smooth,
           src_x, src_y, src_w, src_h,
           dst_x, dst_y, dst_w, dst_h);

        evas_common_cpu_end_opt();
     }

   return EINA_FALSE;
}

static void
_map_image_draw(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   int clip_x, clip_y, clip_w, clip_h;
   DATA32 mul_col;

   if ((dst_w <= 0) || (dst_h <= 0)) return;
   if (!(RECTS_INTERSECT(dst_x, dst_y, dst_w, dst_h,
                         0, 0, dst->cache_entry.w, dst->cache_entry.h))) return;

   if (dc->clip.use)
     {
	clip_x = dc->clip.x;
	clip_y = dc->clip.y;
	clip_w = dc->clip.w;
	clip_h = dc->clip.h;
     }
   else
     {
	clip_x = clip_y = 0;
	clip_w = dst->cache_entry.w;
	clip_h = dst->cache_entry.h;
     }

   mul_col = dc->mul.use ? dc->mul.col : 0xffffffff;

   if (smooth)
     evas_common_scale_rgba_smooth_draw(src, dst,
                                        clip_x, clip_y, clip_w, clip_h,
                                        mul_col, dc->render_op,
                                        src_x, src_y, src_w, src_h,
                                        dst_x, dst_y, dst_w, dst_h);
   else
     evas_common_scale_rgba_sample_draw(src, dst,
                                        clip_x, clip_y, clip_w, clip_h,
                                        mul_col, dc->render_op,
                                        src_x, src_y, src_w, src_h,
                                        dst_x, dst_y, dst_w, dst_h);
}

static Eina_Bool
_map_image_sample_draw(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h)
{
   _map_image_draw(src, dst, dc,
                   src_x, src_y, src_w, src_h,
                   dst_x, dst_y, dst_w, dst_h, 0);
   return EINA_TRUE;
}

static Eina_Bool
_map_image_smooth_draw(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_x, int src_y, int src_w, int src_h, int dst_x, int dst_y, int dst_w, int dst_h)
{
   _map_image_draw(src, dst, dc,
                   src_x, src_y, src_w, src_h,
                   dst_x, dst_y, dst_w, dst_h, 1);
   return EINA_TRUE;
}

static void
_draw_thread_map_draw(void *data)
{
   Evas_Thread_Command_Map *map = data;
   int offset = map->offset;
   RGBA_Map *m = map->map;
   RGBA_Image *im = map->image;
   int dx, dy, dw, dh;

   do
     {
        if (m->count - offset < 3) goto free_out;

        //Fully Transparency. Skip this.
        if (!(m->pts[0 + offset].col & 0xff000000) &&
            !(m->pts[1 + offset].col & 0xff000000) &&
            !(m->pts[2 + offset].col & 0xff000000) &&
            !(m->pts[3 + offset].col & 0xff000000))
          {
             offset += 2;
             continue;
          }

        if ((m->pts[0 + offset].x == m->pts[3 + offset].x) &&
            (m->pts[1 + offset].x == m->pts[2 + offset].x) &&
            (m->pts[0 + offset].y == m->pts[1 + offset].y) &&
            (m->pts[3 + offset].y == m->pts[2 + offset].y) &&
            (m->pts[0 + offset].x <= m->pts[1 + offset].x) &&
            (m->pts[0 + offset].y <= m->pts[2 + offset].y) &&
            (m->pts[0 + offset].u == 0) &&
            (m->pts[0 + offset].v == 0) &&
            (m->pts[1 + offset].u == (int)(im->cache_entry.w << FP)) &&
            (m->pts[1 + offset].v == 0) &&
            (m->pts[2 + offset].u == (int)(im->cache_entry.w << FP)) &&
            (m->pts[2 + offset].v == (int)(im->cache_entry.h << FP)) &&
            (m->pts[3 + offset].u == 0) &&
            (m->pts[3 + offset].v == (int)(im->cache_entry.h << FP)) &&
            (m->pts[0].col == m->pts[1].col) &&
            (m->pts[1].col == m->pts[2].col) &&
            (m->pts[2].col == m->pts[3].col))
          {
             DATA32 col;
             Eina_Bool use;

             dx = m->pts[0 + offset].x >> FP;
             dy = m->pts[0 + offset].y >> FP;
             dw = (m->pts[2 + offset].x >> FP) - dx;
             dh = (m->pts[2 + offset].y >> FP) - dy;

             col = map->image_ctx.mul.col;
             use = map->image_ctx.mul.use;
             if (use) map->image_ctx.mul.col = MUL4_SYM(col, m->pts[0].col);
             else map->image_ctx.mul.col = m->pts[0].col;
             map->image_ctx.mul.use = 1;

             if (map->smooth)
               evas_common_scale_rgba_in_to_out_clip_cb
                 (im, map->surface, &map->image_ctx,
                  0, 0, im->cache_entry.w, im->cache_entry.h,
                  dx, dy, dw, dh, _map_image_smooth_draw);
             else
               evas_common_scale_rgba_in_to_out_clip_cb
                 (im, map->surface, &map->image_ctx,
                  0, 0, im->cache_entry.w, im->cache_entry.h,
                  dx, dy, dw, dh, _map_image_sample_draw);

             map->image_ctx.mul.col = col;
             map->image_ctx.mul.use = use;
          }
        else
          {
             evas_common_map_rgba_draw
               (im, map->surface,
                map->clip.x, map->clip.y, map->clip.w, map->clip.h,
                map->mul_col, map->render_op, m->count - offset, &m->pts[offset],
                map->smooth, map->level);
          }

        evas_common_cpu_end_opt();

        offset += 2;
     }
   while ((m->count > 4) && (m->count - offset >= 3));

 free_out:
   free(m);
   eina_mempool_free(_mp_command_map, map);
}

static Eina_Bool
_map_draw_thread_cmd(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Map *map, int smooth, int level, int offset)
{
   Evas_Thread_Command_Map *cm;
   int clip_x, clip_y, clip_w, clip_h;

   cm = eina_mempool_malloc(_mp_command_map, sizeof (Evas_Thread_Command_Map));
   if (!cm) return EINA_FALSE;

   cm->image = src;
   memcpy(&cm->image_ctx, dc, sizeof(*dc));
   cm->surface = dst;

   if (dc->clip.use)
     {
	clip_x = dc->clip.x;
	clip_y = dc->clip.y;
	clip_w = dc->clip.w;
	clip_h = dc->clip.h;
     }
   else
     {
	clip_x = clip_y = 0;
	clip_w = dst->cache_entry.w;
	clip_h = dst->cache_entry.h;
     }

   EINA_RECTANGLE_SET(&cm->clip, clip_x, clip_y, clip_w, clip_h);

   cm->mul_col = dc->mul.use ? dc->mul.col : 0xffffffff;
   cm->render_op = dc->render_op;

   cm->map = calloc(1, sizeof(RGBA_Map) +
                    sizeof(RGBA_Map_Point) * map->count);
   cm->map->engine_data = map->engine_data;
   cm->map->image.w = map->image.w;
   cm->map->image.h = map->image.h;
   cm->map->uv.w = map->uv.w;
   cm->map->uv.h = map->uv.h;
   cm->map->x = map->x;
   cm->map->y = map->y;
   cm->map->count = map->count;
   memcpy(&cm->map->pts[0], &map->pts[0], sizeof(RGBA_Map_Point) * map->count);

   cm->smooth = smooth;
   cm->level = level;
   cm->offset = offset;

   evas_thread_cmd_enqueue(_draw_thread_map_draw, cm);

   return EINA_TRUE;
}

static void
evas_software_image_map_draw(void *data, void *context, RGBA_Image *surface, RGBA_Image *im, RGBA_Map *m, int smooth, int level, int offset)
{
   if (m->count - offset < 3) return;

   if ((m->pts[0 + offset].x == m->pts[3 + offset].x) &&
       (m->pts[1 + offset].x == m->pts[2 + offset].x) &&
       (m->pts[0 + offset].y == m->pts[1 + offset].y) &&
       (m->pts[3 + offset].y == m->pts[2 + offset].y) &&
       (m->pts[0 + offset].x <= m->pts[1 + offset].x) &&
       (m->pts[0 + offset].y <= m->pts[2 + offset].y) &&
       (m->pts[0 + offset].u == 0) &&
       (m->pts[0 + offset].v == 0) &&
       (m->pts[1 + offset].u == (int)(im->cache_entry.w << FP)) &&
       (m->pts[1 + offset].v == 0) &&
       (m->pts[2 + offset].u == (int)(im->cache_entry.w << FP)) &&
       (m->pts[2 + offset].v == (int)(im->cache_entry.h << FP)) &&
       (m->pts[3 + offset].u == 0) &&
       (m->pts[3 + offset].v == (int)(im->cache_entry.h << FP)) &&
       (m->pts[0 + offset].col == m->pts[1 + offset].col) &&
       (m->pts[1 + offset].col == m->pts[2 + offset].col) &&
       (m->pts[2 + offset].col == m->pts[3 + offset].col))
     {
        DATA32 col;
        int a, r, g, b;
        int dx, dy, dw, dh;
        int mul;

        mul = eng_context_multiplier_get(data, context, &r, &g, &b, &a);
        if (mul) col = MUL4_256(a, r, g, b, m->pts[0 + offset].col);
        else col = m->pts[0 + offset].col;
        eng_context_multiplier_set(data, context, R_VAL(&col), G_VAL(&col), B_VAL(&col), A_VAL(&col));

        dx = m->pts[0 + offset].x >> FP;
        dy = m->pts[0 + offset].y >> FP;
        dw = (m->pts[2 + offset].x >> FP) - dx;
        dh = (m->pts[2 + offset].y >> FP) - dy;
        eng_image_draw
          (data, context, surface, im,
           0, 0, im->cache_entry.w, im->cache_entry.h,
           dx, dy, dw, dh, smooth,
           EINA_FALSE);

        if (mul) eng_context_multiplier_set(data, context, r, g, b, a);
        else eng_context_multiplier_unset(data, context);
     }
   else
     {
#ifdef BUILD_PIPE_RENDER
        if ((cpunum > 1))
	  {
             evas_common_pipe_map_draw(im, surface, context, m, smooth, level);
             return;
          }
        else
#endif
          {
             evas_common_map_rgba(im, surface, context, m->count - offset, &m->pts[offset], smooth, level);
          }
     }
   evas_common_cpu_end_opt();

   if (m->count > 4)
     {
        evas_software_image_map_draw(data, context, surface, im, m, smooth, level, offset + 2);
     }
}

static Eina_Bool
eng_image_map_draw(void *data, void *context, void *surface, void *image, RGBA_Map *m, int smooth, int level, Eina_Bool do_async)
{
   RGBA_Image *im = image;

   if (!im) return EINA_FALSE;
   if (m->count < 3) return EINA_FALSE;

   if (do_async)
     {
        /* Since the thread that'll draw the map won't call eng_image_draw()
         * (which sends the load request of source image to Cserve2) - we need
         * to send the load request here before enqueueing thread command.
         */
        if (im->cache_entry.space == EVAS_COLORSPACE_ARGB8888)
          {
#if EVAS_CSERVE2
             if (evas_cserve2_use_get())
               evas_cache2_image_load_data(&im->cache_entry);
             else
#endif
               evas_cache_image_load_data(&im->cache_entry);

             if (!im->cache_entry.flags.loaded) return EINA_FALSE;
          }

        return evas_common_map_thread_rgba_cb(im, surface, context,
                                              m, smooth, level, 0,
                                              _map_draw_thread_cmd);
     }
   else
     evas_software_image_map_draw(data, context, surface, im, m,
                                  smooth, level, 0);

   return EINA_FALSE;
}

static void
eng_image_map_clean(void *data EINA_UNUSED, RGBA_Map *m)
{
   evas_common_map_rgba_clean(m);
}

static void *
eng_image_map_surface_new(void *data EINA_UNUSED, int w, int h, int alpha)
{
   void *surface;
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
     {
        surface = evas_cache2_image_copied_data(evas_common_image_cache2_get(),
                                                w, h, NULL, alpha,
                                                EVAS_COLORSPACE_ARGB8888);
        evas_cache2_image_pixels(surface);
        return surface;
     }
#endif
   surface = evas_cache_image_copied_data(evas_common_image_cache_get(), 
                                          w, h, NULL, alpha, 
                                          EVAS_COLORSPACE_ARGB8888);
   evas_cache_image_pixels(surface);
   return surface;
}

static void
eng_image_map_surface_free(void *data EINA_UNUSED, void *surface)
{
#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
     {
        evas_cache2_image_unload_data(surface);
        return;
     }
#endif
   evas_cache_image_drop(surface);
}

static void
eng_image_scale_hint_set(void *data EINA_UNUSED, void *image, int hint)
{
   Image_Entry *im;

   if (!image) return;
   im = image;
   im->scale_hint = hint;
}

static int
eng_image_scale_hint_get(void *data EINA_UNUSED, void *image)
{
   Image_Entry *im;

   if (!image) return EVAS_IMAGE_SCALE_HINT_NONE;
   im = image;
   return im->scale_hint;
}

static Eina_Bool
eng_image_animated_get(void *data EINA_UNUSED, void *image)
{
   Image_Entry *im;

   if (!image) return EINA_FALSE;
   im = image;
   return im->animated.animated;
}

static int
eng_image_animated_frame_count_get(void *data EINA_UNUSED, void *image)
{
   Image_Entry *im;

   if (!image) return -1;
   im = image;
   if (!im->animated.animated) return -1;
   return im->animated.frame_count;
}

static Evas_Image_Animated_Loop_Hint
eng_image_animated_loop_type_get(void *data EINA_UNUSED, void *image)
{
   Image_Entry *im;

   if (!image) return EVAS_IMAGE_ANIMATED_HINT_NONE;
   im = image;
   if (!im->animated.animated) return EVAS_IMAGE_ANIMATED_HINT_NONE;
   return im->animated.loop_hint;
}

static int
eng_image_animated_loop_count_get(void *data EINA_UNUSED, void *image)
{
   Image_Entry *im;

   if (!image) return -1;
   im = image;
   if (!im->animated.animated) return -1;
   return im->animated.loop_count;
}

static double
eng_image_animated_frame_duration_get(void *data EINA_UNUSED, void *image, int start_frame, int frame_num)
{
   Image_Entry *im;

   if (!image) return -1;
   im = image;
   if (!im->animated.animated) return -1;
   return evas_common_load_rgba_image_frame_duration_from_file(im, start_frame, frame_num);
}

static Eina_Bool
eng_image_animated_frame_set(void *data EINA_UNUSED, void *image, int frame_index)
{
   Image_Entry *im;

   if (!image) return EINA_FALSE;
   im = image;
   if (!im->animated.animated) return EINA_FALSE;
   if (im->animated.cur_frame == frame_index) return EINA_FALSE;
   im->animated.cur_frame = frame_index;
   return EINA_TRUE;
}

static void
_draw_thread_multi_font_draw(void *data)
{
   Evas_Thread_Command_Multi_Font *mf = data;
   Evas_Font_Array_Data           *itr;

   EINA_INARRAY_FOREACH(mf->texts->array, itr)
     {
        unsigned int r, g, b, a;
        int x = mf->x + itr->x, y = mf->y;

        r = itr->color.r;
        g = itr->color.g;
        b = itr->color.b;
        a = itr->color.a;

        eng_context_color_set(NULL, &mf->context, r, g, b, a);
        evas_common_font_draw(mf->surface, &mf->context, x, y, itr->glyphs);
        evas_common_cpu_end_opt();
     }

   eina_mempool_free(_mp_command_multi_font, mf);
}

static Eina_Bool
_multi_font_draw_thread_cmd(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, Evas_Font_Array *texts)
{
   Evas_Thread_Command_Multi_Font *mf;

   mf = eina_mempool_malloc(_mp_command_multi_font,
                            sizeof(Evas_Thread_Command_Multi_Font));
   if (!mf)
     {
        ERR("Failed to allocate memory on mempool for multiple text_props "
            "commands.");
        return EINA_FALSE;
     }

   memcpy(&mf->context, dc, sizeof(*dc));
   mf->surface = dst;
   mf->x = x;
   mf->y = y;
   mf->texts = texts;

   evas_thread_cmd_enqueue(_draw_thread_multi_font_draw, mf);

   return EINA_TRUE;
}

static Eina_Bool
eng_multi_font_draw(void *data EINA_UNUSED, void *context, void *surface, Evas_Font_Set *font EINA_UNUSED, int x, int y, int w EINA_UNUSED, int h EINA_UNUSED, int ow EINA_UNUSED, int oh EINA_UNUSED, Evas_Font_Array *texts, Eina_Bool do_async)
{
   if (!texts) return EINA_FALSE;

   if (do_async)
     return _multi_font_draw_thread_cmd(surface, context, x, y, texts);

   return EINA_FALSE;
}

static Eina_Bool
eng_pixel_alpha_get(void *image, int x, int y, DATA8 *alpha, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h)
{
   RGBA_Image *im = image;
   int px, py, dx, dy, sx, sy, src_w, src_h;
   double scale_w, scale_h;

   if (!im) return EINA_FALSE;

   if ((dst_region_x > x) || (x >= (dst_region_x + dst_region_w)) ||
       (dst_region_y > y) || (y >= (dst_region_y + dst_region_h)))
     {
        *alpha = 0;
        return EINA_FALSE;
     }

   src_w = im->cache_entry.w;
   src_h = im->cache_entry.h;
   if ((src_w == 0) || (src_h == 0))
     {
        *alpha = 0;
        return EINA_TRUE;
     }

   EINA_SAFETY_ON_TRUE_GOTO(src_region_x < 0, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(src_region_y < 0, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(src_region_x + src_region_w > src_w, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(src_region_y + src_region_h > src_h, error_oob);

   scale_w = (double)dst_region_w / (double)src_region_w;
   scale_h = (double)dst_region_h / (double)src_region_h;

   /* point at destination */
   dx = x - dst_region_x;
   dy = y - dst_region_y;

   /* point at source */
   sx = dx / scale_w;
   sy = dy / scale_h;

   /* pixel point (translated) */
   px = src_region_x + sx;
   py = src_region_y + sy;
   EINA_SAFETY_ON_TRUE_GOTO(px >= src_w, error_oob);
   EINA_SAFETY_ON_TRUE_GOTO(py >= src_h, error_oob);

   switch (im->cache_entry.space)
     {
     case EVAS_COLORSPACE_ARGB8888:
       {
          DATA32 *pixel;

#ifdef EVAS_CSERVE2
          if (evas_cserve2_use_get())
            evas_cache2_image_load_data(&im->cache_entry);
          else
#endif
            evas_cache_image_load_data(&im->cache_entry);

          if (!im->cache_entry.flags.loaded)
            {
               ERR("im %p has no pixels loaded yet", im);
               return EINA_FALSE;
            }

          pixel = im->image.data;
          pixel += ((py * src_w) + px);
          *alpha = ((*pixel) >> 24) & 0xff;
       }
       break;

     default:
        ERR("Colorspace %d not supported.", im->cache_entry.space);
        *alpha = 0;
     }

   return EINA_TRUE;

 error_oob:
   ERR("Invalid region src=(%d, %d, %d, %d), dst=(%d, %d, %d, %d), image=%dx%d",
       src_region_x, src_region_y, src_region_w, src_region_h,
       dst_region_x, dst_region_y, dst_region_w, dst_region_h,
       src_w, src_h);
   *alpha = 0;
   return EINA_TRUE;
}

static void
eng_image_cache_flush(void *data EINA_UNUSED)
{
   int tmp_size;

   tmp_size = evas_common_image_get_cache();
   evas_common_image_set_cache(0);
   evas_common_rgba_image_scalecache_flush();
   evas_common_image_set_cache(tmp_size);
}

static void
eng_image_cache_set(void *data EINA_UNUSED, int bytes)
{
   evas_common_image_set_cache(bytes);
   evas_common_rgba_image_scalecache_size_set(bytes);
}

static int
eng_image_cache_get(void *data EINA_UNUSED)
{
   return evas_common_image_get_cache();
}

static Evas_Font_Set *
eng_font_load(void *data EINA_UNUSED, const char *name, int size,
      Font_Rend_Flags wanted_rend)
{
   return (Evas_Font_Set *) evas_common_font_load(name, size, wanted_rend);
}

static Evas_Font_Set *
eng_font_memory_load(void *data EINA_UNUSED, const char *source, const char *name, int size, const void *fdata, int fdata_size, Font_Rend_Flags wanted_rend)
{
   return (Evas_Font_Set *) evas_common_font_memory_load(source, name, size,
         fdata, fdata_size, wanted_rend);
}

static Evas_Font_Set *
eng_font_add(void *data EINA_UNUSED, Evas_Font_Set *font, const char *name, int size, Font_Rend_Flags wanted_rend)
{
   return (Evas_Font_Set *) evas_common_font_add((RGBA_Font *) font, name,
         size, wanted_rend);
}

static Evas_Font_Set *
eng_font_memory_add(void *data EINA_UNUSED, Evas_Font_Set *font, const char *source, const char *name, int size, const void *fdata, int fdata_size, Font_Rend_Flags wanted_rend)
{
   return (Evas_Font_Set *) evas_common_font_memory_add((RGBA_Font *) font,
         source, name, size, fdata, fdata_size, wanted_rend);
}

static void
eng_font_free(void *data EINA_UNUSED, Evas_Font_Set *font)
{
   evas_common_font_free((RGBA_Font *) font);
}

static int
eng_font_ascent_get(void *data EINA_UNUSED, Evas_Font_Set *font)
{
   return evas_common_font_ascent_get((RGBA_Font *) font);
}

static int
eng_font_descent_get(void *data EINA_UNUSED, Evas_Font_Set *font)
{
   return evas_common_font_descent_get((RGBA_Font *) font);
}

static int
eng_font_max_ascent_get(void *data EINA_UNUSED, Evas_Font_Set *font)
{
   return evas_common_font_max_ascent_get((RGBA_Font *) font);
}

static int
eng_font_max_descent_get(void *data EINA_UNUSED, Evas_Font_Set *font)
{
   return evas_common_font_max_descent_get((RGBA_Font *) font);
}

static void
eng_font_string_size_get(void *data EINA_UNUSED, Evas_Font_Set *font, const Evas_Text_Props *text_props, int *w, int *h)
{
   evas_common_font_query_size((RGBA_Font *) font, text_props, w, h);
}

static int
eng_font_inset_get(void *data EINA_UNUSED, Evas_Font_Set *font, const Evas_Text_Props *text_props)
{
   return evas_common_font_query_inset((RGBA_Font *) font, text_props);
}

static int
eng_font_right_inset_get(void *data EINA_UNUSED, Evas_Font_Set *font, const Evas_Text_Props *text_props)
{
   return evas_common_font_query_right_inset((RGBA_Font *) font, text_props);
}

static int
eng_font_h_advance_get(void *data EINA_UNUSED, Evas_Font_Set *font, const Evas_Text_Props *text_props)
{
   int h, v;

   evas_common_font_query_advance((RGBA_Font *) font, text_props, &h, &v);
   return h;
}

static int
eng_font_v_advance_get(void *data EINA_UNUSED, Evas_Font_Set *font, const Evas_Text_Props *text_props)
{
   int h, v;

   evas_common_font_query_advance((RGBA_Font *) font, text_props, &h, &v);
   return v;
}

static int
eng_font_pen_coords_get(void *data EINA_UNUSED, Evas_Font_Set *font, const Evas_Text_Props *text_props, int pos, int *cpen_x, int *cy, int *cadv, int *ch)
{
   return evas_common_font_query_pen_coords((RGBA_Font *) font, text_props, pos, cpen_x, cy, cadv, ch);
}

static Eina_Bool
eng_font_text_props_info_create(void *data EINA_UNUSED, Evas_Font_Instance *fi, const Eina_Unicode *text, Evas_Text_Props *text_props, const Evas_BiDi_Paragraph_Props *par_props, size_t par_pos, size_t len, Evas_Text_Props_Mode mode)
{
   return evas_common_text_props_content_create((RGBA_Font_Int *) fi, text,
         text_props, par_props, par_pos, len, mode);
}

static int
eng_font_char_coords_get(void *data EINA_UNUSED, Evas_Font_Set *font, const Evas_Text_Props *text_props, int pos, int *cx, int *cy, int *cw, int *ch)
{
   return evas_common_font_query_char_coords((RGBA_Font *) font, text_props, pos, cx, cy, cw, ch);
}

static int
eng_font_char_at_coords_get(void *data EINA_UNUSED, Evas_Font_Set *font, const Evas_Text_Props *text_props, int x, int y, int *cx, int *cy, int *cw, int *ch)
{
   return evas_common_font_query_char_at_coords((RGBA_Font *) font, text_props, x, y, cx, cy, cw, ch);
}

static int
eng_font_last_up_to_pos(void *data EINA_UNUSED, Evas_Font_Set *font, const Evas_Text_Props *text_props, int x, int y)
{
   return evas_common_font_query_last_up_to_pos((RGBA_Font *) font, text_props, x, y);
}

static int
eng_font_run_font_end_get(void *data EINA_UNUSED, Evas_Font_Set *font, Evas_Font_Instance **script_fi, Evas_Font_Instance **cur_fi, Evas_Script_Type script, const Eina_Unicode *text, int run_len)
{
   return evas_common_font_query_run_font_end_get((RGBA_Font *) font,
         (RGBA_Font_Int **) script_fi, (RGBA_Font_Int **) cur_fi,
         script, text, run_len);
}

static void
_draw_thread_font_draw(void *data)
{
   Evas_Thread_Command_Font *font = data;
   RGBA_Draw_Context dc;
   memset(&dc, 0, sizeof(dc));

   dc.font_ext.data = font->font_ext_data;
   dc.font_ext.func.gl_new = font->gl_new;
   dc.font_ext.func.gl_free = font->gl_free;
   dc.font_ext.func.gl_draw = font->gl_draw;
   dc.col.col = font->col;
   dc.clip.use = font->clip_use;
   dc.clip.x = font->clip_rect.x;
   dc.clip.y = font->clip_rect.y;
   dc.clip.w = font->clip_rect.w;
   dc.clip.h = font->clip_rect.h;

   evas_common_font_rgba_draw
     (font->dst, &dc,
      font->x, font->y, font->glyphs, font->func,
      font->ext.x, font->ext.y, font->ext.w, font->ext.h,
      font->im_w, font->im_h);

   eina_mempool_free(_mp_command_font, font);
}

static Eina_Bool
_font_draw_thread_cmd(RGBA_Image *dst, RGBA_Draw_Context *dc, int x, int y, Evas_Glyph_Array *glyphs, RGBA_Gfx_Func func, int ext_x, int ext_y, int ext_w, int ext_h, int im_w, int im_h)
{
   Evas_Thread_Command_Font *cf;

   cf = eina_mempool_malloc(_mp_command_font, sizeof (Evas_Thread_Command_Font));
   if (!cf) return EINA_FALSE;

   cf->dst = dst;
   cf->x = x;
   cf->y = y;
   cf->gl_new = dc->font_ext.func.gl_new;
   cf->gl_free = dc->font_ext.func.gl_free;
   cf->gl_draw = dc->font_ext.func.gl_draw;
   cf->font_ext_data = dc->font_ext.data;
   cf->col = dc->col.col;
   cf->clip_use = dc->clip.use;
   EINA_RECTANGLE_SET(&cf->clip_rect,
                      dc->clip.x, dc->clip.y, dc->clip.w, dc->clip.h);
   cf->glyphs = glyphs;
   cf->func = func;
   EINA_RECTANGLE_SET(&cf->ext, ext_x, ext_y, ext_w, ext_h);
   cf->im_w = im_w;
   cf->im_h = im_h;

   evas_thread_cmd_enqueue(_draw_thread_font_draw, cf);

   return EINA_TRUE;
}

static Eina_Bool
eng_font_draw(void *data EINA_UNUSED, void *context, void *surface, Evas_Font_Set *font EINA_UNUSED, int x, int y, int w EINA_UNUSED, int h EINA_UNUSED, int ow EINA_UNUSED, int oh EINA_UNUSED, Evas_Text_Props *text_props, Eina_Bool do_async)
{
   if (do_async)
     {
        evas_common_font_draw_prepare(text_props);
        if (!text_props->glyphs) return EINA_FALSE;

        return evas_common_font_draw_cb(surface, context, x, y, text_props->glyphs,
                                        _font_draw_thread_cmd);
     }
#ifdef BUILD_PIPE_RENDER
   else if ((cpunum > 1))
     evas_common_pipe_text_draw(surface, context, x, y, text_props);
#endif
   else
     {
        evas_common_font_draw_prepare(text_props);
        evas_common_font_draw(surface, context, x, y, text_props->glyphs);
        evas_common_cpu_end_opt();
     }

   return EINA_FALSE;
}

static void
eng_font_cache_flush(void *data EINA_UNUSED)
{
   int tmp_size;

   tmp_size = evas_common_font_cache_get();
   evas_common_font_cache_set(0);
   evas_common_font_flush();
   evas_common_font_cache_set(tmp_size);
}

static void
eng_font_cache_set(void *data EINA_UNUSED, int bytes)
{
   evas_common_font_cache_set(bytes);
}

static int
eng_font_cache_get(void *data EINA_UNUSED)
{
   return evas_common_font_cache_get();
}

static void
eng_font_hinting_set(void *data EINA_UNUSED, Evas_Font_Set *font, int hinting)
{
   evas_common_font_hinting_set((RGBA_Font *) font, hinting);
}

static int
eng_font_hinting_can_hint(void *data EINA_UNUSED, int hinting)
{
   return evas_common_hinting_available(hinting);
}

static Eina_Bool
eng_canvas_alpha_get(void *data EINA_UNUSED, void *info EINA_UNUSED)
{
   return EINA_TRUE;
}


static int
eng_image_load_error_get(void *data EINA_UNUSED, void *image)
{
   RGBA_Image *im;
   
   if (!image) return EVAS_LOAD_ERROR_NONE;
   im = image;
   return im->cache_entry.load_error;
}

//------------ Evas GL engine code ---------------//
static void *
eng_gl_surface_create(void *data EINA_UNUSED, void *config, int w, int h)
{
#ifdef EVAS_GL
   Render_Engine_GL_Surface *sfc;
   Evas_GL_Config *cfg;

   sfc = calloc(1, sizeof(Render_Engine_GL_Surface));
   if (!sfc) return NULL;

   cfg = (Evas_GL_Config *)config;

   sfc->initialized  = 0;
   sfc->w            = w;
   sfc->h            = h;

   // Color Format
   switch (cfg->color_format)
     {

      case EVAS_GL_RGB_888:
         sfc->internal_fmt = OSMESA_RGB;
         sfc->internal_cpp = 3;
         break;
      case EVAS_GL_RGBA_8888:
         sfc->internal_fmt = OSMESA_BGRA;
         sfc->internal_cpp = 4;
         break;
      default:
         sfc->internal_fmt = OSMESA_RGBA;
         sfc->internal_cpp = 4;
         break;
     }

   // Depth Bits
   switch (cfg->depth_bits)
     {
      case EVAS_GL_DEPTH_BIT_8:
         sfc->depth_bits = 8;
         break;
      case EVAS_GL_DEPTH_BIT_16:
         sfc->depth_bits = 16;
         break;
      case EVAS_GL_DEPTH_BIT_24:
         sfc->depth_bits = 24;
         break;
      case EVAS_GL_DEPTH_BIT_32:
         sfc->depth_bits = 32;
         break;
      case EVAS_GL_DEPTH_NONE:
      default:
         sfc->depth_bits = 0;
         break;
     }
   
   // Stencil Bits
   switch (cfg->stencil_bits)
     {
      case EVAS_GL_STENCIL_BIT_1:
         sfc->stencil_bits = 1;
         break;
      case EVAS_GL_STENCIL_BIT_2:
         sfc->stencil_bits = 2;
         break;
      case EVAS_GL_STENCIL_BIT_4:
         sfc->stencil_bits = 4;
         break;
      case EVAS_GL_STENCIL_BIT_8:
         sfc->stencil_bits = 8;
         break;
      case EVAS_GL_STENCIL_BIT_16:
         sfc->stencil_bits = 16;
         break;
      case EVAS_GL_STENCIL_NONE:
      default:
         sfc->stencil_bits = 0;
         break;
     }

   sfc->buffer = malloc(sizeof(unsigned char)*sfc->internal_cpp*w*h);

   if (!sfc->buffer)
     {
        free(sfc);
        return NULL;
     }

   return sfc;
#else
   (void) config;
   (void) w;
   (void) h;
   return NULL;
#endif
}

static int
eng_gl_surface_destroy(void *data EINA_UNUSED, void *surface)
{
#ifdef EVAS_GL
   Render_Engine_GL_Surface *sfc;

   sfc = (Render_Engine_GL_Surface*)surface;

   if (!sfc) return 0;

   if (sfc->buffer) free(sfc->buffer);

   free(sfc);

   surface = NULL;

   return 1;
#else
   (void) surface;
   return 1;
#endif
}

static void *
eng_gl_context_create(void *data EINA_UNUSED, void *share_context)
{
#ifdef EVAS_GL
   Render_Engine_GL_Context *ctx;
   Render_Engine_GL_Context *share_ctx;

   ctx = calloc(1, sizeof(Render_Engine_GL_Context));

   if (!ctx) return NULL;

   share_ctx = (Render_Engine_GL_Context *)share_context;

   ctx->share_ctx = share_ctx;

#if 0
   if (share_ctx)
      ctx->context = _sym_OSMesaCreateContextExt( OSMESA_RGBA, 8, 0, 0, share_ctx->context );
   else
      ctx->context = _sym_OSMesaCreateContextExt( OSMESA_RGBA, 8, 0, 0, NULL );


   if (!ctx->context)
     {
        ERR("Error creating OSMesa Context.");
        free(ctx);
        return NULL;
     }
#endif

   ctx->initialized = 0;

   return ctx;
#else
   (void) share_context;
   return NULL;
#endif
}

static int
eng_gl_context_destroy(void *data EINA_UNUSED, void *context)
{
#ifdef EVAS_GL
   Render_Engine_GL_Context *ctx;

   ctx = (Render_Engine_GL_Context*)context;

   if (!ctx) return 0;

   _sym_OSMesaDestroyContext(ctx->context);

   free(ctx);
   context = NULL;

   return 1;
#else
   (void) context;
   return 0;
#endif
}

static int
eng_gl_make_current(void *data EINA_UNUSED, void *surface, void *context)
{
#ifdef EVAS_GL
   Render_Engine_GL_Surface *sfc;
   Render_Engine_GL_Context *ctx;
   OSMesaContext share_ctx;
   GLboolean ret;

   sfc = (Render_Engine_GL_Surface*)surface;
   ctx = (Render_Engine_GL_Context*)context;

   // Unset surface/context
   if ((!sfc) || (!ctx))
     {
        if (ctx) ctx->current_sfc = NULL;
        if (sfc) sfc->current_ctx = NULL;
        return 1;
     }

   // Initialize Context if it hasn't been.
   if (!ctx->initialized)
     {
        if (ctx->share_ctx)
           share_ctx = ctx->share_ctx->context;
        else
           share_ctx = NULL;

        ctx->context =  _sym_OSMesaCreateContextExt(sfc->internal_fmt, 
                                                    sfc->depth_bits,
                                                    sfc->stencil_bits,
                                                    0,
                                                    share_ctx);
        if (!ctx->context)
          {
             ERR("Error initializing context.");
             return 0;
          }

        ctx->initialized = 1;
     }


   // Call MakeCurrent
   ret = _sym_OSMesaMakeCurrent(ctx->context, sfc->buffer, GL_UNSIGNED_BYTE, 
                                sfc->w, sfc->h);

   if (ret == GL_FALSE)
     {
        ERR("Error doing MakeCurrent.");
        return 0;
     }

   _sym_OSMesaPixelStore(OSMESA_Y_UP, 0);

   // Set the current surface/context
   ctx->current_sfc = sfc;
   sfc->current_ctx = ctx;

   return 1;
#else
   (void) surface;
   (void) context;
   return 1;
#endif
}

// FIXME!!! Implement later
static void *
eng_gl_string_query(void *data EINA_UNUSED, int name EINA_UNUSED)
{
   return NULL;
}

static void *
eng_gl_proc_address_get(void *data EINA_UNUSED, const char *name)
{
#ifdef EVAS_GL
   if (_sym_OSMesaGetProcAddress) return _sym_OSMesaGetProcAddress(name);
   return dlsym(RTLD_DEFAULT, name);
#else
   (void) name;
   return NULL;
#endif
}

static int
eng_gl_native_surface_get(void *data EINA_UNUSED, void *surface, void *native_surface)
{
#ifdef EVAS_GL
   Render_Engine_GL_Surface *sfc;
   Evas_Native_Surface *ns;

   sfc = (Render_Engine_GL_Surface*)surface;
   ns  = (Evas_Native_Surface*)native_surface;

   if (!sfc) return 0;

   ns->type = EVAS_NATIVE_SURFACE_OPENGL;
   ns->version = EVAS_NATIVE_SURFACE_VERSION;
   ns->data.x11.visual = sfc->buffer;
   
   return 1;
#else
   (void) surface;
   (void) native_surface;
   return 1;
#endif
}


static void *
eng_gl_api_get(void *data EINA_UNUSED)
{
#ifdef EVAS_GL
   return &gl_funcs;
#else
   return NULL;
#endif
}

//------------------------------------------------//

/* The following function require that any engine
   inheriting from software generic to have at the
   top of their render engine structure a
   Render_Engine_Software_Generic structure that is
   initialized by evas_render_engine_software_generic_init().
 */

static void
eng_output_resize(void *data, int w, int h)
{
   Render_Engine_Software_Generic *re;

   re = (Render_Engine_Software_Generic *)data;
   re->outbuf_reconfigure(re->ob, w, h, re->outbuf_get_rot(re->ob),
                          OUTBUF_DEPTH_INHERIT);
   evas_common_tilebuf_free(re->tb);
   re->tb = evas_common_tilebuf_new(w, h);
   if (re->tb)
     evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);
   re->w = w;
   re->h = h;
}

static void
eng_output_tile_size_set(void *data, int w, int h)
{
   Render_Engine_Software_Generic *re;

   re = (Render_Engine_Software_Generic *)data;
   evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void
eng_output_redraws_rect_add(void *data, int x, int y, int w, int h)
{
   Render_Engine_Software_Generic *re;

   re = (Render_Engine_Software_Generic *)data;
   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void
eng_output_redraws_rect_del(void *data, int x, int y, int w, int h)
{
   Render_Engine_Software_Generic *re;

   re = (Render_Engine_Software_Generic *)data;
   evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void
eng_output_redraws_clear(void *data)
{
   Render_Engine_Software_Generic *re;

   re = (Render_Engine_Software_Generic *)data;
   evas_common_tilebuf_clear(re->tb);
}

static Tilebuf_Rect *
_merge_rects(Render_Engine_Merge_Mode merge_mode,
             Tilebuf *tb,
             Tilebuf_Rect *r1,
             Tilebuf_Rect *r2,
             Tilebuf_Rect *r3,
             Tilebuf_Rect *r4)
{
   Tilebuf_Rect *r, *rects;

   if (r1)
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(r1), r)
          {
             evas_common_tilebuf_add_redraw(tb, r->x, r->y, r->w, r->h);
          }
     }
   if (r2)
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(r2), r)
          {
             evas_common_tilebuf_add_redraw(tb, r->x, r->y, r->w, r->h);
          }
     }
   if (r3)
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(r3), r)
          {
             evas_common_tilebuf_add_redraw(tb, r->x, r->y, r->w, r->h);
          }
     }

   if (r4)
     {
        EINA_INLIST_FOREACH(EINA_INLIST_GET(r4), r)
          {
             evas_common_tilebuf_add_redraw(tb, r->x, r->y, r->w, r->h);
          }
     }

   rects = evas_common_tilebuf_get_render_rects(tb);
   // bounding box -> make a bounding box single region update of all regions.
   // yes we could try and be smart and figure out size of regions, how far
   // apart etc. etc. to try and figure out an optimal "set". this is a tradeoff
   // between multiple update regions to render and total pixels to render.
   if (merge_mode == MERGE_BOUNDING && rects)
     {
        int px1, py1, px2, py2;

        px1 = rects->x; py1 = rects->y;
        px2 = rects->x + rects->w; py2 = rects->y + rects->h;
        EINA_INLIST_FOREACH(EINA_INLIST_GET(rects), r)
          {
             if (r->x < px1) px1 = r->x;
             if (r->y < py1) py1 = r->y;
             if ((r->x + r->w) > px2) px2 = r->x + r->w;
             if ((r->y + r->h) > py2) py2 = r->y + r->h;
          }
        evas_common_tilebuf_free_render_rects(rects);
        rects = calloc(1, sizeof(Tilebuf_Rect));
        if (rects)
          {
             rects->x = px1;
             rects->y = py1;
             rects->w = px2 - px1;
             rects->h = py2 - py1;
          }
     }
   evas_common_tilebuf_clear(tb);
   return rects;
}


static void *
eng_output_redraws_next_update_get(void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine_Software_Generic *re;
   RGBA_Image *surface;
   Tilebuf_Rect *rect;
   Eina_Bool first_rect = EINA_FALSE;

#define CLEAR_PREV_RECTS(x) \
   do { \
      if (re->rects_prev[x]) \
        evas_common_tilebuf_free_render_rects(re->rects_prev[x]); \
      re->rects_prev[x] = NULL; \
   } while (0)

   re = (Render_Engine_Software_Generic *)data;
   if (re->end)
     {
        re->end = 0;
        return NULL;
     }

   if (!re->rects)
     {
        int mode = MODE_COPY;

        re->rects = evas_common_tilebuf_get_render_rects(re->tb);
        if (re->rects)
          {
             if (re->outbuf_swap_mode_get) mode = re->outbuf_swap_mode_get(re->ob);
             re->swap_mode = mode;
             if ((re->lost_back) || (re->swap_mode == MODE_FULL))
               {
                  /* if we lost our backbuffer since the last frame redraw all */
                  re->lost_back = 0;
                  evas_common_tilebuf_add_redraw(re->tb, 0, 0, re->w, re->h);
                  evas_common_tilebuf_free_render_rects(re->rects);
                  re->rects = evas_common_tilebuf_get_render_rects(re->tb);
               }
             /* ensure we get rid of previous rect lists we dont need if mode
              * changed/is appropriate */
             evas_common_tilebuf_clear(re->tb);
             CLEAR_PREV_RECTS(3);
             re->rects_prev[3] = re->rects_prev[2];
             re->rects_prev[2] = re->rects_prev[1];
             re->rects_prev[1] = re->rects_prev[0];
             re->rects_prev[0] = re->rects;
             re->rects = NULL;
             switch (re->swap_mode)
               {
                case MODE_FULL:
                case MODE_COPY: // no prev rects needed
                  re->rects = _merge_rects(re->merge_mode, re->tb, re->rects_prev[0], NULL, NULL, NULL);
                  break;
                case MODE_DOUBLE: // double mode - only 1 level of prev rect
                  re->rects = _merge_rects(re->merge_mode, re->tb, re->rects_prev[0], re->rects_prev[1], NULL, NULL);
                  break;
                case MODE_TRIPLE: // triple mode - 2 levels of prev rect
                  re->rects = _merge_rects(re->merge_mode, re->tb, re->rects_prev[0], re->rects_prev[1], re->rects_prev[2], NULL);
                  break;
                case MODE_QUADRUPLE: // keep all
                  re->rects = _merge_rects(re->merge_mode, re->tb, re->rects_prev[0], re->rects_prev[1], re->rects_prev[2], re->rects_prev[3]);
                  break;
                default:
                  break;
               }
             first_rect = EINA_TRUE;
          }
        evas_common_tilebuf_clear(re->tb);
        re->cur_rect = EINA_INLIST_GET(re->rects);
     }
   if (!re->cur_rect) return NULL;
   rect = (Tilebuf_Rect *)re->cur_rect;
   if (re->rects)
     {
        switch (re->swap_mode)
          {
           case MODE_COPY:
           case MODE_DOUBLE:
           case MODE_TRIPLE:
           case MODE_QUADRUPLE:
             rect = (Tilebuf_Rect *)re->cur_rect;
             *x = rect->x;
             *y = rect->y;
             *w = rect->w;
             *h = rect->h;
             *cx = rect->x;
             *cy = rect->y;
             *cw = rect->w;
             *ch = rect->h;
             re->cur_rect = re->cur_rect->next;
             break;
           case MODE_FULL:
             re->cur_rect = NULL;
             if (x) *x = 0;
             if (y) *y = 0;
             if (w) *w = re->w;
             if (h) *h = re->h;
             if (cx) *cx = 0;
             if (cy) *cy = 0;
             if (cw) *cw = re->w;
             if (ch) *ch = re->h;
             break;
           default:
             break;
          }
        if (first_rect)
          {
             // do anything needed fir the first frame
          }
        surface = re->outbuf_new_region_for_update(re->ob,
                                                   *x, *y, *w, *h,
                                                   cx, cy, cw, ch);
        if (!re->cur_rect)
          {
             re->end = 1;
          }
        return surface;
     }
   return NULL;
}

static void
eng_output_redraws_next_update_push(void *data, void *surface, int x, int y, int w, int h, Evas_Render_Mode render_mode)
{
   Render_Engine_Software_Generic *re;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   re = (Render_Engine_Software_Generic *)data;
#if defined(BUILD_PIPE_RENDER)
   evas_common_pipe_map_begin(surface);
#endif /* BUILD_PIPE_RENDER */
   re->outbuf_push_updated_region(re->ob, surface, x, y, w, h);
   re->outbuf_free_region_for_update(re->ob, surface);
   evas_common_cpu_end_opt();
}

static void
eng_output_flush(void *data, Evas_Render_Mode render_mode)
{
   Render_Engine_Software_Generic *re;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   re = (Render_Engine_Software_Generic *)data;
   if (re->outbuf_flush) re->outbuf_flush(re->ob);
   if (re->rects)
     {
        evas_common_tilebuf_free_render_rects(re->rects);
        re->rects = NULL;
     }
}

static void
eng_output_idle_flush(void *data)
{
   Render_Engine_Software_Generic *re;

   re = (Render_Engine_Software_Generic *)data;
   if (re->outbuf_idle_flush) re->outbuf_idle_flush(re->ob);
}


//------------------------------------------------//

/*
 *****
 **
 ** ENGINE API
 **
 *****
 */

static Evas_Func func =
{
   NULL,
     NULL,
     NULL,
     NULL,
     eng_output_resize,
     eng_output_tile_size_set,
     eng_output_redraws_rect_add,
     eng_output_redraws_rect_del,
     eng_output_redraws_clear,
     eng_output_redraws_next_update_get,
     eng_output_redraws_next_update_push,
     eng_output_flush,
     eng_output_idle_flush,
     eng_output_dump,
     /* draw context virtual methods */
     eng_context_new,
     eng_canvas_alpha_get,
     eng_context_free,
     eng_context_clip_set,
     eng_context_clip_clip,
     eng_context_clip_unset,
     eng_context_clip_get,
     eng_context_color_set,
     eng_context_color_get,
     eng_context_multiplier_set,
     eng_context_multiplier_unset,
     eng_context_multiplier_get,
     eng_context_cutout_add,
     eng_context_cutout_clear,
     eng_context_anti_alias_set,
     eng_context_anti_alias_get,
     eng_context_color_interpolation_set,
     eng_context_color_interpolation_get,
     eng_context_render_op_set,
     eng_context_render_op_get,
     /* rect draw funcs */
     eng_rectangle_draw,
     /* line draw funcs */
     eng_line_draw,
     /* polygon draw funcs */
     eng_polygon_point_add,
     eng_polygon_points_clear,
     eng_polygon_draw,
     /* image draw funcs */
     eng_image_load,
     eng_image_mmap,
     eng_image_new_from_data,
     eng_image_new_from_copied_data,
     eng_image_free,
     eng_image_size_get,
     eng_image_size_set,
     NULL,
     eng_image_dirty_region,
     eng_image_data_get,
     eng_image_data_put,
     eng_image_data_preload_request,
     eng_image_data_preload_cancel,
     eng_image_alpha_set,
     eng_image_alpha_get,
     eng_image_border_set,
     eng_image_border_get,
     eng_image_draw,
     eng_image_comment_get,
     eng_image_format_get,
     eng_image_colorspace_set,
     eng_image_colorspace_get,
     eng_image_can_region_get,
     eng_image_native_set,
     eng_image_native_get,
     /* image cache funcs */
     eng_image_cache_flush,
     eng_image_cache_set,
     eng_image_cache_get,
     /* font draw functions */
     eng_font_load,
     eng_font_memory_load,
     eng_font_add,
     eng_font_memory_add,
     eng_font_free,
     eng_font_ascent_get,
     eng_font_descent_get,
     eng_font_max_ascent_get,
     eng_font_max_descent_get,
     eng_font_string_size_get,
     eng_font_inset_get,
     eng_font_h_advance_get,
     eng_font_v_advance_get,
     eng_font_char_coords_get,
     eng_font_char_at_coords_get,
     eng_font_draw,
     /* font cache functions */
     eng_font_cache_flush,
     eng_font_cache_set,
     eng_font_cache_get,
     /* font hinting functions */
     eng_font_hinting_set,
     eng_font_hinting_can_hint,
     eng_image_scale_hint_set,
     eng_image_scale_hint_get,
     /* more font draw functions */
     eng_font_last_up_to_pos,
     eng_image_map_draw,
     eng_image_map_surface_new,
     eng_image_map_surface_free,
     eng_image_map_clean,
     NULL, // eng_image_content_hint_set - software doesn't use it
     NULL, // eng_image_content_hint_get - software doesn't use it
     eng_font_pen_coords_get,
     eng_font_text_props_info_create,
     eng_font_right_inset_get,
     NULL, // need software mesa for gl rendering <- gl_surface_create
     NULL, // need software mesa for gl rendering <- gl_surface_destroy
     NULL, // need software mesa for gl rendering <- gl_context_create
     NULL, // need software mesa for gl rendering <- gl_context_destroy
     NULL, // need software mesa for gl rendering <- gl_make_current
     NULL, // need software mesa for gl rendering <- gl_string_query
     NULL, // need software mesa for gl rendering <- gl_proc_address_get
     NULL, // need software mesa for gl rendering <- gl_native_surface_get
     NULL, // need software mesa for gl rendering <- gl_api_get
     NULL, // need software mesa for gl rendering <- gl_direct_override
     NULL, // need software mesa for gl rendering <- gl_get_pixels_set
     NULL, // need software mesa for gl rendering <- gl_surface_lock
     NULL, // need software mesa for gl rendering <- gl_surface_read_pixels
     NULL, // need software mesa for gl rendering <- gl_surface_unlock
     eng_image_load_error_get,
     eng_font_run_font_end_get,
     eng_image_animated_get,
     eng_image_animated_frame_count_get,
     eng_image_animated_loop_type_get,
     eng_image_animated_loop_count_get,
     eng_image_animated_frame_duration_get,
     eng_image_animated_frame_set,
     NULL,
     eng_multi_font_draw,
     eng_pixel_alpha_get,
     NULL, // eng_context_flush - software doesn't use it
     NULL, // eng_drawable_new
     NULL, // eng_drawable_free
     NULL, // eng_drawable_size_get
     NULL, // eng_image_drawable_set
     NULL, // eng_drawable_render_scene
     NULL, // eng_texture_new
     NULL, // eng_texture_free
     NULL, // eng_texture_data_set
     NULL, // eng_texture_file_set
     NULL, // eng_texture_color_format_get
     NULL, // eng_texture_size_get
     NULL, // eng_texture_wrap_set
     NULL, // eng_texture_wrap_get
     NULL, // eng_texture_filter_set
     NULL, // eng_texture_filter_get
     NULL, // eng_texture_image_set
   /* FUTURE software generic calls go here */
};


//----------------------------------------------------------------//
//                                                                //
//                      Load Symbols                              //
//                                                                //
//----------------------------------------------------------------//
#ifdef EVAS_GL
static void
sym_missing(void)
{
   ERR("GL symbols missing!\n");
}

static int
glue_sym_init(void)
{
   //------------------------------------------------//
   // Use eglGetProcAddress
#define FINDSYM(dst, sym, typ) \
   if (!dst) dst = (typeof(dst))dlsym(gl_lib_handle, sym); \
   if (!dst)  \
     { \
        ERR("Symbol not found %s\n", sym); \
        return 0; \
     }
#define FALLBAK(dst, typ) if (!dst) dst = (typeof(dst))sym_missing;

    //------------------------------------------------------//
   // OSMesa APIs...
   FINDSYM(_sym_OSMesaCreateContextExt, "OSMesaCreateContextExt", glsym_func_osm_ctx);
   FALLBAK(_sym_OSMesaCreateContextExt, glsym_func_void);

   FINDSYM(_sym_OSMesaDestroyContext, "OSMesaDestroyContext", glsym_func_void);
   FALLBAK(_sym_OSMesaDestroyContext, glsym_func_void);

   FINDSYM(_sym_OSMesaMakeCurrent, "OSMesaMakeCurrent", glsym_func_bool);
   FALLBAK(_sym_OSMesaMakeCurrent, glsym_func_void);

   FINDSYM(_sym_OSMesaPixelStore, "OSMesaPixelStore", glsym_func_void);
   FALLBAK(_sym_OSMesaPixelStore, glsym_func_void);

   FINDSYM(_sym_OSMesaGetProcAddress, "OSMesaGetProcAddress", glsym_func_eng_fn);
   FALLBAK(_sym_OSMesaGetProcAddress, glsym_func_void);

#undef FINDSYM
#undef FALLBAK

   return 1;
}

static int
gl_sym_init(void)
{
   //------------------------------------------------//
#define FINDSYM(dst, sym, typ) \
   if (!dst) dst = (typeof(dst))dlsym(gl_lib_handle, sym); \
   if (!dst) DBG("Symbol not found %s\n", sym);
#define FALLBAK(dst, typ) if (!dst) dst = (typeof(dst))sym_missing;

   //------------------------------------------------------//
   // GLES 2.0 APIs...
   FINDSYM(_sym_glActiveTexture, "glActiveTexture", glsym_func_void);
   FALLBAK(_sym_glActiveTexture, glsym_func_void);

   FINDSYM(_sym_glAttachShader, "glAttachShader", glsym_func_void);
   FALLBAK(_sym_glAttachShader, glsym_func_void);

   FINDSYM(_sym_glBindAttribLocation, "glBindAttribLocation", glsym_func_void);
   FALLBAK(_sym_glBindAttribLocation, glsym_func_void);

   FINDSYM(_sym_glBindBuffer, "glBindBuffer", glsym_func_void);
   FALLBAK(_sym_glBindBuffer, glsym_func_void);

   FINDSYM(_sym_glBindFramebuffer, "glBindFramebuffer", glsym_func_void);
   FALLBAK(_sym_glBindFramebuffer, glsym_func_void);

   FINDSYM(_sym_glBindRenderbuffer, "glBindRenderbuffer", glsym_func_void);
   FALLBAK(_sym_glBindRenderbuffer, glsym_func_void);

   FINDSYM(_sym_glBindTexture, "glBindTexture", glsym_func_void);
   FALLBAK(_sym_glBindTexture, glsym_func_void);

   FINDSYM(_sym_glBlendColor, "glBlendColor", glsym_func_void);
   FALLBAK(_sym_glBlendColor, glsym_func_void);

   FINDSYM(_sym_glBlendEquation, "glBlendEquation", glsym_func_void);
   FALLBAK(_sym_glBlendEquation, glsym_func_void);

   FINDSYM(_sym_glBlendEquationSeparate, "glBlendEquationSeparate", glsym_func_void);
   FALLBAK(_sym_glBlendEquationSeparate, glsym_func_void);

   FINDSYM(_sym_glBlendFunc, "glBlendFunc", glsym_func_void);
   FALLBAK(_sym_glBlendFunc, glsym_func_void);

   FINDSYM(_sym_glBlendFuncSeparate, "glBlendFuncSeparate", glsym_func_void);
   FALLBAK(_sym_glBlendFuncSeparate, glsym_func_void);

   FINDSYM(_sym_glBufferData, "glBufferData", glsym_func_void);
   FALLBAK(_sym_glBufferData, glsym_func_void);

   FINDSYM(_sym_glBufferSubData, "glBufferSubData", glsym_func_void);
   FALLBAK(_sym_glBufferSubData, glsym_func_void);

   FINDSYM(_sym_glCheckFramebufferStatus, "glCheckFramebufferStatus", glsym_func_uint);
   FALLBAK(_sym_glCheckFramebufferStatus, glsym_func_uint);

   FINDSYM(_sym_glClear, "glClear", glsym_func_void);
   FALLBAK(_sym_glClear, glsym_func_void);

   FINDSYM(_sym_glClearColor, "glClearColor", glsym_func_void);
   FALLBAK(_sym_glClearColor, glsym_func_void);

   FINDSYM(_sym_glClearDepthf, "glClearDepthf", glsym_func_void);
   FINDSYM(_sym_glClearDepthf, "glClearDepth", glsym_func_void);
   FALLBAK(_sym_glClearDepthf, glsym_func_void);

   FINDSYM(_sym_glClearStencil, "glClearStencil", glsym_func_void);
   FALLBAK(_sym_glClearStencil, glsym_func_void);

   FINDSYM(_sym_glColorMask, "glColorMask", glsym_func_void);
   FALLBAK(_sym_glColorMask, glsym_func_void);

   FINDSYM(_sym_glCompileShader, "glCompileShader", glsym_func_void);
   FALLBAK(_sym_glCompileShader, glsym_func_void);

   FINDSYM(_sym_glCompressedTexImage2D, "glCompressedTexImage2D", glsym_func_void);
   FALLBAK(_sym_glCompressedTexImage2D, glsym_func_void);

   FINDSYM(_sym_glCompressedTexSubImage2D, "glCompressedTexSubImage2D", glsym_func_void);
   FALLBAK(_sym_glCompressedTexSubImage2D, glsym_func_void);

   FINDSYM(_sym_glCopyTexImage2D, "glCopyTexImage2D", glsym_func_void);
   FALLBAK(_sym_glCopyTexImage2D, glsym_func_void);

   FINDSYM(_sym_glCopyTexSubImage2D, "glCopyTexSubImage2D", glsym_func_void);
   FALLBAK(_sym_glCopyTexSubImage2D, glsym_func_void);

   FINDSYM(_sym_glCreateProgram, "glCreateProgram", glsym_func_uint);
   FALLBAK(_sym_glCreateProgram, glsym_func_uint);

   FINDSYM(_sym_glCreateShader, "glCreateShader", glsym_func_uint);
   FALLBAK(_sym_glCreateShader, glsym_func_uint);

   FINDSYM(_sym_glCullFace, "glCullFace", glsym_func_void);
   FALLBAK(_sym_glCullFace, glsym_func_void);

   FINDSYM(_sym_glDeleteBuffers, "glDeleteBuffers", glsym_func_void);
   FALLBAK(_sym_glDeleteBuffers, glsym_func_void);

   FINDSYM(_sym_glDeleteFramebuffers, "glDeleteFramebuffers", glsym_func_void);
   FALLBAK(_sym_glDeleteFramebuffers, glsym_func_void);

   FINDSYM(_sym_glDeleteProgram, "glDeleteProgram", glsym_func_void);
   FALLBAK(_sym_glDeleteProgram, glsym_func_void);

   FINDSYM(_sym_glDeleteRenderbuffers, "glDeleteRenderbuffers", glsym_func_void);
   FALLBAK(_sym_glDeleteRenderbuffers, glsym_func_void);

   FINDSYM(_sym_glDeleteShader, "glDeleteShader", glsym_func_void);
   FALLBAK(_sym_glDeleteShader, glsym_func_void);

   FINDSYM(_sym_glDeleteTextures, "glDeleteTextures", glsym_func_void);
   FALLBAK(_sym_glDeleteTextures, glsym_func_void);

   FINDSYM(_sym_glDepthFunc, "glDepthFunc", glsym_func_void);
   FALLBAK(_sym_glDepthFunc, glsym_func_void);

   FINDSYM(_sym_glDepthMask, "glDepthMask", glsym_func_void);
   FALLBAK(_sym_glDepthMask, glsym_func_void);

   FINDSYM(_sym_glDepthRangef, "glDepthRangef", glsym_func_void);
   FINDSYM(_sym_glDepthRangef, "glDepthRange", glsym_func_void);
   FALLBAK(_sym_glDepthRangef, glsym_func_void);

   FINDSYM(_sym_glDetachShader, "glDetachShader", glsym_func_void);
   FALLBAK(_sym_glDetachShader, glsym_func_void);

   FINDSYM(_sym_glDisable, "glDisable", glsym_func_void);
   FALLBAK(_sym_glDisable, glsym_func_void);

   FINDSYM(_sym_glDisableVertexAttribArray, "glDisableVertexAttribArray", glsym_func_void);
   FALLBAK(_sym_glDisableVertexAttribArray, glsym_func_void);

   FINDSYM(_sym_glDrawArrays, "glDrawArrays", glsym_func_void);
   FALLBAK(_sym_glDrawArrays, glsym_func_void);

   FINDSYM(_sym_glDrawElements, "glDrawElements", glsym_func_void);
   FALLBAK(_sym_glDrawElements, glsym_func_void);

   FINDSYM(_sym_glEnable, "glEnable", glsym_func_void);
   FALLBAK(_sym_glEnable, glsym_func_void);

   FINDSYM(_sym_glEnableVertexAttribArray, "glEnableVertexAttribArray", glsym_func_void);
   FALLBAK(_sym_glEnableVertexAttribArray, glsym_func_void);

   FINDSYM(_sym_glFinish, "glFinish", glsym_func_void);
   FALLBAK(_sym_glFinish, glsym_func_void);

   FINDSYM(_sym_glFlush, "glFlush", glsym_func_void);
   FALLBAK(_sym_glFlush, glsym_func_void);

   FINDSYM(_sym_glFramebufferRenderbuffer, "glFramebufferRenderbuffer", glsym_func_void);
   FALLBAK(_sym_glFramebufferRenderbuffer, glsym_func_void);

   FINDSYM(_sym_glFramebufferTexture2D, "glFramebufferTexture2D", glsym_func_void);
   FALLBAK(_sym_glFramebufferTexture2D, glsym_func_void);

   FINDSYM(_sym_glFrontFace, "glFrontFace", glsym_func_void);
   FALLBAK(_sym_glFrontFace, glsym_func_void);

   FINDSYM(_sym_glGenBuffers, "glGenBuffers", glsym_func_void);
   FALLBAK(_sym_glGenBuffers, glsym_func_void);

   FINDSYM(_sym_glGenerateMipmap, "glGenerateMipmap", glsym_func_void);
   FALLBAK(_sym_glGenerateMipmap, glsym_func_void);

   FINDSYM(_sym_glGenFramebuffers, "glGenFramebuffers", glsym_func_void);
   FALLBAK(_sym_glGenFramebuffers, glsym_func_void);

   FINDSYM(_sym_glGenRenderbuffers, "glGenRenderbuffers", glsym_func_void);
   FALLBAK(_sym_glGenRenderbuffers, glsym_func_void);

   FINDSYM(_sym_glGenTextures, "glGenTextures", glsym_func_void);
   FALLBAK(_sym_glGenTextures, glsym_func_void);

   FINDSYM(_sym_glGetActiveAttrib, "glGetActiveAttrib", glsym_func_void);
   FALLBAK(_sym_glGetActiveAttrib, glsym_func_void);

   FINDSYM(_sym_glGetActiveUniform, "glGetActiveUniform", glsym_func_void);
   FALLBAK(_sym_glGetActiveUniform, glsym_func_void);

   FINDSYM(_sym_glGetAttachedShaders, "glGetAttachedShaders", glsym_func_void);
   FALLBAK(_sym_glGetAttachedShaders, glsym_func_void);

   FINDSYM(_sym_glGetAttribLocation, "glGetAttribLocation", glsym_func_int);
   FALLBAK(_sym_glGetAttribLocation, glsym_func_int);

   FINDSYM(_sym_glGetBooleanv, "glGetBooleanv", glsym_func_void);
   FALLBAK(_sym_glGetBooleanv, glsym_func_void);

   FINDSYM(_sym_glGetBufferParameteriv, "glGetBufferParameteriv", glsym_func_void);
   FALLBAK(_sym_glGetBufferParameteriv, glsym_func_void);

   FINDSYM(_sym_glGetError, "glGetError", glsym_func_uint);
   FALLBAK(_sym_glGetError, glsym_func_uint);

   FINDSYM(_sym_glGetFloatv, "glGetFloatv", glsym_func_void);
   FALLBAK(_sym_glGetFloatv, glsym_func_void);

   FINDSYM(_sym_glGetFramebufferAttachmentParameteriv, "glGetFramebufferAttachmentParameteriv", glsym_func_void);
   FALLBAK(_sym_glGetFramebufferAttachmentParameteriv, glsym_func_void);

   FINDSYM(_sym_glGetIntegerv, "glGetIntegerv", glsym_func_void);
   FALLBAK(_sym_glGetIntegerv, glsym_func_void);

   FINDSYM(_sym_glGetProgramiv, "glGetProgramiv", glsym_func_void);
   FALLBAK(_sym_glGetProgramiv, glsym_func_void);

   FINDSYM(_sym_glGetProgramInfoLog, "glGetProgramInfoLog", glsym_func_void);
   FALLBAK(_sym_glGetProgramInfoLog, glsym_func_void);

   FINDSYM(_sym_glGetRenderbufferParameteriv, "glGetRenderbufferParameteriv", glsym_func_void);
   FALLBAK(_sym_glGetRenderbufferParameteriv, glsym_func_void);

   FINDSYM(_sym_glGetShaderiv, "glGetShaderiv", glsym_func_void);
   FALLBAK(_sym_glGetShaderiv, glsym_func_void);

   FINDSYM(_sym_glGetShaderInfoLog, "glGetShaderInfoLog", glsym_func_void);
   FALLBAK(_sym_glGetShaderInfoLog, glsym_func_void);

   FINDSYM(_sym_glGetShaderPrecisionFormat, "glGetShaderPrecisionFormat", glsym_func_void);
   FALLBAK(_sym_glGetShaderPrecisionFormat, glsym_func_void);

   FINDSYM(_sym_glGetShaderSource, "glGetShaderSource", glsym_func_void);
   FALLBAK(_sym_glGetShaderSource, glsym_func_void);

   FINDSYM(_sym_glGetString, "glGetString", glsym_func_uchar_ptr);
   FALLBAK(_sym_glGetString, glsym_func_const_uchar_ptr);

   FINDSYM(_sym_glGetTexParameterfv, "glGetTexParameterfv", glsym_func_void);
   FALLBAK(_sym_glGetTexParameterfv, glsym_func_void);

   FINDSYM(_sym_glGetTexParameteriv, "glGetTexParameteriv", glsym_func_void);
   FALLBAK(_sym_glGetTexParameteriv, glsym_func_void);

   FINDSYM(_sym_glGetUniformfv, "glGetUniformfv", glsym_func_void);
   FALLBAK(_sym_glGetUniformfv, glsym_func_void);

   FINDSYM(_sym_glGetUniformiv, "glGetUniformiv", glsym_func_void);
   FALLBAK(_sym_glGetUniformiv, glsym_func_void);

   FINDSYM(_sym_glGetUniformLocation, "glGetUniformLocation", glsym_func_int);
   FALLBAK(_sym_glGetUniformLocation, glsym_func_int);

   FINDSYM(_sym_glGetVertexAttribfv, "glGetVertexAttribfv", glsym_func_void);
   FALLBAK(_sym_glGetVertexAttribfv, glsym_func_void);

   FINDSYM(_sym_glGetVertexAttribiv, "glGetVertexAttribiv", glsym_func_void);
   FALLBAK(_sym_glGetVertexAttribiv, glsym_func_void);

   FINDSYM(_sym_glGetVertexAttribPointerv, "glGetVertexAttribPointerv", glsym_func_void);
   FALLBAK(_sym_glGetVertexAttribPointerv, glsym_func_void);

   FINDSYM(_sym_glHint, "glHint", glsym_func_void);
   FALLBAK(_sym_glHint, glsym_func_void);

   FINDSYM(_sym_glIsBuffer, "glIsBuffer", glsym_func_uchar);
   FALLBAK(_sym_glIsBuffer, glsym_func_uchar);

   FINDSYM(_sym_glIsEnabled, "glIsEnabled", glsym_func_uchar);
   FALLBAK(_sym_glIsEnabled, glsym_func_uchar);

   FINDSYM(_sym_glIsFramebuffer, "glIsFramebuffer", glsym_func_uchar);
   FALLBAK(_sym_glIsFramebuffer, glsym_func_uchar);

   FINDSYM(_sym_glIsProgram, "glIsProgram", glsym_func_uchar);
   FALLBAK(_sym_glIsProgram, glsym_func_uchar);

   FINDSYM(_sym_glIsRenderbuffer, "glIsRenderbuffer", glsym_func_uchar);
   FALLBAK(_sym_glIsRenderbuffer, glsym_func_uchar);

   FINDSYM(_sym_glIsShader, "glIsShader", glsym_func_uchar);
   FALLBAK(_sym_glIsShader, glsym_func_uchar);

   FINDSYM(_sym_glIsTexture, "glIsTexture", glsym_func_uchar);
   FALLBAK(_sym_glIsTexture, glsym_func_uchar);

   FINDSYM(_sym_glLineWidth, "glLineWidth", glsym_func_void);
   FALLBAK(_sym_glLineWidth, glsym_func_void);

   FINDSYM(_sym_glLinkProgram, "glLinkProgram", glsym_func_void);
   FALLBAK(_sym_glLinkProgram, glsym_func_void);

   FINDSYM(_sym_glPixelStorei, "glPixelStorei", glsym_func_void);
   FALLBAK(_sym_glPixelStorei, glsym_func_void);

   FINDSYM(_sym_glPolygonOffset, "glPolygonOffset", glsym_func_void);
   FALLBAK(_sym_glPolygonOffset, glsym_func_void);

   FINDSYM(_sym_glReadPixels, "glReadPixels", glsym_func_void);
   FALLBAK(_sym_glReadPixels, glsym_func_void);

   FINDSYM(_sym_glReleaseShaderCompiler, "glReleaseShaderCompiler", glsym_func_void);
   FALLBAK(_sym_glReleaseShaderCompiler, glsym_func_void);

   FINDSYM(_sym_glRenderbufferStorage, "glRenderbufferStorage", glsym_func_void);
   FALLBAK(_sym_glRenderbufferStorage, glsym_func_void);

   FINDSYM(_sym_glSampleCoverage, "glSampleCoverage", glsym_func_void);
   FALLBAK(_sym_glSampleCoverage, glsym_func_void);

   FINDSYM(_sym_glScissor, "glScissor", glsym_func_void);
   FALLBAK(_sym_glScissor, glsym_func_void);

   FINDSYM(_sym_glShaderBinary, "glShaderBinary", glsym_func_void);
   FALLBAK(_sym_glShaderBinary, glsym_func_void);

   FINDSYM(_sym_glShaderSource, "glShaderSource", glsym_func_void);
   FALLBAK(_sym_glShaderSource, glsym_func_void);

   FINDSYM(_sym_glStencilFunc, "glStencilFunc", glsym_func_void);
   FALLBAK(_sym_glStencilFunc, glsym_func_void);

   FINDSYM(_sym_glStencilFuncSeparate, "glStencilFuncSeparate", glsym_func_void);
   FALLBAK(_sym_glStencilFuncSeparate, glsym_func_void);

   FINDSYM(_sym_glStencilMask, "glStencilMask", glsym_func_void);
   FALLBAK(_sym_glStencilMask, glsym_func_void);

   FINDSYM(_sym_glStencilMaskSeparate, "glStencilMaskSeparate", glsym_func_void);
   FALLBAK(_sym_glStencilMaskSeparate, glsym_func_void);

   FINDSYM(_sym_glStencilOp, "glStencilOp", glsym_func_void);
   FALLBAK(_sym_glStencilOp, glsym_func_void);

   FINDSYM(_sym_glStencilOpSeparate, "glStencilOpSeparate", glsym_func_void);
   FALLBAK(_sym_glStencilOpSeparate, glsym_func_void);

   FINDSYM(_sym_glTexImage2D, "glTexImage2D", glsym_func_void);
   FALLBAK(_sym_glTexImage2D, glsym_func_void);

   FINDSYM(_sym_glTexParameterf, "glTexParameterf", glsym_func_void);
   FALLBAK(_sym_glTexParameterf, glsym_func_void);

   FINDSYM(_sym_glTexParameterfv, "glTexParameterfv", glsym_func_void);
   FALLBAK(_sym_glTexParameterfv, glsym_func_void);

   FINDSYM(_sym_glTexParameteri, "glTexParameteri", glsym_func_void);
   FALLBAK(_sym_glTexParameteri, glsym_func_void);

   FINDSYM(_sym_glTexParameteriv, "glTexParameteriv", glsym_func_void);
   FALLBAK(_sym_glTexParameteriv, glsym_func_void);

   FINDSYM(_sym_glTexSubImage2D, "glTexSubImage2D", glsym_func_void);
   FALLBAK(_sym_glTexSubImage2D, glsym_func_void);

   FINDSYM(_sym_glUniform1f, "glUniform1f", glsym_func_void);
   FALLBAK(_sym_glUniform1f, glsym_func_void);

   FINDSYM(_sym_glUniform1fv, "glUniform1fv", glsym_func_void);
   FALLBAK(_sym_glUniform1fv, glsym_func_void);

   FINDSYM(_sym_glUniform1i, "glUniform1i", glsym_func_void);
   FALLBAK(_sym_glUniform1i, glsym_func_void);

   FINDSYM(_sym_glUniform1iv, "glUniform1iv", glsym_func_void);
   FALLBAK(_sym_glUniform1iv, glsym_func_void);

   FINDSYM(_sym_glUniform2f, "glUniform2f", glsym_func_void);
   FALLBAK(_sym_glUniform2f, glsym_func_void);

   FINDSYM(_sym_glUniform2fv, "glUniform2fv", glsym_func_void);
   FALLBAK(_sym_glUniform2fv, glsym_func_void);

   FINDSYM(_sym_glUniform2i, "glUniform2i", glsym_func_void);
   FALLBAK(_sym_glUniform2i, glsym_func_void);

   FINDSYM(_sym_glUniform2iv, "glUniform2iv", glsym_func_void);
   FALLBAK(_sym_glUniform2iv, glsym_func_void);

   FINDSYM(_sym_glUniform3f, "glUniform3f", glsym_func_void);
   FALLBAK(_sym_glUniform3f, glsym_func_void);

   FINDSYM(_sym_glUniform3fv, "glUniform3fv", glsym_func_void);
   FALLBAK(_sym_glUniform3fv, glsym_func_void);

   FINDSYM(_sym_glUniform3i, "glUniform3i", glsym_func_void);
   FALLBAK(_sym_glUniform3i, glsym_func_void);

   FINDSYM(_sym_glUniform3iv, "glUniform3iv", glsym_func_void);
   FALLBAK(_sym_glUniform3iv, glsym_func_void);

   FINDSYM(_sym_glUniform4f, "glUniform4f", glsym_func_void);
   FALLBAK(_sym_glUniform4f, glsym_func_void);

   FINDSYM(_sym_glUniform4fv, "glUniform4fv", glsym_func_void);
   FALLBAK(_sym_glUniform4fv, glsym_func_void);

   FINDSYM(_sym_glUniform4i, "glUniform4i", glsym_func_void);
   FALLBAK(_sym_glUniform4i, glsym_func_void);

   FINDSYM(_sym_glUniform4iv, "glUniform4iv", glsym_func_void);
   FALLBAK(_sym_glUniform4iv, glsym_func_void);

   FINDSYM(_sym_glUniformMatrix2fv, "glUniformMatrix2fv", glsym_func_void);
   FALLBAK(_sym_glUniformMatrix2fv, glsym_func_void);

   FINDSYM(_sym_glUniformMatrix3fv, "glUniformMatrix3fv", glsym_func_void);
   FALLBAK(_sym_glUniformMatrix3fv, glsym_func_void);

   FINDSYM(_sym_glUniformMatrix4fv, "glUniformMatrix4fv", glsym_func_void);
   FALLBAK(_sym_glUniformMatrix4fv, glsym_func_void);

   FINDSYM(_sym_glUseProgram, "glUseProgram", glsym_func_void);
   FALLBAK(_sym_glUseProgram, glsym_func_void);

   FINDSYM(_sym_glValidateProgram, "glValidateProgram", glsym_func_void);
   FALLBAK(_sym_glValidateProgram, glsym_func_void);

   FINDSYM(_sym_glVertexAttrib1f, "glVertexAttrib1f", glsym_func_void);
   FALLBAK(_sym_glVertexAttrib1f, glsym_func_void);

   FINDSYM(_sym_glVertexAttrib1fv, "glVertexAttrib1fv", glsym_func_void);
   FALLBAK(_sym_glVertexAttrib1fv, glsym_func_void);

   FINDSYM(_sym_glVertexAttrib2f, "glVertexAttrib2f", glsym_func_void);
   FALLBAK(_sym_glVertexAttrib2f, glsym_func_void);

   FINDSYM(_sym_glVertexAttrib2fv, "glVertexAttrib2fv", glsym_func_void);
   FALLBAK(_sym_glVertexAttrib2fv, glsym_func_void);

   FINDSYM(_sym_glVertexAttrib3f, "glVertexAttrib3f", glsym_func_void);
   FALLBAK(_sym_glVertexAttrib3f, glsym_func_void);

   FINDSYM(_sym_glVertexAttrib3fv, "glVertexAttrib3fv", glsym_func_void);
   FALLBAK(_sym_glVertexAttrib3fv, glsym_func_void);

   FINDSYM(_sym_glVertexAttrib4f, "glVertexAttrib4f", glsym_func_void);
   FALLBAK(_sym_glVertexAttrib4f, glsym_func_void);

   FINDSYM(_sym_glVertexAttrib4fv, "glVertexAttrib4fv", glsym_func_void);
   FALLBAK(_sym_glVertexAttrib4fv, glsym_func_void);

   FINDSYM(_sym_glVertexAttribPointer, "glVertexAttribPointer", glsym_func_void);
   FALLBAK(_sym_glVertexAttribPointer, glsym_func_void);

   FINDSYM(_sym_glViewport, "glViewport", glsym_func_void);
   FALLBAK(_sym_glViewport, glsym_func_void);

#undef FINDSYM
#undef FALLBAK

   // Checking to see if this function exists is a poor but reasonable way to 
   // check if it's gles but it works for now
   if (_sym_glGetShaderPrecisionFormat != (typeof(_sym_glGetShaderPrecisionFormat))sym_missing ) 
     {
        DBG("GL Library is GLES.");
        gl_lib_is_gles = 1;
     }

   return 1;
}

//--------------------------------------------------------------//
// Wrapped GL APIs to handle desktop compatibility

// Stripping precision code from GLES shader for desktop compatibility
// Code adopted from Meego GL code. Temporary Fix.  
static const char *
opengl_strtok(const char *s, int *n, char **saveptr, char *prevbuf)
{
   char *start;
   char *ret;
   char *p;
   int retlen;
   static const char *delim = " \t\n\r/";

   if (prevbuf) free(prevbuf);

   if (s) 
      *saveptr = (char *)s;
   else 
     {
        if (!(*saveptr) || !(*n))
           return NULL;
        s = *saveptr;
     }

   for (; *n && strchr(delim, *s); s++, (*n)--) 
     {
        if (*s == '/' && *n > 1) 
          {
             if (s[1] == '/') 
               {
                  do 
                    {
                       s++, (*n)--;
                    } 
                  while (*n > 1 && s[1] != '\n' && s[1] != '\r');
               } 
             else if (s[1] == '*') 
               {
                  do 
                    {
                       s++, (*n)--;
                    } 
                  while (*n > 2 && (s[1] != '*' || s[2] != '/'));
                  s++, (*n)--;
               }
          }
     }

   start = (char *)s;
   for (; *n && *s && !strchr(delim, *s); s++, (*n)--);
   if (*n > 0) s++, (*n)--;

   *saveptr = (char *)s;

   retlen = s - start;
   ret = malloc(retlen + 1);
   p = ret;

   while (retlen > 0) 
     {
        if (*start == '/' && retlen > 1) 
          {
             if (start[1] == '/') 
               {
                  do 
                    {
                       start++, retlen--;
                    } 
                  while (retlen > 1 && start[1] != '\n' && start[1] != '\r');
                  start++, retlen--;
                  continue;
               } 
             else if (start[1] == '*') 
               {
                  do 
                    {
                       start++, retlen--;
                    } 
                  while (retlen > 2 && (start[1] != '*' || start[2] != '/'));
                  start += 3, retlen -= 3;
                  continue;
               }
          }
        *(p++) = *(start++), retlen--;
     }

   *p = 0;
   return ret;
}	

static char *
patch_gles_shader(const char *source, int length, int *patched_len)
{
   char *saveptr = NULL;
   char *sp;
   char *p = NULL;

   if (!length) length = strlen(source);

   *patched_len = 0;
   int patched_size = length;
   char *patched = malloc(patched_size + 1);

   if (!patched) return NULL;

   p = (char *)opengl_strtok(source, &length, &saveptr, NULL);
   for (; p; p = (char *)opengl_strtok(0, &length, &saveptr, p)) 
     {
        if (!strncmp(p, "lowp", 4) || !strncmp(p, "mediump", 7) || !strncmp(p, "highp", 5)) 
          {
             continue;
          } 
        else if (!strncmp(p, "precision", 9)) 
          {
             while ((p = (char *)opengl_strtok(0, &length, &saveptr, p)) && !strchr(p, ';'));
          } 
        else 
          {
             if (!strncmp(p, "gl_MaxVertexUniformVectors", 26)) 
               {
                  free(p); 
                  p = strdup("(gl_MaxVertexUniformComponents / 4)");
               } 
             else if (!strncmp(p, "gl_MaxFragmentUniformVectors", 28)) 
               {
                  free(p);
                  p = strdup("(gl_MaxFragmentUniformComponents / 4)");
               } 
             else if (!strncmp(p, "gl_MaxVaryingVectors", 20)) 
               {
                  free(p);
                  p = strdup("(gl_MaxVaryingFloats / 4)");
               }

             int new_len = strlen(p);
             if (*patched_len + new_len > patched_size) 
               {
                  char *tmp;

                  patched_size *= 2;
                  tmp = realloc(patched, patched_size + 1);
                  if (!tmp)
                    {
                       free(patched);
                       free(p);
                       return NULL;
                    }
                  patched = tmp;
               }

             memcpy(patched + *patched_len, p, new_len);
             *patched_len += new_len;
          }     
     }

   patched[*patched_len] = 0;
   /* check that we don't leave dummy preprocessor lines */
   for (sp = patched; *sp;) 
     {
        for (; *sp == ' ' || *sp == '\t'; sp++);
        if (!strncmp(sp, "#define", 7)) 
          {
             for (p = sp + 7; *p == ' ' || *p == '\t'; p++);
             if (*p == '\n' || *p == '\r' || *p == '/') 
               {
                  memset(sp, 0x20, 7);
               }
          }
        for (; *sp && *sp != '\n' && *sp != '\r'; sp++);
        for (; *sp == '\n' || *sp == '\r'; sp++);
     }
   return patched;
}

static void
evgl_glShaderSource(GLuint shader, GLsizei count, const char* const* string, const GLint* length)
{
   int i = 0, len = 0;

   char **s = malloc(count * sizeof(char*));
   GLint *l = malloc(count * sizeof(GLint));

   memset(s, 0, count * sizeof(char*));
   memset(l, 0, count * sizeof(GLint));

   for (i = 0; i < count; ++i) 
     {
        if (length) 
          {
             len = length[i];
             if (len < 0) 
                len = string[i] ? strlen(string[i]) : 0;
          }
        else
           len = string[i] ? strlen(string[i]) : 0;

        if (string[i]) 
          {
             s[i] = patch_gles_shader(string[i], len, &l[i]);
             if (!s[i]) 
               {
                  while(i)
                     free(s[--i]);
                  free(l);
                  free(s);

                  DBG("Patching Shader Failed.");
                  return;
               }
          } 
        else 
          {
             s[i] = NULL;
             l[i] = 0;
          }
     }

   _sym_glShaderSource(shader, count, (const char * const *)s, l);

   while(i)
      free(s[--i]);
   free(l);
   free(s);
}


static void
evgl_glGetShaderPrecisionFormat(GLenum shadertype EINA_UNUSED, GLenum precisiontype EINA_UNUSED, GLint* range, GLint* precision)
{
   if (range)
     {
        range[0] = -126; // floor(log2(FLT_MIN))
        range[1] = 127; // floor(log2(FLT_MAX))
     }
   if (precision)
     {
        precision[0] = 24; // floor(-log2((1.0/16777218.0)));
     }
   return;
}

static void
evgl_glReleaseShaderCompiler(void)
{
   DBG("Not supported in Desktop GL");
   return;
}

static void
evgl_glShaderBinary(GLsizei n EINA_UNUSED, const GLuint* shaders EINA_UNUSED, GLenum binaryformat EINA_UNUSED, const void* binary EINA_UNUSED, GLsizei length EINA_UNUSED)
{
   // FIXME: need to dlsym/getprocaddress for this
   DBG("Not supported in Desktop GL");
   return;
   //n = binaryformat = length = 0;
   //shaders = binary = 0;
}
#endif
//--------------------------------------------------------------//


#ifdef EVAS_GL
static void
override_gl_apis(Evas_GL_API *api)
{

   api->version = EVAS_GL_API_VERSION;

#define ORD(f) EVAS_API_OVERRIDE(f, api, _sym_)
   // GLES 2.0
   ORD(glActiveTexture);
   ORD(glAttachShader);
   ORD(glBindAttribLocation);
   ORD(glBindBuffer);
   ORD(glBindFramebuffer);
   ORD(glBindRenderbuffer);
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
   ORD(glClearDepthf);     
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
   ORD(glDepthRangef);     
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
   ORD(glGetShaderPrecisionFormat);  
   ORD(glGetShaderSource);
   ORD(glGetString);             // FIXME
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
   ORD(glReleaseShaderCompiler); 
   ORD(glRenderbufferStorage);
   ORD(glSampleCoverage);
   ORD(glScissor);
   ORD(glShaderBinary); 
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

#define ORD(f) EVAS_API_OVERRIDE(f, &gl_funcs, evgl_)
   if (!gl_lib_is_gles)
     {
        // Override functions wrapped by Evas_GL
        // GLES2.0 API compat on top of desktop gl
        ORD(glGetShaderPrecisionFormat);
        ORD(glReleaseShaderCompiler);
        ORD(glShaderBinary);
     }

   ORD(glShaderSource);    // Do precision stripping in both cases
#undef ORD
}
#endif

//-------------------------------------------//
static int
gl_lib_init(void)
{
#ifdef EVAS_GL
   // dlopen OSMesa 
   gl_lib_handle = dlopen("libOSMesa.so.9", RTLD_NOW);
   if (!gl_lib_handle) gl_lib_handle = dlopen("libOSMesa.so.8", RTLD_NOW);
   if (!gl_lib_handle) gl_lib_handle = dlopen("libOSMesa.so.7", RTLD_NOW);
   if (!gl_lib_handle) gl_lib_handle = dlopen("libOSMesa.so.6", RTLD_NOW);
   if (!gl_lib_handle) gl_lib_handle = dlopen("libOSMesa.so.5", RTLD_NOW);
   if (!gl_lib_handle) gl_lib_handle = dlopen("libOSMesa.so.4", RTLD_NOW);
   if (!gl_lib_handle) gl_lib_handle = dlopen("libOSMesa.so.3", RTLD_NOW);
   if (!gl_lib_handle) gl_lib_handle = dlopen("libOSMesa.so.2", RTLD_NOW);
   if (!gl_lib_handle) gl_lib_handle = dlopen("libOSMesa.so.1", RTLD_NOW);
   if (!gl_lib_handle) gl_lib_handle = dlopen("libOSMesa.so", RTLD_NOW);
   if (!gl_lib_handle)
     {
        DBG("Unable to open libOSMesa:  %s", dlerror());
        return 0;
     }

   //------------------------------------------------//
   if (!glue_sym_init()) return 0;
   if (!gl_sym_init()) return 0;

   override_gl_apis(&gl_funcs);

   return 1;
#else
   return 0;
#endif
}


static void
init_gl(void)
{
   DBG("Initializing Software OpenGL APIs...\n");

   if (!gl_lib_init())
      DBG("Unable to support EvasGL in this engine module. Install OSMesa to get it running");
   else
     {
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
        ORD(gl_surface_create);
        ORD(gl_surface_destroy);
        ORD(gl_context_create);
        ORD(gl_context_destroy);
        ORD(gl_make_current);
        ORD(gl_string_query);           // FIXME: Need to implement
        ORD(gl_proc_address_get);       // FIXME: Need to implement
        ORD(gl_native_surface_get);
        ORD(gl_api_get);
#undef ORD
     }
}


/*
 *****
 **
 ** MODULE ACCESSIBLE API API
 **
 *****
 */

static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   _evas_soft_gen_log_dom = eina_log_domain_register
     ("evas-software_generic", EVAS_DEFAULT_LOG_COLOR);
   if(_evas_soft_gen_log_dom<0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   _mp_command_rect = eina_mempool_add("chained_mempool",
                                       "Evas_Thread_Command_Rect", NULL,
                                       sizeof (Evas_Thread_Command_Rect), 128);
   _mp_command_line = eina_mempool_add("chained_mempool",
                                       "Evas_Thread_Command_Line", NULL,
                                       sizeof (Evas_Thread_Command_Line), 32);
   _mp_command_polygon = eina_mempool_add("chained_mempool",
                                       "Evas_Thread_Command_Polygon", NULL,
                                       sizeof (Evas_Thread_Command_Polygon), 32);
   _mp_command_image = eina_mempool_add("chained_mempool",
                                       "Evas_Thread_Command_Image", NULL,
                                       sizeof (Evas_Thread_Command_Image), 128);
   _mp_command_font = eina_mempool_add("chained_mempool",
                                       "Evas_Thread_Command_Font", NULL,
                                       sizeof (Evas_Thread_Command_Font), 128);
   _mp_command_map = eina_mempool_add("chained_mempool",
                                       "Evas_Thread_Command_Map", NULL,
                                       sizeof (Evas_Thread_Command_Map), 64);
   _mp_command_multi_font =
     eina_mempool_add("chained_mempool", "Evas_Thread_Command_Multi_Font",
                      NULL, sizeof(Evas_Thread_Command_Multi_Font), 128);

   init_gl();
   evas_common_pipe_init();

   em->functions = (void *)(&func);
   cpunum = eina_cpu_count();
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   eina_mempool_del(_mp_command_rect);
   eina_mempool_del(_mp_command_line);
   eina_mempool_del(_mp_command_polygon);
   eina_mempool_del(_mp_command_image);
   eina_mempool_del(_mp_command_font);
   eina_mempool_del(_mp_command_map);
   eina_log_domain_unregister(_evas_soft_gen_log_dom);
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "software_generic",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, software_generic);

#ifndef EVAS_STATIC_BUILD_SOFTWARE_GENERIC
EVAS_EINA_MODULE_DEFINE(engine, software_generic);
#endif
