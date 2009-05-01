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
// for clients to connect to cserve
EAPI Eina_Bool evas_cserve_init(void);
EAPI int       evas_cserve_use_get(void);
EAPI void      evas_cserve_shutdown(void);
EAPI Eina_Bool evas_cserve_image_load(Image_Entry *ie, const char *file, const char *key, RGBA_Image_Loadopts *lopt);
EAPI Eina_Bool evas_cserve_image_data_load(Image_Entry *ie);
EAPI void      evas_cserve_image_free(Image_Entry *ie);
    
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
    

enum
{
   OP_NOP, // 0
     
     OP_INIT, // 1
     OP_LOAD, // 2
     OP_UNLOAD, // 3
     OP_LOADDATA, // 4
     OP_PRELOAD, // 5
     OP_FORCEDUNLOAD, // 6
     
   OP_INVALID // 6
};

typedef struct
{
   pid_t pid;
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
} Op_Unload;
typedef struct
{
   void *handle;
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
} Op_Forcedunload;

#endif

#endif
