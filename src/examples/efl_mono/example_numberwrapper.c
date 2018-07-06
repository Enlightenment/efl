
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <Eo.h>

#undef EOAPI
#undef EAPI
#define EOAPI EAPI EAPI_WEAK

#ifdef _WIN32
# ifdef EFL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#include "example_numberwrapper.eo.h"


typedef struct Example_Numberwrapper_Data
{
   int number;
   NumberCb cb;
   void *cb_data;
   Eina_Free_Cb free_cb;
} Example_Numberwrapper_Data;

// ##################### //
// Example.Numberwrapper //
// ##################### //


void _example_numberwrapper_number_set(EINA_UNUSED Eo *obj, Example_Numberwrapper_Data *pd, int n)
{
   pd->number = n;
}

void _example_numberwrapper_number_set_call(Eo *obj, EINA_UNUSED Example_Numberwrapper_Data *pd, int n)
{
   example_numberwrapper_number_set(obj, n);
}

int _example_numberwrapper_number_get(EINA_UNUSED const Eo *obj, Example_Numberwrapper_Data *pd)
{
   return pd->number;
}


void _example_numberwrapper_number_callback_set(EINA_UNUSED Eo *obj, Example_Numberwrapper_Data *pd, void *cb_data, NumberCb cb, Eina_Free_Cb cb_free_cb)
{
   if (pd->free_cb)
      pd->free_cb(pd->cb_data);

   pd->cb = cb;
   pd->cb_data = cb_data;
   pd->free_cb = cb_free_cb;
}


int _example_numberwrapper_callback_call(EINA_UNUSED Eo *obj, Example_Numberwrapper_Data *pd)
{
   if (!pd->cb)
     {
        static Eina_Error no_cb_err = 0;
        if (!no_cb_err)
          no_cb_err = eina_error_msg_static_register("Trying to call with no callback set");
        eina_error_set(no_cb_err);
        return -1;
     }

   return pd->cb(pd->cb_data, pd->number);
}

#include "example_numberwrapper.eo.c"

