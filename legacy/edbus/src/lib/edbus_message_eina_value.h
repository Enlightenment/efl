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
EAPI Eina_Value *edbus_message_to_eina_value(const EDBus_Message *msg);

/**
 * Convert Eina_Value to EDBus_Message
 *
 * @param signature dbus signature
 * @param msg where data will be put
 * @param value source of data, must be of type Eina_Value_Type_Struct
 * @return success or not
 */
EAPI Eina_Bool edbus_message_from_eina_value(const char *signature, EDBus_Message *msg, const Eina_Value *value);

/**
 * @}
 */
