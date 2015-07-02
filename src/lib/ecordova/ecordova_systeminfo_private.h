#ifndef _ECORDOVA_SYSTEMINFO_PRIVATE_H
#define _ECORDOVA_SYSTEMINFO_PRIVATE_H

#include "ecordova_private.h"
#include "ecordova_systeminfo.eo.h"

typedef struct _Ecordova_SystemInfo_Data Ecordova_SystemInfo_Data;

/**
 * Ecordova.SystemInfo private data
 */
struct _Ecordova_SystemInfo_Data
{
   Eo                               *obj;
   int                               ref_count;
   bool                              info_initialized;
};

#endif
