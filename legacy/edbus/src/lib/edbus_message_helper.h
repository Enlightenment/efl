typedef void (*EDBus_Dict_Cb_Get)(void *data, const void *key, EDBus_Message_Iter *var);

/**
 * Iterate over a dictionary.
 *
 * @param dict iterator with array of entry
 * @param signature of entry, example: "sv"
 * @param cb callback that will be called in each entry
 * @param data context data
 */
EAPI void edbus_message_iter_dict_iterate(EDBus_Message_Iter *dict, const char *signature, EDBus_Dict_Cb_Get cb, const void *data);
