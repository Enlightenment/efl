#include <libgen.h>
#include <Eina.h>
#include "eo_parser.h"
#include "eolian_database.h"

Eina_Hash *_classes = NULL;
Eina_Hash *_aliases = NULL;
Eina_Hash *_structs = NULL;
Eina_Hash *_classesf = NULL;
Eina_Hash *_aliasesf = NULL;
Eina_Hash *_structsf = NULL;
Eina_Hash *_filenames = NULL;
Eina_Hash *_tfilenames = NULL;

static int _database_init_count = 0;

static void
_hashlist_free(void *data)
{
   eina_list_free((Eina_List*)data);
}

int
database_init()
{
   if (_database_init_count > 0) return ++_database_init_count;
   eina_init();
   _classes = eina_hash_stringshared_new(EINA_FREE_CB(database_class_del));
   _aliases = eina_hash_stringshared_new(EINA_FREE_CB(database_typedef_del));
   _structs = eina_hash_stringshared_new(EINA_FREE_CB(database_type_del));
   _classesf = eina_hash_stringshared_new(NULL);
   _aliasesf = eina_hash_stringshared_new(_hashlist_free);
   _structsf = eina_hash_stringshared_new(_hashlist_free);
   _filenames = eina_hash_string_small_new(free);
   _tfilenames = eina_hash_string_small_new(free);
   return ++_database_init_count;
}

int
database_shutdown()
{
   if (_database_init_count <= 0)
     {
        ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   _database_init_count--;

   if (_database_init_count == 0)
     {
        eina_hash_free(_classes);
        eina_hash_free(_aliases);
        eina_hash_free(_structs);
        eina_hash_free(_classesf);
        eina_hash_free(_aliasesf);
        eina_hash_free(_structsf);
        eina_hash_free(_filenames);
        eina_hash_free(_tfilenames);
        eina_shutdown();
     }
   return _database_init_count;
}

#define EO_SUFFIX ".eo"
#define EOT_SUFFIX ".eot"

static char *
join_path(const char *path, const char *file)
{
   Eina_Strbuf *buf = eina_strbuf_new();
   char *ret;

   eina_strbuf_append(buf, path);
   eina_strbuf_append_char(buf, '/');
   eina_strbuf_append(buf, file);

   ret = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ret;
}

static void
_scan_cb(const char *name, const char *path, void *data EINA_UNUSED)
{
   size_t len;
   Eina_Bool is_eo = eina_str_has_suffix(name, EO_SUFFIX);
   if (!is_eo && !eina_str_has_suffix(name, EOT_SUFFIX)) return;
   len = strlen(name) - (is_eo ? sizeof(EO_SUFFIX) : sizeof(EOT_SUFFIX)) + 1;
   eina_hash_add(is_eo ? _filenames : _tfilenames,
                 eina_stringshare_add_length(name, len), join_path(path, name));
}

EAPI Eina_Bool
eolian_directory_scan(const char *dir)
{
   if (!dir) return EINA_FALSE;
   eina_file_dir_list(dir, EINA_TRUE, _scan_cb, NULL);
   return EINA_TRUE;
}

EAPI Eina_Bool
eolian_system_directory_scan()
{
   Eina_Bool ret;
   Eina_Strbuf *buf = eina_strbuf_new();
   eina_strbuf_append(buf, eina_prefix_data_get(_eolian_prefix));
   eina_strbuf_append(buf, "/include");
   ret = eolian_directory_scan(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return ret;
}

char *
database_class_to_filename(const char *cname)
{
   char *ret;
   Eina_Strbuf *strbuf = eina_strbuf_new();
   eina_strbuf_append(strbuf, cname);
   eina_strbuf_replace_all(strbuf, ".", "_");

   ret = eina_strbuf_string_steal(strbuf);
   eina_strbuf_free(strbuf);

   eina_str_tolower(&ret);

   return ret;
}

EAPI Eina_Bool
eolian_eot_file_parse(const char *filepath)
{
   return eo_parser_database_fill(filepath, EINA_TRUE);
}

EAPI Eina_Bool
eolian_eo_file_parse(const char *filepath)
{
   Eina_Iterator *itr;
   char *bfiledup = strdup(filepath);
   char *bfilename = basename(bfiledup);
   const Eolian_Class *class = eolian_class_get_by_file(bfilename);
   const char *inherit_name;
   Eolian_Implement *impl;
   if (!class)
     {
        if (!eo_parser_database_fill(filepath, EINA_FALSE))
          {
             free(bfiledup);
             return EINA_FALSE;
          }
        class = eolian_class_get_by_file(bfilename);
        if (!class)
          {
             ERR("No class for file %s", bfilename);
             free(bfiledup);
             return EINA_FALSE;
          }
     }
   free(bfiledup);
   itr = eolian_class_inherits_get(class);
   EINA_ITERATOR_FOREACH(itr, inherit_name)
     {
        if (!eolian_class_get_by_name(inherit_name))
          {
             char *filename = database_class_to_filename(inherit_name);
             filepath = eina_hash_find(_filenames, filename);
             free(filename);
             if (!filepath)
               {
                  ERR("Unable to find a file for class %s", inherit_name);
                  return EINA_FALSE;
               }
             if (!eolian_eo_file_parse(filepath)) return EINA_FALSE;
          }
     }
   eina_iterator_free(itr);
   itr = eolian_class_implements_get(class);
   EINA_ITERATOR_FOREACH(itr, impl)
     {
        const Eolian_Class *impl_class;
        const Eolian_Function *impl_func;
        Eolian_Function_Type impl_type = EOLIAN_UNRESOLVED;
        eolian_implement_information_get(impl, &impl_class, &impl_func, &impl_type);
        if (!impl_func)
          {
             ERR("Unable to find function %s", eolian_implement_full_name_get(impl));
             return EINA_FALSE;
          }
     }
   eina_iterator_free(itr);
   return EINA_TRUE;
}

static Eina_Bool _tfile_parse(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Eina_Bool *ret = fdata;
   if (*ret) *ret = eolian_eot_file_parse(data);
   return *ret;
}

EAPI Eina_Bool
eolian_all_eot_files_parse()
{
   Eina_Bool ret = EINA_TRUE;
   eina_hash_foreach(_tfilenames, _tfile_parse, &ret);
   return ret;
}

static Eina_Bool _file_parse(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Eina_Bool *ret = fdata;
   if (*ret) *ret = eolian_eo_file_parse(data);
   return *ret;
}

EAPI Eina_Bool
eolian_all_eo_files_parse()
{
   Eina_Bool ret = EINA_TRUE;
   eina_hash_foreach(_filenames, _file_parse, &ret);
   return ret;
}

