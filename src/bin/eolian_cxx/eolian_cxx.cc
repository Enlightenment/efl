
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

#include "grammar/klass_def.hpp"
#include "grammar/header.hpp"
#include "grammar/impl_header.hpp"
#include "grammar/types_definition.hpp"

namespace eolian_cxx {

/// Program options.
struct options_type
{
   std::vector<std::string> include_dirs;
   std::vector<std::string> in_files;
   mutable Eolian_State* state;
   mutable Eolian_Unit const* unit;
   std::string out_file;
   bool main_header;

   options_type() : main_header(false) {}
   ~options_type()
     {
        eolian_state_free(state);
     }
};

static efl::eina::log_domain domain("eolian_cxx");

static bool
opts_check(eolian_cxx::options_type const& opts)
{
   if (opts.in_files.empty())
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
generate(const Eolian_Class* klass, eolian_cxx::options_type const& opts,
         std::string const& cpp_types_header)
{
   std::string header_decl_file_name = opts.out_file.empty()
     ? (::eolian_object_file_get((const Eolian_Object *)klass) + std::string(".hh")) : opts.out_file;

   std::string header_impl_file_name = header_decl_file_name;
   std::size_t dot_pos = header_impl_file_name.rfind(".hh");
   if (dot_pos != std::string::npos)
     header_impl_file_name.insert(dot_pos, ".impl");
   else
     header_impl_file_name.insert(header_impl_file_name.size(), ".impl");

   efl::eolian::grammar::attributes::klass_def klass_def(klass, opts.unit);
   std::vector<efl::eolian::grammar::attributes::klass_def> klasses{klass_def};
   std::set<efl::eolian::grammar::attributes::klass_def> forward_klasses{};

   std::set<std::string> c_headers;
   std::set<std::string> cpp_headers;
   c_headers.insert(eolian_object_file_get((const Eolian_Object *)klass) + std::string(".h"));
        
   std::function<void(efl::eolian::grammar::attributes::type_def const&)>
     variant_function;
   auto klass_name_function
     = [&] (efl::eolian::grammar::attributes::klass_name const& name)
     {
        Eolian_Class const* klass2 = get_klass(name, opts.unit);
        assert(klass2);
        c_headers.insert(eolian_object_file_get((const Eolian_Object *)klass2) + std::string(".h"));
        cpp_headers.insert(eolian_object_file_get((const Eolian_Object *)klass2) + std::string(".hh"));
        efl::eolian::grammar::attributes::klass_def cls{klass2, opts.unit};
        forward_klasses.insert(cls);
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
          name = efl::eina::get<efl::eolian::grammar::attributes::klass_name>
          (&type.original_type))
         klass_name_function(*name);
       else if(efl::eolian::grammar::attributes::complex_type_def const*
              complex = efl::eina::get<efl::eolian::grammar::attributes::complex_type_def>
               (&type.original_type))
         complex_function(*complex);
     };

   std::function<void(Eolian_Class const*)> klass_function
     = [&] (Eolian_Class const* klass2)
     {
       if(::eolian_class_parent_get(klass2))
         {
           Eolian_Class const* inherit = ::eolian_class_parent_get(klass2);
           c_headers.insert(eolian_object_file_get((const Eolian_Object *)inherit) + std::string(".h"));
           cpp_headers.insert(eolian_object_file_get((const Eolian_Object *)inherit) + std::string(".hh"));
           efl::eolian::grammar::attributes::klass_def klass3{inherit, opts.unit};
           forward_klasses.insert(klass3);

           klass_function(inherit);
         }
       for(efl::eina::iterator<Eolian_Class const> inherit_iterator ( ::eolian_class_extensions_get(klass2))
             , inherit_last; inherit_iterator != inherit_last; ++inherit_iterator)
         {
           Eolian_Class const* inherit = &*inherit_iterator;
           c_headers.insert(eolian_object_file_get((const Eolian_Object *)inherit) + std::string(".h"));
           cpp_headers.insert(eolian_object_file_get((const Eolian_Object *)inherit) + std::string(".hh"));
           efl::eolian::grammar::attributes::klass_def klass3{inherit, opts.unit};
           forward_klasses.insert(klass3);

           klass_function(inherit);
         }

       efl::eolian::grammar::attributes::klass_def klass2_def(klass2, opts.unit);
       for(auto&& f : klass2_def.functions)
         {
           variant_function(f.return_type);
           for(auto&& p : f.parameters)
             {
               variant_function(p.type);
             }
         }
       for(auto&& e : klass2_def.events)
         {
           if(e.type)
             variant_function(*e.type);
         }
     };
   klass_function(klass);

   for(efl::eina::iterator<Eolian_Part const> parts_itr ( ::eolian_class_parts_get(klass))
     , parts_last; parts_itr != parts_last; ++parts_itr)
     {
        Eolian_Class const* eolian_part_klass = ::eolian_part_class_get(&*parts_itr);
        efl::eolian::grammar::attributes::klass_def part_klass(eolian_part_klass, opts.unit);
        forward_klasses.insert(part_klass);
     }

   cpp_headers.erase(eolian_object_file_get((const Eolian_Object *)klass) + std::string(".hh"));

   std::string guard_name;
   as_generator(*(efl::eolian::grammar::string << "_") << efl::eolian::grammar::string << "_EO_HH")
     .generate(std::back_insert_iterator<std::string>(guard_name)
               , std::make_tuple(klass_def.namespaces, klass_def.eolian_name)
               , efl::eolian::grammar::context_null{});

   std::tuple<std::string, std::set<std::string>&, std::set<std::string>&
              , std::vector<efl::eolian::grammar::attributes::klass_def>&
              , std::set<efl::eolian::grammar::attributes::klass_def> const&
              , std::string const&
              , std::vector<efl::eolian::grammar::attributes::klass_def>&
              , std::vector<efl::eolian::grammar::attributes::klass_def>&
              > attributes
   {guard_name, c_headers, cpp_headers, klasses, forward_klasses, cpp_types_header, klasses, klasses};

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

        efl::eolian::grammar::class_header.generate
          (std::ostream_iterator<char>(header_decl), attributes, efl::eolian::grammar::context_null());

        efl::eolian::grammar::impl_header.generate
          (std::ostream_iterator<char>(header_impl), klasses, efl::eolian::grammar::context_null());

        header_impl.close();
        header_decl.close();
     }
   return true;
}

static bool
types_generate(std::string const& fname, options_type const& opts,
               std::string& cpp_types_header)
{
   using namespace efl::eolian::grammar::attributes;

   std::vector<function_def> functions;
   Eina_Iterator *itr = eolian_state_objects_by_file_get(opts.state, fname.c_str());
   /* const */ Eolian_Object *decl;

   // Build list of functions with their parameters
   while(::eina_iterator_next(itr, reinterpret_cast<void**>(&decl)))
     {
        Eolian_Object_Type dt = eolian_object_type_get(decl);
        if (dt != EOLIAN_OBJECT_TYPEDECL)
          continue;

        const Eolian_Typedecl *tp = (const Eolian_Typedecl *)decl;

        if (eolian_typedecl_is_extern(tp))
          continue;

        if (::eolian_typedecl_type_get(tp) != EOLIAN_TYPEDECL_FUNCTION_POINTER)
          continue;

        const Eolian_Function *func = eolian_typedecl_function_pointer_get(tp);
        if (!func) return false;

        function_def def(func, EOLIAN_FUNCTION_POINTER, tp, opts.unit);
        def.c_name = eolian_typedecl_name_get(tp);
        std::replace(def.c_name.begin(), def.c_name.end(), '.', '_');
        functions.push_back(std::move(def));
     }
   ::eina_iterator_free(itr);

   if (functions.empty())
     return true;

   std::stringstream sink;

   sink.write("\n", 1);
   if (!efl::eolian::grammar::types_definition
       .generate(std::ostream_iterator<char>(sink),
                 functions, efl::eolian::grammar::context_null()))
     return false;

   cpp_types_header = sink.str();

   return true;
}

static void
run(options_type const& opts)
{
   if(!opts.main_header)
     {
       const Eolian_Class *klass = nullptr;
       char* dup = strdup(opts.in_files[0].c_str());
       char* base = basename(dup);
       std::string cpp_types_header;
       opts.unit = (Eolian_Unit*)opts.state;
       klass = ::eolian_state_class_by_file_get(opts.state, base);
       free(dup);
       if (klass)
         {
           if (!types_generate(base, opts, cpp_types_header) ||
               !generate(klass, opts, cpp_types_header))
             {
               EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
                 << "Error generating: " << ::eolian_class_short_name_get(klass)
                 << std::endl;
               assert(false && "error generating class");
             }
         }
       else
         {
           std::abort();
         }
     }
   else
     {
       std::set<std::string> headers;
       std::set<std::string> eo_files;

       for(auto&& name : opts.in_files)
         {
           Eolian_Unit const* unit = ::eolian_state_file_path_parse(opts.state, name.c_str());
           if(!unit)
             {
               EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
                 << "Failed parsing: " << name << ".";
             }
           else
             {
               if(!opts.unit)
                 opts.unit = unit;
             }
           char* dup = strdup(name.c_str());
           char* base = basename(dup);
           Eolian_Class const* klass = ::eolian_state_class_by_file_get(opts.state, base);
           free(dup);
           if (klass)
             {
               std::string filename = eolian_object_file_get((const Eolian_Object *)klass);
               headers.insert(filename + std::string(".hh"));
               eo_files.insert(filename);
             }
         }

       using efl::eolian::grammar::header_include_directive;
       using efl::eolian::grammar::implementation_include_directive;

       auto main_header_grammar =
         *header_include_directive // sequence<string>
         << *implementation_include_directive // sequence<string>
         ;

       std::tuple<std::set<std::string>&, std::set<std::string>&> attributes{headers, eo_files};

       std::ofstream main_header;
       main_header.open(opts.out_file);
       if (!main_header.good())
         {
           EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
             << "Can't open output file: " << opts.out_file << std::endl;
           return;
         }
       
       main_header_grammar.generate(std::ostream_iterator<char>(main_header)
                                    , attributes, efl::eolian::grammar::context_null());
     }
}

static void
state_init(options_type const& opts)
{
   Eolian_State *eos = ::eolian_state_new();
   if (!eos)
     {
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
          << "Eolian failed creating state";
        assert(false && "Error creating Eolian state");
     }
   opts.state = eos;
}

static void
database_load(options_type const& opts)
{
   for (auto src : opts.include_dirs)
     {
        if (!::eolian_state_directory_add(opts.state, src.c_str()))
          {
             EINA_CXX_DOM_LOG_WARN(eolian_cxx::domain)
               << "Couldn't load eolian from '" << src << "'.";
          }
     }
   if (!::eolian_state_all_eot_files_parse(opts.state))
     {
        EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
          << "Eolian failed parsing eot files";
        assert(false && "Error parsing eot files");
     }
   if (opts.in_files.empty())
     {
       EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
         << "No input file.";
       assert(false && "Error parsing input file");
     }
   if (!opts.main_header && !::eolian_state_file_path_parse(opts.state, opts.in_files[0].c_str()))
     {
       EINA_CXX_DOM_LOG_ERR(eolian_cxx::domain)
         << "Failed parsing: " << opts.in_files[0] << ".";
       assert(false && "Error parsing input file");
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
       { "in",          required_argument, nullptr, 'I' },
       { "out-file",    required_argument, nullptr, 'o' },
       { "version",     no_argument,       nullptr, 'v' },
       { "help",        no_argument,       nullptr, 'h' },
       { "main-header", no_argument,       nullptr, 'm' },
       { nullptr,       0,                 nullptr,  0  }
     };
   const char* options = "I:D:o:c::marvh";

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
        else if(c == 'm')
          {
            opts.main_header = true;
          }
        else if (c == 'v')
          {
             _print_version();
             if (argc == 2) exit(EXIT_SUCCESS);
          }
     }
   if (optind != argc)
     {
       for(int i = optind; i != argc; ++i)
         opts.in_files.push_back(argv[i]);
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
        eolian_cxx::state_init(opts);
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
