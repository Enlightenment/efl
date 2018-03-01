#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ctype.h>
#include <Eina.h>
#include "eo_parser.h"
#include "eolian_database.h"
#include "eolian_priv.h"

void
database_decl_add(Eolian_Unit *unit, Eina_Stringshare *name,
                  Eolian_Declaration_Type type,
                  Eina_Stringshare *file, void *ptr)
{
   Eolian_Declaration *decl = calloc(1, sizeof(Eolian_Declaration));
   decl->base = *((Eolian_Object *)ptr);
   decl->base.file = eina_stringshare_ref(decl->base.file);
   decl->base.refcount = 0;
   decl->type = type;
   decl->name = name;
   decl->data = ptr;
   EOLIAN_OBJECT_ADD(unit, name, decl, decls);
   eina_hash_set(unit->state->decls_f, file, eina_list_append
                 ((Eina_List*)eina_hash_find(unit->state->decls_f, file), decl));
}

static void
database_decl_del(Eolian_Declaration *decl)
{
   if (!decl || eolian_object_unref(&decl->base)) return;
   eina_stringshare_del(decl->base.file);
   free(decl);
}

EAPI const Eolian_Declaration *
eolian_declaration_get_by_name(const Eolian_Unit *unit, const char *name)
{
   if (!unit) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   const Eolian_Declaration *decl = eina_hash_find(unit->decls, shr);
   eina_stringshare_del(shr);
   return decl;
}

EAPI Eina_Iterator *
eolian_declarations_get_by_file(const Eolian_State *state, const char *fname)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(fname);
   Eina_List *l = eina_hash_find(state->decls_f, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_all_declarations_get(const Eolian_Unit *unit)
{
   return (unit ? eina_hash_iterator_data_new(unit->decls) : NULL);
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
   if (!doc[0])
     return NULL;
   const char *sep = strstr(doc, "\n\n");
   Eina_List *ret = NULL;
   for (;;)
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
   return EOLIAN_DOC_TOKEN_REF;
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

EAPI void
eolian_doc_token_init(Eolian_Doc_Token *tok)
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
   char *ptr = eina_strbuf_string_steal(buf);
   eina_strbuf_free(buf);
   return ptr;
}

static Eolian_Doc_Ref_Type
_resolve_event(const Eolian_Unit *src, char *name, const void **data,
               const void **data2)
{
   /* never trust the user */
   if (name[0] == ',')
     return EOLIAN_DOC_REF_INVALID;

   char *evname = strrchr(name, '.');
   if (!evname)
     return EOLIAN_DOC_REF_INVALID;

   *evname++ = '\0';
   const Eolian_Class *cl = eolian_class_get_by_name(src, name);
   if (!cl)
     return EOLIAN_DOC_REF_INVALID;

   const Eolian_Event *ev = eolian_class_event_get_by_name(cl, evname);
   if (!ev)
     return EOLIAN_DOC_REF_INVALID;

   if (data) *data = cl;
   if (data2) *data2 = ev;
   return EOLIAN_DOC_REF_EVENT;
}

EAPI Eolian_Doc_Ref_Type
eolian_doc_token_ref_get(const Eolian_Unit *unit, const Eolian_Doc_Token *tok,
                         const void **data, const void **data2)
{
   if (tok->type != EOLIAN_DOC_TOKEN_REF)
     return EOLIAN_DOC_REF_INVALID;

   size_t nlen = tok->text_end - tok->text;

   /* events are handled separately */
   if (tok->text[0] == '[')
     {
        /* strip brackets */
        size_t elen = nlen - 2;
        char *ename = alloca(elen + 1);
        memcpy(ename, tok->text + 1, elen);
        ename[elen] = '\0';
        return _resolve_event(unit, ename, data, data2);
     }

   char *name = alloca(nlen + 1);
   memcpy(name, tok->text, nlen);
   name[nlen] = '\0';

   const Eolian_Declaration *decl = eolian_declaration_get_by_name(unit, name);
   if (decl) switch (eolian_declaration_type_get(decl))
     {
      case EOLIAN_DECL_CLASS:
        if (data) *data = eolian_declaration_class_get(decl);
        return EOLIAN_DOC_REF_CLASS;
      case EOLIAN_DECL_ALIAS:
        if (data) *data = eolian_declaration_data_type_get(decl);
        return EOLIAN_DOC_REF_ALIAS;
      case EOLIAN_DECL_STRUCT:
        if (data) *data = eolian_declaration_data_type_get(decl);
        return EOLIAN_DOC_REF_STRUCT;
      case EOLIAN_DECL_ENUM:
        if (data) *data = eolian_declaration_data_type_get(decl);
        return EOLIAN_DOC_REF_ENUM;
      case EOLIAN_DECL_VAR:
        if (data) *data = eolian_declaration_variable_get(decl);
        return EOLIAN_DOC_REF_VAR;
      default:
        /* this will not happen but silence static analyzers */
        return EOLIAN_DOC_REF_INVALID;
     }

   /* from here it can only be a function, a struct field or an enum field */

   char *suffix = strrchr(name, '.');
   /* no suffix, therefore invalid */
   if (!suffix)
     return EOLIAN_DOC_REF_INVALID;

   /* name will terminate before suffix, suffix will be standalone */
   *suffix++ = '\0';

   /* try a struct field */
   const Eolian_Typedecl *tpd = eolian_typedecl_struct_get_by_name(unit, name);
   if (tpd)
     {
        const Eolian_Struct_Type_Field *fld = eolian_typedecl_struct_field_get(tpd, suffix);
        /* field itself is invalid */
        if (!fld)
          return EOLIAN_DOC_REF_INVALID;
        if (data) *data = tpd;
        if (data2) *data2 = fld;
        return EOLIAN_DOC_REF_STRUCT_FIELD;
     }

   /* try an enum field */
   tpd = eolian_typedecl_enum_get_by_name(unit, name);
   if (tpd)
     {
        const Eolian_Enum_Type_Field *fld = eolian_typedecl_enum_field_get(tpd, suffix);
        /* field itself is invalid */
        if (!fld)
          return EOLIAN_DOC_REF_INVALID;
        if (data) *data = tpd;
        if (data2) *data2 = fld;
        return EOLIAN_DOC_REF_ENUM_FIELD;
     }

   /* now it can only be a function or invalid */

   Eolian_Function_Type ftype = EOLIAN_UNRESOLVED;
   if (!strcmp(suffix, "get"))
     ftype = EOLIAN_PROP_GET;
   else if (!strcmp(suffix, "set"))
     ftype = EOLIAN_PROP_SET;

   if (ftype != EOLIAN_UNRESOLVED)
     {
        suffix = strrchr(name, '.');
        /* wrong suffix, therefore invalid */
        if (!suffix)
          return EOLIAN_DOC_REF_INVALID;
        /* re-terminate */
        *suffix++ = '\0';
     }

   const Eolian_Class *cl = eolian_class_get_by_name(unit, name);
   if (!cl)
     return EOLIAN_DOC_REF_INVALID;

   const Eolian_Function *fid = eolian_class_function_get_by_name(cl, suffix, ftype);
   if (!fid)
     return EOLIAN_DOC_REF_INVALID;

   /* got a func */
   if (data) *data = cl;
   if (data2) *data2 = fid;
   return EOLIAN_DOC_REF_FUNC;
}

void
database_unit_init(Eolian_State *state, Eolian_Unit *unit, const char *file)
{
   unit->file  = eina_stringshare_ref(file);
   unit->state = state;

   unit->children   = eina_hash_stringshared_new(NULL);
   unit->classes    = eina_hash_stringshared_new(EINA_FREE_CB(database_class_del));
   unit->globals    = eina_hash_stringshared_new(EINA_FREE_CB(database_var_del));
   unit->constants  = eina_hash_stringshared_new(EINA_FREE_CB(database_var_del));
   unit->aliases    = eina_hash_stringshared_new(EINA_FREE_CB(database_typedecl_del));
   unit->structs    = eina_hash_stringshared_new(EINA_FREE_CB(database_typedecl_del));
   unit->enums      = eina_hash_stringshared_new(EINA_FREE_CB(database_typedecl_del));
   unit->decls      = eina_hash_stringshared_new(EINA_FREE_CB(database_decl_del));
}

void
database_unit_del(Eolian_Unit *unit)
{
   if (!unit)
     return;

   eina_stringshare_del(unit->file);
   eina_hash_free(unit->classes);
   eina_hash_free(unit->globals);
   eina_hash_free(unit->constants);
   eina_hash_free(unit->aliases);
   eina_hash_free(unit->structs);
   eina_hash_free(unit->enums);
   eina_hash_free(unit->decls);
}

static void
_hashlist_free(void *data)
{
   eina_list_free((Eina_List*)data);
}

EAPI Eolian_State *
eolian_state_new(void)
{
   Eolian_State *state = calloc(1, sizeof(Eolian_State));
   if (!state)
     return NULL;

   database_unit_init(state, &state->unit, NULL);

   state->filenames_eo  = eina_hash_string_small_new(free);
   state->filenames_eot = eina_hash_string_small_new(free);

   state->defer = eina_hash_string_small_new(NULL);
   state->units = eina_hash_stringshared_new(EINA_FREE_CB(database_unit_del));

   state->classes_f   = eina_hash_stringshared_new(NULL);
   state->aliases_f   = eina_hash_stringshared_new(_hashlist_free);
   state->structs_f   = eina_hash_stringshared_new(_hashlist_free);
   state->enums_f     = eina_hash_stringshared_new(_hashlist_free);
   state->globals_f   = eina_hash_stringshared_new(_hashlist_free);
   state->constants_f = eina_hash_stringshared_new(_hashlist_free);
   state->decls_f     = eina_hash_stringshared_new(_hashlist_free);

   return state;
}

EAPI void
eolian_state_free(Eolian_State *state)
{
   if (!state)
     return;

   database_unit_del(&state->unit);

   eina_hash_free(state->filenames_eo);
   eina_hash_free(state->filenames_eot);

   eina_hash_free(state->defer);
   eina_hash_free(state->units);

   eina_hash_free(state->classes_f);
   eina_hash_free(state->aliases_f);
   eina_hash_free(state->structs_f);
   eina_hash_free(state->enums_f);
   eina_hash_free(state->globals_f);
   eina_hash_free(state->constants_f);
   eina_hash_free(state->decls_f);

   free(state);
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
   Eolian_State *state = data;
   Eina_Bool is_eo = eina_str_has_suffix(name, EO_SUFFIX);
   if (!is_eo && !eina_str_has_suffix(name, EOT_SUFFIX)) return;
   eina_hash_add(is_eo ? state->filenames_eo : state->filenames_eot,
                 eina_stringshare_add(name), join_path(path, name));
}

EAPI Eina_Bool
eolian_state_directory_add(Eolian_State *state, const char *dir)
{
   if (!dir || !state) return EINA_FALSE;
   eina_file_dir_list(dir, EINA_TRUE, _scan_cb, state);
   return EINA_TRUE;
}

EAPI Eina_Bool
eolian_state_system_directory_add(Eolian_State *state)
{
   Eina_Bool ret;
   Eina_Strbuf *buf = eina_strbuf_new();
   eina_strbuf_append(buf, eina_prefix_data_get(_eolian_prefix));
   eina_strbuf_append(buf, "/include");
   ret = eolian_directory_scan(state, eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return ret;
}

EAPI Eina_Bool
eolian_directory_scan(Eolian_State *state, const char *dir)
{
   return eolian_state_directory_add(state, dir);
}

EAPI Eina_Bool
eolian_system_directory_scan(Eolian_State *state)
{
   return eolian_state_system_directory_add(state);
}

EAPI Eina_Iterator *
eolian_state_eot_files_get(const Eolian_State *state)
{
   if (!state) return NULL;
   return eina_hash_iterator_key_new(state->filenames_eot);
}

EAPI Eina_Iterator *
eolian_state_eo_files_get(const Eolian_State *state)
{
   if (!state) return NULL;
   return eina_hash_iterator_key_new(state->filenames_eo);
}

EAPI Eina_Iterator *
eolian_state_eot_file_paths_get(const Eolian_State *state)
{
   if (!state) return NULL;
   return eina_hash_iterator_data_new(state->filenames_eot);
}

EAPI Eina_Iterator *
eolian_state_eo_file_paths_get(const Eolian_State *state)
{
   if (!state) return NULL;
   return eina_hash_iterator_data_new(state->filenames_eo);
}

EAPI Eina_Iterator *
eolian_all_eot_files_get(const Eolian_State *state)
{
   return eolian_state_eot_files_get(state);
}

EAPI Eina_Iterator *
eolian_all_eo_files_get(const Eolian_State *state)
{
   return eolian_state_eo_files_get(state);
}

EAPI Eina_Iterator *
eolian_all_eot_file_paths_get(const Eolian_State *state)
{
   return eolian_state_eot_file_paths_get(state);
}

EAPI Eina_Iterator *
eolian_all_eo_file_paths_get(const Eolian_State *state)
{
   return eolian_state_eo_file_paths_get(state);
}

static Eolian_Unit *
_eolian_file_parse_nodep(Eolian_Unit *parent, const char *filepath)
{
   Eina_Bool is_eo;
   const char *eopath;

   is_eo = eina_str_has_suffix(filepath, EO_SUFFIX);
   if (!is_eo && !eina_str_has_suffix(filepath, EOT_SUFFIX))
     {
        _eolian_log("file '%s' doesn't have a correct extension", filepath);
        return NULL;
     }
   if (!(eopath = eina_hash_find(is_eo ? parent->state->filenames_eo : parent->state->filenames_eot, filepath)))
     {
        char *vpath = eina_file_path_sanitize(filepath);
        Eolian_Unit *ret = eo_parser_database_fill(parent, vpath, !is_eo);
        free(vpath);
        return ret;
     }
   return eo_parser_database_fill(parent, eopath, !is_eo);
}

static Eina_Bool
_parse_deferred(Eolian_Unit *parent)
{
   Eina_Hash *defer = parent->state->defer;
   if (!defer || !eina_hash_population(defer))
     return EINA_TRUE;
   /* clean room for more deps for later parsing */
   parent->state->defer = eina_hash_string_small_new(NULL);
   Eina_Iterator *itr = eina_hash_iterator_data_new(defer);
   const char *dep;
   EINA_ITERATOR_FOREACH(itr, dep)
     {
        Eolian_Unit *pdep = _eolian_file_parse_nodep(parent, dep);
        if (!pdep || !_parse_deferred(pdep))
          {
             eina_iterator_free(itr);
             eina_hash_free_buckets(parent->state->defer);
             eina_hash_free(defer);
             return EINA_FALSE;
          }
     }
   eina_iterator_free(itr);
   eina_hash_free(defer);
   return EINA_TRUE;
}

static Eina_Bool
_merge_unit_cb(const Eina_Hash *hash EINA_UNUSED,
               const void *key, void *data, void *fdata)
{
   Eina_Hash *dest = fdata;
   if (!eina_hash_find(dest, key))
     {
        eina_hash_add(dest, key, data);
        eolian_object_ref((Eolian_Object *)data);
     }
   return EINA_TRUE;
}

static void
_merge_unit(Eolian_Unit *dest, Eolian_Unit *src)
{
   eina_hash_foreach(src->classes, _merge_unit_cb, dest->classes);
   eina_hash_foreach(src->globals, _merge_unit_cb, dest->globals);
   eina_hash_foreach(src->constants, _merge_unit_cb, dest->constants);
   eina_hash_foreach(src->aliases, _merge_unit_cb, dest->aliases);
   eina_hash_foreach(src->structs, _merge_unit_cb, dest->structs);
   eina_hash_foreach(src->enums, _merge_unit_cb, dest->enums);
   eina_hash_foreach(src->decls, _merge_unit_cb, dest->decls);
}

typedef struct _Merge_Data
{
   Eina_Hash *cycles;
   Eolian_Unit *unit;
} Merge_Data;

static Eina_Bool
_merge_units_cb(const Eina_Hash *hash EINA_UNUSED,
                const void *key EINA_UNUSED, void *data, void *fdata)
{
   Merge_Data *mdata = fdata;
   Merge_Data imdata = { mdata->cycles, data };
   if (!eina_hash_find(imdata.cycles, &imdata.unit))
     {
        eina_hash_add(imdata.cycles, &imdata.unit, imdata.unit);
        eina_hash_foreach(imdata.unit->children, _merge_units_cb, &imdata);
     }
   _merge_unit(mdata->unit, imdata.unit);
   return EINA_TRUE;
}


static void
_merge_units(Eolian_Unit *unit)
{
   Merge_Data mdata = { eina_hash_pointer_new(NULL), unit };
   eina_hash_foreach(unit->children, _merge_units_cb, &mdata);
   eina_hash_free(mdata.cycles);
}

EAPI const Eolian_Unit *
eolian_state_file_parse(Eolian_State *state, const char *filepath)
{
   if (!state)
     return NULL;

   Eolian_Unit *ret = _eolian_file_parse_nodep((Eolian_Unit *)state, filepath);
   if (!ret)
     return NULL;
   if (!_parse_deferred(ret))
     return NULL;
   _merge_units(ret);
   /* FIXME: pass unit properly */
   if (!database_validate(state, &state->unit))
     return NULL;
   return &state->unit;
}

EAPI const Eolian_Unit *
eolian_file_parse(Eolian_State *state, const char *filepath)
{
   return eolian_state_file_parse(state, filepath);
}

typedef struct _Parse_Data
{
   Eolian_State *state;
   Eina_Bool ret;
} Parse_Data;

static Eina_Bool _tfile_parse(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Parse_Data *pd = fdata;
   Eolian_Unit *unit = NULL;
   if (pd->ret)
     unit = eo_parser_database_fill((Eolian_Unit *)pd->state, data, EINA_TRUE);
   pd->ret = !!unit;
   if (pd->ret) pd->ret = _parse_deferred(unit);
   _merge_units(unit);
   return pd->ret;
}

EAPI Eina_Bool
eolian_state_all_eot_files_parse(Eolian_State *state)
{
   Parse_Data pd = { state, EINA_TRUE };

   if (!state)
     return EINA_FALSE;

   eina_hash_foreach(state->filenames_eot, _tfile_parse, &pd);

   /* FIXME: pass unit properly */
   if (pd.ret && !database_validate(state, &state->unit))
     return EINA_FALSE;

   return pd.ret;
}

EAPI Eina_Bool
eolian_all_eot_files_parse(Eolian_State *state)
{
   return eolian_state_all_eot_files_parse(state);
}

static Eina_Bool _file_parse(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Parse_Data *pd = fdata;
   Eolian_Unit *unit = NULL;
   if (pd->ret)
     unit = eo_parser_database_fill((Eolian_Unit *)pd->state, data, EINA_FALSE);
   pd->ret = !!unit;
   if (pd->ret) pd->ret = _parse_deferred(unit);
   _merge_units(unit);
   return pd->ret;
}

EAPI Eina_Bool
eolian_state_all_eo_files_parse(Eolian_State *state)
{
   Parse_Data pd = { state, EINA_TRUE };

   if (!state)
     return EINA_FALSE;

   eina_hash_foreach(state->filenames_eo, _file_parse, &pd);

   /* FIXME: pass unit properly */
   if (pd.ret && !database_validate(state, &state->unit))
     return EINA_FALSE;

   return pd.ret;
}

EAPI Eina_Bool
eolian_all_eo_files_parse(Eolian_State *state)
{
   return eolian_state_all_eo_files_parse(state);
}

EAPI const Eolian_Unit *
eolian_state_unit_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eolian_Unit *unit = eina_hash_find(state->units, shr);
   eina_stringshare_del(shr);
   return unit;
}

EAPI Eina_Iterator *
eolian_state_units_get(const Eolian_State *state)
{
   if (!state) return NULL;
   return eina_hash_iterator_data_new(state->units);
}

EAPI const Eolian_Class *
eolian_state_class_by_name_get(const Eolian_State *state, const char *class_name)
{
   return eolian_unit_class_by_name_get((Eolian_Unit *)state, class_name);
}

EAPI const Eolian_Class *
eolian_state_class_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eolian_Class *cl = eina_hash_find(state->classes_f, shr);
   eina_stringshare_del(shr);
   return cl;
}

EAPI Eina_Iterator *
eolian_state_classes_get(const Eolian_State *state)
{
   return eolian_unit_classes_get((Eolian_Unit *)state);
}

EAPI const Eolian_Variable *
eolian_state_global_by_name_get(const Eolian_State *state, const char *name)
{
   return eolian_unit_global_by_name_get((Eolian_Unit *)state, name);
}

EAPI const Eolian_Variable *
eolian_state_constant_by_name_get(const Eolian_State *state, const char *name)
{
    return eolian_unit_constant_by_name_get((Eolian_Unit *)state, name);
}

EAPI Eina_Iterator *
eolian_state_globals_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eina_List *l = eina_hash_find(state->globals_f, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_state_constants_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eina_List *l = eina_hash_find(state->constants_f, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_state_constants_get(const Eolian_State *state)
{
   return eolian_unit_constants_get((Eolian_Unit *)state);
}

EAPI Eina_Iterator *
eolian_state_globals_get(const Eolian_State *state)
{
   return eolian_unit_globals_get((Eolian_Unit *)state);
}

EAPI Eina_Iterator *
eolian_unit_children_get(const Eolian_Unit *unit)
{
   if (!unit) return NULL;
   return eina_hash_iterator_data_new(unit->children);
}

EAPI const char *
eolian_unit_file_get(const Eolian_Unit *unit)
{
   if (!unit) return NULL;
   return unit->file;
}

EAPI const Eolian_Class *
eolian_unit_class_by_name_get(const Eolian_Unit *unit, const char *class_name)
{
   if (!unit) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(class_name);
   Eolian_Class *cl = eina_hash_find(unit->classes, shr);
   eina_stringshare_del(shr);
   return cl;
}

EAPI Eina_Iterator *
eolian_unit_classes_get(const Eolian_Unit *unit)
{
   return (unit ? eina_hash_iterator_data_new(unit->classes) : NULL);
}

EAPI const Eolian_Class *
eolian_class_get_by_name(const Eolian_Unit *unit, const char *class_name)
{
   return eolian_unit_class_by_name_get(unit, class_name);
}

EAPI const Eolian_Class *
eolian_class_get_by_file(const Eolian_Unit *unit, const char *file_name)
{
   if (!unit) return NULL;
   return eolian_state_class_by_file_get(unit->state, file_name);
}

EAPI Eina_Iterator *
eolian_all_classes_get(const Eolian_Unit *unit)
{
   return eolian_unit_classes_get(unit);
}

EAPI const Eolian_Variable *
eolian_unit_global_by_name_get(const Eolian_Unit *unit, const char *name)
{
   if (!unit) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Variable *v = eina_hash_find(unit->globals, shr);
   eina_stringshare_del(shr);
   return v;
}

EAPI const Eolian_Variable *
eolian_unit_constant_by_name_get(const Eolian_Unit *unit, const char *name)
{
   if (!unit) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Variable *v = eina_hash_find(unit->constants, shr);
   eina_stringshare_del(shr);
   return v;
}

EAPI Eina_Iterator *
eolian_unit_constants_get(const Eolian_Unit *unit)
{
   return (unit ? eina_hash_iterator_data_new(unit->constants) : NULL);
}

EAPI Eina_Iterator *
eolian_unit_globals_get(const Eolian_Unit *unit)
{
   return (unit ? eina_hash_iterator_data_new(unit->globals) : NULL);
}

EAPI const Eolian_Variable *
eolian_variable_global_get_by_name(const Eolian_Unit *unit, const char *name)
{
   return eolian_unit_global_by_name_get(unit, name);
}

EAPI const Eolian_Variable *
eolian_variable_constant_get_by_name(const Eolian_Unit *unit, const char *name)
{
   return eolian_unit_constant_by_name_get(unit, name);
}

EAPI Eina_Iterator *
eolian_variable_globals_get_by_file(const Eolian_Unit *unit, const char *fname)
{
   if (!unit) return NULL;
   return eolian_state_globals_by_file_get(unit->state, fname);
}

EAPI Eina_Iterator *
eolian_variable_constants_get_by_file(const Eolian_Unit *unit,
                                      const char *fname)
{
   if (!unit) return NULL;
   return eolian_state_constants_by_file_get(unit->state, fname);
}

EAPI Eina_Iterator *
eolian_variable_all_constants_get(const Eolian_Unit *unit)
{
   return eolian_unit_constants_get(unit);
}

EAPI Eina_Iterator *
eolian_variable_all_globals_get(const Eolian_Unit *unit)
{
   return eolian_unit_globals_get(unit);
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
