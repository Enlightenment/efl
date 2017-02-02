#ifndef EEZE_SCANNER_H
#define EEZE_SCANNER_H

#include <Eeze.h>

#define EEZE_SCANNER_EDD_SETUP(edd) \
  EET_DATA_DESCRIPTOR_ADD_BASIC((edd), Eeze_Scanner_Event, "device", device, EET_T_INLINED_STRING); \
  EET_DATA_DESCRIPTOR_ADD_BASIC((edd), Eeze_Scanner_Event, "type", type, EET_T_UINT); \
  EET_DATA_DESCRIPTOR_ADD_BASIC((edd), Eeze_Scanner_Event, "volume", volume, EET_T_UCHAR)

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
   Ecore_Poller *poller;
   const char *device;
   Eina_Bool mounted;
} Eeze_Scanner_Device;

#endif
