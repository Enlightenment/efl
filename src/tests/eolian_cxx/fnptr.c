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

typedef struct
{
   void *on_destroy_data;
   Fnptr_Visit_Func on_destroy;
   Eina_Free_Cb on_destroy_free_cb;
} Fnptr_Data;

static void
_fnptr_efl_object_destructor(Eo *obj, Fnptr_Data *pd)
{
   if (pd->on_destroy) pd->on_destroy(pd->on_destroy_data, obj, "bye", 0);
   if (pd->on_destroy_free_cb) pd->on_destroy_free_cb(pd->on_destroy_data);
   efl_destructor(efl_super(obj, MY_CLASS));
}

static void
_fnptr_visit_on_destroy(Eo *obj EINA_UNUSED, Fnptr_Data *pd,
                        void *cb_data, Fnptr_Visit_Func cb, Eina_Free_Cb cb_free_cb)
{
   if (pd->on_destroy_free_cb) pd->on_destroy_free_cb(pd->on_destroy_data);
   pd->on_destroy_data = cb_data;
   pd->on_destroy = cb;
   pd->on_destroy_free_cb = cb_free_cb;
}

static const char *
_fnptr_text(Eo *obj EINA_UNUSED, Fnptr_Data *pd EINA_UNUSED,
            void *cb_data, Fnptr_Text_Func cb, Eina_Free_Cb cb_free_cb,
            int value)
{
   const char *text = NULL;

   if (cb) text = cb(cb_data, value);
   if (cb_free_cb) cb_free_cb(cb_data);

   return text;
}

static int
_fnptr_apply(Eo *obj EINA_UNUSED, Fnptr_Data *pd EINA_UNUSED,
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

static int
_fnptr_sum_lent(Eo *obj EINA_UNUSED, Fnptr_Data *pd EINA_UNUSED, Eina_Iterator *l)
{
   int *i, sum = 0;

   if (!l) return -1;
   EINA_ITERATOR_FOREACH(l, i)
     {
        sum += *i;
     }

   return sum;
}

static int
_fnptr_format(Eo *obj EINA_UNUSED, Fnptr_Data *pd EINA_UNUSED,
              void *cb_data, Fnptr_Format_Func cb, Eina_Free_Cb cb_free_cb)
{
   Eina_Strbuf *buf = eina_strbuf_new();
   Eina_Value value;
   int r = -1;

   eina_value_setup(&value, EINA_VALUE_TYPE_INT);
   eina_value_set(&value, 42);
   if (cb && cb(cb_data, buf, value))
     r = !strcmp(eina_strbuf_string_get(buf), "value 42");
   eina_value_flush(&value);
   eina_strbuf_free(buf);
   if (cb_free_cb) cb_free_cb(cb_data);

   return r;
}

static int
_fnptr_visit(Eo *obj, Fnptr_Data *pd EINA_UNUSED,
             void *cb_data, Fnptr_Visit_Func cb, Eina_Free_Cb cb_free_cb)
{
   int r = 0;

   if (cb) r = cb(cb_data, obj, "hello", 7);
   if (cb_free_cb) cb_free_cb(cb_data);

   return r;
}

#include "fnptr.eo.c"
