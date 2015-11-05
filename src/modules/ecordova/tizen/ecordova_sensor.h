#ifndef ECORDOVA_SENSOR_H
#define ECORDOVA_SENSOR_H

#include "ecordova_tizen.h"

#define MAX_VALUE_SIZE 16

typedef void* sensor_h;
typedef void* sensor_listener_h;

typedef struct
{
	int accuracy;                  /**< Accuracy */
	unsigned long long timestamp;  /**< Timestamp */
	int value_count;               /**< Count of values */
	float values[MAX_VALUE_SIZE];  /**< Sensor values */
} sensor_event_s;

typedef enum
{
	SENSOR_ERROR_NONE                  = TIZEN_ERROR_NONE,                 /**< Successful */
	SENSOR_ERROR_IO_ERROR              = TIZEN_ERROR_IO_ERROR,             /**< I/O error */
	SENSOR_ERROR_INVALID_PARAMETER     = TIZEN_ERROR_INVALID_PARAMETER,    /**< Invalid parameter */
	SENSOR_ERROR_NOT_SUPPORTED         = TIZEN_ERROR_NOT_SUPPORTED,        /**< Unsupported sensor in the current device */
	SENSOR_ERROR_PERMISSION_DENIED     = TIZEN_ERROR_PERMISSION_DENIED,    /**< Permission denied */
	SENSOR_ERROR_OUT_OF_MEMORY         = TIZEN_ERROR_OUT_OF_MEMORY,        /**< Out of memory */
	SENSOR_ERROR_NOT_NEED_CALIBRATION  = TIZEN_ERROR_SENSOR | 0x03,        /**< Sensor doesn't need calibration */
	SENSOR_ERROR_OPERATION_FAILED      = TIZEN_ERROR_SENSOR | 0x06,        /**< Operation failed */
} sensor_error_e;

typedef enum
{
	SENSOR_ALL = -1,                         /**< All sensors */
	SENSOR_ACCELEROMETER,                    /**< Accelerometer */
	SENSOR_GRAVITY,                          /**< Gravity sensor */
	SENSOR_LINEAR_ACCELERATION,              /**< Linear acceleration sensor */
	SENSOR_MAGNETIC,                         /**< Magnetic sensor */
	SENSOR_ROTATION_VECTOR,                  /**< Rotation Vector sensor */
	SENSOR_ORIENTATION,                      /**< Orientation sensor */
	SENSOR_GYROSCOPE,                        /**< Gyroscope sensor */
	SENSOR_LIGHT,                            /**< Light sensor */
	SENSOR_PROXIMITY,                        /**< Proximity sensor */
	SENSOR_PRESSURE,                         /**< Pressure sensor */
	SENSOR_ULTRAVIOLET,                      /**< Ultraviolet sensor */
	SENSOR_TEMPERATURE,                      /**< Temperature sensor */
	SENSOR_HUMIDITY,                         /**< Humidity sensor */
	SENSOR_HRM,                              /**< Heart Rate Monitor sensor @if MOBILE (Since Tizen 2.3.1) @endif */
	SENSOR_HRM_LED_GREEN,                    /**< HRM (LED Green) sensor @if MOBILE (Since Tizen 2.3.1) @endif */
	SENSOR_HRM_LED_IR,                       /**< HRM (LED IR) sensor @if MOBILE (Since Tizen 2.3.1) @endif */
	SENSOR_HRM_LED_RED,                      /**< HRM (LED RED) sensor @if MOBILE (Since Tizen 2.3.1) @endif */
	SENSOR_GYROSCOPE_UNCALIBRATED,           /**< Uncalibrated Gyroscope sensor (Since Tizen 2.4) */
	SENSOR_GEOMAGNETIC_UNCALIBRATED,         /**< Uncalibrated Geomagnetic sensor (Since Tizen 2.4) */
	SENSOR_GYROSCOPE_ROTATION_VECTOR,        /**< Gyroscope-based rotation vector sensor (Since Tizen 2.4) */
	SENSOR_GEOMAGNETIC_ROTATION_VECTOR,      /**< Geomagnetic-based rotation vector sensor (Since Tizen 2.4) */
	SENSOR_LAST,                             /**< End of sensor enum values */
	SENSOR_CUSTOM = 10000                    /**< Custom sensor */
} sensor_type_e;

typedef void (*sensor_event_cb)(sensor_h sensor, sensor_event_s *event, void *data);

extern int (*sensor_listener_start)(sensor_listener_h listener);
extern int (*sensor_listener_read_data)(sensor_listener_h listener, sensor_event_s *event);
extern int (*sensor_destroy_listener)(sensor_listener_h listener);
extern int (*sensor_listener_set_event_cb)(sensor_listener_h listener, unsigned int interval_ms, sensor_event_cb callback, void *data);
extern int (*sensor_get_default_sensor)(sensor_type_e type, sensor_h *sensor);
extern int (*sensor_create_listener)(sensor_h sensor, sensor_listener_h *listener);


#endif
