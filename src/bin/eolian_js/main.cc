
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eolian.h>
#include <Eina.hh>

#include <Eolian_Cxx.hh>
#include <grammar/klass_def.hpp>
#include <eolian/js/domain.hh>
#include <eolian/js/format.hh>
#include <eolian/class.hh>
#include <eolian/grammar/class_registration.hpp>

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <sstream>
#include <stdexcept>

#include <libgen.h>
#include <getopt.h>
#include <cstdlib>
#include <vector>
#include <set>
#include <map>

namespace eolian { namespace js {

efl::eina::log_domain domain("eolian_js");

struct incomplete_complex_type_error : public std::exception
{
  explicit incomplete_complex_type_error(std::string const& msg_arg)
    : msg(msg_arg)
  {}
  const char* what() const noexcept { return msg.c_str(); }

  std::string msg;
};

} }


std::string
_lowercase(std::string str)
{
  transform(begin(str), end(str), begin(str), & ::tolower);
  return str;
}

std::string
_uppercase(std::string str)
{
  transform(begin(str), end(str), begin(str), & ::toupper);
  return str;
}

std::string
_class_name_getter(std::string const& caller_class_prefix, std::string class_name)
{
  std::replace(class_name.begin(), class_name.end(), '.', '_');
  return caller_class_prefix + "_" + class_name + "_cls_name_getter";
}

void
_final_type_and_type_type_get(Eolian_Type const* tp_in, Eolian_Type const*& tp_out, Eolian_Type_Type& tpt_out)
{
  tp_out = tp_in;
  tpt_out = eolian_type_type_get(tp_in);
  if (tpt_out == EOLIAN_TYPE_REGULAR)
    {
       auto tpd = eolian_type_typedecl_get(NULL, tp_out);
       if (tpd && eolian_typedecl_type_get(tpd) == EOLIAN_TYPEDECL_ALIAS &&
           !eolian_typedecl_is_extern(tpd))
         {
           auto btp = eolian_typedecl_aliased_base_get(NULL, tpd);
            if (btp && eolian_type_full_name_get(btp) &&
                strcmp(eolian_type_full_name_get(btp), "__undefined_type") != 0)
              {
                 _final_type_and_type_type_get(btp, tp_out, tpt_out);
              }
         }
    }
}

// std::string
// _eolian_type_cpp_type_named_get(const Eolian_Type *tp, std::string const& caller_class_prefix, std::set<std::string>& need_name_getter, bool in_pointer = false)
// {
//   const auto is_const = eolian_type_is_const(tp);

//   std::string result;

//    if(!in_pointer && (eolian_type_is_ptr(tp) /* || eolian_type_type_get(tp) == EOLIAN_TYPE_TERMINATED_ARRAY
//                                                 || eolian_type_type_get(tp) == EOLIAN_TYPE_STATIC_ARRAY*/))
//    // else if (tpt == EOLIAN_TYPE_POINTER)
//      {
//         // auto btp = eolian_type_base_type_get(tp);
//         auto btp = // eolian_type_type_get(tp) == EOLIAN_TYPE_TERMINATED_ARRAY
//           // || eolian_type_type_get(tp) == EOLIAN_TYPE_STATIC_ARRAY
//           // ? eolian_type_base_type_get(tp)
//           // :
//           tp;
//         result += _eolian_type_cpp_type_named_get(btp, caller_class_prefix, need_name_getter
//                                                   , eolian_type_is_ptr(tp));
//         const auto base_is_const = eolian_type_is_const(btp);

//         Eolian_Type_Type btpt = EOLIAN_TYPE_UNKNOWN_TYPE;
//         _final_type_and_type_type_get(btp, btp, btpt);
//         auto btpd = eolian_type_typedecl_get(eolian_unit, btp);

//         if (btpd && eolian_typedecl_type_get(btpd) == EOLIAN_TYPEDECL_STRUCT)
//           {
//              std::string f = "::make_struct_tag";
//              auto p = result.find(f);
//              if (p == std::string::npos)
//                throw std::runtime_error("missing struct type tag");
//              result.replace(p, f.size(), "::make_struct_ptr_tag");
//              result.pop_back();
//              result += " *";
//              if (is_const) result += " const";
//              result += ">";
//           }
//         else
//           {
//              // if (btpt != EOLIAN_TYPE_POINTER || base_is_const)
//              //    result += ' ';
//              result += '*';
//              if (is_const) result += " const";
//           }
//      }
//    else
//      {
//   Eolian_Type_Type tpt = EOLIAN_TYPE_UNKNOWN_TYPE;
//   _final_type_and_type_type_get(tp, tp, tpt);

//   if (tpt == EOLIAN_TYPE_UNKNOWN_TYPE || tpt == EOLIAN_TYPE_UNDEFINED)
//     return "error";

//    if ((tpt == EOLIAN_TYPE_VOID
//      || tpt == EOLIAN_TYPE_REGULAR
//      // || tpt == EOLIAN_TYPE_COMPLEX
//      || tpt == EOLIAN_TYPE_CLASS)
//      && is_const)
//      {
//         result += "const ";
//      }


//    if (tpt == EOLIAN_TYPE_REGULAR
//     // || tpt == EOLIAN_TYPE_COMPLEX
//     || tpt == EOLIAN_TYPE_CLASS)
//      {
//         for (efl::eina::iterator<const char> first(::eolian_type_namespaces_get(tp)), last; first != last; ++first)
//           {
//             std::string np(&*first);
//             result += np + "_"; // TODO: transform it to the C++ equivalent?
//           }

//         // this comes from ctypes at eo_lexer.c and KEYWORDS at eo_lexer.h
//         const static std::unordered_map<std::string, std::string> type_map = {
//           {"byte", "signed char"},
//           {"ubyte", "unsigned char"},
//           {"char", "char"},
//           {"short", "short"},
//           {"ushort", "unsigned short"},
//           {"int", "int"},
//           {"uint", "unsigned int"},
//           {"long", "long"},
//           {"ulong", "unsigned long"},
//           {"llong", "long long"},
//           {"ullong", "unsigned long long"},
//           {"int8", "int8_t"},
//           {"uint8", "uint8_t"},
//           {"int16", "int16_t"},
//           {"uint16", "uint16_t"},
//           {"int32", "int32_t"},
//           {"uint32", "uint32_t"},
//           {"int64", "int64_t"},
//           {"uint64", "uint64_t"},
//           {"int128", "int128_t"},
//           {"uint128", "uint128_t"},
//           {"size", "size_t"},
//           {"ssize", "ssize_t"},
//           {"intptr", "intptr_t"},
//           {"uintptr", "uintptr_t"},
//           {"ptrdiff", "ptrdiff_t"},
//           {"time", "time_t"},
//           {"float", "float"},
//           {"double", "double"},
//           {"bool", "Eina_Bool"},
//           {"void", "void"},
//           {"generic_value", "Eina_Value"},
//           {"accessor", "Eina_Accessor"},
//           {"array", "Eina_Array"},
//           {"iterator", "Eina_Iterator"},
//           {"hash", "Eina_Hash"},
//           {"list", "Eina_List"},
//           {"string", "const char*"},
//           {"void_ptr", "void *"},
//           {"stringshare", "Eina_Stringshare*"},
//           {"future", "Efl_Future*"}

//         };

//         std::string type_name = eolian_type_name_get(tp);
//         auto it = type_map.find(type_name);
//         if (it != end(type_map))
//           type_name = it->second;
//         result += type_name;

//         if (tpt == EOLIAN_TYPE_CLASS /* || tpt == EOLIAN_TYPE_COMPLEX*/)
//             result += "*"; // Implied pointer

//         auto tpd = eolian_type_typedecl_get(eolian_unit, tp);
//         if (tpd && eolian_typedecl_type_get(tpd) == EOLIAN_TYPEDECL_STRUCT)
//           {
//              result = "efl::eina::js::make_struct_tag<" + result + ">";
//           }

//         // if (tpt == EOLIAN_TYPE_COMPLEX)
//         //   {
//         //      result = "efl::eina::js::make_complex_tag<" + result;

//         //      bool has_subtypes = false;
//         //      const Eolian_Type *subtype = eolian_type_base_type_get(tp);
//         //      while (subtype)
//         //        {
//         //          auto t = _eolian_type_cpp_type_named_get(subtype, caller_class_prefix, need_name_getter);
//         //          auto k = type_class_name(subtype);
//         //          if (!k.empty())
//         //            {
//         //               result += ", " + t + ", " + _class_name_getter(caller_class_prefix, k);
//         //               need_name_getter.insert(k);
//         //            }
//         //          else
//         //            {
//         //               result += ", " + t + ", ::efl::eina::js::nonclass_cls_name_getter";
//         //            }
//         //          has_subtypes = true;
//         //          subtype = eolian_type_next_type_get(subtype);
//         //        }

//         //      if (!has_subtypes)
//         //        throw eolian::js::incomplete_complex_type_error("Incomplete complex type");

//         //      result += ">";
//         //   }
//      }
//    else if (tpt == EOLIAN_TYPE_VOID)
//      result += "void";
//    // else if(tpt == EOLIAN_TYPE_STATIC_ARRAY)
//    //   {
//    //     result += "void";
//    //   }
//    // else if(tpt == EOLIAN_TYPE_TERMINATED_ARRAY)
//    //   {
//    //     result += "void";
//    //   }
//      }
//    // else
//    //   {
//    //      throw std::runtime_error("unhandled Eolian_Type_Type value");
//    //   }

//    /*if (!name.empty())
//      {
//         if (tpt != EOLIAN_TYPE_POINTER)
//           result += ' ';
//         result += name;
//      }*/

//    return result;
// }

// using ParametersIterator = efl::eina::iterator<const ::Eolian_Function_Parameter>;

// std::vector<const ::Eolian_Function_Parameter*>
// _eolian_function_keys_get(const Eolian_Function *function_id, Eolian_Function_Type ftype)
// {
//   std::vector<const ::Eolian_Function_Parameter*> keys;

//   for(ParametersIterator it(::eolian_property_keys_get(function_id, ftype)), last; it != last; ++it)
//     keys.push_back(&*it);

//   return keys;
// }

// std::vector<const ::Eolian_Function_Parameter*>
// _eolian_function_parameters_get(const Eolian_Function *function_id, Eolian_Function_Type function_type)
// {
//   std::vector<const ::Eolian_Function_Parameter*> parameters;

//   ParametersIterator it { (function_type == EOLIAN_METHOD) ?
//                             ::eolian_function_parameters_get(function_id) :
//                             ::eolian_property_values_get(function_id, function_type)
//     }, last;

//   for(; it != last; ++it)
//     parameters.push_back(&*it);

//   return parameters;
// }

// bool
// _function_return_is_missing(Eolian_Function const* func, Eolian_Function_Type func_type)
// {
//    // XXX This function shouldn't exist. Eolian should
//    //     forge functions a priori. Bindings generators
//    //     shouldn't be required to convert such thing.
//    Eolian_Type const* type =
//      ::eolian_function_return_type_get(func, func_type);
//    return !type;
// }

// bool
// _type_is_generatable(const Eolian_Type *tp, bool add_pointer, Eolian_C_Type_Type c_type_type)
// {
//    std::string c_type = eolian_type_c_type_get(eolian_unit, tp, c_type_type);

//    if (add_pointer)
//        c_type += " *";

//    return c_type.find("void *") == std::string::npos;
// }

// bool
// _function_belongs_to(const Eolian_Function *function, std::string klass)
// {
//    const Eolian_Class *cl = eolian_function_class_get(function);
//    const std::string name = cl ? eolian_class_full_name_get(cl) : "";
//    return name.find(klass) == 0;
// }

// bool
// _function_is_generatable(const Eolian_Function *function, Eolian_Function_Type ftp)
// {
//    const auto key_params = _eolian_function_keys_get(function, ftp);
//    const auto parameters = _eolian_function_parameters_get(function, ftp);
//    std::vector<const ::Eolian_Function_Parameter*> full_params;

//    full_params.insert(std::end(full_params), std::begin(key_params), std::end(key_params));
//    full_params.insert(std::end(full_params), std::begin(parameters), std::end(parameters));

//    for (auto parameter : full_params)
//      {
//         auto tp = ::eolian_parameter_type_get(parameter);
//         bool add_pointer = eolian_parameter_direction_get(parameter) != EOLIAN_IN_PARAM;
//         if (!_type_is_generatable(tp, add_pointer, EOLIAN_C_TYPE_PARAM))
//           return false;

//         if (eolian_type_is_ptr(tp) && _function_belongs_to(function, "Efl.Object"))
//           return false;
//      }

//    auto rtp = ::eolian_function_return_type_get(function, ftp);

//    return rtp ? _type_is_generatable(rtp, false, EOLIAN_C_TYPE_RETURN) : true;
// }

// bool
// _function_is_public(const Eolian_Function *function, Eolian_Function_Type t)
// {
//     if (t == EOLIAN_PROPERTY)
//         return _function_is_public(function, EOLIAN_PROP_GET) || _function_is_public(function, EOLIAN_PROP_SET);
//     else
//         return eolian_function_scope_get(function, t) == EOLIAN_SCOPE_PUBLIC;
// }


// void separate_functions(Eolian_Class const* klass, Eolian_Function_Type t, bool ignore_constructors,
//                         std::vector<Eolian_Function const*>& constructor_functions,
//                         std::vector<Eolian_Function const*>& normal_functions)
// {
//    efl::eina::iterator<Eolian_Function> first ( ::eolian_class_functions_get(klass, t) )
//      , last;
//    for(; first != last; ++first)
//      {
//         Eolian_Function const* function = &*first;
//         if (_function_is_public(function, t))
//           {
//              EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << ::eolian_function_full_c_name_get(function, t, EINA_FALSE);
//              if(strcmp("elm_obj_entry_input_panel_imdata_get", ::eolian_function_full_c_name_get(function, t, EINA_FALSE)) != 0 &&
//                 !eolian_function_is_beta(function) &&
//                 // strcmp("data_callback", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("property", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("part_text_anchor_geometry_get", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("children_iterator_new", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("inputs_get", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("constructor", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("render_updates", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("render2_updates", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("efl_canvas_surface_x11_pixmap_set", ::eolian_function_full_c_name_get(function, t, EINA_FALSE)) != 0 && // TODO: remove this
//                 strcmp("efl_canvas_surface_x11_pixmap_get", ::eolian_function_full_c_name_get(function, t, EINA_FALSE)) != 0 && // TODO: remove this
//                 strcmp("efl_canvas_surface_native_buffer_set", ::eolian_function_full_c_name_get(function, t, EINA_FALSE)) != 0 && // TODO: remove this
//                 strcmp("efl_canvas_surface_native_buffer_get", ::eolian_function_full_c_name_get(function, t, EINA_FALSE)) != 0 && // TODO: remove this
//                 strcmp("event_callback_priority_add", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("event_callback_array_priority_add", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("event_callback_array_del", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("event_callback_call", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("event_callback_forwarder_add", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("event_callback_forwarder_del", ::eolian_function_name_get(function)) != 0 && // TODO: remove this
//                 strcmp("event_callback_del", ::eolian_function_name_get(function)) != 0)
//                {
//                   if( ::eolian_function_is_constructor(function, klass))
//                     {
//                        if(!ignore_constructors)
//                          {
//                              EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "is a constructor";
//                              constructor_functions.push_back(function);
//                          }
//                        else
//                          {
//                             EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "ignoring parent's constructors";
//                          }
//                     }
//                   else /*if( std::strcmp( ::eolian_function_full_c_name_get(function, t, EINA_FALSE)
//                          , "eo_parent") != 0)*/
//                     {
//                        EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "is a NOT constructor "
//                                                                  << ::eolian_function_full_c_name_get(function, t, EINA_FALSE);
//                        normal_functions.push_back(function);
//                     }
//                   // else
//                   //   {
//                   //     EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "parent_set as first constructor";
//                   //     constructor_functions.insert(constructor_functions.begin(), function);
//                   //     normal_functions.push_back(function);
//                   //   }
//                }
//           }
//      }
// }

int main(int argc, char** argv)
{
  namespace format = eolian::js::format;

  std::vector<std::string> include_paths;
  std::string out_file, in_file, params_file;
  std::vector<std::string> in_files;
  bool linking = false;
  std::set<Eolian_Class const*> legacy_klasses;
  bool dummy_generation = false;

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
       { "legacy",    required_argument, 0,  'l' },
       { "version",   no_argument,       0,  'v' },
       { "help",      no_argument,       0,  'h' },
       { "link",      no_argument,       0,  'i' },
       { "params",    required_argument, 0,  'p' },
       { "dummy",     no_argument,       0,  'd' },
       { 0,           0,                 0,   0  }
    };
   const char* options = "I:o:l:p:vhid";

   Eolian* eolian = eolian_new();
   Eolian_Unit* eolian_unit = static_cast<Eolian_Unit*>(static_cast<void*>(eolian));

   auto cmd_options = [&] (char c)
     {
        if (c == 'I')
          {
            if (!::eolian_directory_scan(eolian, optarg))
              {
                EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
                << "Couldn't load eolian from '" << optarg << "'.";
              }
          }
        else if (c == 'o')
          {
            if(!out_file.empty())
              {
                // _usage(argv[0]);
                exit(1);
              }
            std::cout << "out file " << optarg << std::endl;
            out_file = optarg;
          }
        else if (c == 'h')
          {
             // _usage(argv[0]);
            exit(1);
          }
        else if (c == 'v')
          {
             // _print_version();
             // if (argc == 2) exit(EXIT_SUCCESS);
          }
        else if(c == 'i')
          {
            linking = true;
          }
        else if(c == 'l')
          {
            // legacy  file
            // std::cout << "legacy file|" << optarg << '|' << std::endl;
            // if (!::eolian_file_parse(eolian, optarg))
            //   {
            //      std::cout
            //        << "Failed parsing: " << optarg << "." << std::endl;
            //      EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
            //        << "Failed parsing: " << optarg << ".";
            //      assert(false && "Error parsing input file");
            //   }
            // char* dup = strdup(optarg);
            // char *bn = basename(dup);
            
            // Eolian_Class const* klass = ::eolian_class_get_by_file(eolian_unit, bn);

            // free(dup);
            
            // std::cout << " class pointer " << klass << std::endl;
            // assert(!!klass);
            // legacy_klasses.insert(klass);
          }
        else if(c == 'p')
          {
            params_file = optarg;
          }
        else if(c == 'd')
          {
            dummy_generation = true;
          }
     };
   
   // get command line options
   int c, idx = 0;
   while ( (c = getopt_long(argc, argv, options, long_options, &idx)) != -1)
     {
       std::cout << "reading, idx " << idx << " optind " << optind << std::endl;
       std::cout << "char " << (char)c << std::endl;
       cmd_options(c);
     }

     if(!linking)
       {
         if (optind == argc-1)
           {
             in_file = argv[optind];
           }
       }
     else
       {
         if (!::eolian_all_eot_files_parse(eolian))
           {
             EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
               << "Eolian failed parsing eot files";
             assert(false && "Error parsing eot files");
           }
         int optional_index = optind;
         while (optional_index != argc)
           {
             std::cout << "opening " << argv[optional_index] << std::endl;
             if (!::eolian_file_parse(eolian, argv[optional_index]))
               {
                 std::cout
                   << "Failed parsing: " << argv[optional_index] << "." << std::endl;
                 EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
                   << "Failed parsing: " << argv[optional_index] << ".";
                 assert(false && "Error parsing input file");
               }
             in_files.push_back(argv[optind]);
             optional_index++;
           }
       }
   
   if(!params_file.empty())
     {
       std::ifstream param_stream(params_file.c_str());
       if(param_stream.is_open())
         {
           std::vector<std::string> lines{argv[0]};
           std::string line;
           while(std::getline(param_stream, line))
             {
               std::cout << "line read " << line << std::endl;
               if(!line.empty())
                 lines.push_back(line);
             }

           std::vector<const char*> more_args(lines.size());
           std::size_t i = 0;
           for(auto&& l : lines)
             {
               more_args[i] = l.c_str();
               ++i;
             }
           optind = 1;
           idx = 0;
           while ( (c = getopt_long(lines.size(), const_cast<char*const*>(&more_args[0]), options, long_options, &idx)) != -1)
             {
               std::cout << "reading, idx " << idx << " optind " << optind << std::endl;
               std::cout << "char " << (char)c << std::endl;
               cmd_options(c);
             }
           std::cout << "while finished idx " << idx << " optind " << optind << std::endl;

           if(!linking)
             {
               if (optind == more_args.size()-1)
                 {
                   in_file = lines[optind];
                 }
             }
           else
             {
               if (!::eolian_all_eot_files_parse(eolian))
                 {
                   EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
                     << "Eolian failed parsing eot files";
                   assert(false && "Error parsing eot files");
                 }
               int optional_index = optind;
               std::cout << "optional_index " << optional_index << std::endl;
               assert(optional_index <= more_args.size());
               while (optional_index != more_args.size())
                 {
                   std::cout << "opening in params " << lines[optional_index] << std::endl;
                   if (!::eolian_file_parse(eolian, lines[optional_index].c_str()))
                     {
                       std::cout
                         << "Failed parsing: " << lines[optional_index] << "." << std::endl;
                       EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
                         << "Failed parsing: " << lines[optional_index] << ".";
                       assert(false && "Error parsing input file");
                     }
                   in_files.push_back(lines[optional_index]);
                   optional_index++;
                 }
             }

         }
     }

   std::cout << "going on" << std::endl;
   
   std::ofstream os (out_file.c_str());
   if(!os.is_open())
     {
       EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "Couldn't open output file " << out_file;
       return -1;
     }

   EINA_CXX_DOM_LOG_DBG(eolian::js::domain) << "output was opened";

   if(!linking)
   {
     // Add include paths to eolian library
     if (!::eolian_file_parse(eolian, in_file.c_str()))
       {
         EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
           << "Failed parsing: " << in_file << ".";
         assert(false && "Error parsing input file");
       }

     // Create filename path for output
     std::string file_basename;
     const Eolian_Class *klass = NULL;
     {
       char* dup = strdup(in_file.c_str());
       char *bn = basename(dup);
       klass = ::eolian_class_get_by_file(eolian_unit, bn);
       file_basename = bn;
       free(dup);
     }
     if(!klass)
       {
         EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "could not find any class defined in this eo file";
         return -1;
       }

   
     std::ostream_iterator<char> iterator = std::ostream_iterator<char>(os);

     // probably not eot file
     if(klass)
       {
         //os << "#if !defined(EFL_JAVASCRIPT_EOLIAN_GENERATE_REGISTER_STRUCT)\n";
         // function to iterate through all inheritance class
         std::function<void(Eolian_Class const*, std::function<void(Eolian_Class const*)>)>
           recurse_inherits
           = [&] (Eolian_Class const* klass, std::function<void(Eolian_Class const*)> function)
           {
             for(efl::eina::iterator<Eolian_Class const> inherit_iterator ( ::eolian_class_inherits_get(klass))
                   , inherit_last; inherit_iterator != inherit_last; ++inherit_iterator)
               {
                 Eolian_Class const* base = &*inherit_iterator;
                 function(base);
                 recurse_inherits(base, function);
               }
           };

         auto generate_includes =
           [&]
           {
             os << "extern \"C\" {\n\n";
     
             auto includes_fun = [&os] (Eolian_Class const* klass)
             {
               os << "#include <" << eolian_class_file_get(klass) << ".h>\n\n";
             };
             // generate include for all inheritance
             recurse_inherits(klass, includes_fun);
             os << "#include <" << eolian_class_file_get(klass) << ".h>\n\n";
       
             os << "}\n\n";
           };

         using efl::eolian::grammar::attributes::unused;
         using efl::eolian::grammar::context_null;
         efl::eolian::grammar::attributes::klass_def klass_def(klass, eolian_unit);
         using efl::eolian::grammar::lower_case;
         using efl::eolian::grammar::string;
         using efl::eolian::grammar::as_generator;
         using efl::eolian::grammar::attribute_reorder;
         using efl::eolian::grammar::upper_case;
         if(!dummy_generation)
           {
             context_null context;

             // as_generator
             //   (attribute_reorder<0, 1, 0, 1>
             //    (
             //        "#define "
             //     << *(upper_case[string] << "_")
             //     << upper_case[string]
             //     << "_BETA\n"
             //     "#define "
             //     << *(upper_case[string] << "_")
             //     << upper_case[string]
             //     << "_PROTECTED\n"
             //    )).generate(iterator, std::make_tuple(klass_def.namespaces, klass_def.eolian_name), context);

             generate_includes();
             
             as_generator("namespace efl { namespace js { namespace binding { namespace {\n")
               .generate(iterator, unused, context);
             
             {
               for(efl::eina::iterator<Eolian_Class const> inherit_iterator ( ::eolian_class_inherits_get(klass))
                     , inherit_last; inherit_iterator != inherit_last; ++inherit_iterator)
                 {
                   efl::eolian::grammar::attributes::klass_def klass_def
                     ( &*inherit_iterator, eolian_unit);
                   as_generator
                     (
                      "static void register_prototype_"
                      << lower_case[*(string << "_")]
                      << lower_case[string]
                      << "(v8::Handle<v8::Object>, v8::Local<v8::ObjectTemplate>, v8::Isolate*);\n"
                     ).generate(iterator, std::make_tuple(klass_def.namespaces, klass_def.cxx_name), context);
                 }
             }

             if (!eolian::js::grammar::class_registration.generate(iterator, klass_def, context))
               {
                 throw std::runtime_error("Failed to generate class");
               }

             as_generator("} } } } // namespace efl { namespace js { namespace binding { namespace {\n\n")
               .generate(iterator, unused, context);
           }
         else
           {
             generate_includes();
             as_generator
               (
                 "namespace efl { namespace js { namespace binding { namespace {\n"
                 "static void register_prototype_"
                 << lower_case[*(string << "_")]
                 << lower_case[string]
                 << "(v8::Handle<v8::Object>, v8::Local<v8::ObjectTemplate>, v8::Isolate*)\n"
                    "{\n}\n"
                    "} } } }\n\n"
                ).generate(iterator, std::make_tuple(klass_def.namespaces, klass_def.cxx_name), context_null{});
           }

         //os << "#else\n";
         // if (!eolian::js::grammar::call_registration.generate(iterator, klass_def, context_null{}))
         // {
         //   throw std::runtime_error("Failed to generate class");
         // }
         //os << "#endif\n";
       }
     }
   else
     {
       // for(auto src : include_paths)
       //   if (!::eolian_directory_scan(eolian, src.c_str()))
       //     {
       //       EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
       //         << "Couldn't load eolian from '" << src << "'.";
       //     }

       // if(!::eolian_all_eo_files_parse(eolian))
       //   {
       //     EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
       //       << "Eolian failed parsing eo files";
       //     assert(false && "Error parsing eot files");
       //   }
       // if (!::eolian_all_eot_files_parse(eolian))
       //   {
       //     EINA_CXX_DOM_LOG_WARN(eolian::js::domain)
       //       << "Eolian failed parsing eot files";
       //     assert(false && "Error parsing eot files");
       //   }

       // ns -> items
       std::map<std::string, std::set<std::string>> namespace_map;
       std::map<std::string, std::set<std::string>> class_map;

       auto fix_name = [] (std::string& name)
         {
           std::transform(name.begin()
                          , name.end()
                          , name.begin()
                          , [] (char c)
         {
           c = ::tolower(c);
           if(c == '.')
             c = '_';
           return c;
         });
         };
       
       // efl::eina::iterator<Eolian_Class const> klasses (eolian_all_classes_get(eolian_unit));
       // for(;klasses != efl::eina::iterator<Eolian_Class const>{}
       //       ;++klasses)
       for(auto&& file : in_files)
         {
           // if(legacy_klasses.find(&*klasses) != legacy_klasses.end())
           //   continue;
           char* dup = strdup(file.c_str());
           char *bn = basename(dup);
            
           Eolian_Class const* klass = ::eolian_class_get_by_file(eolian_unit, bn);
           
           std::string outer_namespace; // starts with global
           std::string name = ::eolian_class_full_name_get(klass);

           std::cout << "not considered legacy " << name << std::endl;
           
           auto iterator_old = name.begin();
           auto iterator = std::find(iterator_old, name.end(), '.');
           while(iterator != name.end())
             {
               std::string namespace_name(iterator_old, iterator);

               // std::cout << "outer namespace " << outer_namespace << " name " << namespace_name << std::endl;
               
               // other identifiers are namespaces
               namespace_map[outer_namespace].insert(namespace_name);
               iterator_old = std::next(iterator);
               if(outer_namespace.empty())
                 outer_namespace = namespace_name;
               else
                 {
                   outer_namespace += '.';
                   outer_namespace += namespace_name;
                 }
               // std::cout << "before loop outer namespace " << outer_namespace << " name " << namespace_name << std::endl;
               iterator = std::find(iterator_old, name.end(), '.');
             }
           // last identifier is the class name
           std::string class_name {iterator_old, iterator};
           fix_name(class_name);
           class_map[outer_namespace].insert(class_name);
           // outer_namespace is still a namespace
           namespace_map[outer_namespace]; // namespace
         }

       // os << "typedef efl::eo::js::namespace_object"
       for(auto&& item : namespace_map)
         {
           std::set<std::string> classes;

           auto mirrored_iterator = class_map.find(item.first);
           if(mirrored_iterator != class_map.end())
             classes = mirrored_iterator->second;

           std::cout << "generating classes structs" << std::endl;

             {
               std::string classes_struct_name = item.first;
               fix_name(classes_struct_name);
               os << "struct " << classes_struct_name << "_classes\n"
                  << "{\n"
                  << "    typedef efl::eo::js::list\n"
                 "     <\n";
               
               auto cls_iterator = classes.begin();
               while(cls_iterator != classes.end())
                 {
                   auto&& cls = *cls_iterator;
                   os << "::efl::eo::js::class_object<efl::eo::js::name<";
                   auto char_iterator = cls.begin();
                   while(char_iterator != cls.end())
                     {
                       os << "'" << *char_iterator << "'";
                       if(std::next(char_iterator) != cls.end())
                         os << ", ";
                       ++char_iterator;
                     }
                   std::string full_name = (item.first == "" ? "" : item.first + '.') + cls;
                   fix_name(full_name);
                   os << ">, ::efl::js::binding::register_" << full_name << "_functor>";
                   cls_iterator++;
                   if(cls_iterator != classes.end())
                     os << ", ";
                 }
               os << "> type;\n};\n";

               ///
             }
         }
       
       for(auto item_iterator = namespace_map.rbegin()
             , last_item_iterator = namespace_map.rend()
             ;item_iterator != last_item_iterator
             ;++item_iterator)
         {
           auto&& item = *item_iterator;
           
           std::set<std::string> namespaces = item.second;
           //bool is_there_classes = false;

           std::cout << "namespace " << item.first << std::endl;

           //if(!namespaces.empty())
             {
               std::string classes_struct_name = item.first;
               auto fix_name = [] (std::string& name)
                 {
                   std::transform(name.begin()
                                  , name.end()
                                  , name.begin()
                                  , [] (char c)
                      {
                        c = ::tolower(c);
                        if(c == '.')
                          c = '_';
                        return c;
                      });
                 };
               fix_name(classes_struct_name);

               os << "struct " << classes_struct_name << "_namespaces\n"
                  << "{\n"
                  << "    typedef efl::eo::js::list\n"
                     "     <\n";

               std::cout << "namespace " << classes_struct_name << " has " << namespaces.size()
                         << " namespaces" << std::endl;
               
               auto namespace_iterator = namespaces.begin();
               while(namespace_iterator != namespaces.end())
                 {
                   auto&& namesp = *namespace_iterator;
                   os << "::efl::eo::js::namespace_object<efl::eo::js::name<";
                   auto char_iterator = namesp.begin();
                   while(char_iterator != namesp.end())
                     {
                       os << "'" << *char_iterator << "'";
                       if(std::next(char_iterator) != namesp.end())
                         os << ", ";
                       ++char_iterator;
                     }
                   std::string full_name = (item.first == "" ? "" : item.first + '.') + namesp;
                   fix_name(full_name);
                   os << ">, " << full_name << "_classes, " << full_name << "_namespaces>";
                   namespace_iterator++;
                   if(namespace_iterator != namespaces.end())
                     os << ", ";
                 }
               os << "> type;\n};\n";
             }
         }
     }
   // std::vector<Eolian_Function const*> constructor_functions;
   // std::vector<Eolian_Function const*> normal_functions;

   // std::set<Eolian_Class const*> classes;

   // // separate normal functions from constructors for all methods and properties
   // separate_functions(klass, EOLIAN_METHOD, false, constructor_functions, normal_functions);
   // separate_functions(klass, EOLIAN_PROPERTY, false, constructor_functions, normal_functions);

   // EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "functions were separated";

   // // function to iterate through all inheritance class
   // std::function<void(Eolian_Class const*, std::function<void(Eolian_Class const*)>)>
   //   recurse_inherits
   //   = [&] (Eolian_Class const* klass, std::function<void(Eolian_Class const*)> function)
   //   {
   //     for(efl::eina::iterator<const char> first ( ::eolian_class_inherits_get(klass))
   //           , last; first != last; ++first)
   //       {
   //         EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << &*first << std::endl;
   //         Eolian_Class const* base = ::eolian_class_get_by_name(eolian_unit, &*first);
   //         function(base);
   //         recurse_inherits(base, function);
   //       }
   //   };

   // // save functions from all inhehritance
   // auto save_functions = [&](Eolian_Class const* klass)
   //   {
   //     if(classes.find(klass) == classes.end())
   //       {
   //         classes.insert(klass);
   //         separate_functions(klass, EOLIAN_METHOD, true, constructor_functions, normal_functions);
   //         separate_functions(klass, EOLIAN_PROPERTY, true, constructor_functions, normal_functions);
   //       }
   //   };
   // // save functions from all inheritance class without constructors
   // recurse_inherits(klass, save_functions);

   // EINA_CXX_DOM_LOG_DBG(eolian::js::domain) << "inherits were recursed";

   // std::ofstream os (out_file.c_str());
   // if(!os.is_open())
   //   {
   //     EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "Couldn't open output file " << out_file;
   //     return -1;
   //   }

   // EINA_CXX_DOM_LOG_DBG(eolian::js::domain) << "output was opened";

   // std::string class_name(name(klass)),
   //             class_full_name(full_name(klass)),
   //             upper_case_class_name(_uppercase(class_name)),
   //             lower_case_class_name(_lowercase(class_name));

   // // Start preamble generation
   // if (getenv("EFL_RUN_IN_TREE"))
   //   {
   //     os << "#ifdef HAVE_CONFIG_H\n";
   //     os << "#include \"config.h\"\n";
   //     os << "#endif\n";

   //     os << "#include <Efl.h>\n";
   //     os << "#include <Ecore.h>\n";
   //     os << "#include <Eo.h>\n\n";
   //   }
   // else
   //   {
   //     os << "#ifdef HAVE_CONFIG_H\n";
   //     os << "#include \"elementary_config.h\"\n";
   //     os << "#endif\n";

   //     os << "#include <Efl.h>\n";
   //     os << "#include <Ecore.h>\n";
   //     os << "#include <Eo.h>\n";
   //     os << "#include <Evas.h>\n";
   //     os << "#include <Edje.h>\n";

   //     os << "#include <Elementary.h>\n\n";
   //     os << "extern \"C\" {\n";
   //     os << "#include <elm_widget.h>\n";
   //     os << "}\n\n";
   //   }
   // os << "#include <Eina_Js.hh>\n\n";
   // os << "#include <Eo_Js.hh>\n\n";
   // os << "#ifdef EAPI\n";
   // os << "# undef EAPI\n";
   // os << "#endif\n";

   // os << "#ifdef _WIN32\n";
   // os << "# define EAPI __declspec(dllimport)\n";
   // os << "#else\n";
   // os << "# ifdef __GNUC__\n";
   // os << "#  if __GNUC__ >= 4\n";
   // os << "#   define EAPI __attribute__ ((visibility(\"default\")))\n";
   // os << "#  else\n";
   // os << "#   define EAPI\n";
   // os << "#  endif\n";
   // os << "# else\n";
   // os << "#  define EAPI\n";
   // os << "# endif\n";
   // os << "#endif /* ! _WIN32 */\n\n";
   // os << "extern \"C\" {\n";

   // // generate include for necessary headers
   // if(is_evas(klass))
   //   os << "#include <Evas.h>\n";

   // auto includes_fun = [&os] (Eolian_Class const* klass)
   //   {
   //     os << "#include <" << eolian_class_file_get(klass) << ".h>\n\n";
   //   };
   // // generate include for all inheritance
   // recurse_inherits(klass, includes_fun);
   // os << "#include <" << eolian_class_file_get(klass) << ".h>\n\n";

   // os << "}\n\n";

   // os << "#ifdef _WIN32\n";
   // os << "# undef EAPI\n";
   // os << "# define EAPI __declspec(dllexport)\n";
   // os << "#endif /* ! _WIN32 */\n\n";
   
   // os << "#include <array>\n\n";

   // EINA_CXX_DOM_LOG_DBG(eolian::js::domain) << "includes added";

   // // generate open namespaces
   // if(namespace_size(klass))
   //   {
   //     std::string space = "";
   //     for(efl::eina::iterator<const char> first(::eolian_class_namespaces_get(klass)), last; first != last; ++first)
   //       {
   //         std::string lower(_lowercase(&*first));
   //         os << "namespace " << lower << " {" << space;
   //         space = " ";
   //       }

   //     os << "\n";
   //   }

   // // generate event map
   // std::string event_map = class_name;
   // event_map += "_ev_info_map";

   // os << "namespace {\n";
   // os << "::efl::eo::js::event_information_map " << event_map << ";\n";
   // os << "}\n";

   // EINA_CXX_DOM_LOG_DBG(eolian::js::domain) << "namespace";

   // // save functions that need a name getter for structs
   // std::set<std::string> need_name_getter;

   // // generate all structs parsed in this file
   // std::stringstream structs_ss;
   // for (efl::eina::iterator<Eolian_Typedecl> first(::eolian_typedecl_structs_get_by_file(eolian_unit, file_basename.c_str()))
   //      , last; first != last; ++first)
   //   {
   //      std::stringstream ss;
   //      bool should_reject_ref = file_basename == "efl_object.eo";
   //      bool has_ref_field = false;

   //      auto tpd = &*first;
   //      if (!tpd || ::eolian_typedecl_type_get(tpd) == EOLIAN_TYPEDECL_STRUCT_OPAQUE)
   //        continue;

   //      auto struct_name = ::eolian_typedecl_name_get(tpd);
   //      auto struct_type_full_name = ::eolian_typedecl_full_name_get(tpd);
   //      if (!struct_name || !struct_type_full_name)
   //        {
   //           EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "Could not get struct type name";
   //           continue;
   //        }
   //      else if(strcmp(struct_type_full_name, "Efl.Callback_Array_Item") == 0)
   //        continue;
   //      std::string struct_c_name = struct_type_full_name;
   //      std::replace(struct_c_name.begin(), struct_c_name.end(), '.', '_');
   //      ss << "  {\n";
   //      ss << "    auto fields_func = [](v8::Isolate* isolate_, v8::Local<v8::ObjectTemplate> prototype_)\n";
   //      ss << "    {\n";
   //      for (efl::eina::iterator<Eolian_Struct_Type_Field> sf(::eolian_typedecl_struct_fields_get(tpd))
   //           , sf_end; sf != sf_end; ++sf)
   //        {
   //           auto field_type = ::eolian_typedecl_struct_field_type_get(&*sf);
   //           auto field_name = ::eolian_typedecl_struct_field_name_get(&*sf);
   //           if (!field_name)
   //             {
   //                EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "Could not get struct field name";
   //                continue;
   //             }
   //           if (should_reject_ref && eolian_type_is_ptr(field_type))
   //             {
   //                has_ref_field = true;
   //                break;
   //             }
   //           std::string field_type_tag_name;
   //           try
   //             {
   //                field_type_tag_name = _eolian_type_cpp_type_named_get(field_type, class_name, need_name_getter);
   //             }
   //           catch(eolian::js::incomplete_complex_type_error const& e)
   //             {
   //                EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "Exception while generating '" << field_name << "' fielf of '" << struct_type_full_name << "' struct: " << e.what();
   //                continue;
   //             }
   //           std::string member_ref = struct_c_name;
   //           member_ref += "::";
   //           member_ref += field_name;

   //           auto k = type_class_name(field_type);
   //           if (!k.empty())
   //             {
   //                need_name_getter.insert(k);
   //                k = _class_name_getter(class_name, k);
   //             }
   //           else
   //             {
   //                k = "::efl::eina::js::nonclass_cls_name_getter";
   //             }
   //           ss << "      prototype_->SetAccessor(::efl::eina::js::compatibility_new<v8::String>(isolate_, \"" << format::format_field(field_name) << "\"),\n";
   //           ss << "        static_cast<v8::AccessorGetterCallback>(&::efl::eo::js::get_struct_member<" << struct_c_name << ", decltype(" << member_ref << "), &" << member_ref << ", " << k << ">),\n";
   //           ss << "        static_cast<v8::AccessorSetterCallback>(&::efl::eo::js::set_struct_member<" << struct_c_name << ", " << field_type_tag_name << ", decltype(" << member_ref << "), &" << member_ref << ", " << k << ">));\n";
   //        }

   //      if (should_reject_ref && has_ref_field)
   //        continue;
   //      ss << "    };\n";
   //      ss << "    auto to_export = ::efl::eo::js::get_namespace({";
   //      bool comma = false;
   //      for (efl::eina::iterator<const char> ns_it(::eolian_typedecl_namespaces_get(tpd)), ns_end; ns_it != ns_end; ++ns_it)
   //        {
   //          if (comma)
   //            ss << ", ";
   //          comma = true;
   //          ss << '"' << format::format_namespace(&*ns_it) << '"';
   //        }
   //      ss << "}, isolate, global);\n";
   //      ss << "    ::efl::eo::js::register_struct<" << struct_c_name << ">(isolate, \""
   //         << format::format_struct(struct_name) << "\", \"" << struct_type_full_name << "\", to_export, fields_func);\n";
   //      ss << "  }\n";

   //      structs_ss << ss.str();
   //   }

   // // generate register function for V8
   // std::stringstream register_from_constructor_begin_ss;
   // register_from_constructor_begin_ss
   //    << "EAPI v8::Local<v8::ObjectTemplate>\n"
   //    << "register_" << lower_case_class_name << "_from_constructor\n"
   //    << "(v8::Isolate* isolate, v8::Handle<v8::FunctionTemplate> constructor, ::efl::eina::js::global_ref<v8::Function>* constructor_from_eo)\n"
   //    << "{\n"
   //    << "  v8::Local<v8::ObjectTemplate> instance = constructor->InstanceTemplate();\n"
   //    << "  instance->SetInternalFieldCount(1);\n"
   //    << "  v8::Handle<v8::ObjectTemplate> prototype = constructor->PrototypeTemplate();\n";

   // std::stringstream functions_ss;
   // std::set<std::string> member_names;
   // std::set<std::string> event_member_names;
   // for(auto function : normal_functions)
   //   {
   //     std::vector<Eolian_Function_Type> function_types;
   //     switch (eolian_function_type_get(function))
   //       {
   //       case EOLIAN_METHOD:
   //         function_types = {EOLIAN_METHOD};
   //         break;
   //       case EOLIAN_PROPERTY:
   //         function_types = {EOLIAN_PROP_GET, EOLIAN_PROP_SET};
   //         break;
   //       case EOLIAN_PROP_GET:
   //         function_types = {EOLIAN_PROP_GET};
   //         break;
   //       case EOLIAN_PROP_SET:
   //         function_types = {EOLIAN_PROP_SET};
   //         break;
   //       default:
   //         EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "Unresolved function type";
   //         continue;
   //       }

   //     // generate function registration
   //     for (const auto function_type : function_types)
   //       {
   //         if (eolian_function_scope_get(function, function_type) != EOLIAN_SCOPE_PUBLIC || !_function_is_generatable(function, function_type))
   //           continue; // Some properties may have public 'get' but protected 'set'.
   //         try
   //           {
   //             std::string member_name;
   //             switch (function_type)
   //               {
   //               case EOLIAN_METHOD:
   //                 member_name = eolian_function_name_get(function);
   //                 break;
   //               case EOLIAN_PROP_SET:
   //                 member_name = eolian_function_name_get(function) + std::string("_set");
   //                 break;
   //               case EOLIAN_PROP_GET:
   //                 member_name = eolian_function_name_get(function) + std::string("_get");
   //                 break;
   //               case EOLIAN_PROPERTY:
   //                 EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "EOLIAN_PROPERTY function type is invalid at this point";
   //                 return -1;
   //               case EOLIAN_UNRESOLVED:
   //               default:
   //                 EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "Unresolved function type";
   //                 return -1;
   //               }

   //             if(member_names.find(member_name) == member_names.end())
   //               {
   //                 member_names.insert(member_name);
   //                  std::stringstream ss;
   //                  auto output_begin = [&] (std::string name)
   //                    {
   //                       if(! ::eolian_function_is_constructor(function, klass))
   //                         ss << "  prototype->Set( ::efl::eina::js::compatibility_new<v8::String>(isolate, \""
   //                            << format::format_method(name) << "\")\n"
   //                            << "    , ::efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, &efl::eo::js::call_function\n"
   //                            << "    , efl::eo::js::call_function_data<\n"
   //                            << "      ::efl::eina::_mpl::tuple_c<std::size_t";
   //                    };

   //                  output_begin(member_name);

   //                  const auto key_params = _eolian_function_keys_get(function, function_type);
   //                  const auto parameters = _eolian_function_parameters_get(function, function_type);

   //                  std::vector<const ::Eolian_Function_Parameter*> full_params;
   //                  full_params.insert(end(full_params), begin(key_params), end(key_params));
   //                  // only one property_get parameter is translated as the function return in C
   //                  const auto param_as_return = (EOLIAN_PROP_GET == function_type) && (parameters.size() == 1)
   //                    && _function_return_is_missing(function, function_type);
   //                  if (!param_as_return)
   //                    full_params.insert(end(full_params), begin(parameters), end(parameters));

   //                  // call_function_data Ins
   //                  std::size_t i = 0;
   //                  for (auto parameter : full_params)
   //                    {
   //                      if (EOLIAN_PROP_SET == function_type)
   //                        ss << ", " << i;
   //                      else
   //                        if (EOLIAN_METHOD == function_type)
   //                          {
   //                             switch (eolian_parameter_direction_get(parameter))
   //                               {
   //                               case EOLIAN_IN_PARAM:
   //                               case EOLIAN_INOUT_PARAM:
   //                                 ss << ", " << i;
   //                               default: break;
   //                               }
   //                          }
   //                      ++i;
   //                    }

   //                  // call_function_data Outs
   //                  ss << ">\n      , ::efl::eina::_mpl::tuple_c<std::size_t";
   //                  auto key_count = key_params.size();
   //                  i = 0;
   //                  for (auto parameter : full_params)
   //                    {
   //                       // ignore keys
   //                       if (key_count > 0)
   //                         {
   //                            --key_count;
   //                            ++i;
   //                            continue;
   //                         }
                         
   //                       // properties doesn't support in/out/inout
   //                       if (EOLIAN_PROP_GET == function_type)
   //                         ss << ", " << i;
   //                       else
   //                         if (EOLIAN_METHOD == function_type)
   //                           {
   //                              switch (eolian_parameter_direction_get(parameter))
   //                                {
   //                                case EOLIAN_OUT_PARAM:
   //                                case EOLIAN_INOUT_PARAM:
   //                                  ss << ", " << i;
   //                                default: break;
   //                                }
   //                           }
   //                       ++i;
   //                    }

   //                  // call_function_data Ownership
   //                  ss << ">\n      , std::tuple<\n";
   //                  auto sep = "";
   //                  for (auto parameter : full_params)
   //                    {
   //                       auto type = eolian_parameter_type_get(parameter);
   //                       if(eolian_type_is_owned(type))
   //                         ss << sep << "       ::std::true_type";
   //                       else
   //                         ss << sep << "       ::std::false_type";
   //                       sep = ",\n";
   //                    }
                    

   //                  // call_function_data Return
   //                  ss << ">\n      , ";

   //                  const Eolian_Type *return_type = nullptr;
   //                  if (param_as_return)
   //                    {
   //                       return_type = eolian_parameter_type_get(parameters[0]);
   //                    }
   //                  else
   //                    {
   //                       return_type = ::eolian_function_return_type_get(function, function_type);
   //                    }
   //                  std::string param = "void";
   //                  if (nullptr != return_type)
   //                    {
   //                       param = _eolian_type_cpp_type_named_get(return_type, class_name, need_name_getter);
   //                    }
   //                  ss << param;
                    

   //                  // call_function_data Parameters
   //                  ss << "\n      , std::tuple<\n";
   //                  sep = "       ";
   //                  key_count = key_params.size();
   //                  for (auto parameter : full_params)
   //                    {
   //                       // TODO: REVIEW ALL THIS TOO!!!
   //                      auto type = eolian_parameter_type_get(parameter);
   //                      auto param = _eolian_type_cpp_type_named_get(type, class_name, need_name_getter);

   //                      if (!key_count && EOLIAN_PROP_GET == function_type)
   //                        param += "*";
   //                      else
   //                        {
   //                           switch(eolian_parameter_direction_get(parameter))
   //                             {
   //                             case EOLIAN_OUT_PARAM:
   //                             case EOLIAN_INOUT_PARAM:
   //                               param += "*";
   //                             default: break;
   //                             }
   //                        }
                        
   //                      ss << sep << param;
   //                      sep = ",\n       ";
                        
   //                      if (key_count > 0) --key_count;
   //                    }
                    

   //                  std::string param_class_names;
   //                  for (auto parameter : full_params)
   //                    {
   //                       param_class_names += '"' + type_class_name(::eolian_parameter_type_get(parameter)) + "\", ";
   //                    }
   //                  param_class_names += '"' + type_class_name(return_type) + '"';

   //                  std::string param_class_names_array = "std::array<const char*, ";
   //                  param_class_names_array += std::to_string(full_params.size() + 1);
   //                  param_class_names_array += ">{{" + param_class_names + "}}";

   //                  auto output_end = [&] (std::string const& name)
   //                    {
   //                       ss << "> >(isolate, " << param_class_names_array << ", & ::" << name << ")));\n";
   //                    };
   //                  switch (function_type)
   //                    {
   //               case EOLIAN_METHOD:
   //                 output_end(eolian_function_full_c_name_get(function, function_type, EINA_FALSE));
   //                 break;
   //                    case EOLIAN_PROP_SET:
   //                      output_end(eolian_function_full_c_name_get(function, function_type, EINA_FALSE) /*+ std::string("_set")*/);
   //                      break;
   //                    case EOLIAN_PROP_GET:
   //                      output_end(eolian_function_full_c_name_get(function, function_type, EINA_FALSE) /*+ std::string("_get")*/);
   //                      break;
   //                    case EOLIAN_PROPERTY:
   //                      EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "EOLIAN_PROPERTY function type is invalid at this point";
   //                      return -1;
   //                    case EOLIAN_UNRESOLVED:
   //                    default:
   //                      EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "Unresolved function type";
   //                      return -1;
   //                    }

   //                  // Write function to functions stream
   //                  functions_ss << ss.str();
   //               }
   //             else
   //               {
   //                      EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "Duplicate member function found in class: " <<
   //                          eolian_class_full_name_get(klass) << ": '" << member_name << "'";
   //               }
   //           }
   //         catch(eolian::js::incomplete_complex_type_error const& e)
   //           {
   //             EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "Exception while generating '" << eolian_function_name_get(function) << "': " << e.what();
   //           }
   //       }
   //   }

   // functions_ss << "\n  prototype->Set(::efl::eina::js::compatibility_new<v8::String>(isolate, \"cast\"),\n"
   //                 "      ::efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, &efl::eina::js::cast_function));\n\n";

   // // generate all events
   // std::stringstream events_ss;
   // auto generate_events = [&] (Eolian_Class const* klass)
   //   {
   //     std::stringstream ss;
   //     for(efl::eina::iterator< ::Eolian_Event> first ( ::eolian_class_events_get(klass))
   //           , last; first != last; ++first)
   //       {
   //         std::string event_name (::eolian_event_name_get(&*first));

   //         if (!eolian_event_is_beta(&*first) &&
   //             event_member_names.find(event_name) == event_member_names.end())
   //           {
   //              auto tp = eolian_event_type_get(&*first);
   //              ss << "  {\n";
   //              ss << "    static efl::eo::js::event_information ev_info{&constructor_from_eo, " << eolian_event_c_name_get(&*first);
   //              ss << ", &efl::eo::js::event_callback<";
   //              ss << (tp ? _eolian_type_cpp_type_named_get(tp, class_name, need_name_getter) : "void");
   //              ss << ">, \"" << type_class_name(tp) << "\"};\n";
   //              ss << "    " << event_map << "[\"" << event_name << "\"] = &ev_info;\n";
   //              ss << "  }\n";
   //              event_member_names.insert(event_name);
   //           }
   //       }
   //       events_ss << ss.str();
   //   };
   // generate_events(klass);
   // recurse_inherits(klass, generate_events);

   // std::stringstream register_from_constructor_end_ss;
   // register_from_constructor_end_ss
   //    << "  prototype->Set(::efl::eina::js::compatibility_new<v8::String>(isolate, \"on\")\n"
   //    << "    , ::efl::eina::js::compatibility_new<v8::FunctionTemplate>(isolate, &efl::eo::js::on_event\n"
   //    << "      , ::efl::eina::js::compatibility_new<v8::External>(isolate, &" << event_map << ")));\n"
   //    << "  static_cast<void>(prototype); /* avoid warnings */\n"
   //    << "  static_cast<void>(isolate); /* avoid warnings */\n"
   //    << "  static_cast<void>(constructor_from_eo); /* avoid warnings */\n"
   //    << "  return instance;\n"
   //    << "}\n\n";

   // std::stringstream name_getters_ss;
   // for (auto const& k : need_name_getter)
   //   {
   //      name_getters_ss << "  struct " << _class_name_getter(class_name, k) << " { static char const* class_name() { return \"" << k << "\"; } };\n";
   //   }

   // os << "namespace {\n";
   // os << name_getters_ss.str();
   // os << "}\n\n";

   // os << register_from_constructor_begin_ss.str();
   // os << functions_ss.str();
   // os << register_from_constructor_end_ss.str();

   // // generate main entry-point for generation
   // os << "EAPI void register_" << lower_case_class_name
   //    << "(v8::Handle<v8::Object> global, v8::Isolate* isolate)\n";
   // os << "{\n";
   // os << "  v8::Handle<v8::FunctionTemplate> constructor = ::efl::eina::js::compatibility_new<v8::FunctionTemplate>\n";
   // os << "    (isolate, efl::eo::js::constructor\n"
   //    << "     , efl::eo::js::constructor_data(isolate\n"
   //       "         , ";

   // EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "before print eo_class";

   // print_eo_class(klass, os);

   // EINA_CXX_DOM_LOG_WARN(eolian::js::domain) << "print eo_class";

   // for(auto function : constructor_functions)
   //   {
   //     auto ftype = eolian_function_type_get(function);
   //     if(ftype == EOLIAN_PROPERTY)
   //       ftype = EOLIAN_PROP_SET;
   //     os << "\n         , & ::"
   //        << eolian_function_full_c_name_get(function, ftype, EINA_FALSE);
   //   }

   // os  << "));\n";

   // os << "  static ::efl::eina::js::global_ref<v8::Function> constructor_from_eo;\n";
   // os << events_ss.str();
   // os << "  register_" << lower_case_class_name << "_from_constructor(isolate, constructor, &constructor_from_eo);\n";

   // os << "  constructor->SetClassName( ::efl::eina::js::compatibility_new<v8::String>(isolate, \""
   //    << format::format_class(class_name)
   //    << "\"));\n";

   // os << "  auto to_export = ::efl::eo::js::get_namespace({";
   // if (namespace_size(klass))
   //   {
   //      bool comma = false;
   //      for (efl::eina::iterator<const char> ns_it(::eolian_class_namespaces_get(klass)), ns_end; ns_it != ns_end; ++ns_it)
   //        {
   //           if (comma)
   //             os << ", ";
   //           comma = true;
   //           os << '"' << format::format_namespace(&*ns_it) << '"';
   //        }
   //   }
   // os << "}, isolate, global);\n";

   // os << "  to_export->Set( ::efl::eina::js::compatibility_new<v8::String>(isolate, \""
   //    << format::format_class(class_name) << "\")"
   //    << ", constructor->GetFunction());\n";


   // os << "  {\n";
   // os << "    v8::Handle<v8::FunctionTemplate> constructor = ::efl::eina::js::compatibility_new<v8::FunctionTemplate>\n";
   // os << "      (isolate, &efl::eo::js::construct_from_eo);\n";
   // os << "    constructor->SetClassName( ::efl::eina::js::compatibility_new<v8::String>(isolate, \""
   //    << format::format_class(class_name)
   //    << "\"));\n";
   // os << "    v8::Local<v8::ObjectTemplate> instance = "
   //    << "register_" << lower_case_class_name << "_from_constructor(isolate, constructor, &constructor_from_eo);\n";
   // os << "    ::efl::eina::js::make_persistent(isolate, instance);\n";
   // os << "    constructor_from_eo = {isolate, constructor->GetFunction()};\n";
   // os << "    ::efl::eina::js::register_class_constructor(\"" << class_full_name << "\", constructor_from_eo.handle());\n";
   // os << "  }\n";

   // os << structs_ss.str();

   // // generate enumerations
   // for (efl::eina::iterator<Eolian_Typedecl> first(::eolian_typedecl_enums_get_by_file(eolian_unit, file_basename.c_str()))
   //      , last; first != last; ++first)
   //   {
   //      auto tpd = &*first;
   //      if (::eolian_typedecl_is_extern(tpd))
   //        continue;
   //      std::string enum_name = ::eolian_typedecl_name_get(tpd);
   //      os << "  {\n";
   //      os << "    auto to_export = ::efl::eo::js::get_namespace({";
   //      bool comma = false;
   //      for (efl::eina::iterator<const char> ns_it(::eolian_typedecl_namespaces_get(tpd)), ns_end; ns_it != ns_end; ++ns_it)
   //        {
   //          if (comma)
   //            os << ", ";
   //          comma = true;
   //          os << '"' << format::format_namespace(&*ns_it) << '"';
   //        }
   //      os << "}, isolate, global);\n";
   //      os << "    v8::Handle<v8::Object> enum_obj = efl::eina::js::compatibility_new<v8::Object>(isolate);\n";
   //      os << "    to_export->Set(efl::eina::js::compatibility_new<v8::String>(isolate, \""
   //         << format::format_enum(enum_name) << "\"), enum_obj);\n";
   //      for (efl::eina::iterator<Eolian_Enum_Type_Field> ef(::eolian_typedecl_enum_fields_get(tpd))
   //           , ef_end; ef != ef_end; ++ef)
   //        {
   //           auto field_name = ::eolian_typedecl_enum_field_name_get(&*ef);
   //           auto field_c_name = ::eolian_typedecl_enum_field_c_name_get(&*ef);
   //           if (!field_name || !field_c_name)
   //             {
   //                EINA_CXX_DOM_LOG_ERR(eolian::js::domain) << "Could not get enum field name";
   //                continue;
   //             }
   //           os << "    enum_obj->Set(efl::eina::js::compatibility_new<v8::String>(isolate, \"" << format::constant(field_name) << "\"),\n";
   //           os << "                  efl::eina::js::compatibility_new<v8::Int32>(isolate, static_cast<int32_t>(::" << field_c_name << ")));\n";
   //        }
   //      os << "  }\n";
   //   }

   // os << "}\n\n";

   // for(std::size_t i = 0, j = namespace_size(klass); i != j; ++i)
   //   os << "}";
   // os << "\n";
}
