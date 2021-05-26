/**
 * @ingroup Eldbus_Message
 * @defgroup Eldbus_Message_Helpers Message Helpers
 *
 * @{
 */

/**
 * @typedef Eldbus_Dict_Cb_Get
 *
 * Callback used when iterating over a dictionary. It is called at each entry iteration.
 *
 * @param data Context data passed to the callback.
 * @param key Key parameter.
 * @param var A message iterator on the current entry.
 */
typedef void (*Eldbus_Dict_Cb_Get)(void *data, const void *key, Eldbus_Message_Iter *var);

/**
 * Iterate over a dictionary.
 *
 * @param dict iterator with array of entry
 * @param signature of entry, example: "sv"
 * @param cb callback that will be called in each entry
 * @param data context data
 */
EAPI void eldbus_message_iter_dict_iterate(Eldbus_Message_Iter *dict, const char *signature, Eldbus_Dict_Cb_Get cb, const void *data) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @}
 */
