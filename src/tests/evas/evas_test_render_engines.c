#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>

#include <Evas.h>

#include "evas_suite.h"

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

START_TEST(evas_render_engines)
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
END_TEST

START_TEST(evas_render_lookup)
{
   const char **itr;

   for (itr = built_engines; *itr != NULL; itr++)
     {
        int id = evas_render_method_lookup(*itr);
        fail_if(id == 0, "could not load engine: %s", *itr);
     }
}
END_TEST

void evas_test_render_engines(TCase *tc)
{
   tcase_add_test(tc, evas_render_engines);
   tcase_add_test(tc, evas_render_lookup);
}
