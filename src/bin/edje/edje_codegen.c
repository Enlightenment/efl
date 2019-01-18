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
#include <Ecore_File.h>

#include "Edje.h"
#define EDJE_EDIT_IS_UNSTABLE_AND_I_KNOW_ABOUT_IT 1
#include "Edje_Edit.h"

static int _log_dom;
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)

static Ecore_Evas *ee = NULL;
static char *file = NULL;
static char *group = NULL;
static char *prefix = NULL;
static FILE *source_fd = NULL;
static FILE *header_fd = NULL;
static Eina_List *externals = NULL;

#define H_HEADER          \
  "#ifndef _%s\n"         \
  "#define _%s\n\n"       \
  "#include <Edje.h>\n"   \
  "#include <Evas.h>\n\n" \
  "#include <stdlib.h>\n\n"

#define H_FOOTER \
  "\n#endif /* _%s */\n"

#define C_HEADER \
  "#include \"%s\"\n\n"

#define C_CODEGEN_OBJECT_ADD                           \
  "Evas_Object *\n"                                    \
  "%s_object_add(Evas *e, const char *file)\n"         \
  "{\n"                                                \
  "   Evas_Object *o;\n\n"                             \
  "   o = edje_object_add(e);\n"                       \
  "   if (!o) return NULL;\n\n"                        \
  "   if (file)\n"                                     \
  "      edje_object_file_set(o, file, \"%s\");\n"     \
  "   else\n"                                          \
  "      edje_object_file_set(o, \"%s\", \"%s\");\n\n" \
  "   return o;\n"                                     \
  "}\n\n"

#define H_CODEGEN_OBJECT_ADD                                           \
  "/**\n * @brief Creates the Edje object and set the edj file\n"      \
  " * @param e The surface\n"                                          \
  " * @param file The path to edj, if NULL it's used the path given\n" \
  " *             to edje_codegen\n */\n"                              \
  "Evas_Object *%s_object_add(Evas *e, const char *file);\n\n"

#define C_CODEGEN_DRAG_SET(option)                                                              \
  "Eina_Bool\n"                                                                                 \
  "%s_%s_drag_" #option "_set(Evas_Object *o, double dx, double dy)\n"                          \
                        "{\n"                                                                   \
                        "   return edje_object_part_drag_" #option "_set(o, \"%s\", dx, dy);\n" \
                                                                   "}\n\n"

#define H_CODEGEN_DRAG_SET(option) \
  "Eina_Bool %s_%s_drag_" #option "_set(Evas_Object *o, double dx, double dy);\n"

#define C_CODEGEN_DRAG_GET(option)                                                              \
  "Eina_Bool\n"                                                                                 \
  "%s_%s_drag_" #option "_get(Evas_Object *o, double *dx, double *dy)\n"                        \
                        "{\n"                                                                   \
                        "   return edje_object_part_drag_" #option "_get(o, \"%s\", dx, dy);\n" \
                                                                   "}\n\n"

#define H_CODEGEN_DRAG_GET(option)                                  \
  "Eina_Bool %s_%s_drag_" #option "_get(Evas_Object *o, double *dx" \
                                  ", double *dy);\n"

#define C_CODEGEN_DRAG_ACTION(option)                                                       \
  "Eina_Bool\n"                                                                             \
  "%s_%s_drag_" #option "(Evas_Object *o, double dx, double dy)\n"                          \
                        "{\n"                                                               \
                        "   return edje_object_part_drag_" #option "(o, \"%s\", dx, dy);\n" \
                                                                   "}\n\n"

#define H_CODEGEN_DRAG_ACTION(option) \
  "Eina_Bool %s_%s_drag_" #option "(Evas_Object *o, double dx, double dy);\n"

#define C_CODEGEN_DRAG_DIR_GET                            \
  "Edje_Drag_Dir\n"                                       \
  "%s_%s_drag_dir_get(Evas_Object *o)\n"                  \
  "{\n"                                                   \
  "   return edje_object_part_drag_dir_get(o, \"%s\");\n" \
  "}\n\n"

#define H_CODEGEN_DRAG_DIR_GET \
  "Edje_Drag_Dir %s_%s_drag_dir_get(Evas_Object *o);\n"

#define C_CODEGEN_PART_TEXT_SET                       \
  "void\n"                                            \
  "%s_%s_set(Evas_Object *o, const char *value)\n"    \
  "{\n"                                               \
  "   edje_object_part_text_set(o, \"%s\", value);\n" \
  "}\n\n"

#define H_CODEGEN_PART_TEXT_SET \
  "void %s_%s_set(Evas_Object *o, const char *value);\n"

#define C_CODEGEN_PART_SWALLOW_SET                   \
  "void\n"                                           \
  "%s_%s_set(Evas_Object *o, Evas_Object *value)\n"  \
  "{\n"                                              \
  "   edje_object_part_swallow(o, \"%s\", value);\n" \
  "}\n\n"

#define H_CODEGEN_PART_SWALLOW_SET \
  "void %s_%s_set(Evas_Object *o, Evas_Object *value);\n"

#define C_CODEGEN_PART_TEXT_GET                       \
  "const char *\n"                                    \
  "%s_%s_get(const Evas_Object *o)\n"                 \
  "{\n"                                               \
  "   return edje_object_part_text_get(o, \"%s\");\n" \
  "}\n\n"

#define H_CODEGEN_PART_TEXT_GET \
  "const char *%s_%s_get(const Evas_Object *o);\n"

#define C_CODEGEN_PART_SWALLOW_GET                       \
  "Evas_Object *\n"                                      \
  "%s_%s_get(const Evas_Object *o)\n"                    \
  "{\n"                                                  \
  "   return edje_object_part_swallow_get(o, \"%s\");\n" \
  "}\n\n"

#define H_CODEGEN_PART_SWALLOW_GET \
  "Evas_Object *%s_%s_get(const Evas_Object *o);\n"

#define C_CODEGEN_PART_BOX_APPEND                              \
  "Eina_Bool\n"                                                \
  "%s_%s_append(Evas_Object *o, Evas_Object *child)\n"         \
  "{\n"                                                        \
  "   return edje_object_part_box_append(o, \"%s\", child);\n" \
  "}\n\n"

#define H_CODEGEN_PART_BOX_APPEND \
  "Eina_Bool %s_%s_append(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_BOX_PREPEND                              \
  "Eina_Bool\n"                                                 \
  "%s_%s_prepend(Evas_Object *o, Evas_Object *child)\n"         \
  "{\n"                                                         \
  "   return edje_object_part_box_prepend(o, \"%s\", child);\n" \
  "}\n\n"

#define H_CODEGEN_PART_BOX_PREPEND \
  "Eina_Bool %s_%s_prepend(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_BOX_INSERT_BEFORE                     \
  "Eina_Bool\n"                                              \
  "%s_%s_insert_before(Evas_Object *o, Evas_Object *child, " \
  "const Evas_Object *reference)\n"                          \
  "{\n"                                                      \
  "   return edje_object_part_box_insert_before(o, \"%s\", " \
  "child, reference);\n"                                     \
  "}\n\n"

#define H_CODEGEN_PART_BOX_INSERT_BEFORE                               \
  "Eina_Bool %s_%s_insert_before(Evas_Object *o, Evas_Object *child, " \
  "const Evas_Object *reference);\n"

#define C_CODEGEN_PART_BOX_INSERT_AT                                        \
  "Eina_Bool\n"                                                             \
  "%s_%s_insert_at(Evas_Object *o, Evas_Object *child, unsigned int pos)\n" \
  "{\n"                                                                     \
  "   return edje_object_part_box_insert_at(o, \"%s\", child, pos);\n"      \
  "}\n\n"

#define H_CODEGEN_PART_BOX_INSERT_AT                               \
  "Eina_Bool %s_%s_insert_at(Evas_Object *o, Evas_Object *child, " \
  "unsigned int pos);\n"

#define C_CODEGEN_PART_BOX_REMOVE                              \
  "Evas_Object *\n"                                            \
  "%s_%s_remove(Evas_Object *o, Evas_Object *child)\n"         \
  "{\n"                                                        \
  "   return edje_object_part_box_remove(o, \"%s\", child);\n" \
  "}\n\n"

#define H_CODEGEN_PART_BOX_REMOVE \
  "Evas_Object *%s_%s_remove(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_BOX_REMOVE_AT                            \
  "Evas_Object *\n"                                             \
  "%s_%s_remove_at(Evas_Object *o, unsigned int pos)\n"         \
  "{\n"                                                         \
  "   return edje_object_part_box_remove_at(o, \"%s\", pos);\n" \
  "}\n\n"

#define H_CODEGEN_PART_BOX_REMOVE_AT \
  "Evas_Object *%s_%s_remove_at(Evas_Object *o, unsigned int pos);\n"

#define C_CODEGEN_PART_BOX_REMOVE_ALL                              \
  "Eina_Bool\n"                                                    \
  "%s_%s_remove_all(Evas_Object *o, Eina_Bool clear)\n"            \
  "{\n"                                                            \
  "   return edje_object_part_box_remove_all(o, \"%s\", clear);\n" \
  "}\n\n"

#define H_CODEGEN_PART_BOX_REMOVE_ALL \
  "Eina_Bool %s_%s_remove_all(Evas_Object *o, Eina_Bool clear);\n"

#define C_CODEGEN_PART_TABLE_CHILD_GET                                    \
  "Evas_Object *\n"                                                       \
  "%s_%s_child_get(Evas_Object *o, unsigned int col, unsigned int row)\n" \
  "{\n"                                                                   \
  "   return edje_object_part_table_child_get(o, \"%s\", col, row);\n"    \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_CHILD_GET                               \
  "Evas_Object * %s_%s_child_get(Evas_Object *o, unsigned int col, " \
  "unsigned int row);\n"

#define C_CODEGEN_PART_TABLE_PACK                                         \
  "Eina_Bool\n"                                                           \
  "%s_%s_pack(Evas_Object *o, Evas_Object *child, unsigned short col, "   \
  "unsigned short row, unsigned short colspan, unsigned short rowspan)\n" \
  "{\n"                                                                   \
  "   return edje_object_part_table_pack(o, \"%s\", child, col, row, "    \
  "colspan, rowspan);\n"                                                  \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_PACK                             \
  "Eina_Bool %s_%s_pack(Evas_Object *o, Evas_Object *child, " \
  "unsigned short col, unsigned short row, unsigned short "   \
  "colspan, unsigned short rowspan);\n"

#define C_CODEGEN_PART_TABLE_UNPACK                              \
  "Eina_Bool\n"                                                  \
  "%s_%s_unpack(Evas_Object *o, Evas_Object *child)\n"           \
  "{\n"                                                          \
  "   return edje_object_part_table_unpack(o, \"%s\", child);\n" \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_UNPACK \
  "Eina_Bool %s_%s_unpack(Evas_Object *o, Evas_Object *child);\n"

#define C_CODEGEN_PART_TABLE_CLEAR                              \
  "Eina_Bool\n"                                                 \
  "%s_%s_clear(Evas_Object *o, Eina_Bool clear)\n"              \
  "{\n"                                                         \
  "   return edje_object_part_table_clear(o, \"%s\", clear);\n" \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_CLEAR \
  "Eina_Bool %s_%s_clear(Evas_Object *o, Eina_Bool clear);\n"

#define C_CODEGEN_PART_TABLE_COL_ROW_SIZE_GET                       \
  "Eina_Bool\n"                                                     \
  "%s_%s_col_row_size_get(Evas_Object *o, int  *cols, int *rows)\n" \
  "{\n"                                                             \
  "   return edje_object_part_table_col_row_size_get(o, \"%s\", "   \
  "cols, rows);\n"                                                  \
  "}\n\n"

#define H_CODEGEN_PART_TABLE_COL_ROW_SIZE_GET \
  "Eina_Bool %s_%s_col_row_size_get(Evas_Object *o, int *cols, int *rows);\n"

#define C_CODEGEN_PART_EXTERNAL_PARAM_SET(type, param_type, field)                                                                                        \
  "Eina_Bool\n"                                                                                                                                           \
  "%s_%s_%s_set(Evas_Object *o, "type "value)\n"                                                                                                          \
                                      "{\n"                                                                                                               \
                                      "   Edje_External_Param param;\n\n"                                                                                 \
                                      "   param.name = \"%s\";\n"                                                                                         \
                                      "   param.type = "param_type ";\n"                                                                                  \
                                                                   "   param."field " = value;\n"                                                         \
                                                                                    "   return edje_object_part_external_param_set(o, \"%s\", &param);\n" \
                                                                                    "}\n\n"

#define H_CODEGEN_PART_EXTERNAL_PARAM_SET(type) \
  "Eina_Bool %s_%s_%s_set(Evas_Object *o, "type "value);\n"

#define C_CODEGEN_PART_EXTERNAL_PARAM_GET(type, param_type, field)                                                                     \
  "Eina_Bool\n"                                                                                                                        \
  "%s_%s_%s_get(Evas_Object *o, "type "*value)\n"                                                                                      \
                                      "{\n"                                                                                            \
                                      "   if (!value) return EINA_FALSE;\n\n"                                                          \
                                      "   Edje_External_Param param;\n\n"                                                              \
                                      "   param.name = \"%s\";\n"                                                                      \
                                      "   param.type = "param_type ";\n"                                                               \
                                                                   "   if (!edje_object_part_external_param_get(o, \"%s\", &param))\n" \
                                                                   "     return EINA_FALSE;\n\n"                                       \
                                                                   "   *value = param."field ";\n"                                     \
                                                                                             "   return EINA_TRUE;\n"                  \
                                                                                             "}\n\n"

#define H_CODEGEN_PART_EXTERNAL_PARAM_GET(type) \
  "Eina_Bool %s_%s_%s_get(Evas_Object *o, "type "*value);\n"

#define C_CODEGEN_PROGRAM_EMIT                       \
  "void\n"                                           \
  "%s_%s_emit(Evas_Object *o)\n"                     \
  "{\n"                                              \
  "   edje_object_signal_emit(o, \"%s\", \"%s\");\n" \
  "}\n\n"

#define H_CODEGEN_PROGRAM_EMIT \
  "void %s_%s_emit(Evas_Object *o);\n"

#define C_CODEGEN_PROGRAM_CALLBACK_ADD                                    \
  "void\n"                                                                \
  "%s_%s_callback_add(Evas_Object *o, Edje_Signal_Cb func, void *data)\n" \
  "{\n"                                                                   \
  "   edje_object_signal_callback_add(o, \"%s\", \"%s\", func, data);\n"  \
  "}\n\n"

#define H_CODEGEN_PROGRAM_CALLBACK_ADD                            \
  "void %s_%s_callback_add(Evas_Object *o, Edje_Signal_Cb func, " \
  "void *data);\n"

#define C_CODEGEN_PROGRAM_CALLBACK_DEL                                         \
  "void\n"                                                                     \
  "%s_%s_callback_del_full(Evas_Object *o, Edje_Signal_Cb func, void *data)\n" \
  "{\n"                                                                        \
  "   edje_object_signal_callback_del_full(o, \"%s\", \"%s\", func, data);\n"  \
  "}\n\n"

#define H_CODEGEN_PROGRAM_CALLBACK_DEL                                 \
  "void %s_%s_callback_del_full(Evas_Object *o, Edje_Signal_Cb func, " \
  "void *data);\n"

typedef struct _Part_External_Info Part_External_Info;
struct _Part_External_Info
{
   const char *description, *name, *source;
   char       *apiname;
   Eina_Bool   draggable;
};

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
   header_fd = fopen(header, "wb");
   if (!header_fd)
     return EINA_FALSE;

   source_fd = fopen(source, "wb");
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

   snprintf(buf, sizeof(buf), C_HEADER, ecore_file_file_get(filename));
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
_write_part_draggable(const char *apiname, const char *partname)
{
   char buf[1024];

#define TEMPLATE_DRAGGABLE(sufix, option)                    \
  do {                                                       \
       const char *template;                                 \
       template = C_CODEGEN_DRAG_##sufix(option);            \
       snprintf(buf, sizeof(buf), template, prefix, apiname, \
                partname);                                   \
       if (fwrite(buf, strlen(buf), 1, source_fd) != 1)      \
         return EINA_FALSE;                                  \
       template = H_CODEGEN_DRAG_##sufix(option);            \
       snprintf(buf, sizeof(buf), template, prefix,          \
                apiname);                                    \
       if (fwrite(buf, strlen(buf), 1, header_fd) != 1)      \
         return EINA_FALSE;                                  \
    } while (0)

   TEMPLATE_DRAGGABLE(SET, value);
   TEMPLATE_DRAGGABLE(GET, value);
   TEMPLATE_DRAGGABLE(SET, size);
   TEMPLATE_DRAGGABLE(GET, size);
   TEMPLATE_DRAGGABLE(SET, page);
   TEMPLATE_DRAGGABLE(GET, page);
   TEMPLATE_DRAGGABLE(SET, step);
   TEMPLATE_DRAGGABLE(GET, step);
   TEMPLATE_DRAGGABLE(ACTION, page);
   TEMPLATE_DRAGGABLE(ACTION, step);

#undef TEMPLATE_DRAGGABLE

   snprintf(buf, sizeof(buf), C_CODEGEN_DRAG_DIR_GET, prefix,
            apiname, partname);
   if (fwrite(buf, strlen(buf), 1, source_fd) != 1)
     return EINA_FALSE;
   snprintf(buf, sizeof(buf), H_CODEGEN_DRAG_DIR_GET, prefix, apiname);
   if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_write_part_external_param(const Part_External_Info *info,
                           const Edje_External_Param_Info *param)
{
   const char *template;
   char buf[1024];

#define WRITE_TEMPLATE(type, param_type, field)                               \
  do {                                                                        \
       template = C_CODEGEN_PART_EXTERNAL_PARAM_SET(type, param_type, field); \
       snprintf(buf, sizeof(buf), template, info->apiname, info->name,        \
                param->name, param->name, info->name);                        \
       if (fwrite(buf, strlen(buf), 1, source_fd) != 1)                       \
         return EINA_FALSE;                                                   \
       template = H_CODEGEN_PART_EXTERNAL_PARAM_SET(type);                    \
       snprintf(buf, sizeof(buf), template, info->apiname, info->name,        \
                param->name);                                                 \
       if (fwrite(buf, strlen(buf), 1, header_fd) != 1)                       \
         return EINA_FALSE;                                                   \
       template =                                                             \
         C_CODEGEN_PART_EXTERNAL_PARAM_GET(type, param_type, field);          \
       snprintf(buf, sizeof(buf), template, info->apiname, info->name,        \
                param->name, param->name, info->name);                        \
       if (fwrite(buf, strlen(buf), 1, source_fd) != 1)                       \
         return EINA_FALSE;                                                   \
       template = H_CODEGEN_PART_EXTERNAL_PARAM_GET(type);                    \
       snprintf(buf, sizeof(buf), template, info->apiname, info->name,        \
                param->name);                                                 \
       if (fwrite(buf, strlen(buf), 1, header_fd) != 1)                       \
         return EINA_FALSE;                                                   \
    } while (0)

   switch (param->type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_INT:
        WRITE_TEMPLATE("int ", "EDJE_EXTERNAL_PARAM_TYPE_INT", "i");
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
        WRITE_TEMPLATE("double ", "EDJE_EXTERNAL_PARAM_TYPE_DOUBLE", "d");
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
        WRITE_TEMPLATE("const char *", "EDJE_EXTERNAL_PARAM_TYPE_STRING", "s");
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
        WRITE_TEMPLATE("Eina_Bool ", "EDJE_EXTERNAL_PARAM_TYPE_BOOL", "i");
        break;

      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
        WRITE_TEMPLATE("const char *", "EDJE_EXTERNAL_PARAM_TYPE_CHOICE", "s");
        break;

      default:
        break;
     }

#undef WRITE_TEMPLATE

   return EINA_TRUE;
}

static Eina_Bool
_write_part_external(Eina_List **parts)
{
   char buf[1024];
   Eina_Iterator *itr;
   Part_External_Info *ei;
   const Eina_Hash_Tuple *tuple;
   Eina_List *l, *l_next;
   const char *name;
   Eina_Bool ret = EINA_TRUE;

   itr = edje_external_iterator_get();
   EINA_ITERATOR_FOREACH(itr, tuple)
     {
        const Edje_External_Type *type = tuple->data;
        const Edje_External_Param_Info *param;
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

        EINA_LIST_FOREACH_SAFE(*parts, l, l_next, ei)
          {
             if (!strcmp(ei->source, name))
               {
                  if (ei->description)
                    {
                       snprintf(buf, sizeof(buf), "\n/**\n * @brief %s\n */\n",
                                ei->description);
                       if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
                         {
                            ret = EINA_FALSE;
                            goto end;
                         }
                    }

                  if (ei->draggable)
                    {
                       if (!_write_part_draggable(ei->apiname, ei->name))
                         {
                            ret = EINA_FALSE;
                            goto end;
                         }
                    }

                  for (param = type->parameters_info; param->name != NULL;
                       param++)
                    if (!_write_part_external_param(ei, param))
                      {
                         ret = EINA_FALSE;
                         goto end;
                      }

                  edje_edit_string_free(ei->description);
                  free(ei->apiname);
                  free(ei);

                  *parts = eina_list_remove_list(*parts, l);
               }
          }
     }

end:
   if (eina_list_count(*parts) == 0) *parts = NULL;
   eina_iterator_free(itr);

   return ret;
}

static Eina_Bool
_write_part(const char *apiname, const char *partname, Edje_Part_Type parttype,
            const char *description, Eina_Bool draggable)
{
   char buf[512];

#define TEMPLATE_NAME(sufix)                                 \
  do {                                                       \
       snprintf(buf, sizeof(buf), C_CODEGEN_##sufix, prefix, \
                apiname, partname);                          \
       if (fwrite(buf, strlen(buf), 1, source_fd) != 1)      \
         goto err;                                           \
       snprintf(buf, sizeof(buf), H_CODEGEN_##sufix, prefix, \
                apiname);                                    \
       if (fwrite(buf, strlen(buf), 1, header_fd) != 1)      \
         goto err;                                           \
    } while (0)

   if (description)
     {
        snprintf(buf, sizeof(buf), "\n/**\n * @brief %s\n */\n", description);
        if (fwrite(buf, strlen(buf), 1, header_fd) != 1)
          goto err;
     }

   switch (parttype)
     {
      case EDJE_PART_TYPE_BOX:
        TEMPLATE_NAME(PART_BOX_APPEND);
        TEMPLATE_NAME(PART_BOX_PREPEND);
        TEMPLATE_NAME(PART_BOX_INSERT_BEFORE);
        TEMPLATE_NAME(PART_BOX_INSERT_AT);
        TEMPLATE_NAME(PART_BOX_REMOVE);
        TEMPLATE_NAME(PART_BOX_REMOVE_AT);
        TEMPLATE_NAME(PART_BOX_REMOVE_ALL);
        break;

      case EDJE_PART_TYPE_TABLE:
        TEMPLATE_NAME(PART_TABLE_PACK);
        TEMPLATE_NAME(PART_TABLE_UNPACK);
        TEMPLATE_NAME(PART_TABLE_CHILD_GET);
        TEMPLATE_NAME(PART_TABLE_CLEAR);
        TEMPLATE_NAME(PART_TABLE_COL_ROW_SIZE_GET);
        break;

      case EDJE_PART_TYPE_TEXT:
        TEMPLATE_NAME(PART_TEXT_SET);
        TEMPLATE_NAME(PART_TEXT_GET);
        break;

      case EDJE_PART_TYPE_SWALLOW:
        TEMPLATE_NAME(PART_SWALLOW_SET);
        TEMPLATE_NAME(PART_SWALLOW_GET);
        break;

      default:
        break;
     }

   if (draggable)
     if (!_write_part_draggable(apiname, partname))
       goto err;

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
   Eina_List *parts, *l, *parts_external = NULL;
   const char *name, *description;
   char *apiname;
   Edje_Part_Type type;
   Eina_Bool draggable, ret = EINA_TRUE;
   Part_External_Info *ei;

   parts = edje_edit_parts_list_get(ed);
   EINA_LIST_FOREACH(parts, l, name)
     {
        if (!(apiname = _part_api_name_get(ed, name)))
          {
             DBG("filter out part '%s': not API.", name);
             continue;
          }

        type = edje_edit_part_type_get(ed, name);
        if (!((type == EDJE_PART_TYPE_TEXT) ||
              (type == EDJE_PART_TYPE_SWALLOW) ||
              (type == EDJE_PART_TYPE_BOX) ||
              (type == EDJE_PART_TYPE_EXTERNAL) ||
              (type == EDJE_PART_TYPE_IMAGE) ||
              (type == EDJE_PART_TYPE_TABLE)))
          {
             ERR("Invalid part type %d", type);
             free(apiname);
             continue;
          }
        if (edje_edit_part_drag_x_get(ed, name) ||
            edje_edit_part_drag_y_get(ed, name))
          draggable = EINA_TRUE;
        else
          draggable = EINA_FALSE;

        description = edje_edit_part_api_description_get(ed, name);
        if (type == EDJE_PART_TYPE_EXTERNAL)
          {
             ei = calloc(1, sizeof(Part_External_Info));
             if (!ei)
               {
                  free(apiname);
                  goto end;
               }
             ei->description = description;
             ei->source = edje_edit_part_source_get(ed, name);
             ei->apiname = apiname;
             ei->name = name;
             ei->draggable = draggable;

             parts_external = eina_list_append(parts_external, ei);
          }
        else
          {
             if (!_write_part(apiname, name, type, description, draggable))
               {
                  ret = EINA_FALSE;
                  edje_edit_string_free(description);
                  free(apiname);
                  goto end;
               }

             edje_edit_string_free(description);
             free(apiname);
          }
     }

   ret = _write_part_external(&parts_external);

end:
   edje_edit_string_list_free(parts);
   EINA_LIST_FREE(parts_external, ei)
     {
        edje_edit_string_free(ei->description);
        free(ei->apiname);
        free(ei);
     }

   return ret;
}

static Eina_Bool
_write_program_emit(const char *apiname, const char *source, const char *sig,
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
_write_program_add(const char *apiname, const char *source, const char *sig,
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
_parse_programs(Evas_Object *ed)
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

        sig = edje_edit_program_signal_get(ed, name);
        if (!sig)
          {
             free(apiname);
             edje_edit_string_free(description);
             continue;
          }

        source = edje_edit_program_source_get(ed, name);
        if (!source)
          {
             free(apiname);
             edje_edit_string_free(sig);
             edje_edit_string_free(description);
             continue;
          }

        if (!_write_program_emit(apiname, source, sig, description))
          {
             ret = EINA_FALSE;
             break;
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
_module_matches(const char *module)
{
   Eina_List *l;
   const char *name;

   EINA_LIST_FOREACH(externals, l, name)
     {
        if (!strcmp(module, name))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

static Eina_Bool
_parse(void)
{
   Evas_Object *ed;
   Eina_Bool ret;
   const char *module_name;
   const Eina_List *modules_available, *l;
   unsigned short modules_loaded = 0;

   modules_available = edje_available_modules_get();
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

   externals = edje_edit_externals_list_get(ed);
   if (externals)
     {
        EINA_LIST_FOREACH(modules_available, l, module_name)
          {
             if (_module_matches(module_name))
               {
                  if (!edje_module_load(module_name))
                    {
                       ERR("error loading external '%s'", module_name);
                       continue;
                    }
                  modules_loaded++;
               }
          }
     }

   if (eina_list_count(externals) != modules_loaded)
     {
        edje_edit_string_list_free(externals);
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

   ecore_app_no_system_modules();

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
   else if (quit_option)
     goto error_getopt;
   else if (arg_index != argc - 4)
     {
        fprintf(stderr, "Incorrect number of parameters. Requires "          \
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

