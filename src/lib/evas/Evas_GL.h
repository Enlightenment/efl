#ifndef _EVAS_GL_H
#define _EVAS_GL_H

#include <Evas.h>
//#include <GL/gl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup Evas_GL Rendering GL on Evas
 * @ingroup Evas_Canvas
 *
 * @brief This group discusses the functions that are used to do OpenGL rendering on Evas. Evas allows you
 *        to use OpenGL to render to specially set up image objects (which act as
 *        render target surfaces).
 *        By default, Evas GL will use an OpenGL-ES 2.0 context and API set.
 *
 *
 * <h2> Evas GL vs. Elementary GLView </h2>
 *
 * While it is possible to Evas and Ecore_Evas to create an OpenGL application,
 * using these low-level APIs can be troublesome for most users. Before
 * diving in Evas GL, please refer to the page @ref elm_opengl_page.
 *
 * Elementary @ref GLView provides a set of helper functions in:
 * @li @ref Elementary_GL_Helpers.h
 *
 * Similarly, two sets of helper functions are provided by Evas GL in the
 * following header files:
 * @li Evas_GL_GLES1_Helpers.h
 * @li Evas_GL_GLES2_Helpers.h
 *
 * @{
 */

/*
 * <h2> Evas GL usage example </h2>
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

// Callbacks we need to handle deletion on the object and updates/draws
static void      on_del       (void *data, Evas *e, Evas_Object *obj, void *event_info);
static void      on_pixels    (void *data, Evas_Object *obj);
// Demo - animator just to keep ticking over asking to draw the image
static Eina_Bool on_animate   (void *data);
// gl stuff
static int       init_shaders (GLData *gld);
static GLuint    load_shader  (GLData *gld, GLenum type, const char *shader_src);

int
main(int argc, char **argv)
{
   // A size by default
   int w = 256, h = 256;
   // Some variables we will use
   Ecore_Evas  *ee;
   Evas *canvas;
   Evas_Object *r1;
   Evas_Native_Surface ns;
   GLData *gld = NULL;

   // Regular low-level EFL (ecore+ecore-evas) init. elm is simpler
   ecore_init();
   ecore_evas_init();
   ee = ecore_evas_gl_x11_new(NULL, 0, 0, 0, 512, 512);
   ecore_evas_title_set(ee, "Ecore_Evas Template");
   canvas = ecore_evas_get(ee);

   // Alloc a data struct to hold our relevant gl info in it
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

   // Create a surface and context
   gld->sfc = evas_gl_surface_create(gld->evasgl, gld->cfg, w, h);
   gld->ctx = evas_gl_context_create(gld->evasgl, NULL);
   //-//
   //-//-//-// END GL INIT BLOB

   // Set up the image object. A filled one by default
   r1 = evas_object_image_filled_add(canvas);
   // Attach important data we need to the object using key names. This just
   // avoids some global variables which means we can do a good cleanup. You can
   // avoid this if you are lazy
   evas_object_data_set(r1, "..gld", gld);
   // When the object is deleted - call the on_del callback. Like the above,
   // this is just being clean
   evas_object_event_callback_add(r1, EVAS_CALLBACK_DEL, on_del, NULL);
   // Set up an actual pixel size for the buffer data. It may be different
   // from the output size. Any windowing system has something like this, only
   // evas has 2 sizes, a pixel size and the output object size
   evas_object_image_size_set(r1, w, h);
   // Set up the native surface info to use the context and surface created
   // above

   //-//-//-// THIS IS WHERE GL INIT STUFF HAPPENS (ALA EGL)
   //-//
   evas_gl_native_surface_get(gld->evasgl, gld->sfc, &ns);
   evas_object_image_native_surface_set(r1, &ns);
   evas_object_image_pixels_get_callback_set(r1, on_pixels, r1);
   //-//
   //-//-//-// END GL INIT BLOB

   // Move the image object somewhere, resize it, and show it. Any windowing
   // system would need this kind of thing - place a child "window"
   evas_object_move(r1, 128, 128);
   evas_object_resize(r1, w, h);
   evas_object_show(r1);

   // Animating - just a demo. As long as you trigger an update on the image
   // object via evas_object_image_pixels_dirty_set(), any display system,
   // mainloop system etc., will have something of this kind unless it's making
   // you spin infinitely by yourself and invent your own animation mechanism
   //
   // NOTE: If you delete r1, this animator will keep running and trying to access
   // r1 so you'd better delete this animator with ecore_animator_del() or
   // structure how you do animation differently. You can also attach it like
   // evasgl, sfc, etc., if this animator is specific to this object
   // then delete it in the del handler for the obj.
   ecore_animator_add(on_animate, r1);

   // Finally show the window for the world to see. Windowing system generic
   ecore_evas_show(ee);

   // Begin the mainloop and tick over the animator, handle events, etc.
   // Also windowing system generic
   ecore_main_loop_begin();

   // Standard EFL shutdown stuff - generic for most systems, EFL or not
   ecore_evas_shutdown();
   ecore_shutdown();
   return 0;
}

static void
on_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   // On delete of our object clean up some things that don't get auto
   // deleted for us as they are not intrinsically bound to the image
   // object as such (you could use the same context and surface across
   // multiple image objects and re-use the evasgl handle multiple times.
   // Here we bind them to only 1 object though by doing this.
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
   // Get some variable we need from the object data keys
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

   // Get the image size, in case it changed, with evas_object_image_size_set()
   evas_object_image_size_get(obj, &w, &h);
   // Set up the context and surface as the current one
   evas_gl_make_current(gld->evasgl, gld->sfc, gld->ctx);

   if (!gld->initialized)
     {
        if (!init_shaders(gld)) printf("Error Initializing Shaders\n");
        gld->initialized = EINA_TRUE;
     }

   // GL Viewport stuff. You can avoid doing this if viewport is all the
   // same as the last frame, if you want
   gl->glViewport(0, 0, w, h);

   // Clear the buffer
   gl->glClearColor(1.0, 0.0, 0.0, 1);
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
   // Just a demo - Animate here whenever an animation tick happens and then
   // mark the image as "dirty" meaning it needs an update the next time evas
   // renders. It will then call the pixel get callback.
   evas_object_image_pixels_dirty_set(data, EINA_TRUE);
   return EINA_TRUE; // Keep looping
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
 */

/**
  * @typedef Evas_GL
  *
  * @brief The structure type of the Evas GL object used to render GL in Evas.
  */
typedef struct _Evas_GL               Evas_GL;

/**
  * @typedef Evas_GL_Surface
  *
  * @brief The structure type of the Evas GL Surface object, a GL rendering target in Evas GL.
  */
typedef struct _Evas_GL_Surface       Evas_GL_Surface;

/**
  * @typedef Evas_GL_Context
  *
  * @brief The structure type of the Evas GL Context object, a GL rendering context in Evas GL.
  */
typedef struct _Evas_GL_Context       Evas_GL_Context;

/**
  * @typedef Evas_GL_Config
  *
  * @brief The structure type of the Evas GL Surface configuration object for surface creation.
  */
typedef struct _Evas_GL_Config        Evas_GL_Config;

/**
  * @typedef Evas_GL_API
  *
  * @brief The structure type of the Evas GL API object that contains the GL APIs to be used in Evas GL.
  */
typedef struct _Evas_GL_API           Evas_GL_API;

/**
  * @typedef Evas_GL_Func
  *
  * @brief Represents a function pointer, that can be used for Evas GL extensions.
  */
typedef void                         *Evas_GL_Func;

/**
  * @typedef EvasGLImage
  *
  * @brief Represents an Evas GL Image object used with Evas GL Image extensions.
  */
typedef void                         *EvasGLImage;

/**
 * @brief Enumeration that defines the available surface color formats.
 */
typedef enum _Evas_GL_Color_Format
{
    EVAS_GL_RGB_888   = 0, /**< Opaque RGB surface */
    EVAS_GL_RGBA_8888 = 1, /**< RGBA surface with alpha */
    EVAS_GL_NO_FBO    = 2  /**< Special value for creating PBuffer surfaces without any attached buffer. @see evas_gl_pbuffer_surface_create. @since_tizen 2.3 */
} Evas_GL_Color_Format;

/**
 * @brief Enumeration that defines the Surface Depth Format.
 * @since_tizen 2.3
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
 * @brief Enumeration that defines the Surface Stencil Format.
 * @since_tizen 2.3
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
 * @brief Enumeration that defines the Configuration Options.
 *
 * @since 1.1
 * @since_tizen 2.3
 */
typedef enum _Evas_GL_Options_Bits
{
   EVAS_GL_OPTIONS_NONE    = 0,     /**< No extra options */
   EVAS_GL_OPTIONS_DIRECT  = (1<<0),/**< Optional hint to allow rendering directly to the Evas window if possible */
   EVAS_GL_OPTIONS_CLIENT_SIDE_ROTATION = (1<<1) /**< Force direct rendering even if the canvas is rotated.
                                                  *   In that case, it is the application's role to rotate the contents of
                                                  *   the Evas_GL view. @see evas_gl_rotation_get */
} Evas_GL_Options_Bits;

/**
 * @brief Enumeration that defines the configuration options for a Multisample Anti-Aliased (MSAA) rendering surface.
 *
 * @since 1.2
 * @since_tizen 2.3
 *
 * @remarks This only works on devices that support the required extensions.
 */
typedef enum _Evas_GL_Multisample_Bits
{
   EVAS_GL_MULTISAMPLE_NONE = 0, /**< No multisample rendering */
   EVAS_GL_MULTISAMPLE_LOW  = 1, /**< MSAA with minimum number of samples */
   EVAS_GL_MULTISAMPLE_MED  = 2, /**< MSAA with half the maximum number of samples */
   EVAS_GL_MULTISAMPLE_HIGH = 3  /**< MSAA with maximum allowed samples */
} Evas_GL_Multisample_Bits;

/**
 * @brief Enumeration that defines the available OpenGL ES version numbers.
 *        They can be used to create OpenGL-ES 1.1 contexts.
 *
 * @since_tizen 2.3
 *
 * @see evas_gl_context_version_create
 *
 * @remarks This will only work with EGL/GLES (but not with desktop OpenGL).
 */
typedef enum _Evas_GL_Context_Version
{
   EVAS_GL_GLES_1_X = 1, /**< OpenGL-ES 1.x */
   EVAS_GL_GLES_2_X = 2, /**< OpenGL-ES 2.x is the default */
   EVAS_GL_GLES_3_X = 3  /**< @internal OpenGL-ES 3.x, not implemented yet */
} Evas_GL_Context_Version;

/**
 * @struct _Evas_GL_Config
 *
 * @brief A structure used to specify the configuration of an @ref Evas_GL_Surface.
 *
 * This structure should be allocated with @ref evas_gl_config_new() and released
 * with @ref evas_gl_config_free().
 *
 * @see evas_gl_surface_create
 * @see evas_gl_pbuffer_surface_create
 *
 * @since_tizen 2.3
 */
struct _Evas_GL_Config
{
   Evas_GL_Color_Format       color_format;     /**< Surface Color Format */
   Evas_GL_Depth_Bits         depth_bits;       /**< Surface Depth Bits */
   Evas_GL_Stencil_Bits       stencil_bits;     /**< Surface Stencil Bits */
   Evas_GL_Options_Bits       options_bits;     /**< Extra Surface Options */
   Evas_GL_Multisample_Bits   multisample_bits; /**< Optional Surface MSAA Bits */
   Evas_GL_Context_Version    gles_version;     /**< @internal Special flag for OpenGL-ES 1.1 indirect rendering surfaces */
};

/** @brief Constant to use when calling @ref evas_gl_string_query to retrieve the available Evas_GL extensions. */
#define EVAS_GL_EXTENSIONS       1


/**
 * @brief Creates a new Evas_GL object and returns a handle for GL rendering with the EFL.
 *
 * @param[in] e The given Evas canvas to use
 *
 * @return The created Evas_GL object, or @c NULL in case of failure
 *
 * @since_tizen 2.3
 */
EAPI Evas_GL                 *evas_gl_new                (Evas *e) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Frees an Evas_GL object.
 *
 * @param[in] evas_gl   The given Evas_GL object to destroy
 *
 * @see evas_gl_new
 *
 * @since_tizen 2.3
 */
EAPI void                     evas_gl_free               (Evas_GL *evas_gl) EINA_ARG_NONNULL(1);

/**
 * @brief Allocates a new config object for the user to fill out.
 *
 * @remarks As long as Evas creates a config object for the user, it takes care
 *          of the backward compatibility issue.
 *
 * @see evas_gl_config_free
 *
 * @return A new config object
 *
 * @since_tizen 2.3
 */
EAPI Evas_GL_Config          *evas_gl_config_new         (void);

/**
 * @brief Frees a config object created from evas_gl_config_new.
 *
 * @param[in] cfg  The configuration structure to free, it can not be accessed afterwards.
 *
 * @remarks As long as Evas creates a config object for the user, it takes care
 *          of the backward compatibility issue.
 *
 * @see evas_gl_config_new
 *
 * @since_tizen 2.3
 */
EAPI void                     evas_gl_config_free        (Evas_GL_Config *cfg) EINA_ARG_NONNULL(1);

/**
 * @brief Creates and returns a new @ref Evas_GL_Surface object for GL Rendering.
 *
 * @param[in] evas_gl The given Evas_GL object
 * @param[in] cfg     The pixel format and configuration of the rendering surface
 * @param[in] w       The width of the surface
 * @param[in] h       The height of the surface
 *
 * @return The created GL surface object,
 *         otherwise @c NULL on failure
 *
 * @see evas_gl_surface_destroy
 *
 * @since_tizen 2.3
 */
EAPI Evas_GL_Surface         *evas_gl_surface_create     (Evas_GL *evas_gl, Evas_GL_Config *cfg, int w, int h) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1,2);

/**
 * @brief Create a pixel buffer surface
 *
 * @param[in] evas_gl     The given Evas_GL object
 * @param[in] cfg         Pixel format and configuration of the pixel buffer surface
 * @param[in] w           Requested width of the buffer
 * @param[in] h           Requested height of the buffer
 * @param[in] attrib_list An optional list of attribute-value pairs terminated by attribute 0, can be @c NULL. Currently, no attributes are supported.
 *
 * @return The created GL surface object,
 *         otherwise @c NULL on failure
 *
 * The surface must be released with @ref evas_gl_surface_destroy.
 *
 * If the color format in @a cfg is @ref EVAS_GL_RGB_888 or @ref EVAS_GL_RGBA_8888,
 * then Evas will automatically generate a framebuffer attached to this PBuffer.
 * Its properties can be queried using @ref evas_gl_native_surface_get.
 * If you want to attach an FBO yourself, or create a PBuffer surface only,
 * please use the color format @ref EVAS_GL_NO_FBO.
 *
 * Creating a 1x1 PBuffer surface can be useful in order to call
 * @ref evas_gl_make_current() from another thread.
 *
 * @note The attribute list can be terminated by EVAS_GL_NONE or 0.
 *       As of now, no special attributes are supported yet. Also, only EGL
 *       is supported at the moment of writing.
 *
 * @see evas_gl_surface_destroy
 *
 * @since_tizen 2.3
 */
EAPI Evas_GL_Surface         *evas_gl_pbuffer_surface_create(Evas_GL *evas_gl, Evas_GL_Config *cfg, int w, int h, const int *attrib_list) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1,2);

/**
 * @brief Destroys an Evas GL Surface.
 *
 * @param[in] evas_gl   The given Evas_GL object
 * @param[in] surf      The given GL surface object
 *
 * @note This function can also destroy pbuffer surfaces.
 */
EAPI void                     evas_gl_surface_destroy    (Evas_GL *evas_gl, Evas_GL_Surface *surf) EINA_ARG_NONNULL(1,2);

/**
 * @brief Creates and returns a new Evas GL context object (OpenGL-ES 2.0).
 *
 * @param[in] evas_gl    The given Evas_GL object
 * @param[in] share_ctx  An Evas_GL context to share with the new context
 *
 * The API in use will be an OpenGL-ES 2.0 API (ie. with framebuffers and shaders).
 * Consider calling @ref evas_gl_context_version_create if you need an OpenGL-ES 1.1
 * context instead.
 *
 * @return The created context,
 *         otherwise @c NULL on failure
 *
 * @see evas_gl_context_version_create
 */
EAPI Evas_GL_Context         *evas_gl_context_create     (Evas_GL *evas_gl, Evas_GL_Context *share_ctx) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Creates and returns a new Evas GL context object for OpenGL-ES 1.1 or 2.0.
 *
 * @param[in] evas_gl    The given Evas_GL object
 * @param[in] share_ctx  A context to share (can be NULL)
 * @param[in] version    Major OpenGL-ES version number
 *
 * @return The created context,
 *         otherwise @c NULL on failure
 *
 * This function can be used to create OpenGL-ES 1.1 contexts, but OpenGL-ES 3.x
 * is not supported yet.
 *
 * The GL API bound to the created context must be queried using
 * @ref evas_gl_context_api_get (instead of @ref evas_gl_api_get).
 *
 * @note GLES 3.x is not supported yet.
 *
 * @see Evas_GL_Context_Version
 * @see evas_gl_context_api_get
 *
 * @since_tizen 2.3
 */
EAPI Evas_GL_Context         *evas_gl_context_version_create(Evas_GL *evas_gl, Evas_GL_Context *share_ctx, Evas_GL_Context_Version version) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Destroys the given Evas GL context object.
 *
 * @param[in] evas_gl The given Evas_GL object
 * @param[in] ctx     The given Evas GL context
 *
 * @see evas_gl_context_create
 * @see evas_gl_context_version_create
 *
 * @since_tizen 2.3
 */
EAPI void                     evas_gl_context_destroy    (Evas_GL *evas_gl, Evas_GL_Context *ctx) EINA_ARG_NONNULL(1,2);

/**
 * @brief Sets the given context as the current context for the given surface.
 *
 * @param[in] evas_gl The given Evas_GL object
 * @param[in] surf The given Evas GL surface
 * @param[in] ctx The given Evas GL context
 * @return @c EINA_TRUE if successful,
 *         otherwise @c EINA_FALSE if not
 *
 * @since_tizen 2.3
 */
EAPI Eina_Bool                evas_gl_make_current       (Evas_GL *evas_gl, Evas_GL_Surface *surf, Evas_GL_Context *ctx) EINA_ARG_NONNULL(1,2);

/**
 * @brief Returns a pointer to a static, null-terminated string describing some aspect of Evas GL.
 *
 * @param[in] evas_gl The given Evas_GL object
 * @param[in] name    A symbolic constant, only @ref EVAS_GL_EXTENSIONS is supported for now
 * @return A string describing some aspect of Evas GL
 *
 * @since_tizen 2.3
 */
EAPI const char              *evas_gl_string_query       (Evas_GL *evas_gl, int name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1) EINA_PURE;

/**
 * @brief Returns a extension function from the Evas_GL glue layer.
 *
 * @param[in] evas_gl  The given Evas_GL object
 * @param[in] name     The name of the function to return
 *
 * The available extension functions may depend on the backend engine Evas GL is
 * running on.
 *
 * @note Evas_GL extensions are not EGL or OpenGL extensions, but Evas_GL-specific
 *       features.
 *
 * @return A function pointer to the Evas_GL extension.
 *
 * @since_tizen 2.3
 */
EAPI Evas_GL_Func             evas_gl_proc_address_get   (Evas_GL *evas_gl, const char *name) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1,2) EINA_PURE;

/**
 * @brief Fills in the Native Surface information from a given Evas GL surface.
 *
 * @param[in]  evas_gl The given Evas_GL object
 * @param[in]  surf    The given Evas GL surface to retrieve the Native Surface information from
 * @param[out] ns      The native surface structure that the function fills in
 * @return @c EINA_TRUE if successful,
 *         otherwise @c EINA_FALSE if not
 *
 * @details This function can be called to later set this native surface as
 *          source of an Evas Object Image. Please refer to
 *          @ref evas_object_image_native_surface_set.
 *
 * @since_tizen 2.3
 */
EAPI Eina_Bool                evas_gl_native_surface_get (Evas_GL *evas_gl, Evas_GL_Surface *surf, Evas_Native_Surface *ns) EINA_ARG_NONNULL(1,2,3);

/**
 * @brief Gets the API for rendering using OpenGL.
 *
 * @details This returns a structure that contains all the OpenGL functions you can
 *          use to render in Evas. These functions consist of all the standard
 *          OpenGL-ES2.0 functions and any additional ones that Evas has decided to provide.
 *          This means that if you have your code ported to OpenGL-ES2.0,
 *          it is going to be easy to render to Evas.
 *
 * @param[in] evas_gl The given Evas_GL object
 *
 * @return The API to use
 *
 * @note This function will always return an OpenGL-ES 2.0 API, please use
 *       @ref evas_gl_context_api_get instead to get an OpenGL-ES 1.1 set of APIs.
 *
 * @since_tizen 2.3
 *
 * @see Evas_GL_API
 * @see evas_gl_context_api_get
 *
 */
EAPI Evas_GL_API             *evas_gl_api_get            (Evas_GL *evas_gl) EINA_ARG_NONNULL(1);

/**
 * @brief Gets the API for rendering using OpenGL with non standard contexts.
 *
 * This function is similar to @ref evas_gl_api_get but takes an extra Evas GL
 * context argument as it is used to get the real API used by special contexts,
 * that have been instanciated with @ref evas_gl_context_version_create().
 *
 * This function can be used to get the GL API for a OpenGL-ES 1.1 context. When
 * using OpenGL-ES 1.1, applications should always use @ref evas_gl_context_api_get
 * and never call @ref evas_gl_api_get (this will always return a OpenGL-ES 2+ API).
 *
 * @param[in] evas_gl The given Evas_GL object
 * @param[in] ctx     Specifies which context to use, based on this, Evas GL
 *                    will return a 1.1- or a 2.0-compatible OpenGL-ES API.
 *
 * @return The API to use. Only the available function pointers will be set
 *         in the structure. All the non compatible functions or unsupported
 *         extension function pointers will be set to NULL.
 *
 * @see Evas_GL_API
 * @see evas_gl_api_get
 * @see evas_gl_context_version_create
 *
 * @since_tizen 2.3
 */
EAPI Evas_GL_API             *evas_gl_context_api_get    (Evas_GL *evas_gl, Evas_GL_Context *ctx) EINA_ARG_NONNULL(1);

/**
 * @brief Get the current rotation of the view, in degrees.
 *
 * This function should be called in order to properly handle the current
 * rotation of the view. It will always return 0 unless the option
 * @ref EVAS_GL_OPTIONS_CLIENT_SIDE_ROTATION has been set.
 *
 * Indeed, in case of direct rendering to the back buffer, the client
 * application is responsible for properly rotating its view. This can generally
 * be done by applying a rotation to a view matrix.
 *
 * @param[in] evas_gl    The current Evas_GL object
 *
 * @note The returned value may not be the same as the window rotation, for
 * example if indirect rendering is used as a fallback, or if the GPU supports
 * transparent rotation of the buffer during swap.
 *
 * @return 0, 90, 180 or 270 depending on the Evas canvas' orientation.
 *
 * @see EVAS_GL_OPTIONS_CLIENT_SIDE_ROTATION
 *
 * @since_tizen 2.3
 */
EAPI int                      evas_gl_rotation_get       (Evas_GL *evas_gl) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Query a surface for its properties
 *
 * @param[in]  evas_gl    The current Evas_GL object
 * @param[in]  surface    An Evas_GL_Surface surface to query
 * @param[in]  attribute  Specifies the attribute to query.
 * @param[out] value      Returns the requested value (usually an int)
 *
 * The currently accepted attributes are the following:
 * @li @ref EVAS_GL_WIDTH,
 * @li @ref EVAS_GL_HEIGHT,
 * @li @ref EVAS_GL_TEXTURE_FORMAT,
 * @li @ref EVAS_GL_TEXTURE_TARGET
 *
 * @return EINA_TRUE in case of success, EINA_FALSE in case of error.
 *
 * @since_tizen 2.3
 */
EAPI Eina_Bool                evas_gl_surface_query      (Evas_GL *evas_gl, Evas_GL_Surface *surface, int attribute, void *value) EINA_ARG_NONNULL(1,2);

/**
 * @brief Returns the last error of any evas_gl function called in the current thread.
 *        Initially, the error is set to @ref EVAS_GL_SUCCESS. A call to @ref evas_gl_error_get
 *        resets the error to @ref EVAS_GL_SUCCESS.
 *
 * @param[in] evas_gl The given Evas_GL object
 *
 * @return @ref EVAS_GL_SUCCESS in case of no error, or any other @c EVAS_GL error code.
 *
 * Since Evas GL is a glue layer for GL imitating EGL, the error codes returned
 * have a similar meaning as those defined in EGL, so please refer to the EGL
 * documentation for more information about the various error codes.
 *
 * @note Evas GL does not specify exactly which error codes will be returned in
 *       which circumstances. This is because different backends may behave
 *       differently and Evas GL will try to give the most meaningful error code
 *       based on the backend's error. Evas GL only tries to provide some
 *       information, so an application can not expect the exact same error
 *       codes as EGL would return.
 *
 * @since_tizen 2.3
 */
EAPI int                      evas_gl_error_get          (Evas_GL *evas_gl) EINA_ARG_NONNULL(1);

/**
 * @brief Returns the Evas GL context object in use or set by @ref evas_gl_make_current.
 *
 * @param[in] evas_gl The given Evas_GL object
 *
 * @return The current context for the calling thread, or @c NULL in case of
 *         failure and when there is no current context in this thread.
 *
 * @since_tizen 2.3
 */
EAPI Evas_GL_Context         *evas_gl_current_context_get (Evas_GL *evas_gl) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);

/**
 * @brief Returns the Evas GL surface object in use or set by @ref evas_gl_make_current
 *
 * @param evas_gl The given Evas_GL object
 *
 * @return The current surface for the calling thread, or @c NULL in case of
 *         failure and when there is no current surface in this thread.
 *
 * This can be used to get a handle to the current surface, so as to switch
 * between contexts back and forth. Note that the OpenGL driver may stall when
 * doing so.
 *
 * @see evas_gl_make_current
 *
 * @since_tizen 2.3
 */
EAPI Evas_GL_Surface         *evas_gl_current_surface_get (Evas_GL *evas_gl) EINA_WARN_UNUSED_RESULT EINA_ARG_NONNULL(1);



/*-------------------------------------------------------------------------
 * Data types, definitions and values for use with Evas GL.
 *
 * The following definitions have been imported from the official GLES/GLES2
 * header files. Please do not include the official GL headers along with
 * Evas_GL.h as these will conflict.
 *-----------------------------------------------------------------------*/

#ifndef KHRONOS_SUPPORT_INT64
typedef unsigned long long khronos_uint64_t;
typedef signed long long   khronos_int64_t;
#endif

// Due to build conflicts on various platforms, we can't use GL[u]int64 directly
typedef khronos_int64_t    EvasGLint64;
typedef khronos_uint64_t   EvasGLuint64;

#if !defined(__gl2_h_)
# define __gl2_h_

#define GL_ES_VERSION_2_0 1

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

#ifndef __gl2ext_h_
#define __gl2ext_h_ 1

#ifndef GL_KHR_blend_equation_advanced
#define GL_KHR_blend_equation_advanced 1
#define GL_BLEND_ADVANCED_COHERENT_KHR    0x9285
#define GL_MULTIPLY_KHR                   0x9294
#define GL_SCREEN_KHR                     0x9295
#define GL_OVERLAY_KHR                    0x9296
#define GL_DARKEN_KHR                     0x9297
#define GL_LIGHTEN_KHR                    0x9298
#define GL_COLORDODGE_KHR                 0x9299
#define GL_COLORBURN_KHR                  0x929A
#define GL_HARDLIGHT_KHR                  0x929B
#define GL_SOFTLIGHT_KHR                  0x929C
#define GL_DIFFERENCE_KHR                 0x929E
#define GL_EXCLUSION_KHR                  0x92A0
#define GL_HSL_HUE_KHR                    0x92AD
#define GL_HSL_SATURATION_KHR             0x92AE
#define GL_HSL_COLOR_KHR                  0x92AF
#define GL_HSL_LUMINOSITY_KHR             0x92B0
#endif /* GL_KHR_blend_equation_advanced */

#ifndef GL_KHR_debug
#define GL_KHR_debug 1
#define GL_SAMPLER                        0x82E6
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR   0x8242
#define GL_DEBUG_NEXT_LOGGED_MESSAGE_LENGTH_KHR 0x8243
#define GL_DEBUG_CALLBACK_FUNCTION_KHR    0x8244
#define GL_DEBUG_CALLBACK_USER_PARAM_KHR  0x8245
#define GL_DEBUG_SOURCE_API_KHR           0x8246
#define GL_DEBUG_SOURCE_WINDOW_SYSTEM_KHR 0x8247
#define GL_DEBUG_SOURCE_SHADER_COMPILER_KHR 0x8248
#define GL_DEBUG_SOURCE_THIRD_PARTY_KHR   0x8249
#define GL_DEBUG_SOURCE_APPLICATION_KHR   0x824A
#define GL_DEBUG_SOURCE_OTHER_KHR         0x824B
#define GL_DEBUG_TYPE_ERROR_KHR           0x824C
#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR 0x824D
#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR 0x824E
#define GL_DEBUG_TYPE_PORTABILITY_KHR     0x824F
#define GL_DEBUG_TYPE_PERFORMANCE_KHR     0x8250
#define GL_DEBUG_TYPE_OTHER_KHR           0x8251
#define GL_DEBUG_TYPE_MARKER_KHR          0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP_KHR      0x8269
#define GL_DEBUG_TYPE_POP_GROUP_KHR       0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION_KHR 0x826B
#define GL_MAX_DEBUG_GROUP_STACK_DEPTH_KHR 0x826C
#define GL_DEBUG_GROUP_STACK_DEPTH_KHR    0x826D
#define GL_BUFFER_KHR                     0x82E0
#define GL_SHADER_KHR                     0x82E1
#define GL_PROGRAM_KHR                    0x82E2
#define GL_VERTEX_ARRAY_KHR               0x8074
#define GL_QUERY_KHR                      0x82E3
#define GL_SAMPLER_KHR                    0x82E6
#define GL_MAX_LABEL_LENGTH_KHR           0x82E8
#define GL_MAX_DEBUG_MESSAGE_LENGTH_KHR   0x9143
#define GL_MAX_DEBUG_LOGGED_MESSAGES_KHR  0x9144
#define GL_DEBUG_LOGGED_MESSAGES_KHR      0x9145
#define GL_DEBUG_SEVERITY_HIGH_KHR        0x9146
#define GL_DEBUG_SEVERITY_MEDIUM_KHR      0x9147
#define GL_DEBUG_SEVERITY_LOW_KHR         0x9148
#define GL_DEBUG_OUTPUT_KHR               0x92E0
#define GL_CONTEXT_FLAG_DEBUG_BIT_KHR     0x00000002
#define GL_STACK_OVERFLOW_KHR             0x0503
#define GL_STACK_UNDERFLOW_KHR            0x0504
#endif /* GL_KHR_debug */

#ifndef GL_KHR_texture_compression_astc_hdr
#define GL_KHR_texture_compression_astc_hdr 1
#define GL_COMPRESSED_RGBA_ASTC_4x4_KHR   0x93B0
#define GL_COMPRESSED_RGBA_ASTC_5x4_KHR   0x93B1
#define GL_COMPRESSED_RGBA_ASTC_5x5_KHR   0x93B2
#define GL_COMPRESSED_RGBA_ASTC_6x5_KHR   0x93B3
#define GL_COMPRESSED_RGBA_ASTC_6x6_KHR   0x93B4
#define GL_COMPRESSED_RGBA_ASTC_8x5_KHR   0x93B5
#define GL_COMPRESSED_RGBA_ASTC_8x6_KHR   0x93B6
#define GL_COMPRESSED_RGBA_ASTC_8x8_KHR   0x93B7
#define GL_COMPRESSED_RGBA_ASTC_10x5_KHR  0x93B8
#define GL_COMPRESSED_RGBA_ASTC_10x6_KHR  0x93B9
#define GL_COMPRESSED_RGBA_ASTC_10x8_KHR  0x93BA
#define GL_COMPRESSED_RGBA_ASTC_10x10_KHR 0x93BB
#define GL_COMPRESSED_RGBA_ASTC_12x10_KHR 0x93BC
#define GL_COMPRESSED_RGBA_ASTC_12x12_KHR 0x93BD
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR 0x93D0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR 0x93D1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR 0x93D2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR 0x93D3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR 0x93D4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR 0x93D5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR 0x93D6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR 0x93D7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR 0x93D8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR 0x93D9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR 0x93DA
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR 0x93DB
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR 0x93DC
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR 0x93DD
#endif /* GL_KHR_texture_compression_astc_hdr */

#ifndef GL_KHR_texture_compression_astc_ldr
#define GL_KHR_texture_compression_astc_ldr 1
#endif /* GL_KHR_texture_compression_astc_ldr */

#ifndef GL_OES_EGL_image
#define GL_OES_EGL_image 1
#endif /* GL_OES_EGL_image */

#ifndef GL_OES_EGL_image_external
#define GL_OES_EGL_image_external 1
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#define GL_TEXTURE_BINDING_EXTERNAL_OES   0x8D67
#define GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES 0x8D68
#define GL_SAMPLER_EXTERNAL_OES           0x8D66
#endif /* GL_OES_EGL_image_external */

#ifndef GL_OES_compressed_ETC1_RGB8_texture
#define GL_OES_compressed_ETC1_RGB8_texture 1
#define GL_ETC1_RGB8_OES                  0x8D64
#endif /* GL_OES_compressed_ETC1_RGB8_texture */

#ifndef GL_OES_compressed_paletted_texture
#define GL_OES_compressed_paletted_texture 1
#define GL_PALETTE4_RGB8_OES              0x8B90
#define GL_PALETTE4_RGBA8_OES             0x8B91
#define GL_PALETTE4_R5_G6_B5_OES          0x8B92
#define GL_PALETTE4_RGBA4_OES             0x8B93
#define GL_PALETTE4_RGB5_A1_OES           0x8B94
#define GL_PALETTE8_RGB8_OES              0x8B95
#define GL_PALETTE8_RGBA8_OES             0x8B96
#define GL_PALETTE8_R5_G6_B5_OES          0x8B97
#define GL_PALETTE8_RGBA4_OES             0x8B98
#define GL_PALETTE8_RGB5_A1_OES           0x8B99
#endif /* GL_OES_compressed_paletted_texture */

#ifndef GL_OES_depth24
#define GL_OES_depth24 1
#define GL_DEPTH_COMPONENT24_OES          0x81A6
#endif /* GL_OES_depth24 */

#ifndef GL_OES_depth32
#define GL_OES_depth32 1
#define GL_DEPTH_COMPONENT32_OES          0x81A7
#endif /* GL_OES_depth32 */

#ifndef GL_OES_depth_texture
#define GL_OES_depth_texture 1
#endif /* GL_OES_depth_texture */

#ifndef GL_OES_element_index_uint
#define GL_OES_element_index_uint 1
#endif /* GL_OES_element_index_uint */

#ifndef GL_OES_fbo_render_mipmap
#define GL_OES_fbo_render_mipmap 1
#endif /* GL_OES_fbo_render_mipmap */

#ifndef GL_OES_fragment_precision_high
#define GL_OES_fragment_precision_high 1
#endif /* GL_OES_fragment_precision_high */

#ifndef GL_OES_get_program_binary
#define GL_OES_get_program_binary 1
#define GL_PROGRAM_BINARY_LENGTH_OES      0x8741
#define GL_NUM_PROGRAM_BINARY_FORMATS_OES 0x87FE
#define GL_PROGRAM_BINARY_FORMATS_OES     0x87FF
#endif /* GL_OES_get_program_binary */

#ifndef GL_OES_mapbuffer
#define GL_OES_mapbuffer 1
#define GL_WRITE_ONLY_OES                 0x88B9
#define GL_BUFFER_ACCESS_OES              0x88BB
#define GL_BUFFER_MAPPED_OES              0x88BC
#define GL_BUFFER_MAP_POINTER_OES         0x88BD
#endif /* GL_OES_mapbuffer */

#ifndef GL_OES_packed_depth_stencil
#define GL_OES_packed_depth_stencil 1
#define GL_DEPTH_STENCIL_OES              0x84F9
#define GL_UNSIGNED_INT_24_8_OES          0x84FA
#define GL_DEPTH24_STENCIL8_OES           0x88F0
#endif /* GL_OES_packed_depth_stencil */

#ifndef GL_OES_required_internalformat
#define GL_OES_required_internalformat 1
#define GL_ALPHA8_OES                     0x803C
#define GL_DEPTH_COMPONENT16_OES          0x81A5
#define GL_LUMINANCE4_ALPHA4_OES          0x8043
#define GL_LUMINANCE8_ALPHA8_OES          0x8045
#define GL_LUMINANCE8_OES                 0x8040
#define GL_RGBA4_OES                      0x8056
#define GL_RGB5_A1_OES                    0x8057
#define GL_RGB565_OES                     0x8D62
#define GL_RGB8_OES                       0x8051
#define GL_RGBA8_OES                      0x8058
#define GL_RGB10_EXT                      0x8052
#define GL_RGB10_A2_EXT                   0x8059
#endif /* GL_OES_required_internalformat */

#ifndef GL_OES_rgb8_rgba8
#define GL_OES_rgb8_rgba8 1
#endif /* GL_OES_rgb8_rgba8 */

#ifndef GL_OES_sample_shading
#define GL_OES_sample_shading 1
#define GL_SAMPLE_SHADING_OES             0x8C36
#define GL_MIN_SAMPLE_SHADING_VALUE_OES   0x8C37
#endif /* GL_OES_sample_shading */

#ifndef GL_OES_sample_variables
#define GL_OES_sample_variables 1
#endif /* GL_OES_sample_variables */

#ifndef GL_OES_shader_image_atomic
#define GL_OES_shader_image_atomic 1
#endif /* GL_OES_shader_image_atomic */

#ifndef GL_OES_shader_multisample_interpolation
#define GL_OES_shader_multisample_interpolation 1
#define GL_MIN_FRAGMENT_INTERPOLATION_OFFSET_OES 0x8E5B
#define GL_MAX_FRAGMENT_INTERPOLATION_OFFSET_OES 0x8E5C
#define GL_FRAGMENT_INTERPOLATION_OFFSET_BITS_OES 0x8E5D
#endif /* GL_OES_shader_multisample_interpolation */

#ifndef GL_OES_standard_derivatives
#define GL_OES_standard_derivatives 1
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES 0x8B8B
#endif /* GL_OES_standard_derivatives */

#ifndef GL_OES_stencil1
#define GL_OES_stencil1 1
#define GL_STENCIL_INDEX1_OES             0x8D46
#endif /* GL_OES_stencil1 */

#ifndef GL_OES_stencil4
#define GL_OES_stencil4 1
#define GL_STENCIL_INDEX4_OES             0x8D47
#endif /* GL_OES_stencil4 */

#ifndef GL_OES_surfaceless_context
#define GL_OES_surfaceless_context 1
#define GL_FRAMEBUFFER_UNDEFINED_OES      0x8219
#endif /* GL_OES_surfaceless_context */

#ifndef GL_OES_texture_3D
#define GL_OES_texture_3D 1
#define GL_TEXTURE_WRAP_R_OES             0x8072
#define GL_TEXTURE_3D_OES                 0x806F
#define GL_TEXTURE_BINDING_3D_OES         0x806A
#define GL_MAX_3D_TEXTURE_SIZE_OES        0x8073
#define GL_SAMPLER_3D_OES                 0x8B5F
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_OES 0x8CD4
#endif /* GL_OES_texture_3D */

#ifndef GL_OES_texture_compression_astc
#define GL_OES_texture_compression_astc 1
#define GL_COMPRESSED_RGBA_ASTC_3x3x3_OES 0x93C0
#define GL_COMPRESSED_RGBA_ASTC_4x3x3_OES 0x93C1
#define GL_COMPRESSED_RGBA_ASTC_4x4x3_OES 0x93C2
#define GL_COMPRESSED_RGBA_ASTC_4x4x4_OES 0x93C3
#define GL_COMPRESSED_RGBA_ASTC_5x4x4_OES 0x93C4
#define GL_COMPRESSED_RGBA_ASTC_5x5x4_OES 0x93C5
#define GL_COMPRESSED_RGBA_ASTC_5x5x5_OES 0x93C6
#define GL_COMPRESSED_RGBA_ASTC_6x5x5_OES 0x93C7
#define GL_COMPRESSED_RGBA_ASTC_6x6x5_OES 0x93C8
#define GL_COMPRESSED_RGBA_ASTC_6x6x6_OES 0x93C9
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES 0x93E0
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES 0x93E1
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES 0x93E2
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES 0x93E3
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES 0x93E4
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES 0x93E5
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES 0x93E6
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES 0x93E7
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES 0x93E8
#define GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES 0x93E9
#endif /* GL_OES_texture_compression_astc */

#ifndef GL_OES_texture_float
#define GL_OES_texture_float 1
#endif /* GL_OES_texture_float */

#ifndef GL_OES_texture_float_linear
#define GL_OES_texture_float_linear 1
#endif /* GL_OES_texture_float_linear */

#ifndef GL_OES_texture_half_float
#define GL_OES_texture_half_float 1
#define GL_HALF_FLOAT_OES                 0x8D61
#endif /* GL_OES_texture_half_float */

#ifndef GL_OES_texture_half_float_linear
#define GL_OES_texture_half_float_linear 1
#endif /* GL_OES_texture_half_float_linear */

#ifndef GL_OES_texture_npot
#define GL_OES_texture_npot 1
#endif /* GL_OES_texture_npot */

#ifndef GL_OES_texture_stencil8
#define GL_OES_texture_stencil8 1
#define GL_STENCIL_INDEX_OES              0x1901
#define GL_STENCIL_INDEX8_OES             0x8D48
#endif /* GL_OES_texture_stencil8 */

#ifndef GL_OES_texture_storage_multisample_2d_array
#define GL_OES_texture_storage_multisample_2d_array 1
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY_OES 0x9102
#define GL_TEXTURE_BINDING_2D_MULTISAMPLE_ARRAY_OES 0x9105
#define GL_SAMPLER_2D_MULTISAMPLE_ARRAY_OES 0x910B
#define GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY_OES 0x910C
#define GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY_OES 0x910D
#endif /* GL_OES_texture_storage_multisample_2d_array */

#ifndef GL_OES_vertex_array_object
#define GL_OES_vertex_array_object 1
#define GL_VERTEX_ARRAY_BINDING_OES       0x85B5
#endif /* GL_OES_vertex_array_object */

#ifndef GL_OES_vertex_half_float
#define GL_OES_vertex_half_float 1
#endif /* GL_OES_vertex_half_float */

#ifndef GL_OES_vertex_type_10_10_10_2
#define GL_OES_vertex_type_10_10_10_2 1
#define GL_UNSIGNED_INT_10_10_10_2_OES    0x8DF6
#define GL_INT_10_10_10_2_OES             0x8DF7
#endif /* GL_OES_vertex_type_10_10_10_2 */

#ifndef GL_AMD_compressed_3DC_texture
#define GL_AMD_compressed_3DC_texture 1
#define GL_3DC_X_AMD                      0x87F9
#define GL_3DC_XY_AMD                     0x87FA
#endif /* GL_AMD_compressed_3DC_texture */

#ifndef GL_AMD_compressed_ATC_texture
#define GL_AMD_compressed_ATC_texture 1
#define GL_ATC_RGB_AMD                    0x8C92
#define GL_ATC_RGBA_EXPLICIT_ALPHA_AMD    0x8C93
#define GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD 0x87EE
#endif /* GL_AMD_compressed_ATC_texture */

#ifndef GL_AMD_performance_monitor
#define GL_AMD_performance_monitor 1
#define GL_COUNTER_TYPE_AMD               0x8BC0
#define GL_COUNTER_RANGE_AMD              0x8BC1
#define GL_UNSIGNED_INT64_AMD             0x8BC2
#define GL_PERCENTAGE_AMD                 0x8BC3
#define GL_PERFMON_RESULT_AVAILABLE_AMD   0x8BC4
#define GL_PERFMON_RESULT_SIZE_AMD        0x8BC5
#define GL_PERFMON_RESULT_AMD             0x8BC6
#endif /* GL_AMD_performance_monitor */

#ifndef GL_AMD_program_binary_Z400
#define GL_AMD_program_binary_Z400 1
#define GL_Z400_BINARY_AMD                0x8740
#endif /* GL_AMD_program_binary_Z400 */

#ifndef GL_ANGLE_depth_texture
#define GL_ANGLE_depth_texture 1
#endif /* GL_ANGLE_depth_texture */

#ifndef GL_ANGLE_framebuffer_blit
#define GL_ANGLE_framebuffer_blit 1
#define GL_READ_FRAMEBUFFER_ANGLE         0x8CA8
#define GL_DRAW_FRAMEBUFFER_ANGLE         0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING_ANGLE 0x8CA6
#define GL_READ_FRAMEBUFFER_BINDING_ANGLE 0x8CAA
#endif /* GL_ANGLE_framebuffer_blit */

#ifndef GL_ANGLE_framebuffer_multisample
#define GL_ANGLE_framebuffer_multisample 1
#define GL_RENDERBUFFER_SAMPLES_ANGLE     0x8CAB
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_ANGLE 0x8D56
#define GL_MAX_SAMPLES_ANGLE              0x8D57
#endif /* GL_ANGLE_framebuffer_multisample */

#ifndef GL_ANGLE_instanced_arrays
#define GL_ANGLE_instanced_arrays 1
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE 0x88FE
#endif /* GL_ANGLE_instanced_arrays */

#ifndef GL_ANGLE_pack_reverse_row_order
#define GL_ANGLE_pack_reverse_row_order 1
#define GL_PACK_REVERSE_ROW_ORDER_ANGLE   0x93A4
#endif /* GL_ANGLE_pack_reverse_row_order */

#ifndef GL_ANGLE_program_binary
#define GL_ANGLE_program_binary 1
#define GL_PROGRAM_BINARY_ANGLE           0x93A6
#endif /* GL_ANGLE_program_binary */

#ifndef GL_ANGLE_texture_compression_dxt3
#define GL_ANGLE_texture_compression_dxt3 1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE 0x83F2
#endif /* GL_ANGLE_texture_compression_dxt3 */

#ifndef GL_ANGLE_texture_compression_dxt5
#define GL_ANGLE_texture_compression_dxt5 1
#define GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE 0x83F3
#endif /* GL_ANGLE_texture_compression_dxt5 */

#ifndef GL_ANGLE_texture_usage
#define GL_ANGLE_texture_usage 1
#define GL_TEXTURE_USAGE_ANGLE            0x93A2
#define GL_FRAMEBUFFER_ATTACHMENT_ANGLE   0x93A3
#endif /* GL_ANGLE_texture_usage */

#ifndef GL_ANGLE_translated_shader_source
#define GL_ANGLE_translated_shader_source 1
#define GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE 0x93A0
#endif /* GL_ANGLE_translated_shader_source */

#ifndef GL_APPLE_copy_texture_levels
#define GL_APPLE_copy_texture_levels 1
#endif /* GL_APPLE_copy_texture_levels */

#ifndef GL_APPLE_framebuffer_multisample
#define GL_APPLE_framebuffer_multisample 1
#define GL_RENDERBUFFER_SAMPLES_APPLE     0x8CAB
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_APPLE 0x8D56
#define GL_MAX_SAMPLES_APPLE              0x8D57
#define GL_READ_FRAMEBUFFER_APPLE         0x8CA8
#define GL_DRAW_FRAMEBUFFER_APPLE         0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING_APPLE 0x8CA6
#define GL_READ_FRAMEBUFFER_BINDING_APPLE 0x8CAA
#endif /* GL_APPLE_framebuffer_multisample */

#ifndef GL_APPLE_rgb_422
#define GL_APPLE_rgb_422 1
#define GL_RGB_422_APPLE                  0x8A1F
#define GL_UNSIGNED_SHORT_8_8_APPLE       0x85BA
#define GL_UNSIGNED_SHORT_8_8_REV_APPLE   0x85BB
#define GL_RGB_RAW_422_APPLE              0x8A51
#endif /* GL_APPLE_rgb_422 */

#ifndef GL_APPLE_sync
#define GL_APPLE_sync 1
#define GL_SYNC_OBJECT_APPLE              0x8A53
#define GL_MAX_SERVER_WAIT_TIMEOUT_APPLE  0x9111
#define GL_OBJECT_TYPE_APPLE              0x9112
#define GL_SYNC_CONDITION_APPLE           0x9113
#define GL_SYNC_STATUS_APPLE              0x9114
#define GL_SYNC_FLAGS_APPLE               0x9115
#define GL_SYNC_FENCE_APPLE               0x9116
#define GL_SYNC_GPU_COMMANDS_COMPLETE_APPLE 0x9117
#define GL_UNSIGNALED_APPLE               0x9118
#define GL_SIGNALED_APPLE                 0x9119
#define GL_ALREADY_SIGNALED_APPLE         0x911A
#define GL_TIMEOUT_EXPIRED_APPLE          0x911B
#define GL_CONDITION_SATISFIED_APPLE      0x911C
#define GL_WAIT_FAILED_APPLE              0x911D
#define GL_SYNC_FLUSH_COMMANDS_BIT_APPLE  0x00000001
#define GL_TIMEOUT_IGNORED_APPLE          0xFFFFFFFFFFFFFFFFull
#endif /* GL_APPLE_sync */

#ifndef GL_APPLE_texture_format_BGRA8888
#define GL_APPLE_texture_format_BGRA8888 1
#define GL_BGRA_EXT                       0x80E1
#define GL_BGRA8_EXT                      0x93A1
#endif /* GL_APPLE_texture_format_BGRA8888 */

#ifndef GL_APPLE_texture_max_level
#define GL_APPLE_texture_max_level 1
#define GL_TEXTURE_MAX_LEVEL_APPLE        0x813D
#endif /* GL_APPLE_texture_max_level */

#ifndef GL_ARM_mali_program_binary
#define GL_ARM_mali_program_binary 1
#define GL_MALI_PROGRAM_BINARY_ARM        0x8F61
#endif /* GL_ARM_mali_program_binary */

#ifndef GL_ARM_mali_shader_binary
#define GL_ARM_mali_shader_binary 1
#define GL_MALI_SHADER_BINARY_ARM         0x8F60
#endif /* GL_ARM_mali_shader_binary */

#ifndef GL_ARM_rgba8
#define GL_ARM_rgba8 1
#endif /* GL_ARM_rgba8 */

#ifndef GL_ARM_shader_framebuffer_fetch
#define GL_ARM_shader_framebuffer_fetch 1
#define GL_FETCH_PER_SAMPLE_ARM           0x8F65
#define GL_FRAGMENT_SHADER_FRAMEBUFFER_FETCH_MRT_ARM 0x8F66
#endif /* GL_ARM_shader_framebuffer_fetch */

#ifndef GL_ARM_shader_framebuffer_fetch_depth_stencil
#define GL_ARM_shader_framebuffer_fetch_depth_stencil 1
#endif /* GL_ARM_shader_framebuffer_fetch_depth_stencil */

#ifndef GL_DMP_shader_binary
#define GL_DMP_shader_binary 1
#define GL_SHADER_BINARY_DMP              0x9250
#endif /* GL_DMP_shader_binary */

#ifndef GL_EXT_blend_minmax
#define GL_EXT_blend_minmax 1
#define GL_MIN_EXT                        0x8007
#define GL_MAX_EXT                        0x8008
#endif /* GL_EXT_blend_minmax */

#ifndef GL_EXT_color_buffer_half_float
#define GL_EXT_color_buffer_half_float 1
#define GL_RGBA16F_EXT                    0x881A
#define GL_RGB16F_EXT                     0x881B
#define GL_RG16F_EXT                      0x822F
#define GL_R16F_EXT                       0x822D
#define GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE_EXT 0x8211
#define GL_UNSIGNED_NORMALIZED_EXT        0x8C17
#endif /* GL_EXT_color_buffer_half_float */

#ifndef GL_EXT_debug_label
#define GL_EXT_debug_label 1
#define GL_PROGRAM_PIPELINE_OBJECT_EXT    0x8A4F
#define GL_PROGRAM_OBJECT_EXT             0x8B40
#define GL_SHADER_OBJECT_EXT              0x8B48
#define GL_BUFFER_OBJECT_EXT              0x9151
#define GL_QUERY_OBJECT_EXT               0x9153
#define GL_VERTEX_ARRAY_OBJECT_EXT        0x9154
#define GL_TRANSFORM_FEEDBACK             0x8E22
#endif /* GL_EXT_debug_label */

#ifndef GL_EXT_debug_marker
#define GL_EXT_debug_marker 1
#endif /* GL_EXT_debug_marker */

#ifndef GL_EXT_discard_framebuffer
#define GL_EXT_discard_framebuffer 1
#define GL_COLOR_EXT                      0x1800
#define GL_DEPTH_EXT                      0x1801
#define GL_STENCIL_EXT                    0x1802
#endif /* GL_EXT_discard_framebuffer */

#ifndef GL_EXT_disjoint_timer_query
#define GL_EXT_disjoint_timer_query 1
#define GL_QUERY_COUNTER_BITS_EXT         0x8864
#define GL_CURRENT_QUERY_EXT              0x8865
#define GL_QUERY_RESULT_EXT               0x8866
#define GL_QUERY_RESULT_AVAILABLE_EXT     0x8867
#define GL_TIME_ELAPSED_EXT               0x88BF
#define GL_TIMESTAMP_EXT                  0x8E28
#define GL_GPU_DISJOINT_EXT               0x8FBB
#endif /* GL_EXT_disjoint_timer_query */

#ifndef GL_EXT_draw_buffers
#define GL_EXT_draw_buffers 1
#define GL_MAX_COLOR_ATTACHMENTS_EXT      0x8CDF
#define GL_MAX_DRAW_BUFFERS_EXT           0x8824
#define GL_DRAW_BUFFER0_EXT               0x8825
#define GL_DRAW_BUFFER1_EXT               0x8826
#define GL_DRAW_BUFFER2_EXT               0x8827
#define GL_DRAW_BUFFER3_EXT               0x8828
#define GL_DRAW_BUFFER4_EXT               0x8829
#define GL_DRAW_BUFFER5_EXT               0x882A
#define GL_DRAW_BUFFER6_EXT               0x882B
#define GL_DRAW_BUFFER7_EXT               0x882C
#define GL_DRAW_BUFFER8_EXT               0x882D
#define GL_DRAW_BUFFER9_EXT               0x882E
#define GL_DRAW_BUFFER10_EXT              0x882F
#define GL_DRAW_BUFFER11_EXT              0x8830
#define GL_DRAW_BUFFER12_EXT              0x8831
#define GL_DRAW_BUFFER13_EXT              0x8832
#define GL_DRAW_BUFFER14_EXT              0x8833
#define GL_DRAW_BUFFER15_EXT              0x8834
#define GL_COLOR_ATTACHMENT0_EXT          0x8CE0
#define GL_COLOR_ATTACHMENT1_EXT          0x8CE1
#define GL_COLOR_ATTACHMENT2_EXT          0x8CE2
#define GL_COLOR_ATTACHMENT3_EXT          0x8CE3
#define GL_COLOR_ATTACHMENT4_EXT          0x8CE4
#define GL_COLOR_ATTACHMENT5_EXT          0x8CE5
#define GL_COLOR_ATTACHMENT6_EXT          0x8CE6
#define GL_COLOR_ATTACHMENT7_EXT          0x8CE7
#define GL_COLOR_ATTACHMENT8_EXT          0x8CE8
#define GL_COLOR_ATTACHMENT9_EXT          0x8CE9
#define GL_COLOR_ATTACHMENT10_EXT         0x8CEA
#define GL_COLOR_ATTACHMENT11_EXT         0x8CEB
#define GL_COLOR_ATTACHMENT12_EXT         0x8CEC
#define GL_COLOR_ATTACHMENT13_EXT         0x8CED
#define GL_COLOR_ATTACHMENT14_EXT         0x8CEE
#define GL_COLOR_ATTACHMENT15_EXT         0x8CEF
#endif /* GL_EXT_draw_buffers */

#ifndef GL_EXT_draw_instanced
#define GL_EXT_draw_instanced 1
#endif /* GL_EXT_draw_instanced */

#ifndef GL_EXT_instanced_arrays
#define GL_EXT_instanced_arrays 1
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_EXT 0x88FE
#endif /* GL_EXT_instanced_arrays */

#ifndef GL_EXT_map_buffer_range
#define GL_EXT_map_buffer_range 1
#define GL_MAP_READ_BIT_EXT               0x0001
#define GL_MAP_WRITE_BIT_EXT              0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT_EXT   0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT_EXT  0x0008
#define GL_MAP_FLUSH_EXPLICIT_BIT_EXT     0x0010
#define GL_MAP_UNSYNCHRONIZED_BIT_EXT     0x0020
#endif /* GL_EXT_map_buffer_range */

#ifndef GL_EXT_multi_draw_arrays
#define GL_EXT_multi_draw_arrays 1
#endif /* GL_EXT_multi_draw_arrays */

#ifndef GL_EXT_multisampled_render_to_texture
#define GL_EXT_multisampled_render_to_texture 1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_SAMPLES_EXT 0x8D6C
#define GL_RENDERBUFFER_SAMPLES_EXT       0x8CAB
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT 0x8D56
#define GL_MAX_SAMPLES_EXT                0x8D57
#endif /* GL_EXT_multisampled_render_to_texture */

#ifndef GL_EXT_multiview_draw_buffers
#define GL_EXT_multiview_draw_buffers 1
#define GL_COLOR_ATTACHMENT_EXT           0x90F0
#define GL_MULTIVIEW_EXT                  0x90F1
#define GL_DRAW_BUFFER_EXT                0x0C01
#define GL_READ_BUFFER_EXT                0x0C02
#define GL_MAX_MULTIVIEW_BUFFERS_EXT      0x90F2
#endif /* GL_EXT_multiview_draw_buffers */

#ifndef GL_EXT_occlusion_query_boolean
#define GL_EXT_occlusion_query_boolean 1
#define GL_ANY_SAMPLES_PASSED_EXT         0x8C2F
#define GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT 0x8D6A
#endif /* GL_EXT_occlusion_query_boolean */

#ifndef GL_EXT_pvrtc_sRGB
#define GL_EXT_pvrtc_sRGB 1
#define GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT 0x8A54
#define GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT 0x8A55
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT 0x8A56
#define GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT 0x8A57
#endif /* GL_EXT_pvrtc_sRGB */

#ifndef GL_EXT_read_format_bgra
#define GL_EXT_read_format_bgra 1
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT 0x8365
#define GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT 0x8366
#endif /* GL_EXT_read_format_bgra */

#ifndef GL_EXT_robustness
#define GL_EXT_robustness 1
#define GL_GUILTY_CONTEXT_RESET_EXT       0x8253
#define GL_INNOCENT_CONTEXT_RESET_EXT     0x8254
#define GL_UNKNOWN_CONTEXT_RESET_EXT      0x8255
#define GL_CONTEXT_ROBUST_ACCESS_EXT      0x90F3
#define GL_RESET_NOTIFICATION_STRATEGY_EXT 0x8256
#define GL_LOSE_CONTEXT_ON_RESET_EXT      0x8252
#define GL_NO_RESET_NOTIFICATION_EXT      0x8261
#endif /* GL_EXT_robustness */

#ifndef GL_EXT_sRGB
#define GL_EXT_sRGB 1
#define GL_SRGB_EXT                       0x8C40
#define GL_SRGB_ALPHA_EXT                 0x8C42
#define GL_SRGB8_ALPHA8_EXT               0x8C43
#define GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT 0x8210
#endif /* GL_EXT_sRGB */

#ifndef GL_EXT_sRGB_write_control
#define GL_EXT_sRGB_write_control 1
#define GL_FRAMEBUFFER_SRGB_EXT           0x8DB9
#endif /* GL_EXT_sRGB_write_control */

#ifndef GL_EXT_separate_shader_objects
#define GL_EXT_separate_shader_objects 1
#define GL_ACTIVE_PROGRAM_EXT             0x8259
#define GL_VERTEX_SHADER_BIT_EXT          0x00000001
#define GL_FRAGMENT_SHADER_BIT_EXT        0x00000002
#define GL_ALL_SHADER_BITS_EXT            0xFFFFFFFF
#define GL_PROGRAM_SEPARABLE_EXT          0x8258
#define GL_PROGRAM_PIPELINE_BINDING_EXT   0x825A
#endif /* GL_EXT_separate_shader_objects */

#ifndef GL_EXT_shader_framebuffer_fetch
#define GL_EXT_shader_framebuffer_fetch 1
#define GL_FRAGMENT_SHADER_DISCARDS_SAMPLES_EXT 0x8A52
#endif /* GL_EXT_shader_framebuffer_fetch */

#ifndef GL_EXT_shader_integer_mix
#define GL_EXT_shader_integer_mix 1
#endif /* GL_EXT_shader_integer_mix */

#ifndef GL_EXT_shader_pixel_local_storage
#define GL_EXT_shader_pixel_local_storage 1
#define GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_FAST_SIZE_EXT 0x8F63
#define GL_MAX_SHADER_PIXEL_LOCAL_STORAGE_SIZE_EXT 0x8F67
#define GL_SHADER_PIXEL_LOCAL_STORAGE_EXT 0x8F64
#endif /* GL_EXT_shader_pixel_local_storage */

#ifndef GL_EXT_shader_texture_lod
#define GL_EXT_shader_texture_lod 1
#endif /* GL_EXT_shader_texture_lod */

#ifndef GL_EXT_shadow_samplers
#define GL_EXT_shadow_samplers 1
#define GL_TEXTURE_COMPARE_MODE_EXT       0x884C
#define GL_TEXTURE_COMPARE_FUNC_EXT       0x884D
#define GL_COMPARE_REF_TO_TEXTURE_EXT     0x884E
#define GL_SAMPLER_2D_SHADOW_EXT          0x8B62
#endif /* GL_EXT_shadow_samplers */

#ifndef GL_EXT_texture_compression_dxt1
#define GL_EXT_texture_compression_dxt1 1
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#endif /* GL_EXT_texture_compression_dxt1 */

#ifndef GL_EXT_texture_compression_s3tc
#define GL_EXT_texture_compression_s3tc 1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#endif /* GL_EXT_texture_compression_s3tc */

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_EXT_texture_filter_anisotropic 1
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif /* GL_EXT_texture_filter_anisotropic */

#ifndef GL_EXT_texture_format_BGRA8888
#define GL_EXT_texture_format_BGRA8888 1
#endif /* GL_EXT_texture_format_BGRA8888 */

#ifndef GL_EXT_texture_rg
#define GL_EXT_texture_rg 1
#define GL_RED_EXT                        0x1903
#define GL_RG_EXT                         0x8227
#define GL_R8_EXT                         0x8229
#define GL_RG8_EXT                        0x822B
#endif /* GL_EXT_texture_rg */

#ifndef GL_EXT_texture_sRGB_decode
#define GL_EXT_texture_sRGB_decode 1
#define GL_TEXTURE_SRGB_DECODE_EXT        0x8A48
#define GL_DECODE_EXT                     0x8A49
#define GL_SKIP_DECODE_EXT                0x8A4A
#endif /* GL_EXT_texture_sRGB_decode */

#ifndef GL_EXT_texture_storage
#define GL_EXT_texture_storage 1
#define GL_TEXTURE_IMMUTABLE_FORMAT_EXT   0x912F
#define GL_ALPHA8_EXT                     0x803C
#define GL_LUMINANCE8_EXT                 0x8040
#define GL_LUMINANCE8_ALPHA8_EXT          0x8045
#define GL_RGBA32F_EXT                    0x8814
#define GL_RGB32F_EXT                     0x8815
#define GL_ALPHA32F_EXT                   0x8816
#define GL_LUMINANCE32F_EXT               0x8818
#define GL_LUMINANCE_ALPHA32F_EXT         0x8819
#define GL_ALPHA16F_EXT                   0x881C
#define GL_LUMINANCE16F_EXT               0x881E
#define GL_LUMINANCE_ALPHA16F_EXT         0x881F
#define GL_R32F_EXT                       0x822E
#define GL_RG32F_EXT                      0x8230
#endif /* GL_EXT_texture_storage */

#ifndef GL_EXT_texture_type_2_10_10_10_REV
#define GL_EXT_texture_type_2_10_10_10_REV 1
#define GL_UNSIGNED_INT_2_10_10_10_REV_EXT 0x8368
#endif /* GL_EXT_texture_type_2_10_10_10_REV */

#ifndef GL_EXT_unpack_subimage
#define GL_EXT_unpack_subimage 1
#define GL_UNPACK_ROW_LENGTH_EXT          0x0CF2
#define GL_UNPACK_SKIP_ROWS_EXT           0x0CF3
#define GL_UNPACK_SKIP_PIXELS_EXT         0x0CF4
#endif /* GL_EXT_unpack_subimage */

#ifndef GL_FJ_shader_binary_GCCSO
#define GL_FJ_shader_binary_GCCSO 1
#define GL_GCCSO_SHADER_BINARY_FJ         0x9260
#endif /* GL_FJ_shader_binary_GCCSO */

#ifndef GL_IMG_multisampled_render_to_texture
#define GL_IMG_multisampled_render_to_texture 1
#define GL_RENDERBUFFER_SAMPLES_IMG       0x9133
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_IMG 0x9134
#define GL_MAX_SAMPLES_IMG                0x9135
#define GL_TEXTURE_SAMPLES_IMG            0x9136
#endif /* GL_IMG_multisampled_render_to_texture */

#ifndef GL_IMG_program_binary
#define GL_IMG_program_binary 1
#define GL_SGX_PROGRAM_BINARY_IMG         0x9130
#endif /* GL_IMG_program_binary */

#ifndef GL_IMG_read_format
#define GL_IMG_read_format 1
#define GL_BGRA_IMG                       0x80E1
#define GL_UNSIGNED_SHORT_4_4_4_4_REV_IMG 0x8365
#endif /* GL_IMG_read_format */

#ifndef GL_IMG_shader_binary
#define GL_IMG_shader_binary 1
#define GL_SGX_BINARY_IMG                 0x8C0A
#endif /* GL_IMG_shader_binary */

#ifndef GL_IMG_texture_compression_pvrtc
#define GL_IMG_texture_compression_pvrtc 1
#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG 0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG 0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG 0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG 0x8C03
#endif /* GL_IMG_texture_compression_pvrtc */

#ifndef GL_IMG_texture_compression_pvrtc2
#define GL_IMG_texture_compression_pvrtc2 1
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG 0x9137
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG 0x9138
#endif /* GL_IMG_texture_compression_pvrtc2 */

#ifndef GL_INTEL_performance_query
#define GL_INTEL_performance_query 1
#define GL_PERFQUERY_SINGLE_CONTEXT_INTEL 0x00000000
#define GL_PERFQUERY_GLOBAL_CONTEXT_INTEL 0x00000001
#define GL_PERFQUERY_WAIT_INTEL           0x83FB
#define GL_PERFQUERY_FLUSH_INTEL          0x83FA
#define GL_PERFQUERY_DONOT_FLUSH_INTEL    0x83F9
#define GL_PERFQUERY_COUNTER_EVENT_INTEL  0x94F0
#define GL_PERFQUERY_COUNTER_DURATION_NORM_INTEL 0x94F1
#define GL_PERFQUERY_COUNTER_DURATION_RAW_INTEL 0x94F2
#define GL_PERFQUERY_COUNTER_THROUGHPUT_INTEL 0x94F3
#define GL_PERFQUERY_COUNTER_RAW_INTEL    0x94F4
#define GL_PERFQUERY_COUNTER_TIMESTAMP_INTEL 0x94F5
#define GL_PERFQUERY_COUNTER_DATA_UINT32_INTEL 0x94F8
#define GL_PERFQUERY_COUNTER_DATA_UINT64_INTEL 0x94F9
#define GL_PERFQUERY_COUNTER_DATA_FLOAT_INTEL 0x94FA
#define GL_PERFQUERY_COUNTER_DATA_DOUBLE_INTEL 0x94FB
#define GL_PERFQUERY_COUNTER_DATA_BOOL32_INTEL 0x94FC
#define GL_PERFQUERY_QUERY_NAME_LENGTH_MAX_INTEL 0x94FD
#define GL_PERFQUERY_COUNTER_NAME_LENGTH_MAX_INTEL 0x94FE
#define GL_PERFQUERY_COUNTER_DESC_LENGTH_MAX_INTEL 0x94FF
#define GL_PERFQUERY_GPA_EXTENDED_COUNTERS_INTEL 0x9500
#endif /* GL_INTEL_performance_query */

#ifndef GL_NV_blend_equation_advanced
#define GL_NV_blend_equation_advanced 1
#define GL_BLEND_OVERLAP_NV               0x9281
#define GL_BLEND_PREMULTIPLIED_SRC_NV     0x9280
#define GL_BLUE_NV                        0x1905
#define GL_COLORBURN_NV                   0x929A
#define GL_COLORDODGE_NV                  0x9299
#define GL_CONJOINT_NV                    0x9284
#define GL_CONTRAST_NV                    0x92A1
#define GL_DARKEN_NV                      0x9297
#define GL_DIFFERENCE_NV                  0x929E
#define GL_DISJOINT_NV                    0x9283
#define GL_DST_ATOP_NV                    0x928F
#define GL_DST_IN_NV                      0x928B
#define GL_DST_NV                         0x9287
#define GL_DST_OUT_NV                     0x928D
#define GL_DST_OVER_NV                    0x9289
#define GL_EXCLUSION_NV                   0x92A0
#define GL_GREEN_NV                       0x1904
#define GL_HARDLIGHT_NV                   0x929B
#define GL_HARDMIX_NV                     0x92A9
#define GL_HSL_COLOR_NV                   0x92AF
#define GL_HSL_HUE_NV                     0x92AD
#define GL_HSL_LUMINOSITY_NV              0x92B0
#define GL_HSL_SATURATION_NV              0x92AE
#define GL_INVERT_OVG_NV                  0x92B4
#define GL_INVERT_RGB_NV                  0x92A3
#define GL_LIGHTEN_NV                     0x9298
#define GL_LINEARBURN_NV                  0x92A5
#define GL_LINEARDODGE_NV                 0x92A4
#define GL_LINEARLIGHT_NV                 0x92A7
#define GL_MINUS_CLAMPED_NV               0x92B3
#define GL_MINUS_NV                       0x929F
#define GL_MULTIPLY_NV                    0x9294
#define GL_OVERLAY_NV                     0x9296
#define GL_PINLIGHT_NV                    0x92A8
#define GL_PLUS_CLAMPED_ALPHA_NV          0x92B2
#define GL_PLUS_CLAMPED_NV                0x92B1
#define GL_PLUS_DARKER_NV                 0x9292
#define GL_PLUS_NV                        0x9291
#define GL_RED_NV                         0x1903
#define GL_SCREEN_NV                      0x9295
#define GL_SOFTLIGHT_NV                   0x929C
#define GL_SRC_ATOP_NV                    0x928E
#define GL_SRC_IN_NV                      0x928A
#define GL_SRC_NV                         0x9286
#define GL_SRC_OUT_NV                     0x928C
#define GL_SRC_OVER_NV                    0x9288
#define GL_UNCORRELATED_NV                0x9282
#define GL_VIVIDLIGHT_NV                  0x92A6
#define GL_XOR_NV                         0x1506
#endif /* GL_NV_blend_equation_advanced */

#ifndef GL_NV_blend_equation_advanced_coherent
#define GL_NV_blend_equation_advanced_coherent 1
#define GL_BLEND_ADVANCED_COHERENT_NV     0x9285
#endif /* GL_NV_blend_equation_advanced_coherent */

#ifndef GL_NV_copy_buffer
#define GL_NV_copy_buffer 1
#define GL_COPY_READ_BUFFER_NV            0x8F36
#define GL_COPY_WRITE_BUFFER_NV           0x8F37
#endif /* GL_NV_copy_buffer */

#ifndef GL_NV_coverage_sample
#define GL_NV_coverage_sample 1
#define GL_COVERAGE_COMPONENT_NV          0x8ED0
#define GL_COVERAGE_COMPONENT4_NV         0x8ED1
#define GL_COVERAGE_ATTACHMENT_NV         0x8ED2
#define GL_COVERAGE_BUFFERS_NV            0x8ED3
#define GL_COVERAGE_SAMPLES_NV            0x8ED4
#define GL_COVERAGE_ALL_FRAGMENTS_NV      0x8ED5
#define GL_COVERAGE_EDGE_FRAGMENTS_NV     0x8ED6
#define GL_COVERAGE_AUTOMATIC_NV          0x8ED7
#define GL_COVERAGE_BUFFER_BIT_NV         0x00008000
#endif /* GL_NV_coverage_sample */

#ifndef GL_NV_depth_nonlinear
#define GL_NV_depth_nonlinear 1
#define GL_DEPTH_COMPONENT16_NONLINEAR_NV 0x8E2C
#endif /* GL_NV_depth_nonlinear */

#ifndef GL_NV_draw_buffers
#define GL_NV_draw_buffers 1
#define GL_MAX_DRAW_BUFFERS_NV            0x8824
#define GL_DRAW_BUFFER0_NV                0x8825
#define GL_DRAW_BUFFER1_NV                0x8826
#define GL_DRAW_BUFFER2_NV                0x8827
#define GL_DRAW_BUFFER3_NV                0x8828
#define GL_DRAW_BUFFER4_NV                0x8829
#define GL_DRAW_BUFFER5_NV                0x882A
#define GL_DRAW_BUFFER6_NV                0x882B
#define GL_DRAW_BUFFER7_NV                0x882C
#define GL_DRAW_BUFFER8_NV                0x882D
#define GL_DRAW_BUFFER9_NV                0x882E
#define GL_DRAW_BUFFER10_NV               0x882F
#define GL_DRAW_BUFFER11_NV               0x8830
#define GL_DRAW_BUFFER12_NV               0x8831
#define GL_DRAW_BUFFER13_NV               0x8832
#define GL_DRAW_BUFFER14_NV               0x8833
#define GL_DRAW_BUFFER15_NV               0x8834
#define GL_COLOR_ATTACHMENT0_NV           0x8CE0
#define GL_COLOR_ATTACHMENT1_NV           0x8CE1
#define GL_COLOR_ATTACHMENT2_NV           0x8CE2
#define GL_COLOR_ATTACHMENT3_NV           0x8CE3
#define GL_COLOR_ATTACHMENT4_NV           0x8CE4
#define GL_COLOR_ATTACHMENT5_NV           0x8CE5
#define GL_COLOR_ATTACHMENT6_NV           0x8CE6
#define GL_COLOR_ATTACHMENT7_NV           0x8CE7
#define GL_COLOR_ATTACHMENT8_NV           0x8CE8
#define GL_COLOR_ATTACHMENT9_NV           0x8CE9
#define GL_COLOR_ATTACHMENT10_NV          0x8CEA
#define GL_COLOR_ATTACHMENT11_NV          0x8CEB
#define GL_COLOR_ATTACHMENT12_NV          0x8CEC
#define GL_COLOR_ATTACHMENT13_NV          0x8CED
#define GL_COLOR_ATTACHMENT14_NV          0x8CEE
#define GL_COLOR_ATTACHMENT15_NV          0x8CEF
#endif /* GL_NV_draw_buffers */

#ifndef GL_NV_draw_instanced
#define GL_NV_draw_instanced 1
#endif /* GL_NV_draw_instanced */

#ifndef GL_NV_explicit_attrib_location
#define GL_NV_explicit_attrib_location 1
#endif /* GL_NV_explicit_attrib_location */

#ifndef GL_NV_fbo_color_attachments
#define GL_NV_fbo_color_attachments 1
#define GL_MAX_COLOR_ATTACHMENTS_NV       0x8CDF
#endif /* GL_NV_fbo_color_attachments */

#ifndef GL_NV_fence
#define GL_NV_fence 1
#define GL_ALL_COMPLETED_NV               0x84F2
#define GL_FENCE_STATUS_NV                0x84F3
#define GL_FENCE_CONDITION_NV             0x84F4
#endif /* GL_NV_fence */

#ifndef GL_NV_framebuffer_blit
#define GL_NV_framebuffer_blit 1
#define GL_READ_FRAMEBUFFER_NV            0x8CA8
#define GL_DRAW_FRAMEBUFFER_NV            0x8CA9
#define GL_DRAW_FRAMEBUFFER_BINDING_NV    0x8CA6
#define GL_READ_FRAMEBUFFER_BINDING_NV    0x8CAA
#endif /* GL_NV_framebuffer_blit */

#ifndef GL_NV_framebuffer_multisample
#define GL_NV_framebuffer_multisample 1
#define GL_RENDERBUFFER_SAMPLES_NV        0x8CAB
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_NV 0x8D56
#define GL_MAX_SAMPLES_NV                 0x8D57
#endif /* GL_NV_framebuffer_multisample */

#ifndef GL_NV_generate_mipmap_sRGB
#define GL_NV_generate_mipmap_sRGB 1
#endif /* GL_NV_generate_mipmap_sRGB */

#ifndef GL_NV_instanced_arrays
#define GL_NV_instanced_arrays 1
#define GL_VERTEX_ATTRIB_ARRAY_DIVISOR_NV 0x88FE
#endif /* GL_NV_instanced_arrays */

#ifndef GL_NV_non_square_matrices
#define GL_NV_non_square_matrices 1
#define GL_FLOAT_MAT2x3_NV                0x8B65
#define GL_FLOAT_MAT2x4_NV                0x8B66
#define GL_FLOAT_MAT3x2_NV                0x8B67
#define GL_FLOAT_MAT3x4_NV                0x8B68
#define GL_FLOAT_MAT4x2_NV                0x8B69
#define GL_FLOAT_MAT4x3_NV                0x8B6A
#endif /* GL_NV_non_square_matrices */

#ifndef GL_NV_read_buffer
#define GL_NV_read_buffer 1
#define GL_READ_BUFFER_NV                 0x0C02
#endif /* GL_NV_read_buffer */

#ifndef GL_NV_read_buffer_front
#define GL_NV_read_buffer_front 1
#endif /* GL_NV_read_buffer_front */

#ifndef GL_NV_read_depth
#define GL_NV_read_depth 1
#endif /* GL_NV_read_depth */

#ifndef GL_NV_read_depth_stencil
#define GL_NV_read_depth_stencil 1
#endif /* GL_NV_read_depth_stencil */

#ifndef GL_NV_read_stencil
#define GL_NV_read_stencil 1
#endif /* GL_NV_read_stencil */

#ifndef GL_NV_sRGB_formats
#define GL_NV_sRGB_formats 1
#define GL_SLUMINANCE_NV                  0x8C46
#define GL_SLUMINANCE_ALPHA_NV            0x8C44
#define GL_SRGB8_NV                       0x8C41
#define GL_SLUMINANCE8_NV                 0x8C47
#define GL_SLUMINANCE8_ALPHA8_NV          0x8C45
#define GL_COMPRESSED_SRGB_S3TC_DXT1_NV   0x8C4C
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_NV 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_NV 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_NV 0x8C4F
#define GL_ETC1_SRGB8_NV                  0x88EE
#endif /* GL_NV_sRGB_formats */

#ifndef GL_NV_shadow_samplers_array
#define GL_NV_shadow_samplers_array 1
#define GL_SAMPLER_2D_ARRAY_SHADOW_NV     0x8DC4
#endif /* GL_NV_shadow_samplers_array */

#ifndef GL_NV_shadow_samplers_cube
#define GL_NV_shadow_samplers_cube 1
#define GL_SAMPLER_CUBE_SHADOW_NV         0x8DC5
#endif /* GL_NV_shadow_samplers_cube */

#ifndef GL_NV_texture_border_clamp
#define GL_NV_texture_border_clamp 1
#define GL_TEXTURE_BORDER_COLOR_NV        0x1004
#define GL_CLAMP_TO_BORDER_NV             0x812D
#endif /* GL_NV_texture_border_clamp */

#ifndef GL_NV_texture_compression_s3tc_update
#define GL_NV_texture_compression_s3tc_update 1
#endif /* GL_NV_texture_compression_s3tc_update */

#ifndef GL_NV_texture_npot_2D_mipmap
#define GL_NV_texture_npot_2D_mipmap 1
#endif /* GL_NV_texture_npot_2D_mipmap */

#ifndef GL_QCOM_alpha_test
#define GL_QCOM_alpha_test 1
#define GL_ALPHA_TEST_QCOM                0x0BC0
#define GL_ALPHA_TEST_FUNC_QCOM           0x0BC1
#define GL_ALPHA_TEST_REF_QCOM            0x0BC2
#endif /* GL_QCOM_alpha_test */

#ifndef GL_QCOM_binning_control
#define GL_QCOM_binning_control 1
#define GL_BINNING_CONTROL_HINT_QCOM      0x8FB0
#define GL_CPU_OPTIMIZED_QCOM             0x8FB1
#define GL_GPU_OPTIMIZED_QCOM             0x8FB2
#define GL_RENDER_DIRECT_TO_FRAMEBUFFER_QCOM 0x8FB3
#endif /* GL_QCOM_binning_control */

#ifndef GL_QCOM_driver_control
#define GL_QCOM_driver_control 1
#endif /* GL_QCOM_driver_control */

#ifndef GL_QCOM_extended_get
#define GL_QCOM_extended_get 1
#define GL_TEXTURE_WIDTH_QCOM             0x8BD2
#define GL_TEXTURE_HEIGHT_QCOM            0x8BD3
#define GL_TEXTURE_DEPTH_QCOM             0x8BD4
#define GL_TEXTURE_INTERNAL_FORMAT_QCOM   0x8BD5
#define GL_TEXTURE_FORMAT_QCOM            0x8BD6
#define GL_TEXTURE_TYPE_QCOM              0x8BD7
#define GL_TEXTURE_IMAGE_VALID_QCOM       0x8BD8
#define GL_TEXTURE_NUM_LEVELS_QCOM        0x8BD9
#define GL_TEXTURE_TARGET_QCOM            0x8BDA
#define GL_TEXTURE_OBJECT_VALID_QCOM      0x8BDB
#define GL_STATE_RESTORE                  0x8BDC
#endif /* GL_QCOM_extended_get */

#ifndef GL_QCOM_extended_get2
#define GL_QCOM_extended_get2 1
#endif /* GL_QCOM_extended_get2 */

#ifndef GL_QCOM_perfmon_global_mode
#define GL_QCOM_perfmon_global_mode 1
#define GL_PERFMON_GLOBAL_MODE_QCOM       0x8FA0
#endif /* GL_QCOM_perfmon_global_mode */

#ifndef GL_QCOM_tiled_rendering
#define GL_QCOM_tiled_rendering 1
#define GL_COLOR_BUFFER_BIT0_QCOM         0x00000001
#define GL_COLOR_BUFFER_BIT1_QCOM         0x00000002
#define GL_COLOR_BUFFER_BIT2_QCOM         0x00000004
#define GL_COLOR_BUFFER_BIT3_QCOM         0x00000008
#define GL_COLOR_BUFFER_BIT4_QCOM         0x00000010
#define GL_COLOR_BUFFER_BIT5_QCOM         0x00000020
#define GL_COLOR_BUFFER_BIT6_QCOM         0x00000040
#define GL_COLOR_BUFFER_BIT7_QCOM         0x00000080
#define GL_DEPTH_BUFFER_BIT0_QCOM         0x00000100
#define GL_DEPTH_BUFFER_BIT1_QCOM         0x00000200
#define GL_DEPTH_BUFFER_BIT2_QCOM         0x00000400
#define GL_DEPTH_BUFFER_BIT3_QCOM         0x00000800
#define GL_DEPTH_BUFFER_BIT4_QCOM         0x00001000
#define GL_DEPTH_BUFFER_BIT5_QCOM         0x00002000
#define GL_DEPTH_BUFFER_BIT6_QCOM         0x00004000
#define GL_DEPTH_BUFFER_BIT7_QCOM         0x00008000
#define GL_STENCIL_BUFFER_BIT0_QCOM       0x00010000
#define GL_STENCIL_BUFFER_BIT1_QCOM       0x00020000
#define GL_STENCIL_BUFFER_BIT2_QCOM       0x00040000
#define GL_STENCIL_BUFFER_BIT3_QCOM       0x00080000
#define GL_STENCIL_BUFFER_BIT4_QCOM       0x00100000
#define GL_STENCIL_BUFFER_BIT5_QCOM       0x00200000
#define GL_STENCIL_BUFFER_BIT6_QCOM       0x00400000
#define GL_STENCIL_BUFFER_BIT7_QCOM       0x00800000
#define GL_MULTISAMPLE_BUFFER_BIT0_QCOM   0x01000000
#define GL_MULTISAMPLE_BUFFER_BIT1_QCOM   0x02000000
#define GL_MULTISAMPLE_BUFFER_BIT2_QCOM   0x04000000
#define GL_MULTISAMPLE_BUFFER_BIT3_QCOM   0x08000000
#define GL_MULTISAMPLE_BUFFER_BIT4_QCOM   0x10000000
#define GL_MULTISAMPLE_BUFFER_BIT5_QCOM   0x20000000
#define GL_MULTISAMPLE_BUFFER_BIT6_QCOM   0x40000000
#define GL_MULTISAMPLE_BUFFER_BIT7_QCOM   0x80000000
#endif /* GL_QCOM_tiled_rendering */

#ifndef GL_QCOM_writeonly_rendering
#define GL_QCOM_writeonly_rendering 1
#define GL_WRITEONLY_RENDERING_QCOM       0x8823
#endif /* GL_QCOM_writeonly_rendering */

#ifndef GL_VIV_shader_binary
#define GL_VIV_shader_binary 1
#define GL_SHADER_BINARY_VIV              0x8FC4
#endif /* GL_VIV_shader_binary */

#else
# ifndef EVAS_GL_NO_GL_H_CHECK
#  error "You may only include either Evas_GL.h OR use your native GLES2 headers. If you use Evas to do GL, then you cannot use the native GLES2 headers."
# endif
#endif


/* OpenGLES 1.0 */

#ifndef __gl_h_
# define __gl_h_ 1

/* Note: This should redefine a lot of the macros already defined above,
 * but the compiler shouldn't raise any warning since the definitions are
 * exactly the same.
 */

/* OpenGL ES core versions */
#define GL_VERSION_ES_CM_1_0          1
#define GL_VERSION_ES_CL_1_0          1
#define GL_VERSION_ES_CM_1_1          1
#define GL_VERSION_ES_CL_1_1          1

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

/* AlphaFunction */
#define GL_NEVER                          0x0200
#define GL_LESS                           0x0201
#define GL_EQUAL                          0x0202
#define GL_LEQUAL                         0x0203
#define GL_GREATER                        0x0204
#define GL_NOTEQUAL                       0x0205
#define GL_GEQUAL                         0x0206
#define GL_ALWAYS                         0x0207

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

/* ClipPlaneName */
#define GL_CLIP_PLANE0                    0x3000
#define GL_CLIP_PLANE1                    0x3001
#define GL_CLIP_PLANE2                    0x3002
#define GL_CLIP_PLANE3                    0x3003
#define GL_CLIP_PLANE4                    0x3004
#define GL_CLIP_PLANE5                    0x3005

/* ColorMaterialFace */
/*      GL_FRONT_AND_BACK */

/* ColorMaterialParameter */
/*      GL_AMBIENT_AND_DIFFUSE */

/* ColorPointerType */
/*      GL_UNSIGNED_BYTE */
/*      GL_FLOAT */
/*      GL_FIXED */

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
#define GL_FOG                            0x0B60
#define GL_LIGHTING                       0x0B50
#define GL_TEXTURE_2D                     0x0DE1
#define GL_CULL_FACE                      0x0B44
#define GL_ALPHA_TEST                     0x0BC0
#define GL_BLEND                          0x0BE2
#define GL_COLOR_LOGIC_OP                 0x0BF2
#define GL_DITHER                         0x0BD0
#define GL_STENCIL_TEST                   0x0B90
#define GL_DEPTH_TEST                     0x0B71
/*      GL_LIGHT0 */
/*      GL_LIGHT1 */
/*      GL_LIGHT2 */
/*      GL_LIGHT3 */
/*      GL_LIGHT4 */
/*      GL_LIGHT5 */
/*      GL_LIGHT6 */
/*      GL_LIGHT7 */
#define GL_POINT_SMOOTH                   0x0B10
#define GL_LINE_SMOOTH                    0x0B20
#define GL_SCISSOR_TEST                   0x0C11
#define GL_COLOR_MATERIAL                 0x0B57
#define GL_NORMALIZE                      0x0BA1
#define GL_RESCALE_NORMAL                 0x803A
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_VERTEX_ARRAY                   0x8074
#define GL_NORMAL_ARRAY                   0x8075
#define GL_COLOR_ARRAY                    0x8076
#define GL_TEXTURE_COORD_ARRAY            0x8078
#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLE_COVERAGE                0x80A0

/* ErrorCode */
#define GL_NO_ERROR                       0
#define GL_INVALID_ENUM                   0x0500
#define GL_INVALID_VALUE                  0x0501
#define GL_INVALID_OPERATION              0x0502
#define GL_STACK_OVERFLOW                 0x0503
#define GL_STACK_UNDERFLOW                0x0504
#define GL_OUT_OF_MEMORY                  0x0505

/* FogMode */
/*      GL_LINEAR */
#define GL_EXP                            0x0800
#define GL_EXP2                           0x0801

/* FogParameter */
#define GL_FOG_DENSITY                    0x0B62
#define GL_FOG_START                      0x0B63
#define GL_FOG_END                        0x0B64
#define GL_FOG_MODE                       0x0B65
#define GL_FOG_COLOR                      0x0B66

/* FrontFaceDirection */
#define GL_CW                             0x0900
#define GL_CCW                            0x0901

/* GetPName */
#define GL_CURRENT_COLOR                  0x0B00
#define GL_CURRENT_NORMAL                 0x0B02
#define GL_CURRENT_TEXTURE_COORDS         0x0B03
#define GL_POINT_SIZE                     0x0B11
#define GL_POINT_SIZE_MIN                 0x8126
#define GL_POINT_SIZE_MAX                 0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE      0x8128
#define GL_POINT_DISTANCE_ATTENUATION     0x8129
#define GL_SMOOTH_POINT_SIZE_RANGE        0x0B12
#define GL_LINE_WIDTH                     0x0B21
#define GL_SMOOTH_LINE_WIDTH_RANGE        0x0B22
#define GL_ALIASED_POINT_SIZE_RANGE       0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE       0x846E
#define GL_CULL_FACE_MODE                 0x0B45
#define GL_FRONT_FACE                     0x0B46
#define GL_SHADE_MODEL                    0x0B54
#define GL_DEPTH_RANGE                    0x0B70
#define GL_DEPTH_WRITEMASK                0x0B72
#define GL_DEPTH_CLEAR_VALUE              0x0B73
#define GL_DEPTH_FUNC                     0x0B74
#define GL_STENCIL_CLEAR_VALUE            0x0B91
#define GL_STENCIL_FUNC                   0x0B92
#define GL_STENCIL_VALUE_MASK             0x0B93
#define GL_STENCIL_FAIL                   0x0B94
#define GL_STENCIL_PASS_DEPTH_FAIL        0x0B95
#define GL_STENCIL_PASS_DEPTH_PASS        0x0B96
#define GL_STENCIL_REF                    0x0B97
#define GL_STENCIL_WRITEMASK              0x0B98
#define GL_MATRIX_MODE                    0x0BA0
#define GL_VIEWPORT                       0x0BA2
#define GL_MODELVIEW_STACK_DEPTH          0x0BA3
#define GL_PROJECTION_STACK_DEPTH         0x0BA4
#define GL_TEXTURE_STACK_DEPTH            0x0BA5
#define GL_MODELVIEW_MATRIX               0x0BA6
#define GL_PROJECTION_MATRIX              0x0BA7
#define GL_TEXTURE_MATRIX                 0x0BA8
#define GL_ALPHA_TEST_FUNC                0x0BC1
#define GL_ALPHA_TEST_REF                 0x0BC2
#define GL_BLEND_DST                      0x0BE0
#define GL_BLEND_SRC                      0x0BE1
#define GL_LOGIC_OP_MODE                  0x0BF0
#define GL_SCISSOR_BOX                    0x0C10
#define GL_SCISSOR_TEST                   0x0C11
#define GL_COLOR_CLEAR_VALUE              0x0C22
#define GL_COLOR_WRITEMASK                0x0C23
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_ALIGNMENT                 0x0D05
#define GL_MAX_LIGHTS                     0x0D31
#define GL_MAX_CLIP_PLANES                0x0D32
#define GL_MAX_TEXTURE_SIZE               0x0D33
#define GL_MAX_MODELVIEW_STACK_DEPTH      0x0D36
#define GL_MAX_PROJECTION_STACK_DEPTH     0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH        0x0D39
#define GL_MAX_VIEWPORT_DIMS              0x0D3A
#define GL_MAX_TEXTURE_UNITS              0x84E2
#define GL_SUBPIXEL_BITS                  0x0D50
#define GL_RED_BITS                       0x0D52
#define GL_GREEN_BITS                     0x0D53
#define GL_BLUE_BITS                      0x0D54
#define GL_ALPHA_BITS                     0x0D55
#define GL_DEPTH_BITS                     0x0D56
#define GL_STENCIL_BITS                   0x0D57
#define GL_POLYGON_OFFSET_UNITS           0x2A00
#define GL_POLYGON_OFFSET_FILL            0x8037
#define GL_POLYGON_OFFSET_FACTOR          0x8038
#define GL_TEXTURE_BINDING_2D             0x8069
#define GL_VERTEX_ARRAY_SIZE              0x807A
#define GL_VERTEX_ARRAY_TYPE              0x807B
#define GL_VERTEX_ARRAY_STRIDE            0x807C
#define GL_NORMAL_ARRAY_TYPE              0x807E
#define GL_NORMAL_ARRAY_STRIDE            0x807F
#define GL_COLOR_ARRAY_SIZE               0x8081
#define GL_COLOR_ARRAY_TYPE               0x8082
#define GL_COLOR_ARRAY_STRIDE             0x8083
#define GL_TEXTURE_COORD_ARRAY_SIZE       0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE       0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE     0x808A
#define GL_VERTEX_ARRAY_POINTER           0x808E
#define GL_NORMAL_ARRAY_POINTER           0x808F
#define GL_COLOR_ARRAY_POINTER            0x8090
#define GL_TEXTURE_COORD_ARRAY_POINTER    0x8092
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
#define GL_PERSPECTIVE_CORRECTION_HINT    0x0C50
#define GL_POINT_SMOOTH_HINT              0x0C51
#define GL_LINE_SMOOTH_HINT               0x0C52
#define GL_FOG_HINT                       0x0C54
#define GL_GENERATE_MIPMAP_HINT           0x8192

/* LightModelParameter */
#define GL_LIGHT_MODEL_AMBIENT            0x0B53
#define GL_LIGHT_MODEL_TWO_SIDE           0x0B52

/* LightParameter */
#define GL_AMBIENT                        0x1200
#define GL_DIFFUSE                        0x1201
#define GL_SPECULAR                       0x1202
#define GL_POSITION                       0x1203
#define GL_SPOT_DIRECTION                 0x1204
#define GL_SPOT_EXPONENT                  0x1205
#define GL_SPOT_CUTOFF                    0x1206
#define GL_CONSTANT_ATTENUATION           0x1207
#define GL_LINEAR_ATTENUATION             0x1208
#define GL_QUADRATIC_ATTENUATION          0x1209

/* DataType */
#define GL_BYTE                           0x1400
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_SHORT                          0x1402
#define GL_UNSIGNED_SHORT                 0x1403
#define GL_FLOAT                          0x1406
#define GL_FIXED                          0x140C

/* LogicOp */
#define GL_CLEAR                          0x1500
#define GL_AND                            0x1501
#define GL_AND_REVERSE                    0x1502
#define GL_COPY                           0x1503
#define GL_AND_INVERTED                   0x1504
#define GL_NOOP                           0x1505
#define GL_XOR                            0x1506
#define GL_OR                             0x1507
#define GL_NOR                            0x1508
#define GL_EQUIV                          0x1509
#define GL_INVERT                         0x150A
#define GL_OR_REVERSE                     0x150B
#define GL_COPY_INVERTED                  0x150C
#define GL_OR_INVERTED                    0x150D
#define GL_NAND                           0x150E
#define GL_SET                            0x150F

/* MaterialFace */
/*      GL_FRONT_AND_BACK */

/* MaterialParameter */
#define GL_EMISSION                       0x1600
#define GL_SHININESS                      0x1601
#define GL_AMBIENT_AND_DIFFUSE            0x1602
/*      GL_AMBIENT */
/*      GL_DIFFUSE */
/*      GL_SPECULAR */

/* MatrixMode */
#define GL_MODELVIEW                      0x1700
#define GL_PROJECTION                     0x1701
#define GL_TEXTURE                        0x1702

/* NormalPointerType */
/*      GL_BYTE */
/*      GL_SHORT */
/*      GL_FLOAT */
/*      GL_FIXED */

/* PixelFormat */
#define GL_ALPHA                          0x1906
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A

/* PixelStoreParameter */
#define GL_UNPACK_ALIGNMENT               0x0CF5
#define GL_PACK_ALIGNMENT                 0x0D05

/* PixelType */
/*      GL_UNSIGNED_BYTE */
#define GL_UNSIGNED_SHORT_4_4_4_4         0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1         0x8034
#define GL_UNSIGNED_SHORT_5_6_5           0x8363

/* ShadingModel */
#define GL_FLAT                           0x1D00
#define GL_SMOOTH                         0x1D01

/* StencilFunction */
/*      GL_NEVER */
/*      GL_LESS */
/*      GL_EQUAL */
/*      GL_LEQUAL */
/*      GL_GREATER */
/*      GL_NOTEQUAL */
/*      GL_GEQUAL */
/*      GL_ALWAYS */

/* StencilOp */
/*      GL_ZERO */
#define GL_KEEP                           0x1E00
#define GL_REPLACE                        0x1E01
#define GL_INCR                           0x1E02
#define GL_DECR                           0x1E03
/*      GL_INVERT */

/* StringName */
#define GL_VENDOR                         0x1F00
#define GL_RENDERER                       0x1F01
#define GL_VERSION                        0x1F02
#define GL_EXTENSIONS                     0x1F03

/* TexCoordPointerType */
/*      GL_SHORT */
/*      GL_FLOAT */
/*      GL_FIXED */
/*      GL_BYTE */

/* TextureEnvMode */
#define GL_MODULATE                       0x2100
#define GL_DECAL                          0x2101
/*      GL_BLEND */
#define GL_ADD                            0x0104
/*      GL_REPLACE */

/* TextureEnvParameter */
#define GL_TEXTURE_ENV_MODE               0x2200
#define GL_TEXTURE_ENV_COLOR              0x2201

/* TextureEnvTarget */
#define GL_TEXTURE_ENV                    0x2300

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
#define GL_GENERATE_MIPMAP                0x8191

/* TextureTarget */
/*      GL_TEXTURE_2D */

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
#define GL_CLIENT_ACTIVE_TEXTURE          0x84E1

/* TextureWrapMode */
#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F

/* VertexPointerType */
/*      GL_SHORT */
/*      GL_FLOAT */
/*      GL_FIXED */
/*      GL_BYTE */

/* LightName */
#define GL_LIGHT0                         0x4000
#define GL_LIGHT1                         0x4001
#define GL_LIGHT2                         0x4002
#define GL_LIGHT3                         0x4003
#define GL_LIGHT4                         0x4004
#define GL_LIGHT5                         0x4005
#define GL_LIGHT6                         0x4006
#define GL_LIGHT7                         0x4007

/* Buffer Objects */
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893

#define GL_ARRAY_BUFFER_BINDING               0x8894
#define GL_ELEMENT_ARRAY_BUFFER_BINDING       0x8895
#define GL_VERTEX_ARRAY_BUFFER_BINDING        0x8896
#define GL_NORMAL_ARRAY_BUFFER_BINDING        0x8897
#define GL_COLOR_ARRAY_BUFFER_BINDING         0x8898
#define GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING 0x889A

#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_BUFFER_SIZE                    0x8764
#define GL_BUFFER_USAGE                   0x8765

/* Texture combine + dot3 */
#define GL_SUBTRACT                       0x84E7
#define GL_COMBINE                        0x8570
#define GL_COMBINE_RGB                    0x8571
#define GL_COMBINE_ALPHA                  0x8572
#define GL_RGB_SCALE                      0x8573
#define GL_ADD_SIGNED                     0x8574
#define GL_INTERPOLATE                    0x8575
#define GL_CONSTANT                       0x8576
#define GL_PRIMARY_COLOR                  0x8577
#define GL_PREVIOUS                       0x8578
#define GL_OPERAND0_RGB                   0x8590
#define GL_OPERAND1_RGB                   0x8591
#define GL_OPERAND2_RGB                   0x8592
#define GL_OPERAND0_ALPHA                 0x8598
#define GL_OPERAND1_ALPHA                 0x8599
#define GL_OPERAND2_ALPHA                 0x859A

#define GL_ALPHA_SCALE                    0x0D1C

#define GL_SRC0_RGB                       0x8580
#define GL_SRC1_RGB                       0x8581
#define GL_SRC2_RGB                       0x8582
#define GL_SRC0_ALPHA                     0x8588
#define GL_SRC1_ALPHA                     0x8589
#define GL_SRC2_ALPHA                     0x858A

#define GL_DOT3_RGB                       0x86AE
#define GL_DOT3_RGBA                      0x86AF

/*------------------------------------------------------------------------*
 * required OES extension tokens
 *------------------------------------------------------------------------*/

/* OES_read_format */
#ifndef GL_OES_read_format
#define GL_OES_read_format 1
#define GL_IMPLEMENTATION_COLOR_READ_TYPE_OES                   0x8B9A
#define GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES                 0x8B9B
#endif

/* OES_point_size_array */
#ifndef GL_OES_point_size_array
#define GL_OES_point_size_array 1
#define GL_POINT_SIZE_ARRAY_OES                                 0x8B9C
#define GL_POINT_SIZE_ARRAY_TYPE_OES                            0x898A
#define GL_POINT_SIZE_ARRAY_STRIDE_OES                          0x898B
#define GL_POINT_SIZE_ARRAY_POINTER_OES                         0x898C
#define GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES                  0x8B9F
#endif

/* GL_OES_point_sprite */
#ifndef GL_OES_point_sprite
#define GL_OES_point_sprite 1
#define GL_POINT_SPRITE_OES                                     0x8861
#define GL_COORD_REPLACE_OES                                    0x8862
#endif

#else
# ifndef EVAS_GL_NO_GL_H_CHECK
#  error "You may only include either Evas_GL.h OR use your native GLES headers. If you use Evas to do GL, then you cannot use the native GLES headers."
# endif
#endif


#ifndef __glext_h_
#define __glext_h_ 1

/*------------------------------------------------------------------------*
 * OES extension tokens
 *------------------------------------------------------------------------*/

/* GL_OES_blend_equation_separate */
#ifndef GL_OES_blend_equation_separate
/* BLEND_EQUATION_RGB_OES same as BLEND_EQUATION_OES */
#define GL_BLEND_EQUATION_RGB_OES                               0x8009
#define GL_BLEND_EQUATION_ALPHA_OES                             0x883D
#endif

/* GL_OES_blend_func_separate */
#ifndef GL_OES_blend_func_separate
#define GL_BLEND_DST_RGB_OES                                    0x80C8
#define GL_BLEND_SRC_RGB_OES                                    0x80C9
#define GL_BLEND_DST_ALPHA_OES                                  0x80CA
#define GL_BLEND_SRC_ALPHA_OES                                  0x80CB
#endif

/* GL_OES_blend_subtract */
#ifndef GL_OES_blend_subtract
#define GL_BLEND_EQUATION_OES                                   0x8009
#define GL_FUNC_ADD_OES                                         0x8006
#define GL_FUNC_SUBTRACT_OES                                    0x800A
#define GL_FUNC_REVERSE_SUBTRACT_OES                            0x800B
#endif

/* GL_OES_draw_texture */
#ifndef GL_OES_draw_texture
#define GL_TEXTURE_CROP_RECT_OES                                0x8B9D
#endif

/* GL_OES_EGL_image */
#ifndef GL_OES_EGL_image
#endif

/* GL_OES_fixed_point */
#ifndef GL_OES_fixed_point
#define GL_FIXED_OES                                            0x140C
#endif

/* GL_OES_framebuffer_object */
#ifndef GL_OES_framebuffer_object
#define GL_NONE_OES                                             0
#define GL_FRAMEBUFFER_OES                                      0x8D40
#define GL_RENDERBUFFER_OES                                     0x8D41
#define GL_RGBA4_OES                                            0x8056
#define GL_RGB5_A1_OES                                          0x8057
#define GL_RGB565_OES                                           0x8D62
#define GL_DEPTH_COMPONENT16_OES                                0x81A5
#define GL_RENDERBUFFER_WIDTH_OES                               0x8D42
#define GL_RENDERBUFFER_HEIGHT_OES                              0x8D43
#define GL_RENDERBUFFER_INTERNAL_FORMAT_OES                     0x8D44
#define GL_RENDERBUFFER_RED_SIZE_OES                            0x8D50
#define GL_RENDERBUFFER_GREEN_SIZE_OES                          0x8D51
#define GL_RENDERBUFFER_BLUE_SIZE_OES                           0x8D52
#define GL_RENDERBUFFER_ALPHA_SIZE_OES                          0x8D53
#define GL_RENDERBUFFER_DEPTH_SIZE_OES                          0x8D54
#define GL_RENDERBUFFER_STENCIL_SIZE_OES                        0x8D55
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_OES               0x8CD0
#define GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_OES               0x8CD1
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_OES             0x8CD2
#define GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_OES     0x8CD3
#define GL_COLOR_ATTACHMENT0_OES                                0x8CE0
#define GL_DEPTH_ATTACHMENT_OES                                 0x8D00
#define GL_STENCIL_ATTACHMENT_OES                               0x8D20
#define GL_FRAMEBUFFER_COMPLETE_OES                             0x8CD5
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES                0x8CD6
#define GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_OES        0x8CD7
#define GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_OES                0x8CD9
#define GL_FRAMEBUFFER_INCOMPLETE_FORMATS_OES                   0x8CDA
#define GL_FRAMEBUFFER_UNSUPPORTED_OES                          0x8CDD
#define GL_FRAMEBUFFER_BINDING_OES                              0x8CA6
#define GL_RENDERBUFFER_BINDING_OES                             0x8CA7
#define GL_MAX_RENDERBUFFER_SIZE_OES                            0x84E8
#define GL_INVALID_FRAMEBUFFER_OPERATION_OES                    0x0506
#endif

/* GL_OES_matrix_get */
#ifndef GL_OES_matrix_get
#define GL_MODELVIEW_MATRIX_FLOAT_AS_INT_BITS_OES               0x898D
#define GL_PROJECTION_MATRIX_FLOAT_AS_INT_BITS_OES              0x898E
#define GL_TEXTURE_MATRIX_FLOAT_AS_INT_BITS_OES                 0x898F
#endif

/* GL_OES_matrix_palette */
#ifndef GL_OES_matrix_palette
#define GL_MAX_VERTEX_UNITS_OES                                 0x86A4
#define GL_MAX_PALETTE_MATRICES_OES                             0x8842
#define GL_MATRIX_PALETTE_OES                                   0x8840
#define GL_MATRIX_INDEX_ARRAY_OES                               0x8844
#define GL_WEIGHT_ARRAY_OES                                     0x86AD
#define GL_CURRENT_PALETTE_MATRIX_OES                           0x8843
#define GL_MATRIX_INDEX_ARRAY_SIZE_OES                          0x8846
#define GL_MATRIX_INDEX_ARRAY_TYPE_OES                          0x8847
#define GL_MATRIX_INDEX_ARRAY_STRIDE_OES                        0x8848
#define GL_MATRIX_INDEX_ARRAY_POINTER_OES                       0x8849
#define GL_MATRIX_INDEX_ARRAY_BUFFER_BINDING_OES                0x8B9E
#define GL_WEIGHT_ARRAY_SIZE_OES                                0x86AB
#define GL_WEIGHT_ARRAY_TYPE_OES                                0x86A9
#define GL_WEIGHT_ARRAY_STRIDE_OES                              0x86AA
#define GL_WEIGHT_ARRAY_POINTER_OES                             0x86AC
#define GL_WEIGHT_ARRAY_BUFFER_BINDING_OES                      0x889E
#endif

/* GL_OES_stencil8 */
#ifndef GL_OES_stencil8
#define GL_STENCIL_INDEX8_OES                                   0x8D48
#endif

/* GL_OES_stencil_wrap */
#ifndef GL_OES_stencil_wrap
#define GL_INCR_WRAP_OES                                        0x8507
#define GL_DECR_WRAP_OES                                        0x8508
#endif

/* GL_OES_texture_cube_map */
#ifndef GL_OES_texture_cube_map
#define GL_NORMAL_MAP_OES                                       0x8511
#define GL_REFLECTION_MAP_OES                                   0x8512
#define GL_TEXTURE_CUBE_MAP_OES                                 0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_OES                         0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_OES                      0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_OES                      0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_OES                      0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_OES                      0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_OES                      0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_OES                      0x851A
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_OES                        0x851C
#define GL_TEXTURE_GEN_MODE_OES                                 0x2500
#define GL_TEXTURE_GEN_STR_OES                                  0x8D60
#endif

/* GL_OES_texture_mirrored_repeat */
#ifndef GL_OES_texture_mirrored_repeat
#define GL_MIRRORED_REPEAT_OES                                  0x8370
#endif

/*------------------------------------------------------------------------*
 * EXT extension tokens
 *------------------------------------------------------------------------*/

/* GL_EXT_texture_lod_bias */
#ifndef GL_EXT_texture_lod_bias
#define GL_MAX_TEXTURE_LOD_BIAS_EXT                             0x84FD
#define GL_TEXTURE_FILTER_CONTROL_EXT                           0x8500
#define GL_TEXTURE_LOD_BIAS_EXT                                 0x8501
#endif

/*------------------------------------------------------------------------*
 * IMG extension tokens
 *------------------------------------------------------------------------*/

/* GL_IMG_texture_env_enhanced_fixed_function */
#ifndef GL_IMG_texture_env_enhanced_fixed_function
#define GL_MODULATE_COLOR_IMG                                   0x8C04
#define GL_RECIP_ADD_SIGNED_ALPHA_IMG                           0x8C05
#define GL_TEXTURE_ALPHA_MODULATE_IMG                           0x8C06
#define GL_FACTOR_ALPHA_MODULATE_IMG                            0x8C07
#define GL_FRAGMENT_ALPHA_MODULATE_IMG                          0x8C08
#define GL_ADD_BLEND_IMG                                        0x8C09
#define GL_DOT3_RGBA_IMG                                        0x86AF
#endif

/* GL_IMG_user_clip_plane */
#ifndef GL_IMG_user_clip_plane
#define GL_CLIP_PLANE0_IMG                                      0x3000
#define GL_CLIP_PLANE1_IMG                                      0x3001
#define GL_CLIP_PLANE2_IMG                                      0x3002
#define GL_CLIP_PLANE3_IMG                                      0x3003
#define GL_CLIP_PLANE4_IMG                                      0x3004
#define GL_CLIP_PLANE5_IMG                                      0x3005
#define GL_MAX_CLIP_PLANES_IMG                                  0x0D32
#endif

/*------------------------------------------------------------------------*
 * OES extension functions
 *------------------------------------------------------------------------*/

/* GL_OES_blend_equation_separate */
#ifndef GL_OES_blend_equation_separate
#define GL_OES_blend_equation_separate 1
#endif

/* GL_OES_blend_func_separate */
#ifndef GL_OES_blend_func_separate
#define GL_OES_blend_func_separate 1
#endif

/* GL_OES_blend_subtract */
#ifndef GL_OES_blend_subtract
#define GL_OES_blend_subtract 1
#endif

/* GL_OES_byte_coordinates */
#ifndef GL_OES_byte_coordinates
#define GL_OES_byte_coordinates 1
#endif

/* GL_OES_draw_texture */
#ifndef GL_OES_draw_texture
#define GL_OES_draw_texture 1
#endif

/* GL_OES_extended_matrix_palette */
#ifndef GL_OES_extended_matrix_palette
#define GL_OES_extended_matrix_palette 1
#endif

/* GL_OES_fixed_point */
#ifndef GL_OES_fixed_point
#define GL_OES_fixed_point 1
#endif

/* GL_OES_framebuffer_object */
#ifndef GL_OES_framebuffer_object
#define GL_OES_framebuffer_object 1
#endif

/* GL_OES_matrix_get */
#ifndef GL_OES_matrix_get
#define GL_OES_matrix_get 1
#endif

/* GL_OES_matrix_palette */
#ifndef GL_OES_matrix_palette
#define GL_OES_matrix_palette 1
#endif

/* GL_OES_query_matrix */
#ifndef GL_OES_query_matrix
#define GL_OES_query_matrix 1
#endif

/* GL_OES_single_precision */
#ifndef GL_OES_single_precision
#define GL_OES_single_precision 1
#endif

/* GL_OES_stencil8 */
#ifndef GL_OES_stencil8
#define GL_OES_stencil8 1
#endif

/* GL_OES_stencil_wrap */
#ifndef GL_OES_stencil_wrap
#define GL_OES_stencil_wrap 1
#endif

/* GL_OES_texture_cube_map */
#ifndef GL_OES_texture_cube_map
#define GL_OES_texture_cube_map 1
#endif

/* GL_OES_texture_env_crossbar */
#ifndef GL_OES_texture_env_crossbar
#define GL_OES_texture_env_crossbar 1
#endif

/* GL_OES_texture_mirrored_repeat */
#ifndef GL_OES_texture_mirrored_repeat
#define GL_OES_texture_mirrored_repeat 1
#endif

/*------------------------------------------------------------------------*
 * APPLE extension functions
 *------------------------------------------------------------------------*/

/* GL_APPLE_texture_2D_limited_npot */
#ifndef GL_APPLE_texture_2D_limited_npot
#define GL_APPLE_texture_2D_limited_npot 1
#endif

/*------------------------------------------------------------------------*
 * EXT extension functions
 *------------------------------------------------------------------------*/

/* GL_EXT_texture_lod_bias */
#ifndef GL_EXT_texture_lod_bias
#define GL_EXT_texture_lod_bias 1
#endif

/*------------------------------------------------------------------------*
 * IMG extension functions
 *------------------------------------------------------------------------*/

/* GL_IMG_texture_env_enhanced_fixed_function */
#ifndef GL_IMG_texture_env_enhanced_fixed_function
#define GL_IMG_texture_env_enhanced_fixed_function 1
#endif

/* GL_IMG_user_clip_plane */
#ifndef GL_IMG_user_clip_plane
#define GL_IMG_user_clip_plane 1
#endif

#else
# ifndef EVAS_GL_NO_GL_H_CHECK
#  error "You may only include either Evas_GL.h OR use your native GLES headers. If you use Evas to do GL, then you cannot use the native GLES headers."
# endif
#endif


/* Some definitions from GLES 3.0.
 * Note: Evas_GL does NOT support GLES 3.
 */

/* Evas can use RGB8_ETC2 and RGBA8_ETC2_EAC internally, depending on the driver */
#define GL_COMPRESSED_RGB8_ETC2           0x9274
#define GL_COMPRESSED_RGBA8_ETC2_EAC      0x9278


// These types are required since we can't include GLES/gl.h or gl2.h
typedef signed int         GLclampx;   // Changed khronos_int32_t
typedef struct __GLsync*   GLsync;



/*
 * EGL-related definitions
 *
 * Note the names have been changed from EGL to EvasGL so as to be
 * platform independent. Except for the error codes, the following
 * EVAS_GL_x definitions have the same values as their EGL_x counterparts.
 * Please note that the error codes have been reset to start from 0 (success).
 */

/* EGL/EvasGL Types */
typedef void *EvasGLSync;
typedef unsigned long long EvasGLTime;

/* @brief Attribute list terminator
 * 0 is also accepted as an attribute terminator.
 * Evas_GL will ensure that the attribute list is always properly terminated
 * (eg. using EGL_NONE for EGL backends) and the values are supported by the
 * backends.
 */
#define EVAS_GL_NONE                            0x3038

/* EGL_KHR_image_base */
#define EVAS_GL_image_base 1
#define EVAS_GL_IMAGE_PRESERVED                 0x30D2  /**< @brief An attribute for @ref evasglCreateImage or @ref evasglCreateImageForContext, the default is @c EINA_FALSE. Please refer to @c EGL_IMAGE_PRESERVED_KHR. */

/* EGL_KHR_image */
#define EVAS_GL_image 1
#define EVAS_GL_NATIVE_PIXMAP                   0x30B0  /**< @internal A target for @ref evasglCreateImage or @ref evasglCreateImageForContext. Since it is X11-specific, it should not be used by Tizen applications. */

/* EGL_KHR_vg_parent_image */
#define EVAS_VG_PARENT_IMAGE                    0x30BA  /**< @brief A target for @ref evasglCreateImage or @ref evasglCreateImageForContext */

/* EGL_KHR_gl_texture_2D_image */
#define EVAS_GL_TEXTURE_2D                      0x30B1  /**< @brief An attribute for @ref evasglCreateImage or @ref evasglCreateImageForContext */
#define EVAS_GL_TEXTURE_LEVEL                   0x30BC  /**< @brief An attribute for @ref evasglCreateImage or @ref evasglCreateImageForContext */

/* EGL_KHR_gl_texture_cubemap_image */
#define EVAS_GL_TEXTURE_CUBE_MAP_POSITIVE_X     0x30B3  /**< @brief A target for @ref evasglCreateImage or @ref evasglCreateImageForContext */
#define EVAS_GL_TEXTURE_CUBE_MAP_NEGATIVE_X     0x30B4  /**< @brief A target for @ref evasglCreateImage or @ref evasglCreateImageForContext */
#define EVAS_GL_TEXTURE_CUBE_MAP_POSITIVE_Y     0x30B5  /**< @brief A target for @ref evasglCreateImage or @ref evasglCreateImageForContext */
#define EVAS_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y     0x30B6  /**< @brief A target for @ref evasglCreateImage or @ref evasglCreateImageForContext */
#define EVAS_GL_TEXTURE_CUBE_MAP_POSITIVE_Z     0x30B7  /**< @brief A target for @ref evasglCreateImage or @ref evasglCreateImageForContext */
#define EVAS_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z     0x30B8  /**< @brief A target for @ref evasglCreateImage or @ref evasglCreateImageForContext */

/* EGL_KHR_gl_texture_3D_image */
#define EVAS_GL_TEXTURE_3D                      0x30B2  /**< @brief A target for @ref evasglCreateImage or @ref evasglCreateImageForContext */
#define EVAS_GL_TEXTURE_ZOFFSET                 0x30BD  /**< @brief An attribute for @ref evasglCreateImage or @ref evasglCreateImageForContext */

/* EGL_KHR_gl_renderbuffer_image */
#define EVAS_GL_RENDERBUFFER                    0x30B9  /**< @brief A target for @ref evasglCreateImage or @ref evasglCreateImageForContext */

/* Out-of-band attribute value */
#define EVAS_GL_DONT_CARE			(-1)

/* EGL_TIZEN_image_native_surface */
#define EVAS_GL_TIZEN_image_native_surface 1
#define EVAS_GL_NATIVE_SURFACE_TIZEN            0x32A1  /**< @brief A target for @ref evasglCreateImage or @ref evasglCreateImageForContext. This is a Tizen specific feature. */

/**
 * @name Evas GL error codes
 *
 * These are the possible return values of @ref evas_gl_error_get.
 * The values are the same as EGL error codes - @c EGL_SUCCESS.
 *
 * Some of the values may be set directly by Evas GL when an obvious error was
 * detected (eg. @c NULL pointers or invalid dimensions), otherwise Evas GL will
 * call the backend's GetError() function and translate to a valid @c EVAS_GL_
 * error code.
 *
 * @since_tizen 2.3
 *
 * @{
 */
#define EVAS_GL_SUCCESS                         0x0000  /**< @brief The last evas_gl_ operation succeeded. A call to @c evas_gl_error_get() will reset the error. */
#define EVAS_GL_NOT_INITIALIZED                 0x0001  /**< @brief Evas GL was not initialized or a @c NULL pointer was passed */
#define EVAS_GL_BAD_ACCESS                      0x0002  /**< @brief Bad access; for more information, please refer to its EGL counterpart */
#define EVAS_GL_BAD_ALLOC                       0x0003  /**< @brief Bad allocation; for more information, please refer to its EGL counterpart */
#define EVAS_GL_BAD_ATTRIBUTE                   0x0004  /**< @brief Bad attribute; for more information, please refer to its EGL counterpart */
#define EVAS_GL_BAD_CONFIG                      0x0005  /**< @brief Bad configuration; for more information, please refer to its EGL counterpart */
#define EVAS_GL_BAD_CONTEXT                     0x0006  /**< @brief Bad context; for more information, please refer to its EGL counterpart */
#define EVAS_GL_BAD_CURRENT_SURFACE             0x0007  /**< @brief Bad current surface; for more information, please refer to its EGL counterpart */
#define EVAS_GL_BAD_DISPLAY                     0x0008  /**< @brief Bad display; for more information, please refer to its EGL counterpart */
#define EVAS_GL_BAD_MATCH                       0x0009  /**< @brief Bad match; for more information, please refer to its EGL counterpart */
#define EVAS_GL_BAD_NATIVE_PIXMAP               0x000A  /**< @internal Bad native pixmap; for more information, please refer to its EGL counterpart */
#define EVAS_GL_BAD_NATIVE_WINDOW               0x000B  /**< @brief Bad native window; for more information, please refer to its EGL counterpart */
#define EVAS_GL_BAD_PARAMETER                   0x000C  /**< @brief Bad parameter; for more information, please refer to its EGL counterpart */
#define EVAS_GL_BAD_SURFACE                     0x000D  /**< @brief Bad surface; for more information, please refer to its EGL counterpart */
/* EGL 1.1 - IMG_power_management */
#define EVAS_GL_CONTEXT_LOST                    0x000E  /**< @brief Context lost; for more information, please refer to its EGL counterpart */

/** @} */

/* EGL_KHR_fence_sync - EVAS_GL_fence_sync */
#define EVAS_GL_fence_sync 1
/* EGL_KHR_reusable_sync - EVAS_GL_reusable_sync */
#define EVAS_GL_reusable_sync 1
/* EGL_KHR_wait_sync - EVAS_GL_wait_sync */
#define EVAS_GL_KHR_wait_sync 1

/**
 * @name Constants used to define and wait for Sync objects.
 * @{
 */
#define EVAS_GL_SYNC_PRIOR_COMMANDS_COMPLETE    0x30F0 /**< @brief A value for @ref evasglGetSyncAttrib with @ref EVAS_GL_SYNC_CONDITION  */
#define EVAS_GL_SYNC_STATUS                     0x30F1 /**< @brief A attribute for @ref evasglGetSyncAttrib */
#define EVAS_GL_SIGNALED                        0x30F2 /**< @brief A value for @ref evasglGetSyncAttrib with @ref EVAS_GL_SYNC_STATUS  */
#define EVAS_GL_UNSIGNALED                      0x30F3 /**< @brief A value for @ref evasglGetSyncAttrib with @ref EVAS_GL_SYNC_STATUS  */
#define EVAS_GL_TIMEOUT_EXPIRED                 0x30F5 /**< @brief A returned by @ref evasglClientWaitSync */
#define EVAS_GL_CONDITION_SATISFIED             0x30F6 /**< @brief A returned by @ref evasglClientWaitSync */
#define EVAS_GL_SYNC_TYPE                       0x30F7 /**< @brief A attribute for @ref evasglGetSyncAttrib */
#define EVAS_GL_SYNC_CONDITION                  0x30F8 /**< @brief A attribute for @ref evasglGetSyncAttrib */
#define EVAS_GL_SYNC_FENCE                      0x30F9 /**< @brief A type for @ref evasglCreateSync */
#define EVAS_GL_SYNC_REUSABLE                   0x30FA /**< @brief A type for @ref evasglCreateSync */

#define EVAS_GL_SYNC_FLUSH_COMMANDS_BIT         0x0001 /**< @brief A flag for @ref evasglClientWaitSync */
#define EVAS_GL_FOREVER                         0xFFFFFFFFFFFFFFFFull  /**< @brief Disable wait timeout */
#define EVAS_GL_NO_SYNC                         ((EvasGLSync) NULL)    /**< @brief Empty sync object, see @ref evasglCreateSync */
/** @} */

/**
 * @name Surface attributes
 * The attributes can be queried using @ref evas_gl_surface_query
 * @{
 */
#define EVAS_GL_HEIGHT                          0x3056 /**< @brief Attribute for @ref evas_gl_surface_query, returns the surface width in pixels (@c value should be an @c int) */
#define EVAS_GL_WIDTH                           0x3057 /**< @brief Attribute for @ref evas_gl_surface_query, returns the surface width in pixels (@c value should be an @c int) */
#define EVAS_GL_TEXTURE_FORMAT                  0x3080 /**< @brief Attribute for @ref evas_gl_surface_query, returns an @ref Evas_GL_Color_Format */
#define EVAS_GL_TEXTURE_TARGET                  0x3081 /**< @brief Attribute for @ref evas_gl_surface_query, returns @ref EVAS_GL_TEXTURE_2D (if format is @c EVAS_GL_RGB_888 or @c EVAS_GL_RGBA_8888) or 0 (meaning @c NO_TEXTURE, from @c EVAS_GL_NO_FBO) (@c value should be an @c int) */
/** @} */


/* Version 1: OpenGLES 2.0 + extensions only
 * Version 2: OpenGLES 1.0 + extensions
 */
#define EVAS_GL_API_VERSION 2

/**
 * @brief The Evas GL API
 * This structure contains function pointers to the available GL functions.
 * Some of these functions may be wrapped internally by Evas GL.
 */
struct _Evas_GL_API
{
   /**
    * The current version number is @c EVAS_GL_API_VERSION (2).
    * This should not be confused with the OpenGL-ES context version.
    */
   int            version;

   /**
    * @anchor gles2
    * @name OpenGL-ES 2.0.
    *
    * Evas_GL_API version 1.
    *
    * The following functions are supported in all OpenGL-ES 2.0 contexts.
    * @{
    */
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
   void         (*glShaderSource) (GLuint shader, GLsizei count, const char** string, const GLint* length);
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
   /** @} */

   /**
    * @name OpenGL-ES 2.0 extensions.
    *
    *  Evas_GL_API version 1
    *
    * The following functions may be supported in OpenGL-ES 2.0 contexts,
    * depending on which extensions Evas has decided to support.
    *
    * @{
    */

   /* GL_OES_EGL_image */
   // Notice these two names have been changed to fit Evas GL and not EGL!
   /** @brief Requires the @c GL_OES_EGL_image extension, similar to @c glEGLImageTargetTexture2DOES. */
   void         (*glEvasGLImageTargetTexture2DOES) (GLenum target, EvasGLImage image);
   /** @brief Requires the @c GL_OES_EGL_image extension, similar to @c glEGLImageTargetRenderbufferStorageOES. */
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
   /** @} */

   /**
    * @name EGL-related extensions
    *
    * Evas_GL_API version 1.
    *
    * @{
    */

   /* EGL_KHR_image - #ifdef EVAS_GL_image */
   /**
    * @anchor evasglCreateImage
    * @brief Create an EvasGLImage for the current context.
    *
    * Common targets are:
    * @li @c EVAS_GL_TEXTURE_2D:<br/>
    * In case of @c EVAS_GL_TEXTURE_2D on EGL, the currently bound EGLContext
    * will be used to create the image. The buffer argument must be a texture
    * ID cast down to a void* pointer.<br/>
    * Requires the @c EVAS_GL_image extension.
    *
    * @li @c EVAS_GL_NATIVE_SURFACE_TIZEN (Tizen platform only):<br/>
    * Requires the @c EVAS_GL_TIZEN_image_native_surface extension.
    *
    * @note Please consider using @ref evasglCreateImageForContext instead.
    */
   EvasGLImage  (*evasglCreateImage) (int target, void* buffer, const int* attrib_list) EINA_WARN_UNUSED_RESULT;

   /**
    * @anchor evasglDestroyImage
    * @brief Destroys an EvasGLImage.
    * Destroy an image created by either @ref evasglCreateImage or @ref evasglCreateImageForContext.
    *
    * Requires the @c EVAS_GL_image extension.
    */
   void         (*evasglDestroyImage) (EvasGLImage image);

   /* Evas_GL_API version 2: */

   /**
    * @anchor evasglCreateImageForContext
    * @brief Create an EvasGLImage for a given context.
    *
    * @param[in]  evas_gl     The current Evas GL object,
    * @param[in]  ctx         A context to create this image for,
    * @param[in]  target      One of @c EVAS_GL_TEXTURE_2D and @c EVAS_GL_NATIVE_SURFACE_TIZEN,
    * @param[in]  buffer      A pointer to a buffer, see below,
    * @param[in]  attrib_list An array of key-value pairs terminated by 0 (see @ref EVAS_GL_IMAGE_PRESERVED)
    *
    * Common targets are:
    * @li @c EVAS_GL_TEXTURE_2D:<br/>
    * In case of @c EVAS_GL_TEXTURE_2D, the buffer argument must be a texture
    * ID cast down to a void* pointer.<br/>
    * Requires the @c EVAS_GL_image extension.
    *
    * @code
EvasGLImage *img = glapi->evasglCreateImageForContext
  (evasgl, ctx, EVAS_GL_TEXTURE_2D, (void*)(intptr_t)texture_id, NULL);
    * @endcode
    *
    * @li @c EVAS_GL_NATIVE_SURFACE_TIZEN (Tizen platform only):<br/>
    * Requires the @c EVAS_GL_TIZEN_image_native_surface extension.
    *
    * @since_tizen 2.3
    */
   EvasGLImage  (*evasglCreateImageForContext) (Evas_GL *evas_gl, Evas_GL_Context *ctx, int target, void* buffer, const int* attrib_list) EINA_WARN_UNUSED_RESULT;

   /* This defines shows that Evas_GL_API supports GLES1 APIs */
#define EVAS_GL_GLES1 1
   /**
    * @name OpenGL-ES 1.1
    *
    * Evas_GL_API version 2.
    *
    * The following functions are some of the standard OpenGL-ES 1.0 functions,
    * that are not also present in the @ref gles2 "OpenGL-ES 2.0 APIs".
    * @{
    */
   /* Available only in Common profile */
   void         (*glAlphaFunc) (GLenum func, GLclampf ref);
   void         (*glClipPlanef) (GLenum plane, const GLfloat *equation);
   void         (*glColor4f) (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
   void         (*glFogf) (GLenum pname, GLfloat param);
   void         (*glFogfv) (GLenum pname, const GLfloat *params);
   void         (*glFrustumf) (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
   void         (*glGetClipPlanef) (GLenum pname, GLfloat eqn[4]);
   void         (*glGetLightfv) (GLenum light, GLenum pname, GLfloat *params);
   void         (*glGetMaterialfv) (GLenum face, GLenum pname, GLfloat *params);
   void         (*glGetTexEnvfv) (GLenum env, GLenum pname, GLfloat *params);
   void         (*glLightModelf) (GLenum pname, GLfloat param);
   void         (*glLightModelfv) (GLenum pname, const GLfloat *params);
   void         (*glLightf) (GLenum light, GLenum pname, GLfloat param);
   void         (*glLightfv) (GLenum light, GLenum pname, const GLfloat *params);
   void         (*glLoadMatrixf) (const GLfloat *m);
   void         (*glMaterialf) (GLenum face, GLenum pname, GLfloat param);
   void         (*glMaterialfv) (GLenum face, GLenum pname, const GLfloat *params);
   void         (*glMultMatrixf) (const GLfloat *m);
   void         (*glMultiTexCoord4f) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
   void         (*glNormal3f) (GLfloat nx, GLfloat ny, GLfloat nz);
   void         (*glOrthof) (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
   void         (*glPointParameterf) (GLenum pname, GLfloat param);
   void         (*glPointParameterfv) (GLenum pname, const GLfloat *params);
   void         (*glPointSize) (GLfloat size);
   void         (*glPointSizePointerOES) (GLenum type, GLsizei stride, const GLvoid * pointer);
   void         (*glRotatef) (GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
   void         (*glScalef) (GLfloat x, GLfloat y, GLfloat z);
   void         (*glTexEnvf) (GLenum target, GLenum pname, GLfloat param);
   void         (*glTexEnvfv) (GLenum target, GLenum pname, const GLfloat *params);
   void         (*glTranslatef) (GLfloat x, GLfloat y, GLfloat z);

   /* Available in both Common and Common-Lite profiles */
   void         (*glAlphaFuncx) (GLenum func, GLclampx ref);
   void         (*glClearColorx) (GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);
   void         (*glClearDepthx) (GLclampx depth);
   void         (*glClientActiveTexture) (GLenum texture);
   void         (*glClipPlanex) (GLenum plane, const GLfixed *equation);
   void         (*glColor4ub) (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
   void         (*glColor4x) (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
   void         (*glColorPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void         (*glDepthRangex) (GLclampx zNear, GLclampx zFar);
   void         (*glDisableClientState) (GLenum array);
   void         (*glEnableClientState) (GLenum array);
   void         (*glFogx) (GLenum pname, GLfixed param);
   void         (*glFogxv) (GLenum pname, const GLfixed *params);
   void         (*glFrustumx) (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
   void         (*glGetClipPlanex) (GLenum pname, GLfixed eqn[4]);
   void         (*glGetFixedv) (GLenum pname, GLfixed *params);
   void         (*glGetLightxv) (GLenum light, GLenum pname, GLfixed *params);
   void         (*glGetMaterialxv) (GLenum face, GLenum pname, GLfixed *params);
   void         (*glGetPointerv) (GLenum pname, GLvoid **params);
   void         (*glGetTexEnviv) (GLenum env, GLenum pname, GLint *params);
   void         (*glGetTexEnvxv) (GLenum env, GLenum pname, GLfixed *params);
   void         (*glGetTexParameterxv) (GLenum target, GLenum pname, GLfixed *params);
   void         (*glLightModelx) (GLenum pname, GLfixed param);
   void         (*glLightModelxv) (GLenum pname, const GLfixed *params);
   void         (*glLightx) (GLenum light, GLenum pname, GLfixed param);
   void         (*glLightxv) (GLenum light, GLenum pname, const GLfixed *params);
   void         (*glLineWidthx) (GLfixed width);
   void         (*glLoadIdentity) (void);
   void         (*glLoadMatrixx) (const GLfixed *m);
   void         (*glLogicOp) (GLenum opcode);
   void         (*glMaterialx) (GLenum face, GLenum pname, GLfixed param);
   void         (*glMaterialxv) (GLenum face, GLenum pname, const GLfixed *params);
   void         (*glMatrixMode) (GLenum mode);
   void         (*glMultMatrixx) (const GLfixed *m);
   void         (*glMultiTexCoord4x) (GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
   void         (*glNormal3x) (GLfixed nx, GLfixed ny, GLfixed nz);
   void         (*glNormalPointer) (GLenum type, GLsizei stride, const GLvoid *pointer);
   void         (*glOrthox) (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
   void         (*glPointParameterx) (GLenum pname, GLfixed param);
   void         (*glPointParameterxv) (GLenum pname, const GLfixed *params);
   void         (*glPointSizex) (GLfixed size);
   void         (*glPolygonOffsetx) (GLfixed factor, GLfixed units);
   void         (*glPopMatrix) (void);
   void         (*glPushMatrix) (void);
   void         (*glRotatex) (GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
   void         (*glSampleCoveragex) (GLclampx value, GLboolean invert);
   void         (*glScalex) (GLfixed x, GLfixed y, GLfixed z);
   void         (*glShadeModel) (GLenum mode);
   void         (*glTexCoordPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void         (*glTexEnvi) (GLenum target, GLenum pname, GLint param);
   void         (*glTexEnvx) (GLenum target, GLenum pname, GLfixed param);
   void         (*glTexEnviv) (GLenum target, GLenum pname, const GLint *params);
   void         (*glTexEnvxv) (GLenum target, GLenum pname, const GLfixed *params);
   void         (*glTexParameterx) (GLenum target, GLenum pname, GLfixed param);
   void         (*glTexParameterxv) (GLenum target, GLenum pname, const GLfixed *params);
   void         (*glTranslatex) (GLfixed x, GLfixed y, GLfixed z);
   void         (*glVertexPointer) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   /** @} */

   /**
    * @name OpenGL-ES 1.1 extensions
    *
    * Evas_GL_API version 2.
    *
    * OpenGL-ES 1.1 specifies a set of extensions on top of OpenGL-ES 1.0.
    * When available, Evas GL will expose these extensions with the following
    * function pointers.
    * @{
    */
   /* GL_OES_blend_equation_separate */
   void         (*glBlendEquationSeparateOES) (GLenum modeRGB, GLenum modeAlpha);

   /* GL_OES_blend_func_separate */
   void         (*glBlendFuncSeparateOES) (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);

   /* GL_OES_blend_subtract */
   void         (*glBlendEquationOES) (GLenum mode);

   /* GL_OES_draw_texture */
   void         (*glDrawTexsOES) (GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
   void         (*glDrawTexiOES) (GLint x, GLint y, GLint z, GLint width, GLint height);
   void         (*glDrawTexxOES) (GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
   void         (*glDrawTexsvOES) (const GLshort *coords);
   void         (*glDrawTexivOES) (const GLint *coords);
   void         (*glDrawTexxvOES) (const GLfixed *coords);
   void         (*glDrawTexfOES) (GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
   void         (*glDrawTexfvOES) (const GLfloat *coords);

   /* GL_OES_fixed_point */
   void         (*glAlphaFuncxOES) (GLenum func, GLclampx ref);
   void         (*glClearColorxOES) (GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);
   void         (*glClearDepthxOES) (GLclampx depth);
   void         (*glClipPlanexOES) (GLenum plane, const GLfixed *equation);
   void         (*glColor4xOES) (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
   void         (*glDepthRangexOES) (GLclampx zNear, GLclampx zFar);
   void         (*glFogxOES) (GLenum pname, GLfixed param);
   void         (*glFogxvOES) (GLenum pname, const GLfixed *params);
   void         (*glFrustumxOES) (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
   void         (*glGetClipPlanexOES) (GLenum pname, GLfixed eqn[4]);
   void         (*glGetFixedvOES) (GLenum pname, GLfixed *params);
   void         (*glGetLightxvOES) (GLenum light, GLenum pname, GLfixed *params);
   void         (*glGetMaterialxvOES) (GLenum face, GLenum pname, GLfixed *params);
   void         (*glGetTexEnvxvOES) (GLenum env, GLenum pname, GLfixed *params);
   void         (*glGetTexParameterxvOES) (GLenum target, GLenum pname, GLfixed *params);
   void         (*glLightModelxOES) (GLenum pname, GLfixed param);
   void         (*glLightModelxvOES) (GLenum pname, const GLfixed *params);
   void         (*glLightxOES) (GLenum light, GLenum pname, GLfixed param);
   void         (*glLightxvOES) (GLenum light, GLenum pname, const GLfixed *params);
   void         (*glLineWidthxOES) (GLfixed width);
   void         (*glLoadMatrixxOES) (const GLfixed *m);
   void         (*glMaterialxOES) (GLenum face, GLenum pname, GLfixed param);
   void         (*glMaterialxvOES) (GLenum face, GLenum pname, const GLfixed *params);
   void         (*glMultMatrixxOES) (const GLfixed *m);
   void         (*glMultiTexCoord4xOES) (GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
   void         (*glNormal3xOES) (GLfixed nx, GLfixed ny, GLfixed nz);
   void         (*glOrthoxOES) (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
   void         (*glPointParameterxOES) (GLenum pname, GLfixed param);
   void         (*glPointParameterxvOES) (GLenum pname, const GLfixed *params);
   void         (*glPointSizexOES) (GLfixed size);
   void         (*glPolygonOffsetxOES) (GLfixed factor, GLfixed units);
   void         (*glRotatexOES) (GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
   void         (*glSampleCoveragexOES) (GLclampx value, GLboolean invert);
   void         (*glScalexOES) (GLfixed x, GLfixed y, GLfixed z);
   void         (*glTexEnvxOES) (GLenum target, GLenum pname, GLfixed param);
   void         (*glTexEnvxvOES) (GLenum target, GLenum pname, const GLfixed *params);
   void         (*glTexParameterxOES) (GLenum target, GLenum pname, GLfixed param);
   void         (*glTexParameterxvOES) (GLenum target, GLenum pname, const GLfixed *params);
   void         (*glTranslatexOES) (GLfixed x, GLfixed y, GLfixed z);

   /* GL_OES_framebuffer_object */
   GLboolean    (*glIsRenderbufferOES) (GLuint renderbuffer);
   void         (*glBindRenderbufferOES) (GLenum target, GLuint renderbuffer);
   void         (*glDeleteRenderbuffersOES) (GLsizei n, const GLuint* renderbuffers);
   void         (*glGenRenderbuffersOES) (GLsizei n, GLuint* renderbuffers);
   void         (*glRenderbufferStorageOES) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
   void         (*glGetRenderbufferParameterivOES) (GLenum target, GLenum pname, GLint* params);
   GLboolean    (*glIsFramebufferOES) (GLuint framebuffer);
   void         (*glBindFramebufferOES) (GLenum target, GLuint framebuffer);
   void         (*glDeleteFramebuffersOES) (GLsizei n, const GLuint* framebuffers);
   void         (*glGenFramebuffersOES) (GLsizei n, GLuint* framebuffers);
   GLenum       (*glCheckFramebufferStatusOES) (GLenum target);
   void         (*glFramebufferRenderbufferOES) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
   void         (*glFramebufferTexture2DOES) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
   void         (*glGetFramebufferAttachmentParameterivOES) (GLenum target, GLenum attachment, GLenum pname, GLint* params);
   void         (*glGenerateMipmapOES) (GLenum target);

   /* GL_OES_matrix_palette */
   void         (*glCurrentPaletteMatrixOES) (GLuint matrixpaletteindex);
   void         (*glLoadPaletteFromModelViewMatrixOES) (void);
   void         (*glMatrixIndexPointerOES) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void         (*glWeightPointerOES) (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

   /* GL_OES_query_matrix */
   GLbitfield   (*glQueryMatrixxOES) (GLfixed mantissa[16], GLint exponent[16]);

   /* GL_OES_single_precision */
   void         (*glDepthRangefOES) (GLclampf zNear, GLclampf zFar);
   void         (*glFrustumfOES) (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
   void         (*glOrthofOES) (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
   void         (*glClipPlanefOES) (GLenum plane, const GLfloat *equation);
   void         (*glGetClipPlanefOES) (GLenum pname, GLfloat eqn[4]);
   void         (*glClearDepthfOES) (GLclampf depth);

   /* GL_OES_texture_cube_map */
   void         (*glTexGenfOES) (GLenum coord, GLenum pname, GLfloat param);
   void         (*glTexGenfvOES) (GLenum coord, GLenum pname, const GLfloat *params);
   void         (*glTexGeniOES) (GLenum coord, GLenum pname, GLint param);
   void         (*glTexGenivOES) (GLenum coord, GLenum pname, const GLint *params);
   void         (*glTexGenxOES) (GLenum coord, GLenum pname, GLfixed param);
   void         (*glTexGenxvOES) (GLenum coord, GLenum pname, const GLfixed *params);
   void         (*glGetTexGenfvOES) (GLenum coord, GLenum pname, GLfloat *params);
   void         (*glGetTexGenivOES) (GLenum coord, GLenum pname, GLint *params);
   void         (*glGetTexGenxvOES) (GLenum coord, GLenum pname, GLfixed *params);

   /* GL_OES_vertex_array_object */
   void         (*glBindVertexArrayOES) (GLuint array);
   void         (*glDeleteVertexArraysOES) (GLsizei n, const GLuint *arrays);
   void         (*glGenVertexArraysOES) (GLsizei n, GLuint *arrays);
   GLboolean    (*glIsVertexArrayOES) (GLuint array);

   /* GL_APPLE_copy_texture_levels */
   void         (*glCopyTextureLevelsAPPLE) (GLuint destinationTexture, GLuint sourceTexture, GLint sourceBaseLevel, GLsizei sourceLevelCount);
   void         (*glRenderbufferStorageMultisampleAPPLE) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
   void         (*glResolveMultisampleFramebufferAPPLE) (void);
   GLsync       (*glFenceSyncAPPLE) (GLenum condition, GLbitfield flags);
   GLboolean    (*glIsSyncAPPLE) (GLsync sync);
   void         (*glDeleteSyncAPPLE) (GLsync sync);
   GLenum       (*glClientWaitSyncAPPLE) (GLsync sync, GLbitfield flags, EvasGLuint64 timeout);
   void         (*glWaitSyncAPPLE) (GLsync sync, GLbitfield flags, EvasGLuint64 timeout);
   void         (*glGetInteger64vAPPLE) (GLenum pname, EvasGLint64 *params);
   void         (*glGetSyncivAPPLE) (GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);

   /* GL_EXT_map_buffer_range */
   void*        (*glMapBufferRangeEXT) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
   void         (*glFlushMappedBufferRangeEXT) (GLenum target, GLintptr offset, GLsizeiptr length);

   /* GL_EXT_multisampled_render_to_texture */
   void         (*glRenderbufferStorageMultisampleEXT) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
   void         (*glFramebufferTexture2DMultisampleEXT) (GLenum, GLenum, GLenum, GLuint, GLint, GLsizei);

   /* GL_EXT_robustness */
   GLenum       (*glGetGraphicsResetStatusEXT) (void);
   void         (*glReadnPixelsEXT) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
   void         (*glGetnUniformfvEXT) (GLuint program, GLint location, GLsizei bufSize, float *params);
   void         (*glGetnUniformivEXT) (GLuint program, GLint location, GLsizei bufSize, GLint *params);

   /* GL_EXT_texture_storage */
   void         (*glTexStorage1DEXT) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
   void         (*glTexStorage2DEXT) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
   void         (*glTexStorage3DEXT) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
   void         (*glTextureStorage1DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
   void         (*glTextureStorage2DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
   void         (*glTextureStorage3DEXT) (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);

   /* GL_IMG_user_clip_plane */
   void         (*glClipPlanefIMG) (GLenum, const GLfloat *);
   void         (*glClipPlanexIMG) (GLenum, const GLfixed *);

   /* GL_IMG_multisampled_render_to_texture */
   void         (*glRenderbufferStorageMultisampleIMG) (GLenum, GLsizei, GLenum, GLsizei, GLsizei);
   void         (*glFramebufferTexture2DMultisampleIMG) (GLenum, GLenum, GLenum, GLuint, GLint, GLsizei);

   /* GL_QCOM_tiled_rendering */
   void         (*glStartTilingQCOM) (GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask);
   void         (*glEndTilingQCOM) (GLbitfield preserveMask);


   /*------- EvasGL / EGL-related functions -------*/
   /**
    * @name Evas GL Sync object functions
    * @since_tizen 2.3
    * @{ */
   /**
    * @anchor evasglCreateSync
    * @brief Requires the extension @c EGL_KHR_fence_sync, similar to eglCreateSyncKHR.
    */
   EvasGLSync   (*evasglCreateSync) (Evas_GL *evas_gl, unsigned int type, const int *attrib_list);
   /** @anchor evasglDestroySync
    * @brief Requires the extension @c EGL_KHR_fence_sync, similar to eglDestroySyncKHR.
    */
   Eina_Bool    (*evasglDestroySync) (Evas_GL *evas_gl, EvasGLSync sync);
   /** @anchor evasglClientWaitSync
    * @brief Requires the extension @c EGL_KHR_fence_sync, similar to eglClientWaitSyncKHR.
    */
   int          (*evasglClientWaitSync) (Evas_GL *evas_gl, EvasGLSync sync, int flags, EvasGLTime timeout);
   /** @anchor evasglSignalSync
    * @brief Requires the extension @c EGL_KHR_reusable_sync, similar to eglSignalSyncKHR.
    */
   Eina_Bool    (*evasglSignalSync) (Evas_GL *evas_gl, EvasGLSync sync, unsigned mode);
   /** @anchor evasglGetSyncAttrib
    * @brief Requires the extension @c EGL_KHR_fence_sync, similar to eglGetSyncAttribKHR.
    */
   Eina_Bool    (*evasglGetSyncAttrib) (Evas_GL *evas_gl, EvasGLSync sync, int attribute, int *value);
   /** @anchor evasglWaitSync
    * @brief Requires the extension @c EGL_KHR_wait_sync, similar to eglWaitSyncKHR.
    */
   int          (*evasglWaitSync) (Evas_GL *evas_gl, EvasGLSync sync, int flags);
   /** @} */

   /* future calls will be added down here for expansion */
};


#ifdef __cplusplus
}
#endif

#endif
/**
 * @}
 */
