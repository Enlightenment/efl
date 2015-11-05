#ifndef _ECORDOVA_DEVICEMOTION_PRIVATE_H
#define _ECORDOVA_DEVICEMOTION_PRIVATE_H

#include "ecordova_private.h"
#include "ecordova_sensor.h"

typedef struct _Ecordova_DeviceMotion_Data Ecordova_DeviceMotion_Data;

/**
 * Ecordova.DeviceMotion private data
 */
struct _Ecordova_DeviceMotion_Data
{
   Eo        *obj;
   sensor_h   sensor;
   Eina_Hash *listeners;
};

#endif
