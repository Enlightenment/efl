#ifndef _ECORDOVA_DEVICEORIENTATION_PRIVATE_H
#define _ECORDOVA_DEVICEORIENTATION_PRIVATE_H

#include "ecordova_private.h"

#include <sensor.h>

typedef struct _Ecordova_DeviceOrientation_Data Ecordova_DeviceOrientation_Data;

/**
 * Ecordova.DeviceOrientation private data
 */
struct _Ecordova_DeviceOrientation_Data
{
   Eo        *obj;
   sensor_h   sensor;
   Eina_Hash *listeners;
};

#endif
