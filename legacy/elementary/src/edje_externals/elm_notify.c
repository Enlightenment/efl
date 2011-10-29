#include "private.h"
#include <assert.h>


typedef struct _Elm_Params_Notify Elm_Params_Notify;

struct _Elm_Params_Notify {
	Elm_Params base;
	Evas_Object *content; /* part name whose obj is to be set as content */
	Eina_Bool repeat_events_exists;
	Eina_Bool repeat_events;
	Eina_Bool timeout_exists;
	double timeout;

	const char *orient;
};


static const char *orients[] = {
		"top",
		"center",
		"bottom",
		"left",
		"right",
		"top_left",
		"top_right",
		"bottom_left",
		"bottom_right",
		NULL
};

static Elm_Notify_Orient _orient_get(const char *orient)
{
   unsigned int i;

   assert(sizeof(orients)/sizeof(orients[0]) ==
	  ELM_NOTIFY_ORIENT_LAST + 1);

   for (i = 0; i < ELM_NOTIFY_ORIENT_LAST; i++)
     if (!strcmp(orient, orients[i])) return i;

   return ELM_NOTIFY_ORIENT_LAST;
}

static void external_notify_state_set(void *data __UNUSED__,
		Evas_Object *obj, const void *from_params,
		const void *to_params, float pos __UNUSED__)
{
	const Elm_Params_Notify *p;

	if (to_params) p = to_params;
	else if (from_params) p = from_params;
	else return;

	if (p->content) {
		elm_object_content_set(obj, p->content);
	}
	if (p->repeat_events_exists)
		elm_notify_repeat_events_set(obj, p->repeat_events);
	if (p->timeout_exists)
		elm_notify_timeout_set(obj, p->timeout);
	if (p->orient)
	{
		Elm_Notify_Orient set = _orient_get(p->orient);
		if (set == ELM_NOTIFY_ORIENT_LAST) return;
		elm_notify_orient_set(obj, set);
	}
}

static Eina_Bool external_notify_param_set(void *data __UNUSED__,
		Evas_Object *obj, const Edje_External_Param *param)
{
	if ((!strcmp(param->name, "content"))
			&& (param->type == EDJE_EXTERNAL_PARAM_TYPE_STRING))
	{
		Evas_Object *content = external_common_param_edje_object_get(obj, param);
		if ((strcmp(param->s, "")) && (!content))
			return EINA_FALSE;
		elm_object_content_set(obj, content);
		return EINA_TRUE;
	}
	else if ((!strcmp(param->name, "repeat_events"))
			&& (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL))
	{
		elm_notify_repeat_events_set(obj, param->i);
		return EINA_TRUE;
	}
	else if ((!strcmp(param->name, "timeout"))
			&& (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE))
	{
		elm_notify_timeout_set(obj, param->d);
		return EINA_TRUE;
	}
	else if ((!strcmp(param->name, "orient"))
			&& (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE))
	{
		Elm_Notify_Orient set = _orient_get(param->s);
		if (set == ELM_NOTIFY_ORIENT_LAST) return EINA_FALSE;
		elm_notify_orient_set(obj, set);
		return EINA_TRUE;
	}

	ERR("unknown parameter '%s' of type '%s'",
			param->name, edje_external_param_type_str(param->type));

	return EINA_FALSE;
}

static Eina_Bool external_notify_param_get(void *data __UNUSED__,
		const Evas_Object *obj, Edje_External_Param *param)
{
	if (!strcmp(param->name, "content"))
	{
		/* not easy to get content name back from live object */
		return EINA_FALSE;
	}
	else if ((!strcmp(param->name, "repeat_events"))
			&& (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL))
	{
		param->i = elm_notify_repeat_events_get(obj);
		return EINA_TRUE;
	}
	else if ((!strcmp(param->name, "timeout"))
			&& (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE))
	{
		param->d = elm_notify_timeout_get(obj);
		return EINA_TRUE;
	}
	else if ((!strcmp(param->name, "orient"))
			&& (param->type == EDJE_EXTERNAL_PARAM_TYPE_CHOICE))
	{
		Elm_Notify_Orient set = elm_notify_orient_get(obj);
		if (set == ELM_NOTIFY_ORIENT_LAST) return EINA_FALSE;
		param->s = orients[set];
		return EINA_TRUE;
	}

	ERR("unknown parameter '%s' of type '%s'",
			param->name, edje_external_param_type_str(param->type));

	return EINA_FALSE;
}

static void * external_notify_params_parse(void *data __UNUSED__, Evas_Object *obj,
		const Eina_List *params) {
	Elm_Params_Notify *mem;
	Edje_External_Param *param;
	const Eina_List *l;

        mem = calloc(1, sizeof(Elm_Params_Notify));
	if (!mem)
		return NULL;

	EINA_LIST_FOREACH(params, l, param)
	{
		if (!strcmp(param->name, "content"))
			mem->content = external_common_param_edje_object_get(obj, param);
		else if (!strcmp(param->name, "timeout"))
		{
			mem->timeout = param->d;
			mem->timeout_exists = EINA_TRUE;
		}
		else if (!strcmp(param->name, "repeat_events"))
		{
			mem->repeat_events = param->i;
			mem->repeat_events_exists = EINA_TRUE;
		}
		else if (!strcmp(param->name, "orient"))
			  mem->orient = eina_stringshare_add(param->s);
	}

	return mem;
}

static Evas_Object *external_notify_content_get(void *data __UNUSED__,
		const Evas_Object *obj, const char *content)
{
	if (!strcmp(content, "content"))
		return elm_notify_content_get(obj);

	ERR("unknown content '%s'", content);
	return NULL;
}

static void external_notify_params_free(void *params) {
	free(params);
}

static Edje_External_Param_Info external_notify_params[] = {
		DEFINE_EXTERNAL_COMMON_PARAMS,
		EDJE_EXTERNAL_PARAM_INFO_STRING("content"),
		EDJE_EXTERNAL_PARAM_INFO_BOOL("repeat_events"),
		EDJE_EXTERNAL_PARAM_INFO_DOUBLE("timeout"),
		EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(notify, "notify");
DEFINE_EXTERNAL_TYPE_SIMPLE(notify, "Notify")
;
