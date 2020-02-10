#ifndef _ECORE_WL2_SUITE_H
# define _ECORE_WL2_SUITE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <check.h>
#include "../efl_check.h"
#include <stdio.h>
#include <unistd.h>
#include <Ecore.h>
#include <Ecore_Wl2.h>

void ecore_wl2_test_init(TCase *tc);
void ecore_wl2_test_display(TCase *tc);
void ecore_wl2_test_window(TCase *tc);
void ecore_wl2_test_input(TCase *tc);
void ecore_wl2_test_output(TCase *tc);

#endif
