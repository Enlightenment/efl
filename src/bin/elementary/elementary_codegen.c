#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include <Ecore.h>
#include <Ecore_File.h>
#include <Ecore_Evas.h>
#include <Ecore_Getopt.h>
#include <Edje.h>
#define EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT 1
#include <Edje_Edit.h>
#include <Eina.h>

#include <Elementary.h>

#include <ctype.h>
#include <fcntl.h>
#include <locale.h>
#include <stdio.h>
#include <unistd.h>

#ifndef ENABLE_NLS
# ifndef libintl_setlocale
#  define libintl_setlocale(c, l)
# endif
#endif

static int _log_dom;
#define DBG(...)  EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)
#define ERR(...)  EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)
#define WRN(...)  EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)

static Ecore_Evas *ee = NULL;
static char *file = NULL;
static char *group = NULL;
static char *prefix = NULL;
static FILE *source_fd = NULL;
static FILE *header_fd = NULL;

#define H_HEADER                       \
  "#ifndef _%s\n"                      \
  "#define _%s\n\n"                    \
  "#include <Edje.h>\n"                \
  "#include <Elementary.h>\n"          \
  "#include <Evas.h>\n\n"              \
  "#include <stdlib.h>\n\n"

#define H_FOOTER                       \
  "\n#endif /* _%s */\n"

#define C_HEADER                       \
  "#include \"%s\"\n\n"

#define H_CODEGEN_LAYOUT_ADD                                                \
  "/**\n * @brief Creates the layout object and set the theme\n"            \
  " * @param o The parent\n"                                                \
  " * @param th The theme to add to, or if NULL, the default theme\n"       \
  " * @param edje_file The path to edj, if NULL it's used the path given\n" \
  " *                  to elementary_codegen\n */\n"                        \
  "Evas_Object *%s_layout_add(Evas_Object *o, Elm_Theme *th, "              \
  "const char *edje_file);\n"

#define C_CODEGEN_LAYOUT_ADD                                               \
  "Evas_Object *\n"                                                        \
  "%s_layout_add(Evas_Object *o, Elm_Theme *th, const char *edje_file)\n"  \
  "{\n"                                                                    \
  "   Evas_Object *l;\n\n"                                                 \
  "   if (edje_file)\n"                                                    \
  "     elm_theme_extension_add(th, edje_file);\n"                         \
  "   else\n"                                                              \
  "     elm_theme_extension_add(th, \"./%s\");\n\n"                        \
  "   l = elm_layout_add(o);\n"                                            \
  "   if (!l) return NULL;\n\n"                                            \
  "   if (!elm_layout_theme_set(l, \"%s\", \"%s\", \"%s\"))\n"             \
  "     {\n"                                                               \
  "        evas_object_del(l);\n"                                          \
  "        return NULL;\n"                                                 \
  "     }\n\n"                                                             \
  "   return l;\n"                                                         \
  "}\n\n"

#define C_CODEGEN_PART_CONTENT_SET                             \
  "void\n"                                                     \
  "%s_%s_set(Evas_Object *o, Evas_Object *value)\n"            \
  "{\n"                                                        \
  "   elm_layout_content_set(o, \"%s\", value);\n"             \
  "}\n\n"

#define C_CODEGEN_PART_TEXT_SET                                \
  "void\n"                                                     \
  "%s_%s_set(Evas_Object *o, const char *value)\n"             \
  "{\n"                                                        \
  "   elm_layout_text_set(o, \"%s\", value);\n"                \
  "}\n\n"

#define C_CODEGEN_PART_CONTENT_UNSET                          \
  "Evas_Object *\n"                                           \
  "%s_%s_unset(Evas_Object *o)\n"                             \
  "{\n"                                                       \
  "   return elm_layout_content_unset(o, \"%s\");\n"          \
  "}\n\n"

#define H_CODEGEN_PART_CONTENT_SET                            \
  "void %s_%s_set(Evas_Object *o, Evas_Object *value);\n"

#define H_CODEGEN_PART_TEXT_SET                               \
  "void %s_%s_set(Evas_Object *o, const char *value);\n"

#define H_CODEGEN_PART_CONTENT_UNSET                    \
  "Evas_Object *%s_%s_unset(Evas_Object *o);\n"

#define C_CODEGEN_PART_CONTENT_GET                      \
  "Evas_Object *\n"                                     \
  "%s_%s_get(const Evas_Object *o)\n"                   \
  "{\n"                                                 \
  "   return elm_layout_content_get(o, \"%s\");\n"      \
  "}\n\n"

#define H_CODEGEN_PART_CONTENT_GET                      \
  "Evas_Object *%s_%s_get(const Evas_Object *o);\n"

#define C_CODEGEN_PART_TEXT_GET                        \
  "const char *\n"                                     \
  "%s_%s_get(const Evas_Object *o)\n"                  \
  "{\n"                                                \
  "   return elm_layout_text_get(o, \"%s\");\n"        \
  "}\n\n"

#define H_CODEGEN_PART_TEXT_GET                        \
  "const char *%s_%s_get(const Evas_Object *o);\n"

#define C_CODEGEN_PART_BOX_APPEND                                \
  "Eina_Bool\n"                                                  \
  "%s_%s_append(Evas_Object *o, Evas_Object *child)\n"           \
  "{\n"                                                          \
  "   return elm_layout_box_append(o, \"%s\", child);\n"         \
  "}\n\n"

#define H_CODEGEN_PART_BOX_APPEND                                  \
  "Eina_Bool %s_%s_append(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_BOX_PREPEND                                \
  "Eina_Bool\n"                                                   \
  "%s_%s_prepend(Evas_Object *o, Evas_Object *child)\n"           \
  "{\n"                                                           \
  "   return elm_layout_box_prepend(o, \"%s\", child);\n"         \
  "}\n\n"

#define H_CODEGEN_PART_BOX_PREPEND                                 \
  "Eina_Bool %s_%s_prepend(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_BOX_INSERT_BEFORE                          \
  "Eina_Bool\n"                                                   \
  "%s_%s_insert_before(Evas_Object *o, Evas_Object *child, "      \
  "const Evas_Object *reference)\n"                               \
  "{\n"                                                           \
  "   return elm_layout_box_insert_before(o, \"%s\", "            \
  "child, reference);\n"                                          \
  "}\n\n"

#define H_CODEGEN_PART_BOX_INSERT_BEFORE                                \
  "Eina_Bool %s_%s_insert_before(Evas_Object *o, Evas_Object *child, "  \
  "const Evas_Object *reference);\n"

#define C_CODEGEN_PART_BOX_INSERT_AT                                        \
  "Eina_Bool\n"                                                             \
  "%s_%s_insert_at(Evas_Object *o, Evas_Object *child, unsigned int pos)\n" \
  "{\n"                                                                     \
  "   return elm_layout_box_insert_at(o, \"%s\", child, pos);\n"            \
  "}\n\n"

#define H_CODEGEN_PART_BOX_INSERT_AT                                     \
  "Eina_Bool %s_%s_insert_at(Evas_Object *o, Evas_Object *child, "       \
  "unsigned int pos);\n"

#define C_CODEGEN_PART_BOX_REMOVE                             \
  "Evas_Object *\n"                                           \
  "%s_%s_remove(Evas_Object *o, Evas_Object *child)\n"        \
  "{\n"                                                       \
  "   return elm_layout_box_remove(o, \"%s\", child);\n"      \
  "}\n\n"

#define H_CODEGEN_PART_BOX_REMOVE                                    \
  "Evas_Object *%s_%s_remove(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_BOX_REMOVE_ALL                           \
  "Eina_Bool\n"                                                 \
  "%s_%s_remove_all(Evas_Object *o, Eina_Bool clear)\n"         \
  "{\n"                                                         \
  "   return elm_layout_box_remove_all(o, \"%s\", clear);\n"    \
  "}\n\n"

#define H_CODEGEN_PART_BOX_REMOVE_ALL                               \
  "Eina_Bool %s_%s_remove_all(Evas_Object *o, Eina_Bool clear);\n"

#define C_CODEGEN_PART_TABLE_PACK                                         \
  "Eina_Bool\n"                                                           \
  "%s_%s_pack(Evas_Object *o, Evas_Object *child, unsigned short col, "   \
  "unsigned short row, unsigned short colspan, unsigned short rowspan)\n" \
  "{\n"                                                                   \
  "   return elm_layout_table_pack(o, \"%s\", child, col, row, "          \
  "colspan, rowspan);\n"                                                  \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_PACK                                \
  "Eina_Bool %s_%s_pack(Evas_Object *o, Evas_Object *child, "    \
  "unsigned short col, unsigned short row, unsigned short "      \
  "colspan, unsigned short rowspan);\n"

#define C_CODEGEN_PART_TABLE_UNPACK                                  \
  "Evas_Object *\n"                                                  \
  "%s_%s_unpack(Evas_Object *o, Evas_Object *child)\n"               \
  "{\n"                                                              \
  "   return elm_layout_table_unpack(o, \"%s\", child);\n"           \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_UNPACK                                  \
  "Evas_Object *%s_%s_unpack(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_TABLE_CLEAR                               \
  "Eina_Bool\n"                                                  \
  "%s_%s_clear(Evas_Object *o, Eina_Bool clear)\n"               \
  "{\n"                                                          \
  "   return elm_layout_table_clear(o, \"%s\", clear);\n"        \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_CLEAR                                   \
  "Eina_Bool %s_%s_clear(Evas_Object *o, Eina_Bool clear);\n"

#define C_CODEGEN_PROGRAM_EMIT                          \
  "void\n"                                              \
  "%s_%s_emit(Evas_Object *o)\n"                        \
  "{\n"                                                 \
  "   elm_layout_signal_emit(o, \"%s\", \"%s\");\n"     \
  "}\n\n"

#define H_CODEGEN_PROGRAM_EMIT                          \
  "void %s_%s_emit(Evas_Object *o);\n"

#define C_CODEGEN_PROGRAM_CALLBACK_ADD                                      \
  "void\n"                                                                  \
  "%s_%s_callback_add(Evas_Object *o, Edje_Signal_Cb func, void *data)\n"   \
  "{\n"                                                                     \
  "   elm_layout_signal_callback_add(o, \"%s\", \"%s\", func, data);\n"     \
  "}\n\n"

#define H_CODEGEN_PROGRAM_CALLBACK_ADD                                  \
  "void %s_%s_callback_add(Evas_Object *o, Edje_Signal_Cb func, "       \
  "void *data);\n"

#define C_CODEGEN_PROGRAM_CALLBACK_DEL                              \
  "void\n"                                                          \
  "%s_%s_callback_del(Evas_Object *o, Edje_Signal_Cb func)\n"       \
  "{\n"                                                             \
  "   elm_layout_signal_callback_del(o, \"%s\", \"%s\", func);\n"   \
  "}\n\n"

#define H_CODEGEN_PROGRAM_CALLBACK_DEL                              \
  "void %s_%s_callback_del(Evas_Object *o, Edje_Signal_Cb func);\n"

const Ecore_Getopt optdesc =
{
   "elm_codegen",
   "%prog [options] <file.edj> <group> <source_file_name> <header_file_name>",
   PACKAGE_VERSION,
   "(C) 2012 - The Enlightenment Project",
   "BSD",
   "elm_codegen generates the boilerplate code to get and set the "
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
_header_standardize(const char *filename)
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
_file_descriptors_open(const char *source, const char *header)
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
_file_descriptors_close(void)
{
   Eina_Bool ret = EINA_FALSE;

   if (!fclose(header_fd))
     ret = EINA_TRUE;

   if (!fclose(source_fd))
     ret &= EINA_TRUE;

   return ret;
}

static Eina_Bool
_headers_write(const char *filename)
{
   char buf[512];
   char *str;

   str = _header_standardize(filename);
   snprintf(buf, sizeof(buf), H_HEADER, str, str);
   if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
     {
        free(str);
        return EINA_FALSE;
     }

   free(str);

   snprintf(buf, sizeof(buf), C_HEADER, ecore_file_file_get(filename));
   if (fwrite(buf, strlen(buf), 1, source_fd) != 1)
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_footer_write(const char *filename)
{
   char buf[512];
   char *str;

   str = _header_standardize(filename);
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
_theme_set_write(void)
{
   Eina_Bool ret = EINA_FALSE;
   char *str[3];  /* *klas, *style, *group */
   char *token, *_group, buf[512];
   int i;

   str[0] = str[1] = str[2] = NULL;
   if (strncmp(group, "elm/", 4)) return EINA_FALSE;

   _group = strdup(group);
   if (!_group) return EINA_FALSE;

   strtok(_group, "/");
   for (i = 0; i < 3; i++)
     {
        token = strtok(NULL, "/");
        if (!token) break;

        str[i] = token;
     }

   if (!str[0] || !str[1] || !str[2])
     goto end;

   snprintf(buf, sizeof(buf), C_CODEGEN_LAYOUT_ADD, prefix, file, str[0],
            str[1], str[2]);
   if (fwrite(buf, strlen(buf), 1, source_fd) != 1)
     goto end;

   snprintf(buf, sizeof(buf), H_CODEGEN_LAYOUT_ADD, prefix);
   if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
     goto end;

   ret = EINA_TRUE;

end:
   free(_group);
   return ret;
}

static Eina_Bool
_part_write(const char *apiname, const char *partname, const char *description,
            Edje_Part_Type type)
{
   char buf[1024];

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

   switch (type)
     {
      case EDJE_PART_TYPE_BOX:
         TEMPLATE_NAME(BOX_APPEND);
         TEMPLATE_NAME(BOX_PREPEND);
         TEMPLATE_NAME(BOX_INSERT_BEFORE);
         TEMPLATE_NAME(BOX_INSERT_AT);
         TEMPLATE_NAME(BOX_REMOVE);
         TEMPLATE_NAME(BOX_REMOVE_ALL);
         break;

      case EDJE_PART_TYPE_TABLE:
         TEMPLATE_NAME(TABLE_PACK);
         TEMPLATE_NAME(TABLE_UNPACK);
         TEMPLATE_NAME(TABLE_CLEAR);
         break;

      case EDJE_PART_TYPE_TEXT:
         TEMPLATE_NAME(TEXT_SET);
         TEMPLATE_NAME(TEXT_GET);
         break;

      default:
         TEMPLATE_NAME(CONTENT_SET);
         TEMPLATE_NAME(CONTENT_UNSET);
         TEMPLATE_NAME(CONTENT_GET);
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
_parts_parse(Evas_Object *ed)
{
   Eina_List *parts, *l;
   const char *name, *description;
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
        if ((type != EDJE_PART_TYPE_SWALLOW) &&
            (type != EDJE_PART_TYPE_TEXT)    &&
            (type != EDJE_PART_TYPE_BOX)     &&
            (type != EDJE_PART_TYPE_TABLE))
          {
             free(apiname);
             continue;
          }

        description = edje_edit_part_api_description_get(ed, name);
        if (!_part_write(apiname, name, description, type))
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
_program_emit_write(const char *apiname, const char *source, const char *sig,
                    const char *description)
{
   char buf[512];

   snprintf(buf, sizeof(buf), C_CODEGEN_PROGRAM_EMIT, prefix,
            apiname, sig, source);
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
_program_add_write(const char *apiname, const char *source, const char *sig,
                   const char *description)
{
   char buf[512];

   snprintf(buf, sizeof(buf), C_CODEGEN_PROGRAM_CALLBACK_ADD, prefix,
            apiname, sig, source);
   if (fwrite(buf, strlen(buf), 1, source_fd) != 1)
     goto err;

   snprintf(buf, sizeof(buf), C_CODEGEN_PROGRAM_CALLBACK_DEL, prefix,
            apiname, sig, source);
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
_programs_parse(Evas_Object *ed)
{
   Eina_Bool ret = EINA_TRUE;
   Eina_List *programs, *l;
   const char *name, *source = NULL, *sig = NULL, *description;
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

        description = edje_edit_program_api_description_get(ed, name);
        type = edje_edit_program_action_get(ed, name);
        if (type == EDJE_ACTION_TYPE_SIGNAL_EMIT)
          {
             const char *str, *str2;
             str = edje_edit_program_state_get(ed, name);
             str2 = edje_edit_program_state2_get(ed, name);

             if (!_program_add_write(apiname, str2, str, description))
               {
                  ret = EINA_FALSE;
                  edje_edit_string_free(str);
                  edje_edit_string_free(str2);
                  break;
               }

             edje_edit_string_free(str);
             edje_edit_string_free(str2);
          }

        sig = edje_edit_program_signal_get(ed, name);
        if (!sig) sig = eina_stringshare_add("");

        source = edje_edit_program_source_get(ed, name);
        if (!source) source = eina_stringshare_add("");

        if (strlen (sig))
          {
             if (!_program_emit_write(apiname, source, sig, description))
               {
                  ret = EINA_FALSE;
                  break;
               }
          }

        edje_edit_string_free(description);
        edje_edit_string_free(sig);
        edje_edit_string_free(source);
        free(apiname);
     }

   edje_edit_string_list_free(programs);
   if (!ret)
     {
        edje_edit_string_free(description);
        edje_edit_string_free(sig);
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

   ret = _parts_parse(ed) && _programs_parse(ed);

   evas_object_del(ed);
   return ret;
}

int
main(int argc, char *argv[])
{
   Eina_Bool quit_option = EINA_FALSE;
   char *source = NULL, *header = NULL;
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

   _log_dom = eina_log_domain_register("elementary_codegen", EINA_COLOR_YELLOW);
   if (_log_dom < 0)
     {
        EINA_LOG_CRIT("could not register log domain 'elementary_codegen'");
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
        fprintf(stderr, "Incorrect number of parameters. Requires "  \
                "fours arguments, an edje, the group, "              \
                "the source output (foo.c) and the header(foo.h).\n" \
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

   if (!_file_descriptors_open(source, header))
     {
        ERR("Could not create the source files, error %d (%s)",
            errno, strerror(errno));
        ret = 4;
        goto error_getopt;
     }

   if (!_headers_write(header))
     {
        ERR("Could not write the header, error %d (%s)",
            errno, strerror(errno));
        ret = 5;
        goto error_getopt;
     }

   if (!_theme_set_write())
     WRN("Theme set getter/setter not created. Group name: %s invalid.", group);

   if (!_parse())
     {
        ERR("Could not parsing the EDJE");
        ret = 6;
        goto error_getopt;
     }

   if (!_footer_write(header))
     {
        ERR("Could not write the footer, error %d (%s)",
            errno, strerror(errno));
        ret = 7;
        goto error_getopt;
     }

   if (!_file_descriptors_close())
     {
        ERR("Could not close the source files, error %d (%s)",
            errno, strerror(errno));
        ret = 8;
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
