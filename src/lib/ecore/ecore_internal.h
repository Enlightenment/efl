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

   if (eina_list_count(childrens) < start + count)
     return eina_value_error_init(EFL_MODEL_ERROR_INCORRECT_VALUE);

   eina_value_array_setup(&v, EINA_VALUE_TYPE_OBJECT, 4);

   childrens = eina_list_nth_list(childrens, start);

   EINA_LIST_FOREACH(childrens, l, child)
     {
        if (count == 0)
          break;
        count--;

        eina_value_array_append(&v, child);
     }

   return v;
}

#define EFL_COMPOSITE_MODEL_CHILD_INDEX "child.index"

#define EFL_COMPOSITE_MODEL_PROPERTIES(name, dyn, sta, ...) \
  EFL_COMPOSITE_MODEL_PROPERTIES_SUPER(name, NULL, NULL, (dyn), sta, ##__VA_ARGS__)

#define EFL_COMPOSITE_MODEL_PROPERTIES_SUPER(name, obj, klass, dyn, ...)       \
  Eina_Iterator *name;                                                         \
  do                                                                           \
    {                                                                          \
       static const char *static_props__[] = {                                 \
         __VA_ARGS__                                                           \
       };                                                                      \
       name = _efl_composite_model_properties_mix(                             \
         ((obj) ? efl_model_properties_get(efl_super((obj), (klass))) : NULL), \
         (dyn),                                                                \
         EINA_C_ARRAY_ITERATOR_NEW(static_props__));                           \
    }                                                                          \
  while (0)


static inline Eina_Iterator *
_efl_composite_model_properties_mix(Eina_Iterator *super, Eina_Iterator *dyn, Eina_Iterator *sta)
{
   Eina_Iterator *its[3];
   int i = 0;

   if (sta)   its[i++] = sta;
   if (dyn)   its[i++] = dyn;
   if (super) its[i++] = super;

   switch (i)
     {
      case 1: return its[0];
      case 2: return eina_multi_iterator_new(its[0], its[1]);
      case 3: return eina_multi_iterator_new(its[0], its[1], its[2]);
      default: return NULL;
     };
}

#define EFL_COMPOSITE_LOOKUP_RETURN(Remember, Parent, View, Base)       \
  Efl_Model *Remember;                                                  \
  char buf[1024];                                                       \
                                                                        \
  snprintf(buf, sizeof (buf), Base"-%p", Parent);                       \
  Remember = efl_key_wref_get(View, buf);                               \
  if (Remember)                                                         \
    {                                                                   \
       efl_ref(Remember);                                               \
       return Remember;                                                 \
    }

#define EFL_COMPOSITE_REMEMBER_RETURN(Remember, View)  \
  efl_key_wref_set(View, buf, Remember);               \
  return Remember;


static inline Efl_Model *
_efl_composite_lookup(const Efl_Class *self, Eo *parent, Efl_Model *view, unsigned int index)
{
   EFL_COMPOSITE_LOOKUP_RETURN(remember, parent, view, "_efl.composite_model");

   remember = efl_add_ref(self, parent,
                          efl_ui_view_model_set(efl_added, view),
                          efl_composite_model_index_set(efl_added, index),
                          efl_loop_model_volatile_make(efl_added));
   if (!remember) return NULL;

   EFL_COMPOSITE_REMEMBER_RETURN(remember, view);
}

/* Result from eina_future_all_* is an EINA_VALUE_TYPE_ARRAY that contain Eina_Value of
   Eo Model. It is expected that children slice get return an EINA_VALUE_TYPE_ARRAY that
   contain Eo Model directly.
*/
static inline Eina_Value
_efl_future_all_repack(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   unsigned int i, len;
   Eina_Value created = EINA_VALUE_EMPTY;
   Eina_Value r = EINA_VALUE_EMPTY;

   eina_value_array_setup(&r, EINA_VALUE_TYPE_OBJECT, 4);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, created)
     {
        Eo *target = NULL;

        if (eina_value_type_get(&created) != EINA_VALUE_TYPE_OBJECT)
          goto on_error;

        target = eina_value_object_get(&created);
        if (!target) goto on_error;

        eina_value_array_append(&r, target);
     }

   return r;

 on_error:
   eina_value_flush(&r);
   return eina_value_error_init(EFL_MODEL_ERROR_UNKNOWN);
}

#undef EAPI
#define EAPI

#endif
