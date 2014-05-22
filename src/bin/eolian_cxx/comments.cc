
#include "comments.hh"
#include "safe_strings.hh"

static std::string
_comment_parameter(Eolian_Function_Parameter param)
{
   Eolian_Parameter_Dir direction;
   Eina_Stringshare *description;

   eolian_parameter_information_get
     (param, &direction, NULL, NULL, &description);

   std::string doc = "@param";
   if (direction == EOLIAN_IN_PARAM) doc += " ";
   else if (direction == EOLIAN_OUT_PARAM) doc += "[out] ";
   else if (direction == EOLIAN_INOUT_PARAM) doc += "[inout] ";
   else assert(false);

   doc += safe_strshare(eolian_parameter_name_get(param));
   doc += " ";
   doc += safe_str(description);

   return doc;
}

static std::string
_comment_parameters_list(const Eina_List *params)
{
   std::string doc = "";
   const Eina_List *it;
   void *curr;
   EINA_LIST_FOREACH (params, it, curr)
     {
        doc += _comment_parameter
          (static_cast<Eolian_Function_Parameter>(curr)) + "\n";
     }
   return doc;
}

static std::string
_comment_brief_and_params(Eolian_Function function,
                          const char *key = EOLIAN_COMMENT)
{
   std::string doc = "";
   std::string func = safe_str(eolian_function_description_get(function, key));
   if (func != "")
     {
        doc += "@brief " + func + "\n\n";
     }
   std::string params = _comment_parameters_list(eolian_parameters_list_get(function));
   if (params != "")
     {
        doc += params + "\n";
     }
   return doc;
}

static std::string
_comment_return(Eolian_Function function,
                Eolian_Function_Type rettype)
{
   std::string doc = "";
   std::string ret = safe_str(eolian_function_return_type_get(function, rettype));
   std::string comment = safe_str(eolian_function_return_comment_get(function, rettype));
   if (ret != "void" && ret != "" && comment != "")
     {
        doc = "@return " + comment;
     }
   return doc;
}

namespace detail {

std::string
eolian_class_comment(const Eolian_Class kls)
{
   return safe_str(eolian_class_description_get(kls));
}

std::string
eolian_constructor_comment(Eolian_Function constructor)
{
   return _comment_brief_and_params(constructor);
}

std::string eolian_function_comment(Eolian_Function function)
{
   std::string doc = _comment_brief_and_params(function);
   doc += _comment_return(function, EOLIAN_METHOD);
   return doc;
}

std::string eolian_property_getter_comment(Eolian_Function property)
{
   std::string doc = _comment_brief_and_params
     (property, EOLIAN_COMMENT_GET);
   doc += _comment_return(property, EOLIAN_PROP_GET);
   return doc;
}

std::string eolian_property_setter_comment(Eolian_Function property)
{
   std::string doc = _comment_brief_and_params
     (property, EOLIAN_COMMENT_SET);
   doc += _comment_return(property, EOLIAN_PROP_SET);
   return doc;
}

} // namespace detail
