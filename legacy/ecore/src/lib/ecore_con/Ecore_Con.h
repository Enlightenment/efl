#ifndef _ECORE_CON_H
#define _ECORE_CON_H

#ifdef __cplusplus
extern "C" {
#endif
   
#ifndef _ECORE_CON_PRIVATE_H
   typedef void Ecore_Con_Server; /**< A connection handle */
   typedef void Ecore_Con_Client; /**< A connection handle */
   
   typedef enum _Ecore_Con_Type
     {
	ECORE_CON_LOCAL_USER,
	  ECORE_CON_LOCAL_SYSTEM,
	  ECORE_CON_REMOTE_SYSTEM,
     } Ecore_Con_Type;
   
#endif
   
   typedef struct _Ecore_Con_Event_Client_Add  Ecore_Con_Event_Client_Add;
   typedef struct _Ecore_Con_Event_Client_Del  Ecore_Con_Event_Client_Del;
   typedef struct _Ecore_Con_Event_Server_Add  Ecore_Con_Event_Server_Add;
   typedef struct _Ecore_Con_Event_Server_Del  Ecore_Con_Event_Server_Del;
   typedef struct _Ecore_Con_Event_Client_Data Ecore_Con_Event_Client_Data;
   typedef struct _Ecore_Con_Event_Server_Data Ecore_Con_Event_Server_Data;

   struct _Ecore_Con_Event_Client_Add
     {
	Ecore_Con_Client *client;
     };

   struct _Ecore_Con_Event_Client_Del
     {
	Ecore_Con_Client *client;
     };

   struct _Ecore_Con_Event_Server_Add
     {
	Ecore_Con_Server *server;
     };

   struct _Ecore_Con_Event_Server_Del
     {
	Ecore_Con_Server *server;
     };
   
   struct _Ecore_Con_Event_Client_Data
     {
	Ecore_Con_Client *client;
	void             *data;
	int               size;
     };
   
   struct _Ecore_Con_Event_Server_Data
     {
	Ecore_Con_Server *server;
	void             *data;
	int               size;
     };
   
   extern int ECORE_CON_EVENT_CLIENT_ADD;
   extern int ECORE_CON_EVENT_CLIENT_DEL;
   extern int ECORE_CON_EVENT_SERVER_ADD;
   extern int ECORE_CON_EVENT_SERVER_DEL;
   extern int ECORE_CON_EVENT_CLIENT_DATA;
   extern int ECORE_CON_EVENT_SERVER_DATA;
   
   int               ecore_con_init(void);
   int               ecore_con_shutdown(void);
   
   Ecore_Con_Server *ecore_con_server_add(Ecore_Con_Type type, char *name, int port, const void *data);
   
   Ecore_Con_Server *ecore_con_server_connect(Ecore_Con_Type type, char *name, int port, const void *data);
   void             *ecore_con_server_del(Ecore_Con_Server *svr);
   void             *ecore_con_server_data_get(Ecore_Con_Server *svr);
   int               ecore_con_server_connected_get(Ecore_Con_Server *svr);
   int               ecore_con_server_send(Ecore_Con_Server *svr, void *data, int size);
   
   int               ecore_con_client_send(Ecore_Con_Client *cl, void *data, int size);
   Ecore_Con_Server *ecore_con_client_server_get(Ecore_Con_Client *cl);
   void             *ecore_con_client_del(Ecore_Con_Client *cl);
   void              ecore_con_client_data_set(Ecore_Con_Client *cl, const void *data);
   void             *ecore_con_client_data_get(Ecore_Con_Client *cl);
   
#ifdef __cplusplus
}
#endif

#endif
