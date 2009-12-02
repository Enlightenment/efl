/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"

static Eina_Hash *type_registry = NULL;
static int init_count = 0;

/**
 * Register given type name to return the given information.
 *
 * @param type_name name to register and be known by edje's "source:"
 *        parameter of "type: EXTERNAL" parts.
 * @param type_info meta-information describing how to interact with it.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (like
 *         type already registered).
 *
 * @see edje_external_type_array_register()
 */
EAPI Eina_Bool
edje_external_type_register(const char *type_name, const Edje_External_Type *type_info)
{
   if (!type_name)
     return EINA_FALSE;
   if (!type_info)
     return EINA_FALSE;

   if (type_info->abi_version != EDJE_EXTERNAL_TYPE_ABI_VERSION)
     {
	printf("EDJE ERROR: external type '%s' (%p) has incorrect abi version. "
	       "got %#x where %#x was expected.\n",
	       type_name, type_info,
	       type_info->abi_version, EDJE_EXTERNAL_TYPE_ABI_VERSION);
	return EINA_FALSE;
     }

   if (eina_hash_find(type_registry, type_name))
     {
	printf("EDJE ERROR: external type '%s' already registered\n", type_name);
	return EINA_FALSE;
     }
   return eina_hash_add(type_registry, type_name, type_info);
}

/**
 * Unregister given type name previously registered.
 *
 * @param type_name name to unregister. It should be registered with
 *        edje_external_type_register() before.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (like
 *         type_name did not exist).
 *
 * @see edje_external_type_array_unregister()
 */
EAPI Eina_Bool
edje_external_type_unregister(const char *type_name)
{
   if (!type_name)
     return EINA_FALSE;
   return eina_hash_del_by_key(type_registry, type_name);
}

/**
 * Register a batch of types and their information.
 *
 * This is the recommended function to add information as it's faster
 * than the single version edje_external_type_register().
 *
 * @note the given array is not modified, but the type name strings
 *       are @b not duplicated! That is, all type names must be @b
 *       live until they are unregistered! This was choosen to save
 *       some memory and most people will just define the array as a
 *       global static const type anyway.
 *
 * @param arrray @c NULL terminated array with type name and
 *        information. Note that type name or information are not
 *        modified by are @b referenced, so they must keep alive after
 *        this function returns!
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure (like
 *         type already registered).
 *
 * @see edje_external_type_register()
 */
EAPI void
edje_external_type_array_register(const Edje_External_Type_Info *array)
{
   const Edje_External_Type_Info *itr;

   if (!array)
     return;

   for (itr = array; itr->name; itr++)
     {
	if (itr->info->abi_version != EDJE_EXTERNAL_TYPE_ABI_VERSION)
	  {
	     printf("EDJE ERROR: external type '%s' (%p) has incorrect abi "
		    "version. got %#x where %#x was expected.\n",
		    itr->name, itr->info,
		    itr->info->abi_version, EDJE_EXTERNAL_TYPE_ABI_VERSION);
	     continue;
	  }

	eina_hash_direct_add(type_registry, itr->name, itr->info);
     }
}

/**
 * Unregister a batch of given external type previously registered.
 *
 * @param array @c NULL terminated array, should be the same as the
 *        one used to register with edje_external_type_array_register()
 *
 * @see edje_external_type_unregister()
 */
EAPI void
edje_external_type_array_unregister(const Edje_External_Type_Info *array)
{
   const Edje_External_Type_Info *itr;

   if (!array)
     return;

   for (itr = array; itr->name; itr++)
     eina_hash_del(type_registry, itr->name, itr->info);
}

/**
 * Return the current ABI version for Edje_External_Type structure.
 *
 * Always check this number before accessing Edje_External_Type in
 * your own software. If the number is not the same, your software may
 * access invalid memory and crash, or just get garbage values.
 *
 * @warning @b NEVER, EVER define your own Edje_External_Type using the
 *          return of this function as it will change as Edje library
 *          (libedje.so) changes, but your type definition will
 *          not. Instead, use #EDJE_EXTERNAL_TYPE_ABI_VERSION.
 *
 * Summary:
 *   - use edje_external_type_abi_version_get() to check.
 *   - use #EDJE_EXTERNAL_TYPE_ABI_VERSION to define/declare.
 *
 * @return version this edje library was compiled.
 */
EAPI unsigned int
edje_external_type_abi_version_get(void)
{
   return EDJE_EXTERNAL_TYPE_ABI_VERSION;
}

EAPI Eina_Iterator *
edje_external_iterator_get(void)
{
   return eina_hash_iterator_tuple_new(type_registry);
}

EAPI Edje_External_Param *
edje_external_param_find(const Eina_List *params, const char *key)
{
   const Eina_List *l;
   Edje_External_Param *param;

   EINA_LIST_FOREACH(params, l, param)
      if (!strcmp(param->name, key)) return param;

   return NULL;
}

EAPI Eina_Bool
edje_external_param_int_get(const Eina_List *params, const char *key, int *ret)
{
   Edje_External_Param *param;

   if (!params) return EINA_FALSE;
   param = edje_external_param_find(params, key);

   if (param && param->type == EDJE_EXTERNAL_PARAM_TYPE_INT && ret)
     {
	*ret = param->i;
	return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_external_param_double_get(const Eina_List *params, const char *key, double *ret)
{
   Edje_External_Param *param;

   if (!params) return EINA_FALSE;
   param = edje_external_param_find(params, key);

   if (param && param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE && ret)
     {
	*ret = param->d;
	return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_external_param_string_get(const Eina_List *params, const char *key, const char **ret)
{
   Edje_External_Param *param;

   if (!params) return EINA_FALSE;
   param = edje_external_param_find(params, key);

   if (param && param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING && ret)
     {
	*ret = param->s;
	return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI const Edje_External_Param_Info *
edje_external_param_info_get(const char *type_name)
{
   Edje_External_Type *type;

   type = eina_hash_find(type_registry, type_name);
   if (!type)
     return NULL;
   return type->parameters_info;
}

void
_edje_external_init()
{
   if (!type_registry)
     type_registry = eina_hash_string_superfast_new(NULL);

   init_count++;
}

void
_edje_external_shutdown()
{
   if (--init_count == 0)
     {
	eina_hash_free(type_registry);
	type_registry = NULL;
     }
}

Evas_Object *
_edje_external_type_add(const char *type_name, Evas *evas, Evas_Object *parent, const Eina_List *params)
{
   Edje_External_Type *type;
   Evas_Object *obj;

   type = eina_hash_find(type_registry, type_name);
   if (!type)
     {
	printf("EDJE ERROR: external type '%s' not registered\n", type_name);
	return NULL;
     }

   obj = type->add(type->data, evas, parent, params);
   if (!obj)
     {
	printf("EDJE ERROR: external type '%s' returned NULL from constructor\n", type_name);
	return NULL;
     }

   evas_object_data_set(obj, "Edje_External_Type", type);

   printf("evas object: %p, external type: %p, data_get: %p\n", obj, type, evas_object_data_get(obj, "Edje_External_Type"));
   return obj;
}

void
_edje_external_signal_emit(Evas_Object *obj, const char *emission, const char *source)
{
   Edje_External_Type *type;

   type = evas_object_data_get(obj, "Edje_External_Type");
   if (!type)
     {
	printf("EDJE ERROR: external type data not found.\n");
	return;
     }

   type->signal_emit(type->data, obj, emission, source);
}

void
_edje_external_params_free(Eina_List *external_params, unsigned int free_strings)
{
   Edje_External_Param *param;

   EINA_LIST_FREE(external_params, param)
     {
	if (free_strings)
	  {
	     if (param->name) eina_stringshare_del(param->name);
	     if (param->s) eina_stringshare_del(param->s);
	  }
	free(param);
     }
}

void
_edje_external_recalc_apply(Edje *ed, Edje_Real_Part *ep,
      Edje_Calc_Params *params,
      Edje_Part_Description *chosen_desc)
{
   Edje_External_Type *type;
   void *params1, *params2 = NULL;
   if (!ep->swallowed_object) return;

   type = evas_object_data_get(ep->swallowed_object, "Edje_External_Type");

   if (!type) return;

   if (!type->state_set) return;

   params1 = ep->param1.external_params ? ep->param1.external_params : ep->param1.description->external_params;
   if (ep->param2 && ep->param2->description)
     params2 = ep->param2->external_params ? ep->param2->external_params : ep->param2->description->external_params;

   type->state_set(type->data, ep->swallowed_object,
	 params1, params2, ep->description_pos);
}

void *
_edje_external_params_parse(Evas_Object *obj, const Eina_List *params)
{
   Edje_External_Type *type;

   type = evas_object_data_get(obj, "Edje_External_Type");
   if (!type) return NULL;

   if (!type->params_parse) return NULL;

   return type->params_parse(type->data, obj, params);
}

void
_edje_external_parsed_params_free(Evas_Object *obj, void *params)
{
   Edje_External_Type *type;

   if (!params) return;

   type = evas_object_data_get(obj, "Edje_External_Type");
   if (!type) return;

   if (!type->params_free) return;

   type->params_free(params);
}
