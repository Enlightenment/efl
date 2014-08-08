#include <Eina.h>
#include "eo_lexer.h"

void
database_var_del(Eolian_Variable *var)
{
   if (!var) return;
   const char *sp;
   if (var->base.file) eina_stringshare_del(var->base.file);
   if (var->base_type)
     database_type_del(var->base_type);
   if (var->name) eina_stringshare_del(var->name);
   if (var->full_name) eina_stringshare_del(var->full_name);
   if (var->namespaces) EINA_LIST_FREE(var->namespaces, sp)
      eina_stringshare_del(sp);
   if (var->value) database_expr_del(var->value);
   if (var->comment) eina_stringshare_del(var->comment);
   free(var);
}

Eina_Bool
database_var_global_add(Eolian_Variable *var)
{
   if (!_globals) return EINA_FALSE;
   eina_hash_set(_globals, var->full_name, var);
   eina_hash_set(_globalsf, var->base.file, eina_list_append
                 ((Eina_List*)eina_hash_find(_globalsf, var->base.file), var));
   return EINA_TRUE;
}

Eina_Bool
database_var_constant_add(Eolian_Variable *var)
{
   if (!_constants) return EINA_FALSE;
   eina_hash_set(_constants, var->full_name, var);
   eina_hash_set(_constantsf, var->base.file, eina_list_append
                 ((Eina_List*)eina_hash_find(_constantsf, var->base.file), var));
   return EINA_TRUE;
}
