#ifndef __EOLIAN_DATABASE_H
#define __EOLIAN_DATABASE_H

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
   Eolian        *state;
   Eina_Hash     *children;
   Eina_Hash     *classes;
   Eina_Hash     *globals;
   Eina_Hash     *constants;
   Eina_Hash     *aliases;
   Eina_Hash     *structs;
   Eina_Hash     *enums;
   Eina_Hash     *decls;
};

struct _Eolian
{
   Eolian_Unit unit;

   Eina_Hash *filenames_eo; /* filename to full path mapping */
   Eina_Hash *filenames_eot;

   Eina_Hash *parsing;
   Eina_Hash *parsed;
   Eina_Hash *defer;

   Eina_Hash *units;

   Eina_Hash *classes_f;
   Eina_Hash *aliases_f;
   Eina_Hash *structs_f;
   Eina_Hash *enums_f;
   Eina_Hash *globals_f;
   Eina_Hash *constants_f;
   Eina_Hash *decls_f;
};

typedef struct _Eolian_Object
{
   const char *file;
   int line;
   int column;
   int refcount;
   Eina_Bool validated;
} Eolian_Object;

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

struct _Eolian_Documentation
{
   Eolian_Object base;
   Eina_Stringshare *summary;
   Eina_Stringshare *description;
   Eina_Stringshare *since;
};

struct _Eolian_Declaration
{
   Eolian_Declaration_Type type;
   Eina_Stringshare *name;
   void *data;
};

struct _Eolian_Class
{
   Eolian_Object base;
   Eina_Stringshare *full_name;
   Eina_List *namespaces; /* List Eina_Stringshare * */
   Eina_Stringshare *name;
   Eolian_Class_Type type;
   Eolian_Documentation *doc;
   Eina_Stringshare *legacy_prefix;
   Eina_Stringshare *eo_prefix;
   Eina_Stringshare *ev_prefix;
   Eina_Stringshare *data_type;
   Eina_List *inherits; /* Eolian_Class */
   Eina_List *properties; /* Eolian_Function */
   Eina_List *methods; /* Eolian_Function */
   Eina_List *implements; /* Eolian_Implement */
   Eina_List *constructors; /* Eolian_Constructor */
   Eina_List *events; /* Eolian_Event */
   Eina_List *parts; /* Eolian_Part */
   Eina_Bool class_ctor_enable:1;
   Eina_Bool class_dtor_enable:1;
};

struct _Eolian_Function
{
   Eolian_Object base;
   Eolian_Object set_base;
   Eina_Stringshare *name;
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
   Eina_Stringshare *get_legacy;
   Eina_Stringshare *set_legacy;
   Eolian_Documentation *get_return_doc;
   Eolian_Documentation *set_return_doc;
   Eina_Bool obj_is_const :1; /* True if the object has to be const. Useful for a few methods. */
   Eina_Bool get_return_warn_unused :1; /* also used for methods */
   Eina_Bool set_return_warn_unused :1;
   Eina_Bool get_only_legacy: 1;
   Eina_Bool set_only_legacy: 1;
   Eina_Bool is_class :1;
   Eina_Bool is_beta :1;
   Eina_List *ctor_of;
   Eolian_Class *klass;
};

struct _Eolian_Part
{
   Eolian_Object base;
   Eina_Stringshare *name;
   Eolian_Class *klass;
   Eolian_Documentation *doc;
};

struct _Eolian_Function_Parameter
{
   Eolian_Object base;
   Eina_Stringshare *name;
   Eolian_Type *type;
   Eolian_Expression *value;
   Eolian_Documentation *doc;
   Eolian_Parameter_Dir param_dir;
   Eina_Bool nonull :1; /* True if this argument cannot be NULL - deprecated */
   Eina_Bool nullable :1; /* True if this argument is nullable */
   Eina_Bool optional :1; /* True if this argument is optional */
};

struct _Eolian_Type
{
   Eolian_Object base;
   Eolian_Type_Type type;
   Eolian_Type_Builtin_Type btype;
   Eolian_Type *base_type;
   Eolian_Type *next_type;
   Eina_Stringshare *name;
   Eina_Stringshare *full_name;
   Eina_List        *namespaces;
   Eina_Stringshare *freefunc;
   union
   {
      Eolian_Class *klass;
      Eolian_Typedecl *tdecl;
   };
   Eina_Bool is_const  :1;
   Eina_Bool is_ptr    :1;
   Eina_Bool owned     :1;
   Eina_Bool legacy    :1;
};

struct _Eolian_Typedecl
{
   Eolian_Object base;
   Eolian_Typedecl_Type type;
   Eolian_Type      *base_type;
   Eina_Stringshare *name;
   Eina_Stringshare *full_name;
   Eina_List        *namespaces;
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
   const Eolian_Function *foo_id;
   Eina_Stringshare *full_name;
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
   Eina_Stringshare *full_name;
   Eina_Bool is_optional: 1;
};

struct _Eolian_Event
{
   Eolian_Object base;
   Eina_Stringshare *name;
   Eolian_Documentation *doc;
   Eolian_Type *type;
   Eolian_Class *klass;
   Eolian_Object_Scope scope;
   Eina_Bool is_beta :1;
   Eina_Bool is_hot  :1;
   Eina_Bool is_restart :1;
};

struct _Eolian_Struct_Type_Field
{
   Eolian_Object     base;
   Eina_Stringshare *name;
   Eolian_Type      *type;
   Eolian_Documentation *doc;
};

struct _Eolian_Enum_Type_Field
{
   Eolian_Object      base;
   Eolian_Typedecl   *base_enum;
   Eina_Stringshare  *name;
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
   Eina_Stringshare     *name;
   Eina_Stringshare     *full_name;
   Eina_List            *namespaces;
   Eolian_Type          *base_type;
   Eolian_Expression    *value;
   Eolian_Documentation *doc;
   Eina_Bool is_extern :1;
};

char *database_class_to_filename(const char *cname);
Eina_Bool database_validate(Eolian *state, const Eolian_Unit *src);

void database_decl_add(Eolian *state, Eina_Stringshare *name,
                       Eolian_Declaration_Type type,
                       Eina_Stringshare *file, void *ptr);

void database_doc_del(Eolian_Documentation *doc);

void database_unit_init(Eolian *state, Eolian_Unit *unit);
void database_unit_del(Eolian_Unit *unit);

/* types */

void database_type_add(Eolian *state, Eolian_Typedecl *def);
void database_struct_add(Eolian *state, Eolian_Typedecl *tp);
void database_enum_add(Eolian *state, Eolian_Typedecl *tp);
void database_type_del(Eolian_Type *tp);
void database_typedecl_del(Eolian_Typedecl *tp);

void database_type_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name, Eolian_C_Type_Type ctype);
void database_typedecl_to_str(const Eolian_Typedecl *tp, Eina_Strbuf *buf);

Eolian_Typedecl *database_type_decl_find(const Eolian_Unit *src, const Eolian_Type *tp);

Eina_Bool database_type_is_ownable(const Eolian_Unit *unit, const Eolian_Type *tp);

/* expressions */

Eolian_Value database_expr_eval(const Eolian_Unit *unit, Eolian_Expression *expr, Eolian_Expression_Mask mask);
Eolian_Value database_expr_eval_type(const Eolian_Unit *unit, Eolian_Expression *expr, const Eolian_Type *type);
void database_expr_del(Eolian_Expression *expr);
void database_expr_print(Eolian_Expression *expr);

/* variables */

void database_var_del(Eolian_Variable *var);
void database_var_add(Eolian *state, Eolian_Variable *var);

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
