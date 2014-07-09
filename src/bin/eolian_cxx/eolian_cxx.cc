
#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

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

#include "convert.hh"
#include "type_lookup.hh"

#include "convert.hh"
#include "eolian_wrappers.hh"
#include "safe_strings.hh"

namespace eolian_cxx {

/// Program options.
struct options_type
{
   std::vector<std::string> include_dirs;
   std::string in_file;
   std::string out_file;
   std::string out_dir;
   std::string classname;
   bool recurse;
   bool generate_all;

   options_type()
     : include_dirs()
     , in_file()
     , out_file()
     , out_dir()
     , classname()
     , recurse(false)
     , generate_all(false)
   {}
};

efl::eina::log_domain domain("eolian_cxx");

static bool
opts_check(eolian_cxx::options_type const& opts)
{
   if (!opts.generate_all && opts.in_file.empty())
     {
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
          << "Nothing to generate?" << std::endl;
     }
   else if (opts.generate_all && !opts.in_file.empty())
     {
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
          << "Didn't expect to receive input files (" << opts.in_file
          << ") with parameter -a."
          << std::endl;
     }
   else if (opts.generate_all && !opts.out_file.empty())
     {
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
          << "Can't use -a and -o together." << std::endl;
     }
   else if (opts.generate_all && opts.include_dirs.empty())
     {
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
          << "Option -a requires at least one include directory (-I)."
          << std::endl;
     }
   else
     {
        return true; // valid.
     }
   return false;
}

efl::eolian::eo_generator_options
generator_options(const Eolian_Class& klass)
{
   efl::eolian::eo_generator_options gen_opts;
   gen_opts.c_headers.push_back(class_base_file(klass) + ".h");

   void *cur = NULL;
   const Eina_List *itr, *inheritances = ::eolian_class_inherits_list_get(&klass);
   EINA_LIST_FOREACH(inheritances, itr, cur)
     {
        const Eolian_Class *ext = ::eolian_class_find_by_name(static_cast<const char*>(cur));
        std::string eo_parent_file = class_base_file(*ext);
        if (!eo_parent_file.empty())
          {
             // we have our own eo_base.hh
             std::string eo_base_eo = "eo_base.eo";
             if (eo_parent_file.length() < eo_base_eo.length() ||
                 !std::equal(eo_base_eo.begin(), eo_base_eo.end(),
                             eo_parent_file.end() - eo_base_eo.length()))
               {
                  gen_opts.cxx_headers.push_back(eo_parent_file + ".hh");
               }
          }
        else
          {
             EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
               << "Couldn't find source file for class '" << ext << "'"
               << std::endl;
          }
     }
   return gen_opts;
}

static bool
generate(const Eolian_Class& klass, eolian_cxx::options_type const& opts)
{
   efl::eolian::eo_class cls = eolian_cxx::convert_eolian_class(klass);
   efl::eolian::eo_generator_options gen_opts = generator_options(klass);
   std::string outname = opts.out_file.empty() ? (class_base_file(klass) + ".hh") : opts.out_file;
   if (!opts.out_dir.empty())
     {
        outname = opts.out_dir + "/" + outname;
     }
   if(opts.out_file == "-")
     {
        efl::eolian::generate(std::cout, cls, gen_opts);
     }
   else
     {
        std::ofstream outfile;
        outfile.open(outname);
        if (outfile.good())
          {
             efl::eolian::generate(outfile, cls, gen_opts);
             outfile.close();
          }
        else
          {
             EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
               << "Can't open output file: " << outname << std::endl;
             return false;
          }
     }
   return true;
}

static void
run(options_type const& opts)
{
   const Eolian_Class *klass = NULL;
   if (!opts.classname.empty())
     klass = class_from_name(opts.classname);
   else if (!opts.in_file.empty())
     klass = class_from_file(opts.in_file);
   if (klass)
     {
        if (!generate(*klass, opts))
          goto err;
     }
   else
     {
        auto classes = class_list_all();
        for (const Eolian_Class& c : classes)
          {
             if (!generate(c, opts))
               {
                  klass = &c;
                  goto err;
               }
          }
    }
   return;
 err:
   EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
     << "Error generating: " << class_name(*klass)
     << std::endl;
   std::abort();
}

static void
database_load(options_type const& opts)
{
   for (auto src : opts.include_dirs)
     {
        if (!::eolian_directory_scan(src.c_str()))
          {
             EINA_CXX_DOM_LOG_WARN(eolian_cxx::domain)
               << "Couldn't load eolian from '" << src << "'.";
          }
     }
   if (!getenv("EFL_RUN_IN_TREE"))
     {
        eolian_system_directory_scan();
     }
   if (!::eolian_all_eot_files_parse())
     {
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
          << "Eolian failed parsing eot files";
        std::abort();
     }
   if (!opts.in_file.empty())
     {
        if (!::eolian_eo_file_parse(opts.in_file.c_str()))
          {
             EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
               << "Failed parsing: " << opts.in_file << ".";
             std::abort();
          }
     }
   if (!::eolian_all_eo_files_parse())
     {
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
          << "Eolian failed parsing input files";
        std::abort();
     }
}

} // namespace eolian_cxx {

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
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain) <<
          "Option -" + option + " already set (" + value + ")";
     }
}

static eolian_cxx::options_type
opts_get(int argc, char **argv)
{
   eolian_cxx::options_type opts;

   const struct option long_options[] =
     {
       { "in",        required_argument, 0,  'I' },
       { "out-dir",   required_argument, 0,  'D' },
       { "out-file",  required_argument, 0,  'o' },
       { "class",     required_argument, 0,  'c' },
       { "all",       no_argument,       0,  'a' },
       { "recurse",   no_argument,       0,  'r' },
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
        else if (c == 'D')
          {
             _assert_not_dup("D", opts.out_dir);
             opts.out_dir = optarg;
          }
        else if (c == 'o')
          {
             _assert_not_dup("o", opts.out_file);
             opts.out_file = optarg;
          }
        else if (c == 'c')
          {
             _assert_not_dup("c", opts.classname);
             opts.classname = optarg;
          }
        else if (c == 'a')
          {
             opts.generate_all = true;
          }
        else if (c == 'r')
          {
             opts.recurse = true;
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

   if (!eolian_cxx::opts_check(opts))
     {
        _usage(argv[0]);
        std::abort();
     }

   return opts;
}

int main(int argc, char **argv)
{
   efl::eina::eina_init eina_init;
   efl::eolian::eolian_init eolian_init;
   eolian_cxx::options_type opts = opts_get(argc, argv);
   eolian_cxx::database_load(opts);
   eolian_cxx::run(opts);
   return 0;
}
