#ifndef EEZE_SCANNER_H
#define EEZE_SCANNER_H

#include <Eeze.h>

typedef enum
{
   EEZE_SCANNER_EVENT_TYPE_NONE,
   EEZE_SCANNER_EVENT_TYPE_ADD = EEZE_UDEV_EVENT_ADD,
   EEZE_SCANNER_EVENT_TYPE_REMOVE = EEZE_UDEV_EVENT_REMOVE,
   EEZE_SCANNER_EVENT_TYPE_CHANGE = EEZE_UDEV_EVENT_CHANGE
} Eeze_Scanner_Event_Type;

typedef struct
{
   const char *device;
   Eeze_Scanner_Event_Type type;
   Eina_Bool volume;
} Eeze_Scanner_Event;

typedef struct
{
   const char *device;
   Eina_Bool mounted : 1;
} Eeze_Scanner_Device;

#endif
