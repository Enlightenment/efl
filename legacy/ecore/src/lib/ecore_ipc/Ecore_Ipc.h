#ifndef _ECORE_IPC_H
#define _ECORE_IPC_H

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

/**
 * @file Ecore_Ipc.h
 * @brief Ecore inter-process communication functions.
 */

#ifdef __cplusplus
extern "C" {
#endif
   
#ifndef _ECORE_IPC_PRIVATE_H
   typedef void Ecore_Ipc_Server; /**< An IPC connection handle */
   typedef void Ecore_Ipc_Client; /**< An IPC connection handle */
#endif

   typedef enum _Ecore_Ipc_Type
     {
	ECORE_IPC_LOCAL_USER,
	  ECORE_IPC_LOCAL_SYSTEM,
	  ECORE_IPC_REMOTE_SYSTEM,
          ECORE_IPC_USE_SSL = 16
     } Ecore_Ipc_Type;
   
   typedef struct _Ecore_Ipc_Event_Client_Add  Ecore_Ipc_Event_Client_Add;
   typedef struct _Ecore_Ipc_Event_Client_Del  Ecore_Ipc_Event_Client_Del;
   typedef struct _Ecore_Ipc_Event_Server_Add  Ecore_Ipc_Event_Server_Add;
   typedef struct _Ecore_Ipc_Event_Server_Del  Ecore_Ipc_Event_Server_Del;
   typedef struct _Ecore_Ipc_Event_Client_Data Ecore_Ipc_Event_Client_Data;
   typedef struct _Ecore_Ipc_Event_Server_Data Ecore_Ipc_Event_Server_Data;
   
   struct _Ecore_Ipc_Event_Client_Add
     {
	Ecore_Ipc_Client *client;
     };
   
   struct _Ecore_Ipc_Event_Client_Del
     {
	Ecore_Ipc_Client *client;
     };
   
   struct _Ecore_Ipc_Event_Server_Add
     {
	Ecore_Ipc_Server *server;
     };
   
   struct _Ecore_Ipc_Event_Server_Del
     {
	Ecore_Ipc_Server *server;
     };
   
   struct _Ecore_Ipc_Event_Client_Data
     {
	Ecore_Ipc_Client *client;
	/* FIXME: this needs to become an ipc message */
	int               major;
	int               minor;
	int               ref;
	int               ref_to;
	int               response;
	void             *data;
	int               size;
     };
   
   struct _Ecore_Ipc_Event_Server_Data
     {
	Ecore_Ipc_Server *server;
	/* FIXME: this needs to become an ipc message */
	int               major;
	int               minor;
	int               ref;
	int               ref_to;
	int               response;
	void             *data;
	int               size;
     };
   
   extern int ECORE_IPC_EVENT_CLIENT_ADD;
   extern int ECORE_IPC_EVENT_CLIENT_DEL;
   extern int ECORE_IPC_EVENT_SERVER_ADD;
   extern int ECORE_IPC_EVENT_SERVER_DEL;
   extern int ECORE_IPC_EVENT_CLIENT_DATA;
   extern int ECORE_IPC_EVENT_SERVER_DATA;
   
   EAPI int               ecore_ipc_init(void);
   EAPI int               ecore_ipc_shutdown(void);
   
   /* FIXME: need to add protocol type parameter */
   EAPI Ecore_Ipc_Server *ecore_ipc_server_add(Ecore_Ipc_Type type, char *name, int port, const void *data);
   
   /* FIXME: need to add protocol type parameter */
   EAPI Ecore_Ipc_Server *ecore_ipc_server_connect(Ecore_Ipc_Type type, char *name, int port, const void *data);
   EAPI void             *ecore_ipc_server_del(Ecore_Ipc_Server *svr);
   EAPI void             *ecore_ipc_server_data_get(Ecore_Ipc_Server *svr);
   EAPI int               ecore_ipc_server_connected_get(Ecore_Ipc_Server *svr);
   /* FIXME: this needs to become an ipc message */
   EAPI int               ecore_ipc_server_send(Ecore_Ipc_Server *svr, int major, int minor, int ref, int ref_to, int response, void *data, int size);

   /* FIXME: this needs to become an ipc message */
   EAPI int               ecore_ipc_client_send(Ecore_Ipc_Client *cl, int major, int minor, int ref, int ref_to, int response, void *data, int size);
   EAPI Ecore_Ipc_Server *ecore_ipc_client_server_get(Ecore_Ipc_Client *cl);
   EAPI void             *ecore_ipc_client_del(Ecore_Ipc_Client *cl);
   EAPI void              ecore_ipc_client_data_set(Ecore_Ipc_Client *cl, const void *data);
   EAPI void             *ecore_ipc_client_data_get(Ecore_Ipc_Client *cl);
   
   EAPI int               ecore_ipc_ssl_available_get(void);
   /* FIXME: need to add a callback to "ok" large ipc messages greater than */
   /*        a certain size (seurity/DOS attack safety) */
   
#ifdef __cplusplus
}
#endif

#endif
