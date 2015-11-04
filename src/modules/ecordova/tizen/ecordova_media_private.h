#ifndef _ECORDOVA_MEDIA_PRIVATE_H
#define _ECORDOVA_MEDIA_PRIVATE_H

#include "ecordova_private.h"

#include <player.h>
#include <recorder.h>


typedef struct _Ecordova_Media_Data Ecordova_Media_Data;

/**
 * Ecordova.Media private data
 */
struct _Ecordova_Media_Data
{
   Eo                    *obj;
   player_h               player;
   recorder_h             recorder;
   Ecordova_Media_Status  status;
   Eina_List             *pending;
   Eina_Bool                   record_pending;
};

#endif
