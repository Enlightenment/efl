#include "private.h"

typedef struct _Elm_Params_Anchorblock
{
   Elm_Params base;
   const char *text;
} Elm_Params_Anchorblock;

static void
external_anchorblock_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Anchorblock *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->text)
     {
	elm_object_text_set(obj, p->text);
     }
}

static Eina_Bool
external_anchorblock_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "text"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     elm_object_text_set(obj, param->s);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_anchorblock_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "text"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     param->s = elm_object_text_get(obj);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_anchorblock_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Anchorblock *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = ELM_NEW(Elm_Params_Anchorblock);
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "text"))
	  mem->text = eina_stringshare_add(param->s);
     }

   return mem;
}

static Evas_Object *external_anchorblock_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
	ERR("No content.");
	return NULL;
}

static void
external_anchorblock_params_free(void *params)
{
   Elm_Params_Anchorblock *mem = params;

   if (mem->text)
     eina_stringshare_del(mem->text);
   free(mem);
}

static Edje_External_Param_Info external_anchorblock_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("text", "some text"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(anchorblock, "anchorblock")
DEFINE_EXTERNAL_TYPE_SIMPLE(anchorblock, "Anchorblock");
