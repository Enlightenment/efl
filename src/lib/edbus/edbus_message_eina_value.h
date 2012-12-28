/**
 * @ingroup EDBus_Message
 * @defgroup Eina_Value Eina_Value to/from Message
 * @{
 */

/**
 * Convert EDBus_Message to Eina_Value
 *
 * @param msg Message
 * @return Eina_Value of type Eina_Value_Type_Struct
 */
EAPI Eina_Value *edbus_message_to_eina_value(const EDBus_Message *msg) EINA_ARG_NONNULL(1) EINA_WARN_UNUSED_RESULT;

/**
 * Convert EDBus_Message_Iter of type variant, struct or dict entry to
 * Eina_Value.
 *
 * @param iter Message iterator
 * @return Eina_Value of type Eina_Value_Type_Struct
 */
EAPI Eina_Value *edbus_message_iter_struct_like_to_eina_value(const EDBus_Message_Iter *iter);

/**
 * Convert Eina_Value to EDBus_Message
 *
 * @param signature dbus signature
 * @param msg where data will be put
 * @param value source of data, must be of type Eina_Value_Type_Struct
 * @return success or not
 */
EAPI Eina_Bool edbus_message_from_eina_value(const char *signature, EDBus_Message *msg, const Eina_Value *value) EINA_ARG_NONNULL(1, 2, 3);

/**
 * @}
 */
