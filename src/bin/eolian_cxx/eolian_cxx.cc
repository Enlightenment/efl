
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

extern "C"
{
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Eo.h>
#include <Eolian.h>
}

#include <Eina.hh>
#include <Eolian_Cxx.hh>

#include "eo_read.h"
#include "convert.hh"
#include "safe_strings.hh"

namespace {

// Program options.
struct options_type
{
   std::vector<std::string> in_srcs;
   std::string out_file;
   std::string out_dir;
   std::string classname;
   std::string name_space;
   bool recurse;
   bool generate_all;

   options_type()
     : in_srcs()
     , out_file("")
     , out_dir("")
     , classname("")
     , name_space("")
     , recurse(false)
     , generate_all(false)
   {}
};

efl::eina::log_domain domain("eolian_cxx");

}

static void
_opt_error(std::string message)
{
   EINA_CXX_DOM_LOG_ERR(::domain) << message << std::endl;
   exit(EXIT_FAILURE);
}

static void
_assert_not_dup(std::string option, std::string value)
{
   if (value != "")
     {
        _opt_error("Option -" + option + " already set (" + value + ")");
     }
}

// Try to guess classname from input filenames.
// Precondition: Input sources must be loaded into Eolian Database
// otherwise we can't infer the classname from the .eo files.
// Precondition: Input options must have opts.classname == "".
static std::string
_guess_classname_from_sources(::options_type& opts)
{
   for (auto filename : opts.in_srcs)
     {
        if (const char *cls = eolian_class_find_by_file(filename.c_str()))
          {
             return cls;
          }
     }
   return "";
}

std::pair<std::string, std::string> get_filename_info(std::string path)
{
  const size_t last = path.rfind("lib/");
  if (last != std::string::npos)
    {
      path.erase(0, last+4);

      std::string::iterator slash
        = std::find(path.begin(), path.end(), '/');
      if(slash != path.end())
        {
          std::string namespace_ (path.begin(), slash);
          std::string filename (slash+1, path.end());
          return {filename, namespace_};
        }
    }
  return {path, std::string()};
}

efl::eolian::eo_generator_options
_resolve_includes(std::string const& classname)
{
   efl::eolian::eo_generator_options gen_opts;

   std::string cls_name = classname;
   std::transform(cls_name.begin(), cls_name.end(), cls_name.begin(), ::tolower);

   std::string eo_file = safe_str(eolian_class_file_get(classname.c_str()));
   gen_opts.c_headers.push_back(get_filename_info(eo_file).first + ".h");

   void *cur = NULL;
   const Eina_List *itr, *inheritances = eolian_class_inherits_list_get(classname.c_str());
   EINA_LIST_FOREACH(inheritances, itr, cur)
     {
        const char *ext = static_cast<const char*>(cur);
        std::string eo_parent_file = safe_str(eolian_class_file_get(ext));
        if (!eo_parent_file.empty())
          {
             std::string filename, namespace_;
             std::tie(filename, namespace_) = get_filename_info(eo_parent_file);
             // we have our own eo_base.hh
             std::string eo_base_eo = "eo_base.eo";
             if (filename.length() < eo_base_eo.length() ||
                 !std::equal(eo_base_eo.begin(), eo_base_eo.end(),
                             filename.end() - eo_base_eo.length()))
               {
                  gen_opts.cxx_headers.push_back(filename + ".hh");
               }
          }
        else
          {
             EINA_CXX_DOM_LOG_ERR(::domain)
               << "Couldn't find source file for class '" << ext << "'";
          }
     }
   return gen_opts;
}

static void
_generate(const std::string classname, ::options_type const& opts)
{
   efl::eolian::eo_class cls = ::c_to_cxx(classname.c_str());
   cls.name_space = opts.name_space;
   efl::eolian::eo_class_validate(cls);
   efl::eolian::eo_generator_options gen_opts = _resolve_includes(classname);
   std::string outname = (opts.out_file == "") ? (cls.name + ".eo.hh") : opts.out_file;

   if (opts.out_dir != "")
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
        assert(outfile.good());
        efl::eolian::generate(outfile, cls, gen_opts);
        outfile.close();
     }
}

static void
_run(options_type const& opts)
{
   if (opts.classname != "")
     {
        _generate(opts.classname.c_str(), opts);
     }
   else
     {
        efl::eina::range_ptr_list<const char* const>
          classes(eolian_class_names_list_get());
        for (auto cls : classes)
          {
             if (opts.classname == "" || opts.classname == cls)
               {
                  _generate(cls, opts);
               }
          }
    }
}

static void
_print_version()
{
   std::cerr
     << "Eolian C++ Binding Generator (EFL "
     << PACKAGE_VERSION << ")" << std::endl;
}

static void
_validate_options(::options_type const& opts)
{
   if (opts.in_srcs.size() == 0)
     {
        _opt_error("You must provide at least one input source (-I). "
                   "Either an .eo file or a directory of .eo files.");
     }
   else if (opts.out_file != "" && opts.generate_all)
     {
        _opt_error("Options -a and -o can't be used together.");
     }
   else if (!opts.generate_all && opts.classname == "")
     {
        _opt_error("Neither -a nor -c provided. "
                   "Don't know what to generate.");
     }
}

static void
_resolve_classname(options_type& opts)
{
   if (opts.classname == "")
     {
        std::string cls = _guess_classname_from_sources(opts);
        opts.classname = cls;
     }
   if (opts.classname == "" && opts.out_file != "")
     {
        EINA_CXX_DOM_LOG_ERR(::domain)
          << "Unknown output class for " << opts.out_file
          << " : Missing '-c' option?";
        std::abort();
     }
}

static void
_load_classes(options_type const& opts)
{
   for (auto src : opts.in_srcs)
     {
        if (eolian_read_from_fs(src.c_str()) == NULL)
          {
             EINA_CXX_DOM_LOG_WARN(::domain)
               << "Couldn't load eolian file: " << src;
          }
     }
}

static void
_usage(const char *progname)
{
   std::cerr
     << progname
     << " [options]" << std::endl
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

static ::options_type
_read_options(int argc, char **argv)
{
   ::options_type opts;

   const struct option long_options[] =
     {
       { "in",        required_argument, 0,  'I' },
       { "out-dir",   required_argument, 0,  'D' },
       { "out-file",  required_argument, 0,  'o' },
       { "class",     required_argument, 0,  'c' },
       { "namespace", required_argument, 0,  'n' },
       { "all",       no_argument,       0,  'a' },
       { "recurse",   no_argument,       0,  'r' },
       { "version",   no_argument,       0,  'v' },
       { "help",      no_argument,       0,  'h' },
       { 0,           0,                 0,   0  }
     };
   const char* options = "I:D:o:c:n:arvh";

   int c, idx;
   while ( (c = getopt_long(argc, argv, options, long_options, &idx)) != -1)
     {
        if (c == 'I')
          {
             opts.in_srcs.push_back(optarg);
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
        else if (c == 'n')
          {
             _assert_not_dup("n", opts.name_space);
             opts.name_space = optarg;
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
   return opts;
}

int main(int argc, char **argv)
{
   efl::eina::eina_init eina_init;
   efl::eolian::eolian_init eolian_init;
#if DEBUG
   domain.set_level(efl::eina::log_level::debug);
#endif
   options_type opts = _read_options(argc, argv);
   _load_classes(opts);
   _resolve_classname(opts);
   _validate_options(opts);
   _run(opts);
   return 0;
}
