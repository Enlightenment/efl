#include <Elementary.h>

#ifdef EAPI
# undef EAPI
#endif

#define EAPI __attribute__ ((visibility("default")))

/* #ifdef _WIN32 */
/* # ifdef EFL_EDJE_BUILD */
/* #  ifdef DLL_EXPORT */
/* #   define EAPI __declspec(dllexport) */
/* #  else */
/* #   define EAPI */
/* #  endif /\* ! DLL_EXPORT *\/ */
/* # else */
/* #  define EAPI __declspec(dllimport) */
/* # endif /\* ! EFL_EDJE_BUILD *\/ */
/* #else */
/* # ifdef __GNUC__ */
/* #  if __GNUC__ >= 4 */
/* #   define EAPI __attribute__ ((visibility("default"))) */
/* #  else */
/* #   define EAPI */
/* #  endif */
/* # else */
/* #  define EAPI */
/* # endif */
/* #endif */

typedef struct {
    const char *label;
} Elm_Params;

void  external_signal(void *data, Evas_Object *obj, const char *signal, const char *source);
void  external_common_params_free(void *params);
void *external_common_params_parse_internal(size_t params_size, void *data, Evas_Object *obj, const Eina_List *params);
void  external_commom_icon_param_parse(Evas_Object *icon, Evas_Object *obj, const Eina_List *params);
#define external_common_params_parse(type, data, obj, params)   \
    external_common_params_parse_internal(sizeof(type), data, obj, params)


#define DEFINE_EXTERNAL_TYPE(type_name, name)           \
static const char *                                     \
external_##type_name##_label_get(void *data) {          \
    return name;                                        \
}                                                       \
                                                        \
const Edje_External_Type external_##type_name##_type = {\
    .abi_version = EDJE_EXTERNAL_TYPE_ABI_VERSION,      \
    .module = "elm",                                    \
    .add = external_##type_name##_add,                  \
    .signal_emit = external_signal,                     \
    .state_set = external_##type_name##_state_set,      \
    .params_parse = external_##type_name##_params_parse,\
    .params_free = external_##type_name##_params_free,  \
    .icon_add = NULL,                                   \
    .label_get = external_##type_name##_label_get,      \
    .parameters_info = external_##type_name##_params,   \
    .data = NULL                                        \
};

#define DEFINE_EXTERNAL_TYPE_SIMPLE(type_name, name)    \
static Evas_Object *                                \
external_##type_name##_add(void *data, Evas *evas, Evas_Object *parent, const Eina_List *params) \
{\
    return elm_##type_name##_add(parent);   \
}\
\
DEFINE_EXTERNAL_TYPE(type_name, name)


#define DEFINE_EXTERNAL_COMMON_PARAMS           \
    EDJE_EXTERNAL_PARAM_INFO_STRING_DEFAULT("label", "label goes here")
