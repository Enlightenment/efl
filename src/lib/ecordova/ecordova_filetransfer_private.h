#ifndef _ECORDOVA_FILETRANSFER_PRIVATE_H
#define _ECORDOVA_FILETRANSFER_PRIVATE_H

#include "ecordova_private.h"

#include <Ecore_File.h>

typedef struct _Ecordova_FileTransfer_Data Ecordova_FileTransfer_Data;

typedef struct _Ecordova_FileTransfer_Job Ecordova_FileTransfer_Job;

/**
 * Ecordova.FileTransfer private data
 */
struct _Ecordova_FileTransfer_Data
{
   Eo                        *obj;
   Ecordova_FileTransfer_Job *job;
};

#endif
