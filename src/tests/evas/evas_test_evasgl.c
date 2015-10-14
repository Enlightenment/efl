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
#include <dlfcn.h>

#define EFL_GFX_FILTER_BETA
#include "evas_suite.h"
#include "Evas.h"
#include "Evas_GL.h"
#include "Ecore_Evas.h"

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
   evas_init(); \
   ecore_evas_init(); \
   ee = ecore_evas_new(engine, 0, 0, 1, 1, options); \
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
   ecore_evas_shutdown(); \
   evas_shutdown(); \
   do {} while (0)

static void
_test_evasgl_init(const char *engine, const char *options)
{
   START_EVASGL_TEST(engine, options);
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
   fail_if(!evas_gl_make_current(evgl, NULL, ctx));

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
_test_evasgl_current_get(const char *engine, const char *options)
{
   if (!_detect_osmesa())
     return;

   START_EVASGL_TEST(engine, options);
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

   fail_if(!evas_gl_make_current(evgl, NULL, NULL));
   evgl2 = evas_gl_current_evas_gl_get(&ctx2, &sfc2);
   fail_if(evgl2 != evgl); /* this should not reset current Evas GL */
   fail_if(sfc2 != NULL);
   fail_if(ctx2 != NULL);

   evas_gl_context_destroy(evgl, ctx);
   evas_gl_surface_destroy(evgl, sfc);
   evas_gl_free(evgl);
   END_EVASGL_TEST();
}

static void
_test_evasgl_context_version(const char *engine, const char *options)
{
   if (!_detect_osmesa())
     return;

   START_EVASGL_TEST(engine, options);
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

START_TEST(evas_test_evasgl_init_opengl_x11)
{
   _test_evasgl_init("opengl_x11", NULL);
}
END_TEST

START_TEST(evas_test_evasgl_current_get_opengl_x11)
{
   _test_evasgl_current_get("opengl_x11", NULL);
}
END_TEST

START_TEST(evas_test_evasgl_context_version_opengl_x11)
{
   _test_evasgl_context_version("opengl_x11", NULL);
}
END_TEST

START_TEST(evas_test_evasgl_init_buffer)
{
   _test_evasgl_init("buffer", NULL);
}
END_TEST

START_TEST(evas_test_evasgl_current_get_buffer)
{
   _test_evasgl_current_get("buffer", NULL);
}
END_TEST

START_TEST(evas_test_evasgl_context_version_buffer)
{
   _test_evasgl_context_version("buffer", NULL);
}
END_TEST

void evas_test_evasgl(TCase *tc)
{
   if (getenv("DISPLAY"))
     {
        tcase_add_test(tc, evas_test_evasgl_init_opengl_x11);
        tcase_add_test(tc, evas_test_evasgl_current_get_opengl_x11);
        tcase_add_test(tc, evas_test_evasgl_context_version_opengl_x11);
     }

   tcase_add_test(tc, evas_test_evasgl_init_buffer);
   tcase_add_test(tc, evas_test_evasgl_current_get_buffer);
   tcase_add_test(tc, evas_test_evasgl_context_version_buffer);
}
