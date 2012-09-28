#ifndef EEZE_SENSOR_PRIVATE_H
#define EEZE_SENSOR_PRIVATE_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <Eeze_Sensor.h>

#ifndef EEZE_SENSOR_COLOR_DEFAULT
#define EEZE_SENSOR_COLOR_DEFAULT EINA_COLOR_BLUE
#endif
extern int _eeze_sensor_log_dom;
#ifdef CRI
#undef CRI
#endif

#ifdef ERR
#undef ERR
#endif
#ifdef INF
#undef INF
#endif
#ifdef WARN
#undef WARN
#endif
#ifdef DBG
#undef DBG
#endif

#define CRI(...)  EINA_LOG_DOM_CRIT(_eeze_sensor_log_dom, __VA_ARGS__)
#define DBG(...)  EINA_LOG_DOM_DBG(_eeze_sensor_log_dom, __VA_ARGS__)
#define INF(...)  EINA_LOG_DOM_INFO(_eeze_sensor_log_dom, __VA_ARGS__)
#define WARN(...) EINA_LOG_DOM_WARN(_eeze_sensor_log_dom, __VA_ARGS__)
#define ERR(...)  EINA_LOG_DOM_ERR(_eeze_sensor_log_dom, __VA_ARGS__)

/**
 * @typedef Eeze_Sensor
 * @since 1.8
 *
 * Handle for an Eeze_Sensor.
 */
typedef struct _Eeze_Sensor
{
   Eina_Array *modules_array;  /**< Array of available runtime modules */
   Eina_Hash  *modules; /**< Hash with loaded modules */
}  Eeze_Sensor;

/**
 * @typedef Eeze_Sensor_Module;
 * @since 1.8
 *
 * Loadable module data structure.
 */
typedef struct _Eeze_Sensor_Module
{
   Eina_Bool (*init)(void); /**< Pointer to module init function */
   Eina_Bool (*shutdown)(void); /**< Pointer to module shutdown function */
   Eina_Bool (*async_read)(Eeze_Sensor_Type sensor_type, void *user_data); /**< Pointer to module async_read function */
   Eina_Bool (*read)(Eeze_Sensor_Type sensor_type, Eeze_Sensor_Obj *obj); /**< Pointer to module read function */
   Eina_List *sensor_list; /**< List of sensor objects attached to the module */
} Eeze_Sensor_Module;

Eina_Bool eeze_sensor_module_register(const char *name, Eeze_Sensor_Module *mod);
Eina_Bool eeze_sensor_module_unregister(const char *name);
#endif // EEZE_SENSOR_PRIVATE_H
