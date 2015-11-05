#ifndef ECORDOVA_TIZEN_H
#define ECORDOVA_TIZEN_H

extern int _ecordova_log_dom;

#define CRI(...) EINA_LOG_DOM_CRIT(_ecordova_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_ecordova_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_ecordova_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_ecordova_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_ecordova_log_dom, __VA_ARGS__)

extern int (*system_info_get_platform_string)(const char *key, char **value);

/* Check if slp error or not */
#define TIZEN_ERROR_MIN_PLATFORM_ERROR (-1073741824LL) /* = -2147483648 / 2 */
/* Tizen Contacts Error */
#define TIZEN_ERROR_CONTACTS		-0x02010000
/* Tizen Sensor Error */
#define TIZEN_ERROR_SENSOR		-0x02440000

/**
 * @brief Enumeration for tizen errors.
 * @since_tizen @if MOBILE 2.3 @elseif WEARABLE 2.3.1 @endif
 *
*/
typedef enum
{
    TIZEN_ERROR_NONE = 0, /**< Successful */
    TIZEN_ERROR_OUT_OF_MEMORY = -ENOMEM, /**< Out of memory */
    TIZEN_ERROR_INVALID_PARAMETER = -EINVAL, /**< Invalid function parameter */
    TIZEN_ERROR_FILE_NO_SPACE_ON_DEVICE = -ENOSPC, /**< No space left on device */
    TIZEN_ERROR_PERMISSION_DENIED = -EACCES, /**< Permission denied */
    TIZEN_ERROR_NO_DATA = -ENODATA, /**< No data available */
    TIZEN_ERROR_IO_ERROR = -EIO, /**< I/O error */

    TIZEN_ERROR_UNKNOWN = TIZEN_ERROR_MIN_PLATFORM_ERROR, /**< Unknown error */

    /* This is a place to add new errors here.
     * Do not assign integer values explicitly. Values are auto-assigned.
     */
    TIZEN_ERROR_TIMED_OUT, /**< Time out */
    TIZEN_ERROR_NOT_SUPPORTED, /**< Not supported */
    TIZEN_ERROR_USER_NOT_CONSENTED, /**< Not Consented */
    TIZEN_ERROR_END_OF_COLLECTION,
} tizen_error_e;

typedef enum {
	SYSTEM_INFO_ERROR_NONE              = TIZEN_ERROR_NONE,          /**< Successful */
	SYSTEM_INFO_ERROR_INVALID_PARAMETER = TIZEN_ERROR_INVALID_PARAMETER, /**< Invalid parameter */
	SYSTEM_INFO_ERROR_OUT_OF_MEMORY     = TIZEN_ERROR_OUT_OF_MEMORY, /**< Out of memory */
	SYSTEM_INFO_ERROR_IO_ERROR          = TIZEN_ERROR_IO_ERROR,	 /**< An input/output error occurred when reading value from system */
	SYSTEM_INFO_ERROR_PERMISSION_DENIED = TIZEN_ERROR_PERMISSION_DENIED, /**< No permission to use the API */
} system_info_error_e;

#endif
