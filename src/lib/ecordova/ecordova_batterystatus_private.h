#ifndef _ECORDOVA_BATTERYSTATUS_PRIVATE_H
#define _ECORDOVA_BATTERYSTATUS_PRIVATE_H

#include "ecordova_private.h"
#include "ecordova_batterystatus.eo.h"

typedef struct _Ecordova_BatteryStatus_Data Ecordova_BatteryStatus_Data;

/**
 * Ecordova.BatteryStatus private data
 */
struct _Ecordova_BatteryStatus_Data
{
   Eo                               *obj;
   int                               callback_ref_count;
   Ecordova_BatteryStatus_EventInfo *info;
};

#endif
