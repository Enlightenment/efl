#include <assert.h>

#include "private.h"

typedef struct _Elm_Params_Toolbar
{
   Elm_Params base;
   int icon_size;
   Eina_Bool icon_size_exists:1;
   double align;
   const char *shrink_mode;
   Eina_Bool align_exists:1;
   Eina_Bool always_select:1;
   Eina_Bool always_select_exists:1;
   Eina_Bool no_select:1;
   Eina_Bool no_select_exists:1;
   Eina_Bool horizontal:1;
   Eina_Bool horizontal_exists:1;
   Eina_Bool homogeneous:1;
   Eina_Bool homogeneous_exists:1;
} Elm_Params_Toolbar;

#define SHRINK_GET(CHOICES, STR)         \
   unsigned int i;                       \
   for (i = 0; i < (sizeof(CHOICES)/sizeof(CHOICES[0])); ++i) \
     if (!strcmp(STR, CHOICES[i]))       \
       return i;

static const char *_toolbar_shrink_modes[] = {"none", "hide",
                                              "scroll", "menu", NULL};

static Elm_Toolbar_Shrink_Mode
_toolbar_shrink_choices_setting_get(const char *shrink_mode_str)
{
   assert(sizeof(_toolbar_shrink_modes) /
          sizeof(_toolbar_shrink_modes[0]) == ELM_TOOLBAR_SHRINK_LAST + 1);
   SHRINK_GET(_toolbar_shrink_modes, shrink_mode_str);
   return ELM_TOOLBAR_SHRINK_LAST;
}

static void
external_toolbar_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Toolbar *p;
   Elm_Toolbar_Shrink_Mode shrink_mode;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->icon_size_exists)
     elm_toolbar_icon_size_set(obj, p->icon_size);
   if (p->align_exists)
     elm_toolbar_align_set(obj, p->align);
   if (p->always_select_exists)
     elm_toolbar_always_select_mode_set(obj, p->always_select);
   if (p->no_select_exists)
     elm_toolbar_no_select_mode_set(obj, p->no_select);
   if (p->horizontal_exists)
     elm_toolbar_horizontal_set(obj, p->horizontal);
   if (p->homogeneous_exists)
     elm_toolbar_homogeneous_set(obj, p->homogeneous);
   if (p->shrink_mode)
     {
        shrink_mode = _toolbar_shrink_choices_setting_get(p->shrink_mode);
        elm_toolbar_mode_shrink_set(obj, shrink_mode);
     }
}

static Eina_Bool
external_toolbar_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   Elm_Toolbar_Shrink_Mode shrink_mode;

   if (!strcmp(param->name, "icon size"))
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
   else if (!strcmp(param->name, "always select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_toolbar_always_select_mode_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "no select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_toolbar_no_select_mode_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_toolbar_horizontal_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "homogeneous"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_toolbar_homogeneous_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "shrink"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             shrink_mode = _toolbar_shrink_choices_setting_get(param->s);
             elm_toolbar_mode_shrink_set(obj, shrink_mode);
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
   if (!strcmp(param->name, "icon size"))
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
   else if (!strcmp(param->name, "always select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->d = elm_toolbar_always_select_mode_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "no select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_toolbar_no_select_mode_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_toolbar_horizontal_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "homogeneous"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_toolbar_homogeneous_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "shrink"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             Elm_Toolbar_Shrink_Mode shrink_mode;
             shrink_mode = elm_toolbar_mode_shrink_get(obj);
             param->s = _toolbar_shrink_modes[shrink_mode];
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
        if (!strcmp(param->name, "icon size"))
          {
             mem->icon_size = param->i;
             mem->icon_size_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "align"))
          {
             mem->align = param->d;
             mem->align_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "always select"))
          {
             mem->always_select = param->i;
             mem->always_select_exists = param->i;
          }
        else if (!strcmp(param->name, "no select"))
          {
             mem->no_select = param->i;
             mem->no_select_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "horizontal"))
          {
             mem->horizontal = param->i;
             mem->horizontal_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "homogeneous"))
          {
             mem->homogeneous = param->i;
             mem->homogeneous_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "shrink"))
          mem->shrink_mode = eina_stringshare_add(param->s);
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
   if (mem->shrink_mode)
     eina_stringshare_del(mem->shrink_mode);
   free(mem);
}

static Edje_External_Param_Info external_toolbar_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("shrink"),
   EDJE_EXTERNAL_PARAM_INFO_INT("icon size"),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE("align"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("always select"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("no select"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("horizontal"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("homogeneous"),

   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(toolbar, "toolbar");
DEFINE_EXTERNAL_TYPE_SIMPLE(toolbar, "Toolbar");
