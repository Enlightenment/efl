#ifndef _ECORDOVA_DIRECTORYREADER_PRIVATE_H
#define _ECORDOVA_DIRECTORYREADER_PRIVATE_H

#include "ecordova_private.h"

typedef struct _Ecordova_DirectoryReader_Data Ecordova_DirectoryReader_Data;

/**
 * Ecordova.DirectoryReader private data
 */
struct _Ecordova_DirectoryReader_Data
{
   Eo        *obj;
   char      *native;
   Eina_List *pending;
};

#endif
