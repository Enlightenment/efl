/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <stdlib.h>
#include <stdio.h>   /* for printf */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include "Ecore.h"
#include "Ecore_WinCE.h"
#include "ecore_wince_private.h"


/***** Global declarations *****/

double              _ecore_wince_event_last_time = 0.0;

HINSTANCE _ecore_wince_instance = NULL;

EAPI int ECORE_WINCE_EVENT_KEY_DOWN              = 0;
EAPI int ECORE_WINCE_EVENT_KEY_UP                = 0;
EAPI int ECORE_WINCE_EVENT_WINDOW_FOCUS_IN       = 0;
EAPI int ECORE_WINCE_EVENT_WINDOW_FOCUS_OUT      = 0;
EAPI int ECORE_WINCE_EVENT_WINDOW_DAMAGE         = 0;
EAPI int ECORE_WINCE_EVENT_WINDOW_CREATE         = 0;
EAPI int ECORE_WINCE_EVENT_WINDOW_DESTROY        = 0;
EAPI int ECORE_WINCE_EVENT_WINDOW_SHOW           = 0;
EAPI int ECORE_WINCE_EVENT_WINDOW_HIDE           = 0;
EAPI int ECORE_WINCE_EVENT_WINDOW_DELETE_REQUEST = 0;


/***** Private declarations *****/

static int       _ecore_wince_init_count = 0;


LRESULT CALLBACK _ecore_wince_window_procedure(HWND   window,
                                               UINT   message,
                                               WPARAM window_param,
                                               LPARAM data_param);


/***** API *****/

int
ecore_wince_init()
{
   WNDCLASS wc;

   if (_ecore_wince_init_count > 0)
     {
	_ecore_wince_init_count++;
	return _ecore_wince_init_count;
     }

   _ecore_wince_instance = GetModuleHandle(NULL);
   if (!_ecore_wince_instance)
     return 0;

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
        FreeLibrary(_ecore_wince_instance);
        return 0;
     }

   if (!ECORE_WINCE_EVENT_KEY_DOWN)
     {
        ECORE_WINCE_EVENT_KEY_DOWN              = ecore_event_type_new();
        ECORE_WINCE_EVENT_KEY_UP                = ecore_event_type_new();
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
   _ecore_wince_init_count++;
   if (_ecore_wince_init_count > 0) return _ecore_wince_init_count;
   if (!_ecore_wince_instance) return _ecore_wince_init_count;

   UnregisterClass(ECORE_WINCE_WINDOW_CLASS, _ecore_wince_instance);
   FreeLibrary(_ecore_wince_instance);
   _ecore_wince_instance = NULL;

   if (_ecore_wince_init_count < 0) _ecore_wince_init_count = 0;

   return _ecore_wince_init_count;
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
     case WM_HOTKEY:
       printf (" * ecore message : keystroke down (hotkey)\n");
       _ecore_wince_event_handle_key_press(data);
       return 0;
     case WM_KEYDOWN:
       printf (" * ecore message : keystroke down\n");
       _ecore_wince_event_handle_key_press(data);
       return 0;
     case WM_KEYUP:
       printf (" * ecore message : keystroke up\n");
       _ecore_wince_event_handle_key_release(data);
       return 0;
     case WM_SETFOCUS:
       printf (" * ecore message : focus in\n");
       _ecore_wince_event_handle_focus_in(data);
       return 0;
     case WM_KILLFOCUS:
       printf (" * ecore message : focus out\n");
       _ecore_wince_event_handle_focus_out(data);
       return 0;
       /* Window notifications */
     case WM_CREATE:
       {
         RECT rect;
         GetClientRect(window, &rect);
         printf (" *** ecore message : create %ld %ld\n",
                 rect.right - rect.left, rect.bottom - rect.top);
       }
       _ecore_wince_event_handle_create_notify(data);
       return 0;
     case WM_DESTROY:
       printf (" * ecore message : destroy\n");
       _ecore_wince_event_handle_destroy_notify(data);
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
         _ecore_wince_event_handle_map_notify(data);
       else
         _ecore_wince_event_handle_unmap_notify(data);

       return 0;
     case WM_CLOSE:
       printf (" * ecore message : close\n");
       _ecore_wince_event_handle_delete_request(data);
       return 0;
       /* GDI notifications */
     case WM_PAINT:
       {
          RECT rect;

          ValidateRect(window, NULL);
          if (GetUpdateRect(window, &rect, FALSE))
            {
               data->update = rect;
               _ecore_wince_event_handle_expose(data);
            }
          return 0;
       }
     default:
       return DefWindowProc(window, message, window_param, data_param);
     }

}
