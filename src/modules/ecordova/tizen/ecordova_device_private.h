#ifndef _ECORDOVA_DEVICE_PRIVATE_H
#define _ECORDOVA_DEVICE_PRIVATE_H

#include "ecordova_private.h"

typedef struct _Ecordova_Device_Data Ecordova_Device_Data;

/**
 * Ecordova.Device private data
 */
struct _Ecordova_Device_Data
{
   Eo   *obj;
   char *uuid;
   char *version;
   char *model;
   char *platform;
};

#endif
