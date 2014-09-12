#ifndef __EO_DEFINITIONS_H__
#define __EO_DEFINITIONS_H__

#include <Eina.h>
#include <Eolian.h>

#include "eolian_database.h"

/* TEMPS */

typedef struct _Eo_Lexer_Temps
{
   Eolian_Class *kls;
   Eina_List *classes;
   Eina_List *str_bufs;
   Eina_List *type_defs;
   Eina_List *var_defs;
   Eina_List *str_items;
   Eina_List *expr_defs;
   Eina_List *strs;
} Eo_Lexer_Temps;

void eo_definitions_temps_free(Eo_Lexer_Temps *tmp);

#endif /* __EO_DEFINITIONS_H__ */
