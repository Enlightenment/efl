#ifndef _ECORE_IPC_PRIVATE_H
#define _ECORE_IPC_PRIVATE_H


extern int _ecore_ipc_log_dom;

#ifdef ECORE_IPC_DEFAULT_LOG_COLOR
# undef ECORE_IPC_DEFAULT_LOG_COLOR
#endif
#define ECORE_IPC_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ecore_ipc_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ecore_ipc_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ecore_ipc_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ecore_ipc_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_ipc_log_dom, __VA_ARGS__)

#define ECORE_MAGIC_IPC_SERVER             0x87786556
#define ECORE_MAGIC_IPC_CLIENT             0x78875665

typedef struct _Ecore_Ipc_Msg_Head Ecore_Ipc_Msg_Head;


#if defined (_MSC_VER) || (defined (__SUNPRO_C) && __SUNPRO_C < 0x5100)
# pragma pack(1)
# define ECORE_IPC_STRUCT_PACKED
#elif defined (__GNUC__) || (defined (__SUNPRO_C) && __SUNPRO_C >= 0x5100)
# define ECORE_IPC_STRUCT_PACKED __attribute__((packed))
#else
# define ECORE_IPC_STRUCT_PACKED
#endif

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
} ECORE_IPC_STRUCT_PACKED;
#ifdef __sgi
#pragma pack 0
#endif

struct _Ecore_Ipc_Client
{
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
