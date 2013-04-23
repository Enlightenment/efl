#ifndef ELDBUS_PENDING_H
#define ELDBUS_PENDING_H 1

/**
 * @defgroup Eldbus_Pending Pending
 * @ingroup Eldbus
 *
 * @{
 */

EAPI void                  eldbus_pending_data_set(Eldbus_Pending *pending, const char *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);
EAPI void                 *eldbus_pending_data_get(const Eldbus_Pending *pending, const char *key) EINA_ARG_NONNULL(1, 2);
EAPI void                 *eldbus_pending_data_del(Eldbus_Pending *pending, const char *key) EINA_ARG_NONNULL(1, 2);
EAPI void                  eldbus_pending_cancel(Eldbus_Pending *pending) EINA_ARG_NONNULL(1);

EAPI const char           *eldbus_pending_destination_get(const Eldbus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_pending_path_get(const Eldbus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_pending_interface_get(const Eldbus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_pending_method_get(const Eldbus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Add a callback function to be called when pending will be freed.
 */
EAPI void                  eldbus_pending_free_cb_add(Eldbus_Pending *pending, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove callback registered in eldbus_pending_free_cb_add().
 */
EAPI void                  eldbus_pending_free_cb_del(Eldbus_Pending *pending, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */
#endif
