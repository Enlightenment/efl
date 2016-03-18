
#include <vector>
#include <set>
#include <map>
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
typedef std::map<efl::eolian::eo_event, bool> event_map;

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

   ancestor.insert(class_format_cxx(safe_str(klass_name)));

   Eina_Iterator* inheritances = ::eolian_class_inherits_get(klass);
   void* curr = 0;

   EINA_ITERATOR_FOREACH(inheritances, curr)
     {
        add_ancestor_recursive(static_cast<const char*>(curr), ancestor);
     }
   eina_iterator_free(inheritances);
}

void
add_events_recursive(event_map& events, Eolian_Class const& klass, std::set<std::string>& ancestors)
{
   for (efl::eolian::eo_event const& e : event_list(klass))
     {
        auto it = events.find(e);
        if (it == events.end())
          events[e] = true;
        else
          it->second = false;
     }

   Eina_Iterator* inheritances = ::eolian_class_inherits_get(&klass);
   void* curr = 0;

   EINA_ITERATOR_FOREACH(inheritances, curr)
     {
        const char* ancestor_name = static_cast<const char*>(curr);
        if (!ancestor_name || ancestors.find(ancestor_name) != ancestors.end())
          continue;

        Eolian_Class const* ancestor_klass = ::eolian_class_get_by_name(ancestor_name);
        if (!ancestor_klass)
          {
             std::cerr << "Error: could not get eolian class name `" << ancestor_name << "'" << std::endl;
             continue;
          }
        ancestors.insert(ancestor_name);
        add_events_recursive(events, *ancestor_klass, ancestors);
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
             parameter_type(*id),
             parameter_name(*id)
          });
     }
   eina_iterator_free(parameters);
   return list;
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
       function_scope(prop_),
       function_is_beta(prop_),
       function_name(prop_) + "_set",
       function_impl(prop_) + "_set",
       function_return_type(prop_, eolian_cxx::setter),
       _convert_eolian_parameters(::eolian_property_values_get(&prop_, EOLIAN_PROP_SET),
                                  eolian_cxx::setter),
       convert_comments_function(klass, prop_, eolian_cxx::setter)
     };
   efl::eolian::parameters_container_type keys =
     _convert_eolian_parameters(::eolian_property_keys_get(&prop_, EOLIAN_PROP_SET),
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
       function_scope(prop_),
       function_is_beta(prop_),
       function_name(prop_) + "_get",
       function_impl(prop_) + "_get",
       function_return_type(prop_, eolian_cxx::getter),
       _convert_eolian_parameters(::eolian_property_values_get(&prop_, EOLIAN_PROP_GET),
                                  eolian_cxx::getter),
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
     _convert_eolian_parameters(::eolian_property_keys_get(&prop_, EOLIAN_PROP_GET),
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

static efl::eolian::eo_function
_convert_function(Eolian_Class const& klass, Eolian_Function const& func)
{
   return {
     function_type(func),
     function_scope(func),
     function_is_beta(func),
     function_name(func),
     function_impl(func),
     function_return_type(func),
     _convert_eolian_parameters(func),
     convert_comments_function(klass, func, eolian_cxx::method)
   };
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
        cls.parents.push_back(class_format_cxx(safe_str(klass_name)));
        add_ancestor_recursive(klass_name, ancestors);
     }
   eina_iterator_free(inheritances);

   cls.ancestors.assign(ancestors.begin(), ancestors.end());
}

void
convert_eolian_events(efl::eolian::eo_class& cls, Eolian_Class const& klass)
{
   efl::eolian::events_container_type events = event_list(klass);
   cls.own_events.reserve(cls.own_events.size() + events.size());
   cls.own_events.insert(cls.own_events.end(), events.begin(), events.end());

   event_map concrete_events;
   std::set<std::string> ancestors;

   add_events_recursive(concrete_events, klass, ancestors);

   for (auto const& e : events)
     {
        concrete_events[e] = true;
     }

   for (auto const& pair : concrete_events)
     {
        if (pair.second)
          cls.concrete_events.push_back(pair.first);
     }
}

efl::eolian::eo_class
convert_eolian_class_new(Eolian_Class const& klass)
{
   efl::eolian::eo_class cls;
   cls.type = class_type(klass);
   cls.name = class_name(klass);
   cls.name_space = class_namespace_full(klass);
   if(cls.name_space.empty())
     cls.name_space = "nonamespace";
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

        if (function_is_visible(func, function_op_type(func)) &&
            !function_is_constructor(klass, func))
          {
             cls.functions.push_back(_convert_function(klass, func));
          }
     }
   if (class_eo_name(klass) != "EO_BASE_CLASS")
     for(efl::eina::iterator<const Eolian_Constructor> first ( ::eolian_class_constructors_get(&klass))
      , last; first != last; ++first)
       {
          Eolian_Constructor const& ctor = *first;
          Eolian_Function const& func = *(::eolian_constructor_function_get(&ctor));

          efl::eolian::eo_function f;
          if (function_op_type(func) == EOLIAN_METHOD)
            f = _convert_function(klass, func);
          else
            f = _convert_property_set_to_function(klass, func);


          (::eolian_constructor_is_optional(&ctor) ?
            cls.optional_constructors :
            cls.constructors
          ).push_back({
             function_name(func),
             f.impl,
             f.params,
             f.comment
          });
       }

   cls.all_constructors = cls.constructors;
   cls.all_constructors.insert(cls.all_constructors.end(),
    cls.optional_constructors.begin(), cls.optional_constructors.end());

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
