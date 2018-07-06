#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <locale.h>
#include <fnmatch.h>

#include <Ecore.h>
#include <Ecore_Getopt.h>

#include "Edje.h"

static int _log_dom;
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)
#define CRI(...) EINA_LOG_DOM_CRIT(_log_dom, __VA_ARGS__)

#define INDENT  "   "
#define INDENT2 INDENT INDENT
#define INDENT3 INDENT2 INDENT
#define INDENT4 INDENT3 INDENT

static char *module_patterns_str = NULL;

static int detail = 1;
static Eina_Bool machine = EINA_FALSE;
static char *type_glob = NULL;
static char *const *module_patterns;
static const Eina_List *modules;

static char *
_module_patterns_str_new(void)
{
   Eina_Strbuf *buf;
   char *const *itr;
   char *ret;
   if (!module_patterns) return strdup("*");

   buf = eina_strbuf_new();
   for (itr = module_patterns; *itr != NULL; itr++)
     {
        eina_strbuf_append(buf, *itr);
        if (itr[1]) eina_strbuf_append(buf, ", ");
     }
   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static Eina_Bool
module_matches(const char *name)
{
   char *const *itr;
   if (!module_patterns) return EINA_TRUE;

   for (itr = module_patterns; *itr != NULL; itr++)
     if (fnmatch(*itr, name, 0) == 0) return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
type_matches(const char *name)
{
   if (!type_glob) return EINA_TRUE;
   return fnmatch(type_glob, name, 0) == 0;
}

static int
_types_sort(const void *pa, const void *pb)
{
   const Eina_Hash_Tuple *ha = pa, *hb = pb;
   const Edje_External_Type *ta = ha->data, *tb = hb->data;
   const char *na = ha->key, *nb = hb->key;
   int r;

   if (!ta->module) return -1;
   if (!tb->module) return 1;
   r = strcmp(ta->module, tb->module);
   if (r != 0) return r;

   if (!na) return -1;
   if (!nb) return 1;
   return strcmp(na, nb);
}

static const char *
_param_type_str_get(const Edje_External_Param_Info *param)
{
   switch (param->type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_INT: return "int";

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE: return "double";

      case EDJE_EXTERNAL_PARAM_TYPE_STRING: return "string";

      case EDJE_EXTERNAL_PARAM_TYPE_BOOL: return "bool";

      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE: return "choice";

      default:
        ERR("Unknown parameter type %d", param->type);
        return "???";
     }
}

static const char *
_param_value_str_get(const Edje_External_Type *type, const Edje_External_Param_Info *param, char *buf, size_t buflen)
{
   switch (param->type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_INT:
        if (param->info.i.def == EDJE_EXTERNAL_INT_UNSET) return NULL;
        snprintf(buf, buflen, "%d", param->info.i.def);
        return buf;

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
        if (EINA_DBL_EQ(param->info.d.def, EDJE_EXTERNAL_DOUBLE_UNSET)) return NULL;
        snprintf(buf, buflen, "%g", param->info.d.def);
        return buf;

      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
        return param->info.s.def;

      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
        if (param->info.b.def == 0) return "0";
        else if (param->info.b.def == 1)
          return "1";
        return NULL;

      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
      {
         char *def;
         if (param->info.c.def) return param->info.c.def;
         if (!param->info.c.def_get) return NULL;
         def = param->info.c.def_get(type->data, param);
         if (!def) return NULL;
         eina_strlcpy(buf, def, buflen);
         free(def);
         return buf;
      }

      default:
        ERR("Unknown parameter type %d", param->type);
        return NULL;
     }
}

static const char *
_param_flags_str_get(const Edje_External_Param_Info *param)
{
   static char buf[] = "GET|SET|STATE|CONSTRUCTOR";

   if (param->flags == EDJE_EXTERNAL_PARAM_FLAGS_NONE) return "NONE";
   if (param->flags == EDJE_EXTERNAL_PARAM_FLAGS_REGULAR) return "REGULAR";

   buf[0] = '\0';

   if (param->flags & EDJE_EXTERNAL_PARAM_FLAGS_GET)
     strcat(buf, "GET");

   if (param->flags & EDJE_EXTERNAL_PARAM_FLAGS_SET)
     {
        if (buf[0] != '\0') strcat(buf, "|");
        strcat(buf, "SET");
     }

   if (param->flags & EDJE_EXTERNAL_PARAM_FLAGS_STATE)
     {
        if (buf[0] != '\0') strcat(buf, "|");
        strcat(buf, "STATE");
     }

   if (param->flags & EDJE_EXTERNAL_PARAM_FLAGS_CONSTRUCTOR)
     {
        if (buf[0] != '\0') strcat(buf, "|");
        strcat(buf, "CONSTRUCTOR");
     }

   return buf;
}

static void
_param_choices_print(const char *const *choices)
{
   if (machine) puts("CHOICES-BEGIN");
   else fputs(", choices:", stdout);
   for (; *choices != NULL; choices++)
     {
        if (machine) puts(*choices);
        else printf(" \"%s\"", *choices);
     }
   if (machine) puts("CHOICES-END");
}

static void
_param_extra_details(const Edje_External_Type *type, const Edje_External_Param_Info *param)
{
   const char *str = _param_flags_str_get(param);
   if (machine) printf("FLAGS: %s\n", str);
   else printf(" /* flags: %s", str);

   switch (param->type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_INT:
        if (param->info.i.min != EDJE_EXTERNAL_INT_UNSET)
          {
             if (machine) printf("MIN: %d\n", param->info.i.min);
             else printf(", min: %d", param->info.i.min);
          }
        if (param->info.i.max != EDJE_EXTERNAL_INT_UNSET)
          {
             if (machine) printf("MAX: %d\n", param->info.i.max);
             else printf(", max: %d", param->info.i.max);
          }
        if (param->info.i.step != EDJE_EXTERNAL_INT_UNSET)
          {
             if (machine) printf("STEP: %d\n", param->info.i.step);
             else printf(", step: %d", param->info.i.step);
          }
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
        if (EINA_DBL_EQ(param->info.d.min, EDJE_EXTERNAL_DOUBLE_UNSET))
          {
             if (machine) printf("MIN: %g\n", param->info.d.min);
             else printf(", min: %g", param->info.d.min);
          }
        if (EINA_DBL_EQ(param->info.d.max, EDJE_EXTERNAL_DOUBLE_UNSET))
          {
             if (machine) printf("MAX: %g\n", param->info.d.max);
             else printf(", max: %g", param->info.d.max);
          }
        if (EINA_DBL_EQ(param->info.d.step, EDJE_EXTERNAL_DOUBLE_UNSET))
          {
             if (machine) printf("STEP: %g\n", param->info.d.step);
             else printf(", step: %g", param->info.d.step);
          }
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
        if (param->info.s.accept_fmt)
          {
             if (machine) printf("ACCEPT_FMT: %s\n", param->info.s.accept_fmt);
             else printf(", accept_fmt: \"%s\"", param->info.s.accept_fmt);
          }
        if (param->info.s.deny_fmt)
          {
             if (machine) printf("DENY_FMT: %s\n", param->info.s.deny_fmt);
             else printf(", deny_fmt: \"%s\"", param->info.s.deny_fmt);
          }
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
        if (param->info.b.false_str)
          {
             if (machine) printf("FALSE_STR: %s\n", param->info.b.false_str);
             else printf(", false_str: \"%s\"", param->info.b.false_str);
          }
        if (param->info.b.true_str)
          {
             if (machine) printf("TRUE_STR: %s\n", param->info.b.true_str);
             else printf(", true_str: \"%s\"", param->info.b.true_str);
          }
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
      {
         if (param->info.c.choices)
           _param_choices_print(param->info.c.choices);
         else if (param->info.c.query)
           {
              char **choices = param->info.c.query(type->data, param);
              if (choices)
                {
                   char **itr;
                   _param_choices_print((const char *const *)choices);
                   for (itr = choices; *itr; itr++)
                     free(*itr);
                   free(choices);
                }
           }
      }
      break;

      default:
        ERR("Unknown parameter type %d", param->type);
     }

   if (!machine) fputs(" */", stdout);  /* \n not desired */
}

static int
_info_list(void)
{
   Eina_Iterator *itr;
   Eina_List *types;
   const Eina_Hash_Tuple *tuple;
   const Eina_List *l;
   const char *name, *last_module;
   Eina_Bool module_found = EINA_FALSE, type_found = EINA_FALSE;
   Eina_Bool in_module = EINA_FALSE;

   EINA_LIST_FOREACH(modules, l, name)
     {
        if (!module_matches(name))
          {
             DBG("filter out module '%s': does not match '%s'",
                 name, module_patterns_str);
             continue;
          }

        if (!edje_module_load(name))
          {
             ERR("error loading external '%s'", name);
             continue;
          }

        module_found = EINA_TRUE;
     }

   itr = edje_external_iterator_get();
   types = NULL;
   EINA_ITERATOR_FOREACH(itr, tuple)
     {
        const Edje_External_Type *type = tuple->data;
        name = tuple->key;

        if (!type)
          {
             ERR("no type value for '%s'", name);
             continue;
          }
        else if (type->abi_version != edje_external_type_abi_version_get())
          {
             ERR("type '%s' with incorrect abi_version %u (expected %u)",
                 name, type->abi_version, edje_external_type_abi_version_get());
             continue;
          }

        if (!type_matches(name))
          {
             DBG("filter out type '%s': does not match '%s'", name, type_glob);
             continue;
          }

        types = eina_list_append(types, tuple);
        type_found = EINA_TRUE;
     }
   eina_iterator_free(itr);

   last_module = NULL;
   types = eina_list_sort(types, 0, _types_sort);
   EINA_LIST_FREE(types, tuple)
     {
        Eina_Bool changed_module = EINA_FALSE;
        const Edje_External_Type *type = tuple->data;
        const Edje_External_Param_Info *param;
        name = tuple->key;

        if ((last_module) && (type->module))
          {
             changed_module = ((last_module != type->module) &&
                               (!strcmp(last_module, type->module)));
          }
        else if ((!last_module) && (type->module))
          changed_module = EINA_TRUE;

        if (changed_module)
          {
             if (in_module)
               {
                  if (machine) puts("TYPES-END\nMODULE-END");
                  else puts(INDENT "}\n}");
               }

             if (machine)
               printf("MODULE-BEGIN\n"
                      "NAME: %s\n"
                      "FRIENDLY-NAME: %s\n"
                      "TYPES-BEGIN\n",
                      type->module, type->module_name);
             else
               printf("module {\n"
                      INDENT "name: \"%s\";\n"
                      INDENT "friendly_name: \"%s\";\n"
                      INDENT "types {\n",
                      type->module, type->module_name);

             in_module = EINA_TRUE;
          }

        if (machine) printf("TYPE-BEGIN\nNAME: %s\n", name);
        else printf(INDENT2 "type {\n" INDENT3 "name: \"%s\";\n", name);

        if (detail > 1)
          {
             const char *str;

             if (!type->label_get) str = NULL;
             else str = type->label_get(type->data);
             if (machine) printf("LABEL: %s\n", str ? str : "");
             else if (str)
               printf(INDENT3 "label: \"%s\";\n", str);

             if (!type->description_get) str = NULL;
             else str = type->description_get(type->data);
             if (machine) printf("DESCRIPTION: %s\n", str ? str : "");
             else if (str)
               printf(INDENT3 "description: \"%s\";\n", str);
          }

        if (machine) puts("PARAMS-BEGIN");
        else puts(INDENT3 "params {");

        for (param = type->parameters_info; param->name != NULL; param++)
          {
             const char *pt = _param_type_str_get(param);
             char buf[128];

             if (machine)
               printf("PARAM-BEGIN\nNAME: %s\nTYPE: %s\n", param->name, pt);
             else printf(INDENT4 "%s: \"%s\"", pt, param->name);

             if (detail > 0)
               {
                  const char *str = _param_value_str_get
                      (type, param, buf, sizeof(buf));
                  if (machine) printf("DEFAULT: %s\n", str ? str : "");
                  else if (str)
                    printf(" \"%s\"", str);

                  if (detail > 1)
                    {
                       if (!machine) putchar(';');
                       _param_extra_details(type, param);
                    }
               }

             if (machine) puts("PARAM-END");
             else if (detail > 1)
               putchar('\n');
             else puts(";");
          }

        if (machine) puts("PARAMS-END\nTYPE-END");
        else puts(INDENT3 "}\n" INDENT2 "}");

        last_module = type->module;
     }

   if (in_module)
     {
        if (machine) puts("MODULE-END");
        else puts(INDENT "}\n}");
     }

   if (!module_found) WRN("no modules match '%s'", module_patterns_str);
   if (!type_found) WRN("no types match '%s'", type_glob);
   return (!module_found) || (!type_found);
}

static int
_types_names_list(void)
{
   Eina_Iterator *itr;
   Eina_List *types;
   const Eina_Hash_Tuple *tuple;
   const Eina_List *l;
   const char *name;
   Eina_Bool module_found = EINA_FALSE, type_found = EINA_FALSE;

   EINA_LIST_FOREACH(modules, l, name)
     {
        if (!module_matches(name))
          {
             DBG("filter out module '%s': does not match '%s'",
                 name, module_patterns_str);
             continue;
          }

        if (!edje_module_load(name))
          {
             ERR("error loading external '%s'", name);
             continue;
          }

        module_found = EINA_TRUE;
     }

   itr = edje_external_iterator_get();
   types = NULL;
   EINA_ITERATOR_FOREACH(itr, tuple)
     {
        const Edje_External_Type *type = tuple->data;
        name = tuple->key;

        if (!type)
          {
             ERR("no type value for '%s'", name);
             continue;
          }
        else if (type->abi_version != edje_external_type_abi_version_get())
          {
             ERR("type '%s' with incorrect abi_version %u (expected %u)",
                 name, type->abi_version, edje_external_type_abi_version_get());
             continue;
          }

        if (!type_matches(name))
          {
             DBG("filter out type '%s': does not match '%s'", name, type_glob);
             continue;
          }

        types = eina_list_append(types, tuple);
        type_found = EINA_TRUE;
     }
   eina_iterator_free(itr);

   types = eina_list_sort(types, 0, _types_sort);
   EINA_LIST_FREE(types, tuple)
     puts(tuple->key);

   if (!module_found) WRN("no modules match '%s'", module_patterns_str);
   if (!type_found) WRN("no types match '%s'", type_glob);
   return (!module_found) || (!type_found);
}

static int
_modules_names_list(void)
{
   const Eina_List *l;
   const char *name;
   Eina_Bool found = EINA_FALSE;

   EINA_LIST_FOREACH(modules, l, name)
     {
        if (!module_matches(name))
          {
             DBG("filter out module '%s': does not match '%s'",
                 name, module_patterns_str);
             continue;
          }
        found = EINA_TRUE;
        puts(name);
     }

   if (!found) WRN("no modules match '%s'", module_patterns_str);
   return !found;
}

static const char *mode_choices[] = {
   "info",
   "modules-names",
   "types-names",
   NULL,
};

static const char *detail_choices[] = {
   "none",
   "terse",
   "all",
   NULL
};

const Ecore_Getopt optdesc = {
   "edje_external_inspector",
   "%prog [options] [module|module-glob] ... [module|module-glob]",
   PACKAGE_VERSION,
   "(C) 2010 - The Enlightenment Project",
   "BSD",
   "Edje external module inspector.",
   0,
   {
      ECORE_GETOPT_CHOICE('m', "mode", "Choose which mode to operate.",
                          mode_choices),
      ECORE_GETOPT_STORE_STR('t', "type", "Limit output to type (or glob)."),
      ECORE_GETOPT_CHOICE('d', "detail", "Choose detail level (default=terse)",
                          detail_choices),
      ECORE_GETOPT_STORE_TRUE('M', "machine", "Produce machine readable output."),
      ECORE_GETOPT_LICENSE('L', "license"),
      ECORE_GETOPT_COPYRIGHT('C', "copyright"),
      ECORE_GETOPT_VERSION('V', "version"),
      ECORE_GETOPT_HELP('h', "help"),
      ECORE_GETOPT_SENTINEL
   }
};

int
main(int argc, char **argv)
{
   Eina_Bool quit_option = EINA_FALSE;
   char *mode = NULL;
   char *detail_name = NULL;
   int arg_index;
   int ret = 0;
   Ecore_Getopt_Value values[] = {
      ECORE_GETOPT_VALUE_STR(mode),
      ECORE_GETOPT_VALUE_STR(type_glob),
      ECORE_GETOPT_VALUE_STR(detail_name),
      ECORE_GETOPT_VALUE_BOOL(machine),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_BOOL(quit_option),
      ECORE_GETOPT_VALUE_NONE
   };

   setlocale(LC_NUMERIC, "C");

   ecore_app_no_system_modules();

   ecore_init();
   eina_init();
   edje_init();

   _log_dom = eina_log_domain_register
       ("edje_external_inspector", EINA_COLOR_YELLOW);
   if (_log_dom < 0)
     {
        EINA_LOG_CRIT
          ("could not register log domain 'edje_external_inspector'");
        ret = 1;
        goto error_log;
     }

   arg_index = ecore_getopt_parse(&optdesc, values, argc, argv);
   if (arg_index < 0)
     {
        ERR("could not parse arguments.");
        ret = 1;
        goto error_getopt;
     }
   else if (quit_option)
     goto error_getopt;

   if (!mode) mode = (char *)mode_choices[0];

   if (detail_name)
     {
        if (!strcmp(detail_name, "none")) detail = 0;
        else if (!strcmp(detail_name, "terse"))
          detail = 1;
        else if (!strcmp(detail_name, "all"))
          detail = 2;
        else ERR("Unknown detail level: '%s'", detail_name);
     }

   if (arg_index < argc) module_patterns = argv + arg_index;
   else module_patterns = NULL;

   modules = edje_available_modules_get();
   module_patterns_str = _module_patterns_str_new();

   if (!strcmp(mode, "info")) ret = _info_list();
   else if (!strcmp(mode, "modules-names"))
     ret = _modules_names_list();
   else if (!strcmp(mode, "types-names"))
     ret = _types_names_list();
   else
     {
        ERR("Unknown mode: %s", mode);
        ret = 1;
     }

   free(module_patterns_str);

error_getopt:
   eina_log_domain_unregister(_log_dom);
error_log:
   edje_shutdown();
   ecore_shutdown();
   eina_shutdown();

   return ret;
}

