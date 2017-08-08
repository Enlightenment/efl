#include <Ecore.h>
#include <Eina.h>
#include <stdlib.h>

static unsigned int _count = 0;

static Eina_Bool
_timeout(void *data)
{
   Efl_Promise2 *p = data;
   Eina_Value *v = eina_value_new(EINA_VALUE_TYPE_INT);

   eina_value_set(v, ++_count);
   if (_count != 10)
     efl_promise2_resolve(p, v);
   else
     efl_promise2_reject(p, v);
   return EINA_FALSE;
}

Efl_Future2 *
_async_future_get(void)
{
   Efl_Promise2 *p;

   p = efl_promise2_new(NULL, NULL);
   ecore_timer_add(0.1, _timeout, p);
   return efl_future2_new(p, NULL, NULL, NULL, NULL);
}

static Eina_Value *
_async_future_err(void *data EINA_UNUSED, const Eina_Value *value)
{
   unsigned int i;
   eina_value_get(value, &i);
   printf("Last step: %u\n", i);
   ecore_main_loop_quit();
   return NULL;
}

static Eina_Value *
_async_future_end(void *data EINA_UNUSED, const Eina_Value *value)
{
   unsigned int i;
   Eina_Value *v = eina_value_new(EINA_VALUE_TYPE_INT);

   eina_value_get(value, &i);
   printf("Step: %u\n", i);
   efl_future2_then(_async_future_get(), _async_future_end,
                    _async_future_err, NULL, NULL);
   return NULL;
}

int
main(int argc EINA_UNUSED, char *argv[] EINA_UNUSED)
{
   Efl_Future2 *f;

   if (!eina_init())
     {
        fprintf(stderr, "Could not init eina\n");
        return EXIT_FAILURE;
     }

   if (!ecore_init())
     {
        fprintf(stderr, "Could not init ecore\n");
        goto err_ecore;
     }

   efl_future2_then(_async_future_get(), _async_future_end,
                    _async_future_err, NULL, NULL);

   ecore_main_loop_begin();

   eina_shutdown();
   ecore_shutdown();
   return EXIT_SUCCESS;

 err_ecore:
   eina_shutdown();
   return EXIT_FAILURE;
}
