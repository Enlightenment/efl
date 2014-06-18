#ifndef __EO_DEFINITIONS_H__
#define __EO_DEFINITIONS_H__

#include <Eina.h>
#include <Eolian.h>

/* RET */

typedef struct _eo_ret_def
{
   Eolian_Type type;
   const char *comment;
   const char *dflt_ret_val;
   Eina_Bool warn_unused:1;
} Eo_Ret_Def;

/* PARAM */

typedef enum _param_way
{
   PARAM_IN,
   PARAM_OUT,
   PARAM_INOUT,
   PARAM_WAY_LAST
} Param_Way;

typedef struct _eo_param_def
{
   Param_Way way;
   Eolian_Type type;
   const char *name;
   const char *comment;
   Eina_Bool nonull:1;
} Eo_Param_Def;

/* ACCESSOR */

typedef enum _eo_accessor_type
{
   SETTER,
   GETTER,
   ACCESSOR_TYPE_LAST
} Eo_Accessor_Type;

typedef struct _eo_accessor_param
{
   const char *name;
   const char *attrs;
} Eo_Accessor_Param;

typedef struct _eo_accessor_def
{
   Eo_Accessor_Type type;
   Eo_Ret_Def *ret;
   const char *comment;
   const char* legacy;
   Eina_List *params; /* List of Eo_Accessor_Param */
} Eo_Accessor_Def;

/* PROPERTY */

typedef struct _eo_property_def
{
   const char *name;
   Eina_List *keys;
   Eina_List *values;
   Eina_List *accessors;
   int scope;
} Eo_Property_Def;

/* METHOD */

typedef enum _eo_method_type {
     METH_REGULAR,
     METH_CONSTRUCTOR,
     METH_TYPE_LAST
} Eo_Method_Type;

typedef struct _eo_method_def
{
   Eo_Ret_Def *ret;
   Eo_Method_Type type;
   const char *name;
   const char *comment;
   Eina_List *params;
   const char* legacy;
   Eina_Bool obj_const;
   int scope;
} Eo_Method_Def;

/* SIGNAL */

typedef struct _eo_event_def
{
   const char *name;
   const char *type;
   const char *comment;
} Eo_Event_Def;

/* IMPLEMENT */

typedef struct _eo_implement_def
{
   const char *meth_name;
} Eo_Implement_Def;

/* CLASS */

typedef struct _eo_class_def
{
   const char *name;
   Eolian_Class_Type type;
   const char *comment;
   const char *legacy_prefix;
   const char *eo_prefix;
   const char *data_type;
   Eina_List *inherits;
   Eina_List *implements;
   Eina_List *events;
   Eina_List *constructors;
   Eina_List *properties;
   Eina_List *methods;
} Eo_Class_Def;

/* TYPE */

typedef struct _eo_type_def
{
   const char *alias;
   Eolian_Type type;
} Eo_Type_Def;

/* TEMPS */

typedef struct _Eo_Lexer_Temps
{
   Eina_List *str_bufs;
   Eina_List *params;
   const char *legacy_def;
   Eo_Class_Def *kls;
   Eo_Ret_Def *ret_def;
   Eo_Type_Def *type_def;
   Eo_Property_Def *prop;
   Eo_Method_Def *meth;
   Eo_Param_Def *param;
   Eo_Accessor_Def *accessor;
   Eo_Accessor_Param *accessor_param;
   Eina_List *str_items;
   Eo_Event_Def *event;
   Eo_Implement_Def *impl;
   Eolian_Type type;
} Eo_Lexer_Temps;

void eo_definitions_class_def_free(Eo_Class_Def *kls);
void eo_definitions_type_def_free(Eo_Type_Def *type);
void eo_definitions_temps_free(Eo_Lexer_Temps *tmp);

#endif /* __EO_DEFINITIONS_H__ */
