/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"

static Eina_Hash *type_registry = NULL;
static int init_count = 0;

/**
 * @brief Converts type identifier to string nicer representation.
 *
 * This may be used to debug or other informational purposes.
 *
 * @param type the identifier to convert.
 * @return the string with the string representation, or @c "(unknown)".
 */
EAPI const char *
edje_external_param_type_str(Edje_External_Param_Type type)
{
   switch (type)
     {
      case EDJE_EXTERNAL_PARAM_TYPE_INT:
	 return "INT";
      case EDJE_EXTERNAL_PARAM_TYPE_DOUBLE:
	 return "DOUBLE";
      case EDJE_EXTERNAL_PARAM_TYPE_STRING:
	 return "STRING";
      case EDJE_EXTERNAL_PARAM_TYPE_BOOL:
	 return "BOOL";
      case EDJE_EXTERNAL_PARAM_TYPE_CHOICE:
	 return "CHOICE";
      default:
	 return "(unknown)";
     }
}

/**
 * @brief Get the object created by this external part.
 *
 * Parts of type external creates the part object using information
 * provided by external plugins. It's somehow like "swallow"
 * (edje_object_part_swallow()), but it's all set automatically.
 *
 * This function returns the part created by such external plugins and
 * being currently managed by this Edje.
 *
 * @note Almost all swallow rules apply: you should not move, resize,
 *       hide, show, set the color or clipper of such part. It's a bit
 *       more restrictive as one must @b never delete this object!
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The externally created object, or NULL if there is none or
 *         part is not an external.
 */
EAPI Evas_Object *
edje_object_part_external_object_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	ERR("no part '%s'", part);
	return NULL;
     }
   if (rp->part->type != EDJE_PART_TYPE_EXTERNAL)
     {
	ERR("cannot get external object of a part '%s' that is not EXTERNAL",
	    rp->part->name);
	return NULL;
     }
   return rp->swallowed_object;
}

/**
 * @brief Set the parameter for the external part.
 *
 * Parts of type external may carry extra properties that have
 * meanings defined by the external plugin. For instance, it may be a
 * string that defines a button label and setting this property will
 * change that label on the fly.
 *
 * @note external parts have parameters set when they change
 *       states. Those parameters will never be changed by this
 *       function. The interpretation of how state_set parameters and
 *       param_set will interact is up to the external plugin.
 *
 * @note this function will not check if parameter value is valid
 *       using #Edje_External_Param_Info minimum, maximum, valid
 *       choices and others. However these should be checked by the
 *       underlying implementation provided by the external
 *       plugin. This is done for performance reasons.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param param the parameter details, including its name, type and
 *        actual value. This pointer should be valid, and the
 *        parameter must exist in
 *        #Edje_External_Type::parameters_info, with the exact type,
 *        otherwise the operation will fail and @c EINA_FALSE will be
 *        returned.
 *
 * @return @c EINA_TRUE if everything went fine, @c EINA_FALSE on errors.
 */
EAPI Eina_Bool
edje_object_part_external_param_set(Evas_Object *obj, const char *part, const Edje_External_Param *param)
{
   Edje *ed;
   Edje_Real_Part *rp;

   if ((!param) || (!param->name)) return EINA_FALSE;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	ERR("no part '%s'", part);
	return EINA_FALSE;
     }

   return _edje_external_param_set(rp->swallowed_object, param);
}

/**
 * @brief Get the parameter for the external part.
 *
 * Parts of type external may carry extra properties that have
 * meanings defined by the external plugin. For instance, it may be a
 * string that defines a button label. This property can be modifed by
 * state parameters, by explicit calls to
 * edje_object_part_external_param_set() or getting the actual object
 * with edje_object_part_external_object_get() and calling native
 * functions.
 *
 * This function asks the external plugin what is the current value,
 * independent on how it was set.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name

 * @param param the parameter details. It is used as both input and
 *        output variable. This pointer should be valid, and the
 *        parameter must exist in
 *        #Edje_External_Type::parameters_info, with the exact type,
 *        otherwise the operation will fail and @c EINA_FALSE will be
 *        returned.
 *
 * @return @c EINA_TRUE if everything went fine and @p param members
 *         are filled with information, @c EINA_FALSE on errors and @p
 *         param member values are not set or valid.
 */
EAPI Eina_Bool
edje_object_part_external_param_get(const Evas_Object *obj, const char *part, Edje_External_Param *param)
{
   Edje *ed;
   Edje_Real_Part *rp;

   if ((!param) || (!param->name)) return EINA_FALSE;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	ERR("no part '%s'", part);
	return EINA_FALSE;
     }

   return _edje_external_param_get(rp->swallowed_object, param);
}

/**
 * Facility to query the type of the given parameter of the given part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param param the parameter name to use.
 *
 * @return @c EDJE_EXTERNAL_PARAM_TYPE_MAX on errors, or another value
 *         from #Edje_External_Param_Type on success.
 */
EAPI Edje_External_Param_Type
edje_object_part_external_param_type_get(const Evas_Object *obj, const char *part, const char *param)
{
   Edje *ed;
   Edje_Real_Part *rp;
   Edje_External_Type *type;
   Edje_External_Param_Info *info;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EDJE_EXTERNAL_PARAM_TYPE_MAX;

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	ERR("no part '%s'", part);
	return EDJE_EXTERNAL_PARAM_TYPE_MAX;
     }
   type = evas_object_data_get(rp->swallowed_object, "Edje_External_Type");
   if (!type)
     {
	ERR("no external type for object %p", obj);
	return EDJE_EXTERNAL_PARAM_TYPE_MAX;
     }
   if (!type->parameters_info)
     {
	ERR("no parameters information for external type '%s'",
	    type->module_name);
	return EDJE_EXTERNAL_PARAM_TYPE_MAX;
     }
   for (info = type->parameters_info; info->name != NULL; info++)
     if (strcmp(info->name, param) == 0)
       return info->type;

   ERR("no parameter '%s' external type '%s'", param, type->module_name);
   return EDJE_EXTERNAL_PARAM_TYPE_MAX;
}


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
	ERR("external type '%s' (%p) has incorrect abi version. "
	       "got %#x where %#x was expected.",
	       type_name, type_info,
	       type_info->abi_version, EDJE_EXTERNAL_TYPE_ABI_VERSION);
	return EINA_FALSE;
     }

   if (eina_hash_find(type_registry, type_name))
     {
	ERR("External type '%s' already registered", type_name);
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
	     ERR("external type '%s' (%p) has incorrect abi "
		    "version. got %#x where %#x was expected.",
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

EAPI Eina_Bool
edje_external_param_bool_get(const Eina_List *params, const char *key, Eina_Bool *ret)
{
   Edje_External_Param *param;

   if (!params) return EINA_FALSE;
   param = edje_external_param_find(params, key);

   if (param && param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL && ret)
     {
	*ret = param->i;
	return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
edje_external_param_choice_get(const Eina_List *params, const char *key, const char **ret)
{
   Edje_External_Param *param;

   if (!params) return EINA_FALSE;
   param = edje_external_param_find(params, key);

   if (param && param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE && ret)
     {
	*ret = param->s;
	return EINA_TRUE;
     }

   return EINA_FALSE;
}

/**
 * Get the array of parameters information about a type given its name.
 *
 * @note the type names and other strings are static, that means they
 *       @b NOT translated. One must use
 *       Edje_External_Type::translate() to translate those.
 *
 * @return the NULL terminated array, or @c NULL if type is unknown or
 *         it does not have any parameter information.
 *
 * @see edje_external_type_get()
 */
EAPI const Edje_External_Param_Info *
edje_external_param_info_get(const char *type_name)
{
   Edje_External_Type *type;

   type = eina_hash_find(type_registry, type_name);
   if (!type)
     return NULL;
   return type->parameters_info;
}

EAPI const Edje_External_Type *
edje_external_type_get(const char *type_name)
{
   return eina_hash_find(type_registry, type_name);
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
_edje_external_type_add(const char *type_name, Evas *evas, Evas_Object *parent, const Eina_List *params, const char *part_name)
{
   Edje_External_Type *type;
   Evas_Object *obj;

   type = eina_hash_find(type_registry, type_name);
   if (!type)
     {
	ERR("external type '%s' not registered", type_name);
	return NULL;
     }

   obj = type->add(type->data, evas, parent, params, part_name);
   if (!obj)
     {
	ERR("External type '%s' returned NULL from constructor", type_name);
	return NULL;
     }

   evas_object_data_set(obj, "Edje_External_Type", type);

   return obj;
}

void
_edje_external_signal_emit(Evas_Object *obj, const char *emission, const char *source)
{
   Edje_External_Type *type;

   type = evas_object_data_get(obj, "Edje_External_Type");
   if (!type)
     {
	ERR("External type data not found.");
	return;
     }

   type->signal_emit(type->data, obj, emission, source);
}

Eina_Bool
_edje_external_param_set(Evas_Object *obj, const Edje_External_Param *param)
{
   Edje_External_Type *type = evas_object_data_get(obj, "Edje_External_Type");
   if (!type)
     {
	ERR("no external type for object %p", obj);
	return EINA_FALSE;
     }
   if (!type->param_set)
     {
	ERR("external type '%s' from module '%s' does not provide param_set()",
	    type->module_name, type->module);
	return EINA_FALSE;
     }
   return type->param_set(type->data, obj, param);
}

Eina_Bool
_edje_external_param_get(const Evas_Object *obj, Edje_External_Param *param)
{
   Edje_External_Type *type = evas_object_data_get(obj, "Edje_External_Type");
   if (!type)
     {
	ERR("no external type for object %p", obj);
	return EINA_FALSE;
     }
   if (!type->param_get)
     {
	ERR("external type '%s' from module '%s' does not provide param_set()",
	    type->module_name, type->module);
	return EINA_FALSE;
     }
   return type->param_get(type->data, obj, param);
}

void
_edje_external_params_free(Eina_List *external_params, Eina_Bool free_strings)
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

   params1 = ep->param1.external_params ?
		  ep->param1.external_params :
		  ep->param1.description->external_params;

   if (ep->param2 && ep->param2->description)
     params2 = ep->param2->external_params ?
		  ep->param2->external_params :
		  ep->param2->description->external_params;

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
