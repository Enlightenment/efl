
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eina.h>
#include <Eo.h>

#include <check.h>

struct Generated_Future_Data {};
typedef struct Generated_Future_Data Generated_Future_Data;

static void _generated_future_method_progress_type(Eo *obj EINA_UNUSED, Generated_Future_Data *pd EINA_UNUSED,
                                                    Efl_Future **future1 EINA_UNUSED)
{
}

static Efl_Future * _generated_future_prop1_get(Eo *obj EINA_UNUSED, Generated_Future_Data *pd EINA_UNUSED)
{
   return NULL;
}

static void _generated_future_prop2_get(Eo *obj EINA_UNUSED, Generated_Future_Data *pd EINA_UNUSED, int *i EINA_UNUSED,
                                         Efl_Future **future1 EINA_UNUSED)
{
}

static void _generated_future_prop3_set(Eo *obj EINA_UNUSED, Generated_Future_Data *pd EINA_UNUSED,
                                         Efl_Future *future1 EINA_UNUSED)
{
}

static void _generated_future_method1(Eo *obj EINA_UNUSED, Generated_Future_Data *pd EINA_UNUSED,
                                       Efl_Future **future1 EINA_UNUSED)
{
}

static void _generated_future_method2(Eo *obj EINA_UNUSED, Generated_Future_Data *pd EINA_UNUSED,
                                       Efl_Future **future1 EINA_UNUSED)
{
}

static void _generated_future_method3(Eo *obj EINA_UNUSED, Generated_Future_Data *pd EINA_UNUSED,
                                       Efl_Future *future1 EINA_UNUSED)
{
   ck_assert(future1 != NULL);
}

static void _generated_future_method_multiple_args_1(Eo *obj EINA_UNUSED, Generated_Future_Data *pd EINA_UNUSED,
                                                     Efl_Future **future1 EINA_UNUSED, void *data EINA_UNUSED)
{
}

static void _generated_future_method_multiple_args_2(Eo *obj EINA_UNUSED, Generated_Future_Data *pd EINA_UNUSED,
                                                     void *data EINA_UNUSED, Efl_Future **future1 EINA_UNUSED)
{
}

#include "generated_future.eo.h"
#include "generated_future.eo.c"
