#include "ecore_xcb_private.h"
#include <xcb/xcb_event.h>

/* local variables */
static void (*_error_func)(void *data) = NULL;
static void *_error_data = NULL;
static void (*_io_error_func)(void *data) = NULL;
static void *_io_error_data = NULL;
static int _error_request_code = 0;
static int _error_code = 0;
static Ecore_X_ID _error_resource_id = 0;

/**
 * Set the error handler.
 * @param func The error handler function
 * @param data The data to be passed to the handler function
 *
 * Set the X error handler function
 */
EAPI void
ecore_x_error_handler_set(void        (*func)(void *data),
                          const void *data)
{
   _error_func = func;
   _error_data = (void *)data;
}

/**
 * Set the I/O error handler.
 * @param func The I/O error handler function
 * @param data The data to be passed to the handler function
 *
 * Set the X I/O error handler function
 */
EAPI void
ecore_x_io_error_handler_set(void        (*func)(void *data),
                             const void *data)
{
   _io_error_func = func;
   _io_error_data = (void *)data;
}

/**
 * Get the request code that caused the error.
 * @return The request code causing the X error
 *
 * Return the X request code that caused the last X error
 */
EAPI int
ecore_x_error_request_get(void)
{
   return _error_request_code;
}

/**
 * Get the error code from the error.
 * @return The error code from the X error
 *
 * Return the error code from the last X error
 */
EAPI int
ecore_x_error_code_get(void)
{
   return _error_code;
}

/**
 * Get the resource id that caused the error.
 * @return The resource id causing the X error
 *
 * Return the X resource id that caused the last X error
 */
EAPI Ecore_X_ID
ecore_x_error_resource_id_get(void)
{
   return _error_resource_id;
}

int
_ecore_xcb_error_handle(xcb_generic_error_t *err)
{
   WRN("Got Error:");
   WRN("\tEvent: %s", xcb_event_get_request_label(err->major_code));
   WRN("\tError: %s", xcb_event_get_error_label(err->error_code));

   if (err->error_code == XCB_VALUE)
     WRN("\tBad Value: %d", ((xcb_value_error_t *)err)->bad_value);
   else if (err->error_code == XCB_WINDOW)
     WRN("\tBad Window: %d", ((xcb_window_error_t *)err)->bad_value);

   _error_request_code = err->sequence;
   _error_code = err->error_code;
   _error_resource_id = err->resource_id;
   if (_error_func)
     _error_func(_error_data);

   return 0;
}

int
_ecore_xcb_io_error_handle(xcb_generic_error_t *err)
{
   CRI("IO Error:");
   if (err)
     {
        CRI("\tRequest: %d", err->sequence);
        CRI("\tCode: %d", err->error_code);
     }
   if (_io_error_func)
     _io_error_func(_io_error_data);
   else
     exit(-1);

   return 0;
}

