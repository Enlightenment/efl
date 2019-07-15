#ifndef ECORE_WL2_TEST_HELPERS_H
# define ECORE_WL2_TEST_HELPERS_H

# include <Ecore_Wl2.h>

# define ECORE_WL2_TEST_DISPLAY_SETUP() _setup_display()

static Ecore_Wl2_Display *
_setup_display(void)
{
   Ecore_Wl2_Display *disp;

   disp = ecore_wl2_display_create(NULL);
   return disp;
}

#endif
