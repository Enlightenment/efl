#ifndef __EO_DEFINITIONS_H__
#define __EO_DEFINITIONS_H__

#include <Eina.h>
#include <Eolian.h>

#include "eolian_database.h"

/* CLASS */

typedef struct _Eo_Class_Def
{
   Eolian_Object base;
   Eina_Stringshare *name;
   Eolian_Class_Type type;
   Eina_Stringshare *description;
   Eina_Stringshare *legacy_prefix;
   Eina_Stringshare *eo_prefix;
   Eina_Stringshare *data_type;
   Eina_List *inherits;
   Eina_List *properties;
   Eina_List *methods;
   Eina_List *implements;
   Eina_List *constructors;
   Eina_List *events;
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
   Eolian_Function *func;
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
