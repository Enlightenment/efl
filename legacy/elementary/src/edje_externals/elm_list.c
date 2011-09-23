#include <assert.h>

#include "private.h"

typedef struct _Elm_Params_List
{
   Elm_Params base;
   const char *policy_h;
   const char *policy_v;
   const char *mode;
   Eina_Bool h_mode : 1;
   Eina_Bool h_mode_exists : 1;
   Eina_Bool multi : 1;
   Eina_Bool multi_exists : 1;
   Eina_Bool always_select : 1;
   Eina_Bool always_select_exists : 1;
} Elm_Params_List;

#define CHOICE_GET(CHOICES, STR)                \
  unsigned int i;                               \
  for (i = 0; i < (sizeof(CHOICES)/sizeof(CHOICES[0])); i++)         \
    if (strcmp(STR, CHOICES[i]) == 0)           \
      return i

static const char *scroller_policy_choices[] = {"auto", "on", "off", NULL};
static const char *list_mode_choices[] = {"compress", "scroll", "limit",
                                          "expand", NULL};

static Elm_Scroller_Policy
_scroller_policy_choices_setting_get(const char *policy_str)
{
   assert(sizeof(scroller_policy_choices)/
          sizeof(scroller_policy_choices[0]) == ELM_SCROLLER_POLICY_LAST + 1);
   CHOICE_GET(scroller_policy_choices, policy_str);
   return ELM_SCROLLER_POLICY_LAST;
}

static Elm_List_Mode
_list_mode_setting_get(const char *mode_str)
{
   assert(sizeof(list_mode_choices)/sizeof(list_mode_choices[0]) ==
          ELM_LIST_LAST + 1);
   CHOICE_GET(list_mode_choices, mode_str);
   return ELM_LIST_LAST;
}

static void
external_list_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_List *p;
   Elm_Scroller_Policy policy_h, policy_v;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->mode)
     {
        Elm_List_Mode set = _list_mode_setting_get(p->mode);

        if (set != ELM_LIST_LAST)
          elm_list_mode_set(obj, set);
     }

   if ((p->policy_h) && (p->policy_v))
     {
        policy_h = _scroller_policy_choices_setting_get(p->policy_h);
        policy_v = _scroller_policy_choices_setting_get(p->policy_v);
        elm_list_scroller_policy_set(obj, policy_h, policy_v);
     }
   else if ((p->policy_h) || (p->policy_v))
     {
        elm_list_scroller_policy_get(obj, &policy_h, &policy_v);
        if (p->policy_h)
          {
             policy_h = _scroller_policy_choices_setting_get(p->policy_h);
             elm_list_scroller_policy_set(obj, policy_h, policy_v);
          }
        else
          {
             policy_v = _scroller_policy_choices_setting_get(p->policy_v);
             elm_list_scroller_policy_set(obj, policy_h, policy_v);
          }
     }

   if (p->h_mode_exists)
     elm_list_horizontal_set(obj, p->h_mode);
   if (p->multi_exists)
     elm_list_multi_select_set(obj, p->multi);
   if (p->always_select_exists)
     elm_list_always_select_mode_set(obj, p->always_select);
}

static Eina_Bool
external_list_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "list mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
          {
             Elm_List_Mode set = _list_mode_setting_get(param->s);
             if (set == ELM_LIST_LAST) return EINA_FALSE;
             elm_list_mode_set(obj, set);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal scroll"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
          {
             Elm_Scroller_Policy h, v;
             elm_list_scroller_policy_get(obj, &h, &v);
             h = _scroller_policy_choices_setting_get(param->s);
             if (h == ELM_SCROLLER_POLICY_LAST) return EINA_FALSE;
             elm_list_scroller_policy_set(obj, h, v);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "vertical scroll"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
          {
             Elm_Scroller_Policy h, v;
             elm_list_scroller_policy_get(obj, &h, &v);
             v = _scroller_policy_choices_setting_get(param->s);
             if (v == ELM_SCROLLER_POLICY_LAST) return EINA_FALSE;
             elm_list_scroller_policy_set(obj, h, v);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_list_horizontal_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "multi-select mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_list_multi_select_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "always-select mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_list_always_select_mode_set(obj, param->i);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_list_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "horizontal mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_list_horizontal_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "multi-select mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_list_multi_select_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "always-select mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_list_always_select_mode_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal scroll"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
          {
             Elm_Scroller_Policy h, v;
             elm_list_scroller_policy_get(obj, &h, &v);

             param->s = scroller_policy_choices[h];
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "vertical scroll"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
          {
             Elm_Scroller_Policy h, v;
             elm_list_scroller_policy_get(obj, &h, &v);

             param->s = scroller_policy_choices[v];
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "list mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
          {
             Elm_List_Mode m = elm_list_mode_get(obj);

             if (m == ELM_LIST_LAST)
               return EINA_FALSE;

             param->s = list_mode_choices[m];
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_list_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_List *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = ELM_NEW(Elm_Params_List);
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "horizontal mode"))
          {
             mem->h_mode = param->i;
             mem->h_mode_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "multi-select mode"))
          {
             mem->multi = param->i;
             mem->multi_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "always-select mode"))
          {
             mem->always_select = param->i;
             mem->always_select_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "horizontal scroll"))
          mem->policy_h = eina_stringshare_add(param->s);
        else if (!strcmp(param->name, "vertical scroll"))
          mem->policy_v = eina_stringshare_add(param->s);
        else if (!strcmp(param->name, "list mode"))
          mem->mode = eina_stringshare_add(param->s);
     }
   return mem;
}

static Evas_Object *external_list_content_get(void *data __UNUSED__, const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
   ERR("No content.");
   return NULL;
}

static void
external_list_params_free(void *params)
{
   Elm_Params_List *mem = params;

   if (mem->mode)
     eina_stringshare_del(mem->mode);
   if (mem->policy_h)
     eina_stringshare_del(mem->policy_h);
   if (mem->policy_v)
     eina_stringshare_del(mem->policy_v);

   free(mem);
}

static Edje_External_Param_Info external_list_params[] = {
  DEFINE_EXTERNAL_COMMON_PARAMS,
  EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL("list mode", "scroll",
                                       list_mode_choices),
  EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL("horizontal scroll", "auto",
                                       scroller_policy_choices),
  EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL("vertical scroll", "auto",
                                       scroller_policy_choices),
  EDJE_EXTERNAL_PARAM_INFO_BOOL("horizontal mode"),
  EDJE_EXTERNAL_PARAM_INFO_BOOL("multi-select mode"),
  EDJE_EXTERNAL_PARAM_INFO_BOOL("always-select mode"),
  EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(list, "list")
DEFINE_EXTERNAL_TYPE_SIMPLE(list, "List");
