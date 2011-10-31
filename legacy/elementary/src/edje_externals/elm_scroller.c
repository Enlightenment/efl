#include "private.h"
#include <assert.h>


typedef struct _Elm_Params_Scroller Elm_Params_Scroller;

struct _Elm_Params_Scroller {
	Elm_Params base;
	Evas_Object *content;
};



static void external_scroller_state_set(void *data __UNUSED__,
		Evas_Object *obj, const void *from_params,
		const void *to_params, float pos __UNUSED__)
{
	const Elm_Params_Scroller *p;

	if (to_params) p = to_params;
	else if (from_params) p = from_params;
	else return;

	if (p->content) {
		elm_object_content_set(obj, p->content);
	}
}

static Eina_Bool external_scroller_param_set(void *data __UNUSED__,
		Evas_Object *obj, const Edje_External_Param *param)
{
	if (!strcmp(param->name, "content")
			&& param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING)
	{
		Evas_Object *content = external_common_param_edje_object_get(obj, param);
		if ((strcmp(param->s, "")) && (!content))
			return EINA_FALSE;
		elm_object_content_set(obj, content);
		return EINA_TRUE;
	}

	ERR("unknown parameter '%s' of type '%s'",
			param->name, edje_external_param_type_str(param->type));

	return EINA_FALSE;
}

static Eina_Bool external_scroller_param_get(void *data __UNUSED__,
		const Evas_Object *obj __UNUSED__, Edje_External_Param *param)
{
	if (!strcmp(param->name, "content"))
	{
		/* not easy to get content name back from live object */
		return EINA_FALSE;
	}

	ERR("unknown parameter '%s' of type '%s'",
			param->name, edje_external_param_type_str(param->type));

	return EINA_FALSE;
}

static void * external_scroller_params_parse(void *data __UNUSED__, Evas_Object *obj,
		const Eina_List *params) {
	Elm_Params_Scroller *mem;
	Edje_External_Param *param;
	const Eina_List *l;

        mem = ELM_NEW(Elm_Params_Scroller);
	if (!mem)
		return NULL;

	EINA_LIST_FOREACH(params, l, param)
	{
		if (!strcmp(param->name, "content"))
			mem->content = external_common_param_edje_object_get(obj, param);
	}

	return mem;
}

static Evas_Object *external_scroller_content_get(void *data __UNUSED__,
		const Evas_Object *obj, const char *content)
{
	if (!strcmp(content, "content"))
		return elm_object_content_get(obj);

	ERR("unknown content '%s'", content);
	return NULL;
}

static void external_scroller_params_free(void *params) {
	external_common_params_free(params);
}

static Edje_External_Param_Info external_scroller_params[] = {
		DEFINE_EXTERNAL_COMMON_PARAMS,
		EDJE_EXTERNAL_PARAM_INFO_STRING("content"),
		EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(scroller, "scroller");
DEFINE_EXTERNAL_TYPE_SIMPLE(scroller, "Scroller")
;
