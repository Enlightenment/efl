#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>   /* for printf */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Input.h>

#include "Ecore_WinCE.h"
#include "ecore_wince_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

static int       _ecore_wince_init_count = 0;

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
          RECT                rect;
          Ecore_WinCE_Window *w = NULL;

          w = (Ecore_WinCE_Window *)GetWindowLong(window, GWL_USERDATA);

          if (GetClientRect(window, &rect))
            {
               POINT pt;

               INF("mouse in window");

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
               ERR("GetClientRect() failed");
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
           (data->data_param == SW_OTHERZOOM))
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
     case WM_ERASEBKGND:
       return 1;
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
_ecore_wince_error_print_cb(const Eina_Log_Domain *d __UNUSED__,
                            Eina_Log_Level  level __UNUSED__,
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

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/


double              _ecore_wince_double_click_time = 0.25;
long                _ecore_wince_event_last_time = 0;
Ecore_WinCE_Window *_ecore_wince_event_last_window = NULL;
HINSTANCE           _ecore_wince_instance = NULL;
int                 _ecore_wince_log_dom_global = -1;

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

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Ecore_WinCE_Group Ecore_WinCE library
 *
 * Ecore_WinCE is a library that wraps Windows CE graphic functions
 * and integrate them nicely into the Ecore main loop.
 *
 * @{
 */

/**
 * @brief Initialize the Ecore_WinCE library.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up the Windows CE graphic system. It returns 0 on
 * failure, otherwise it returns the number of times it has already been
 * called.
 *
 * When Ecore_WinCE is not used anymore, call ecore_wince_shutdown()
 * to shut down the Ecore_WinCE library.
 */
EAPI int
ecore_wince_init()
{
   WNDCLASS wc;

   if (++_ecore_wince_init_count != 1)
     return _ecore_wince_init_count;

   if (!eina_init())
     return --_ecore_wince_init_count;

   eina_log_print_cb_set(_ecore_wince_error_print_cb, NULL);
   _ecore_wince_log_dom_global = eina_log_domain_register
     ("ecore_wince", ECORE_WINCE_DEFAULT_LOG_COLOR);
   if (_ecore_wince_log_dom_global < 0)
     {
        EINA_LOG_ERR("Ecore_WinCE: Could not register log domain");
        goto shutdown_eina;
      }

   if (!ecore_event_init())
     {
        ERR("Ecore_WinCE: Could not init ecore_event");
        goto unregister_log_domain;
     }

   _ecore_wince_instance = GetModuleHandle(NULL);
   if (!_ecore_wince_instance)
     {
        ERR("GetModuleHandle() failed");
        goto shutdown_ecore_event;
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
        ERR("RegisterClass() failed");
        goto free_library;
     }

   if (!ECORE_WINCE_EVENT_MOUSE_IN)
     {
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

   return _ecore_wince_init_count;

 free_library:
   FreeLibrary(_ecore_wince_instance);
 shutdown_ecore_event:
   ecore_event_shutdown();
 unregister_log_domain:
   eina_log_domain_unregister(_ecore_wince_log_dom_global);
 shutdown_eina:
   eina_shutdown();

   return --_ecore_wince_init_count;
}

/**
 * @brief Shut down the Ecore_WinCE library.
 *
 * @return 0 when the library is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the Ecore_WinCE library. It returns 0 when it has
 * been called the same number of times than ecore_wince_init(). In that case
 * it shuts down all the Windows CE graphic system.
 */
EAPI int
ecore_wince_shutdown()
{
   HWND task_bar;

   if (--_ecore_wince_init_count != 0)
     return _ecore_wince_init_count;

   /* force task bar to be shown (in case the application exits */
   /* while being fullscreen) */
   task_bar = FindWindow(L"HHTaskBar", NULL);
   if (task_bar)
     {
        ShowWindow(task_bar, SW_SHOW);
        EnableWindow(task_bar, TRUE);
     }

   if (!UnregisterClass(ECORE_WINCE_WINDOW_CLASS, _ecore_wince_instance))
     ERR("UnregisterClass() failed");

   if (!FreeLibrary(_ecore_wince_instance))
     ERR("FreeLibrary() failed");

   _ecore_wince_instance = NULL;

   ecore_event_shutdown();
   eina_log_domain_unregister(_ecore_wince_log_dom_global);
   _ecore_wince_log_dom_global = -1;
   eina_shutdown();

   return _ecore_wince_init_count;
}

/**
 * @brief Set the timeout for a double and triple clicks to be flagged.
 *
 * @param t The time in seconds.
 *
 * This function sets the time @p t between clicks before the
 * double_click flag is set in a button down event. If 3 clicks occur
 * within double this time, the triple_click flag is also set.
 */
EAPI void
ecore_wince_double_click_time_set(double t)
{
   if (t < 0.0) t = 0.0;
   _ecore_wince_double_click_time = t;
}

/**
 * @brief Retrieve the double and triple click flag timeout.
 *
 * @return The timeout for double clicks in seconds.
 *
 * This function returns the double clicks in seconds. If
 * ecore_wince_double_click_time_set() has not been called, the
 * default value is returned. See ecore_wince_double_click_time_set()
 * for more informations.
 */
EAPI double
ecore_wince_double_click_time_get(void)
{
   return _ecore_wince_double_click_time;
}

/**
 * @brief Return the last event time.
 *
 * @return The last envent time.
 *
 * This function returns the last event time.
 */
EAPI long
ecore_wince_current_time_get(void)
{
   return _ecore_wince_event_last_time;
}

/**
 * @}
 */
