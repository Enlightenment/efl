/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>   /* for printf */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Eina.h>

#include "Ecore.h"
#include "Ecore_WinCE.h"
#include "ecore_wince_private.h"


/***** Global declarations *****/

double              _ecore_wince_double_click_time = 0.25;
double              _ecore_wince_event_last_time = 0.0;
Ecore_WinCE_Window *_ecore_wince_event_last_window = NULL;
HINSTANCE           _ecore_wince_instance = NULL;

int ECORE_WINCE_EVENT_KEY_DOWN              = 0;
int ECORE_WINCE_EVENT_KEY_UP                = 0;
int ECORE_WINCE_EVENT_MOUSE_BUTTON_DOWN     = 0;
int ECORE_WINCE_EVENT_MOUSE_BUTTON_UP       = 0;
int ECORE_WINCE_EVENT_MOUSE_MOVE            = 0;
int ECORE_WINCE_EVENT_MOUSE_IN              = 0;
int ECORE_WINCE_EVENT_MOUSE_OUT             = 0;
int ECORE_WINCE_EVENT_WINDOW_FOCUS_IN       = 0;
int ECORE_WINCE_EVENT_WINDOW_FOCUS_OUT      = 0;
int ECORE_WINCE_EVENT_WINDOW_DAMAGE         = 0;
int ECORE_WINCE_EVENT_WINDOW_CREATE         = 0;
int ECORE_WINCE_EVENT_WINDOW_DESTROY        = 0;
int ECORE_WINCE_EVENT_WINDOW_SHOW           = 0;
int ECORE_WINCE_EVENT_WINDOW_HIDE           = 0;
int ECORE_WINCE_EVENT_WINDOW_DELETE_REQUEST = 0;


/***** Private declarations *****/

static int       _ecore_wince_init_count = 0;


LRESULT CALLBACK _ecore_wince_window_procedure(HWND   window,
                                               UINT   message,
                                               WPARAM window_param,
                                               LPARAM data_param);

static void      _ecore_wince_error_print_cb(Eina_Error_Level level,
                                             const char      *file,
                                             const char      *fnc,
                                             int              line,
                                             const char      *fmt,
                                             void            *data,
                                             va_list          args);


/***** API *****/

int
ecore_wince_init()
{
   WNDCLASS wc;

   eina_error_print_cb_set(_ecore_wince_error_print_cb, NULL);

   EINA_ERROR_PINFO("initializing ecore_wince (current count: %d)\n", _ecore_wince_init_count);

   if (_ecore_wince_init_count > 0)
     {
	_ecore_wince_init_count++;
	return _ecore_wince_init_count;
     }

   _ecore_wince_instance = GetModuleHandle(NULL);
   if (!_ecore_wince_instance)
     {
        EINA_ERROR_PERR("GetModuleHandle() failed\n");
        return 0;
     }

   memset (&wc, 0, sizeof (wc));
   wc.style = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc = _ecore_wince_window_procedure;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = _ecore_wince_instance;
   wc.hIcon = NULL;
   wc.hCursor = LoadCursor (NULL, IDC_ARROW);
   wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
   wc.lpszMenuName =  NULL;
   wc.lpszClassName = ECORE_WINCE_WINDOW_CLASS;

   if(!RegisterClass(&wc))
     {
        EINA_ERROR_PERR("RegisterClass() failed\n");
        FreeLibrary(_ecore_wince_instance);
        return 0;
     }

   if (!ECORE_WINCE_EVENT_KEY_DOWN)
     {
        ECORE_WINCE_EVENT_KEY_DOWN              = ecore_event_type_new();
        ECORE_WINCE_EVENT_KEY_UP                = ecore_event_type_new();
        ECORE_WINCE_EVENT_MOUSE_BUTTON_DOWN     = ecore_event_type_new();
        ECORE_WINCE_EVENT_MOUSE_BUTTON_UP       = ecore_event_type_new();
        ECORE_WINCE_EVENT_MOUSE_MOVE            = ecore_event_type_new();
        ECORE_WINCE_EVENT_MOUSE_IN              = ecore_event_type_new();
        ECORE_WINCE_EVENT_MOUSE_OUT             = ecore_event_type_new();
        ECORE_WINCE_EVENT_WINDOW_FOCUS_IN       = ecore_event_type_new();
        ECORE_WINCE_EVENT_WINDOW_FOCUS_OUT      = ecore_event_type_new();
        ECORE_WINCE_EVENT_WINDOW_DAMAGE         = ecore_event_type_new();
        ECORE_WINCE_EVENT_WINDOW_CREATE         = ecore_event_type_new();
        ECORE_WINCE_EVENT_WINDOW_DESTROY        = ecore_event_type_new();
        ECORE_WINCE_EVENT_WINDOW_SHOW           = ecore_event_type_new();
        ECORE_WINCE_EVENT_WINDOW_HIDE           = ecore_event_type_new();
        ECORE_WINCE_EVENT_WINDOW_DELETE_REQUEST = ecore_event_type_new();
     }

   _ecore_wince_init_count++;

   return _ecore_wince_init_count;
}

int
ecore_wince_shutdown()
{
   HWND task_bar;

   EINA_ERROR_PINFO("shutting down ecore_wince (current count: %d)\n", _ecore_wince_init_count);

   _ecore_wince_init_count--;
   if (_ecore_wince_init_count > 0) return _ecore_wince_init_count;
   if (!_ecore_wince_instance) return _ecore_wince_init_count;

   /* force task bar to be shown (in case the application exits */
   /* while being fullscreen) */
   task_bar = FindWindow(L"HHTaskBar", NULL);
   if (task_bar)
     {
        ShowWindow(task_bar, SW_SHOW);
        EnableWindow(task_bar, TRUE);
     }

   if (!UnregisterClass(ECORE_WINCE_WINDOW_CLASS, _ecore_wince_instance))
     {
        EINA_ERROR_PERR("UnregisterClass() failed\n");
     }
   if (!FreeLibrary(_ecore_wince_instance))
     {
        EINA_ERROR_PERR("FreeLibrary() failed\n");
     }
   _ecore_wince_instance = NULL;

   if (_ecore_wince_init_count < 0) _ecore_wince_init_count = 0;

   return _ecore_wince_init_count;
}

/**
 * Sets the timeout for a double and triple clicks to be flagged.
 *
 * This sets the time between clicks before the double_click flag is
 * set in a button down event. If 3 clicks occur within double this
 * time, the triple_click flag is also set.
 *
 * @param t The time in seconds
 */
EAPI void
ecore_wince_double_click_time_set(double t)
{
   if (t < 0.0) t = 0.0;
   _ecore_wince_double_click_time = t;
}

/**
 * Retrieves the double and triple click flag timeout.
 *
 * See @ref ecore_wince_double_click_time_set for more information.
 *
 * @return The timeout for double clicks in seconds.
 */
EAPI double
ecore_wince_double_click_time_get(void)
{
   return _ecore_wince_double_click_time;
}

/**
 * Return the last event time
 */
EAPI double
ecore_wince_current_time_get(void)
{
   return _ecore_wince_event_last_time;
}


/***** Private functions definitions *****/

LRESULT CALLBACK
_ecore_wince_window_procedure(HWND   window,
                              UINT   message,
                              WPARAM window_param,
                              LPARAM data_param)
{
   Ecore_WinCE_Callback_Data *data;
   POINTS                     pt;
   DWORD                      coord;

   data = (Ecore_WinCE_Callback_Data *)malloc(sizeof(Ecore_WinCE_Callback_Data));
   if (!data) return DefWindowProc(window, message, window_param, data_param);

   data->window = window;
   data->message = message;
   data->window_param = window_param;
   data->data_param = data_param;
   data->time = GetTickCount();
   coord = GetMessagePos();
   pt = MAKEPOINTS(coord);
   data->x = pt.x;
   data->y = pt.y;

   switch (data->message)
     {
       /* Keyboard input notifications */
     case WM_CHAR:
       _ecore_wince_event_handle_key_press(data, 0);
       break;
     case WM_HOTKEY:
       _ecore_wince_event_handle_key_press(data, 1);
       break;
     case WM_KEYDOWN:
     case WM_SYSKEYDOWN:
       _ecore_wince_event_handle_key_press(data, 1);
       break;
     case WM_KEYUP:
     case WM_SYSKEYUP:
       _ecore_wince_event_handle_key_release(data, 1);
       break;
     case WM_SETFOCUS:
       _ecore_wince_event_handle_focus_in(data);
       break;
     case WM_KILLFOCUS:
       _ecore_wince_event_handle_focus_out(data);
       break;
       /* Mouse input notifications */
     case WM_LBUTTONDOWN:
       _ecore_wince_event_handle_button_press(data, 1);
       break;
     case WM_LBUTTONUP:
       _ecore_wince_event_handle_button_release(data, 1);
       break;
     case WM_MOUSEMOVE:
       {
          RECT                        rect;
          struct _Ecore_WinCE_Window *w = NULL;

          w = (struct _Ecore_WinCE_Window *)GetWindowLong(window, GWL_USERDATA);

          if (GetClientRect(window, &rect))
            {
               POINT pt;

               EINA_ERROR_PINFO("mouse in window\n");

               pt.x = LOWORD(data_param);
               pt.y = HIWORD(data_param);
               if (!PtInRect(&rect, pt))
                 {
                    if (w->pointer_is_in)
                      {
                         w->pointer_is_in = 0;
                         _ecore_wince_event_handle_leave_notify(data);
                      }
                 }
               else
                 {
                    if (!w->pointer_is_in)
                      {
                         w->pointer_is_in = 1;
                         _ecore_wince_event_handle_enter_notify(data);
                      }
                 }
            }
          else
            {
               EINA_ERROR_PERR("GetClientRect() failed\n");
            }
          _ecore_wince_event_handle_motion_notify(data);

          break;
       }
       /* Window notifications */
     case WM_CREATE:
       _ecore_wince_event_handle_create_notify(data);
       break;
     case WM_DESTROY:
       _ecore_wince_event_handle_destroy_notify(data);
       break;
     case WM_SHOWWINDOW:
       if ((data->data_param == SW_OTHERUNZOOM) ||
           (data->data_param == SW_OTHERUNZOOM))
         break;

       if (data->window_param)
         _ecore_wince_event_handle_map_notify(data);
       else
         _ecore_wince_event_handle_unmap_notify(data);

       break;
     case WM_CLOSE:
       _ecore_wince_event_handle_delete_request(data);
       break;
       /* GDI notifications */
     case WM_PAINT:
       {
          PAINTSTRUCT paint;

          if (BeginPaint(window, &paint))
            {
               data->update = paint.rcPaint;
               _ecore_wince_event_handle_expose(data);
               EndPaint(window, &paint);
            }
          break;
       }
     default:
       return DefWindowProc(window, message, window_param, data_param);
     }

   return 0;
}

static void
_ecore_wince_error_print_cb(Eina_Error_Level level __UNUSED__,
                             const char     *file __UNUSED__,
                             const char     *fnc,
                             int             line,
                             const char     *fmt,
                             void           *data __UNUSED__,
                             va_list         args)
{
   fprintf(stderr, "[%s:%d] ", fnc, line);
   vfprintf(stderr, fmt, args);
}
