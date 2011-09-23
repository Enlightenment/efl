#include <assert.h>

#include "private.h"

typedef struct _Elm_Params_Map
{
   Elm_Params base;
   const char *map_source;
   const char *zoom_mode;
   double zoom;
   Eina_Bool zoom_set:1;
} Elm_Params_Map;

static const char *zoom_choices[] = {"manual",	"auto fit", "auto fill", NULL};

static const char *source_choices[] =
  {"Mapnik", "Osmarender", "CycleMap", "Maplint"};

static Elm_Map_Zoom_Mode
_zoom_mode_get(const char *map_src)
{
   unsigned int i;

   assert(sizeof(zoom_choices)/sizeof(zoom_choices[0]) ==
	  ELM_MAP_ZOOM_MODE_LAST + 1);

   for (i = 0; i < ELM_MAP_ZOOM_MODE_LAST; i++)
     if (!strcmp(map_src, zoom_choices[i])) return i;

   return ELM_MAP_ZOOM_MODE_LAST;
}

static void
external_map_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Map *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->map_source)
     {
	elm_map_source_name_set(obj, p->map_source);
     }
   if (p->zoom_mode)
     {
	Elm_Map_Zoom_Mode set = _zoom_mode_get(p->zoom_mode);
	if (set == ELM_MAP_ZOOM_MODE_LAST) return;
	elm_map_zoom_mode_set(obj, set);
     }
   if (p->zoom_set) elm_map_zoom_set(obj, p->zoom);
}

static Eina_Bool
external_map_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "map source"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
	  {
	     elm_map_source_name_set(obj, param->s);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "zoom mode"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
	  {
	     Elm_Map_Zoom_Mode set = _zoom_mode_get(param->s);
	     if (set == ELM_MAP_ZOOM_MODE_LAST) return EINA_FALSE;
	     elm_map_zoom_mode_set(obj, set);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "zoom level"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
	  {
	     elm_map_zoom_set(obj, param->d);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_map_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "map source"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
	  {
	     const char *set = elm_map_source_name_get(obj);
	     param->s = set;
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "zoom mode"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
	  {
	     Elm_Map_Zoom_Mode set = elm_map_zoom_mode_get(obj);
	     if (set == ELM_MAP_ZOOM_MODE_LAST) return EINA_FALSE;
	     param->s = zoom_choices[set];
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "zoom level"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
	  {
	     param->d = elm_map_zoom_get(obj);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_map_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Map *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Map));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "map source"))
	  mem->map_source = eina_stringshare_add(param->s);
	if (!strcmp(param->name, "zoom mode"))
	  mem->zoom_mode = eina_stringshare_add(param->s);
	else if (!strcmp(param->name, "zoom level"))
	  {
	     mem->zoom = param->d;
	     mem->zoom_set = EINA_TRUE;
	  }
     }

   return mem;
}

static Evas_Object *external_map_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
	ERR("No content.");
	return NULL;
}

static void
external_map_params_free(void *params)
{
   Elm_Params_Map *mem = params;

   if (mem->map_source)
     eina_stringshare_del(mem->map_source);
   if (mem->zoom_mode)
     eina_stringshare_del(mem->zoom_mode);
   free(mem);
}

static Edje_External_Param_Info external_map_params[] =
  {
    DEFINE_EXTERNAL_COMMON_PARAMS,
    EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL("map source", "Mapnik", source_choices),
    EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL("zoom mode", "manual", zoom_choices),
    EDJE_EXTERNAL_PARAM_INFO_DOUBLE("zoom level"),
    EDJE_EXTERNAL_PARAM_INFO_SENTINEL
  };

DEFINE_EXTERNAL_ICON_ADD(map, "map")
DEFINE_EXTERNAL_TYPE_SIMPLE(map, "Map")

