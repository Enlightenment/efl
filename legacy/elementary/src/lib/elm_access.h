/**
 * @defgroup Access Access
 * @ingroup Elementary
 *
 * TODO: description
 *
 */

#define ELM_OBJ_ACCESS_CLASS elm_obj_access_class_get()

const Eo_Class *elm_obj_access_class_get(void) EINA_CONST;

EAPI void elm_access_external_info_set(Evas_Object *obj, const char *text);

EAPI char * elm_access_external_info_get(const Evas_Object *obj);
