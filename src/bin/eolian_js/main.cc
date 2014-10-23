
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

   std::string class_name = name(*klass);
   std::transform(class_name.begin(), class_name.end(), class_name.begin()
                  , [] (unsigned char c) { return std::toupper(c); });
   os << "#ifndef EFL_GENERATED_EOLIAN_CLASS_GUARD_" << class_name << "_H\n";
   os << "#define EFL_GENERATED_EOLIAN_CLASS_GUARD_" << class_name << "_H\n\n";

   std::vector<std::string> namespace_;
   
   for(efl::eina::iterator<const char> first (::eolian_class_namespaces_get(klass))
         , last; first != last; ++first)
     namespace_.push_back(&*first);
 
   os << "namespace ";
   for(auto first = namespace_.begin()
         , last = namespace_.end(); first != last; ++first)
     {
       os << *first;
       if(std::next(first) != last) os << "::";
     }
   os << " {\n";

   std::vector<Eolian_Function const*> functions;

   for(efl::eina::iterator<Eolian_Function> first
         ( ::eolian_class_functions_get(klass, EOLIAN_METHOD) )
         , last; first != last; ++first)
     functions.push_back(&*first);

}
