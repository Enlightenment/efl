#include "private.h"

typedef struct _Elm_Params_Slideshow
{
   Elm_Params base;
   double timeout;
   const char *transition;
   const char *layout;
   Eina_Bool loop:1;
   Eina_Bool timeout_exists:1;
   Eina_Bool loop_exists:1;
} Elm_Params_Slideshow;

static const char *transitions[] =
{
   "fade", "black_fade", "horizontal", "vertical", "square", NULL
};
static const char *layout[] = { "fullscreen", "not_fullscreen", NULL };

static void
external_slideshow_state_set(void *data EINA_UNUSED, Evas_Object *obj,
                             const void *from_params, const void *to_params,
                             float pos EINA_UNUSED)
{
   const Elm_Params_Slideshow *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->timeout_exists)
     elm_slideshow_timeout_set(obj , p->timeout);
   if (p->loop_exists)
     elm_slideshow_loop_set(obj, p->loop);
   if (p->transition)
     elm_slideshow_transition_set(obj, p->transition);
   if (p->layout)
     elm_slideshow_layout_set(obj, p->layout);
}

static Eina_Bool
external_slideshow_param_set(void *data EINA_UNUSED, Evas_Object *obj,
                             const Edje_External_Param *param)
{
   if (!strcmp(param->name, "timeout"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             elm_slideshow_timeout_set(obj, param->d);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "loop"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_slideshow_loop_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "transition"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             elm_slideshow_transition_set(obj, param->s);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "layout"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             elm_slideshow_layout_set(obj, param->s);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_slideshow_param_get(void *data EINA_UNUSED, const Evas_Object *obj,
                             Edje_External_Param *param)
{
   if (!strcmp(param->name, "timeout"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             param->d = elm_slideshow_timeout_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "loop"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_slideshow_loop_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "transition"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             param->s = elm_slideshow_transition_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "layout"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             param->s = elm_slideshow_layout_get(obj);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_slideshow_params_parse(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const Eina_List *params)
{
   Elm_Params_Slideshow *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Slideshow));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "timeout"))
          {
             mem->timeout = param->d;
             mem->timeout_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "loop"))
          {
             mem->loop = param->i;
             mem->loop_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "transition"))
          {
             mem->transition = param->s;
          }
        else if (!strcmp(param->name, "layout"))
          {
             mem->layout = param->s;
          }
     }

   return mem;
}

static Evas_Object *external_slideshow_content_get(void *data EINA_UNUSED,
                                                   const Evas_Object *obj EINA_UNUSED,
                                                   const char *content EINA_UNUSED)
{
   ERR("No content.");
   return NULL;
}

static void
external_slideshow_params_free(void *params EINA_UNUSED)
{
   return;
}

static Edje_External_Param_Info external_slideshow_params[] = {
     DEFINE_EXTERNAL_COMMON_PARAMS,
     EDJE_EXTERNAL_PARAM_INFO_DOUBLE("timeout"),
     EDJE_EXTERNAL_PARAM_INFO_BOOL("loop"),
     EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL("transition", "fade", transitions),
     EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL("layout", "fullscreen", layout),
     EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(slideshow, "slideshow");
DEFINE_EXTERNAL_TYPE_SIMPLE(slideshow, "Slideshow");
