#include "ecore_xcb_private.h"
#include <xcb/xcb_event.h>

/* local variables */
static void (*_error_func)(void *data) = NULL;
static void *_error_data = NULL;
static void (*_io_error_func)(void *data) = NULL;
static void *_io_error_data = NULL;
static int _error_request_code = 0;
static int _error_code = 0;

/**
 * Set the error handler.
 * @param func The error handler function
 * @param data The data to be passed to the handler function
 *
 * Set the X error handler function
 */
EAPI void 
ecore_x_error_handler_set(void (*func)(void *data), const void *data) 
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
ecore_x_io_error_handler_set(void (*func)(void *data), const void *data) 
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

int 
_ecore_xcb_error_handle(xcb_generic_error_t *err) 
{
   WRN("Got Error:");
   WRN("\tEvent: %s", xcb_event_get_request_label(err->major_code));
   WRN("\tError: %s", xcb_event_get_error_label(err->error_code));
   if (err->error_code == XCB_EVENT_ERROR_BAD_VALUE)
     WRN("\tBad Value: %d", ((xcb_value_error_t *)err)->bad_value);
   else if (err->error_code == XCB_EVENT_ERROR_BAD_WINDOW) 
     WRN("\tBad Window: %d", ((xcb_window_error_t *)err)->bad_value);

   _error_request_code = err->sequence;
   _error_code = err->error_code;
   if (_error_func)
     _error_func(_error_data);

   return 0;
}

int 
_ecore_xcb_io_error_handle(xcb_generic_error_t *err) 
{
   CRIT("IO Error:");
   CRIT("\tRequest: %d", err->sequence);
   CRIT("\tCode: %d", err->error_code);
   if (_io_error_func)
     _io_error_func(_io_error_data);
   else
     exit(-1);

   return 0;
}
