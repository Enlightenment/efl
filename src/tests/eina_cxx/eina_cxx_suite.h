#ifndef _EINA_CXX_SUITE_H
#define _EINA_CXX_SUITE_H

#ifdef __cplusplus
#include <cassert>
#include <algorithm>

#include <check.h>
#include "../efl_check.h"
void eina_test_inlist(TCase* tc);
void eina_test_inarray(TCase* tc);
void eina_test_ptrlist(TCase* tc);
void eina_test_ptrarray(TCase* tc);
void eina_test_iterator(TCase* tc);
void eina_test_stringshare(TCase* tc);
void eina_test_error(TCase* tc);
void eina_test_accessor(TCase* tc);
void eina_test_thread(TCase* tc);
void eina_test_optional(TCase* tc);
void eina_test_value(TCase* tc);
void eina_test_log(TCase* tc);
#endif
#define EINA_CXX_TEST_API
#define EINA_CXX_TEST_API_WEAK

#endif /* _EINA_CXX_SUITE_H */
