#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <Eina.h>
#include "eo_lexer.h"

void
database_var_del(Eolian_Variable *var)
{
   if (!var || eolian_object_unref(&var->base)) return;
   const char *sp;
   if (var->base.file) eina_stringshare_del(var->base.file);
   if (var->base_type)
     database_type_del(var->base_type);
   if (var->name) eina_stringshare_del(var->name);
   if (var->full_name) eina_stringshare_del(var->full_name);
   if (var->namespaces) EINA_LIST_FREE(var->namespaces, sp)
      eina_stringshare_del(sp);
   if (var->value) database_expr_del(var->value);
   database_doc_del(var->doc);
   free(var);
}

static void
database_var_global_add(Eolian_Unit *unit, Eolian_Variable *var)
{
   eolian_object_add(&var->base, var->full_name, unit->state->unit.globals);
   eolian_object_add(&var->base, var->full_name, unit->globals);
   eina_hash_set(unit->state->globals_f, var->base.file, eina_list_append
                 ((Eina_List*)eina_hash_find(unit->state->globals_f, var->base.file), var));
   database_decl_add(unit, var->full_name, EOLIAN_DECL_VAR, var->base.file, var);
}

static void
database_var_constant_add(Eolian_Unit *unit, Eolian_Variable *var)
{
   eolian_object_add(&var->base, var->full_name, unit->state->unit.constants);
   eolian_object_add(&var->base, var->full_name, unit->constants);
   eina_hash_set(unit->state->constants_f, var->base.file, eina_list_append
                 ((Eina_List*)eina_hash_find(unit->state->constants_f, var->base.file), var));
   database_decl_add(unit, var->full_name, EOLIAN_DECL_VAR, var->base.file, var);
}

void
database_var_add(Eolian_Unit *unit, Eolian_Variable *var)
{
   if (var->type == EOLIAN_VAR_GLOBAL)
     database_var_global_add(unit, var);
   else
     database_var_constant_add(unit, var);
}
