#ifndef _ECORE_CON_PRIVATE_H
#define _ECORE_CON_PRIVATE_H

#define ECORE_MAGIC_CON_SERVER             0x77665544
#define ECORE_MAGIC_CON_CLIENT             0x77556677

#if USE_OPENSSL
#include <openssl/ssl.h>
#endif

#define READBUFSIZ 65536

typedef struct _Ecore_Con_Client Ecore_Con_Client;
typedef struct _Ecore_Con_Server Ecore_Con_Server;

typedef enum _Ecore_Con_Type
{
   ECORE_CON_LOCAL_USER,
   ECORE_CON_LOCAL_SYSTEM,
   ECORE_CON_REMOTE_SYSTEM
#if USE_OPENSSL
  ,ECORE_CON_USE_SSL = 16
#endif
} Ecore_Con_Type;

struct _Ecore_Con_Client
{
   Ecore_List        __list_data;
   ECORE_MAGIC;
   int               fd;
   Ecore_Con_Server *server;
   void             *data;
   Ecore_Fd_Handler *fd_handler;
   int               buf_size;
   int               buf_offset;
   unsigned char    *buf;
   char              dead : 1;
};

struct _Ecore_Con_Server
{
   Ecore_List        __list_data;
   ECORE_MAGIC;
   int               fd;
   Ecore_Con_Type    type;
   char             *name;
   int               port;
   char             *path;
   void             *data;
   Ecore_Fd_Handler *fd_handler;
   Ecore_Con_Client *clients;
   int               write_buf_size;
   int               write_buf_offset;
   unsigned char    *write_buf;
   char              dead : 1;
   char              created : 1;
   char              connecting : 1;

   /* put the read buffer here to play nice with OpenSSL */
   unsigned char     read_buf[READBUFSIZ];

#if USE_OPENSSL
   SSL_CTX          *ssl_ctx;
   SSL              *ssl;
#endif
};

#endif
