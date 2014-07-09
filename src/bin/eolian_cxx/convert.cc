
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
convert_eolian_parameters(Eina_List const* parameters,
                          Eolian_Function_Type func_type)
{
   if (parameters == NULL) return {};
   assert(func_type != EOLIAN_PROPERTY);

   const Eina_List *it;
   void *curr;
   efl::eolian::parameters_container_type list;
   EINA_LIST_FOREACH (parameters, it, curr)
     {
        const Eolian_Function_Parameter *id =
          (static_cast<const Eolian_Function_Parameter*>(curr));
        list.push_back
          ({
             parameter_type(*id, func_type),
             parameter_name(*id)
          });
     }
   return list;
}

static efl::eolian::parameters_container_type
convert_eolian_parameters(Eolian_Function const& func, getter_t func_type)
{
   return convert_eolian_parameters
     (::eolian_parameters_list_get(&func), func_type.value);
}

static efl::eolian::parameters_container_type
convert_eolian_parameters(Eina_List const* parameters, getter_t func_type)
{
   return convert_eolian_parameters(parameters, func_type.value);
}

static efl::eolian::parameters_container_type
convert_eolian_parameters(Eina_List const* parameters, setter_t func_type)
{
   return convert_eolian_parameters(parameters, func_type.value);
}

static efl::eolian::parameters_container_type
convert_eolian_parameters(Eolian_Function const& func)
{
   assert(function_type(func) != EOLIAN_PROPERTY);
   return convert_eolian_parameters
     (::eolian_parameters_list_get(&func), function_type(func));
}

static efl::eolian::functions_container_type
convert_eolian_property_to_functions(Eolian_Class const& klass)
{
   efl::eolian::functions_container_type container;
   std::string cxx_classname = safe_lower(class_name(klass));
   const Eina_List *properties =
     ::eolian_class_functions_list_get(&klass, EOLIAN_PROPERTY); // XXX
   const Eina_List *it;
   void *curr;
   std::string prefix(class_prefix(klass));
   EINA_LIST_FOREACH (properties, it, curr)
     {
        Eolian_Function *prop_ = static_cast<Eolian_Function*>(curr);
        if (property_is_getter(*prop_))
          {
             efl::eolian::parameters_container_type params
               = convert_eolian_parameters(*prop_, eolian_cxx::getter);

             efl::eolian::eo_function get_;
             get_.type = efl::eolian::eo_function::regular_;
             get_.name = function_name(*prop_) + "_get";
             get_.impl = function_impl(*prop_, prefix) + "_get";

             efl::eolian::eolian_type_instance ret =
               function_return_type(*prop_, eolian_cxx::getter);

             // if the getter has a single parameter and a void return
             // it is transformed into a getter with no parameters
             // that actually returns what would be the first argument.
             if (params.size() == 1 && efl::eolian::type_is_void(ret))
               {
                  get_.ret = params[0].type;
                  get_.params.clear();
               }
             else // otherwise just create the described getter
               {
                  get_.ret = ret;
                  get_.params = params;
                  std::transform
                    (params.begin(), params.end(), get_.params.begin(),
                     [](efl::eolian::eo_parameter const& param)
                     {
                        efl::eolian::eolian_type getter_param_type =
                          type_to_native(param.type);
                        getter_param_type/*.native*/ += "*"; // XXX implement complex types
                        return efl::eolian::eo_parameter
                          { { getter_param_type }, param.name };
                     });
               }
             efl::eolian::parameters_container_type keys =
               convert_eolian_parameters(::eolian_property_keys_list_get(prop_),
                                         eolian_cxx::getter);
             if (!keys.empty())
               {
                  keys.reserve(keys.size() + get_.params.size());
                  keys.insert(keys.end(), get_.params.begin(),
                              get_.params.end());
                  get_.params = keys;
               }
             get_.comment = convert_comments_function(*prop_, eolian_cxx::getter);
             container.push_back(get_);
          }
        if (property_is_setter(*prop_))
          {
             const Eina_List *keys_ = eolian_property_keys_list_get(prop_);
             const Eina_List *args_ = eolian_parameters_list_get(prop_);
             Eina_List *params_ = eina_list_merge(eina_list_clone(keys_), eina_list_clone(args_));
             efl::eolian::parameters_container_type params =
               convert_eolian_parameters(params_, eolian_cxx::setter);
             eina_list_free(params_);
             efl::eolian::eo_function set_;
             set_.type = efl::eolian::eo_function::regular_;
             set_.name = function_name(*prop_) + "_set";
             set_.impl = function_impl(*prop_, prefix) + "_set";
             set_.params = params;
             set_.ret = function_return_type(*prop_, eolian_cxx::setter);
             set_.comment = convert_comments_function(*prop_, eolian_cxx::setter);
             container.push_back(set_);
          }
     }
   return container;
}

void
convert_eolian_inheritances(efl::eolian::eo_class& cls, Eolian_Class const& klass)
{
   const Eina_List *inheritances =
     ::eolian_class_inherits_list_get(&klass);
   const Eina_List *it;
   void *curr;

   if (eina_list_count(inheritances) == 0
    || eina_list_data_get(inheritances) == NULL)
     {
        cls.parent = "efl::eo::base";
        return;
     }
   else
     {
        const char *ptr = static_cast<const char*>
          (eina_list_data_get(inheritances));
        std::string parent = class_format_cxx(safe_lower(ptr));

        // "eo_base" is the Eolian name for EO_BASE_CLASS.
        cls.parent =
          (parent == "eo_base" || parent == "eo::base" || parent == "")
          ? "efl::eo::base"
          : parent;
     }
   inheritances = eina_list_next(inheritances);
   EINA_LIST_FOREACH (inheritances, it, curr)
     {
        std::string extension = safe_lower(static_cast<const char*>(curr));
        cls.extensions.push_back(class_format_cxx(extension));
     }
}

void
convert_eolian_implements(efl::eolian::eo_class& cls, Eolian_Class const& klass)
{
   const Eina_List *it;
   std::string prefix(class_prefix(klass));
   void *impl_desc_;

   EINA_LIST_FOREACH(::eolian_class_implements_list_get(&klass), it, impl_desc_)
     {
        const Eolian_Implement *impl_desc = static_cast<Eolian_Implement*>(impl_desc_);
        const Eolian_Class *impl_class;
        const Eolian_Function *impl_func;
        Eolian_Function_Type impl_type;
        eolian_implement_information_get
          (impl_desc, &impl_class, &impl_func, &impl_type);
        if (impl_type == EOLIAN_CTOR)
          {
             efl::eolian::eo_constructor constructor;
             std::string parent = safe_lower(eolian_class_full_name_get(impl_class));
             if (parent == "eo_base" || parent == "eo.base") parent = "eo";
             constructor.name = parent + "_" + function_name(*impl_func);
             constructor.params = convert_eolian_parameters(*impl_func);
             constructor.comment = convert_comments_function(*impl_func, eolian_cxx::ctor);
             cls.constructors.push_back(constructor);
          }
     }
}

void
convert_eolian_constructors(efl::eolian::eo_class& cls, Eolian_Class const& klass)
{
   const Eina_List *it;
   void *curr;
   std::string prefix(class_prefix(klass));
   const Eina_List *constructors =
     ::eolian_class_functions_list_get(&klass, EOLIAN_CTOR);
   EINA_LIST_FOREACH (constructors, it, curr)
     {
        Eolian_Function *eo_constructor = static_cast<Eolian_Function*>(curr);
        efl::eolian::eo_constructor constructor;
        constructor.name = function_impl(*eo_constructor, prefix);
        constructor.params = convert_eolian_parameters(*eo_constructor);
        constructor.comment = convert_comments_function(*eo_constructor, eolian_cxx::ctor);
        cls.constructors.push_back(constructor);
     }
}

void
convert_eolian_functions(efl::eolian::eo_class& cls, Eolian_Class const& klass)
{
   const Eina_List *it;
   void *curr;

   const Eina_List *eolian_functions =
     ::eolian_class_functions_list_get(&klass, EOLIAN_METHOD);
   EINA_LIST_FOREACH (eolian_functions, it, curr)
     {
        efl::eolian::eo_function func_;
        Eolian_Function *eol_func = static_cast<Eolian_Function*>(curr);
        // XXX Eolian only provides regular methods so far
        func_.type = efl::eolian::eo_function::regular_;
        func_.name = function_name(*eol_func);
        func_.impl = function_impl(*eol_func, class_prefix(klass));
        func_.ret = function_return_type(*eol_func);
        func_.params = convert_eolian_parameters(*eol_func);
        func_.comment = convert_comments_function(*eol_func, eolian_cxx::method);
        cls.functions.push_back(func_);
     }
}

void
convert_eolian_properties(efl::eolian::eo_class& cls, Eolian_Class const& klass)
{
   efl::eolian::functions_container_type properties
     = convert_eolian_property_to_functions(klass);
   cls.functions.insert
     (cls.functions.end(), properties.begin(), properties.end());
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
   cls.name = safe_lower(class_name(klass));
   cls.name_space = class_namespace_full(klass);
   cls.eo_name = class_eo_name(klass);
   cls.comment = convert_comments_class(klass);
   return cls;
}

efl::eolian::eo_class
convert_eolian_class(const Eolian_Class& klass)
{
   efl::eolian::eo_class cls(eolian_cxx::convert_eolian_class_new(klass));
   eolian_cxx::convert_eolian_inheritances(cls, klass);
   eolian_cxx::convert_eolian_implements(cls, klass);
   eolian_cxx::convert_eolian_constructors(cls, klass);
   eolian_cxx::convert_eolian_functions(cls, klass);
   eolian_cxx::convert_eolian_properties(cls, klass);
   eolian_cxx::convert_eolian_events(cls, klass);
   efl::eolian::eo_class_validate(cls);
   return cls;
}

} // namespace eolian_cxx {
