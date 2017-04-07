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

extern Eina_Hash *_classes;
extern Eina_Hash *_aliases;
extern Eina_Hash *_structs;
extern Eina_Hash *_enums;
extern Eina_Hash *_globals;
extern Eina_Hash *_constants;
extern Eina_Hash *_classesf;
extern Eina_Hash *_aliasesf;
extern Eina_Hash *_structsf;
extern Eina_Hash *_enumsf;
extern Eina_Hash *_globalsf;
extern Eina_Hash *_constantsf;
extern Eina_Hash *_filenames; /* Hash: filename without extension -> full path */
extern Eina_Hash *_tfilenames;

/* a hash holding all declarations, for redef checking etc */
extern Eina_Hash *_decls;

/* holds parsed/currently parsing eot files to keep track */
extern Eina_Hash *_parsedeos;
extern Eina_Hash *_parsingeos;

/* for deferred dependency parsing */
extern Eina_Hash *_defereos;

typedef struct _Eolian_Object
{
   const char *file;
   int line;
   int column;
} Eolian_Object;

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
   Eina_List *inherits; /* List Eina_Stringshare * */
   Eina_List *properties; /* List prop_name -> Eolian_Function */
   Eina_List *methods; /* List meth_name -> Eolian_Function */
   Eina_List *implements; /* List implements name -> Eolian_Implement */
   Eina_List *constructors; /* List constructors name -> Eolian_Constructor */
   Eina_List *events; /* List event_name -> Eolian_Event */
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
   Eina_Bool is_c_only :1;
   Eina_Bool is_beta :1;
   Eina_List *ctor_of;
   Eolian_Class *klass;
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
   Eolian_Type *base_type;
   Eolian_Type *next_type;
   Eina_Stringshare *name;
   Eina_Stringshare *full_name;
   Eina_List        *namespaces;
   Eina_Stringshare *freefunc;
   size_t static_size;
   Eina_Bool is_const  :1;
   Eina_Bool is_own    :1;
   Eina_Bool is_ptr    :1;
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
   Eina_Stringshare *name;
   Eolian_Object     base;
   Eolian_Type      *type;
   Eolian_Documentation *doc;
};

struct _Eolian_Enum_Type_Field
{
   Eolian_Typedecl   *base_enum;
   Eina_Stringshare  *name;
   Eolian_Object      base;
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
         Eolian_Unary_Operator unop;
         Eolian_Expression *expr;
      };
      Eolian_Value_Union value;
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

struct _Eolian_Unit
{
   Eina_List     *dependencies;
   Eina_List     *classes;
   Eina_List     *globals;
   Eina_List     *constants;
   Eina_List     *aliases;
   Eina_List     *structs;
   Eina_List     *enums;
};

int database_init(void);
int database_shutdown(void);

char *database_class_to_filename(const char *cname);
Eina_Bool database_validate();

void database_decl_add(Eina_Stringshare *name, Eolian_Declaration_Type type,
                       Eina_Stringshare *file, void *ptr);

void database_doc_del(Eolian_Documentation *doc);

void database_unit_del(Eolian_Unit *unit);

/* types */

void database_type_add(Eolian_Typedecl *def);
void database_struct_add(Eolian_Typedecl *tp);
void database_enum_add(Eolian_Typedecl *tp);
void database_type_del(Eolian_Type *tp);
void database_typedecl_del(Eolian_Typedecl *tp);

void database_type_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name);
void database_typedecl_to_str(const Eolian_Unit *src, const Eolian_Typedecl *tp, Eina_Strbuf *buf);

/* expressions */

Eolian_Value database_expr_eval(const Eolian_Unit *unit, const Eolian_Expression *expr, Eolian_Expression_Mask mask);
void database_expr_del(Eolian_Expression *expr);
void database_expr_print(Eolian_Expression *expr);

/* variables */

void database_var_del(Eolian_Variable *var);
void database_var_add(Eolian_Variable *var);

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


#endif
