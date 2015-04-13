
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eolian.h>

#include <Eina.hh>

#include <eolian/js/domain.hh>
#include <eolian/class.hh>

#include <iostream>
#include <fstream>

#include <getopt.h>
#include <cstdlib>
#include <vector>
#include <set>

namespace eolian { namespace js {

efl::eina::log_domain domain("eina_cxx");

} }

int main(int argc, char** argv)
{
  std::vector<std::string> include_paths;
  std::string out_file, in_file;

  efl::eina::eina_init eina_init;
  struct eolian_init
  {
    eolian_init() { ::eolian_init(); }
    ~eolian_init() { ::eolian_shutdown(); }
  } eolian_init;
  
  const struct option long_options[] =
    {
       { "in",        required_argument, 0,  'I' },
       { "out-file",  required_argument, 0,  'o' },
       { "version",   no_argument,       0,  'v' },
       { "help",      no_argument,       0,  'h' },
       { 0,           0,                 0,   0  }
    };
   const char* options = "I:D:o:c:arvh";

   int c, idx;
   while ( (c = getopt_long(argc, argv, options, long_options, &idx)) != -1)
     {
        if (c == 'I')
          {
            include_paths.push_back(optarg);
          }
        else if (c == 'o')
          {
            if(!out_file.empty())
              {
                // _usage(argv[0]);
                return 1;
              }
            out_file = optarg;
          }
        else if (c == 'h')
          {
             // _usage(argv[0]);
            return 1;
          }
        else if (c == 'v')
          {
             // _print_version();
             // if (argc == 2) exit(EXIT_SUCCESS);
          }
     }

   if (optind == argc-1)
     {
        in_file = argv[optind];
     }


   for(auto src : include_paths)
     if (!::eolian_directory_scan(src.c_str()))
       {
         EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
           << "Couldn't load eolian from '" << src << "'.";
       }
   if (!::eolian_all_eot_files_parse())
     {
       EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
         << "Eolian failed parsing eot files";
       assert(false && "Error parsing eot files");
     }
   if (!::eolian_eo_file_parse(in_file.c_str()))
     {
       EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
         << "Failed parsing: " << in_file << ".";
       assert(false && "Error parsing input file");
     }

   const Eolian_Class *klass = NULL;
   {
     char* dup = strdup(in_file.c_str());
     char *bn = basename(dup);
     klass = ::eolian_class_get_by_file(bn);
     free(dup);
   }
   if(!klass)
     {
       EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "could not find any class defined in this eo file";
       return -1;
     }

   std::vector<Eolian_Function const*> constructor_functions;
   std::vector<Eolian_Function const*> normal_functions;

   std::set<Eolian_Class const*> classes;
   
   auto separate_functions = [&] (Eolian_Class const* klass, Eolian_Function_Type t
                                  , bool ignore_constructors)
     {
       efl::eina::iterator<Eolian_Function> first ( ::eolian_class_functions_get(klass, t) )
       , last;
       for(; first != last; ++first)
         {
           Eolian_Function const* function = &*first;
           if(eolian_function_scope_get(function) == EOLIAN_SCOPE_PUBLIC)
             {
               EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << ::eolian_function_full_c_name_get(function);
               if(strcmp("elm_obj_entry_input_panel_imdata_get", ::eolian_function_full_c_name_get(function))
                  != 0)
                 {
               if( ::eolian_function_is_constructor(function, klass))
                 {
                   if(!ignore_constructors)
                     {
                       EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "is a constructor";
                       constructor_functions.push_back(function);
                     }
                   else
                     {
                       EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "ignoring parent's constructors";
                     }
                 }
               else /*if( std::strcmp( ::eolian_function_full_c_name_get(function)
                      , "eo_parent") != 0)*/
                 {
                   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "is a NOT constructor "
                                                            << ::eolian_function_full_c_name_get(function);
                   normal_functions.push_back(function);
                 }
               // else
               //   {
               //     EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "parent_set as first constructor";
               //     constructor_functions.insert(constructor_functions.begin(), function);
               //     normal_functions.push_back(function);
               //   }
                 }
             }
         }
     };
   separate_functions(klass, EOLIAN_METHOD, false);
   separate_functions(klass, EOLIAN_PROPERTY, false);

   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "functions were separated";

   std::function<void(Eolian_Class const*, std::function<void(Eolian_Class const*)>)>
     recurse_inherits
     = [&] (Eolian_Class const* klass, std::function<void(Eolian_Class const*)> function)
     {
       for(efl::eina::iterator<const char> first ( ::eolian_class_inherits_get(klass))
             , last; first != last; ++first)
         {
           EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << &*first << std::endl;
           Eolian_Class const* base = ::eolian_class_get_by_name(&*first);
           function(base);
           recurse_inherits(base, function);
         }
     };
   
   std::function<void(Eolian_Class const*)> save_functions
     = [&] (Eolian_Class const* klass)
     {
       if(classes.find(klass) == classes.end())
         {
           classes.insert(klass);
           separate_functions(klass, EOLIAN_METHOD, true);
           separate_functions(klass, EOLIAN_PROPERTY, true);
         }
     };
   recurse_inherits(klass, save_functions);

   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "inherits were recursed";
   
   std::ofstream os (out_file.c_str());
   if(!os.is_open())
     {
       EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "Couldn't open output file " << out_file;
       return -1;
     }

   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "output was opened";
   
   std::string class_name (name(klass)), upper_case_class_name(class_name)
     , lower_case_class_name(class_name);
   std::transform(upper_case_class_name.begin(), upper_case_class_name.end(), upper_case_class_name.begin()
                  , [] (unsigned char c) { return std::toupper(c); });
   std::transform(lower_case_class_name.begin(), lower_case_class_name.end(), lower_case_class_name.begin()
                  , [] (unsigned char c) { return std::tolower(c); });

   if (getenv("EFL_RUN_IN_TREE"))
     {
       os << "#ifdef HAVE_CONFIG_H\n";
       os << "#include \"config.h\"\n";
       os << "#endif\n";

       os << "extern \"C\"\n";
       os << "{\n";
       os << "#include <Efl.h>\n";
       os << "}\n";
       os << "#include <Eo.h>\n\n";
     }
   else
     {
       os << "#ifdef HAVE_CONFIG_H\n";
       os << "#include \"elementary_config.h\"\n";
       os << "#endif\n";

       os << "extern \"C\"\n";
       os << "{\n";
       os << "#include <Efl.h>\n";
       os << "}\n";

       os << "#include <Eo.h>\n\n";
       os << "#include <Evas.h>\n\n";
       os << "#include <Edje.h>\n\n";


       os << "#include <Elementary.h>\n\n";
       os << "extern \"C\" {\n";
       os << "#include <elm_widget.h>\n";
       os << "}\n\n";
     }
   os << "#include <Eo_Js.hh>\n\n";
   os << "#include EINA_STRINGIZE(V8_INCLUDE_HEADER)\n\n";
   os << "extern \"C\" {\n";

   if(is_evas(klass))
     os << "#include <Evas.h>\n";

   auto includes_fun = [&os] (Eolian_Class const* klass)
     {
       os << "#include <" << eolian_class_file_get(klass) << ".h>\n\n";
     };
   recurse_inherits(klass, includes_fun);
   os << "#include <" << eolian_class_file_get(klass) << ".h>\n\n";

   os << "}\n";

   
   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "includes added";
   
   if(namespace_size(klass))
     {
       os << "namespace ";
       print_lower_case_namespace(klass, os);
       os << " {\n";
     }

   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "namespace";

   os << "static ::efl::eina::js::compatibility_persistent<v8::ObjectTemplate> persistent_instance;\n";
   os << "static ::efl::eina::js::compatibility_persistent<v8::Function> constructor_from_eo;\n";
   
   os << "EAPI v8::Local<v8::ObjectTemplate>\nregister_" << lower_case_class_name << "_from_constructor\n"
      << "(v8::Isolate* isolate, v8::Handle<v8::FunctionTemplate> constructor)\n";
   os << "{\n";

   os << "  v8::Local<v8::ObjectTemplate> instance = constructor->InstanceTemplate();\n";
   os << "  instance->SetInternalFieldCount(1);\n";

   os << "  v8::Handle<v8::ObjectTemplate> prototype = constructor->PrototypeTemplate();\n";

   for(auto function : normal_functions)
     {
       auto output_begin = [&] (std::string name)
         {
           if(! ::eolian_function_is_constructor(function, klass))
             os << "  prototype->Set( ::efl::eina::js::compatibility_new<v8::String>(isolate, \""
                << name << "\")\n"
                << "    , ::efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, &efl::eo::js::call_function\n"
                << "    , efl::eo::js::call_function_data<\n"
                << "      ::efl::eina::_mpl::tuple_c<std::size_t";
         };
       switch(eolian_function_type_get(function))
         {
         case EOLIAN_METHOD:
           output_begin(eolian_function_name_get(function));
           break;
         case EOLIAN_PROPERTY:
           output_begin(eolian_function_name_get(function) + std::string("_set"));
           break;
         // case EOLIAN_PROP_GET:
         //   output_begin(eolian_function_name_get(function) + std::string("_get"));
         //   break;
         // case EOLIAN_PROP_SET:
         //   output_begin(eolian_function_name_get(function) + std::string("_set"));
         case EOLIAN_PROP_GET:
         case EOLIAN_PROP_SET:
         default:
           continue;
         }
       // os << __func__ << ":" << __LINE__;
       std::size_t i = 0;
       for(efl::eina::iterator< ::Eolian_Function_Parameter> parameter
             ( ::eolian_function_parameters_get(function) )
             , last; parameter != last; ++parameter, ++i)
         {
           // os << __func__ << ":" << __LINE__;
           switch(eolian_parameter_direction_get(&*parameter))
             {
             case EOLIAN_IN_PARAM:
             case EOLIAN_INOUT_PARAM:
               os << ", " << i;
             default: break;
             }
         }
       // os << __func__ << ":" << __LINE__;
       os << ">\n      , ::efl::eina::_mpl::tuple_c<std::size_t";
       i = 0;
       for(efl::eina::iterator< ::Eolian_Function_Parameter> parameter
             ( ::eolian_function_parameters_get(function) )
             , last; parameter != last; ++parameter, ++i)
         {
           switch(eolian_parameter_direction_get(&*parameter))
             {
             case EOLIAN_OUT_PARAM:
             case EOLIAN_INOUT_PARAM:
               os << ", " << i;
             default: break;
             }
         }
       os << ">\n      , std::tuple<\n";
       efl::eina::iterator< ::Eolian_Function_Parameter> parameter
             ( ::eolian_function_parameters_get(function) )
         , last;
       // os << __func__ << ":" << __LINE__;
       while(parameter != last)
         {
           // os << __func__ << ":" << __LINE__;
           Eolian_Type const* type = eolian_parameter_type_get(&*parameter);
           if(eolian_type_is_own(type))
             os << "       ::std::true_type";
           else
             os << "       ::std::false_type";
           if(++parameter != last)
             os << ",\n";
         }
       // // os << __func__ << ":" << __LINE__;
       auto output_end = [&] (std::string const& name)
         {
           os << "> >(isolate, & ::" << name << ")));\n";
         };
       switch(eolian_function_type_get(function))
         {
         case EOLIAN_METHOD:
           output_end(eolian_function_full_c_name_get(function));
           break;
         case EOLIAN_PROPERTY:
           output_end(eolian_function_full_c_name_get(function) + std::string("_set"));
           break;
         case EOLIAN_PROP_SET:
           output_end(eolian_function_full_c_name_get(function) + std::string("_set"));
           break;
         case EOLIAN_PROP_GET:
           output_end(eolian_function_full_c_name_get(function) + std::string("_get"));
           break;
         case EOLIAN_UNRESOLVED:
           EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "Unresolved function";
           return -1;
           break;
         }
     }

   auto generate_events = [&] (Eolian_Class const* klass)
     {
       for(efl::eina::iterator< ::Eolian_Event> first ( ::eolian_class_events_get(klass))
             , last; first != last; ++first)
         {

           os << "  {\n";
           os << "    static efl::eo::js::event_information const event_information\n";
           os << "     = {&constructor_from_eo, ";
           os << eolian_event_c_name_get(&*first);
           os << "};\n\n";
           os << "    /* should include event " << ::eolian_event_name_get(&*first) << "*/" << std::endl;
           os << "    prototype->Set( ::efl::eina::js::compatibility_new<v8::String>(isolate, \"event_";
           std::string event_name (::eolian_event_name_get(&*first));
           std::replace(event_name.begin(), event_name.end(), ',', '_');
           os << event_name << "\")\n      , ::efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, &efl::eo::js::event_call\n"
              << "        , ::efl::eina::js::compatibility_new<v8::External>(isolate, const_cast<efl::eo::js::event_information*>"
              << "(&event_information)) ));\n";
           os << "  }\n\n";
           
         }
     };
   generate_events(klass);
   recurse_inherits(klass, generate_events);

   os << "  static_cast<void>(prototype); /* avoid warnings */\n";
   os << "  static_cast<void>(isolate); /* avoid warnings */\n";
   os << "  return instance;\n";
   os << "}\n\n";

   os << "EAPI void register_" << lower_case_class_name
      << "(v8::Handle<v8::Object> global, v8::Isolate* isolate)\n";
   os << "{\n";
   os << "  v8::Handle<v8::FunctionTemplate> constructor = ::efl::eina::js::compatibility_new<v8::FunctionTemplate>\n";
   os << "    (isolate, efl::eo::js::constructor\n"
      << "     , efl::eo::js::constructor_data(isolate\n"
         "         , ";

   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "before print eo_class";
   
   print_eo_class(klass, os);

   EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "print eo_class";
   
   for(auto function : constructor_functions)
     {
       os << "\n         , & ::"
          << eolian_function_full_c_name_get(function);
       if(eolian_function_type_get(function) == EOLIAN_PROPERTY)
         os << "_set";
     }
   
   os  << "));\n";

   os << "  register_" << lower_case_class_name << "_from_constructor(isolate, constructor);\n";

   os << "  constructor->SetClassName( ::efl::eina::js::compatibility_new<v8::String>(isolate, \""
      << class_name
      << "\"));\n";

   os << "  global->Set( ::efl::eina::js::compatibility_new<v8::String>(isolate, \""
      << class_name << "\")"
      << ", constructor->GetFunction());\n";


   os << "  {\n";
   os << "    v8::Handle<v8::FunctionTemplate> constructor = ::efl::eina::js::compatibility_new<v8::FunctionTemplate>\n";
   os << "      (isolate, &efl::eo::js::construct_from_eo);\n";
   os << "    constructor->SetClassName( ::efl::eina::js::compatibility_new<v8::String>(isolate, \""
      << class_name
      << "\"));\n";
   os << "    v8::Local<v8::ObjectTemplate> instance = "
      << "register_" << lower_case_class_name << "_from_constructor(isolate, constructor);\n";
   os << "    persistent_instance = {isolate, instance};\n";
   os << "    constructor_from_eo = {isolate, constructor->GetFunction()};\n";
   os << "  }\n";
   
   os << "}\n\n";
   
   for(std::size_t i = 0, j = namespace_size(klass); i != j; ++i)
     os << "}";
   os << "\n";


}
