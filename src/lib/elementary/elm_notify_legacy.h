/**
 * @brief Add a new notify to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Elm_Notify
 */
EAPI Evas_Object                 *elm_notify_add(Evas_Object *parent);

/**
 * @brief Set the notify parent
 *
 * @param obj The notify object
 * @param parent The new parent
 *
 * Once the parent object is set, a previously set one will be disconnected
 * and replaced.
 *
 * @ingroup Elm_Notify
 */
EAPI void                         elm_notify_parent_set(Evas_Object *obj, Evas_Object *parent);

/**
 * @brief Get the notify parent
 *
 * @param obj The notify object
 * @return The parent
 *
 * @see elm_notify_parent_set()
 *
 * @ingroup Elm_Notify
 */
EAPI Evas_Object                 *elm_notify_parent_get(const Evas_Object *obj);

#include "elm_notify_eo.legacy.h"