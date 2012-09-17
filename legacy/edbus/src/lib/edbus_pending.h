#ifndef EDBUS_PENDING_H
#define EDBUS_PENDING_H 1

/**
 * @defgroup EDBus_Pending Pending
 *
 * @{
 */
EAPI void                  edbus_pending_data_set(EDBus_Pending *pending, const char *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);
EAPI void                 *edbus_pending_data_get(const EDBus_Pending *pending, const char *key) EINA_ARG_NONNULL(1, 2);
EAPI void                 *edbus_pending_data_del(EDBus_Pending *pending, const char *key) EINA_ARG_NONNULL(1, 2);
EAPI void                  edbus_pending_cancel(EDBus_Pending *pending) EINA_ARG_NONNULL(1);

EAPI const char           *edbus_pending_destination_get(const EDBus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_pending_path_get(const EDBus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_pending_interface_get(const EDBus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_pending_method_get(const EDBus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Add a callback function to be called when pending will be freed.
 */
EAPI void                  edbus_pending_cb_free_add(EDBus_Pending *pending, EDBus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Remove callback registered in edbus_pending_cb_free_add().
 */
EAPI void                  edbus_pending_cb_free_del(EDBus_Pending *pending, EDBus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);
/**
 * @}
 */
#endif
