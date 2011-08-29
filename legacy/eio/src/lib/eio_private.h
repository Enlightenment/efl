#ifndef EIO_PRIVATE_H_
#define EIO_PRIVATE_H_

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_FEATURES_H
# include <features.h>
#endif

#include <stdio.h>
#include <string.h>

#ifndef _MSC_VER
# include <unistd.h>
# include <libgen.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pwd.h>

#ifdef HAVE_GRP_H
# include <grp.h>
#endif

#ifdef EFL_HAVE_POSIX_THREADS
# include <pthread.h>
#endif

#ifdef EFL_HAVE_WIN32_THREADS
# include <windows.h>
#endif

#include <Ecore.h>

#include "Eio.h"

/* Keeping 32 Eio_File_Progress alive should be enought */
#define EIO_PROGRESS_LIMIT 32

/* Huge TLB == 16M on most system */
#define EIO_PACKET_SIZE 65536
#define EIO_PACKET_COUNT 256

#define EIO_PACKED_TIME 0.001

typedef struct _Eio_File_Ls Eio_File_Ls;
typedef struct _Eio_File_Direct_Ls Eio_File_Direct_Ls;
typedef struct _Eio_File_Char_Ls Eio_File_Char_Ls;
typedef struct _Eio_File_Mkdir Eio_File_Mkdir;
typedef struct _Eio_File_Mkdir Eio_File_Chmod;
typedef struct _Eio_File_Unlink Eio_File_Unlink;
typedef struct _Eio_File_Stat Eio_File_Stat;
typedef struct _Eio_File_Progress Eio_File_Progress;
typedef struct _Eio_File_Move Eio_File_Move;
typedef struct _Eio_File_Chown Eio_File_Chown;
typedef struct _Eio_Monitor_Backend Eio_Monitor_Backend;

typedef struct _Eio_File_Xattr Eio_File_Xattr;

typedef struct _Eio_Dir_Copy Eio_Dir_Copy;

typedef struct _Eio_File_Direct_Info Eio_File_Direct_Info;
typedef struct _Eio_File_Char Eio_File_Char;

typedef struct _Eio_File_Associate Eio_File_Associate;

struct _Eio_File_Associate
{
   void *data;

   Eina_Free_Cb free_cb;
};

struct _Eio_File_Direct_Info
{
   Eina_File_Direct_Info info;

   Eina_Hash *associated;
};

struct _Eio_File_Char
{
   const char *filename;

   Eina_Hash *associated;
};

struct _Eio_File
{
   Ecore_Thread *thread;
   const void *data;
   void *container;

   int error;

   Eio_Error_Cb error_cb;
   Eio_Done_Cb done_cb;

   struct {
      Eina_Hash *associated;
   } worker, main;
};

struct _Eio_File_Ls
{
   Eio_File common;
   const char *directory;
};

struct _Eio_File_Direct_Ls
{
   Eio_File_Ls ls;

   Eio_Filter_Direct_Cb filter_cb;
   Eio_Main_Direct_Cb main_cb;

   Eina_List *pack;
   double start;
};

struct _Eio_File_Char_Ls
{
   Eio_File_Ls ls;

   Eio_Filter_Cb filter_cb;
   Eio_Main_Cb main_cb;
};

struct _Eio_File_Mkdir
{
   Eio_File common;

   const char *path;
   mode_t mode;
};

struct _Eio_File_Unlink
{
   Eio_File common;

   const char *path;
};

struct _Eio_File_Stat
{
   Eio_File common;

   Eio_Stat_Cb done_cb;

   struct stat buffer;
   const char *path;
};

struct _Eio_File_Xattr
{
   Eio_File common;

   Eio_Done_Length_Cb done_cb;

   const char *path;
   const char *attribute;
   int flags;

   char *xattr_data;
   unsigned int xattr_size;
};

struct _Eio_File_Progress
{
   Eio_File common;

   Eio_Progress_Cb progress_cb;

   const char *source;
   const char *dest;

   Eio_File_Op op;
};

struct _Eio_File_Move
{
   Eio_File_Progress progress;

   Eio_File *copy;
};

struct _Eio_Dir_Copy
{
   Eio_File_Progress progress;
   Eio_Filter_Direct_Cb filter_cb;

   Eina_List *files;
   Eina_List *dirs;
   Eina_List *links;
};

struct _Eio_File_Chown
{
   Eio_File common;

   const char *path;
   const char *user;
   const char *group;
};

struct _Eio_Monitor
{
   Eio_Monitor_Backend *backend;
   Eio_File *exist;

   const char *path;

   EINA_REFCOUNT;
   int error;

   Eina_Bool fallback : 1;
   Eina_Bool rename : 1;
};

/* Be aware that ecore_thread_run could call cancel_cb if something goes wrong. */
Eina_Bool eio_file_set(Eio_File *common,
		       Eio_Done_Cb done_cb,
		       Eio_Error_Cb error_cb,
		       const void *data,
		       Ecore_Thread_Cb job_cb,
		       Ecore_Thread_Cb end_cb,
		       Ecore_Thread_Cb cancel_cb);

/* Be aware that ecore_thread_run could call cancel_cb if something goes wrong. */
Eina_Bool eio_long_file_set(Eio_File *common,
			    Eio_Done_Cb done_cb,
			    Eio_Error_Cb error_cb,
			    const void *data,
			    Ecore_Thread_Cb heavy_cb,
			    Ecore_Thread_Notify_Cb notify_cb,
			    Ecore_Thread_Cb end_cb,
			    Ecore_Thread_Cb cancel_cb);

void eio_file_container_set(Eio_File *common, void *container);

void eio_file_error(Eio_File *common);
void eio_file_thread_error(Eio_File *common, Ecore_Thread *thread);

Eio_File_Direct_Info *eio_direct_info_malloc(void);
void eio_direct_info_free(Eio_File_Direct_Info *data);

Eio_File_Char *eio_char_malloc(void);
void eio_char_free(Eio_File_Char *data);

Eio_File_Associate *eio_associate_malloc(const void *data, Eina_Free_Cb free_cb);
void eio_associate_free(void *data);

Eio_Progress *eio_progress_malloc(void);
void eio_progress_free(Eio_Progress *progress);

void eio_progress_send(Ecore_Thread *thread, Eio_File_Progress *op,
                       long long current, long long max);
void eio_progress_cb(Eio_Progress *progress, Eio_File_Progress *op);

Eina_Bool eio_file_copy_do(Ecore_Thread *thread, Eio_File_Progress *copy);

void eio_monitor_init(void);
void eio_monitor_backend_init(void);
void eio_monitor_fallback_init(void);

void eio_monitor_shutdown(void);
void eio_monitor_backend_shutdown(void);
void eio_monitor_fallback_shutdown(void);
void eio_monitor_backend_add(Eio_Monitor *monitor);
void eio_monitor_fallback_add(Eio_Monitor *monitor);

void eio_monitor_backend_del(Eio_Monitor *monitor);
void eio_monitor_fallback_del(Eio_Monitor *monitor);

void _eio_monitor_send(Eio_Monitor *monitor, const char *filename, int event_code);
void _eio_monitor_rename(Eio_Monitor *monitor, const char *newpath);

void eio_async_end(void *data, Ecore_Thread *thread);
void eio_async_error(void *data, Ecore_Thread *thread);

#endif
