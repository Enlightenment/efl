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
#endif

EAPI Eina_Bool
ecore_x_input_multi_select(Ecore_X_Window win)
{
#ifdef ECORE_XI2
   int find = 0;
   static int num;
   static XIDeviceInfo *devs = NULL;
   static int checked = 0;
   static int check_ret = 1;
   
   if (!checked)
     {
        int event, error;
        int major = 2, minor = 0;
        
        checked = 1;
        if (!XQueryExtension(_ecore_x_disp, "XInputExtension", 
                             &_ecore_x_xi2_opcode, &event, &error))
          {
             _ecore_x_xi2_opcode = -1;
             check_ret = 0;
             return 0;
          }
   
        if (XIQueryVersion(_ecore_x_disp, &major, &minor) == BadRequest)
          {
             _ecore_x_xi2_opcode = -1;
             check_ret = 0;
             return 0;
          }
        devs = XIQueryDevice(_ecore_x_disp, XIAllDevices, &num);
//        XIFreeDeviceInfo(devs);
     }
   else
     {
        if (!check_ret) return 0;
     }

   if (devs)
     {
        int i;
        
        for (i = 0; i < num; i++)
          {
             XIDeviceInfo *dev = &(devs[i]);
             
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
     }
   
   return find;
#else
   return 0;
#endif
}
