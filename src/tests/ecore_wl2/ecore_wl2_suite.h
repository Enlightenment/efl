#ifndef _ECORE_WL2_SUITE_H
# define _ECORE_WL2_SUITE_H

# include <check.h>
# include "../efl_check.h"

void ecore_wl2_test_init(TCase *tc);
void ecore_wl2_test_display(TCase *tc);
void ecore_wl2_test_window(TCase *tc);
void ecore_wl2_test_input(TCase *tc);

#endif
