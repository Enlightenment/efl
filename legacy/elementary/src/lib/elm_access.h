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
typedef void (*Elm_Access_Activate_Cb)(void *data, Evas_Object *part_obj, Elm_Object_Item *item);

/**
 * @brief Register object item as an accessible object.
 * @since 1.8
 *
 * @param item The elementary object item
 *
 * @ingroup Access
 */
EAPI Evas_Object * elm_access_object_item_register(Elm_Object_Item *item);

/**
 * @brief Unregister accessible object of the object item.
 * @since 1.8
 *
 * @param item The elementary object item
 *
 * @ingroup Access
 */
EAPI void elm_access_object_item_unregister(Elm_Object_Item *item);

/**
 * @brief Get an accessible object of the object item.
 * @since 1.8
 *
 * @param item The elementary object item
 * @return Accessible object of the object item or NULL for any error
 *
 * @ingroup Access
 */
EAPI Evas_Object * elm_access_object_item_access_get(Elm_Object_Item *item);

/**
 * @brief Register evas object as an accessible object.
 * @since 1.8
 *
 * @param obj The evas object to register as an accessible object.
 * @param parent The elementary object which is used for creating
 * accessible object.
 *
 * @ingroup Access
 */
EAPI Evas_Object * elm_access_object_register(Evas_Object *obj, Evas_Object *parent);

/**
 * @brief Unregister accessible object.
 * @since 1.8
 *
 * @param obj The Evas object to unregister accessible object.
 *
 * @ingroup Access
 */
EAPI void elm_access_object_unregister(Evas_Object *obj);

/**
 * @brief Get an accessible object of the evas object.
 * @since 1.8
 *
 * @param obj The evas object.
 * @return Accessible object of the evas object or NULL for any error
 *
 * @ingroup Access
 */
EAPI Evas_Object * elm_access_object_access_get(Evas_Object *obj);

/**
 * @brief Set text to give information for specific type.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param type The type of content that will be read
 * @param text The text information that will be read
 *
 * @see elm_access_content_cb_set
 * @ingroup Access
 */
EAPI void elm_access_text_set(Evas_Object *obj, int type, const char *text);

/**
 * @brief Set text to give information for specific type.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param type The type of content that will be read
 *
 * @see elm_access_content_cb_set
 * @ingroup Access
 */
EAPI char * elm_access_text_get(Evas_Object *obj, int type);

/**
 * @brief Set content callback to give information for specific type.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param type The type of content that will be read
 * @param func The function to be called when the content is read
 * @param data The data pointer to be passed to @p func
 *
 * The type would be one of ELM_ACCESS_TYPE, ELM_ACCESS_INFO,
 * ELM_ACCESS_STATE, ELM_ACCESS_CONTEXT_INFO.
 *
 * In the case of button widget, the content of ELM_ACCESS_TYPE would be
 * "button". The label of button such as "ok", "cancel" is for ELM_ACCESS_INFO.
 * If the button is disabled, content of ELM_ACCESS_STATE would be "disabled".
 * And if there is contextual information, ELM_ACCESS_CONTEXT_INFO is used.
 *
 * @ingroup Access
 */
EAPI void elm_access_content_cb_set(Evas_Object *obj, int type, Elm_Access_Content_Cb func, const void *data);

/**
 * @brief Set activate callback to activate highlight object.
 * @since 1.8
 *
 * @param obj Accessible object.
 * @param func The function to be called when the activate gesture is detected
 * @param data The data pointer to be passed to @p func
 *
 * @ingroup Access
 */
EAPI void elm_access_activate_cb_set(Evas_Object *obj, Elm_Access_Activate_Cb func, void *data);

//TODO: remvoe below - use elm_access_text_set(); or elm_access_cb_set();
EINA_DEPRECATED EAPI void elm_access_external_info_set(Evas_Object *obj, const char *text);
EINA_DEPRECATED EAPI char * elm_access_external_info_get(const Evas_Object *obj);
