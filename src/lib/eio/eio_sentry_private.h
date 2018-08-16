#ifndef _EIO_SENTRY_PRIVATE_H
#define _EIO_SENTRY_PRIVATE_H

typedef struct _Eio_Sentry_Data Eio_Sentry_Data;

struct _Eio_Sentry_Data
{
   Eo *object;
   Eina_Hash *targets;
   Eina_Array *handlers;
};

#endif
