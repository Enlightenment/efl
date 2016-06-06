
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

#include "grammar/klass_def.hpp"
#include "grammar/header.hpp"
#include "grammar/impl_header.hpp"

namespace eolian_cxx {

/// Program options.
struct options_type
{
   std::vector<std::string> include_dirs;
   std::string in_file;
   std::string out_file;
};

efl::eina::log_domain domain("eolian_cxx");

static bool
opts_check(eolian_cxx::options_type const& opts)
{
   if (opts.in_file.empty())
     {
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
          << "Nothing to generate?" << std::endl;
     }
   else
     {
        return true; // valid.
     }
   return false;
}

static bool
generate(const Eolian_Class* klass, eolian_cxx::options_type const& opts)
{
   std::string header_decl_file_name = opts.out_file.empty()
     ? (::eolian_class_file_get(klass) + std::string(".hh")) : opts.out_file;

   std::string header_impl_file_name = header_decl_file_name;
   std::size_t dot_pos = header_impl_file_name.rfind(".hh");
   if (dot_pos != std::string::npos)
     header_impl_file_name.insert(dot_pos, ".impl");
   else
     header_impl_file_name.insert(header_impl_file_name.size(), ".impl");

   efl::eolian::grammar::attributes::klass_def klass_def(klass);
   std::vector<efl::eolian::grammar::attributes::klass_def> klasses{klass_def};

   std::set<std::string> c_headers;
   std::set<std::string> cpp_headers;
   c_headers.insert(eolian_class_file_get(klass) + std::string(".h"));
        
   std::function<void(efl::eolian::grammar::attributes::type_def const&)>
     variant_function;
   auto klass_name_function
     = [&] (efl::eolian::grammar::attributes::klass_name const& name)
     {
        Eolian_Class const* klass = get_klass(name);
        assert(klass);
        c_headers.insert(eolian_class_file_get(klass) + std::string(".h"));
        cpp_headers.insert(eolian_class_file_get(klass) + std::string(".hh"));
     };
   auto complex_function
     = [&] (efl::eolian::grammar::attributes::complex_type_def const& complex)
     {
       for(auto&& t : complex.subtypes)
         {
           variant_function(t);
         }
     };
   variant_function = [&] (efl::eolian::grammar::attributes::type_def const& type)
     {
       if(efl::eolian::grammar::attributes::klass_name const*
          name = efl::eolian::grammar::attributes::get<efl::eolian::grammar::attributes::klass_name>
          (&type.original_type))
         klass_name_function(*name);
       else if(efl::eolian::grammar::attributes::complex_type_def const*
              complex = efl::eolian::grammar::attributes::get<efl::eolian::grammar::attributes::complex_type_def>
               (&type.original_type))
         complex_function(*complex);
     };

   std::function<void(Eolian_Class const*)> klass_function
     = [&] (Eolian_Class const* klass)
     {
       for(efl::eina::iterator<const char> inherit_iterator ( ::eolian_class_inherits_get(klass))
             , inherit_last; inherit_iterator != inherit_last; ++inherit_iterator)
         {
           Eolian_Class const* inherit = ::eolian_class_get_by_name(&*inherit_iterator);
           c_headers.insert(eolian_class_file_get(inherit) + std::string(".h"));
           cpp_headers.insert(eolian_class_file_get(inherit) + std::string(".hh"));

           klass_function(inherit);
         }

       efl::eolian::grammar::attributes::klass_def klass_def(klass);
       for(auto&& f : klass_def.functions)
         {
           variant_function(f.return_type);
           for(auto&& p : f.parameters)
             {
               variant_function(p.type);
             }
         }
       for(auto&& e : klass_def.events)
         {
           if(e.type)
             variant_function(*e.type);
         }
     };
   klass_function(klass);
   
   cpp_headers.erase(eolian_class_file_get(klass) + std::string(".hh"));
   
   std::string guard_name;
   as_generator(*(efl::eolian::grammar::string << "_") << efl::eolian::grammar::string << "_EO_HH")
     .generate(std::back_insert_iterator<std::string>(guard_name)
               , std::make_tuple(klass_def.namespaces, klass_def.eolian_name)
               , efl::eolian::grammar::context_null{});

   std::tuple<std::string, std::set<std::string>&, std::set<std::string>&
              , std::vector<efl::eolian::grammar::attributes::klass_def>&
              , std::vector<efl::eolian::grammar::attributes::klass_def>&
              , std::vector<efl::eolian::grammar::attributes::klass_def>&
              , std::vector<efl::eolian::grammar::attributes::klass_def>&
              > attributes
   {guard_name, c_headers, cpp_headers, klasses, klasses, klasses, klasses};

   if(opts.out_file == "-")
     {
        std::ostream_iterator<char> iterator(std::cout);

        efl::eolian::grammar::class_header.generate(iterator, attributes, efl::eolian::grammar::context_null());
        std::endl(std::cout);

        efl::eolian::grammar::impl_header.generate(iterator, klasses, efl::eolian::grammar::context_null());

        std::endl(std::cout);
        std::flush(std::cout);
        std::flush(std::cerr);
     }
   else
     {
        std::ofstream header_decl;
        header_decl.open(header_decl_file_name);
        if (!header_decl.good())
          {
             EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
               << "Can't open output file: " << header_decl_file_name << std::endl;
             return false;
          }

        std::ofstream header_impl;
        header_impl.open(header_impl_file_name);
        if (!header_impl.good())
          {
             EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
               << "Can't open output file: " << header_impl_file_name << std::endl;
             return false;
          }

#if 1
        efl::eolian::grammar::class_header.generate
          (std::ostream_iterator<char>(header_decl), attributes, efl::eolian::grammar::context_null());

        efl::eolian::grammar::impl_header.generate
          (std::ostream_iterator<char>(header_impl), klasses, efl::eolian::grammar::context_null());
#else
        efl::eolian::generate(header_decl, header_impl, cls, gen_opts);
#endif

        header_impl.close();
        header_decl.close();
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
   EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
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
             EINA_CXX_DOM_LOG_WARN(eolian_cxx::domain)
               << "Couldn't load eolian from '" << src << "'.";
          }
     }
   if (!::eolian_all_eot_files_parse())
     {
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
          << "Eolian failed parsing eot files";
        assert(false && "Error parsing eot files");
     }
   if (opts.in_file.empty())
     {
       EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
         << "No input file.";
       assert(false && "Error parsing input file");
     }
   if (!::eolian_file_parse(opts.in_file.c_str()))
     {
       EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
         << "Failed parsing: " << opts.in_file << ".";
       assert(false && "Error parsing input file");
     }
   if (!::eolian_database_validate(EINA_FALSE))
     {
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
          << "Eolian failed validating database.";
        assert(false && "Error validating database");
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

   if (!eolian_cxx::opts_check(opts))
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
        eolian_cxx::options_type opts = opts_get(argc, argv);
        eolian_cxx::database_load(opts);
        eolian_cxx::run(opts);
     }
   catch(std::exception const& e)
     {
       std::cerr << "EOLCXX: Eolian C++ failed generation for the following reason: " << e.what() << std::endl;
       std::cout << "EOLCXX: Eolian C++ failed generation for the following reason: " << e.what() << std::endl;
       return -1;
     }
   return 0;
}
