#ifndef _ECORE_CON_H
#define _ECORE_CON_H

#include <time.h>
#include <libgen.h>
#ifdef _WIN32
# include <ws2tcpip.h>
#else
# include <netdb.h>
#endif
#include <Eina.h>

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_ECORE_CON_BUILD
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
 * @defgroup Ecore_Con_Group Ecore_Con - Connection functions
 * @ingroup Ecore
 *
 * The Ecore Connection Library ( @c Ecore_Con ) provides simple mechanisms
 * for communications between programs using reliable sockets.  It saves
 * the programmer from having to worry about file descriptors and waiting
 * for incoming connections.
 *
 * There are two main objects in the @c Ecore_Con library: the @c
 * Ecore_Con_Server and the @c Ecore_Con_Client.
 *
 * The @c Ecore_Con_Server represents a server that can be connected to.
 * It is used regardless of whether the program is acting as a server or
 * client itself.
 *
 * To create a listening server call @c ecore_con_server_add(), optionally using
 * an ECORE_CON_USE_* encryption type OR'ed with the type for encryption.
 *
 * To connect to a server, call @c ecore_con_server_connect().  Data can
 * then be sent to the server using the @c ecore_con_server_send().
 *
 * Functions are described in the following groupings:
 * @li @ref Ecore_Con_Lib_Group
 * @li @ref Ecore_Con_Server_Group
 * @li @ref Ecore_Con_Client_Group
 * @li @ref Ecore_Con_Url_Group
 *
 * Events are described in @ref Ecore_Con_Events_Group.
 */


/**
 * @defgroup Ecore_Con_Events_Group Ecore Connection Events Functions
 * @ingroup Ecore_Con_Group
 *
 * @li ECORE_CON_CLIENT_ADD: Whenever a client connection is made to an
 * @c Ecore_Con_Server, an event of this type is emitted, allowing the
 * retrieval of the client's ip with @ref ecore_con_client_ip_get and
 * associating data with the client using ecore_con_client_data_set.
 * @li ECORE_CON_EVENT_CLIENT_DEL: Whenever a client connection to an
 * @c Ecore_Con_Server, an event of this type is emitted.  The contents of
 * the data with this event are variable, but if the client object in the data
 * is non-null, it must be freed with @ref ecore_con_client_del.
 * @li ECORE_CON_EVENT_SERVER_ADD: Whenever a server object is created
 * with @ref ecore_con_server_connect, an event of this type is emitted,
 * allowing for data to be serialized and sent to the server using
 * @ref ecore_con_server_send. At this point, the http handshake has
 * occurred.
 * @li ECORE_CON_EVENT_SERVER_DEL: Whenever a server object is destroyed,
 * usually by the server connection being refused or dropped, an event of this
 * type is emitted.  The contents of the data with this event are variable,
 * but if the server object in the data is non-null, it must be freed
 * with @ref ecore_con_server_del.
 * @li ECORE_CON_EVENT_CLIENT_DATA: Whenever a client connects to your server
 * object and sends data, an event of this type is emitted.  The data will contain both
 * the size and contents of the message sent by the client.  It should be noted that
 * data within this object is transient, so it must be duplicated in order to be
 * retained.  This event will continue to occur until the client has stopped sending its
 * message, so a good option for storing this data is an Eina_Strbuf.  Once the message has
 * been received in full, the client object must be freed with ecore_con_client_free.
 * @li ECORE_CON_EVENT_SERVER_DATA: Whenever your server object connects to its destination
 * and receives data, an event of this type is emitted.  The data will contain both
 * the size and contents of the message sent by the server.  It should be noted that
 * data within this object is transient, so it must be duplicated in order to be
 * retained.  This event will continue to occur until the server has stopped sending its
 * message, so a good option for storing this data is an Eina_Strbuf.  Once the message has
 * been received in full, the server object must be freed with ecore_con_server_free.
 *
 */

/**
 * @defgroup Ecore_Con_Buffer Ecore Connection Buffering
 * @ingroup Ecore_Con_Group
 * 
 * As Ecore_Con works on an event driven design, as data arrives, events will
 * be produced containing the data that arrived. It is up to the user of
 * Ecore_Con to either parse as they go, append to a file to later parse the
 * whole file in one go, or append to memory to parse or handle later.
 * 
 * To help with this Eina has some handy API's. The Eina_Binbuf and 
 * Eina_Strbuf APIs, abstract dynamic buffer management and make it trivial 
 * to handle buffers at runtime, without having to manage them. Eina_Binbuf 
 * makes it possible to create, expand, reset and slice a blob of memory - 
 * all via API. No system calls, no pointer manipulations and no size 
 * calculation.
 * 
 * Additional functions include adding content at specified byte positions in 
 * the buffer, escaping the inputs, find and replace strings. This provides 
 * extreme flexibility to play around, with a dynamic blob of memory.
 * 
 * It is good to free it (using eina_binbuf_free()) after using it.
 * 
 * Eina_Binbuf compliments Ecore_Con use cases, where dynamic sizes of data
 * arrive from the network (think http download in chunks). Using
 * Eina_Binbuf provides enough flexibility to handle data as it arrives and
 * to defer its processing until desired, without having to think about
 * where to store the temporary data and how to manage its size.
 * 
 * An example of how to use these with Ecore_Con follows.
 * 
 * @code
 * #include <Eina.h>
 * #include <Ecore.h>
 * #include <Ecore_Con.h>
 * 
 * static Eina_Bool
 * data_callback(void *data, int type, void *event)
 * {
 *    Ecore_Con_Event_Url_Data *url_data = event;
 *    if ( url_data->size > 0)
 *      {
 *         // append data as it arrives - don't worry where or how it gets stored.
 *         // Also don't worry about size, expanding, reallocing etc.
 *         // just keep appending - size is automatically handled.
 * 
 *         eina_binbuf_append_length(data, url_data->data, url_data->size);
 * 
 *         fprintf(stderr, "Appended %d \n", url_data->size);
 *      }
 *    return EINA_TRUE;
 * }
 * 
 * 
 * 
 * static Eina_Bool
 * completion_callback(void *data, int type, void *event)
 * {
 *    Ecore_Con_Event_Url_Complete *url_complete = event;
 *    printf("download completed with status code: %d\n", url_complete->status);
 * 
 *    // get the data back from Eina_Binbuf
 *    char *ptr = eina_binbuf_string_get(data);
 *    size_t size = eina_binbuf_length_get(data);
 * 
 *    // process data as required (write to file)
 *    fprintf(stderr, "Size of data = %d bytes\n", size);
 *    int fd = open("./elm.png", O_CREAT);
 *    write(fd, ptr, size);
 *    close(fd);
 *   
 *    // free it when done.
 *    eina_binbuf_free(data);
 * 
 *    ecore_main_loop_quit();
 * 
 *    return EINA_TRUE;
 * }
 * 
 * 
 * int
 * main(int argc, char **argv)
 * {
 * 
 *    const char *url = "http://www.enlightenment.org/p/index/d/logo.png";
 * 
 *    ecore_init();
 *    ecore_con_init();
 *    ecore_con_url_init();
 *   
 * 
 *    // This is single additional line to manage dynamic network data.
 *    Eina_Binbuf *data = eina_binbuf_new();
 *    Ecore_Con_Url *url_con = ecore_con_url_new(url);
 * 
 *    ecore_event_handler_add(ECORE_CON_EVENT_URL_COMPLETE,
 *                                                       completion_callback,
 *                                                       data);
 *    ecore_event_handler_add(ECORE_CON_EVENT_URL_DATA,
 *                                                       data_callback,
 *                                                       data);
 *    ecore_con_url_get(url_con);
 * 
 *    ecore_main_loop_begin();
 *    return 0;
 * }
 * @endcode
 */

#ifdef __cplusplus
extern "C" {
#endif
#define ECORE_CON_USE_SSL ECORE_CON_USE_SSL2
#define ECORE_CON_REMOTE_SYSTEM ECORE_CON_REMOTE_TCP


/**
 * @typedef Ecore_Con_Server
 * A connection handle to a server
 * @ingroup Ecore_Con_Server_Group
 */
typedef struct _Ecore_Con_Server Ecore_Con_Server;

/**
 * @typedef Ecore_Con_Client
 * A connection handle to a client
 * @ingroup Ecore_Con_Client_Group
 */
typedef struct _Ecore_Con_Client Ecore_Con_Client;

/**
 * @typedef Ecore_Con_Socks
 * An object representing a SOCKS proxy
 * @ingroup Ecore_Con_Socks_Group
 * @since 1.2
 */
typedef struct Ecore_Con_Socks Ecore_Con_Socks;

/**
 * @typedef Ecore_Con_Url
 * A handle to an http upload/download object
 * @ingroup Ecore_Con_Url_Group
 */
typedef struct _Ecore_Con_Url Ecore_Con_Url;


/**
 * @addtogroup Ecore_Con_Events_Group
 * @{
 */

/**
 * @typedef Ecore_Con_Event_Client_Add
 * Used as the @p data param for the corresponding event
 */
typedef struct _Ecore_Con_Event_Client_Add Ecore_Con_Event_Client_Add;

/**
 * @typedef Ecore_Con_Event_Client_Upgrade
 * Used as the @p data param for the corresponding event
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Client_Upgrade Ecore_Con_Event_Client_Upgrade;

/**
 * @typedef Ecore_Con_Event_Client_Del
 * Used as the @p data param for the corresponding event
 */
typedef struct _Ecore_Con_Event_Client_Del Ecore_Con_Event_Client_Del;

/**
 * @typedef Ecore_Con_Event_Client_Error
 * Used as the @p data param for the corresponding event
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Client_Error Ecore_Con_Event_Client_Error;

/**
 * @typedef Ecore_Con_Event_Server_Add
 * Used as the @p data param for the corresponding event
 */
typedef struct _Ecore_Con_Event_Server_Add Ecore_Con_Event_Server_Add;

/**
 * @typedef Ecore_Con_Event_Server_Upgrade
 * Used as the @p data param for the corresponding event
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Server_Upgrade Ecore_Con_Event_Server_Upgrade;

/**
 * @typedef Ecore_Con_Event_Server_Del
 * Used as the @p data param for the corresponding event
 */
typedef struct _Ecore_Con_Event_Server_Del Ecore_Con_Event_Server_Del;

/**
 * @typedef Ecore_Con_Event_Server_Error
 * Used as the @p data param for the corresponding event
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Server_Error Ecore_Con_Event_Server_Error;

/**
 * @typedef Ecore_Con_Event_Client_Data
 * Used as the @p data param for the corresponding event
 */
typedef struct _Ecore_Con_Event_Client_Data Ecore_Con_Event_Client_Data;

/**
 * @typedef Ecore_Con_Event_Server_Data
 * Used as the @p data param for the corresponding event
 */
typedef struct _Ecore_Con_Event_Server_Data Ecore_Con_Event_Server_Data;

/**
 * @typedef Ecore_Con_Event_Client_Write
 * Used as the @p data param for the corresponding event
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Client_Write Ecore_Con_Event_Client_Write;

/**
 * @typedef Ecore_Con_Event_Server_Write
 * Used as the @p data param for the corresponding event
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Server_Write Ecore_Con_Event_Server_Write;

/**
 * @typedef Ecore_Con_Event_Proxy_Bind
 * Used as the @p data param for the corresponding event
 * @since 1.2
 */
typedef struct _Ecore_Con_Event_Proxy_Bind Ecore_Con_Event_Proxy_Bind;

/**
 * @typedef Ecore_Con_Event_Url_Data
 * Used as the @p data param for the corresponding event
 * @ingroup Ecore_Con_Url_Group
 */
typedef struct _Ecore_Con_Event_Url_Data Ecore_Con_Event_Url_Data;

/**
 * @typedef Ecore_Con_Event_Url_Complete
 * Used as the @p data param for the corresponding event
 * @ingroup Ecore_Con_Url_Group
 */
typedef struct _Ecore_Con_Event_Url_Complete Ecore_Con_Event_Url_Complete;

/**
 * @typedef Ecore_Con_Event_Url_Progress
 * Used as the @p data param for the corresponding event
 * @ingroup Ecore_Con_Url_Group
 */
typedef struct _Ecore_Con_Event_Url_Progress Ecore_Con_Event_Url_Progress;

/**
 * @struct _Ecore_Con_Event_Client_Add
 * Used as the @p data param for the @ref ECORE_CON_EVENT_CLIENT_ADD event
 */
struct _Ecore_Con_Event_Client_Add
{
   Ecore_Con_Client *client; /** the client that connected */
};

/**
 * @struct _Ecore_Con_Event_Client_Upgrade
 * Used as the @p data param for the @ref ECORE_CON_EVENT_CLIENT_UPGRADE event
 * @since 1.1
 */
struct _Ecore_Con_Event_Client_Upgrade
{
   Ecore_Con_Client *client; /** the client that completed handshake */
};

/**
 * @struct _Ecore_Con_Event_Client_Del
 * Used as the @p data param for the @ref ECORE_CON_EVENT_CLIENT_DEL event
 */
struct _Ecore_Con_Event_Client_Del
{
   Ecore_Con_Client *client; /** the client that was lost */
};

/**
 * @struct _Ecore_Con_Event_Client_Error
 * Used as the @p data param for the @ref ECORE_CON_EVENT_CLIENT_ERROR event
 */
struct _Ecore_Con_Event_Client_Error
{
   Ecore_Con_Client *client; /** the client for which an error occurred */
   char *error; /**< the error string describing what happened */
};

/**
 * @struct _Ecore_Con_Event_Server_Add
 * Used as the @p data param for the @ref ECORE_CON_EVENT_SERVER_ADD event
 */
struct _Ecore_Con_Event_Server_Add
{
   Ecore_Con_Server *server; /** the server that was connected to */
};

/**
 * @struct _Ecore_Con_Event_Server_Upgrade
 * Used as the @p data param for the @ref ECORE_CON_EVENT_SERVER_UPGRADE event
 * @since 1.1
 */
struct _Ecore_Con_Event_Server_Upgrade
{
   Ecore_Con_Server *server; /** the server that was connected to */
};

/**
 * @struct _Ecore_Con_Event_Server_Del
 * Used as the @p data param for the @ref ECORE_CON_EVENT_SERVER_DEL event
 */
struct _Ecore_Con_Event_Server_Del
{
   Ecore_Con_Server *server; /** the client that was lost */
};

/**
 * @struct _Ecore_Con_Event_Server_Error
 * Used as the @p data param for the @ref ECORE_CON_EVENT_SERVER_ERROR event
 */
struct _Ecore_Con_Event_Server_Error
{
   Ecore_Con_Server *server; /** the server for which an error occurred */
   char *error; /**< the error string describing what happened */
};

/**
 * @struct _Ecore_Con_Event_Client_Data
 * Used as the @p data param for the @ref ECORE_CON_EVENT_CLIENT_DATA event
 */
struct _Ecore_Con_Event_Client_Data
{
   Ecore_Con_Client *client; /**< the client that connected */
   void *data;               /**< the data that the client sent */
   int size;                 /**< the length of the data sent */
};

/**
 * @struct _Ecore_Con_Event_Server_Data
 * Used as the @p data param for the @ref ECORE_CON_EVENT_SERVER_DATA event
 */
struct _Ecore_Con_Event_Server_Data
{
   Ecore_Con_Server *server; /**< the server that was connected to */
   void *data;               /**< the data that the server sent */
   int size;                 /**< the length of the data sent */
};

/**
 * @struct _Ecore_Con_Event_Client_Write
 * Used as the @p data param for the @ref ECORE_CON_EVENT_CLIENT_WRITE event
 */
struct _Ecore_Con_Event_Client_Write
{
   Ecore_Con_Client *client; /**< the client that connected */
   int size;                 /**< the length of the data sent */
};

/**
 * @struct _Ecore_Con_Event_Server_Write
 * Used as the @p data param for the @ref ECORE_CON_EVENT_SERVER_WRITE event
 */
struct _Ecore_Con_Event_Server_Write
{
   Ecore_Con_Server *server; /**< the server that was connected to */
   int size;                 /**< the length of the data sent */
};

/**
 * @struct _Ecore_Con_Event_Proxy_Bind
 * Used as the @p data param for the @ref ECORE_CON_EVENT_PROXY_BIND event
 * @ingroup Ecore_Con_Socks_Group
 * @since 1.2
 */
struct _Ecore_Con_Event_Proxy_Bind
{
   Ecore_Con_Server *server; /**< the server object connected to the proxy */
   const char *ip;           /**< the proxy-bound ip address */
   int port;                 /**< the proxy-bound port */
};

/**
 * @struct _Ecore_Con_Event_Url_Data
 * Used as the @p data param for the @ref ECORE_CON_EVENT_URL_DATA event
 * @ingroup Ecore_Con_Url_Group
 */
struct _Ecore_Con_Event_Url_Data
{
   Ecore_Con_Url *url_con; /**< a pointer to the connection object */
   int size; /**< the size of the current received data (in bytes) */
   unsigned char data[1]; /**< the data received on this event */
};

/**
 * @struct _Ecore_Con_Event_Url_Complete
 * Used as the @p data param for the @ref ECORE_CON_EVENT_URL_COMPLETE event
 * @ingroup Ecore_Con_Url_Group
 */
struct _Ecore_Con_Event_Url_Complete
{
   Ecore_Con_Url *url_con; /**< a pointer to the connection object */
   int status; /**< HTTP status code of the operation (200, 404, 401, etc.) */
};

/**
 * @struct _Ecore_Con_Event_Url_Progress
 * Used as the @p data param for the @ref ECORE_CON_EVENT_URL_PROGRESS event
 * @ingroup Ecore_Con_Url_Group
 */
struct _Ecore_Con_Event_Url_Progress
{
   Ecore_Con_Url *url_con; /**< a pointer to the connection object */
   struct
   {
      double total; /**< total size of the downloading data (in bytes) */
      double now; /**< current size of the downloading data (in bytes) */
   } down; /**< download info */
   struct
   {
      double total; /**< total size of the uploading data (in bytes) */
      double now; /**< current size of the uploading data (in bytes) */
   } up; /**< upload info */
};

/** A client has connected to the server */
EAPI extern int ECORE_CON_EVENT_CLIENT_ADD;
/** A client has disconnected from the server */
EAPI extern int ECORE_CON_EVENT_CLIENT_DEL;
/** A client experienced an error
 * @since 1.1
 */
EAPI extern int ECORE_CON_EVENT_CLIENT_ERROR;
/** A client connection has been upgraded to SSL
 * @since 1.1
 */
EAPI extern int ECORE_CON_EVENT_CLIENT_UPGRADE;
/** A server was created */
EAPI extern int ECORE_CON_EVENT_SERVER_ADD;
/** A server connection was lost */
EAPI extern int ECORE_CON_EVENT_SERVER_DEL;
/** A server experienced an error
 * @since 1.1
 */
EAPI extern int ECORE_CON_EVENT_SERVER_ERROR;
/** A server connection has been upgraded to SSL
 * @since 1.1
 */
EAPI extern int ECORE_CON_EVENT_SERVER_UPGRADE;
/** A server connection has sent data to its client
 * @since 1.1
 */
EAPI extern int ECORE_CON_EVENT_CLIENT_WRITE;
/** A server connection object has sent data
 * @since 1.1
 */
EAPI extern int ECORE_CON_EVENT_SERVER_WRITE;
/** A client connected to the server has sent data */
EAPI extern int ECORE_CON_EVENT_CLIENT_DATA;
/** A server connection object has data */
EAPI extern int ECORE_CON_EVENT_SERVER_DATA;
/** A server connection has successfully negotiated an ip:port binding
 * @since 1.2
 */
EAPI extern int ECORE_CON_EVENT_PROXY_BIND;
/** A URL object has data */
EAPI extern int ECORE_CON_EVENT_URL_DATA;
/** A URL object has completed its transfer to and from the server and can be reused */
EAPI extern int ECORE_CON_EVENT_URL_COMPLETE;
/** A URL object has made progress in its transfer */
EAPI extern int ECORE_CON_EVENT_URL_PROGRESS;

/**
 * @}
 */

/**
 * @defgroup Ecore_Con_Lib_Group Ecore Connection Library Functions
 * @ingroup Ecore_Con_Group
 *
 * Utility functions that set up and shut down the Ecore Connection
 * library.
 *
 * There's also ecore_con_lookup() that can be used to make simple asynchronous
 * DNS lookups.
 *
 * A simple example of how to use these functions:
 * @li @ref ecore_con_lookup_example_c
 *
 * @{
 */

/**
 * @typedef Ecore_Con_Dns_Cb
 * A callback type for use with @ref ecore_con_lookup.
 */
typedef void (*Ecore_Con_Dns_Cb)(const char *canonname,
                                 const char *ip,
                                 struct sockaddr *addr,
                                 int addrlen,
                                 void *data);

/**
 * @typedef Ecore_Con_Type
 * @enum _Ecore_Con_Type
 * Types for an ecore_con client/server object.  A correct way to set this type is
 * with an ECORE_CON_$TYPE, optionally OR'ed with an ECORE_CON_$USE if encryption is desired,
 * and LOAD_CERT if the previously loaded certificate should be used.
 * @code
 * ECORE_CON_REMOTE_TCP | ECORE_CON_USE_TLS | ECORE_CON_LOAD_CERT
 * @endcode
 * @ingroup Ecore_Con_Server_Group
 */
typedef enum _Ecore_Con_Type
{
   /** Socket in ~/.ecore */
   ECORE_CON_LOCAL_USER = 0,
   /** Socket in /tmp */
   ECORE_CON_LOCAL_SYSTEM = 1,
   /** Abstract socket */
   ECORE_CON_LOCAL_ABSTRACT = 2,
   /** Remote server using TCP */
   ECORE_CON_REMOTE_TCP = 3,
   /** Remote multicast server */
   ECORE_CON_REMOTE_MCAST = 4,
   /** Remote server using UDP */
   ECORE_CON_REMOTE_UDP = 5,
   /** Remote broadcast using UDP */
   ECORE_CON_REMOTE_BROADCAST = 6,
   /** Remote connection sending packets immediately */
   ECORE_CON_REMOTE_NODELAY = 7,
   /** Remote connection sending data in large chunks
    * @note Only available on Linux
    * @since 1.2
    */
   ECORE_CON_REMOTE_CORK = 8,
   /** Use SSL2: UNSUPPORTED. **/
   ECORE_CON_USE_SSL2 = (1 << 4),
   /** Use SSL3 */
   ECORE_CON_USE_SSL3 = (1 << 5),
   /** Use TLS */
   ECORE_CON_USE_TLS = (1 << 6),
   /** Use both TLS and SSL3 */
   ECORE_CON_USE_MIXED = ECORE_CON_USE_SSL3 | ECORE_CON_USE_TLS,
   /** Attempt to use the loaded certificate */
   ECORE_CON_LOAD_CERT = (1 << 7),
   /** Disable all types of proxy on the server
    * @note Only functional for clients
    * @since 1.2
    */
   ECORE_CON_NO_PROXY = (1 << 8),
   ECORE_CON_SOCKET_ACTIVATE = (1 << 9) 
} Ecore_Con_Type;

/**
 * Initialises the Ecore_Con library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 *
 * @note This function already calls ecore_init() internally, so you don't need
 * to call it explicitly.
 */
EAPI int               ecore_con_init(void);

/**
 * Shuts down the Ecore_Con library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 * @note This function already calls ecore_shutdown() internally, so you don't
 * need to call it explicitly unless you called ecore_init() explicitly too.
 */
EAPI int               ecore_con_shutdown(void);

/**
 * Do an asynchronous DNS lookup.
 *
 * @param name IP address or server name to translate.
 * @param done_cb Callback to notify when done.
 * @param data User data to be given to done_cb.
 * @return @c EINA_TRUE if the request did not fail to be set up, @c EINA_FALSE
 * if it failed.
 *
 * This function performs a DNS lookup on the hostname specified by @p name,
 * then calls @p done_cb with the result and the @p data given as parameter.
 * The result will be given to the @p done_cb as follows:
 * @li @c canonname - the canonical name of the address
 * @li @c ip - the resolved ip address
 * @li @c addr - a pointer to the socket address
 * @li @c addrlen - the length of the socket address, in bytes
 * @li @c data - the data pointer given as parameter to ecore_con_lookup()
 */
EAPI Eina_Bool         ecore_con_lookup(const char *name,
                                            Ecore_Con_Dns_Cb done_cb,
                                            const void *data);

/**
 * @}
 */

/**
 * @defgroup Ecore_Con_SSL_Group Ecore Connection SSL Functions
 * @ingroup Ecore_Con_Group
 *
 * @{
 */
EAPI int               ecore_con_ssl_available_get(void);
EAPI Eina_Bool         ecore_con_ssl_server_cert_add(Ecore_Con_Server *svr, const char *cert);
EAPI Eina_Bool         ecore_con_ssl_server_privkey_add(Ecore_Con_Server *svr, const char *key_file);
EAPI Eina_Bool         ecore_con_ssl_server_crl_add(Ecore_Con_Server *svr, const char *crl_file);
EAPI Eina_Bool         ecore_con_ssl_server_cafile_add(Ecore_Con_Server *svr, const char *ca_file);
EAPI void              ecore_con_ssl_server_verify(Ecore_Con_Server *svr);
EAPI void              ecore_con_ssl_server_verify_basic(Ecore_Con_Server *svr);
EAPI void              ecore_con_ssl_server_verify_name_set(Ecore_Con_Server *svr, const char *name);
EAPI const char       *ecore_con_ssl_server_verify_name_get(Ecore_Con_Server *svr);
EAPI Eina_Bool         ecore_con_ssl_server_upgrade(Ecore_Con_Server *svr, Ecore_Con_Type compl_type);
EAPI Eina_Bool         ecore_con_ssl_client_upgrade(Ecore_Con_Client *cl, Ecore_Con_Type compl_type);

/**
 * @}
 */

EAPI Ecore_Con_Socks *ecore_con_socks4_remote_add(const char *ip, int port, const char *username);
EAPI Eina_Bool        ecore_con_socks4_remote_exists(const char *ip, int port, const char *username);
EAPI void             ecore_con_socks4_remote_del(const char *ip, int port, const char *username);
EAPI Ecore_Con_Socks *ecore_con_socks5_remote_add(const char *ip, int port, const char *username, const char *password);
EAPI Eina_Bool        ecore_con_socks5_remote_exists(const char *ip, int port, const char *username, const char *password);
EAPI void             ecore_con_socks5_remote_del(const char *ip, int port, const char *username, const char *password);
EAPI void             ecore_con_socks_lookup_set(Ecore_Con_Socks *ecs, Eina_Bool enable);
EAPI Eina_Bool        ecore_con_socks_lookup_get(Ecore_Con_Socks *ecs);
EAPI void             ecore_con_socks_bind_set(Ecore_Con_Socks *ecs, Eina_Bool is_bind);
EAPI Eina_Bool        ecore_con_socks_bind_get(Ecore_Con_Socks *ecs);
EAPI unsigned int     ecore_con_socks_version_get(Ecore_Con_Socks *ecs);
EAPI void             ecore_con_socks_remote_del(Ecore_Con_Socks *ecs);
EAPI void             ecore_con_socks_apply_once(Ecore_Con_Socks *ecs);
EAPI void             ecore_con_socks_apply_always(Ecore_Con_Socks *ecs);

/**
 * @defgroup Ecore_Con_Server_Group Ecore Connection Server Functions
 * @ingroup Ecore_Con_Group
 *
 * This group of functions is applied to an @ref Ecore_Con_Server object. It
 * doesn't mean that they should be used in the server application, but on the
 * server object. In fact, most of them should be used in the client
 * application, when retrieving information or sending data.
 *
 * Setting up a server is very simple: you just need to start it with
 * ecore_con_server_add() and setup some callbacks to the events
 * @ref ECORE_CON_EVENT_CLIENT_ADD, @ref ECORE_CON_EVENT_CLIENT_DEL and
 * @ref ECORE_CON_EVENT_CLIENT_DATA, that will be called when a client is
 * communicating with the server:
 *
 * @code
 * if (!(svr = ecore_con_server_add(ECORE_CON_REMOTE_TCP, "127.0.0.1", 8080, NULL)))
 *   exit(1);
 *
 * ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, _add_cb, NULL);
 * ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, _del_cb, NULL);
 * ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, _data_cb, NULL);
 *
 * ecore_main_loop_begin();
 * @endcode
 *
 * The function ecore_con_server_connect() can be used to write a client that
 * connects to a server. The resulting code will be very similar to the server
 * code:
 *
 * @code
 * if (!(svr = ecore_con_server_connect(ECORE_CON_REMOTE_TCP, "127.0.0.1", 8080, NULL)))
 *   exit(1);
 *
 * ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, _add_cb, NULL);
 * ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, _del_cb, NULL);
 * ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, _data_cb, NULL);
 *
 * ecore_main_loop_begin();
 * @endcode
 *
 * After these two pieces of code are executed, respectively, in the server and
 * client code, the server will be up and running and the client will try to
 * connect to it. The connection, with its subsequent messages being sent from
 * server to client and client to server, can be represented in the following
 * sequence diagram:
 *
 * @htmlonly
 * <img src="ecore_con-client-server.png" style="max-width: 400px"/>
 * <a href="ecore_con-client-server.png">Full size</a>
 * @endhtmlonly
 *
 * @image rtf ecore_con-client-server.png
 * @image latex ecore_con-client-server.eps width=\textwidth
 *
 * Please notice the important difference between these two codes: the first is
 * used for writing a @b server, while the second should be used for writing a
 * @b client.
 *
 * A reference for the @c client functions can be found at @ref
 * Ecore_Con_Client_Group.
 *
 * Examples of usage for this API can be found here:
 * @li @ref ecore_con_server_simple_example_c
 * @li @ref ecore_con_client_simple_example_c
 *
 * @{
 */

/**
 * Creates a server to listen for connections.
 *
 * @param  type The connection type.
 * @param  name       Name to associate with the socket.  It is used when
 *                    generating the socket name of a Unix socket, or for
 *                    determining what host to listen on for TCP sockets.
 *                    @c NULL will not be accepted.
 * @param  port       Number to identify socket.  When a Unix socket is used,
 *                    it becomes part of the socket name.  When a TCP socket
 *                    is used, it is used as the TCP port.
 * @param  data       Data to associate with the created Ecore_Con_Server
 *                    object.
 * @return A new Ecore_Con_Server.
 *
 * The socket on which the server listens depends on the connection
 * type:
 * @li If @a type is @c ECORE_CON_LOCAL_USER, the server will listen on
 *     the Unix socket "~/.ecore/[name]/[port]".
 * @li If @a type is @c ECORE_CON_LOCAL_SYSTEM, the server will listen
 *     on Unix socket "/tmp/.ecore_service|[name]|[port]".
 * @li If @a type is @c ECORE_CON_REMOTE_TCP, the server will listen
 *     on TCP port @c port.
 *
 * More information about the @p type can be found at @ref _Ecore_Con_Type.
 *
 * The @p data parameter can be fetched later using ecore_con_server_data_get()
 * or changed with ecore_con_server_data_set().
 */
EAPI Ecore_Con_Server *ecore_con_server_add(Ecore_Con_Type type,
                                            const char *name, int port,
                                            const void *data);

/**
 * Creates a connection to the specified server and returns an associated object.
 *
 * @param  type The connection type.
 * @param  name       Name used when determining what socket to connect to.
 *                    It is used to generate the socket name when the socket
 *                    is a Unix socket.  It is used as the hostname when
 *                    connecting with a TCP socket.
 * @param  port       Number to identify the socket to connect to.  Used when
 *                    generating the socket name for a Unix socket, or as the
 *                    TCP port when connecting to a TCP socket.
 * @param  data       Data to associate with the created Ecore_Con_Server
 *                    object.
 * @return A new Ecore_Con_Server.
 *
 * The socket to which the connection is made depends on the connection type:
 * @li If @a type is @c ECORE_CON_LOCAL_USER, the function will
 *     connect to the server at the Unix socket
 *     "~/.ecore/[name]/[port]".
 * @li If @a type is @c ECORE_CON_LOCAL_SYSTEM, the function will
 *     connect to the server at the Unix socket
 *     "/tmp/.ecore_service|[name]|[port]".
 * @li If @a type is @c ECORE_CON_REMOTE_TCP, the function will
 *     connect to the server at the TCP port "[name]:[port]".
 *
 * More information about the @p type can be found at @ref _Ecore_Con_Type.
 *
 * This function won't block. It will either succeed, or fail due to invalid
 * parameters, failed memory allocation, etc., returning @c NULL on that case.
 *
 * However, even if this call returns a valid @ref Ecore_Con_Server, the
 * connection will only be successfully completed if an event of type
 * @ref ECORE_CON_EVENT_SERVER_ADD is received. If it fails to complete, an
 * @ref ECORE_CON_EVENT_SERVER_DEL will be received.
 *
 * The @p data parameter can be fetched later using ecore_con_server_data_get()
 * or changed with ecore_con_server_data_set().
 */
EAPI Ecore_Con_Server *ecore_con_server_connect(Ecore_Con_Type type,
                                                const char *name, int port,
                                                const void *data);
/**
 * Closes the connection and frees the given server.
 *
 * @param   svr The given server.
 * @return  Data associated with the server when it was created.
 *
 * All the clients connected to this server will be disconnected.
 *
 * @see ecore_con_server_add, ecore_con_server_connect
 */
EAPI void *            ecore_con_server_del(Ecore_Con_Server *svr);

/**
 * Retrieves the data associated with the given server.
 *
 * @param   svr The given server.
 * @return  The associated data.
 *
 * @see ecore_con_server_data_set()
 */
EAPI void *            ecore_con_server_data_get(Ecore_Con_Server *svr);
/**
 * Sets the data associated with the given server.
 *
 * @param svr The given server.
 * @param data The data to associate with @p svr
 * @return  The previously associated data, if any.
 *
 * @see ecore_con_server_data_get()
 */
EAPI void *            ecore_con_server_data_set(Ecore_Con_Server *svr,
                                                 void *data);
/**
 * Retrieves whether the given server is currently connected.
 *
 * @param   svr The given server.
 * @return @c EINA_TRUE if the server is connected, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool         ecore_con_server_connected_get(Ecore_Con_Server *svr);
/**
 * Retrieves the current list of clients.
 *
 * @param   svr The given server.
 * @return  The list of clients on this server.
 *
 * Each node in the returned list points to an @ref Ecore_Con_Client. This list
 * cannot be modified or freed. It can also change if new clients are connected
 * or disconnected, and will become invalid when the server is deleted/freed.
 */
EAPI const Eina_List * ecore_con_server_clients_get(Ecore_Con_Server *svr);

/**
 * Retrieves the name of server.
 *
 * @param   svr The given server.
 * @return  The name of the server.
 *
 * The name returned is the name used to connect on this server.
 */
EAPI const char *      ecore_con_server_name_get(Ecore_Con_Server *svr);

/**
 * Retrieves the server port in use.
 *
 * @param   svr The given server.
 * @return  The server port in use.
 *
 * The port where the server is listening for connections.
 */
EAPI int               ecore_con_server_port_get(Ecore_Con_Server *svr);
/**
 * @brief Check how long a server has been connected
 *
 * @param svr The server to check
 * @return The total time, in seconds, that the server has been
 * connected/running
 *
 * This function is used to find out the time that has been elapsed since
 * ecore_con_server_add() succeeded.
 */
EAPI double            ecore_con_server_uptime_get(Ecore_Con_Server *svr);
/**
 * Sends the given data to the given server.
 *
 * @param   svr  The given server.
 * @param   data The given data.
 * @param   size Length of the data, in bytes, to send.
 * @return  The number of bytes sent.  @c 0 will be returned if there is an
 *          error.
 *
 * This function will send the given data to the server as soon as the program
 * is back to the main loop. Thus, this function returns immediately
 * (non-blocking). If the data needs to be sent @b now, call
 * ecore_con_server_flush() after this one.
 *
 * @see ecore_con_client_send()
 * @see ecore_con_server_flush()
 */
EAPI int               ecore_con_server_send(Ecore_Con_Server *svr,
                                             const void *data,
                                             int size);
/**
 * Sets a limit on the number of clients that can be handled concurrently
 * by the given server, and a policy on what to do if excess clients try to
 * connect.
 *
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
 *
 * Beware that if you set this once ecore is already running, you may
 * already have pending CLIENT_ADD events in your event queue.  Those
 * clients have already connected and will not be affected by this call.
 * Only clients subsequently trying to connect will be affected.
 */
EAPI void              ecore_con_server_client_limit_set(Ecore_Con_Server *svr,
                                                         int client_limit,
                                                         char reject_excess_clients);
/**
 * Gets the IP address of a server that has been connected to.
 *
 * @param   svr           The given server.
 * @return  A pointer to an internal string that contains the IP address of
 *          the connected server in the form "XXX.YYY.ZZZ.AAA" IP notation.
 *          This string should not be modified or trusted to stay valid after
 *          deletion for the @p svr object. If no IP is known @c NULL is
 *          returned.
 */
EAPI const char *      ecore_con_server_ip_get(Ecore_Con_Server *svr);
/**
 * Flushes all pending data to the given server.
 *
 * @param   svr           The given server.
 *
 * This function will block until all data is sent to the server.
 *
 * @see ecore_con_server_send()
 * @see ecore_con_client_flush()
 */
EAPI void              ecore_con_server_flush(Ecore_Con_Server *svr);
/**
 * Set the default time after which an inactive client will be disconnected
 *
 * @param svr The server object
 * @param timeout The timeout, in seconds, to disconnect after
 *
 * This function is used by the server to set the default idle timeout on
 * clients. If the any of the clients becomes idle for a time higher than this
 * value, it will be disconnected. A value of < 1 disables the idle timeout.
 *
 * This timeout is not affected by the one set by
 * ecore_con_client_timeout_set(). A client will be disconnected whenever the
 * client or the server timeout is reached. That means, the lower timeout value
 * will be used for that client if ecore_con_client_timeout_set() is used on it.
 *
 * @see ecore_con_server_timeout_get()
 * @see ecore_con_client_timeout_set()
 */
EAPI void              ecore_con_server_timeout_set(Ecore_Con_Server *svr, double timeout);
/**
 * Get the default time after which an inactive client will be disconnected
 *
 * @param svr The server object
 * @return The timeout, in seconds, to disconnect after
 *
 * This function is used to get the idle timeout for clients.  A value of < 1
 * means the idle timeout is disabled.
 *
 * @see ecore_con_server_timeout_set()
 * @see ecore_con_client_timeout_get()
 */
EAPI double            ecore_con_server_timeout_get(Ecore_Con_Server *svr);

/**
 * Get the fd that the server is connected to
 *
 * @param svr The server object
 * @return The fd, or -1 on failure
 *
 * This function returns the fd which is used by the underlying server connection.
 * It should not be tampered with unless you REALLY know what you are doing.
 * @note This function is only valid for servers created with ecore_con_server_connect()
 * @warning Seriously. Don't use this unless you know what you are doing.
 * @since 1.1
 */
EAPI int               ecore_con_server_fd_get(Ecore_Con_Server *svr);

/**
 * Get the fd that the client is connected to
 *
 * @param cl The client object
 * @return The fd, or -1 on failure
 *
 * This function returns the fd which is used by the underlying client connection.
 * It should not be tampered with unless you REALLY know what you are doing.
 * @since 1.1
 */
EAPI int               ecore_con_client_fd_get(Ecore_Con_Client *cl);
/**
 * @}
 */

/**
 * @defgroup Ecore_Con_Client_Group Ecore Connection Client Functions
 * @ingroup Ecore_Con_Group
 *
 * Functions to communicate with and/or set options on a client.
 *
 * This set of functions, as explained in @ref Ecore_Con_Server_Group, is used
 * to send data to a client, or to set options and get information about this
 * client. Most of them should be used on the server, applied on the client
 * object.
 *
 * If you need to implement a client, the way to connect to a server is
 * described in @ref Ecore_Con_Server_Group.
 *
 * An example of usage of these functions can be found at:
 * @li @ref ecore_con_client_simple_example_c
 *
 * @{
 */

/**
 * Sends the given data to the given client.
 *
 * @param   cl   The given client.
 * @param   data The given data.
 * @param   size Length of the data, in bytes, to send.
 * @return  The number of bytes sent.  @c 0 will be returned if there is an
 *          error.
 *
 * This function will send the given data to the client as soon as the program
 * is back to the main loop. Thus, this function returns immediately
 * (non-blocking). If the data needs to be sent @b now, call
 * ecore_con_client_flush() after this one.
 *
 * @see ecore_con_server_send()
 * @see ecore_con_client_flush()
 */
EAPI int               ecore_con_client_send(Ecore_Con_Client *cl,
                                             const void *data,
                                             int size);
/**
 * Retrieves the server representing the socket the client has
 * connected to.
 *
 * @param   cl The given client.
 * @return  The server that the client connected to.
 */
EAPI Ecore_Con_Server *ecore_con_client_server_get(Ecore_Con_Client *cl);
/**
 * Closes the connection and frees memory allocated to the given client.
 *
 * @param   cl The given client.
 * @return  Data associated with the client.
 */
EAPI void *            ecore_con_client_del(Ecore_Con_Client *cl);
/**
 * Sets the data associated with the given client to @p data.
 *
 * @param   cl   The given client.
 * @param   data What to set the data to.
 */
EAPI void              ecore_con_client_data_set(Ecore_Con_Client *cl,
                                                 const void       *data);
/**
 * Retrieves the data associated with the given client.
 *
 * @param   cl The given client.
 * @return  The data associated with @p cl.
 */
EAPI void *            ecore_con_client_data_get(Ecore_Con_Client *cl);

/**
 * Gets the IP address of a client that has connected.
 *
 * @param   cl            The given client.
 * @return  A pointer to an internal string that contains the IP address of
 *          the connected client in the form "XXX.YYY.ZZZ.AAA" IP notation.
 *
 * The returned string should not be modified, freed or trusted to stay valid
 * after deletion for the @p cl object. If no IP is known @c NULL is returned.
 */
EAPI const char *      ecore_con_client_ip_get(Ecore_Con_Client *cl);
/**
 * Flushes all pending data to the given client.
 *
 * @param   cl            The given client.
 *
 * This function will block until all data is sent to the server.
 *
 * @see ecore_con_client_send()
 * @see ecore_con_server_flush()
 */
EAPI void              ecore_con_client_flush(Ecore_Con_Client *cl);
/**
 * @brief Check how long a client has been connected
 *
 * @param cl The client to check
 * @return The total time, in seconds, that the client has been connected to
 * the server
 *
 * This function is used to find out how long a client has been connected for.
 */
EAPI double            ecore_con_client_uptime_get(Ecore_Con_Client *cl);
/**
 * Get the default time after which the client will be disconnected when
 * inactive
 *
 * @param cl The client object
 * @return The timeout, in seconds, to disconnect after
 *
 * This function is used to get the idle timeout for a client.  A value of < 1
 * means the idle timeout is disabled.
 *
 * @see ecore_con_client_timeout_set()
 */
EAPI double            ecore_con_client_timeout_get(Ecore_Con_Client *cl);
/**
 * Set the time after which the client will be disconnected when inactive
 *
 * @param cl The client object
 * @param timeout The timeout, in seconds, to disconnect after
 *
 * This function is used by the server to set the idle timeout on a specific
 * client. If the client becomes idle for a time higher than this value, it will
 * be disconnected. A value of < 1 disables the idle timeout.
 *
 * This timeout is not affected by the one set by
 * ecore_con_server_timeout_set(). A client will be disconnected whenever the
 * client or the server timeout is reached. That means, the lower timeout value
 * will be used for that client if ecore_con_server_timeout_set() is used on the
 * server.
 *
 * @see ecore_con_client_timeout_get()
 * @see ecore_con_server_timeout_set()
 */
EAPI void              ecore_con_client_timeout_set(Ecore_Con_Client *cl, double timeout);
/**
 * Returns whether the client is still connected
 *
 * @param   cl The given client.
 * @return @c EINA_TRUE if connected, @c EINA_FALSE otherwise.
 */
EAPI Eina_Bool         ecore_con_client_connected_get(Ecore_Con_Client *cl);
/**
 * @brief Return the port that the client has connected to
 *
 * @param cl The client
 * @return The port that @p cl has connected to, or -1 on error
 * Use this function to return the port on which a given client has connected.
 */
EAPI int               ecore_con_client_port_get(Ecore_Con_Client *cl);



/**
 * @}
 */

/**
 * @defgroup Ecore_Con_Url_Group Ecore URL Connection Functions
 * @ingroup Ecore_Con_Group
 *
 * Utility functions that set up, use and shut down the Ecore URL
 * Connection library.
 *
 * These functions are a shortcut to make it easy to perform http requests
 * (POST, GET, etc).
 *
 * Brief usage:
 * 1. Create an Ecore_Con_Url object with ecore_con_url_new(url);
 * 2. Register to receive the #ECORE_CON_EVENT_URL_COMPLETE event
 *    (and optionally the #ECORE_CON_EVENT_URL_DATA and
 *    #ECORE_CON_EVENT_URL_PROGRESS event to receive
 *    the response, e.g. for HTTP/FTP downloads)
 * 3. Perform the operation with ecore_con_url_get(...);
 *
 * Note that it is good to reuse @ref Ecore_Con_Url objects wherever possible,
 * but bear in mind that each one can only perform one operation at a time.  You
 * need to wait for the #ECORE_CON_EVENT_URL_COMPLETE event before re-using or
 * destroying the object.
 *
 * If it's necessary to change the @ref Ecore_Con_Url object url, use
 * ecore_con_url_url_set().
 *
 * Simple Usage 1 (HTTP GET):
 * @code
 *   ecore_con_url_url_set(url_con, "http://www.google.com");
 *   ecore_con_url_get(url_con);
 * @endcode
 *
 * Simple usage 2 (HTTP POST):
 * @code
 *   ecore_con_url_url_set(url_con, "http://www.example.com/post_handler.cgi");
 *   ecore_con_url_post(url_con, data, data_length, "multipart/form-data");
 * @endcode
 *
 * Simple Usage 3 (FTP download):
 * @code
 *   fd = creat(filename, 0644)
 *   ecore_con_url_url_set(url_con, "ftp://ftp.example.com/pub/myfile");
 *   ecore_con_url_fd_set(url_con, fd);
 *   ecore_con_url_get(url_con);
 * @endcode
 *
 * Simple Usage 4 (FTP upload as ftp://ftp.example.com/file):
 * @code
 *   ecore_con_url_url_set(url_con, "ftp://ftp.example.com");
 *   ecore_con_url_ftp_upload(url_con, "/tmp/file", "user", "pass", NULL);
 * @endcode
 *
 * Simple Usage 5 (FTP upload as ftp://ftp.example.com/dir/file):
 * @code
 *   ecore_con_url_url_set(url_con, "ftp://ftp.example.com");
 *   ecore_con_url_ftp_upload(url_con, "/tmp/file", "user", "pass","dir");
 * @endcode
 *
 * These are complete examples for the API:
 * @li @ref ecore_con_url_download_example.c "Downloading a file"
 * @li @ref ecore_con_url_headers_example.c "Setting many options for the connection"
 *
 * @{
 */

/**
 * @typedef Ecore_Con_Url_Time
 * @enum _Ecore_Con_Url_Time
 * The type of condition to use when making an HTTP request dependent on time,
 * so that headers such as "If-Modified-Since" are used.
 */
typedef enum _Ecore_Con_Url_Time
{
   /**
    * Do not place time restrictions on the HTTP requests.
    */
   ECORE_CON_URL_TIME_NONE = 0,
   /**
    * Add the "If-Modified-Since" HTTP header, so that the request is performed
    * by the server only if the target has been modified since the time value
    * passed to it in the request.
    */
   ECORE_CON_URL_TIME_IFMODSINCE,
   /**
    * Add the "If-Unmodified-Since" HTTP header, so that the request is
    * performed by the server only if the target has NOT been modified since
    * the time value passed to it in the request.
    */
   ECORE_CON_URL_TIME_IFUNMODSINCE
} Ecore_Con_Url_Time;

/**
 * @typedef Ecore_Con_Url_Http_Version
 * @enum _Ecore_Con_Url_Http_Version
 * The http version to use
 * @since 1.2
 */
typedef enum _Ecore_Con_Url_Http_Version
{
   /**
    * HTTP version 1.0
    * @since 1.2
    */
   ECORE_CON_URL_HTTP_VERSION_1_0,
   /**
    * HTTP version 1.1 (default)
    * @since 1.2
    */
   ECORE_CON_URL_HTTP_VERSION_1_1
} Ecore_Con_Url_Http_Version;

/**
 * Change the HTTP version used for the request
 * @param url_con Connection object through which the request will be sent.
 * @param version The version to be used
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure to change version.
 * @since 1.2
 * @see ecore_con_url_pipeline_get()
 */
EAPI Eina_Bool         ecore_con_url_http_version_set(Ecore_Con_Url *url_con, Ecore_Con_Url_Http_Version version);
   
/**
 * Initialises the Ecore_Con_Url library.
 * @return Number of times the library has been initialised without being
 *          shut down.
 *
 * @note This function doesn't call ecore_con_init(). You still need to call it
 * explicitly before calling this one.
 */
EAPI int               ecore_con_url_init(void);

/**
 * Shuts down the Ecore_Con_Url library.
 * @return  Number of calls that still uses Ecore_Con_Url
 *
 * @note This function doesn't call ecore_con_shutdown(). You still need to call
 * it explicitly after calling this one.
 */
EAPI int               ecore_con_url_shutdown(void);

/**
 * Enable or disable HTTP 1.1 pipelining.
 * @param enable @c EINA_TRUE will turn it on, @c EINA_FALSE will disable it.
 *
 * Pipelining allows to send one request after another one, without having to
 * wait for the reply of the first request. The respective replies are received
 * in the order that the requests were sent.
 *
 * Enabling this feature will be valid for all requests done using @c
 * ecore_con_url.
 *
 * See http://en.wikipedia.org/wiki/HTTP_pipelining for more info.
 *
 * @see ecore_con_url_pipeline_get()
 */
EAPI void              ecore_con_url_pipeline_set(Eina_Bool enable);
/**
 * Is HTTP 1.1 pipelining enable ?
 * @return @c EINA_TRUE if it is enable.
 *
 * @see ecore_con_url_pipeline_set()
 */
EAPI Eina_Bool         ecore_con_url_pipeline_get(void);

/**
 * Creates and initializes a new Ecore_Con_Url connection object.
 *
 * @param url URL that will receive requests. Can be changed using
 *            ecore_con_url_url_set.
 *
 * @return @c NULL on error, a new Ecore_Con_Url on success.
 *
 * Creates and initializes a new Ecore_Con_Url connection object that can be
 * used for sending requests.
 *
 * @see ecore_con_url_custom_new()
 * @see ecore_con_url_url_set()
 */
EAPI Ecore_Con_Url *   ecore_con_url_new(const char *url);
/**
 * Creates a custom connection object.
 *
 * @param url URL that will receive requests
 * @param custom_request Custom request (e.g. GET, POST, HEAD, PUT, etc)
 *
 * @return @c NULL on error, a new Ecore_Con_Url on success.
 *
 * Creates and initializes a new Ecore_Con_Url for a custom request (e.g. HEAD,
 * SUBSCRIBE and other obscure HTTP requests). This object should be used like
 * one created with ecore_con_url_new().
 *
 * @see ecore_con_url_new()
 * @see ecore_con_url_url_set()
 */
EAPI Ecore_Con_Url *   ecore_con_url_custom_new(const char *url,
                                                const char *custom_request);
/**
 * Destroys a Ecore_Con_Url connection object.
 *
 * @param url_con Connection object to free.
 *
 * @see ecore_con_url_new()
 */
EAPI void              ecore_con_url_free(Ecore_Con_Url *url_con);
/**
 * Sets the URL to send the request to.
 *
 * @param url_con Connection object through which the request will be sent.
 * @param url URL that will receive the request
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 */
EAPI Eina_Bool         ecore_con_url_url_set(Ecore_Con_Url *url_con,
                                             const char *url);
/**
 * Gets the URL to send the request to.
 *
 * @param url_con Connection object through which the request will be sent.
 * @return URL that will receive the request, @c NULL on failure. URL is
 * stringshared.
 * @since 1.1
 */
EAPI const char       *ecore_con_url_url_get(Ecore_Con_Url *url_con);
/**
 * Associates data with a connection object.
 *
 * @param url_con Connection object to associate data.
 * @param data Data to be set.
 *
 * Associates data with a connection object, which can be retrieved later with
 * ecore_con_url_data_get()).
 *
 * @see ecore_con_url_data_get()
 */
EAPI void              ecore_con_url_data_set(Ecore_Con_Url *url_con,
                                              void *data);
/**
 * Retrieves data associated with a Ecore_Con_Url connection object.
 *
 * @param url_con Connection object to retrieve data from.
 *
 * @return Data associated with the given object.
 *
 * Retrieves data associated with a Ecore_Con_Url connection object (previously
 * set with ecore_con_url_data_set()).
 *
 * @see ecore_con_url_data_set()
 */
EAPI void *            ecore_con_url_data_get(Ecore_Con_Url *url_con);
/**
 * Adds an additional header to the request connection object.
 *
 * @param url_con Connection object
 * @param key Header key
 * @param value Header value
 *
 * Adds an additional header (User-Agent, Content-Type, etc.) to the request
 * connection object. This addition will be valid for only one
 * ecore_con_url_get() or ecore_con_url_post() call.
 *
 * Some functions like ecore_con_url_time() also add headers to the request.
 *
 * @see ecore_con_url_get()
 * @see ecore_con_url_post()
 * @see ecore_con_url_additional_headers_clear()
 */
EAPI void              ecore_con_url_additional_header_add(Ecore_Con_Url *url_con,
                                                           const char *key,
                                                           const char *value);
/**
 * Cleans additional headers.
 *
 * @param url_con Connection object to clean additional headers.
 *
 * Cleans additional headers associated with a connection object (previously
 * added with ecore_con_url_additional_header_add()).
 *
 * @see ecore_con_url_additional_header_add()
 * @see ecore_con_url_get()
 * @see ecore_con_url_post()
 */
EAPI void              ecore_con_url_additional_headers_clear(Ecore_Con_Url *url_con);
/**
 * Retrieves headers from last request sent.
 *
 * @param url_con Connection object to retrieve response headers from.
 *
 * Retrieves a list containing the response headers. This function should be
 * used after an ECORE_CON_EVENT_URL_COMPLETE event (headers should normally be
 * ready at that time).
 *
 * @return List of response headers. This list must not be modified by the user.
 */
EAPI const Eina_List * ecore_con_url_response_headers_get(Ecore_Con_Url *url_con);
/**
 * Setup a file for receiving response data.
 *
 * @param url_con Connection object to set file
 * @param fd File descriptor associated with the file. A negative value will
 * unset any previously set fd.
 *
 * Sets up a file to have response data written into. Note that
 * ECORE_CON_EVENT_URL_DATA events will not be emitted if a file has been set to
 * receive the response data.
 *
 * This call can be used to easily setup a file where the downloaded data will
 * be saved.
 */
EAPI void              ecore_con_url_fd_set(Ecore_Con_Url *url_con, int fd);
/**
 * Retrieves the number of bytes received.
 *
 * Retrieves the number of bytes received on the last request of the given
 * connection object.
 *
 * @param url_con Connection object which the request was sent on.
 *
 * @return Number of bytes received on request.
 *
 * @see ecore_con_url_get()
 * @see ecore_con_url_post()
 */
EAPI int               ecore_con_url_received_bytes_get(Ecore_Con_Url *url_con);
/**
 * Sets url_con to use http auth, with given username and password, "safely" or not.
 *
 * @param url_con Connection object to perform a request on, previously created
 *    with ecore_con_url_new() or ecore_con_url_custom_new().
 * @param username Username to use in authentication
 * @param password Password to use in authentication
 * @param safe Whether to use "safer" methods (eg, NOT http basic auth)
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 *
 * @attention Requires libcurl >= 7.19.1 to work, otherwise will always return
 * @c 0.
 */
EAPI Eina_Bool         ecore_con_url_httpauth_set(Ecore_Con_Url *url_con,
                                                  const char *username,
                                                  const char *password,
                                                  Eina_Bool safe);
/**
 * Sends a get request.
 *
 * @param url_con Connection object to perform a request on, previously created
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 *
 * The request is performed immediately, but you need to setup event handlers
 * for #ECORE_CON_EVENT_URL_DATA, #ECORE_CON_EVENT_URL_COMPLETE or
 * #ECORE_CON_EVENT_URL_PROGRESS to get more information about its result.
 *
 * @see ecore_con_url_custom_new()
 * @see ecore_con_url_additional_headers_clear()
 * @see ecore_con_url_additional_header_add()
 * @see ecore_con_url_data_set()
 * @see ecore_con_url_data_get()
 * @see ecore_con_url_response_headers_get()
 * @see ecore_con_url_time()
 * @see ecore_con_url_post()
 */
EAPI Eina_Bool         ecore_con_url_get(Ecore_Con_Url *url_con);
/**
 * Sends a post request.
 *
 * @param url_con Connection object to perform a request on, previously created
 *                with ecore_con_url_new() or ecore_con_url_custom_new().
 * @param data    Payload (data sent on the request). Can be @c NULL.
 * @param length  Payload length. If @c -1, rely on automatic length
 *                calculation via @c strlen() on @p data.
 * @param content_type Content type of the payload (e.g. text/xml). Can be @c
 * NULL.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 *
 * The request starts immediately, but you need to setup event handlers
 * for #ECORE_CON_EVENT_URL_DATA, #ECORE_CON_EVENT_URL_COMPLETE or
 * #ECORE_CON_EVENT_URL_PROGRESS to get more information about its result.
 *
 * This call won't block your main loop.
 *
 * @see ecore_con_url_custom_new()
 * @see ecore_con_url_additional_headers_clear()
 * @see ecore_con_url_additional_header_add()
 * @see ecore_con_url_data_set()
 * @see ecore_con_url_data_get()
 * @see ecore_con_url_response_headers_get()
 * @see ecore_con_url_time()
 * @see ecore_con_url_get()
 */
EAPI Eina_Bool         ecore_con_url_post(Ecore_Con_Url *url_con,
                                          const void *data, long length,
                                          const char *content_type);
/**
 * Sets whether HTTP requests should be conditional, dependent on
 * modification time.
 *
 * @param url_con   Ecore_Con_Url to act upon.
 * @param time_condition Condition to use for HTTP requests.
 * @param timestamp Time since 1 Jan 1970 to use in the condition.
 *
 * This function may set the header "If-Modified-Since" or
 * "If-Unmodified-Since", depending on the value of @p time_condition, with the
 * value @p timestamp.
 *
 * @sa ecore_con_url_get()
 * @sa ecore_con_url_post()
 */
EAPI void              ecore_con_url_time(Ecore_Con_Url *url_con,
                                          Ecore_Con_Url_Time time_condition,
                                          double timestamp);

/**
 * @brief Uploads a file to an ftp site.
 *
 * @param url_con The Ecore_Con_Url object to send with
 * @param filename The path to the file to send
 * @param user The username to log in with
 * @param pass The password to log in with
 * @param upload_dir The directory to which the file should be uploaded
 * @return @c EINA_TRUE on success, @c EINA_FALSE otherwise.
 *
 * Upload @p filename to an ftp server set in @p url_con using @p user
 * and @p pass to directory @p upload_dir
 */
EAPI Eina_Bool         ecore_con_url_ftp_upload(Ecore_Con_Url *url_con,
                                                const char *filename,
                                                const char *user,
                                                const char *pass,
                                                const char *upload_dir);
/**
 * Toggle libcurl's verbose output.
 *
 * @param url_con Ecore_Con_Url instance which will be acted upon.
 * @param verbose Whether or not to enable libcurl's verbose output.
 *
 * If @p verbose is @c EINA_TRUE, libcurl will output a lot of verbose
 * information about its operations, which is useful for
 * debugging. The verbose information will be sent to stderr.
 */
EAPI void              ecore_con_url_verbose_set(Ecore_Con_Url *url_con,
                                                 Eina_Bool verbose);
/**
 * Enable or disable EPSV extension
 * @param url_con  The Ecore_Con_Url instance which will be acted upon.
 * @param use_epsv Boolean to enable/disable the EPSV extension.
 */
EAPI void              ecore_con_url_ftp_use_epsv_set(Ecore_Con_Url *url_con,
                                                      Eina_Bool use_epsv);

/**
 * Enables the cookie engine for subsequent HTTP requests.
 *
 * @param url_con Ecore_Con_Url instance which will be acted upon.
 *
 * After this function is called, cookies set by the server in HTTP responses
 * will be parsed and stored, as well as sent back to the server in new HTTP
 * requests.
 *
 * @note Even though this function is called @c ecore_con_url_cookies_init(),
 * there is no symmetrical shutdown operation.
 */
EAPI void              ecore_con_url_cookies_init(Ecore_Con_Url *url_con);
/**
 * Controls whether session cookies from previous sessions shall be loaded.
 *
 * @param url_con Ecore_Con_Url instance which will be acted upon.
 * @param ignore  If @c EINA_TRUE, ignore session cookies when loading cookies
 *                from files. If @c EINA_FALSE, all cookies will be loaded.
 *
 * Session cookies are cookies with no expire date set, which usually means
 * they are removed after the current session is closed.
 *
 * By default, when Ecore_Con_Url loads cookies from a file, all cookies are
 * loaded, including session cookies, which, most of the time, were supposed
 * to be loaded and valid only for that session.
 *
 * If @p ignore is set to @c EINA_TRUE, when Ecore_Con_Url loads cookies from
 * the files passed to @c ecore_con_url_cookies_file_add(), session cookies
 * will not be loaded.
 *
 * @see ecore_con_url_cookies_file_add()
 */
EAPI void              ecore_con_url_cookies_ignore_old_session_set(Ecore_Con_Url *url_con,
                                                                    Eina_Bool ignore);
/**
 * Clears currently loaded cookies.
 * @param url_con      Ecore_Con_Url instance which will be acted upon.
 *
 * The cleared cookies are removed and will not be sent in subsequent HTTP
 * requests, nor will they be written to the cookiejar file set via
 * @c ecore_con_url_cookies_jar_file_set().
 *
 * @note This function will initialize the cookie engine if it has not been
 *       initialized yet.
 * @note The cookie files set by ecore_con_url_cookies_file_add() aren't loaded
 *       immediately, just when the request is started. Thus, if you ask to
 *       clear the cookies, but has a file already set by that function, the
 *       cookies will then be loaded and you will have old cookies set. In order
 *       to don't have any old cookie set, you need to don't call
 *       ecore_con_url_cookies_file_add() ever on the @p url_con handler, and
 *       call this function to clear any cookie set by a previous request on
 *       this handler.
 *
 * @see ecore_con_url_cookies_session_clear()
 * @see ecore_con_url_cookies_ignore_old_session_set()
 */
EAPI void              ecore_con_url_cookies_clear(Ecore_Con_Url *url_con);
/**
 * Clears currently loaded session cookies.
 *
 * @param url_con      Ecore_Con_Url instance which will be acted upon.
 *
 * Session cookies are cookies with no expire date set, which usually means
 * they are removed after the current session is closed.
 *
 * The cleared cookies are removed and will not be sent in subsequent HTTP
 * requests, nor will they be written to the cookiejar file set via
 * @c ecore_con_url_cookies_jar_file_set().
 *
 * @note This function will initialize the cookie engine if it has not been
 *       initialized yet.
 * @note The cookie files set by ecore_con_url_cookies_file_add() aren't loaded
 *       immediately, just when the request is started. Thus, if you ask to
 *       clear the session cookies, but has a file already set by that function,
 *       the session cookies will then be loaded and you will have old cookies
 *       set.  In order to don't have any old session cookie set, you need to
 *       don't call ecore_con_url_cookies_file_add() ever on the @p url_con
 *       handler, and call this function to clear any session cookie set by a
 *       previous request on this handler. An easier way to don't use old
 *       session cookies is by using the function
 *       ecore_con_url_cookies_ignore_old_session_set().
 *
 * @see ecore_con_url_cookies_clear()
 * @see ecore_con_url_cookies_ignore_old_session_set()
 */
EAPI void              ecore_con_url_cookies_session_clear(Ecore_Con_Url *url_con);
/**
 * Adds a file to the list of files from which to load cookies.
 *
 * @param url_con   Ecore_Con_Url instance which will be acted upon.
 * @param file_name Name of the file that will be added to the list.
 *
 * Files must contain cookies defined according to two possible formats:
 *
 * @li HTTP-style header ("Set-Cookie: ...").
 * @li <a href="http://www.cookiecentral.com/faq/#3.5">Netscape/Mozilla cookie data format.</a>
 *
 * Cookies will only be @b read from this file. If you want to save cookies to a
 * file, use ecore_con_url_cookies_jar_file_set(). Also notice that this
 * function supports the both types of cookie file cited above, while
 * ecore_con_url_cookies_jar_file_set() will save only in the Netscape/Mozilla's
 * format.
 *
 * Please notice that the file will not be read immediately, but rather added
 * to a list of files that will be loaded and parsed at a later time.
 *
 * @note This function will initialize the cookie engine if it has not been
 *       initialized yet.
 *
 * @see ecore_con_url_cookies_ignore_old_session_set()
 * @see ecore_con_url_cookies_jar_file_set()
 */
EAPI void              ecore_con_url_cookies_file_add(Ecore_Con_Url *url_con,
                                                      const char * const file_name);
/**
 * Sets the name of the file to which all current cookies will be written when
 * either cookies are flushed or Ecore_Con is shut down.
 *
 * @param url_con        Ecore_Con_Url instance which will be acted upon.
 * @param cookiejar_file File to which the cookies will be written.
 *
 * @return @c EINA_TRUE is the file name has been set successfully,
 *         @c EINA_FALSE otherwise.
 *
 * Cookies are written following Netscape/Mozilla's data format, also known as
 * cookie-jar.
 *
 * Cookies will only be @b saved to this file. If you need to read cookies from
 * a file, use ecore_con_url_cookies_file_add() instead.
 *
 * @note This function will initialize the cookie engine if it has not been
 *       initialized yet.
 *
 * @see ecore_con_url_cookies_jar_write()
 */
EAPI Eina_Bool         ecore_con_url_cookies_jar_file_set(Ecore_Con_Url *url_con,
                                                          const char * const cookiejar_file);
/**
 * Writes all current cookies to the cookie jar immediately.
 *
 * @param url_con Ecore_Con_Url instance which will be acted upon.
 *
 * A cookie-jar file must have been previously set by
 * @c ecore_con_url_jar_file_set, otherwise nothing will be done.
 *
 * @note This function will initialize the cookie engine if it has not been
 *       initialized yet.
 *
 * @see ecore_con_url_cookies_jar_file_set()
 */
EAPI void              ecore_con_url_cookies_jar_write(Ecore_Con_Url *url_con);

EAPI void              ecore_con_url_ssl_verify_peer_set(Ecore_Con_Url *url_con,
                                                         Eina_Bool verify);
EAPI int               ecore_con_url_ssl_ca_set(Ecore_Con_Url *url_con,
                                                const char *ca_path);

/**
 * Set HTTP proxy to use.
 *
 * The parameter should be a char * to a zero terminated string holding
 * the host name or dotted IP address. To specify port number in this string,
 * append :[port] to the end of the host name.
 * The proxy string may be prefixed with [protocol]:// since any such prefix
 * will be ignored.
 * The proxy's port number may optionally be specified with the separate option.
 * If not specified, libcurl will default to using port 1080 for proxies.
 *
 * @param url_con Connection object that will use the proxy.
 * @param proxy Porxy string or @c NULL to disable
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 * @since 1.2
 */
EAPI Eina_Bool ecore_con_url_proxy_set(Ecore_Con_Url *url_con, const char *proxy);

/**
 * Set zero terminated username to use for proxy.
 *
 * if socks protocol is used for proxy, protocol should be socks5 and above.
 *
 * @param url_con Connection object that will use the proxy.
 * @param username Username string.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 *
 * @see ecore_con_url_proxy_set()
 *
 * @since 1.2
 */
EAPI Eina_Bool ecore_con_url_proxy_username_set(Ecore_Con_Url *url_con, const char *username);

/**
 * Set zero terminated password to use for proxy.
 *
 * if socks protocol is used for proxy, protocol should be socks5 and above.
 *
 * @param url_con Connection object that will use the proxy.
 * @param password Password string.
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on error.
 *
 * @see ecore_con_url_proxy_set()
 *
 * @since 1.2
 */
EAPI Eina_Bool ecore_con_url_proxy_password_set(Ecore_Con_Url *url_con, const char *password);

/**
 * Set timeout in seconds.
 *
 * the maximum time in seconds that you allow the ecore con url transfer
 * operation to take. Normally, name lookups can take a considerable time
 * and limiting operations to less than a few minutes risk aborting perfectly
 * normal operations.
 *
 * @param url_con Connection object that will use the timeout.
 * @param timeout time in seconds.
 *
 * @see ecore_con_url_cookies_jar_file_set()
 *
 * @since 1.2
 */
EAPI void ecore_con_url_timeout_set(Ecore_Con_Url *url_con, double timeout);

/**
 * Get the returned HTTP STATUS code
 *
 * This is used to, at any time, try to return the status code for a transmission.
 * @param url_con Connection object
 * @return A valid HTTP STATUS code, or 0 on failure
 *
 * @since 1.2
 */
EAPI int ecore_con_url_status_code_get(Ecore_Con_Url *url_con);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
