#ifndef _ECORE_IPC_H
#define _ECORE_IPC_H

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_IPC_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif
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

/**
 * @file Ecore_Ipc.h
 * @brief Ecore inter-process communication functions.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Ecore_Ipc_Server Ecore_Ipc_Server; /**< An IPC connection handle */
typedef struct _Ecore_Ipc_Client Ecore_Ipc_Client; /**< An IPC connection handle */

EAPI unsigned short     _ecore_ipc_swap_16(unsigned short v) EINA_DEPRECATED;
EAPI unsigned int       _ecore_ipc_swap_32(unsigned int v) EINA_DEPRECATED;
EAPI unsigned long long _ecore_ipc_swap_64(unsigned long long v) EINA_DEPRECATED;

#ifdef WORDS_BIGENDIAN
#define ECORE_IPC_SWAP2NET64(x) eina_swap64(x)
#define ECORE_IPC_SWAP2CPU64(x) eina_swap64(x)
#define ECORE_IPC_SWAP2NET32(x) eina_swap32(x)
#define ECORE_IPC_SWAP2CPU32(x) eina_swap32(x)
#define ECORE_IPC_SWAP2NET16(x) eina_swap16(x)
#define ECORE_IPC_SWAP2CPU16(x) eina_swap16(x)
#define ECORE_IPC_SWAP2NET8(x) (x)
#define ECORE_IPC_SWAP2CPU8(x) (x)
#else
#define ECORE_IPC_SWAP2NET64(x) (x)
#define ECORE_IPC_SWAP2CPU64(x) (x)
#define ECORE_IPC_SWAP2NET32(x) (x)
#define ECORE_IPC_SWAP2CPU32(x) (x)
#define ECORE_IPC_SWAP2NET16(x) (x)
#define ECORE_IPC_SWAP2CPU16(x) (x)
#define ECORE_IPC_SWAP2NET8(x) (x)
#define ECORE_IPC_SWAP2CPU8(x) (x)
#endif

/* 1, 2, 4 and 8 byte datatypes */
/* unpacking */
#define ECORE_IPC_GET64(v)\
    { \
        p->v = ECORE_IPC_SWAP2CPU64(*(long long *)(ptr)); \
        ptr += 8; \
    }
#define ECORE_IPC_GET32(v)\
    { \
        p->v = ECORE_IPC_SWAP2CPU32(*(int *)(ptr)); \
        ptr += 4; \
    }
#define ECORE_IPC_GET16(v)\
    { \
        p->v = ECORE_IPC_SWAP2CPU16(*(short *)(ptr)); \
        ptr += 2; \
    }
#define ECORE_IPC_GET8(v) \
    { \
        p->v = ECORE_IPC_SWAP2CPU8(*(char *)(ptr)); \
        ptr += 1; \
    }
/* packing */
#define ECORE_IPC_PUT64(v)\
    { \
        *(long long *)(ptr) = ECORE_IPC_SWAP2NET64(p->v); \
        ptr += 8; \
    }
#define ECORE_IPC_PUT32(v)\
    { \
        *(int *)(ptr) = ECORE_IPC_SWAP2NET32(p->v); \
        ptr += 4; \
    }
#define ECORE_IPC_PUT16(v)\
    { \
        *(short *)(ptr) = ECORE_IPC_SWAP2NET16(p->v); \
        ptr += 2; \
    }
#define ECORE_IPC_PUT8(v) \
    { \
        *(char *)(ptr) = ECORE_IPC_SWAP2NET8(p->v); \
        ptr += 1; \
    }
/* padding data */
#define ECORE_IPC_PAD8()   ptr += 1
#define ECORE_IPC_PAD16()  ptr += 2
#define ECORE_IPC_PAD32()  ptr += 4
#define ECORE_IPC_PAD64()  ptr += 8

/* counting data when encoding lists */
#define ECORE_IPC_CNT8()    len += 1
#define ECORE_IPC_CNT16()   len += 2
#define ECORE_IPC_CNT32()   len += 4
#define ECORE_IPC_CNT64()   len += 8

/* strings */
#define ECORE_IPC_CHEKS() if (*((unsigned char *)d + s - 1) != 0) return 0;
#define ECORE_IPC_GETS(v) \
    { \
        if (ptr < ((unsigned char *)d + s)) \
            { \
                p->v = (char *)ptr; \
                ptr += strlen(p->v) + 1; \
            } \
    } 
#define ECORE_IPC_PUTS(v, l)\
    { \
        strcpy((char *)ptr, p->v); \
        ptr += l + 1; \
    }

/* handy to calculate what sized block we need to alloc */
#define ECORE_IPC_SLEN(l, v) ((l = strlen(p->v)) + 1)
#define ECORE_IPC_CNTS(v)   len += strlen(p->v) + 1

/* saves typing function headers */
#define ECORE_IPC_DEC_STRUCT_PROTO(x) static int x(void *d, int s, void *pp)
#define ECORE_IPC_ENC_STRUCT_PROTO(x) static void *x(void *pp, int *s)
#define ECORE_IPC_DEC_EINA_LIST_PROTO(x) static Eina_List *x(void *d, int s)
#define ECORE_IPC_ENC_EINA_LIST_PROTO(x) static void *x(Eina_List *lp, int *s)


/* decoder setup - saves typing. requires data packet of exact size, or fail */
#define ECORE_IPC_DEC_STRUCT_HEAD_EXACT(typ, x) \
    typ *p; \
    unsigned char *ptr; \
    p = (typ *)pp; \
    if (!d) return 0; if (s != (x)) return 0; \
    ptr = d;
/* decoder setup - saves typing. requires data packet of a minimum size */
#define ECORE_IPC_DEC_STRUCT_HEAD_MIN(typ, x) \
    typ *p; \
    unsigned char *ptr; \
    p = (typ *)pp; \
    if (!d) return 0; if (s < (x)) return 0; \
    ptr = d;
/* footer for the hell of it */
#define ECORE_IPC_DEC_STRUCT_FOOT() return 1
/* header for encoder - gives native struct type and size of flattened packet */
#define ECORE_IPC_ENC_STRUCT_HEAD(typ, sz) \
    typ *p; \
    unsigned char *d, *ptr; \
    int len; \
    *s = 0; \
    if(!pp) return NULL; \
    p = (typ *)pp; \
    len = sz; \
    d = malloc(len); \
    if (!d) return NULL; \
    *s = len; \
    ptr = d;
/* footer for the hell of it */
#define ECORE_IPC_ENC_STRUCT_FOOT() return d

#define ECORE_IPC_DEC_EINA_LIST_HEAD(typ) \
    unsigned char *ptr; \
    Eina_List *l; \
    typ *p; \
    l = NULL; \
    ptr = d; \
    while(ptr < (unsigned char *)(d + s)) \
        { \
            p = malloc(sizeof(typ));

#define ECORE_IPC_DEC_EINA_LIST_FOOT() \
            l = eina_list_append(l, p); \
        } \
    return l
#define ECORE_IPC_ENC_EINA_LIST_HEAD_START(typ) \
    Eina_List *l; \
    typ *p; \
    unsigned char *d, *ptr; \
    int len; \
    *s = 0; \
    len = 0; \
    for (l = lp; l; l = l->next) \
      { \
         p = l->data;
#define ECORE_IPC_ENC_EINA_LIST_HEAD_FINISH() \
      } \
    d = malloc(len); \
    if(!d) return NULL; \
    *s = len; \
    ptr = d; \
    for (l = lp; l; l = l->next) \
      { \
         p = l->data;

#define ECORE_IPC_ENC_EINA_LIST_FOOT() \
      } \
   return d

typedef enum _Ecore_Ipc_Type
{
   ECORE_IPC_LOCAL_USER,
   ECORE_IPC_LOCAL_SYSTEM,
   ECORE_IPC_REMOTE_SYSTEM,
   ECORE_IPC_USE_SSL = (1 << 4),
   ECORE_IPC_NO_PROXY = (1 << 5)
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
   
EAPI extern int ECORE_IPC_EVENT_CLIENT_ADD;
EAPI extern int ECORE_IPC_EVENT_CLIENT_DEL;
EAPI extern int ECORE_IPC_EVENT_SERVER_ADD;
EAPI extern int ECORE_IPC_EVENT_SERVER_DEL;
EAPI extern int ECORE_IPC_EVENT_CLIENT_DATA;
EAPI extern int ECORE_IPC_EVENT_SERVER_DATA;

EAPI int               ecore_ipc_init(void);
EAPI int               ecore_ipc_shutdown(void);

/* FIXME: need to add protocol type parameter */
EAPI Ecore_Ipc_Server *ecore_ipc_server_add(Ecore_Ipc_Type type, const char *name, int port, const void *data);

/* FIXME: need to add protocol type parameter */
EAPI Ecore_Ipc_Server *ecore_ipc_server_connect(Ecore_Ipc_Type type, char *name, int port, const void *data);
EAPI void             *ecore_ipc_server_del(Ecore_Ipc_Server *svr);
EAPI void             *ecore_ipc_server_data_get(Ecore_Ipc_Server *svr);
EAPI Eina_Bool         ecore_ipc_server_connected_get(Ecore_Ipc_Server *svr);
EAPI Eina_List        *ecore_ipc_server_clients_get(Ecore_Ipc_Server *svr);
/* FIXME: this needs to become an ipc message */
EAPI int               ecore_ipc_server_send(Ecore_Ipc_Server *svr, int major, int minor, int ref, int ref_to, int response, const void *data, int size);
EAPI void              ecore_ipc_server_client_limit_set(Ecore_Ipc_Server *svr, int client_limit, char reject_excess_clients);
EAPI void              ecore_ipc_server_data_size_max_set(Ecore_Ipc_Server *srv, int size);
EAPI int               ecore_ipc_server_data_size_max_get(Ecore_Ipc_Server *srv);
EAPI const char       *ecore_ipc_server_ip_get(Ecore_Ipc_Server *svr);
EAPI void              ecore_ipc_server_flush(Ecore_Ipc_Server *svr);
    
/* FIXME: this needs to become an ipc message */
EAPI int               ecore_ipc_client_send(Ecore_Ipc_Client *cl, int major, int minor, int ref, int ref_to, int response, const void *data, int size);
EAPI Ecore_Ipc_Server *ecore_ipc_client_server_get(Ecore_Ipc_Client *cl);
EAPI void             *ecore_ipc_client_del(Ecore_Ipc_Client *cl);
EAPI void              ecore_ipc_client_data_set(Ecore_Ipc_Client *cl, const void *data);
EAPI void             *ecore_ipc_client_data_get(Ecore_Ipc_Client *cl);
EAPI void              ecore_ipc_client_data_size_max_set(Ecore_Ipc_Client *cl, int size);
EAPI int               ecore_ipc_client_data_size_max_get(Ecore_Ipc_Client *cl);
EAPI const char       *ecore_ipc_client_ip_get(Ecore_Ipc_Client *cl);
EAPI void              ecore_ipc_client_flush(Ecore_Ipc_Client *cl);

EAPI int               ecore_ipc_ssl_available_get(void);
/* FIXME: need to add a callback to "ok" large ipc messages greater than */
/*        a certain size (security/DOS attack safety) */
   
#ifdef __cplusplus
}
#endif

#endif
