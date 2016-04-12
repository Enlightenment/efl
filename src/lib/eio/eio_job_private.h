#ifndef _EIO_FILE_PRIVATE_H
#define _EIO_FILE_PRIVATE_H

typedef struct _Eio_Job_Data Eio_Job_Data;

struct _Eio_Job_Data
{
   Eo *object;
   Eina_List *operations;
};

#endif
