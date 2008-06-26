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
#include <windowsx.h>

#include "Ecore.h"
#include "Ecore_Win32.h"
#include "ecore_win32_private.h"


/***** Global declarations *****/

HINSTANCE           _ecore_win32_instance = NULL;
double              _ecore_win32_double_click_time = 0.25;
double              _ecore_win32_event_last_time = 0.0;
Ecore_Win32_Window *_ecore_win32_event_last_window = NULL;

int ECORE_WIN32_EVENT_KEY_DOWN              = 0;
int ECORE_WIN32_EVENT_KEY_UP                = 0;
int ECORE_WIN32_EVENT_MOUSE_BUTTON_DOWN     = 0;
int ECORE_WIN32_EVENT_MOUSE_BUTTON_UP       = 0;
int ECORE_WIN32_EVENT_MOUSE_MOVE            = 0;
int ECORE_WIN32_EVENT_MOUSE_IN              = 0;
int ECORE_WIN32_EVENT_MOUSE_OUT             = 0;
int ECORE_WIN32_EVENT_MOUSE_WHEEL           = 0;
int ECORE_WIN32_EVENT_WINDOW_FOCUS_IN       = 0;
int ECORE_WIN32_EVENT_WINDOW_FOCUS_OUT      = 0;
int ECORE_WIN32_EVENT_WINDOW_DAMAGE         = 0;
int ECORE_WIN32_EVENT_WINDOW_CREATE         = 0;
int ECORE_WIN32_EVENT_WINDOW_DESTROY        = 0;
int ECORE_WIN32_EVENT_WINDOW_SHOW           = 0;
int ECORE_WIN32_EVENT_WINDOW_HIDE           = 0;
int ECORE_WIN32_EVENT_WINDOW_CONFIGURE      = 0;
int ECORE_WIN32_EVENT_WINDOW_RESIZE         = 0;
int ECORE_WIN32_EVENT_WINDOW_DELETE_REQUEST = 0;


/***** Private declarations *****/

static int       _ecore_win32_init_count = 0;

LRESULT CALLBACK _ecore_win32_window_procedure(HWND   window,
                                               UINT   message,
                                               WPARAM window_param,
                                               LPARAM data_param);


/***** API *****/


int
ecore_win32_init()
{
   WNDCLASS wc;

   if (_ecore_win32_init_count > 0)
     {
	_ecore_win32_init_count++;
	return _ecore_win32_init_count;
     }

   printf (" *** ecore_win32_init\n");
   _ecore_win32_instance = GetModuleHandle(NULL);
   if (!_ecore_win32_instance)
     return 0;

   memset (&wc, 0, sizeof (WNDCLASS));
   wc.style = CS_HREDRAW | CS_VREDRAW;
   wc.lpfnWndProc = _ecore_win32_window_procedure;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = _ecore_win32_instance;
   wc.hIcon = LoadIcon (NULL, IDI_APPLICATION);
   wc.hCursor = LoadCursor (NULL, IDC_ARROW);
   wc.hbrBackground = (HBRUSH)(1 + COLOR_BTNFACE);
   wc.lpszMenuName =  NULL;
   wc.lpszClassName = ECORE_WIN32_WINDOW_CLASS;

   if(!RegisterClass(&wc))
     {
        FreeLibrary(_ecore_win32_instance);
        return 0;
     }

   if (!ECORE_WIN32_EVENT_KEY_DOWN)
     {
        ECORE_WIN32_EVENT_KEY_DOWN              = ecore_event_type_new();
        ECORE_WIN32_EVENT_KEY_UP                = ecore_event_type_new();
        ECORE_WIN32_EVENT_MOUSE_BUTTON_DOWN     = ecore_event_type_new();
        ECORE_WIN32_EVENT_MOUSE_BUTTON_UP       = ecore_event_type_new();
        ECORE_WIN32_EVENT_MOUSE_MOVE            = ecore_event_type_new();
        ECORE_WIN32_EVENT_MOUSE_IN              = ecore_event_type_new();
        ECORE_WIN32_EVENT_MOUSE_OUT             = ecore_event_type_new();
        ECORE_WIN32_EVENT_MOUSE_WHEEL           = ecore_event_type_new();
        ECORE_WIN32_EVENT_WINDOW_FOCUS_IN       = ecore_event_type_new();
        ECORE_WIN32_EVENT_WINDOW_FOCUS_OUT      = ecore_event_type_new();
        ECORE_WIN32_EVENT_WINDOW_DAMAGE         = ecore_event_type_new();
        ECORE_WIN32_EVENT_WINDOW_CREATE         = ecore_event_type_new();
        ECORE_WIN32_EVENT_WINDOW_DESTROY        = ecore_event_type_new();
        ECORE_WIN32_EVENT_WINDOW_SHOW           = ecore_event_type_new();
        ECORE_WIN32_EVENT_WINDOW_HIDE           = ecore_event_type_new();
        ECORE_WIN32_EVENT_WINDOW_CONFIGURE      = ecore_event_type_new();
        ECORE_WIN32_EVENT_WINDOW_RESIZE         = ecore_event_type_new();
        ECORE_WIN32_EVENT_WINDOW_DELETE_REQUEST = ecore_event_type_new();
     }

   _ecore_win32_init_count++;

   return _ecore_win32_init_count;
}

int
ecore_win32_shutdown()
{
   _ecore_win32_init_count++;
   if (_ecore_win32_init_count > 0) return _ecore_win32_init_count;
   if (!_ecore_win32_instance) return _ecore_win32_init_count;

   UnregisterClass(ECORE_WIN32_WINDOW_CLASS, _ecore_win32_instance);
   FreeLibrary(_ecore_win32_instance);
   _ecore_win32_instance = NULL;

   if (_ecore_win32_init_count < 0) _ecore_win32_init_count = 0;

   return _ecore_win32_init_count;
}

int
ecore_win32_screen_depth_get()
{
   HDC dc;
   int depth;

   dc = GetDC(NULL);
   if (!dc)
     return 0;

   depth = GetDeviceCaps(dc, BITSPIXEL);
   ReleaseDC(NULL, dc);

   return depth;
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
void
ecore_win32_double_click_time_set(double t)
{
   if (t < 0.0) t = 0.0;
   _ecore_win32_double_click_time = t;
}

/**
 * Retrieves the double and triple click flag timeout.
 *
 * See @ref ecore_win32_double_click_time_set for more information.
 *
 * @return The timeout for double clicks in seconds.
 */
double
ecore_win32_double_click_time_get(void)
{
   return _ecore_win32_double_click_time;
}

/**
 * Return the last event time
 */
double
ecore_win32_current_time_get(void)
{
   return _ecore_win32_event_last_time;
}


/***** Private functions definitions *****/


LRESULT CALLBACK
_ecore_win32_window_procedure(HWND   window,
                              UINT   message,
                              WPARAM window_param,
                              LPARAM data_param)
{
   Ecore_Win32_Callback_Data *data;
   POINTS                     pt;
   DWORD                      coord;

   data = (Ecore_Win32_Callback_Data *)malloc(sizeof(Ecore_Win32_Callback_Data));
   if (!data) return DefWindowProc(window, message, window_param, data_param);

   data->window = window;
   data->message = message;
   data->window_param = window_param;
   data->data_param = data_param;
   data->time = GetMessageTime();
   coord = GetMessagePos();
   pt = MAKEPOINTS(coord);
   data->x = pt.x;
   data->y = pt.y;

   switch (data->message)
     {
       /* Keyboard input notifications */
     case WM_KEYDOWN:
       printf (" * ecore message : keystroke down\n");
       _ecore_win32_event_handle_key_press(data, 1);
       return 0;
     case WM_CHAR:
       _ecore_win32_event_handle_key_press(data, 0);
       return 0;
     case WM_KEYUP:
       printf (" * ecore message : keystroke up\n");
       _ecore_win32_event_handle_key_release(data, 1);
       return 0;
     case WM_SETFOCUS:
       printf (" * ecore message : focus in\n");
       _ecore_win32_event_handle_focus_in(data);
       return 0;
     case WM_KILLFOCUS:
       printf (" * ecore message : focus out\n");
       _ecore_win32_event_handle_focus_out(data);
       return 0;
       /* Mouse input notifications */
     case WM_LBUTTONDOWN:
       printf (" * ecore message : lbuttondown\n");
       _ecore_win32_event_handle_button_press(data, 1);
       return 0;
     case WM_MBUTTONDOWN:
       printf (" * ecore message : mbuttondown\n");
       _ecore_win32_event_handle_button_press(data, 2);
       return 0;
     case WM_RBUTTONDOWN:
       printf (" * ecore message : rbuttondown\n");
       _ecore_win32_event_handle_button_press(data, 3);
       return 0;
     case WM_LBUTTONUP:
       printf (" * ecore message : lbuttonup\n");
       _ecore_win32_event_handle_button_release(data, 1);
       return 0;
     case WM_MBUTTONUP:
       printf (" * ecore message : mbuttonup\n");
       _ecore_win32_event_handle_button_release(data, 2);
       return 0;
     case WM_RBUTTONUP:
       printf (" * ecore message : rbuttonup\n");
       _ecore_win32_event_handle_button_release(data, 3);
       return 0;
     case WM_MOUSEMOVE:
       {
          RECT                        rect;
          struct _Ecore_Win32_Window *w = NULL;

          w = (struct _Ecore_Win32_Window *)GetWindowLong(window, GWL_USERDATA);

          if (GetClientRect(window, &rect))
          {
             POINT pt;

             pt.x = GET_X_LPARAM(data_param);
             pt.y = GET_Y_LPARAM(data_param);
             if (!PtInRect(&rect, pt))
               {
                  if (w->pointer_is_in)
                    {
                       w->pointer_is_in = 0;
                       _ecore_win32_event_handle_leave_notify(data);
                    }
               }
             else
               {
                  if (!w->pointer_is_in)
                    {
                       w->pointer_is_in = 1;
                       _ecore_win32_event_handle_enter_notify(data);
                    }

               }
          }
          _ecore_win32_event_handle_motion_notify(data);

          return 0;
       }
     case WM_MOUSEWHEEL:
       printf (" * ecore message : mouse wheel\n");
       _ecore_win32_event_handle_button_press(data, 4);
       return 0;
       /* Window notifications */
     case WM_CREATE:
       {
         RECT rect;
         GetClientRect(window, &rect);
         printf (" *** ecore message : create %ld %ld\n",
                 rect.right - rect.left, rect.bottom - rect.top);
       }
       _ecore_win32_event_handle_create_notify(data);
       return 0;
     case WM_DESTROY:
       printf (" * ecore message : destroy\n");
       _ecore_win32_event_handle_destroy_notify(data);
       return 0;
     case WM_SHOWWINDOW:
       {
         RECT rect;
         GetClientRect(window, &rect);
         printf (" *** ecore message : show %ld %ld\n",
                 rect.right - rect.left, rect.bottom - rect.top);
       }
       if ((data->data_param == SW_OTHERUNZOOM) ||
           (data->data_param == SW_OTHERUNZOOM))
         return 0;

       if (data->window_param)
         _ecore_win32_event_handle_map_notify(data);
       else
         _ecore_win32_event_handle_unmap_notify(data);

       return 0;
     case WM_CLOSE:
       printf (" * ecore message : close\n");
       _ecore_win32_event_handle_delete_request(data);
       return 0;
     case WM_MOVING:
       printf (" * ecore message : moving\n");
       return TRUE;
     case WM_MOVE:
       printf (" * ecore message : moved\n");
       return 0;
     case WM_SIZING:
       printf (" * ecore message : sizing\n");
       _ecore_win32_event_handle_resize(data);
       _ecore_win32_event_handle_configure_notify(data);
       return TRUE;
     case WM_SIZE:
       printf (" * ecore message : sized\n");
       return 0;
/*      case WM_WINDOWPOSCHANGING: */
/*        { */
/*          RECT rect; */
/*          GetClientRect(window, &rect); */
/*          printf (" *** ecore message : WINDOWPOSCHANGING %ld %ld\n", */
/*                  rect.right - rect.left, rect.bottom - rect.top); */
/*        } */
/*        _ecore_win32_event_handle_configure_notify(data); */
/*        return 0; */
     case WM_WINDOWPOSCHANGED:
       {
         RECT rect;
         GetClientRect(window, &rect);
         printf (" *** ecore message : WINDOWPOSCHANGED %ld %ld\n",
                 rect.right - rect.left, rect.bottom - rect.top);
       }
       _ecore_win32_event_handle_configure_notify(data);
       return 0;
     case WM_ENTERSIZEMOVE :
       printf (" * ecore message : WM_ENTERSIZEMOVE \n");
       return 0;
     case WM_EXITSIZEMOVE:
       printf (" * ecore message : WM_EXITSIZEMOVE\n");
       return 0;
       /* GDI notifications */
     case WM_PAINT:
       {
         RECT rect;

         printf (" * ecore message : paint\n");
         if (GetUpdateRect(window, &rect, FALSE))
           {
              PAINTSTRUCT ps;
              HDC         hdc;

              hdc = BeginPaint(window, &ps);
              data->update = rect;
              _ecore_win32_event_handle_expose(data);
              EndPaint(window, &ps);
              printf (" *    %ld %ld %ld %ld\n",
                      rect.left,
                      rect.top,
                      rect.right - rect.left,
                      rect.bottom - rect.top);
           }
         return 0;
       }
     case WM_SETREDRAW:
       printf (" * ecore message : WM_SETREDRAW\n");
       return 0;
     case WM_SYNCPAINT:
       printf (" * ecore message : WM_SYNCPAINT\n");
       return 0;
     default:
       return DefWindowProc(window, message, window_param, data_param);
     }

}
