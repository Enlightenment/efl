#ifndef EOLIAN_KLASS_HH
#define EOLIAN_KLASS_HH

#include <ostream>

inline std::string name(Eolian_Class const* klass)
{
  return ::eolian_class_name_get(klass);
}

inline std::size_t namespace_size(Eolian_Class const* klass)
{
   std::size_t size = 0;
   for(efl::eina::iterator<const char> first (::eolian_class_namespaces_get(klass))
         , last; first != last; ++first)
     ++size;
   return size;
}

inline void print_namespace(Eolian_Class const* klass, std::ostream& os)
{
  std::vector<std::string> namespace_;
   for(efl::eina::iterator<const char> first (::eolian_class_namespaces_get(klass))
         , last; first != last; ++first)
     namespace_.push_back(&*first);
   for(auto first = namespace_.begin(), last = namespace_.end()
         ; first != last; ++first)
     {
       os << *first;
       if(std::next(first) != last) os << "::";
     }
}

inline void print_eo_class(Eolian_Class const* klass, std::ostream& os)
{
  auto toupper = [] (unsigned char c) { return std::toupper(c); };
  
  std::vector<std::string> namespace_;
   for(efl::eina::iterator<const char> first (::eolian_class_namespaces_get(klass))
         , last; first != last; ++first)
     namespace_.push_back(&*first);
   namespace_.push_back(name(klass));
   namespace_.push_back("CLASS");
   for(auto first = namespace_.begin(), last = namespace_.end()
         ; first != last; ++first)
     {
       std::string upper(*first);
       std::transform(upper.begin(), upper.end(), upper.begin(), toupper);
       os << upper;
       if(std::next(first) != last) os << "_";
     }
}

#endif
