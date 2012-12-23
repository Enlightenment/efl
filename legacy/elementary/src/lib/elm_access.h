/**
 * @defgroup Access Access
 * @ingroup Elementary
 *
 * TODO: description
 *
 */

#define ELM_OBJ_ACCESS_CLASS elm_obj_access_class_get()

const Eo_Class *elm_obj_access_class_get(void) EINA_CONST;

#define ELM_ACCESS_TYPE          0    /* when reading out widget or item
                                       * this is read first */
#define ELM_ACCESS_INFO          1    /* next read is info - this is
                                       * normally label */
#define ELM_ACCESS_STATE         2    /* if there is a state (eg checkbox)
                                       * then read state out */
#define ELM_ACCESS_CONTEXT_INFO  3    /* to give contextual information */

typedef char *(*Elm_Access_Content_Cb)(void *data, Evas_Object *obj);

/**
 * @brief Register evas object as an accessible object.
 * @since 1.8
 *
 * @param parent accessibility parent object. this should be one of widget.
 * @param target evas object to register as an accessible object.
 *
 * @ingroup Access
 */
EAPI void elm_access_object_register(Evas_Object *parent, Evas_Object *target);

/**
 * @brief Unregister accessible object.
 * @since 1.8
 *
 * @param obj accessible object.
 *
 * @ingroup Access
 */
EAPI void elm_access_object_unregister(Evas_Object *target);

/**
 * @brief Set text to give information for specific type.
 * @since 1.8
 *
 * @param obj accessible object.
 * @param type
 * @param text

 * @ingroup Access
 */
EAPI void elm_access_text_set(Evas_Object *obj, int type, const char *text);

/**
 * @brief Set text to give information for specific type.
 * @since 1.8
 *
 * @param obj accessible object.
 * @param type

 * @ingroup Access
 */
EAPI char * elm_access_text_get(Evas_Object *obj, int type);

/**
 * @brief Set callback to give information for specific type.
 * @since 1.8
 *
 * @param obj accessible object.
 * @param type
 * @param func
 * @param data
 *
 * @ingroup Access
 */
EAPI void elm_access_cb_set(Evas_Object *obj, int type, Elm_Access_Content_Cb func, const void *data);

//TODO: remvoe below - use elm_access_text_set(); or elm_access_cb_set();
EINA_DEPRECATED EAPI void elm_access_external_info_set(Evas_Object *obj, const char *text);
EINA_DEPRECATED EAPI char * elm_access_external_info_get(const Evas_Object *obj);
