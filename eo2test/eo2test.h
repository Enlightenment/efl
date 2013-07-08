#ifndef _NEOTEST_H
#define _NEOTEST_H

#include "Eo.h"

extern EAPI Eo_Op TEST_BASE_ID;
#define TEST_ID(func_name) (TEST_BASE_ID + (TEST_SUB_ID_ ## func_name))

enum {
     TEST_SUB_ID_inst_func,
     TEST_SUB_ID_LAST
};

static inline EO_FUNC(inst_func, int, TEST_ID, func(objid, a), 0, int a)

EAPI const Eo_Class *test_class_get(void);
#define TEST_CLASS test_class_get()

#endif
