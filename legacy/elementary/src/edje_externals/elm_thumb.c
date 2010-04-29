#include <assert.h>

#include "private.h"

typedef struct _Elm_Params_Thumb
{
   const char *animate;
   Eina_Bool keep_aspect:1;
   Eina_Bool aspect_exists:1;
} Elm_Params_Thumb;

static const char* choices[] = {"loop", "start", "stop", NULL};

static Elm_Thumb_Animation_Setting
_anim_setting_get(const char *anim_str)
{
   unsigned int i;

   assert(sizeof(choices)/sizeof(choices[0]) == ELM_THUMB_ANIMATION_LAST + 1);

   for (i = 0; i < sizeof(choices); i++)
     {
	if (strcmp(anim_str, choices[i]) == 0)
	  return i;
     }
   return ELM_THUMB_ANIMATION_LAST;
}

static void
external_thumb_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Thumb *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->animate)
     {
	Elm_Thumb_Animation_Setting set = _anim_setting_get(p->animate);
	if (set != ELM_THUMB_ANIMATION_LAST)
	   elm_thumb_animate_set(obj, set);
     }
   if (p->aspect_exists)
     elm_thumb_keep_aspect_set(obj, p->keep_aspect);
}

static Eina_Bool
external_thumb_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "animate"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
	  {
	     Elm_Thumb_Animation_Setting set = _anim_setting_get(param->s);
	     if (set == ELM_THUMB_ANIMATION_LAST) return EINA_FALSE;
	     elm_thumb_animate_set(obj, set);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "aspect"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     elm_thumb_keep_aspect_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_thumb_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "animate"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
	  {
	     Elm_Thumb_Animation_Setting anim_set = elm_thumb_animate_get(obj);

	     if (anim_set == ELM_THUMB_ANIMATION_LAST)
	       return EINA_FALSE;

	     param->s = choices[anim_set];
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "aspect"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     param->i = elm_thumb_keep_aspect_get(obj);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_thumb_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Thumb *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Thumb));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "animate"))
	  mem->animate = eina_stringshare_add(param->s);
	else if (!strcmp(param->name, "aspect"))
	  {
	     mem->keep_aspect = !!param->i;
	     mem->aspect_exists = EINA_TRUE;
	  }
     }

   return mem;
}

static void
external_thumb_params_free(void *params)
{
   Elm_Params_Thumb *mem = params;

   if (mem->animate)
     eina_stringshare_del(mem->animate);
   free(mem);
}

static Edje_External_Param_Info external_thumb_params[] =
  {
    EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL("animate", "loop", choices),
    EDJE_EXTERNAL_PARAM_INFO_BOOL("keep aspect"),
    EDJE_EXTERNAL_PARAM_INFO_SENTINEL
  };

DEFINE_EXTERNAL_ICON_ADD(thumb, "thumb")

static Evas_Object *
external_thumb_add(void *data __UNUSED__, Evas *evas __UNUSED__, Evas_Object *edje, const Eina_List *params __UNUSED__, const char *part_name)
{
   Evas_Object *parent, *obj;
   parent = elm_widget_parent_widget_get(edje);
   if (!parent) parent = edje;
   elm_need_ethumb(); /* extra command needed */
   obj = elm_thumb_add(parent);
   external_signals_proxy(obj, edje, part_name);
   return obj;
}
DEFINE_EXTERNAL_TYPE(thumb, "Thumbnail")
