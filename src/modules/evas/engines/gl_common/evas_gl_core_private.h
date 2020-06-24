#ifndef _EVAS_GL_CORE_PRIVATE_H
#define _EVAS_GL_CORE_PRIVATE_H
#include "evas_gl_private.h"
#include "evas_gl_core.h"
#include "evas_gl_api_ext.h"
#define EVAS_GL_NO_GL_H_CHECK 1
#include "Evas_GL.h"

//#include "evas_gl_ext.h"

extern int _evas_gl_log_dom;

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_evas_gl_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_evas_gl_log_dom, __VA_ARGS__)
#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_evas_gl_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_evas_gl_log_dom, __VA_ARGS__)

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_evas_gl_log_dom, __VA_ARGS__)


struct _EVGL_Interface
{
   // Returns the native display of evas engine.
   void       *(*display_get)(void *data);

   // Returns the Window surface that evas uses for direct rendering opt
   void       *(*evas_surface_get)(void *data);
   void       *(*native_window_create)(void *data);
   int        (*native_window_destroy)(void *data, void *window);

   // Creates/Destroys the native surface from evas engine.
   void       *(*surface_create)(void *data, void *native_window);
   int         (*surface_destroy)(void *data, void *surface);

   // Creates/Destroys the native surface from evas engine.
   void       *(*context_create)(void *data, void *share_ctx, Evas_GL_Context_Version version);
   int         (*context_destroy)(void *data, void *context);

   // Calls the make_current from evas_engine.
   int         (*make_current)(void *data, void *surface, void *context, int flush);

   // Returns the get proc_address function
   void       *(*proc_address_get)(const char *name);

   // Returns the string of supported extensions
   const char *(*ext_string_get)(void *data);

   // Returns the current rotation angle of evas
   int         (*rotation_angle_get)(void *data);

   // Create a pbuffer surface
   void       *(*pbuffer_surface_create)(void *data, EVGL_Surface *evgl_sfc, const int *attrib_list);
   int         (*pbuffer_surface_destroy)(void *data, void *surface);

   // Create a surface for 1.x & 3.x rendering (could be pbuffer or xpixmap for instance)
   void       *(*indirect_surface_create)(EVGL_Engine *evgl, void *data, EVGL_Surface *evgl_sfc, Evas_GL_Config *cfg, int w, int h);

   // Destroy 1.x & 3.x surface (could be pbuffer or xpixmap for instance)
   int         (*indirect_surface_destroy)(void *data, EVGL_Surface *evgl_sfc);

   // Create an indirect rendering context for GLES 1.x
   void       *(*gles_context_create)(void *data, EVGL_Context *share_ctx, EVGL_Surface *evgl_sfc);

   // Check native window surface config for Evas GL Direct Rendering
   void        (*native_win_surface_config_get)(void *data, int *win_depth, int *win_stencil, int *win_msaa);
};

struct _EVGL_Surface
{
   int w, h;

   //-------------------------//
   // Related to FBO Surface

   // MSAA
   GLint   msaa_samples;

   // Color Buffer Target
   GLuint  color_buf;
   GLint   color_ifmt;
   GLenum  color_fmt;

   // Depth Buffer Target
   GLuint  depth_buf;
   GLenum  depth_fmt;

   // Stencil Buffer Target
   GLuint  stencil_buf;
   GLenum  stencil_fmt;

   // Depth_Stencil Target
   GLuint  depth_stencil_buf;
   GLenum  depth_stencil_fmt;

   // Direct Rendering Options
   unsigned direct_fb_opt : 1;
   unsigned client_side_rotation : 1;
   unsigned alpha : 1;

   // Flag indicating this surface is used for indirect rendering
   unsigned indirect : 1;
   unsigned yinvert : 1;

   // Moved from evgl_engine
   unsigned direct_override : 1;
   unsigned direct_mem_opt : 1;

   // Init Flag
   unsigned buffers_skip_allocate : 1;
   unsigned buffers_allocated : 1;

   void   *cfg;
   int     cfg_index;


   // Rough estimate of buffer in memory per renderbuffer
   // 0. color 1. depth 2. stencil 3. depth_stencil
   int     buffer_mem[4];

   //-------------------------//
   // Used if indirect == 1
   EVGLNative_Surface indirect_sfc;
   void              *indirect_sfc_native;
   void              *indirect_sfc_visual;
   void              *indirect_sfc_config;
   void              *egl_image;

   //-------------------------//
   // Related to PBuffer Surface
   struct {
      EVGLNative_Surface    native_surface;
      Evas_GL_Color_Format  color_fmt;
      Eina_Bool             is_pbuffer : 1;
   } pbuffer;


   //-------------------------//

   EVGL_Context *current_ctx;
};



struct _EVGL_Context
{
   EVGLNative_Context context;

   Evas_GL_Context_Version version;
   int                     version_minor;

   // Context FBO
   GLuint       surface_fbo;

   // Current FBO
   GLuint       current_fbo;
   GLuint       current_draw_fbo;    //for GLES3
   GLuint       current_read_fbo;    //for GLES3

   // Direct Rendering Related
   unsigned     scissor_enabled : 1;
   unsigned     scissor_updated : 1;
   unsigned     direct_scissor : 1;
   unsigned     viewport_updated : 1;
   unsigned     extension_checked : 1;
   unsigned     fbo_image_supported : 1;
   unsigned     pixmap_image_supported : 1;

   int          scissor_coord[4];
   int          viewport_coord[4];
   int          viewport_direct[4];

   // For GLES1/GLES3 with indirect rendering
   EVGLNative_Context indirect_context;

   // Partial Rendering
   int          partial_render;

   EVGL_Surface *current_sfc;

   //glGetError
   GLenum gl_error;
};

typedef enum _EVGL_Color_Bit
{
   COLOR_NONE      = 0,
   COLOR_RGB_888   = 0x1,
   COLOR_RGBA_8888 = 0x3,
} EVGL_Color_Bit;


typedef enum _EVGL_Depth_Bit
{
   DEPTH_NONE   = 0,
   DEPTH_BIT_8  = 0x1,
   DEPTH_BIT_16 = 0x3,
   DEPTH_BIT_24 = 0x7,
   DEPTH_BIT_32 = 0xF,
   DEPTH_STENCIL = 0xFF,
} EVGL_Depth_Bit;

typedef enum _EVGL_Stencil_Bit
{
   STENCIL_NONE   = 0,
   STENCIL_BIT_1  = 0x1,
   STENCIL_BIT_2  = 0x3,
   STENCIL_BIT_4  = 0x7,
   STENCIL_BIT_8  = 0xF,
   STENCIL_BIT_16 = 0x1F,
} EVGL_Stencil_Bit;


struct _EVGL_Surface_Format
{
   int index;

   EVGL_Color_Bit       color_bit;
   GLint                color_ifmt;
   GLenum               color_fmt;

   EVGL_Depth_Bit       depth_bit;
   GLenum               depth_fmt;

   EVGL_Stencil_Bit     stencil_bit;
   GLenum               stencil_fmt;

   GLenum               depth_stencil_fmt;

   int                  samples;
};

struct _EVGL_Cap
{
   EVGL_Surface_Format  fbo_fmts[100];
   int                  num_fbo_fmts;

   int                  max_w;
   int                  max_h;

   int                  msaa_supported;
   int                  msaa_samples[3];  // High, Med, Low
};

struct _EVGL_Resource
{
   Eina_Thread id;

   EVGLNative_Display   display;
   EVGLNative_Context   context;
   EVGLNative_Window    window;
   EVGLNative_Surface   surface;

   EVGL_Context        *current_ctx;
   void                *current_eng;

   int error_state;

   struct {
        EVGLNative_Surface   surface;
        int                  rendered;

        int                  rot;
        int                  win_w;
        int                  win_h;

        struct {
             int             x, y, w, h;
        } img;

        struct {
             int             x, y, w, h;
        } clip;

        struct {
             int             preserve;
             Eina_Bool       enabled : 1;
        } partial;

        Eina_Bool            enabled : 1;
        Eina_Bool            in_get_pixels : 1;
        Eina_Bool            render_op_copy : 1;
   } direct;
   struct {
        GLclampf r, g, b, a;
   } clear_color;
   struct {
        void *data;
        void *surface;
        void *context;
   } stored;

};

struct _EVGL_Engine
{
   int initted;

   const EVGL_Interface     *funcs;

   EVGL_Cap            caps;

   const char         *gl_ext;
   const char         *evgl_ext;

   // Resource context/surface per Thread in TLS for evasgl use
   LK(resource_lock);
   Eina_TLS           resource_key;
   Eina_List         *resource_list;
   Eina_Thread        main_tid;

   // Add more debug logs (DBG levels 4 and 6)
   int                api_debug_mode;

   // Force Off for Debug purposes
   int                direct_force_off;

   // Other DR flags for debugging purposes
   int                direct_override;
   int                direct_mem_opt;

   // Force Direct Scissoring off for Debug purposes
   int                direct_scissor_off;

   // Keep track of all the current surfaces/contexts
   Eina_List         *surfaces;
   Eina_List         *contexts;

   //void              *engine_data;
   Eina_Hash         *safe_extensions;
};


// Evas GL Engine
extern EVGL_Engine   *evgl_engine;

// Internally used functions
extern void           _evgl_api_gles2_get(Evas_GL_API *api, Eina_Bool debug);
extern void           _evgl_api_gles1_get(Evas_GL_API *api, Eina_Bool debug);
extern void           _evgl_api_gles3_get(Evas_GL_API *funcs, void *(*get_proc_address)(const char *), Eina_Bool debug, int minor_version);
extern EVGL_Resource *_evgl_tls_resource_get(void);
extern EVGL_Resource *_evgl_tls_resource_create(void *data);
extern void           _evgl_tls_resource_destroy(void *data);
extern EVGL_Context  *_evgl_current_context_get(void);
extern int            _evgl_not_in_pixel_get(void);
extern int            _evgl_direct_enabled(void);
extern EVGLNative_Context _evgl_native_context_get(Evas_GL_Context *ctx);
extern void          *_evgl_engine_data_get(Evas_GL *evasgl);
Eina_Bool             _evgl_api_gles2_ext_init(void *getproc, const char *glueexts);
Eina_Bool             _evgl_api_gles1_ext_init(void *getproc, const char *glueexts);
Eina_Bool             _evgl_api_gles3_ext_init(void *getproc, const char *glueexts);
Evas_GL_API*          _evgl_api_gles1_internal_get(void);
Evas_GL_API*          _evgl_api_gles3_internal_get(void);

#endif //_EVAS_GL_CORE_PRIVATE_H
