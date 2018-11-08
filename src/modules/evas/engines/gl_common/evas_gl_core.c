#include "evas_gl_core_private.h"

#ifndef _WIN32
# include <dlfcn.h>
#endif

// EVGL GL Format Pair
typedef struct _GL_Format
{
   int    bit;
   GLenum fmt;
} GL_Format;

// Extended struct size based on the 314 functions found in gl31.h
#define EVAS_GL_API_STRUCT_SIZE (sizeof(Evas_GL_API) + 300 * sizeof(void*))
static Evas_GL_API *gles1_funcs = NULL;
static Evas_GL_API *gles2_funcs = NULL;
static Evas_GL_API *gles3_funcs = NULL;

EVGL_Engine *evgl_engine = NULL;
int _evas_gl_log_dom   = -1;
int _evas_gl_log_level = -1;

typedef void           *(*glsym_func_void_ptr) ();
glsym_func_void_ptr glsym_evas_gl_native_context_get = NULL;
glsym_func_void_ptr glsym_evas_gl_engine_data_get = NULL;

static void _surface_cap_print(int error);
static void _surface_context_list_print();
static void _internal_resources_destroy(void *eng_data, EVGL_Resource *rsc);
static void *_egl_image_create(EVGL_Context *context, int target, void *buffer);
static void _egl_image_destroy(void *image);
static int _evgl_direct_renderable(EVGL_Resource *rsc, EVGL_Surface *sfc);

// NOTE: These constants are "hidden", kinda non public API. They should not
// be used unless you know exactly what you are doing.
// These vars replace environment variables.
#define EVAS_GL_OPTIONS_DIRECT_MEMORY_OPTIMIZE 0x1000
#define EVAS_GL_OPTIONS_DIRECT_OVERRIDE        0x2000

extern void (*EXT_FUNC_GLES1(glGenFramebuffersOES)) (GLsizei n, GLuint* framebuffers);
extern void (*EXT_FUNC_GLES1(glBindFramebufferOES)) (GLenum target, GLuint framebuffer);
extern void (*EXT_FUNC_GLES1(glFramebufferTexture2DOES)) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void (*EXT_FUNC_GLES1(glFramebufferRenderbufferOES)) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern GLenum (*EXT_FUNC_GLES1(glCheckFramebufferStatusOES)) (GLenum target);

//---------------------------------------------------------------//
// Internal Resources:
//  - Surface and Context used for internal buffer creation
//---------------------------------------------------------------//
static void *
_internal_resources_create(void *eng_data)
{
   EVGL_Resource *rsc = NULL;

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("EVGL Engine not initialized!");
        return NULL;
     }

   // Allocate resource
   rsc = calloc(1, sizeof(EVGL_Resource));
   if (!rsc)
     {
        ERR("Error allocating EVGL_Resource");
        return NULL;
     }
   rsc->id = eina_thread_self();
   rsc->error_state = EVAS_GL_SUCCESS;

   // Get display
   rsc->display = evgl_engine->funcs->display_get(eng_data);
   if (!rsc->display)
     {
        ERR("Error getting display");
        goto error;
     }

   return rsc;

error:
   _internal_resources_destroy(eng_data, rsc);
   return NULL;
}

static void
_internal_resources_destroy(void *eng_data, EVGL_Resource *rsc)
{
   if ((!eng_data) || (!rsc)) return;

   if (rsc->context)
     evgl_engine->funcs->context_destroy(eng_data, rsc->context);
   if (rsc->surface)
     evgl_engine->funcs->surface_destroy(eng_data, rsc->surface);
   if (rsc->window)
     evgl_engine->funcs->native_window_destroy(eng_data, rsc->window);

   free(rsc);
}

static int
_internal_resource_make_current(void *eng_data, EVGL_Surface *sfc, EVGL_Context *ctx)
{
   EVGL_Resource *rsc = NULL;
   void *surface = NULL;
   void *context = NULL;
   int ret = 0;

   // Retrieve the resource object
   if (!(rsc = _evgl_tls_resource_get()))
     {
        if (!(rsc = _evgl_tls_resource_create(eng_data)))
          {
             ERR("Error creating resources in tls.");
             return 0;
          }
     }

   // Set context from input or from resource
   if (ctx)
     context = ctx->context;
   else
     {
        if (!rsc->context)
          {
             // Create a resource context
             rsc->context = evgl_engine->funcs->context_create(eng_data, NULL, EVAS_GL_GLES_2_X);
             if (!rsc->context)
               {
                  ERR("Internal resource context creation failed.");
                  return 0;
               }
          }

        context = (void*)rsc->context;
     }

   if (sfc)
     {
        if (_evgl_direct_renderable(rsc, sfc)) // Direct rendering
          {
             // Do Nothing
          }
        else if ((ctx) && (ctx->pixmap_image_supported)) // Pixmap surface
          {
             if (!sfc->indirect_sfc)
               {
                  evgl_engine->funcs->indirect_surface_create(evgl_engine, eng_data, sfc, sfc->cfg, sfc->w, sfc->h);
                  if (sfc->egl_image) _egl_image_destroy(sfc->egl_image);
                  sfc->egl_image = _egl_image_create(NULL, EVAS_GL_NATIVE_PIXMAP, sfc->indirect_sfc_native);
               }
             surface = (void*)sfc->indirect_sfc;

             if (!ctx->indirect_context)
               ctx->indirect_context = evgl_engine->funcs->gles_context_create(eng_data, ctx, sfc);
             context = (void*)ctx->indirect_context;
          }
        else if (sfc->pbuffer.native_surface) // Pbuffer surface
          {
             surface = (void*)sfc->pbuffer.native_surface;
          }
        else // FBO
          {
             // Do Nothing
          }
     }

   if (!surface)
     {
        // Set the surface to evas surface if it's there
        if (rsc->id == evgl_engine->main_tid)
           rsc->direct.surface = evgl_engine->funcs->evas_surface_get(eng_data);

        if (rsc->direct.surface)
           surface = (void*)rsc->direct.surface;
        else
          {
             if (!rsc->window)
               {
                  // Create resource surface
                  rsc->window = evgl_engine->funcs->native_window_create(eng_data);
                  if (!rsc->window)
                    {
                       ERR("Error creating native window");
                       return 0;
                    }
               }

             if (!rsc->surface)
               {
                  rsc->surface = evgl_engine->funcs->surface_create(eng_data, rsc->window);
                  if (!rsc->surface)
                    {
                       ERR("Error creating native surface");
                       return 0;
                    }
               }

             surface = (void*)rsc->surface;
          }
     }

   // Do the make current
   if (evgl_engine->api_debug_mode)
     DBG("Calling make_current(%p, %p)", surface, context);
   ret = evgl_engine->funcs->make_current(eng_data, surface, context, 1);
   if (!ret)
     {
        ERR("Engine make_current with internal resources failed.");
        return 0;
     }

   return 1;
}

//---------------------------------------------------------------//
// Surface Related Functions
//  - Texture/ Renderbuffer Creation/ Attachment to FBO
//  - Surface capability check
//  - Internal config choose function
//---------------------------------------------------------------//
// Gen Texture
static void
_texture_create(GLuint *tex)
{
   glGenTextures(1, tex);
}

// Create and allocate 2D texture
static void
_texture_allocate_2d(GLuint tex, GLint ifmt, GLenum fmt, GLenum type, int w, int h)
{
   //if (!(*tex))
   //   glGenTextures(1, tex);
   GLint curr_tex = 0;
   glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_tex);

   glBindTexture(GL_TEXTURE_2D, tex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, ifmt, w, h, 0, fmt, type, NULL);
   glBindTexture(GL_TEXTURE_2D, (GLuint)curr_tex);
}

// Destroy Texture
static void
_texture_destroy(GLuint *tex)
{
   GLint curr_tex = 0;
   glGetIntegerv(GL_TEXTURE_BINDING_2D, &curr_tex);

   if ((GLuint)curr_tex == *tex) glBindTexture(GL_TEXTURE_2D, 0);
   if (*tex)
     {
        glDeleteTextures(1, tex);
        *tex = 0;
     }
}

// Attach 2D texture with the given format to already bound FBO
// *NOTE: attach2 here is used for depth_stencil attachment in GLES env.
static void
_texture_attach_2d(GLuint tex, GLenum attach, GLenum attach2, int samples, Evas_GL_Context_Version version)
{
   if (samples && (version == EVAS_GL_GLES_2_X))
     {
#ifdef GL_GLES
        if (EXT_FUNC(glFramebufferTexture2DMultisample))
          {
             EXT_FUNC(glFramebufferTexture2DMultisample)(GL_FRAMEBUFFER,
                                                         attach,
                                                         GL_TEXTURE_2D, tex,
                                                         0, samples);

             if (attach2)
               EXT_FUNC(glFramebufferTexture2DMultisample)(GL_FRAMEBUFFER,
                                                           attach2,
                                                           GL_TEXTURE_2D, tex,
                                                           0, samples);
          }
        else
#endif
        ERR("MSAA not supported.  Should not have come in here...!");
     }
   else if (version == EVAS_GL_GLES_1_X)
     {
        if (EXT_FUNC_GLES1(glFramebufferTexture2DOES))
          EXT_FUNC_GLES1(glFramebufferTexture2DOES)(GL_FRAMEBUFFER, attach, GL_TEXTURE_2D, tex, 0);

        if (attach2)
          if (EXT_FUNC_GLES1(glFramebufferTexture2DOES))
            EXT_FUNC_GLES1(glFramebufferTexture2DOES)(GL_FRAMEBUFFER, attach2, GL_TEXTURE_2D, tex, 0);
     }
   else
     {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attach, GL_TEXTURE_2D, tex, 0);

        if (attach2)
           glFramebufferTexture2D(GL_FRAMEBUFFER, attach2, GL_TEXTURE_2D, tex, 0);

     }
}

static void *
_egl_image_create(EVGL_Context *context, int target, void *buffer)
{
#ifdef GL_GLES
   EGLDisplay dpy = EGL_NO_DISPLAY;
   EGLContext ctx = EGL_NO_CONTEXT;
   EVGL_Resource *rsc = NULL;
   int attribs[10];
   int n = 0;

   // Retrieve the resource object
   if (!(rsc = _evgl_tls_resource_get()))
     {
        ERR("Error creating resources in tls.");
        return NULL;
     }

   dpy = (EGLDisplay)rsc->display;
   if (target == EGL_GL_TEXTURE_2D_KHR)
     {
        ctx = (EGLContext)context->context;
        attribs[n++] = EGL_GL_TEXTURE_LEVEL_KHR;
        attribs[n++] = 0;
     }
   attribs[n++] = EGL_IMAGE_PRESERVED_KHR;
   attribs[n++] = 0;
   attribs[n++] = EGL_NONE;

   return EXT_FUNC_EGL(eglCreateImage)(dpy, ctx, target, (EGLClientBuffer)(uintptr_t)buffer, attribs);
#else
   (void) context; (void) target; (void) buffer;
   return NULL;
#endif
}

static void
_egl_image_destroy(void *image)
{
#ifdef GL_GLES
   EGLDisplay dpy = EGL_NO_DISPLAY;
   EVGL_Resource *rsc = NULL;

   // Retrieve the resource object
   if (!(rsc = _evgl_tls_resource_get()))
     {
        ERR("Error creating resources in tls.");
        return;
     }

   dpy = (EGLDisplay)rsc->display;
   if (!dpy) return;

   EXT_FUNC_EGL(eglDestroyImage)(dpy, image);
#else
   (void) image;
#endif
}

static void
_framebuffer_create(GLuint *buf, Evas_GL_Context_Version version)
{
   if (version == EVAS_GL_GLES_1_X)
     {
        if (EXT_FUNC_GLES1(glGenFramebuffersOES))
            EXT_FUNC_GLES1(glGenFramebuffersOES)(1, buf);
     }
   else
     {
        glGenFramebuffers(1, buf);
     }
}

static void
_framebuffer_bind(GLuint buf, Evas_GL_Context_Version version)
{
   if (version == EVAS_GL_GLES_1_X)
     {
        if (EXT_FUNC_GLES1(glBindFramebufferOES))
          EXT_FUNC_GLES1(glBindFramebufferOES)(GL_FRAMEBUFFER, buf);
     }
   else
     {
        glBindFramebuffer(GL_FRAMEBUFFER, buf);
     }
}

static void
_framebuffer_draw_bind(GLuint buf, Evas_GL_Context_Version version)
{
   if (version == EVAS_GL_GLES_3_X)
     glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buf);
}

//This function is not needed in EvasGL backend engine with GLES 2.0.
//But It is useful when EvasGL backend works with GLES 3.X and use read buffers.
static void
_framebuffer_read_bind(GLuint buf, Evas_GL_Context_Version version)
{
   if (version == EVAS_GL_GLES_3_X)
     glBindFramebuffer(GL_READ_FRAMEBUFFER, buf);
}

static GLenum
_framebuffer_check(Evas_GL_Context_Version version)
{
   GLenum ret = 0;
   if (version == EVAS_GL_GLES_1_X)
     {
        if (EXT_FUNC_GLES1(glCheckFramebufferStatusOES))
          ret = EXT_FUNC_GLES1(glCheckFramebufferStatusOES)(GL_FRAMEBUFFER);
     }
   else
     {
        ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
     }
   return ret;
}

// Gen Renderbuffer
static void
_renderbuffer_create(GLuint *buf)
{
   glGenRenderbuffers(1, buf);
}


// Attach a renderbuffer with the given format to already bound FBO
static void
_renderbuffer_allocate(GLuint buf, GLenum fmt, int w, int h, int samples)
{
   glBindRenderbuffer(GL_RENDERBUFFER, buf);
   if (samples)
     {
        if (glsym_glRenderbufferStorageMultisample)
          glsym_glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, fmt, w, h);
        else
          {
#ifdef GL_GLES
             if (EXT_FUNC(glRenderbufferStorageMultisample))
               EXT_FUNC(glRenderbufferStorageMultisample)(GL_RENDERBUFFER, samples, fmt, w, h);
             else
#endif
               ERR("MSAA not supported.  Should not have come in here...!");
          }
     }
   else
     glRenderbufferStorage(GL_RENDERBUFFER, fmt, w, h);
   glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

static void
_renderbuffer_destroy(GLuint *buf)
{
   if (*buf)
     {
        glDeleteRenderbuffers(1, buf);
        *buf = 0;
     }
}

// Attach a renderbuffer with the given format to already bound FBO
static void
_renderbuffer_attach(GLuint buf, GLenum attach, Evas_GL_Context_Version version)
{
   if (version == EVAS_GL_GLES_1_X)
     {
        if (EXT_FUNC_GLES1(glFramebufferRenderbufferOES))
          EXT_FUNC_GLES1(glFramebufferRenderbufferOES)(GL_FRAMEBUFFER, attach, GL_RENDERBUFFER, buf);
     }
   else
     {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attach, GL_RENDERBUFFER, buf);
     }
}

// Check whether the given FBO surface config is supported by the driver
// TODO - we also should test with GLES3's formats.
static int
_fbo_surface_cap_test(GLint color_ifmt, GLenum color_fmt,
                      GLenum depth_fmt, GLenum stencil_fmt, int mult_samples)
{
   GLuint fbo = 0;
   GLuint color_buf = 0;
   GLuint depth_buf = 0;
   GLuint stencil_buf = 0;
   GLuint depth_stencil_buf = 0;
   int depth_stencil = 0;
   int fb_status = 0;
   int w = 2, h = 2;   // Test it with a simple (2,2) surface.  Should I test it with NPOT?
   Evas_GL_Context_Version ver = evas_gl_common_version_check(NULL);

   // Gen FBO
   glGenFramebuffers(1, &fbo);
   glBindFramebuffer(GL_FRAMEBUFFER, fbo);

   // FIXME: GLES 3 support for MSAA is NOT IMPLEMENTED!
   // Needs to use RenderbufferStorageMultisample + FramebufferRenderbuffer

   // Color Buffer Texture
   if ((color_ifmt) && (color_fmt))
     {
        _texture_create(&color_buf);
        _texture_allocate_2d(color_buf, color_ifmt, color_fmt, GL_UNSIGNED_BYTE, w, h);
        _texture_attach_2d(color_buf, GL_COLOR_ATTACHMENT0, 0, mult_samples, ver);
     }

      // Check Depth_Stencil Format First
#ifdef GL_GLES
   if ((depth_fmt == GL_DEPTH_STENCIL_OES) && (!mult_samples))
     {
        _texture_create(&depth_stencil_buf);
        _texture_allocate_2d(depth_stencil_buf, depth_fmt,
                           depth_fmt, GL_UNSIGNED_INT_24_8_OES, w, h);
        _texture_attach_2d(depth_stencil_buf, GL_DEPTH_ATTACHMENT,
                           GL_STENCIL_ATTACHMENT, mult_samples, EINA_FALSE);
        depth_stencil = 1;
     }
   else if ((depth_fmt == GL_DEPTH24_STENCIL8_OES) && (mult_samples))
#else
   if (depth_fmt == GL_DEPTH24_STENCIL8)
#endif
     {
        _renderbuffer_create(&depth_stencil_buf);
        _renderbuffer_allocate(depth_stencil_buf, depth_fmt, w, h, mult_samples);
#ifdef GL_GLES
        _renderbuffer_attach(depth_stencil_buf, GL_DEPTH_ATTACHMENT, EINA_FALSE);
        _renderbuffer_attach(depth_stencil_buf, GL_STENCIL_ATTACHMENT, EINA_FALSE);
#else
        _renderbuffer_attach(depth_stencil_buf, GL_DEPTH_STENCIL_ATTACHMENT, EINA_FALSE);
#endif
        depth_stencil = 1;
     }

   // Depth Attachment
   if ((!depth_stencil) && (depth_fmt))
     {
        _renderbuffer_create(&depth_buf);
        _renderbuffer_allocate(depth_buf, depth_fmt, w, h, mult_samples);
        _renderbuffer_attach(depth_buf, GL_DEPTH_ATTACHMENT, EVAS_GL_GLES_2_X);
     }

   // Stencil Attachment
   if ((!depth_stencil) && (stencil_fmt))
     {
        _renderbuffer_create(&stencil_buf);
        _renderbuffer_allocate(stencil_buf, stencil_fmt, w, h, mult_samples);
        _renderbuffer_attach(stencil_buf, GL_STENCIL_ATTACHMENT, EVAS_GL_GLES_2_X);
     }

   // Check FBO for completeness
   fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

   // Delete Created Resources
   _texture_destroy(&color_buf);
   _renderbuffer_destroy(&depth_buf);
   _renderbuffer_destroy(&stencil_buf);
#ifdef GL_GLES
   _texture_destroy(&depth_stencil_buf);
#else
   _renderbuffer_destroy(&depth_stencil_buf);
#endif

   // Delete FBO
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
   if (fbo) glDeleteFramebuffers(1, &fbo);

   // Return the result
   if (fb_status != GL_FRAMEBUFFER_COMPLETE)
     {
        int err = glGetError();

        if (err != GL_NO_ERROR)
           DBG("glGetError() returns %x ", err);

        return 0;
     }
   else
      return 1;
}

static int
_surface_cap_test(EVGL_Surface_Format *fmt, GL_Format *color,
                  GL_Format *depth, GL_Format *stencil, int samples)
{
   int ret = 0;

   if ( (depth->bit == DEPTH_STENCIL) && (stencil->bit != STENCIL_BIT_8))
      return 0;

   ret = _fbo_surface_cap_test((GLint)color->fmt,
                               color->fmt,
                               depth->fmt,
                               stencil->fmt, samples);
   if (ret)
     {
        fmt->color_bit  = color->bit;
        fmt->color_ifmt = (GLint)color->fmt;
        fmt->color_fmt  = color->fmt;
        fmt->samples = samples;

        if (depth->bit == DEPTH_STENCIL)
          {
             // Depth Stencil Case
             fmt->depth_stencil_fmt = depth->fmt;
             fmt->depth_bit   = DEPTH_BIT_24;
             fmt->depth_fmt   = 0;
             fmt->stencil_bit = STENCIL_BIT_8;
             fmt->stencil_fmt = 0;
          }
        else
          {
             fmt->depth_stencil_fmt = 0;
             fmt->depth_bit = depth->bit;
             fmt->depth_fmt = depth->fmt;
             fmt->stencil_bit = stencil->bit;
             fmt->stencil_fmt = stencil->fmt;
          }
     }

   return ret;
}


static int
_surface_cap_check()
{
   int num_fmts = 0;
   int i, j, k, m;

   GL_Format color[]   = {
                           { COLOR_RGB_888,   GL_RGB },
                           { COLOR_RGBA_8888, GL_RGBA },
                           { -1, -1 },
                         };

#ifdef GL_GLES
   GL_Format depth[]   = {
                           { DEPTH_NONE,   0 },
                           { DEPTH_STENCIL, GL_DEPTH_STENCIL_OES },
                           { DEPTH_STENCIL, GL_DEPTH24_STENCIL8_OES },
                           { DEPTH_BIT_8,   GL_DEPTH_COMPONENT },
                           { DEPTH_BIT_16,  GL_DEPTH_COMPONENT16 },
                           { DEPTH_BIT_24,  GL_DEPTH_COMPONENT24_OES },
                           { DEPTH_BIT_32,  GL_DEPTH_COMPONENT32_OES },
                           { -1, -1 },
                         };
   GL_Format stencil[] = {
                           { STENCIL_NONE, 0 },
                           { STENCIL_BIT_1, GL_STENCIL_INDEX1_OES },
                           { STENCIL_BIT_4, GL_STENCIL_INDEX4_OES },
                           { STENCIL_BIT_8, GL_STENCIL_INDEX8 },
                           { -1, -1 },
                         };
#else
   GL_Format depth[]   = {
                           { DEPTH_NONE,   0 },
                           { DEPTH_STENCIL, GL_DEPTH24_STENCIL8 },
                           { DEPTH_BIT_8,   GL_DEPTH_COMPONENT },
                           { DEPTH_BIT_16,  GL_DEPTH_COMPONENT16 },
                           { DEPTH_BIT_24,  GL_DEPTH_COMPONENT24 },
                           { DEPTH_BIT_32,  GL_DEPTH_COMPONENT32 },
                           { -1, -1 },
                         };
   GL_Format stencil[] = {
                           { STENCIL_NONE, 0 },
                           { STENCIL_BIT_1, GL_STENCIL_INDEX1 },
                           { STENCIL_BIT_4, GL_STENCIL_INDEX4 },
                           { STENCIL_BIT_8, GL_STENCIL_INDEX8 },
                           { -1, -1 },
                         };
#endif

   int msaa_samples[4] = {0, -1, -1, -1};  // { NO_MSAA, LOW, MED, HIGH }

   EVGL_Surface_Format *fmt = NULL;

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("EVGL Engine not initialized!");
        return 0;
     }

   // Check Surface Cap for MSAA
   if (evgl_engine->caps.msaa_supported)
     {
        if ((evgl_engine->caps.msaa_samples[2] != evgl_engine->caps.msaa_samples[1]) &&
            (evgl_engine->caps.msaa_samples[2] != evgl_engine->caps.msaa_samples[0]))
             msaa_samples[3] = evgl_engine->caps.msaa_samples[2];    // HIGH
        if ((evgl_engine->caps.msaa_samples[1] != evgl_engine->caps.msaa_samples[0]))
             msaa_samples[2] = evgl_engine->caps.msaa_samples[1];    // MED
        if (evgl_engine->caps.msaa_samples[0])
             msaa_samples[1] = evgl_engine->caps.msaa_samples[0];    // LOW
     }


   // MSAA
   for ( m = 0; m < 4; m++)
     {
        if (msaa_samples[m] < 0) continue;

        // Color Formats
        i = 0;
        while ( color[i].bit >= 0 )
          {
             j = 0;
             // Depth Formats
             while ( depth[j].bit >= 0 )
               {
                  k = 0;
                  // Stencil Formats
                  while ( stencil[k].bit >= 0)
                    {
                       fmt = &evgl_engine->caps.fbo_fmts[num_fmts];
                       if (_surface_cap_test(fmt, &color[i], &depth[j], &stencil[k], msaa_samples[m]))
                          num_fmts++;
                       k++;
                    }
                  j++;
               }
             i++;
          }
     }

   return num_fmts;
}

static int
_surface_cap_load(Eet_File *ef)
{
   int res = 0, i = 0, length = 0;
   char tag[80];
   char *data = NULL;

   data = eet_read(ef, "num_fbo_fmts", &length);
   if ((!data) || (length <= 0)) goto finish;
   if (data[length - 1] != 0) goto finish;
   evgl_engine->caps.num_fbo_fmts = atoi(data);
   free(data);
   data = NULL;

   // !!!FIXME
   // Should use eet functionality instead of just reading using sscanfs...
   for (i = 0; i < evgl_engine->caps.num_fbo_fmts; ++i)
     {
        EVGL_Surface_Format *fmt = &evgl_engine->caps.fbo_fmts[i];

        snprintf(tag, sizeof(tag), "fbo_%d", i);
        data = eet_read(ef, tag, &length);
        if ((!data) || (length <= 0)) goto finish;
        if (data[length - 1] != 0) goto finish;
        sscanf(data, "%d%d%d%d%d%d%d%d%d%d",
               &(fmt->index),
               (int*)(&(fmt->color_bit)), &(fmt->color_ifmt), &(fmt->color_fmt),
               (int*)(&(fmt->depth_bit)), &(fmt->depth_fmt),
               (int*)(&(fmt->stencil_bit)), &(fmt->stencil_fmt),
               &(fmt->depth_stencil_fmt),
               &(fmt->samples));
        free(data);
        data = NULL;
     }
   res = 1;

finish:
   if (data) free(data);
   return res;
}

static int
_surface_cap_save(Eet_File *ef)
{
   int i = 0;
   char tag[80], data[80];

   snprintf(data, sizeof(data), "%d", evgl_engine->caps.num_fbo_fmts);
   if (eet_write(ef, "num_fbo_fmts", data, strlen(data) + 1, 1) < 0)
      return 0;

   // !!!FIXME
   // Should use eet functionality instead of just writing out using snprintfs...
   for (i = 0; i < evgl_engine->caps.num_fbo_fmts; ++i)
     {
        EVGL_Surface_Format *fmt = &evgl_engine->caps.fbo_fmts[i];

        snprintf(tag, sizeof(tag), "fbo_%d", i);
        snprintf(data, sizeof(data), "%d %d %d %d %d %d %d %d %d %d",
                 fmt->index,
                 fmt->color_bit, fmt->color_ifmt, fmt->color_fmt,
                 fmt->depth_bit, fmt->depth_fmt,
                 fmt->stencil_bit, fmt->stencil_fmt,
                 fmt->depth_stencil_fmt,
                 fmt->samples);
        if (eet_write(ef, tag, data, strlen(data) + 1, 1) < 0) return 0;
     }

   return 1;
}

static int
_surface_cap_cache_load()
{
   /* check eet */
   Eet_File *et = NULL;
   char cap_dir_path[PATH_MAX];
   char cap_file_path[PATH_MAX];

   if (!evas_gl_common_file_cache_dir_check(cap_dir_path, sizeof(cap_dir_path)))
      return 0;

   if (!evas_gl_common_file_cache_file_check(cap_dir_path, "surface_cap",
                                             cap_file_path, sizeof(cap_dir_path)))
      return 0;

   /* use eet */
   if (!eet_init()) return 0;
   et = eet_open(cap_file_path, EET_FILE_MODE_READ);
   if (!et) goto error;

   if (!_surface_cap_load(et))
      goto error;

   if (et) eet_close(et);
   eet_shutdown();
   return 1;

error:
   if (et) eet_close(et);
   eet_shutdown();
   return 0;
}

static int
_surface_cap_cache_save()
{
   /* check eet */
   Eet_File *et = NULL; //check eet file
   int tmpfd = -1;
   char cap_dir_path[PATH_MAX];
   char cap_file_path[PATH_MAX];
   char tmp_file_name[PATH_MAX + PATH_MAX + 128];
   Eina_Tmpstr *tmp_file_path = NULL;

   /* use eet */
   if (!eet_init()) return 0;

   if (!evas_gl_common_file_cache_dir_check(cap_dir_path, sizeof(cap_dir_path)))
     {
        if (!evas_gl_common_file_cache_mkpath(cap_dir_path))
          return 0; /* we can't make directory */
     }

   evas_gl_common_file_cache_file_check(cap_dir_path, "surface_cap", cap_file_path,
                                        sizeof(cap_dir_path));

   /* use mkstemp for writing */
   snprintf(tmp_file_name, sizeof(tmp_file_name), "%s.XXXXXX.cache", cap_file_path);
   tmpfd = eina_file_mkstemp(tmp_file_name, &tmp_file_path);
   if (tmpfd < 0) goto error;

   et = eet_open(tmp_file_path, EET_FILE_MODE_WRITE);
   if (!et) goto error;

   if (!_surface_cap_save(et)) goto error;

   if (eet_close(et) != EET_ERROR_NONE) goto destroyed;
   if (rename(tmp_file_path, cap_file_path) < 0) goto destroyed;
   eina_tmpstr_del(tmp_file_path);
   close(tmpfd);
   eet_shutdown();

   return 1;

destroyed:
   et = NULL;

error:
   if (tmpfd >= 0) close(tmpfd);
   if (et) eet_close(et);
   if (evas_gl_common_file_cache_file_exists(tmp_file_path))
     unlink(tmp_file_path);
   eina_tmpstr_del(tmp_file_path);
   eet_shutdown();
   return 0;
}

static int
_surface_cap_init(void *eng_data)
{
   int gles_version;
   int ret = 0;
   int max_size = 0;
   int max_samples = 0;

   // Do internal make current
   if (!_internal_resource_make_current(eng_data, NULL, NULL))
     {
        ERR("Error doing an internal resource make current");
        return ret;
     }

   // Query the max width and height of the surface
   glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &max_size);

   evgl_engine->caps.max_w = max_size;
   evgl_engine->caps.max_h = max_size;
   DBG("Max Surface Width: %d   Height: %d", evgl_engine->caps.max_w, evgl_engine->caps.max_h);

   gles_version = evas_gl_common_version_check(NULL);

   // Check for MSAA support
   if (gles_version == 3)
     {
        glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
        INF("MSAA support for GLES 3 is not implemented yet!");
        max_samples = 0;
     }
#ifdef GL_GLES
   else if (EXTENSION_SUPPORT(IMG_multisampled_render_to_texture))
     {
        glGetIntegerv(GL_MAX_SAMPLES_IMG, &max_samples);
     }
   else if (EXTENSION_SUPPORT(EXT_multisampled_render_to_texture))
     {
        glGetIntegerv(GL_MAX_SAMPLES_EXT, &max_samples);
     }
   else
     {
        const char *exts = (const char *) glGetString(GL_EXTENSIONS);

        if (exts && strstr(exts, "EXT_multisampled_render_to_texture"))
          glGetIntegerv(GL_MAX_SAMPLES_EXT, &max_samples);
        else if (exts && strstr(exts, "IMG_multisampled_render_to_texture"))
          glGetIntegerv(GL_MAX_SAMPLES_IMG, &max_samples);
     }
#endif

   if (max_samples >= 2)
     {
        evgl_engine->caps.msaa_samples[0] = 2;
        evgl_engine->caps.msaa_samples[1] = (max_samples >> 1) < 2 ? 2 : (max_samples >> 1);
        evgl_engine->caps.msaa_samples[2] = max_samples;
        evgl_engine->caps.msaa_supported  = 1;
     }

   // Load Surface Cap
   if (!_surface_cap_cache_load())
     {
        // Check Surface Cap
        evgl_engine->caps.num_fbo_fmts = _surface_cap_check();
        _surface_cap_cache_save();
        DBG("Ran Evas GL Surface Cap and Cached the existing values.");
     }
   else
     {
        DBG("Loaded cached Evas GL Surface Cap values.");
     }

   if (evgl_engine->caps.num_fbo_fmts)
     {
        _surface_cap_print(0);
        DBG("Number of supported surface formats: %d", evgl_engine->caps.num_fbo_fmts);
        ret = 1;
     }
   else
     {
        ERR("There are no available surface formats. Error!");
     }

   // Destroy internal resources
   _evgl_tls_resource_destroy(eng_data);

   return ret;
}

static int
_context_ext_check(EVGL_Context *ctx)
{
   if (!ctx)
      return 0;

   if (ctx->extension_checked)
      return 1;

#ifdef GL_GLES
   int fbo_supported = 0;
   int egl_image_supported = 0;
   int texture_image_supported = 0;

   switch (ctx->version)
     {
      case EVAS_GL_GLES_1_X:
         if (EXTENSION_SUPPORT_GLES1(framebuffer_object))
           fbo_supported = 1;
         break;
      case EVAS_GL_GLES_2_X:
      case EVAS_GL_GLES_3_X:
      default:
         fbo_supported = 1;
     }

   if (EXTENSION_SUPPORT_EGL(EGL_KHR_image_base))
     egl_image_supported = 1;
   if (EXTENSION_SUPPORT_EGL(EGL_KHR_gl_texture_2D_image))
     texture_image_supported = 1;

   if (egl_image_supported)
     {
        if (fbo_supported && texture_image_supported)
          ctx->fbo_image_supported = 1;
        else
          ctx->pixmap_image_supported = 1;
     }
#endif

   if (ctx->version == EVAS_GL_GLES_3_X)
     {
        /* HACK, as of 2017/03/08:
         * Some NVIDIA drivers pretend to support GLES 3.1 with EGL but in
         * fact none of the new functions are available, neither through
         * dlsym() nor eglGetProcAddress(). GLX/OpenGL should work though.
         * This is a fixup for glGetString(GL_VERSION).
         */
        if (!gles3_funcs->glVertexBindingDivisor)
          ctx->version_minor = 0;
        else
          ctx->version_minor = 1;
     }

   ctx->extension_checked = 1;

   return 1;
}


static const char *
_glenum_string_get(GLenum e)
{
   switch (e)
     {
      case 0:
         return "0";
      case GL_RGB:
         return "GL_RGB";
      case GL_RGBA:
         return "GL_RGBA";

#ifdef GL_GLES
         // Depth
      case GL_DEPTH_COMPONENT:
         return "GL_DEPTH_COMPONENT";
      case GL_DEPTH_COMPONENT16:
         return "GL_DEPTH_COMPONENT16";
      case GL_DEPTH_COMPONENT24_OES:
         return "GL_DEPTH_COMPONENT24_OES";
      case GL_DEPTH_COMPONENT32_OES:
       return "GL_DEPTH_COMPONENT32_OES";

         // Stencil
      case GL_STENCIL_INDEX1_OES:
         return "GL_STENCIL_INDEX1_OES";
      case GL_STENCIL_INDEX4_OES:
         return "GL_STENCIL_INDEX4_OES";
      case GL_STENCIL_INDEX8:
         return "GL_STENCIL_INDEX8";

         // Depth_Stencil
      case GL_DEPTH_STENCIL_OES:
         return "GL_DEPTH_STENCIL_OES";
      case GL_DEPTH24_STENCIL8_OES:
         return "GL_DEPTH24_STENCIL8_OES";
#else
         // Depth
      case GL_DEPTH_COMPONENT:
         return "GL_DEPTH_COMPONENT";
      case GL_DEPTH_COMPONENT16:
         return "GL_DEPTH_COMPONENT16";
      case GL_DEPTH_COMPONENT24:
         return "GL_DEPTH_COMPONENT24";
      case GL_DEPTH_COMPONENT32:
         return "GL_DEPTH_COMPONENT32";

         // Stencil
      case GL_STENCIL_INDEX:
         return "GL_STENCIL_INDEX";
      case GL_STENCIL_INDEX1:
         return "GL_STENCIL_INDEX1";
      case GL_STENCIL_INDEX4:
         return "GL_STENCIL_INDEX4";
      case GL_STENCIL_INDEX8:
         return "GL_STENCIL_INDEX8";

         // Depth_Stencil
      case GL_DEPTH_STENCIL:
        return "GL_DEPTH_STENCIL";
      case GL_DEPTH24_STENCIL8:
         return "GL_DEPTH24_STENCIL8";
#endif
      default:
         return "ERR";
     }
}

static void
_surface_cap_print(int error)
{
   int i = 0;
#define PRINT_LOG(...) \
   if (error) \
      ERR(__VA_ARGS__); \
   else \
      DBG(__VA_ARGS__);

   PRINT_LOG("----------------------------------------------------------------------------------------------------------------");
   PRINT_LOG("                 Evas GL Supported Surface Format                                                               ");
   PRINT_LOG("----------------------------------------------------------------------------------------------------------------");
   PRINT_LOG(" Max Surface Width: %d Height: %d", evgl_engine->caps.max_w, evgl_engine->caps.max_h);
   PRINT_LOG(" Multisample Support: %d", evgl_engine->caps.msaa_supported);
   //if (evgl_engine->caps.msaa_supported)
     {
        PRINT_LOG("             Low  Samples: %d", evgl_engine->caps.msaa_samples[0]);
        PRINT_LOG("             Med  Samples: %d", evgl_engine->caps.msaa_samples[1]);
        PRINT_LOG("             High Samples: %d", evgl_engine->caps.msaa_samples[2]);
     }
   PRINT_LOG("[Index] [Color Format]  [------Depth Bits------]      [----Stencil Bits---]     [---Depth_Stencil---]  [Samples]");

#define PRINT_SURFACE_CAP(IDX, COLOR, DEPTH, STENCIL, DS, SAMPLE) \
     { \
        PRINT_LOG("  %3d  %10s    %25s  %25s  %25s  %5d", IDX, _glenum_string_get(COLOR), _glenum_string_get(DEPTH), _glenum_string_get(STENCIL), _glenum_string_get(DS), SAMPLE ); \
     }

   for (i = 0; i < evgl_engine->caps.num_fbo_fmts; ++i)
     {
        EVGL_Surface_Format *fmt = &evgl_engine->caps.fbo_fmts[i];
        PRINT_SURFACE_CAP(i, fmt->color_fmt, fmt->depth_fmt, fmt->stencil_fmt, fmt->depth_stencil_fmt, fmt->samples);
     }

#undef PRINT_SURFACE_CAP
#undef PRINT_LOG
}

static void
_surface_context_list_print()
{
   Eina_List *l;
   EVGL_Surface *s;
   EVGL_Context *c;
   int count = 0;

   // Only print them when the log level is 6
   if (_evas_gl_log_level < 6) return;

#define RESET "\e[m"
#define GREEN "\e[1;32m"
#define YELLOW "\e[1;33m"
#define RED "\e[1;31m"

   LKL(evgl_engine->resource_lock);

   DBG( YELLOW "-----------------------------------------------" RESET);
   DBG("Total Number of active Evas GL Surfaces: %d", eina_list_count(evgl_engine->surfaces));

   EINA_LIST_FOREACH(evgl_engine->surfaces, l, s)
     {
        DBG( YELLOW "\t-----------------------------------------------" RESET);
        DBG( RED "\t[Surface %d]" YELLOW " Ptr: %p" RED " Appx Mem: %d Byte", count++, s, (s->buffer_mem[0]+s->buffer_mem[1]+s->buffer_mem[2]+s->buffer_mem[3]));
        DBG( GREEN "\t\t Size:" RESET " (%d, %d)",  s->w, s->h);

        if (s->buffer_mem[0])
          {
             DBG( GREEN "\t\t Color Format:" RESET " %s", _glenum_string_get(s->color_fmt));
             DBG( GREEN "\t\t Color Buffer Appx. Mem Usage:" RESET " %d Byte", s->buffer_mem[0]);
          }
        if (s->buffer_mem[1])
          {
             DBG( GREEN "\t\t Depth Format:" RESET " %s", _glenum_string_get(s->depth_fmt));
             DBG( GREEN "\t\t Depth Buffer Appx. Mem Usage: " RESET "%d Byte", s->buffer_mem[1]);
          }
        if (s->buffer_mem[2])
          {
             DBG( GREEN "\t\t Stencil Format:" RESET " %s", _glenum_string_get(s->stencil_fmt));
             DBG( GREEN "\t\t Stencil Buffer Appx. Mem Usage:" RESET " %d Byte", s->buffer_mem[2]);
          }
        if (s->buffer_mem[3])
          {
             DBG( GREEN "\t\t D-Stencil Format:" RESET " %s", _glenum_string_get(s->depth_stencil_fmt));
             DBG( GREEN "\t\t D-Stencil Buffer Appx. Mem Usage:" RESET " %d Byte", s->buffer_mem[3]);
          }
        if (s->msaa_samples)
           DBG( GREEN "\t\t MSAA Samples:" RESET " %d", s->msaa_samples);
        if (s->direct_fb_opt)
           DBG( GREEN "\t\t Direct Option Enabled" RESET );
        DBG( YELLOW "\t-----------------------------------------------" RESET);
     }

   count = 0;

   DBG( YELLOW "-----------------------------------------------" RESET);
   DBG("Total Number of active Evas GL Contexts: %d", eina_list_count(evgl_engine->contexts));
   EINA_LIST_FOREACH(evgl_engine->contexts, l, c)
     {
        DBG( YELLOW "\t-----------------------------------------------" RESET);
        DBG( RED "\t[Context %d]" YELLOW " Ptr: %p", count++, c);
     }
   DBG( YELLOW "-----------------------------------------------" RESET);

   LKU(evgl_engine->resource_lock);

#undef RESET
#undef GREEN
#undef YELLOW
#undef RED
}

//--------------------------------------------------------//
// Start from here.....
//--------------------------------------------------------//
static int
_surface_buffers_fbo_set(EVGL_Surface *sfc, GLuint fbo, Evas_GL_Context_Version version)
{
   int status;

   _framebuffer_bind(fbo, version);

   // Detach any previously attached buffers
   _texture_attach_2d(0, GL_COLOR_ATTACHMENT0, 0, 0, version);
   _renderbuffer_attach(0, GL_DEPTH_ATTACHMENT, version);
   _renderbuffer_attach(0, GL_STENCIL_ATTACHMENT, version);
#ifdef GL_GLES
   _texture_attach_2d(0, GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT, 0, version);
#else
    _renderbuffer_attach(0, GL_DEPTH_STENCIL_ATTACHMENT, version);
#endif

   // Render Target Texture
   if (sfc->color_buf)
     _texture_attach_2d(sfc->color_buf, GL_COLOR_ATTACHMENT0, 0, sfc->msaa_samples, version);

   // Depth Stencil RenderBuffer - Attach it to FBO
   if (sfc->depth_stencil_buf)
     {
#ifdef GL_GLES
        if (sfc->depth_stencil_fmt == GL_DEPTH_STENCIL_OES)
          _texture_attach_2d(sfc->depth_stencil_buf, GL_DEPTH_ATTACHMENT,
                             GL_STENCIL_ATTACHMENT, sfc->msaa_samples, version);
        else
          {
             _renderbuffer_attach(sfc->depth_stencil_buf, GL_DEPTH_ATTACHMENT, version);
             _renderbuffer_attach(sfc->depth_stencil_buf, GL_STENCIL_ATTACHMENT, version);
          }
#else
       _renderbuffer_attach(sfc->depth_stencil_buf, GL_DEPTH_STENCIL_ATTACHMENT, version);
#endif
     }

   // Depth RenderBuffer - Attach it to FBO
   if (sfc->depth_buf)
     _renderbuffer_attach(sfc->depth_buf, GL_DEPTH_ATTACHMENT, version);

   // Stencil RenderBuffer - Attach it to FBO
   if (sfc->stencil_buf)
     _renderbuffer_attach(sfc->stencil_buf, GL_STENCIL_ATTACHMENT, version);

   // Check FBO for completeness
   status = _framebuffer_check(version);
   if (status != GL_FRAMEBUFFER_COMPLETE)
     {
        ERR("FBO not complete. Error Code: %x!", status);
        return 0;
     }

   return 1;
}

static int
_surface_buffers_create(EVGL_Surface *sfc)
{
   // Create buffers
   if (sfc->color_fmt)
     {
        _texture_create(&sfc->color_buf);
     }


   // Depth_stencil buffers or separate buffers
   if (sfc->depth_stencil_fmt)
     {
#ifdef GL_GLES
        if (sfc->depth_stencil_fmt == GL_DEPTH_STENCIL_OES)
          _texture_create(&sfc->depth_stencil_buf);
        else
#endif
          _renderbuffer_create(&sfc->depth_stencil_buf);
     }
   else
     {
        if (sfc->depth_fmt)
          {
             _renderbuffer_create(&sfc->depth_buf);
          }
        if (sfc->stencil_fmt)
          {
             _renderbuffer_create(&sfc->stencil_buf);
          }
     }

   return 1; //ret;
}


static int
_surface_buffers_allocate(void *eng_data EINA_UNUSED, EVGL_Surface *sfc, int w, int h, Evas_GL_Context_Version version EINA_UNUSED/* unused ecept gles */)
{
   // Create buffers
   if (sfc->color_fmt)
     {
        _texture_allocate_2d(sfc->color_buf, sfc->color_ifmt, sfc->color_fmt,
                             GL_UNSIGNED_BYTE, w, h);
        if (sfc->egl_image)
          {
             _egl_image_destroy(sfc->egl_image);
             sfc->egl_image = NULL;
          }
        if ((sfc->current_ctx) && (sfc->current_ctx->fbo_image_supported) && (w) && (h))
          sfc->egl_image = _egl_image_create(sfc->current_ctx, EGL_GL_TEXTURE_2D_KHR, (void *)(uintptr_t)sfc->color_buf);

        sfc->buffer_mem[0] = w * h * 4;
     }

   // Depth_stencil buffers or separate buffers
   if (sfc->depth_stencil_fmt)
     {
#ifdef GL_GLES
        if (sfc->depth_stencil_fmt == GL_DEPTH_STENCIL_OES)
          {
             if (version == EVAS_GL_GLES_3_X)
               {
                  _texture_allocate_2d(sfc->depth_stencil_buf, GL_DEPTH24_STENCIL8_OES,
                     sfc->depth_stencil_fmt, GL_UNSIGNED_INT_24_8_OES,
                     w, h);
               }
             else
               {
                  _texture_allocate_2d(sfc->depth_stencil_buf, sfc->depth_stencil_fmt,
                     sfc->depth_stencil_fmt, GL_UNSIGNED_INT_24_8_OES,
                     w, h);
               }
          }
        else
#endif
          {
             _renderbuffer_allocate(sfc->depth_stencil_buf,
                                    sfc->depth_stencil_fmt, w, h,
                                    sfc->msaa_samples);
          }
        sfc->buffer_mem[3] = w * h * 4;
     }
   else
     {
        if (sfc->depth_fmt)
          {
             _renderbuffer_allocate(sfc->depth_buf, sfc->depth_fmt, w, h,
                                    sfc->msaa_samples);
             sfc->buffer_mem[1] = w * h * 3; // Assume it's 24 bits
          }
        if (sfc->stencil_fmt)
          {
             _renderbuffer_allocate(sfc->stencil_buf, sfc->stencil_fmt, w,
                                    h, sfc->msaa_samples);
             sfc->buffer_mem[2] = w * h; // Assume it's 8 bits
          }
     }

   return 1; //ret;
}

static int
_surface_buffers_destroy(EVGL_Surface *sfc)
{
   if (sfc->egl_image)
     {
        _egl_image_destroy(sfc->egl_image);
        sfc->egl_image = NULL;
     }
   if (sfc->color_buf)
      _texture_destroy(&sfc->color_buf);
   if (sfc->depth_buf)
      _renderbuffer_destroy(&sfc->depth_buf);
   if (sfc->stencil_buf)
      _renderbuffer_destroy(&sfc->stencil_buf);
   if (sfc->depth_stencil_buf)
     {
#ifdef GL_GLES
        if (sfc->depth_stencil_fmt == GL_DEPTH_STENCIL_OES)
          _texture_destroy(&sfc->depth_stencil_buf);
        else
#endif
          _renderbuffer_destroy(&sfc->depth_stencil_buf);
     }

   return 1;
}

static int
_internal_config_set(void *eng_data, EVGL_Surface *sfc, Evas_GL_Config *cfg)
{
   int i = 0, cfg_index = -1;
   int color_bit = 0, depth_bit = 0, stencil_bit = 0, msaa_samples = 0;
   int depth_size = 0;
   int native_win_depth = 0, native_win_stencil = 0, native_win_msaa = 0;

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        return 0;
     }

   // Convert Config Format to bitmask friendly format
   color_bit = (1 << cfg->color_format);
   if (cfg->depth_bits)
     {
        depth_bit = (1 << (cfg->depth_bits-1));
        depth_size = 8 * cfg->depth_bits;
     }
   if (cfg->stencil_bits) stencil_bit = (1 << (cfg->stencil_bits-1));
   if (cfg->multisample_bits)
      msaa_samples = evgl_engine->caps.msaa_samples[cfg->multisample_bits-1];

try_again:
   // Run through all the available formats and choose the first match
   for (i = 0; i < evgl_engine->caps.num_fbo_fmts; ++i)
     {
        // Check if the MSAA is supported.  Fallback if not.
        if ((msaa_samples) && (evgl_engine->caps.msaa_supported))
          {
             if (msaa_samples > evgl_engine->caps.fbo_fmts[i].samples)
                  continue;
          }

        if (color_bit & evgl_engine->caps.fbo_fmts[i].color_bit)
          {
             if (depth_bit)
               {
                  if (!(depth_bit & evgl_engine->caps.fbo_fmts[i].depth_bit))
                     continue;
               }

             if (stencil_bit)
               {
                  if (!(stencil_bit & evgl_engine->caps.fbo_fmts[i].stencil_bit))
                     continue;
               }

             // Set the surface format
             sfc->color_ifmt        = evgl_engine->caps.fbo_fmts[i].color_ifmt;
             sfc->color_fmt         = evgl_engine->caps.fbo_fmts[i].color_fmt;
             sfc->depth_fmt         = evgl_engine->caps.fbo_fmts[i].depth_fmt;
             sfc->stencil_fmt       = evgl_engine->caps.fbo_fmts[i].stencil_fmt;
             sfc->depth_stencil_fmt = evgl_engine->caps.fbo_fmts[i].depth_stencil_fmt;
             sfc->msaa_samples      = evgl_engine->caps.fbo_fmts[i].samples;

             // Direct Rendering Option
             if (cfg->options_bits & EVAS_GL_OPTIONS_DIRECT)
               {
                  Eina_Bool support_win_cfg = EINA_FALSE;

                  if (evgl_engine->funcs->native_win_surface_config_get)
                    evgl_engine->funcs->native_win_surface_config_get(eng_data, &native_win_depth, &native_win_stencil, &native_win_msaa);
                  if ((native_win_depth >= depth_size)
                      && (native_win_stencil >= stencil_bit)
                      && (native_win_msaa >= msaa_samples))
                    {
                       DBG("Win cfg can support the Req Evas GL's config successfully");
                       support_win_cfg = EINA_TRUE;
                    }
                  else
                    {
                       ERR("Win config can't support Evas GL direct rendering, "
                           "win: [depth %d, stencil %d, msaa %d] "
                           "want: [depth %d, stencil %d, msaa %d]. %s",
                           native_win_depth, native_win_stencil, native_win_msaa,
                           depth_size, stencil_bit, msaa_samples,
                           sfc->direct_override ?
                              "Forcing direct rendering anyway." :
                              "Falling back to indirect rendering (FBO).");
                       support_win_cfg = EINA_FALSE;
                    }

                  if (sfc->direct_override || support_win_cfg)
                    {
                       sfc->direct_fb_opt = EINA_TRUE;

                       // Extra flags for direct rendering
                       sfc->client_side_rotation = !!(cfg->options_bits & EVAS_GL_OPTIONS_CLIENT_SIDE_ROTATION);
                       sfc->alpha = (cfg->color_format == EVAS_GL_RGBA_8888);
                    }
                  else
                    {
                       const char *s1[] = { "", ":depth8", ":depth16", ":depth24", ":depth32" };
                       const char *s2[] = { "", ":stencil1", ":stencil2", ":stencil4", ":stencil8", ":stencil16" };
                       const char *s3[] = { "", ":msaa_low", ":msaa_mid", ":msaa_high" };
                       INF("Can not enable direct rendering with depth %d, stencil %d "
                           "and MSAA %d. When using Elementary GLView, try to call "
                           "elm_config_accel_preference_set(\"opengl%s%s%s\") before "
                           "creating any window.",
                           depth_size, stencil_bit, msaa_samples,
                           s1[cfg->depth_bits], s2[cfg->stencil_bits], s3[cfg->multisample_bits]);
                    }

                  // When direct rendering is enabled, FBO configuration should match
                  // window surface configuration as FBO will be used in fallback cases.
                  // So we search again for the formats that match window surface's.
                  if (sfc->direct_fb_opt &&
                      ((native_win_depth != depth_size) ||
                       (native_win_stencil != stencil_bit) ||
                       (native_win_msaa != msaa_samples)))
                    {
                       if (native_win_depth < 8) depth_bit = 0;
                       else depth_bit = (1 << ((native_win_depth / 8) - 1));
                       depth_size = native_win_depth;
                       stencil_bit = native_win_stencil;
                       msaa_samples = native_win_msaa;
                       goto try_again;
                    }
               }

             cfg_index = i;
             break;
          }
     }

   if (cfg_index < 0)
     {
        ERR("Unable to find a matching config format (depth:%d, stencil:%d, msaa:%d)",
            depth_size, stencil_bit, msaa_samples);
        if ((stencil_bit > 8) || (depth_size > 24))
          {
             INF("Please note that Evas GL might not support 32-bit depth or "
                 "16-bit stencil buffers, so depth24, stencil8 are the maximum "
                 "recommended values.");
             if (depth_size > 24)
               {
                  depth_bit = 4; // see DEPTH_BIT_24
                  depth_size = 24;
               }
             if (stencil_bit > 8) stencil_bit = 8; // see STENCIL_BIT_8
             DBG("Fallback to depth:%d, stencil:%d", depth_size, stencil_bit);
             goto try_again;
          }
        else if (msaa_samples > 0)
          {
             msaa_samples /= 2;
             if (msaa_samples == 1) msaa_samples = 0;
             DBG("Fallback to msaa:%d", msaa_samples);
             goto try_again;
          }
        return 0;
     }
   else
     {
        DBG("-------------Evas GL Surface Config---------------");
        DBG("Selected Config Index: %d", cfg_index);
        DBG("  Color Format     : %s", _glenum_string_get(sfc->color_fmt));
        DBG("  Depth Format     : %s", _glenum_string_get(sfc->depth_fmt));
        DBG("  Stencil Format   : %s", _glenum_string_get(sfc->stencil_fmt));
        DBG("  D-Stencil Format : %s", _glenum_string_get(sfc->depth_stencil_fmt));
        DBG("  MSAA Samples     : %d", sfc->msaa_samples);
        DBG("  Direct Option    : %d%s", sfc->direct_fb_opt, sfc->direct_override ? " (override)" : "");
        DBG("  Client-side Rot. : %d", sfc->client_side_rotation);
        DBG("--------------------------------------------------");
        sfc->cfg_index = cfg_index;
        return 1;
     }
}

static int
_evgl_direct_renderable(EVGL_Resource *rsc, EVGL_Surface *sfc)
{
   if (evgl_engine->direct_force_off) return 0;
   if (rsc->id != evgl_engine->main_tid) return 0;
   if (!sfc->direct_fb_opt) return 0;
   if (!rsc->direct.enabled) return 0;

   return 1;
}

//---------------------------------------------------------------//
// Functions used by Evas GL module
//---------------------------------------------------------------//
EVGL_Resource *
_evgl_tls_resource_get(void)
{
   EVGL_Resource *rsc = NULL;

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        return NULL;
     }

   if (evgl_engine->resource_key)
     rsc = eina_tls_get(evgl_engine->resource_key);

   return rsc;
}

static void
_evgl_tls_resource_destroy_cb(void *data)
{
   EVGL_Resource *rsc = data;

   LKL(evgl_engine->resource_lock);
   evgl_engine->resource_list = eina_list_remove(evgl_engine->resource_list, rsc);
   LKU(evgl_engine->resource_lock);

   _internal_resources_destroy(rsc->current_eng, rsc);
}

EVGL_Resource *
_evgl_tls_resource_create(void *eng_data)
{
   EVGL_Resource *rsc;

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        return NULL;
     }

   // Initialize Resource TLS
   if (!evgl_engine->resource_key && eina_tls_cb_new(&evgl_engine->resource_key, _evgl_tls_resource_destroy_cb) == EINA_FALSE)
     {
        ERR("Error creating tls key");
        return NULL;
     }
   DBG("TLS KEY created: %d", evgl_engine->resource_key);

   // Create internal resources if it hasn't been created already
   if (!(rsc = _internal_resources_create(eng_data)))
     {
        ERR("Error creating internal resources.");
        return NULL;
     }

   // Set the resource in TLS
   if (eina_tls_set(evgl_engine->resource_key, (void*)rsc) == EINA_TRUE)
     {
        // Add to the resource resource list for cleanup
        LKL(evgl_engine->resource_lock);
        evgl_engine->resource_list = eina_list_prepend(evgl_engine->resource_list, rsc);
        LKU(evgl_engine->resource_lock);
        return rsc;
     }
   else
     {
        ERR("Failed setting TLS Resource");
        _internal_resources_destroy(eng_data, rsc);
        return NULL;
     }
}

void
_evgl_tls_resource_destroy(void *eng_data)
{
   Eina_List *l;
   EVGL_Resource *rsc;
   EVGL_Surface *sfc;
   EVGL_Context *ctx;

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        return;
     }

   EINA_LIST_FOREACH(evgl_engine->surfaces, l, sfc)
     {
        evgl_surface_destroy(eng_data, sfc);
     }
   EINA_LIST_FOREACH(evgl_engine->contexts, l, ctx)
     {
        evgl_context_destroy(eng_data, ctx);
     }

   LKL(evgl_engine->resource_lock);

   eina_list_free(evgl_engine->surfaces);
   evgl_engine->surfaces = NULL;

   eina_list_free(evgl_engine->contexts);
   evgl_engine->contexts = NULL;

   EINA_LIST_FOREACH(evgl_engine->resource_list, l, rsc)
     {
        _internal_resources_destroy(eng_data, rsc);
     }
   eina_list_free(evgl_engine->resource_list);
   evgl_engine->resource_list = NULL;

   LKU(evgl_engine->resource_lock);

   // Destroy TLS
   if (evgl_engine->resource_key)
      eina_tls_free(evgl_engine->resource_key);
   evgl_engine->resource_key = 0;
}

EAPI void * /* EVGL_Context */
evas_gl_common_current_context_get(void)
{
   EVGL_Resource *rsc;

   if (!(rsc = _evgl_tls_resource_get()))
     {
        ERR("No current context set.");
        return NULL;
     }
   else
     return rsc->current_ctx;
}

EAPI void *
evgl_current_native_context_get(EVGL_Context *ctx)
{
   EVGLNative_Context context;

   if (!ctx)
     return NULL;

   context = ctx->context;

   if ((ctx->pixmap_image_supported) && evgl_direct_rendered())
     context = ctx->indirect_context;

   return context;
}

int
_evgl_not_in_pixel_get(void)
{
   EVGL_Resource *rsc;
   EVGL_Context *ctx;

   if (!(rsc=_evgl_tls_resource_get()))
     return 1;

   if (rsc->id != evgl_engine->main_tid)
     return 0;

   ctx = rsc->current_ctx;
   if (!ctx || !ctx->current_sfc)
     return 0;

   // if indirect rendering, we don't care. eg. elm_glview's init cb
   if (!ctx->current_sfc->direct_fb_opt)
     return 0;

   return !rsc->direct.in_get_pixels;
}

int
_evgl_direct_enabled(void)
{
   EVGL_Resource *rsc;
   EVGL_Surface  *sfc;

   if (!(rsc=_evgl_tls_resource_get())) return 0;
   if (!(rsc->current_ctx)) return 0;
   if (!(sfc=rsc->current_ctx->current_sfc)) return 0;

   return _evgl_direct_renderable(rsc, sfc);
}

EAPI void
evas_gl_common_error_set(int error_enum)
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get()))
     {
        WRN("evgl: Unable to set error!");
        return;
     }

   rsc->error_state = error_enum;
}

EAPI int
evas_gl_common_error_get(void)
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get()))
     {
        WRN("evgl: Unable to get error!");
        return EVAS_GL_NOT_INITIALIZED;
     }

   return rsc->error_state;
}

EVGLNative_Context
_evgl_native_context_get(Evas_GL_Context *ctx)
{
   EVGL_Context *evglctx;

   if (!glsym_evas_gl_native_context_get)
     {
        ERR("Engine can't get a pointer to the native context");
        return NULL;
     }

   evglctx = glsym_evas_gl_native_context_get(ctx);
   if (!evglctx) return NULL;
   return evgl_current_native_context_get(evglctx);
}

void *
_evgl_engine_data_get(Evas_GL *evasgl)
{
   if (!glsym_evas_gl_engine_data_get)
     {
        ERR("Engine can't get a pointer to the native display");
        return NULL;
     }

   return glsym_evas_gl_engine_data_get(evasgl);
}

//---------------------------------------------------------------//
// Exported functions for evas_engine to use

// Initialize engine
//    - Allocate the engine struct
//    - Assign engine funcs form evas_engine
//    - Create internal resources: internal context, surface for resource creation
//    - Initialize extensions
//    - Check surface capability
//
// This code should be called during eng_setup() in evas_engine
EVGL_Engine *
evgl_engine_init(void *eng_data, const EVGL_Interface *efunc)
{
   int direct_off = 0, direct_soff = 0, debug_mode = 0;
   char *s = NULL;

   if (evgl_engine) return evgl_engine;

   // Initialize Log Domain
   if (_evas_gl_log_dom < 0)
      _evas_gl_log_dom = eina_log_domain_register("EvasGL", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_gl_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return NULL;
     }

   // Store the Log Level
   _evas_gl_log_level = eina_log_domain_level_get("EvasGL");

   // Check the validity of the efunc
   if ((!efunc) ||
       (!efunc->surface_create) ||
       (!efunc->context_create) ||
       (!efunc->make_current))
     {
        ERR("Invalid Engine Functions for Evas GL Engine.");
        return NULL;
     }

   // Allocate engine
   evgl_engine = calloc(1, sizeof(EVGL_Engine));
   if (!evgl_engine)
     {
        ERR("Error allocating EVGL Engine. GL initialization failed.");
        goto error;
     }

   LKI(evgl_engine->resource_lock);

   // Assign functions
   evgl_engine->funcs = efunc;
   evgl_engine->safe_extensions = eina_hash_string_small_new(NULL);

   // Main Thread ID
   evgl_engine->main_tid = eina_thread_self();

   // Surface Caps
   if (!_surface_cap_init(eng_data))
     {
        ERR("Error initializing surface cap");
        goto error;
     }

   // NOTE: Should we finally remove these variables?
   // Check for Direct rendering override env var.
   if ((s = getenv("EVAS_GL_DIRECT_OVERRIDE")))
     {
        WRN("EVAS_GL_DIRECT_OVERRIDE is set to '%s' for the whole application. "
            "This should never be done except for debugging purposes.", s);
        if (atoi(s) == 1)
          evgl_engine->direct_override = 1;
     }

   // Check if Direct Rendering Memory Optimzation flag is on
   // Creates resources on demand when it fallsback to fbo rendering
   if ((s = getenv("EVAS_GL_DIRECT_MEM_OPT")))
     {
        WRN("EVAS_GL_DIRECT_MEM_OPT is set to '%s' for the whole application. "
            "This should never be done except for debugging purposes.", s);
        if (atoi(s) == 1)
          evgl_engine->direct_mem_opt = 1;
     }

   // Check if Direct Rendering Override Force Off flag is on
   s = getenv("EVAS_GL_DIRECT_OVERRIDE_FORCE_OFF");
   if (s) direct_off = atoi(s);
   if (direct_off == 1)
      evgl_engine->direct_force_off = 1;

   // Check if Direct Rendering Override Force Off flag is on
   s = getenv("EVAS_GL_DIRECT_SCISSOR_OFF");
   if (s) direct_soff = atoi(s);
   if (direct_soff == 1)
      evgl_engine->direct_scissor_off = 1;

   // Check if API Debug mode is on
   s = getenv("EVAS_GL_API_DEBUG");
   if (s) debug_mode = atoi(s);
   if (debug_mode == 1)
      evgl_engine->api_debug_mode = 1;

   return evgl_engine;

error:
   ERR("Failed to initialize EvasGL!");
   if (evgl_engine)
     {
        eina_hash_free(evgl_engine->safe_extensions);
        if (evgl_engine->resource_key)
          eina_tls_free(evgl_engine->resource_key);
        LKD(evgl_engine->resource_lock);
        free(evgl_engine);
     }
   evgl_engine = NULL;
   return NULL;
}

// Terminate engine and all the resources
//    - destroy all internal resources
//    - free allocated engine struct
void
evgl_engine_shutdown(void *eng_data)
{
   // Check if engine is valid
   if (!evgl_engine)
     {
        EINA_LOG_INFO("EvasGL Engine is not initialized.");
        return;
     }

   if (evgl_engine->safe_extensions)
     eina_hash_free(evgl_engine->safe_extensions);
   evgl_engine->safe_extensions = NULL;

   if (gles1_funcs) free(gles1_funcs);
   if (gles2_funcs) free(gles2_funcs);
   if (gles3_funcs) free(gles3_funcs);
   gles1_funcs = NULL;
   gles2_funcs = NULL;
   gles3_funcs = NULL;

   // Destroy internal resources
   _evgl_tls_resource_destroy(eng_data);

   LKD(evgl_engine->resource_lock);

   // Log
   eina_log_domain_unregister(_evas_gl_log_dom);
   _evas_gl_log_dom = -1;

   // Free engine
   free(evgl_engine);
   evgl_engine = NULL;
}

void *
evgl_surface_create(void *eng_data, Evas_GL_Config *cfg, int w, int h)
{
   EVGL_Surface *sfc = NULL;
   Eina_Bool dbg;

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        evas_gl_common_error_set(EVAS_GL_BAD_ACCESS);
        return NULL;
     }
   dbg = evgl_engine->api_debug_mode;

   if (!cfg)
     {
        ERR("Invalid Config!");
        evas_gl_common_error_set(EVAS_GL_BAD_CONFIG);
        return NULL;
     }

   // Check the size of the surface
   if ((w > evgl_engine->caps.max_w) || (h > evgl_engine->caps.max_h))
     {
        ERR("Requested surface size [%d, %d] is greater than max supported size [%d, %d]",
             w, h, evgl_engine->caps.max_w, evgl_engine->caps.max_h);
        evas_gl_common_error_set(EVAS_GL_BAD_PARAMETER);
        return NULL;
     }

   // Allocate surface structure
   sfc = calloc(1, sizeof(EVGL_Surface));
   if (!sfc)
     {
        ERR("Surface allocation failed.");
        evas_gl_common_error_set(EVAS_GL_BAD_ALLOC);
        goto error;
     }

   // Set surface info
   sfc->w = w;
   sfc->h = h;

   // Extra options for DR
   if (cfg->options_bits & EVAS_GL_OPTIONS_DIRECT_MEMORY_OPTIMIZE)
     {
        DBG("Setting DIRECT_MEMORY_OPTIMIZE bit");
        sfc->direct_mem_opt = EINA_TRUE;
     }
   else if (evgl_engine->direct_mem_opt == 1)
     sfc->direct_mem_opt = EINA_TRUE;

   if (cfg->options_bits & EVAS_GL_OPTIONS_DIRECT_OVERRIDE)
     {
        DBG("Setting DIRECT_OVERRIDE bit");
        sfc->direct_override = EINA_TRUE;
     }
   else if (evgl_engine->direct_override == 1)
     sfc->direct_override = EINA_TRUE;

   // Set the internal config value
   if (!_internal_config_set(eng_data, sfc, cfg))
     {
        ERR("Unsupported Format!");
        evas_gl_common_error_set(EVAS_GL_BAD_CONFIG);
        goto error;
     }
   sfc->cfg = cfg;

   // Keep track of all the created surfaces
   LKL(evgl_engine->resource_lock);
   evgl_engine->surfaces = eina_list_prepend(evgl_engine->surfaces, sfc);
   LKU(evgl_engine->resource_lock);

   if (dbg) DBG("Created surface sfc %p (eng %p)", sfc, eng_data);

   _surface_context_list_print();

   return sfc;

error:
   if (sfc) free(sfc);
   return NULL;
}

void *
evgl_pbuffer_surface_create(void *eng_data, Evas_GL_Config *cfg,
                            int w, int h, const int *attrib_list)
{
   EVGL_Surface *sfc = NULL;
   void *pbuffer;
   Eina_Bool dbg;

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        evas_gl_common_error_set(EVAS_GL_BAD_ACCESS);
        return NULL;
     }
   dbg = evgl_engine->api_debug_mode;

   if (!cfg)
     {
        ERR("Invalid Config!");
        evas_gl_common_error_set(EVAS_GL_BAD_CONFIG);
        return NULL;
     }

   if (!evgl_engine->funcs->pbuffer_surface_create)
     {
        ERR("Engine can not create PBuffers");
        evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        return NULL;
     }

   // Allocate surface structure
   sfc = calloc(1, sizeof(EVGL_Surface));
   if (!sfc)
     {
        ERR("Surface allocation failed.");
        evas_gl_common_error_set(EVAS_GL_BAD_ALLOC);
        goto error;
     }

   sfc->w = w;
   sfc->h = h;
   sfc->pbuffer.color_fmt = cfg->color_format;
   sfc->pbuffer.is_pbuffer = EINA_TRUE;

   // If the surface is defined as RGB or RGBA, then create an FBO
   if (sfc->pbuffer.color_fmt == EVAS_GL_NO_FBO)
      sfc->buffers_skip_allocate = 1;

   if (!sfc->buffers_skip_allocate)
     {
        // Set the internal config value
        if (!_internal_config_set(eng_data, sfc, cfg))
          {
             ERR("Unsupported Format!");
             evas_gl_common_error_set(EVAS_GL_BAD_CONFIG);
             goto error;
          }
     }
   sfc->cfg = cfg;

   pbuffer = evgl_engine->funcs->pbuffer_surface_create
     (eng_data, sfc, attrib_list);

   if (!pbuffer)
     {
        ERR("Engine failed to create a PBuffer");
        goto error;
     }

   sfc->pbuffer.native_surface = pbuffer;

   // Keep track of all the created surfaces
   LKL(evgl_engine->resource_lock);
   evgl_engine->surfaces = eina_list_prepend(evgl_engine->surfaces, sfc);
   LKU(evgl_engine->resource_lock);

   if (dbg) DBG("Created PBuffer surface sfc %p:%p (eng %p)", sfc, pbuffer, eng_data);

   return sfc;

error:
   free(sfc);
   return NULL;
}

int
evgl_surface_destroy(void *eng_data, EVGL_Surface *sfc)
{
   EVGL_Resource *rsc;
   Eina_List *l;
   EVGL_Context *ctx;
   Eina_Bool dbg;

   // Check input parameter
   if ((!evgl_engine) || (!sfc))
     {
        ERR("Invalid input data.  Engine: %p  Surface:%p", evgl_engine, sfc);
        return 0;
     }

   if ((dbg = evgl_engine->api_debug_mode))
     DBG("Destroying surface sfc %p (eng %p)", sfc, eng_data);

   // Retrieve the resource object
   rsc = _evgl_tls_resource_get();
   if (rsc && rsc->current_ctx)
     {
        // Make current to current context to destroy surface buffers
        if (!_internal_resource_make_current(eng_data, sfc, rsc->current_ctx))
          {
             ERR("Error doing an internal resource make current");
             return 0;
          }

        // Destroy created buffers
        if (!_surface_buffers_destroy(sfc))
          {
             ERR("Error deleting surface resources.");
             return 0;
          }

        if (rsc->current_ctx->current_sfc == sfc)
          {
             if (evgl_engine->api_debug_mode)
               {
                  ERR("The surface is still current before it's being destroyed.");
                  ERR("Doing make_current(NULL, NULL)");
               }
             else
               {
                  WRN("The surface is still current before it's being destroyed.");
                  WRN("Doing make_current(NULL, NULL)");
               }
             evgl_make_current(eng_data, NULL, NULL);
          }
     }

   // Destroy indirect surface
   if (sfc->indirect)
     {
        int ret;
        if (dbg) DBG("sfc %p is used for indirect rendering", sfc);

        if (!evgl_engine->funcs->indirect_surface_destroy)
          {
             ERR("Error destroying indirect surface");
             return 0;
          }

        DBG("Destroying special surface used for indirect rendering");
        ret = evgl_engine->funcs->indirect_surface_destroy(eng_data, sfc);

        if (!ret)
          {
             ERR("Engine failed to destroy indirect surface.");
             return ret;
          }
     }

   // Destroy PBuffer surface
   if (sfc->pbuffer.native_surface)
     {
        int ret;
        if (dbg) DBG("Surface sfc %p is a pbuffer: %p", sfc, sfc->pbuffer.native_surface);

        if (!evgl_engine->funcs->pbuffer_surface_destroy)
          {
             ERR("Error destroying PBuffer surface");
             return 0;
          }

        DBG("Destroying PBuffer surface");
        ret = evgl_engine->funcs->pbuffer_surface_destroy(eng_data, sfc->pbuffer.native_surface);

        if (!ret)
          {
             ERR("Engine failed to destroy the PBuffer.");
             return ret;
          }
     }

   if (dbg) DBG("Calling make_current(NULL, NULL)");
   if (!evgl_engine->funcs->make_current(eng_data, NULL, NULL, 0))
     {
        ERR("Error doing make_current(NULL, NULL).");
        return 0;
     }

   // Remove it from the list
   LKL(evgl_engine->resource_lock);
   evgl_engine->surfaces = eina_list_remove(evgl_engine->surfaces, sfc);
   LKU(evgl_engine->resource_lock);

   EINA_LIST_FOREACH(evgl_engine->contexts, l, ctx)
     {
        if (ctx->current_sfc == sfc)
          ctx->current_sfc = NULL;
     }

   free(sfc);

   _surface_context_list_print();

   return 1;

}

void *
evgl_context_create(void *eng_data, EVGL_Context *share_ctx,
                    Evas_GL_Context_Version version,
                    void *(*native_context_get)(void *),
                    void *(*engine_data_get)(void *))
{
   EVGL_Context *ctx   = NULL;

   // A little bit ugly. But it works even when dlsym(DEFAULT) doesn't work.
   glsym_evas_gl_native_context_get = native_context_get;
   glsym_evas_gl_engine_data_get = engine_data_get;

   // Check the input
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        evas_gl_common_error_set(EVAS_GL_BAD_ACCESS);
        return NULL;
     }

   if ((version < EVAS_GL_GLES_1_X) || (version > EVAS_GL_GLES_3_X))
     {
        ERR("Invalid context version number %d", version);
        evas_gl_common_error_set(EVAS_GL_BAD_PARAMETER);
        return NULL;
     }

   if (evgl_engine->api_debug_mode)
     DBG("Creating context GLESv%d (eng = %p, shctx = %p)", version, eng_data, share_ctx);

   // Allocate context object
   ctx = calloc(1, sizeof(EVGL_Context));
   if (!ctx)
     {
        ERR("Error allocating context object.");
        evas_gl_common_error_set(EVAS_GL_BAD_ALLOC);
        return NULL;
     }

   // Set default values
   ctx->version = version;
   ctx->version_minor = 0;
   ctx->scissor_coord[0] = 0;
   ctx->scissor_coord[1] = 0;
   ctx->scissor_coord[2] = evgl_engine->caps.max_w;
   ctx->scissor_coord[3] = evgl_engine->caps.max_h;
   ctx->gl_error = GL_NO_ERROR;

   // Call engine create context
   if (share_ctx)
      ctx->context = evgl_engine->funcs->context_create(eng_data, share_ctx->context, version);
   else
      ctx->context = evgl_engine->funcs->context_create(eng_data, NULL, version);

   // Call engine create context
   if (!ctx->context)
     {
        ERR("Error creating context from the Engine.");
        free(ctx);
        return NULL;
     }

   // Keep track of all the created context
   LKL(evgl_engine->resource_lock);
   evgl_engine->contexts = eina_list_prepend(evgl_engine->contexts, ctx);
   LKU(evgl_engine->resource_lock);

   if (evgl_engine->api_debug_mode)
     DBG("Created ctx %p", ctx);

   return ctx;
}

int
evgl_context_destroy(void *eng_data, EVGL_Context *ctx)
{
   EVGL_Resource *rsc;
   Eina_Bool dbg;

   // Check the input
   if ((!evgl_engine) || (!ctx))
     {
        ERR("Invalid input data.  Engine: %p  Context:%p", evgl_engine, ctx);
        return 0;
     }

   // Retrieve the resource object
   if (!(rsc = _evgl_tls_resource_get()))
     {
        ERR("Error retrieving resource from TLS");
        return 0;
     }

   if ((dbg = evgl_engine->api_debug_mode))
     DBG("Destroying context (eng = %p, ctx = %p)", eng_data, ctx);

   if ((rsc->current_ctx) && (rsc->current_ctx == ctx))
     {
        if (evgl_engine->api_debug_mode)
          {
             ERR("The context is still current before it's being destroyed. "
                 "Calling make_current(NULL, NULL)");
          }
        else
          {
             WRN("The context is still current before it's being destroyed. "
                 "Calling make_current(NULL, NULL)");
          }
        evgl_make_current(eng_data, NULL, NULL);
     }

   if (ctx->current_sfc && (ctx->current_sfc->current_ctx == ctx))
     ctx->current_sfc->current_ctx = NULL;

   // Delete the FBO
   if (ctx->surface_fbo)
     {
        // Set the context current with resource context/surface
        if (!_internal_resource_make_current(eng_data, ctx->current_sfc, ctx))
          {
             ERR("Error doing an internal resource make current");
             return 0;
          }
        glDeleteFramebuffers(1, &ctx->surface_fbo);
     }

   // Retrieve the resource object
   rsc = _evgl_tls_resource_get();
   if (rsc && rsc->current_ctx == ctx)
     {
        // Unset the currrent context
        if (dbg) DBG("Calling make_current(NULL, NULL)");
        if (!evgl_engine->funcs->make_current(eng_data, NULL, NULL, 0))
          {
             ERR("Error doing make_current(NULL, NULL).");
             return 0;
          }
        rsc->current_ctx = NULL;
     }

   // Destroy indirect rendering context
   if (ctx->indirect_context &&
       !evgl_engine->funcs->context_destroy(eng_data, ctx->indirect_context))
     {
        ERR("Error destroying the indirect context.");
        return 0;
     }

   // Destroy engine context
   if (!evgl_engine->funcs->context_destroy(eng_data, ctx->context))
     {
        ERR("Error destroying the engine context.");
        return 0;
     }

   // Remove it from the list
   LKL(evgl_engine->resource_lock);
   evgl_engine->contexts = eina_list_remove(evgl_engine->contexts, ctx);
   LKU(evgl_engine->resource_lock);

   // Free context
   free(ctx);

   return 1;
}

int
evgl_make_current(void *eng_data, EVGL_Surface *sfc, EVGL_Context *ctx)
{
   Eina_Bool dbg = EINA_FALSE;
   EVGL_Resource *rsc;
   int curr_fbo = 0, curr_draw_fbo = 0, curr_read_fbo = 0;
   Eina_Bool ctx_changed = EINA_FALSE;

   // Check the input validity. If either sfc is valid but ctx is NULL, it's also error.
   // sfc can be NULL as evas gl supports surfaceless make current
   if ( (!evgl_engine) || ((sfc) && (!ctx)) )
     {
        ERR("Invalid Input: Engine: %p, Surface: %p, Context: %p", evgl_engine, sfc, ctx);
        if (!evgl_engine) evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
        if (!ctx) evas_gl_common_error_set(EVAS_GL_BAD_CONTEXT);
        return 0;
     }

   // Get TLS Resources
   rsc = _evgl_tls_resource_get();
   if (!rsc)
     {
        DBG("Creating new TLS for this thread: %lu", (unsigned long)eina_thread_self());
        rsc = _evgl_tls_resource_create(eng_data);
        if (!rsc) return 0;
     }

   // Abuse debug mode - extra tracing for make_current and friends
   dbg = evgl_engine->api_debug_mode;
   if (dbg) DBG("(eng = %p, sfc = %p, ctx = %p), rsc = %p", eng_data, sfc, ctx, rsc);

   // Unset
   if ((!sfc) && (!ctx))
     {
        if (rsc->current_ctx)
          {
             if (rsc->direct.partial.enabled)
               evgl_direct_partial_render_end();

             if (rsc->current_ctx->version == EVAS_GL_GLES_3_X)
               {
                  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &curr_draw_fbo);
                  if ((rsc->current_ctx->surface_fbo == (GLuint) curr_draw_fbo) ||
                      (rsc->current_ctx->current_sfc &&
                       rsc->current_ctx->current_sfc->color_buf == (GLuint) curr_draw_fbo))
                    {
                       glBindFramebuffer(GL_FRAMEBUFFER, 0);
                       rsc->current_ctx->current_draw_fbo = 0;
                       rsc->current_ctx->current_read_fbo = 0;
                    }
               }
             else
               {
                  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &curr_fbo);
                  if ((rsc->current_ctx->surface_fbo == (GLuint) curr_fbo) ||
                      (rsc->current_ctx->current_sfc &&
                       rsc->current_ctx->current_sfc->color_buf == (GLuint) curr_fbo))
                    {
                       glBindFramebuffer(GL_FRAMEBUFFER, 0);
                       rsc->current_ctx->current_fbo = 0;
                    }
               }

          }

        if (dbg) DBG("Calling make_current(NULL, NULL)");
        if (!evgl_engine->funcs->make_current(eng_data, NULL, NULL, 0))
          {
             ERR("Error calling make_current(NULL, NULL).");
             return 0;
          }

        // FIXME -- What is this "FIXME" about?
        if (rsc->current_ctx)
          {
             rsc->current_ctx->current_sfc = NULL;
             rsc->current_ctx = NULL;
             rsc->current_eng = NULL;
          }
        if (dbg) DBG("Call to make_current(NULL, NULL) was successful.");
        return 1;
     }

   // Disable partial rendering for previous context
   if ((rsc->current_ctx) && (rsc->current_ctx != ctx))
     {
        evas_gl_common_tiling_done(NULL);
        rsc->current_ctx->partial_render = 0;
     }

   // Do a make current
   if (!_internal_resource_make_current(eng_data, sfc, ctx))
     {
        ERR("Error doing a make current with internal surface. Context: %p", ctx);
        evas_gl_common_error_set(EVAS_GL_BAD_CONTEXT);
        return 0;
     }

   if (rsc->current_ctx != ctx)
     ctx_changed = EINA_TRUE;

   rsc->current_ctx = ctx;
   rsc->current_eng = eng_data;

   // Check whether extensions are supported for the current context version
   // to use fbo & egl image passing to evas
   if (!ctx->extension_checked)
     {
        if (!evgl_api_get(eng_data, ctx->version, EINA_FALSE))
          {
             ERR("Unable to get the list of GL APIs for version %d", ctx->version);
             evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
             return 0;
          }

        if (!_context_ext_check(ctx))
          {
             ERR("Unable to check required extension for the current context");
             evas_gl_common_error_set(EVAS_GL_NOT_INITIALIZED);
             return 0;
          }
     }

   if (sfc) sfc->current_ctx = ctx;
   else
     {
        DBG("Performing surfaceless make current");
        glViewport(0, 0, 0, 0);
        glScissor(0, 0, 0, 0);
        rsc->direct.rendered = 0;
        goto finish;
     }

   if (!sfc->buffers_skip_allocate)
     {
        if (!sfc->color_buf && !_surface_buffers_create(sfc))
          {
             ERR("Unable to create specified surfaces.");
             evas_gl_common_error_set(EVAS_GL_BAD_ALLOC);
             return 0;
          }

        // Allocate or free resources depending on what mode (direct of fbo) it's
        // running only if the env var EVAS_GL_DIRECT_MEM_OPT is set.
        if (sfc->direct_mem_opt)
          {
             if (_evgl_direct_renderable(rsc, sfc))
               {
                  if (dbg) DBG("sfc %p is direct renderable (has buffers: %d).", sfc, (int) sfc->buffers_allocated);

                  // Destroy created resources
                  if (sfc->buffers_allocated)
                    {
                       if (!_surface_buffers_allocate(eng_data, sfc, 0, 0, ctx->version))
                         {
                            ERR("Unable to destroy surface buffers!");
                            evas_gl_common_error_set(EVAS_GL_BAD_ALLOC);
                            return 0;
                         }
                       sfc->buffers_allocated = 0;
                    }
               }
             else
               {
                  if (sfc->direct_override)
                    {
                       DBG("Not creating fallback surfaces even though it should. Use at OWN discretion!");
                    }
                  else
                    {
                       // Create internal buffers if not yet created
                       if (!sfc->buffers_allocated)
                         {
                            if (dbg) DBG("Allocating buffers for sfc %p", sfc);
                            if (!_surface_buffers_allocate(eng_data, sfc, sfc->w, sfc->h, ctx->version))
                              {
                                 ERR("Unable Create Specificed Surfaces.  Unsupported format!");
                                 evas_gl_common_error_set(EVAS_GL_BAD_ALLOC);
                                 return 0;
                              }
                            sfc->buffers_allocated = 1;
                         }
                    }
               }
          }
        else
          {
             if (!sfc->buffers_allocated)
               {
                  if (!_surface_buffers_allocate(eng_data, sfc, sfc->w, sfc->h, ctx->version))
                    {
                       ERR("Unable Create Allocate Memory for Surface.");
                       evas_gl_common_error_set(EVAS_GL_BAD_ALLOC);
                       return 0;
                    }
                  sfc->buffers_allocated = 1;
               }
          }
     }

   if (ctx->pixmap_image_supported)
     {
        if (dbg) DBG("ctx %p is GLES %d", ctx, ctx->version);
        if (_evgl_direct_renderable(rsc, sfc))
          {
             // Transition from indirect rendering to direct rendering
             if (!rsc->direct.rendered)
               {
                  // Restore viewport and scissor test to direct rendering mode
                  glViewport(ctx->viewport_direct[0], ctx->viewport_direct[1], ctx->viewport_direct[2], ctx->viewport_direct[3]);
                  if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
                    glEnable(GL_SCISSOR_TEST);
               }
             if (dbg) DBG("sfc %p is direct renderable.", sfc);
             rsc->direct.rendered = 1;
          }
        else
          {
             // Transition from direct rendering to indirect rendering
             if (rsc->direct.rendered)
               {
                  glViewport(ctx->viewport_coord[0], ctx->viewport_coord[1], ctx->viewport_coord[2], ctx->viewport_coord[3]);
                  if ((ctx->direct_scissor) && (!ctx->scissor_enabled))
                    glDisable(GL_SCISSOR_TEST);
             }

             if (ctx->version == EVAS_GL_GLES_3_X)
               {
                  ctx->current_draw_fbo = 0;
                  ctx->current_read_fbo = 0;
               }
             else
               {
                  ctx->current_fbo = 0;
               }

             rsc->direct.rendered = 0;
          }
     }
   else
     {
        // Direct Rendering
        if (_evgl_direct_renderable(rsc, sfc))
          {
             if (dbg) DBG("sfc %p is direct renderable.", sfc);

             // Create FBO if it hasn't been created
             if (!ctx->surface_fbo)
               _framebuffer_create(&ctx->surface_fbo, ctx->version);

             if (ctx->version == EVAS_GL_GLES_3_X)
               {
                  // This is to transition from FBO rendering to direct rendering
                  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &curr_draw_fbo);
                  if (ctx->surface_fbo == (GLuint)curr_draw_fbo)
                    {
                       _framebuffer_draw_bind(0, ctx->version);
                       ctx->current_draw_fbo = 0;
                    }
                  else if (ctx->current_sfc && (ctx->current_sfc->pbuffer.is_pbuffer))
                    {
                       // Using the same context, we were rendering on a pbuffer
                       _framebuffer_draw_bind(0, ctx->version);
                       ctx->current_draw_fbo = 0;
                    }

                  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &curr_read_fbo);
                  if (ctx->surface_fbo == (GLuint)curr_read_fbo)
                    {
                       _framebuffer_read_bind(0, ctx->version);
                       ctx->current_read_fbo = 0;
                    }
                  else if (ctx->current_sfc && (ctx->current_sfc->pbuffer.is_pbuffer))
                    {
                       _framebuffer_read_bind(0, ctx->version);
                       ctx->current_read_fbo = 0;
                    }

                  if (ctx->current_read_fbo == 0)
                    {
                       // If master clip is set and clip is greater than 0, do partial render
                       if (rsc->direct.partial.enabled)
                         {
                            if (!ctx->partial_render)
                              {
                                 evgl_direct_partial_render_start();
                                 ctx->partial_render = 1;
                              }
                         }
                    }
               }
             else
               {
                  // This is to transition from FBO rendering to direct rendering
                  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &curr_fbo);
                  if (ctx->surface_fbo == (GLuint)curr_fbo)
                    {
                       _framebuffer_bind(0, ctx->version);
                       ctx->current_fbo = 0;
                    }
                  else if (ctx->current_sfc && (ctx->current_sfc->pbuffer.is_pbuffer))
                    {
                       // Using the same context, we were rendering on a pbuffer
                       _framebuffer_bind(0, ctx->version);
                       ctx->current_fbo = 0;
                    }

                  if (ctx->current_fbo == 0)
                    {
                       // If master clip is set and clip is greater than 0, do partial render
                       if (rsc->direct.partial.enabled)
                         {
                            if (!ctx->partial_render)
                              {
                                 evgl_direct_partial_render_start();
                                 ctx->partial_render = 1;
                              }
                         }
                    }
               }

             rsc->direct.rendered = 1;
          }
        else if (sfc->pbuffer.native_surface)
          {
             if (dbg) DBG("Surface sfc %p is a pbuffer: %p", sfc, sfc->pbuffer.native_surface);

             // Call end tiling
             if (rsc->direct.partial.enabled)
               evgl_direct_partial_render_end();

             // Bind to the previously bound buffer (may be 0)
             if (ctx->version == EVAS_GL_GLES_3_X)
               {
                  if (ctx->current_draw_fbo)
                    {
                       _framebuffer_draw_bind(ctx->current_draw_fbo, ctx->version);
                    }
               }
             else
               {
                  if (ctx->current_fbo)
                    {
                       _framebuffer_bind(ctx->current_fbo, ctx->version);
                    }
               }

             rsc->direct.rendered = 0;
          }
        else
          {
             if (dbg) DBG("Surface sfc %p is a normal surface.", sfc);

             // Create FBO if it hasn't been created
             if (!ctx->surface_fbo)
               _framebuffer_create(&ctx->surface_fbo, ctx->version);

             // Attach fbo and the buffers
             if ((ctx_changed) || (ctx->current_sfc != sfc) || (rsc->direct.rendered))
               {
                  sfc->current_ctx = ctx;
                  if ((sfc->direct_mem_opt) && (sfc->direct_override))
                    {
                       DBG("Not creating fallback surfaces even though it should. Use at OWN discretion!");
                    }
                  else
                    {
                       // If it's transitioning from direct render to fbo render
                       // Call end tiling
                       if (rsc->direct.partial.enabled)
                          evgl_direct_partial_render_end();

                       if (!_surface_buffers_fbo_set(sfc, ctx->surface_fbo, ctx->version))
                         {
                            ERR("Attaching buffers to context fbo failed. Engine: %p  Surface: %p Context FBO: %u", evgl_engine, sfc, ctx->surface_fbo);
                            evas_gl_common_error_set(EVAS_GL_BAD_CONTEXT);
                            return 0;
                         }
                    }

                  // Bind to the previously bound buffer

                  if (ctx->version == EVAS_GL_GLES_3_X)
                    {
                       if (ctx->current_draw_fbo)
                         _framebuffer_draw_bind(ctx->current_draw_fbo, ctx->version);

                       if (ctx->current_read_fbo)
                         _framebuffer_read_bind(ctx->current_read_fbo, ctx->version);
                    }
                  else
                    {
                       if (ctx->current_fbo)
                         _framebuffer_bind(ctx->current_fbo, ctx->version);
                    }
               }
             rsc->direct.rendered = 0;
          }
     }

finish:
   ctx->current_sfc = sfc;

   _surface_context_list_print();

   return 1;
}

const char *
evgl_string_query(int name)
{
   EVGL_Resource *rsc;
   int ctx_version = EVAS_GL_GLES_2_X;

   switch(name)
     {
      case EVAS_GL_EXTENSIONS:
        {
           Eina_Strbuf *extstr = eina_strbuf_new();
           const char *glstr = NULL, *eglstr = NULL, *str = NULL;
           rsc = _evgl_tls_resource_get();
           if ((rsc) && (rsc->current_ctx))
             ctx_version = rsc->current_ctx->version;
           glstr = evgl_api_ext_string_get(EINA_FALSE, ctx_version);
           if (glstr)
             eina_strbuf_append(extstr, glstr);
           eglstr = evgl_api_ext_egl_string_get();
           if (eglstr)
             eina_strbuf_append(extstr, eglstr);
           str = eina_strbuf_string_steal(extstr);
           eina_strbuf_free(extstr);
           return str;
        }

      default:
         return "";
     };
}

void
evgl_safe_extension_add(const char *name, void *funcptr)
{
   if (!name) return;

   if (evgl_engine->api_debug_mode)
     DBG("Whitelisting function [%p] %s", funcptr, name);

   if (funcptr)
     eina_hash_set(evgl_engine->safe_extensions, name, funcptr);
   else
     eina_hash_set(evgl_engine->safe_extensions, name, (void*) 0x1);
}

Eina_Bool
evgl_safe_extension_get(const char *name, void **pfuncptr)
{
   static Eina_Bool _unsafe_checked = EINA_FALSE, _unsafe = EINA_FALSE;
   void *func;

   if (!name || !*name)
     return EINA_FALSE;

   // use this for debugging or if you want to break everything
   if (!_unsafe_checked)
     {
        if (getenv("EVAS_GL_UNSAFE_EXTENSIONS"))
          _unsafe = EINA_TRUE;
     }

   if (_unsafe)
     return EINA_TRUE;

   func = eina_hash_find(evgl_engine->safe_extensions, name);
   if (!func) return EINA_FALSE;

   // this is for safe extensions of which we didn't resolve the address
   if (func == (void *) 0x1)
     {
        if (pfuncptr) *pfuncptr = NULL;
        return EINA_TRUE;
     }

   if (pfuncptr) *pfuncptr = func;
   return EINA_TRUE;
}

void *
evgl_native_surface_buffer_get(EVGL_Surface *sfc, Eina_Bool *is_egl_image)
{
   void *buf = NULL;
   *is_egl_image = EINA_FALSE;

   if (!evgl_engine)
     {
        ERR("Invalid input data.  Engine: %p", evgl_engine);
        return NULL;
     }

   if (sfc->egl_image)
     {
        buf = sfc->egl_image;
        *is_egl_image = EINA_TRUE;
     }
   else
     {
        buf = (void *)(uintptr_t)sfc->color_buf;
     }

   return buf;
}

int
evgl_native_surface_yinvert_get(EVGL_Surface *sfc)
{
   int ret = 0;
   if (!evgl_engine)
     {
        ERR("Invalid input data.  Engine: %p", evgl_engine);
        return 0;
     }

   if (sfc->indirect)
      ret = sfc->yinvert;

   return ret;
}

int
evgl_native_surface_get(EVGL_Surface *sfc, Evas_Native_Surface *ns)
{
   // Check the input
   if ((!evgl_engine) || (!ns))
     {
        ERR("Invalid input data.  Engine: %p  NS:%p", evgl_engine, ns);
        return 0;
     }

   ns->type = EVAS_NATIVE_SURFACE_EVASGL;
   ns->version = EVAS_NATIVE_SURFACE_VERSION;
   ns->data.evasgl.surface = sfc;

   return 1;
}

int
evgl_direct_rendered(void)
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get())) return 0;

   return rsc->direct.rendered;
}

/*
 * This function can tell the engine whether a surface can be directly
 * rendered to the Evas, despite any window rotation. For that purpose,
 * we let the engine know the surface flags for this texture
 */
Eina_Bool
evgl_native_surface_direct_opts_get(Evas_Native_Surface *ns,
                                    Eina_Bool *direct_render,
                                    Eina_Bool *client_side_rotation,
                                    Eina_Bool *direct_override)
{
   EVGL_Surface *sfc;

   if (direct_render) *direct_render = EINA_FALSE;
   if (direct_override) *direct_override = EINA_FALSE;
   if (client_side_rotation) *client_side_rotation = EINA_FALSE;

   if (!evgl_engine) return EINA_FALSE;
   if (!ns) return EINA_FALSE;

   if (ns->type == EVAS_NATIVE_SURFACE_EVASGL &&
            ns->data.evasgl.surface)
     {
        sfc = ns->data.evasgl.surface;
     }
   else return EINA_FALSE;

   if (evgl_engine->api_debug_mode)
     {
        DBG("Found native surface:  DR:%d CSR:%d",
            (int) sfc->direct_fb_opt, (int) sfc->client_side_rotation);
     }

   if (direct_render) *direct_render = sfc->direct_fb_opt;
   if (direct_override) *direct_override |= sfc->direct_override;
   if (client_side_rotation) *client_side_rotation = sfc->client_side_rotation;
   return EINA_TRUE;
}

void
evgl_direct_info_set(int win_w, int win_h, int rot,
                     int img_x, int img_y, int img_w, int img_h,
                     int clip_x, int clip_y, int clip_w, int clip_h,
                     int render_op, void *surface)
{
   EVGL_Resource *rsc;
   EVGL_Surface *sfc = surface;

   if (!(rsc = _evgl_tls_resource_get()))
     return;

   /* Check for direct rendering
    *
    * DR is allowed iif:
    * - Rotation == 0
    * OR: - Client-Side Rotation is set on the surface
    *     - Direct Override is set
    *
    * If the surface is not found, we assume indirect rendering.
    */

   if (sfc &&
       ((rot == 0) || sfc->client_side_rotation || sfc->direct_override))
     {
        if (evgl_engine->api_debug_mode)
          DBG("Direct rendering is enabled.");

        rsc->direct.enabled = EINA_TRUE;

        rsc->direct.win_w   = win_w;
        rsc->direct.win_h   = win_h;
        rsc->direct.rot     = rot;

        rsc->direct.img.x   = img_x;
        rsc->direct.img.y   = img_y;
        rsc->direct.img.w   = img_w;
        rsc->direct.img.h   = img_h;

        rsc->direct.clip.x  = clip_x;
        rsc->direct.clip.y  = clip_y;
        rsc->direct.clip.w  = clip_w;
        rsc->direct.clip.h  = clip_h;

        rsc->direct.render_op_copy = (render_op == EVAS_RENDER_COPY);
     }
   else
     {
        if (evgl_engine->api_debug_mode)
          DBG("Direct rendering is disabled.");

        rsc->direct.enabled = EINA_FALSE;
     }
}

void
evgl_direct_info_clear(void)
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get())) return;

   rsc->direct.enabled = EINA_FALSE;
}

void
evgl_get_pixels_pre(void)
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get())) return;

   rsc->direct.in_get_pixels = EINA_TRUE;
}

void
evgl_get_pixels_post(void)
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get())) return;

   rsc->direct.in_get_pixels = EINA_FALSE;
}

Evas_GL_API *
evgl_api_get(void *eng_data, Evas_GL_Context_Version version, Eina_Bool alloc_only)
{
   Evas_GL_API *api = NULL;
   int minor_version = 0;

   if (version == EVAS_GL_GLES_2_X)
     {
        if (!gles2_funcs) gles2_funcs = calloc(1, EVAS_GL_API_STRUCT_SIZE);
        api = gles2_funcs;
     }
   else if (version == EVAS_GL_GLES_1_X)
     {
        if (!gles1_funcs) gles1_funcs = calloc(1, EVAS_GL_API_STRUCT_SIZE);
        api = gles1_funcs;
     }
   else if (version == EVAS_GL_GLES_3_X)
     {
        if (evas_gl_common_version_check(&minor_version) < 3)
          {
             ERR("OpenGL ES 3.x is not supported.");
             return NULL;
          }
        if (!gles3_funcs) gles3_funcs = calloc(1, EVAS_GL_API_STRUCT_SIZE);
        api = gles3_funcs;
     }
   if (!api) return NULL;
   if (alloc_only && (api->version == EVAS_GL_API_VERSION))
     return api;

#ifdef GL_GLES
    if (!evgl_api_egl_ext_init(evgl_engine->funcs->proc_address_get, evgl_engine->funcs->ext_string_get(eng_data)))
      {
         ERR("EGL extensions initialization failed");
      }
#endif
   if (version == EVAS_GL_GLES_2_X)
     {
        _evgl_api_gles2_get(api, evgl_engine->api_debug_mode);
        evgl_api_gles2_ext_get(api, evgl_engine->funcs->proc_address_get, evgl_engine->funcs->ext_string_get(eng_data));
     }
   else if (version == EVAS_GL_GLES_1_X)
     {
        _evgl_api_gles1_get(api, evgl_engine->api_debug_mode);
        evgl_api_gles1_ext_get(api, evgl_engine->funcs->proc_address_get, evgl_engine->funcs->ext_string_get(eng_data));
     }
   else if (version == EVAS_GL_GLES_3_X)
     {
        void *(*get_proc_address)(const char *) = NULL;
        const char *egl_exts;

        egl_exts = evgl_engine->funcs->ext_string_get(eng_data);
        if (egl_exts && strstr(egl_exts, "EGL_KHR_get_all_proc_addresses"))
          get_proc_address = evgl_engine->funcs->proc_address_get;

        _evgl_api_gles3_get(api, get_proc_address, evgl_engine->api_debug_mode, minor_version);
        evgl_api_gles3_ext_get(api, evgl_engine->funcs->proc_address_get, evgl_engine->funcs->ext_string_get(eng_data));
     }

   return api;
}


void
evgl_direct_partial_info_set(int pres)
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get())) return;

   rsc->direct.partial.enabled  = EINA_TRUE;
   rsc->direct.partial.preserve = pres;
}

void
evgl_direct_partial_info_clear()
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get())) return;

   rsc->direct.partial.enabled = EINA_FALSE;
}

void
evgl_direct_override_get(Eina_Bool *override, Eina_Bool *force_off)
{
   if (!evgl_engine) return;
   if (override)  *override  = evgl_engine->direct_override;
   if (force_off) *force_off = evgl_engine->direct_force_off;
}

void
evgl_direct_partial_render_start()
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get())) return;

   evas_gl_common_tiling_start(NULL,
                               rsc->direct.rot,
                               rsc->direct.win_w,
                               rsc->direct.win_h,
                               rsc->direct.clip.x,
                               rsc->direct.win_h - rsc->direct.clip.y - rsc->direct.clip.h,
                               rsc->direct.clip.w,
                               rsc->direct.clip.h,
                               rsc->direct.partial.preserve);

   if (!rsc->direct.partial.preserve)
      rsc->direct.partial.preserve = GL_COLOR_BUFFER_BIT0_QCOM;
}

void
evgl_direct_partial_render_end()
{
   EVGL_Context *ctx;
   ctx = evas_gl_common_current_context_get();

   if (!ctx) return;

   if (ctx->partial_render)
     {
        evas_gl_common_tiling_done(NULL);
        ctx->partial_render = 0;
     }
}

EAPI void
evas_gl_common_context_restore_set(Eina_Bool enable)
{
   _need_context_restore = enable;
}

//-----------------------------------------------------//


