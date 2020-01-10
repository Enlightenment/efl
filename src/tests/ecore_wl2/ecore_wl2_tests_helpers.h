#ifndef ECORE_WL2_TEST_HELPERS_H
# define ECORE_WL2_TEST_HELPERS_H

# include <Ecore_Wl2.h>

static Ecore_Wl2_Display *
_display_connect(void)
{
   return ecore_wl2_display_connect(NULL);
}

#endif
