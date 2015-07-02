#ifndef _ECORDOVA_GEOLOCATION_PRIVATE_H
#define _ECORDOVA_GEOLOCATION_PRIVATE_H

#include "ecordova_private.h"

#include <locations.h>

typedef struct _Ecordova_Geolocation_Data Ecordova_Geolocation_Data;

/**
 * Ecordova.Geolocation private data
 */
struct _Ecordova_Geolocation_Data
{
   Eo        *obj;
   Eina_Hash *watchers;
};

#endif
