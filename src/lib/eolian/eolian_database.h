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

typedef struct _Eolian_Object
{
   const char *file;
   int line;
   int column;
} Eolian_Object;

struct _Eolian_Class
{
   Eolian_Object base;
   Eina_Stringshare *full_name;
   Eina_List *namespaces; /* List Eina_Stringshare * */
   Eina_Stringshare *name;
   Eolian_Class_Type type;
   Eina_Stringshare *description;
   Eina_Stringshare *legacy_prefix;
   Eina_Stringshare *eo_prefix;
   Eina_Stringshare *data_type;
   Eina_List *inherits; /* List Eina_Stringshare * */
   Eina_List *properties; /* List prop_name -> Eolian_Function */
   Eina_List *methods; /* List meth_name -> Eolian_Function */
   Eina_List *constructors; /* List constructor_name -> Eolian_Function */
   Eina_List *implements; /* List implements name -> Eolian_Implement */
   Eina_List *events; /* List event_name -> Eolian_Event */
   Eina_Bool class_ctor_enable:1;
   Eina_Bool class_dtor_enable:1;
};

struct _Eolian_Function
{
   Eolian_Object base;
   Eolian_Object set_base;
   Eina_Stringshare *name;
   Eina_List *keys; /* list of Eolian_Function_Parameter */
   Eina_List *params; /* list of Eolian_Function_Parameter */
   Eolian_Function_Type type;
   Eolian_Object_Scope scope;
   Eolian_Type *get_ret_type;
   Eolian_Type *set_ret_type;
   Eolian_Expression *get_ret_val;
   Eolian_Expression *set_ret_val;
   Eina_Stringshare *get_legacy;
   Eina_Stringshare *set_legacy;
   Eina_Stringshare *get_description;
   Eina_Stringshare *set_description;
   Eina_Stringshare *get_return_comment;
   Eina_Stringshare *set_return_comment;
   Eina_Bool obj_is_const :1; /* True if the object has to be const. Useful for a few methods. */
   Eina_Bool get_virtual_pure :1;
   Eina_Bool set_virtual_pure :1;
   Eina_Bool get_return_warn_unused :1; /* also used for methods */
   Eina_Bool set_return_warn_unused :1;
   Eina_Bool is_class :1;
};

struct _Eolian_Function_Parameter
{
   Eolian_Object base;
   Eina_Stringshare *name;
   Eolian_Type *type;
   Eina_Stringshare *description;
   Eolian_Parameter_Dir param_dir;
   Eina_Bool is_const_on_get :1; /* True if const in this the get property */
   Eina_Bool is_const_on_set :1; /* True if const in this the set property */
   Eina_Bool nonull :1; /* True if this argument cannot be NULL */
};

struct _Eolian_Type
{
   Eolian_Object base;
   Eolian_Type_Type type;
   union {
      /* functions */
      struct {
         Eina_List   *arguments;
         Eolian_Type *ret_type;
      };
      /* everything else */
      struct {
         Eina_List   *subtypes;
         Eolian_Type *base_type;
         Eina_Stringshare *name;
         Eina_Stringshare *full_name;
         Eina_List        *namespaces;
         Eina_Hash        *fields;
         Eina_List        *field_names;
         Eina_Stringshare *comment;
         Eina_Stringshare *legacy;
      };
   };
   Eina_Bool is_const  :1;
   Eina_Bool is_own    :1;
   Eina_Bool is_extern :1;
};

struct _Eolian_Implement
{
   Eolian_Object base;
   const Eolian_Class *klass;
   Eina_Stringshare *full_name;
};

struct _Eolian_Event
{
   Eolian_Object base;
   Eina_Stringshare *name;
   Eina_Stringshare *comment;
   Eolian_Type *type;
   int scope;
};

typedef struct _Eolian_Struct_Field
{
   Eolian_Object     base;
   Eolian_Type      *type;
   Eina_Stringshare *comment;
} Eolian_Struct_Field;

typedef struct _Eolian_Enum_Field
{
   Eolian_Object      base;
   Eolian_Expression *value;
   Eina_Stringshare  *comment;
} Eolian_Enum_Field;

typedef union
{
   char               c;
   Eina_Bool          b;
   const    char     *s;
   signed   int       i;
   unsigned int       u;
   signed   long      l;
   unsigned long      ul;
   signed   long long ll;
   unsigned long long ull;
   float              f;
   double             d;
   long double        ld;
} Eolian_Value;

typedef enum
{
   EOLIAN_BINOP_INVALID = -1,

   EOLIAN_BINOP_ADD, /* + int, float */
   EOLIAN_BINOP_SUB, /* - int, float */
   EOLIAN_BINOP_MUL, /* * int, float */
   EOLIAN_BINOP_DIV, /* / int, float */
   EOLIAN_BINOP_MOD, /* % int */

   EOLIAN_BINOP_EQ, /* == all types */
   EOLIAN_BINOP_NQ, /* != all types */
   EOLIAN_BINOP_GT, /* >  int, float */
   EOLIAN_BINOP_LT, /* <  int, float */
   EOLIAN_BINOP_GE, /* >= int, float */
   EOLIAN_BINOP_LE, /* <= int, float */

   EOLIAN_BINOP_AND, /* && all types */
   EOLIAN_BINOP_OR,  /* || all types */

   EOLIAN_BINOP_BAND, /* &  int */
   EOLIAN_BINOP_BOR,  /* |  int */
   EOLIAN_BINOP_BXOR, /* ^  int */
   EOLIAN_BINOP_LSH,  /* << int */
   EOLIAN_BINOP_RSH   /* >> int */
} Eolian_Binary_Operator;

typedef enum
{
   EOLIAN_UNOP_INVALID = -1,

   EOLIAN_UNOP_UNM, /* - sint */
   EOLIAN_UNOP_UNP, /* + sint */

   EOLIAN_UNOP_NOT,  /* ! int, float, bool */
   EOLIAN_UNOP_BNOT, /* ~ int */
} Eolian_Unary_Operator;

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
      Eolian_Value value;
   };
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
   Eina_Stringshare     *comment;
   Eina_Bool is_extern :1;
};

int database_init();
int database_shutdown();

/* types */

Eina_Bool database_type_add(Eolian_Type *def);
Eina_Bool database_struct_add(Eolian_Type *tp);
Eina_Bool database_enum_add(Eolian_Type *tp);
void database_type_del(Eolian_Type *tp);
void database_typedef_del(Eolian_Type *tp);

void database_type_print(Eolian_Type *type);
void database_type_to_str(const Eolian_Type *tp, Eina_Strbuf *buf, const char *name);

/* expressions */

Eolian_Expression_Type database_expr_eval(const Eolian_Expression *expr, Eolian_Expression_Mask mask, Eina_Value *out);
void database_expr_del(Eolian_Expression *expr);
void database_expr_print(Eolian_Expression *expr);

/* variables */

void database_var_del(Eolian_Variable *var);
Eina_Bool database_var_add(Eolian_Variable *var);

/* classes */

Eolian_Class *database_class_add(const char *class_name, Eolian_Class_Type type);
void database_class_del(Eolian_Class *cl);

char *database_class_to_filename(const char *cname);

Eina_Bool database_class_inherit_add(Eolian_Class *cl, const char *inherit_class_name);
Eina_Bool database_class_function_add(Eolian_Class *cl, Eolian_Function *foo_id);
Eina_Bool database_class_implement_add(Eolian_Class *cl, Eolian_Implement *impl_id);
Eina_Bool database_class_event_add(Eolian_Class *cl, Eolian_Event *event_desc);

void database_class_description_set(Eolian_Class *cl, const char *description);
void database_class_legacy_prefix_set(Eolian_Class *cl, const char *legacy_prefix);
void database_class_eo_prefix_set(Eolian_Class *cl, const char *eo_prefix);
void database_class_data_type_set(Eolian_Class *cl, const char *data_type);
Eina_Bool database_class_ctor_enable_set(Eolian_Class *cl, Eina_Bool enable);
Eina_Bool database_class_dtor_enable_set(Eolian_Class *cl, Eina_Bool enable);

Eina_Bool database_class_name_validate(const char *class_name, const Eolian_Class **cl);

/* functions */

Eolian_Function *database_function_new(const char *function_name, Eolian_Function_Type foo_type);
void database_function_del(Eolian_Function *fid);

void database_function_type_set(Eolian_Function *function_id, Eolian_Function_Type foo_type);
void database_function_legacy_set(Eolian_Function *fid, Eolian_Function_Type ftype, const char *legacy);
void database_function_description_set(Eolian_Function *fid, Eolian_Function_Type ftype, const char *desc);
void database_function_return_type_set(Eolian_Function *foo_id, Eolian_Function_Type ftype, Eolian_Type *ret_type);
void database_function_return_comment_set(Eolian_Function *foo_id, Eolian_Function_Type ftype, const char *ret_comment);
void database_function_return_default_val_set(Eolian_Function *foo_id, Eolian_Function_Type ftype, Eolian_Expression *ret_default_value);
void database_function_return_flag_set_as_warn_unused(Eolian_Function *foo_id, Eolian_Function_Type ftype, Eina_Bool warn_unused);

void database_function_object_set_as_const(Eolian_Function *foo_id, Eina_Bool is_const);
void database_function_set_as_class(Eolian_Function *foo_id, Eina_Bool is_class);
Eina_Bool database_function_set_as_virtual_pure(Eolian_Function *function_id, Eolian_Function_Type type);
void database_function_scope_set(Eolian_Function *function_id, Eolian_Object_Scope scope);

Eolian_Function_Parameter *database_property_key_add(Eolian_Function *foo_id, Eolian_Type *type, const char *name, const char *description);
Eolian_Function_Parameter *database_property_value_add(Eolian_Function *foo_id, Eolian_Type *type, const char *name, const char *description);
Eolian_Function_Parameter *database_method_parameter_add(Eolian_Function *foo_id, Eolian_Parameter_Dir param_dir, Eolian_Type *type, const char *name, const char *description);

/* func parameters */

Eolian_Function_Parameter *database_parameter_add(Eolian_Type *type, const char *name, const char *description);
void database_parameter_del(Eolian_Function_Parameter *pdesc);

void database_parameter_const_attribute_set(Eolian_Function_Parameter *param, Eina_Bool is_get, Eina_Bool is_const);
void database_parameter_nonull_set(Eolian_Function_Parameter *param, Eina_Bool nonull);

/* implements */

void database_implement_del(Eolian_Implement *impl);

/* events */

Eolian_Event *database_event_new(const char *event_name, const char *event_type, const char *event_desc);
void database_event_del(Eolian_Event *event);


#endif
