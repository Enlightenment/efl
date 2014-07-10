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
   Eina_Stringshare *dflt_ret_val;
   Eina_Bool warn_unused:1;
} Eo_Ret_Def;

/* PARAM */

typedef enum _Param_Way
{
   PARAM_IN,
   PARAM_OUT,
   PARAM_INOUT,
   PARAM_WAY_LAST
} Param_Way;

typedef struct _Eo_Param_Def
{
   Param_Way way;
   Eolian_Type *type;
   Eina_Stringshare *name;
   Eina_Stringshare *comment;
   Eina_Bool nonull:1;
} Eo_Param_Def;

/* ACCESSOR */

typedef enum _Eo_Accessor_Type
{
   SETTER,
   GETTER,
   ACCESSOR_TYPE_LAST
} Eo_Accessor_Type;

typedef struct _Eo_Accessor_Param
{
   Eina_Stringshare *name;
   Eina_Bool is_const:1;
} Eo_Accessor_Param;

typedef struct _Eo_Accessor_Def
{
   Eo_Accessor_Type type;
   Eo_Ret_Def *ret;
   Eina_Stringshare *comment;
   Eina_Stringshare* legacy;
   Eina_List *params; /* List of Eo_Accessor_Param */
} Eo_Accessor_Def;

/* PROPERTY */

typedef struct _Eo_Property_Def
{
   Eina_Stringshare *name;
   Eina_List *keys;
   Eina_List *values;
   Eina_List *accessors;
   int scope;
} Eo_Property_Def;

/* METHOD */

typedef enum _Eo_Method_Type {
     METH_REGULAR,
     METH_CONSTRUCTOR,
     METH_TYPE_LAST
} Eo_Method_Type;

typedef struct _Eo_Method_Def
{
   Eo_Ret_Def *ret;
   Eo_Method_Type type;
   Eina_Stringshare *name;
   Eina_Stringshare *comment;
   Eina_List *params;
   Eina_Stringshare *legacy;
   Eina_Bool obj_const;
   int scope;
} Eo_Method_Def;

/* SIGNAL */

typedef struct _Eo_Event_Def
{
   Eina_Stringshare *name;
   Eina_Stringshare *type;
   Eina_Stringshare *comment;
} Eo_Event_Def;

/* IMPLEMENT */

typedef struct _Eo_Implement_Def
{
   Eina_Stringshare *meth_name;
} Eo_Implement_Def;

/* CLASS */

typedef struct _Eo_Class_Def
{
   Eina_Stringshare *name;
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
   Eolian_Typedef *typedef_def;
   Eina_List *type_defs;
   Eo_Property_Def *prop;
   Eo_Method_Def *meth;
   Eo_Param_Def *param;
   Eo_Accessor_Def *accessor;
   Eo_Accessor_Param *accessor_param;
   Eina_List *str_items;
   Eo_Event_Def *event;
   Eo_Implement_Def *impl;
} Eo_Lexer_Temps;

void eo_definitions_class_def_free(Eo_Class_Def *kls);
void eo_definitions_temps_free(Eo_Lexer_Temps *tmp);

#endif /* __EO_DEFINITIONS_H__ */
