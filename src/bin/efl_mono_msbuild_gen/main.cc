/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.hh>


namespace efl_mono_msbuild_gen {

/// Program options.
struct options_type
{
   std::string out_file;
   std::string out_assembly;
   std::string target_type;
   std::string platform;
   std::vector<std::string> cs_files;
   std::vector<std::string> defines;
   std::vector<std::string> references;
   std::vector<std::string> lib_paths;
};

efl::eina::log_domain domain("efl_mono_msbuild_gen");

namespace {

void
_print_version()
{
   std::cerr
     << "EFL Mono MSBuild Generator (EFL "
     << PACKAGE_VERSION << ")" << std::endl;
}

static void
_usage(const char *progname)
{
   std::cerr
     << progname
     << " [options] [files]" << std::endl
     << "Options:" << std::endl
     << "  -o, --out-file <file>   The output file name. [default: <classname>.eo.hh]" << std::endl
     << "  -v, --version           Print the version." << std::endl
     << "  -h, --help              Print this help." << std::endl;
   exit(EXIT_FAILURE);
}

static void
_assert_not_dup(std::string option, std::string value)
{
   if (value != "")
     {
        EINA_CXX_DOM_LOG_ERR(efl_mono_msbuild_gen::domain) <<
          "Option -" + option + " already set (" + value + ")";
     }
}

static bool
opts_check(efl_mono_msbuild_gen::options_type const& opts)
{
   if (opts.out_file.empty())
     {
        EINA_CXX_DOM_LOG_ERR(efl_mono_msbuild_gen::domain)
          << "Nowhere to generate?" << std::endl;
     }
   else
     return true; // valid.
   return false;
}

static efl_mono_msbuild_gen::options_type
opts_get(int argc, char **argv)
{
   efl_mono_msbuild_gen::options_type opts;

   const struct option long_options[] =
     {
       { "out-file",  required_argument, 0,  'o' },
       { "assembly",  required_argument, 0,  'a' },
       { "target-type", required_argument, 0,  't' },
       { "platform",  required_argument, 0,  'p' },
       { "define",    required_argument, 0,  'd' },
       { "reference", required_argument, 0,  'r' },
       { "lib-path",  required_argument, 0,  'l' },
       { "version",   no_argument,       0,  'v' },
       { "help",      no_argument,       0,  'h' },
       { 0,           0,                 0,   0  }
     };
   const char* options = "o:a:t:p:d:r:l:vh";

   int c, idx;
   while ( (c = getopt_long(argc, argv, options, long_options, &idx)) != -1)
     {
        if (c == 'o')
          {
             _assert_not_dup("o", opts.out_file);
             opts.out_file = optarg;
          }
        else if (c == 'a')
          {
             _assert_not_dup("a", opts.out_assembly);
             opts.out_assembly = optarg;
          }
        else if (c == 't')
          {
             _assert_not_dup("t", opts.target_type);
             opts.target_type = optarg;
          }
        else if (c == 'p')
          {
             _assert_not_dup("p", opts.platform);
             opts.platform = optarg;
          }
        else if (c == 'd')
          {
             opts.defines.push_back(optarg);
          }
        else if (c == 'r')
          {
             opts.references.push_back(optarg);
          }
        else if (c == 'l')
          {
             opts.lib_paths.push_back(optarg);
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

   for (int i = optind; i < argc; ++i)
     {
        opts.cs_files.push_back(argv[i]);
     }

   if (!efl_mono_msbuild_gen::opts_check(opts))
     {
        _usage(argv[0]);
        assert(false && "Wrong options passed in command-line");
     }

   return opts;
}

static void
run(options_type const& opts)
{
   std::ofstream os (opts.out_file.c_str());
   if(!os.is_open())
     {
        throw std::runtime_error(std::string{"Couldn't open output file "} + opts.out_file);
     }

   os << "<Project DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n";

   os << "  <PropertyGroup>\n";
   if (opts.target_type == "library")
     os << "    <OutputType>Library</OutputType>\n";
   //os << "    <Version>" << opts.version << "</Version>\n"; // TODO
   if (!opts.defines.empty())
     {
        os << "    <DefineConstants>$(DefineConstants)";
        for (auto const& def : opts.defines)
          {
              os << ";" << def;
          }
        os << "</DefineConstants>\n";
     }
   os << "  </PropertyGroup>\n";

   if (!opts.cs_files.empty())
     {
        os << "  <ItemGroup>\n";
        for (auto fname : opts.cs_files)
          {
             std::replace(fname.begin(), fname.end(), '/', '\\');
             os << "    <CSFile Include=\"" << fname << "\" />\n";
          }
        os << "  </ItemGroup>\n";
     }

   if (!opts.references.empty())
     {
        os << "  <ItemGroup>\n";
        for (auto libname : opts.references)
          {
             os << "    <Reference Include=\"" << libname << "\" />\n";
          }
        os << "  </ItemGroup>\n";
     }

   if (!opts.lib_paths.empty())
     {
        os << "  <ItemGroup>\n";
        for (auto libpath : opts.lib_paths)
          {
             std::replace(libpath.begin(), libpath.end(), '/', '\\');
             os << "    <LibPath Include=\"" << libpath << "\" />\n";
          }
        os << "  </ItemGroup>\n";
     }

   os << "  <Target Name=\"Build\">\n";
   os << "    <CSC Sources=\"@(CSFile)\" References=\"@(Reference)\" AdditionalLibPaths=\"$(LibPath)\" DefineConstants=\"$(DefineConstants)\" ";
   {
      if (!opts.out_assembly.empty())
        {
           os << "OutputAssembly=\"" << opts.out_assembly << "\" ";
        }
      if (!opts.target_type.empty())
        {
           os << "TargetType=\"" << opts.target_type << "\" ";
        }
      if (!opts.platform.empty())
        {
           os << "Platform=\"" << opts.platform << "\" ";
        }
   }
   os << "/>\n";
   os << "  </Target>\n";

   os << "</Project>\n";
}

} // anonymous namespace
} // namespace efl_mono_msbuild_gen

int main(int argc, char **argv)
{
   try
     {
        efl::eina::eina_init eina_init;
        efl_mono_msbuild_gen::options_type opts = efl_mono_msbuild_gen::opts_get(argc, argv);
        efl_mono_msbuild_gen::run(opts);
     }
   catch(std::exception const& e)
     {
        EINA_CXX_DOM_LOG_ERR(efl_mono_msbuild_gen::domain) << "EFL Mono MSBuild generation for the following reason: "  << e.what() << std::endl;
        return -1;
     }
   return 0;
}
