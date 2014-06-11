#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Elementary.h>
#include "elm_suite.h"

#define ELM_INTERFACE_ATSPI_ACCESSIBLE_PROTECTED
#include "elm_interface_atspi_accessible.h"
#include "elm_interface_atspi_accessible.eo.h"


START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *hoversel;
   Elm_Atspi_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "hoversel", ELM_WIN_BASIC);

   hoversel = elm_hoversel_add(win);
   eo_do(hoversel, role = elm_interface_atspi_accessible_role_get());

   ck_assert(role == ELM_ATSPI_ROLE_GLASS_PANE);

   elm_shutdown();
}
END_TEST

void elm_test_hoversel(TCase *tc)
{
 tcase_add_test(tc, elm_atspi_role_get);
}
