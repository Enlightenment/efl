#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <ctype.h>
#include <Eina.h>
#include "eo_parser.h"
#include "eolian_database.h"
#include "eolian_priv.h"

void
database_object_add(Eolian_Unit *unit, const Eolian_Object *obj)
{
   /* object storage */
   eina_hash_add(unit->objects, obj->name, obj);
   eina_hash_add(unit->state->staging.unit.objects, obj->name, obj);
   eina_hash_set(unit->state->staging.objects_f, obj->file, eina_list_append
                 ((Eina_List *)eina_hash_find(unit->state->staging.objects_f, obj->file), obj));
}

EAPI Eolian_Object_Type
eolian_object_type_get(const Eolian_Object *obj)
{
   if (!obj) return EOLIAN_OBJECT_UNKNOWN;
   return obj->type;
}

EAPI const Eolian_Unit *
eolian_object_unit_get(const Eolian_Object *obj)
{
   if (!obj) return NULL;
   return obj->unit;
}

EAPI const char *
eolian_object_file_get(const Eolian_Object *obj)
{
   if (!obj) return NULL;
   return obj->file;
}

EAPI int
eolian_object_line_get(const Eolian_Object *obj)
{
   if (!obj) return 0;
   return obj->line;
}

EAPI int
eolian_object_column_get(const Eolian_Object *obj)
{
   if (!obj) return 0;
   return obj->column;
}

EAPI const char *
eolian_object_name_get(const Eolian_Object *obj)
{
   if (!obj) return NULL;
   return obj->name;
}

typedef struct _Eolian_Namespace_List
{
   Eina_Iterator itr;
   char *curp;
} Eolian_Namespace_List;

static Eina_Bool
_nmsp_iterator_next(Eolian_Namespace_List *it, void **data)
{
   if (!it || !it->curp)
     return EINA_FALSE;

   char *ndot = strchr(it->curp, '.');
   if (!ndot)
     return EINA_FALSE;

   *ndot = '\0';
   if (data) *data = it->curp;
   it->curp = ndot + 1;
   return EINA_TRUE;
}

static void *
_nmsp_container_get(Eina_Iterator *it EINA_UNUSED)
{
   return NULL;
}

EAPI const char *
eolian_object_short_name_get(const Eolian_Object *obj)
{
   if (!obj || !obj->name) return NULL;
   const char *ldot = strrchr(obj->name, '.');
   if (ldot)
     return ldot + 1;
   return obj->name;
}

EAPI Eina_Iterator *
eolian_object_namespaces_get(const Eolian_Object *obj)
{
   if (!obj || !obj->name || !strchr(obj->name, '.')) return NULL;

   size_t nstrl = strlen(obj->name) + 1;
   size_t anstrl = nstrl - 1 - (nstrl - 1) % sizeof(void *) + sizeof(void *);

   Eolian_Namespace_List *it = malloc(sizeof(Eolian_Namespace_List) + anstrl);
   memset(&it->itr, 0, sizeof(Eina_Iterator));
   it->curp = (char *)(it + 1);
   memcpy(it->curp, obj->name, nstrl);

   EINA_MAGIC_SET(&it->itr, EINA_MAGIC_ITERATOR);
   it->itr.version = EINA_ITERATOR_VERSION;
   it->itr.next = FUNC_ITERATOR_NEXT(_nmsp_iterator_next);
   it->itr.get_container = _nmsp_container_get;
   it->itr.free = FUNC_ITERATOR_FREE(free);
   return &it->itr;
}

EAPI Eina_Bool
eolian_object_is_beta(const Eolian_Object *obj)
{
   if (!obj) return EINA_FALSE;
   return obj->is_beta;
}

void database_doc_del(Eolian_Documentation *doc)
{
   if (!doc) return;
   eina_stringshare_del(doc->summary);
   eina_stringshare_del(doc->description);
   eina_stringshare_del(doc->since);
   eina_list_free(doc->ref_dbg);
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

static Eolian_Object_Type
_resolve_event(char *name, const Eolian_Unit *unit1, const Eolian_Unit *unit2,
               const Eolian_Object **data1, const Eolian_Object **data2)
{
   /* never trust the user */
   if (name[0] == ',')
     return EOLIAN_OBJECT_UNKNOWN;

   char *evname = strrchr(name, '.');
   if (!evname)
     return EOLIAN_OBJECT_UNKNOWN;

   *evname++ = '\0';
   const Eolian_Class *cl = eolian_unit_class_by_name_get(unit1, name);
   if (!cl && unit2)
     cl = eolian_unit_class_by_name_get(unit2, name);
   if (!cl)
     return EOLIAN_OBJECT_UNKNOWN;

   const Eolian_Event *ev = eolian_class_event_by_name_get(cl, evname);
   if (!ev)
     return EOLIAN_OBJECT_UNKNOWN;

   if (data1) *data1 = &cl->base;
   if (data2) *data2 = &ev->base;
   return EOLIAN_OBJECT_EVENT;
}

Eolian_Object_Type
database_doc_token_ref_resolve(const Eolian_Doc_Token *tok,
                               const Eolian_Unit *unit1, const Eolian_Unit *unit2,
                               const Eolian_Object **data1, const Eolian_Object **data2)
{
   if (tok->type != EOLIAN_DOC_TOKEN_REF)
     return EOLIAN_OBJECT_UNKNOWN;

   size_t nlen = tok->text_end - tok->text;

   /* events are handled separately */
   if (tok->text[0] == '[')
     {
        /* strip brackets */
        size_t elen = nlen - 2;
        char *ename = alloca(elen + 1);
        memcpy(ename, tok->text + 1, elen);
        ename[elen] = '\0';
        return _resolve_event(ename, unit1, unit2, data1, data2);
     }

   char *name = alloca(nlen + 1);
   memcpy(name, tok->text, nlen);
   name[nlen] = '\0';

   const Eolian_Object *decl = eolian_unit_object_by_name_get(unit1, name);
   if (!decl && unit2)
     decl = eolian_unit_object_by_name_get(unit2, name);
   if (decl)
     {
       if (data1) *data1 = decl;
       Eolian_Object_Type tp = eolian_object_type_get(decl);
       switch (tp)
         {
          case EOLIAN_OBJECT_CLASS:
          case EOLIAN_OBJECT_TYPEDECL:
          case EOLIAN_OBJECT_VARIABLE:
            /* we only allow certain types to be referenced */
            return tp;
          default:
            return EOLIAN_OBJECT_UNKNOWN;
         }
     }

   /* from here it can only be a function, a struct field or an enum field */

   char *suffix = strrchr(name, '.');
   /* no suffix, therefore invalid */
   if (!suffix)
     return EOLIAN_OBJECT_UNKNOWN;

   /* name will terminate before suffix, suffix will be standalone */
   *suffix++ = '\0';

   /* try a struct field */
   const Eolian_Typedecl *tpd = eolian_unit_struct_by_name_get(unit1, name);
   if (!tpd && unit2)
     tpd = eolian_unit_struct_by_name_get(unit2, name);
   if (tpd)
     {
        const Eolian_Struct_Type_Field *fld = eolian_typedecl_struct_field_get(tpd, suffix);
        /* field itself is invalid */
        if (!fld)
          return EOLIAN_OBJECT_UNKNOWN;
        if (data1) *data1 = &tpd->base;
        if (data2) *data2 = &fld->base;
        return EOLIAN_OBJECT_STRUCT_FIELD;
     }

   /* try an enum field */
   tpd = eolian_unit_enum_by_name_get(unit1, name);
   if (!tpd && unit2)
     tpd = eolian_unit_enum_by_name_get(unit2, name);
   if (tpd)
     {
        const Eolian_Enum_Type_Field *fld = eolian_typedecl_enum_field_get(tpd, suffix);
        /* field itself is invalid */
        if (!fld)
          return EOLIAN_OBJECT_UNKNOWN;
        if (data1) *data1 = &tpd->base;
        if (data2) *data2 = &fld->base;
        return EOLIAN_OBJECT_ENUM_FIELD;
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
          return EOLIAN_OBJECT_UNKNOWN;
        /* re-terminate */
        *suffix++ = '\0';
     }

   const Eolian_Class *cl = eolian_unit_class_by_name_get(unit1, name);
   if (!cl && unit2)
     cl = eolian_unit_class_by_name_get(unit2, name);
   if (!cl)
     return EOLIAN_OBJECT_UNKNOWN;

   const Eolian_Function *fid = eolian_class_function_by_name_get(cl, suffix, ftype);
   if (!fid)
     return EOLIAN_OBJECT_UNKNOWN;

   /* got a func */
   if (data1) *data1 = &cl->base;
   if (data2) *data2 = &fid->base;
   return EOLIAN_OBJECT_FUNCTION;
}

EAPI Eolian_Object_Type
eolian_doc_token_ref_resolve(const Eolian_Doc_Token *tok, const Eolian_State *state,
                             const Eolian_Object **data, const Eolian_Object **data2)
{
   return database_doc_token_ref_resolve(tok, (const Eolian_Unit *)state, NULL, data, data2);
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
   unit->objects    = eina_hash_stringshared_new(NULL);
}

static void
_unit_contents_del(Eolian_Unit *unit)
{
   eina_stringshare_del(unit->file);
   eina_hash_free(unit->children);
   eina_hash_free(unit->classes);
   eina_hash_free(unit->globals);
   eina_hash_free(unit->constants);
   eina_hash_free(unit->aliases);
   eina_hash_free(unit->structs);
   eina_hash_free(unit->enums);
   eina_hash_free(unit->objects);
}

void
database_unit_del(Eolian_Unit *unit)
{
   if (!unit)
     return;

   _unit_contents_del(unit);
   free(unit);
}

static Eina_Bool
_hashlist_free_cb(const Eina_Hash *hash EINA_UNUSED,
                  const void *key EINA_UNUSED,
                  void *data, void *fdata EINA_UNUSED)
{
   eina_list_free((Eina_List *)data);
   return EINA_TRUE;
}

static void
_hashlist_free(Eina_Hash *h)
{
   eina_hash_foreach(h, _hashlist_free_cb, NULL);
   eina_hash_free(h);
}

static void
_hashlist_free_buckets(Eina_Hash *h)
{
   eina_hash_foreach(h, _hashlist_free_cb, NULL);
   eina_hash_free_buckets(h);
}

static void
_state_area_init(Eolian_State *state, Eolian_State_Area *a)
{
   database_unit_init(state, &a->unit, NULL);

   a->units = eina_hash_stringshared_new(NULL);

   a->classes_f   = eina_hash_stringshared_new(NULL);
   a->aliases_f   = eina_hash_stringshared_new(NULL);
   a->structs_f   = eina_hash_stringshared_new(NULL);
   a->enums_f     = eina_hash_stringshared_new(NULL);
   a->globals_f   = eina_hash_stringshared_new(NULL);
   a->constants_f = eina_hash_stringshared_new(NULL);
   a->objects_f   = eina_hash_stringshared_new(NULL);
}

static Eina_Bool
_ulist_free_cb(const Eina_Hash *hash EINA_UNUSED,
               const void *key EINA_UNUSED,
               void *data, void *fdata EINA_UNUSED)
{
   database_unit_del((Eolian_Unit *)data);
   return EINA_TRUE;
}

static void
_state_area_contents_del(Eolian_State_Area *a)
{
   _unit_contents_del(&a->unit);

   eina_hash_foreach(a->units, _ulist_free_cb, NULL);
   eina_hash_free(a->units);

   eina_hash_free(a->classes_f);
   _hashlist_free(a->aliases_f);
   _hashlist_free(a->structs_f);
   _hashlist_free(a->enums_f);
   _hashlist_free(a->globals_f);
   _hashlist_free(a->constants_f);
   _hashlist_free(a->objects_f);
}

static void
_default_panic_cb(const Eolian_State *state EINA_UNUSED, const char *msg)
{
   _eolian_log(msg);
}

static void
_default_error_cb(const Eolian_Object *obj, const char *msg, void *data EINA_UNUSED)
{
   if (obj)
     _eolian_log_line(obj->file, obj->line, obj->column, msg);
   else
     _eolian_log(msg);
}

EAPI Eolian_State *
eolian_state_new(void)
{
   Eolian_State *state = calloc(1, sizeof(Eolian_State));
   if (!state)
     return NULL;

   state->panic = _default_panic_cb;

   if (setjmp(state->jmp_env))
     {
        state->panic(state, state->panic_msg);
        eina_stringshare_del(state->panic_msg);
        exit(EXIT_FAILURE);
     }

   state->error = _default_error_cb;

   _state_area_init(state, &state->main);
   _state_area_init(state, &state->staging);

   state->filenames_eo  = eina_hash_string_small_new(free);
   state->filenames_eot = eina_hash_string_small_new(free);

   state->defer = eina_hash_string_small_new(NULL);

   return state;
}

EAPI void
eolian_state_free(Eolian_State *state)
{
   if (!state)
     return;

   _state_area_contents_del(&state->main);
   _state_area_contents_del(&state->staging);

   eina_hash_free(state->filenames_eo);
   eina_hash_free(state->filenames_eot);

   eina_hash_free(state->defer);

   free(state);
}

EAPI Eolian_Panic_Cb
eolian_state_panic_cb_set(Eolian_State *state, Eolian_Panic_Cb cb)
{
   Eolian_Panic_Cb old_cb = state->panic;
   state->panic = cb ? cb : _default_panic_cb;
   return old_cb;
}

EAPI Eolian_Error_Cb
eolian_state_error_cb_set(Eolian_State *state, Eolian_Error_Cb cb)
{
   Eolian_Error_Cb old_cb = state->error;
   state->error = cb ? cb : _default_error_cb;
   return old_cb;
}

EAPI void *
eolian_state_error_data_set(Eolian_State *state, void *data)
{
   void *old_data = state->error_data;
   state->error_data = data;
   return old_data;
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
   ret = eolian_state_directory_add(state, eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);
   return ret;
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

static Eolian_Unit *
_eolian_file_parse_nodep(Eolian_Unit *parent, const char *filepath)
{
   Eina_Bool is_eo;
   const char *eopath;

   is_eo = eina_str_has_suffix(filepath, EO_SUFFIX);
   if (!is_eo && !eina_str_has_suffix(filepath, EOT_SUFFIX))
     {
        eolian_state_log(parent->state,
                         "file '%s' doesn't have a correct extension",
                         filepath);
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

static void
_state_clean(Eolian_State *state)
{
   eina_hash_free_buckets(state->defer);

   Eolian_State_Area *st = &state->staging;

   Eolian_Unit *stu = &st->unit;
   eina_hash_free_buckets(stu->classes);
   eina_hash_free_buckets(stu->globals);
   eina_hash_free_buckets(stu->constants);
   eina_hash_free_buckets(stu->aliases);
   eina_hash_free_buckets(stu->structs);
   eina_hash_free_buckets(stu->enums);
   eina_hash_free_buckets(stu->objects);

   eina_hash_foreach(st->units, _ulist_free_cb, NULL);
   eina_hash_free_buckets(st->units);

   eina_hash_free_buckets(st->classes_f);

   _hashlist_free_buckets(st->aliases_f);
   _hashlist_free_buckets(st->structs_f);
   _hashlist_free_buckets(st->enums_f);
   _hashlist_free_buckets(st->globals_f);
   _hashlist_free_buckets(st->constants_f);
   _hashlist_free_buckets(st->objects_f);
}

void
database_defer(Eolian_State *state, const char *fname, Eina_Bool isdep)
{
   void *nval = (void *)((size_t)isdep + 1);
   size_t found = (size_t)eina_hash_find(state->defer, fname);
   /* add if not found or upgrade to dep if requested */
   if (!found)
     eina_hash_add(state->defer, fname, nval);
   else if ((found <= 1) && isdep)
     eina_hash_set(state->defer, fname, nval);
}

static Eina_Bool _parse_deferred(Eolian_Unit *parent);
static void _merge_units(Eolian_Unit *unit);

typedef struct _Defer_Data
{
   Eolian_Unit *parent;
   Eina_Bool succ;
} Defer_Data;

static Eina_Bool
_defer_hash_cb(const Eina_Hash *hash EINA_UNUSED, const void *key,
               void *data, void *fdata)
{
   Defer_Data *d = fdata;
   Eina_Bool alone = ((size_t)data <= 1);
   Eolian_Unit *parent = d->parent;
   /* not a dependency; parse standalone */
   if (alone)
     parent = &parent->state->staging.unit;
   Eolian_Unit *pdep = _eolian_file_parse_nodep(parent, key);
   d->succ = (pdep && _parse_deferred(pdep));
   /* standalone-parsed stuff forms its own dependency trees,
    * so we have to merge the units manually and separately
    */
   if (d->succ && alone)
     _merge_units(pdep);
   return d->succ;
}

static Eina_Bool
_parse_deferred(Eolian_Unit *parent)
{
   Eina_Hash *defer = parent->state->defer;
   if (!eina_hash_population(defer))
     return EINA_TRUE;
   /* clean room for more deps for later parsing */
   parent->state->defer = eina_hash_string_small_new(NULL);
   Defer_Data d = { parent, EINA_FALSE };
   eina_hash_foreach(defer, _defer_hash_cb, &d);
   if (!d.succ)
     eina_hash_free_buckets(parent->state->defer);
   eina_hash_free(defer);
   return d.succ;
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

static Eina_Bool
_merge_unit_cb_noref(const Eina_Hash *hash EINA_UNUSED,
               const void *key, void *data, void *fdata)
{
   Eina_Hash *dest = fdata;
   if (!eina_hash_find(dest, key))
     eina_hash_add(dest, key, data);
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
   eina_hash_foreach(src->objects, _merge_unit_cb_noref, dest->objects);
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

static Eina_Bool
_merge_staging_cb(const Eina_Hash *hash EINA_UNUSED,
                  const void *key, void *data, void *fdata)
{
   eina_hash_add((Eina_Hash *)fdata, key, data);
   return EINA_TRUE;
}

static void
_merge_staging(Eolian_State *state)
{
   Eolian_State_Area *amain = &state->main, *staging = &state->staging;
   _merge_unit(&amain->unit, &staging->unit);

   eina_hash_foreach(staging->units, _merge_staging_cb, amain->units);
   eina_hash_free_buckets(staging->units);

#define EOLIAN_STAGING_MERGE_LIST(name) \
   eina_hash_foreach(staging->name##_f, _merge_staging_cb, amain->name##_f); \
   eina_hash_free_buckets(staging->name##_f);

   EOLIAN_STAGING_MERGE_LIST(classes);
   EOLIAN_STAGING_MERGE_LIST(aliases);
   EOLIAN_STAGING_MERGE_LIST(structs);
   EOLIAN_STAGING_MERGE_LIST(enums);
   EOLIAN_STAGING_MERGE_LIST(globals);
   EOLIAN_STAGING_MERGE_LIST(constants);
   EOLIAN_STAGING_MERGE_LIST(objects);

#undef EOLIAN_STAGING_MERGE_LIST

   _state_clean(state);
}

EAPI const Eolian_Unit *
eolian_state_file_parse(Eolian_State *state, const char *filepath)
{
   if (!state)
     return NULL;

   _state_clean(state);
   Eolian_Unit *ret = _eolian_file_parse_nodep(&state->staging.unit, filepath);
   if (!ret)
     return NULL;
   if (!_parse_deferred(ret))
     return NULL;
   _merge_units(ret);
   if (!database_validate(&state->staging.unit))
     return NULL;
   _merge_staging(state);
   return ret;
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
     unit = eo_parser_database_fill(&pd->state->staging.unit, data, EINA_TRUE);
   pd->ret = !!unit;
   if (pd->ret) pd->ret = _parse_deferred(unit);
   if (pd->ret) _merge_units(unit);
   return pd->ret;
}

EAPI Eina_Bool
eolian_state_all_eot_files_parse(Eolian_State *state)
{
   Parse_Data pd = { state, EINA_TRUE };

   if (!state)
     return EINA_FALSE;

   _state_clean(state);
   eina_hash_foreach(state->filenames_eot, _tfile_parse, &pd);

   if (pd.ret && !database_validate(&state->staging.unit))
     return EINA_FALSE;

   _merge_staging(state);

   return pd.ret;
}

static Eina_Bool _file_parse(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED, void *data, void *fdata)
{
   Parse_Data *pd = fdata;
   Eolian_Unit *unit = NULL;
   if (pd->ret)
     unit = eo_parser_database_fill(&pd->state->staging.unit, data, EINA_FALSE);
   pd->ret = !!unit;
   if (pd->ret) pd->ret = _parse_deferred(unit);
   if (pd->ret) _merge_units(unit);
   return pd->ret;
}

EAPI Eina_Bool
eolian_state_all_eo_files_parse(Eolian_State *state)
{
   Parse_Data pd = { state, EINA_TRUE };

   if (!state)
     return EINA_FALSE;

   _state_clean(state);
   eina_hash_foreach(state->filenames_eo, _file_parse, &pd);

   if (pd.ret && !database_validate(&state->staging.unit))
     return EINA_FALSE;

   _merge_staging(state);

   return pd.ret;
}

EAPI Eina_Bool
eolian_state_check(const Eolian_State *state)
{
   return database_check(state);
}

EAPI const Eolian_Unit *
eolian_state_unit_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eolian_Unit *unit = eina_hash_find(state->main.units, shr);
   eina_stringshare_del(shr);
   return unit;
}

EAPI Eina_Iterator *
eolian_state_units_get(const Eolian_State *state)
{
   if (!state) return NULL;
   return eina_hash_iterator_data_new(state->main.units);
}

EAPI Eina_Iterator *
eolian_state_objects_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eina_List *l = eina_hash_find(state->main.objects_f, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI const Eolian_Class *
eolian_state_class_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eolian_Class *cl = eina_hash_find(state->main.classes_f, shr);
   eina_stringshare_del(shr);
   return cl;
}

EAPI Eina_Iterator *
eolian_state_globals_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eina_List *l = eina_hash_find(state->main.globals_f, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_state_constants_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eina_List *l = eina_hash_find(state->main.constants_f, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_state_aliases_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eina_List *l = eina_hash_find(state->main.aliases_f, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_state_structs_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eina_List *l = eina_hash_find(state->main.structs_f, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_state_enums_by_file_get(const Eolian_State *state, const char *file_name)
{
   if (!state) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(file_name);
   Eina_List *l = eina_hash_find(state->main.enums_f, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI const Eolian_State *
eolian_unit_state_get(const Eolian_Unit *unit)
{
   if (!unit) return NULL;
   return unit->state;
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

EAPI const char *
eolian_unit_file_path_get(const Eolian_Unit *unit)
{
   if (!unit || !unit->file) return NULL;
   Eina_Bool is_eo = eina_str_has_suffix(unit->file, EO_SUFFIX);
   return eina_hash_find(is_eo
     ? unit->state->filenames_eo
     : unit->state->filenames_eot, unit->file);
}

EAPI const Eolian_Object *
eolian_unit_object_by_name_get(const Eolian_Unit *unit, const char *name)
{
   if (!unit) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Object *o = eina_hash_find(unit->objects, shr);
   eina_stringshare_del(shr);
   return o;
}

EAPI Eina_Iterator *eolian_unit_objects_get(const Eolian_Unit *unit)
{
   return (unit ? eina_hash_iterator_data_new(unit->objects) : NULL);
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

EAPI const Eolian_Typedecl *
eolian_unit_alias_by_name_get(const Eolian_Unit *unit, const char *name)
{
   if (!unit) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Typedecl *tp = eina_hash_find(unit->aliases, shr);
   eina_stringshare_del(shr);
   if (!tp) return NULL;
   return tp;
}

EAPI const Eolian_Typedecl *
eolian_unit_struct_by_name_get(const Eolian_Unit *unit, const char *name)
{
   if (!unit) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Typedecl *tp = eina_hash_find(unit->structs, shr);
   eina_stringshare_del(shr);
   if (!tp) return NULL;
   return tp;
}

EAPI const Eolian_Typedecl *
eolian_unit_enum_by_name_get(const Eolian_Unit *unit, const char *name)
{
   if (!unit) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Typedecl *tp = eina_hash_find(unit->enums, shr);
   eina_stringshare_del(shr);
   if (!tp) return NULL;
   return tp;
}

EAPI Eina_Iterator *
eolian_unit_aliases_get(const Eolian_Unit *unit)
{
   return (unit ? eina_hash_iterator_data_new(unit->aliases) : NULL);
}

EAPI Eina_Iterator *
eolian_unit_structs_get(const Eolian_Unit *unit)
{
   return (unit ? eina_hash_iterator_data_new(unit->structs) : NULL);
}

EAPI Eina_Iterator *
eolian_unit_enums_get(const Eolian_Unit *unit)
{
   return (unit ? eina_hash_iterator_data_new(unit->enums) : NULL);
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
