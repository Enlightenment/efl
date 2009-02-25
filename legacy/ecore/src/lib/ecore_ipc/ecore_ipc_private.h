#ifndef _ECORE_IPC_PRIVATE_H
#define _ECORE_IPC_PRIVATE_H

#include "Ecore_Data.h"

#if USE_GNUTLS_OPENSSL
# include <gnutls/openssl.h>
#elif USE_OPENSSL
# include <openssl/ssl.h>
#endif

#define ECORE_MAGIC_IPC_SERVER             0x87786556
#define ECORE_MAGIC_IPC_CLIENT             0x78875665

typedef struct _Ecore_Ipc_Client Ecore_Ipc_Client;
typedef struct _Ecore_Ipc_Server Ecore_Ipc_Server;
typedef struct _Ecore_Ipc_Msg_Head Ecore_Ipc_Msg_Head;


#ifdef __sgi
#pragma pack 4
#endif
struct _Ecore_Ipc_Msg_Head
{
      int major;
      int minor;
      int ref;
      int ref_to;
      int response;
      int size;
} 
#ifdef _GNU_C_
__attribute__ ((packed));
#endif
;
#ifdef __sgi
#pragma pack 0
#endif

struct _Ecore_Ipc_Client
{
   Ecore_List2        __in_list;
   ECORE_MAGIC;
   Ecore_Con_Client  *client;
   void              *data;
   unsigned char     *buf;
   int                buf_size;
   int                max_buf_size;
   
   struct {
      Ecore_Ipc_Msg_Head i, o;
   } prev;
   
   int               event_count;
   char              delete_me : 1;
};
   
struct _Ecore_Ipc_Server
{
   ECORE_MAGIC;
   Ecore_Con_Server *server;
   Eina_List        *clients;
   Eina_List        *client_list;
   void              *data;
   unsigned char     *buf;
   int                buf_size;
   int                max_buf_size;

   struct {
      Ecore_Ipc_Msg_Head i, o;
   } prev;
   
   int               event_count;
   char              delete_me : 1;
};

#endif
