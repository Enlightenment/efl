#ifndef _ECORDOVA_ENTRY_PRIVATE_H
#define _ECORDOVA_ENTRY_PRIVATE_H

//#include "ecordova_private.h"

#include <Eio.h>

#include <stdbool.h>

typedef struct _Ecordova_Entry_Data Ecordova_Entry_Data;

/**
 * Ecordova.Entry private data
 */
struct _Ecordova_Entry_Data
{
   Eo                  *obj;
   Eina_Bool            is_file;
   Eina_Bool            is_directory;
   char                *name;
   char                *path;
   Ecordova_FileSystem *file_system;
   char                *native;
   Eina_List           *pending;
};

typedef void(*Ecordova_Entry_Success_Callback)(Eo *, void *);
typedef void(*Ecordova_Entry_Error_Callback)(Eo *, int);

typedef struct
{
  Ecordova_Entry_Data             *pd;
  Eio_File                        *file;
  char                            *path;
  char                            *name;
  char                            *native;
  Ecordova_Entry_Success_Callback  success_cb;
  Ecordova_Entry_Error_Callback    error_cb;
  unsigned long                    count;
} Eio_Operation_Data;

Eio_Operation_Data *_data_new(Ecordova_Entry_Data *, Ecordova_Entry_Success_Callback, Ecordova_Entry_Error_Callback);
void _data_free(Eio_Operation_Data *);
void _metadata_get(Ecordova_Entry_Data *, Ecordova_Entry_Success_Callback, Ecordova_Entry_Error_Callback);
void _entry_remove(Ecordova_Entry_Data *, Ecordova_Entry_Success_Callback, Ecordova_Entry_Error_Callback, bool);
void _error_notify(Eo *, int);
void _eio_error_cb(void *, Eio_File *, int);
void _eio_removed_cb(void *, Eio_File *);
Eina_Bool _eio_filter_cb(void *, Eio_File *, const Eina_File_Direct_Info *);
void _eio_progress_cb(void *, Eio_File *, const Eio_Progress *);
Ecordova_FileError _translate_errno(int);

#endif
