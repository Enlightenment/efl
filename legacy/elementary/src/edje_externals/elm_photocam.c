#include <assert.h>

#include "private.h"

typedef struct _Elm_Params_Photocam
{
   Elm_Params base;
   const char *file;
   double zoom;
   const char *zoom_mode;
   Eina_Bool paused:1;
   Eina_Bool paused_exists:1;
   Eina_Bool zoom_exists:1;
} Elm_Params_Photocam;

static const char* choices[] = {"manual", "auto fit", "auto fill", NULL};

static Elm_Photocam_Zoom_Mode
_zoom_mode_setting_get(const char *zoom_mode_str)
{
   unsigned int i;

   assert(sizeof(choices)/sizeof(choices[0]) == ELM_PHOTOCAM_ZOOM_MODE_LAST + 1);

   for (i = 0; i < ELM_PHOTOCAM_ZOOM_MODE_LAST; i++)
     {
	if (!strcmp(zoom_mode_str, choices[i]))
	  return i;
     }
   return ELM_PHOTOCAM_ZOOM_MODE_LAST;
}

static void
external_photocam_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Photocam *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->file)
     elm_photocam_file_set(obj, p->file);
   if (p->zoom_exists)
     elm_photocam_zoom_set(obj, p->zoom);
   if (p->zoom_mode)
     {
	Elm_Photocam_Zoom_Mode set = _zoom_mode_setting_get(p->zoom_mode);
	if (set == ELM_PHOTOCAM_ZOOM_MODE_LAST) return;
	elm_photocam_zoom_mode_set(obj, set);
     }
   if (p->paused_exists)
     elm_photocam_paused_set(obj, p->paused);
}

static Eina_Bool
external_photocam_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "file"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     elm_photocam_file_set(obj, param->s);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "zoom"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
	  {
	     elm_photocam_zoom_set(obj, param->d);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "zoom mode"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     Elm_Photocam_Zoom_Mode set = _zoom_mode_setting_get(param->s);
	     if (set == ELM_PHOTOCAM_ZOOM_MODE_LAST) return EINA_FALSE;
	     elm_photocam_zoom_mode_set(obj, set);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "paused"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     elm_photocam_paused_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_photocam_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "file"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     param->s = elm_photocam_file_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "zoom"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
	  {
	     param->d = elm_photocam_zoom_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "zoom mode"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     Elm_Photocam_Zoom_Mode zoom_mode_set = elm_photocam_zoom_mode_get(obj);

	     if (zoom_mode_set == ELM_PHOTOCAM_ZOOM_MODE_LAST)
	       return EINA_FALSE;

	     param->s = choices[zoom_mode_set];
	     return EINA_TRUE;
	  }
     }
   else if(!strcmp(param->name, "paused"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     param->i = elm_photocam_paused_get(obj);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_photocam_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Photocam *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Photocam));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "file"))
	  mem->file = eina_stringshare_add(param->s);
	else if (!strcmp(param->name, "zoom"))
	  {
	     mem->zoom = param->d;
	     mem->zoom_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "zoom mode"))
	  mem->zoom_mode = eina_stringshare_add(param->s);
	else if (!strcmp(param->name, "paused"))
	  {
	     mem->paused = !!param->i;
	     mem->paused_exists = EINA_TRUE;
	  }
     }

   return mem;
}

static Evas_Object *external_photocam_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
	ERR("No content.");
	return NULL;
}

static void
external_photocam_params_free(void *params)
{
   Elm_Params_Photocam *mem = params;

   if (mem->file)
     eina_stringshare_del(mem->file);
   if (mem->zoom_mode)
     eina_stringshare_del(mem->zoom_mode);
   free(mem);
}

static Edje_External_Param_Info external_photocam_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("file"),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE("zoom"),
   EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL("zoom mode", "manual", choices),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("paused"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(photocam, "photocam");
DEFINE_EXTERNAL_TYPE_SIMPLE(photocam, "Photocam");
