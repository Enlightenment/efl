#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <sys/time.h>

#include <Eina.h>
#include <Ecore.h>
#include <Eeze.h>
#include <Eeze_Sensor.h>
#include "eeze_sensor_private.h"

static int _eeze_sensor_udev_log_dom = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...)  EINA_LOG_DOM_ERR(_eeze_sensor_udev_log_dom, __VA_ARGS__)

static Eeze_Sensor_Module *esensor_module;

static Eina_List *devices;

static void
_dummy_free(void *user_data EINA_UNUSED, void *func_data EINA_UNUSED)
{
/* Don't free the event data after dispatching the event. We keep track of
 * it on our own
 */
}

static Eina_Bool
udev_init(void)
{
   /* We only offer a temperature sensor right now */
    Eeze_Sensor_Obj *obj = calloc(1, sizeof(Eeze_Sensor_Obj));
    obj->type = EEZE_SENSOR_TYPE_TEMPERATURE;
    esensor_module->sensor_list = eina_list_append(esensor_module->sensor_list, obj);

   devices = eeze_udev_find_by_type(EEZE_UDEV_TYPE_IS_IT_HOT_OR_IS_IT_COLD_SENSOR, NULL);

   return EINA_TRUE;
}

static Eina_Bool
udev_shutdown(void)
{
   char *data;

   EINA_LIST_FREE(devices, data) eina_stringshare_del(data);

   return EINA_TRUE;
}

static double
_udev_read(void)
{
   Eina_List *l;
   double cur, temp;
   char *syspath;
   const char *test;
   char buf[256];
   int x, y, sensors = 0;
   temp = -274.0; /* Absolute zero is -273.15° C */

   if (eina_list_count(devices))
     {
        temp = 0.0;
        EINA_LIST_FOREACH(devices, l, syspath)
          {
             for (x = 1, y = 0; x < 15; x++)
               {
                  if (y >= 2) break;
                  sprintf(buf, "temp%d_input", x);
                  if ((test = eeze_udev_syspath_get_sysattr(syspath, buf)))
                    {
                       y = 0;
                       cur = strtod(test, NULL);
                       if (cur > -274.0)
                         {
                            /* temp is in celcius * 1000 */
                            temp += (cur / 1000);
                            sensors++;
                         }
                    }
                  /* keep checking for temp sensors until we get 2 in a row that don't exist */
                  else y++;
               }
           }
        if (sensors) temp /= (double)sensors;
   }

   return temp;
}

static Eina_Bool
udev_read(Eeze_Sensor_Obj *obj)
{
   switch (obj->type)
     {
      case EEZE_SENSOR_TYPE_TEMPERATURE:
        obj->accuracy = -1;
        obj->data[0] = _udev_read();
        obj->data[1] = 0;
        obj->data[2] = 0;
        obj->timestamp = ecore_time_get();
        break;

      default:
        ERR("Not possible to read from this sensor type.");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Eina_Bool
udev_async_read(Eeze_Sensor_Obj *obj, void *user_data)
{
   if (user_data) obj->user_data = user_data;

   switch (obj->type)
     {
      case EEZE_SENSOR_TYPE_TEMPERATURE:
        obj->accuracy = -1;
        obj->data[0] = _udev_read();
        obj->data[1] = 0;
        obj->data[2] = 0;
        obj->timestamp = ecore_time_get();
        ecore_event_add(EEZE_SENSOR_EVENT_TEMPERATURE, obj, _dummy_free, NULL);
        break;

      default:
        ERR("Not possible to read from this sensor type.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/* This function gets called when the module is loaded from the disk. Its the
 * entry point to anything in this module. After setting ourself up we register
 * into the core of eeze sensor to make our functionality available.
 */
static Eina_Bool
sensor_udev_init(void)
{

   _eeze_sensor_udev_log_dom = eina_log_domain_register("eeze_sensor_udev", EINA_COLOR_BLUE);
   if (_eeze_sensor_udev_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register 'eeze_sensor_udev' log domain.");
        return EINA_FALSE;
     }

   /* Check to avoid multi-init */
   if (esensor_module) return EINA_FALSE;

   /* Set module function pointer to allow calls into the module */
   esensor_module = calloc(1, sizeof(Eeze_Sensor_Module));
   if (!esensor_module) return EINA_FALSE;

   /* Setup our function pointers to allow the core accessing this modules
    * functions
    */
   esensor_module->init = udev_init;
   esensor_module->shutdown = udev_shutdown;
   esensor_module->read = udev_read;
   esensor_module->async_read = udev_async_read;

   if (!eeze_sensor_module_register("udev", esensor_module))
     {
        ERR("Failed to register udev module.");
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

/* Cleanup when the module gets unloaded. Unregister ourself from the core to
 * avoid calls into a not loaded module.
 */
static void
sensor_udev_shutdown(void)
{
   Eeze_Sensor_Obj *sens;

   eeze_sensor_module_unregister("udev");
   EINA_LIST_FREE(esensor_module->sensor_list, sens) free(sens);

   eina_log_domain_unregister(_eeze_sensor_udev_log_dom);

   free(esensor_module);
   esensor_module = NULL;

   _eeze_sensor_udev_log_dom = -1;
}

EINA_MODULE_INIT(sensor_udev_init);
EINA_MODULE_SHUTDOWN(sensor_udev_shutdown);
