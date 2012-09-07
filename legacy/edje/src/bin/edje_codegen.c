#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "Edje.h"
#define EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT 1
#include "Edje_Edit.h"

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Getopt.h>
#include <Eina.h>

#include <ctype.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <unistd.h>

static int _log_dom;
#define DBG(...)  EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)
#define ERR(...)  EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)

static Ecore_Evas *ee = NULL;
static char *file = NULL;
static char *group = NULL;
static char *prefix = NULL;
static FILE *source_fd = NULL;
static FILE *header_fd = NULL;

#define H_HEADER                                        \
  "#ifndef _%s\n"                                       \
  "#define _%s\n\n"                                     \
  "#include <Edje.h>\n"                                 \
  "#include <Evas.h>\n\n"                               \
  "#include <stdlib.h>\n\n"

#define H_FOOTER                                        \
  "\n#endif /* _%s */\n"

#define C_HEADER                                        \
  "#include \"%s\"\n\n"

#define C_CODEGEN_OBJECT_ADD                              \
  "Evas_Object *\n"                                       \
  "%s_object_add(Evas *e, const char *file)\n"            \
  "{\n"                                                   \
  "   Evas_Object *o;\n\n"                                \
  "   o = edje_object_add(e);\n"                          \
  "   if (!o) return NULL;\n\n"                           \
  "   if (file)\n"                                        \
  "      edje_object_file_set(o, file, \"%s\");\n"        \
  "   else\n"                                             \
  "      edje_object_file_set(o, \"%s\", \"%s\");\n\n"    \
  "   return o;\n"                                        \
  "}\n\n"

#define H_CODEGEN_OBJECT_ADD                                           \
  "/**\n * @brief Creates the Edje object and set the edj file\n"      \
  " * @param e The surface\n"                                          \
  " * @param file The path to edj, if NULL it's used the path given\n" \
  " *             to edje_codegen\n */\n"                              \
  "Evas_Object * %s_object_add(Evas *e, const char *file);\n\n"

#define C_CODEGEN_PART_TEXT_SET                         \
  "void\n"                                              \
  "%s_%s_set(Evas_Object *o, const char *value)\n"      \
  "{\n"                                                 \
  "   edje_object_part_text_set(o, \"%s\", value);\n"   \
  "}\n\n"

#define C_CODEGEN_PART_SWALLOW_SET                             \
  "void\n"                                                     \
  "%s_%s_set(Evas_Object *o, Evas_Object *value)\n"            \
  "{\n"                                                        \
  "   edje_object_part_swallow(o, \"%s\", value);\n"           \
  "}\n\n"

#define H_CODEGEN_PART_SET                              \
  "void %s_%s_set(Evas_Object *o, %svalue);\n"

#define C_CODEGEN_PART_GET                              \
  "%s\n"                                                \
  "%s_%s_get(const Evas_Object *o)\n"                   \
  "{\n"                                                 \
  "   return edje_object_part_%s_get(o, \"%s\");\n"     \
  "}\n\n"

#define H_CODEGEN_PART_GET                              \
  "%s %s_%s_get(const Evas_Object *o);\n"

#define C_CODEGEN_PROGRAM_EMIT                          \
  "void\n"                                              \
  "%s_%s_emit(Evas_Object *o)\n"                        \
  "{\n"                                                 \
  "   edje_object_signal_emit(o, \"%s\", \"%s\");\n"    \
  "}\n\n"

#define H_CODEGEN_PROGRAM_EMIT	        \
  "void %s_%s_emit(Evas_Object *o);\n"

#define C_CODEGEN_PROGRAM_CALLBACK_ADD                                        \
  "void\n"                                                                    \
  "%s_%s_callback_add(Evas_Object *o, Edje_Signal_Cb func, void *data)\n"     \
  "{\n"                                                                       \
  "   edje_object_signal_callback_add(o, \"%s\", \"%s\", func, data);\n"      \
  "}\n\n"

#define H_CODEGEN_PROGRAM_CALLBACK_ADD                                  \
  "void %s_%s_callback_add(Evas_Object *o, Edje_Signal_Cb func, "       \
  "void *data);\n"

#define C_CODEGEN_PROGRAM_CALLBACK_DEL                                          \
  "void\n"                                                                      \
  "%s_%s_callback_del_full(Evas_Object *o, Edje_Signal_Cb func, void *data)\n"  \
  "{\n"                                                                         \
  "   edje_object_signal_callback_del_full(o, \"%s\", \"%s\", func, data);\n"   \
  "}\n\n"

#define H_CODEGEN_PROGRAM_CALLBACK_DEL                                  \
  "void %s_%s_callback_del_full(Evas_Object *o, Edje_Signal_Cb func, "  \
  "void *data);\n"


const Ecore_Getopt optdesc = {
  "edje_codegen",
  "%prog [options] <file.edj> <group> <source_file_name> <header_file_name>",
  PACKAGE_VERSION,
  "(C) 2012 - The Enlightenment Project",
  "BSD",
  "Edje generates the boilerplate code to get and set the "
  "parts of a group from a compiled (binary) edje "
  "file avoiding common errors with typos.\n",
  0,
  {
    ECORE_GETOPT_STORE_STR('p', "prefix", "The prefix for the " \
			   "generataed code."),
    ECORE_GETOPT_LICENSE('L', "license"),
    ECORE_GETOPT_COPYRIGHT('C', "copyright"),
    ECORE_GETOPT_VERSION('V', "version"),
    ECORE_GETOPT_HELP('h', "help"),
    ECORE_GETOPT_SENTINEL
  }
};

static char *
_standardizes_header(const char *filename)
{
   char *str, *itr, *aux;

   aux = strrchr(filename, '/');
   str = itr = strdup(aux ? aux + 1 : filename);

   for (; *itr; itr++)
     if (*itr == '.')
       *itr = '_';
     else
       *itr = toupper(*itr);

   return str;
}

static Eina_Bool
_open_file_descriptors(const char *source, const char *header)
{
   header_fd = fopen(header, "w");
   if (!header_fd)
     return EINA_FALSE;

   source_fd = fopen(source, "w");
   if (!source_fd)
     goto err;

   return EINA_TRUE;

 err:
   fclose(header_fd);
   return EINA_FALSE;
}

static Eina_Bool
_close_file_descriptors(void)
{
   Eina_Bool ret = EINA_FALSE;

   if (!fclose(header_fd))
     ret = EINA_TRUE;

   if (!fclose(source_fd))
     ret &= EINA_TRUE;

   return ret;
}

static const char *
_part_type_name_get(Edje_Part_Type t)
{
   switch (t)
     {
      case EDJE_PART_TYPE_TEXT:
         return "text";
      case EDJE_PART_TYPE_SWALLOW:
         return "swallow";

      case EDJE_PART_TYPE_NONE:
      case EDJE_PART_TYPE_LAST:
         ERR("Invalid part type %d", t);
      default:
         return NULL;
     }

   return NULL;
}

static Eina_Bool
_write_headers(const char *filename)
{
   char buf[512];
   char *str;

   str = _standardizes_header(filename);
   snprintf(buf, sizeof(buf), H_HEADER, str, str);
   if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
     {
	free(str);
	return EINA_FALSE;
     }

   free(str);

   snprintf(buf, sizeof(buf), C_HEADER, filename);
   if (fwrite(buf, strlen(buf), 1, source_fd) != 1)
	return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_write_footer(const char *filename)
{
   char buf[512];
   char *str;

   str = _standardizes_header(filename);
   snprintf(buf, sizeof(buf), H_FOOTER, str);
   if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
     {
	free(str);
	return EINA_FALSE;
     }

   free(str);

   return EINA_TRUE;
}

static Eina_Bool
_write_object_get(void)
{
   char buf[512];

   snprintf(buf, sizeof(buf), H_CODEGEN_OBJECT_ADD, prefix);
   if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
     return EINA_FALSE;

   snprintf(buf, sizeof(buf), C_CODEGEN_OBJECT_ADD, prefix, group, file, group);
   if (fwrite(buf, strlen(buf), 1, source_fd) != 1)
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_write_part(const char *apiname, const char *partname,
	    const char *parttype, const char *description)
{
   char buf[512];
   const char *type, *template;

   if (!strcmp(parttype, "text"))
     {
        type = "const char *";
        template = C_CODEGEN_PART_TEXT_SET;
     }
   else
     {
        type = "Evas_Object *";
        template = C_CODEGEN_PART_SWALLOW_SET;
     }

   snprintf(buf, sizeof(buf), template, prefix, apiname, partname);
   if (fwrite(buf, strlen(buf), 1, source_fd) != 1)
     goto err;

   if (description)
     {
	snprintf(buf, sizeof(buf), "\n/**\n * @brief %s\n */\n", description);
	if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
	  goto err;
     }

   snprintf(buf, sizeof(buf), H_CODEGEN_PART_SET, prefix, apiname,
	    type);
   if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
     goto err;

   snprintf(buf, sizeof(buf), H_CODEGEN_PART_GET, type, prefix, apiname);
   if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
     goto err;

   snprintf(buf, sizeof(buf), C_CODEGEN_PART_GET, type, prefix, apiname,
	    parttype, partname);
   if (fwrite(buf, strlen(buf), 1, source_fd) != 1)
     goto err;

   return EINA_TRUE;

 err:
   ERR("Could not write the part: %s", partname);
   return EINA_FALSE;
}

static inline Eina_Bool
_c_id_allowed(char c)
{
   if ((c >= '0') && (c <= '9')) return EINA_TRUE;
   if ((c >= 'a') && (c <= 'z')) return EINA_TRUE;
   if ((c >= 'A') && (c <= 'Z')) return EINA_TRUE;

   return EINA_FALSE;
}

static char *
_api_name_fix(const char *orig)
{
   char *d, *d_end, buf[256];
   const char *s;

   if (!orig) return NULL;

   s = orig;
   d = buf;
   d_end = d + sizeof(buf) - 1;

   for (; (*s != '\0') && (d < d_end); s++, d++)
     if (_c_id_allowed(*s)) *d = *s;
     else *d = '_';
   *d = '\0';

   return strdup(buf);
}

static char *
_part_api_name_get(Evas_Object *ed, const char *program)
{
   const char *orig;
   char *fix;

   orig = edje_edit_part_api_name_get(ed, program);
   fix = _api_name_fix(orig);
   edje_edit_string_free(orig);

   return fix;
}

static Eina_Bool
_parse_parts(Evas_Object *ed)
{
   Eina_List *parts, *l;
   const char *name, *typename, *description;
   char *apiname;
   Edje_Part_Type type;
   Eina_Bool ret = EINA_TRUE;

   parts = edje_edit_parts_list_get(ed);
   EINA_LIST_FOREACH(parts, l, name)
     {
	if (!(apiname = _part_api_name_get(ed, name)))
	  {
	     DBG("filter out part '%s': not API.", name);
	     continue;
	  }

	type = edje_edit_part_type_get(ed, name);
	typename = _part_type_name_get(type);
	if (!typename)
	  {
	     free(apiname);
	     continue;
	  }

	description = edje_edit_part_api_description_get(ed, name);
	if (!_write_part(apiname, name, typename, description))
	  {
	     ret = EINA_FALSE;
	     edje_edit_string_free(description);
	     free(apiname);
	     break;
	  }

	edje_edit_string_free(description);
	free(apiname);
     }

   edje_edit_string_list_free(parts);
   return ret;
}

static Eina_Bool
_write_program_emit(const char *apiname, const char *source, const char *signal,
		    const char *description)
{
   char buf[512];

   snprintf(buf, sizeof(buf), C_CODEGEN_PROGRAM_EMIT, prefix,
	    apiname, signal, source);
   if (fwrite(buf, strlen(buf), 1, source_fd) != 1)
     goto err;

   if (description)
     {
	snprintf(buf, sizeof(buf), "\n/**\n * @brief %s\n */\n", description);
	if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
	  goto err;
     }

   snprintf(buf, sizeof(buf), H_CODEGEN_PROGRAM_EMIT, prefix,
	    apiname);
   if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
     goto err;

   return EINA_TRUE;

 err:
   ERR("Could not write the program: %s", apiname);
   return EINA_FALSE;
}

static Eina_Bool
_write_program_add(const char *apiname, const char *source, const char *signal,
		   const char *description)
{
  char buf[512];

   snprintf(buf, sizeof(buf), C_CODEGEN_PROGRAM_CALLBACK_ADD, prefix,
	    apiname, signal, source);
   if (fwrite(buf, strlen(buf), 1, source_fd) != 1)
     goto err;

   snprintf(buf, sizeof(buf), C_CODEGEN_PROGRAM_CALLBACK_DEL, prefix,
	    apiname, signal, source);
   if (fwrite(buf, strlen(buf), 1, source_fd) != 1)
     goto err;

   if (description)
     {
	snprintf(buf, sizeof(buf), "\n/**\n * @brief %s\n */\n", description);
	if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
	  goto err;
     }

   snprintf(buf, sizeof(buf), H_CODEGEN_PROGRAM_CALLBACK_ADD, prefix,
	    apiname);
   if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
     goto err;

   snprintf(buf, sizeof(buf), H_CODEGEN_PROGRAM_CALLBACK_DEL, prefix,
	    apiname);
   if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
     goto err;

   return EINA_TRUE;

 err:
   ERR("Could not write the program [action]: %s", apiname);
   return EINA_FALSE;
}

static char *
_program_api_name_get(Evas_Object *ed, const char *program)
{
   const char *orig;
   char *fix;

   orig = edje_edit_program_api_name_get(ed, program);
   fix = _api_name_fix(orig);
   edje_edit_string_free(orig);

   return fix;
}

static Eina_Bool
_parse_programs(Evas_Object *ed)
{
   Eina_Bool ret = EINA_TRUE;
   Eina_List *programs, *l;
   const char *name, *source, *signal, *description;
   char *apiname;
   Edje_Action_Type type;

   programs = edje_edit_programs_list_get(ed);
   EINA_LIST_FOREACH(programs, l, name)
     {
	if (!(apiname = _program_api_name_get(ed, name)))
	  {
	     DBG("filter out program '%s': not API.", name);
	     continue;
	  }

	signal = edje_edit_program_signal_get(ed, name);
	if (!signal)
	  {
	     free(apiname);
	     continue;
	  }

	source = edje_edit_program_source_get(ed, name);
	if (!source)
	  {
	     free(apiname);
	     edje_edit_string_free(signal);
	     continue;
	  }

	description = edje_edit_program_api_description_get(ed, name);
	if (!_write_program_emit(apiname, source, signal, description))
	  {
	     ret = EINA_FALSE;
	     break;
	  }

	type = edje_edit_program_action_get(ed, name);
	if (type == EDJE_ACTION_TYPE_SIGNAL_EMIT)
	  {
	     const char *str, *str2;
	     str = edje_edit_program_state_get(ed, name);
	     str2 = edje_edit_program_state2_get(ed, name);

	     if (!_write_program_add(apiname, str2, str, description))
	       {
		  ret = EINA_FALSE;
		  edje_edit_string_free(str);
		  edje_edit_string_free(str2);
		  break;
	       }

	     edje_edit_string_free(str);
	     edje_edit_string_free(str2);
	  }

	edje_edit_string_free(description);
	edje_edit_string_free(signal);
	edje_edit_string_free(source);
	free(apiname);
     }

   edje_edit_string_list_free(programs);
   if (!ret)
     {
	edje_edit_string_free(description);
	edje_edit_string_free(signal);
	edje_edit_string_free(source);
	free(apiname);
     }

   return ret;
}

static Eina_Bool
_parse(void)
{
   Evas_Object *ed;
   Eina_Bool ret;

   ed = edje_edit_object_add(ecore_evas_get(ee));
   if (!edje_object_file_set(ed, file, group))
     {
	Edje_Load_Error err = edje_object_load_error_get(ed);
	const char *errmsg = edje_load_error_str(err);
	ERR("could not load group '%s' from file '%s': %s",
	    group, file, errmsg);
	evas_object_del(ed);
	return EINA_FALSE;
     }

   ret = _parse_parts(ed) && _parse_programs(ed);

   evas_object_del(ed);
   return ret;
}

int
main(int argc, char *argv[])
{
   Eina_Bool quit_option = EINA_FALSE;
   char *source, *header;
   int arg_index, ret = 0;
   Ecore_Getopt_Value values[] = {
     ECORE_GETOPT_VALUE_STR(prefix),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_BOOL(quit_option),
     ECORE_GETOPT_VALUE_NONE
   };

   setlocale(LC_NUMERIC, "C");

   eina_init();
   ecore_init();
   ecore_evas_init();
   edje_init();

   if (argc < 2)
     {
        fprintf(stderr, "Missing action. See '--help or -h'.\n");
        ret = 1;
	goto error_log;
     }

   _log_dom = eina_log_domain_register("edje_codegen", EINA_COLOR_YELLOW);
   if (_log_dom < 0)
     {
        EINA_LOG_CRIT("could not register log domain 'edje_codegen'");
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
   else if (quit_option) goto error_getopt;
   else if (arg_index != argc - 4)
     {
        fprintf(stderr, "Incorrect number of parameters. Requires "	\
		"fours arguments, an edje, the group, "			\
		"the source output (foo.c) and the header(foo.h).\n"    \
		"See %s --help\n", argv[0]);
        ret = 1;
        goto error_getopt;
     }

   file = argv[arg_index++];

   // check if the file is accessible
   if (access(file, R_OK) == -1)
     {
        ERR("File '%s' not accessible, error %d (%s).\n",
            file, errno, strerror(errno));
        ret = 1;
        goto error_getopt;
     }

   group = argv[arg_index++];
   source = argv[arg_index++];
   header = argv[arg_index++];

   if (!edje_file_group_exists(file, group))
     {
	ERR("The group %s not exists", group);
	ret = 2;
	goto error_getopt;
     }

   ee = ecore_evas_buffer_new(1, 1);
   if (!ee)
     {
	ERR("could not create ecore_evas_buffer");
	ret = 3;
	goto error_getopt;
     }

   if (!_open_file_descriptors(source, header))
     {
	ERR("Could not create the source files, error %d (%s)",
	    errno, strerror(errno));
	ret = 4;
	goto error_getopt;
     }

   if (!_write_headers(header))
     {
	ERR("Could not write the header, error %d (%s)",
	    errno, strerror(errno));
	ret = 5;
	goto error_getopt;
     }

   if (!_write_object_get())
     {
	ERR("Could not write the object get, error %d (%s)",
	    errno, strerror(errno));
	ret = 6;
	goto error_getopt;
     }

   if (!_parse())
     {
	ERR("Could not parsing the EDJE");
	ret = 7;
	goto error_getopt;
     }

   if (!_write_footer(header))
     {
	ERR("Could not write the footer, error %d (%s)",
	    errno, strerror(errno));
	ret = 8;
	goto error_getopt;
     }

   if (!_close_file_descriptors())
     {
	ERR("Could not close the source files, error %d (%s)",
	    errno, strerror(errno));
	ret = 9;
     }

 error_getopt:
   if (ee)
     ecore_evas_free(ee);

   eina_log_domain_unregister(_log_dom);
 error_log:
   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   eina_shutdown();

   if (ret > 4)
     {
	unlink(header);
	unlink(source);
     }

   return ret;
}
