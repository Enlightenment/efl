
#define EFL_MODEL_COMPOSITE_PROPERTIES(name, dyn, sta, ...) \
  EFL_MODEL_COMPOSITE_PROPERTIES_SUPER(name, NULL, NULL, (dyn), sta, ##__VA_ARGS__)

#define EFL_MODEL_COMPOSITE_PROPERTIES_SUPER(name, obj, klass, dyn, sta, ...)  \
  Eina_Iterator *name;                                                         \
  do                                                                           \
    {                                                                          \
       static const char *static_props__[] = {                                 \
         sta,                                                                  \
         ##__VA_ARGS__                                                         \
       };                                                                      \
       name = _efl_model_composite_properties_mix(                             \
         ((obj) ? efl_model_properties_get(efl_super((obj), (klass))) : NULL), \
         (dyn),                                                                \
         EINA_C_ARRAY_ITERATOR_NEW(static_props__));                           \
    }                                                                          \
  while (0)


static inline Eina_Iterator *
_efl_model_composite_properties_mix(Eina_Iterator *super, Eina_Iterator *dyn, Eina_Iterator *sta)
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

