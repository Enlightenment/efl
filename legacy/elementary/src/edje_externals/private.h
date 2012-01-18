#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include "Elementary.h"
#include "elm_priv.h"

#undef CRITICAL
#undef ERR
#undef WRN
#undef INF
#undef DBG

#define CRITICAL(...) EINA_LOG_DOM_CRIT(_elm_ext_log_dom, __VA_ARGS__)
#define ERR(...)      EINA_LOG_DOM_ERR (_elm_ext_log_dom, __VA_ARGS__)
#define WRN(...)      EINA_LOG_DOM_WARN(_elm_ext_log_dom, __VA_ARGS__)
#define INF(...)      EINA_LOG_DOM_INFO(_elm_ext_log_dom, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG (_elm_ext_log_dom, __VA_ARGS__)

typedef struct {
    const char *style;
    Eina_Bool disabled:1;
    Eina_Bool disabled_exists:1;
} Elm_Params;

extern int _elm_ext_log_dom;

void external_elm_init(void);
void external_signal(void *data, Evas_Object *obj, const char *signal, const char *source);
void external_signals_proxy(Evas_Object *obj, Evas_Object *edje, const char *part_name);
const char *external_translate(void *data, const char *orig);
void external_common_params_free(void *params);
void *external_common_params_parse_internal(size_t params_size, void *data, Evas_Object *obj, const Eina_List *params);
Evas_Object *external_common_param_icon_get(Evas_Object *obj, const Edje_External_Param *param);
Evas_Object *external_common_param_edje_object_get(Evas_Object *obj, const Edje_External_Param *p);
void external_common_icon_param_parse(Evas_Object **icon, Evas_Object *obj, const Eina_List *params);
Eina_Bool external_common_param_get(void *data, const Evas_Object *obj, Edje_External_Param *param);
Eina_Bool external_common_param_set(void *data, Evas_Object *obj, const Edje_External_Param *param);
void external_common_state_set(void *data, Evas_Object *obj, const void *from_params, const void *to_params, float pos);
void external_common_params_parse(void *mem, void *data, Evas_Object *obj, const Eina_List *params);

#define DEFINE_EXTERNAL_TYPE(type_name, name)           \
static Eina_Bool                                        \
_external_##type_name##_param_set(void *data, Evas_Object *obj, const Edje_External_Param *param) \
{                                                       \
   if (external_common_param_set(data, obj, param))     \
      return EINA_TRUE;                                 \
   return external_##type_name##_param_set(data, obj, param); \
}                                                       \
static Eina_Bool                                        \
_external_##type_name##_param_get(void *data, const Evas_Object *obj, Edje_External_Param *param) \
{                                                       \
   if (external_common_param_get(data, obj, param))     \
      return EINA_TRUE;                                 \
   return external_##type_name##_param_get(data, obj, param); \
}                                                       \
static const char *                                     \
external_##type_name##_label_get(void *data __UNUSED__) \
{                                                       \
    return name;                                        \
}                                                       \
                                                        \
static void                                             \
_external_##type_name##_state_set(void *data __UNUSED__, Evas_Object *obj, const void *from_params, const void *to_params, float pos __UNUSED__) \
{                                                       \
   external_common_state_set(data, obj, from_params, to_params, pos); \
   external_##type_name##_state_set(data, obj, from_params, to_params, pos); \
}                                                       \
                                                        \
static void *                                           \
_external_##type_name##_params_parse(void *data __UNUSED__, Evas_Object *obj __UNUSED__, const Eina_List *params) \
{                                                       \
   void *mem = external_##type_name##_params_parse(data, obj, params); \
   external_common_params_parse(mem, data, obj, params); \
   return mem;                                          \
}                                                       \
static void                                             \
_external_##type_name##_params_free(void *params)       \
{                                                       \
   external_common_params_free(params);                 \
   external_##type_name##_params_free(params);          \
}                                                       \
                                                        \
const Edje_External_Type external_##type_name##_type = {\
    .abi_version = EDJE_EXTERNAL_TYPE_ABI_VERSION,      \
    .module = "elm",                                    \
    .module_name = "Elementary",                        \
    .add = external_##type_name##_add,                  \
    .state_set = _external_##type_name##_state_set,     \
    .signal_emit = external_signal,                     \
    .param_set = _external_##type_name##_param_set,     \
    .param_get = _external_##type_name##_param_get,     \
    .params_parse = _external_##type_name##_params_parse,\
    .params_free = _external_##type_name##_params_free, \
    .label_get = external_##type_name##_label_get,      \
    .content_get = external_##type_name##_content_get,  \
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
   external_elm_init();                                                 \
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
    EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("style", "default"), \
    EDJE_EXTERNAL_PARAM_INFO_BOOL_DEFAULT("disabled", EINA_FALSE)
