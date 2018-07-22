/**
 * @typedef Elm_Object_Item
 * An Elementary Object item handle.
 * @ingroup Elm_General
 */
typedef Eo Elm_Object_Item;

/**
 * @typedef Elm_Object_Item_Signal_Cb
 *
 * Elm_Object_Item Signal Callback functions' prototype definition. @c data
 * will have the auxiliary data pointer at the time the callback registration.
 * @c it will be a pointer the Elm_Object_Item that have the edje object where
 * the signal comes from. @c emission will identify the exact signal's emission
 * string and @c source the exact signal's source one.
 *
 * @see elm_object_item_signal_callback_add()
 * @since 1.8
 *
 * @ingroup Elm_General
 */
typedef void                  (*Elm_Object_Item_Signal_Cb)(void *data, Elm_Object_Item *it, const char *emission, const char *source);

#ifndef EFL_NOLEGACY_API_SUPPORT
#include "elm_widget_item.eo.legacy.h"
#endif

#define elm_object_item_content_set(it, content) elm_object_item_part_content_set((it), NULL, (content))

#define elm_object_item_content_get(it) elm_object_item_part_content_get((it), NULL)

#define elm_object_item_content_unset(it) elm_object_item_part_content_unset((it), NULL)

/**
 * Macro to set a label of an object item.
 *
 * @param it The Elementary object item.
 * @param label The new text of the label.
 *
 * @note Elementary object items may have many labels.
 *
 * @ingroup Elm_General
 */
#define elm_object_item_text_set(it, label) elm_object_item_part_text_set((it), NULL, (label))

#define elm_object_item_text_get(it) elm_object_item_part_text_get((it), NULL)

#define elm_object_item_domain_translatable_text_set(it, domain, text) elm_object_item_domain_translatable_part_text_set((it), NULL, (domain), (text))

#define elm_object_item_translatable_text_set(it, text) elm_object_item_domain_translatable_part_text_set((it), NULL, NULL, (text))

#define elm_object_item_translatable_part_text_set(it, part, text) elm_object_item_domain_translatable_part_text_set((it), (part), NULL, (text))

#define elm_object_item_translatable_text_get(it) elm_object_item_translatable_part_text_get((it), NULL)

#define elm_object_item_part_text_translatable_set(it, part, translatable) elm_object_item_domain_part_text_translatable_set((it), (part), NULL, (translatable))

#define elm_object_item_domain_text_translatable_set(it, domain, translatable) elm_object_item_domain_part_text_translatable_set((it), NULL, (domain), (translatable))

#define elm_object_item_text_translatable_set(it, translatable) elm_object_item_domain_part_text_translatable_set((it), NULL, NULL, (translatable))

/**
 * Get the data associated with an object item
 * @param it The Elementary object item
 * @return The data associated with @p it
 *
 * @note Every elm_object_item supports this API
 * @ingroup Elm_General
 */
EAPI void *elm_object_item_data_get(const Elm_Object_Item *it);

/**
 * Set the data associated with an object item
 * @param it The Elementary object item
 * @param data The data to be associated with @p it
 *
 * @note Every elm_object_item supports this API
 * @ingroup Elm_General
 */
EAPI void elm_object_item_data_set(Elm_Object_Item *it, void *data);

/** Delete the given item.
 *
 * @ingroup Elm_General
 */
EAPI void elm_object_item_del(Elm_Object_Item *obj);
