#ifndef _ECORE_CON_PRIVATE_H
#define _ECORE_CON_PRIVATE_H

#include "Ecore_Data.h"

#define ECORE_MAGIC_CON_SERVER             0x77665544
#define ECORE_MAGIC_CON_CLIENT             0x77556677

#if USE_OPENSSL
#include <openssl/ssl.h>
#endif
#ifdef HAVE_CURL
#include <curl/curl.h>
#endif

#define READBUFSIZ 65536

typedef struct _Ecore_Con_Client Ecore_Con_Client;
typedef struct _Ecore_Con_Server Ecore_Con_Server;
#ifdef HAVE_CURL
typedef struct _Ecore_Con_Url    Ecore_Con_Url;
#else
typedef void Ecore_Con_Url;
#endif

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
   char             *ip;
   int               event_count;
   char              dead : 1;
   char              delete_me : 1;
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
   Ecore_List       *clients;
   int               write_buf_size;
   int               write_buf_offset;
   unsigned char    *write_buf;
   int               event_count;
   int               client_limit;
   pid_t             ppid;
#if USE_OPENSSL
   SSL_CTX          *ssl_ctx;
   SSL              *ssl;
#endif
   char             *ip;
   char              dead : 1;
   char              created : 1;
   char              connecting : 1;
   char              reject_excess_clients : 1;
   char              delete_me : 1;
};

#ifdef HAVE_CURL
struct _Ecore_Con_Url
{
   /* FIXME: ECORE_MAGIC ? */
   CURL             *curl_easy;
   char             *url;
   struct curl_slist *headers;
   Ecore_Fd_Handler *fd_handler;
   char              active : 1;
};
#endif

/* from ecore_con_dns.c */
int ecore_con_dns_init(void);
int ecore_con_dns_shutdown(void);
#endif
