#ifndef _ECORDOVA_NETWORKINFORMATION_PRIVATE_H
#define _ECORDOVA_NETWORKINFORMATION_PRIVATE_H

#include "ecordova_private.h"

#include <net_connection.h>

typedef struct _Ecordova_NetworkInformation_Data Ecordova_NetworkInformation_Data;

/**
 * Ecordova.NetworkInformation private data
 */
struct _Ecordova_NetworkInformation_Data
{
   Eo           *obj;
   connection_h  connection;
};

#endif
