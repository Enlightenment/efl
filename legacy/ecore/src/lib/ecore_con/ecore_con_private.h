#ifndef _ECORE_CON_PRIVATE_H
#define _ECORE_CON_PRIVATE_H

#include "Ecore_Con.h"
#include "Ecore_Data.h"

#define ECORE_MAGIC_CON_SERVER             0x77665544
#define ECORE_MAGIC_CON_CLIENT             0x77556677
#define ECORE_MAGIC_CON_URL                0x77074255

#define ECORE_CON_TYPE 0x0f
#define ECORE_CON_SSL  0xf0

#if USE_OPENSSL
#include <openssl/ssl.h>
#endif
#ifdef HAVE_CURL
#include <curl/curl.h>
#endif

#define READBUFSIZ 65536

struct _Ecore_Con_Client
{
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
   ECORE_MAGIC;
   CURL              *curl_easy;
   struct curl_slist *headers;
   char              *url;

   Ecore_Con_Url_Time condition;
   time_t             time;
   void              *data;

   Ecore_Fd_Handler  *fd_handler;
   int		      fd;
   int		      flags;

   int		      received;
   int		      write_fd;

   unsigned char      active : 1;
};
#endif

/* from ecore_con_dns.c */
int ecore_con_dns_init(void);
int ecore_con_dns_shutdown(void);
#endif
