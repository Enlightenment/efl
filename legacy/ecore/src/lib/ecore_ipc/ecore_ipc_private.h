#ifndef _ECORE_IPC_PRIVATE_H
#define _ECORE_IPC_PRIVATE_H

#define ECORE_MAGIC_IPC_SERVER             0x87786556
#define ECORE_MAGIC_IPC_CLIENT             0x78875665

typedef struct _Ecore_Ipc_Client Ecore_Ipc_Client;
typedef struct _Ecore_Ipc_Server Ecore_Ipc_Server;
   
struct _Ecore_Ipc_Client
{
   Ecore_List        __list_data;
   ECORE_MAGIC;
   Ecore_Con_Client  *client;
   void              *data;
   unsigned char     *buf;
   int                buf_size;
};
   
struct _Ecore_Ipc_Server
{
   Ecore_List        __list_data;
   ECORE_MAGIC;
   Ecore_Con_Server *server;
   Ecore_Ipc_Client *clients;
   void              *data;
   unsigned char     *buf;
   int                buf_size;
};

#endif
