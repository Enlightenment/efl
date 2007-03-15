/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef _ECORE_DBUS_H
#define _ECORE_DBUS_H

#include "Ecore_Data.h"
#include "Ecore_Con.h"

#ifdef EAPI
#undef EAPI
#endif
#ifdef _MSC_VER
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif

#define ECORE_DBUS_MAJOR_PROTOCOL_VERSION 0x1

#ifdef __cplusplus
extern "C" {
#endif

   typedef struct _Ecore_DBus_Server            Ecore_DBus_Server;
   typedef struct _Ecore_DBus_Event_Server_Add  Ecore_DBus_Event_Server_Add;
   typedef struct _Ecore_DBus_Event_Server_Del  Ecore_DBus_Event_Server_Del;
   typedef struct _Ecore_DBus_Event_Server_Data Ecore_DBus_Event_Server_Data;
   typedef struct _Ecore_DBus_Event_Server_Data Ecore_DBus_Event_Signal;
   typedef struct _Ecore_DBus_Event_Server_Data Ecore_DBus_Event_Method_Call;
   typedef struct _Ecore_DBus_Event_Server_Data Ecore_DBus_Method_Return;
   typedef struct _Ecore_DBus_Message           Ecore_DBus_Message;
   typedef struct _Ecore_DBus_Message_Arg       Ecore_DBus_Message_Arg;
   typedef struct _Ecore_DBus_Message_Field     Ecore_DBus_Message_Field;
   typedef struct _Ecore_DBus_Address           Ecore_DBus_Address;

   typedef void (*Ecore_DBus_Method_Call_Cb) (void *data, Ecore_DBus_Event_Method_Call *event);

   typedef struct _Ecore_DBus_Object Ecore_DBus_Object;
   typedef struct _Ecore_DBus_Object_Method Ecore_DBus_Object_Method;
   typedef struct _Ecore_DBus_Object_Registry Ecore_DBus_Object_Registry;

   struct _Ecore_DBus_Object
     {
	Ecore_DBus_Server *server;
	char *path; /* dbus path name */
	Ecore_List *methods;
     };

   struct _Ecore_DBus_Object_Method
     {
	Ecore_DBus_Object *object;
	char *interface;
	char *name; 

	Ecore_DBus_Method_Call_Cb func;
	void *data;
     };
   struct _Ecore_DBus_Object_Registry
     {
	Ecore_Hash *objects;
     };


   typedef enum _Ecore_DBus_Message_Type
     {
	ECORE_DBUS_MESSAGE_TYPE_INVALID,
	ECORE_DBUS_MESSAGE_TYPE_METHOD_CALL,
	ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN,
	ECORE_DBUS_MESSAGE_TYPE_ERROR,
	ECORE_DBUS_MESSAGE_TYPE_SIGNAL
     } Ecore_DBus_Message_Type;

   typedef enum _Ecore_DBus_Message_Flag
     {
	ECORE_DBUS_MESSAGE_FLAG_NO_REPLY_EXPECTED,
	ECORE_DBUS_MESSAGE_FLAG_NO_AUTO_START
     } Ecore_DBus_Message_Flag;

   typedef enum _Ecore_DBus_Data_Type
     {
	ECORE_DBUS_DATA_TYPE_INVALID          = ((int) '\0'),
	ECORE_DBUS_DATA_TYPE_BYTE             = ((int) 'y'),
	ECORE_DBUS_DATA_TYPE_BOOLEAN          = ((int) 'b'), /* 0,1 */
	ECORE_DBUS_DATA_TYPE_INT16            = ((int) 'n'),
	ECORE_DBUS_DATA_TYPE_UINT16           = ((int) 'q'),
	ECORE_DBUS_DATA_TYPE_INT32            = ((int) 'i'),
	ECORE_DBUS_DATA_TYPE_UINT32           = ((int) 'u'),
	ECORE_DBUS_DATA_TYPE_INT64            = ((int) 'x'),
	ECORE_DBUS_DATA_TYPE_UINT64           = ((int) 't'),
	ECORE_DBUS_DATA_TYPE_DOUBLE           = ((int) 'd'),
	ECORE_DBUS_DATA_TYPE_STRING           = ((int) 's'),
	ECORE_DBUS_DATA_TYPE_OBJECT_PATH      = ((int) 'o'),
	ECORE_DBUS_DATA_TYPE_SIGNATURE        = ((int) 'g'),
	ECORE_DBUS_DATA_TYPE_ARRAY            = ((int) 'a'),
	ECORE_DBUS_DATA_TYPE_VARIANT          = ((int) 'v'),
	ECORE_DBUS_DATA_TYPE_STRUCT           = ((int) 'r'),
	ECORE_DBUS_DATA_TYPE_STRUCT_BEGIN     = ((int) '('),
	ECORE_DBUS_DATA_TYPE_STRUCT_END       = ((int) ')'),
	ECORE_DBUS_DATA_TYPE_DICT_ENTRY       = ((int) 'e'),
	ECORE_DBUS_DATA_TYPE_DICT_ENTRY_BEGIN = ((int) '{'),
	ECORE_DBUS_DATA_TYPE_DICT_ENTRY_END   = ((int) '}'),
     } Ecore_DBus_Data_Type;

   struct _Ecore_DBus_Event_Server_Add
     {
	Ecore_DBus_Server *server;
     };

   struct _Ecore_DBus_Event_Server_Del
     {
	Ecore_DBus_Server *server;
     };

   struct _Ecore_DBus_Event_Server_Data
     {
	Ecore_DBus_Server       *server;
	Ecore_DBus_Message_Type  type;
	Ecore_DBus_Message      *message;
	struct {
	     const char   *path;
	     const char   *interface;
	     const char   *member;
	     const char   *error_name;
	     unsigned int  reply_serial;
	     const char   *destination;
	     const char   *sender;
	     const char   *signature;
	} header;
	Ecore_DBus_Message_Arg *args;
     };

   struct _Ecore_DBus_Message_Arg
     {
	Ecore_DBus_Data_Type  type;
	void                 *value;
     };

   typedef enum _Ecore_DBus_Message_Header_Field
     {
	ECORE_DBUS_HEADER_FIELD_INVALID,
	ECORE_DBUS_HEADER_FIELD_PATH,
	ECORE_DBUS_HEADER_FIELD_INTERFACE,
	ECORE_DBUS_HEADER_FIELD_MEMBER,
	ECORE_DBUS_HEADER_FIELD_ERROR_NAME,
	ECORE_DBUS_HEADER_FIELD_REPLY_SERIAL,
	ECORE_DBUS_HEADER_FIELD_DESTINATION,
	ECORE_DBUS_HEADER_FIELD_SENDER,
	ECORE_DBUS_HEADER_FIELD_SIGNATURE
     } Ecore_DBus_Message_Header_Field;


   struct _Ecore_DBus_Address 
     {
	char *transport;
	Ecore_List *keys;
	Ecore_List *vals;
     };

   EAPI extern int ECORE_DBUS_EVENT_SERVER_ADD;
   EAPI extern int ECORE_DBUS_EVENT_SERVER_DEL;
   EAPI extern int ECORE_DBUS_EVENT_METHOD_CALL;
   EAPI extern int ECORE_DBUS_EVENT_SIGNAL;

   /* callback */
   typedef void (*Ecore_DBus_Method_Return_Cb)(void *data, Ecore_DBus_Method_Return *reply);
   typedef void (*Ecore_DBus_Error_Cb)(void *data, const char *error);

   /* init */
   EAPI int ecore_dbus_init(void);
   EAPI int ecore_dbus_shutdown(void);

   /* connection */
   EAPI Ecore_DBus_Server *ecore_dbus_server_system_connect(const void *data);
   EAPI Ecore_DBus_Server *ecore_dbus_server_session_connect(const void *data);
   EAPI Ecore_DBus_Server *ecore_dbus_server_starter_connect(const void *data);

   EAPI Ecore_DBus_Server *ecore_dbus_server_connect(Ecore_Con_Type type, const char *name, int port, const void *data);


   EAPI void               ecore_dbus_server_del(Ecore_DBus_Server *svr);


   /* message */
   EAPI int           ecore_dbus_server_send(Ecore_DBus_Server *svr, const char *command, int length);

   EAPI unsigned int ecore_dbus_message_new_method_call(Ecore_DBus_Server *svr, 
					 const char *path, const char *interface,
					 const char *method, const char *destination, 
					 Ecore_DBus_Method_Return_Cb method_cb,
					 Ecore_DBus_Error_Cb error_cb,
					 const void *data,
					 const char *signature, ...);
   EAPI unsigned int ecore_dbus_message_new_method_return(Ecore_DBus_Server *svr,
					 int reply_serial, const char *destination,
					 const char *signature, ...);
   EAPI unsigned int ecore_dbus_message_new_error(Ecore_DBus_Server *svr, 
	                                 const char *error_name,
					 int reply_serial, const char *destination,
					 const char *error_message);
   EAPI unsigned int ecore_dbus_message_new_signal(Ecore_DBus_Server *svr,
					 const char *path,
					 const char *interface, const char *signal_name,
					 const char *destination, const void *data,
					 const char *signature, ...);

   EAPI void          ecore_dbus_message_del(Ecore_DBus_Message *msg);
   EAPI void          ecore_dbus_message_print(Ecore_DBus_Message *msg);
   EAPI void         *ecore_dbus_message_header_field_get(Ecore_DBus_Message *msg, Ecore_DBus_Message_Header_Field field);
   EAPI void         *ecore_dbus_message_body_field_get(Ecore_DBus_Message *msg, unsigned int pos);
   EAPI int           ecore_dbus_message_serial_get(Ecore_DBus_Message *m);

   /* methods */
   EAPI int ecore_dbus_method_hello(Ecore_DBus_Server *svr, Ecore_DBus_Method_Return_Cb method_cb, Ecore_DBus_Error_Cb, void *data);
   EAPI int ecore_dbus_method_list_names(Ecore_DBus_Server *svr, Ecore_DBus_Method_Return_Cb method_cb, Ecore_DBus_Error_Cb, void *data);
   EAPI int ecore_dbus_method_name_has_owner(Ecore_DBus_Server *svr, char *name, Ecore_DBus_Method_Return_Cb method_cb, Ecore_DBus_Error_Cb, void *data);
   EAPI int ecore_dbus_method_start_service_by_name(Ecore_DBus_Server *svr, char *name, unsigned int flags, Ecore_DBus_Method_Return_Cb method_cb, Ecore_DBus_Error_Cb, void *data);
   EAPI int ecore_dbus_method_get_name_owner(Ecore_DBus_Server *svr, char *name, Ecore_DBus_Method_Return_Cb method_cb, Ecore_DBus_Error_Cb, void *data);
   EAPI int ecore_dbus_method_get_connection_unix_user(Ecore_DBus_Server *svr, char *connection, Ecore_DBus_Method_Return_Cb method_cb, Ecore_DBus_Error_Cb, void *data);
   EAPI int ecore_dbus_method_add_match(Ecore_DBus_Server *svr, char *match, Ecore_DBus_Method_Return_Cb method_cb, Ecore_DBus_Error_Cb, void *data);
   EAPI int ecore_dbus_method_remove_match(Ecore_DBus_Server *svr, char *match, Ecore_DBus_Method_Return_Cb method_cb, Ecore_DBus_Error_Cb, void *data);
   EAPI int ecore_dbus_method_request_name(Ecore_DBus_Server *svr, char *name, int flags, Ecore_DBus_Method_Return_Cb method_cb, Ecore_DBus_Error_Cb, void *data);
   EAPI int ecore_dbus_method_release_name(Ecore_DBus_Server *svr, char *name, Ecore_DBus_Method_Return_Cb method_cb, Ecore_DBus_Error_Cb, void *data);

   /* addresses */
   EAPI Ecore_DBus_Address *ecore_dbus_address_new(void);
   EAPI void                ecore_dbus_address_free(Ecore_DBus_Address *address);

   EAPI Ecore_List         *ecore_dbus_address_parse(const char *address);
   EAPI char               *ecore_dbus_address_string(Ecore_DBus_Address *address);

   EAPI const char         *ecore_dbus_address_value_get(Ecore_DBus_Address *address, const char *key);
   EAPI void                ecore_dbus_print_address_list(Ecore_List *addresses);
   EAPI Ecore_DBus_Server  *ecore_dbus_address_list_connect(Ecore_List *addrs, const void *data);
   EAPI Ecore_DBus_Server  *ecore_dbus_address_connect(Ecore_DBus_Address *addr, const void *data);

   /* object */
   EAPI Ecore_DBus_Object        *ecore_dbus_object_add(Ecore_DBus_Server *svr, const char *path);
   EAPI void                      ecore_dbus_object_free(Ecore_DBus_Object *obj);

   EAPI Ecore_DBus_Object_Method *ecore_dbus_object_method_add(Ecore_DBus_Object *obj, const char *interface, const char *method_name, Ecore_DBus_Method_Call_Cb func, void *data);
   EAPI void                      ecore_dbus_object_method_remove(Ecore_DBus_Object *obj, Ecore_DBus_Object_Method *method);
   EAPI void                      ecore_dbus_object_method_free(Ecore_DBus_Object_Method *method);

#ifdef __cplusplus
}
#endif
#endif
