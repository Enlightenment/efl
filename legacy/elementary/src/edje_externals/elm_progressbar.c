#include "private.h"

typedef struct _Elm_Params_Progressbar
{
   Elm_Params base;
   const char *label;
   Evas_Object *icon;
   const char *unit;
   double value;
   Evas_Coord span;
   Eina_Bool value_exists:1;
   Eina_Bool span_exists:1;
   Eina_Bool inverted:1;
   Eina_Bool inverted_exists:1;
   Eina_Bool horizontal:1;
   Eina_Bool horizontal_exists:1;
} Elm_Params_Progressbar;

static void
external_progressbar_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Progressbar *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->label)
     elm_object_text_set(obj, p->label);
   if (p->icon)
     elm_progressbar_icon_set(obj, p->icon);
   if (p->span_exists)
     elm_progressbar_span_size_set(obj, p->span);
   if (p->value_exists)
     elm_progressbar_value_set(obj, p->value);
   if (p->inverted_exists)
     elm_progressbar_inverted_set(obj, p->inverted);
   if (p->horizontal_exists)
     elm_progressbar_horizontal_set(obj, p->horizontal);
   if (p->unit)
     elm_progressbar_unit_format_set(obj, p->unit);
}

static Eina_Bool
external_progressbar_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "label"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     elm_object_text_set(obj, param->s);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "icon"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     Evas_Object *icon = external_common_param_icon_get(obj, param);
	     if ((strcmp(param->s, "")) && (!icon)) return EINA_FALSE;
	     elm_progressbar_icon_set(obj, icon);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "value"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
	  {
	     elm_progressbar_value_set(obj, param->d);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "horizontal"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     elm_progressbar_horizontal_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "inverted"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     elm_progressbar_inverted_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "span"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
	  {
	     elm_progressbar_span_size_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "unit format"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     elm_progressbar_unit_format_set(obj, param->s);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_progressbar_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "label"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     param->s = elm_object_text_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "icon"))
     {
	/* not easy to get icon name back from live object */
	return EINA_FALSE;
     }
   else if (!strcmp(param->name, "value"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
	  {
	     param->d = elm_progressbar_value_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "horizontal"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     param->i = elm_progressbar_horizontal_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "inverted"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     param->i = elm_progressbar_inverted_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "span"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
	  {
	     param->i = elm_progressbar_span_size_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "unit format"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     param->s = elm_progressbar_unit_format_get(obj);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_progressbar_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Progressbar *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Progressbar));
   if (!mem)
     return NULL;

   external_common_icon_param_parse(&mem->icon, obj, params);

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "span"))
	  {
	     mem->span = param->i;
	     mem->span_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "value"))
	  {
	     mem->value = param->d;
	     mem->value_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "inverted"))
	  {
	     mem->inverted = !!param->i;
	     mem->inverted_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "horizontal"))
	  {
	      mem->horizontal = !!param->i;
	      mem->horizontal_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "unit format"))
	  mem->unit = eina_stringshare_add(param->s);
	else if (!strcmp(param->name, "label"))
	  mem->label = eina_stringshare_add(param->s);
     }

   return mem;
}

static Evas_Object *external_progressbar_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
	ERR("No content.");
	return NULL;
}

static void
external_progressbar_params_free(void *params)
{
   Elm_Params_Progressbar *mem = params;

   if (mem->unit)
     eina_stringshare_del(mem->unit);
   if (mem->label)
      eina_stringshare_del(mem->label);
   free(params);
}

static Edje_External_Param_Info external_progressbar_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("label"),
   EDJE_EXTERNAL_PARAM_INFO_STRING("icon"),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE("value"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("horizontal"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("inverted"),
   EDJE_EXTERNAL_PARAM_INFO_INT("span"),
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("unit format", "%1.2f"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(progressbar, "progressbar")
DEFINE_EXTERNAL_TYPE_SIMPLE(progressbar, "Progressbar")
