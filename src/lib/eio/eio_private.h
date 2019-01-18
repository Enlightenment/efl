#ifndef EIO_PRIVATE_H_
#define EIO_PRIVATE_H_

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <sys/types.h>

#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_FEATURES_H
# include <features.h>
#endif

#ifdef _WIN32
# include <Evil.h>
#endif

#include <Ecore.h>

#include "Eio.h"

#ifdef _WIN32
# ifdef _WIN64
typedef struct __stat64 _eio_stat_t;
#  define _eio_stat(p, b) _stat64(p, b)
#  define _eio_lstat(p, b) _stat64(p, b)
# else
typedef struct _stat _eio_stat_t;
#  define _eio_stat(p, b) _stat(p, b)
#  define _eio_lstat(p, b) _stat(p, b)
# endif
#else
typedef struct stat _eio_stat_t;
# define _eio_stat(p, b) stat(p, b)
# define _eio_lstat(p, b) lstat(p, b)

# include <grp.h>
# include <pwd.h>
#endif

/* Keeping 8 Eio_File_Progress alive should be enought */
#define EIO_PROGRESS_LIMIT 8

/* Huge TLB == 16M on most system */
#define EIO_PACKET_SIZE 65536
#define EIO_PACKET_COUNT 256

#define EIO_PACKED_TIME 0.003

extern int _eio_log_dom_global;

#ifdef EIO_DEFAULT_LOG_COLOR
# undef EIO_DEFAULT_LOG_COLOR
#endif /* ifdef EIO_DEFAULT_LOG_COLOR */
#define EIO_DEFAULT_LOG_COLOR EINA_COLOR_CYAN
#ifdef ERR
# undef ERR
#endif /* ifdef ERR */
#define ERR(...)  EINA_LOG_DOM_ERR(_eio_log_dom_global, __VA_ARGS__)
#ifdef DBG
# undef DBG
#endif /* ifdef DBG */
#define DBG(...)  EINA_LOG_DOM_DBG(_eio_log_dom_global, __VA_ARGS__)
#ifdef INF
# undef INF
#endif /* ifdef INF */
#define INF(...)  EINA_LOG_DOM_INFO(_eio_log_dom_global, __VA_ARGS__)
#ifdef WRN
# undef WRN
#endif /* ifdef WRN */
#define WRN(...)  EINA_LOG_DOM_WARN(_eio_log_dom_global, __VA_ARGS__)
#ifdef CRI
# undef CRI
#endif /* ifdef CRI */
#define CRI(...) EINA_LOG_DOM_CRIT(_eio_log_dom_global, __VA_ARGS__)

typedef void (*Eio_Array_Cb)(void *data, Eio_File *common, Eina_Array *g);

typedef struct _Eio_Eet_Open Eio_Eet_Open;
typedef struct _Eio_Eet_Simple Eio_Eet_Simple;
typedef struct _Eio_Eet_Write Eio_Eet_Write;
typedef struct _Eio_Eet_Read Eio_Eet_Read;
typedef struct _Eio_Eet_Image_Write Eio_Eet_Image_Write;
typedef struct _Eio_File_Map Eio_File_Map;
typedef struct _Eio_File_Map_Rule Eio_File_Map_Rule;
typedef struct _Eio_File_Ls Eio_File_Ls;
typedef struct _Eio_File_Direct_Ls Eio_File_Direct_Ls;
typedef struct _Eio_File_Dir_Ls Eio_File_Dir_Ls;
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
   // Do not put anything before info
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

   uint64_t length;
};

struct _Eio_Eet_Simple
{
   Eio_File common;

   Eet_File *ef;
   Eio_Eet_Error_Cb error_cb;
   Eet_Error error;
};

struct _Eio_Eet_Write
{
   Eio_File common;

   Eet_File *ef;
   Eet_Data_Descriptor *edd;
   const char *name;
   const char *cipher_key;
   void *write_data;
   int compress;
   int size;

   int result;
   Eio_Done_Int_Cb done_cb;
};

struct _Eio_Eet_Read
{
   Eio_File common;

   Eet_File *ef;
   Eet_Data_Descriptor *edd;
   const char *name;
   const char *cipher_key;

   int size;

   void *result;
   union {
      Eio_Done_ERead_Cb eread;
      Eio_Done_Data_Cb data;
      Eio_Done_Read_Cb read;
   } done_cb;
};

struct _Eio_Eet_Image_Write
{
   Eio_File common;

   Eet_File *ef;
   const char *name;
   const char *cipher_key;
   void *write_data;

   unsigned int w;
   unsigned int h;
   int alpha;
   int compress;
   int quality;
   int lossy;

   int result;
   Eio_Done_Int_Cb done_cb;
};

struct _Eio_Eet_Open
{
   Eio_File common;

   Eio_Eet_Open_Cb eet_cb;
   const char *filename;
   Eet_File_Mode mode;

   Eet_File *result;
};

struct _Eio_File_Map
{
   Eio_File common;

   Eio_Open_Cb open_cb;
   const char *name;
   Eina_Bool shared;

   Eina_File *result;
};

struct _Eio_File_Map_Rule
{
   Eio_File common;
   Eio_Filter_Map_Cb filter_cb;
   Eio_Map_Cb map_cb;

   Eina_File_Populate rule;
   Eina_File *file;

   unsigned long int offset;
   unsigned long int length;

   void *result;
};

struct _Eio_File_Ls
{
   Eio_File common;
   const char *directory;
   Eina_Iterator *ls;

   Eina_Bool gather;
};

struct _Eio_File_Direct_Ls
{
   Eio_File_Ls ls;

   Eio_Filter_Direct_Cb filter_cb;
   union {
      Eio_Main_Direct_Cb main_cb;
      Eio_Array_Cb main_internal_cb;
   };

   Eina_List *pack;
   double start;
};

struct _Eio_File_Dir_Ls
{
   Eio_File_Ls ls;

   Eio_Filter_Dir_Cb filter_cb;
   union {
      Eio_Main_Direct_Cb main_cb;
      Eio_Array_Cb main_internal_cb;
   };

   Eina_List *pack;
   double start;
};

struct _Eio_File_Char_Ls
{
   Eio_File_Ls ls;

   Eio_Filter_Cb filter_cb;
   union {
      Eio_Main_Cb main_cb;
      Eio_Array_Cb main_internal_cb;
   };
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

   Eina_Stat buffer;
   const char *path;
};

typedef enum {
  EIO_XATTR_DATA,
  EIO_XATTR_STRING,
  EIO_XATTR_DOUBLE,
  EIO_XATTR_INT
} Eio_File_Xattr_Op;

struct _Eio_File_Xattr
{
   Eio_File common;

   const char *path;
   const char *attribute;
   Eina_Xattr_Flags flags;

   Eio_File_Xattr_Op op;

   union {
     struct {
       Eio_Done_Data_Cb done_cb;

       char *xattr_data;
       ssize_t xattr_size;
     } xdata;
     struct {
       Eio_Done_String_Cb done_cb;

       char *xattr_string;
     } xstring;
     struct {
       Eio_Done_Double_Cb done_cb;

       double xattr_double;
     } xdouble;
     struct {
       Eio_Done_Int_Cb done_cb;

       int xattr_int;
     } xint;
   } todo;

   Eina_Bool set : 1;
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

   time_t mtime;

   Eina_Bool fallback : 1;
   Eina_Bool rename : 1;
   Eina_Bool delete_me : 1;
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

void eio_file_free(Eio_File *common);
void eio_async_free(Eio_File_Ls *async);

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

Eina_List *eio_pack_send(Ecore_Thread *thread, Eina_List *pack, double *start);

void *eio_common_alloc(size_t size);
void eio_common_free(Eio_File *common);

void eio_file_register(Eio_File *common);
void eio_file_unregister(Eio_File *common);

Eio_File * _eio_file_ls(const char *dir,
                        Eio_Array_Cb main_internal_cb,
                        Eio_Done_Cb done_cb,
                        Eio_Error_Cb error_cb,
                        const void *data);
Eio_File * _eio_file_direct_ls(const char *dir,
                               Eio_Array_Cb main_internal_cb,
                               Eio_Done_Cb done_cb,
                               Eio_Error_Cb error_cb,
                               const void *data);
Eio_File * _eio_dir_direct_ls(const char *dir,
                              Eio_Array_Cb main_internal_cb,
                              Eio_Done_Cb done_cb,
                              Eio_Error_Cb error_cb,
                              const void *data);
Eio_File * _eio_file_stat_ls(const char *dir,
                             Eio_Array_Cb main_internal_cb,
                             Eio_Done_Cb done_cb,
                             Eio_Error_Cb error_cb,
                             const void *data);
Eio_File * _eio_dir_stat_ls(const char *dir,
                            Eio_Array_Cb main_internal_cb,
                            Eio_Done_Cb done_cb,
                            Eio_Error_Cb error_cb,
                            const void *data);
Eio_File * _eio_file_xattr(const char *path,
                           Eio_Array_Cb main_internal_cb,
                           Eio_Done_Cb done_cb,
                           Eio_Error_Cb error_cb,
                           const void *data);

// Sharing notifier between recursive and non recursive code.
void _eio_string_notify(void *data, Ecore_Thread *thread EINA_UNUSED, void *msg_data);
void _eio_direct_notify(void *data, Ecore_Thread *thread EINA_UNUSED, void *msg_data);

static inline Eina_Value
_efl_io_manager_future_cancel(Eo *o EINA_UNUSED, void *data, Eina_Error error)
{
   if (error == ECANCELED) eio_file_cancel(data);

   return eina_value_error_init(error);
}

static inline Eina_Future *
_efl_io_manager_future(const Eo *o, Eina_Future *f, Eio_File *h)
{
   return efl_future_then(o, f,
                          .error = _efl_io_manager_future_cancel,
                          .data = h);
}

EINA_VALUE_STRUCT_DESC_DEFINE(_eina_stat_desc,
                              NULL,
                              sizeof (Eina_Stat),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Eina_Stat, dev),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Eina_Stat, ino),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_UINT, Eina_Stat, mode),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_UINT, Eina_Stat, nlink),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_UINT, Eina_Stat, uid),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_UINT, Eina_Stat, gid),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Eina_Stat, rdev),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Eina_Stat, size),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Eina_Stat, blksize),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Eina_Stat, blocks),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_TIMESTAMP, Eina_Stat, atime),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Eina_Stat, atimensec),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_TIMESTAMP, Eina_Stat, mtime),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Eina_Stat, mtimensec),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_TIMESTAMP, Eina_Stat, ctime),
                              EINA_VALUE_STRUCT_MEMBER(EINA_VALUE_TYPE_ULONG, Eina_Stat, ctimensec));

#endif
