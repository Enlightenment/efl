#ifndef EFL_COMPOSITE_MODEL_PRIVATE_H_
# define EFL_COMPOSITE_MODEL_PRIVATE_H_

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

#endif
