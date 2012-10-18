#ifndef EDBUS_MESSAGE_H
#define EDBUS_MESSAGE_H 1

/**
 * @defgroup EDBus_Message Message
 *
 * @{
 */

/**
 * @brief Constructs a new message to invoke a method on a remote object.
 *
 * @param dest bus name or unique id of the remote applications
 * @param path object path
 * @param iface interface name
 * @param method name of method that will be called
 *
 * @return a new EDBus_Message, free with edbus_message_unref()
 */
EAPI EDBus_Message        *edbus_message_method_call_new(const char *dest, const char *path, const char *iface, const char *method) EINA_ARG_NONNULL(1, 2, 3, 4) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * @brief Increase message reference.
 */
EAPI EDBus_Message        *edbus_message_ref(EDBus_Message *msg) EINA_ARG_NONNULL(1);
/**
 * @brief Decrease message reference.
 * If reference == 0 message will be freed and all its children.
 */
EAPI void                  edbus_message_unref(EDBus_Message *msg) EINA_ARG_NONNULL(1);

EAPI const char           *edbus_message_path_get(const EDBus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_message_interface_get(const EDBus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_message_member_get(const EDBus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_message_destination_get(const EDBus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_message_sender_get(const EDBus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *edbus_message_signature_get(const EDBus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Creates a new message that is an error reply to another message.
 *
 * @param reply the message we're replying to
 * @param error_name the error name
 * @param error_msg the error message string
 *
 * @return new EDBus_Message, free with edbus_message_unref()
 */
EAPI EDBus_Message        *edbus_message_error_new(const EDBus_Message *reply, const char *error_name, const char *error_msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Constructs a message that is a reply to a method call.
 *
 * @param msg the message we're replying to
 *
 * @return new EDBus_Message, free with edbus_message_unref()
 */
EAPI EDBus_Message        *edbus_message_method_return_new(const EDBus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Constructs a new message representing a signal emission.
 *
 * @param path of the object that was emiting the signal
 * @param interface
 * @param name
 *
 * @return new EDBus_Message, free with edbus_message_unref()
 */
EAPI EDBus_Message        *edbus_message_signal_new(const char *path, const char *interface, const char *name) EINA_ARG_NONNULL(1, 2, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief If EDBus_Message is a message error return EINA_TRUE and fills
 * name and text if their pointers is not null.
 */
EAPI Eina_Bool             edbus_message_error_get(const EDBus_Message *msg, const char **name, const char **text) EINA_ARG_NONNULL(1);

/**
 * @brief Get data from EDBus_Message. For each complete type we must have
 * a pointer to store its value. In case of complex type EDBus_Message_Iter
 * needs to be need.
 */
EAPI Eina_Bool             edbus_message_arguments_get(const EDBus_Message *msg, const char *signature, ...) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Get data from EDBus_Message. For each complete type we must have
 * a pointer to store its value, in case of complex type
 * EDBus_Message_Iter needs to be used.
 */
EAPI Eina_Bool             edbus_message_arguments_vget(const EDBus_Message *msg, const char *signature, va_list ap) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Set data to EDBus_Message.
 *
 * This function only supports basic type, for complex types use
 * edbus_message_iter_* functions.
 */
EAPI Eina_Bool             edbus_message_arguments_set(EDBus_Message *msg, const char *signature, ...) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Set data to EDBus_Message.
 *
 * This function only supports basic types, for complex types use
 * edbus_message_iter_* functions.
 */
EAPI Eina_Bool             edbus_message_arguments_vset(EDBus_Message *msg, const char *signature, va_list ap) EINA_ARG_NONNULL(1, 2);

/**
 * @defgroup EDBus_Message_Iter Iterator
 * @{
 */
/**
 * @brief Create and appends a typed iterator to another iterator.
 *
 * After append data to returned iterator it must be closed calling
 * edbus_message_iter_container_close().
 *
 * Container types are for example struct, variant, and array.
 * For variants, the contained_signature should be the type of the single
 * value inside the variant. For structs and dict entries, contained_signature
 * should be NULL; it will be set to whatever types you write into the struct.
 * For arrays, contained_signature should be the type of the array elements.
 *
 * @param iter parent of the new iterator
 * @param type of iterator (e.g struct, dict, variant or array)
 * @param contained_signature signature of what iterator will store
 *
 * @return the new iterator
 */
EAPI EDBus_Message_Iter *edbus_message_iter_container_new(EDBus_Message_Iter *iter, int type, const char* contained_signature) EINA_ARG_NONNULL(1, 3) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Append a basic type to EDBus_Iterator.
 */
EAPI Eina_Bool               edbus_message_iter_basic_append(EDBus_Message_Iter *iter, int type, ...) EINA_ARG_NONNULL(1, 3);
/**
 * @brief Set data to EDBus_Message_Iter. For each complete in signature
 * you need pass the value, in case of complex type a pointer to be allocated a
 * EDBus_Message_Iter that you need fill and close.
 *
 * It's not possible open two iterators at same Iterator. Example:
 * "aiai", to set this you need create and put the first array with
 * edbus_message_iter_container_new() fill array with data and close then
 * you could open the second array with edbus_message_iter_container_new().
 *
 * @param iter iterator
 * @param signature of data
 * @param ... values
 *
 * @note This function don't support variant, use instead
 * edbus_message_iter_container_new() to create the variant fill
 * data and close it..
 */
EAPI Eina_Bool               edbus_message_iter_arguments_set(EDBus_Message_Iter *iter, const char *signature, ...) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Set data to EDBus_Message_Iter. For each complete in signature
 * you need pass the value, in case of complex type a pointer to be allocated a
 * EDBus_Message_Iter that you need fill and close.
 *
 * It's not possible open two iterators at same Iterator. Example:
 * "aiai", to set this you need create and put the first array with
 * edbus_message_iter_container_new() fill array with data and close then
 * you could open the second array with edbus_message_iter_container_new().
 *
 * @param iter iterator
 * @param signature of data
 * @param ap va_list with the values
 *
 * @note This function don't support variant, use instead
 * edbus_message_iter_container_new() to create the variant fill
 * data and close it.
 */
EAPI Eina_Bool               edbus_message_iter_arguments_vset(EDBus_Message_Iter *iter, const char *signature, va_list ap) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Closes a container-typed value appended to the message.
 *
 * @param iter parent of the sub-iterator
 * @param sub the iterator that will be closed
 *
 * @return EINA_FALSE if iterator was already close or if not enough memory
 */
EAPI Eina_Bool               edbus_message_iter_container_close(EDBus_Message_Iter *iter, EDBus_Message_Iter *sub) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Get the main EDBus_Message_Iter from the EDBus_Message.
 */
EAPI EDBus_Message_Iter *edbus_message_iter_get(const EDBus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Get a basic type from EDBus_Iterator.
 */
EAPI void                    edbus_message_iter_basic_get(EDBus_Message_Iter *iter, void *value) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Returns the current signature of a message iterator.
 *
 * @note The returned string must be freed.
 */
EAPI char                   *edbus_message_iter_signature_get(EDBus_Message_Iter *iter) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
/**
 * @brief Moves the iterator to the next field, if any.
 * @param iter iterator
 *
 * @return if iterator was reach to end return EINA_FALSE
 */
EAPI Eina_Bool               edbus_message_iter_next(EDBus_Message_Iter *iter) EINA_ARG_NONNULL(1);
/**
 * @brief Get a complete type from EDBus_Message_Iter if is not at the end
 * of iterator and move to next field.
 * Useful to iterate over arrays.
 *
 * @param iter iterator
 * @param type of the next completed type in Iterator
 * @param ... pointer of where data will be stored
 *
 * @param if iterator was reach to end or if type different of the type that
 * iterator points return EINA_FALSE
 *
 */
EAPI Eina_Bool               edbus_message_iter_get_and_next(EDBus_Message_Iter *iter, char type, ...) EINA_ARG_NONNULL(1, 2, 3);
/**
 * @brief Reads a block of fixed-length values from the message iterator.
 *
 * Fixed-length values are those basic types that are not string-like,
 * such as integers, bool, double. The returned block will be from the
 * current position in the array until the end of the array.
 *
 * There is one exception here: although EDBUS_TYPE_UNIX_FD is considered a
 * 'fixed' type arrays of this type may not be read with this function.
 *
 * The value argument should be the address of a location to store the returned
 * array. So for int32 it should be a "const dbus_int32_t**" The returned value
 * is by reference and should not be freed.
 *
 * Because the array is not copied, this function runs in constant time and is
 * fast; it's much preferred over walking the entire array with an iterator.
 */
EAPI Eina_Bool edbus_message_iter_fixed_array_get(EDBus_Message_Iter *iter, int signature, void *value, int *n_elements) EINA_ARG_NONNULL(1, 3, 4);
/**
 * @brief Get data from EDBus_Message_Iter, for each complete type must have
 * a pointer to store his value, in case of complex type a
 * EDBus_Message_Iter will be need.
 *
 * @param iter iterator
 * @param signature of the complete data types on interator
 * @param ... pointers of where data will be stored
 *
 * @return EINA_FALSE if signature different from signature in iterator
 */
EAPI Eina_Bool               edbus_message_iter_arguments_get(EDBus_Message_Iter *iter, const char *signature, ...) EINA_ARG_NONNULL(1, 2);
/**
 * @brief Get data from EDBus_Message_Iter, for each complete type must have
 * a pointer to store his value, in case of complex type a
 * EDBus_Message_Iter will be need.
 *
 * @param iter iterator
 * @param signature of the complete data types on interator
 * @param ap va_list of the pointers of where data will be stored
 *
 * @return EINA_FALSE if signature different from signature in iterator
 */
EAPI Eina_Bool               edbus_message_iter_arguments_vget(EDBus_Message_Iter *iter, const char *signature, va_list ap) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Manually delete the iterator.
 *
 * Iterators are usually bound to the life of @ref EDBus_Message
 * they were created from, being deleted automatically once the
 * message is deleted.
 *
 * However when dealing with huge arrays or dicts it may become a
 * major memory impact to leave the unused iterators alive. By
 * calling this function one states the iterator is not used anymore
 * and can be deleted.
 *
 * @param iter the iterator to be deleted.
 */
EAPI void                  edbus_message_iter_del(EDBus_Message_Iter *iter) EINA_ARG_NONNULL(1);
/**
 * @}
 */
/**
 * @}
 */
#endif
