#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include "Elementary.h"
#include "elm_priv.h"

typedef struct {
    const char *label;
} Elm_Params;

void external_signal(void *data, Evas_Object *obj, const char *signal, const char *source);
void external_signals_proxy(Evas_Object *obj, Evas_Object *edje, const char *part_name);
const char *external_translate(void *data, const char *orig);
void external_common_params_free(void *params);
void *external_common_params_parse_internal(size_t params_size, void *data, Evas_Object *obj, const Eina_List *params);
Evas_Object *external_common_param_icon_get(Evas_Object *obj, const Edje_External_Param *param);
Evas_Object *external_common_param_edje_object_get(Evas_Object *obj, const Edje_External_Param *p);
void external_common_icon_param_parse(Evas_Object **icon, Evas_Object *obj, const Eina_List *params);
#define external_common_params_parse(type, data, obj, params)   \
    external_common_params_parse_internal(sizeof(type), data, obj, params)


#define DEFINE_EXTERNAL_TYPE(type_name, name)           \
static const char *                                     \
external_##type_name##_label_get(void *data __UNUSED__) \
{                                                       \
    return name;                                        \
}                                                       \
                                                        \
const Edje_External_Type external_##type_name##_type = {\
    .abi_version = EDJE_EXTERNAL_TYPE_ABI_VERSION,      \
    .module = "elm",                                    \
    .module_name = "Elementary",                        \
    .add = external_##type_name##_add,                  \
    .state_set = external_##type_name##_state_set,      \
    .signal_emit = external_signal,                     \
    .param_set = external_##type_name##_param_set,      \
    .param_get = external_##type_name##_param_get,      \
    .params_parse = external_##type_name##_params_parse,\
    .params_free = external_##type_name##_params_free,  \
    .label_get = external_##type_name##_label_get,      \
    .description_get = NULL,                            \
    .icon_add = external_##type_name##_icon_add,        \
    .preview_add = NULL,                                \
    .translate = external_translate,                    \
    .parameters_info = external_##type_name##_params,   \
    .data = NULL                                        \
};

#define DEFINE_EXTERNAL_TYPE_SIMPLE(type_name, name)    \
static Evas_Object *                                \
external_##type_name##_add(void *data __UNUSED__, Evas *evas __UNUSED__, Evas_Object *edje, const Eina_List *params __UNUSED__, const char *part_name) \
{									\
   Evas_Object *parent, *obj;						\
   parent = elm_widget_parent_widget_get(edje);			\
   if (!parent) parent = edje;						\
   obj = elm_##type_name##_add(parent);				\
   external_signals_proxy(obj, edje, part_name);			\
   return obj;								\
}									\
									\
DEFINE_EXTERNAL_TYPE(type_name, name)

#define DEFINE_EXTERNAL_ICON_ADD(type_name, name)                   \
Evas_Object *                                                       \
external_##type_name##_icon_add(void *data __UNUSED__, Evas *e) {   \
   Evas_Object *ic;                                                 \
   int w = 20, h = 10;                                              \
                                                                    \
   ic = edje_object_add(e);                                         \
   edje_object_file_set(ic, PACKAGE_DATA_DIR"/edje_externals/icons.edj", "ico_"name);\
   evas_object_size_hint_min_set(ic, w, h);                         \
   evas_object_size_hint_max_set(ic, w, h);                         \
                                                                    \
   return ic;                                                       \
}

#define DEFINE_EXTERNAL_COMMON_PARAMS           \
    EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("label", "label goes here")
