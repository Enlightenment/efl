
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eina.h>
#include <Eo.h>

#include <check.h>

struct Generated_Promise_Data {};
typedef struct Generated_Promise_Data Generated_Promise_Data;

static void _generated_promise_method_progress_type(Eo *obj EINA_UNUSED, Generated_Promise_Data *pd EINA_UNUSED,
                                                    Eina_Promise_Owner *promise1 EINA_UNUSED)
{
}

static Eina_Promise * _generated_promise_prop1_get(Eo *obj EINA_UNUSED, Generated_Promise_Data *pd EINA_UNUSED)
{
   return NULL;
}

static void _generated_promise_prop2_get(Eo *obj EINA_UNUSED, Generated_Promise_Data *pd EINA_UNUSED, int *i EINA_UNUSED,
                                         Eina_Promise **promise1 EINA_UNUSED)
{
}

static void _generated_promise_prop3_set(Eo *obj EINA_UNUSED, Generated_Promise_Data *pd EINA_UNUSED,
                                         Eina_Promise *promise1 EINA_UNUSED)
{
}

static void _generated_promise_method1(Eo *obj EINA_UNUSED, Generated_Promise_Data *pd EINA_UNUSED,
                                       Eina_Promise_Owner *promise1 EINA_UNUSED)
{
}

static void _generated_promise_method2(Eo *obj EINA_UNUSED, Generated_Promise_Data *pd EINA_UNUSED,
                                       Eina_Promise **promise1 EINA_UNUSED)
{
}

static void _generated_promise_method3(Eo *obj EINA_UNUSED, Generated_Promise_Data *pd EINA_UNUSED,
                                       Eina_Promise *promise1 EINA_UNUSED)
{
   ck_assert(promise1 != NULL);
}

static void _generated_promise_method_multiple_args_1(Eo *obj EINA_UNUSED, Generated_Promise_Data *pd EINA_UNUSED,
                                                      Eina_Promise_Owner *promise1 EINA_UNUSED, void *data EINA_UNUSED)
{
}

static void _generated_promise_method_multiple_args_2(Eo *obj EINA_UNUSED, Generated_Promise_Data *pd EINA_UNUSED,
                                                      void *data EINA_UNUSED,  Eina_Promise_Owner *promise1 EINA_UNUSED)
{
}

#include "generated_promise.eo.h"
#include "generated_promise.eo.c"
