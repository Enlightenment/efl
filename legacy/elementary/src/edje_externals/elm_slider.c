#include "private.h"

typedef struct _Elm_Params_Slider
{
   Elm_Params base;
   Evas_Object *icon;
   const char *indicator, *unit;
   double min, max, value;
   int inverted, span, horizontal;
} Elm_Params_Slider;

static void
external_slider_state_set(void *data, Evas_Object *obj, const void *from_params, const void *to_params, float pos)
{
   const Elm_Params_Slider *p1 = from_params, *p2 = to_params;

   p1 = from_params;
   p2 = to_params;

   if (!p2)
     {
	elm_slider_label_set(obj, p1->base.label);
	elm_slider_icon_set(obj, p1->icon);
	elm_slider_span_size_set(obj, p1->span);
	elm_slider_min_max_set(obj, p1->min, p1->max);
	elm_slider_value_set(obj, p1->value);
	elm_slider_inverted_set(obj, p1->inverted);;
	elm_slider_horizontal_set(obj, p1->horizontal);
	elm_slider_indicator_format_set(obj, p1->indicator);
	elm_slider_unit_format_set(obj, p1->unit);
	return;
     }

   elm_slider_label_set(obj, p2->base.label);
   elm_slider_icon_set(obj, p2->icon);
   elm_slider_span_size_set(obj, p2->span);
   elm_slider_min_max_set(obj, p2->min, p2->max);
   elm_slider_value_set(obj, p2->value);
   elm_slider_inverted_set(obj, p2->inverted);
   elm_slider_horizontal_set(obj, p2->horizontal);
   elm_slider_indicator_format_set(obj, p2->indicator);
   elm_slider_unit_format_set(obj, p2->unit);
}

static void *
external_slider_params_parse(void *data, Evas_Object *obj, const Eina_List *params)
{
   Elm_Params_Slider *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = external_common_params_parse(Elm_Params_Slider, data, obj, params);
   if (!mem)
     return NULL;

   external_common_icon_param_parse(&mem->icon, obj, params);

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "span"))
	  mem->span = param->i;
	else if (!strcmp(param->name, "min"))
	  mem->min = param->d;
	else if (!strcmp(param->name, "max"))
	  mem->max = param->d;
	else if (!strcmp(param->name, "value"))
	  mem->value = param->d;
	else if (!strcmp(param->name, "inverted"))
	  mem->inverted = param->i;
	else if (!strcmp(param->name, "horizontal"))
	  mem->horizontal = param->i;
	else if (!strcmp(param->name, "unit format"))
	  mem->unit = eina_stringshare_add(param->s);
	else if (!strcmp(param->name, "indicator format"))
	  mem->indicator = eina_stringshare_add(param->s);
     }

   return mem;
}

static void
external_slider_params_free(void *params)
{
   Elm_Params_Slider *mem = params;

   if (mem->icon)
     evas_object_del(mem->icon);
   if (mem->unit)
     eina_stringshare_del(mem->unit);
   if (mem->indicator)
     eina_stringshare_del(mem->indicator);
   external_common_params_free(params);
}

static Edje_External_Param_Info external_slider_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("icon"),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE("min"),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT("max", 10.0),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE("value"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("horizontal"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("inverted"),
   EDJE_EXTERNAL_PARAM_INFO_INT("span"),
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("unit format", "%1.2f"),
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("indicator format", "%1.2f"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_TYPE_SIMPLE(slider, "Slider")
