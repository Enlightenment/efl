/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

#ifdef ECORE_XI2
int _ecore_x_xi2_opcode = -1;

static XIDeviceInfo *_ecore_x_xi2_devs = NULL;
static int _ecore_x_xi2_num = 0;
#endif

void
_ecore_x_input_init(void)
{
#ifdef ECORE_XI2
   int event, error;
   int major = 2, minor = 0;
   
   if (!XQueryExtension(_ecore_x_disp, "XInputExtension", 
                        &_ecore_x_xi2_opcode, &event, &error))
     {
        _ecore_x_xi2_opcode = -1;
        return;
     }
   
   if (XIQueryVersion(_ecore_x_disp, &major, &minor) == BadRequest)
     {
        _ecore_x_xi2_opcode = -1;
        return;
     }
   _ecore_x_xi2_devs = XIQueryDevice(_ecore_x_disp, XIAllDevices, 
                                     &_ecore_x_xi2_num);
#endif   
}

void
_ecore_x_input_shutdown(void)
{
#ifdef ECORE_XI2
   if (_ecore_x_xi2_devs)
     {
        XIFreeDeviceInfo(_ecore_x_xi2_devs);
        _ecore_x_xi2_devs = NULL;
     }
   _ecore_x_xi2_num = 0;
   _ecore_x_xi2_opcode = -1;
#endif   
}

EAPI Eina_Bool
ecore_x_input_multi_select(Ecore_X_Window win)
{
#ifdef ECORE_XI2
   int i, find = 0;

   if (!_ecore_x_xi2_devs) return 0;

   for (i = 0; i < _ecore_x_xi2_num; i++)
     {
        XIDeviceInfo *dev = &(_ecore_x_xi2_devs[i]);

        if (dev->use == XIFloatingSlave)
          {
             XIEventMask eventmask;
             unsigned char mask[1] = { 0 };

             eventmask.deviceid = dev->deviceid;
             eventmask.mask_len = sizeof(mask);
             eventmask.mask = mask;
             XISetMask(mask, XI_ButtonPress);
             XISetMask(mask, XI_ButtonRelease);
             XISetMask(mask, XI_Motion);
             XISelectEvents(_ecore_x_disp, win, &eventmask, 1);
             find = 1;
          }
     }

   return find;
#else
   return 0;
#endif
}
