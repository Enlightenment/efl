#ifndef _EFL_PROMISE2_PRIV_H_
#define _EFL_PROMISE2_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Eina.h>
#include "efl_promise2.h"

extern const Eina_Value_Type *EINA_VALUE_TYPE_PROMISE2;
extern const Eina_Value_Type *EINA_VALUE_TYPE_FUTURE2;

Efl_Promise2 *eina_value_promise2_steal(Eina_Value *value);

#ifdef __cplusplus
}
#endif
#endif
