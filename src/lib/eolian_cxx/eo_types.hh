
#ifndef EOLIAN_CXX_EO_TYPES_HH
#define EOLIAN_CXX_EO_TYPES_HH

#include <algorithm>
#include <string>
#include <vector>
#include <cassert>

namespace efl { namespace eolian {

struct eo_constructor;
struct eo_parameter;
struct eo_function;
struct eo_event;

typedef std::vector<std::string> extensions_container_type;
typedef std::vector<std::string> includes_container_type;
typedef std::vector<eo_constructor> constructors_container_type;
typedef std::vector<eo_function> functions_container_type;
typedef std::vector<eo_parameter> parameters_container_type;
typedef std::vector<eo_event> events_container_type;

struct eo_generator_options
{
   includes_container_type cxx_headers;
   includes_container_type c_headers;
};

struct eo_class
{
   enum eo_class_type
     {
       regular_, regular_noninst_, interface_, mixin_
     };

   eo_class_type type;
   std::string name;
   std::string eo_name;
   std::string parent;
   extensions_container_type extensions;
   constructors_container_type constructors;
   functions_container_type functions;
   events_container_type events;
   std::string comment;
   std::string name_space;
};

struct eo_parameter
{
   std::string type;
   std::string name;
};

struct eo_constructor
{
   std::string name;
   parameters_container_type params;
   std::string comment;
};

struct eo_function
{
   enum eo_function_type
     {
       regular_, class_
     };
   eo_function_type type;
   std::string name;
   std::string impl;
   std::string ret;
   parameters_container_type params;
   std::string comment;
};

inline bool
function_is_void(eo_function const& func)
{
   return func.ret.empty() || func.ret.compare("void") == 0;
}

inline bool
function_is_static(eo_function const& func)
{
   return func.type == eo_function::class_;
}

struct eo_event
{
   std::string name;
   std::string eo_name;
   std::string comment;
};


// XXX mocked implementation. waiting for complex types...
typedef std::string eolian_type_instance;
typedef std::string eolian_type;
inline bool
type_is_void(eolian_type_instance const& type)
{
   return type.empty() || type.compare("void") == 0;
}

} }

namespace eolian_cxx {
inline efl::eolian::eolian_type
type_to_native(efl::eolian::eolian_type const& type)
{
   return type;
}
}

#endif // EFL_EOLIAN_CXX_EO_TYPES_HH
