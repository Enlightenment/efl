
#include <vector>
#include <algorithm>
#include <cassert>
#include <cstddef>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.hh>
#include <Eolian.h>

#include "eo_types.hh"
#include "eo_validate.hh"

#include "safe_strings.hh"
#include "convert_comments.hh"
#include "eolian_wrappers.hh"

namespace eolian_cxx {

extern efl::eina::log_domain domain;

static efl::eolian::parameters_container_type
_convert_eolian_parameters(Eina_Iterator *parameters,
                          Eolian_Function_Type func_type)
{
   if (parameters == NULL) return {};
   assert(func_type != EOLIAN_PROPERTY);

   void *curr;
   efl::eolian::parameters_container_type list;
   EINA_ITERATOR_FOREACH(parameters, curr)
     {
        const Eolian_Function_Parameter *id =
          (static_cast<const Eolian_Function_Parameter*>(curr));
        list.push_back
          ({
             parameter_type(*id, func_type),
             parameter_name(*id)
          });
     }
   eina_iterator_free(parameters);
   return list;
}

static efl::eolian::parameters_container_type
_convert_eolian_parameters(Eolian_Function const& func, getter_t func_type)
{
   return _convert_eolian_parameters
     (::eolian_function_parameters_get(&func), func_type.value);
}

static efl::eolian::parameters_container_type
_convert_eolian_parameters(Eina_Iterator *parameters, getter_t func_type)
{
   return _convert_eolian_parameters(parameters, func_type.value);
}

static efl::eolian::parameters_container_type
_convert_eolian_parameters(Eina_Iterator *parameters, setter_t func_type)
{
   return _convert_eolian_parameters(parameters, func_type.value);
}

static efl::eolian::parameters_container_type
_convert_eolian_parameters(Eolian_Function const& func)
{
   assert(function_op_type(func) != EOLIAN_PROPERTY);
   return _convert_eolian_parameters
     (::eolian_function_parameters_get(&func), function_op_type(func));
}

static efl::eolian::eo_function
_convert_property_set_to_function(Eolian_Class const& klass,
                                  Eolian_Function const& prop_)
{
   std::string prefix(class_prefix(klass));
   efl::eolian::eo_function set_ =
     {
       efl::eolian::eo_function::regular_,
       function_name(prop_) + "_set",
       function_impl(prop_, prefix) + "_set",
       function_return_type(prop_, eolian_cxx::setter),
       _convert_eolian_parameters(::eolian_function_parameters_get(&prop_),
                                  eolian_cxx::setter),
       convert_comments_function(klass, prop_, eolian_cxx::setter)
     };
   efl::eolian::parameters_container_type keys =
     _convert_eolian_parameters(::eolian_property_keys_get(&prop_),
                                eolian_cxx::setter);
   if (!keys.empty())
     {
        keys.reserve(keys.size() + set_.params.size());
        keys.insert(keys.end(), set_.params.begin(),
                    set_.params.end());
        set_.params = keys;
     }
   return set_;
}

static efl::eolian::eo_function
_convert_property_get_to_function(Eolian_Class const& klass,
                                  Eolian_Function const& prop_)
{
   std::string prefix(class_prefix(klass));
   efl::eolian::eo_function get_ =
     {
       efl::eolian::eo_function::regular_,
       function_name(prop_) + "_get",
       function_impl(prop_, prefix) + "_get",
       function_return_type(prop_, eolian_cxx::getter),
       _convert_eolian_parameters(prop_, eolian_cxx::getter),
       convert_comments_function(klass, prop_, eolian_cxx::getter)
     };

   // if the getter has a single parameter and a void return
   // it is transformed into a getter with no parameters
   // that actually returns what would be the first argument.
   if (get_.params.size() == 1 && efl::eolian::type_is_void(get_.ret) &&
       !function_return_is_explicit_void(prop_, eolian_cxx::getter))
     {
        get_.ret = get_.params[0].type;
        get_.params.clear();
     }
   else // otherwise just create the described getter
     {
        std::transform
          (get_.params.begin(), get_.params.end(), get_.params.begin(),
           [](efl::eolian::eo_parameter const& param)
           {
              efl::eolian::eolian_type getter_param_type =
                type_to_native(param.type);
              getter_param_type.native += "*";
              return efl::eolian::eo_parameter
                { { getter_param_type }, param.name };
           });
     }
   efl::eolian::parameters_container_type keys =
     _convert_eolian_parameters(::eolian_property_keys_get(&prop_),
                                eolian_cxx::getter);
   if (!keys.empty())
     {
        keys.reserve(keys.size() + get_.params.size());
        keys.insert(keys.end(), get_.params.begin(),
                    get_.params.end());
        get_.params = keys;
     }
   return get_;
}

static std::string
_convert_class_name(std::string const& name)
{
  return (name == "eo_base" || name == "eo::base" || name == "")
    ? "efl::eo::base"
    : name;
}

void
convert_eolian_inheritances(efl::eolian::eo_class& cls, Eolian_Class const& klass)
{
   Eina_Iterator *inheritances =
     ::eolian_class_inherits_get(&klass);
   void *curr;

   if (!eina_iterator_next(inheritances, &curr) || !curr)
     {
        cls.parent = "efl::eo::base";
        eina_iterator_free(inheritances);
        return;
     }
   else
     {
        // First element is the parent
        const char *ptr = static_cast<const char*>(curr);
        cls.parent = _convert_class_name(class_format_cxx(safe_lower(ptr)));

        EINA_ITERATOR_FOREACH(inheritances, curr)
         {
           std::string extension = safe_lower(static_cast<const char*>(curr));
           cls.extensions.push_back(_convert_class_name(class_format_cxx(extension)));
         }
     }
   eina_iterator_free(inheritances);
}

void
convert_eolian_events(efl::eolian::eo_class& cls, Eolian_Class const& klass)
{
   efl::eolian::events_container_type events = event_list(klass);
   cls.events.reserve(cls.events.size() + events.size());
   cls.events.insert(cls.events.end(), events.begin(), events.end());
}

efl::eolian::eo_class
convert_eolian_class_new(Eolian_Class const& klass)
{
   efl::eolian::eo_class cls;
   cls.type = class_type(klass);
   cls.name = class_name(klass);
   cls.name_space = class_namespace_full(klass);
   cls.eo_name = class_eo_name(klass);
   cls.comment = convert_comments_class(klass);
   return cls;
}

void
convert_eolian_implements(efl::eolian::eo_class& cls, Eolian_Class const& klass)
{
   std::string prefix(class_prefix(klass));
   efl::eina::iterator<const Eolian_Implement> itr = implements_get(klass);
   efl::eina::iterator<const Eolian_Implement> end;

   while (itr != end)
     {
        const Eolian_Implement impl = *itr;
        assert(!!implement_function(impl));
        assert(!!implement_class(impl));
        Eolian_Function const& func = *implement_function(impl);
        Eolian_Class const& icls = *implement_class(impl);

        if (implement_is_property_get(impl))
          {
             cls.functions.push_back
               (_convert_property_get_to_function(icls, func));
          }
        else if (implement_is_property_set(impl))
          {
             cls.functions.push_back
               (_convert_property_set_to_function(icls, func));
          }
        else if (function_op_type(func) == eolian_cxx::property.value)
          {
             cls.functions.push_back
               (_convert_property_get_to_function(icls, func));
             cls.functions.push_back
               (_convert_property_set_to_function(icls, func));
          }
        else if (function_is_constructor(klass, func))
          {
             cls.constructors.push_back({
                  function_impl(func, class_prefix(icls)),
                  _convert_eolian_parameters(func),
                  convert_comments_function(icls, func)
             });
          }
        else if (implement_is_visible(impl))
          {
             cls.functions.push_back({
                 function_type(func),
                 function_name(func),
                 function_impl(func, class_prefix(icls)),
                 function_return_type(func),
                 _convert_eolian_parameters(func),
                 convert_comments_function(icls, func, eolian_cxx::method)
               });
          }
        ++itr;
     }
}

efl::eolian::eo_class
convert_eolian_class(const Eolian_Class& klass)
{
   efl::eolian::eo_class cls(eolian_cxx::convert_eolian_class_new(klass));
   eolian_cxx::convert_eolian_inheritances(cls, klass);
   eolian_cxx::convert_eolian_implements(cls, klass);
   eolian_cxx::convert_eolian_events(cls, klass);
   efl::eolian::eo_class_validate(cls);
   return cls;
}

} // namespace eolian_cxx {
