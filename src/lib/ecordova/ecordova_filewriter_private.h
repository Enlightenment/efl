#ifndef _ECORDOVA_FILEWRITER_PRIVATE_H
#define _ECORDOVA_FILEWRITER_PRIVATE_H

#include "ecordova_private.h"

typedef struct _Ecordova_FileWriter_Data Ecordova_FileWriter_Data;

/**
 * Ecordova.FileWriter private data
 */
struct _Ecordova_FileWriter_Data
{
   Eo                        *obj;
   Ecordova_FileError         error;
   Ecordova_FileWriter_State  state;
   char                      *url;
   long                       offset;
   long                       length;
   FILE                      *stream;
   char                      *data;
   long                       data_size;
   long                       truncate_size;
   Ecore_Thread              *thread;
};

#endif
