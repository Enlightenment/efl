#ifndef _NEOTEST_H
#define _NEOTEST_H

#include "Eo.h"

extern EAPI Eo_Op TEST_BASE_ID;
#define TEST_ID(func_name) (TEST_BASE_ID + (TEST_SUB_ID_ ## func_name))

enum {
     TEST_SUB_ID_inst_func_set,
     TEST_SUB_ID_inst_func_get,
     TEST_SUB_ID_LAST
};

EAPI int inst_func_set(eo2_a, int a);

static inline EO_FUNC_BODY(inst_func_get, int, TEST_ID, EO_FUNC_CALL(), -1)

EAPI const Eo_Class *test_class_get(void);
#define TEST_CLASS test_class_get()

#endif
