#ifndef EIO_PRIVATE_H_
#define EIO_PRIVATE_H_

#include <Ecore.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Eio.h"

/* Keeping 32 Eio_File_Progress alive should be enought */
#define EIO_PROGRESS_LIMIT 32

/* Huge TLB == 16M on most system */
#define EIO_PACKET_SIZE 65536
#define EIO_PACKET_COUNT 256

typedef struct _Eio_File_Ls Eio_File_Ls;
typedef struct _Eio_File_Direct_Ls Eio_File_Direct_Ls;
typedef struct _Eio_File_Char_Ls Eio_File_Char_Ls;
typedef struct _Eio_File_Mkdir Eio_File_Mkdir;
typedef struct _Eio_File_Unlink Eio_File_Unlink;
typedef struct _Eio_File_Stat Eio_File_Stat;
typedef struct _Eio_File_Progress Eio_File_Progress;
typedef struct _Eio_File_Move Eio_File_Move;

typedef struct _Eio_Dir_Copy Eio_Dir_Copy;

struct _Eio_File
{
   Ecore_Thread *thread;
   const void *data;

   int error;

   Eio_Error_Cb error_cb;
   Eio_Done_Cb done_cb;
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

struct _Eio_File_Progress
{
   Eio_File common;

   Eio_Progress_Cb progress_cb;

   const char *source;
   const char *dest;
};

struct _Eio_File_Move
{
   Eio_File_Progress progress;

   Eio_File *copy;
};

struct _Eio_Dir_Copy
{
   Eio_File_Progress progress;

   Eina_List *files;
   Eina_List *dirs;
};

/* Be aware that ecore_thread_run could call cancel_cb if something goes wrong. */
Eina_Bool eio_file_set(Eio_File *common,
		       Eio_Done_Cb done_cb,
		       Eio_Error_Cb error_cb,
		       const void *data,
		       Ecore_Cb job_cb,
		       Ecore_Cb end_cb,
		       Ecore_Cb cancel_cb);

/* Be aware that ecore_thread_run could call cancel_cb if something goes wrong. */
Eina_Bool eio_long_file_set(Eio_File *common,
			    Eio_Done_Cb done_cb,
			    Eio_Error_Cb error_cb,
			    const void *data,
			    Ecore_Thread_Heavy_Cb heavy_cb,
			    Ecore_Thread_Notify_Cb notify_cb,
			    Ecore_Cb end_cb,
			    Ecore_Cb cancel_cb);

void eio_file_error(Eio_File *common);
void eio_file_thread_error(Eio_File *common);

Eio_Progress *eio_progress_malloc(void);
void eio_progress_free(Eio_Progress *progress);

#endif
