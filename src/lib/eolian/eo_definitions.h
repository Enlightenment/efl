#ifndef __EO_DEFINITIONS_H__
#define __EO_DEFINITIONS_H__

#include <Eina.h>
#include <Eolian.h>

#include "eolian_database.h"

/* METHOD */

typedef struct _Eo_Method_Def
{
   Eolian_Object base;
   Eolian_Type *ret_type;
   Eina_Stringshare *ret_comment;
   Eolian_Expression *default_ret_val;
   Eina_Stringshare *name;
   Eina_Stringshare *comment;
   Eina_List *params;
   Eina_Stringshare *legacy;
   Eina_Bool obj_const;
   int scope;
   Eina_Bool is_class:1;
   Eina_Bool only_legacy:1;
   Eina_Bool ret_warn_unused:1;
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
   Eina_List *constructors;
   Eina_List *events;
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
   Eina_List *type_defs;
   Eina_List *var_defs;
   Eolian_Function *prop;
   Eo_Method_Def *meth;
   Eolian_Function_Parameter *param;
   Eina_List *str_items;
   Eolian_Event *event;
   Eolian_Implement *impl;
   Eolian_Constructor *ctor;
   Eina_List *expr_defs;
   Eina_List *strs;
} Eo_Lexer_Temps;

void eo_definitions_class_def_free(Eo_Class_Def *kls);
void eo_definitions_temps_free(Eo_Lexer_Temps *tmp);

#endif /* __EO_DEFINITIONS_H__ */
