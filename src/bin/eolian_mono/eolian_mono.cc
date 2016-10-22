
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

namespace eolian_mono {

/// Program options.
struct options_type
{
   std::vector<std::string> include_dirs;
   std::string in_file;
   std::string out_file;
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
   else
     {
        return true; // valid.
     }
   return false;
}

static bool
generate(const Eolian_Class* klass, eolian_mono::options_type const& opts)
{
   std::string class_file_name = opts.out_file.empty()
     ? (::eolian_class_file_get(klass) + std::string(".cs")) : opts.out_file;

   efl::eolian::grammar::attributes::klass_def klass_def(klass);
   std::vector<efl::eolian::grammar::attributes::klass_def> klasses{klass_def};

   if(opts.out_file == "-")
     {
        std::ostream_iterator<char> iterator(std::cout);

        eolian_mono::klass.generate
          (iterator, klass_def, efl::eolian::grammar::context_null());
        std::endl(std::cout);
     }
   else
     {
        std::ofstream class_def;
        class_def.open(class_file_name);
        if (!class_def.good())
          {
             EINA_CXX_DOM_LOG_ERR(eolian_mono::domain)
               << "Can't open output file: " << class_file_name << std::endl;
             return false;
          }

        eolian_mono::klass.generate
          (std::ostream_iterator<char>(class_def), klass_def, efl::eolian::grammar::context_null());

        class_def.close();
     }
   return true;
}

static void
run(options_type const& opts)
{
   const Eolian_Class *klass = NULL;
   char* dup = strdup(opts.in_file.c_str());
   char* base = basename(dup);
   klass = ::eolian_class_get_by_file(base);
   free(dup);
   if (klass)
     {
        if (!generate(klass, opts))
          goto err;
     }
   else
     {
        std::abort();
    }
   return;
 err:
   EINA_CXX_DOM_LOG_ERR(eolian_mono::domain)
     << "Error generating: " << ::eolian_class_name_get(klass)
     << std::endl;
   assert(false && "error generating class");
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


