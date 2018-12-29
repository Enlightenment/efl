#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>
#include "elm_priv.h"

const char *_efl_model_property_itemw = "item.width";
const char *_efl_model_property_itemh = "item.height";
const char *_efl_model_property_selfw = "self.width";
const char *_efl_model_property_selfh = "self.height";
const char *_efl_model_property_totalw = "total.width";
const char *_efl_model_property_totalh = "total.height";

static Eina_Iterator *
_efl_ui_model_size_properties_child(void)
{
   const char *properties[] = {
     _efl_model_property_itemw, _efl_model_property_itemh, _efl_model_property_selfh, _efl_model_property_selfw
   };
   return EINA_C_ARRAY_ITERATOR_NEW(properties);
}

static Eina_Iterator *
_efl_ui_model_size_properties_root(void)
{
   const char *properties[] = {
     _efl_model_property_itemw, _efl_model_property_itemh
   };
   return EINA_C_ARRAY_ITERATOR_NEW(properties);
}

static Eina_Iterator *
_efl_ui_model_size_efl_model_properties_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Eina_Iterator *super;
   Eina_Iterator *prop;

   super = efl_model_properties_get(efl_super(obj, EFL_UI_MODEL_SIZE_CLASS));
   if (efl_isa(efl_parent_get(obj), EFL_UI_MODEL_SIZE_CLASS))
     prop = _efl_ui_model_size_properties_child();
   else
     prop = _efl_ui_model_size_properties_root();

   return eina_multi_iterator_new(super, prop);
}

#include "efl_ui_model_size.eo.c"
