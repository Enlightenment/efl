#ifndef _ECORDOVA_FILEREADER_PRIVATE_H
#define _ECORDOVA_FILEREADER_PRIVATE_H

#include "ecordova_private.h"

#include <Eio.h>

typedef struct _Ecordova_FileReader_Data Ecordova_FileReader_Data;

/**
 * Ecordova.FileReader private data
 */
struct _Ecordova_FileReader_Data
{
   Eo                        *obj;
   Ecordova_FileError         error;
   Ecordova_FileReader_State  state;
   char                      *result;
   size_t                     result_length;
   Ecore_Thread              *thread;
   char                      *url;
   long                       offset;
   long                       length;
};

#endif
