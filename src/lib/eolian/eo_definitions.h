#ifndef __EO_DEFINITIONS_H__
#define __EO_DEFINITIONS_H__

#include <Eina.h>
#include <Eolian.h>

#include "eolian_database.h"

/* RET */

typedef struct _Eo_Ret_Def
{
   Eolian_Type *type;
   Eina_Stringshare *comment;
   Eolian_Expression *default_ret_val;
   Eina_Bool warn_unused:1;
} Eo_Ret_Def;

/* PARAM */

typedef struct _Eo_Param_Def
{
   Eolian_Object base;
   Eolian_Parameter_Dir way;
   Eolian_Type *type;
   Eina_Stringshare *name;
   Eina_Stringshare *comment;
   Eina_Bool nonull:1;
} Eo_Param_Def;

/* ACCESSOR */

typedef enum _Eo_Accessor_Type
{
   SETTER,
   GETTER
} Eo_Accessor_Type;

typedef struct _Eo_Accessor_Param
{
   Eina_Stringshare *name;
   Eina_Bool is_const:1;
} Eo_Accessor_Param;

typedef struct _Eo_Accessor_Def
{
   Eolian_Object base;
   Eo_Accessor_Type type;
   Eo_Ret_Def *ret;
   Eina_Stringshare *comment;
   Eina_Stringshare* legacy;
   Eina_List *params; /* List of Eo_Accessor_Param */
   Eina_Bool only_legacy:1;
} Eo_Accessor_Def;

/* PROPERTY */

typedef struct _Eo_Property_Def
{
   Eolian_Object base;
   Eina_Stringshare *name;
   Eina_List *keys;
   Eina_List *values;
   Eina_List *accessors;
   int scope;
   Eina_Bool is_class:1;
   Eina_Bool is_constructing:1;
} Eo_Property_Def;

/* METHOD */

typedef struct _Eo_Method_Def
{
   Eolian_Object base;
   Eo_Ret_Def *ret;
   Eina_Stringshare *name;
   Eina_Stringshare *comment;
   Eina_List *params;
   Eina_Stringshare *legacy;
   Eina_Bool obj_const;
   int scope;
   Eina_Bool is_class:1;
   Eina_Bool is_constructing:1;
   Eina_Bool only_legacy:1;
} Eo_Method_Def;

/* CLASS */

typedef struct _Eo_Class_Def
{
   Eolian_Object base;
   Eina_Stringshare *name;
   Eina_Stringshare *file;
   Eolian_Class_Type type;
   Eina_Stringshare *comment;
   Eina_Stringshare *legacy_prefix;
   Eina_Stringshare *eo_prefix;
   Eina_Stringshare *data_type;
   Eina_List *inherits;
   Eina_List *implements;
   Eina_List *events;
   Eina_List *constructors;
   Eina_List *properties;
   Eina_List *methods;
} Eo_Class_Def;

/* TEMPS */

typedef struct _Eo_Lexer_Temps
{
   Eina_List *str_bufs;
   Eina_List *params;
   Eina_Stringshare *legacy_def;
   Eo_Class_Def *kls;
   Eo_Ret_Def *ret_def;
   Eina_List *type_defs;
   Eina_List *var_defs;
   Eo_Property_Def *prop;
   Eo_Method_Def *meth;
   Eo_Param_Def *param;
   Eo_Accessor_Def *accessor;
   Eo_Accessor_Param *accessor_param;
   Eina_List *str_items;
   Eolian_Event *event;
   Eolian_Implement *impl;
   Eina_List *expr_defs;
   Eina_List *strs;
} Eo_Lexer_Temps;

void eo_definitions_class_def_free(Eo_Class_Def *kls);
void eo_definitions_temps_free(Eo_Lexer_Temps *tmp);

#endif /* __EO_DEFINITIONS_H__ */
