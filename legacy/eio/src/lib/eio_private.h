#ifndef EIO_PRIVATE_H_
#define EIO_PRIVATE_H_

#include <Ecore.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Eio.h"

typedef struct _Eio_File_Ls Eio_File_Ls;
typedef struct _Eio_File_Direct_Ls Eio_File_Direct_Ls;
typedef struct _Eio_File_Char_Ls Eio_File_Char_Ls;
typedef struct _Eio_File_Mkdir Eio_File_Mkdir;
typedef struct _Eio_File_Unlink Eio_File_Unlink;
typedef struct _Eio_File_Stat Eio_File_Stat;

struct _Eio_File
{
   Ecore_Thread *thread;
   const void *data;

   Eio_Done_Cb done_cb;
   Eio_Done_Cb error_cb;
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

Eina_Bool eio_file_set(Eio_File *common,
		       Eio_Done_Cb done_cb,
		       Eio_Done_Cb error_cb,
		       const void *data,
		       Ecore_Cb job_cb,
		       Ecore_Cb end_cb,
		       Ecore_Cb cancel_cb);

Eina_Bool eio_long_file_set(Eio_File *common,
			    Eio_Done_Cb done_cb,
			    Eio_Done_Cb error_cb,
			    const void *data,
			    Ecore_Thread_Heavy_Cb heavy_cb,
			    Ecore_Thread_Notify_Cb notify_cb,
			    Ecore_Cb end_cb,
			    Ecore_Cb cancel_cb);

void eio_file_error(Eio_File *common);
void eio_file_thread_error(Eio_File *common);

#endif
