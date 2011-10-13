#include "private.h"

typedef struct _Elm_Params_Web
{
   Elm_Params base;
   const char *uri;
   double zoom;
   Elm_Web_Zoom_Mode zoom_mode;
   Eina_Bool inwin_mode;
   Eina_Bool zoom_set:1;
   Eina_Bool inwin_mode_set:1;
} Elm_Params_Web;

static const char *zoom_choices[] = {"manual", "auto fit", "auto fill", NULL};

static Elm_Web_Zoom_Mode
_zoom_mode_get(const char *zoom)
{
   unsigned int i;

   for (i = 0; i < ELM_WEB_ZOOM_MODE_LAST; i++)
     if (!strcmp(zoom, zoom_choices[i])) return i;

   return ELM_WEB_ZOOM_MODE_LAST;
}

static void
external_web_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Web *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->uri)
     elm_web_uri_set(obj, p->uri);
   if (p->zoom_mode < ELM_WEB_ZOOM_MODE_LAST)
     elm_web_zoom_mode_set(obj, p->zoom_mode);
   if (p->zoom_set)
     elm_web_zoom_set(obj, p->zoom);
   if (p->inwin_mode_set)
     elm_web_inwin_mode_set(obj, p->inwin_mode);
}

static Eina_Bool
external_web_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "uri"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             elm_web_uri_set(obj, param->s);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "zoom level"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             elm_web_zoom_set(obj, param->d);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "zoom mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
          {
             Elm_Web_Zoom_Mode mode = _zoom_mode_get(param->s);
             if (mode == ELM_WEB_ZOOM_MODE_LAST)
               return EINA_FALSE;
             elm_web_zoom_mode_set(obj, mode);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "inwin mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_web_inwin_mode_set(obj, !!param->i);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_web_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "uri"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             param->s = elm_web_uri_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "zoom level"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             param->d = elm_web_zoom_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "zoom mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE)
          {
             Elm_Web_Zoom_Mode mode = elm_web_zoom_mode_get(obj);
             if (mode == ELM_WEB_ZOOM_MODE_LAST)
               return EINA_FALSE;
             param->s = zoom_choices[mode];
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "inwin mode"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_web_inwin_mode_get(obj);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_web_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Web *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Web));
   if (!mem) return NULL;

   mem->zoom_mode = ELM_WEB_ZOOM_MODE_LAST;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "zoom level"))
          {
             mem->zoom = param->d;
             mem->zoom_set = EINA_TRUE;
          }
        else if (!strcmp(param->name, "zoom mode"))
          mem->zoom_mode = _zoom_mode_get(param->s);
        else if (!strcmp(param->name, "uri"))
          mem->uri = eina_stringshare_add(param->s);
        else if (!strcmp(param->name, "inwin mode"))
          {
             mem->inwin_mode = !!param->i;
             mem->inwin_mode_set = EINA_TRUE;
          }
     }

   return mem;
}

static void
external_web_params_free(void *params)
{
   Elm_Params_Web *mem = params;

   if (mem->uri)
     eina_stringshare_del(mem->uri);
   free(mem);
}

static Evas_Object *
external_web_content_get(void *data __UNUSED__, const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
   return NULL;
}

static Edje_External_Param_Info external_web_params[] =
{
   EDJE_EXTERNAL_PARAM_INFO_STRING("uri"),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE_DEFAULT("zoom level", 1.0),
   EDJE_EXTERNAL_PARAM_INFO_CHOICE_FULL("zoom mode", "manual", zoom_choices),
   EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT("inwin mode", EINA_FALSE),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

static Evas_Object *
external_web_add(void *data __UNUSED__, Evas *evas __UNUSED__, Evas_Object *edje, const Eina_List *params __UNUSED__, const char *part_name)
{
   Evas_Object *parent, *obj;
   external_elm_init();
   parent = elm_widget_parent_widget_get(edje);
   if (!parent) parent = edje;
   elm_need_web(); /* extra command needed */
   obj = elm_web_add(parent);
   external_signals_proxy(obj, edje, part_name);
   return obj;
}

DEFINE_EXTERNAL_ICON_ADD(web, "web")
DEFINE_EXTERNAL_TYPE(web, "Web")
