#ifndef ELDBUS_PENDING_H
#define ELDBUS_PENDING_H 1

/**
 * @defgroup Eldbus_Pending Pending
 * @ingroup Eldbus
 *
 * @{
 */

/**
 * @brief Set data to a Eldbus_Pending object.
 *
 * @param pending The Eldbus_Pending object on which the data is set.
 * @param key A string corresponding to the key associated with the data.
 * @param data The data to set to the Eldbus_Pending object.
 */
EAPI void                  eldbus_pending_data_set(Eldbus_Pending *pending, const char *key, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Get the data set to a Eldbus_Pending object.
 *
 * @param pending The Eldbus_Pending object on which the data is set.
 * @param key A string corresponding to the key associated with the data.
 * @return The data set to the Eldbus_Pending object.
 */
EAPI void                 *eldbus_pending_data_get(const Eldbus_Pending *pending, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Delete the data set to a Eldbus_Pending object.
 *
 * @param pending The Eldbus_Pending object on which the data is set.
 * @param key A string corresponding to the key associated with the data.
 */
EAPI void                 *eldbus_pending_data_del(Eldbus_Pending *pending, const char *key) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Cancel the pending message.
 *
 * @param pending The Eldbus_Pending object corresponding to the pending message.
 */
EAPI void                  eldbus_pending_cancel(Eldbus_Pending *pending) EINA_ARG_NONNULL(1);

/**
 * @brief Get the destination of the pending message.
 *
 * @param pending The Eldbus_Pending object.
 * @return A string corresponding to the destination of the message.
 */
EAPI const char           *eldbus_pending_destination_get(const Eldbus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the path of the pending message.
 *
 * @param pending The Eldbus_Pending object.
 * @return A string corresponding to the path of the message.
 */
EAPI const char           *eldbus_pending_path_get(const Eldbus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the interface of the pending message.
 *
 * @param pending The Eldbus_Pending object.
 * @return A string corresponding to the interface of the message.
 */
EAPI const char           *eldbus_pending_interface_get(const Eldbus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the method of the pending message.
 *
 * @param pending The Eldbus_Pending object.
 * @return A string corresponding to the method of the message.
 */
EAPI const char           *eldbus_pending_method_get(const Eldbus_Pending *pending) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Add a callback function to be called when pending will be freed.
 *
 * @param pending The Eldbus_Pending object on which the callback is registered.
 * @param cb The callback to register.
 * @param data The data to pass to the callback.
 */
EAPI void                  eldbus_pending_free_cb_add(Eldbus_Pending *pending, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove callback registered in eldbus_pending_free_cb_add().
 *
 * @param pending The Eldbus_Pending object on which the callback was registered.
 * @param cb The callback that was registered.
 * @param data The data passed to the callback.
 */
EAPI void                  eldbus_pending_free_cb_del(Eldbus_Pending *pending, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @}
 */
#endif
