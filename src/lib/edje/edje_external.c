#include "edje_private.h"

static Eina_Hash *type_registry = NULL;
static int init_count = 0;

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

EOLIAN Evas_Object*
_edje_part_external_object_get(Eo *obj EINA_UNUSED, Edje *ed, const char *part)
{
   Edje_Real_Part *rp;

   if ((!ed) || (!part)) return NULL;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(&ed, (char *)part);
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
   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
       (!rp->typedata.swallow)) return NULL;

   return rp->typedata.swallow->swallowed_object;
}

EOLIAN Eina_Bool
_edje_part_external_param_set(Eo *obj, Edje *ed, const char *part, const Edje_External_Param *param)
{
   Edje_Real_Part *rp;

   if ((!param) || (!param->name)) return EINA_FALSE;

   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(&ed, (char *)part);
   if (!rp)
     {
	ERR("no part '%s'", part);
	return EINA_FALSE;
     }

   if (_edje_external_param_set(obj, rp, param))
      return EINA_TRUE;
   return EINA_FALSE;
}

EOLIAN Eina_Bool
_edje_part_external_param_get(Eo *obj, Edje *ed, const char *part, Edje_External_Param *param)
{
   Edje_Real_Part *rp;

   if ((!param) || (!param->name)) return EINA_FALSE;

   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(&ed, (char *)part);
   if (!rp)
     {
	ERR("no part '%s'", part);
	return EINA_FALSE;
     }

   return _edje_external_param_get(obj, rp, param);
}

EOLIAN Evas_Object*
_edje_part_external_content_get(Eo *obj EINA_UNUSED, Edje *ed, const char *part, const char *content)
{
   Edje_Real_Part *rp;

   if (!content) return NULL;

   if ((!ed) || (!part)) return NULL;

   rp = _edje_real_part_recursive_get(&ed, (char *)part);
   if (!rp)
     {
	ERR("no part '%s'", part);
	return NULL;
     }
   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
       (!rp->typedata.swallow)) return NULL;

   return _edje_external_content_get(rp->typedata.swallow->swallowed_object, content);
}

EOLIAN Edje_External_Param_Type
_edje_part_external_param_type_get(Eo *obj, Edje *ed, const char *part, const char *param)
{
   Edje_Real_Part *rp;
   Edje_External_Type *type;
   Edje_External_Param_Info *info;

   if ((!ed) || (!part)) return EDJE_EXTERNAL_PARAM_TYPE_MAX;

   rp = _edje_real_part_recursive_get(&ed, (char *)part);
   if (!rp)
     {
	ERR("no part '%s'", part);
	return EDJE_EXTERNAL_PARAM_TYPE_MAX;
     }
  if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
      (!rp->typedata.swallow)) return EDJE_EXTERNAL_PARAM_TYPE_MAX;
   type = evas_object_data_get(rp->typedata.swallow->swallowed_object, "Edje_External_Type");
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
   for (info = type->parameters_info; info->name; info++)
     if (strcmp(info->name, param) == 0) return info->type;;

   ERR("no parameter '%s' external type '%s'", param, type->module_name);

   return EDJE_EXTERNAL_PARAM_TYPE_MAX;
}

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

EAPI Eina_Bool
edje_external_type_unregister(const char *type_name)
{
   if (!type_name)
     return EINA_FALSE;
   return eina_hash_del_by_key(type_registry, type_name);
}

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

EAPI void
edje_external_type_array_unregister(const Edje_External_Type_Info *array)
{
   const Edje_External_Type_Info *itr;

   if (!array)
     return;

   for (itr = array; itr->name; itr++)
     eina_hash_del(type_registry, itr->name, itr->info);
}

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
_edje_external_init(void)
{
   if (!type_registry)
     type_registry = eina_hash_string_superfast_new(NULL);

   init_count++;
}

void
_edje_external_shutdown(void)
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
_edje_external_param_set(Evas_Object *obj, Edje_Real_Part *rp, const Edje_External_Param *param)
{
   Evas_Object *swallowed_object;
   
   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
      (!rp->typedata.swallow)) return EINA_FALSE;
   swallowed_object = rp->typedata.swallow->swallowed_object;
   Edje_External_Type *type = evas_object_data_get(swallowed_object, "Edje_External_Type");
   if (!type)
     {
	if ((rp->part->type == EDJE_PART_TYPE_TEXT) ||
            (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK))
	  {
	     if ((param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING) &&
		 (!strcmp (param->name, "text")) && (obj))
		{
	           return edje_object_part_text_set(obj, rp->part->name, param->s);
		}
	  }

	ERR("no external type for object %p", swallowed_object);
	return EINA_FALSE;
     }
   if (!type->param_set)
     {
	ERR("external type '%s' from module '%s' does not provide param_set()",
	    type->module_name, type->module);
	return EINA_FALSE;
     }
   return type->param_set(type->data, swallowed_object, param);
}

Eina_Bool
_edje_external_param_get(const Evas_Object *obj, Edje_Real_Part *rp, Edje_External_Param *param)
{
   Evas_Object *swallowed_object;
   
   if ((rp->type != EDJE_RP_TYPE_SWALLOW) ||
      (!rp->typedata.swallow)) return EINA_FALSE;
   swallowed_object = rp->typedata.swallow->swallowed_object;
   Edje_External_Type *type = evas_object_data_get(swallowed_object, "Edje_External_Type");
   if (!type)
     {
	if ((rp->part->type == EDJE_PART_TYPE_TEXT) ||
            (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK))
	  {
	     const char *text;
	     param->type = EDJE_EXTERNAL_PARAM_TYPE_STRING;
	     param->name = "text";
	     text = edje_object_part_text_get(obj, rp->part->name);
             param->s = text;
	     return EINA_TRUE;
	  }
       
	ERR("no external type for object %p", swallowed_object);
	return EINA_FALSE;
     }
   if (!type->param_get)
     {
	ERR("external type '%s' from module '%s' does not provide param_get()",
	    type->module_name, type->module);
	return EINA_FALSE;
     }
   return type->param_get(type->data, swallowed_object, param);
}

Evas_Object*
_edje_external_content_get(const Evas_Object *obj, const char *content)
{
   Edje_External_Type *type = evas_object_data_get(obj, "Edje_External_Type");
   if (!type)
     {
	ERR("no external type for object %p", obj);
	return NULL;
     }
   if (!type->content_get)
     {
	ERR("external type '%s' from module '%s' does not provide content_get()",
	    type->module_name, type->module);
	return NULL;
     }
   return type->content_get(type->data, obj, content);
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
_edje_external_recalc_apply(Edje *ed EINA_UNUSED, Edje_Real_Part *ep,
			    Edje_Calc_Params *params EINA_UNUSED,
			    Edje_Part_Description_Common *chosen_desc EINA_UNUSED)
{
   Edje_External_Type *type;
   Edje_Part_Description_External *ext;
   void *params1, *params2 = NULL;

   if ((ep->type != EDJE_RP_TYPE_SWALLOW) ||
      (!ep->typedata.swallow)) return;
   if (!ep->typedata.swallow->swallowed_object) return;
   type = evas_object_data_get(ep->typedata.swallow->swallowed_object, "Edje_External_Type");

   if ((!type) || (!type->state_set)) return;

   ext = (Edje_Part_Description_External*) ep->param1.description;

   params1 = ep->param1.external_params ?
     ep->param1.external_params : ext->external_params;

   if (ep->param2 && ep->param2->description)
     {
	ext = (Edje_Part_Description_External*) ep->param2->description;

	params2 = ep->param2->external_params ?
          ep->param2->external_params : ext->external_params;
     }

   type->state_set(type->data, ep->typedata.swallow->swallowed_object,
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
