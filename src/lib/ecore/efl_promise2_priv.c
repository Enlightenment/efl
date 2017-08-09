#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "efl_promise2_priv.h"

static Eina_Bool
_promise_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   Efl_Promise2 **tmem = mem;
   *tmem = NULL;
   return EINA_TRUE;
}

static Eina_Bool
_promise_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   Efl_Promise2 **tmem = mem;
   *tmem = NULL;
   return EINA_TRUE;
}

static Eina_Bool
_promise_copy(const Eina_Value_Type *type EINA_UNUSED,
              const void *src, void *dst)
{
   Efl_Promise2 * const *psrc = src;
   Efl_Promise2 **pdst = dst;
   *pdst = *psrc;
   return EINA_TRUE;
}

static int
_promise_compare(const Eina_Value_Type *type EINA_UNUSED,
                 const void *a, const void *b)
{
   const Efl_Promise2 * const *p1 = a;
   const Efl_Promise2 * const *p2 = b;

   if (*p1 == *p2)
     return 0;
   if (*p1 > *p2)
     return 1;
   return -1;
}

static Eina_Bool
_promise_vset(const Eina_Value_Type *type EINA_UNUSED, void *mem, va_list args)
{
   Efl_Promise2 **dst = mem;
   Efl_Promise2 **src = va_arg(args, Efl_Promise2 **);
   *dst = *src;
   return EINA_TRUE;
}

static Eina_Bool
_promise_pset(const Eina_Value_Type *type EINA_UNUSED,
              void *mem, const void *ptr)
{
   Efl_Promise2 **dst = mem;
   Efl_Promise2 * const *src = ptr;
   *dst = *src;
   return EINA_TRUE;
}

static Eina_Bool
_promise_pget(const Eina_Value_Type *type EINA_UNUSED,
              const void *mem, void *ptr)
{
   Efl_Promise2 * const *src = mem;
   Efl_Promise2 **dst = ptr;
   *dst = *src;
   return EINA_TRUE;
}

static const Eina_Value_Type _EINA_VALUE_TYPE_PROMISE2 = {
  .version = EINA_VALUE_TYPE_VERSION,
  .value_size = sizeof(Efl_Promise2 *),
  .name = "Efl_Promise2",
  .setup = _promise_setup,
  .flush = _promise_flush,
  .copy = _promise_copy,
  .compare = _promise_compare,
  .convert_to = NULL,
  .convert_from = NULL,
  .vset = _promise_vset,
  .pset = _promise_pset,
  .pget = _promise_pget
};

static const Eina_Value_Type _EINA_VALUE_TYPE_FUTURE2 = {
  .version = EINA_VALUE_TYPE_VERSION,
  .value_size = sizeof(Efl_Future2 *),
  .name = "Efl_Future2",
  .setup = _promise_setup,
  .flush = _promise_flush,
  .copy = _promise_copy,
  .compare = _promise_compare,
  .convert_to = NULL,
  .convert_from = NULL,
  .vset = _promise_vset,
  .pset = _promise_pset,
  .pget = _promise_pget
};

const Eina_Value_Type *EINA_VALUE_TYPE_PROMISE2 = &_EINA_VALUE_TYPE_PROMISE2;
const Eina_Value_Type *EINA_VALUE_TYPE_FUTURE2 = &_EINA_VALUE_TYPE_FUTURE2;

Efl_Promise2 *
eina_value_promise2_steal(Eina_Value *value)
{
   Efl_Promise2 *p;
   eina_value_pget(value, &p);
   eina_value_flush(value);
   return p;
}
