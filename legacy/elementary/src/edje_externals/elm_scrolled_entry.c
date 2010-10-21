#include "private.h"

typedef struct _Elm_Params_Entry
{
   Elm_Params base;
   const char *text;
   Eina_Bool text_set:1;
   Eina_Bool editable:1;
   Eina_Bool single:1;
   Eina_Bool password:1;
   Eina_Bool editable_exists:1;
   Eina_Bool single_exists:1;
   Eina_Bool password_exists:1;
} Elm_Params_Entry;

static void
external_scrolled_entry_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Entry *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if ((!p->text_set) && (p->text))
     {
	elm_scrolled_entry_entry_set(obj, p->text);
	((Elm_Params_Entry *)p)->text_set = EINA_TRUE;
     }
   if (p->editable_exists)
     elm_scrolled_entry_editable_set(obj, p->editable);
   if (p->single_exists)
     elm_scrolled_entry_single_line_set(obj, p->single);
   if (p->password_exists)
     elm_scrolled_entry_password_set(obj, p->password);
}

static Eina_Bool
external_scrolled_entry_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "text"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     elm_scrolled_entry_entry_set(obj, param->s);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "editable"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     elm_scrolled_entry_editable_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "single line"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     elm_scrolled_entry_single_line_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "password"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     elm_scrolled_entry_password_set(obj, param->i);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_scrolled_entry_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "text"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	  {
	     param->s = elm_scrolled_entry_entry_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "editable"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     param->i = elm_scrolled_entry_editable_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "single line"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     param->i = elm_scrolled_entry_single_line_get(obj);
	     return EINA_TRUE;
	  }
     }
   else if (!strcmp(param->name, "password"))
     {
	if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
	  {
	     param->i = elm_scrolled_entry_password_get(obj);
	     return EINA_TRUE;
	  }
     }

   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_scrolled_entry_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Entry *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = calloc(1, sizeof(Elm_Params_Entry));
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
	if (!strcmp(param->name, "text"))
	  mem->text = eina_stringshare_add(param->s);
	else if (!strcmp(param->name, "single line"))
	  {
	     mem->single = !!param->i;
	     mem->single_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "password"))
	  {
	     mem->password = !!param->i;
	     mem->password_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "editable"))
	  {
	     mem->editable = param->i;
	     mem->editable_exists = EINA_TRUE;
	  }
     }

   return mem;
}

static Evas_Object *external_scrolled_entry_content_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
	ERR("No content.");
	return NULL;
}

static void
external_scrolled_entry_params_free(void *params)
{
   Elm_Params_Entry *mem = params;

   if (mem->text)
     eina_stringshare_del(mem->text);
   free(mem);
}

static Edje_External_Param_Info external_scrolled_entry_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("text", "some text"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("editable"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("single line"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("password"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(scrolled_entry, "scrolled_entry")
DEFINE_EXTERNAL_TYPE_SIMPLE(scrolled_entry, "Entry");
