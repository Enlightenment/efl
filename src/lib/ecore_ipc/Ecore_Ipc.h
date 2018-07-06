#ifndef _ECORE_IPC_H
#define _ECORE_IPC_H

#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_BUILD
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
 * @defgroup Ecore_IPC_Group Ecore_IPC - Ecore inter-process communication functions.
 * @ingroup Ecore
 *
 * Functions that set up and shut down the Ecore IPC Library.
 *
 * @{
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

/**
 * @typedef Ecore_Ipc_Type
 *
 * Enum containing IPC types.
 */
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

/**
 * @struct _Ecore_Ipc_Event_Client_Add
 *
 * An IPC structure for client_add event.
 */
struct _Ecore_Ipc_Event_Client_Add
{
   Ecore_Ipc_Client *client; /**< An IPC connection handle */
};

/**
 * @struct _Ecore_Ipc_Event_Client_Del
 *
 * An IPC structure for client_del event.
 */
struct _Ecore_Ipc_Event_Client_Del
{
   Ecore_Ipc_Client *client; /**< An IPC connection handle */
};

/**
 * @struct _Ecore_Ipc_Event_Server_Add
 *
 * An IPC structure for server_add event.
 */
struct _Ecore_Ipc_Event_Server_Add
{
   Ecore_Ipc_Server *server; /**< An IPC connection handle */
};

/**
 * @struct _Ecore_Ipc_Event_Server_Del
 *
 * An IPC structure for server_del event.
 */
struct _Ecore_Ipc_Event_Server_Del
{
   Ecore_Ipc_Server *server; /**< An IPC connection handle */

};

/**
 * @struct _Ecore_Ipc_Event_Client_Data
 *
 * An IPC structure for client_data event.
 */
struct _Ecore_Ipc_Event_Client_Data
{
   Ecore_Ipc_Client *client; /**< An IPC connection handle */
   /* FIXME: this needs to become an ipc message */
   int               major; /**< The message major opcode number */
   int               minor; /**< The message minor opcode number */
   int               ref; /**< The message reference number */
   int               ref_to; /**< Reference number of the message it refers to */
   int               response; /**< Requires response */
   void             *data; /**< The message data */
   int               size; /**< The data length (in bytes) */
};

/**
 * @struct _Ecore_Ipc_Event_Server_Data
 *
 * An IPC structure for server_data event.
 */
struct _Ecore_Ipc_Event_Server_Data
{
   Ecore_Ipc_Server *server; /**< An IPC connection handle */
   /* FIXME: this needs to become an ipc message */
   int               major; /**< The message major opcode number */
   int               minor; /**< The message minor opcode number */
   int               ref; /**< The message reference number */
   int               ref_to; /**< Reference number of the message it refers to */
   int               response; /**< Requires response */
   void             *data; /**< The message data */
   int               size; /**< The data length (in bytes) */
};
   
EAPI extern int ECORE_IPC_EVENT_CLIENT_ADD;
EAPI extern int ECORE_IPC_EVENT_CLIENT_DEL;
EAPI extern int ECORE_IPC_EVENT_SERVER_ADD;
EAPI extern int ECORE_IPC_EVENT_SERVER_DEL;
EAPI extern int ECORE_IPC_EVENT_CLIENT_DATA;
EAPI extern int ECORE_IPC_EVENT_SERVER_DATA;

/**
 * @ingroup Ecore_IPC_Group
 * @brief Initializes the Ecore IPC library.
 * @return  Number of times the library has been initialised without
 *          being shut down.
 */
EAPI int               ecore_ipc_init(void);

/**
 * @ingroup Ecore_IPC_Group
 * @brief Shuts down the Ecore IPC library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 */
EAPI int               ecore_ipc_shutdown(void);

/**
 * @defgroup Ecore_IPC_Server_Group IPC Server Functions
 * @ingroup Ecore_IPC_Group
 *
 * Functions the deal with IPC server objects.
 */

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Creates an IPC server that listens for connections.
 *
 * For more details about the @p type, @p name and @p port
 * parameters, see the @ref ecore_con_server_add documentation.
 *
 * @param   type      The connection type.
 * @param   name       Name to associate with the socket used for connection.
 * @param   port       Number to identify with socket used for connection.
 * @param   data       Data to associate with the IPC server.
 * @return  New IPC server.  If there is an error, @c NULL is returned.
 * @todo    Need to add protocol type parameter to this function.
 */
EAPI Ecore_Ipc_Server *ecore_ipc_server_add(Ecore_Ipc_Type type, const char *name, int port, const void *data);

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Creates an IPC server object to represent the IPC server listening
 * on the given port.
 *
 * For more details about the @p type, @p name and @p port
 * parameters, see the @ref ecore_con_server_connect documentation.
 *
 * @param   type       The IPC connection type.
 * @param   name       Name used to determine which socket to use for the
 *                     IPC connection.
 * @param   port       Number used to identify the socket to use for the
 *                     IPC connection.
 * @param   data       Data to associate with the server.
 * @return  A new IPC server.  @c NULL is returned on error.
 * @todo    Need to add protocol type parameter.
 */
EAPI Ecore_Ipc_Server *ecore_ipc_server_connect(Ecore_Ipc_Type type, char *name, int port, const void *data);

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Closes the connection and frees the given IPC server.
 * @param   svr The given IPC server.
 * @return  The data associated with the server when it was created.
 */
EAPI void             *ecore_ipc_server_del(Ecore_Ipc_Server *svr);

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Retrieves the data associated with the given IPC server.
 * @param   svr The given IPC server.
 * @return  The associated data.
 */
EAPI void             *ecore_ipc_server_data_get(Ecore_Ipc_Server *svr);

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Retrieves whether the given IPC server is currently connected.
 * @param   svr The given IPC server.
 * @return @c EINA_TRUE if the server is connected, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool         ecore_ipc_server_connected_get(Ecore_Ipc_Server *svr);

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Retrieves the list of clients for this server.
 * @param   svr The given IPC server.
 * @return  An Eina_List with the clients.
 */
EAPI Eina_List        *ecore_ipc_server_clients_get(Ecore_Ipc_Server *svr);

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Sends a message to the given IPC server.
 *
 * The content of the parameters, excluding the @p svr parameter, is up to
 * the client.
 *
 * @param   svr      The given IPC server.
 * @param   major    Major opcode of the message.
 * @param   minor    Minor opcode of the message.
 * @param   ref      Message reference number.
 * @param   ref_to   Reference number of the message this message refers to.
 * @param   response Requires response.
 * @param   data     The data to send as part of the message.
 * @param   size     Length of the data, in bytes, to send.
 * @return  Number of bytes sent.  @c 0 is returned if there is an error.
 * @todo    This function needs to become an IPC message.
 * @todo Fix up the documentation: Make sure what ref_to and response are.
 */
EAPI int               ecore_ipc_server_send(Ecore_Ipc_Server *svr, int major, int minor, int ref, int ref_to, int response, const void *data, int size);

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Sets a limit on the number of clients that can be handled concurrently
 * by the given server, and a policy on what to do if excess clients try to
 * connect.
 * Beware that if you set this once ecore is already running, you may
 * already have pending CLIENT_ADD events in your event queue.  Those
 * clients have already connected and will not be affected by this call.
 * Only clients subsequently trying to connect will be affected.
 * @param   svr           The given server.
 * @param   client_limit  The maximum number of clients to handle
 *                        concurrently.  -1 means unlimited (default).  0
 *                        effectively disables the server.
 * @param   reject_excess_clients  Set to 1 to automatically disconnect
 *                        excess clients as soon as they connect if you are
 *                        already handling client_limit clients.  Set to 0
 *                        (default) to just hold off on the "accept()"
 *                        system call until the number of active clients
 *                        drops. This causes the kernel to queue up to 4096
 *                        connections (or your kernel's limit, whichever is
 *                        lower).
 */
EAPI void              ecore_ipc_server_client_limit_set(Ecore_Ipc_Server *svr, int client_limit, char reject_excess_clients);

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Sets the max data payload size for an Ipc message in bytes
 *
 * @param   svr           The given server.
 * @param   size          The maximum data payload size in bytes.
 */
EAPI void              ecore_ipc_server_data_size_max_set(Ecore_Ipc_Server *svr, int size);

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Gets the max data payload size for an Ipc message in bytes
 *
 * @param   svr           The given server.
 * @return The maximum data payload in bytes.
 */
EAPI int               ecore_ipc_server_data_size_max_get(Ecore_Ipc_Server *svr);

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Gets the IP address of a server that has been connected to.
 *
 * @param   svr           The given server.
 * @return  A pointer to an internal string that contains the IP address of
 *          the connected server in the form "XXX.YYY.ZZZ.AAA" IP notation.
 *          This string should not be modified or trusted to stay valid after
 *          deletion for the @p svr object. If no IP is known NULL is returned.
 */
EAPI const char       *ecore_ipc_server_ip_get(Ecore_Ipc_Server *svr);

/**
 * @ingroup Ecore_IPC_Server_Group
 * @brief Flushes all pending data to the given server. Will return when done.
 *
 * @param   svr           The given server.
 */
EAPI void              ecore_ipc_server_flush(Ecore_Ipc_Server *svr);

/**
 * @defgroup Ecore_IPC_Client_Group IPC Client Functions
 * @ingroup Ecore_IPC_Group
 *
 * Functions that deal with IPC client objects.
 */

/**
 * @ingroup Ecore_IPC_Client_Group
 * @brief Sends a message to the given IPC client.
 *
 * @param   cl       The given IPC client.
 * @param   major    Major opcode of the message.
 * @param   minor    Minor opcode of the message.
 * @param   ref      Reference number of the message.
 * @param   ref_to   Reference number of the message this message refers to.
 * @param   response Requires response.
 * @param   data     The data to send as part of the message.
 * @param   size     Length of the data, in bytes, to send.
 * @return  The number of bytes sent.  @c 0 will be returned if there is
 *          an error.
 * @todo    This function needs to become an IPC message.
 * @todo    Make sure ref_to and response parameters are described correctly.
 */
EAPI int               ecore_ipc_client_send(Ecore_Ipc_Client *cl, int major, int minor, int ref, int ref_to, int response, const void *data, int size);

/**
 * @ingroup Ecore_IPC_Client_Group
 * @brief Retrieves the IPC server that the given IPC client is connected to.
 *
 * @param   cl The given IPC client.
 * @return  The IPC server the IPC client is connected to.
 */
EAPI Ecore_Ipc_Server *ecore_ipc_client_server_get(Ecore_Ipc_Client *cl);

/**
 * @ingroup Ecore_IPC_Client_Group
 * @brief Closes the connection and frees memory allocated to the given IPC
 * client.
 *
 * @param   cl The given client.
 * @return  Data associated with the client.
 */
EAPI void             *ecore_ipc_client_del(Ecore_Ipc_Client *cl);

/**
 * @ingroup Ecore_IPC_Client_Group
 * @brief Sets the IPC data associated with the given IPC client to @p data.
 *
 * @param   cl   The given IPC client.
 * @param   data The data to associate with the IPC client.
 */
EAPI void              ecore_ipc_client_data_set(Ecore_Ipc_Client *cl, const void *data);

/**
 * @ingroup Ecore_IPC_Client_Group
 * @brief Retrieves the data that has been associated with the given IPC client.
 *
 * @param   cl The given client.
 * @return  The data associated with the IPC client.
 */
EAPI void             *ecore_ipc_client_data_get(Ecore_Ipc_Client *cl);

/**
 * @ingroup Ecore_IPC_Client_Group
 * @brief Sets the max data payload size for an Ipc message in bytes
 *
 * @param   cl        The given client.
 * @param   size          The maximum data payload size in bytes.
 */
EAPI void              ecore_ipc_client_data_size_max_set(Ecore_Ipc_Client *cl, int size);

/**
 * @ingroup Ecore_IPC_Client_Group
 * @brief Gets the max data payload size for an Ipc message in bytes
 *
 * @param   cl            The given client.
 * @return The maximum data payload size in bytes on success, @c -1 on failure.
 */
EAPI int               ecore_ipc_client_data_size_max_get(Ecore_Ipc_Client *cl);

/**
 * @ingroup Ecore_IPC_Client_Group
 * @brief Gets the IP address of a client that has been connected to.
 *
 * @param   cl            The given client.
 * @return  A pointer to an internal string that contains the IP address of
 *          the connected server in the form "XXX.YYY.ZZZ.AAA" IP notation.
 *          This string should not be modified or trusted to stay valid after
 *          deletion for the @p cl object. If no IP is known @c NULL is
 *          returned.
 */
EAPI const char       *ecore_ipc_client_ip_get(Ecore_Ipc_Client *cl);

/**
 * @ingroup Ecore_IPC_Client_Group
 * @brief Flushes all pending data to the given client. Will return when done.
 *
 * @param   cl            The given client.
 */
EAPI void              ecore_ipc_client_flush(Ecore_Ipc_Client *cl);

/**
 * @ingroup Ecore_Con_Client_Group
 * @brief Returns if SSL support is available
 *
 * @return  1 if SSL is available, 0 if it is not.
 */
EAPI int               ecore_ipc_ssl_available_get(void);
/* FIXME: need to add a callback to "ok" large ipc messages greater than */
/*        a certain size (security/DOS attack safety) */

#ifdef __cplusplus
}
#endif

#undef EAPI
#define EAPI

/**
 * @}
 */
#endif
