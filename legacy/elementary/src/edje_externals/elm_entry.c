#include "private.h"

typedef struct _Elm_Params_Entry
{
   Elm_Params base;
   const char *label;
   const char *entry;
   Evas_Object *icon;
   Eina_Bool scrollable:1;
   Eina_Bool scrollable_exists:1;
   Eina_Bool single_line:1;
   Eina_Bool single_line_exists:1;
   Eina_Bool password:1;
   Eina_Bool password_exists:1;
   Eina_Bool horizontal_bounce:1;
   Eina_Bool horizontal_bounce_exists:1;
   Eina_Bool vertical_bounce:1;
   Eina_Bool vertical_bounce_exists:1;
   Eina_Bool editable:1;
   Eina_Bool editable_exists:1;
} Elm_Params_Entry;

static void
external_entry_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Entry *p;
   Eina_Bool hbounce, vbounce;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->label)
     elm_object_text_set(obj, p->label);
   if (p->icon)
     elm_entry_icon_set(obj, p->icon);
   if (p->entry)
     elm_entry_entry_set(obj, p->entry);
   if (p->scrollable_exists)
     elm_entry_scrollable_set(obj, p->scrollable);
   if (p->single_line_exists)
     elm_entry_single_line_set(obj, p->single_line);
   if (p->password_exists)
     elm_entry_password_set(obj, p->password);
   if (p->horizontal_bounce_exists && p->vertical_bounce_exists)
     elm_entry_bounce_set(obj, p->horizontal_bounce, p->vertical_bounce);
   else if (p->horizontal_bounce_exists || p->vertical_bounce_exists)
     {
        elm_entry_bounce_get(obj, &hbounce, &vbounce);
        if (p->horizontal_bounce_exists)
          elm_entry_bounce_set(obj, p->horizontal_bounce, vbounce);
        else
          elm_entry_bounce_set(obj, hbounce, p->vertical_bounce);
     }
   if (p->editable_exists)
     elm_entry_editable_set(obj, p->editable);
}

static Eina_Bool
external_entry_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "label"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             elm_object_text_set(obj, param->s);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "icon"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             Evas_Object *icon = external_common_param_icon_get(obj, param);
             elm_entry_icon_set(obj, icon);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "entry"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             elm_entry_entry_set(obj, param->s);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "scrollable"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_entry_scrollable_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "single line"))
     {
        if(param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_entry_single_line_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "password"))
     {
        if(param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_entry_password_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool hbounce, vbounce;
             elm_entry_bounce_get(obj, NULL, &vbounce);
             hbounce = !!param->i;
             elm_entry_bounce_set(obj, hbounce, vbounce);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "vertical bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool hbounce, vbounce;
             elm_entry_bounce_get(obj, &hbounce, NULL);
             vbounce = !!param->i;
             elm_entry_bounce_set(obj, hbounce, vbounce);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "editable"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_entry_editable_set(obj, param->i);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_entry_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "label"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             param->s = elm_object_text_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "icon"))
     {
        /* not easy to get icon name back from live object */
        return EINA_FALSE;
     }
   else if (!strcmp(param->name, "entry"))
     {
        if (param->type ==  EDJE_EXTERNAL_PARAM_TYPE_STRING)
          {
             param->s = elm_entry_entry_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "scrollable"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_entry_scrollable_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "single line"))
     {
        if (param->type ==  EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_entry_single_line_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "password"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_entry_password_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool hbounce;
             elm_entry_bounce_get(obj, &hbounce, NULL);
             param->i = hbounce;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "vertical bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool vbounce;
             elm_entry_bounce_get(obj, NULL, &vbounce);
             param->i = vbounce;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "editable"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_entry_editable_get(obj);
             return EINA_TRUE;
          }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_entry_params_parse(void *data __UNUSED__, Evas_Object *obj, const Eina_List *params)
{
   Elm_Params_Entry *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = ELM_NEW(Elm_Params_Entry);
   if (!mem)
     return NULL;

   external_common_icon_param_parse(&mem->icon, obj, params);

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "label"))
          {
             mem->label = eina_stringshare_add(param->s);
          }
        else if (!strcmp(param->name, "entry"))
          {
             mem->entry = eina_stringshare_add(param->s);
          }
        else if (!strcmp(param->name, "scrollable"))
          {
             mem->scrollable = !!param->i;
             mem->scrollable_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "single line"))
          {
             mem->single_line = !!param->i;
             mem->single_line_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "password"))
          {
             mem->password = !!param->i;
             mem->password_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "horizontal bounce"))
          {
             mem->horizontal_bounce = !!param->i;
             mem->horizontal_bounce_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "vertical bounce"))
          {
             mem->vertical_bounce = !!param->i;
             mem->vertical_bounce_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "editable"))
          {
             mem->editable = !!param->i;
             mem->editable_exists = EINA_TRUE;
          }
     }

   return mem;
}

static Evas_Object *external_entry_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
	ERR("No content.");
	return NULL;
}

static void
external_entry_params_free(void *params)
{
   Elm_Params_Entry *mem = params;
   if (mem->label)
     eina_stringshare_del(mem->label);
   if (mem->entry)
     eina_stringshare_del(mem->entry);
   free(params);
}

static Edje_External_Param_Info external_entry_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING("label"),
   EDJE_EXTERNAL_PARAM_INFO_STRING("icon"),
   EDJE_EXTERNAL_PARAM_INFO_STRING("entry"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("scrollable"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("single line"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("password"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("horizontal bounce"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("vertical bounce"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("editable"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(entry, "entry");
DEFINE_EXTERNAL_TYPE_SIMPLE(entry, "Entry");
