#ifndef _ECORDOVA_MEDIA_PRIVATE_H
#define _ECORDOVA_MEDIA_PRIVATE_H

#include "ecordova_private.h"

#include <player.h>
#include <recorder.h>

#include <stdbool.h>

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
   bool                   record_pending;
};

#endif
