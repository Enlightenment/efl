
#include "convert_comments.hh"
#include "safe_strings.hh"

namespace eolian_cxx {

static std::string
_comment_parameter(Eolian_Function_Parameter *param)
{
   Eolian_Parameter_Dir direction = eolian_parameter_direction_get(param);

   std::string doc = "@param";
   if (direction == EOLIAN_IN_PARAM) doc += " ";
   else if (direction == EOLIAN_OUT_PARAM) doc += "[out] ";
   else if (direction == EOLIAN_INOUT_PARAM) doc += "[inout] ";
   else assert(false);

   doc += safe_str(::eolian_parameter_name_get(param));
   doc += " ";
   /* FIXME */
   doc += safe_str(NULL);

   return doc;
}

static std::string
_comment_parameters_list(Eina_Iterator *params)
{
   std::string doc = "";
   void *curr;
   EINA_ITERATOR_FOREACH(params, curr)
     {
        doc += _comment_parameter
          (static_cast<Eolian_Function_Parameter*>(curr)) + "\n";
     }
   eina_iterator_free(params);
   return doc;
}

static std::string
_comment_brief_and_params(Eolian_Function const& function, Eolian_Function_Type)
{
   std::string doc = "";
   /* FIXME */
   std::string func = safe_str(NULL);
   if (func != "")
     {
        doc += "@brief " + func + "\n\n";
     }
   std::string params = _comment_parameters_list(::eolian_function_parameters_get(&function));
   if (params != "")
     {
        doc += params + "\n";
     }
   return doc;
}

static std::string
_comment_return(Eolian_Function const& function,
                Eolian_Function_Type rettype)
{
   const Eolian_Type *rettypet = ::eolian_function_return_type_get(&function, rettype);
   const char *rettypes = NULL;
   if (rettypet) rettypes = ::eolian_type_c_type_get(rettypet);
   std::string doc = "";
   std::string ret = safe_str(rettypes);
   if (rettypes) eina_stringshare_del(rettypes);
   /* FIXME */
   std::string comment = safe_str(NULL);
   if (ret != "void" && ret != "" && comment != "")
     {
        doc = "@return " + comment;
     }
   return doc;
}

std::string
convert_comments_class(Eolian_Class const&)
{
   /* FIXME */
   return safe_str(NULL);
}

std::string
convert_comments_function(Eolian_Class const& klass,
                          Eolian_Function const& function,
                          Eolian_Function_Type func_type)
{
   std::string doc = _comment_brief_and_params(function, func_type);
   if (!function_is_constructor(klass, function))
     doc += _comment_return(function, func_type);
   return doc;
}

} // namespace eolian_cxx
