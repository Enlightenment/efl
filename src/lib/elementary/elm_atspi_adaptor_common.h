
#define ELM_ACCESS_OBJECT_PATH_PREFIX2  "/org/a11y/atspi/accessible"

#define ELM_ATSPI_PROPERTY_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, class, msg, error) \
   if (!(obj) || !efl_isa(obj, class)) \
     { \
        *(error) = _dbus_invalid_ref_error_new(msg); \
        return EINA_FALSE; \
     }

#define ELM_ATSPI_PROPERTY_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, class, msg, error) \
   if (!(obj) || !efl_isa(obj, class)) \
     { \
        *(error) = _dbus_invalid_ref_error_new(msg); \
        return EINA_FALSE; \
     }

#define ELM_ATSPI_OBJ_CHECK_OR_RETURN_DBUS_ERROR(obj, class, msg) \
   if (!(obj) || !efl_isa(obj, class)) \
     return _dbus_invalid_ref_error_new(msg);

static inline Eldbus_Message *_dbus_invalid_ref_error_new(const Eldbus_Message *msg)
{
  return eldbus_message_error_new(msg, "org.freedesktop.DBus.Error.UnknownObject", "Path is not valid accessible object reference.");
}

Efl_Access* efl_access_unmarshal(const Eldbus_Message *msg);

void efl_access_marshal(Efl_Access *obj, Eldbus_Message_Iter *iter);

void eldbus_message_iter_efl_access_reference_append(Eldbus_Message_Iter *iter, Eldbus_Connection *conn, Efl_Access *obj);

void eldbus_message_iter_efl_access_interfaces_append(Eldbus_Message_Iter *iter, Efl_Access *obj);

void eldbus_message_iter_efl_access_reference_array_append(Eldbus_Message_Iter *iter, Eldbus_Connection *conn, Eina_List *obj);
