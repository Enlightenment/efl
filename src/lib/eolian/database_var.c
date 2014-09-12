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

static void
database_var_global_add(Eolian_Variable *var)
{
   eina_hash_set(_globals, var->full_name, var);
   eina_hash_set(_globalsf, var->base.file, eina_list_append
                 ((Eina_List*)eina_hash_find(_globalsf, var->base.file), var));
}

static void
database_var_constant_add(Eolian_Variable *var)
{
   eina_hash_set(_constants, var->full_name, var);
   eina_hash_set(_constantsf, var->base.file, eina_list_append
                 ((Eina_List*)eina_hash_find(_constantsf, var->base.file), var));
}

void
database_var_add(Eolian_Variable *var)
{
   if (var->type == EOLIAN_VAR_GLOBAL)
     database_var_global_add(var);
   else
     database_var_constant_add(var);
}
