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
 * @internal
 * @defgroup Ecore_Con_Group Ecore_Con - Connection functions
 * @ingroup Ecore_Group
 *
 * The Ecore Connection Library ( @c Ecore_Con ) provides simple mechanisms
 * for communications between programs using reliable sockets. It saves
 * you from having to worry about file descriptors and waiting
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
 * @internal
 * @defgroup Ecore_Con_Events_Group Ecore Connection Events Functions
 * @ingroup Ecore_Con_Group
 *
 * @li ECORE_CON_CLIENT_ADD: Whenever a client connection is made to an
 * @c Ecore_Con_Server, an event of this type is emitted, allowing the
 * retrieval of the client's IP with @ref ecore_con_client_ip_get and
 * associating data with the client using ecore_con_client_data_set.
 * @li ECORE_CON_EVENT_CLIENT_DEL: Whenever a client connection to an
 * @c Ecore_Con_Server, an event of this type is emitted. The contents of
 * the data with this event are variable, but if the client object in the data
 * is non-null, it must be freed with @ref ecore_con_client_del.
 * @li ECORE_CON_EVENT_SERVER_ADD: Whenever a server object is created
 * with @ref ecore_con_server_connect, an event of this type is emitted,
 * allowing for data to be serialized and sent to the server using
 * @ref ecore_con_server_send. At this point, the HTTP handshake has
 * occurred.
 * @li ECORE_CON_EVENT_SERVER_DEL: Whenever a server object is destroyed,
 * usually by the server connection being refused or dropped, an event of this
 * type is emitted. The contents of the data with this event are variable,
 * but if the server object in the data is non-null, it must be freed
 * with @ref ecore_con_server_del.
 * @li ECORE_CON_EVENT_CLIENT_DATA: Whenever a client connects to your server
 * object and sends data, an event of this type is emitted. The data contains both
 * the size and contents of the message sent by the client. It should be noted that
 * data within this object is transient, so it must be duplicated in order to be
 * retained. This event continues to occur until the client has stopped sending its
 * message, so a good option for storing this data is an Eina_Strbuf. Once the message has
 * been received in full, the client object must be freed with ecore_con_client_free.
 * @li ECORE_CON_EVENT_SERVER_DATA: Whenever your server object connects to its destination
 * and receives data, an event of this type is emitted. The data contains both
 * the size and contents of the message sent by the server. It should be noted that
 * data within this object is transient, so it must be duplicated in order to be
 * retained. This event continues to occur until the server has stopped sending its
 * message, so a good option for storing this data is an Eina_Strbuf. Once the message has
 * been received in full, the server object must be freed with ecore_con_server_free.
 *
 */

/**
 * @internal
 * @defgroup Ecore_Con_Buffer Ecore Connection Buffering
 * @ingroup Ecore_Con_Group
 *
 * As Ecore_Con works on an event driven design, as data arrives, events are
 * produced containing the data that arrived. It is up to the user of
 * Ecore_Con to either parse as they go, append to a file to later parse the
 * whole file in one go, or append to memory to parse or handle leter.
 *
 * To help with this, Eina has some handy API's. The Eina_Binbuf and
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
 *         // Append data as it arrives - do not worry where or how it gets stored.
 *         // Also do not worry about size, expanding, reallocing etc.
 *         // Just keep appending - size is automatically handled.
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
 *    // Get the data back from Eina_Binbuf
 *    char *ptr = eina_binbuf_string_get(data);
 *    size_t size = eina_binbuf_length_get(data);
 *
 *    // Process data as required (write to file)
 *    fprintf(stderr, "Size of data = %d bytes\n", size);
 *    int fd = open("./elm.png", O_CREAT);
 *    write(fd, ptr, size);
 *    close(fd);
 *
 *    // Free it when done.
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
 * @brief The structure type containing a connection handle to a server.
 * @ingroup Ecore_Con_Server_Group
 */
typedef struct _Ecore_Con_Server Ecore_Con_Server;

/**
 * @typedef Ecore_Con_Client
 * @brief The structure type containing a connection handle to a client.
 * @ingroup Ecore_Con_Client_Group
 */
typedef struct _Ecore_Con_Client Ecore_Con_Client;

/**
 * @typedef Ecore_Con_Socks
 * @brief The structure type containing an object representing a SOCKS proxy.
 *
 * @since 1.2
 *
 * @ingroup Ecore_Con_Socks_Group
 */
typedef struct Ecore_Con_Socks Ecore_Con_Socks;

/**
 * @typedef Ecore_Con_Url
 * @brief The structure type containing a handle to an HTTP upload/download object.
 * @ingroup Ecore_Con_Url_Group
 */
typedef struct _Ecore_Con_Url Ecore_Con_Url;


/**
 * @internal
 * @addtogroup Ecore_Con_Events_Group
 *
 * @{
 */

/**
 * @typedef Ecore_Con_Event_Client_Add
 * @brief The structure type used as the @a data param for the corresponding event
 */
typedef struct _Ecore_Con_Event_Client_Add Ecore_Con_Event_Client_Add;

/**
 * @typedef Ecore_Con_Event_Client_Upgrade
 * @brief The structure type used as the @a data param for the corresponding event.
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Client_Upgrade Ecore_Con_Event_Client_Upgrade;

/**
 * @typedef Ecore_Con_Event_Client_Del
 * @brief The structure type containing a connection used as the @a data param for the corresponding event.
 */
typedef struct _Ecore_Con_Event_Client_Del Ecore_Con_Event_Client_Del;

/**
 * @typedef Ecore_Con_Event_Client_Error
 * @brief The structure type used as the @a data param for the corresponding event.
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Client_Error Ecore_Con_Event_Client_Error;

/**
 * @typedef Ecore_Con_Event_Server_Add
 * @brief The structure type used as the @a data param for the corresponding event.
 */
typedef struct _Ecore_Con_Event_Server_Add Ecore_Con_Event_Server_Add;

/**
 * @typedef Ecore_Con_Event_Server_Upgrade
 * @brief The structure type used as the @a data param for the corresponding event.
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Server_Upgrade Ecore_Con_Event_Server_Upgrade;

/**
 * @typedef Ecore_Con_Event_Server_Del
 * @brief The structure type used as the @a data param for the corresponding event.
 */
typedef struct _Ecore_Con_Event_Server_Del Ecore_Con_Event_Server_Del;

/**
 * @typedef Ecore_Con_Event_Server_Error
 * @brief The structure type used as the @a data param for the corresponding event.
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Server_Error Ecore_Con_Event_Server_Error;

/**
 * @typedef Ecore_Con_Event_Client_Data
 * @brief The structure type used as the @a data param for the corresponding event.
 */
typedef struct _Ecore_Con_Event_Client_Data Ecore_Con_Event_Client_Data;

/**
 * @typedef Ecore_Con_Event_Server_Data
 * @brief The structure type used as the @a data param for the corresponding event.
 */
typedef struct _Ecore_Con_Event_Server_Data Ecore_Con_Event_Server_Data;

/**
 * @typedef Ecore_Con_Event_Client_Write
 * @brief The structure type used as the @a data param for the corresponding event.
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Client_Write Ecore_Con_Event_Client_Write;

/**
 * @typedef Ecore_Con_Event_Server_Write
 * @brief The structure type used as the @a data param for the corresponding event.
 * @since 1.1
 */
typedef struct _Ecore_Con_Event_Server_Write Ecore_Con_Event_Server_Write;

/**
 * @typedef Ecore_Con_Event_Proxy_Bind
 * @brief The structure type used as the @a data param for the corresponding event.
 * @since 1.2
 */
typedef struct _Ecore_Con_Event_Proxy_Bind Ecore_Con_Event_Proxy_Bind;

/**
 * @typedef Ecore_Con_Event_Url_Data
 * @brief The structure type used as the @a data param for the corresponding event.
 * @ingroup Ecore_Con_Url_Group
 */
typedef struct _Ecore_Con_Event_Url_Data Ecore_Con_Event_Url_Data;

/**
 * @typedef Ecore_Con_Event_Url_Complete
 * @brief The structure type used as the @a data param for the corresponding event.
 * @ingroup Ecore_Con_Url_Group
 */
typedef struct _Ecore_Con_Event_Url_Complete Ecore_Con_Event_Url_Complete;

/**
 * @typedef Ecore_Con_Event_Url_Progress
 * @brief The structure type used as the @a data param for the corresponding event.
 * @ingroup Ecore_Con_Url_Group
 */
typedef struct _Ecore_Con_Event_Url_Progress Ecore_Con_Event_Url_Progress;

/**
 * @internal
 * @struct _Ecore_Con_Event_Client_Add
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_CLIENT_ADD event.
 */
struct _Ecore_Con_Event_Client_Add
{
   Ecore_Con_Client *client; /** the client that connected */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Client_Upgrade
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_CLIENT_UPGRADE event.
 * @since 1.1
 */
struct _Ecore_Con_Event_Client_Upgrade
{
   Ecore_Con_Client *client; /** The client that completed handshake */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Client_Del
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_CLIENT_DEL event.
 */
struct _Ecore_Con_Event_Client_Del
{
   Ecore_Con_Client *client; /** The client that was lost */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Client_Error
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_CLIENT_ERROR event.
 */
struct _Ecore_Con_Event_Client_Error
{
   Ecore_Con_Client *client; /** The client for which an error occurred */
   char *error; /**< The error string describing what happened */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Server_Add
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_SERVER_ADD event.
 */
struct _Ecore_Con_Event_Server_Add
{
   Ecore_Con_Server *server; /** The server that is connected to */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Server_Upgrade
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_SERVER_UPGRADE event.
 * @since 1.1
 */
struct _Ecore_Con_Event_Server_Upgrade
{
   Ecore_Con_Server *server; /** The server that is connected to */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Server_Del
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_SERVER_DEL event.
 */
struct _Ecore_Con_Event_Server_Del
{
   Ecore_Con_Server *server; /** The client that is lost */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Server_Error
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_SERVER_ERROR event.
 */
struct _Ecore_Con_Event_Server_Error
{
   Ecore_Con_Server *server; /** The server for which an error occurred */
   char *error; /**< The error string describing what happened */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Client_Data
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_CLIENT_DATA event.
 */
struct _Ecore_Con_Event_Client_Data
{
   Ecore_Con_Client *client; /**< The client that connected */
   void *data;               /**< The data that the client sent */
   int size;                 /**< The length of the data sent */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Server_Data
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_SERVER_DATA event.
 */
struct _Ecore_Con_Event_Server_Data
{
   Ecore_Con_Server *server; /**< The server that is connected to */
   void *data;               /**< The data that the server sent */
   int size;                 /**< The length of the data sent */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Client_Write
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_CLIENT_WRITE event.
 */
struct _Ecore_Con_Event_Client_Write
{
   Ecore_Con_Client *client; /**< The client that connected */
   int size;                 /**< The length of the data sent */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Server_Write
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_SERVER_WRITE event.
 */
struct _Ecore_Con_Event_Server_Write
{
   Ecore_Con_Server *server; /**< The server that is connected to */
   int size;                 /**< The length of the data sent */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Proxy_Bind
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_PROXY_BIND event.
 *
 * @since 1.2
 * @ingroup Ecore_Con_Socks_Group
 */
struct _Ecore_Con_Event_Proxy_Bind
{
   Ecore_Con_Server *server; /**< The server object connected to the proxy */
   const char *ip;           /**< The proxy-bound ip address */
   int port;                 /**< The proxy-bound port */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Url_Data
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_URL_DATA event.
 * @ingroup Ecore_Con_Url_Group
 */
struct _Ecore_Con_Event_Url_Data
{
   Ecore_Con_Url *url_con; /**< A pointer to the connection object */
   int size; /**< The size of the current received data (in bytes) */
   unsigned char data[1]; /**< The data received on this event */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Url_Complete
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_URL_COMPLETE event.
 * @ingroup Ecore_Con_Url_Group
 */
struct _Ecore_Con_Event_Url_Complete
{
   Ecore_Con_Url *url_con; /**< A pointer to the connection object */
   int status; /**< HTTP status code of the operation (200, 404, 401, etc.) */
};

/**
 * @internal
 * @struct _Ecore_Con_Event_Url_Progress
 * @brief The structure type used as the @a data param for the @ref ECORE_CON_EVENT_URL_PROGRESS event.
 * @ingroup Ecore_Con_Url_Group
 */
struct _Ecore_Con_Event_Url_Progress
{
   Ecore_Con_Url *url_con; /**< A pointer to the connection object */
   struct
   {
      double total; /**< Total size of the downloading data (in bytes) */
      double now; /**< Current size of the downloading data (in bytes) */
   } down; /**< Download info */
   struct
   {
      double total; /**< Total size of the uploading data (in bytes) */
      double now; /**< Current size of the uploading data (in bytes) */
   } up; /**< Upload info */
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
/** A server is created */
EAPI extern int ECORE_CON_EVENT_SERVER_ADD;
/** A server connection is lost */
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
 * @internal
 * @defgroup Ecore_Con_Lib_Group Ecore Connection Library Functions
 * @ingroup Ecore_Con_Group
 *
 * @brief This group provides utility functions that set up and shut down the Ecore Connection
 *        library.
 *
 * There is also ecore_con_lookup() that can be used to make simple asynchronous
 * DNS lookups.
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
 * @brief Enumeration of the types for an ecore_con client/server object. A correct way to set this type is
 *        with an ECORE_CON_$TYPE, optionally OR'ed with an ECORE_CON_$USE if encryption is desired,
 *        and LOAD_CERT if the previously loaded certificate should be used.
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
   ECORE_CON_NO_PROXY = (1 << 8)
} Ecore_Con_Type;

/**
 * @brief   Initialises the Ecore_Con library.
 *
 * @remarks This function already calls ecore_init() internally. So you do not need
 *          to call it explicitly.
 * @return  The number of times the library has been initialised without being shut down
 */
EAPI int               ecore_con_init(void);

/**
 * @brief   Shuts down the Ecore_Con library.
 *
 * @remarks This function already calls ecore_shutdown() internally. So you do not
 *          need to call it explicitly unless you called ecore_init() explicitly too.
 *
 * @return	The number of times the library has been initialised without being shut down
 */
EAPI int               ecore_con_shutdown(void);

/**
 * @brief   Does an asynchronous DNS lookup.
 *
 * @details This function performs a DNS lookup on the hostname specified by @a name,
 *          then calls @a done_cb with the result and the @a data given as parameter.
 *          The result is given to the @a done_cb as follows:
 *          @li @c canonname - the canonical name of the address
 *          @li @c ip - the resolved IP address
 *          @li @c addr - a pointer to the socket address
 *          @li @c addrlen - the length of the socket address, in bytes
 *          @li @c data - the data pointer given as parameter to ecore_con_lookup()
 *
 * @param[in]   name     The IP address or server name to translate
 * @param[in]   done_cb  The callback to notify when done
 * @param[in]   data     The user data to be given to done_cb
 * @return  @c EINA_TRUE if the request did not fail to be set up, \n
 *          otherwise @c EINA_FALSE if it failed
 */
EAPI Eina_Bool         ecore_con_lookup(const char *name,
                                            Ecore_Con_Dns_Cb done_cb,
                                            const void *data);

/**
 * @}
 */

/**
 * @internal
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
 * @internal
 * @defgroup Ecore_Con_Server_Group Ecore Connection Server Functions
 * @ingroup Ecore_Con_Group
 *
 * @brief This group of functions is applied to an @ref Ecore_Con_Server object. It
 *        does not mean that they should be used in the server application, but on the
 *        server object. In fact, most of them should be used in the client
 *        application, when retrieving information or sending data.
 *
 * Setting up a server is very simple: you just need to start it with
 * ecore_con_server_add() and setup some callbacks to the events
 * @ref ECORE_CON_EVENT_CLIENT_ADD, @ref ECORE_CON_EVENT_CLIENT_DEL and
 * @ref ECORE_CON_EVENT_CLIENT_DATA, that is called when a client is
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
 * connects to a server. The resulting code is very similar to the server
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
 * client code, the server is up and running and the client tries to
 * connect to it. The connection, with its subsequent messages being sent from
 * server to client and client to server, can be represented in the following
 * sequence diagram:
 *
 * @htmlonly
 * <a href="ecore_con-client-server.png">Full size</a>
 * @endhtmlonly
 *
 * @image html ecore_con-client-server.png
 * @image rtf ecore_con-client-server.png
 * @image latex ecore_con-client-server.eps "ecore con client server" width=\textwidth
 *
 * Note the important difference between these two codes: the first is
 * used for writing a @b server, while the second should be used for writing a
 * @b client.
 *
 * A reference for the @c client functions can be found at @ref
 * Ecore_Con_Client_Group.
 *
 * @{
 */

/**
 * @brief   Creates a server to listen for connections.
 *
 * @remarks The socket on which the server listens depends on the connection type:
 *          @li If @a type is @c ECORE_CON_LOCAL_USER, the server listens on
 *              the Unix socket "~/.ecore/[name]/[port]".
 *          @li If @a type is @c ECORE_CON_LOCAL_SYSTEM, the server listens
 *              on Unix socket "/tmp/.ecore_service|[name]|[port]".
 *          @li If @a type is @c ECORE_CON_REMOTE_TCP, the server listens
 *              on TCP port @c port.
 *
 * @remarks More information about the @a type can be found at @ref _Ecore_Con_Type.
 *
 * @remarks The @a data parameter can be fetched later using ecore_con_server_data_get()
 *          or changed with ecore_con_server_data_set().
 *
 * @param[in]   type  The connection type
 * @param[in]   name  The name to associate with the socket \n
 *                It is used when generating the socket name of a Unix socket, or for
 *                determining what host to listen on for TCP sockets.
 *                @c NULL is not accepted.
 * @param[in]   port  The number to identify socket \n
 *                When a Unix socket is used, it becomes part of the socket name.  
 *                When a TCP socket is used, it is used as the TCP port.
 * @param[in]   data  The data to associate with the created Ecore_Con_Server object
 * @return  A new Ecore_Con_Server
 */
EAPI Ecore_Con_Server *ecore_con_server_add(Ecore_Con_Type type,
                                            const char *name, int port,
                                            const void *data);

/**
 * @brief   Creates a connection to the specified server and returns an associated object.
 *
 * @remarks The socket to which the connection is made depends on the connection type:
 *          @li If @a type is @c ECORE_CON_LOCAL_USER, the function
 *              connects to the server at the Unix socket
 *              "~/.ecore/[name]/[port]".
 *          @li If @a type is @c ECORE_CON_LOCAL_SYSTEM, the function
 *              connects to the server at the Unix socket
 *              "/tmp/.ecore_service|[name]|[port]".
 *          @li If @a type is @c ECORE_CON_REMOTE_TCP, the function
 *              connects to the server at the TCP port "[name]:[port]".
 *
 * @remarks More information about the @a type can be found at @ref _Ecore_Con_Type.
 *
 * @remarks This function does not block. It either succeeds, or fails due to invalid
 *          parameters, failed memory allocation, etc., returning @c NULL in that case.
 *
 * @remarks However, even if this call returns a valid @ref Ecore_Con_Server, the
 *          connection is only successfully completed if an event of type
 *          @ref ECORE_CON_EVENT_SERVER_ADD is received. If it fails to complete, an
 *          @ref ECORE_CON_EVENT_SERVER_DEL is received.
 *
 * @remarks The @a data parameter can be fetched later using ecore_con_server_data_get()
 *          or changed with ecore_con_server_data_set().
 *
 * @param[in]   type  The connection type
 * @param[in]   name  The name used when determining what socket to connect to \n
 *                It is used to generate the socket name when the socket
 *                is a Unix socket. It is used as the hostname when
 *                connecting with a TCP socket.
 * @param[in]   port  The number to identify the socket to connect to \n 
 *                It is used when generating the socket name for a Unix socket, 
 *                or as the TCP port when connecting to a TCP socket.
 * @param[in]   data  The data to associate with the created Ecore_Con_Server object
 * @return  A new Ecore_Con_Server
 */
EAPI Ecore_Con_Server *ecore_con_server_connect(Ecore_Con_Type type,
                                                const char *name, int port,
                                                const void *data);
/**
 * @brief   Closes the connection and frees the given server.
 *
 * @remarks All the clients connected to this server is disconnected.
 *
 * @param[in]   svr  The given server
 * @return  The data associated with the server when it is created
 *
 * @see ecore_con_server_add, ecore_con_server_connect
 */
EAPI void *            ecore_con_server_del(Ecore_Con_Server *svr);

/**
 * @brief   Gets the data associated with the given server.
 *
 * @param[in]   svr  The given server
 * @return  The associated data
 *
 * @see ecore_con_server_data_set()
 */
EAPI void *            ecore_con_server_data_get(Ecore_Con_Server *svr);
/**
 * @brief   Sets the data associated with the given server.
 *
 * @param[in]   svr   The given server
 * @param[in]   data  The data to associate with @a svr
 * @return  The previously associated data, if any
 *
 * @see ecore_con_server_data_get()
 */
EAPI void *            ecore_con_server_data_set(Ecore_Con_Server *svr,
                                                 void *data);
/**
 * @brief   Checks whether the given server is currently connected.
 *
 * @param[in]   svr  The given server
 * @return  @c EINA_TRUE if the server is connected, \n
 *          otherwise @c EINA_FALSE if the server is not connected
 */
EAPI Eina_Bool         ecore_con_server_connected_get(Ecore_Con_Server *svr);
/**
 * @brief   Gets the current list of clients.
 *
 * @remarks Each node in the returned list points to an @ref Ecore_Con_Client. This list
 *          cannot be modified or freed. It can also change if new clients are connected
 *          or disconnected, and becomes invalid when the server is deleted or freed.
 *
 * @param[in]   svr  The given server
 * @return  The list of clients on this server
 */
EAPI const Eina_List * ecore_con_server_clients_get(Ecore_Con_Server *svr);

/**
 * @brief   Gets the name of server.
 *
 * @remarks The name returned is the name used to connect on this server.
 *
 * @param[in]   svr  The given server
 * @return  The name of the server
 */
EAPI const char *      ecore_con_server_name_get(Ecore_Con_Server *svr);

/**
 * @brief   Gets the server port in use.
 *
 * @remarks The port where the server is listening for connections.
 *
 * @param[in]   svr  The given server
 * @return  The server port in use
 */
EAPI int               ecore_con_server_port_get(Ecore_Con_Server *svr);
/**
 * @brief   Checks how long a server has been connected.
 *
 * @details This function is used to find out the time that has been elapsed since
 *          ecore_con_server_add() succeeded.
 *
 * @param[in]   svr  The server to check
 * @return  The total time, in seconds, that the server has been connected or running
 */
EAPI double            ecore_con_server_uptime_get(Ecore_Con_Server *svr);
/**
 * @brief   Sends the given data to the given server.
 *
 * @details This function sends the given data to the server as soon as the program
 *          is back to the main loop. Thus, this function returns immediately
 *          (non-blocking). If the data needs to be sent @b now, call
 *          ecore_con_server_flush() after this one.
 *
 * @param[in]   svr   The given server
 * @param[in]   data  The given data
 * @param[in]   size  The length of the data, in bytes, to send
 * @return  The number of bytes sent, \n  
 *          otherwise @c 0 if there is an error
 *
 * @see ecore_con_client_send()
 * @see ecore_con_server_flush()
 */
EAPI int               ecore_con_server_send(Ecore_Con_Server *svr,
                                             const void *data,
                                             int size);
/**
 * @brief   Sets a limit on the number of clients that can be handled concurrently
 *          by the given server, and a policy on what to do if excess clients try to
 *          connect.
 *
 * @remarks Beware that if you set this once, ecore is already running. You may
 *          already have pending CLIENT_ADD events in your event queue. Those
 *          clients have already connected and is not affected by this call.
 *          Only clients subsequently trying to connect is affected.
 *
 * @param[in]   svr                    The given server
 * @param[in]   client_limit           The maximum number of clients to handle concurrently \n 
 *                                 @c -1 means unlimited (default), @c 0 effectively disables the server.
 * @param[in]   reject_excess_clients  Set to @c 1 to automatically disconnect excess clients 
 *                                 as soon as they connect if you are already handling client_limit clients \n  
 *                                 otherwise set to @c 0 (default) to just hold off on the "accept()"
 *                                 system call until the number of active clients drops \n 
 *                                 This causes the kernel to queue up to 4096
 *                                 connections (or your kernel's limit, whichever is lower).
 */
EAPI void              ecore_con_server_client_limit_set(Ecore_Con_Server *svr,
                                                         int client_limit,
                                                         char reject_excess_clients);
/**
 * @brief   Gets the IP address of a server that has been connected to.
 *
 * @param[in]   svr  The given server
 * @return  A pointer to an internal string that contains the IP address of
 *          the connected server in the form "XXX.YYY.ZZZ.AAA" IP notation \n
 *          This string should not be modified or trusted to stay valid after
 *          deletion for the @a svr object. If no IP is known, @c NULL is returned.
 */
EAPI const char *      ecore_con_server_ip_get(Ecore_Con_Server *svr);
/**
 * @brief   Flushes all pending data to the given server.
 *
 * @details This function blocks until all data is sent to the server.
 *
 * @param[in]   svr  The given server
 *
 * @see ecore_con_server_send()
 * @see ecore_con_client_flush()
 */
EAPI void              ecore_con_server_flush(Ecore_Con_Server *svr);
/**
 * @brief   Sets the default time after which an inactive client is disconnected.
 *
 * @details This function is used by the server to set the default idle timeout on
 *          clients. If the any of the clients becomes idle for a time higher than this
 *          value, it is disconnected. A value of < @c 1 disables the idle timeout.
 *
 * @remarks This timeout is not affected by the one set by
 *          ecore_con_client_timeout_set(). A client is disconnected whenever the
 *          client or the server timeout is reached. That means, the lower timeout value
 *          is used for that client if ecore_con_client_timeout_set() is used on it.
 *
 * @param[in]   svr      The server object
 * @param[in]   timeout  The timeout, in seconds, to disconnect after
 *
 * @see ecore_con_server_timeout_get()
 * @see ecore_con_client_timeout_set()
 */
EAPI void              ecore_con_server_timeout_set(Ecore_Con_Server *svr, double timeout);
/**
 * @brief   Gets the default time after which an inactive client is disconnected.
 *
 * @details This function is used to get the idle timeout for clients. A value of < @c 1
 *          means the idle timeout is disabled.
 *
 * @param[in]   svr  The server object
 * @return  The timeout, in seconds, to disconnect after
 *
 * @see ecore_con_server_timeout_set()
 * @see ecore_con_client_timeout_get()
 */
EAPI double            ecore_con_server_timeout_get(Ecore_Con_Server *svr);

/**
 * @brief   Get the fd that the server is connected to.
 *
 * @details This function returns the fd which is used by the underlying server connection.
 *          It should not be tampered with unless you REALLY know what you are doing.
 * @since   1.1
 *
 * @remarks This function is only valid for servers created with ecore_con_server_connect()
 * @remarks Do not use this unless you know what you are doing.
 *
 * @param[in]   svr  The server object
 * @return  The fd, \n
 *          otherwise @c -1 on failure
 */
EAPI int               ecore_con_server_fd_get(Ecore_Con_Server *svr);

/**
 * @brief   Gets the fd that the client is connected to.
 *
 * @details This function returns the fd which is used by the underlying client connection.
 *          It should not be tampered with unless you REALLY know what you are doing.
 * 
 * @since   1.1
 *
 * @param[in]   cl  The client object
 * @return  The fd, \n 
 *          otherwise @c -1 on failure
 */
EAPI int               ecore_con_client_fd_get(Ecore_Con_Client *cl);
/**
 * @}
 */

/**
 * @internal
 * @defgroup Ecore_Con_Client_Group Ecore Connection Client Functions
 * @ingroup Ecore_Con_Group
 *
 * @brief This group provides functions to communicate with and/or set options on a client.
 *
 * This set of functions, as explained in @ref Ecore_Con_Server_Group, is used
 * to send data to a client, or to set options and get information about this
 * client. Most of them should be used on the server, applied on the client
 * object.
 *
 * If you need to implement a client, the way to connect to a server is
 * described in @ref Ecore_Con_Server_Group.
 *
 * @{
 */

/**
 * @brief   Sends the given data to the given client.
 *
 * @details This function sends the given data to the client as soon as the program
 *          is back to the main loop. Thus, this function returns immediately
 *          (non-blocking). If the data needs to be sent @b now, call
 *          ecore_con_client_flush() after this one.
 *
 * @param[in]   cl    The given client
 * @param[in]   data  The given data
 * @param[in]   size  The length of the data, in bytes, to send
 * @return  The number of bytes sent, \n 
 *          otherwise @c 0 if there is an error
 *
 * @see ecore_con_server_send()
 * @see ecore_con_client_flush()
 */
EAPI int               ecore_con_client_send(Ecore_Con_Client *cl,
                                             const void *data,
                                             int size);
/**
 * @brief   Gets the server representing the socket the client has connected to.
 *
 * @param[in]   cl  The given client
 * @return  The server that the client connected to
 */
EAPI Ecore_Con_Server *ecore_con_client_server_get(Ecore_Con_Client *cl);

/**
 * @brief   Closes the connection and frees the memory allocated to the given client.
 *
 * @param[in]   cl  The given client
 * @return  The data associated with the client
 */
EAPI void *            ecore_con_client_del(Ecore_Con_Client *cl);

/**
 * @brief   Sets the data associated with the given client to @a data.
 *
 * @param[in]   cl    The given client
 * @param[in]   data  The data to set
 */
EAPI void              ecore_con_client_data_set(Ecore_Con_Client *cl,
                                                 const void       *data);
/**
 * @brief   Gets the data associated with the given client.
 *
 * @param[in]   cl  The given client
 * @return  The data associated with @a cl
 */
EAPI void *            ecore_con_client_data_get(Ecore_Con_Client *cl);

/**
 * @brief   Gets the IP address of a client that has connected.
 *
 * @remarks The returned string should not be modified, freed or trusted to stay valid
 *          after deletion for the @a cl object. If no IP is known @c NULL is returned.
 *
 * @param[in]   cl  The given client
 * @return  A pointer to an internal string that contains the IP address of
 *          the connected client in the form "XXX.YYY.ZZZ.AAA" IP notation
 */
EAPI const char *      ecore_con_client_ip_get(Ecore_Con_Client *cl);

/**
 * @brief   Flushes all pending data to the given client.
 *
 * @details This function blocks until all data is sent to the server.
 *
 * @param[in]   cl  The given client
 *
 * @see ecore_con_client_send()
 * @see ecore_con_server_flush()
 */
EAPI void              ecore_con_client_flush(Ecore_Con_Client *cl);

/**
 * @brief   Checks how long a client has been connected.
 *
 * @details This function is used to find out how long a client has been connected for.
 *
 * @param[in]   cl  The client to check
 * @return  The total time, in seconds, that the client has been connected to the server
 */
EAPI double            ecore_con_client_uptime_get(Ecore_Con_Client *cl);

/**
 * @brief   Gets the default time after which the client is disconnected when inactive.
 *
 * @details This function is used to get the idle timeout for a client.  A value of < @c 1
 *          means the idle timeout is disabled.
 *
 * @param[in]   cl  The client object
 * @return  The timeout, in seconds, to disconnect after
 *
 * @see ecore_con_client_timeout_set()
 */
EAPI double            ecore_con_client_timeout_get(Ecore_Con_Client *cl);

/**
 * @brief   Sets the time after which the client is disconnected when inactive.
 *
 * @details This function is used by the server to set the idle timeout on a specific
 *          client. If the client becomes idle for a time higher than this value, it is
 *          disconnected. A value of < @c 1 disables the idle timeout.
 *
 * @details This timeout is not affected by the one set by
 *          ecore_con_server_timeout_set(). A client is disconnected whenever the
 *          client or the server timeout is reached. That means, the lower timeout value
 *          is used for that client if ecore_con_server_timeout_set() is used on the server.
 *
 * @param[in]   cl       The client object
 * @param[in]   timeout  The timeout, in seconds, to disconnect after
 *
 * @see ecore_con_client_timeout_get()
 * @see ecore_con_server_timeout_set()
 */
EAPI void              ecore_con_client_timeout_set(Ecore_Con_Client *cl, double timeout);

/**
 * @brief   Checks whether the client is still connected.
 *
 * @param[in]   cl  The given client
 * @return  @c EINA_TRUE if connected, \n
 *          otherwise @c EINA_FALSE if it is not connected
 */
EAPI Eina_Bool         ecore_con_client_connected_get(Ecore_Con_Client *cl);
/**
 * @brief   Gets the port that the client has connected to.
 *
 * @remarks Use this function to return the port on which a given client has connected.
 *
 * @param[in]   cl  The client
 * @return  The port that @a cl has connected to, \n
 *          otherwise @c -1 on error
 */
EAPI int               ecore_con_client_port_get(Ecore_Con_Client *cl);

/**
 * @}
 */

/**
 * @internal
 * @defgroup Ecore_Con_Url_Group Ecore URL Connection Functions
 * @ingroup Ecore_Con_Group
 *
 * @brief This group provides utility functions that set up, use and shut down the Ecore URL
 *        Connection library.
 *
 * These functions are shortcuts to make it easy to perform HTTP requests
 * (POST, GET, etc).
 *
 * Brief usage details:
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
 * If it is necessary to change the @ref Ecore_Con_Url object url, use
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
 * @{
 */

/**
 * @typedef Ecore_Con_Url_Time
 * @enum _Ecore_Con_Url_Time
 * @brief Enumeration for the type of condition to use when making an HTTP request dependent on time,
 *        so that headers such as "If-Modified-Since" are used.
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
 * @brief Enumeration of the HTTP version to use.
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
 * @brief   Changes the HTTP version used for the request.
 * @since   1.2
 *
 * @param[in]   url_con  The connection object through which the request is sent
 * @param[in]   version  The version to be used
 * @return  @c EINA_TRUE if the version is changed successfully, \n
 *          otherwise @c EINA_FALSE on failure to change version
 * @see ecore_con_url_pipeline_get()
 */
EAPI Eina_Bool         ecore_con_url_http_version_set(Ecore_Con_Url *url_con, Ecore_Con_Url_Http_Version version);

/**
 * @brief   Initialises the Ecore_Con_Url library.
 *
 * @remarks This function does not call ecore_con_init(). You still need to call it
 *          explicitly before calling this one.
 * @return  The number of times the library has been initialised without being shut down
 */
EAPI int               ecore_con_url_init(void);

/**
 * @brief   Shuts down the Ecore_Con_Url library.
 *
 * @remarks This function does not call ecore_con_shutdown(). You still need to call
 *          it explicitly after calling this one.
 * @return  The number of calls that still uses Ecore_Con_Url
 */
EAPI int               ecore_con_url_shutdown(void);

/**
 * @brief   Enables or disables HTTP 1.1 pipelining.
 *
 * @remarks Pipelining allows to send one request after another one, without having to
 *          wait for the reply of the first request. The respective replies are received
 *          in the order that the requests were sent.
 *
 * @remarks Enabling this feature is valid for all requests done using @c ecore_con_url.
 *
 * @remarks See http://en.wikipedia.org/wiki/HTTP_pipelining for more info.
 *
 * @param[in]   enable  Set @c EINA_TRUE to turn it on, \n
 *                  otherwise @c EINA_FALSE to disable it
 * @see ecore_con_url_pipeline_get()
 */
EAPI void              ecore_con_url_pipeline_set(Eina_Bool enable);
/**
 * @brief   Checks whether HTTP 1.1 pipelining is enabled.
 *
 * @return  @c EINA_TRUE if it is enabled, \n
 *          otherwise @c EINA_FALSE if it is not enabled
 *
 * @see ecore_con_url_pipeline_set()
 */
EAPI Eina_Bool         ecore_con_url_pipeline_get(void);

/**
 * @brief   Creates and initializes a new Ecore_Con_Url connection object.
 *
 * @details This function creates and initializes a new Ecore_Con_Url connection object that can be
 *          used for sending requests.
 *
 * @param[in]   url  The URL that receives requests \n 
 *               This can be changed using ecore_con_url_url_set.
 * @return  A new Ecore_Con_Url, \n
 *          otherwise @c NULL on error
 *
 * @see ecore_con_url_custom_new()
 * @see ecore_con_url_url_set()
 */
EAPI Ecore_Con_Url *   ecore_con_url_new(const char *url);
/**
 * @brief   Creates a custom connection object.
 *
 * @details This function creates and initializes a new Ecore_Con_Url for a custom request (e.g. HEAD,
 *          SUBSCRIBE and other obscure HTTP requests). This object should be used like
 *          the one created with ecore_con_url_new().
 *
 * @param[in]   url             The URL that receives requests
 * @param[in]   custom_request  The custom request (e.g. GET, POST, HEAD, PUT, etc)
 * @return  A new Ecore_Con_Url, \n
 *          otherwise @c NULL on error
 *
 * @see ecore_con_url_new()
 * @see ecore_con_url_url_set()
 */
EAPI Ecore_Con_Url *   ecore_con_url_custom_new(const char *url,
                                                const char *custom_request);
/**
 * @brief  Destroys an Ecore_Con_Url connection object.
 *
 * @param[in]  url_con  The connection object to free
 *
 * @see ecore_con_url_new()
 */
EAPI void              ecore_con_url_free(Ecore_Con_Url *url_con);

/**
 * @brief   Sets the URL to send the request to.
 *
 * @param[in]   url_con  The connection object through which the request is sent
 * @param[in]   url      The URL that receives the request
 *
 * @return  @c EINA_TRUE if the URL is set successfully, \n
 *          otherwise @c EINA_FALSE on error
 */
EAPI Eina_Bool         ecore_con_url_url_set(Ecore_Con_Url *url_con,
                                             const char *url);
/**
 * @brief   Gets the URL to send the request to.
 * @since   1.1
 *
 * @param[in]   url_con  The connection object through which the request is sent
 * @return  The URL that receives the request, \n
 *          otherwise @c NULL on failure \n 
 *          URL is stringshared.
 */
EAPI const char       *ecore_con_url_url_get(Ecore_Con_Url *url_con);

/**
 * @brief   Associates data with a connection object.
 *
 * @remarks Associates data with a connection object, which can be retrieved later with
 *          ecore_con_url_data_get()).
 *
 * @param[in]   url_con  The connection object to associate data
 * @param[in]   data     The data to be set
 *
 * @see ecore_con_url_data_get()
 */
EAPI void              ecore_con_url_data_set(Ecore_Con_Url *url_con,
                                              void *data);
/**
 * @brief   Gets data associated with a Ecore_Con_Url connection object.
 *
 * @details This function gets data associated with a Ecore_Con_Url connection object (previously
 *          set with ecore_con_url_data_set()).
 *
 * @param[in]   url_con  The connection object to retrieve data from
 * @return  The data associated with the given object
 *
 * @see ecore_con_url_data_set()
 */
EAPI void *            ecore_con_url_data_get(Ecore_Con_Url *url_con);

/**
 * @brief   Adds an additional header to the request connection object.
 *
 * @details This function adds an additional header (User-Agent, Content-Type, etc.) to the request
 *          connection object. This addition is valid for only one
 *          ecore_con_url_get() or ecore_con_url_post() call.
 *
 * @remarks Some functions like ecore_con_url_time() also add headers to the request.
 *
 * @param[in]   url_con  The connection object
 * @param[in]   key      The header key
 * @param[in]   value    The header value
 *
 * @see ecore_con_url_get()
 * @see ecore_con_url_post()
 * @see ecore_con_url_additional_headers_clear()
 */
EAPI void              ecore_con_url_additional_header_add(Ecore_Con_Url *url_con,
                                                           const char *key,
                                                           const char *value);

/**
 * @brief   Cleans additional headers.
 *
 * @details This function cleans additional headers associated with a connection object (previously
 *          added with ecore_con_url_additional_header_add()).
 *
 * @param[in]   url_con  The connection object to clean additional headers
 *
 * @see ecore_con_url_additional_header_add()
 * @see ecore_con_url_get()
 * @see ecore_con_url_post()
 */
EAPI void              ecore_con_url_additional_headers_clear(Ecore_Con_Url *url_con);

/**
 * @brief   Gets headers from last request sent.
 *
 * @details This function retrieves a list containing the response headers. This function should be
 *          used after an ECORE_CON_EVENT_URL_COMPLETE event (headers should normally be
 *          ready at that time).
 *
 * @param[in]   url_con  The connection object to retrieve response headers from
 * @return  The list of response headers \n 
 *          This list must not be modified by the user.
 */
EAPI const Eina_List * ecore_con_url_response_headers_get(Ecore_Con_Url *url_con);

/**
 * @brief   Sets up a file for receiving response data.
 *
 * @details This function sets up a file to have response data written into. Note that
 *          ECORE_CON_EVENT_URL_DATA events are not emitted if a file has been set to
 *          receive the response data.
 *
 * @remarks This function can be used to easily setup a file where the downloaded data
 *          is saved.
 *
 * @param[in]   url_con  The connection object to set file
 * @param[in]   fd       The file descriptor associated with the file \n 
 *                   A negative value unsets any previously set @a fd.
 */
EAPI void              ecore_con_url_fd_set(Ecore_Con_Url *url_con, int fd);

/**
 * @brief   Gets the number of bytes received.
 *
 * @details This function retrieves the number of bytes received on the last request of the given
 *          connection object.
 *
 * @param[in]   url_con	The connection object which the request is sent on
 * @return  The number of bytes received on request
 *
 * @see ecore_con_url_get()
 * @see ecore_con_url_post()
 */
EAPI int               ecore_con_url_received_bytes_get(Ecore_Con_Url *url_con);

/**
 * @brief   Sets @a url_con to use http auth, with the given @a username and @a password, "safely" or not.
 *
 * @remarks This function requires libcurl >= 7.19.1 to work. Otherwise it always returns @c 0.
 *
 * @param[in]   url_con   The connection object to perform a request on, previously created
 *                    with ecore_con_url_new() or ecore_con_url_custom_new().
 * @param[in]   username  The username to use in authentication
 * @param[in]   password  The password to use in authentication
 * @param[in]   safe      Set @c EINA_TRUE to use "safer" methods (eg, NOT http basic auth), \n
 *                    otherwise set @c EINA_FALSE to not use it
 * @return  @c EINA_TRUE if it is set successfully, 
 *          otherwise @c EINA_FALSE on error
 */
EAPI Eina_Bool         ecore_con_url_httpauth_set(Ecore_Con_Url *url_con,
                                                  const char *username,
                                                  const char *password,
                                                  Eina_Bool safe);
/**
 * @brief   Sends a get request.
 *
 * @remarks The request is performed immediately, but you need to setup event handlers
 *          for #ECORE_CON_EVENT_URL_DATA, #ECORE_CON_EVENT_URL_COMPLETE or
 *          #ECORE_CON_EVENT_URL_PROGRESS to get more information about its result.
 *
 * @param[in]   url_con  The connection object to perform a request on, previously created
 * @return  @c EINA_TRUE if the request is sent successfully, \n 
 *          otherwise @c EINA_FALSE on error
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
 * @brief   Sends a post request.
 *
 * @remarks The request starts immediately, but you need to setup event handlers
 *          for #ECORE_CON_EVENT_URL_DATA, #ECORE_CON_EVENT_URL_COMPLETE or
 *          #ECORE_CON_EVENT_URL_PROGRESS to get more information about its result.
 *
 * @remarks This call does not block your main loop.
 *
 * @param[in]   url_con       The connection object to perform a request on, previously created
 *                        with ecore_con_url_new() or ecore_con_url_custom_new().
 * @param[in]   data          The payload (data sent on the request) \n 
 *                        This can be @c NULL.
 * @param[in]   length        The payload length \n 
 *                        If this is @c -1, it relies on automatic length 
 *                        calculation via @c strlen() on @a data.
 * @param[in]   content_type  The content type of the payload (e.g. text/xml) \n 
 *                        This can be @c NULL.
 * @return  @c EINA_TRUE if the request is sent successfully, 
 *          otherwise @c EINA_FALSE on error
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
 * @brief   Sets whether HTTP requests should be conditional, dependent on
 *          modification time.
 *
 * @details This function may set the header "If-Modified-Since" or
 *          "If-Unmodified-Since", depending on the value of @a time_condition, with the
 *          value @a timestamp.
 *
 * @param[in]   url_con         The Ecore_Con_Url to act upon
 * @param[in]   time_condition  The condition to use for HTTP requests
 * @param[in]   timestamp       The time since 1 Jan 1970 to use in the condition
 *
 * @see ecore_con_url_get()
 * @see ecore_con_url_post()
 */
EAPI void              ecore_con_url_time(Ecore_Con_Url *url_con,
                                          Ecore_Con_Url_Time time_condition,
                                          double timestamp);

/**
 * @brief   Uploads a file to an FTP site.
 *
 * @remarks Upload @a filename to an FTP server set in @a url_con using @a user
 *          and @a pass to directory @a upload_dir
 *
 * @param[in]   url_con     The Ecore_Con_Url object to send with
 * @param[in]   filename    The path to the file to send
 * @param[in]   user        The username to log in with
 * @param[in]   pass        The password to log in with
 * @param[in]   upload_dir  The directory to which the file should be uploaded
 * @return  @c EINA_TRUE if the file is uploaded successfully, \n 
 *          otherwise @c EINA_FALSE on error
 */
EAPI Eina_Bool         ecore_con_url_ftp_upload(Ecore_Con_Url *url_con,
                                                const char *filename,
                                                const char *user,
                                                const char *pass,
                                                const char *upload_dir);
/**
 * @brief   Toggles libcurl's verbose output.
 *
 * @remarks If @a verbose is @c EINA_TRUE, libcurl outputs a lot of verbose
 *          information about its operations, which is useful for
 *          debugging. The verbose information is sent to stderr.
 *
 * @param[in]   url_con  The Ecore_Con_Url instance which is acted upon
 * @param[in]   verbose  Set @c EINA_TRUE to enable libcurl's verbose output, \n
 *                   otherwise @c EINA_FALSE to disable verbose output
 */
EAPI void              ecore_con_url_verbose_set(Ecore_Con_Url *url_con,
                                                 Eina_Bool verbose);

/**
 * @brief   Enables or disables EPSV extension.
 *
 * @param[in]   url_con   The Ecore_Con_Url instance which is acted upon
 * @param[in]   use_epsv  Set @c EINA_TRUE to enable the EPSV extension, \n
 *                    otherwise @c EINA_FALSE to disable it
 */
EAPI void              ecore_con_url_ftp_use_epsv_set(Ecore_Con_Url *url_con,
                                                      Eina_Bool use_epsv);

/**
 * @brief   Enables the cookie engine for subsequent HTTP requests.
 *
 * @remarks After this function is called, cookies set by the server in HTTP responses
 *          are parsed and stored, as well as sent back to the server in new HTTP requests.
 *
 * @remarks Even though this function is called @c ecore_con_url_cookies_init(),
 *          there is no symmetrical shutdown operation.
 *
 * @param[in]   url_con  The Ecore_Con_Url instance which is acted upon
 */
EAPI void              ecore_con_url_cookies_init(Ecore_Con_Url *url_con);

/**
 * @brief   Sets whether session cookies from previous sessions shall be loaded.
 *
 * @remarks Session cookies are cookies with no expire date set, which usually means
 *          they are removed after the current session is closed.
 *
 * @remarks By default, when Ecore_Con_Url loads cookies from a file, all cookies are
 *          loaded, including session cookies, which, most of the time, were supposed
 *          to be loaded and valid only for that session.
 *
 * @remarks If @a ignore is set to @c EINA_TRUE, when Ecore_Con_Url loads cookies from
 *          the files passed to @c ecore_con_url_cookies_file_add(), session cookies
 *          are not loaded.
 *
 * @param[in]   url_con  The Ecore_Con_Url instance which is acted upon
 * @param[in]   ignore   Set @c EINA_TRUE to ignore session cookies when loading cookies from files, \n 
 *                   otherwise set @c EINA_FALSE to load all cookies
 *
 * @see ecore_con_url_cookies_file_add()
 */
EAPI void              ecore_con_url_cookies_ignore_old_session_set(Ecore_Con_Url *url_con,
                                                                    Eina_Bool ignore);

/**
 * @brief   Clears currently loaded cookies.
 *
 * @remarks The cleared cookies are removed and not sent in subsequent HTTP
 *          requests, nor are they written to the cookiejar file set using
 *          @c ecore_con_url_cookies_jar_file_set().
 *
 * @remarks This function initializes the cookie engine if it has not been
 *          initialized yet.
 * @remarks The cookie files set by ecore_con_url_cookies_file_add() are not loaded
 *          immediately, just when the request is started. Thus, if you ask to
 *          clear the cookies, but has a file already set by that function, the
 *          cookies are then loaded and you have old cookies set. In order
 *          to not have any old cookie set, you need to not call
 *          ecore_con_url_cookies_file_add() ever on the @a url_con handler, and
 *          call this function to clear any cookie set by a previous request on
 *          this handler.
 *
 * @param[in]   url_con  The Ecore_Con_Url instance which are acted upon
 *
 * @see ecore_con_url_cookies_session_clear()
 * @see ecore_con_url_cookies_ignore_old_session_set()
 */
EAPI void              ecore_con_url_cookies_clear(Ecore_Con_Url *url_con);

/**
 * @brief   Clears currently loaded session cookies.
 *
 * @remarks Session cookies are cookies with no expire date set, which usually means
 *          they are removed after the current session is closed.
 *
 * @remarks The cleared cookies are removed and not sent in subsequent HTTP
 *          requests, nor are they be written to the cookiejar file set using
 *          @c ecore_con_url_cookies_jar_file_set().
 *
 * @remarks This function initializes the cookie engine if it has not been
 *          initialized yet.
 * @remarks The cookie files set by ecore_con_url_cookies_file_add() are not loaded
 *          immediately, just when the request is started. Thus, if you ask to
 *          clear the session cookies, but has a file already set by that function,
 *          the session cookies are then loaded and you have old cookies
 *          set. In order to not have any old session cookie set, you need to
 *          not call ecore_con_url_cookies_file_add() ever on the @a url_con
 *          handler, and call this function to clear any session cookie set by a
 *          previous request on this handler. An easier way to not use old
 *          session cookies is by using the function
 *          ecore_con_url_cookies_ignore_old_session_set().
 *
 * @param[in]   url_con  The Ecore_Con_Url instance which are acted upon
 *
 * @see ecore_con_url_cookies_clear()
 * @see ecore_con_url_cookies_ignore_old_session_set()
 */
EAPI void              ecore_con_url_cookies_session_clear(Ecore_Con_Url *url_con);

/**
 * @brief   Adds a file to the list of files from which to load cookies.
 *
 * @remarks The files must contain cookies defined according to two possible formats:
 *          @li HTTP-style header ("Set-Cookie: ...").
 *          @li <a href="http://www.cookiecentral.com/faq/#3.5">Netscape/Mozilla cookie data format.</a>
 *
 * @remarks Cookies are only @b read from this file. If you want to save cookies to a
 *          file, use ecore_con_url_cookies_jar_file_set(). Also notice that this
 *          function supports the both types of cookie file cited above, while
 *          ecore_con_url_cookies_jar_file_set() saves only in the Netscape/Mozilla's format.
 *
 * @remarks Please notice that the file is not read immediately, but rather added
 *          to a list of files that is loaded and parsed at a later time.
 *
 * @remarks This function initializes the cookie engine if it has not been
 *          initialized yet.
 *
 * @param[in]   url_con    The Ecore_Con_Url instance which is acted upon
 * @param[in]   file_name  The name of the file that is added to the list
 *
 * @see ecore_con_url_cookies_ignore_old_session_set()
 * @see ecore_con_url_cookies_jar_file_set()
 */
EAPI void              ecore_con_url_cookies_file_add(Ecore_Con_Url *url_con,
                                                      const char * const file_name);

/**
 * @brief   Sets the name of the file to which all current cookies are written when
 *          either cookies are flushed or Ecore_Con is shut down.
 *
 * @remarks Cookies are written following Netscape/Mozilla's data format, also known as
 *          cookie-jar.
 *
 * @remarks Cookies are only @b saved to this file. If you need to read cookies from
 *          a file, use ecore_con_url_cookies_file_add() instead.
 *
 * @remarks This function initializes the cookie engine if it has not been initialized yet.
 *
 * @param[in]   url_con         The Ecore_Con_Url instance which are acted upon
 * @param[in]   cookiejar_file  The file to which the cookies are written
 * @return  @c EINA_TRUE is the file name has been set successfully, \n
 *          otherwise @c EINA_FALSE on failure
 *
 * @see ecore_con_url_cookies_jar_write()
 */
EAPI Eina_Bool         ecore_con_url_cookies_jar_file_set(Ecore_Con_Url *url_con,
                                                          const char * const cookiejar_file);

/**
 * @brief   Writes all current cookies to the cookie jar immediately.
 *
 * @remarks A cookie-jar file must have been previously set by
 *          ecore_con_url_jar_file_set(), otherwise nothing is done.
 *
 * @remarks This function initializes the cookie engine if it has not been initialized yet.
 *
 * @param[in]   url_con  The Ecore_Con_Url instance which is acted upon
 *
 * @see ecore_con_url_cookies_jar_file_set()
 */
EAPI void              ecore_con_url_cookies_jar_write(Ecore_Con_Url *url_con);

EAPI void              ecore_con_url_ssl_verify_peer_set(Ecore_Con_Url *url_con,
                                                         Eina_Bool verify);
EAPI int               ecore_con_url_ssl_ca_set(Ecore_Con_Url *url_con,
                                                const char *ca_path);

/**
 * @brief   Sets the HTTP proxy to use.
 * @since   1.2
 *
 * @remarks The parameter should be a char * to a zero terminated string holding
 *          the host name or dotted IP address. To specify port number in this string,
 *          append :[port] to the end of the host name.
 *          The proxy string may be prefixed with [protocol]:// since any such prefix
 *          is ignored.
 *          The proxy's port number may optionally be specified with the separate option.
 *          If not specified, libcurl defaults to using port 1080 for proxies.
 *
 * @param[in]   url_con  The connection object that uses the proxy
 * @param[in]   proxy    The proxy string, \n
 *                   otherwise set @c NULL to disable
 * @return  @c EINA_TRUE if the proxy is set successfully, 
 *          otherwise @c EINA_FALSE on error
 */
EAPI Eina_Bool ecore_con_url_proxy_set(Ecore_Con_Url *url_con, const char *proxy);

/**
 * @brief   Sets zero terminated username to use for proxy.
 * @since   1.2
 *
 * @remarks If socks protocol is used for proxy, protocol should be socks5 and above.
 *
 * @param[in]   url_con   The connection object that uses the proxy
 * @param[in]   username  The username string
 *
 * @return  @c EINA_TRUE if the username is set successfully, \n
 *          otherwise @c EINA_FALSE on error
 *
 * @see ecore_con_url_proxy_set()
 *
 */
EAPI Eina_Bool ecore_con_url_proxy_username_set(Ecore_Con_Url *url_con, const char *username);

/**
 * @brief   Sets zero terminated password to use for proxy.
 * @since   1.2
 *
 * @remarks If socks protocol is used for proxy, protocol should be socks5 and above.
 *
 * @param[in]   url_con   The connection object that uses the proxy
 * @param[in]   password  The password string
 *
 * @return  @c EINA_TRUE if the password is set successfully, \n
 *          otherwise @c EINA_FALSE on error
 *
 * @see ecore_con_url_proxy_set()
 *
 */
EAPI Eina_Bool ecore_con_url_proxy_password_set(Ecore_Con_Url *url_con, const char *password);

/**
 * @brief   Sets timeout in seconds.
 *
 * @since   1.2
 *
 * @remarks The maximum time in seconds that you allow the ecore_con_url_transfer
 *          operation to take. Normally, name lookups can take a considerable time
 *          and limiting operations to less than a few minutes risk aborting perfectly
 *          normal operations.
 *
 * @param[in]   url_con  The connection object that uses the timeout
 * @param[in]   timeout  The time in seconds
 *
 * @see ecore_con_url_cookies_jar_file_set()
 */
EAPI void ecore_con_url_timeout_set(Ecore_Con_Url *url_con, double timeout);

/**
 * @brief   Gets the returned HTTP STATUS code.
 * @since   1.2
 *
 * @remarks This is used to try to return the status code for a transmission.
 *
 * @param[in]   url_con  The connection object
 * @return  A valid HTTP STATUS code, \n
 *          otherwise @c 0 on failure
 *
 */
EAPI int ecore_con_url_status_code_get(Ecore_Con_Url *url_con);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
