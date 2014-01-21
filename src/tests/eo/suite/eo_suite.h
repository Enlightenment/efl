#ifndef _EO_SUITE_H
#define _EO_SUITE_H

#include <check.h>

void eo_test_init(TCase *tc);
void eo_test_general(TCase *tc);
void eo_test_class_errors(TCase *tc);
void eo_test_call_errors(TCase *tc);
void eo_test_value(TCase *tc);
void eo_test_threaded_calls(TCase *tc);

#endif /* _EO_SUITE_H */
