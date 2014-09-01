#include <libgen.h>
#include <Eina.h>
#include "eo_parser.h"
#include "eolian_database.h"

Eina_Hash *_classes    = NULL;
Eina_Hash *_aliases    = NULL;
Eina_Hash *_structs    = NULL;
Eina_Hash *_enums      = NULL;
Eina_Hash *_globals    = NULL;
Eina_Hash *_constants  = NULL;
Eina_Hash *_classesf   = NULL;
Eina_Hash *_aliasesf   = NULL;
Eina_Hash *_structsf   = NULL;
Eina_Hash *_enumsf     = NULL;
Eina_Hash *_globalsf   = NULL;
Eina_Hash *_constantsf = NULL;
Eina_Hash *_filenames  = NULL;
Eina_Hash *_tfilenames = NULL;
Eina_Hash *_depclasses = NULL;

static int _database_init_count = 0;

static void
_hashlist_free(void *data)
{
   eina_list_free((Eina_List*)data);
}

static void
_deplist_free(Eina_List *data)
{
   Eolian_Dependency *dep;
   EINA_LIST_FREE(data, dep)
     {
        eina_stringshare_del(dep->base.file);
        eina_stringshare_del(dep->filename);
        eina_stringshare_del(dep->name);
        free(dep);
     }
}

int
database_init()
{
   if (_database_init_count > 0) return ++_database_init_count;
   eina_init();
   _classes    = eina_hash_stringshared_new(EINA_FREE_CB(database_class_del));
   _aliases    = eina_hash_stringshared_new(EINA_FREE_CB(database_typedef_del));
   _structs    = eina_hash_stringshared_new(EINA_FREE_CB(database_type_del));
   _enums      = eina_hash_stringshared_new(EINA_FREE_CB(database_type_del));
   _globals    = eina_hash_stringshared_new(EINA_FREE_CB(database_var_del));
   _constants  = eina_hash_stringshared_new(EINA_FREE_CB(database_var_del));
   _classesf   = eina_hash_stringshared_new(NULL);
   _aliasesf   = eina_hash_stringshared_new(_hashlist_free);
   _structsf   = eina_hash_stringshared_new(_hashlist_free);
   _enumsf     = eina_hash_stringshared_new(_hashlist_free);
   _globalsf   = eina_hash_stringshared_new(_hashlist_free);
   _constantsf = eina_hash_stringshared_new(_hashlist_free);
   _filenames  = eina_hash_string_small_new(free);
   _tfilenames = eina_hash_string_small_new(free);
   _depclasses = eina_hash_stringshared_new(EINA_FREE_CB(_deplist_free));
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
        eina_hash_free(_classes   ); _classes    = NULL;
        eina_hash_free(_aliases   ); _aliases    = NULL;
        eina_hash_free(_structs   ); _structs    = NULL;
        eina_hash_free(_enums     ); _enums      = NULL;
        eina_hash_free(_globals   ); _globals    = NULL;
        eina_hash_free(_constants ); _constants  = NULL;
        eina_hash_free(_classesf  ); _classesf   = NULL;
        eina_hash_free(_aliasesf  ); _aliasesf   = NULL;
        eina_hash_free(_structsf  ); _structsf   = NULL;
        eina_hash_free(_enumsf    ); _enumsf     = NULL;
        eina_hash_free(_globalsf  ); _globalsf   = NULL;
        eina_hash_free(_constantsf); _constantsf = NULL;
        eina_hash_free(_filenames ); _filenames  = NULL;
        eina_hash_free(_tfilenames); _tfilenames = NULL;
        eina_hash_free(_depclasses); _depclasses = NULL;
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

/*
 * ret false -> clash, class = NULL
 * ret true && class -> only one class corresponding
 * ret true && !class -> no class corresponding
 */
Eina_Bool
database_class_name_validate(const char *class_name, const Eolian_Class **cl)
{
   char *name = strdup(class_name);
   char *colon = name + 1;
   const Eolian_Class *found_class = NULL;
   const Eolian_Class *candidate;
   if (cl) *cl = NULL;
   do
     {
        colon = strchr(colon, '.');
        if (colon) *colon = '\0';
        candidate = eolian_class_get_by_name(name);
        if (candidate)
          {
             if (found_class)
               {
                  ERR("Name clash between class %s and class %s",
                        candidate->full_name,
                        found_class->full_name);
                  free(name);
                  return EINA_FALSE; // Names clash
               }
             found_class = candidate;
          }
        if (colon) *colon++ = '.';
     }
   while(colon);
   if (cl) *cl = found_class;
   free(name);
   return EINA_TRUE;
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
   Eina_List *depl;
   Eolian_Dependency *dep;
   char *bfiledup = strdup(filepath);
   char *bfilename = basename(bfiledup);
   const Eolian_Class *class = eolian_class_get_by_file(bfilename);
   const char *inherit_name;
   Eolian_Implement *impl;
   Eolian_Constructor *ctor;
   Eina_Bool failed_dep = EINA_FALSE;
   if (!class)
     {
        if (!eo_parser_database_fill(filepath, EINA_FALSE))
          {
             free(bfiledup);
             goto error;
          }
        class = eolian_class_get_by_file(bfilename);
        if (!class)
          {
             ERR("No class for file %s", bfilename);
             free(bfiledup);
             goto error;
          }
     }
   free(bfiledup);
   /* parse dependencies first */
   depl = eina_hash_find(_depclasses, eolian_class_file_get(class));
   if (!depl)
     goto inherits;
   eina_hash_set(_depclasses, eolian_class_file_get(class), NULL);
   EINA_LIST_FREE(depl, dep)
     {
        if (failed_dep) goto free;
        if (!eolian_class_get_by_name(dep->name) &&
            !eolian_eo_file_parse(dep->filename))
          {
             eina_log_print(_eolian_log_dom, EINA_LOG_LEVEL_ERR,
                 dep->base.file, "", dep->base.line, "failed to parse "
                   "dependency '%s' at column %d", dep->name, dep->base.column);
             failed_dep = EINA_TRUE; /* do not parse anymore stuff */
          }
free:
        eina_stringshare_del(dep->base.file);
        eina_stringshare_del(dep->filename);
        eina_stringshare_del(dep->name);
        free(dep);
     }
   if (failed_dep)
     goto error;
   /* and then inherits */
inherits:
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
                  goto error;
               }
             if (!eolian_eo_file_parse(filepath)) goto error;
          }
     }
   eina_iterator_free(itr);
   itr = eolian_class_implements_get(class);
   EINA_ITERATOR_FOREACH(itr, impl)
     {
        Eolian_Function_Type impl_type = EOLIAN_UNRESOLVED;
        const Eolian_Function *impl_func = eolian_implement_function_get(impl, &impl_type);
        if (!impl_func)
          {
             ERR("Unable to find function %s", eolian_implement_full_name_get(impl));
             goto error;
          }
     }
   eina_iterator_free(itr);
   itr = eolian_class_constructors_get(class);
   EINA_ITERATOR_FOREACH(itr, ctor)
     {
        const Eolian_Function *ctor_func = eolian_constructor_function_get(ctor);
        if (!ctor_func)
          {
             ERR("Unable to find function %s", eolian_constructor_full_name_get(ctor));
             goto error;
          }
     }
   eina_iterator_free(itr);

   return EINA_TRUE;

error:
   return EINA_FALSE;
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

EAPI Eina_Bool
eolian_database_validate(void)
{
   return database_validate();
}
