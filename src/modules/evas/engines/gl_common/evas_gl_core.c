#include "evas_gl_core_private.h"
#include <dlfcn.h>

// EVGL GL Format Pair
typedef struct _GL_Format
{
   int    bit;
   GLenum fmt;
} GL_Format;

// Globals
static Evas_GL_API gl_funcs;
EVGL_Engine *evgl_engine = NULL;
int _evas_gl_log_dom   = -1;
int _evas_gl_log_level = -1;

static void _surface_cap_print(int error);
static void _surface_context_list_print();
static void _internal_resources_destroy(void *eng_data, EVGL_Resource *rsc);


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

   // Create resource surface
   rsc->window = evgl_engine->funcs->native_window_create(eng_data);
   if (!rsc->window)
     {
        ERR("Error creating native window");
        goto error;
     }

   rsc->surface = evgl_engine->funcs->surface_create(eng_data, rsc->window);
   if (!rsc->surface)
     {
        ERR("Error creating native surface");
        goto error;
     }

   // Create a resource context
   rsc->context = evgl_engine->funcs->context_create(eng_data, NULL);
   if (!rsc->context)
     {
        ERR("Internal resource context creations failed.");
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
_internal_resource_make_current(void *eng_data, EVGL_Context *ctx)
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
             ERR("Error creting resources in tls.");
             return 0;
          }
     }

   // Set context from input or from resource
   if (ctx)
      context = ctx->context;
   else
      context = (void*)rsc->context;

   // Set the surface to evas surface if it's there
   if (rsc->id == evgl_engine->main_tid)
      rsc->direct.surface = evgl_engine->funcs->evas_surface_get(eng_data);

   if (rsc->direct.surface)
      surface = (void*)rsc->direct.surface;
   else
      surface = (void*)rsc->surface;

   // Do the make current
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
void
_texture_create(GLuint *tex)
{
   glGenTextures(1, tex);
}

// Create and allocate 2D texture
void
_texture_allocate_2d(GLuint tex, GLint ifmt, GLenum fmt, GLenum type, int w, int h)
{
   //if (!(*tex))
   //   glGenTextures(1, tex);
   glBindTexture(GL_TEXTURE_2D, tex );
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, ifmt, w, h, 0, fmt, type, NULL);
   glBindTexture(GL_TEXTURE_2D, 0);
}

// Destroy Texture
void
_texture_destroy(GLuint *tex)
{
   if (*tex)
     {
        glDeleteTextures(1, tex);
        *tex = 0;
     }
}

// Attach 2D texture with the given format to already bound FBO
// *NOTE: attach2 here is used for depth_stencil attachment in GLES env.
void
_texture_attach_2d(GLuint tex, GLenum attach, GLenum attach2, int samples)
{
   if (samples)
     {
#ifdef GL_GLES
        //<<< TODO : CHECK EXTENSION SUPPORT>>>
        EXT_FUNC(glFramebufferTexture2DMultisample)(GL_FRAMEBUFFER,
                                                    attach,
                                                    GL_TEXTURE_2D, tex,
                                                    0, samples);

        if (attach2)
           EXT_FUNC(glFramebufferTexture2DMultisample)(GL_FRAMEBUFFER,
                                                       attach2,
                                                       GL_TEXTURE_2D, tex,
                                                       0, samples);
#else
        ERR("MSAA not supported.  Should not have come in here...!");
#endif
     }
   else
     {
        glFramebufferTexture2D(GL_FRAMEBUFFER, attach, GL_TEXTURE_2D, tex, 0);

        if (attach2)
           glFramebufferTexture2D(GL_FRAMEBUFFER, attach2, GL_TEXTURE_2D, tex, 0);

     }
}

// Gen Renderbuffer
void
_renderbuffer_create(GLuint *buf)
{
   glGenRenderbuffers(1, buf);
}


// Attach a renderbuffer with the given format to already bound FBO
void
_renderbuffer_allocate(GLuint buf, GLenum fmt, int w, int h, int samples)
{
   glBindRenderbuffer(GL_RENDERBUFFER, buf);
   if (samples)
#ifdef GL_GLES
      EXT_FUNC(glRenderbufferStorageMultisample)(GL_RENDERBUFFER, samples, fmt, w, h);
#else
      ERR("MSAA not supported.  Should not have come in here...!");
#endif
   else
      glRenderbufferStorage(GL_RENDERBUFFER, fmt, w, h);
   glBindRenderbuffer(GL_RENDERBUFFER, 0);

   return;
   samples = 0;
}

void
_renderbuffer_destroy(GLuint *buf)
{
   if (*buf)
     {
        glDeleteRenderbuffers(1, buf);
        *buf = 0;
     }
}

// Attach a renderbuffer with the given format to already bound FBO
void
_renderbuffer_attach(GLuint buf, GLenum attach)
{
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, attach, GL_RENDERBUFFER, buf);
}

// Check whether the given FBO surface config is supported by the driver
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

   // Gen FBO
   glGenFramebuffers(1, &fbo);
   glBindFramebuffer(GL_FRAMEBUFFER, fbo);

   // Color Buffer Texture
   if ((color_ifmt) && (color_fmt))
     {
        _texture_create(&color_buf);
        _texture_allocate_2d(color_buf, color_ifmt, color_fmt, GL_UNSIGNED_BYTE, w, h);
        _texture_attach_2d(color_buf, GL_COLOR_ATTACHMENT0, 0, mult_samples);
     }

   // Check Depth_Stencil Format First
#ifdef GL_GLES
   if (depth_fmt == GL_DEPTH_STENCIL_OES)
     {
        _texture_create(&depth_stencil_buf);
        _texture_allocate_2d(depth_stencil_buf, depth_fmt,
                           depth_fmt, GL_UNSIGNED_INT_24_8_OES, w, h);
        _texture_attach_2d(depth_stencil_buf, GL_DEPTH_ATTACHMENT,
                           GL_STENCIL_ATTACHMENT, mult_samples);
        depth_stencil = 1;
     }
#else
   if (depth_fmt == GL_DEPTH24_STENCIL8)
     {
        _renderbuffer_create(&depth_stencil_buf);
        _renderbuffer_allocate(depth_stencil_buf, depth_fmt, w, h, mult_samples);
        _renderbuffer_attach(depth_stencil_buf, GL_DEPTH_STENCIL_ATTACHMENT);
        depth_stencil = 1;
     }
#endif

   // Depth Attachment
   if ((!depth_stencil) && (depth_fmt))
     {
        _renderbuffer_create(&depth_buf);
        _renderbuffer_allocate(depth_buf, depth_fmt, w, h, mult_samples);
        _renderbuffer_attach(depth_buf, GL_DEPTH_ATTACHMENT);
     }

   // Stencil Attachment
   if ((!depth_stencil) && (stencil_fmt))
     {
        _renderbuffer_create(&stencil_buf);
        _renderbuffer_allocate(stencil_buf, stencil_fmt, w, h, mult_samples);
        _renderbuffer_attach(stencil_buf, GL_STENCIL_ATTACHMENT);
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
       // Put Error Log...
      return 0;
   }
   else
      return 1;
}

int
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


int
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
   int tmpfd;
   int res = 0;
   char cap_dir_path[PATH_MAX];
   char cap_file_path[PATH_MAX];
   char tmp_file[PATH_MAX];

   if (!evas_gl_common_file_cache_dir_check(cap_dir_path, sizeof(cap_dir_path)))
     {
        res = evas_gl_common_file_cache_mkpath(cap_dir_path);
        if (!res) return 0; /* we can't make directory */
     }

   evas_gl_common_file_cache_file_check(cap_dir_path, "surface_cap", cap_file_path,
                                        sizeof(cap_dir_path));

   /* use mkstemp for writing */
   snprintf(tmp_file, sizeof(tmp_file), "%s.XXXXXX", cap_file_path);
   tmpfd = mkstemp(tmp_file);
   if (tmpfd < 0) goto error;
   close(tmpfd);

   /* use eet */
   if (!eet_init()) goto error;

   et = eet_open(tmp_file, EET_FILE_MODE_WRITE);
   if (!et) goto error;

   if (!_surface_cap_save(et)) goto error;

   if (eet_close(et) != EET_ERROR_NONE) goto error;
   if (rename(tmp_file,cap_file_path) < 0) goto error;
   eet_shutdown();
   return 1;

error:
   if (et) eet_close(et);
   if (evas_gl_common_file_cache_file_exists(tmp_file)) unlink(tmp_file);
   eet_shutdown();
   return 0;
}

static int
_surface_cap_init(void *eng_data)
{
   int max_size = 0;

   // Do internal make current
   if (!_internal_resource_make_current(eng_data, NULL))
     {
        ERR("Error doing an internal resource make current");
        return 0;
     }

   // Query the max width and height of the surface
   glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &max_size);

   evgl_engine->caps.max_w = max_size;
   evgl_engine->caps.max_h = max_size;
   DBG("Max Surface Width: %d   Height: %d", evgl_engine->caps.max_w, evgl_engine->caps.max_h);

   // Check for MSAA support
#ifdef GL_GLES
   int max_samples = 0;

   if (EXTENSION_SUPPORT(IMG_multisampled_render_to_texture))
     {
        glGetIntegerv(GL_MAX_SAMPLES_IMG, &max_samples);
     }
   else if (EXTENSION_SUPPORT(EXT_multisampled_render_to_texture))
     {
        glGetIntegerv(GL_MAX_SAMPLES_EXT, &max_samples);
     }

   if (max_samples >= 2)
     {
        evgl_engine->caps.msaa_samples[0] = 2;
        evgl_engine->caps.msaa_samples[1] = (max_samples >> 1) < 2 ? 2 : (max_samples >> 1);
        evgl_engine->caps.msaa_samples[2] = max_samples;
        evgl_engine->caps.msaa_supported  = 1;
     }
#endif

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
        return 1;
     }
   else
     {
        ERR("There are no available surface formats. Error!");
        return 0;
     }
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
   PRINT_LOG("----------------------------------------------------------------------------------------------------------------\n");
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

#undef RESET
#undef GREEN
#undef YELLOW
#undef RED
}

//--------------------------------------------------------//
// Start from here.....
//--------------------------------------------------------//
static int
_surface_buffers_fbo_set(EVGL_Surface *sfc, GLuint fbo)
{
   int status;

   glBindFramebuffer(GL_FRAMEBUFFER, fbo);

   // Detach any previously attached buffers
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, 0, 0);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
#ifdef GL_GLES
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
   glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
#else
   glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
#endif


   // Render Target Texture
   if (sfc->color_buf)
      _texture_attach_2d(sfc->color_buf, GL_COLOR_ATTACHMENT0, 0, sfc->msaa_samples);


   // Depth Stencil RenderBuffer - Attach it to FBO
   if (sfc->depth_stencil_buf)
     {
#ifdef GL_GLES
        _texture_attach_2d(sfc->depth_stencil_buf, GL_DEPTH_ATTACHMENT,
                           GL_STENCIL_ATTACHMENT, sfc->msaa_samples);
#else
        _renderbuffer_attach(sfc->depth_stencil_buf, GL_DEPTH_STENCIL_ATTACHMENT);
#endif
     }

   // Depth RenderBuffer - Attach it to FBO
   if (sfc->depth_buf)
      _renderbuffer_attach(sfc->depth_buf, GL_DEPTH_ATTACHMENT);

   // Stencil RenderBuffer - Attach it to FBO
   if (sfc->stencil_buf)
      _renderbuffer_attach(sfc->stencil_buf, GL_STENCIL_ATTACHMENT);

   // Check FBO for completeness
   status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
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
        _texture_create(&sfc->depth_stencil_buf);
#else
        _renderbuffer_create(&sfc->depth_stencil_buf);
#endif
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
_surface_buffers_allocate(void *eng_data, EVGL_Surface *sfc, int w, int h, int mc)
{
   // Set the context current with resource context/surface
   if (mc)
      if (!_internal_resource_make_current(eng_data, NULL))
        {
           ERR("Error doing an internal resource make current");
           return 0;
        }

   // Create buffers
   if (sfc->color_fmt)
     {
        _texture_allocate_2d(sfc->color_buf, sfc->color_ifmt, sfc->color_fmt,
                             GL_UNSIGNED_BYTE, w, h);
        sfc->buffer_mem[0] = w * h * 4;
     }

   // Depth_stencil buffers or separate buffers
   if (sfc->depth_stencil_fmt)
     {
#ifdef GL_GLES
        _texture_allocate_2d(sfc->depth_stencil_buf, sfc->depth_stencil_fmt,
                             sfc->depth_stencil_fmt, GL_UNSIGNED_INT_24_8_OES,
                             w, h);
#else
        _renderbuffer_allocate(sfc->depth_stencil_buf, sfc->depth_stencil_fmt,
                               w, h, sfc->msaa_samples);
#endif
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
   if (sfc->color_buf)
      _texture_destroy(&sfc->color_buf);
   if (sfc->depth_buf)
      _renderbuffer_destroy(&sfc->depth_buf);
   if (sfc->stencil_buf)
      _renderbuffer_destroy(&sfc->stencil_buf);
   if (sfc->depth_stencil_buf)
     {
#ifdef GL_GLES
        _texture_destroy(&sfc->depth_stencil_buf);
#else
        _renderbuffer_destroy(&sfc->depth_stencil_buf);
#endif
     }

   return 1;
}

static int
_internal_config_set(EVGL_Surface *sfc, Evas_GL_Config *cfg)
{
   int i = 0, cfg_index = -1;
   int color_bit = 0, depth_bit = 0, stencil_bit = 0, msaa_samples = 0;

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        return 0;
     }

   // Convert Config Format to bitmask friendly format
   color_bit = (1 << cfg->color_format);
   if (cfg->depth_bits) depth_bit = (1 << (cfg->depth_bits-1));
   if (cfg->stencil_bits) stencil_bit = (1 << (cfg->stencil_bits-1));
   if (cfg->multisample_bits)
      msaa_samples = evgl_engine->caps.msaa_samples[cfg->multisample_bits-1];

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
             if ( (!stencil_bit) || (evgl_engine->direct_override) )
                sfc->direct_fb_opt = cfg->options_bits & EVAS_GL_OPTIONS_DIRECT;

             cfg_index = i;
             break;
          }
     }

   if (cfg_index < 0)
     {
        ERR("Unable to find the matching config format.");
        return 0;
     }
   else
     {
        DBG("-------------Surface Config---------------");
        DBG("Selected Config Index: %d", cfg_index);
        DBG("  Color Format     : %s", _glenum_string_get(sfc->color_fmt));
        DBG("  Depth Format     : %s", _glenum_string_get(sfc->depth_fmt));
        DBG("  Stencil Format   : %s", _glenum_string_get(sfc->stencil_fmt));
        DBG("  D-Stencil Format : %s", _glenum_string_get(sfc->depth_stencil_fmt));
        DBG("  MSAA Samples     : %d", sfc->msaa_samples);
        DBG("  Direct Option    : %d", sfc->direct_fb_opt);
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
_evgl_tls_resource_get()
{
   EVGL_Resource *rsc = NULL;

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        return NULL;
     }

   rsc = eina_tls_get(evgl_engine->resource_key);

   if (!rsc)
      return NULL;
   else
      return rsc;
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
        rsc->id = evgl_engine->resource_count++;
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

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        return;
     }

   if (!(rsc = _evgl_tls_resource_get()))
     {
        ERR("Error retrieving resource from TLS");
        return;
     }

   LKL(evgl_engine->resource_lock);
   EINA_LIST_FOREACH(evgl_engine->resource_list, l, rsc)
     {
        _internal_resources_destroy(eng_data, rsc);
     }
   eina_list_free(evgl_engine->resource_list);
   LKU(evgl_engine->resource_lock);

   // Destroy TLS
   if (evgl_engine->resource_key)
      eina_tls_free(evgl_engine->resource_key);
   evgl_engine->resource_key = 0;
}

EVGL_Context *
_evgl_current_context_get()
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

int
_evgl_not_in_pixel_get()
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get())) return 1;

   EVGL_Context *ctx = rsc->current_ctx;

   if ((!evgl_engine->direct_force_off) &&
       (rsc->id == evgl_engine->main_tid) &&
       (ctx) &&
       (ctx->current_sfc) &&
       (ctx->current_sfc->direct_fb_opt) &&
       (!rsc->direct.enabled))
      return 1;
   else
      return 0;
}

int
_evgl_direct_enabled()
{
   EVGL_Resource *rsc;
   EVGL_Surface  *sfc;

   if (!(rsc=_evgl_tls_resource_get())) return 0;
   if (!(rsc->current_ctx)) return 0;
   if (!(sfc=rsc->current_ctx->current_sfc)) return 0;

   return _evgl_direct_renderable(rsc, sfc);
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
   int direct_mem_opt = 0, direct_off = 0, direct_soff = 0, debug_mode = 0;
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

   // Assign functions
   evgl_engine->funcs = efunc;

   // Initialize Resource TLS
   if (eina_tls_new(&evgl_engine->resource_key) == EINA_FALSE)
     {
        ERR("Error creating tls key");
        goto error;
     }
   DBG("TLS KEY created: %d", evgl_engine->resource_key);

   // Initialize Extensions
   if (efunc->proc_address_get && efunc->ext_string_get)
      evgl_api_ext_init(efunc->proc_address_get, efunc->ext_string_get(eng_data));
   else
      ERR("Proc address get function not available.  Extension not initialized.");

   if (efunc->ext_string_get)
     DBG("GLUE Extension String: %s", efunc->ext_string_get(eng_data));
   DBG("GL Extension String: %s", glGetString(GL_EXTENSIONS));

   // Surface Caps
   if (!_surface_cap_init(eng_data))
     {
        ERR("Error initializing surface cap");
        goto error;
     }

   // Check if Direct Rendering Memory Optimzation flag is on
   // Creates resources on demand when it fallsback to fbo rendering
   s = getenv("EVAS_GL_DIRECT_MEM_OPT");
   if (s) direct_mem_opt = atoi(s);
   if (direct_mem_opt == 1)
      evgl_engine->direct_mem_opt = 1;

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

   // Maint Thread ID (get tid not available in eina thread yet)
   evgl_engine->main_tid = 0;

   // Clear Function Pointers
   memset(&gl_funcs, 0, sizeof(Evas_GL_API));

   return evgl_engine;

error:
   if (evgl_engine)
     {
        if (evgl_engine->resource_key)
          eina_tls_free(evgl_engine->resource_key);
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
   if (!evgl_engine) return;

   // Log
   eina_log_domain_unregister(_evas_gl_log_dom);
   _evas_gl_log_dom = -1;

   // Destroy internal resources
   _evgl_tls_resource_destroy(eng_data);

   // Free engine
   free(evgl_engine);
   evgl_engine = NULL;
}

void *
evgl_surface_create(void *eng_data, Evas_GL_Config *cfg, int w, int h)
{
   EVGL_Surface *sfc = NULL;
   char *s = NULL;
   int direct_override = 0, direct_mem_opt = 0;

   // Check if engine is valid
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        return NULL;
     }

   if (!cfg)
     {
        ERR("Invalid Config!");
        return NULL;
     }

   // Check the size of the surface
   if ((w > evgl_engine->caps.max_w) || (h > evgl_engine->caps.max_h))
     {
        ERR("Requested surface size [%d, %d] is greater than max supported size [%d, %d]",
             w, h, evgl_engine->caps.max_w, evgl_engine->caps.max_h);
        return NULL;
     }

   // Check for Direct rendering override env var.
   if (!evgl_engine->direct_override)
      if ((s = getenv("EVAS_GL_DIRECT_OVERRIDE")))
        {
           direct_override = atoi(s);
           if (direct_override == 1)
              evgl_engine->direct_override = 1;
        }

   // Check if Direct Rendering Memory Optimzation flag is on
   // Creates resources on demand when it fallsback to fbo rendering
   if (!evgl_engine->direct_mem_opt)
     if ((s = getenv("EVAS_GL_DIRECT_MEM_OPT")))
       {
          direct_mem_opt = atoi(s);
          if (direct_mem_opt == 1)
             evgl_engine->direct_mem_opt = 1;
       }

   // Allocate surface structure
   sfc = calloc(1, sizeof(EVGL_Surface));
   if (!sfc)
     {
        ERR("Surface allocation failed.");
        goto error;
     }

   // Set surface info
   sfc->w = w;
   sfc->h = h;

   // Set the internal config value
   if (!_internal_config_set(sfc, cfg))
     {
        ERR("Unsupported Format!");
        goto error;
     }

   // Set the context current with resource context/surface
   if (!_internal_resource_make_current(eng_data, NULL))
     {
        ERR("Error doing an internal resource make current");
        goto error;
     }

   // Create internal buffers
   if (!_surface_buffers_create(sfc))
     {
        ERR("Unable Create Specificed Surfaces.");
        goto error;
     };

   // Allocate resources for fallback unless the flag is on
   if (!evgl_engine->direct_mem_opt)
     {
        if (!_surface_buffers_allocate(eng_data, sfc, sfc->w, sfc->h, 0))
          {
             ERR("Unable Create Allocate Memory for Surface.");
             goto error;
          }
     }

   if (!evgl_engine->funcs->make_current(eng_data, NULL, NULL, 0))
     {
        ERR("Error doing make_current(NULL, NULL).");
        goto error;
     }

   // Keep track of all the created surfaces
   evgl_engine->surfaces = eina_list_prepend(evgl_engine->surfaces, sfc);

   return sfc;

error:
   if (sfc) free(sfc);
   return NULL;
}


int
evgl_surface_destroy(void *eng_data, EVGL_Surface *sfc)
{
   EVGL_Resource *rsc;

   // Check input parameter
   if ((!evgl_engine) || (!sfc))
     {
        ERR("Invalid input data.  Engine: %p  Surface:%p", evgl_engine, sfc);
        return 0;
     }

   // Retrieve the resource object
   if (!(rsc = _evgl_tls_resource_get()))
     {
        ERR("Error retrieving resource from TLS");
        return 0;
     }

   if ((rsc->current_ctx) && (rsc->current_ctx->current_sfc == sfc) )
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

   // Set the context current with resource context/surface
   if (!_internal_resource_make_current(eng_data, NULL))
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

   if (!evgl_engine->funcs->make_current(eng_data, NULL, NULL, 0))
     {
        ERR("Error doing make_current(NULL, NULL).");
        return 0;
     }

   // Remove it from the list
   evgl_engine->surfaces = eina_list_remove(evgl_engine->surfaces, sfc);

   free(sfc);
   sfc = NULL;

   return 1;

}

void *
evgl_context_create(void *eng_data, EVGL_Context *share_ctx)
{
   EVGL_Context *ctx   = NULL;

   // Check the input
   if (!evgl_engine)
     {
        ERR("Invalid EVGL Engine!");
        return NULL;
     }

   // Allocate context object
   ctx = calloc(1, sizeof(EVGL_Context));
   if (!ctx)
     {
        ERR("Error allocating context object.");
        return NULL;
     }

   if (share_ctx)
      ctx->context = evgl_engine->funcs->context_create(eng_data, share_ctx->context);
   else
      ctx->context = evgl_engine->funcs->context_create(eng_data, NULL);

   // Call engine create context
   if (!ctx->context)
     {
        ERR("Error creating context from the Engine.");
        free(ctx);
        return NULL;
     }

   // Keep track of all the created context
   evgl_engine->contexts = eina_list_prepend(evgl_engine->contexts, ctx);

   return ctx;
}


int
evgl_context_destroy(void *eng_data, EVGL_Context *ctx)
{
   // Check the input
   if ((!evgl_engine) || (!ctx))
     {
        ERR("Invalid input data.  Engine: %p  Context:%p", evgl_engine, ctx);
        return 0;
     }

   // Set the context current with resource context/surface
   if (!_internal_resource_make_current(eng_data, NULL))
     {
        ERR("Error doing an internal resource make current");
        return 0;
     }

   // Delete the FBO
   if (ctx->surface_fbo)
      glDeleteFramebuffers(1, &ctx->surface_fbo);

   // Unset the currrent context
   if (!evgl_engine->funcs->make_current(eng_data, NULL, NULL, 0))
     {
        ERR("Error doing make_current(NULL, NULL).");
        return 0;
     }

   // Destroy engine context
   if (!evgl_engine->funcs->context_destroy(eng_data, ctx->context))
     {
        ERR("Error destroying the engine context.");
        return 0;
     }

   // Remove it from the list
   evgl_engine->contexts = eina_list_remove(evgl_engine->contexts, ctx);

   // Free context
   free(ctx);
   ctx = NULL;

   return 1;
}


int
evgl_make_current(void *eng_data, EVGL_Surface *sfc, EVGL_Context *ctx)
{
   EVGL_Resource *rsc;
   int curr_fbo = 0;

   // Check the input validity. If either sfc or ctx is NULL, it's also error.
   if ( (!evgl_engine) ||
        ((!sfc) && ctx) ||
        (sfc && (!ctx)) )
     {
        ERR("Invalid Inputs. Engine: %p  Surface: %p   Context: %p!", evgl_engine, sfc, ctx);
        return 0;
     }

   // Get TLS Resources
   if (!(rsc = _evgl_tls_resource_get())) return 0;

   // Unset
   if ((!sfc) && (!ctx))
     {
        if (rsc->current_ctx)
          {
             if (rsc->direct.partial.enabled)
                evgl_direct_partial_render_end();
          }

        if (!evgl_engine->funcs->make_current(eng_data, NULL, NULL, 0))
          {
             ERR("Error doing make_current(NULL, NULL).");
             return 0;
          }
        //FIXME!!!
        if (rsc->current_ctx)
          {
             rsc->current_ctx->current_sfc = NULL;
             rsc->current_ctx = NULL;
             rsc->current_eng = NULL;
          }
        return 1;
     }

   // Disable partial rendering for previous context
   if ((rsc->current_ctx) && (rsc->current_ctx != ctx))
     {
        evas_gl_common_tiling_done(NULL);
        rsc->current_ctx->partial_render = 0;
     }

   // Allocate or free resources depending on what mode (direct of fbo) it's
   // running only if the env var EVAS_GL_DIRECT_MEM_OPT is set.
   if (evgl_engine->direct_mem_opt)
     {
        if (_evgl_direct_renderable(rsc, sfc))
          {
             // Destroy created resources
             if (sfc->buffers_allocated)
               {
                  if (!_surface_buffers_allocate(eng_data, sfc, 0, 0, 1))
                    {
                       ERR("Unable to destroy surface buffers!");
                       return 0;
                    }
                  sfc->buffers_allocated = 0;
               }
          }
        else
          {
             if (evgl_engine->direct_override)
               {
                  DBG("Not creating fallback surfaces even though it should. Use at OWN discretion!");
               }
             else
               {
                  // Create internal buffers if not yet created
                  if (!sfc->buffers_allocated)
                    {
                       if (!_surface_buffers_allocate(eng_data, sfc, sfc->w, sfc->h, 1))
                         {
                            ERR("Unable Create Specificed Surfaces.  Unsupported format!");
                            return 0;
                         };
                       sfc->buffers_allocated = 1;
                    }
               }
          }
     }

   // Do a make current
   if (!_internal_resource_make_current(eng_data, ctx))
     {
        ERR("Error doing a make current with internal surface. Context: %p", ctx);
        return 0;
     }

   // Normal FBO Rendering
   // Create FBO if it hasn't been created
   if (!ctx->surface_fbo)
      glGenFramebuffers(1, &ctx->surface_fbo);

   // Direct Rendering
   if (_evgl_direct_renderable(rsc, sfc))
     {
        // This is to transition from FBO rendering to direct rendering
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &curr_fbo);
        if (ctx->surface_fbo == (GLuint)curr_fbo)
          {
             glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

        rsc->direct.rendered = 1;
     }
   else
     {
        // Attach fbo and the buffers
        if ((ctx->current_sfc != sfc) || (ctx != sfc->current_ctx))
          {
             sfc->current_ctx = ctx;
             if ((evgl_engine->direct_mem_opt) && (evgl_engine->direct_override))
               {
                  DBG("Not creating fallback surfaces even though it should. Use at OWN discretion!");
               }
             else
               {
                  // If it's transitioning from direct render to fbo render
                  // Call end tiling
                  if (rsc->direct.partial.enabled)
                     evgl_direct_partial_render_end();

                  if (!_surface_buffers_fbo_set(sfc, ctx->surface_fbo))
                    {
                       ERR("Attaching buffers to context fbo failed. Engine: %p  Surface: %p Context FBO: %u", evgl_engine, sfc, ctx->surface_fbo);
                       return 0;
                    }
               }

             // Bind to the previously bound buffer
             if (ctx->current_fbo)
                glBindFramebuffer(GL_FRAMEBUFFER, ctx->current_fbo);
          }
        rsc->direct.rendered = 0;
     }

   ctx->current_sfc = sfc;
   rsc->current_ctx = ctx;
   rsc->current_eng = eng_data;

   _surface_context_list_print();

   return 1;
}

const char *
evgl_string_query(int name)
{
   switch(name)
     {
      case EVAS_GL_EXTENSIONS:
         return (void*)evgl_api_ext_string_get();
      default:
         return "";
     };
}

void *
evgl_proc_address_get(const char *name EINA_UNUSED)
{
   // Will eventually deprecate this function
   return NULL;
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

   ns->type = EVAS_NATIVE_SURFACE_OPENGL;
   ns->version = EVAS_NATIVE_SURFACE_VERSION;
   ns->data.opengl.texture_id = sfc->color_buf;
   ns->data.opengl.framebuffer_id = sfc->color_buf;
   ns->data.opengl.x = 0;
   ns->data.opengl.y = 0;
   ns->data.opengl.w = sfc->w;
   ns->data.opengl.h = sfc->h;

   if (sfc->direct_fb_opt)
      ns->data.opengl.framebuffer_id = 0;

   return 1;
}

int
evgl_direct_rendered()
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get())) return 0;

   return rsc->direct.rendered;
}



void
evgl_direct_info_set(int win_w, int win_h, int rot, int img_x, int img_y, int img_w, int img_h, int clip_x, int clip_y, int clip_w, int clip_h)
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get())) return;

   // Normally direct rendering isn't allowed if alpha is on and
   // rotation is not 0.  BUT, if override is on, allow it.
   if ( (rot==0) ||
        ((rot!=0) && (evgl_engine->direct_override)) )
     {
        rsc->direct.enabled = EINA_TRUE;

        rsc->direct.win_w  = win_w;
        rsc->direct.win_h  = win_h;
        rsc->direct.rot    = rot;

        rsc->direct.img.x  = img_x;
        rsc->direct.img.y  = img_y;
        rsc->direct.img.w  = img_w;
        rsc->direct.img.h  = img_h;

        rsc->direct.clip.x = clip_x;
        rsc->direct.clip.y = clip_y;
        rsc->direct.clip.w = clip_w;
        rsc->direct.clip.h = clip_h;
     }
   else
     {
        rsc->direct.enabled = EINA_FALSE;
     }
}

void
evgl_direct_info_clear()
{
   EVGL_Resource *rsc;

   if (!(rsc=_evgl_tls_resource_get())) return;

   rsc->direct.enabled = EINA_FALSE;
}

Evas_GL_API *
evgl_api_get()
{
   _evgl_api_get(&gl_funcs, evgl_engine->api_debug_mode);

   return &gl_funcs;
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
evgl_direct_override_get(int *override, int *force_off)
{
   *override  = evgl_engine->direct_override;
   *force_off = evgl_engine->direct_force_off;
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
   ctx = _evgl_current_context_get();

   if (!ctx) return;

   if (ctx->partial_render)
     {
        evas_gl_common_tiling_done(NULL);
        ctx->partial_render = 0;
     }
}
//-----------------------------------------------------//


