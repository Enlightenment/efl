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

// The iterators read these after the function has returned, so they cannot
// live on its stack. The names are those of the _efl_model_property_* above.
static Eina_Iterator *
_efl_ui_size_model_properties_child(void)
{
   static const char *properties[] = {
     "item.width", "item.height", "self.height", "self.width"
   };
   return EINA_C_ARRAY_ITERATOR_NEW(properties);
}

static Eina_Iterator *
_efl_ui_size_model_properties_root(void)
{
   static const char *properties[] = {
     "item.width", "item.height"
   };
   return EINA_C_ARRAY_ITERATOR_NEW(properties);
}

static Eina_Iterator *
_efl_ui_size_model_efl_model_properties_get(const Eo *obj, void *pd EINA_UNUSED)
{
   Eina_Iterator *super;
   Eina_Iterator *prop;

   super = efl_model_properties_get(efl_super(obj, EFL_UI_SIZE_MODEL_CLASS));
   if (efl_isa(efl_parent_get(obj), EFL_UI_SIZE_MODEL_CLASS))
     prop = _efl_ui_size_model_properties_child();
   else
     prop = _efl_ui_size_model_properties_root();

   return eina_multi_iterator_new(super, prop);
}

#include "efl_ui_size_model.eo.c"
