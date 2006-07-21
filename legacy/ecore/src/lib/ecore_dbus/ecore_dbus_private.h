/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef _ECORE_DBUS_PRIVATE_H
#define _ECORE_DBUS_PRIVATE_H

typedef struct _Ecore_DBus_Auth Ecore_DBus_Auth;
typedef unsigned char *(*Ecore_DBus_Auth_Transaction)(void *);

typedef enum _Ecore_DBus_Auth_Type
{
   ECORE_DBUS_AUTH_TYPE_EXTERNAL,
   ECORE_DBUS_AUTH_MAGIC_COOKIE,
   ECORE_DBUS_AUTH_TYPE_ECORE_DBUS_COOKIE_SHA1,
   ECORE_DBUS_AUTH_TYPE_KERBEROS_V4,
   ECORE_DBUS_AUTH_TYPE_SKEY
} Ecore_DBus_Auth_Type;

typedef enum _Ecore_DBus_Message_Type
{
   ECORE_DBUS_MESSAGE_TYPE_INVALID,
   ECORE_DBUS_MESSAGE_TYPE_METHOD_CALL,
   ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN,
   ECORE_DBUS_MESSAGE_TYPE_ERROR,
   ECORE_DBUS_MESSAGE_TYPE_SIGNAL
} Ecore_DBus_Message_Type;

struct _Ecore_DBus_Server
{
   Ecore_List2        __list_data;
   Ecore_Con_Server    *server;
   int                  authenticated;
   int                  auth_type;
   int                  auth_type_transaction;
   int                  cnt_msg;
};

struct _Ecore_DBus_Message
{
   /* header fields */
   unsigned char                byte_order;
   unsigned char                type;
   unsigned char                flags;
   unsigned char                protocol;
   unsigned int                 hlength;
   unsigned int                 blength;
   unsigned int                 serial;

   Ecore_DBus_Server           *ref_server;

   unsigned char               *header;
   unsigned int                 hpos;
   Ecore_DBus_Message_Field    *header_fields;
   unsigned char               *body;
   unsigned int                 bpos;
   Ecore_DBus_Message_Field    *body_fields;
   unsigned char               *signature;
};

struct _Ecore_DBus_Message_Field
{
   Ecore_List2          __list_data;
   Ecore_DBus_Data_Type   type;
   unsigned int           offset;
   unsigned int           count;  /* number of elements, usefull for structs, arrays, dicts */
   unsigned int           hfield; /* if the field is a header field, we need the type of it */
};

struct _Ecore_DBus_Auth
{
   char                        *name;
   unsigned int                 num_transactions;
   Ecore_DBus_Auth_Transaction  transactions[5];
};

/* ecore_dbus_utils.c */
void          _ecore_dbus_message_append_length(unsigned char **buf, unsigned int old_length, unsigned int size);
void          _ecore_dbus_message_increase_length(unsigned char **buf, unsigned int new_length);
void          _ecore_dbus_message_padding(unsigned char **buf, unsigned int *old_length, int size);
void          _ecore_dbus_message_padding_skip(unsigned int *old_length, int size);
void          _ecore_dbus_message_append_nbytes(unsigned char *buf, unsigned char *data, unsigned int data_length);
void          _ecore_dbus_message_append_byte(unsigned char *buf, unsigned char c);
void          _ecore_dbus_message_append_uint32(unsigned char *buf, unsigned int i);
unsigned char _ecore_dbus_message_read_byte(unsigned char *buf);
unsigned int  _ecore_dbus_message_read_uint32(unsigned char *buf);

/* ecore_dbus.c */
Ecore_DBus_Message       *_ecore_dbus_message_new(Ecore_DBus_Server *svr);
Ecore_DBus_Message_Field *_ecore_dbus_message_new_field(Ecore_DBus_Data_Type type, unsigned int offset);
/* ecore_dbus_marshal.c */
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_byte(unsigned char **buf, unsigned int *old_length, unsigned char c);
/*
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_boolean(unsigned char **buf, unsigned int *old_length, uint32_t i);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_int16(unsigned char **buf, unsigned int *old_length, int16_t i);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_uint16(unsigned char **buf, unsigned int *old_length, uint16_t i);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_int32(unsigned char **buf, unsigned int *old_length, int32_t i);
*/
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_uint32(unsigned char **buf, unsigned int *old_length, uint32_t i);
/*
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_int64(unsigned char **buf, unsigned int *old_length, int64_t i);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_uint64(unsigned char **buf, unsigned int *old_length, uint64_t i);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_double(unsigned char **buf, unsigned int *old_length, double i);
*/
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_string(unsigned char **buf, unsigned int *old_length, char *str);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_object_path(unsigned char **buf, unsigned int *old_length, char *str);
/*
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_signature(unsigned char **buf, unsigned int *old_length, char *str);
*/
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_array(unsigned char **msg, unsigned int *old_length, Ecore_DBus_Data_Type contained_type);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_struct_begin(unsigned char **msg, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_struct_end(unsigned char **msg, unsigned int *old_length);
/* 
 * FIXME: These aren't correct !
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_variant(unsigned char **msg, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_dict_entry(unsigned char **msg, unsigned int *old_length);
*/
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_data(unsigned char **buf, unsigned int *old_length, Ecore_DBus_Data_Type type, void *data);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_custom_header(unsigned char **buf, unsigned int *old_length, unsigned int code, Ecore_DBus_Data_Type type, void *data);
/* ecore_dbus_unmarshal.c */
/*
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_byte(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_boolean(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_int16(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_uint16(unsigned char *buf, unsigned int *old_length);
*/
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_int32(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_uint32(unsigned char *buf, unsigned int *old_length);
/*
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_int64(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_uint64(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_double(unsigned char *buf, unsigned int *old_length);
*/
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_string(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_object_path(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_signature(unsigned char *buf, unsigned int *old_length);
/*
 * FIXME: These aren't correct!
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_array(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_variant(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_struct(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_dict_entry(unsigned char *buf, unsigned int *old_length);
*/
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_data(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_custom_header(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message       *_ecore_dbus_message_unmarshal(Ecore_DBus_Server *svr, unsigned char *message);

/* Errors */
#define DBUS_ERROR_FAILED                     "org.freedesktop.DBus.Error.Failed"
#define DBUS_ERROR_NO_MEMORY                  "org.freedesktop.DBus.Error.NoMemory"
#define DBUS_ERROR_ACTIVATE_SERVICE_NOT_FOUND "org.freedesktop.DBus.Error.ServiceNotFound"
#define DBUS_ERROR_SERVICE_DOES_NOT_EXIST     "org.freedesktop.DBus.Error.ServiceDoesNotExist"
#define DBUS_ERROR_SERVICE_HAS_NO_OWNER       "org.freedesktop.DBus.Error.ServiceHasNoOwner"
#define DBUS_ERROR_NO_REPLY                   "org.freedesktop.DBus.Error.NoReply"
#define DBUS_ERROR_IO_ERROR                   "org.freedesktop.DBus.Error.IOError"
#define DBUS_ERROR_BAD_ADDRESS                "org.freedesktop.DBus.Error.BadAddress"
#define DBUS_ERROR_NOT_SUPPORTED              "org.freedesktop.DBus.Error.NotSupported"
#define DBUS_ERROR_LIMITS_EXCEEDED            "org.freedesktop.DBus.Error.LimitsExceeded"
#define DBUS_ERROR_ACCESS_DENIED              "org.freedesktop.DBus.Error.AccessDenied"
#define DBUS_ERROR_AUTH_FAILED                "org.freedesktop.DBus.Error.AuthFailed"
#define DBUS_ERROR_NO_SERVER                  "org.freedesktop.DBus.Error.NoServer"
#define DBUS_ERROR_TIMEOUT                    "org.freedesktop.DBus.Error.Timeout"
#define DBUS_ERROR_NO_NETWORK                 "org.freedesktop.DBus.Error.NoNetwork"
#define DBUS_ERROR_ADDRESS_IN_USE             "org.freedesktop.DBus.Error.AddressInUse"
#define DBUS_ERROR_DISCONNECTED               "org.freedesktop.DBus.Error.Disconnected"
#define DBUS_ERROR_INVALID_ARGS               "org.freedesktop.DBus.Error.InvalidArgs"
#define DBUS_ERROR_FILE_NOT_FOUND             "org.freedesktop.DBus.Error.FileNotFound"
#define DBUS_ERROR_UNKNOWN_METHOD             "org.freedesktop.DBus.Error.UnknownMethod"
#define DBUS_ERROR_TIMED_OUT                  "org.freedesktop.DBus.Error.TimedOut"
#define DBUS_ERROR_MATCH_RULE_NOT_FOUND       "org.freedesktop.DBus.Error.MatchRuleNotFound"
#define DBUS_ERROR_MATCH_RULE_INVALID         "org.freedesktop.DBus.Error.MatchRuleInvalid"
#define DBUS_ERROR_SPAWN_EXEC_FAILED          "org.freedesktop.DBus.Error.Spawn.ExecFailed"
#define DBUS_ERROR_SPAWN_FORK_FAILED          "org.freedesktop.DBus.Error.Spawn.ForkFailed"
#define DBUS_ERROR_SPAWN_CHILD_EXITED         "org.freedesktop.DBus.Error.Spawn.ChildExited"
#define DBUS_ERROR_SPAWN_CHILD_SIGNALED       "org.freedesktop.DBus.Error.Spawn.ChildSignaled"
#define DBUS_ERROR_SPAWN_FAILED               "org.freedesktop.DBus.Error.Spawn.Failed"
#define DBUS_ERROR_UNIX_PROCESS_ID_UNKNOWN    "org.freedesktop.DBus.Error.UnixProcessIdUnknown"

#endif
