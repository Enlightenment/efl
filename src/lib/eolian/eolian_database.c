#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

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
Eina_Hash *_decls      = NULL;
Eina_Hash *_declsf     = NULL;

Eina_Hash *_parsedeos  = NULL;
Eina_Hash *_parsingeos = NULL;

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
   _decls      = eina_hash_stringshared_new(free);
   _declsf     = eina_hash_stringshared_new(_hashlist_free);
   _parsedeos  = eina_hash_string_small_new(NULL);
   _parsingeos = eina_hash_string_small_new(NULL);
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
        eina_hash_free(_decls     ); _decls      = NULL;
        eina_hash_free(_declsf    ); _declsf     = NULL;
        eina_hash_free(_parsedeos ); _parsedeos  = NULL;
        eina_hash_free(_parsingeos); _parsingeos = NULL;
        eina_shutdown();
     }
   return _database_init_count;
}

void
database_decl_add(Eina_Stringshare *name, Eolian_Declaration_Type type,
                  Eina_Stringshare *file, void *ptr)
{
   Eolian_Declaration *decl = calloc(1, sizeof(Eolian_Declaration));
   decl->type = type;
   decl->name = name;
   decl->data = ptr;
   eina_hash_set(_decls, name, decl);
   eina_hash_set(_declsf, file, eina_list_append
                 ((Eina_List*)eina_hash_find(_declsf, file), decl));
}

EAPI Eina_Iterator *
eolian_declarations_get_by_file(const char *fname)
{
   if (!_declsf) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(fname);
   Eina_List *l = eina_hash_find(_declsf, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eolian_Declaration_Type
eolian_declaration_type_get(const Eolian_Declaration *decl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(decl, EOLIAN_DECL_UNKNOWN);
   return decl->type;
}

EAPI Eina_Stringshare *
eolian_declaration_name_get(const Eolian_Declaration *decl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(decl, NULL);
   return decl->name;
}

EAPI const Eolian_Class *
eolian_declaration_class_get(const Eolian_Declaration *decl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(decl, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(decl->type == EOLIAN_DECL_CLASS, NULL);
   return (const Eolian_Class *)decl->data;
}

EAPI const Eolian_Type *
eolian_declaration_data_type_get(const Eolian_Declaration *decl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(decl, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(decl->type == EOLIAN_DECL_ALIAS ||
                                   decl->type == EOLIAN_DECL_STRUCT ||
                                   decl->type == EOLIAN_DECL_ENUM, NULL);
   return (const Eolian_Type *)decl->data;
}


EAPI const Eolian_Variable *
eolian_declaration_variable_get(const Eolian_Declaration *decl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(decl, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(decl->type == EOLIAN_DECL_VAR, NULL);
   return (const Eolian_Variable *)decl->data;
}

void database_doc_del(Eolian_Documentation *doc)
{
   if (!doc) return;
   eina_stringshare_del(doc->summary);
   eina_stringshare_del(doc->description);
   free(doc);
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
   Eina_Bool is_eo = eina_str_has_suffix(name, EO_SUFFIX);
   if (!is_eo && !eina_str_has_suffix(name, EOT_SUFFIX)) return;
   eina_hash_add(is_eo ? _filenames : _tfilenames,
                 eina_stringshare_add(name), join_path(path, name));
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
   eina_strbuf_append(strbuf, ".eo");

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
                  fprintf(stderr, "eolian: name clash between classes '%s' and '%s'\n",
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
eolian_file_parse(const char *filepath)
{
   Eina_Bool is_eo;
   const char *eopath;
   if (_database_init_count <= 0)
     return EINA_FALSE;
   is_eo = eina_str_has_suffix(filepath, EO_SUFFIX);
   if (!is_eo && !eina_str_has_suffix(filepath, EOT_SUFFIX))
     {
        fprintf(stderr, "eolian: file '%s' doesn't have a correct extension\n", filepath);
        return EINA_FALSE;
     }
   if (!(eopath = eina_hash_find(is_eo ? _filenames : _tfilenames, filepath)))
     eopath = filepath;
   return eo_parser_database_fill(eopath, !is_eo);
}

static Eina_Bool _tfile_parse(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Eina_Bool *ret = fdata;
   if (*ret) *ret = eo_parser_database_fill(data, EINA_TRUE);
   return *ret;
}

EAPI Eina_Bool
eolian_all_eot_files_parse()
{
   Eina_Bool ret = EINA_TRUE;

   if (_database_init_count <= 0)
     return EINA_FALSE;

   eina_hash_foreach(_tfilenames, _tfile_parse, &ret);
   return ret;
}

static Eina_Bool _file_parse(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Eina_Bool *ret = fdata;
   if (*ret) *ret = eo_parser_database_fill(data, EINA_FALSE);
   return *ret;
}

EAPI Eina_Bool
eolian_all_eo_files_parse()
{
   Eina_Bool ret = EINA_TRUE;

   if (_database_init_count <= 0)
     return EINA_FALSE;

   eina_hash_foreach(_filenames, _file_parse, &ret);
   return ret;
}

EAPI Eina_Bool
eolian_database_validate(void)
{
   if (_database_init_count <= 0)
     return EINA_FALSE;

   return database_validate();
}

EAPI Eina_Iterator *
eolian_all_eot_files_get(void)
{
   if (!_tfilenames) return NULL;
   return eina_hash_iterator_key_new(_tfilenames);
}

EAPI Eina_Iterator *
eolian_all_eo_files_get(void)
{
   if (!_filenames) return NULL;
   return eina_hash_iterator_key_new(_filenames);
}

EAPI Eina_Iterator *
eolian_all_eot_file_paths_get(void)
{
   if (!_tfilenames) return NULL;
   return eina_hash_iterator_data_new(_tfilenames);
}

EAPI Eina_Iterator *
eolian_all_eo_file_paths_get(void)
{
   if (!_filenames) return NULL;
   return eina_hash_iterator_data_new(_filenames);
}
