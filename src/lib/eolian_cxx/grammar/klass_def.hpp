#ifndef EOLIAN_CXX_KLASS_DEF_HH
#define EOLIAN_CXX_KLASS_DEF_HH

#include "grammar/type_traits.hpp"
#include "grammar/attributes.hpp"
#include "grammar/qualifier_def.hpp"
#include "grammar/string.hpp"
#include "grammar/sequence.hpp"
#include "grammar/kleene.hpp"
#include "grammar/case.hpp"

#include <Eolian.h>

#include <Eina.hh>

#include <vector>
#include <memory>
#include <set>

namespace efl { namespace eolian { namespace grammar {

namespace attributes {

struct complex_type_def;

}

namespace attributes {

template <typename...Args, std::size_t I>
bool lexicographical_compare_impl(std::tuple<Args...> const&
                                  , std::tuple<Args...> const&
                                  , std::integral_constant<std::size_t, I>
                                  , std::true_type)
{
  return true;
}
template <typename...Args, std::size_t I>
bool lexicographical_compare_impl(std::tuple<Args...> const& lhs
                                  , std::tuple<Args...> const& rhs
                                  , std::integral_constant<std::size_t, I>
                                  , std::false_type)
{
  return std::get<I>(lhs) < std::get<I>(rhs)
      || (!(std::get<I>(rhs) < std::get<I>(lhs))
          && lexicographical_compare_impl(lhs, rhs, std::integral_constant<std::size_t, I+1>()
                                          , std::integral_constant<bool, I + 1 == sizeof...(Args)>())
         )
    ;
}
template <typename...Args>
bool lexicographical_compare(std::tuple<Args...> const& lhs
                             , std::tuple<Args...> const& rhs)
{
  return lexicographical_compare_impl(lhs, rhs, std::integral_constant<std::size_t, 0ul>(), std::false_type());
}
template <typename T, typename U>
bool lexicographical_compare(std::tuple<T, U> const& lhs
                             , std::tuple<T, U> const& rhs)
{
  return std::get<0>(lhs) < std::get<0>(rhs)
       || (!(std::get<0>(rhs) < std::get<0>(lhs))
          && std::get<1>(lhs) < std::get<1>(rhs));
}

enum class typedecl_type
{
  unknown,
  struct_,
  struct_opaque,
  enum_,
  alias,
  function_ptr,
};

inline typedecl_type typedecl_type_get(Eolian_Typedecl const* decl)
{
  if (!decl)
    return typedecl_type::unknown;

  Eolian_Typedecl_Type t = eolian_typedecl_type_get(decl);
  switch (t)
  {
  case EOLIAN_TYPEDECL_UNKNOWN: return typedecl_type::unknown;
  case EOLIAN_TYPEDECL_STRUCT: return typedecl_type::struct_;
  case EOLIAN_TYPEDECL_STRUCT_OPAQUE: return typedecl_type::struct_opaque;
  case EOLIAN_TYPEDECL_ENUM: return typedecl_type::enum_;
  case EOLIAN_TYPEDECL_ALIAS: return typedecl_type::alias;
  case EOLIAN_TYPEDECL_FUNCTION_POINTER: return typedecl_type::function_ptr;
  default: return typedecl_type::unknown;
  }
}

struct type_def;
bool operator==(type_def const& rhs, type_def const& lhs);
bool operator!=(type_def const& rhs, type_def const& lhs);

enum class class_type
{
  regular, abstract_, mixin, interface_
};

struct klass_name
{
   std::vector<std::string> namespaces;
   std::string eolian_name;
   qualifier_def base_qualifier;
   class_type type;

   klass_name() {
   }

   klass_name(std::vector<std::string> namespaces
              , std::string eolian_name, qualifier_def base_qualifier
              , class_type type)
     : namespaces(namespaces), eolian_name(eolian_name), base_qualifier(base_qualifier)
     , type(type) {}
   klass_name(Eolian_Class const* klass, qualifier_def base_qualifier)
     : eolian_name( ::eolian_class_short_name_get(klass))
              , base_qualifier(base_qualifier)
   {
     for(efl::eina::iterator<const char> namespace_iterator ( ::eolian_class_namespaces_get(klass))
           , namespace_last; namespace_iterator != namespace_last; ++namespace_iterator)
       {
         namespaces.push_back(&*namespace_iterator);
       }
     switch(eolian_class_type_get(klass))
       {
       case EOLIAN_CLASS_REGULAR:
         type = class_type::regular;
         break;
       case EOLIAN_CLASS_ABSTRACT:
         type = class_type::abstract_;
         break;
       case EOLIAN_CLASS_MIXIN:
         type = class_type::mixin;
         break;
       case EOLIAN_CLASS_INTERFACE:
         type = class_type::interface_;
         break;
       default:
         throw std::runtime_error("Class with unknown type");
       }
   }
};

inline bool operator==(klass_name const& lhs, klass_name const& rhs)
{
  return lhs.namespaces == rhs.namespaces && lhs.eolian_name == rhs.eolian_name
    && lhs.base_qualifier == rhs.base_qualifier/* && lhs.pointers == rhs.pointers*/;
}
inline bool operator!=(klass_name const& lhs, klass_name const& rhs)
{
  return !(lhs == rhs);
}
inline bool operator<(klass_name const& lhs, klass_name const& rhs)
{
  typedef std::tuple<std::vector<std::string>const&
                     , std::string const&
                     , qualifier_def const&
                     , class_type
                     > tuple_type;
  return lexicographical_compare(tuple_type(lhs.namespaces, lhs.eolian_name
                                            , lhs.base_qualifier
                                            , lhs.type)
                                 , tuple_type(rhs.namespaces, rhs.eolian_name
                                              , rhs.base_qualifier
                                              , rhs.type));
}

struct documentation_def
{
   std::string summary;
   std::string description;
   std::string since;
   std::vector<std::string> desc_paragraphs;

   documentation_def() {}
   documentation_def(std::string summary, std::string description, std::string since)
     : summary(summary), description(description), since(since)
   {}
   documentation_def(Eolian_Documentation const* eolian_doc)
   {
      const char *str;

      if (!eolian_doc)
        return;

      str = eolian_documentation_summary_get(eolian_doc);
      if (str)
        summary = str;

      str = eolian_documentation_description_get(eolian_doc);
      if (str)
        description = str;

      str = eolian_documentation_since_get(eolian_doc);
      if (str)
        since = str;

      efl::eina::ptr_list<const char> l(eolian_documentation_string_split(description.c_str()));
      for (auto&& i : l)
        desc_paragraphs.push_back({&i});
   }

   friend inline bool operator==(documentation_def const& lhs, documentation_def const& rhs)
   {
      return lhs.summary == rhs.summary
        && lhs.description == rhs.description
        && lhs.since == rhs.since;
   }
};

template <>
struct tuple_element<0ul, klass_name>
{
  typedef std::vector<std::string> type;
  static type& get(klass_name& klass) { return klass.namespaces; }
  static type const& get(klass_name const& klass) { return klass.namespaces; }
};
template <>
struct tuple_element<1ul, klass_name>
{
  typedef std::string type;
  static type& get(klass_name& klass) { return klass.eolian_name; }
  static type const& get(klass_name const& klass) { return klass.eolian_name; }
};
template <int N>
struct tuple_element<N, klass_name const> : tuple_element<N, klass_name> {};

template <int N>
typename tuple_element<N, klass_name>::type&
get(klass_name& klass)
{
  return tuple_element<N, klass_name>::get(klass);
}
template <int N>
typename tuple_element<N, klass_name>::type const&
get(klass_name const& klass)
{
  return tuple_element<N, klass_name>::get(klass);
}

struct regular_type_def
{
   regular_type_def() : type_type(typedecl_type::unknown), is_undefined(false) {}
   regular_type_def(std::string base_type, qualifier_def qual, std::vector<std::string> namespaces
                    , typedecl_type type_type = typedecl_type::unknown, bool is_undefined = false)
     : base_type(std::move(base_type)), base_qualifier(qual), namespaces(std::move(namespaces))
     , type_type(type_type), is_undefined(is_undefined) {}

   bool is_type(typedecl_type tt) const { return type_type == tt; }
   bool is_unknown() const { return is_type(typedecl_type::unknown); }
   bool is_struct() const { return is_type(typedecl_type::struct_); }
   bool is_struct_opaque() const { return is_type(typedecl_type::struct_opaque); }
   bool is_enum() const { return is_type(typedecl_type::enum_); }
   bool is_alias() const { return is_type(typedecl_type::alias); }
   bool is_function_ptr() const { return is_type(typedecl_type::function_ptr); }

   std::string base_type;
   qualifier_def base_qualifier;
   std::vector<std::string> namespaces;
   typedecl_type type_type;
   bool is_undefined;
};

inline bool operator==(regular_type_def const& rhs, regular_type_def const& lhs)
{
  return rhs.base_type == lhs.base_type && rhs.base_qualifier == lhs.base_qualifier;
}
inline bool operator!=(regular_type_def const& rhs, regular_type_def const& lhs)
{
  return !(rhs == lhs);
}

struct complex_type_def
{
   regular_type_def outer;
   std::vector<type_def> subtypes;
};

inline bool operator==(complex_type_def const& lhs, complex_type_def const& rhs)
{
  return lhs.outer == rhs.outer && lhs.subtypes == rhs.subtypes;
}
inline bool operator!=(complex_type_def const& lhs, complex_type_def const& rhs)
{
  return !(lhs == rhs);
}

struct type_def
{
   typedef eina::variant<klass_name, regular_type_def, complex_type_def> variant_type;
   variant_type original_type;
   std::string c_type;
   bool has_own;
   bool is_ptr;

   type_def() {}
   type_def(variant_type original_type, std::string c_type, bool has_own)
     : original_type(original_type), c_type(c_type), has_own(has_own) {}

   type_def(Eolian_Type const* eolian_type, Eolian_Unit const* unit, Eolian_C_Type_Type ctype)
   {
     set(eolian_type, unit, ctype);
   }
   void set(Eolian_Type const* eolian_type, Eolian_Unit const* unit, Eolian_C_Type_Type ctype);
   void set(Eolian_Expression_Type eolian_exp_type);

   friend inline bool operator<(type_def const& lhs, type_def const& rhs)
   {
      return lhs.c_type < rhs.c_type;
   }
};

struct get_qualifier_visitor
{
  typedef qualifier_def result_type;
  template <typename T>
  qualifier_def operator()(T const& object) const
  {
     return object.base_qualifier;
  }
  qualifier_def operator()(complex_type_def const& complex) const
  {
    return complex.outer.base_qualifier;
  }
};

inline bool operator==(type_def const& lhs, type_def const& rhs)
{
  return lhs.original_type == rhs.original_type && lhs.c_type == rhs.c_type;
}
inline bool operator!=(type_def const& lhs, type_def const& rhs)
{
  return !(lhs == rhs);
}

type_def const void_ {attributes::regular_type_def{"void", {qualifier_info::is_none, {}}, {}}, "void", false};

inline void type_def::set(Eolian_Type const* eolian_type, Eolian_Unit const* unit, Eolian_C_Type_Type ctype)
{
   c_type = ::eolian_type_c_type_get(eolian_type, ctype);
   // ::eina_stringshare_del(stringshare); // this crashes
   Eolian_Type const* stp = eolian_type_base_type_get(eolian_type);
   has_own = !!::eolian_type_is_owned(eolian_type);
   is_ptr = !!::eolian_type_is_ptr(eolian_type);
   switch( ::eolian_type_type_get(eolian_type))
     {
     case EOLIAN_TYPE_VOID:
       original_type = attributes::regular_type_def{"void", {qualifiers(eolian_type), {}}, {}};
       break;
     case EOLIAN_TYPE_REGULAR:
       if (!stp)
         {
           bool is_undefined = false;
           Eolian_Typedecl const* decl = eolian_type_typedecl_get(eolian_type);
           typedecl_type type_type = (decl ? typedecl_type_get(decl) : typedecl_type::unknown);
           if(decl && eolian_typedecl_type_get(decl) == EOLIAN_TYPEDECL_ALIAS)
             {
               Eolian_Type const* aliased = eolian_typedecl_base_type_get(decl);
               if(aliased && eolian_type_type_get(aliased) == EOLIAN_TYPE_UNDEFINED)
                 {
                   is_undefined = true;
                 }
             }
           if(c_type == "va_list *")
             throw std::runtime_error("");
           std::vector<std::string> namespaces;
           for(efl::eina::iterator<const char> namespace_iterator( ::eolian_type_namespaces_get(eolian_type))
                 , namespace_last; namespace_iterator != namespace_last; ++namespace_iterator)
             namespaces.push_back(&*namespace_iterator);
           original_type = {regular_type_def{ ::eolian_type_short_name_get(eolian_type), {qualifiers(eolian_type), {}}, namespaces, type_type, is_undefined}};
         }
       else
         {
           complex_type_def complex
            {{::eolian_type_short_name_get(eolian_type), {qualifiers(eolian_type), {}}, {}}, {}};
           while (stp)
             {
                complex.subtypes.push_back({stp, unit, EOLIAN_C_TYPE_DEFAULT});
                stp = eolian_type_next_type_get(stp);
             }
           original_type = complex;
         }
       break;
     case EOLIAN_TYPE_CLASS:
       {
          Eolian_Class const* klass = eolian_type_class_get(eolian_type);
          original_type = klass_name(klass, {qualifiers(eolian_type), {}});
       }
       break;
     default:
       throw std::runtime_error("Type not supported");
       break;
     }
}

inline void type_def::set(Eolian_Expression_Type eolian_exp_type)
{
    switch(eolian_exp_type)
      {
      case EOLIAN_EXPR_INT:
        original_type = attributes::regular_type_def{"int", {{}, {}}, {}};
        c_type = "int";
        break;
      default:
        // FIXME implement the remaining types
        EINA_LOG_ERR("Unsupported expression type");
        std::abort();
        break;
      }
}

struct alias_def
{
  std::string eolian_name;
  std::string cxx_name;
  std::vector<std::string> namespaces;
  bool is_undefined;
  type_def base_type {};
  documentation_def documentation;

  alias_def(Eolian_Typedecl const* alias_obj, Eolian_Unit const* unit)
  {
     cxx_name = eolian_name = ::eolian_typedecl_short_name_get(alias_obj);

     for(efl::eina::iterator<const char> namespace_iterator( ::eolian_typedecl_namespaces_get(alias_obj))
          , namespace_last; namespace_iterator != namespace_last; ++namespace_iterator)
       {
          this->namespaces.push_back((&*namespace_iterator));
       }

     Eolian_Type const* bt = ::eolian_typedecl_base_type_get(alias_obj);
     if (eolian_type_type_get(bt) == EOLIAN_TYPE_UNDEFINED)
       is_undefined = true;
     else
       {
          base_type = type_def(::eolian_typedecl_base_type_get(alias_obj), unit, EOLIAN_C_TYPE_DEFAULT);
          is_undefined = false;
       }

     documentation = ::eolian_typedecl_documentation_get(alias_obj);

  }
};

enum class parameter_direction
{
  unknown, in, inout, out
};

namespace detail {
struct add_optional_qualifier_visitor
{
  typedef void result_type;
  template <typename T>
  void operator()(T&  object) const
  {
    object.base_qualifier.qualifier |= qualifier_info::is_optional;
  }
  void operator()(complex_type_def& complex) const
  {
    (*this)(complex.outer);
  }
};
}

struct parameter_def
{
  parameter_direction direction;
  type_def type;
  std::string param_name;
  documentation_def documentation;
  Eolian_Unit const* unit;

  friend inline bool operator==(parameter_def const& lhs, parameter_def const& rhs)
  {
    return lhs.direction == rhs.direction
      && lhs.type == rhs.type
      && lhs.param_name == rhs.param_name
      && lhs.documentation == rhs.documentation;
  }
  friend inline bool operator!=(parameter_def const& lhs, parameter_def const& rhs)
  {
    return !(lhs == rhs);
  }

  parameter_def(parameter_direction direction, type_def type, std::string param_name,
                documentation_def documentation, Eolian_Unit const* unit)
    : direction(std::move(direction)), type(std::move(type)), param_name(std::move(param_name)), documentation(documentation), unit(unit) {}
  parameter_def(Eolian_Function_Parameter const* param, Eolian_Unit const* unit)
    : type( ::eolian_parameter_type_get(param), unit, EOLIAN_C_TYPE_PARAM)
    , param_name( ::eolian_parameter_name_get(param)), unit(unit)
  {
     Eolian_Parameter_Dir direction = ::eolian_parameter_direction_get(param);
     switch(direction)
       {
       case EOLIAN_UNKNOWN_PARAM:
       case EOLIAN_IN_PARAM:
         this->direction = parameter_direction::in;
         break;
       case EOLIAN_INOUT_PARAM:
         this->direction = parameter_direction::inout;
         break;
       case EOLIAN_OUT_PARAM:
         this->direction = parameter_direction::out;
         break;
       }
     if( ::eolian_parameter_is_optional(param))
       type.original_type.visit(detail::add_optional_qualifier_visitor{});

     documentation = eolian_parameter_documentation_get(param);
  }
};

template <>
struct tuple_element<0ul, parameter_def>
{
  typedef parameter_direction type;
  static type const& get(parameter_def const& p) { return p.direction; }
  static type& get(parameter_def& p) { return p.direction; }
};
template <>
struct tuple_element<1ul, parameter_def>
{
  typedef type_def type;
  static type const& get(parameter_def const& p) { return p.type; }
  static type& get(parameter_def& p) { return p.type; }
};
template <>
struct tuple_element<2ul, parameter_def>
{
  typedef std::string type;
  static type const& get(parameter_def const& p) { return p.param_name; }
  static type& get(parameter_def& p) { return p.param_name; }
};
template <>
struct tuple_element<3ul, parameter_def>
{
  typedef std::string type;
  static type const& get(parameter_def const& p) { return p.type.c_type; }
  static type& get(parameter_def& p) { return p.type.c_type; }
};
template <int I>
typename tuple_element<I, parameter_def>::type const& get(parameter_def const& p)
{ return tuple_element<I, parameter_def>::get(p); }
template <int I>
typename tuple_element<I, parameter_def>::type& get(parameter_def& p)
{ return tuple_element<I, parameter_def>::get(p); }

enum class function_type
{
  unresolved,
  property,
  prop_set,
  prop_get,
  method,
  function_pointer
};

struct function_def
{
  klass_name klass; // Klass information for function_def as method
  type_def return_type;
  std::string name;
  std::vector<parameter_def> parameters;
  std::string c_name;
  std::string filename;
  std::vector<std::string> namespaces; // Namespaces for top-level function pointers
  documentation_def documentation;
  documentation_def return_documentation;
  documentation_def property_documentation;
  function_type type;
  bool is_beta;
  bool is_protected;
  bool is_static;
  Eolian_Unit const* unit;

  friend inline bool operator==(function_def const& lhs, function_def const& rhs)
  {
    return lhs.klass == rhs.klass
      && lhs.return_type == rhs.return_type
      && lhs.name == rhs.name
      && lhs.parameters == rhs.parameters
      && lhs.c_name == rhs.c_name
      && lhs.filename == rhs.filename
      && lhs.namespaces == rhs.namespaces
      && lhs.documentation == rhs.documentation
      && lhs.return_documentation == rhs.return_documentation
      && lhs.property_documentation == rhs.property_documentation
      && lhs.type == rhs.type
      && lhs.is_beta == rhs.is_beta
      && lhs.is_protected == rhs.is_protected
      && lhs.is_static == rhs.is_static;
  }
  friend inline bool operator!=(function_def const& lhs, function_def const& rhs)
  {
    return !(lhs == rhs);
  }
  function_def() = default;
  function_def(klass_name _klass,
               type_def _return_type, std::string const& _name,
               std::vector<parameter_def> const& _parameters,
               std::string const& _c_name,
               std::string _filename,
               std::vector<std::string> const& _namespaces,
               documentation_def _documentation,
               documentation_def _return_documentation,
               documentation_def _property_documentation,
               function_type _type,
               bool _is_beta = false,
               bool _is_protected = false,
               Eolian_Unit const* unit = nullptr)
    : klass(_klass), return_type(_return_type), name(_name),
      parameters(_parameters), c_name(_c_name), filename(_filename),
      namespaces(_namespaces),
      documentation(_documentation),
      return_documentation(_return_documentation),
      property_documentation(_property_documentation),
      type(_type),
      is_beta(_is_beta), is_protected(_is_protected),
      unit(unit) {}

  function_def( ::Eolian_Function const* function, Eolian_Function_Type type, Eolian_Typedecl const* tp, Eolian_Unit const* unit)
    : return_type(void_), unit(unit)
  {
    Eolian_Type const* r_type = ::eolian_function_return_type_get(function, type);
    name = ::eolian_function_name_get(function);
    if(r_type)
      return_type.set(r_type, unit, EOLIAN_C_TYPE_RETURN);
     if(type == EOLIAN_METHOD || type == EOLIAN_FUNCTION_POINTER)
       {
          for(efl::eina::iterator<Eolian_Function_Parameter> param_iterator ( ::eolian_function_parameters_get(function))
            , param_last; param_iterator != param_last; ++param_iterator)
            {
               parameters.push_back({&*param_iterator, unit});
            }
       }
     else if(type == EOLIAN_PROP_GET || type == EOLIAN_PROP_SET)
       {
         if(type == EOLIAN_PROP_GET)
           name += "_get";
         else
           name += "_set";
         for(efl::eina::iterator<Eolian_Function_Parameter> param_iterator
               ( ::eolian_property_keys_get(function, type))
               , param_last; param_iterator != param_last; ++param_iterator)
           {
              parameters.push_back({&*param_iterator, unit});
           }
         std::vector<parameter_def> values;
         for(efl::eina::iterator<Eolian_Function_Parameter> param_iterator
               ( ::eolian_property_values_get(function, type))
               , param_last; param_iterator != param_last; ++param_iterator)
           {
              values.push_back({&*param_iterator, unit});
           }

         if(!r_type && type == EOLIAN_PROP_GET && values.size() == 1)
           {
             return_type = values[0].type;
           }
         else if(type == EOLIAN_PROP_GET)
           {
             for(auto&& v : values)
               {
                 v.direction = parameter_direction::out;
                 parameters.push_back(v);
               }
           }
         else
           parameters.insert(parameters.end(), values.begin(), values.end());
       }
     c_name = eolian_function_full_c_name_get(function, type, EINA_FALSE);
     if (type != EOLIAN_FUNCTION_POINTER)
       {
          const Eolian_Class *eolian_klass = eolian_function_class_get(function);
          filename = eolian_object_file_get((const Eolian_Object *)eolian_klass);
          klass = klass_name(eolian_klass,
              {attributes::qualifier_info::is_none, std::string()});
       }
     else
       {
          filename = "";

          if (tp)
            {
               for (efl::eina::iterator<const char> ns_iterator(::eolian_typedecl_namespaces_get(tp)), ns_last;
                    ns_iterator != ns_last;
                    ns_iterator++)
                 namespaces.push_back(&*ns_iterator);
            }
       }
     is_beta = eolian_function_is_beta(function);
     is_protected = eolian_function_scope_get(function, type) == EOLIAN_SCOPE_PROTECTED;
     is_static = eolian_function_is_class(function);

     return_documentation = eolian_function_return_documentation_get(function, type);

     Eolian_Implement const* implement = eolian_function_implement_get(function);
     if (!implement)
       return;

     documentation = eolian_implement_documentation_get(implement, type);


     if (type == EOLIAN_PROP_GET || type == EOLIAN_PROP_SET)
       property_documentation = eolian_implement_documentation_get(implement, EOLIAN_PROPERTY);

     switch (type)
       {
       case EOLIAN_UNRESOLVED:
         this->type = function_type::unresolved;
         break;
       case EOLIAN_PROPERTY:
         this->type = function_type::property;
         break;
       case EOLIAN_PROP_GET:
         this->type = function_type::prop_get;
         break;
       case EOLIAN_PROP_SET:
         this->type = function_type::prop_set;
         break;
       case EOLIAN_METHOD:
         this->type = function_type::method;
         break;
       case EOLIAN_FUNCTION_POINTER:
         this->type = function_type::function_pointer;
         break;
       }
  }

  std::string template_statement() const
  {
     std::string statement;
     char template_typename = 'F';
     for (auto const& param : this->parameters)
       {
          attributes::regular_type_def const* typ =
                efl::eina::get<attributes::regular_type_def>(&param.type.original_type);
          if (typ && typ->is_function_ptr())
            {
               char typenam[2] = { 0, };
               typenam[0] = template_typename++;
               if (statement.empty())
                 statement = std::string("template <typename ") + typenam;
               else
                 statement += std::string(", typename ") + typenam;
            }
       }
     if (statement.empty()) return statement;
     else return statement + ">";
  }

  std::vector<std::string> opening_statements() const
  {
     std::vector<std::string> statements;
     char template_typename = 'F';
     for (auto const& param : this->parameters)
       {
          attributes::regular_type_def const* typ =
                efl::eina::get<attributes::regular_type_def>(&param.type.original_type);
          if (typ && typ->is_function_ptr())
            {
               char typenam[2] = { 0, };
               typenam[0] = template_typename++;
               std::string statement = "auto fw_" + param.param_name + " = new ::efl::eolian::function_wrapper<";
               statement += param.type.c_type + ", " + typenam + ">(" + param.param_name + ");";
               statements.push_back(statement);
            }
       }
     return statements;
  }
};

template <>
struct tuple_element<0ul, function_def>
{
   typedef type_def type;
   static type& get(function_def& f) { return f.return_type; }
   static type const& get(function_def const& f) { return f.return_type; }
};

template <>
struct tuple_element<1ul, function_def>
{
   typedef std::string type;
   static type& get(function_def& f) { return f.name; }
   static type const& get(function_def const& f) { return f.name; }
};

template <>
struct tuple_element<2ul, function_def>
{
   typedef std::vector<parameter_def> type;
   static type& get(function_def& f) { return f.parameters; }
   static type const& get(function_def const& f) { return f.parameters; }
};

// template <int N>
// struct tuple_element<N, function_def const> : tuple_element<N, function_def> {};
// template <int N>
// struct tuple_element<N, function_def&> : tuple_element<N, function_def> {};
// template <int N>
// struct tuple_element<N, function_def const&> : tuple_element<N, function_def> {};

// template <std::size_t I>
// typename tuple_element<I, function_def>::type const&
// get(function_def const& f)
// {
//   return tuple_element<I, function_def>::get(f);
// }

// template <std::size_t I>
// typename tuple_element<I, function_def>::type&
// get(function_def& f)
// {
//   return tuple_element<I, function_def>::get(f);
// }

struct compare_klass_name_by_name
{
  bool operator()(klass_name const& lhs, klass_name const& rhs) const
  {
    return lhs.namespaces < rhs.namespaces
        || (!(rhs.namespaces < lhs.namespaces) && lhs.eolian_name < rhs.eolian_name);
  }
};

struct event_def
{
  klass_name klass;
  eina::optional<type_def> type;
  std::string name, c_name;
  bool beta, protect;
  documentation_def documentation;

  friend inline bool operator==(event_def const& lhs, event_def const& rhs)
  {
    return lhs.klass == rhs.klass
      && lhs.type == rhs.type
      && lhs.name == rhs.name
      && lhs.c_name == rhs.c_name
      && lhs.beta == rhs.beta
      && lhs.protect == rhs.protect
      && lhs.documentation == rhs.documentation;
  }
  friend inline bool operator!=(event_def const& lhs, event_def const& rhs)
  {
    return !(lhs == rhs);
  }

  event_def(klass_name _klass, type_def type, std::string name, std::string c_name,
          bool beta, bool protect, documentation_def documentation)
    : klass(_klass), type(type), name(name), c_name(c_name), beta(beta), protect(protect)
    , documentation(documentation) {}

  event_def(Eolian_Event const* event, Eolian_Class const* cls, Eolian_Unit const* unit)
    : klass(cls, {attributes::qualifier_info::is_none, std::string()})
    , type( ::eolian_event_type_get(event) ? eina::optional<type_def>{{::eolian_event_type_get(event), unit, EOLIAN_C_TYPE_DEFAULT}} : eina::optional<type_def>{})
    , name( ::eolian_event_name_get(event))
    , c_name( ::eolian_event_c_name_get(event))
    , beta( ::eolian_event_is_beta(event))
    , protect( ::eolian_event_scope_get(event) == EOLIAN_SCOPE_PROTECTED)
    , documentation( ::eolian_event_documentation_get(event)) {}
};

template <>
struct tuple_element<0, event_def>
{
  typedef eina::optional<type_def> type;
  static type& get(event_def& def) { return def.type; }
  static type const& get(event_def const& def) { return def.type; }
};
template <>
struct tuple_element<1, event_def>
{
  typedef std::string type;
  static type& get(event_def& def) { return def.name; }
  static type const& get(event_def const& def) { return def.name; }
};
template <>
struct tuple_element<2, event_def>
{
  typedef std::string type;
  static type& get(event_def& def) { return def.c_name; }
  static type const& get(event_def const& def) { return def.c_name; }
};
template <int N>
struct tuple_element<N, event_def const> : tuple_element<N, event_def> {};
template <int N>
auto get(event_def const& def) -> decltype(tuple_element<N, event_def>::get(def))
{
  return tuple_element<N, event_def>::get(def);
}
template <int N>
auto get(event_def& def) -> decltype(tuple_element<N, event_def>::get(def))
{
  return tuple_element<N, event_def>::get(def);
}

struct part_def
{
   klass_name klass;
   std::string name;
   documentation_def documentation;
   //bool beta, protect; // can it be applied??

   friend inline bool operator==(part_def const& lhs, part_def const& rhs)
   {
     return lhs.klass == rhs.klass
       && lhs.name == rhs.name;
   }
   friend inline bool operator!=(part_def const& lhs, part_def const& rhs)
   {
     return !(lhs == rhs);
   }
   friend inline bool operator<(part_def const& lhs, part_def const& rhs)
   {
      return lhs.name < rhs.name ||
            lhs.klass < rhs.klass;
   }

   part_def(Eolian_Part const* part, Eolian_Unit const*)
      : klass(klass_name(::eolian_part_class_get(part), {attributes::qualifier_info::is_none, std::string()}))
      , name(::eolian_part_name_get(part))
      , documentation(::eolian_part_documentation_get(part)) {}
};

inline Eolian_Class const* get_klass(klass_name const& klass_name_, Eolian_Unit const* unit);

struct klass_def
{
  std::string eolian_name;
  std::string cxx_name;
  std::string filename;
  documentation_def documentation;
  std::vector<std::string> namespaces;
  std::vector<function_def> functions;
  std::set<klass_name, compare_klass_name_by_name> inherits;
  class_type type;
  std::vector<event_def> events;
  std::set<klass_name, compare_klass_name_by_name> immediate_inherits;
  std::set<part_def> parts;
  Eolian_Unit const* unit;

  friend inline bool operator==(klass_def const& lhs, klass_def const& rhs)
  {
    return lhs.eolian_name == rhs.eolian_name
      && lhs.cxx_name == rhs.cxx_name
      && lhs.filename == lhs.filename
      && lhs.namespaces == rhs.namespaces
      && lhs.functions == rhs.functions
      && lhs.inherits == rhs.inherits
      && lhs.type == rhs.type
      && lhs.events == rhs.events
      && lhs.parts == rhs.parts;
  }
  friend inline bool operator!=(klass_def const& lhs, klass_def const& rhs)
  {
    return !(lhs == rhs);
  }
  friend inline bool operator<(klass_def const& lhs, klass_def const& rhs)
  {
     return lhs.eolian_name < rhs.eolian_name
       || lhs.cxx_name < rhs.cxx_name
       || lhs.namespaces < rhs.namespaces
       || lhs.parts < rhs.parts;
  }

  klass_def(std::string eolian_name, std::string cxx_name, std::string filename
            , documentation_def documentation
            , std::vector<std::string> namespaces
            , std::vector<function_def> functions
            , std::set<klass_name, compare_klass_name_by_name> inherits
            , class_type type
            , std::set<klass_name, compare_klass_name_by_name> immediate_inherits)
    : eolian_name(eolian_name), cxx_name(cxx_name), filename(filename)
    , documentation(documentation)
    , namespaces(namespaces)
    , functions(functions), inherits(inherits), type(type)
    , immediate_inherits(immediate_inherits)
  {}
  klass_def(std::string _eolian_name, std::string _cxx_name
            , std::vector<std::string> _namespaces
            , std::vector<function_def> _functions
            , std::set<klass_name, compare_klass_name_by_name> _inherits
            , class_type _type, Eolian_Unit const* unit)
    : eolian_name(_eolian_name), cxx_name(_cxx_name)
    , namespaces(_namespaces)
    , functions(_functions), inherits(_inherits), type(_type), unit(unit)
  {}
  klass_def(Eolian_Class const* klass, Eolian_Unit const* unit) : unit(unit)
  {
     for(efl::eina::iterator<const char> namespace_iterator( ::eolian_class_namespaces_get(klass))
           , namespace_last; namespace_iterator != namespace_last; ++namespace_iterator)
       {
          this->namespaces.push_back(&*namespace_iterator);
       }
     cxx_name = eolian_name = eolian_class_short_name_get(klass);
     filename = eolian_object_file_get((const Eolian_Object *)klass);
     for(efl::eina::iterator<Eolian_Function const> eolian_functions ( ::eolian_class_functions_get(klass, EOLIAN_PROPERTY))
       , functions_last; eolian_functions != functions_last; ++eolian_functions)
       {
         Eolian_Function const* function = &*eolian_functions;
         Eolian_Function_Type func_type = ::eolian_function_type_get(function);
         if(func_type == EOLIAN_PROPERTY)
           {
             try {
                if(! ::eolian_function_is_legacy_only(function, EOLIAN_PROP_GET)
                   && ::eolian_function_scope_get(function, EOLIAN_PROP_GET) != EOLIAN_SCOPE_PRIVATE)
                  functions.push_back({function, EOLIAN_PROP_GET, NULL,  unit});
             } catch(std::exception const&) {}
             try {
                if(! ::eolian_function_is_legacy_only(function, EOLIAN_PROP_SET)
                   && ::eolian_function_scope_get(function, EOLIAN_PROP_SET) != EOLIAN_SCOPE_PRIVATE)
                  functions.push_back({function, EOLIAN_PROP_SET, NULL, unit});
             } catch(std::exception const&) {}
           }
         else
           try {
             if(! ::eolian_function_is_legacy_only(function, func_type)
                && ::eolian_function_scope_get(function, func_type) != EOLIAN_SCOPE_PRIVATE)
               functions.push_back({function, func_type, NULL, unit});
           } catch(std::exception const&) {}
       }
     for(efl::eina::iterator<Eolian_Function const> eolian_functions ( ::eolian_class_functions_get(klass, EOLIAN_METHOD))
       , functions_last; eolian_functions != functions_last; ++eolian_functions)
       {
         try {
             Eolian_Function const* function = &*eolian_functions;
             Eolian_Function_Type func_type = eolian_function_type_get(function);
             if(! ::eolian_function_is_legacy_only(function, EOLIAN_METHOD)
                && ::eolian_function_scope_get(function, func_type) != EOLIAN_SCOPE_PRIVATE)
               functions.push_back({function, EOLIAN_METHOD, NULL, unit});
         } catch(std::exception const&) {}
       }
     if(::eolian_class_parent_get(klass))
       immediate_inherits.insert({::eolian_class_parent_get(klass), {}});
     for(efl::eina::iterator<Eolian_Class const> inherit_iterator ( ::eolian_class_extensions_get(klass))
           , inherit_last; inherit_iterator != inherit_last; ++inherit_iterator)
       {
         Eolian_Class const* inherit = &*inherit_iterator;
         immediate_inherits.insert({inherit, {}});
       }
     std::function<void(Eolian_Class const*)> inherit_algo =
       [&] (Eolian_Class const* inherit_klass)
       {
         if(::eolian_class_parent_get(inherit_klass))
           {
             Eolian_Class const* inherit = ::eolian_class_parent_get(inherit_klass);
             inherits.insert({inherit, {}});
             inherit_algo(inherit);
           }
         for(efl::eina::iterator<Eolian_Class const> inherit_iterator ( ::eolian_class_extensions_get(inherit_klass))
               , inherit_last; inherit_iterator != inherit_last; ++inherit_iterator)
           {
             Eolian_Class const* inherit = &*inherit_iterator;
             inherits.insert({inherit, {}});
             inherit_algo(inherit);
           }
       };
     inherit_algo(klass);

     for(efl::eina::iterator<Eolian_Part const> parts_itr ( ::eolian_class_parts_get(klass))
       , parts_last; parts_itr != parts_last; ++parts_itr)
       {
          parts.insert({&*parts_itr, unit});
       }

     switch(eolian_class_type_get(klass))
       {
       case EOLIAN_CLASS_REGULAR:
         type = class_type::regular;
         break;
       case EOLIAN_CLASS_ABSTRACT:
         type = class_type::abstract_;
         break;
       case EOLIAN_CLASS_MIXIN:
         type = class_type::mixin;
         break;
       case EOLIAN_CLASS_INTERFACE:
         type = class_type::interface_;
         break;
       default:
         throw std::runtime_error("Class with unknown type");
       }
     for(efl::eina::iterator<Eolian_Event const> event_iterator( ::eolian_class_events_get(klass))
           , event_last; event_iterator != event_last; ++event_iterator)
       {
         try {
           events.push_back({&*event_iterator, klass, unit});
         } catch(std::exception const&) {}
       }

     documentation = eolian_class_documentation_get(klass);
  }

  // TODO memoize the return?
  std::vector<function_def> get_all_methods() const
  {
      std::vector<function_def> ret;

      std::copy(functions.cbegin(), functions.cend(), std::back_inserter(ret));

      for (auto inherit : inherits)
       {
          klass_def klass(get_klass(inherit, unit), unit);
          std::copy(klass.functions.cbegin(), klass.functions.cend(),
                    std::back_inserter(ret));
       }

      return ret;
  }

  std::vector<part_def> get_all_parts() const
  {
      std::vector<part_def> ret;

      std::copy(parts.cbegin(), parts.cend(), std::back_inserter(ret));

      for (auto inherit : inherits)
        {
           klass_def klass(get_klass(inherit, unit), unit);
           std::copy(klass.parts.cbegin(), klass.parts.cend(),
                     std::back_inserter(ret));
        }

      return ret;
  }

  std::vector<event_def> get_all_events() const
  {
      std::vector<event_def> ret;

      std::copy(events.cbegin(), events.cend(), std::back_inserter(ret));

      for (auto inherit : inherits)
        {
           klass_def klass(get_klass(inherit, unit), unit);
           std::copy(klass.events.cbegin(), klass.events.cend(),
                     std::back_inserter(ret));
        }

      return ret;
  }
};

struct value_def
{
  typedef eina::variant<int> variant_type; // FIXME support other types
  variant_type value;
  std::string literal;
  type_def type;

  value_def() {}
  value_def(Eolian_Value value_obj)
  {
    type.set(value_obj.type);
    value = value_obj.value.i;
    literal = eolian_expression_value_to_literal(&value_obj);
  }
};

struct enum_value_def
{
  value_def value;
  std::string name;
  std::string c_name;
  documentation_def documentation;

  enum_value_def(Eolian_Enum_Type_Field const* enum_field, Eolian_Unit const*)
  {
      name = eolian_typedecl_enum_field_name_get(enum_field);
      c_name = eolian_typedecl_enum_field_c_name_get(enum_field);
      auto exp = eolian_typedecl_enum_field_value_get(enum_field, EINA_TRUE);
      value = eolian_expression_eval(exp, EOLIAN_MASK_INT); // FIXME hardcoded int
      documentation = eolian_typedecl_enum_field_documentation_get(enum_field);
  }
};

struct enum_def
{
  std::string eolian_name;
  std::string cxx_name;
  std::vector<std::string> namespaces;
  std::vector<enum_value_def> fields;
  documentation_def documentation;

  enum_def(Eolian_Typedecl const* enum_obj, Eolian_Unit const* unit)
  {
     for(efl::eina::iterator<const char> namespace_iterator( ::eolian_typedecl_namespaces_get(enum_obj))
           , namespace_last; namespace_iterator != namespace_last; ++namespace_iterator)
       {
          this->namespaces.push_back((&*namespace_iterator));
       }
     cxx_name = eolian_name = eolian_typedecl_short_name_get(enum_obj);

     for (efl::eina::iterator<const Eolian_Enum_Type_Field> field_iterator(::eolian_typedecl_enum_fields_get(enum_obj))
             , field_last; field_iterator != field_last; ++field_iterator)
       {
          // Fill the types
          enum_value_def field_def(&*field_iterator, unit);
          this->fields.push_back(field_def);
       }

     documentation = ::eolian_typedecl_documentation_get(enum_obj);
  }
};

struct struct_field_def
{
  type_def type;
  std::string name;
  documentation_def documentation;

  struct_field_def(Eolian_Struct_Type_Field const* struct_field, Eolian_Unit const* unit)
  {
     name = eolian_typedecl_struct_field_name_get(struct_field);
     try {
        type.set(eolian_typedecl_struct_field_type_get(struct_field), unit, EOLIAN_C_TYPE_DEFAULT);
     } catch(std::runtime_error const&) { /* Silently skip pointer fields*/ }
     documentation = ::eolian_typedecl_struct_field_documentation_get(struct_field);
  }

};

struct struct_def
{
  std::string eolian_name;
  std::string cxx_name;
  std::vector<std::string> namespaces;
  std::vector<struct_field_def> fields;
  documentation_def documentation;

  struct_def(Eolian_Typedecl const* struct_obj, Eolian_Unit const* unit)
  {
     for(efl::eina::iterator<const char> namespace_iterator( ::eolian_typedecl_namespaces_get(struct_obj))
           , namespace_last; namespace_iterator != namespace_last; ++namespace_iterator)
       {
          this->namespaces.push_back((&*namespace_iterator));
       }
     cxx_name = eolian_name = eolian_typedecl_short_name_get(struct_obj);

     for(efl::eina::iterator<const Eolian_Struct_Type_Field> field_iterator(::eolian_typedecl_struct_fields_get(struct_obj))
             , field_last; field_iterator != field_last; ++field_iterator)
       {
          struct_field_def field_def(&*field_iterator, unit);
          this->fields.push_back(field_def);
       }

     documentation = ::eolian_typedecl_documentation_get(struct_obj);
  }
};

inline klass_name get_klass_name(klass_def const& klass)
{
  return {klass.namespaces, klass.eolian_name, {qualifier_info::is_none, {}}, klass.type};
}

inline Eolian_Class const* get_klass(klass_name const& klass_name_, Eolian_Unit const* unit)
{
  std::string klass_name;
  if(!as_generator(*(string << ".") << string)
     .generate(std::back_insert_iterator<std::string>(klass_name)
               , std::make_tuple(klass_name_.namespaces, klass_name_.eolian_name)
               , context_null{}))
    return nullptr;
  else
    return ::eolian_unit_class_by_name_get(unit, klass_name.c_str());
}

inline std::vector<std::string> cpp_namespaces(std::vector<std::string> namespaces)
{
  if(namespaces.empty())
    namespaces.push_back("nonamespace");
  return namespaces;
}

inline bool has_events(klass_def const &klass)
{
    for (auto&& e : klass.events)
      {
         (void)e;
         return true;
      }

    for (auto&& c : klass.inherits)
      {
        attributes::klass_def parent(get_klass(c, klass.unit), klass.unit);
         for (auto&& e : parent.events)
           {
              (void)e;
              return true;
           }
      }

    return false;
}

template<typename T>
inline bool has_type_return(klass_def const &klass, T visitor)
{
    for (auto&& f : klass.functions)
      {
         if (f.return_type.original_type.visit(visitor))
           return true;
      }

    for (auto&& c : klass.inherits)
      {
        attributes::klass_def parent(get_klass(c, klass.unit), klass.unit);
         if (has_type_return(parent, visitor))
           return true;
      }

    return false;
}

struct string_return_visitor
{
    typedef string_return_visitor visitor_type;
    typedef bool result_type;
    template <typename T>
    bool operator()(T const&) const { return false; }
    bool operator()(regular_type_def const& regular) const
    {
        return regular.base_type == "string";
    }
};

struct stringshare_return_visitor
{
    typedef stringshare_return_visitor visitor_type;
    typedef bool result_type;
    template <typename T>
    bool operator()(T const&) const { return false; }
    bool operator()(regular_type_def const& regular) const
    {
        return regular.base_type == "stringshare";
    }
};

inline bool has_string_return(klass_def const &klass)
{
    return has_type_return(klass, string_return_visitor{});
}

inline bool has_stringshare_return(klass_def const &klass)
{
    return has_type_return(klass, stringshare_return_visitor{});
}

}
namespace type_traits {

template <>
struct is_tuple<attributes::parameter_def> : std::true_type {};
template <>
struct is_tuple<attributes::event_def> : std::true_type {};
  
}

} } }

#endif
