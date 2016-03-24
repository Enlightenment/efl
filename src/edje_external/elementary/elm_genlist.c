#include <assert.h>
#include "private.h"

typedef struct _Elm_Params_Genlist
{
   Elm_Params base;
   const char *horizontal;
   Eina_Bool multi:1;
   Eina_Bool multi_exists:1;
   Eina_Bool always_select:1;
   Eina_Bool always_select_exists:1;
   Eina_Bool no_select:1;
   Eina_Bool no_select_exists:1;
   Eina_Bool compress_exists:1;
   Eina_Bool homogeneous:1;
   Eina_Bool homogeneous_exists:1;
   Eina_Bool h_bounce:1;
   Eina_Bool h_bounce_exists:1;
   Eina_Bool v_bounce:1;
   Eina_Bool v_bounce_exists:1;
} Elm_Params_Genlist;

static const char* list_horizontal_choices[] =
{
   "compress", "scroll", "limit", "expand",
   NULL
};

static Elm_List_Mode
_list_horizontal_setting_get(const char *horizontal_str)
{
   unsigned int i;

   assert(sizeof(list_horizontal_choices) / sizeof(list_horizontal_choices[0])
          == ELM_LIST_LAST + 1);

   for (i = 0; i < ELM_LIST_LAST; i++)
     {
        if (!strcmp(horizontal_str, list_horizontal_choices[i]))
          return i;
     }
   return ELM_LIST_LAST;
}

static void
external_genlist_state_set(void *data EINA_UNUSED, Evas_Object *obj,
                           const void *from_params, const void *to_params,
                           float pos EINA_UNUSED)
{
   const Elm_Params_Genlist *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->horizontal)
     {
        Elm_List_Mode set = _list_horizontal_setting_get(p->horizontal);

        if (set != ELM_LIST_LAST)
          elm_genlist_mode_set(obj, set);
     }
   if (p->multi_exists)
     elm_genlist_multi_select_set(obj, p->multi);
   if (p->no_select_exists)
     {
        if (p->no_select)
          elm_genlist_select_mode_set (obj, ELM_OBJECT_SELECT_MODE_NONE);
        else
          elm_genlist_select_mode_set (obj, ELM_OBJECT_SELECT_MODE_DEFAULT);
     }
   if (p->always_select_exists)
     {
        if (p->always_select)
          elm_genlist_select_mode_set (obj, ELM_OBJECT_SELECT_MODE_ALWAYS);
        else
          elm_genlist_select_mode_set (obj, ELM_OBJECT_SELECT_MODE_DEFAULT);
     }
   if (p->homogeneous_exists)
     elm_genlist_homogeneous_set(obj, p->homogeneous);
   if ((p->h_bounce_exists) && (p->v_bounce_exists))
     elm_scroller_bounce_set(obj, p->h_bounce, p->v_bounce);
   else if ((p->h_bounce_exists) || (p->v_bounce_exists))
     {
        Eina_Bool h_bounce, v_bounce;

        elm_scroller_bounce_get(obj, &h_bounce, &v_bounce);
        if (p->h_bounce_exists)
          elm_scroller_bounce_set(obj, p->h_bounce, v_bounce);
        else
          elm_scroller_bounce_set(obj, h_bounce, p->v_bounce);
     }
}

static Eina_Bool
external_genlist_param_set(void *data EINA_UNUSED, Evas_Object *obj,
                           const Edje_External_Param *param)
{
   if (!strcmp(param->name, "horizontal mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
          {
             Elm_List_Mode set = _list_horizontal_setting_get(param->s);

             if (set == ELM_LIST_LAST) return EINA_FALSE;
             elm_genlist_mode_set(obj, set);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "multi select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_genlist_multi_select_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "always select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             if (param->i)
               elm_genlist_select_mode_set (obj, ELM_OBJECT_SELECT_MODE_ALWAYS);
             else
               elm_genlist_select_mode_set (obj, ELM_OBJECT_SELECT_MODE_DEFAULT);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "no select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             if (param->i)
               elm_genlist_select_mode_set (obj, ELM_OBJECT_SELECT_MODE_NONE);
             else
               elm_genlist_select_mode_set (obj, ELM_OBJECT_SELECT_MODE_DEFAULT);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "homogeneous"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_genlist_homogeneous_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "height bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool h_bounce, v_bounce;
             elm_scroller_bounce_get(obj, &h_bounce, &v_bounce);
             elm_scroller_bounce_set(obj, param->i, v_bounce);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "width bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool h_bounce, v_bounce;
             elm_scroller_bounce_get(obj, &h_bounce, &v_bounce);
             elm_scroller_bounce_set(obj, h_bounce, param->i);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_genlist_param_get(void *data EINA_UNUSED, const Evas_Object *obj,
                           Edje_External_Param *param)
{
   if (!strcmp(param->name, "horizontal mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
          {
             Elm_List_Mode list_horizontal_set = elm_genlist_mode_get(obj);

             if (list_horizontal_set == ELM_LIST_LAST)
               return EINA_FALSE;

             param->s = list_horizontal_choices[list_horizontal_set];
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "multi select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_genlist_multi_select_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "always select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             if (elm_genlist_select_mode_get (obj) ==
                 ELM_OBJECT_SELECT_MODE_ALWAYS)
               param->i = EINA_TRUE;
             else
               param->i = EINA_FALSE;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "no select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             if (elm_genlist_select_mode_get (obj) ==
                 ELM_OBJECT_SELECT_MODE_NONE)
               param->i = EINA_TRUE;
             else
               param->i = EINA_FALSE;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "homogeneous"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_genlist_homogeneous_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "height bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool h_bounce, v_bounce;
             elm_scroller_bounce_get(obj, &h_bounce, &v_bounce);
             param->i = h_bounce;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "width bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool h_bounce, v_bounce;
             elm_scroller_bounce_get(obj, &h_bounce, &v_bounce);
             param->i = v_bounce;
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_genlist_params_parse(void *data EINA_UNUSED,
                              Evas_Object *obj EINA_UNUSED,
                              const Eina_List *params)
{
   Elm_Params_Genlist *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = ELM_NEW(Elm_Params_Genlist);
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "horizontal mode"))
          mem->horizontal = eina_stringshare_add(param->s);
        else if (!strcmp(param->name, "multi select"))
          {
             mem->multi = !!param->i;
             mem->multi_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "always select"))
          {
             mem->always_select = !!param->i;
             mem->always_select_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "no select"))
          {
             mem->no_select = !!param->i;
             mem->no_select_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "homogeneous"))
          {
             mem->homogeneous = !!param->i;
             mem->homogeneous_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "height bounce"))
          {
             mem->h_bounce = !!param->i;
             mem->h_bounce_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "width bounce"))
          {
             mem->v_bounce = !!param->i;
             mem->v_bounce_exists = EINA_TRUE;
          }
     }

   return mem;
}

static Evas_Object *external_genlist_content_get(void *data EINA_UNUSED,
                                                 const Evas_Object *obj EINA_UNUSED,
                                                 const char *content EINA_UNUSED)
{
   ERR("No content.");
   return NULL;
}

static void
external_genlist_params_free(void *params)
{
   Elm_Params_Genlist *mem = params;

   if (mem->horizontal)
     eina_stringshare_del(mem->horizontal);

   free(mem);
}

static Edje_External_Param_Info external_genlist_params[] = {
     DEFINE_EXTERNAL_COMMON_PARAMS,
     EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL("horizontal mode", "scroll",
                                          list_horizontal_choices),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("multi select"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("always select"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("no select"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("homogeneous"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("height bounce"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("width bounce"),
     EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(genlist, "genlist");
DEFINE_EXTERNAL_TYPE_SIMPLE(genlist, "Generic List");
