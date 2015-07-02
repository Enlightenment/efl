#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_systeminfo_private.h"

#include <vconf.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS ECORDOVA_SYSTEMINFO_CLASS
#define MY_CLASS_NAME "Ecordova_SystemInfo"

static Eina_Bool _add_cb(void *, Eo *, const Eo_Event_Description *, void *);
static Eina_Bool _del_cb(void *, Eo *, const Eo_Event_Description *, void *);
static void _battery_callback_add(Ecordova_SystemInfo_Data *);
static void _battery_callback_del(Ecordova_SystemInfo_Data *);
static void _on_battery_changed(keynode_t *, void *);

static Eo *
_ecordova_systeminfo_eo_base_constructor(Eo *obj,
                                         Ecordova_SystemInfo_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_systeminfo_constructor(Eo *obj,
                                 Ecordova_SystemInfo_Data *pd)
{
   DBG("(%p)", obj);

   eo_do(obj, eo_event_callback_add(EO_EV_CALLBACK_ADD, _add_cb, pd));
   eo_do(obj, eo_event_callback_add(EO_EV_CALLBACK_DEL, _del_cb, pd));
}

static void
_ecordova_systeminfo_eo_base_destructor(Eo *obj,
                                        Ecordova_SystemInfo_Data *pd)
{
   DBG("(%p)", obj);

   if (pd->ref_count > 0)
     _battery_callback_del(pd);

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
   Ecordova_SystemInfo_Data *pd = (Ecordova_SystemInfo_Data*)data;
   const Eo_Callback_Array_Item *array = (const Eo_Callback_Array_Item*)event_info;

   for (size_t i = 0; (desc = array[i].desc); ++i)
     {
        if (ECORDOVA_SYSTEMINFO_EVENT_BATTERY_CHANGED != desc) continue;

        ++pd->ref_count;
        if (1 == pd->ref_count)
          _battery_callback_add(pd);
     }

   return EO_CALLBACK_CONTINUE;
}

static Eina_Bool
_del_cb(void *data,
        Eo *obj EINA_UNUSED,
        const Eo_Event_Description *desc,
        void *event_info)
{
   Ecordova_SystemInfo_Data *pd = (Ecordova_SystemInfo_Data*)data;
   const Eo_Callback_Array_Item *array = (const Eo_Callback_Array_Item*)event_info;

   for (size_t i = 0; (desc = array[i].desc); ++i)
     {
        if (ECORDOVA_SYSTEMINFO_EVENT_BATTERY_CHANGED != desc) continue;

        --pd->ref_count;
        if (0 == pd->ref_count)
          _battery_callback_del(pd);
     }

   return EO_CALLBACK_CONTINUE;
}

static void
_register_vconf_callback(Ecordova_SystemInfo_Data *pd,
                         const char *in_key,
                         vconf_callback_fn cb)
{
   if (0 != vconf_notify_key_changed(in_key, cb, pd))
     ERR("%s, %s", "Failed to register vconf callback", in_key);
}

static void
_unregister_vconf_callback(const char *in_key,
                           vconf_callback_fn cb)
{
   if (0 != vconf_ignore_key_changed(in_key, cb))
     ERR("%s, %s", "Failed to unregister vconf callback", in_key);
}

static void
_battery_callback_add(Ecordova_SystemInfo_Data *pd)
{
   _register_vconf_callback(pd, VCONFKEY_SYSMAN_BATTERY_CAPACITY, _on_battery_changed);
   _register_vconf_callback(pd, VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, _on_battery_changed);
}

static void
_battery_callback_del(Ecordova_SystemInfo_Data *pd EINA_UNUSED)
{
   _unregister_vconf_callback(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, _on_battery_changed);
   _unregister_vconf_callback(VCONFKEY_SYSMAN_BATTERY_CAPACITY, _on_battery_changed);
}

static void
_on_battery_changed(keynode_t *node EINA_UNUSED, void *data)
{
   Ecordova_SystemInfo_Data *pd = (Ecordova_SystemInfo_Data*)data;
   eo_do(pd->obj, eo_event_callback_call
     (ECORDOVA_SYSTEMINFO_EVENT_BATTERY_CHANGED, NULL));
}

#include "ecordova_systeminfo.eo.c"
