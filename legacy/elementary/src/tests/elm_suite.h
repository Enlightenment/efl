#ifndef _ELM_SUITE_H
#define _ELM_SUITE_H

#include <check.h>

void elm_test_init(TCase *tc);
void elm_test_check(TCase *tc);
void elm_test_colorselector(TCase *tc);
void elm_test_entry(TCase *tc);
void elm_test_atspi(TCase *tc);

#endif /* _ELM_SUITE_H */
