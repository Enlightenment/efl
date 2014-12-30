
#include <vector>
#include <set>
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

void
add_ancestor_recursive(const char* klass_name, std::set<std::string>& ancestor)
{
   if (!klass_name)
     return;

   Eolian_Class const* klass = ::eolian_class_get_by_name(klass_name);
   if (!klass)
     {
        std::cerr << "Error: could not get eolian class name `" << klass_name << "'" << std::endl;
        return;
     }

   ancestor.insert(class_format_cxx(safe_lower(klass_name)));

   Eina_Iterator* inheritances = ::eolian_class_inherits_get(klass);
   void* curr = 0;

   EINA_ITERATOR_FOREACH(inheritances, curr)
     {
        add_ancestor_recursive(static_cast<const char*>(curr), ancestor);
     }
   eina_iterator_free(inheritances);
}

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
   efl::eolian::eo_function set_ =
     {
       efl::eolian::eo_function::regular_,
       function_name(prop_) + "_set",
       function_impl(prop_) + "_set",
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
   efl::eolian::eo_function get_ =
     {
       efl::eolian::eo_function::regular_,
       function_name(prop_) + "_get",
       function_impl(prop_) + "_get",
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


void
convert_eolian_inheritances(efl::eolian::eo_class& cls, Eolian_Class const& klass)
{
   Eina_Iterator *inheritances =
     ::eolian_class_inherits_get(&klass);
   void *curr;

   std::set<std::string> ancestors;

   EINA_ITERATOR_FOREACH(inheritances, curr)
     {
        const char* klass_name = static_cast<const char*>(curr);
        cls.parents.push_back(class_format_cxx(safe_lower(klass_name)));
        add_ancestor_recursive(klass_name, ancestors);
     }
   eina_iterator_free(inheritances);

   cls.ancestors.assign(ancestors.begin(), ancestors.end());
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
convert_eolian_functions(efl::eolian::eo_class& cls, Eolian_Class const& klass)
{
   for(efl::eina::iterator<const Eolian_Function> first ( ::eolian_class_functions_get(&klass, EOLIAN_METHOD))
    , last; first != last; ++first)
     {
        Eolian_Function const& func = *first;
        Eolian_Function_Type const func_type = function_op_type(func);

        if (!function_is_visible(func, func_type))
          continue;

        if (function_is_constructor(klass, func))
          {
             cls.constructors.push_back({
                  function_impl(func),
                  _convert_eolian_parameters(func),
                  convert_comments_function(klass, func)
             });
          }
        else
          {
             cls.functions.push_back({
                 function_type(func),
                 function_name(func),
                 function_impl(func),
                 function_return_type(func),
                 _convert_eolian_parameters(func),
                 convert_comments_function(klass, func, eolian_cxx::method)
               });
          }
     }
   for(efl::eina::iterator<const Eolian_Function> first ( ::eolian_class_functions_get(&klass, EOLIAN_PROPERTY))
    , last; first != last; ++first)
     {
        Eolian_Function const& func = *first;
        Eolian_Function_Type t = ::eolian_function_type_get(&func);

        if (!function_is_visible(func, t))
          continue;

        if(t == EOLIAN_PROP_GET)
          {
             cls.functions.push_back
               (_convert_property_get_to_function(klass, func));
          }
        else if(t == EOLIAN_PROP_SET)
          {
             cls.functions.push_back
               (_convert_property_set_to_function(klass, func));
          }
        else if(t == EOLIAN_PROPERTY)
          {
             cls.functions.push_back
               (_convert_property_get_to_function(klass, func));
             cls.functions.push_back
               (_convert_property_set_to_function(klass, func));
          }
       else
          {
             std::cerr << "Error: Inconsistent type for Eolian function \'" << ::eolian_function_name_get(&func) << "\'." << std::endl;
             throw std::runtime_error("Invalid Eolian function type");
          }
     }
}

efl::eolian::eo_class
convert_eolian_class(const Eolian_Class& klass)
{
   efl::eolian::eo_class cls(eolian_cxx::convert_eolian_class_new(klass));
   eolian_cxx::convert_eolian_inheritances(cls, klass);
   eolian_cxx::convert_eolian_functions(cls, klass);
   eolian_cxx::convert_eolian_events(cls, klass);
   efl::eolian::eo_class_validate(cls);
   return cls;
}

} // namespace eolian_cxx {
