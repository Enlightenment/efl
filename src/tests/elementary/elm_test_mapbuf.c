#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_OBJECT_BETA
#include <Elementary.h>
#include "elm_suite.h"

EFL_START_TEST (elm_mapbuf_legacy_type_check)
{
   Evas_Object *win, *mapbuf;
   const char *type;

   win = win_add(NULL, "mapbuf", ELM_WIN_BASIC);

   mapbuf = elm_mapbuf_add(win);

   type = elm_object_widget_type_get(mapbuf);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "Elm_Mapbuf"));

   type = evas_object_type_get(mapbuf);
   ck_assert(type != NULL);
   ck_assert(!strcmp(type, "elm_mapbuf"));

}
EFL_END_TEST

EFL_START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *mapbuf;
   Efl_Access_Role role;

   win = win_add(NULL, "mapbuf", ELM_WIN_BASIC);

   mapbuf = elm_mapbuf_add(win);
   role = efl_access_object_role_get(mapbuf);

   ck_assert(role == EFL_ACCESS_ROLE_IMAGE_MAP);

}
EFL_END_TEST

void elm_test_mapbuf(TCase *tc)
{
   tcase_add_test(tc, elm_mapbuf_legacy_type_check);
   tcase_add_test(tc, elm_atspi_role_get);
}
