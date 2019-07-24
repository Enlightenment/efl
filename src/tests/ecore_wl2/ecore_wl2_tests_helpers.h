#ifndef ECORE_WL2_TEST_HELPERS_H
# define ECORE_WL2_TEST_HELPERS_H

# include <Ecore_Wl2.h>

static Ecore_Wl2_Display *
_display_setup(void)
{
   Ecore_Wl2_Display *disp;

   disp = ecore_wl2_display_create(NULL);
   return disp;
}

static Ecore_Wl2_Display *
_display_connect(void)
{
   Ecore_Wl2_Display *disp;

   disp = ecore_wl2_display_connect(NULL);
   return disp;
}

#endif
