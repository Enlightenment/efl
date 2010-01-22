#include "private.h"

typedef struct _Elm_Params_Entry
{
   const char *text;
   int single, password;
} Elm_Params_Entry;

static void
external_scrolled_entry_state_set(void *data, Evas_Object *obj, const void *from_params, const void *to_params, float pos)
{
   const Elm_Params_Entry *p1 = from_params, *p2 = to_params;

   p1 = from_params;
   p2 = to_params;

   if (!p2)
     {
	elm_scrolled_entry_entry_set(obj, p1->text);
	elm_scrolled_entry_single_line_set(obj, p1->single);
	elm_scrolled_entry_password_set(obj, p1->password);
	return;
     }

   elm_scrolled_entry_entry_set(obj, p2->text);
   elm_scrolled_entry_single_line_set(obj, p2->single);
   elm_scrolled_entry_password_set(obj, p2->password);
}

static void *
external_scrolled_entry_params_parse(void *data, Evas_Object *obj, const Eina_List *params)
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
	  mem->single = param->i;
	else if (!strcmp(param->name, "password"))
	  mem->password = param->i;
     }

   return mem;
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
   EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("text", "some text"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("single line"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("password"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(scrolled_entry, "scrolled_entry")
DEFINE_EXTERNAL_TYPE_SIMPLE(scrolled_entry, "Entry");
