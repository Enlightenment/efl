#include <vector>
#include <algorithm>
#include <cassert>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Eina.hh>
#include <Eo.h>
#include <Eolian.h>

#include "eo_types.hh"
#include "safe_strings.hh"
#include "comments.hh"

static std::string
_resolve_param_type(Eolian_Function_Parameter id, bool is_get)
{
   Eolian_Parameter_Dir dir;
   const char *type;
   bool is_const;
   std::string res;

   eolian_parameter_information_get(id, &dir, &type, NULL, NULL);
   is_const = eolian_parameter_const_attribute_get(id, is_get);
   res = safe_str(type);
   assert(res != "");
   if (is_const) res = std::string("const ") + res;
   if (dir == EOLIAN_OUT_PARAM || dir == EOLIAN_INOUT_PARAM) res += "*";
   return res;
}

static efl::eolian::parameters_container_type
_get_params(const Eina_List *eolian_params, bool is_get = false)
{
   const Eina_List *it;
   void *curr;
   if (eolian_params == NULL)
     {
        return efl::eolian::parameters_container_type();
     }
   efl::eolian::parameters_container_type list;
   EINA_LIST_FOREACH (eolian_params, it, curr)
     {
        Eolian_Function_Parameter id =
          (static_cast<Eolian_Function_Parameter>(curr));
        list.push_back({
             _resolve_param_type(id, is_get),
             safe_strshare(eolian_parameter_name_get(id))
        });
     }
   return list;
}

static efl::eolian::functions_container_type
_get_properties(const Eolian_Class klass)
{
   efl::eolian::functions_container_type container;

   std::string cxx_classname = eolian_class_name_get(klass);
   std::transform(cxx_classname.begin(), cxx_classname.end(),
                  cxx_classname.begin(), ::tolower);

   const Eina_List *properties;
   properties = eolian_class_functions_list_get(klass, EOLIAN_PROPERTY);

   const Eina_List *it;
   void *curr;
   std::string prefix(safe_str(eolian_class_eo_prefix_get(klass)));
   EINA_LIST_FOREACH (properties, it, curr)
     {
        Eolian_Function property = static_cast<Eolian_Function>(curr);
        Eolian_Function_Type type = eolian_function_type_get(property);
        std::string name = safe_str(eolian_function_name_get(property));
        if (type == EOLIAN_PROP_GET || type == EOLIAN_PROPERTY)
          {
             const Eina_List *keys_ = eolian_property_keys_list_get(property);
             efl::eolian::parameters_container_type params = _get_params
               (eolian_parameters_list_get(property), true);
             efl::eolian::eo_function getter;
             getter.type = efl::eolian::eo_function::regular_;
             getter.name = name + "_get";
             getter.impl = (prefix != "" ? prefix : cxx_classname) + "_" + getter.name;
             std::string ret = safe_str
               (eolian_function_return_type_get(property, EOLIAN_PROP_GET));
             if (ret == "") ret = "void";

             // if the getter has a single parameter and void return
             // we translate it to a getter with no parameters that
             // returns its type.
             if ((ret == "void") && params.size() == 1)
               {
                  getter.ret = params[0].type;
                  getter.params.clear();
               }
             else // otherwise just create the described getter
               {
                  getter.ret = ret;
                  getter.params = params;
                  std::transform
                    (params.begin(), params.end(), getter.params.begin(),
                     [](efl::eolian::eo_parameter const& param)
                     {
                        return efl::eolian::eo_parameter
                          { param.type + "*", param.name };
                     });
               }
             if (eina_list_count(keys_) > 0)
               {
                  efl::eolian::parameters_container_type keys = _get_params(keys_, true);
                  keys.reserve(keys.size() + getter.params.size());
                  keys.insert(keys.end(), getter.params.begin(), getter.params.end());
                  getter.params = keys;
               }
             getter.comment = detail::eolian_property_getter_comment(property);
             container.push_back(getter);
          }
        if (type == EOLIAN_PROP_SET || type == EOLIAN_PROPERTY)
          {
             const Eina_List *keys_ = eolian_property_keys_list_get(property);
             const Eina_List *args_ = eolian_parameters_list_get(property);
             Eina_List *params_ = eina_list_merge(eina_list_clone(keys_), eina_list_clone(args_));
             efl::eolian::parameters_container_type params = _get_params(params_);
             eina_list_free(params_);
             efl::eolian::eo_function setter;
             setter.type = efl::eolian::eo_function::regular_;
             setter.name = name + "_set";
             setter.impl = (prefix != "" ? prefix : cxx_classname) + "_" + setter.name;
             setter.params = params;
             setter.ret = safe_str(eolian_function_return_type_get
                                   (property, EOLIAN_PROP_SET));
             if (setter.ret == "") setter.ret = "void";
             setter.comment = detail::eolian_property_setter_comment(property);
             container.push_back(setter);
          }
     }
   return container;
}

namespace detail {

void
convert_eolian_inheritances(efl::eolian::eo_class& cls, const Eolian_Class klass)
{
   const Eina_List *inheritances = eolian_class_inherits_list_get(klass);
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
        std::string parent =
          static_cast<const char*>(eina_list_data_get(inheritances));
        std::transform(parent.begin(), parent.end(), parent.begin(), ::tolower);
        // "eo_base" is the Eolian name for EO_BASE_CLASS.
        cls.parent = (parent == "eo_base" || parent == "") ? "efl::eo::base" : parent;
     }

   inheritances = eina_list_next(inheritances);
   EINA_LIST_FOREACH (inheritances, it, curr)
     {
        std::string extension = static_cast<const char*>(curr);
        std::transform
          (extension.begin(), extension.end(), extension.begin(), ::tolower);
        cls.extensions.push_back(extension);
     }
}

void
convert_eolian_implements(efl::eolian::eo_class& cls, const Eolian_Class klass)
{
   const Eina_List *it;
   std::string prefix(safe_str(eolian_class_eo_prefix_get(klass)));
   void *impl_desc_;
   EINA_LIST_FOREACH(eolian_class_implements_list_get(klass), it, impl_desc_)
     {
        Eolian_Implement impl_desc = static_cast<Eolian_Implement>(impl_desc_);
        Eolian_Class impl_class;
        Eolian_Function impl_func;
        Eolian_Function_Type impl_type;
        eolian_implement_information_get
          (impl_desc, &impl_class, &impl_func, &impl_type);
        if (impl_type == EOLIAN_CTOR)
          {
             efl::eolian::eo_constructor constructor;
             std::string parent = safe_str(eolian_class_full_name_get(impl_class));
             if(parent == "Eo_Base") parent = "eo";
             else std::transform(parent.begin(), parent.end(), parent.begin(), ::tolower);
             constructor.name = parent + "_" + safe_str(eolian_function_name_get(impl_func));
             constructor.params = _get_params
               (eolian_parameters_list_get(impl_func));
             constructor.comment = detail::eolian_constructor_comment
               (impl_func);
             cls.constructors.push_back(constructor);
          }
     }
}

void
convert_eolian_constructors(efl::eolian::eo_class& cls, const Eolian_Class klass)
{
   const Eina_List *it;
   void *curr;
   std::string prefix(safe_str(eolian_class_eo_prefix_get(klass)));
   const Eina_List *constructors =
     eolian_class_functions_list_get(klass, EOLIAN_CTOR);
   EINA_LIST_FOREACH (constructors, it, curr)
     {
        Eolian_Function eolian_constructor = static_cast<Eolian_Function>(curr);
        efl::eolian::eo_constructor constructor;
        constructor.name = (prefix != "" ? prefix : cls.name) + "_" + safe_str
          (eolian_function_name_get(eolian_constructor));
        constructor.params = _get_params
          (eolian_parameters_list_get(eolian_constructor));
        constructor.comment = detail::eolian_constructor_comment
          (eolian_constructor);
        cls.constructors.push_back(constructor);
     }
}

void
convert_eolian_functions(efl::eolian::eo_class& cls, const Eolian_Class klass)
{
   const Eina_List *it;
   void *curr;

   const Eina_List *eolian_functions =
     eolian_class_functions_list_get(klass, EOLIAN_METHOD);
   EINA_LIST_FOREACH (eolian_functions, it, curr)
     {
        efl::eolian::eo_function function;
        Eolian_Function eolian_function = static_cast<Eolian_Function>(curr);
        std::string prefix(safe_str(eolian_class_eo_prefix_get(klass)));
        // XXX Eolian only provides regular methods so far
        function.type = efl::eolian::eo_function::regular_;
        function.name = safe_str(eolian_function_name_get(eolian_function));
        function.impl = ( prefix != "" ? prefix : cls.name ) + "_" + function.name;
        function.ret = safe_str(eolian_function_return_type_get
                                (eolian_function, EOLIAN_METHOD));
        if(function.ret == "") function.ret = "void";
        function.params = _get_params(eolian_parameters_list_get(eolian_function));
        function.comment = detail::eolian_function_comment(eolian_function);
        cls.functions.push_back(function);
     }
}

void
convert_eolian_properties(efl::eolian::eo_class& cls, const Eolian_Class klass)
{
   efl::eolian::functions_container_type properties = _get_properties(klass);
   cls.functions.insert(cls.functions.end(), properties.begin(), properties.end());
}

} // namespace detail {

efl::eolian::eo_class
_cxx_new(const Eolian_Class klass)
{
   using namespace efl::eolian;
   eo_class cls;
   Eolian_Class_Type cls_type = ::eolian_class_type_get(klass);
   if      (cls_type == EOLIAN_CLASS_REGULAR)   cls.type = eo_class::regular_;
   else if (cls_type == EOLIAN_CLASS_ABSTRACT)  cls.type = eo_class::regular_noninst_;
   else if (cls_type == EOLIAN_CLASS_MIXIN)     cls.type = eo_class::mixin_;
   else if (cls_type == EOLIAN_CLASS_INTERFACE) cls.type = eo_class::interface_;
   else    { assert(false); }
   std::string prefix(safe_str(eolian_class_eo_prefix_get(klass)));
   cls.name = eolian_class_name_get(klass);
   cls.eo_name = (prefix != "" ? prefix : cls.name) + "_CLASS";
   cls.comment = detail::eolian_class_comment(klass);
   std::transform(cls.name.begin(), cls.name.end(), cls.name.begin(), ::tolower);
   std::transform(cls.eo_name.begin(), cls.eo_name.end(), cls.eo_name.begin(), ::toupper);
   return cls;
}

efl::eolian::eo_class
c_to_cxx(const char *classname)
{
   Eolian_Class klass = eolian_class_find_by_name(classname);
   efl::eolian::eo_class cls(_cxx_new(klass));
   detail::convert_eolian_inheritances(cls, klass);
   detail::convert_eolian_implements(cls, klass);
   detail::convert_eolian_constructors(cls, klass);
   detail::convert_eolian_functions(cls, klass);
   detail::convert_eolian_properties(cls, klass);
   return cls;
}
