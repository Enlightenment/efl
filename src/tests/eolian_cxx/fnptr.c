#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#include <Eo.h>
#include <Ecore.h>

#define EOLIANCXXTEST_API
#define EOLIANCXXTEST_API_WEAK

#include "fnptr.eo.h"

#define MY_CLASS FNPTR_CLASS

static const char *
_fnptr_text(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED,
            void *cb_data, Fnptr_Text_Func cb, Eina_Free_Cb cb_free_cb,
            int value)
{
   const char *text = NULL;

   if (cb) text = cb(cb_data, value);
   if (cb_free_cb) cb_free_cb(cb_data);

   return text;
}

static int
_fnptr_apply(Eo *obj EINA_UNUSED, void *pd EINA_UNUSED,
             void *first_data, Fnptr_Int_Func first, Eina_Free_Cb first_free_cb,
             void *second_data, Fnptr_Twice_Func second, Eina_Free_Cb second_free_cb,
             int value)
{
   int r = 0;

   if (first) r += first(first_data, value);
   if (first_free_cb) first_free_cb(first_data);
   if (second) r += second(second_data, value);
   if (second_free_cb) second_free_cb(second_data);

   return r;
}

#include "fnptr.eo.c"
