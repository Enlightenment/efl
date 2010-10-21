#include "private.h"

typedef struct _Elm_Params_Toolbar
{
   Elm_Params base;
   int icon_size;
   Eina_Bool icon_size_exists:1;
   double align;
   Eina_Bool align_exists:1;
} Elm_Params_Toolbar;

static void
external_toolbar_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Toolbar *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->icon_size_exists)
     elm_toolbar_icon_size_set(obj, p->icon_size);
   if (p->align_exists)
     elm_toolbar_align_set(obj, p->align);
}

static Eina_Bool
external_toolbar_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "icon_size"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
	  {
	     elm_toolbar_icon_size_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "align"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
	  {
	     elm_toolbar_align_set(obj, param->d);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_toolbar_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "icon_size"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
	  {
	     param->i = elm_toolbar_icon_size_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "align"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
	  {
	     param->d = elm_toolbar_align_get(obj);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_toolbar_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Toolbar *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Toolbar));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "icon_size"))
	  {
	     mem->icon_size = param->i;
	     mem->icon_size_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "align"))
	  {
	     mem->align = param->d;
	     mem->align_exists = EINA_TRUE;
	  }
     }

   return mem;
}

static Evas_Object *external_toolbar_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
	ERR("No content.");
	return NULL;
}

static void
external_toolbar_params_free(void *params)
{
   Elm_Params_Toolbar *mem = params;
   free(mem);
}

static Edje_External_Param_Info external_toolbar_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_INT("icon_size"),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE("align"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(toolbar, "toolbar");
DEFINE_EXTERNAL_TYPE_SIMPLE(toolbar, "Toolbar");
