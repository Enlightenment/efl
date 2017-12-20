// Internal header are to be used only inside efl itself.
#ifndef _ECORE_INTERNAL_H
#define _ECORE_INTERNAL_H

#ifdef EAPI
# undef EAPI
#endif

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

EAPI void ecore_loop_arguments_send(int argc, const char **argv);
EAPI Eina_Bool efl_loop_message_process(Eo *obj);

static inline Eina_Value
efl_model_list_value_get(Eina_List *childrens,
                         unsigned int start,
                         unsigned int count)
{
   Eina_Value v = EINA_VALUE_EMPTY;
   Eina_List *l;
   Eo *child;

   eina_value_array_setup(&v, EINA_VALUE_TYPE_OBJECT, eina_list_count(childrens));

   EINA_LIST_FOREACH(childrens, l, child)
     {
        if (start != 0)
          {
             start--;
             continue;
          }
        if (count == 0)
          continue;
        count--;

        eina_value_array_append(&v, child);
     }

   return v;
}

#undef EAPI
#define EAPI

#endif
