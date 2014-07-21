#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"

#define ELM_INTERFACE_ATSPI_COMPONENT_PROTECTED
#include "elm_interface_atspi_component.eo.h"


START_TEST (elm_atspi_role_get)
{
   Evas_Object *win;
   Elm_Atspi_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "win", ELM_WIN_BASIC);

   eo_do(win, role = elm_interface_atspi_accessible_role_get());

   ck_assert(role == ELM_ATSPI_ROLE_WINDOW);

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_component_position)
{
   Eina_Bool ret;
   int x, y;

   elm_init(0, NULL);

   Eo *win = elm_win_add(NULL, "win", ELM_WIN_BASIC);

   eo_do(win, ret = elm_interface_atspi_component_position_set(EINA_TRUE, 45, 45));
   ck_assert(ret == EINA_TRUE);

   Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(win));
   ck_assert(ee != NULL);
   ecore_evas_geometry_get(ee, &x, &y, NULL, NULL);

   ck_assert((x == 45) && (y == 45));

   elm_shutdown();
}
END_TEST

START_TEST (elm_atspi_component_size)
{
   Eina_Bool ret;
   int w, h;

   elm_init(0, NULL);

   Eo *win = elm_win_add(NULL, "win", ELM_WIN_BASIC);
   evas_object_resize(win, 50, 50);

   eo_do(win, ret = elm_interface_atspi_component_size_set(100, 100));
   ck_assert(ret == EINA_TRUE);

   evas_object_geometry_get(win, NULL, NULL, &w, &h);
   ck_assert((w == 100) && (h == 100));

   elm_shutdown();
}
END_TEST

void elm_test_win(TCase *tc)
{
   tcase_add_test(tc, elm_atspi_role_get);
   tcase_add_test(tc, elm_atspi_component_position);
   tcase_add_test(tc, elm_atspi_component_size);
}
