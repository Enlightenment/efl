#include <Elementary.h>
#include "private.h"

void
external_signal(void *data, Evas_Object *obj, const char *signal, const char *source)
{
   printf("External Signal received: '%s' '%s'\n", signal, source);
}

void *
external_common_params_parse_internal(size_t params_size, void *data, Evas_Object *obj, const Eina_List *params)
{
   Elm_Params *p;
   const Eina_List *l;
   Edje_External_Param *param;

   if (params_size < sizeof(Elm_Params))
     return NULL;

   p = calloc(1, params_size);
   if (!p)
     return NULL;

   EINA_LIST_FOREACH(params, l, param)
     {
    if (!strcmp(param->name, "label"))
      p->label = param->s;
    if(!strcmp(param->name, "state"))
      p->state = param->i;
     }
   return p;
}

void
external_common_icon_param_parse(Evas_Object **icon, Evas_Object *obj, const Eina_List *params)
{
   Edje_External_Param *p;

   p = edje_external_param_find(params, "icon");
   if (p)
     {
    Evas_Object *parent = evas_object_smart_parent_get(obj);
    const char *file;
    edje_object_file_get(parent, &file, NULL);
    *icon = elm_icon_add(parent);

    if (!elm_icon_file_set(*icon, file, p->s))
      elm_icon_standard_set(*icon, p->s);
     }
}

void
external_common_params_free(void *params)
{
   Elm_Params *p = params;
   free(p);
};

#define DEFINE_TYPE(type_name) \
  extern const Edje_External_Type external_##type_name##_type;
#include "modules.inc"
#undef DEFINE_TYPE

static Edje_External_Type_Info elm_external_types[] =
{
#define DEFINE_TYPE(type_name)              \
  {"elm/"#type_name, &external_##type_name##_type},
#include "modules.inc"
#undef DEFINE_TYPE
   {NULL, NULL}
};

static Eina_Bool
elm_mod_init(void)
{
   edje_external_type_array_register(elm_external_types);
   return EINA_TRUE;
}

static void
elm_mod_shutdown(void)
{
   edje_external_type_array_unregister(elm_external_types);
}

EINA_MODULE_INIT(elm_mod_init);
EINA_MODULE_SHUTDOWN(elm_mod_shutdown);
