#ifndef ELDBUS_SIGNAL_HANDLER_H
#define ELDBUS_SIGNAL_HANDLER_H 1

/**
 * @defgroup Eldbus_Signal_Handler Signal Handler
 * @ingroup Eldbus
 *
 * @{
 */

/**
 * @brief Add a signal handler.
 *
 * @param conn connection where the signal is emitted
 * @param sender bus name or unique id of where the signal is emitted
 * @param path path of remote object
 * @param interface that signal belongs
 * @param member name of the signal
 * @param cb callback that will be called when this signal is received
 * @param cb_data data that will be passed to callback
 */
EAPI Eldbus_Signal_Handler *eldbus_signal_handler_add(Eldbus_Connection *conn, const char *sender, const char *path, const char *interface, const char *member, Eldbus_Signal_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 6);

/**
 * @brief Increase signal handler reference.
 */
EAPI Eldbus_Signal_Handler *eldbus_signal_handler_ref(Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1);

/**
 * @brief Decrease signal handler reference.
 * If reference == 0 signal handler will be freed.
 */
EAPI void                  eldbus_signal_handler_unref(Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1);

/**
 * @brief Decrease signal handler reference like eldbus_signal_handler_unref()
 * but if reference > 0 this signal handler will stop listening to signals. In other
 * words it will be canceled but memory will not be freed.
 */
EAPI void                  eldbus_signal_handler_del(Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1);

/**
 * @brief Add extra argument in match of signal handler to obtain specifics signals.
 *
 * Example:
 * eldbus_signal_handler_match_extra_set(sh, "arg0", "org.bansheeproject.Banshee", "arg1", "", NULL);
 * With this extra arguments this signal handler callback only will be called
 * when Banshee is started.
 *
 * @note For now only argX is supported.
 *
 * @param sh signal handler
 * @param ... variadic of key and value and must be ended with a NULL
 *
 * @note For more information:
 * http://dbus.freedesktop.org/doc/dbus-specification.html#message-bus-routing-match-rules
 */
EAPI Eina_Bool             eldbus_signal_handler_match_extra_set(Eldbus_Signal_Handler *sh, ...) EINA_ARG_NONNULL(1) EINA_SENTINEL;

/**
 * @brief Add extra argument in match of signal handler to obtain specifics signals.
 *
 * Example:
 * eldbus_signal_handler_match_extra_set(sh, "arg0", "org.bansheeproject.Banshee", "arg1", "", NULL);
 * With this extra arguments this signal handler callback only will be called
 * when Banshee is started.
 *
 * @note For now is only supported argX.
 *
 * @param sh signal handler
 * @param ap va_list with the keys and values, must be ended with a NULL
 *
 * @note To information:
 * http://dbus.freedesktop.org/doc/dbus-specification.html#message-bus-routing-match-rules
 */
EAPI Eina_Bool             eldbus_signal_handler_match_extra_vset(Eldbus_Signal_Handler *sh, va_list ap) EINA_ARG_NONNULL(1);

/**
 * @brief Add a callback function to be called when signal handler will be freed.
 */
EAPI void                  eldbus_signal_handler_free_cb_add(Eldbus_Signal_Handler *handler, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Remove callback registered in eldbus_signal_handler_free_cb_add().
 */
EAPI void                  eldbus_signal_handler_free_cb_del(Eldbus_Signal_Handler *handler, Eldbus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

EAPI const char           *eldbus_signal_handler_sender_get(const Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_signal_handler_path_get(const Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_signal_handler_interface_get(const Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_signal_handler_member_get(const Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_signal_handler_match_get(const Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

EAPI Eldbus_Connection     *eldbus_signal_handler_connection_get(const Eldbus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */
#endif
