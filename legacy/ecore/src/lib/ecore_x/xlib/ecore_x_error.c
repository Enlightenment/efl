#include "ecore_private.h"
#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

static void _ecore_x_error_handle(Display * d, XErrorEvent * ev);
static int  _ecore_x_io_error_handle(Display *d);

static void (*_error_func) (void *data) = NULL;
static void *_error_data = NULL;
static void (*_io_error_func) (void *data) = NULL;
static void *_io_error_data = NULL;
static int   _error_request_code = 0;
static int   _error_code = 0;

/**
 * Set the error handler.
 * @param func The error handler function
 * @param data The data to be passed to the handler function
 * 
 * Set the X error handler function
 */
EAPI void
ecore_x_error_handler_set(void (*func) (void *data), const void *data)
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
ecore_x_io_error_handler_set(void (*func) (void *data), const void *data)
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

void
_ecore_x_error_handler_init(void)
{
   XSetErrorHandler((XErrorHandler)_ecore_x_error_handle);
   XSetIOErrorHandler((XIOErrorHandler)_ecore_x_io_error_handle);   
}

static void
_ecore_x_error_handle(Display *d, XErrorEvent *ev)
{
    if (d == _ecore_x_disp)
     {
	_error_request_code = ev->request_code;
	_error_code = ev->error_code;
	if (_error_func) _error_func(_error_data);
     }
}

static int
_ecore_x_io_error_handle(Display *d)
{
    if (d == _ecore_x_disp)
     {
	if (_io_error_func) _io_error_func(_io_error_data);
	else exit(-1);
     }
   return 0;
}
