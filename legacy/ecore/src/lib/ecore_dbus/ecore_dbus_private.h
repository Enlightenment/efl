#ifndef _ECORE_DBUS_PRIVATE_H
#define _ECORE_DBUS_PRIVATE_H

struct _Ecore_DBus_Client
{
   Ecore_DBus_Client	*client;
};

struct _Ecore_DBus_Server
{
   Ecore_List        	__list_data;
   Ecore_Con_Server 	*server;
   Ecore_DBus_Client 	*clients;
   /* unsigned char		*buf; */
   /* int			buf_size; */
   unsigned int		cnt_msg;
   Ecore_List		*msg_queue;
   /* void			*data;*/
   unsigned int		authenticated;
   int			auth_type;
   unsigned int		auth_type_transaction;
};

/* Errors */
/* WARNING dont change order or ABI breaks. */
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
