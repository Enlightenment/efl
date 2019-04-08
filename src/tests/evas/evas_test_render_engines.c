#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Evas.h>
#include <Evas_Engine_Buffer.h>

#include "evas_suite.h"
#include "evas_tests_helpers.h"

static Eina_Bool
_find_list(const Eina_List *lst, const char *item)
{
   const Eina_List *n;
   const char *s;
   EINA_LIST_FOREACH(lst, n, s)
     {
        if (strcmp(s, item) == 0)
          return EINA_TRUE;
     }
   return EINA_FALSE;
}

static const char *built_engines[] = {
  "software_generic",
  "buffer",
#ifdef BUILD_ENGINE_FB
  "fb",
#endif
#ifdef BUILD_ENGINE_GL_X11
  "gl_x11",
#endif
#ifdef BUILD_ENGINE_SOFTWARE_DDRAW
  "software_ddraw",
#endif
#ifdef BUILD_ENGINE_SOFTWARE_GDI
  "software_gdi",
#endif
#ifdef BUILD_ENGINE_SOFTWARE_X11
  "software_x11",
#endif
#ifdef BUILD_ENGINE_WAYLAND_EGL
  "wayland_egl",
#endif
#ifdef BUILD_ENGINE_WAYLAND_SHM
  "wayland_shm",
#endif
  NULL
};

EFL_START_TEST(evas_render_engines)
{
   Eina_List *lst = evas_render_method_list();
   const char **itr;

   for (itr = built_engines; *itr != NULL; itr++)
     {
        Eina_Bool found = _find_list(lst, *itr);
        fail_if(!found, "module should be built, but was not found: %s", *itr);
     }

   evas_render_method_list_free(lst);
}
EFL_END_TEST

EFL_START_TEST(evas_render_lookup)
{
   const char **itr;

   for (itr = built_engines; *itr != NULL; itr++)
     {
        int id = evas_render_method_lookup(*itr);
        fail_if(id == 0, "could not load engine: %s", *itr);
     }
}
EFL_END_TEST

static unsigned int counter;

static void
render_post(void *data EINA_UNUSED, Evas *e EINA_UNUSED, void *event_info EINA_UNUSED)
{
   counter++;
}

static void
render_flush_post(void *data EINA_UNUSED, Evas *e, void *event_info EINA_UNUSED)
{
   counter++;
   evas_event_callback_add(e, EVAS_CALLBACK_RENDER_POST, render_post, NULL);
}

static void
render_flush_pre(void *data EINA_UNUSED, Evas *e, void *event_info EINA_UNUSED)
{
   counter++;
   evas_event_callback_add(e, EVAS_CALLBACK_RENDER_FLUSH_POST, render_flush_post, NULL);
}

static void
render_pre(void *data EINA_UNUSED, Evas *e, void *event_info EINA_UNUSED)
{
   counter++;
   evas_event_callback_add(e, EVAS_CALLBACK_RENDER_FLUSH_PRE, render_flush_pre, NULL);
}

EFL_START_TEST(evas_render_callbacks)
{
   Evas_Object *rect;
   Evas *evas = EVAS_TEST_INIT_EVAS();
   Evas_Engine_Info_Buffer *einfo;

   einfo = (Evas_Engine_Info_Buffer *)evas_engine_info_get(evas);
   /* 500 x 500 */
   einfo->info.dest_buffer_row_bytes = 500 * sizeof(int);
   einfo->info.dest_buffer = malloc(einfo->info.dest_buffer_row_bytes * 500);
   ck_assert(evas_engine_info_set(evas, (Evas_Engine_Info *)einfo));

   rect = evas_object_rectangle_add(evas);
   evas_object_color_set(rect, 255, 0, 0, 255);
   evas_object_resize(rect, 500, 500);
   evas_object_show(rect);

   evas_event_callback_add(evas, EVAS_CALLBACK_RENDER_PRE, render_pre, NULL);
   evas_render(evas);
   ck_assert_int_eq(counter, 4);
   evas_free(evas);
}
EFL_END_TEST

void evas_test_render_engines(TCase *tc)
{
   tcase_add_test(tc, evas_render_engines);
   tcase_add_test(tc, evas_render_lookup);
   tcase_add_test(tc, evas_render_callbacks);
}
