#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eo_lexer.h"

void
database_var_del(Eolian_Variable *var)
{
   if (!var || eolian_object_unref(&var->base)) return;
   eina_stringshare_del(var->base.file);
   eina_stringshare_del(var->base.name);
   if (var->base_type)
     database_type_del(var->base_type);
   if (var->value) database_expr_del(var->value);
   database_doc_del(var->doc);
   free(var);
}

static void
database_var_global_add(Eolian_Unit *unit, Eolian_Variable *var)
{
   EOLIAN_OBJECT_ADD(unit, var->base.name, var, globals);
   eina_hash_set(unit->state->staging.globals_f, var->base.file, eina_list_append
                 ((Eina_List*)eina_hash_find(unit->state->staging.globals_f, var->base.file), var));
}

static void
database_var_constant_add(Eolian_Unit *unit, Eolian_Variable *var)
{
   EOLIAN_OBJECT_ADD(unit, var->base.name, var, constants);
   eina_hash_set(unit->state->staging.constants_f, var->base.file, eina_list_append
                 ((Eina_List*)eina_hash_find(unit->state->staging.constants_f, var->base.file), var));
}

void
database_var_add(Eolian_Unit *unit, Eolian_Variable *var)
{
   if (var->type == EOLIAN_VAR_GLOBAL)
     database_var_global_add(unit, var);
   else
     database_var_constant_add(unit, var);
   database_object_add(unit, &var->base);
}
