
#ifndef EOLIAN_CXX_EO_TYPES_HH
#define EOLIAN_CXX_EO_TYPES_HH

#include <string>
#include <vector>

namespace efl { namespace eolian {

struct eo_constructor;
struct eo_parameter;
struct eo_function;
struct eo_event;

typedef std::vector<std::string> extensions_container_type;
typedef std::vector<eo_constructor> constructors_container_type;
typedef std::vector<eo_function> functions_container_type;
typedef std::vector<eo_parameter> parameters_container_type;
typedef std::vector<eo_event> events_container_type;

struct eo_generator_options
{
   std::vector<std::string> cxx_headers;
   std::vector<std::string> c_headers;
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
   return func.ret == "void" || func.ret == "";
}

struct eo_event
{
   std::string name;
   parameters_container_type params;
   bool is_hot;
   std::string comment;
};

} }

#endif // EFL_EOLIAN_CXX_EO_TYPES_HH
