#ifndef _NEOTEST_H
#define _NEOTEST_H

#include "Eo.h"

// hack as it's needed by EO_FUNC_BODY
// and does not belong to Eo_Class_Description for now
extern Eo2_Op_Description op_descs[];

EAPI int inst_func_set(eo2_a, int a, int b);

static inline EO_FUNC_BODY(inst_func_get, int, EO_FUNC_CALL(), -1)

EAPI const Eo_Class *test_class_get(void);
#define TEST_CLASS test_class_get()

#endif
