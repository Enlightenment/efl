#ifndef EOLIAN_KLASS_HH
#define EOLIAN_KLASS_HH

#include <Eo.hh>
#include <Eina.hh>

#include <eolian/js/domain.hh>

#include <ostream>

inline std::string name(Eolian_Class const* klass)
{
  return ::eolian_class_short_name_get(klass);
}

inline std::string full_name(Eolian_Class const* klass)
{
  return ::eolian_class_name_get(klass);
}

inline std::string full_name_transformed(Eolian_Class const* klass)
{
  auto r = full_name(klass);
  std::replace(r.begin(), r.end(), '.', '_');
  return r;
}

inline std::size_t namespace_size(Eolian_Class const* klass)
{
   std::size_t size = 0;
   for(efl::eina::iterator<const char> first (::eolian_class_namespaces_get(klass))
         , last; first != last; ++first)
     ++size;
   return size;
}

inline std::string type_class_name(Eolian_Type const* tp)
{
   if (tp)
     {
        Eolian_Type_Type tpt = ::eolian_type_type_get(tp);
          {
             tp = ::eolian_type_aliased_base_get(tp);
             tpt = ::eolian_type_type_get(tp);
             if (tpt == EOLIAN_TYPE_CLASS)
               {
                  Eolian_Class const* klass = ::eolian_type_class_get(tp);
                  if (klass)
                    {
                       Eina_Stringshare* klass_name = ::eolian_class_name_get(klass);
                       if (!klass_name)
                         throw std::runtime_error("Could not get Eo class name");

                       return klass_name;
                    } // TODO: else should throw std::runtime_error("Could not get Eo class");
               }
             else if (tpt == EOLIAN_TYPE_REGULAR)
               {
                  auto tpd = eolian_type_typedecl_get(tp);
                  if (tpd && eolian_typedecl_type_get(tpd) == EOLIAN_TYPEDECL_STRUCT)
                    {
                       auto struct_type_full_name = ::eolian_type_name_get(tp);
                       if (!struct_type_full_name)
                         throw std::runtime_error("Could not get struct name");
                       return struct_type_full_name;
                    }
               }
          }
     }
   return "";
}

inline void print_lower_case_namespace(Eolian_Class const* klass, std::ostream& os)
{
  std::vector<std::string> namespace_;
   for(efl::eina::iterator<const char> first (::eolian_class_namespaces_get(klass))
         , last; first != last; ++first)
     namespace_.push_back(&*first);
   for(auto first = namespace_.begin(), last = namespace_.end()
         ; first != last; ++first)
     {
       std::string lower(*first);
       std::transform(lower.begin(), lower.end(), lower.begin(), tolower);
       os << lower;
       if(std::next(first) != last) os << "::";
     }
}

inline void print_eo_class(Eolian_Class const* klass, std::ostream& os)
{
   assert(klass != 0);
   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "print_eo_class";

   auto toupper = [] (unsigned char c) { return std::toupper(c); };
  
   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "print_eo_class";
   std::vector<std::string> namespace_;
   for(efl::eina::iterator<const char> first (::eolian_class_namespaces_get(klass))
         , last; first != last; ++first)
     namespace_.push_back(&*first);
   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "namespace";
   namespace_.push_back(name(klass));
   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "class";
   switch(eolian_class_type_get(klass))
     {
     case EOLIAN_CLASS_REGULAR:
     case EOLIAN_CLASS_ABSTRACT:
       EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "";
       namespace_.push_back("CLASS");
       break;
     case EOLIAN_CLASS_INTERFACE:
       EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "";
       namespace_.push_back("INTERFACE");
       break;
     case EOLIAN_CLASS_MIXIN:
       EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "";
       namespace_.push_back("MIXIN");
       break;
     default:
       EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "default ?";
       std::abort();
     }
   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "";
   for(auto first = namespace_.begin(), last = namespace_.end()
         ; first != last; ++first)
     {
       EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "";
       std::string upper(*first);
       std::transform(upper.begin(), upper.end(), upper.begin(), toupper);
       os << upper;
       if(std::next(first) != last) os << "_";
     }
   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "";
}

inline bool is_evas(Eolian_Class const* klass)
{
  EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "is_evas";
  efl::eina::iterator<const char> first (::eolian_class_namespaces_get(klass));
  return first != efl::eina::iterator<const char>()
    && std::strcmp(&*first, "Evas") == 0;
}

#endif
