/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef _ECORE_DBUS_PRIVATE_H
#define _ECORE_DBUS_PRIVATE_H

typedef unsigned char *(*Ecore_DBus_Auth_Transaction)(void *);

typedef struct _Ecore_DBus_Auth                      Ecore_DBus_Auth;

typedef struct _Ecore_DBus_Message_Field_Container   Ecore_DBus_Message_Field_Container;

#define ECORE_DBUS_MESSAGE_FIELD(x) ((Ecore_DBus_Message_Field *)(x))
#define ECORE_DBUS_MESSAGE_FIELD_CONTAINER(x) ((Ecore_DBus_Message_Field_Container *)(x))
typedef struct _Ecore_DBus_Message_Field_Byte        Ecore_DBus_Message_Field_Byte;
typedef struct _Ecore_DBus_Message_Field_Boolean     Ecore_DBus_Message_Field_Boolean;
typedef struct _Ecore_DBus_Message_Field_Int16       Ecore_DBus_Message_Field_Int16;
typedef struct _Ecore_DBus_Message_Field_UInt16      Ecore_DBus_Message_Field_UInt16;
typedef struct _Ecore_DBus_Message_Field_Int32       Ecore_DBus_Message_Field_Int32;
typedef struct _Ecore_DBus_Message_Field_UInt32      Ecore_DBus_Message_Field_UInt32;
typedef struct _Ecore_DBus_Message_Field_Int64       Ecore_DBus_Message_Field_Int64;
typedef struct _Ecore_DBus_Message_Field_UInt64      Ecore_DBus_Message_Field_UInt64;
typedef struct _Ecore_DBus_Message_Field_Double      Ecore_DBus_Message_Field_Double;
typedef struct _Ecore_DBus_Message_Field_String      Ecore_DBus_Message_Field_String;
typedef struct _Ecore_DBus_Message_Field_Object_Path Ecore_DBus_Message_Field_Object_Path;
typedef struct _Ecore_DBus_Message_Field_Signature   Ecore_DBus_Message_Field_Signature;
typedef struct _Ecore_DBus_Message_Field_Array       Ecore_DBus_Message_Field_Array;
typedef struct _Ecore_DBus_Message_Field_Variant     Ecore_DBus_Message_Field_Variant;
typedef struct _Ecore_DBus_Message_Field_Struct      Ecore_DBus_Message_Field_Struct;
typedef struct _Ecore_DBus_Message_Field_Dict_Entry  Ecore_DBus_Message_Field_Dict_Entry;

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
   /* server to send to */
   Ecore_DBus_Server *server;

   /* header fields */
   unsigned char      byte_order;
   unsigned char      type;
   unsigned char      flags;
   unsigned char      protocol;
   unsigned int       serial;
   unsigned int       body_length;
   unsigned char     *signature;

   /* message */
   unsigned int       size;
   unsigned int       length;
   unsigned char     *buffer;

   void              *header;
   Ecore_List        *fields;
   Ecore_List        *recurse;
};

struct _Ecore_DBus_Message_Field
{
   Ecore_DBus_Data_Type   type;
   unsigned int           offset;
   unsigned char         *buffer;
};

struct _Ecore_DBus_Message_Field_Container
{
   Ecore_DBus_Message_Field __field;
   Ecore_List                *values;
};

struct _Ecore_DBus_Message_Field_Byte
{
   Ecore_DBus_Message_Field __field;
   unsigned char              value;
};

struct _Ecore_DBus_Message_Field_Boolean
{
   Ecore_DBus_Message_Field __field;
   unsigned int               value;
};

struct _Ecore_DBus_Message_Field_Int16
{
   Ecore_DBus_Message_Field __field;
   short                      value;
};

struct _Ecore_DBus_Message_Field_UInt16
{
   Ecore_DBus_Message_Field __field;
   unsigned short             value;
};

struct _Ecore_DBus_Message_Field_Int32
{
   Ecore_DBus_Message_Field __field;
   int                        value;
};

struct _Ecore_DBus_Message_Field_UInt32
{
   Ecore_DBus_Message_Field __field;
   unsigned int               value;
};

struct _Ecore_DBus_Message_Field_Int64
{
   Ecore_DBus_Message_Field __field;
   long long                  value;
};

struct _Ecore_DBus_Message_Field_UInt64
{
   Ecore_DBus_Message_Field __field;
   unsigned long long         value;
};

struct _Ecore_DBus_Message_Field_Double
{
   Ecore_DBus_Message_Field __field;
   double                     value;
};

struct _Ecore_DBus_Message_Field_String
{
   Ecore_DBus_Message_Field __field;
   char                      *value;
};

struct _Ecore_DBus_Message_Field_Object_Path
{
   Ecore_DBus_Message_Field __field;
   char                      *value;
};

struct _Ecore_DBus_Message_Field_Signature
{
   Ecore_DBus_Message_Field __field;
   char                      *value;
};

struct _Ecore_DBus_Message_Field_Array
{
   Ecore_DBus_Message_Field_Container __field;
   Ecore_DBus_Data_Type                 contained_type;
   unsigned int                         start;
   unsigned int                         end;
};

struct _Ecore_DBus_Message_Field_Variant
{
   Ecore_DBus_Message_Field_Container __field;
   Ecore_DBus_Data_Type                 contained_type;
   void                                *value;
};

struct _Ecore_DBus_Message_Field_Struct
{
   Ecore_DBus_Message_Field_Container __field;
};

struct _Ecore_DBus_Message_Field_Dict_Entry
{
   Ecore_DBus_Message_Field __field;
   Ecore_DBus_Message_Field  *key;
   Ecore_DBus_Message_Field  *value;
};

struct _Ecore_DBus_Auth
{
   char                        *name;
   unsigned int                 num_transactions;
   Ecore_DBus_Auth_Transaction  transactions[5];
};

/* ecore_dbus_message.c */
Ecore_DBus_Message *_ecore_dbus_message_new(Ecore_DBus_Server *svr);
void                _ecore_dbus_message_free(Ecore_DBus_Message *msg);
void               *_ecore_dbus_message_field_new(Ecore_DBus_Message *msg, Ecore_DBus_Data_Type type);
/* ecore_dbus_utils.c */
void          _ecore_dbus_message_padding(Ecore_DBus_Message *msg, unsigned int size);
void          _ecore_dbus_message_padding_skip(Ecore_DBus_Message *msg, unsigned int size);
void          _ecore_dbus_message_append_bytes(Ecore_DBus_Message *msg, unsigned char *c, unsigned int size);
void          _ecore_dbus_message_append_byte(Ecore_DBus_Message *msg, unsigned char c);
void          _ecore_dbus_message_append_uint32(Ecore_DBus_Message *msg, unsigned int i);
unsigned char _ecore_dbus_message_read_byte(Ecore_DBus_Message *msg);
unsigned int  _ecore_dbus_message_read_uint32(Ecore_DBus_Message *msg);
int           _ecore_dbus_alignment_get(Ecore_DBus_Data_Type type);
/* ecore_dbus_marshal.c */
Ecore_DBus_Message_Field_Byte   *_ecore_dbus_message_marshal_byte(Ecore_DBus_Message *msg, unsigned char c);
#if 0
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_boolean(unsigned char **buf, unsigned int *old_length, uint32_t i);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_int16(unsigned char **buf, unsigned int *old_length, int16_t i);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_uint16(unsigned char **buf, unsigned int *old_length, uint16_t i);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_int32(unsigned char **buf, unsigned int *old_length, int32_t i);
#endif
Ecore_DBus_Message_Field_UInt32 *_ecore_dbus_message_marshal_uint32(Ecore_DBus_Message *msg, unsigned int i);
#if 0
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_int64(unsigned char **buf, unsigned int *old_length, int64_t i);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_uint64(unsigned char **buf, unsigned int *old_length, uint64_t i);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_double(unsigned char **buf, unsigned int *old_length, double i);
#endif
Ecore_DBus_Message_Field_String      *_ecore_dbus_message_marshal_string(Ecore_DBus_Message *msg, char *str);
Ecore_DBus_Message_Field_Object_Path *_ecore_dbus_message_marshal_object_path(Ecore_DBus_Message *msg, char *str);
Ecore_DBus_Message_Field_Signature   *_ecore_dbus_message_marshal_signature(Ecore_DBus_Message *msg, char *str);
Ecore_DBus_Message_Field_Array       *_ecore_dbus_message_marshal_array_begin(Ecore_DBus_Message *msg, Ecore_DBus_Data_Type contained_type);
void                                  _ecore_dbus_message_marshal_array_end(Ecore_DBus_Message *msg, Ecore_DBus_Message_Field_Array *arr);
Ecore_DBus_Message_Field_Struct      *_ecore_dbus_message_marshal_struct_begin(Ecore_DBus_Message *msg);
void                                  _ecore_dbus_message_marshal_struct_end(Ecore_DBus_Message *msg, Ecore_DBus_Message_Field_Struct *s);
Ecore_DBus_Message_Field_Variant     *_ecore_dbus_message_marshal_variant(Ecore_DBus_Message *msg, Ecore_DBus_Data_Type type, void *data);
#if 0
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_dict_entry(unsigned char **msg, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_data(unsigned char **buf, unsigned int *old_length, Ecore_DBus_Data_Type type, void *data);
Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_custom_header(unsigned char **buf, unsigned int *old_length, unsigned int code, Ecore_DBus_Data_Type type, void *data);
#endif

/* ecore_dbus_unmarshal.c */
/*
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_byte(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_boolean(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_int16(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_uint16(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_int32(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_uint32(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_int64(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_uint64(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_double(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_string(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_object_path(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_signature(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_array(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_variant(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_struct(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_dict_entry(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_data(unsigned char *buf, unsigned int *old_length);
Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_custom_header(unsigned char *buf, unsigned int *old_length);
*/
Ecore_DBus_Message       *_ecore_dbus_message_unmarshal(Ecore_DBus_Server *svr, unsigned char *message, int size);

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
