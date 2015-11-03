#ifndef _ECORDOVA_MEDIAFILE_PRIVATE_H
#define _ECORDOVA_MEDIAFILE_PRIVATE_H

#include "ecordova_private.h"

#include <metadata_extractor.h>

typedef struct _Ecordova_MediaFile_Data Ecordova_MediaFile_Data;

/**
 * Ecordova.MediaFile private data
 */
struct _Ecordova_MediaFile_Data
{
   Eo                         *obj;
   metadata_extractor_h        extractor;
   Ecordova_Capture_ErrorCode  error;
   Ecordova_MediaFileData      metadata;
};

#endif
