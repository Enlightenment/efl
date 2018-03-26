#ifndef _EOLIAN_CXX_SUITE_H
#define _EOLIAN_CXX_SUITE_H

#include <check.h>
#include "../efl_check.h"
void eolian_cxx_test_parse(TCase* tc);
void eolian_cxx_test_wrapper(TCase* tc);
void eolian_cxx_test_generate(TCase* tc);
void eolian_cxx_test_address_of(TCase* tc);
void eolian_cxx_test_inheritance(TCase* tc);
void eolian_cxx_test_binding(TCase* tc);
void eolian_cxx_test_cyclic(TCase* tc);
void eolian_cxx_test_documentation(TCase* tc);

#endif /* _EOLIAN_CXX_SUITE_H */
