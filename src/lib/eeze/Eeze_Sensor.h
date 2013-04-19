#ifndef EEZE_SENSOR_H
#define EEZE_SENSOR_H

#ifdef EAPI
# undef EAPI
#endif

#ifdef __GNUC__
# if __GNUC__ >= 4
#  define EAPI __attribute__ ((visibility("default")))
# else
#  define EAPI
# endif
#else
# define EAPI
#endif

#include <Eina.h>

/**
 * @file Eeze_Sensor.h
 * @brief Sensor information subsystem
 *
 * Eeze sensor functions allow you to gather sensor information from different
 * sensor sources available on your hardware. It supports a plugin architecture
 * to support different hardware platforms and devices. These plugins can be
 * loaded at runtime.
 *
 * Right now we have support for the Tizen sensor framework as well as a simple
 * fake plugin to be used as a test harness for development. Plugins for other
 * sensor frameworks or platforms are always welcome.
 *
 * Synchronous as well as asynchronous reads are possible. As reading the
 * physical sensor might be bound to a high latency and thus cost the value of
 * the last read is cached within Eeze_Sensor together with a timestamp of the
 * actual read out. This can speed up the value access for application while the
 * values are still getting updating asynchronously in the background. The
 * timestamp should be checked if the specific time requirements are needed.
 *
 * As an alternative the sensor could be read synchronously. With the
 * disadvantage that the function call will block until the data is read from
 * the sensor.
 *
 * @since 1.8
 *
 * @addtogroup Eeze_Sensor Sensor
 * Gather sensor information from different sources. Works based on plugins,
 * with a Tizen plugin being available.
 *
 * @ingroup Eeze
 * @{
 */

/**
 * @enum Eeze_Sensor_Type
 *
 * All sensor types known by Eeze Sensor. This list of types include real
 * physical types like proximity or light as well as "aggregated" types like
 * facedown or doubletap. All types with MOTION in their name can be used as
 * real events coming from the underlying system. This is not supported on all
 * systems.
 *
 * @since 1.8
 */
typedef enum
{
   EEZE_SENSOR_TYPE_ACCELEROMETER, /**< Accelerometer sensor */
   EEZE_SENSOR_TYPE_GRAVITY, /**< Gravity sensor */
   EEZE_SENSOR_TYPE_LINEAR_ACCELERATION, /**<  */
   EEZE_SENSOR_TYPE_DEVICE_ORIENTATION, /**<  */
   EEZE_SENSOR_TYPE_MAGNETIC, /**< Magnetic sensor */
   EEZE_SENSOR_TYPE_ORIENTATION, /**< Orientation sensor */
   EEZE_SENSOR_TYPE_GYROSCOPE, /**< Gyroscope sensor */
   EEZE_SENSOR_TYPE_LIGHT, /**< Light sensor */
   EEZE_SENSOR_TYPE_PROXIMITY, /**< Proximity sensor */
   EEZE_SENSOR_TYPE_MOTION_SNAP, /**< Snap motion sensor */
   EEZE_SENSOR_TYPE_MOTION_SHAKE, /**< Shake motion sensor */
   EEZE_SENSOR_TYPE_MOTION_DOUBLETAP, /**< Doubletap motion sensor */
   EEZE_SENSOR_TYPE_MOTION_PANNING, /**< Panning motion sensor */
   EEZE_SENSOR_TYPE_MOTION_PANNING_BROWSE, /**< */
   EEZE_SENSOR_TYPE_MOTION_TILT, /**< */
   EEZE_SENSOR_TYPE_MOTION_FACEDOWN, /**< Facedown motion sensor */
   EEZE_SENSOR_TYPE_MOTION_DIRECT_CALL, /**< */
   EEZE_SENSOR_TYPE_MOTION_SMART_ALERT, /**< */
   EEZE_SENSOR_TYPE_MOTION_NO_MOVE, /**< */
   /* Non-Tizen from here */
   EEZE_SENSOR_TYPE_BAROMETER, /**< Barometer sensor */
   EEZE_SENSOR_TYPE_TEMPERATURE, /**< Temperature sensor */
   EEZE_SENSOR_TYPE_LAST = 0xFF /**< Last item to mark end of enum */
}  Eeze_Sensor_Type;

/**
 * @defgroup Eeze_Sensor_Events Available eeze sensor events
 * @brief Sensor events that are emitted from the library as ecore events
 * @ingroup Eeze
 *
 * Event types used to register ecore_event_handler on. These events are used
 * for #eeze_sensor_async_read to deliver read out data. It is also used for
 * generated events like facedown or shake. Subscribing to these events in your
 * application allowsyou to react on these changes in an efficient way without
 * polling for new updates and wasting power and computing cycles.
 *
 * @since 1.8
 * @{
 */
EAPI int EEZE_SENSOR_EVENT_ACCELEROMETER;
EAPI int EEZE_SENSOR_EVENT_GRAVITY;
EAPI int EEZE_SENSOR_EVENT_LINEAR_ACCELERATION;
EAPI int EEZE_SENSOR_EVENT_DEVICE_ORIENTATION;
EAPI int EEZE_SENSOR_EVENT_MAGNETIC;
EAPI int EEZE_SENSOR_EVENT_ORIENTATION;
EAPI int EEZE_SENSOR_EVENT_GYROSCOPE;
EAPI int EEZE_SENSOR_EVENT_LIGHT;
EAPI int EEZE_SENSOR_EVENT_PROXIMITY;
EAPI int EEZE_SENSOR_EVENT_SNAP;
EAPI int EEZE_SENSOR_EVENT_SHAKE;
EAPI int EEZE_SENSOR_EVENT_DOUBLETAP;
EAPI int EEZE_SENSOR_EVENT_PANNING;
EAPI int EEZE_SENSOR_EVENT_PANNING_BROWSE;
EAPI int EEZE_SENSOR_EVENT_TILT;
EAPI int EEZE_SENSOR_EVENT_FACEDOWN;
EAPI int EEZE_SENSOR_EVENT_DIRECT_CALL;
EAPI int EEZE_SENSOR_EVENT_SMART_ALERT;
EAPI int EEZE_SENSOR_EVENT_NO_MOVE;
EAPI int EEZE_SENSOR_EVENT_BAROMETER;
EAPI int EEZE_SENSOR_EVENT_TEMPERATURE;
/**@}*/

/**
 * @typedef Eeze_Sensor_Obj;
 *
 * Object for a sensor type. Keeps information about the type and holds the
 * data for the accessor functions. As this information gets also updated by
 * asynchronous reads it might be a good idea to check the timestamp value to
 * see when the data has been updated. The timestamp is given as floating point
 * value in seconds.
 *
 * You are not supposed to access the raw data values from here but use the
 * getter functions for it. Using the raw values from this struct might break
 * your applications later if the internal structure changes.
 *
 * @since 1.8
 */
typedef struct _Eeze_Sensor_Obj
{
   Eeze_Sensor_Type type; /**< Sensor type see #Eeze_Sensor_Type */
   int   accuracy; /**< Accuracy of the sensor value */
   float data[3]; /**< Sensor data depending on the sensor type */
   double timestamp; /**< Timestamp of data read */
   Eina_Bool continuous_flow; /**< FUTURE USE: Continuous flow of sensor read out */
   void *user_data; /**< Data pointer used for passing data to the asynchronous callback */
} Eeze_Sensor_Obj;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create sensor object to operate on.
 * @param type Sensor type to create object from.
 * @return Sensor object for the given type.
 *
 * Takes the sensor type and create an object for it to operate on. During this
 * it also does an initial sensor data read to fill the sensor data into the
 * object. The #eeze_sensor_free function must be used to destroy the object
 * and release its memory.
 *
 * For every sensor type you want to work with this is the first thing you have
 * to do. Create the object from the type and everything else the operates on
 * this object.
 *
 * This also takes into account what runtime modules are loaded and handles
 * them in a given priority to pick up the best sensor source for your sensor
 * object.
 *
 * @since 1.8
 */
EAPI Eeze_Sensor_Obj *eeze_sensor_new(Eeze_Sensor_Type type);

/**
 * @brief Free a sensor object.
 * @param sens Sensor object to operate on.
 *
 * Free an sensor object when it is no longer needed. Always use this function
 * to cleanup unused sensor objects.
 *
 * @since 1.8
 */
EAPI void eeze_sensor_free(Eeze_Sensor_Obj *sens);

/**
 * @brief Get accuracy from sensor object.
 * @param sens Sensor object to operate on.
 * @param accuracy Pointer to write accuracy value into.
 * @return EINA_TRUE for success and EINA_FALSE for failure
 *
 * Access function to get the accuracy property from the sensor object. The
 * accuracy value can have the following values and meaning:
 * -1 Undefined accuracy
 * 0 Bad accurancy
 * 1 Normal accuracy
 * 2 Good accuracy
 * 3 Very good accuracy
 *
 * @since 1.8
 */
EAPI Eina_Bool eeze_sensor_accuracy_get(Eeze_Sensor_Obj *sens, int *accuracy);

/**
 * @brief Get data from all three data properties
 * @param sens Sensor object to operate on.
 * @param x Pointer to write first data property value into.
 * @param y Pointer to write second data property value into.
 * @param z Pointer to write third data property value into.
 * @return EINA_TRUE for success and EINA_FALSE for failure
 *
 * Access function to get all three data properties from the sensor object.
 * This is used for sensor types that offer all three values. Like accelerometer
 * and magnetic.
 *
 * @since 1.8
 */
EAPI Eina_Bool eeze_sensor_xyz_get(Eeze_Sensor_Obj *sens, float *x, float *y, float *z);

/**
 * @brief Get data from first two data properties
 * @param sens Sensor object to operate on.
 * @param x Pointer to write first data property value into.
 * @param y Pointer to write second data property value into.
 * @return EINA_TRUE for success and EINA_FALSE for failure
 *
 * Access function to get the first two data properties from the sensor object.
 * This is used for sensor types that offer two values. Like panning.
 *
 * @since 1.8
 */
EAPI Eina_Bool eeze_sensor_xy_get(Eeze_Sensor_Obj *sens, float *x, float *y);

/**
 * @brief Get the data from  first data property
 * @param sens Sensor object to operate on.
 * @param x Pointer to write first data property value into.
 * @return EINA_TRUE for success and EINA_FALSE for failure
 *
 * Access function to get the first data property from the sensor object. This
 * is used for sensor types that only offer one value. Like light or proximity.
 *
 * @since 1.8
 */
EAPI Eina_Bool eeze_sensor_x_get(Eeze_Sensor_Obj *sens, float *x);

/**
 * @brief Get timestamp from sensor object.
 * @param sens Sensor object to operate on.
 * @param timestamp Pointer to write timestamp value into.
 * @return EINA_TRUE for success and EINA_FALSE for failure
 *
 * Access function to get the timestamp property from the sensor object. It
 * allows you to determine if the values have been updated since the last time
 * you requested them. Timestamp is given as a floating point value in seconds.
 *
 * @since 1.8
 */
EAPI Eina_Bool eeze_sensor_timestamp_get(Eeze_Sensor_Obj *sens, double *timestamp);

/**
 * @brief Read out sensor data
 * @param sens Sensor object to operate on.
 * @return EINA_TRUE for success and EINA_FALSE for failure
 *
 * This function reads sensor data from the device and fills the sensor object
 * with the data. This call is synchronous and blocks until the data is read out
 * and updated in the sensor object. For simple applications this is fine and
 * the easiest way to use the API. A more efficient way is to use
 * #eeze_sensor_async_read which allows the sensor readout to happen in the
 * background and the application would check the timestamp of the data to
 * determine how recent the data is.
 *
 * @since 1.8
 */
EAPI Eina_Bool eeze_sensor_read(Eeze_Sensor_Obj *sens);

/**
 * @brief Asynchronous read out sensor data
 * @param sens Sensor object to operate on.
 * @param user_data Data to pass to the callback function.
 * @return EINA_TRUE for success and EINA_FALSE for failure
 *
 * This function reads sensor data from the device and fills the sensor object
 * with the data. The read is done asynchronously and thus does not block after
 * calling. Instead the given the application can determine how recent the
 * values are from the timestamp value that can be accessed through
 * #eeze_sensor_timestamp_get.
 *
 * This function is more efficient but needs a bit more work in the application.
 * An easier way is to use the synchronous #eeze_sensor_read functions. The
 * downside of it is that it blocks until the data was read out from the
 * physical sensor. That might be a long time depending on the hardware and its
 * interface.
 *
 * The extra data passed in as user_data here will be available in the user_data
 * pointer of the sensor object when the ecore event arrives.
 *
 * @since 1.8
 */
EAPI Eina_Bool       eeze_sensor_async_read(Eeze_Sensor_Obj *sens, void *user_data);

#ifdef __cplusplus
}
#endif
/** @} */
#endif
