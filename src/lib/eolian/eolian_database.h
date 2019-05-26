#ifndef __EOLIAN_DATABASE_H
#define __EOLIAN_DATABASE_H

#include <setjmp.h>

#include <Eolian.h>

extern int _eolian_log_dom;
extern Eina_Prefix *_eolian_prefix;

#ifdef CRI
#undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_eolian_log_dom, __VA_ARGS__)

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eolian_log_dom, __VA_ARGS__)

#ifdef WRN
#undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_eolian_log_dom, __VA_ARGS__)

#ifdef INF
#undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_eolian_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eolian_log_dom, __VA_ARGS__)

struct _Eolian_Unit
{
   const char    *file;
   Eolian_State  *state;
   Eina_Hash     *children;
   Eina_Hash     *classes;
   Eina_Hash     *globals;
   Eina_Hash     *constants;
   Eina_Hash     *aliases;
   Eina_Hash     *structs;
   Eina_Hash     *enums;
   Eina_Hash     *objects;
   unsigned short version;
};

typedef struct _Eolian_State_Area
{
   Eolian_Unit unit;

   Eina_Hash *units;

   Eina_Hash *classes_f;
   Eina_Hash *aliases_f;
   Eina_Hash *structs_f;
   Eina_Hash *enums_f;
   Eina_Hash *globals_f;
   Eina_Hash *constants_f;
   Eina_Hash *objects_f;
} Eolian_State_Area;

struct _Eolian_State
{
   Eolian_State_Area main;
   Eolian_State_Area staging;

   Eolian_Panic_Cb panic;
   Eina_Stringshare *panic_msg;
   jmp_buf jmp_env;

   Eolian_Error_Cb error;
   void *error_data;

   Eina_Hash *filenames_eo; /* filename to full path mapping */
   Eina_Hash *filenames_eot;

   Eina_Hash *defer;
};

struct _Eolian_Object
{
   Eolian_Unit *unit;
   Eina_Stringshare *file;
   Eina_Stringshare *name;
   Eina_Stringshare *c_name;
   int line;
   int column;
   int refcount;
   Eolian_Object_Type type;
   Eina_Bool validated: 1;
   Eina_Bool is_beta: 1;
};

static inline void
eolian_object_ref(Eolian_Object *obj)
{
   ++obj->refcount;
}

static inline Eina_Bool
eolian_object_unref(Eolian_Object *obj)
{
   return (--obj->refcount > 0);
}

static inline void
eolian_object_add(Eolian_Object *obj, Eina_Stringshare *name, Eina_Hash *hash)
{
   eina_hash_add(hash, name, obj);
   eolian_object_ref(obj);
}

#define EOLIAN_OBJECT_ADD(tunit, name, obj, memb) \
{ \
   eolian_object_add(&obj->base, name, tunit->state->staging.unit.memb); \
   eolian_object_add(&obj->base, name, tunit->memb); \
}

static inline void eolian_state_vlog(const Eolian_State *state, const Eolian_Object *obj, const char *fmt, va_list args) EINA_ARG_NONNULL(1, 3);
static inline void eolian_state_log(const Eolian_State *state, const char *fmt, ...) EINA_ARG_NONNULL(1, 2) EINA_PRINTF(2, 3);
static inline void eolian_state_log_obj(const Eolian_State *state, const Eolian_Object *obj, const char *fmt, ...) EINA_ARG_NONNULL(1, 2, 3) EINA_PRINTF(3, 4);

static inline void eolian_state_panic(Eolian_State *state, const char *fmt, ...) EINA_ARG_NONNULL(1, 2) EINA_PRINTF(2, 3);

static inline void
eolian_state_vlog(const Eolian_State *state, const Eolian_Object *obj,
                 const char *fmt, va_list args)
{
   Eina_Strbuf *sb = eina_strbuf_new();
   eina_strbuf_append_vprintf(sb, fmt, args);
   state->error(obj, eina_strbuf_string_get(sb), state->error_data);
   eina_strbuf_free(sb);
}

static inline void
eolian_state_log(const Eolian_State *state, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   eolian_state_vlog(state, NULL, fmt, args);
   va_end(args);
}

static inline void
eolian_state_log_obj(const Eolian_State *state, const Eolian_Object *obj,
                     const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   eolian_state_vlog(state, obj, fmt, args);
   va_end(args);
}

static inline void
eolian_state_panic(Eolian_State *state, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   state->panic_msg = eina_stringshare_vprintf(fmt, args);
   va_end(args);
   longjmp(state->jmp_env, 1);
}

struct _Eolian_Documentation
{
   Eolian_Object base;
   Eina_Stringshare *summary;
   Eina_Stringshare *description;
   Eina_Stringshare *since;
   Eina_List *ref_dbg;
};

struct _Eolian_Class
{
   Eolian_Object base;
   Eolian_Class_Type type;
   Eolian_Documentation *doc;
   Eina_Stringshare *c_prefix;
   Eina_Stringshare *ev_prefix;
   Eina_Stringshare *data_type;
   union {
      Eolian_Class *parent;
      Eina_Stringshare *parent_name;
   };
   Eina_List *extends; /* Eolian_Class */
   Eina_List *properties; /* Eolian_Function */
   Eina_List *methods; /* Eolian_Function */
   Eina_List *implements; /* Eolian_Implement */
   Eina_List *constructors; /* Eolian_Constructor */
   Eina_List *events; /* Eolian_Event */
   Eina_List *parts; /* Eolian_Part */
   Eina_List *composite; /* Eolian_Class */
   Eina_List *requires; /* a list of required other classes only used internally */
   Eina_List *callables; /* internal for now */
   Eina_Bool class_ctor_enable:1;
   Eina_Bool class_dtor_enable:1;
};

struct _Eolian_Function
{
   Eolian_Object base;
   Eolian_Object set_base;
   union { /* lists of Eolian_Function_Parameter */
       Eina_List *params;
       struct {
           Eina_List *prop_values;
           Eina_List *prop_values_get;
           Eina_List *prop_values_set;
           Eina_List *prop_keys;
           Eina_List *prop_keys_get;
           Eina_List *prop_keys_set;
       };
   };
   Eolian_Function_Type type;
   Eolian_Object_Scope get_scope;
   Eolian_Object_Scope set_scope;
   Eolian_Type *get_ret_type;
   Eolian_Type *set_ret_type;
   Eolian_Expression *get_ret_val;
   Eolian_Expression *set_ret_val;
   Eolian_Implement *impl;
   Eolian_Documentation *get_return_doc;
   Eolian_Documentation *set_return_doc;
   Eina_Bool obj_is_const :1; /* True if the object has to be const. Useful for a few methods. */
   Eina_Bool get_return_no_unused :1; /* also used for methods */
   Eina_Bool set_return_no_unused :1;
   Eina_Bool is_class :1;
   Eina_List *ctor_of;
   Eolian_Class *klass;
};

struct _Eolian_Part
{
   Eolian_Object base;
   /* when not validated, class name is stored */
   union
   {
      Eina_Stringshare *klass_name;
      Eolian_Class *klass;
   };
   Eolian_Documentation *doc;
};

struct _Eolian_Function_Parameter
{
   Eolian_Object base;
   Eolian_Type *type;
   Eolian_Expression *value;
   Eolian_Documentation *doc;
   Eolian_Parameter_Dir param_dir;
   Eina_Bool optional :1; /* True if this argument is optional */
};

struct _Eolian_Type
{
   Eolian_Object base;
   Eolian_Type_Type type;
   Eolian_Type_Builtin_Type btype;
   Eolian_Type *base_type;
   Eolian_Type *next_type;
   Eina_Stringshare *freefunc;
   union
   {
      Eolian_Class *klass;
      Eolian_Typedecl *tdecl;
   };
   Eina_Bool is_const  :1;
   Eina_Bool is_ptr    :1;
   Eina_Bool owned     :1;
};

struct _Eolian_Typedecl
{
   Eolian_Object base;
   Eolian_Typedecl_Type type;
   Eolian_Type      *base_type;
   Eina_Hash        *fields;
   Eina_List        *field_list;
   Eolian_Function *function_pointer;
   Eolian_Documentation *doc;
   Eina_Stringshare *legacy;
   Eina_Stringshare *freefunc;
   Eina_Bool is_extern :1;
};

struct _Eolian_Implement
{
   Eolian_Object base;
   const Eolian_Class *klass;
   const Eolian_Class *implklass;
   const Eolian_Function *foo_id;
   Eolian_Documentation *common_doc;
   Eolian_Documentation *get_doc;
   Eolian_Documentation *set_doc;
   Eina_Bool is_prop_get :1;
   Eina_Bool is_prop_set :1;
   Eina_Bool get_pure_virtual :1;
   Eina_Bool set_pure_virtual :1;
   Eina_Bool get_auto: 1;
   Eina_Bool set_auto: 1;
   Eina_Bool get_empty: 1;
   Eina_Bool set_empty: 1;
};

struct _Eolian_Constructor
{
   Eolian_Object base;
   const Eolian_Class *klass;
   Eina_Bool is_optional: 1;
   Eina_Bool is_ctor_param : 1;
};

struct _Eolian_Event
{
   Eolian_Object base;
   Eolian_Documentation *doc;
   Eolian_Type *type;
   Eolian_Class *klass;
   Eolian_Object_Scope scope;
   Eina_Bool is_hot  :1;
   Eina_Bool is_restart :1;
};

struct _Eolian_Struct_Type_Field
{
   Eolian_Object     base;
   Eolian_Type      *type;
   Eolian_Documentation *doc;
};

struct _Eolian_Enum_Type_Field
{
   Eolian_Object      base;
   Eolian_Typedecl   *base_enum;
   Eolian_Expression *value;
   Eolian_Documentation *doc;
   Eina_Bool is_public_value :1;
};

struct _Eolian_Expression
{
   Eolian_Object base;
   Eolian_Expression_Type type;
   union
   {
      struct
      {
         Eolian_Binary_Operator binop;
         Eolian_Expression *lhs;
         Eolian_Expression *rhs;
      };
      struct
      {
         union
         {
            Eolian_Unary_Operator unop;
            Eolian_Value_Union value;
         };
         Eolian_Expression *expr;
      };
   };
   Eina_Bool weak_lhs :1;
   Eina_Bool weak_rhs :1;
};

struct _Eolian_Variable
{
   Eolian_Object         base;
   Eolian_Variable_Type  type;
   Eolian_Type          *base_type;
   Eolian_Expression    *value;
   Eolian_Documentation *doc;
   Eina_Bool is_extern :1;
};

char *database_class_to_filename(const char *cname);
Eina_Bool database_validate(const Eolian_Unit *src);
Eina_Bool database_check(const Eolian_State *state);
/* if isdep is EINA_TRUE, parse as a dependency of current unit */
void database_defer(Eolian_State *state, const char *fname, Eina_Bool isdep);

void database_object_add(Eolian_Unit *unit, const Eolian_Object *obj);

void database_doc_del(Eolian_Documentation *doc);

void database_unit_init(Eolian_State *state, Eolian_Unit *unit, const char *file);
void database_unit_del(Eolian_Unit *unit);

Eolian_Object_Type database_doc_token_ref_resolve(const Eolian_Doc_Token *tok,
                                                  const Eolian_Unit *unit1,
                                                  const Eolian_Unit *unit2,
                                                  const Eolian_Object **data1,
                                                  const Eolian_Object **data2);

/* types */

void database_type_add(Eolian_Unit *unit, Eolian_Typedecl *tp);
void database_struct_add(Eolian_Unit *unit, Eolian_Typedecl *tp);
void database_enum_add(Eolian_Unit *unit, Eolian_Typedecl *tp);
void database_type_del(Eolian_Type *tp);
void database_typedecl_del(Eolian_Typedecl *tp);

void database_type_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name, Eolian_C_Type_Type ctype);
void database_typedecl_to_str(const Eolian_Typedecl *tp, Eina_Strbuf *buf);

Eolian_Typedecl *database_type_decl_find(const Eolian_Unit *src, const Eolian_Type *tp);

Eina_Bool database_type_is_ownable(const Eolian_Unit *unit, const Eolian_Type *tp, Eina_Bool allow_void);

/* expressions */

typedef void (*Expr_Obj_Cb)(const Eolian_Object *obj, void *data);

Eolian_Value database_expr_eval(const Eolian_Unit *unit, Eolian_Expression *expr, Eolian_Expression_Mask mask, Expr_Obj_Cb cb, void *data);
Eolian_Value database_expr_eval_type(const Eolian_Unit *unit, Eolian_Expression *expr, const Eolian_Type *type, Expr_Obj_Cb cb, void *data);
void database_expr_del(Eolian_Expression *expr);
void database_expr_print(Eolian_Expression *expr);

/* variables */

void database_var_del(Eolian_Variable *var);
void database_var_add(Eolian_Unit *unit, Eolian_Variable *var);

/* classes */
void database_class_del(Eolian_Class *cl);

/* functions */
void database_function_del(Eolian_Function *fid);
void database_function_constructor_add(Eolian_Function *func, const Eolian_Class *klass);
Eina_Bool database_function_is_type(Eolian_Function *fid, Eolian_Function_Type ftype);

/* func parameters */
void database_parameter_del(Eolian_Function_Parameter *pdesc);

/* implements */
void database_implement_del(Eolian_Implement *impl);

/* constructors */
void database_constructor_del(Eolian_Constructor *ctor);

/* events */
void database_event_del(Eolian_Event *event);

/* parts */
void database_part_del(Eolian_Part *part);

#endif
