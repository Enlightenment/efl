
#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <libgen.h>

#include <string>
#include <algorithm>
#include <stdexcept>
#include <iosfwd>
#include <type_traits>
#include <cassert>

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <Eolian.h>

#include <Eina.hh>
#include <Eolian_Cxx.hh>

#include <eolian_mono/klass.hh>
#include <eolian_mono/enum_definition.hh>
#include <eolian_mono/type_impl.hh>
#include <eolian_mono/marshall_type_impl.hh>
#include <eolian_mono/marshall_annotation.hh>

namespace eolian_mono {

/// Program options.
struct options_type
{
   std::vector<std::string> include_dirs;
   std::string in_file;
   std::string out_file;
   std::string dllimport;
};

efl::eina::log_domain domain("eolian_mono");

static bool
opts_check(eolian_mono::options_type const& opts)
{
   if (opts.in_file.empty())
     {
        EINA_CXX_DOM_LOG_ERR(eolian_mono::domain)
          << "Nothing to generate?" << std::endl;
     }
   else if (opts.out_file.empty())
     {
        EINA_CXX_DOM_LOG_ERR(eolian_mono::domain)
          << "Nowhere to generate?" << std::endl;
     }
   else
     return true; // valid.
   return false;
}

static void
run(options_type const& opts)
{
   const Eolian_Class *klass = NULL;
   char* dup = strdup(opts.in_file.c_str());
   std::string basename_input = basename(dup);
   klass = ::eolian_class_get_by_file(basename_input.c_str());
   free(dup);

   std::string class_file_name = opts.out_file;

   std::ofstream output_file;
   std::ostream_iterator<char> iterator
     {[&]
     {
       if(opts.out_file == "-")
         return std::ostream_iterator<char>(std::cout);
       else
         {
           output_file.open(class_file_name);
           if (!output_file.good())
             {
               EINA_CXX_DOM_LOG_ERR(eolian_mono::domain)
                 << "Can't open output file: " << class_file_name << std::endl;
               throw std::runtime_error("");
             }
           return std::ostream_iterator<char>(output_file);
         }
     }()};

   as_generator("using System;\nusing System.Runtime.InteropServices;\n")
     .generate(iterator, efl::eolian::grammar::attributes::unused, efl::eolian::grammar::context_null());
   
   if (klass)
     {
       efl::eolian::grammar::attributes::klass_def klass_def(klass);
       std::vector<efl::eolian::grammar::attributes::klass_def> klasses{klass_def};

       eolian_mono::klass
         .generate(iterator, klass_def, efl::eolian::grammar::context_cons<eolian_mono::library_context>({opts.dllimport}));
     }
   //else
     {
       for (efl::eina::iterator<const Eolian_Typedecl> enum_iterator( ::eolian_typedecl_enums_get_by_file(basename_input.c_str()))
               , enum_last; enum_iterator != enum_last; ++enum_iterator)
         {
            efl::eolian::grammar::attributes::enum_def enum_(&*enum_iterator);
            eolian_mono::enum_definition.generate(iterator, enum_, efl::eolian::grammar::context_null());
         }

       for (efl::eina::iterator<const Eolian_Typedecl> struct_iterator( ::eolian_typedecl_structs_get_by_file(basename_input.c_str()))
               , struct_last; struct_iterator != struct_last; ++struct_iterator)
         {
            efl::eolian::grammar::attributes::struct_def struct_(&*struct_iterator);
         }

    }
}

static void
database_load(options_type const& opts)
{
   for (auto src : opts.include_dirs)
     {
        if (!::eolian_directory_scan(src.c_str()))
          {
             EINA_CXX_DOM_LOG_WARN(eolian_mono::domain)
               << "Couldn't load eolian from '" << src << "'.";
          }
     }
   if (!::eolian_all_eot_files_parse())
     {
        EINA_CXX_DOM_LOG_ERR(eolian_mono::domain)
          << "Eolian failed parsing eot files";
        assert(false && "Error parsing eot files");
     }
   if (opts.in_file.empty())
     {
       EINA_CXX_DOM_LOG_ERR(eolian_mono::domain)
         << "No input file.";
       assert(false && "Error parsing input file");
     }
   if (!::eolian_file_parse(opts.in_file.c_str()))
     {
       EINA_CXX_DOM_LOG_ERR(eolian_mono::domain)
         << "Failed parsing: " << opts.in_file << ".";
       assert(false && "Error parsing input file");
     }
   if (!::eolian_database_validate(EINA_FALSE))
     {
        EINA_CXX_DOM_LOG_ERR(eolian_mono::domain)
          << "Eolian failed validating database.";
        assert(false && "Error validating database");
     }
}

} // namespace eolian_mono {

static void
_print_version()
{
   std::cerr
     << "Eolian C++ Binding Generator (EFL "
     << PACKAGE_VERSION << ")" << std::endl;
}

static void
_usage(const char *progname)
{
   std::cerr
     << progname
     << " [options] [file.eo]" << std::endl
     << " A single input file must be provided (unless -a is specified)." << std::endl
     << "Options:" << std::endl
     << "  -a, --all               Generate bindings for all Eo classes." << std::endl
     << "  -c, --class <name>      The Eo class name to generate code for." << std::endl
     << "  -D, --out-dir <dir>     Output directory where generated code will be written." << std::endl
     << "  -I, --in <file/dir>     The source containing the .eo descriptions." << std::endl
     << "  -o, --out-file <file>   The output file name. [default: <classname>.eo.hh]" << std::endl
     << "  -n, --namespace <ns>    Wrap generated code in a namespace. [Eg: efl::ecore::file]" << std::endl
     << "  -r, --recurse           Recurse input directories loading .eo files." << std::endl
     << "  -v, --version           Print the version." << std::endl
     << "  -h, --help              Print this help." << std::endl;
   exit(EXIT_FAILURE);
}

static void
_assert_not_dup(std::string option, std::string value)
{
   if (value != "")
     {
        EINA_CXX_DOM_LOG_ERR(eolian_mono::domain) <<
          "Option -" + option + " already set (" + value + ")";
     }
}

static eolian_mono::options_type
opts_get(int argc, char **argv)
{
   eolian_mono::options_type opts;

   const struct option long_options[] =
     {
       { "in",        required_argument, 0,  'I' },
       { "out-file",  required_argument, 0,  'o' },
       { "version",   no_argument,       0,  'v' },
       { "help",      no_argument,       0,  'h' },
       { "dllimport",      required_argument,       0,  'l' },
       { 0,           0,                 0,   0  }
     };
   const char* options = "I:D:o:c:arvh";

   int c, idx;
   while ( (c = getopt_long(argc, argv, options, long_options, &idx)) != -1)
     {
        if (c == 'I')
          {
             opts.include_dirs.push_back(optarg);
          }
        else if (c == 'o')
          {
             _assert_not_dup("o", opts.out_file);
             opts.out_file = optarg;
          }
        else if (c == 'h')
          {
           _usage(argv[0]);
          }
        else if (c == 'l')
          {
            opts.dllimport = optarg;
          }
        else if (c == 'v')
          {
             _print_version();
             if (argc == 2) exit(EXIT_SUCCESS);
          }
     }
   if (optind == argc-1)
     {
        opts.in_file = argv[optind];
     }

   if (!eolian_mono::opts_check(opts))
     {
        _usage(argv[0]);
        assert(false && "Wrong options passed in command-line");
     }

   return opts;
}

int main(int argc, char **argv)
{
   try
     {
        efl::eina::eina_init eina_init;
        efl::eolian::eolian_init eolian_init;
        eolian_mono::options_type opts = opts_get(argc, argv);
        eolian_mono::database_load(opts);
        eolian_mono::run(opts);
     }
   catch(std::exception const& e)
     {
       std::cerr << "EOLCXX: Eolian C++ failed generation for the following reason: " << e.what() << std::endl;
       std::cout << "EOLCXX: Eolian C++ failed generation for the following reason: " << e.what() << std::endl;
       return -1;
     }
   return 0;
}


