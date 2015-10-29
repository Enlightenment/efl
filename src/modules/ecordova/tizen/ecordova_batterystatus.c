#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_batterystatus_private.h"
#include "ecordova_systeminfo.eo.h"

#include <vconf.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS ECORDOVA_BATTERYSTATUS_CLASS
#define MY_CLASS_NAME "Ecordova_BatteryStatus"

#define STATUS_CRITICAL 5
#define STATUS_LOW 20

static Eina_Bool _add_cb(void *, Eo *, const Eo_Event_Description *, void *);
static Eina_Bool _del_cb(void *, Eo *, const Eo_Event_Description *, void *);
static void _start(Ecordova_BatteryStatus_Data *);
static void _stop(Ecordova_BatteryStatus_Data *);
static Eina_Bool _on_battery_changed(void *, Eo *, const Eo_Event_Description *, void *);
static bool _fetch_level(int *);
static bool _fetch_charging_is(Eina_Bool *);

static Eo_Base *
_ecordova_batterystatus_eo_base_constructor(Eo *obj,
                                            Ecordova_BatteryStatus_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_batterystatus_constructor(Eo *obj,
                                    Ecordova_BatteryStatus_Data *pd)
{
   DBG("(%p)", obj);

   eo_do(obj, eo_event_callback_add(EO_EV_CALLBACK_ADD, _add_cb, pd));
   eo_do(obj, eo_event_callback_add(EO_EV_CALLBACK_DEL, _del_cb, pd));
}

static void
_ecordova_batterystatus_eo_base_destructor(Eo *obj,
                                           Ecordova_BatteryStatus_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->callback_ref_count)
     _stop(pd);

   eo_do(obj, eo_event_callback_del(EO_EV_CALLBACK_ADD, _add_cb, pd));
   eo_do(obj, eo_event_callback_del(EO_EV_CALLBACK_DEL, _del_cb, pd));

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Eina_Bool
_add_cb(void *data,
        Eo *obj EINA_UNUSED,
        const Eo_Event_Description *desc,
        void *event_info)
{
   Ecordova_BatteryStatus_Data *pd = (Ecordova_BatteryStatus_Data*)data;
   const Eo_Callback_Array_Item *array = (const Eo_Callback_Array_Item*)event_info;

   for (size_t i = 0; (desc = array[i].desc); ++i)
     {
        if (ECORDOVA_BATTERYSTATUS_EVENT_BATTERY_STATUS == desc ||
            ECORDOVA_BATTERYSTATUS_EVENT_BATTERY_CRITICAL == desc ||
            ECORDOVA_BATTERYSTATUS_EVENT_BATTERY_LOW == desc)
          {
             ++pd->callback_ref_count;
             if (1 == pd->callback_ref_count)
               _start(pd);
          }
     }

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_del_cb(void *data,
        Eo *obj EINA_UNUSED,
        const Eo_Event_Description *desc,
        void *event_info)
{
   Ecordova_BatteryStatus_Data *pd = (Ecordova_BatteryStatus_Data*)data;
   const Eo_Callback_Array_Item *array = (const Eo_Callback_Array_Item*)event_info;

   for (size_t i = 0; (desc = array[i].desc); ++i)
     {
        if (ECORDOVA_BATTERYSTATUS_EVENT_BATTERY_STATUS == desc ||
            ECORDOVA_BATTERYSTATUS_EVENT_BATTERY_CRITICAL == desc ||
            ECORDOVA_BATTERYSTATUS_EVENT_BATTERY_LOW == desc)
           --pd->callback_ref_count;
     }

   if (0 == pd->callback_ref_count)
     _stop(pd);

   return EO_CALLBACK_CONTINUE;
}

static void
_start(Ecordova_BatteryStatus_Data *pd)
{
   eo_do(_ecordova_systeminfo,
     eo_event_callback_add(ECORDOVA_SYSTEMINFO_EVENT_BATTERY_CHANGED,
                           _on_battery_changed,
                           pd));
}

static void
_stop(Ecordova_BatteryStatus_Data *pd)
{
   free(pd->info);
   pd->info = NULL;

   eo_do(_ecordova_systeminfo,
     eo_event_callback_del(ECORDOVA_SYSTEMINFO_EVENT_BATTERY_CHANGED,
                           _on_battery_changed,
                           pd));
}

static bool
_fetch_level(int *level)
{
    int value = 0;
    bool ret = vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CAPACITY, &value) == 0;
    if (ret)
      ERR("%s", "Failed to get battery capacity");

    *level = value;
    return ret;
}

static bool
_fetch_charging_is(Eina_Bool *charging_is)
{
    int value = 0;
    int ret = vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, &value) == 0;
    if (!ret)
      ERR("%s", "Failed to get battery change");

    *charging_is = (0 != value) ? EINA_TRUE : EINA_FALSE;
    return ret;
}

static Eina_Bool
_on_battery_changed(void *data,
                    Eo *obj EINA_UNUSED,
                    const Eo_Event_Description *desc EINA_UNUSED,
                    void *event_info EINA_UNUSED)
{
   Ecordova_BatteryStatus_Data *pd = (Ecordova_BatteryStatus_Data*)data;

   Ecordova_BatteryStatus_EventInfo info;
   if (!_fetch_level(&info.level) ||
       !_fetch_charging_is(&info.plugged_is))
     return EO_CALLBACK_CONTINUE;

   if (!pd->info ||
       pd->info->level != info.level ||
       pd->info->plugged_is != info.plugged_is)
     {
        eo_do(pd->obj, eo_event_callback_call
          (ECORDOVA_BATTERYSTATUS_EVENT_BATTERY_STATUS, &info));

        if (!info.plugged_is)
          {
             if ((!pd->info || pd->info->level > STATUS_CRITICAL) &&
                 info.level <= STATUS_CRITICAL)
               eo_do(pd->obj, eo_event_callback_call
                 (ECORDOVA_BATTERYSTATUS_EVENT_BATTERY_CRITICAL, &info));
             else
             if ((!pd->info || pd->info->level > STATUS_LOW) &&
                 info.level <= STATUS_LOW)
               eo_do(pd->obj, eo_event_callback_call
                 (ECORDOVA_BATTERYSTATUS_EVENT_BATTERY_LOW, &info));
          }

        if (!pd->info)
          pd->info = malloc(sizeof(Ecordova_BatteryStatus_EventInfo));
        *pd->info = info;
     }
   return EO_CALLBACK_CONTINUE;
}

#include "ecordova_batterystatus.eo.c"
