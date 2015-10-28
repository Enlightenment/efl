#ifndef _ECORDOVA_FILE_PRIVATE_H
#define _ECORDOVA_FILE_PRIVATE_H

#include "ecordova_private.h"

typedef struct _Ecordova_File_Data Ecordova_File_Data;

/**
 * Ecordova.File private data
 */
struct _Ecordova_File_Data
{
   Eo            *obj;
   char          *name;
   char          *url;
   char          *type;
   time_t         modified_date;
   long           size;
   long           start;
   long           end;
};

#endif
