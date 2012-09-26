#ifndef EDBUS_SIGNAL_HANDLER_H
#define EDBUS_SIGNAL_HANDLER_H 1

/**
 * @defgroup EDBus_Signal_Handler Signal Handler
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
EAPI EDBus_Signal_Handler *edbus_signal_handler_add(EDBus_Connection *conn, const char *sender, const char *path, const char *interface, const char *member, EDBus_Signal_Cb cb, const void *cb_data) EINA_ARG_NONNULL(1, 6);

/**
 * @brief Increase signal handler reference.
 */
EAPI EDBus_Signal_Handler *edbus_signal_handler_ref(EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1);
/**
 * @brief Decrease signal handler reference.
 * If reference == 0 signal handler will be freed.
 */
EAPI void                  edbus_signal_handler_unref(EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1);
/**
 * @brief Decrease signal handler reference like edbus_signal_handler_unref()
 * but if reference > 0 this signal handler will stop listen signals. In other
 * words will be canceled but memory will not be freed.
 */
EAPI void                  edbus_signal_handler_del(EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1);
/**
 * @brief Add extra argument in match of signal handler to obtain specifics signals.
 *
 * Example:
 * edbus_signal_handler_match_extra_set(sh, "arg0", "org.bansheeproject.Banshee", "arg1", "", NULL);
 * With this extra arguments this signal handler callback only will be called
 * when Banshee is started.
 *
 * @note For now is only supported argX.
 *
 * @param sh signal handler
 * @param ... variadic of key and value and must be ended with a NULL
 *
 * @note To information:
 * http://dbus.freedesktop.org/doc/dbus-specification.html#message-bus-routing-match-rules
 */
EAPI Eina_Bool             edbus_signal_handler_match_extra_set(EDBus_Signal_Handler *sh, ...) EINA_ARG_NONNULL(1) EINA_SENTINEL;
/**
 * @brief Add extra argument in match of signal handler to obtain specifics signals.
 *
 * Example:
 * edbus_signal_handler_match_extra_set(sh, "arg0", "org.bansheeproject.Banshee", "arg1", "", NULL);
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
EAPI Eina_Bool             edbus_signal_handler_match_extra_vset(EDBus_Signal_Handler *sh, va_list ap);

/**
 * @brief Add a callback function to be called when signal handler will be freed.
 */
EAPI void                  edbus_signal_handler_cb_free_add(EDBus_Signal_Handler *handler, EDBus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Remove callback registered in edbus_signal_handler_cb_free_add().
 */
EAPI void                  edbus_signal_handler_cb_free_del(EDBus_Signal_Handler *handler, EDBus_Free_Cb cb, const void *data) EINA_ARG_NONNULL(1, 2);

EAPI const char           *edbus_signal_handler_sender_get(const EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_signal_handler_path_get(const EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_signal_handler_interface_get(const EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_signal_handler_member_get(const EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_signal_handler_match_get(const EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

EAPI EDBus_Connection     *edbus_signal_handler_connection_get(const EDBus_Signal_Handler *handler) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @}
 */
#endif
