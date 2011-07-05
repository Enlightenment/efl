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
 * @defgroup Ecore_Con_Events_Group Events
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
 * @typedef Ecore_Con_Url
 * A handle to an http upload/download object
 * @ingroup Ecore_Con_Url_Group
 */
typedef struct _Ecore_Con_Url Ecore_Con_Url;


/**
 * @addtogroup Ecore_Con_Events_Group Events
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
 * @typedef Ecore_Con_Event_Url_Data
 * Used as the @p data param for the corresponding event
 */
typedef struct _Ecore_Con_Event_Url_Data Ecore_Con_Event_Url_Data;

/**
 * @typedef Ecore_Con_Event_Url_Complete
 * Used as the @p data param for the corresponding event
 */
typedef struct _Ecore_Con_Event_Url_Complete Ecore_Con_Event_Url_Complete;

/**
 * @typedef Ecore_Con_Event_Url_Progress
 * Used as the @p data param for the corresponding event
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
 * @struct _Ecore_Con_Event_Url_Data
 * Used as the @p data param for the @ref ECORE_CON_EVENT_URL_DATA event
 */
struct _Ecore_Con_Event_Url_Data
{
   Ecore_Con_Url *url_con;
   int size;
   unsigned char data[1];
};

/**
 * @struct _Ecore_Con_Event_Url_Complete
 * Used as the @p data param for the @ref ECORE_CON_EVENT_URL_COMPLETE event
 */
struct _Ecore_Con_Event_Url_Complete
{
   Ecore_Con_Url *url_con;
   int status;
};

/**
 * @struct _Ecore_Con_Event_Url_Progress
 * Used as the @p data param for the @ref ECORE_CON_EVENT_URL_PROGRESS event
 */
struct _Ecore_Con_Event_Url_Progress
{
   Ecore_Con_Url *url_con;
   struct
   {
      double total;
      double now;
   } down;
   struct
   {
      double total;
      double now;
   } up;
};

/** A client has connected to the server */
EAPI extern int ECORE_CON_EVENT_CLIENT_ADD;
/** A client has disconnected from the server */
EAPI extern int ECORE_CON_EVENT_CLIENT_DEL;
/** A client experienced an error */
EAPI extern int ECORE_CON_EVENT_CLIENT_ERROR;
/** A client connection has been upgraded to SSL
 * @since 1.1
 */
EAPI extern int ECORE_CON_EVENT_CLIENT_UPGRADE;
/** A server was created */
EAPI extern int ECORE_CON_EVENT_SERVER_ADD;
/** A server connection was lost */
EAPI extern int ECORE_CON_EVENT_SERVER_DEL;
/** A server experienced an error */
EAPI extern int ECORE_CON_EVENT_SERVER_ERROR;
/** A server connection has been upgraded to SSL
 * @since 1.1
 */
EAPI extern int ECORE_CON_EVENT_SERVER_UPGRADE;
/** A client connected to the server has sent data */
EAPI extern int ECORE_CON_EVENT_CLIENT_DATA;
/** A server connection object has data */
EAPI extern int ECORE_CON_EVENT_SERVER_DATA;
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
   ECORE_CON_REMOTE_NODELAY = 7,
   /** Use SSL2: UNSUPPORTED. **/
   ECORE_CON_USE_SSL2 = (1 << 4),
   /** Use SSL3 */
   ECORE_CON_USE_SSL3 = (1 << 5),
   /** Use TLS */
   ECORE_CON_USE_TLS = (1 << 6),
   /** Use both TLS and SSL3 */
   ECORE_CON_USE_MIXED = ECORE_CON_USE_SSL3 | ECORE_CON_USE_TLS,
   /** Attempt to use the loaded certificate */
   ECORE_CON_LOAD_CERT = (1 << 7)
} Ecore_Con_Type;

/**
 * Initialises the Ecore_Con library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 */
EAPI int               ecore_con_init(void);

/**
 * Shuts down the Ecore_Con library.
 * @return  Number of times the library has been initialised without being
 *          shut down.
 */
EAPI int               ecore_con_shutdown(void);

/**
 * Do an asynchronous DNS lookup.
 *
 * @param name IP address or server name to translate.
 * @param done_cb Callback to notify when done.
 * @param data User data to be given to done_cb.
 * @return EINA_TRUE if the request did not fail to be set up, EINA_FALSE if it
 * failed.
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
EAPI Eina_Bool         ecore_con_ssl_server_upgrade(Ecore_Con_Server *svr, Ecore_Con_Type compl_type);
EAPI Eina_Bool         ecore_con_ssl_client_upgrade(Ecore_Con_Client *cl, Ecore_Con_Type compl_type);

/**
 * @}
 */

/**
 * @defgroup Ecore_Con_Server_Group Ecore Connection Server Functions
 *
 * @{
 */

EAPI Ecore_Con_Server *ecore_con_server_add(Ecore_Con_Type type,
                                            const char *name, int port,
                                            const void *data);

EAPI Ecore_Con_Server *ecore_con_server_connect(Ecore_Con_Type type,
                                                const char *name, int port,
                                                const void *data);
EAPI void *            ecore_con_server_del(Ecore_Con_Server *svr);
EAPI void *            ecore_con_server_data_get(Ecore_Con_Server *svr);
EAPI void *            ecore_con_server_data_set(Ecore_Con_Server *svr,
                                                 void *data);
EAPI Eina_Bool         ecore_con_server_connected_get(Ecore_Con_Server *svr);
EAPI Eina_List *       ecore_con_server_clients_get(Ecore_Con_Server *svr);
EAPI const char *      ecore_con_server_name_get(Ecore_Con_Server *svr);
EAPI int               ecore_con_server_port_get(Ecore_Con_Server *svr);
EAPI double            ecore_con_server_uptime_get(Ecore_Con_Server *svr);
EAPI int               ecore_con_server_send(Ecore_Con_Server *svr,
                                             const void *data,
                                             int size);
EAPI void              ecore_con_server_client_limit_set(Ecore_Con_Server *svr,
                                                         int client_limit,
                                                         char reject_excess_clients);
EAPI const char *      ecore_con_server_ip_get(Ecore_Con_Server *svr);
EAPI void              ecore_con_server_flush(Ecore_Con_Server *svr);
EAPI void              ecore_con_server_timeout_set(Ecore_Con_Server *svr, double timeout);
EAPI double            ecore_con_server_timeout_get(Ecore_Con_Server *svr);

/**
 * @}
 */

/**
 * @defgroup Ecore_Con_Client_Group Ecore Connection Client Functions
 *
 * @{
 */

EAPI int               ecore_con_client_send(Ecore_Con_Client *cl,
                                             const void *data,
                                             int size);
EAPI Ecore_Con_Server *ecore_con_client_server_get(Ecore_Con_Client *cl);
EAPI void *            ecore_con_client_del(Ecore_Con_Client *cl);
EAPI void              ecore_con_client_data_set(Ecore_Con_Client *cl,
                                                 const void *data);
EAPI void *            ecore_con_client_data_get(Ecore_Con_Client *cl);
EAPI const char *      ecore_con_client_ip_get(Ecore_Con_Client *cl);
EAPI void              ecore_con_client_flush(Ecore_Con_Client *cl);
EAPI double            ecore_con_client_uptime_get(Ecore_Con_Client *cl);
EAPI double            ecore_con_client_timeout_get(Ecore_Con_Client *cl);
EAPI void              ecore_con_client_timeout_set(Ecore_Con_Client *cl, double timeout);
EAPI Eina_Bool         ecore_con_client_connected_get(Ecore_Con_Client *cl);
EAPI int               ecore_con_client_port_get(Ecore_Con_Client *cl);

/**
 * @}
 */

/**
 * @defgroup Ecore_Con_Url_Group Ecore URL Connection Functions
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

EAPI int               ecore_con_url_init(void);
EAPI int               ecore_con_url_shutdown(void);

EAPI void              ecore_con_url_pipeline_set(Eina_Bool enable);
EAPI Eina_Bool         ecore_con_url_pipeline_get(void);

EAPI Ecore_Con_Url *   ecore_con_url_new(const char *url);
EAPI Ecore_Con_Url *   ecore_con_url_custom_new(const char *url,
                                                const char *custom_request);
EAPI void              ecore_con_url_free(Ecore_Con_Url *url_con);
EAPI Eina_Bool         ecore_con_url_url_set(Ecore_Con_Url *url_con,
                                             const char *url);
EAPI void              ecore_con_url_data_set(Ecore_Con_Url *url_con,
                                              void *data);
EAPI void *            ecore_con_url_data_get(Ecore_Con_Url *url_con);
EAPI void              ecore_con_url_additional_header_add(Ecore_Con_Url *url_con,
                                                           const char *key,
                                                           const char *value);
EAPI void              ecore_con_url_additional_headers_clear(Ecore_Con_Url *url_con);
EAPI const Eina_List * ecore_con_url_response_headers_get(Ecore_Con_Url *url_con);
EAPI void              ecore_con_url_fd_set(Ecore_Con_Url *url_con, int fd);
EAPI int               ecore_con_url_received_bytes_get(Ecore_Con_Url *url_con);
EAPI Eina_Bool         ecore_con_url_httpauth_set(Ecore_Con_Url *url_con,
                                                  const char *username,
                                                  const char *password,
                                                  Eina_Bool safe);
EINA_DEPRECATED EAPI Eina_Bool         ecore_con_url_send(Ecore_Con_Url *url_con,
                                                          const void *data, long length,
                                                          const char *content_type);
EAPI Eina_Bool         ecore_con_url_get(Ecore_Con_Url *url_con);
EAPI Eina_Bool         ecore_con_url_post(Ecore_Con_Url *url_con,
                                          const void *data, long length,
                                          const char *content_type);
EAPI void              ecore_con_url_time(Ecore_Con_Url *url_con,
                                          Ecore_Con_Url_Time time_condition,
                                          double timestamp);

EAPI Eina_Bool         ecore_con_url_ftp_upload(Ecore_Con_Url *url_con,
                                                const char *filename,
                                                const char *user,
                                                const char *pass,
                                                const char *upload_dir);
EAPI void              ecore_con_url_verbose_set(Ecore_Con_Url *url_con,
                                                 Eina_Bool verbose);
EAPI void              ecore_con_url_ftp_use_epsv_set(Ecore_Con_Url *url_con,
                                                      Eina_Bool use_epsv);

EAPI void              ecore_con_url_cookies_init(Ecore_Con_Url *url_con);
EAPI void              ecore_con_url_cookies_ignore_old_session_set(Ecore_Con_Url *url_con,
                                                                    Eina_Bool ignore);
EAPI void              ecore_con_url_cookies_clear(Ecore_Con_Url *url_con);
EAPI void              ecore_con_url_cookies_session_clear(Ecore_Con_Url *url_con);
EAPI void              ecore_con_url_cookies_file_add(Ecore_Con_Url *url_con,
                                                      const char * const file_name);
EAPI Eina_Bool         ecore_con_url_cookies_jar_file_set(Ecore_Con_Url *url_con,
                                                          const char * const cookiejar_file);
EAPI void              ecore_con_url_cookies_jar_write(Ecore_Con_Url *url_con);

EAPI void              ecore_con_url_ssl_verify_peer_set(Ecore_Con_Url *url_con,
                                                         Eina_Bool verify);
EAPI int               ecore_con_url_ssl_ca_set(Ecore_Con_Url *url_con,
                                                const char *ca_path);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif
