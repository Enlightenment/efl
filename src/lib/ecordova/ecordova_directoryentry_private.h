#ifndef _ECORDOVA_DIRECTORYENTRY_PRIVATE_H
#define _ECORDOVA_DIRECTORYENTRY_PRIVATE_H

#include "ecordova_private.h"

typedef struct _Ecordova_DirectoryEntry_Data Ecordova_DirectoryEntry_Data;

/**
 * Ecordova.DirectoryEntry private data
 */
struct _Ecordova_DirectoryEntry_Data
{
   Eo *obj;
};

void split_path(const char *, const char *, char **, char **, char **);

#endif
