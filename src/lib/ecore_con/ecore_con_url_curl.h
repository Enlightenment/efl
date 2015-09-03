#ifndef ECORE_CON_URL_CURL_H
#define ECORE_CON_URL_CURL_H 1

// all the types, defines, enums etc. from curl that we actually USE.
// we have to add to this if we use more things from curl not already
// defined here. see curl headers to get them from
typedef enum
{
   CURLM_CALL_MULTI_PERFORM = -1,
   CURLM_OK = 0
} CURLMcode;
typedef enum
{
   CURLE_OK = 0,
   CURLE_OPERATION_TIMEDOUT = 28
} CURLcode;
#define CURLOPTTYPE_LONG          0
#define CURLOPTTYPE_OBJECTPOINT   10000
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
   CINIT(FOLLOWLOCATION, LONG, 52),
   CINIT(PROGRESSFUNCTION, FUNCTIONPOINT, 56),
   CINIT(PROGRESSDATA, OBJECTPOINT, 57),
   CINIT(POSTFIELDSIZE, LONG, 60),
   CINIT(SSL_VERIFYPEER, LONG, 64),
   CINIT(CAINFO, OBJECTPOINT, 65),
   CINIT(CONNECTTIMEOUT, LONG, 78),
   CINIT(HEADERFUNCTION, FUNCTIONPOINT, 79),
   CINIT(COOKIEJAR, OBJECTPOINT, 82),
   CINIT(HTTP_VERSION, LONG, 84),
   CINIT(FTP_USE_EPSV, LONG, 85),
   CINIT(COOKIESESSION, LONG, 96),
   CINIT(PROXYTYPE, LONG, 101),
   CINIT(ACCEPT_ENCODING, OBJECTPOINT, 102),
   CINIT(HTTPAUTH, LONG, 107),
   CINIT(INFILESIZE_LARGE, OFF_T, 115),
   CINIT(COOKIELIST, OBJECTPOINT, 135),
   CINIT(USERNAME, OBJECTPOINT, 173),
   CINIT(PASSWORD, OBJECTPOINT, 174)
} CURLoption;
#define CURLINFO_STRING   0x100000
#define CURLINFO_LONG     0x200000
#define CURLINFO_DOUBLE   0x300000
#define CURLINFO_SLIST    0x400000
#define CURLINFO_MASK     0x0fffff
#define CURLINFO_TYPEMASK 0xf00000
typedef enum
{
   CURLINFO_RESPONSE_CODE = CURLINFO_LONG + 2,
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
   CINIT(PIPELINING, LONG, 3)
} CURLMoption;
typedef enum
{
   CURL_TIMECOND_NONE = 0,
   CURL_TIMECOND_IFMODSINCE = 1,
   CURL_TIMECOND_IFUNMODSINCE = 2
} curl_TimeCond;
enum
{
   CURL_HTTP_VERSION_1_0 = 1,
   CURL_HTTP_VERSION_1_1 = 2,
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
#define CURLAUTH_BASIC      (((unsigned long)1) << 0)
#define CURLAUTH_DIGEST_IE  (((unsigned long)1) << 4)
#define CURLAUTH_ANY        (~CURLAUTH_DIGEST_IE)
#define CURLAUTH_ANYSAFE    (~(CURLAUTH_BASIC | CURLAUTH_DIGEST_IE))
#define CURL_READFUNC_ABORT 0x10000000

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
   CURL                   *(*curl_easy_init)(void);
   CURLcode                (*curl_easy_setopt)(CURL *curl, CURLoption option, ...);
   const char             *(*curl_easy_strerror)(CURLcode);
   void                    (*curl_easy_cleanup)(CURL *curl);
   CURLcode                (*curl_easy_getinfo)(CURL *curl, CURLINFO info, ...);
   void                    (*curl_slist_free_all)(struct curl_slist *);
   struct curl_slist      *(*curl_slist_append)(struct curl_slist *list,
                                                const char *string);
   curl_version_info_data *(*curl_version_info)(CURLversion);
};

#define CURL_MIN_TIMEOUT 100

extern Ecore_Con_Curl *_c;
extern Eina_Bool _c_fail;
extern double _c_timeout;

Eina_Bool _c_init(void);
void _c_shutdown(void);

#endif
