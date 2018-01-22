#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <stdlib.h>

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"

static int _ecore_x_error_handle(Display *d,
                                 XErrorEvent *ev);
static int _ecore_x_io_error_handle(Display *d);

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
ecore_x_error_handler_set(void (*func)(void *data),
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
ecore_x_io_error_handler_set(void (*func)(void *data),
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
//FIXME: Use Ecore_X_Error_Code type when 2.0 is released
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

void
_ecore_x_error_handler_init(void)
{
   XSetErrorHandler((XErrorHandler)_ecore_x_error_handle);
   XSetIOErrorHandler((XIOErrorHandler)_ecore_x_io_error_handle);
}

static int
_ecore_x_error_handle(Display *d,
                      XErrorEvent *ev)
{
   if (!_ecore_xlib_sync) goto skip;
   switch (ev->error_code)
     {
      case BadRequest:	/* bad request code */
        ERR("BadRequest");
        break;
      case BadValue:	/* int parameter out of range */
        ERR("BadValue");
        break;
      case BadWindow:	/* parameter not a Window */
        ERR("BadWindow");
        break;
      case BadPixmap:	/* parameter not a Pixmap */
        ERR("BadPixmap");
        break;
      case BadAtom:	/* parameter not an Atom */
        ERR("BadAtom");
        break;
      case BadCursor:	/* parameter not a Cursor */
        ERR("BadCursor");
        break;
      case BadFont:	/* parameter not a Font */
        ERR("BadFont");
        break;
      case BadMatch:	/* parameter mismatch */
        ERR("BadMatch");
        break;
      case BadDrawable:	/* parameter not a Pixmap or Window */
        ERR("BadDrawable");
        break;
      case BadAccess:	/* depending on context */
        ERR("BadAccess");
        break;
      case BadAlloc:	/* insufficient resources */
        ERR("BadAlloc");
        break;
      case BadColor:	/* no such colormap */
        ERR("BadColor");
        break;
      case BadGC:	/* parameter not a GC */
        ERR("BadGC");
        break;
      case BadIDChoice:	/* choice not in range or already used */
        ERR("BadIDChoice");
        break;
      case BadName:	/* font or color name doesn't exist */
        ERR("BadName");
        break;
      case BadLength:	/* Request length incorrect */
        ERR("BadLength");
        break;
      case BadImplementation:	/* server is defective */
        ERR("BadImplementation");
        break;
     }
skip:
   if (d == _ecore_x_disp)
     {
        _error_request_code = ev->request_code;
        _error_code = ev->error_code;
        _error_resource_id = ev->resourceid;
        if (_error_func)
          _error_func(_error_data);
     }
   return 0;
}

static int
_ecore_x_io_error_handle(Display *d)
{
   if (d == _ecore_x_disp)
     {
        if (_io_error_func)
          {
             _ecore_x_disp = NULL;
             _ecore_x_shutdown();
             _io_error_func(_io_error_data);
          }
        else
          exit(-1);
     }

   return 0;
}

