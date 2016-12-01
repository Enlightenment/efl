#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ctype.h>
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

Eina_Hash *_defereos = NULL;

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
   _aliases    = eina_hash_stringshared_new(EINA_FREE_CB(database_typedecl_del));
   _structs    = eina_hash_stringshared_new(EINA_FREE_CB(database_typedecl_del));
   _enums      = eina_hash_stringshared_new(EINA_FREE_CB(database_typedecl_del));
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
   _defereos   = eina_hash_string_small_new(NULL);
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
        eina_hash_free(_defereos  ); _defereos   = NULL;
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

EAPI const Eolian_Declaration *
eolian_declaration_get_by_name(const char *name)
{
   if (!_decls) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   const Eolian_Declaration *decl = eina_hash_find(_decls, shr);
   eina_stringshare_del(shr);
   return decl;
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

EAPI Eina_Iterator *
eolian_all_declarations_get(void)
{
   return (_decls ? eina_hash_iterator_data_new(_decls) : NULL);
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

EAPI const Eolian_Typedecl *
eolian_declaration_data_type_get(const Eolian_Declaration *decl)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(decl, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(decl->type == EOLIAN_DECL_ALIAS ||
                                   decl->type == EOLIAN_DECL_STRUCT ||
                                   decl->type == EOLIAN_DECL_ENUM, NULL);
   return (const Eolian_Typedecl *)decl->data;
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
   eina_stringshare_del(doc->since);
   free(doc);
}

EAPI Eina_Stringshare *
eolian_documentation_summary_get(const Eolian_Documentation *doc)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(doc, NULL);
   return doc->summary;
}

EAPI Eina_Stringshare *
eolian_documentation_description_get(const Eolian_Documentation *doc)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(doc, NULL);
   return doc->description;
}

EAPI Eina_Stringshare *
eolian_documentation_since_get(const Eolian_Documentation *doc)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(doc, NULL);
   return doc->since;
}

EAPI Eina_List *
eolian_documentation_string_split(const char *doc)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(doc, NULL);
   const char *sep = strstr(doc, "\n\n");
   Eina_List *ret = NULL;
   while (doc)
     {
        Eina_Strbuf *buf = eina_strbuf_new();
        if (sep)
          eina_strbuf_append_length(buf, doc, sep - doc);
        else
          eina_strbuf_append(buf, doc);
        eina_strbuf_trim(buf);
        if (eina_strbuf_length_get(buf))
          ret = eina_list_append(ret, eina_strbuf_string_steal(buf));
        eina_strbuf_free(buf);
        if (!sep)
          break;
        doc = sep + 2;
        sep = strstr(doc, "\n\n");
     }
   return ret;
}

static Eina_Bool
_skip_ref_word(const char **doc)
{
   if (((*doc)[0] != '_') && !isalpha((*doc)[0]))
     return EINA_FALSE;

   while (((*doc)[0] == '_') || isalnum((*doc)[0]))
     ++*doc;

   return EINA_TRUE;
}

/* this make sure the format is correct at least, it cannot verify the
 * correctness of the reference itself (but Eolian will do it in its
 * lexer, so there is nothing to worry about; all references are guaranteed
 * to be right
 */
static Eolian_Doc_Token_Type
_get_ref_token(const char *doc, const char **doc_end)
{
   /* not a ref at all, for convenience */
   if (doc[0] != '@')
     return EOLIAN_DOC_TOKEN_UNKNOWN;

   ++doc;

   Eina_Bool is_event = (doc[0] == '[');
   if (is_event)
     ++doc;

   if (_skip_ref_word(&doc))
     {
        while (doc[0] == '.')
          {
             ++doc;
             if (!_skip_ref_word(&doc))
               {
                  --doc;
                  break;
               }
          }
        if (is_event) while (doc[0] == ',')
          {
             ++doc;
             if (!_skip_ref_word(&doc))
               {
                  --doc;
                  break;
               }
          }
     }
   else
     return EOLIAN_DOC_TOKEN_UNKNOWN;

   if (is_event)
     {
        if (doc[0] != ']')
          return EOLIAN_DOC_TOKEN_UNKNOWN;
        ++doc;
     }

   if (doc_end)
     *doc_end = doc;

   /* got a reference */
   return is_event ? EOLIAN_DOC_TOKEN_REF_EVENT : EOLIAN_DOC_TOKEN_REF;
}

EAPI const char *
eolian_documentation_tokenize(const char *doc, Eolian_Doc_Token *ret)
{
   /* token is used for statekeeping, so force it */
   EINA_SAFETY_ON_NULL_RETURN_VAL(ret, NULL);

   /* we've reached the end or invalid input */
   if (!doc || !doc[0])
     {
        ret->text = ret->text_end = NULL;
        ret->type = EOLIAN_DOC_TOKEN_UNKNOWN;
        return NULL;
     }

   Eina_Bool cont = (ret->type != EOLIAN_DOC_TOKEN_UNKNOWN);

   /* we can only check notes etc at beginning of parsing */
   if (cont)
     goto mloop;

#define CMP_MARK_NOTE(doc, note) !strncmp(doc, note ": ", sizeof(note) + 1)

   /* different types of notes */
   if (CMP_MARK_NOTE(doc, "Note"))
     {
        ret->text = doc;
        ret->text_end = doc + sizeof("Note:");
        ret->type = EOLIAN_DOC_TOKEN_MARK_NOTE;
        return ret->text_end;
     }
   else if (CMP_MARK_NOTE(doc, "Warning"))
     {
        ret->text = doc;
        ret->text_end = doc + sizeof("Warning:");
        ret->type = EOLIAN_DOC_TOKEN_MARK_WARNING;
        return ret->text_end;
     }
   else if (CMP_MARK_NOTE(doc, "Remark"))
     {
        ret->text = doc;
        ret->text_end = doc + sizeof("Remark:");
        ret->type = EOLIAN_DOC_TOKEN_MARK_REMARK;
        return ret->text_end;
     }
   else if (CMP_MARK_NOTE(doc, "TODO"))
     {
        ret->text = doc;
        ret->text_end = doc + sizeof("TODO:");
        ret->type = EOLIAN_DOC_TOKEN_MARK_TODO;
        return ret->text_end;
     }

#undef CMP_MARK_NOTE

mloop:

   /* monospace markup ($foo) */
   if ((doc[0] == '$') && ((doc[1] == '_') || isalpha(doc[1])))
     {
        ret->text = ++doc;
        ret->text_end = ret->text;
        while ((ret->text_end[0] == '_') || isalnum(ret->text_end[0]))
          ++ret->text_end;
        ret->type = EOLIAN_DOC_TOKEN_MARKUP_MONOSPACE;
        return ret->text_end;
     }

   /* references */
   Eolian_Doc_Token_Type rtp = _get_ref_token(doc, &ret->text_end);
   if (rtp != EOLIAN_DOC_TOKEN_UNKNOWN)
     {
        ret->text = doc + 1;
        ret->type = rtp;
        return ret->text_end;
     }

   const char *schr = doc, *pschr = NULL;
   /* keep finding potential tokens until a suitable one is found
    * terminate text token there (it also means next token can directly
    * be tested for event/monospace)
    */
   while ((schr = strpbrk(schr, "@$")))
     {
        /* escape sequences */
        if ((schr != doc) && (schr[-1] == '\\'))
          {
             schr += 1;
             continue;
          }
        /* monospace markup */
        if ((schr[0] == '$') && ((schr[1] == '_') || isalpha(schr[1])))
          {
             pschr = schr;
             break;
          }
        /* references */
        if (_get_ref_token(schr, NULL) != EOLIAN_DOC_TOKEN_UNKNOWN)
          {
             pschr = schr;
             break;
          }
        /* nothing, keep matching text from next char on */
        schr += 1;
     }

   /* figure out where we actually end */
   ret->text = doc;
   ret->text_end = pschr ? pschr : (doc + strlen(doc));
   ret->type = EOLIAN_DOC_TOKEN_TEXT;
   return ret->text_end;
}

EAPI void eolian_doc_token_init(Eolian_Doc_Token *tok)
{
   if (!tok)
     return;
   tok->type = EOLIAN_DOC_TOKEN_UNKNOWN;
   tok->text = tok->text_end = NULL;
}

EAPI Eolian_Doc_Token_Type
eolian_doc_token_type_get(const Eolian_Doc_Token *tok)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tok, EOLIAN_DOC_TOKEN_UNKNOWN);
   return tok->type;
}

EAPI char *
eolian_doc_token_text_get(const Eolian_Doc_Token *tok)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(tok, NULL);
   if (tok->type == EOLIAN_DOC_TOKEN_UNKNOWN)
     return NULL;
   Eina_Strbuf *buf = eina_strbuf_new();
   for (const char *p = tok->text; p != tok->text_end; ++p)
     {
        if (*p == '\\') ++p;
        if (p != tok->text_end)
          eina_strbuf_append_char(buf, *p);
     }
   return eina_strbuf_string_steal(buf);
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

   ret = eina_file_path_sanitize(eina_strbuf_string_get(buf));
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

const Eolian_Class *
database_object_class_fill(const char *class_name, const Eolian_Class **cl)
{
   if (*cl) return *cl;
   char *name = strdup(class_name);
   char *ldot = strrchr(name + 1, '.');
   if (ldot) *ldot = '\0';
   const Eolian_Class *found = eolian_class_get_by_name(name);
   *cl = found;
   free(name);
   return found;
}

static Eina_Bool
_eolian_file_parse_nodep(const char *filepath)
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
     {
        char *vpath = eina_file_path_sanitize(filepath);
        Eina_Bool ret = eo_parser_database_fill(vpath, !is_eo);
        free(vpath);
        return ret;
     }
   return eo_parser_database_fill(eopath, !is_eo);
}

EAPI Eina_Bool
eolian_file_parse(const char *filepath)
{
   const char *dep;
   if (!_eolian_file_parse_nodep(filepath))
     return EINA_FALSE;
   /* parse doc dependencies (deferred eo files) */
   Eina_Iterator *itr = eina_hash_iterator_data_new(_defereos);
   EINA_ITERATOR_FOREACH(itr, dep)
     {
        if (!_eolian_file_parse_nodep(dep))
          {
             eina_iterator_free(itr);
             eina_hash_free_buckets(_defereos);
             return EINA_FALSE;
          }
     }
   eina_iterator_free(itr);
   eina_hash_free_buckets(_defereos);
   return EINA_TRUE;
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
eolian_database_validate()
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
