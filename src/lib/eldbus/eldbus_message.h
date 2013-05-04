#ifndef ELDBUS_MESSAGE_H
#define ELDBUS_MESSAGE_H 1

/**
 * @defgroup Eldbus_Message Message
 * @ingroup Eldbus
 *
 * @{
 */

/**
 * @brief Increase message reference.
 */
EAPI Eldbus_Message        *eldbus_message_ref(Eldbus_Message *msg) EINA_ARG_NONNULL(1);

/**
 * @brief Decrease message reference.
 *
 * When refcount reaches zero the message and all its resources will be
 * freed.
 */
EAPI void                  eldbus_message_unref(Eldbus_Message *msg) EINA_ARG_NONNULL(1);

EAPI const char           *eldbus_message_path_get(const Eldbus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_message_interface_get(const Eldbus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_message_member_get(const Eldbus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_message_destination_get(const Eldbus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_message_sender_get(const Eldbus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;
EAPI const char           *eldbus_message_signature_get(const Eldbus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Create a new message to invoke a method on a remote object.
 *
 * @param dest bus name or unique id of the remote application
 * @param path object path
 * @param iface interface name
 * @param method name of the method to be called
 *
 * @return a new Eldbus_Message, free with eldbus_message_unref()
 */
EAPI Eldbus_Message        *eldbus_message_method_call_new(const char *dest, const char *path, const char *iface, const char *method) EINA_ARG_NONNULL(1, 2, 3, 4) EINA_WARN_UNUSED_RESULT EINA_MALLOC;

/**
 * @brief Create a new message that is an error reply to another message.
 *
 * @param msg the message we're replying to
 * @param error_name the error name
 * @param error_msg the error message string
 *
 * @return a new Eldbus_Message, free with eldbus_message_unref()
 */
EAPI Eldbus_Message        *eldbus_message_error_new(const Eldbus_Message *msg, const char *error_name, const char *error_msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Create a message that is a reply to a method call.
 *
 * @param msg the message we're replying to
 *
 * @return new Eldbus_Message, free with eldbus_message_unref()
 */
EAPI Eldbus_Message        *eldbus_message_method_return_new(const Eldbus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;


/**
 * @brief Get the error text and name from a Eldbus_Message.
 *
 * If @param msg is an error message return EINA_TRUE and fill in the name and
 * text of the error.
 *
 * @param msg The message containing the error
 * @param name Variable in which to store the error name or @c NULL if it's not
 * desired.
 * @param text Variable in which to store the error text or @c NULL if it's not
 * desired.
 */
EAPI Eina_Bool             eldbus_message_error_get(const Eldbus_Message *msg, const char **name, const char **text) EINA_ARG_NONNULL(1);

/**
 * @brief Get the arguments from an Eldbus_Message
 *
 * Get the arguments of an Eldbus_Message storing them in the locations pointed
 * to by the pointer arguments that follow @param signature. Each pointer
 * argument must be of a type that is appropriate for the correspondent complete
 * type in @param signature. For complex types such as arrays, structs,
 * dictionaries or variants, a pointer to Eldbus_Message_Iter* must be provided.
 *
 * @param msg The message to get the arguments from.
 * @param signature The signature of the arguments user is expecting to read
 * from @param msg
 * @param ... The pointers in which to store the message arguments
 *
 * @return EINA_TRUE if the arguments were read succesfully and stored in the
 * respective pointer arguments.
 */
EAPI Eina_Bool             eldbus_message_arguments_get(const Eldbus_Message *msg, const char *signature, ...) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get the arguments from an Eldbus_Message using a va_list.
 *
 * @param msg The message to get the arguments from.
 * @param signature The signature user is expecting to read from @param msg.
 * @param ap The va_list containing the pointer arguments.
 *
 * @see eldbus_message_arguments_get()
 *
 * @return EINA_TRUE if the arguments were read succesfully and stored in the
 * respective pointer arguments.
 */
EAPI Eina_Bool             eldbus_message_arguments_vget(const Eldbus_Message *msg, const char *signature, va_list ap) EINA_ARG_NONNULL(1, 2) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Append arguments into an Eldbus_Message
 *
 * Append arguments into an Eldbus_Message according to the @param signature
 * provided. For each complete type in @param signature, a value of the
 * corresponding type must be provided.
 *
 * This function supports only basic types. For complex types use
 * eldbus_message_iter_* family of functions.
 *
 * @param msg The message in which arguments are being appended.
 * @param signature Signature of the arguments that are being appended.
 * @param ... Values of each argument to append in @param msg.
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool             eldbus_message_arguments_append(Eldbus_Message *msg, const char *signature, ...) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Append arguments into an Eldbus_Message using a va_list.
 *
 * @param msg The message in which arguments are being appended.
 * @param signature Signature of the arguments that are being appended.
 * @param ap The va_list containing the arguments to append.
 *
 * @see eldbus_message_arguments_append().
 *
 * @return EINA_TRUE on success, EINA_FALSE otherwise.
 */
EAPI Eina_Bool             eldbus_message_arguments_vappend(Eldbus_Message *msg, const char *signature, va_list ap) EINA_ARG_NONNULL(1, 2);

/**
 * @defgroup Eldbus_Message_Iter Iterator
 * @{
 */

/**
 * @brief Create and append a typed iterator to another iterator.
 *
 * After append data to returned iterator it must be closed calling
 * eldbus_message_iter_container_close().
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
EAPI Eldbus_Message_Iter *eldbus_message_iter_container_new(Eldbus_Message_Iter *iter, int type, const char* contained_signature) EINA_ARG_NONNULL(1, 3) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Append a basic type into an Eldbus_Iterator.
 */
EAPI Eina_Bool               eldbus_message_iter_basic_append(Eldbus_Message_Iter *iter, int type, ...) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Append an argument into an Eldbus_Message_Iter. For each complete type
 * you need to provide the correspondent value. In case of complex types you
 * need to provide an Eldbus_Message_Iter** to be allocated and then filled in.
 *
 * It's not possible to open two iterators at same iterator with this function.
 * For example, to create a message with signature="aiai" you need to create the
 * first container with eldbus_message_iter_container_new(), fill the array,
 * close it with eldbus_message_iter_container_close() and then do the same for
 * the second array.
 *
 * @param iter iterator in which data will be appended
 * @param signature signature of the contained data
 * @param ... values for each complete type
 *
 * @see eldbus_message_iter_container_new()
 * @see eldbus_message_iter_container_close()
 *
 * @note This function doesn't support variant, use
 * eldbus_message_iter_container_new() instead to create the variant, fill
 * with data and close it.
 */
EAPI Eina_Bool               eldbus_message_iter_arguments_append(Eldbus_Message_Iter *iter, const char *signature, ...) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Set data to Eldbus_Message_Iter. For each complete in signature
 * you need pass the value, in case of complex type a pointer to be allocated a
 * Eldbus_Message_Iter that you need fill and close.
 *
 * It's not possible open two iterators at same Iterator. Example:
 * "aiai", to set this you need create and put the first array with
 * eldbus_message_iter_container_new() fill array with data and close then
 * you could open the second array with eldbus_message_iter_container_new().
 *
 * @param iter iterator
 * @param signature of data
 * @param ap va_list with the values
 *
 * @note This function don't support variant, use instead
 * eldbus_message_iter_container_new() to create the variant fill
 * data and close it.
 */
EAPI Eina_Bool               eldbus_message_iter_arguments_vappend(Eldbus_Message_Iter *iter, const char *signature, va_list ap) EINA_ARG_NONNULL(1, 2, 3);


/**
 * Append a array of basic type with fixed size to Eldbus_Message_Iter.
 *
 * @param iter iterator
 * @param type basic type that will be appended
 * @param array data to append
 * @param size of array
 */
EAPI Eina_Bool               eldbus_message_iter_fixed_array_append(Eldbus_Message_Iter *iter, int type, const void *array, unsigned int size) EINA_ARG_NONNULL(1, 3);

/**
 * @brief Closes a container-typed value appended to the message.
 *
 * @param iter parent of the sub-iterator
 * @param sub the iterator that will be closed
 *
 * @return EINA_FALSE if iterator was already close or if not enough memory
 */
EAPI Eina_Bool               eldbus_message_iter_container_close(Eldbus_Message_Iter *iter, Eldbus_Message_Iter *sub) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Get the main Eldbus_Message_Iter from the Eldbus_Message.
 */
EAPI Eldbus_Message_Iter *eldbus_message_iter_get(const Eldbus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Get a basic type from Eldbus_Iterator.
 */
EAPI void                    eldbus_message_iter_basic_get(Eldbus_Message_Iter *iter, void *value) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Returns the current signature of a message iterator.
 *
 * @note The returned string must be freed.
 */
EAPI char                   *eldbus_message_iter_signature_get(Eldbus_Message_Iter *iter) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * @brief Moves the iterator to the next field, if any.
 * @param iter iterator
 *
 * @return if iterator was reach to end return EINA_FALSE
 */
EAPI Eina_Bool               eldbus_message_iter_next(Eldbus_Message_Iter *iter) EINA_ARG_NONNULL(1);

/**
 * @brief Get a complete type from Eldbus_Message_Iter if is not at the end
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
EAPI Eina_Bool               eldbus_message_iter_get_and_next(Eldbus_Message_Iter *iter, char type, ...) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @brief Reads a block of fixed-length values from the message iterator.
 *
 * Fixed-length values are those basic types that are not string-like,
 * such as integers, bool, double. The returned block will be from the
 * current position in the array until the end of the array.
 *
 * There is one exception here: although ELDBUS_TYPE_UNIX_FD is considered a
 * 'fixed' type arrays of this type may not be read with this function.
 *
 * The value argument should be the address of a location to store the returned
 * array. So for int32 it should be a "const dbus_int32_t**" The returned value
 * is by reference and should not be freed.
 *
 * Because the array is not copied, this function runs in constant time and is
 * fast; it's much preferred over walking the entire array with an iterator.
 */
EAPI Eina_Bool eldbus_message_iter_fixed_array_get(Eldbus_Message_Iter *iter, int signature, void *value, int *n_elements) EINA_ARG_NONNULL(1, 3, 4);

/**
 * @brief Get data from Eldbus_Message_Iter, for each complete type must have
 * a pointer to store his value, in case of complex type a
 * Eldbus_Message_Iter will be need.
 *
 * @param iter iterator
 * @param signature of the complete data types on interator
 * @param ... pointers of where data will be stored
 *
 * @return EINA_FALSE if signature different from signature in iterator
 */
EAPI Eina_Bool               eldbus_message_iter_arguments_get(Eldbus_Message_Iter *iter, const char *signature, ...) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Get data from Eldbus_Message_Iter, for each complete type must have
 * a pointer to store his value, in case of complex type a
 * Eldbus_Message_Iter will be need.
 *
 * @param iter iterator
 * @param signature of the complete data types on interator
 * @param ap va_list of the pointers of where data will be stored
 *
 * @return EINA_FALSE if signature different from signature in iterator
 */
EAPI Eina_Bool               eldbus_message_iter_arguments_vget(Eldbus_Message_Iter *iter, const char *signature, va_list ap) EINA_ARG_NONNULL(1, 2);

/**
 * @brief Manually delete the iterator.
 *
 * Iterators are usually bound to the life of @ref Eldbus_Message
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
EAPI void                  eldbus_message_iter_del(Eldbus_Message_Iter *iter) EINA_ARG_NONNULL(1);

/**
 * @}
 */

#include "eldbus_message_helper.h"
#include "eldbus_message_eina_value.h"

/**
 * @}
 */
#endif
