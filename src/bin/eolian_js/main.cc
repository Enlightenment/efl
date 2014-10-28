
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
       EINA_CXX_DOM_LOG_ERR(eolian::js::domain)
         << "Eolian failed parsing eot files";
       assert(false && "Error parsing eot files");
     }
   if (!::eolian_eo_file_parse(in_file.c_str()))
     {
       EINA_CXX_DOM_LOG_ERR(eolian::js::domain)
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
       EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "could not find any class defined in this eo file";
       return -1;
     }

   std::ofstream os (out_file.c_str());
   if(!os.is_open())
     {
       EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "Couldn't open output file " << out_file;
       return -1;
     }

   std::string class_name (name(klass)), upper_case_class_name(class_name)
     , lower_case_class_name(class_name);
   std::transform(upper_case_class_name.begin(), upper_case_class_name.end(), upper_case_class_name.begin()
                  , [] (unsigned char c) { return std::toupper(c); });
   std::transform(lower_case_class_name.begin(), lower_case_class_name.end(), lower_case_class_name.begin()
                  , [] (unsigned char c) { return std::tolower(c); });
   os << "#ifndef EFL_GENERATED_EOLIAN_CLASS_GUARD_" << upper_case_class_name << "_H\n";
   os << "#define EFL_GENERATED_EOLIAN_CLASS_GUARD_" << upper_case_class_name << "_H\n\n";


   os << "#ifdef HAVE_CONFIG_H\n";
   os << "#include \"config.h\"\n";
   os << "#endif\n";
   os << "extern \"C\"\n";
   os << "{\n";
   os << "#include <Efl.h>\n";
   os << "}\n";
   os << "#include <Eo_Js.hh>\n\n";
   os << "#include <Eo.h>\n\n";
   os << "#include <v8.h>\n\n";
   os << "extern \"C\" {\n";

   if(is_evas(klass))
     os << "#include <Evas.h>\n";
   
   os << "#include <" << eolian_class_file_get(klass) << ".h>\n\n";
   os << "}\n";

   os << "namespace ";
   print_lower_case_namespace(klass, os);
   os << " {\n";

   os << "EAPI void register_" << lower_case_class_name
      << "(v8::Handle<v8::Object> global, v8::Isolate* isolate)\n";
   os << "{\n";
   os << "  v8::Handle<v8::FunctionTemplate> constructor = v8::FunctionTemplate::New\n";
   os << "    (isolate, efl::eo::js::constructor, efl::eo::js::constructor_data(isolate, ";
   print_eo_class(klass, os);
   os  << "));\n";
   os << "  constructor->SetClassName(v8::String::NewFromUtf8(isolate, \""
      << class_name
      << "\"));\n";
   os << "  v8::Handle<v8::ObjectTemplate> instance = constructor->InstanceTemplate();\n";
   os << "  instance->SetInternalFieldCount(1);\n";

   efl::eina::iterator<Eolian_Function> first ( ::eolian_class_functions_get(klass, EOLIAN_METHOD) )
     , last;

   if(first != last)
     os << "  v8::Handle<v8::ObjectTemplate> prototype = constructor->PrototypeTemplate();\n";

   for(; first != last; ++first)
     {
       Eolian_Function const* function = &*first;
       os << "  prototype->Set( ::v8::String::NewFromUtf8(isolate, \""
          << eolian_function_name_get(function) << "\")\n"
          << "    , v8::FunctionTemplate::New(isolate, &efl::eo::js::call_function\n"
          << "    , efl::eo::js::call_function_data(isolate, & ::"
          << eolian_function_full_c_name_get(function) << ")));\n";
     }

   os << "  global->Set(v8::String::NewFromUtf8(isolate, \""
      << class_name << "\")"
      << ", constructor->GetFunction());\n";

   os << "}\n";

   for(std::size_t i = 0, j = namespace_size(klass); i != j; ++i)
     os << "}";
   os << "\n";
   os << "\n#endif\n\n";
}
