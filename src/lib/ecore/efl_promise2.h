#ifndef _EFL_PROMISE2_H_
#define _EFL_PROMISE2_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <Eina.h>

typedef struct _Efl_Promise2 Efl_Promise2;
typedef struct _Efl_Future2 Efl_Future2;
typedef Eina_Value *(*Efl_Future2_Cb)(void *data, const Eina_Value *value);

EAPI Efl_Promise2 *efl_promise2_new(Eina_Free_Cb cancel_cb, const void *data);
EAPI Eina_Bool efl_promise2_resolve(Efl_Promise2 *p, Eina_Value *value);
EAPI Eina_Bool efl_promise2_reject(Efl_Promise2 *p, Eina_Value *value);
EAPI void efl_promise2_cancel(Efl_Promise2 *p);
EAPI Eina_Value *efl_promise2_as_value(Efl_Promise2 *p);
EAPI Eina_Value *efl_future2_as_value(Efl_Future2 *f);
EAPI Efl_Future2 *efl_future2_new(Efl_Promise2 *p,
                                  Efl_Future2_Cb success,
                                  Efl_Future2_Cb error,
                                  Eina_Free_Cb free_cb,
                                  const void *data);
EAPI Efl_Future2 *efl_future2_then(Efl_Future2 *prev, Efl_Future2_Cb success,
                                   Efl_Future2_Cb error, Eina_Free_Cb free_cb,
                                   const void *data);

#ifdef __cplusplus
}
#endif
#endif
