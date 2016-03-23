#include <assert.h>
#include "private.h"

typedef struct _Elm_Params_Fileselector
{
   Elm_Params base;
   Eina_Bool is_save:1;
   Eina_Bool is_save_set:1;
   Eina_Bool folder_only:1;
   Eina_Bool folder_only_set:1;
   Eina_Bool show_buttons:1;
   Eina_Bool show_buttons_set:1;
   Eina_Bool expandable:1;
   Eina_Bool expandable_set:1;
} Elm_Params_Fileselector;

static void
external_fileselector_state_set(void *data EINA_UNUSED, Evas_Object *obj,
                                const void *from_params, const void *to_params,
                                float pos EINA_UNUSED)
{
   const Elm_Params_Fileselector *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if ((p->is_save_set) && (p->is_save))
     elm_fileselector_is_save_set(obj, p->is_save);
   if (p->folder_only_set)
     elm_fileselector_folder_only_set(obj, p->folder_only);
   if (p->show_buttons_set)
     elm_fileselector_buttons_ok_cancel_set(obj, p->show_buttons);
   if (p->expandable_set)
     elm_fileselector_expandable_set(obj, p->expandable);
}

static Eina_Bool
external_fileselector_param_set(void *data EINA_UNUSED, Evas_Object *obj,
                                const Edje_External_Param *param)
{
   if (!strcmp(param->name, "save"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_fileselector_is_save_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "folder only"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_fileselector_folder_only_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "show buttons"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_fileselector_buttons_ok_cancel_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "expandable"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_fileselector_expandable_set(obj, param->i);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_fileselector_param_get(void *data EINA_UNUSED, const Evas_Object *obj,
                                Edje_External_Param *param)
{
   if (!strcmp(param->name, "save"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_fileselector_is_save_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "folder only"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_fileselector_folder_only_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "show buttons"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_fileselector_buttons_ok_cancel_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "expandable"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_fileselector_expandable_get(obj);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_fileselector_params_parse(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const Eina_List *params)
{
   Elm_Params_Fileselector *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Fileselector));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "save"))
          {
             mem->is_save = !!param->i;
             mem->is_save_set = EINA_TRUE;
          }
        else if (!strcmp(param->name, "folder only"))
          {
             mem->folder_only = !!param->i;
             mem->folder_only_set = EINA_TRUE;
          }
        else if (!strcmp(param->name, "show buttons"))
          {
             mem->show_buttons = !!param->i;
             mem->show_buttons_set = EINA_TRUE;
          }
        else if (!strcmp(param->name, "expandable"))
          {
             mem->expandable = !!param->i;
             mem->expandable_set = EINA_TRUE;
          }
     }

   return mem;
}

static Evas_Object *external_fileselector_content_get(void *data EINA_UNUSED,
                                                      const Evas_Object *obj EINA_UNUSED,
                                                      const char *content EINA_UNUSED)
{
   ERR("No content.");
   return NULL;
}

static void
external_fileselector_params_free(void *params)
{
   Elm_Params_Fileselector *mem = params;
   free(mem);
}

static Edje_External_Param_Info external_fileselector_params[] =
{
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_BOOL("save"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("folder only"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("show buttons"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("expandable"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(fileselector, "fileselector");
DEFINE_EXTERNAL_TYPE_SIMPLE(fileselector, "Fileselector");
