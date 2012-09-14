#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <locale.h>
#include <unistd.h>
#include <errno.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Getopt.h>
#include <Ecore_Evas.h>

#include "Edje.h"
#define EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT 1
#include "Edje_Edit.h"

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

#define H_CODEGEN_PART_TEXT_SET                              \
  "void %s_%s_set(Evas_Object *o, const char *value);\n"

#define C_CODEGEN_PART_SWALLOW_SET                             \
  "void\n"                                                     \
  "%s_%s_set(Evas_Object *o, Evas_Object *value)\n"            \
  "{\n"                                                        \
  "   edje_object_part_swallow(o, \"%s\", value);\n"           \
  "}\n\n"

#define H_CODEGEN_PART_SWALLOW_SET                             \
  "void %s_%s_set(Evas_Object *o, Evas_Object *value);\n"

#define C_CODEGEN_PART_TEXT_GET                         \
  "const char *\n"                                      \
  "%s_%s_get(const Evas_Object *o)\n"                   \
  "{\n"                                                 \
  "   return edje_object_part_text_get(o, \"%s\");\n"   \
  "}\n\n"

#define H_CODEGEN_PART_TEXT_GET                         \
  "const char *%s_%s_get(const Evas_Object *o);\n"

#define C_CODEGEN_PART_SWALLOW_GET                        \
  "Evas_Object *\n"                                       \
  "%s_%s_get(const Evas_Object *o)\n"                     \
  "{\n"                                                   \
  "   return edje_object_part_swallow_get(o, \"%s\");\n"  \
  "}\n\n"

#define H_CODEGEN_PART_SWALLOW_GET                         \
  "Evas_Object *%s_%s_get(const Evas_Object *o);\n"

#define C_CODEGEN_PART_BOX_APPEND                                \
  "Eina_Bool\n"                                                  \
  "%s_%s_append(Evas_Object *o, Evas_Object *child)\n"           \
  "{\n"                                                          \
  "   return edje_object_part_box_append(o, \"%s\", child);\n"   \
  "}\n\n"

#define H_CODEGEN_PART_BOX_APPEND                                      \
  "Eina_Bool %s_%s_append(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_BOX_PREPEND                                \
  "Eina_Bool\n"                                                   \
  "%s_%s_prepend(Evas_Object *o, Evas_Object *child)\n"           \
  "{\n"                                                           \
  "   return edje_object_part_box_prepend(o, \"%s\", child);\n"   \
  "}\n\n"

#define H_CODEGEN_PART_BOX_PREPEND                                      \
  "Eina_Bool %s_%s_prepend(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_BOX_INSERT_BEFORE                          \
  "Eina_Bool\n"                                                   \
  "%s_%s_insert_before(Evas_Object *o, Evas_Object *child, "      \
  "const Evas_Object *reference)\n"                               \
  "{\n"                                                           \
  "   return edje_object_part_box_insert_before(o, \"%s\", "      \
  "child, reference);\n"   \
  "}\n\n"

#define H_CODEGEN_PART_BOX_INSERT_BEFORE                                \
  "Eina_Bool %s_%s_insert_before(Evas_Object *o, Evas_Object *child, "  \
  "const Evas_Object *reference);\n"

#define C_CODEGEN_PART_BOX_INSERT_AT                                        \
  "Eina_Bool\n"                                                             \
  "%s_%s_insert_at(Evas_Object *o, Evas_Object *child, unsigned int pos)\n" \
  "{\n"                                                                     \
  "   return edje_object_part_box_insert_at(o, \"%s\", child, pos);\n"      \
  "}\n\n"

#define H_CODEGEN_PART_BOX_INSERT_AT                                     \
  "Eina_Bool %s_%s_insert_at(Evas_Object *o, Evas_Object *child, "       \
  "unsigned int pos);\n"

#define C_CODEGEN_PART_BOX_REMOVE                                   \
  "Evas_Object *\n"                                                 \
  "%s_%s_remove(Evas_Object *o, Evas_Object *child)\n"              \
  "{\n"                                                             \
  "   return edje_object_part_box_remove(o, \"%s\", child);\n"      \
  "}\n\n"

#define H_CODEGEN_PART_BOX_REMOVE                                    \
  "Evas_Object *%s_%s_remove(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_BOX_REMOVE_AT                                 \
  "Evas_Object *\n"                                                  \
  "%s_%s_remove_at(Evas_Object *o, unsigned int pos)\n"              \
  "{\n"                                                              \
  "   return edje_object_part_box_remove_at(o, \"%s\", pos);\n"      \
  "}\n\n"

#define H_CODEGEN_PART_BOX_REMOVE_AT                                 \
  "Evas_Object *%s_%s_remove_at(Evas_Object *o, unsigned int pos);\n"

#define C_CODEGEN_PART_BOX_REMOVE_ALL                                \
  "Eina_Bool\n"                                                      \
  "%s_%s_remove_all(Evas_Object *o, Eina_Bool clear)\n"              \
  "{\n"                                                              \
  "   return edje_object_part_box_remove_all(o, \"%s\", clear);\n"   \
  "}\n\n"

#define H_CODEGEN_PART_BOX_REMOVE_ALL                                     \
  "Eina_Bool %s_%s_remove_all(Evas_Object *o, Eina_Bool clear);\n"

#define C_CODEGEN_PART_TABLE_CHILD_GET                                    \
  "Evas_Object *\n"                                                       \
  "%s_%s_child_get(Evas_Object *o, unsigned int col, unsigned int row)\n" \
  "{\n"                                                                   \
  "   return edje_object_part_table_child_get(o, \"%s\", col, row);\n"    \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_CHILD_GET                                \
  "Evas_Object * %s_%s_child_get(Evas_Object *o, unsigned int col, "  \
  "unsigned int row);\n"

#define C_CODEGEN_PART_TABLE_PACK                                         \
  "Eina_Bool\n"                                                           \
  "%s_%s_pack(Evas_Object *o, Evas_Object *child, unsigned short col, "   \
  "unsigned short row, unsigned short colspan, unsigned short rowspan)\n" \
  "{\n"                                                                   \
  "   return edje_object_part_table_pack(o, \"%s\", child, col, row, "    \
  "colspan, rowspan);\n"                                                  \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_PACK                                \
  "Eina_Bool %s_%s_pack(Evas_Object *o, Evas_Object *child, "    \
  "unsigned short col, unsigned short row, unsigned short "      \
  "colspan, unsigned short rowspan);\n"

#define C_CODEGEN_PART_TABLE_UNPACK                                  \
  "Eina_Bool\n"                                                      \
  "%s_%s_unpack(Evas_Object *o, Evas_Object *child)\n"               \
  "{\n"                                                              \
  "   return edje_object_part_table_unpack(o, \"%s\", child);\n"     \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_UNPACK                                  \
  "Eina_Bool %s_%s_unpack(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_TABLE_CLEAR                               \
  "Eina_Bool\n"                                                  \
  "%s_%s_clear(Evas_Object *o, Eina_Bool clear)\n"               \
  "{\n"                                                          \
  "   return edje_object_part_table_clear(o, \"%s\", clear);\n"  \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_CLEAR                                   \
  "Eina_Bool %s_%s_clear(Evas_Object *o, Eina_Bool clear);\n"

#define C_CODEGEN_PART_TABLE_COL_ROW_SIZE_GET                        \
  "Eina_Bool\n"                                                      \
  "%s_%s_col_row_size_get(Evas_Object *o, int  *cols, int *rows)\n"  \
  "{\n"                                                              \
  "   return edje_object_part_table_col_row_size_get(o, \"%s\", "    \
  "cols, rows);\n"                                                   \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_COL_ROW_SIZE_GET                                \
  "Eina_Bool %s_%s_col_row_size_get(Evas_Object *o, int *cols, int *rows);\n"

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
	    Edje_Part_Type parttype, const char *description)
{
   char buf[512];
   const char *type, *template;

#define TEMPLATE_NAME(sufix)                                      \
   do {                                                           \
     snprintf(buf, sizeof(buf), C_CODEGEN_PART_##sufix, prefix,   \
              apiname, partname);                                 \
     if (fwrite(buf, strlen(buf), 1, source_fd) != 1)             \
       goto err;                                                  \
     snprintf(buf, sizeof(buf), H_CODEGEN_PART_##sufix, prefix,   \
              apiname);                                           \
     if (fwrite(buf, strlen(buf), 1, header_fd) != 1)             \
       goto err;                                                  \
   } while(0)

   if (description)
     {
	snprintf(buf, sizeof(buf), "\n/**\n * @brief %s\n */\n", description);
	if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
	  goto err;
     }

   switch (parttype)
     {
      case EDJE_PART_TYPE_BOX:
	 TEMPLATE_NAME(BOX_APPEND);
	 TEMPLATE_NAME(BOX_PREPEND);
	 TEMPLATE_NAME(BOX_INSERT_BEFORE);
	 TEMPLATE_NAME(BOX_INSERT_AT);
	 TEMPLATE_NAME(BOX_REMOVE);
	 TEMPLATE_NAME(BOX_REMOVE_AT);
	 TEMPLATE_NAME(BOX_REMOVE_ALL);
	 break;

      case EDJE_PART_TYPE_TABLE:
	TEMPLATE_NAME(TABLE_PACK);
	TEMPLATE_NAME(TABLE_UNPACK);
	TEMPLATE_NAME(TABLE_CHILD_GET);
	TEMPLATE_NAME(TABLE_CLEAR);
	TEMPLATE_NAME(TABLE_COL_ROW_SIZE_GET);
	break;

      case EDJE_PART_TYPE_TEXT:
	TEMPLATE_NAME(TEXT_SET);
	TEMPLATE_NAME(TEXT_GET);
	break;

      default:
	TEMPLATE_NAME(SWALLOW_SET);
	TEMPLATE_NAME(SWALLOW_GET);
	break;
     }

#undef TEMPLATE_NAME

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
	if (!((type == EDJE_PART_TYPE_TEXT)    ||
	      (type == EDJE_PART_TYPE_SWALLOW) ||
	      (type == EDJE_PART_TYPE_BOX)     ||
	      (type == EDJE_PART_TYPE_TABLE)))
	  {
	     ERR("Invalid part type %d", type);
	     free(apiname);
	     continue;
	  }

	description = edje_edit_part_api_description_get(ed, name);
	if (!_write_part(apiname, name, type, description))
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

 error_log:
   edje_shutdown();
   ecore_evas_shutdown();
   ecore_shutdown();
   eina_log_domain_unregister(_log_dom);
   eina_shutdown();

   if (ret > 4)
     {
	unlink(header);
	unlink(source);
     }

   return ret;
}
