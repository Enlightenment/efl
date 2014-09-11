#ifndef __EO_DEFINITIONS_H__
#define __EO_DEFINITIONS_H__

#include <Eina.h>
#include <Eolian.h>

#include "eolian_database.h"

/* TEMPS */

typedef struct _Eo_Lexer_Temps
{
   Eina_List *str_bufs;
   Eina_List *params;
   Eina_Stringshare *legacy_def;
   Eolian_Class *kls;
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

void eo_definitions_temps_free(Eo_Lexer_Temps *tmp);

#endif /* __EO_DEFINITIONS_H__ */
