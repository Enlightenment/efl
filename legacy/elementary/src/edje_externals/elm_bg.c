#include <assert.h>
#include "private.h"

typedef struct _Elm_Params_Bg
{
   Elm_Params base;
   const char *file;
   const char *option;
} Elm_Params_Bg;

#define OPTION_GET(CHOICES, STR)                                \
   unsigned int i;                                              \
   for (i = 0; i < (sizeof(CHOICES) / sizeof(CHOICES[0])); ++i) \
     if (!strcmp(STR, CHOICES[i]))                              \
       return i;

static const char *_bg_options[] = {"center", "scale", "stretch", "tile", NULL};

static Elm_Bg_Option
_bg_option_get(const char *option)
{
   assert(sizeof(_bg_options) / sizeof(_bg_options[0])
          == ELM_BG_OPTION_TILE + 2);
   OPTION_GET(_bg_options, option);
   return -1;
}

static void
external_bg_state_set(void *data __UNUSED__, Evas_Object *obj,
                      const void *from_params, const void *to_params,
                      float pos __UNUSED__)
{
   const Elm_Params_Bg *p;
   Elm_Bg_Option option;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->option)
     {
        option = _bg_option_get(p->option);
        elm_bg_option_set(obj, option);
     }
   if (p->file)
     {
        elm_bg_file_set(obj, p->file, NULL);
     }
}

static Eina_Bool
external_bg_param_set(void *data __UNUSED__, Evas_Object *obj,
                      const Edje_External_Param *param)
{
   if ((!strcmp(param->name, "file"))
       && (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING))
     {
        return elm_bg_file_set(obj, param->s, NULL);
     }
   else if ((!strcmp(param->name, "select_mode"))
            && (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING))
     {
        Elm_Bg_Option option;
        option = _bg_option_get(param->s);
        elm_bg_option_set(obj, option);
        return EINA_TRUE;
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_bg_param_get(void *data __UNUSED__,
                      const Evas_Object *obj __UNUSED__,
                      Edje_External_Param *param)
{
   if ((!strcmp(param->name, "file"))
       && (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING))
     {
        elm_bg_file_get(obj, &(param->s), NULL);
        return EINA_TRUE;
     }
   else if ((!strcmp(param->name, "option"))
            && (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING))
     {
        Elm_Bg_Option option;
        option = elm_bg_option_get(obj);
        param->s = _bg_options[option];
        return EINA_TRUE;
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_bg_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__,
                         const Eina_List *params)
{
   Elm_Params_Bg *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Bg));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "file"))
          mem->file = eina_stringshare_add(param->s);
        else if (!strcmp(param->name, "option"))
          mem->option = eina_stringshare_add(param->s);
     }

   return mem;
}

static Evas_Object *
external_bg_content_get(void *data __UNUSED__,
                        const Evas_Object *obj __UNUSED__,
                        const char *content __UNUSED__)
{
   ERR("no content");
   return NULL;
}

static void
external_bg_params_free(void *params)
{
   Elm_Params_Bg *mem = params;

   if (mem->file)
     eina_stringshare_del(mem->file);

   if (mem->option)
     eina_stringshare_del(mem->option);

   free(mem);
}

static Edje_External_Param_Info external_bg_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("file"),
   EDJE_EXTERNAL_PARAM_INFO_STRING("option"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(bg, "bg");
DEFINE_EXTERNAL_TYPE_SIMPLE(bg, "Bg");
