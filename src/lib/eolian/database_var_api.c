#include <Eina.h>
#include "eolian_database.h"
#include "eo_definitions.h"

EAPI const Eolian_Variable *
eolian_variable_global_get_by_name(const char *name)
{
   if (!_globals) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Variable *v = eina_hash_find(_globals, shr);
   eina_stringshare_del(shr);
   return v;
}

EAPI const Eolian_Variable *
eolian_variable_constant_get_by_name(const char *name)
{
   if (!_constants) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(name);
   Eolian_Variable *v = eina_hash_find(_constants, shr);
   eina_stringshare_del(shr);
   return v;
}

EAPI Eina_Iterator *
eolian_variable_globals_get_by_file(const char *fname)
{
   if (!_globalsf) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(fname);
   Eina_List *l = eina_hash_find(_globalsf, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eina_Iterator *
eolian_variable_constants_get_by_file(const char *fname)
{
   if (!_constantsf) return NULL;
   Eina_Stringshare *shr = eina_stringshare_add(fname);
   Eina_List *l = eina_hash_find(_constantsf, shr);
   eina_stringshare_del(shr);
   if (!l) return NULL;
   return eina_list_iterator_new(l);
}

EAPI Eolian_Variable_Type
eolian_variable_type_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, EOLIAN_VAR_UNKNOWN);
   return var->type;
}

EAPI Eina_Stringshare *
eolian_variable_description_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return eina_stringshare_ref(var->comment);
}

EAPI Eina_Stringshare *
eolian_variable_file_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return eina_stringshare_ref(var->base.file);
}

EAPI const Eolian_Type *
eolian_variable_base_type_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return var->base_type;
}

EAPI Eina_Stringshare *
eolian_variable_name_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return eina_stringshare_ref(var->name);
}

EAPI Eina_Stringshare *
eolian_variable_full_name_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   return eina_stringshare_ref(var->full_name);
}

EAPI Eina_Iterator *
eolian_variable_namespaces_get(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, NULL);
   if (!var->namespaces) return NULL;
   return eina_list_iterator_new(var->namespaces);
}

EAPI Eina_Bool
eolian_variable_is_extern(const Eolian_Variable *var)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(var, EINA_FALSE);
   return var->is_extern;
}
