#ifndef _ECORE_DBUS_H
#define _ECORE_DBUS_H
#endif

#ifdef EAPI
#undef EAPI
#endif
#ifdef WIN32
# ifdef BUILDING_DLL
#  define EAPI __declspec(dllexport)
# else
#  define EAPI __declspec(dllimport)
# endif
#else
# ifdef GCC_HASCLASSVISIBILITY
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#endif

#include <Ecore_Data.h>

#ifdef __cplusplus
extern "C" {
#endif
   
   typedef struct _Ecore_DBus_Server Ecore_DBus_Server; 
   typedef struct _Ecore_DBus_Client Ecore_DBus_Client; 
   typedef struct _Ecore_DBus_Event_Server_Add Ecore_DBus_Event_Server_Add;
   typedef struct _Ecore_DBus_Event_Server_Del Ecore_DBus_Event_Server_Del;
   typedef struct _Ecore_DBus_Event_Server_Data Ecore_DBus_Event_Server_Data;
   typedef struct _Ecore_DBus_Message Ecore_DBus_Message;
   typedef struct _Ecore_DBus_Message_Field Ecore_DBus_Message_Field;
   typedef struct _Ecore_DBus_Auth Ecore_DBus_Auth;
   typedef unsigned char* (*Ecore_DBus_Auth_Transaction)(void*);
   
   typedef enum _Ecore_DBus_Type
     {
	ECORE_DBUS_BUS_SESSION,    
	  ECORE_DBUS_BUS_SYSTEM,     
	  ECORE_DBUS_BUS_ACTIVATION  
     } Ecore_DBus_Type;
   
   typedef enum _Ecore_DBus_Message_Type
     {
	ECORE_DBUS_MESSAGE_TYPE_INVALID,
	  ECORE_DBUS_MESSAGE_TYPE_METHOD_CALL,
	  ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN,
	  ECORE_DBUS_MESSAGE_TYPE_ERROR,
	  ECORE_DBUS_MESSAGE_TYPE_SIGNAL
     } Ecore_DBus_Message_Type;
   
   /* message data types */
   typedef enum _Ecore_DBus_Data_Type
     {
	ECORE_DBUS_DATA_TYPE_INVALID = ((int) '\0'),
	ECORE_DBUS_DATA_TYPE_NIL = ((int) '\0'),
	ECORE_DBUS_DATA_TYPE_BYTE = ((int) 'y'),
	ECORE_DBUS_DATA_TYPE_BOOLEAN = ((int) 'b'), /* 0,1 */
	ECORE_DBUS_DATA_TYPE_INT32 = ((int) 'i'),
	ECORE_DBUS_DATA_TYPE_UINT32 =((int) 'u'),
	ECORE_DBUS_DATA_TYPE_INT64 = ((int) 'x'),
	ECORE_DBUS_DATA_TYPE_UINT64 = ((int) 't'),
	ECORE_DBUS_DATA_TYPE_DOUBLE = ((int) 'd'),
	ECORE_DBUS_DATA_TYPE_STRING = ((int) 's'),
	ECORE_DBUS_DATA_TYPE_CUSTOM = ((int) 'c'),
	ECORE_DBUS_DATA_TYPE_ARRAY  =((int) 'a'),
	ECORE_DBUS_DATA_TYPE_DICT = ((int) 'm'),
	ECORE_DBUS_DATA_TYPE_OBJECT_PATH  = ((int) 'o')
     } Ecore_DBus_Data_Type;
   
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
   
   typedef enum _Ecore_DBus_Auth_Type
     {
	ECORE_DBUS_AUTH_TYPE_EXTERNAL,
	  ECORE_DBUS_AUTH_MAGIC_COOKIE,
	  ECORE_DBUS_AUTH_TYPE_DBUS_COOKIE_SHA1,
	  ECORE_DBUS_AUTH_TYPE_KERBEROS_V4,
	  ECORE_DBUS_AUTH_TYPE_SKEY
     } Ecore_DBus_Auth_Type;
   
   struct _Ecore_DBus_Auth
     {
	char		*name;
	unsigned int	num_transactions;
	Ecore_DBus_Auth_Transaction transactions[5];
     };
   
   struct _Ecore_DBus_Event_Server_Add
     {
	Ecore_DBus_Server *server;
     };
   
   struct _Ecore_DBus_Event_Server_Del
     {
	Ecore_DBus_Server *server;
     };
   
   struct _Ecore_DBus_Message_Field
     {
	Ecore_List		__list_data;
	Ecore_DBus_Data_Type	type;
	unsigned int		offset;
	unsigned int		count;	/* number of elements, usefull for structs, arrays, dicts */
	unsigned int		hfield;	/* if the field is a header field, we need the type of it */
     };
   
   struct _Ecore_DBus_Message
     {
	/* header fields */
	unsigned char		byte_order;
	unsigned char		type;
	unsigned char		flags;
	unsigned char		protocol;
	unsigned long		hlength; 
	unsigned long		blength;
	unsigned long		serial;
	
	Ecore_DBus_Server	*ref_server;
	
	unsigned char		*header;
	unsigned int 		hpos;
	Ecore_DBus_Message_Field 		*header_fields;
	unsigned char		*body;
	unsigned int		bpos;
	Ecore_DBus_Message_Field 		*body_fields; 
	unsigned char		*signature;
     };
   
   extern int ECORE_DBUS_EVENT_SERVER_ADD;
   extern int ECORE_DBUS_EVENT_SERVER_DATA;
   
   /* Message byte order */
#define DBUS_LITTLE_ENDIAN ('l')  /* LSB first */
#define DBUS_BIG_ENDIAN    ('B')  /* MSB first */
   
   /* Protocol version */
#define DBUS_MAJOR_PROTOCOL_VERSION 0
   
   
   /* Max length in bytes of a service or interface or member name */
#define DBUS_MAXIMUM_NAME_LENGTH 256
   
   /* Max length of a match rule string */
#define DBUS_MAXIMUM_MATCH_RULE_LENGTH 1024
   
   
   /* Header flags */
#define DBUS_HEADER_FLAG_NO_REPLY_EXPECTED 0x1
#define DBUS_HEADER_FLAG_AUTO_ACTIVATION   0x2
   
   
#define DBUS_HEADER_FIELD_LAST DBUS_HEADER_FIELD_SIGNATURE
   
   /* Services */
#define DBUS_SERVICE_ORG_FREEDESKTOP_DBUS      "org.freedesktop.DBus"
   
   /* Paths */
#define DBUS_PATH_ORG_FREEDESKTOP_DBUS  "/org/freedesktop/DBus"
#define DBUS_PATH_ORG_FREEDESKTOP_LOCAL "/org/freedesktop/Local"
   
   /* Interfaces, these #define don't do much other than
    *  * catch typos at compile time
    *   */
#define DBUS_INTERFACE_ORG_FREEDESKTOP_DBUS  "org.freedesktop.DBus"
#define DBUS_INTERFACE_ORG_FREEDESKTOP_INTROSPECTABLE "org.freedesktop.Introspectable"
   
   /* This is a special interface whose methods can only be invoked
    * by the local implementation (messages from remote apps aren't
    * allowed to specify this interface).
    */
#define DBUS_INTERFACE_ORG_FREEDESKTOP_LOCAL "org.freedesktop.Local"
   /* Service owner flags */
#define DBUS_SERVICE_FLAG_PROHIBIT_REPLACEMENT 0x1
#define DBUS_SERVICE_FLAG_REPLACE_EXISTING     0x2
   
   /* Service replies */
#define DBUS_SERVICE_REPLY_PRIMARY_OWNER  0x1
#define DBUS_SERVICE_REPLY_IN_QUEUE       0x2
#define DBUS_SERVICE_REPLY_SERVICE_EXISTS 0x4
#define DBUS_SERVICE_REPLY_ALREADY_OWNER  0x8
   
   /* Activation replies */
#define DBUS_ACTIVATION_REPLY_ACTIVATED      0x0
#define DBUS_ACTIVATION_REPLY_ALREADY_ACTIVE 0x1
   
   /* connection */
   EAPI int ecore_dbus_init(void);
   EAPI Ecore_DBus_Server* ecore_dbus_server_connect(Ecore_DBus_Type compl_type, char *name, int port, const void *data);
   /* message */	
   EAPI unsigned int ecore_dbus_message_new_method_call(Ecore_DBus_Server *svr,char *service,char *path,char *interface,char *method,char *fmt, ...);
   EAPI void ecore_dbus_message_print(Ecore_DBus_Message *msg);
   EAPI void * ecore_dbus_get_header_field(Ecore_DBus_Message *,Ecore_DBus_Message_Field *, Ecore_DBus_Message_Header_Field);
   EAPI void * ecore_dbus_get_body_field(Ecore_DBus_Message *,Ecore_DBus_Message_Field *, unsigned int);

#ifdef __cplusplus
}
#endif
