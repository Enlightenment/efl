#ifndef _EVAS_GL_H
#define _EVAS_GL_H

#include <Evas.h>
//#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Evas_GL Rendering GL on Evas
 *
 * Functions that are used to do OpenGL rendering on Evas. Evas allows you
 * to use OpenGL to render to specially set up image objects (which act as
 * render target surfaces).
 *
 * Below is an illustrative example of how to use OpenGL to render to an
 * object in Evas.
 *
 * @code
// Simple Evas_GL example
#include <Ecore_Evas.h>
#include <Ecore.h>
#include <Evas_GL.h>
#include <stdio.h>

// GL related data here..
typedef struct _GLData
{
   Evas_GL_Context *ctx;
   Evas_GL_Surface *sfc;
   Evas_GL_Config  *cfg;
   Evas_GL         *evasgl;
   Evas_GL_API     *glapi;
   GLuint           program;
   GLuint           vtx_shader;
   GLuint           fgmt_shader;
   Eina_Bool        initialized : 1;
} GLData;

// callbacks we want to handle deletion on the object and updates/draws
static void      on_del       (void *data, Evas *e, Evas_Object *obj, void *event_info);
static void      on_pixels    (void *data, Evas_Object *obj);
// demo - animator just to keep ticking over saying to draw the image
static Eina_Bool on_animate   (void *data);
// gl stuff
static int       init_shaders (GLData *gld);
static GLuint    load_shader  (GLData *gld, GLenum type, const char *shader_src);

int
main(int argc, char **argv)
{
   // a size by default
   int w = 256, h = 256;
   // some variables we will use
   Ecore_Evas  *ee;
   Evas *canvas;
   Evas_Object *r1;
   Evas_Native_Surface ns;
   GLData *gld = NULL;

   // regular low-leve EFL (ecore+ecore-evas) init. elm is simpler
   ecore_init();
   ecore_evas_init();
   ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 512, 512);
   ecore_evas_title_set(ee, "Ecore_Evas Template");
   canvas = ecore_evas_get(ee);

   // alloc a data struct to hold our relevant gl info in
   if (!(gld = calloc(1, sizeof(GLData)))) return 0;

   //-//-//-// THIS IS WHERE GL INIT STUFF HAPPENS (ALA EGL)
   //-//
   // get the evas gl handle for doing gl things
   gld->evasgl = evas_gl_new(canvas);
   gld->glapi = evas_gl_api_get(gld->evasgl);

   // Set a surface config
   gld->cfg = evas_gl_config_new();
   gld->cfg->color_format = EVAS_GL_RGBA_8888;
   //gld->cfg->depth_bits   = EVAS_GL_DEPTH_NONE;        // Othe config options
   //gld->cfg->stencil_bits = EVAS_GL_STENCIL_NONE;
   //gld->cfg->options_bits = EVAS_GL_OPTIONS_NONE;

   // create a surface and context
   gld->sfc = evas_gl_surface_create(gld->evasgl, gld->cfg, w, h);
   gld->ctx = evas_gl_context_create(gld->evasgl, NULL);
   //-//
   //-//-//-// END GL INIT BLOB

   // set up the image object. a filled one by default
   r1 = evas_object_image_filled_add(canvas);
   // attach important data we need to the object using key names. This just
   // avoids some global variables and means we can do nice cleanup. You can
   // avoid this if you are lazy
   evas_object_data_set(r1, "..gld", gld);
   // when the object is deleted - call the on_del callback. like the above,
   // this is just being clean
   evas_object_event_callback_add(r1, EVAS_CALLBACK_DEL, on_del, NULL);
   // set up an actual pixel size fot the buffer data. It may be different
   // to the output size. any windowing system has something like this, just
   // evas has 2 sizes, a pixel size and the output object size
   evas_object_image_size_set(r1, w, h);
   // set up the native surface info to use the context and surface created
   // above

   //-//-//-// THIS IS WHERE GL INIT STUFF HAPPENS (ALA EGL)
   //-//
   evas_gl_native_surface_get(gld->evasgl, gld->sfc, &ns);
   evas_object_image_native_surface_set(r1, &ns);
   evas_object_image_pixels_get_callback_set(r1, on_pixels, r1);
   //-//
   //-//-//-// END GL INIT BLOB

   // move the image object somewhere, resize it and show it. any windowing
   // system would need this kind of thing - place a child "window"
   evas_object_move(r1, 128, 128);
   evas_object_resize(r1, w, h);
   evas_object_show(r1);

   // animating - just a demo. as long as you trigger an update on the image
   // object via evas_object_image_pixels_dirty_set(). any display system,
   // mainloop siztem etc. will have something of this kind unless it's making
   // you spin infinitely yourself and invent your own animation mechanism
   //
   // NOTE: if you delete r1, this animator will keep running trying to access
   // r1 so you'd better delete this animator with ecore_animator_del() or
   // structure how you do animation differently. you can also attach it like
   // evasgl, sfc, etc. etc. if this animator is specific to this object
   // only and delete it in the del handler for the obj.
   ecore_animator_add(on_animate, r1);

   // finally show the window for the world to see. windowing system generic
   ecore_evas_show(ee);

   // begin the mainloop and tick over the animator, handle events etc.
   // also windowing system generic
   ecore_main_loop_begin();

   // standard EFL shutdown stuff - generic for most systems, EFL or not
   ecore_evas_shutdown();
   ecore_shutdown();
   return 0;
}

static void
on_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   // on delete of our object clean up some things that don't get auto
   // celeted for us as they are not intrinsically bound to the image
   // object as such (you could use the same context and surface across
   // multiple image objects and re-use the evasgl handle too multiple times.
   // here we bind them to 1 object only though by doing this.
   GLData *gld = evas_object_data_get(obj, "..gld");
   if (!gld) return;
   Evas_GL_API *gl = gld->glapi;

   evas_object_data_del(obj, "..gld");

   // Do a make_current before deleting all the GL stuff.
   evas_gl_make_current(gld->evasgl, gld->sfc, gld->ctx);
   gl->glDeleteShader(gld->vtx_shader);
   gl->glDeleteShader(gld->fgmt_shader);
   gl->glDeleteProgram(gld->program);

   evas_gl_surface_destroy(gld->evasgl, gld->sfc);
   evas_gl_context_destroy(gld->evasgl, gld->ctx);
   evas_gl_config_free(gld->cfg);
   evas_gl_free(gld->evasgl);
   free(gld);
}

static void
on_pixels(void *data, Evas_Object *obj)
{
   // get some variable we need from the object data keys
   GLData *gld = evas_object_data_get(obj, "..gld");
   if (!gld) return;
   Evas_GL_API *gl = gld->glapi;
   GLfloat vVertices[] =
     {
         0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f
     };
   int w, h;

   // get the image size in case it changed with evas_object_image_size_set()
   evas_object_image_size_get(obj, &w, &h);
   // set up the context and surface as the current one
   evas_gl_make_current(gld->evasgl, gld->sfc, gld->ctx);

   if (!gld->initialized)
     {
        if (!init_shaders(gld)) printf("Error Initializing Shaders\n");
        gld->initialized = EINA_TRUE;
     }

   // GL Viewport stuff. you can avoid doing this if viewport is all the
   // same as last frame if you want
   gl->glViewport(0, 0, w, h);

   // Clear the buffer
   gl->glClearColor(0.0, 0.0, 1.0, 1);
   gl->glClear(GL_COLOR_BUFFER_BIT);

   // Draw a Triangle
   gl->glEnable(GL_BLEND);

   gl->glUseProgram(gld->program);

   gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vVertices);
   gl->glEnableVertexAttribArray(0);

   gl->glDrawArrays(GL_TRIANGLES, 0, 3);

   // Optional - Flush the GL pipeline
   gl->glFlush();
}

static Eina_Bool
on_animate(void *data)
{
   // just a demo - animate here whenever an animation tick happens and then
   // mark the image as "dirty" meaning it needs an update next time evas
   // renders. it will call the pixel get callback then.
   evas_object_image_pixels_dirty_set(data, EINA_TRUE);
   return EINA_TRUE; // keep looping
}

static GLuint
load_shader(GLData *gld, GLenum type, const char *shader_src)
{
   Evas_GL_API *gl = gld->glapi;
   GLuint shader;
   GLint compiled = 0;

   // Create the shader object
   if (!(shader = gl->glCreateShader(type))) return 0;
   gl->glShaderSource(shader, 1, &shader_src, NULL);
   // Compile the shader
   gl->glCompileShader(shader);
   gl->glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

   if (!compiled)
     {
        GLint len = 0;

        gl->glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 1)
          {
             char *info = malloc(sizeof(char) * len);

             if (info)
               {
                  gl->glGetShaderInfoLog(shader, len, NULL, info);
                  printf("Error compiling shader:\n"
                         "%s\n", info);
                  free(info);
               }
          }
        gl->glDeleteShader(shader);
        return 0;
     }
   return shader;
}

// Initialize the shader and program object
static int
init_shaders(GLData *gld)
{
   Evas_GL_API *gl = gld->glapi;
   const char vShaderStr[] =
      "attribute vec4 vPosition;    \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = vPosition;  \n"
      "}                            \n";
   const char fShaderStr[] =
      "precision mediump float;                    \n"
      "void main()                                 \n"
      "{                                           \n"
      "  gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );\n"
      "}                                           \n";
   GLint linked = 0;

   // Load the vertex/fragment shaders
   gld->vtx_shader  = load_shader(gld, GL_VERTEX_SHADER, vShaderStr);
   gld->fgmt_shader = load_shader(gld, GL_FRAGMENT_SHADER, fShaderStr);

   // Create the program object
   if (!(gld->program = gl->glCreateProgram())) return 0;

   gl->glAttachShader(gld->program, gld->vtx_shader);
   gl->glAttachShader(gld->program, gld->fgmt_shader);

   // Bind vPosition to attribute 0
   gl->glBindAttribLocation(gld->program, 0, "vPosition");
   // Link the program
   gl->glLinkProgram(gld->program);
   gl->glGetProgramiv(gld->program, GL_LINK_STATUS, &linked);

   if (!linked)
     {
        GLint len = 0;

        gl->glGetProgramiv(gld->program, GL_INFO_LOG_LENGTH, &len);
        if (len > 1)
          {
             char *info = malloc(sizeof(char) * len);

             if (info)
               {
                  gl->glGetProgramInfoLog(gld->program, len, NULL, info);
                  printf("Error linking program:\n"
                         "%s\n", info);
                  free(info);
               }
          }
        gl->glDeleteProgram(gld->program);
        return 0;
     }
   return 1;
}
 * @endcode
 *
 * @ingroup Evas_Canvas
 */

/**
 * @addtogroup Evas_GL
 * @{
 */

/**
  * @typedef Evas_GL
  *
  * Evas GL Object for rendering gl in Evas.
  */ 
typedef struct _Evas_GL               Evas_GL;

/**
  * @typedef Evas_GL_Surface
  *
  * Evas GL Surface object, a GL rendering target in Evas GL.
  */ 
typedef struct _Evas_GL_Surface       Evas_GL_Surface;

/**
  * @typedef Evas_GL_Context
  *
  * Evas GL Context object, a GL rendering context in Evas GL.
  */ 
typedef struct _Evas_GL_Context       Evas_GL_Context;

/**
  * @typedef Evas_GL_Config
  *
  * Evas GL Surface configuration object for surface creation.
  */ 
typedef struct _Evas_GL_Config        Evas_GL_Config;

/**
  * @typedef Evas_GL_API
  *
  * Evas GL API object that contains the GL APIs to be used in Evas GL.
  */ 
typedef struct _Evas_GL_API           Evas_GL_API;

/**
  * @typedef Evas_GL_Func
  *
  * Evas GL Function Object used as a function pointer.
  */ 
typedef void                         *Evas_GL_Func;

/**
  * @typedef EvasGLImage
  *
  * Evas GL Image Object used in Evas GL Image extension.
  */ 
typedef void                         *EvasGLImage;

/**
 * Surface Color Format
 */
typedef enum _Evas_GL_Color_Format
{
    EVAS_GL_RGB_888   = 0,
    EVAS_GL_RGBA_8888 = 1
} Evas_GL_Color_Format;

/**
 * Surface Depth Format
 */
typedef enum _Evas_GL_Depth_Bits
{
    EVAS_GL_DEPTH_NONE   = 0,
    EVAS_GL_DEPTH_BIT_8  = 1,
    EVAS_GL_DEPTH_BIT_16 = 2,
    EVAS_GL_DEPTH_BIT_24 = 3,
    EVAS_GL_DEPTH_BIT_32 = 4
} Evas_GL_Depth_Bits;

/**
 * Surface Stencil Format
 */
typedef enum _Evas_GL_Stencil_Bits
{
    EVAS_GL_STENCIL_NONE   = 0,
    EVAS_GL_STENCIL_BIT_1  = 1,
    EVAS_GL_STENCIL_BIT_2  = 2,
    EVAS_GL_STENCIL_BIT_4  = 3,
    EVAS_GL_STENCIL_BIT_8  = 4,
    EVAS_GL_STENCIL_BIT_16 = 5
} Evas_GL_Stencil_Bits;

/**
 * Configuration Options.
 *
 * @since 1.1
 */
typedef enum _Evas_GL_Options_Bits
{
   EVAS_GL_OPTIONS_NONE    = 0,     /**< No extra options */
   EVAS_GL_OPTIONS_DIRECT  = (1<<0) /**< Optional hint to allow rendering directly to evas' window when possible */
} Evas_GL_Options_Bits;

/**
 * Configuration Option for Multisample Anti-aliased (MSAA) rendering surface.
 * Only works in supported device.
 *
 * @since 1.2
 */
typedef enum _Evas_GL_Multisample_Bits
{
   EVAS_GL_MULTISAMPLE_NONE = 0, /**< No multisample rendering */
   EVAS_GL_MULTISAMPLE_LOW  = 1, /**< MSAA with mininum number of samples */
   EVAS_GL_MULTISAMPLE_MED  = 2, /**< MSAA with half the number of max samples */
   EVAS_GL_MULTISAMPLE_HIGH = 3  /**< MSAA with maximum allowed samples */
} Evas_GL_Multisample_Bits;

/**
  * @struct _Evas_GL_Config
  *
  * Evas GL Surface configuration
  */
struct _Evas_GL_Config
{
   Evas_GL_Color_Format       color_format;     /**< Surface Color Format */
   Evas_GL_Depth_Bits         depth_bits;       /**< Surface Depth Bits */
   Evas_GL_Stencil_Bits       stencil_bits;     /**< Surface Stencil Bits */
   Evas_GL_Options_Bits       options_bits;     /**< Extra Surface Options */
   Evas_GL_Multisample_Bits   multisample_bits; /**< Optional Surface MSAA Bits */
};

#define EVAS_GL_EXTENSIONS       1


/**
 * Creates a new Evas_GL object and returns a handle for gl rendering on efl.
 *
 * @param e The given evas canvas OpenGL is to be used on.
 * @return The created evas_gl object, or NULL on failure.
 */
EAPI Evas_GL                 *evas_gl_new                (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Frees the created Evas_GL object.
 *
 * @param evas_gl The given Evas_GL object.
 */
EAPI void                     evas_gl_free               (Evas_GL *evas_gl) EINA_ARG_NONNULL(1);

/**
 * Allocates a new config object for the user to fill out.
 *
 * As long as the Evas creates a config object for the user, it takes care
 * of the backward compatibility issue.
 */
EAPI Evas_GL_Config          *evas_gl_config_new         (void);

/**
 * Frees a config object created from evas_gl_config_new.
 *
 * As long as the Evas creates a config object for the user, it takes care
 * of the backward compatibility issue.
 */
EAPI void                     evas_gl_config_free        (Evas_GL_Config *cfg) EINA_ARG_NONNULL(1);

/**
 * Creates and returns new Evas_GL_Surface object for GL Rendering.
 *
 * @param evas_gl The given Evas_GL object.
 * @param cfg The pixel format and configuration of the rendering surface.
 * @param w The width of the surface.
 * @param h The height of the surface.
 * @return The created GL surface object, or NULL on failure.
 */
EAPI Evas_GL_Surface         *evas_gl_surface_create     (Evas_GL *evas_gl, Evas_GL_Config *cfg, int w, int h) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1,2);

/**
 * Destroys the created Evas GL Surface.
 *
 * @param evas_gl The given Evas_GL object.
 * @param surf The given GL surface object.
 */
EAPI void                     evas_gl_surface_destroy    (Evas_GL *evas_gl, Evas_GL_Surface *surf) EINA_ARG_NONNULL(1,2);

/**
 * Creates and returns a new Evas GL context object
 *
 * @param evas_gl The given Evas_GL object.
 * @return The created context, or NULL on failure.
 */
EAPI Evas_GL_Context         *evas_gl_context_create     (Evas_GL *evas_gl, Evas_GL_Context *share_ctx) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * Destroys the given Evas GL context object
 *
 * @param evas_gl The given Evas_GL object.
 * @param ctx The given Evas GL context.
 */
EAPI void                     evas_gl_context_destroy    (Evas_GL *evas_gl, Evas_GL_Context *ctx) EINA_ARG_NONNULL(1,2);

/**
 * Sets the given context as a current context for the given surface
 *
 * @param evas_gl The given Evas_GL object.
 * @param surf The given Evas GL surface.
 * @param ctx The given Evas GL context.
 * @return @c EINA_TRUE if successful, @c EINA_FALSE if not.
 */
EAPI Eina_Bool                evas_gl_make_current       (Evas_GL *evas_gl, Evas_GL_Surface *surf, Evas_GL_Context *ctx) EINA_ARG_NONNULL(1,2);

/**
 * Returns a pointer to a static, zero-terminated string describing some aspect of evas_gl.
 *
 * @param evas_gl The given Evas_GL object.
 * @param name Specifies a symbolic constant, one of EVAS_GL_EXTENSIONS...
 */
EAPI const char              *evas_gl_string_query       (Evas_GL *evas_gl, int name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * Returns a GL or the Glue Layer's extension function.
 *
 * @param evas_gl The given Evas_GL object.
 * @param name The name of the function to return.
 */
EAPI Evas_GL_Func             evas_gl_proc_address_get   (Evas_GL *evas_gl, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1,2) EINA_PURE;

/**
 * Fills in the Native Surface information from the given Evas GL surface.
 *
 * @param evas_gl The given Evas_GL object.
 * @param surf The given Evas GL surface to retrieve the Native Surface info from.
 * @param ns The native surface structure that the function fills in.
 * @return @c EINA_TRUE if successful, @c EINA_FALSE if not.
 */
EAPI Eina_Bool                evas_gl_native_surface_get (Evas_GL *evas_gl, Evas_GL_Surface *surf, Evas_Native_Surface *ns) EINA_ARG_NONNULL(1,2,3);

/**
 * Get the API for rendering using OpenGL
 *
 * @param evas_gl The given Eva_GL object.
 * @return The API to use.
 *
 * This returns a structure that contains all the OpenGL functions you can
 * use to render in Evas. These functions consist of all the standard
 * OpenGL-ES2.0 functions and any extra ones Evas has decided to provide in
 * addition. This means that if you have your code ported to OpenGL-ES2.0,
 * it will be easy to render to Evas.
 *
 */
EAPI Evas_GL_API             *evas_gl_api_get            (Evas_GL *evas_gl) EINA_ARG_NONNULL(1);

#if !defined(__gl_h_) && !defined(__gl2_h_)
# define __gl_h_
# define __gl2_h_

/*
 * This document is licensed under the SGI Free Software B License Version
 * 2.0. For details, see http://oss.sgi.com/projects/FreeB/ .
 */

/*-------------------------------------------------------------------------
 * Data type definitions
 *-----------------------------------------------------------------------*/

typedef void             GLvoid;
typedef char             GLchar;
typedef unsigned int     GLenum;
typedef unsigned char    GLboolean;
typedef unsigned int     GLbitfield;
typedef signed char      GLbyte;       // Changed khronos_int8_t
typedef short            GLshort;
typedef int              GLint;
typedef int              GLsizei;
typedef unsigned char    GLubyte;      // Changed khronos_uint8_t
typedef unsigned short   GLushort;
typedef unsigned int     GLuint;
typedef float            GLfloat;      // Changed khronos_float_t
typedef float            GLclampf;     // Changed khronos_float_t
typedef signed int       GLfixed;      // Changed khronos_int32_t

/* GL types for handling large vertex buffer objects */
typedef signed long int  GLintptr;     // Changed khronos_intptr_t
typedef signed long int  GLsizeiptr;   // Changed khronos_ssize_t

//#if (!defined(__gl2_h_) && !defined(__gl_h_))

/* OpenGL ES core versions */
//#define GL_ES_VERSION_2_0                 1

/* ClearBufferMask */
#define GL_DEPTH_BUFFER_BIT               0x00000100
#define GL_STENCIL_BUFFER_BIT             0x00000400
#define GL_COLOR_BUFFER_BIT               0x00004000

/* Boolean */
#define GL_FALSE                          0
#define GL_TRUE                           1

/* BeginMode */
#define GL_POINTS                         0x0000
#define GL_LINES                          0x0001
#define GL_LINE_LOOP                      0x0002
#define GL_LINE_STRIP                     0x0003
#define GL_TRIANGLES                      0x0004
#define GL_TRIANGLE_STRIP                 0x0005
#define GL_TRIANGLE_FAN                   0x0006

/* AlphaFunction (not supported in ES20) */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* BlendingFactorDest */
#define GL_ZERO                           0
#define GL_ONE                            1
#define GL_SRC_COLOR                      0x0300
#define GL_ONE_MINUS_SRC_COLOR            0x0301
#define GL_SRC_ALPHA                      0x0302
#define GL_ONE_MINUS_SRC_ALPHA            0x0303
#define GL_DST_ALPHA                      0x0304
#define GL_ONE_MINUS_DST_ALPHA            0x0305

/* BlendingFactorSrc */
/*      GL_ZERO */
/*      GL_ONE */
#define GL_DST_COLOR                      0x0306
#define GL_ONE_MINUS_DST_COLOR            0x0307
#define GL_SRC_ALPHA_SATURATE             0x0308
/*      GL_SRC_ALPHA */
/*      GL_ONE_MINUS_SRC_ALPHA */
/*      GL_DST_ALPHA */
/*      GL_ONE_MINUS_DST_ALPHA */

/* BlendEquationSeparate */
#define GL_FUNC_ADD                       0x8006
#define GL_BLEND_EQUATION                 0x8009
#define GL_BLEND_EQUATION_RGB             0x8009    /* same as BLEND_EQUATION */
#define GL_BLEND_EQUATION_ALPHA           0x883D

/* BlendSubtract */
#define GL_FUNC_SUBTRACT                  0x800A
#define GL_FUNC_REVERSE_SUBTRACT          0x800B

/* Separate Blend Functions */
#define GL_BLEND_DST_RGB                  0x80C8
#define GL_BLEND_SRC_RGB                  0x80C9
#define GL_BLEND_DST_ALPHA                0x80CA
#define GL_BLEND_SRC_ALPHA                0x80CB
#define GL_CONSTANT_COLOR                 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR       0x8002
#define GL_CONSTANT_ALPHA                 0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA       0x8004
#define GL_BLEND_COLOR                    0x8005

/* Buffer Objects */
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_ARRAY_BUFFER_BINDING           0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING   0x8895

#define GL_STREAM_DRAW                    0x88E0
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_BUFFER_SIZE                    0x8764
#define GL_BUFFER_USAGE                   0x8765

#define GL_CURRENT_VERTEX_ATTRIB          0x8626

/* CullFaceMode */
#define GL_FRONT                          0x0404
#define GL_BACK                           0x0405
#define GL_FRONT_AND_BACK                 0x0408

/* DepthFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* EnableCap */
#define GL_TEXTURE_2D                     0x0DE1
#define GL_CULL_FACE                      0x0B44
#define GL_BLEND                          0x0BE2
#define GL_DITHER                         0x0BD0
#define GL_STENCIL_TEST                   0x0B90
#define GL_DEPTH_TEST                     0x0B71
#define GL_SCISSOR_TEST                   0x0C11
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_COVERAGE                0x80A0

/* ErrorCode */
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_OUT_OF_MEMORY                  0x0505

/* FrontFaceDirection */
#define GL_CW                             0x0900
#define GL_CCW                            0x0901

/* GetPName */
#define GL_LINE_WIDTH                     0x0B21
#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_STENCIL_BACK_FUNC              0x8800
#define GL_STENCIL_BACK_FAIL              0x8801
#define GL_STENCIL_BACK_PASS_DEPTH_FAIL   0x8802
#define GL_STENCIL_BACK_PASS_DEPTH_PASS   0x8803
#define GL_STENCIL_BACK_REF               0x8CA3
#define GL_STENCIL_BACK_VALUE_MASK        0x8CA4
#define GL_STENCIL_BACK_WRITEMASK         0x8CA5
#define GL_VIEWPORT                       0x0BA2
#define GL_SCISSOR_BOX                    0x0C10
/*      GL_SCISSOR_TEST */
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_RED_BITS                       0x0D52
#define GL_GREEN_BITS                     0x0D53
#define GL_BLUE_BITS                      0x0D54
#define GL_ALPHA_BITS                     0x0D55
#define GL_DEPTH_BITS                     0x0D56
#define GL_STENCIL_BITS                   0x0D57
#define GL_POLYGON_OFFSET_UNITS           0x2A00
/*      GL_POLYGON_OFFSET_FILL */
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_TEXTURE_BINDING_2D             0x8069
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB

/* GetTextureParameter */
/*      GL_TEXTURE_MAG_FILTER */
/*      GL_TEXTURE_MIN_FILTER */
/*      GL_TEXTURE_WRAP_S */
/*      GL_TEXTURE_WRAP_T */

#define GL_NUM_COMPRESSED_TEXTURE_FORMATS 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS     0x86A3

/* HintMode */
#define GL_DONT_CARE                      0x1100
#define GL_FASTEST                        0x1101
#define GL_NICEST                         0x1102

/* HintTarget */
#define GL_GENERATE_MIPMAP_HINT            0x8192

/* DataType */
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_INT                            0x1404
#define GL_UNSIGNED_INT                   0x1405
#define GL_FLOAT                          0x1406
#define GL_FIXED                          0x140C

/* PixelFormat */
#define GL_DEPTH_COMPONENT                0x1902
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A

/* PixelType */
/*      GL_UNSIGNED_BYTE */
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_SHORT_5_6_5           0x8363

/* Shaders */
#define GL_FRAGMENT_SHADER                  0x8B30
#define GL_VERTEX_SHADER                    0x8B31
#define GL_MAX_VERTEX_ATTRIBS               0x8869
#define GL_MAX_VERTEX_UNIFORM_VECTORS       0x8DFB
#define GL_MAX_VARYING_VECTORS              0x8DFC
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS   0x8B4C
#define GL_MAX_TEXTURE_IMAGE_UNITS          0x8872
#define GL_MAX_FRAGMENT_UNIFORM_VECTORS     0x8DFD
#define GL_SHADER_TYPE                      0x8B4F
#define GL_DELETE_STATUS                    0x8B80
#define GL_LINK_STATUS                      0x8B82
#define GL_VALIDATE_STATUS                  0x8B83
#define GL_ATTACHED_SHADERS                 0x8B85
#define GL_ACTIVE_UNIFORMS                  0x8B86
#define GL_ACTIVE_UNIFORM_MAX_LENGTH        0x8B87
#define GL_ACTIVE_ATTRIBUTES                0x8B89
#define GL_ACTIVE_ATTRIBUTE_MAX_LENGTH      0x8B8A
#define GL_SHADING_LANGUAGE_VERSION         0x8B8C
#define GL_CURRENT_PROGRAM                  0x8B8D

/* StencilFunction */
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207

/* StencilOp */
/*      GL_ZERO */
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
#define GL_INVERT                         0x150A
#define GL_INCR_WRAP                      0x8507
#define GL_DECR_WRAP                      0x8508

/* StringName */
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

/* TextureMagFilter */
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601

/* TextureMinFilter */
/*      GL_NEAREST */
/*      GL_LINEAR */
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703

/* TextureParameterName */
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803

/* TextureTarget */
/*      GL_TEXTURE_2D */
#define GL_TEXTURE                        0x1702

#define GL_TEXTURE_CUBE_MAP               0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP       0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X    0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X    0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y    0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y    0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z    0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z    0x851A
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE      0x851C

/* TextureUnit */
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7
#define GL_TEXTURE8                       0x84C8
#define GL_TEXTURE9                       0x84C9
#define GL_TEXTURE10                      0x84CA
#define GL_TEXTURE11                      0x84CB
#define GL_TEXTURE12                      0x84CC
#define GL_TEXTURE13                      0x84CD
#define GL_TEXTURE14                      0x84CE
#define GL_TEXTURE15                      0x84CF
#define GL_TEXTURE16                      0x84D0
#define GL_TEXTURE17                      0x84D1
#define GL_TEXTURE18                      0x84D2
#define GL_TEXTURE19                      0x84D3
#define GL_TEXTURE20                      0x84D4
#define GL_TEXTURE21                      0x84D5
#define GL_TEXTURE22                      0x84D6
#define GL_TEXTURE23                      0x84D7
#define GL_TEXTURE24                      0x84D8
#define GL_TEXTURE25                      0x84D9
#define GL_TEXTURE26                      0x84DA
#define GL_TEXTURE27                      0x84DB
#define GL_TEXTURE28                      0x84DC
#define GL_TEXTURE29                      0x84DD
#define GL_TEXTURE30                      0x84DE
#define GL_TEXTURE31                      0x84DF
#define GL_ACTIVE_TEXTURE                 0x84E0

/* TextureWrapMode */
#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_MIRRORED_REPEAT                0x8370

/* Uniform Types */
#define GL_FLOAT_VEC2                     0x8B50
#define GL_FLOAT_VEC3                     0x8B51
#define GL_FLOAT_VEC4                     0x8B52
#define GL_INT_VEC2                       0x8B53
#define GL_INT_VEC3                       0x8B54
#define GL_INT_VEC4                       0x8B55
#define GL_BOOL                           0x8B56
#define GL_BOOL_VEC2                      0x8B57
#define GL_BOOL_VEC3                      0x8B58
#define GL_BOOL_VEC4                      0x8B59
#define GL_FLOAT_MAT2                     0x8B5A
#define GL_FLOAT_MAT3                     0x8B5B
#define GL_FLOAT_MAT4                     0x8B5C
#define GL_SAMPLER_2D                     0x8B5E
#define GL_SAMPLER_CUBE                   0x8B60

/* Vertex Arrays */
#define GL_VERTEX_ATTRIB_ARRAY_ENABLED        0x8622
#define GL_VERTEX_ATTRIB_ARRAY_SIZE           0x8623
#define GL_VERTEX_ATTRIB_ARRAY_STRIDE         0x8624
#define GL_VERTEX_ATTRIB_ARRAY_TYPE           0x8625
#define GL_VERTEX_ATTRIB_ARRAY_NORMALIZED     0x886A
#define GL_VERTEX_ATTRIB_ARRAY_POINTER        0x8645
#define GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING 0x889F

/* Read Format */
#define GL_IMPLEMENTATION_COLOR_READ_TYPE   0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT 0x8B9B

/* Shader Source */
#define GL_COMPILE_STATUS                 0x8B81
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_SHADER_SOURCE_LENGTH           0x8B88
#define GL_SHADER_COMPILER                0x8DFA

/* Shader Binary */
#define GL_SHADER_BINARY_FORMATS          0x8DF8
#define GL_NUM_SHADER_BINARY_FORMATS      0x8DF9

/* Shader Precision-Specified Types */
#define GL_LOW_FLOAT                      0x8DF0
#define GL_MEDIUM_FLOAT                   0x8DF1
#define GL_HIGH_FLOAT                     0x8DF2
#define GL_LOW_INT                        0x8DF3
#define GL_MEDIUM_INT                     0x8DF4
#define GL_HIGH_INT                       0x8DF5

/* Framebuffer Object. */
#define GL_FRAMEBUFFER                    0x8D40
#define GL_RENDERBUFFER                   0x8D41

#define GL_RGBA4                          0x8056
#define GL_RGB5_A1                        0x8057
#define GL_RGB565                         0x8D62
#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_STENCIL_INDEX                  0x1901
#define GL_STENCIL_INDEX8                 0x8D48

#define GL_RENDERBUFFER_WIDTH             0x8D42
#define GL_RENDERBUFFER_HEIGHT            0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT   0x8D44
#define GL_RENDERBUFFER_RED_SIZE          0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE        0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE         0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE        0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE        0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE      0x8D55

#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE           0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME           0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL         0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE 0x8CD3

#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_STENCIL_ATTACHMENT             0x8D20

#define GL_NONE                           0

#define GL_FRAMEBUFFER_COMPLETE                      0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT         0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT 0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS         0x8CD9
#define GL_FRAMEBUFFER_UNSUPPORTED                   0x8CDD

#define GL_FRAMEBUFFER_BINDING            0x8CA6
#define GL_RENDERBUFFER_BINDING           0x8CA7
#define GL_MAX_RENDERBUFFER_SIZE          0x84E8

#define GL_INVALID_FRAMEBUFFER_OPERATION  0x0506

#else
# ifndef EVAS_GL_NO_GL_H_CHECK
#  error "You may only include either Evas_GL.h OR use your native OpenGL's headers. If you use Evas to do GL, then you cannot use the native gl headers."
# endif
#endif


#if !defined(__glext_h_) && !defined(__gl2ext_h_)
# define __glext_h_
# define __gl2ext_h_

//---------------------------//
// GLES extension defines

/* GL_OES_compressed_ETC1_RGB8_texture */
#define GL_ETC1_RGB8_OES                                        0x8D64

/* GL_OES_compressed_paletted_texture */
#define GL_PALETTE4_RGB8_OES                                    0x8B90
#define GL_PALETTE4_RGBA8_OES                                   0x8B91
#define GL_PALETTE4_R5_G6_B5_OES                                0x8B92
#define GL_PALETTE4_RGBA4_OES                                   0x8B93
#define GL_PALETTE4_RGB5_A1_OES                                 0x8B94
#define GL_PALETTE8_RGB8_OES                                    0x8B95
#define GL_PALETTE8_RGBA8_OES                                   0x8B96
#define GL_PALETTE8_R5_G6_B5_OES                                0x8B97
#define GL_PALETTE8_RGBA4_OES                                   0x8B98
#define GL_PALETTE8_RGB5_A1_OES                                 0x8B99

/* GL_OES_depth24 */
#define GL_DEPTH_COMPONENT24_OES                                0x81A6

/* GL_OES_depth32 */
#define GL_DEPTH_COMPONENT32_OES                                0x81A7

/* GL_OES_get_program_binary */
#define GL_PROGRAM_BINARY_LENGTH_OES                            0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS_OES                       0x87FE
#define GL_PROGRAM_BINARY_FORMATS_OES                           0x87FF

/* GL_OES_mapbuffer */
#define GL_WRITE_ONLY_OES                                       0x88B9
#define GL_BUFFER_ACCESS_OES                                    0x88BB
#define GL_BUFFER_MAPPED_OES                                    0x88BC
#define GL_BUFFER_MAP_POINTER_OES                               0x88BD

/* GL_OES_packed_depth_stencil */
#define GL_DEPTH_STENCIL_OES                                    0x84F9
#define GL_UNSIGNED_INT_24_8_OES                                0x84FA
#define GL_DEPTH24_STENCIL8_OES                                 0x88F0

/* GL_OES_rgb8_rgba8 */
#define GL_RGB8_OES                                             0x8051
#define GL_RGBA8_OES                                            0x8058

/* GL_OES_standard_derivatives */
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES                  0x8B8B

/* GL_OES_stencil1 */
#define GL_STENCIL_INDEX1_OES                                   0x8D46

/* GL_OES_stencil4 */
#define GL_STENCIL_INDEX4_OES                                   0x8D47

/* GL_OES_texture_3D */
#define GL_TEXTURE_WRAP_R_OES                                   0x8072
#define GL_TEXTURE_3D_OES                                       0x806F
#define GL_TEXTURE_BINDING_3D_OES                               0x806A
#define GL_MAX_3D_TEXTURE_SIZE_OES                              0x8073
#define GL_SAMPLER_3D_OES                                       0x8B5F
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_OES        0x8CD4

/* GL_OES_texture_float */
/* No new tokens introduced by this extension. */

/* GL_OES_texture_float_linear */
/* No new tokens introduced by this extension. */

/* GL_OES_texture_half_float */
#define GL_HALF_FLOAT_OES                                       0x8D61

/* GL_OES_texture_half_float_linear */
/* No new tokens introduced by this extension. */

/* GL_OES_texture_npot */
/* No new tokens introduced by this extension. */

/* GL_OES_vertex_half_float */
/* GL_HALF_FLOAT_OES defined in GL_OES_texture_half_float already. */

/* GL_OES_vertex_type_10_10_10_2 */
#define GL_UNSIGNED_INT_10_10_10_2_OES                          0x8DF6
#define GL_INT_10_10_10_2_OES                                   0x8DF7

/*------------------------------------------------------------------------*
 * AMD extension tokens
 *------------------------------------------------------------------------*/

/* GL_AMD_compressed_3DC_texture */
#define GL_3DC_X_AMD                                            0x87F9
#define GL_3DC_XY_AMD                                           0x87FA

/* GL_AMD_compressed_ATC_texture */
#define GL_ATC_RGB_AMD                                          0x8C92
#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD                          0x8C93
#define GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD                      0x87EE

/* GL_AMD_performance_monitor */
#define GL_COUNTER_TYPE_AMD                                     0x8BC0
#define GL_COUNTER_RANGE_AMD                                    0x8BC1
#define GL_UNSIGNED_INT64_AMD                                   0x8BC2
#define GL_PERCENTAGE_AMD                                       0x8BC3
#define GL_PERFMON_RESULT_AVAILABLE_AMD                         0x8BC4
#define GL_PERFMON_RESULT_SIZE_AMD                              0x8BC5
#define GL_PERFMON_RESULT_AMD                                   0x8BC6

/* GL_AMD_program_binary_Z400 */
#define GL_Z400_BINARY_AMD                                      0x8740

/*------------------------------------------------------------------------*
 * EXT extension tokens
 *------------------------------------------------------------------------*/

/* GL_EXT_blend_minmax */
#define GL_MIN_EXT                                              0x8007
#define GL_MAX_EXT                                              0x8008

/* GL_EXT_discard_framebuffer */
#define GL_COLOR_EXT                                            0x1800
#define GL_DEPTH_EXT                                            0x1801
#define GL_STENCIL_EXT                                          0x1802

/* GL_EXT_multi_draw_arrays */
/* No new tokens introduced by this extension. */

/* GL_EXT_read_format_bgra */
#define GL_BGRA_EXT                                             0x80E1
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT                       0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT                       0x8366

/* GL_EXT_texture_filter_anisotropic */
#define GL_TEXTURE_MAX_ANISOTROPY_EXT                           0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT                       0x84FF

/* GL_EXT_texture_format_BGRA8888 */
#define GL_BGRA_EXT                                             0x80E1

/* GL_EXT_texture_type_2_10_10_10_REV */
#define GL_UNSIGNED_INT_2_10_10_10_REV_EXT                      0x8368

/*------------------------------------------------------------------------*
 * IMG extension tokens
 *------------------------------------------------------------------------*/

/* GL_IMG_program_binary */
#define GL_SGX_PROGRAM_BINARY_IMG                               0x9130

/* GL_IMG_read_format */
#define GL_BGRA_IMG                                             0x80E1
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_IMG                       0x8365

/* GL_IMG_shader_binary */
#define GL_SGX_BINARY_IMG                                       0x8C0A

/* GL_IMG_texture_compression_pvrtc */
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG                      0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG                      0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG                     0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG                     0x8C03

/* GL_IMG_multisampled_render_to_texture */
#define GL_RENDERBUFFER_SAMPLES_IMG                             0x9133
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG               0x9134
#define GL_MAX_SAMPLES_IMG                                      0x9135
#define GL_TEXTURE_SAMPLES_IMG                                  0x9136

/*------------------------------------------------------------------------*
 * NV extension tokens
 *------------------------------------------------------------------------*/

/* GL_NV_fence */
#define GL_ALL_COMPLETED_NV                                     0x84F2
#define GL_FENCE_STATUS_NV                                      0x84F3
#define GL_FENCE_CONDITION_NV                                   0x84F4

/*------------------------------------------------------------------------*
 * QCOM extension tokens
 *------------------------------------------------------------------------*/

/* GL_QCOM_driver_control */
/* No new tokens introduced by this extension. */

/* GL_QCOM_extended_get */
#define GL_TEXTURE_WIDTH_QCOM                                   0x8BD2
#define GL_TEXTURE_HEIGHT_QCOM                                  0x8BD3
#define GL_TEXTURE_DEPTH_QCOM                                   0x8BD4
#define GL_TEXTURE_INTERNAL_FORMAT_QCOM                         0x8BD5
#define GL_TEXTURE_FORMAT_QCOM                                  0x8BD6
#define GL_TEXTURE_TYPE_QCOM                                    0x8BD7
#define GL_TEXTURE_IMAGE_VALID_QCOM                             0x8BD8
#define GL_TEXTURE_NUM_LEVELS_QCOM                              0x8BD9
#define GL_TEXTURE_TARGET_QCOM                                  0x8BDA
#define GL_TEXTURE_OBJECT_VALID_QCOM                            0x8BDB
#define GL_STATE_RESTORE                                        0x8BDC

/* GL_QCOM_extended_get2 */
/* No new tokens introduced by this extension. */

/* GL_QCOM_perfmon_global_mode */
#define GL_PERFMON_GLOBAL_MODE_QCOM                             0x8FA0

/* GL_QCOM_writeonly_rendering */
#define GL_WRITEONLY_RENDERING_QCOM                             0x8823

/*------------------------------------------------------------------------*
 * End of extension tokens, start of corresponding extension functions
 *------------------------------------------------------------------------*/

/* EvasGL_KHR_image */
#define EVAS_GL_NATIVE_PIXMAP                                   0x30B0  /* evasglCreateImage target */

/* EvasGL_KHR_vg_parent_image */
#define EVAS_VG_PARENT_IMAGE                                    0x30BA  /* evasglCreateImage target */

/* EvasGL_KHR_gl_texture_2D_image */
#define EVAS_GL_TEXTURE_2D                                      0x30B1  /* evasglCreateImage target */
#define EVAS_GL_TEXTURE_LEVEL                                   0x30BC  /* evasglCreateImage attribute */

/* EvasGL_KHR_gl_texture_cubemap_image */
#define EVAS_GL_TEXTURE_CUBE_MAP_POSITIVE_X                     0x30B3  /* evasglCreateImage target */
#define EVAS_GL_TEXTURE_CUBE_MAP_NEGATIVE_X                     0x30B4  /* evasglCreateImage target */
#define EVAS_GL_TEXTURE_CUBE_MAP_POSITIVE_Y                     0x30B5  /* evasglCreateImage target */
#define EVAS_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y                     0x30B6  /* evasglCreateImage target */
#define EVAS_GL_TEXTURE_CUBE_MAP_POSITIVE_Z                     0x30B7  /* evasglCreateImage target */
#define EVAS_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z                     0x30B8  /* evasglCreateImage target */

/* EvasGL_KHR_gl_texture_3D_image */
#define EVAS_GL_TEXTURE_3D                                      0x30B2  /* evasglCreateImage target */
#define EVAS_GL_TEXTURE_ZOFFSET                                 0x30BD  /* evasglCreateImage attribute */

/* EvasGL_KHR_gl_renderbuffer_image */
#define EVAS_GL_RENDERBUFFER                                    0x30B9  /* evasglCreateImage target */

#else
# ifndef EVAS_GL_NO_GL_H_CHECK
#  error "You may only include either Evas_GL.h OR use your native OpenGL's headers. If you use Evas to do GL, then you cannot use the native gl headers."
# endif
#endif

#define EVAS_GL_API_VERSION 1
struct _Evas_GL_API
{
   int            version;

   /* version 1: */
   /*------- GLES 2.0 -------*/
   void         (*glActiveTexture) (GLenum texture);
   void         (*glAttachShader) (GLuint program, GLuint shader);
   void         (*glBindAttribLocation) (GLuint program, GLuint index, const char* name);
   void         (*glBindBuffer) (GLenum target, GLuint buffer);
   void         (*glBindFramebuffer) (GLenum target, GLuint framebuffer);
   void         (*glBindRenderbuffer) (GLenum target, GLuint renderbuffer);
   void         (*glBindTexture) (GLenum target, GLuint texture);
   void         (*glBlendColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
   void         (*glBlendEquation) ( GLenum mode );
   void         (*glBlendEquationSeparate) (GLenum modeRGB, GLenum modeAlpha);
   void         (*glBlendFunc) (GLenum sfactor, GLenum dfactor);
   void         (*glBlendFuncSeparate) (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
   void         (*glBufferData) (GLenum target, GLsizeiptr size, const void* data, GLenum usage);
   void         (*glBufferSubData) (GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
   GLenum       (*glCheckFramebufferStatus) (GLenum target);
   void         (*glClear) (GLbitfield mask);
   void         (*glClearColor) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
   void         (*glClearDepthf) (GLclampf depth);
   void         (*glClearStencil) (GLint s);
   void         (*glColorMask) (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
   void         (*glCompileShader) (GLuint shader);
   void         (*glCompressedTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void* data);
   void         (*glCompressedTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void* data);
   void         (*glCopyTexImage2D) (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
   void         (*glCopyTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
   GLuint       (*glCreateProgram) (void);
   GLuint       (*glCreateShader) (GLenum type);
   void         (*glCullFace) (GLenum mode);
   void         (*glDeleteBuffers) (GLsizei n, const GLuint* buffers);
   void         (*glDeleteFramebuffers) (GLsizei n, const GLuint* framebuffers);
   void         (*glDeleteProgram) (GLuint program);
   void         (*glDeleteRenderbuffers) (GLsizei n, const GLuint* renderbuffers);
   void         (*glDeleteShader) (GLuint shader);
   void         (*glDeleteTextures) (GLsizei n, const GLuint* textures);
   void         (*glDepthFunc) (GLenum func);
   void         (*glDepthMask) (GLboolean flag);
   void         (*glDepthRangef) (GLclampf zNear, GLclampf zFar);
   void         (*glDetachShader) (GLuint program, GLuint shader);
   void         (*glDisable) (GLenum cap);
   void         (*glDisableVertexAttribArray) (GLuint index);
   void         (*glDrawArrays) (GLenum mode, GLint first, GLsizei count);
   void         (*glDrawElements) (GLenum mode, GLsizei count, GLenum type, const void* indices);
   void         (*glEnable) (GLenum cap);
   void         (*glEnableVertexAttribArray) (GLuint index);
   void         (*glFinish) (void);
   void         (*glFlush) (void);
   void         (*glFramebufferRenderbuffer) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
   void         (*glFramebufferTexture2D) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
   void         (*glFrontFace) (GLenum mode);
   void         (*glGenBuffers) (GLsizei n, GLuint* buffers);
   void         (*glGenerateMipmap) (GLenum target);
   void         (*glGenFramebuffers) (GLsizei n, GLuint* framebuffers);
   void         (*glGenRenderbuffers) (GLsizei n, GLuint* renderbuffers);
   void         (*glGenTextures) (GLsizei n, GLuint* textures);
   void         (*glGetActiveAttrib) (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name);
   void         (*glGetActiveUniform) (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, char* name);
   void         (*glGetAttachedShaders) (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
   int          (*glGetAttribLocation) (GLuint program, const char* name);
   void         (*glGetBooleanv) (GLenum pname, GLboolean* params);
   void         (*glGetBufferParameteriv) (GLenum target, GLenum pname, GLint* params);
   GLenum       (*glGetError) (void);
   void         (*glGetFloatv) (GLenum pname, GLfloat* params);
   void         (*glGetFramebufferAttachmentParameteriv) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
   void         (*glGetIntegerv) (GLenum pname, GLint* params);
   void         (*glGetProgramiv) (GLuint program, GLenum pname, GLint* params);
   void         (*glGetProgramInfoLog) (GLuint program, GLsizei bufsize, GLsizei* length, char* infolog);
   void         (*glGetRenderbufferParameteriv) (GLenum target, GLenum pname, GLint* params);
   void         (*glGetShaderiv) (GLuint shader, GLenum pname, GLint* params);
   void         (*glGetShaderInfoLog) (GLuint shader, GLsizei bufsize, GLsizei* length, char* infolog);
   void         (*glGetShaderPrecisionFormat) (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
   void         (*glGetShaderSource) (GLuint shader, GLsizei bufsize, GLsizei* length, char* source);
   const GLubyte* (*glGetString) (GLenum name);
   void         (*glGetTexParameterfv) (GLenum target, GLenum pname, GLfloat* params);
   void         (*glGetTexParameteriv) (GLenum target, GLenum pname, GLint* params);
   void         (*glGetUniformfv) (GLuint program, GLint location, GLfloat* params);
   void         (*glGetUniformiv) (GLuint program, GLint location, GLint* params);
   int          (*glGetUniformLocation) (GLuint program, const char* name);
   void         (*glGetVertexAttribfv) (GLuint index, GLenum pname, GLfloat* params);
   void         (*glGetVertexAttribiv) (GLuint index, GLenum pname, GLint* params);
   void         (*glGetVertexAttribPointerv) (GLuint index, GLenum pname, void** pointer);
   void         (*glHint) (GLenum target, GLenum mode);
   GLboolean    (*glIsBuffer) (GLuint buffer);
   GLboolean    (*glIsEnabled) (GLenum cap);
   GLboolean    (*glIsFramebuffer) (GLuint framebuffer);
   GLboolean    (*glIsProgram) (GLuint program);
   GLboolean    (*glIsRenderbuffer) (GLuint renderbuffer);
   GLboolean    (*glIsShader) (GLuint shader);
   GLboolean    (*glIsTexture) (GLuint texture);
   void         (*glLineWidth) (GLfloat width);
   void         (*glLinkProgram) (GLuint program);
   void         (*glPixelStorei) (GLenum pname, GLint param);
   void         (*glPolygonOffset) (GLfloat factor, GLfloat units);
   void         (*glReadPixels) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void* pixels);
   void         (*glReleaseShaderCompiler) (void);
   void         (*glRenderbufferStorage) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
   void         (*glSampleCoverage) (GLclampf value, GLboolean invert);
   void         (*glScissor) (GLint x, GLint y, GLsizei width, GLsizei height);
   void         (*glShaderBinary) (GLsizei n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLsizei length);
   void         (*glShaderSource) (GLuint shader, GLsizei count, const char* const * string, const GLint* length);
   void         (*glStencilFunc) (GLenum func, GLint ref, GLuint mask);
   void         (*glStencilFuncSeparate) (GLenum face, GLenum func, GLint ref, GLuint mask);
   void         (*glStencilMask) (GLuint mask);
   void         (*glStencilMaskSeparate) (GLenum face, GLuint mask);
   void         (*glStencilOp) (GLenum fail, GLenum zfail, GLenum zpass);
   void         (*glStencilOpSeparate) (GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
   void         (*glTexImage2D) (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
   void         (*glTexParameterf) (GLenum target, GLenum pname, GLfloat param);
   void         (*glTexParameterfv) (GLenum target, GLenum pname, const GLfloat* params);
   void         (*glTexParameteri) (GLenum target, GLenum pname, GLint param);
   void         (*glTexParameteriv) (GLenum target, GLenum pname, const GLint* params);
   void         (*glTexSubImage2D) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void* pixels);
   void         (*glUniform1f) (GLint location, GLfloat x);
   void         (*glUniform1fv) (GLint location, GLsizei count, const GLfloat* v);
   void         (*glUniform1i) (GLint location, GLint x);
   void         (*glUniform1iv) (GLint location, GLsizei count, const GLint* v);
   void         (*glUniform2f) (GLint location, GLfloat x, GLfloat y);
   void         (*glUniform2fv) (GLint location, GLsizei count, const GLfloat* v);
   void         (*glUniform2i) (GLint location, GLint x, GLint y);
   void         (*glUniform2iv) (GLint location, GLsizei count, const GLint* v);
   void         (*glUniform3f) (GLint location, GLfloat x, GLfloat y, GLfloat z);
   void         (*glUniform3fv) (GLint location, GLsizei count, const GLfloat* v);
   void         (*glUniform3i) (GLint location, GLint x, GLint y, GLint z);
   void         (*glUniform3iv) (GLint location, GLsizei count, const GLint* v);
   void         (*glUniform4f) (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void         (*glUniform4fv) (GLint location, GLsizei count, const GLfloat* v);
   void         (*glUniform4i) (GLint location, GLint x, GLint y, GLint z, GLint w);
   void         (*glUniform4iv) (GLint location, GLsizei count, const GLint* v);
   void         (*glUniformMatrix2fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
   void         (*glUniformMatrix3fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
   void         (*glUniformMatrix4fv) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
   void         (*glUseProgram) (GLuint program);
   void         (*glValidateProgram) (GLuint program);
   void         (*glVertexAttrib1f) (GLuint indx, GLfloat x);
   void         (*glVertexAttrib1fv) (GLuint indx, const GLfloat* values);
   void         (*glVertexAttrib2f) (GLuint indx, GLfloat x, GLfloat y);
   void         (*glVertexAttrib2fv) (GLuint indx, const GLfloat* values);
   void         (*glVertexAttrib3f) (GLuint indx, GLfloat x, GLfloat y, GLfloat z);
   void         (*glVertexAttrib3fv) (GLuint indx, const GLfloat* values);
   void         (*glVertexAttrib4f) (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void         (*glVertexAttrib4fv) (GLuint indx, const GLfloat* values);
   void         (*glVertexAttribPointer) (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* ptr);
   void         (*glViewport) (GLint x, GLint y, GLsizei width, GLsizei height);

   /*------- GLES 2.0 Extensions -------*/
   // Notice these two names have been changed to fit Evas GL and not EGL!
   /* GL_OES_EvasGL_image */
   void         (*glEvasGLImageTargetTexture2DOES) (GLenum target, EvasGLImage image);
   void         (*glEvasGLImageTargetRenderbufferStorageOES) (GLenum target, EvasGLImage image);

   /* GL_OES_get_program_binary */
   void 	(*glGetProgramBinaryOES) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
   void 	(*glProgramBinaryOES) (GLuint program, GLenum binaryFormat, const void *binary, GLint length);   
   /* GL_OES_mapbuffer */
   void* 	(*glMapBufferOES) (GLenum target, GLenum access);
   GLboolean 	(*glUnmapBufferOES) (GLenum target);
   void 	(*glGetBufferPointervOES) (GLenum target, GLenum pname, void** params);
   /* GL_OES_texture_3D */
   void 	(*glTexImage3DOES) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels);
   void 	(*glTexSubImage3DOES) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
   void 	(*glCopyTexSubImage3DOES) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
   void 	(*glCompressedTexImage3DOES) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void* data);
   void 	(*glCompressedTexSubImage3DOES) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void* data);
   void 	(*glFramebufferTexture3DOES) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);

   /* AMD_performance_monitor */
   void 	(*glGetPerfMonitorGroupsAMD) (GLint* numGroups, GLsizei groupsSize, GLuint* groups);
   void 	(*glGetPerfMonitorCountersAMD) (GLuint group, GLint* numCounters, GLint* maxActiveCounters, GLsizei counterSize, GLuint* counters);
   void 	(*glGetPerfMonitorGroupStringAMD) (GLuint group, GLsizei bufSize, GLsizei* length, char* groupString);
   void 	(*glGetPerfMonitorCounterStringAMD) (GLuint group, GLuint counter, GLsizei bufSize, GLsizei* length, char* counterString);
   void 	(*glGetPerfMonitorCounterInfoAMD) (GLuint group, GLuint counter, GLenum pname, void* data);
   void 	(*glGenPerfMonitorsAMD) (GLsizei n, GLuint* monitors);
   void 	(*glDeletePerfMonitorsAMD) (GLsizei n, GLuint* monitors);
   void 	(*glSelectPerfMonitorCountersAMD) (GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint* countersList);
   void 	(*glBeginPerfMonitorAMD) (GLuint monitor);
   void 	(*glEndPerfMonitorAMD) (GLuint monitor);
   void 	(*glGetPerfMonitorCounterDataAMD) (GLuint monitor, GLenum pname, GLsizei dataSize, GLuint* data, GLint* bytesWritten);

   /* GL_EXT_discard_framebuffer */
   void 	(*glDiscardFramebufferEXT) (GLenum target, GLsizei numAttachments, const GLenum* attachments);

   /* GL_EXT_multi_draw_arrays */
   void 	(*glMultiDrawArraysEXT) (GLenum mode, GLint* first, GLsizei* count, GLsizei primcount);
   void 	(*glMultiDrawElementsEXT) (GLenum mode, const GLsizei* count, GLenum type, const GLvoid** indices, GLsizei primcount);

   /* GL_NV_fence */
   void 	(*glDeleteFencesNV) (GLsizei n, const GLuint* fences);
   void 	(*glGenFencesNV) (GLsizei n, GLuint* fences);
   GLboolean 	(*glIsFenceNV) (GLuint fence);
   GLboolean 	(*glTestFenceNV) (GLuint fence);
   void 	(*glGetFenceivNV) (GLuint fence, GLenum pname, GLint* params);
   void 	(*glFinishFenceNV) (GLuint fence);
   void 	(*glSetFenceNV) (GLuint, GLenum);

   /* GL_QCOM_driver_control */
   void 	(*glGetDriverControlsQCOM) (GLint* num, GLsizei size, GLuint* driverControls);
   void 	(*glGetDriverControlStringQCOM) (GLuint driverControl, GLsizei bufSize, GLsizei* length, char* driverControlString);
   void 	(*glEnableDriverControlQCOM) (GLuint driverControl);
   void 	(*glDisableDriverControlQCOM) (GLuint driverControl);

   /* GL_QCOM_extended_get */
   void 	(*glExtGetTexturesQCOM) (GLuint* textures, GLint maxTextures, GLint* numTextures);
   void 	(*glExtGetBuffersQCOM) (GLuint* buffers, GLint maxBuffers, GLint* numBuffers);
   void 	(*glExtGetRenderbuffersQCOM) (GLuint* renderbuffers, GLint maxRenderbuffers, GLint* numRenderbuffers);
   void 	(*glExtGetFramebuffersQCOM) (GLuint* framebuffers, GLint maxFramebuffers, GLint* numFramebuffers);
   void 	(*glExtGetTexLevelParameterivQCOM) (GLuint texture, GLenum face, GLint level, GLenum pname, GLint* params);
   void 	(*glExtTexObjectStateOverrideiQCOM) (GLenum target, GLenum pname, GLint param);
   void 	(*glExtGetTexSubImageQCOM) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, void* texels);
   void 	(*glExtGetBufferPointervQCOM) (GLenum target, void** params);


   /* GL_QCOM_extended_get2 */
   void 	(*glExtGetShadersQCOM) (GLuint* shaders, GLint maxShaders, GLint* numShaders);
   void 	(*glExtGetProgramsQCOM) (GLuint* programs, GLint maxPrograms, GLint* numPrograms);
   GLboolean 	(*glExtIsProgramBinaryQCOM) (GLuint program);
   void 	(*glExtGetProgramBinarySourceQCOM) (GLuint program, GLenum shadertype, char* source, GLint* length);

   //------- EGL Related Extensions -------//
   /* EvasGL_KHR_image */
   EvasGLImage  (*evasglCreateImage) (int target, void* buffer, int* attrib_list);
   void         (*evasglDestroyImage) (EvasGLImage image);

   /* future calls will be added down here for expansion */
   /* version 2: */
};


#ifdef __cplusplus
}
#endif

#endif
/**
 * @}
 */
