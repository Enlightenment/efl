#include <assert.h>

#include "private.h"

typedef struct _Elm_Params_Gengrid
{
   Elm_Params base;
   Eina_Bool multi : 1;
   Eina_Bool multi_exists : 1;
   Eina_Bool no_select : 1;
   Eina_Bool no_select_exists : 1;
   Eina_Bool always_select : 1;
   Eina_Bool always_select_exists : 1;
   Eina_Bool h_bounce:1;
   Eina_Bool h_bounce_exists:1;
   Eina_Bool v_bounce:1;
   Eina_Bool v_bounce_exists:1;
   double    h_pagerel;
   Eina_Bool h_pagerel_exists : 1;
   double    v_pagerel;
   Eina_Bool v_pagerel_exists : 1;
   int       h_itemsize;
   Eina_Bool h_itemsize_exists : 1;
   int       v_itemsize;
   Eina_Bool v_itemsize_exists : 1;
   Eina_Bool horizontal : 1;
   Eina_Bool horizontal_exists : 1;
   Eina_Bool align_x_exists;
   double align_x;
   Eina_Bool align_y_exists;
   double align_y;
} Elm_Params_Gengrid;

static void
external_gengrid_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__)
{
   const Elm_Params_Gengrid *p;

   if (to_params) p = to_params;
   else if (from_params) p = from_params;
   else return;

   if (p->multi_exists)
     elm_gengrid_multi_select_set(obj, p->multi);
   if (p->no_select_exists)
     elm_gengrid_no_select_mode_set (obj, p->no_select);
   if (p->always_select_exists)
     elm_gengrid_always_select_mode_set (obj, p->always_select);
   if (p->h_bounce_exists)
     {
        Eina_Bool h_bounce, v_bounce;
        elm_gengrid_bounce_get(obj, &h_bounce, &v_bounce);
        elm_gengrid_bounce_set(obj, p->h_bounce, v_bounce);
     }
   if (p->v_bounce_exists)
     {
        Eina_Bool h_bounce, v_bounce;
        elm_gengrid_bounce_get(obj, &h_bounce, &v_bounce);
        elm_gengrid_bounce_set(obj, h_bounce, p->v_bounce);
     }
   if (p->h_pagerel_exists)
     {
        double h_pagerel, v_pagerel;
        elm_gengrid_page_relative_get(obj, &h_pagerel, &v_pagerel);
        elm_gengrid_page_relative_set(obj, h_pagerel, p->v_pagerel);
     }
   if (p->v_pagerel_exists)
     {
        double h_pagerel, v_pagerel;
        elm_gengrid_page_relative_get(obj, &h_pagerel, &v_pagerel);
        elm_gengrid_page_relative_set(obj, p->h_pagerel, v_pagerel);
     }
   if (p->h_itemsize_exists)
     {
        int h_itemsize, v_itemsize;
        elm_gengrid_item_size_get(obj, &h_itemsize, &v_itemsize);
        elm_gengrid_item_size_set(obj, h_itemsize, p->v_itemsize);
     }
   if (p->v_itemsize_exists)
     {
        int h_itemsize, v_itemsize;
        elm_gengrid_item_size_get(obj, &h_itemsize, &v_itemsize);
        elm_gengrid_item_size_set(obj, p->h_itemsize, v_itemsize);
     }
   else if (p->align_x_exists || p->align_y_exists)
     {
        double x, y;
	elm_gengrid_align_get(obj, &x, &y);
	if (p->align_x_exists)
	  elm_gengrid_align_set(obj, p->align_x, y);
	else
	  elm_gengrid_align_set(obj, x, p->align_y);
     }
   if (p->horizontal_exists)
     {
        elm_gengrid_horizontal_set(obj, p->horizontal);
     }
}

static Eina_Bool
external_gengrid_param_set(void *data __UNUSED__, Evas_Object *obj, const Edje_External_Param *param)
{
   if (!strcmp(param->name, "multi select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_gengrid_multi_select_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "no selected"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_gengrid_no_select_mode_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "always select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_gengrid_always_select_mode_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "height bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool h_bounce, v_bounce;
             elm_gengrid_bounce_get(obj, &h_bounce, &v_bounce);
             elm_gengrid_bounce_set(obj, param->i, v_bounce);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "width bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool h_bounce, v_bounce;
             elm_gengrid_bounce_get(obj, &h_bounce, &v_bounce);
             elm_gengrid_bounce_set(obj, h_bounce, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal page relative"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             double h_pagerel, v_pagerel;
             elm_gengrid_page_relative_get(obj, &h_pagerel, &v_pagerel);
             elm_gengrid_page_relative_set(obj, param->d, v_pagerel);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "vertical page relative"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             double h_pagerel, v_pagerel;
             elm_gengrid_page_relative_get(obj, &h_pagerel, &v_pagerel);
             elm_gengrid_page_relative_set(obj, h_pagerel, param->d);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal item size"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             int h_itemsize, v_itemsize;
             elm_gengrid_item_size_get(obj, &h_itemsize, &v_itemsize);
             elm_gengrid_item_size_set(obj, param->i, v_itemsize);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "vertical item size"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             int h_itemsize, v_itemsize;
             elm_gengrid_item_size_get(obj, &h_itemsize, &v_itemsize);
             elm_gengrid_item_size_set(obj, h_itemsize, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             elm_gengrid_horizontal_set(obj, param->i);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "align x")
		   && param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
     {
	double x, y;
	elm_gengrid_align_get(obj, &x, &y);
	elm_gengrid_align_set(obj, param->d, y);
	return EINA_TRUE;
     }
   else if (!strcmp(param->name, "align y")
		   && param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
     {
	double x, y;
	elm_gengrid_align_get(obj, &x, &y);
	elm_gengrid_align_set(obj, x, param->d);
	return EINA_TRUE;
     }
   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static Eina_Bool
external_gengrid_param_get(void *data __UNUSED__, const Evas_Object *obj, Edje_External_Param *param)
{
   if (!strcmp(param->name, "multi select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_gengrid_multi_select_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "no selected"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_gengrid_no_select_mode_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "always select"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_gengrid_always_select_mode_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "height bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool h_bounce, v_bounce;
             elm_gengrid_bounce_get(obj, &h_bounce, &v_bounce);
             param->i = h_bounce;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "width bounce"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             Eina_Bool h_bounce, v_bounce;
             elm_gengrid_bounce_get(obj, &h_bounce, &v_bounce);
             param->i = v_bounce;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal page relative"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             double h_pagerel, v_pagerel;
             elm_gengrid_page_relative_get(obj, &h_pagerel, &v_pagerel);
             param->d = h_pagerel;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "vertical page relative"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
          {
             double h_pagerel, v_pagerel;
             elm_gengrid_page_relative_get(obj, &h_pagerel, &v_pagerel);
             param->d = v_pagerel;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal item size"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             int h_itemsize, v_itemsize;
             elm_gengrid_item_size_get(obj, &h_itemsize, &v_itemsize);
             param->i = h_itemsize;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "vertical item size"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_INT)
          {
             int h_itemsize, v_itemsize;
             elm_gengrid_item_size_get(obj, &h_itemsize, &v_itemsize);
             param->i = v_itemsize;
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "horizontal"))
     {
        if (param->type == EDJE_EXTERNAL_PARAM_TYPE_BOOL)
          {
             param->i = elm_gengrid_horizontal_get(obj);
             return EINA_TRUE;
          }
     }
   else if (!strcmp(param->name, "align x")
		   && param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
     {
	double x, y;
	elm_gengrid_align_get(obj, &x, &y);
	param->d = x;
	return EINA_TRUE;
     }
   else if (!strcmp(param->name, "align y")
		   && param->type == EDJE_EXTERNAL_PARAM_TYPE_DOUBLE)
     {
	double x, y;
	elm_gengrid_align_get(obj, &x, &y);
	param->d = y;
	return EINA_TRUE;
     }
   ERR("unknown parameter '%s' of type '%s'",
       param->name, edje_external_param_type_str(param->type));

   return EINA_FALSE;
}

static void *
external_gengrid_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params)
{
   Elm_Params_Gengrid *mem;
   Edje_External_Param *param;
   const Eina_List *l;

   mem = ELM_NEW(Elm_Params_Gengrid);
   if (!mem)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
        if (!strcmp(param->name, "multi select"))
          {
             mem->multi = !!param->i;
             mem->multi_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "no selected"))
          {
             mem->no_select = !!param->i;
             mem->no_select_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "always select"))
          {
             mem->always_select = !!param->i;
             mem->always_select_exists = EINA_TRUE;
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
        else if (!strcmp(param->name, "horizontal page relative"))
          {
             mem->h_pagerel = param->d;
             mem->h_pagerel_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "vertical page relative"))
          {
             mem->v_pagerel = param->d;
             mem->v_pagerel_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "horizontal item size"))
          {
             mem->h_itemsize = param->i;
             mem->h_itemsize_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "vertical item size"))
          {
             mem->v_itemsize = param->i;
             mem->v_itemsize_exists = EINA_TRUE;
          }
        else if (!strcmp(param->name, "horizontal"))
          {
             mem->horizontal = !!param->i;
             mem->horizontal_exists = EINA_TRUE;
          }
	else if (!strcmp(param->name, "align x"))
	  {
	     mem->align_x = param->d;
	     mem->align_x_exists = EINA_TRUE;
	  }
	else if (!strcmp(param->name, "align y"))
	  {
	     mem->align_y = param->d;
	     mem->align_y_exists = EINA_TRUE;
	  }
     }

   return mem;
}

static Evas_Object *
external_gengrid_content_get(void *data __UNUSED__, const Evas_Object *obj __UNUSED__, const char *content __UNUSED__)
{
   ERR("No content.");
   return NULL;
}

static void
external_gengrid_params_free(void *params)
{
   Elm_Params_Gengrid *mem = params;
   free(mem);
}

static Edje_External_Param_Info external_gengrid_params[] = {
   DEFINE_EXTERNAL_COMMON_PARAMS,
   EDJE_EXTERNAL_PARAM_INFO_BOOL("multi select"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("no select"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("always select"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("height bounce"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("width bounce"),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE("horizontal page relative"),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE("vertical page relative"),
   EDJE_EXTERNAL_PARAM_INFO_INT("horizontal item size"),
   EDJE_EXTERNAL_PARAM_INFO_INT("vertical item size"),
   EDJE_EXTERNAL_PARAM_INFO_BOOL("horizontal"),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE("align x"),
   EDJE_EXTERNAL_PARAM_INFO_DOUBLE("align y"),
   EDJE_EXTERNAL_PARAM_INFO_SENTINEL
};

DEFINE_EXTERNAL_ICON_ADD(gengrid, "gengrid");
DEFINE_EXTERNAL_TYPE_SIMPLE(gengrid, "Generic Grid");
