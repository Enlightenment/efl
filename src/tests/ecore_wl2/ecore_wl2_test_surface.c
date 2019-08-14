#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Wl2.h>

#include "ecore_wl2_suite.h"

static Ecore_Wl2_Display *
_display_connect(void)
{
   Ecore_Wl2_Display *disp;

   disp = ecore_wl2_display_connect(NULL);
   return disp;
}

static Ecore_Wl2_Window *
_window_create(Ecore_Wl2_Display *disp)
{
   Ecore_Wl2_Window *win;

   win = ecore_wl2_window_new(disp, NULL, 100, 100, 500, 500);
   return win;
}

EFL_START_TEST(wl2_surface_create)
{
   Ecore_Wl2_Display *disp;
   Ecore_Wl2_Window *win;
   Ecore_Wl2_Surface *surf;

   disp = _display_connect();
   ck_assert(disp != NULL);

   win = _window_create(disp);
   ck_assert(win != NULL);

   surf = ecore_wl2_surface_create(win, EINA_FALSE);
   ck_assert(surf != NULL);
}
EFL_END_TEST

void
ecore_wl2_test_surface(TCase *tc)
{
   if (getenv("WAYLAND_DISPLAY"))
     {
        /* surface tests can only run if there is an existing compositor */
        tcase_add_test(tc, wl2_surface_create);
     }
}
