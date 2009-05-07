#ifndef EVAS_CS_H
#define EVAS_CS_H 1

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <errno.h>

#include "evas_common.h"
#include "evas_private.h"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef EFL_EVAS_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVAS_BUILD */
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
#endif /* ! _WIN32 */

#ifdef EVAS_CSERVE

#define LENGTH_OF_SOCKADDR_UN(s) (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))

typedef struct _Server Server;
typedef struct _Client Client;
typedef struct _Mem Mem;

struct _Server
{
   char *socket_path;
   int fd;
   Eina_List *clients;
   int (*func) (void *fdata, Server *s, Client *c, int opcode, int size, unsigned char *data);
   void *data;
   pid_t pid;
   int server_id;
};

struct _Client
{
   Server *server;
   unsigned char *buf;
   int bufsize, bufalloc;
   int fd;
   unsigned char *inbuf;
   int inbufsize, inbufalloc;
   unsigned char dead : 1;
   void (*func) (void *fdata, Client *c);
   void *data;
   pid_t pid;
};

struct _Mem
{
   unsigned char *data;
   char *name;
   int fd;
   int id;
   int offset;
   int size;
   int ref;
   Eina_Bool write : 1;
};

//// for comms
enum
{
   OP_NOP, // 0
     
     OP_INIT, // 1
     OP_LOAD, // 2
     OP_UNLOAD, // 3
     OP_LOADDATA, // 4
     OP_UNLOADDATA, // 5
     OP_USELESSDATA, // 6
     OP_PRELOAD, // 7
     OP_FORCEDUNLOAD, // 8
     
     OP_GETCONFIG, // 9
     OP_SETCONFIG, // 10
     OP_GETSTATS, // 11
     OP_GETINFO, // 12
     
   OP_INVALID // 13
};

typedef struct
{
   pid_t pid;
   int server_id;
} Op_Init;
typedef struct
{
   struct {
      int    scale_down_by;
      double dpi;
      int    w, h;
   } lopt;
} Op_Load; // +"file""key"
typedef struct
{
   void *handle;
   int server_id;
   struct {
      int id;
      int offset;
      int size;
   } mem;
   struct {
      int w, h;
      Eina_Bool alpha : 1;
   } image;
} Op_Load_Reply;
typedef struct
{
   void *handle;
   int server_id;
} Op_Unload;
typedef struct
{
   void *handle;
   int server_id;
} Op_Unloaddata;
typedef struct
{
   void *handle;
   int server_id;
} Op_Loaddata;
typedef struct
{
   struct {
      int id;
      int offset;
      int size;
   } mem;
} Op_Loaddata_Reply;
typedef struct
{
   void *handle;
   int server_id;
} Op_Preload;
typedef struct
{
   void *handle;
   struct {
      int id;
      int offset;
      int size;
   } mem;
} Op_Preload_Reply;
typedef struct
{
   void *handle;
   int server_id;
} Op_Forcedunload;
typedef struct
{
   int cache_max_usage;
   int cache_item_timeout;
   int cache_item_timeout_check;
} Op_Getconfig_Reply;
typedef struct
{
   int cache_max_usage;
   int cache_item_timeout;
   int cache_item_timeout_check;
} Op_Setconfig;
typedef struct
{
   int saved_memory;
   int wasted_memory;
   int saved_memory_peak;
   int wasted_memory_peak;
   double saved_time_image_header_load;
   double saved_time_image_data_load;
} Op_Getstats_Reply;
typedef struct
{
   struct {
      int mem_total;
      int count;
   } active, cached;
} Op_Getinfo_Reply; // + N active Info Items + N cached items
typedef struct
{
   int file_key_size;
   int w, h;
   time_t file_mod_time;
   time_t file_checked_time;
   time_t cached_time;
   int refcount;
   int data_refcount;
   int memory_footprint;
   double head_load_time;
   double data_load_time;
   Eina_Bool alpha : 1;
   Eina_Bool data_loaded : 1;
   Eina_Bool active : 1;
   Eina_Bool dead : 1;
   Eina_Bool useless : 1;
} Op_Getinfo_Item; // + "file""key"


// for clients to connect to cserve
EAPI Eina_Bool evas_cserve_init(void);
EAPI int       evas_cserve_use_get(void);
EAPI void      evas_cserve_shutdown(void);
EAPI Eina_Bool evas_cserve_image_load(Image_Entry *ie, const char *file, const char *key, RGBA_Image_Loadopts *lopt);
EAPI Eina_Bool evas_cserve_image_data_load(Image_Entry *ie);
EAPI void      evas_cserve_image_unload(Image_Entry *ie);
EAPI void      evas_cserve_image_useless(Image_Entry *ie);
EAPI void      evas_cserve_image_free(Image_Entry *ie);
EAPI Eina_Bool evas_cserve_config_get(Op_Getconfig_Reply *config);
EAPI Eina_Bool evas_cserve_config_set(Op_Setconfig *config);
EAPI Eina_Bool evas_cserve_stats_get(Op_Getstats_Reply *stats);
EAPI Op_Getinfo_Reply *evas_cserve_info_get(void);
    
// for the server
EAPI Server *evas_cserve_server_add(void);
EAPI void evas_cserve_server_del(Server *s);
EAPI void evas_cserve_client_send(Client *c, int opcode, int size, unsigned char *data);
EAPI void evas_cserve_server_message_handler_set(Server *s, int (*func) (void *fdata, Server *s, Client *c, int opcode, int size, unsigned char *data), void *data);
EAPI void evas_cserve_server_wait(Server *s, int timeout);
    
//// for memory
// for server
EAPI Mem *evas_cserve_mem_new(int size, const char *name);
EAPI void evas_cserve_mem_free(Mem *m);
    
// for client
EAPI Mem *evas_cserve_mem_open(int pid, int id, const char *name, int size, int write);
EAPI void evas_cserve_mem_close(Mem *m);

// for both
EAPI Eina_Bool evas_cserve_mem_resize(Mem *m, int size);
EAPI void      evas_cserve_mem_del(int pid, int id);

#endif

#endif
