#ifndef ECORE_CON_URL_CURL_H
#define ECORE_CON_URL_CURL_H 1

#ifdef USE_CURL_H
/* During development you can set USE_CURL_H to use the system's
 * curl.h instead of the local replicated values, it will provide all
 * constants and type-checking.
 */
#include <curl/curl.h>
#else
#ifdef __WIN32__
# include <winsock2.h>
#else
# include <sys/socket.h>
#endif

// all the types, defines, enums etc. from curl that we actually USE.
// we have to add to this if we use more things from curl not already
// defined here. see curl headers to get them from
typedef enum
{
  CURLM_CALL_MULTI_PERFORM = -1, /* please call curl_multi_perform() or
                                    curl_multi_socket*() soon */
  CURLM_OK,
  CURLM_BAD_HANDLE,      /* the passed-in handle is not a valid CURLM handle */
  CURLM_BAD_EASY_HANDLE, /* an easy handle was not good/valid */
  CURLM_OUT_OF_MEMORY,   /* if you ever get this, you're in deep sh*t */
  CURLM_INTERNAL_ERROR,  /* this is a libcurl bug */
  CURLM_BAD_SOCKET,      /* the passed in socket argument did not match */
  CURLM_UNKNOWN_OPTION,  /* curl_multi_setopt() with unsupported option */
  CURLM_ADDED_ALREADY,   /* an easy handle already added to a multi handle was
                            attempted to get added - again */
  CURLM_LAST
} CURLMcode;

#ifndef curl_socket_typedef
/* socket typedef */
#if defined(WIN32) && !defined(__LWIP_OPT_H__) && !defined(LWIP_HDR_OPT_H)
typedef SOCKET curl_socket_t;
#define CURL_SOCKET_BAD INVALID_SOCKET
#else
typedef int curl_socket_t;
#define CURL_SOCKET_BAD -1
#endif
#define curl_socket_typedef
#endif /* curl_socket_typedef */

typedef enum  {
  CURLSOCKTYPE_IPCXN,  /* socket created for a specific IP connection */
  CURLSOCKTYPE_ACCEPT, /* socket created by accept() call */
  CURLSOCKTYPE_LAST    /* never use */
} curlsocktype;

struct curl_sockaddr {
  int family;
  int socktype;
  int protocol;
  unsigned int addrlen; /* addrlen was a socklen_t type before 7.18.0 but it
                           turned really ugly and painful on the systems that
                           lack this type */
  struct sockaddr addr;
};

#define CURL_POLL_NONE   0
#define CURL_POLL_IN     1
#define CURL_POLL_OUT    2
#define CURL_POLL_INOUT  3
#define CURL_POLL_REMOVE 4

#define CURL_SOCKET_TIMEOUT CURL_SOCKET_BAD

#define CURL_CSELECT_IN   0x01
#define CURL_CSELECT_OUT  0x02
#define CURL_CSELECT_ERR  0x04

typedef enum {
  CURLINFO_TEXT = 0,
  CURLINFO_HEADER_IN,    /* 1 */
  CURLINFO_HEADER_OUT,   /* 2 */
  CURLINFO_DATA_IN,      /* 3 */
  CURLINFO_DATA_OUT,     /* 4 */
  CURLINFO_SSL_DATA_IN,  /* 5 */
  CURLINFO_SSL_DATA_OUT, /* 6 */
  CURLINFO_END
} curl_infotype;

typedef enum {
  CURLE_OK = 0,
  CURLE_UNSUPPORTED_PROTOCOL,    /* 1 */
  CURLE_FAILED_INIT,             /* 2 */
  CURLE_URL_MALFORMAT,           /* 3 */
  CURLE_NOT_BUILT_IN,            /* 4 - [was obsoleted in August 2007 for
                                    7.17.0, reused in April 2011 for 7.21.5] */
  CURLE_COULDNT_RESOLVE_PROXY,   /* 5 */
  CURLE_COULDNT_RESOLVE_HOST,    /* 6 */
  CURLE_COULDNT_CONNECT,         /* 7 */
  CURLE_FTP_WEIRD_SERVER_REPLY,  /* 8 */
  CURLE_REMOTE_ACCESS_DENIED,    /* 9 a service was denied by the server
                                    due to lack of access - when login fails
                                    this is not returned. */
  CURLE_FTP_ACCEPT_FAILED,       /* 10 - [was obsoleted in April 2006 for
                                    7.15.4, reused in Dec 2011 for 7.24.0]*/
  CURLE_FTP_WEIRD_PASS_REPLY,    /* 11 */
  CURLE_FTP_ACCEPT_TIMEOUT,      /* 12 - timeout occurred accepting server
                                    [was obsoleted in August 2007 for 7.17.0,
                                    reused in Dec 2011 for 7.24.0]*/
  CURLE_FTP_WEIRD_PASV_REPLY,    /* 13 */
  CURLE_FTP_WEIRD_227_FORMAT,    /* 14 */
  CURLE_FTP_CANT_GET_HOST,       /* 15 */
  CURLE_HTTP2,                   /* 16 - A problem in the http2 framing layer.
                                    [was obsoleted in August 2007 for 7.17.0,
                                    reused in July 2014 for 7.38.0] */
  CURLE_FTP_COULDNT_SET_TYPE,    /* 17 */
  CURLE_PARTIAL_FILE,            /* 18 */
  CURLE_FTP_COULDNT_RETR_FILE,   /* 19 */
  CURLE_OBSOLETE20,              /* 20 - NOT USED */
  CURLE_QUOTE_ERROR,             /* 21 - quote command failure */
  CURLE_HTTP_RETURNED_ERROR,     /* 22 */
  CURLE_WRITE_ERROR,             /* 23 */
  CURLE_OBSOLETE24,              /* 24 - NOT USED */
  CURLE_UPLOAD_FAILED,           /* 25 - failed upload "command" */
  CURLE_READ_ERROR,              /* 26 - couldn't open/read from file */
  CURLE_OUT_OF_MEMORY,           /* 27 */
  /* Note: CURLE_OUT_OF_MEMORY may sometimes indicate a conversion error
           instead of a memory allocation error if CURL_DOES_CONVERSIONS
           is defined
  */
  CURLE_OPERATION_TIMEDOUT,      /* 28 - the timeout time was reached */
  CURLE_OBSOLETE29,              /* 29 - NOT USED */
  CURLE_FTP_PORT_FAILED,         /* 30 - FTP PORT operation failed */
  CURLE_FTP_COULDNT_USE_REST,    /* 31 - the REST command failed */
  CURLE_OBSOLETE32,              /* 32 - NOT USED */
  CURLE_RANGE_ERROR,             /* 33 - RANGE "command" didn't work */
  CURLE_HTTP_POST_ERROR,         /* 34 */
  CURLE_SSL_CONNECT_ERROR,       /* 35 - wrong when connecting with SSL */
  CURLE_BAD_DOWNLOAD_RESUME,     /* 36 - couldn't resume download */
  CURLE_FILE_COULDNT_READ_FILE,  /* 37 */
  CURLE_LDAP_CANNOT_BIND,        /* 38 */
  CURLE_LDAP_SEARCH_FAILED,      /* 39 */
  CURLE_OBSOLETE40,              /* 40 - NOT USED */
  CURLE_FUNCTION_NOT_FOUND,      /* 41 */
  CURLE_ABORTED_BY_CALLBACK,     /* 42 */
  CURLE_BAD_FUNCTION_ARGUMENT,   /* 43 */
  CURLE_OBSOLETE44,              /* 44 - NOT USED */
  CURLE_INTERFACE_FAILED,        /* 45 - CURLOPT_INTERFACE failed */
  CURLE_OBSOLETE46,              /* 46 - NOT USED */
  CURLE_TOO_MANY_REDIRECTS,      /* 47 - catch endless re-direct loops */
  CURLE_UNKNOWN_OPTION,          /* 48 - User specified an unknown option */
  CURLE_TELNET_OPTION_SYNTAX,    /* 49 - Malformed telnet option */
  CURLE_OBSOLETE50,              /* 50 - NOT USED */
  CURLE_PEER_FAILED_VERIFICATION, /* 51 - peer's certificate or fingerprint
                                     wasn't verified fine */
  CURLE_GOT_NOTHING,             /* 52 - when this is a specific error */
  CURLE_SSL_ENGINE_NOTFOUND,     /* 53 - SSL crypto engine not found */
  CURLE_SSL_ENGINE_SETFAILED,    /* 54 - can not set SSL crypto engine as
                                    default */
  CURLE_SEND_ERROR,              /* 55 - failed sending network data */
  CURLE_RECV_ERROR,              /* 56 - failure in receiving network data */
  CURLE_OBSOLETE57,              /* 57 - NOT IN USE */
  CURLE_SSL_CERTPROBLEM,         /* 58 - problem with the local certificate */
  CURLE_SSL_CIPHER,              /* 59 - couldn't use specified cipher */
  CURLE_SSL_CACERT,              /* 60 - problem with the CA cert (path?) */
  CURLE_BAD_CONTENT_ENCODING,    /* 61 - Unrecognized/bad encoding */
  CURLE_LDAP_INVALID_URL,        /* 62 - Invalid LDAP URL */
  CURLE_FILESIZE_EXCEEDED,       /* 63 - Maximum file size exceeded */
  CURLE_USE_SSL_FAILED,          /* 64 - Requested FTP SSL level failed */
  CURLE_SEND_FAIL_REWIND,        /* 65 - Sending the data requires a rewind
                                    that failed */
  CURLE_SSL_ENGINE_INITFAILED,   /* 66 - failed to initialise ENGINE */
  CURLE_LOGIN_DENIED,            /* 67 - user, password or similar was not
                                    accepted and we failed to login */
  CURLE_TFTP_NOTFOUND,           /* 68 - file not found on server */
  CURLE_TFTP_PERM,               /* 69 - permission problem on server */
  CURLE_REMOTE_DISK_FULL,        /* 70 - out of disk space on server */
  CURLE_TFTP_ILLEGAL,            /* 71 - Illegal TFTP operation */
  CURLE_TFTP_UNKNOWNID,          /* 72 - Unknown transfer ID */
  CURLE_REMOTE_FILE_EXISTS,      /* 73 - File already exists */
  CURLE_TFTP_NOSUCHUSER,         /* 74 - No such user */
  CURLE_CONV_FAILED,             /* 75 - conversion failed */
  CURLE_CONV_REQD,               /* 76 - caller must register conversion
                                    callbacks using curl_easy_setopt options
                                    CURLOPT_CONV_FROM_NETWORK_FUNCTION,
                                    CURLOPT_CONV_TO_NETWORK_FUNCTION, and
                                    CURLOPT_CONV_FROM_UTF8_FUNCTION */
  CURLE_SSL_CACERT_BADFILE,      /* 77 - could not load CACERT file, missing
                                    or wrong format */
  CURLE_REMOTE_FILE_NOT_FOUND,   /* 78 - remote file not found */
  CURLE_SSH,                     /* 79 - error from the SSH layer, somewhat
                                    generic so the error message will be of
                                    interest when this has happened */

  CURLE_SSL_SHUTDOWN_FAILED,     /* 80 - Failed to shut down the SSL
                                    connection */
  CURLE_AGAIN,                   /* 81 - socket is not ready for send/recv,
                                    wait till it's ready and try again (Added
                                    in 7.18.2) */
  CURLE_SSL_CRL_BADFILE,         /* 82 - could not load CRL file, missing or
                                    wrong format (Added in 7.19.0) */
  CURLE_SSL_ISSUER_ERROR,        /* 83 - Issuer check failed.  (Added in
                                    7.19.0) */
  CURLE_FTP_PRET_FAILED,         /* 84 - a PRET command failed */
  CURLE_RTSP_CSEQ_ERROR,         /* 85 - mismatch of RTSP CSeq numbers */
  CURLE_RTSP_SESSION_ERROR,      /* 86 - mismatch of RTSP Session Ids */
  CURLE_FTP_BAD_FILE_LIST,       /* 87 - unable to parse FTP file list */
  CURLE_CHUNK_FAILED,            /* 88 - chunk callback reported error */
  CURLE_NO_CONNECTION_AVAILABLE, /* 89 - No connection available, the
                                    session will be queued */
  CURLE_SSL_PINNEDPUBKEYNOTMATCH, /* 90 - specified pinned public key did not
                                     match */
  CURLE_SSL_INVALIDCERTSTATUS,   /* 91 - invalid certificate status */
  CURLE_HTTP2_STREAM,            /* 92 - stream error in HTTP/2 framing layer
                                    */
  CURL_LAST /* never use! */
} CURLcode;
#define CURLOPTTYPE_LONG          0
#define CURLOPTTYPE_OBJECTPOINT   10000
#define CURLOPTTYPE_STRINGPOINT   10000
#define CURLOPTTYPE_FUNCTIONPOINT 20000
#define CURLOPTTYPE_OFF_T         30000
#define CINIT(na, t, nu) CURLOPT_ ## na = CURLOPTTYPE_ ## t + nu
typedef enum
{
   CINIT(FILE, OBJECTPOINT, 1),
   CINIT(URL, OBJECTPOINT, 2),
   CINIT(PROXY, OBJECTPOINT, 4),
   CINIT(USERPWD, OBJECTPOINT, 5),
   CINIT(INFILE, OBJECTPOINT, 9),
   CINIT(WRITEFUNCTION, FUNCTIONPOINT, 11),
   CINIT(READFUNCTION, FUNCTIONPOINT, 12),
   CINIT(POSTFIELDS, OBJECTPOINT, 15),
   CINIT(USERAGENT, STRINGPOINT, 18),
   CINIT(HTTPHEADER, OBJECTPOINT, 23),
   CINIT(WRITEHEADER, OBJECTPOINT, 29),
   CINIT(COOKIEFILE, OBJECTPOINT, 31),
   CINIT(TIMECONDITION, LONG, 33),
   CINIT(TIMEVALUE, LONG, 34),
   CINIT(CUSTOMREQUEST, OBJECTPOINT, 36),
   CINIT(VERBOSE, LONG, 41),
   CINIT(NOPROGRESS, LONG, 43),
   CINIT(NOBODY, LONG, 44),
   CINIT(UPLOAD, LONG, 46),
   CINIT(POST, LONG, 47),
   CINIT(PUT, LONG, 54),
   CINIT(FOLLOWLOCATION, LONG, 52),
   CINIT(PROGRESSFUNCTION, FUNCTIONPOINT, 56),
   CINIT(PROGRESSDATA, OBJECTPOINT, 57),
   CINIT(POSTFIELDSIZE, LONG, 60),
   CINIT(SSL_VERIFYPEER, LONG, 64),
   CINIT(CAINFO, OBJECTPOINT, 65),
   CINIT(CONNECTTIMEOUT, LONG, 78),
   CINIT(CONNECTTIMEOUT_MS, LONG, 156),
   CINIT(HEADERFUNCTION, FUNCTIONPOINT, 79),
   CINIT(HTTPGET, LONG, 80),
   CINIT(SSL_VERIFYHOST, LONG, 81),
   CINIT(COOKIEJAR, OBJECTPOINT, 82),
   CINIT(HTTP_VERSION, LONG, 84),
   CINIT(FTP_USE_EPSV, LONG, 85),
   CINIT(DEBUGFUNCTION, FUNCTIONPOINT, 94),
   CINIT(DEBUGDATA, OBJECTPOINT, 95),
   CINIT(COOKIESESSION, LONG, 96),
   CINIT(CAPATH, STRINGPOINT, 97),
   CINIT(BUFFERSIZE, LONG, 98),
   CINIT(NOSIGNAL, LONG, 99),
   CINIT(PROXYTYPE, LONG, 101),
   CINIT(ACCEPT_ENCODING, OBJECTPOINT, 102),
   CINIT(PRIVATE, OBJECTPOINT, 103),
   CINIT(HTTPAUTH, LONG, 107),
   CINIT(INFILESIZE_LARGE, OFF_T, 115),
   CINIT(POSTFIELDSIZE_LARGE, OFF_T, 120),
   CINIT(COOKIELIST, OBJECTPOINT, 135),
   CINIT(MAX_SEND_SPEED_LARGE, OFF_T, 145),
   CINIT(MAX_RECV_SPEED_LARGE, OFF_T, 146),
   CINIT(OPENSOCKETFUNCTION, FUNCTIONPOINT, 163),
   CINIT(OPENSOCKETDATA, OBJECTPOINT, 164),
   CINIT(CRLFILE, STRINGPOINT, 169),
   CINIT(USERNAME, OBJECTPOINT, 173),
   CINIT(PASSWORD, OBJECTPOINT, 174),
   CINIT(CLOSESOCKETFUNCTION, FUNCTIONPOINT, 208),
   CINIT(CLOSESOCKETDATA, OBJECTPOINT, 209),
   CINIT(XFERINFOFUNCTION, FUNCTIONPOINT, 219),
#define CURLOPT_XFERINFODATA CURLOPT_PROGRESSDATA
} CURLoption;
#define CURLINFO_STRING   0x100000
#define CURLINFO_LONG     0x200000
#define CURLINFO_DOUBLE   0x300000
#define CURLINFO_SLIST    0x400000
#define CURLINFO_MASK     0x0fffff
#define CURLINFO_TYPEMASK 0xf00000
typedef enum
{
   CURLINFO_EFFECTIVE_URL    = CURLINFO_STRING + 1,
   CURLINFO_RESPONSE_CODE    = CURLINFO_LONG + 2,
   CURLINFO_CONTENT_LENGTH_DOWNLOAD = CURLINFO_DOUBLE + 15,
   CURLINFO_CONTENT_TYPE     = CURLINFO_STRING + 18,
   CURLINFO_PRIVATE          = CURLINFO_STRING + 21,
   CURLINFO_HTTP_VERSION     = CURLINFO_LONG   + 46,
   CURLINFO_OS_ERRNO         = CURLINFO_LONG   + 25,
   CURLINFO_LOCAL_IP         = CURLINFO_STRING + 41,
   CURLINFO_LOCAL_PORT       = CURLINFO_LONG   + 42,
} CURLINFO;
typedef enum
{
   CURLVERSION_FOURTH = 3
} CURLversion;
typedef enum
{
   CURLMSG_DONE = 1
} CURLMSG;
#undef CINIT
#define CINIT(name, type, num) CURLMOPT_ ## name = CURLOPTTYPE_ ## type + num
typedef enum
{
   CINIT(SOCKETFUNCTION, FUNCTIONPOINT, 1),
   CINIT(SOCKETDATA, OBJECTPOINT, 2),
   CINIT(PIPELINING, LONG, 3),
   CINIT(TIMERFUNCTION, FUNCTIONPOINT, 4),
   CINIT(TIMERDATA, OBJECTPOINT, 5)
} CURLMoption;
typedef enum
{
   CURL_TIMECOND_NONE = 0,
   CURL_TIMECOND_IFMODSINCE = 1,
   CURL_TIMECOND_IFUNMODSINCE = 2
} curl_TimeCond;
enum
{
   CURL_HTTP_VERSION_NONE = 0,
   CURL_HTTP_VERSION_1_0 = 1,
   CURL_HTTP_VERSION_1_1 = 2,
   CURL_HTTP_VERSION_2_0 = 3
};
typedef enum
{
   CURLPROXY_HTTP = 0,
   CURLPROXY_SOCKS4 = 4,
   CURLPROXY_SOCKS5 = 5,
   CURLPROXY_SOCKS4A = 6,
   CURLPROXY_SOCKS5_HOSTNAME = 7
} curl_proxytype;

#define CURL_GLOBAL_SSL     (1 << 0)
#define CURL_GLOBAL_WIN32   (1 << 1)
#define CURL_GLOBAL_ALL     (CURL_GLOBAL_SSL | CURL_GLOBAL_WIN32)
#define CURLOPT_ENCODING    CURLOPT_ACCEPT_ENCODING
#define CURLOPT_WRITEDATA   CURLOPT_FILE
#define CURLOPT_READDATA    CURLOPT_INFILE
#define CURLOPT_HEADERDATA  CURLOPT_WRITEHEADER
#define CURLVERSION_NOW     CURLVERSION_FOURTH
#define CURLAUTH_NONE       ((unsigned long)0)
#define CURLAUTH_BASIC      (((unsigned long)1) << 0)
#define CURLAUTH_DIGEST       (((unsigned long)1)<<1)
#define CURLAUTH_NEGOTIATE    (((unsigned long)1)<<2)
#define CURLAUTH_DIGEST_IE  (((unsigned long)1) << 4)
#define CURLAUTH_ANY        (~CURLAUTH_DIGEST_IE)
#define CURLAUTH_NTLM         (((unsigned long)1)<<3)
#define CURLAUTH_NTLM_WB      (((unsigned long)1)<<5)
#define CURLAUTH_ONLY         (((unsigned long)1)<<31)
#define CURLAUTH_ANY          (~CURLAUTH_DIGEST_IE)
#define CURLAUTH_ANYSAFE      (~(CURLAUTH_BASIC|CURLAUTH_DIGEST_IE))
#define CURL_READFUNC_ABORT 0x10000000
#define CURL_READFUNC_PAUSE 0x10000001
#define CURL_WRITEFUNC_PAUSE 0x10000001

#define CURLPAUSE_RECV      (1<<0)
#define CURLPAUSE_RECV_CONT (0)

#define CURLPAUSE_SEND      (1<<2)
#define CURLPAUSE_SEND_CONT (0)

typedef void CURLM;
typedef void CURL;
struct curl_slist
{
   char              *data;
   struct curl_slist *next;
};
typedef struct
{
   CURLversion        age;
   const char        *version;
   unsigned int       version_num;
   const char        *host;
   int                features;
   const char        *ssl_version;
   long               ssl_version_num;
   const char        *libz_version;
   const char *const *protocols;
   const char        *ares;
   int                ares_num;
   const char        *libidn;
   int                iconv_ver_num;
   const char        *libssh_version;
} curl_version_info_data;
typedef struct
{
   CURLMSG msg;
   CURL   *easy_handle;
   union
   {
      void    *whatever;
      CURLcode result;
   } data;
} CURLMsg;

#endif /* USE_CURL_H */


typedef struct _Ecore_Con_Curl Ecore_Con_Curl;

struct _Ecore_Con_Curl
{
   Eina_Module            *mod;

   CURLM                  *_curlm;

   CURLcode                (*curl_global_init)(long flags);
   void                    (*curl_global_cleanup)(void);
   CURLM                  *(*curl_multi_init)(void);
   CURLMcode               (*curl_multi_timeout)(CURLM *multi_handle,
                                                 long *milliseconds);
   CURLMcode               (*curl_multi_cleanup)(CURLM *multi_handle);
   CURLMcode               (*curl_multi_remove_handle)(CURLM *multi_handle,
                                                       CURL *curl_handle);
   const char             *(*curl_multi_strerror)(CURLMcode);
   CURLMsg                *(*curl_multi_info_read)(CURLM * multi_handle,
                                                   int *msgs_in_queue);
   CURLMcode               (*curl_multi_fdset)(CURLM *multi_handle,
                                               fd_set *read_fd_set,
                                               fd_set *write_fd_set,
                                               fd_set *exc_fd_set,
                                               int *max_fd);
   CURLMcode               (*curl_multi_perform)(CURLM *multi_handle,
                                                 int *running_handles);
   CURLMcode               (*curl_multi_add_handle)(CURLM *multi_handle,
                                                    CURL *curl_handle);
   CURLMcode               (*curl_multi_setopt)(CURLM *multi_handle,
                                                CURLMoption option, ...);
   CURLMcode               (*curl_multi_socket_action)(CURLM *multi_handle,
                                                       curl_socket_t fd,
                                                       int ev_bitmask,
                                                       int *running_handles);
   CURLMcode               (*curl_multi_assign)(CURLM *multi_handle,
                                                curl_socket_t sockfd,
                                                void *sockp);
   CURL                   *(*curl_easy_init)(void);
   CURLcode                (*curl_easy_setopt)(CURL *curl, CURLoption option, ...);
   const char             *(*curl_easy_strerror)(CURLcode);
   void                    (*curl_easy_cleanup)(CURL *curl);
   CURLcode                (*curl_easy_getinfo)(CURL *curl, CURLINFO info, ...);
   CURLcode                (*curl_easy_pause)(CURL *curl, int bitmask);
   void                    (*curl_slist_free_all)(struct curl_slist *);
   struct curl_slist      *(*curl_slist_append)(struct curl_slist *list,
                                                const char *string);
   time_t                  (*curl_getdate)(const char *p, const time_t *unused);
   curl_version_info_data *(*curl_version_info)(CURLversion);

   int                     ref; /* Reference on the structure */
};

#define CURL_MIN_TIMEOUT 100

extern Ecore_Con_Curl *_c;
extern Eina_Bool _c_fail;
extern double _c_timeout;

Eina_Bool _c_init(void);
void _c_shutdown(void);
Eina_Error _curlcode_to_eina_error(const CURLcode code);
Eina_Error _curlmcode_to_eina_error(const CURLMcode code);


/* only for legacy support to implement behavior that we're not exposing anymore */
CURL *efl_net_dialer_http_curl_get(const Eo *o);

#endif
