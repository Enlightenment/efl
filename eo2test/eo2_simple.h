#ifndef _EO2_SIMPLE_H
#define _EO2_SIMPLE_H

#include "Eo.h"

EAPI void inc2(eo2_a);

EAPI int get2(eo2_a);

EAPI void set2(eo2_a, int x);

// hack as it's needed by EO_FUNC_BODY
// and does not belong to Eo_Class_Description for now
/* extern Eo2_Op_Description op_descs[]; */
/* static inline EO_FUNC_BODY(inst_func_get, int, EO_FUNC_CALL(), -1, op_descs) */

EAPI const Eo_Class *eo2_simple_class_get(void);
#define EO2_SIMPLE_CLASS eo2_simple_class_get()

#endif
