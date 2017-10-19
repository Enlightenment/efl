#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_ACCESS_PROTECTED
#include <Elementary.h>
#include "elm_suite.h"


START_TEST (elm_atspi_role_get)
{
   Evas_Object *win, *slideshow;
   Efl_Access_Role role;

   elm_init(1, NULL);
   win = elm_win_add(NULL, "slideshow", ELM_WIN_BASIC);

   slideshow = elm_slideshow_add(win);
   role = efl_access_role_get(slideshow);

   ck_assert(role == EFL_ACCESS_ROLE_DOCUMENT_PRESENTATION);

   elm_shutdown();
}
END_TEST

void elm_test_slideshow(TCase *tc)
{
 tcase_add_test(tc, elm_atspi_role_get);
}
