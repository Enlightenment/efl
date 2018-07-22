/* Test Evas GL EAPIs.
 *
 * This will try with opengl_x11 and buffer (OSMesa) and silently fail if
 * the engine or the GL library can't be initialized. This is to test Evas GL
 * APIs when they can actually work, ie. when OSMesa exists or when the engine
 * is GL.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#ifndef _WIN32
# include <dlfcn.h>
#endif

#define EFL_GFX_FILTER_BETA

#include <Evas.h>
#include <Evas_GL.h>
#include <Ecore_Evas.h>

#include "evas_suite.h"

static int
_detect_osmesa(void)
{
   /* assume that if libOSMesa.so links, then we can create an Evas GL */
   void *lib = dlopen("libOSMesa.so", RTLD_NOW);
   if (!lib)
     {
        printf("Could not find OSMesa! Skipping Evas GL tests.\n");
        return 0;
     }
   dlclose(lib);
   return 1;
}

#define START_EVASGL_TEST(engine, options) \
   Ecore_Evas *ee; Evas *evas; Evas_Object *im = NULL; \
   if (!strcmp(engine, "buffer") && !_detect_osmesa()) return; \
   putenv("EVAS_GL_API_DEBUG=1"); \
   if (!options || !strcmp(engine, "buffer")) ee = ecore_evas_new(engine, 0, 0, 1, 1, NULL); \
   else ee = ecore_evas_gl_x11_options_new(NULL, 0, 0, 0, 1, 1, options); \
   if (!ee) { printf("Could not create ecore evas. Skipping Evas GL tests.\n"); \
              goto init_failed; } \
   ecore_evas_show(ee); \
   ecore_evas_manual_render_set(ee, EINA_TRUE); \
   evas = ecore_evas_get(ee); \
   im = evas_object_image_filled_add(evas); \
   evas_object_geometry_set(im, 0, 0, 1, 1); \
   evas_object_show(im); \
   ecore_evas_manual_render(ee); \
   do {} while (0)

#define END_EVASGL_TEST() \
   init_failed: \
   evas_object_del(im); \
   ecore_evas_free(ee); \
   do {} while (0)

static void
_test_evasgl_init(const char *engine)
{
   START_EVASGL_TEST(engine, NULL);
   Evas_GL_Context *ctx;
   Evas_GL_Surface *sfc;
   Evas_GL_Config *cfg;
   Evas_GL_API *gl;
   Evas_GL *evgl;

   fail_if(!(evgl = evas_gl_new(evas)));
   fail_if(!(cfg = evas_gl_config_new()));
   fail_if(!(sfc = evas_gl_surface_create(evgl, cfg, 1, 1)));
   fail_if(!(ctx = evas_gl_context_create(evgl, NULL)));

   /* valid current states */
   fail_if(!evas_gl_make_current(evgl, sfc, ctx));
   fail_if(!evas_gl_make_current(evgl, NULL, NULL));

   /* no context but surface: invalid */
   fprintf(stderr, "================ IGNORE ERRORS BEGIN ================\n");
   fail_if(evas_gl_make_current(evgl, sfc, NULL) != EINA_FALSE);
   fprintf(stderr, "================  IGNORE ERRORS END  ================\n");

   /* API verification */
   fail_if(!(gl = evas_gl_api_get(evgl)));

   fail_if(!evas_gl_make_current(evgl, NULL, NULL));
   evas_gl_context_destroy(evgl, ctx);
   evas_gl_surface_destroy(evgl, sfc);
   evas_gl_config_free(cfg);
   evas_gl_free(evgl);

   END_EVASGL_TEST();
}

static void
_test_evasgl_current_get(const char *engine)
{
   START_EVASGL_TEST(engine, NULL);
   Evas_GL_Context *ctx, *ctx2;
   Evas_GL_Surface *sfc, *sfc2;
   Evas_GL_Config *cfg;
   Evas_GL *evgl, *evgl2;

   ctx2 = (void*)(intptr_t)0x1;
   sfc2 = (void*)(intptr_t)0x1;
   evgl2 = evas_gl_current_evas_gl_get(&ctx2, &sfc2);
   fail_if(evgl2 || ctx2 || sfc2);

   fail_if(!(evgl = evas_gl_new(evas)));
   fail_if(!(cfg = evas_gl_config_new()));
   fail_if(!(sfc = evas_gl_surface_create(evgl, cfg, 1, 1)));
   fail_if(!(ctx = evas_gl_context_create(evgl, NULL)));

   ctx2 = (void*)(intptr_t)0x1;
   sfc2 = (void*)(intptr_t)0x1;
   evgl2 = evas_gl_current_evas_gl_get(&ctx2, &sfc2);
   fail_if(evgl2 || ctx2 || sfc2);

   fail_if(!evas_gl_make_current(evgl, sfc, ctx));
   evgl2 = evas_gl_current_evas_gl_get(&ctx2, &sfc2);
   fail_if(evgl2 != evgl);
   fail_if(sfc2 != sfc);
   fail_if(ctx2 != ctx);
   fail_if(evas_gl_current_surface_get(evgl) != sfc);
   fail_if(evas_gl_current_context_get(evgl) != ctx);

   fail_if(!evas_gl_make_current(evgl, NULL, NULL));
   evgl2 = evas_gl_current_evas_gl_get(&ctx2, &sfc2);
   fail_if(evgl2 != evgl); /* this should not reset current Evas GL */
   fail_if(sfc2 != NULL);
   fail_if(ctx2 != NULL);
   fail_if(evas_gl_current_surface_get(evgl));
   fail_if(evas_gl_current_context_get(evgl));

   evas_gl_context_destroy(evgl, ctx);
   evas_gl_surface_destroy(evgl, sfc);
   evas_gl_free(evgl);
   END_EVASGL_TEST();
}

static void
_test_evasgl_context_version(const char *engine)
{
   START_EVASGL_TEST(engine, NULL);
   Evas_GL_Context *ctx, *ctx2;
   Evas_GL_Surface *sfc;
   Evas_GL_Config *cfg;
   Evas_GL_API *gl;
   Evas_GL *evgl;
   const char *ver, *ext, *glsl;
   int vmaj, vmin;

   fail_if(!(evgl = evas_gl_new(evas)));
   fail_if(!(cfg = evas_gl_config_new()));
   fail_if(!(sfc = evas_gl_surface_create(evgl, cfg, 1, 1)));

   /* test only GLES 2 - GLES 3 and GLES 1 not supported yet with OSMesa */
   fail_if(!(ctx = evas_gl_context_version_create(evgl, NULL, EVAS_GL_GLES_2_X)));
   fail_if(!(gl = evas_gl_api_get(evgl)));
   fail_if(gl->version != EVAS_GL_API_VERSION); /* valid for make check */

   fail_if(!evas_gl_make_current(evgl, sfc, ctx));
   fail_if(!(ver = (const char *) gl->glGetString(GL_VERSION)));
   printf("GL_VERSION: %s\n", ver);
   fail_if(sscanf(ver, "OpenGL ES %d.%d", &vmaj, &vmin) != 2);
   fail_if((vmaj < 2) || (vmin < 0));

   /* verify function pointers - GLES 2 only */
   typedef void (*fptr_t)();
   const int idx_glViewport = ((char*) &gl->glViewport - (char*) &gl->glActiveTexture) / sizeof(fptr_t);
   fptr_t *fptr= &gl->glActiveTexture;
   for (int i = 0; i < idx_glViewport; i++, fptr++)
     fail_if(!*fptr);

   /* mesa supports a ton of extensions but there are no extra pointers set in
    * Evas_GL_API for those extensions */
   fail_if(!(ext = (const char *) gl->glGetString(GL_EXTENSIONS)));
   printf("GL_EXTENSIONS: %s\n", ext);

   /* GLSL string format check */
   fail_if(!(glsl = (const char *) gl->glGetString(GL_SHADING_LANGUAGE_VERSION)));
   printf("GL_SHADING_LANGUAGE_VERSION: %s\n", glsl);
   fail_if(sscanf(glsl, "OpenGL ES GLSL ES %d", &vmaj) != 1);

   /* shared ctx */
   fail_if(!(ctx2 = evas_gl_context_version_create(evgl, ctx, EVAS_GL_GLES_2_X)));
   evas_gl_context_destroy(evgl, ctx2);

   fail_if(!evas_gl_make_current(evgl, NULL, NULL));
   evas_gl_context_destroy(evgl, ctx);
   evas_gl_surface_destroy(evgl, sfc);
   evas_gl_free(evgl);
   END_EVASGL_TEST();
}

static void
_test_evasgl_surfaceless_context(const char *engine)
{
   START_EVASGL_TEST(engine, NULL);
   Evas_GL_Context *ctx;
   Evas_GL_Surface *sfc;
   Evas_GL_Config *cfg;
   Evas_GL_API *gl;
   Evas_GL *evgl;
   const char *eexts;

   fail_if(!(evgl = evas_gl_new(evas)));
   fail_if(!(ctx = evas_gl_context_create(evgl, NULL)));

   // FIXME: evas_gl_string_query will fail before the first make_current (GL)
   fail_if(!(cfg = evas_gl_config_new()));
   fail_if(!(sfc = evas_gl_surface_create(evgl, cfg, 1, 1)));
   fail_if(!evas_gl_make_current(evgl, sfc, ctx));
   // FIXME

   fail_if(!(gl = evas_gl_context_api_get(evgl, ctx)));

   eexts = evas_gl_string_query(evgl, EVAS_GL_EXTENSIONS);
   if (eexts && strstr(eexts, "EGL_KHR_surfaceless_context"))
     {
        fail_if(!evas_gl_make_current(evgl, NULL, ctx));
        fail_if(!gl->glGetString(GL_VERSION));
     }
   else printf("Surfaceless context not supported. Skipped.\n");

   fail_if(!evas_gl_make_current(evgl, NULL, NULL));
   evas_gl_context_destroy(evgl, ctx);
   evas_gl_surface_destroy(evgl, sfc);
   evas_gl_config_free(cfg);
   evas_gl_free(evgl);
   END_EVASGL_TEST();
}

static void
_test_evasgl_glreadpixels(const char *engine)
{
   /* simple test verifying surface render works as expected */

   START_EVASGL_TEST(engine, NULL);
   Evas_GL_Context *ctx;
   Evas_GL_Surface *sfc;
   Evas_GL_Config *cfg;
   Evas_GL_API *gl;
   Evas_GL *evgl;
   unsigned int pixel;

   fail_if(!(evgl = evas_gl_new(evas)));
   fail_if(!(cfg = evas_gl_config_new()));
   fail_if(!(ctx = evas_gl_context_create(evgl, NULL)));
   fail_if(!(sfc = evas_gl_surface_create(evgl, cfg, 1, 1)));
   fail_if(!evas_gl_make_current(evgl, sfc, ctx));
   fail_if(!(gl = evas_gl_context_api_get(evgl, ctx)));

   gl->glClearColor(0, 1, 0, 1);
   gl->glClear(GL_COLOR_BUFFER_BIT);
   gl->glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
   fail_if(gl->glGetError() != GL_NO_ERROR);
   fail_if(pixel != 0xFF00FF00);

   fail_if(!evas_gl_make_current(evgl, NULL, NULL));
   evas_gl_context_destroy(evgl, ctx);
   evas_gl_surface_destroy(evgl, sfc);
   evas_gl_config_free(cfg);
   evas_gl_free(evgl);
   END_EVASGL_TEST();
}

static void
_test_evasgl_fbo(const char *engine)
{
   /* simple test verifying FBO render works as expected */

   START_EVASGL_TEST(engine, NULL);
   Evas_GL_Context *ctx;
   Evas_GL_Surface *sfc;
   Evas_GL_Config *cfg;
   Evas_GL_API *gl;
   Evas_GL *evgl;
   unsigned int pixel;
   GLuint fbo, tex, vtx, frg, prg, u;
   GLint status;

   static const char *vertex =
         "#ifdef GL_ES\n"
         "precision mediump float;\n"
         "#endif\n"
         "attribute vec4 vertex;\n"
         "void main()\n"
         "{\n"
         "   gl_Position = vertex;\n"
         "}\n";
   static const char *fragment =
         "#ifdef GL_ES\n"
         "precision mediump float;\n"
         "#endif\n"
         "uniform vec4 color;\n"
         "void main()\n"
         "{\n"
         "   gl_FragColor = color;\n"
         "}\n";
   static const GLfloat color[] = { 1, 0, 0, 1 };
   static const GLfloat vertices[] = {
       1.0,  1.0,  0.0,
      -1.0, -1.0,  0.0,
       1.0, -1.0,  0.0,
       1.0,  1.0,  0.0,
      -1.0,  1.0,  0.0,
      -1.0, -1.0,  0.0
   };

   /* surface & context */
   fail_if(!(evgl = evas_gl_new(evas)));
   fail_if(!(cfg = evas_gl_config_new()));
   cfg->color_format = EVAS_GL_RGBA_8888;
   fail_if(!(ctx = evas_gl_context_create(evgl, NULL)));
   fail_if(!(sfc = evas_gl_surface_create(evgl, cfg, 1, 1)));
   fail_if(!evas_gl_make_current(evgl, sfc, ctx));
   fail_if(!(gl = evas_gl_context_api_get(evgl, ctx)));

   /* generate fbo */
   gl->glGenFramebuffers(1, &fbo);
   gl->glGenTextures(1, &tex);
   gl->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
   gl->glBindTexture(GL_TEXTURE_2D, tex);
   gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
   fail_if(gl->glGetError() != GL_NO_ERROR);

   /* prepare program */
   vtx = gl->glCreateShader(GL_VERTEX_SHADER);
   gl->glShaderSource(vtx, 1, &vertex, NULL);
   gl->glCompileShader(vtx);
   gl->glGetShaderiv(vtx, GL_COMPILE_STATUS, &status);
   fail_if((gl->glGetError() != GL_NO_ERROR) || !status);
   frg = gl->glCreateShader(GL_FRAGMENT_SHADER);
   gl->glShaderSource(frg, 1, &fragment, NULL);
   gl->glCompileShader(frg);
   gl->glGetShaderiv(frg, GL_COMPILE_STATUS, &status);
   fail_if((gl->glGetError() != GL_NO_ERROR) || !status);
   prg = gl->glCreateProgram();
   gl->glAttachShader(prg, vtx);
   gl->glAttachShader(prg, frg);
   gl->glBindAttribLocation(prg, 0, "vertex");
   gl->glLinkProgram(prg);
   gl->glGetProgramiv(prg, GL_LINK_STATUS, &status);
   fail_if((gl->glGetError() != GL_NO_ERROR) || !status);

   /* clear */
   gl->glClearColor(0, 0, 0, 0);
   gl->glClear(GL_COLOR_BUFFER_BIT);
   fail_if(gl->glGetError() != GL_NO_ERROR);

   /* set parameters */
   gl->glUseProgram(prg);
   u = gl->glGetUniformLocation(prg, "color");
   gl->glUniform4fv(u, 1, color);
   gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
   gl->glEnableVertexAttribArray(0);
   fail_if(gl->glGetError() != GL_NO_ERROR);

   /* draw */
   gl->glDrawArrays(GL_TRIANGLES, 0, 6);
   gl->glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
   fail_if(gl->glGetError() != GL_NO_ERROR);
   fail_if(pixel != 0xFF0000FF);

   fail_if(!evas_gl_make_current(evgl, NULL, NULL));
   evas_gl_context_destroy(evgl, ctx);
   evas_gl_surface_destroy(evgl, sfc);
   evas_gl_config_free(cfg);
   evas_gl_free(evgl);
   END_EVASGL_TEST();
}

static void
_test_evasgl_pbuffer(const char *engine)
{
   /* check support for PBuffer */

   // FIXME: No SW engine support!
   if (!strcmp(engine, "buffer"))
     {
        printf("PBuffer not supported (SW engine). Skipped.\n");
        return;
     }

   START_EVASGL_TEST(engine, NULL);
   Evas_GL_Context *ctx;
   Evas_GL_Surface *sfc;
   Evas_GL_Config *cfg;
   Evas_GL_API *gl;
   Evas_GL *evgl;
   unsigned int pixel;

   fail_if(!(evgl = evas_gl_new(evas)));
   fail_if(!(cfg = evas_gl_config_new()));
   cfg->color_format = EVAS_GL_RGBA_8888;
   fail_if(!(ctx = evas_gl_context_create(evgl, NULL)));
   fail_if(!(sfc = evas_gl_pbuffer_surface_create(evgl, cfg, 1, 1, NULL)));
   fail_if(!evas_gl_make_current(evgl, sfc, ctx));
   fail_if(!(gl = evas_gl_context_api_get(evgl, ctx)));

   /* clear */
   gl->glClearColor(0, 1, 0, 1);
   gl->glClear(GL_COLOR_BUFFER_BIT);
   gl->glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &pixel);
   fail_if(gl->glGetError() != GL_NO_ERROR);
   fail_if(pixel != 0xFF00FF00);

   fail_if(!evas_gl_make_current(evgl, NULL, NULL));
   evas_gl_context_destroy(evgl, ctx);
   evas_gl_surface_destroy(evgl, sfc);
   evas_gl_config_free(cfg);
   evas_gl_free(evgl);
   END_EVASGL_TEST();
}

static void
_test_evasgl_formats(const char *engine)
{
   /* check that Evas GL always falls back to working surface configs */

   const int options[] = { ECORE_EVAS_GL_X11_OPT_GL_DEPTH, 24,
                           ECORE_EVAS_GL_X11_OPT_GL_STENCIL, 8,
                           ECORE_EVAS_GL_X11_OPT_GL_MSAA, 4,
                           0 };
   const int *opts = options;

   START_EVASGL_TEST(engine, opts);
   Evas_GL_Context *ctx;
   Evas_GL_Surface *sfc;
   Evas_GL_Config *cfg;
   Evas_GL *evgl;

   fail_if(!(evgl = evas_gl_new(evas)));
   fail_if(!(cfg = evas_gl_config_new()));
   fail_if(!(ctx = evas_gl_context_create(evgl, NULL)));

   // don't test DEPTH_32 or STENCIL_16
   for (Evas_GL_Color_Format fmt = EVAS_GL_RGB_888; fmt < EVAS_GL_NO_FBO; fmt++)
     for (Evas_GL_Depth_Bits depth = EVAS_GL_DEPTH_NONE; depth <= EVAS_GL_DEPTH_BIT_24; depth++)
       for (Evas_GL_Stencil_Bits stencil = EVAS_GL_STENCIL_NONE; stencil <= EVAS_GL_STENCIL_BIT_8; stencil++)
         for (Evas_GL_Options_Bits opt = EVAS_GL_OPTIONS_NONE; opt <= 2; opt += 2)
           for (Evas_GL_Multisample_Bits msaa = EVAS_GL_MULTISAMPLE_NONE; msaa <= EVAS_GL_MULTISAMPLE_HIGH; msaa++)
             {
                cfg->color_format = fmt;
                cfg->depth_bits = depth;
                cfg->stencil_bits = stencil;
                cfg->options_bits = opt;
                cfg->multisample_bits = msaa;
                cfg->gles_version = EVAS_GL_GLES_2_X;
                /*
                fprintf(stderr, "Testing surface %d %d %d %d %d\n",
                        cfg->color_format, cfg->depth_bits, cfg->stencil_bits,
                        cfg->multisample_bits, cfg->options_bits);
                        */
                fail_if(!(sfc = evas_gl_surface_create(evgl, cfg, 1, 1)));
                fail_if(!evas_gl_make_current(evgl, sfc, ctx));
                fail_if(!evas_gl_make_current(evgl, NULL, NULL));
                evas_gl_surface_destroy(evgl, sfc);
             }

   fail_if(!evas_gl_make_current(evgl, NULL, NULL));
   evas_gl_context_destroy(evgl, ctx);
   evas_gl_config_free(cfg);
   evas_gl_free(evgl);
   END_EVASGL_TEST();
}

#define TEST_ADD_OPT(name, opt) \
   EFL_START_TEST(evas ## name ## _opengl_x11) \
   { name("opengl_x11"); } \
   EFL_END_TEST \
   EFL_START_TEST(evas ## name ## _buffer) \
   { name("buffer"); } \
   EFL_END_TEST
#define TEST_ADD(name) TEST_ADD_OPT(name, NULL)

TEST_ADD(_test_evasgl_init)
TEST_ADD(_test_evasgl_current_get)
TEST_ADD(_test_evasgl_context_version)
TEST_ADD(_test_evasgl_surfaceless_context)
TEST_ADD(_test_evasgl_glreadpixels)
TEST_ADD(_test_evasgl_fbo)
TEST_ADD(_test_evasgl_formats)
TEST_ADD(_test_evasgl_pbuffer)

/* APIs still to test:
 *
 * evas_gl_rotation_get
 * evas_gl_surface_query
 * evas_gl_error_get
 * evas_gl_native_surface_get
 * evas_gl_proc_address_get
 *
 * Most of these actually need support in SW engine to be improved.
 */

void evas_test_evasgl(TCase *tc)
{
#undef TEST_ADD
#define TEST_ADD(name) tcase_add_test(tc, evas ## name ## _opengl_x11);

   if (getenv("DISPLAY"))
     {
        TEST_ADD(_test_evasgl_init);
        TEST_ADD(_test_evasgl_current_get);
        TEST_ADD(_test_evasgl_context_version);
        TEST_ADD(_test_evasgl_surfaceless_context);
        TEST_ADD(_test_evasgl_glreadpixels);
        TEST_ADD(_test_evasgl_fbo);
        TEST_ADD(_test_evasgl_pbuffer);
        TEST_ADD(_test_evasgl_formats);
     }

#undef TEST_ADD
#define TEST_ADD(name) tcase_add_test(tc, evas ## name ## _buffer);

   TEST_ADD(_test_evasgl_init);
   TEST_ADD(_test_evasgl_current_get);
   TEST_ADD(_test_evasgl_context_version);
   TEST_ADD(_test_evasgl_surfaceless_context);
   TEST_ADD(_test_evasgl_glreadpixels);
   TEST_ADD(_test_evasgl_fbo);
   TEST_ADD(_test_evasgl_pbuffer);
   TEST_ADD(_test_evasgl_formats);
}
