#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <windowsx.h>
#include <dbt.h>

#include <Eina.h>
#include <Ecore.h>
#include <Ecore_Input.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

/* OLE IID for Drag'n Drop */

#define INITGUID
#include <basetyps.h>
DEFINE_OLEGUID(IID_IEnumFORMATETC, 0x00000103L, 0, 0);
DEFINE_OLEGUID(IID_IDataObject,    0x0000010EL, 0, 0);
DEFINE_OLEGUID(IID_IDropSource,    0x00000121L, 0, 0);
DEFINE_OLEGUID(IID_IDropTarget,    0x00000122L, 0, 0);
DEFINE_OLEGUID(IID_IUnknown,       0x00000000L, 0, 0);

#define IDI_ICON 101

typedef BOOL WINAPI (*efl_AddClipboardFormatListener)(_In_ HWND hwnd);
typedef BOOL WINAPI (*efl_RemoveClipboardFormatListener)(_In_ HWND hwnd);

static int _ecore_win32_init_count = 0;

LRESULT CALLBACK
_ecore_win32_window_procedure(HWND   window,
                              UINT   message,
                              WPARAM window_param,
                              LPARAM data_param)
{
   Ecore_Win32_Callback_Data  _data;
   Ecore_Win32_Callback_Data *data;
   DWORD                      coord;

   data = &_data;

   data->window = window;
   data->message = message;
   data->window_param = window_param;
   data->data_param = data_param;
   data->timestamp = GetMessageTime();
   coord = GetMessagePos();
   data->x = GET_X_LPARAM(coord);
   data->y = GET_Y_LPARAM(coord);

   switch (data->message)
     {
       /* Keyboard input notifications */
     case WM_KEYDOWN:
     case WM_SYSKEYDOWN:
       INF("key down message");
       _ecore_win32_event_handle_key_press(data);
       return 0;
     /* case WM_CHAR: */
     /* case WM_SYSCHAR: */
     case WM_KEYUP:
     case WM_SYSKEYUP:
       INF("key up message");
       _ecore_win32_event_handle_key_release(data);
       return 0;
     case WM_SETFOCUS:
       INF("setfocus message");
       _ecore_win32_event_handle_focus_in(data);
       return 0;
     case WM_KILLFOCUS:
       INF("kill focus message");
       _ecore_win32_event_handle_focus_out(data);
       return 0;
       /* Mouse input notifications */
     case WM_LBUTTONDOWN:
       INF("left button down message");
       SetCapture(window);
       _ecore_win32_event_handle_button_press(data, 1);
       return 0;
     case WM_MBUTTONDOWN:
       INF("middle button down message");
       _ecore_win32_event_handle_button_press(data, 2);
       return 0;
     case WM_RBUTTONDOWN:
       INF("right button down message");
       _ecore_win32_event_handle_button_press(data, 3);
       return 0;
     case WM_LBUTTONUP:
       {
          Ecore_Win32_Window *w = NULL;

          INF("left button up message");

          ReleaseCapture();
          w = (Ecore_Win32_Window *)GetWindowLongPtr(window, GWLP_USERDATA);
          if (w->drag.dragging)
            {
               w->drag.dragging = EINA_FALSE;
               return 0;
            }

          _ecore_win32_event_handle_button_release(data, 1);
          return 0;
       }
     case WM_MBUTTONUP:
       INF("middle button up message");
       _ecore_win32_event_handle_button_release(data, 2);
       return 0;
     case WM_RBUTTONUP:
       INF("right button up message");
       _ecore_win32_event_handle_button_release(data, 3);
       return 0;
     case WM_MOUSEMOVE:
       {
          RECT                rect;
          Ecore_Win32_Window *w = NULL;

          w = (Ecore_Win32_Window *)GetWindowLongPtr(window, GWLP_USERDATA);

          /*
           * Windows can send several WM_MOUSEMOVE messages, see:
           * http://blogs.msdn.com/b/oldnewthing/archive/2003/10/01/55108.aspx
           * so we discard those which have the same mouse coordinates
           */
          if ((w->drag.current_mouse_x == GET_X_LPARAM(data_param)) &&
              (w->drag.current_mouse_y == GET_Y_LPARAM(data_param)))
            return 0;

          INF("mouse move message");

          w->drag.current_mouse_x = GET_X_LPARAM(data_param);
          w->drag.current_mouse_y = GET_Y_LPARAM(data_param);

          if (w->drag.dragging)
            {
               POINT pt;

               pt.x = GET_X_LPARAM(data_param);
               pt.y = GET_Y_LPARAM(data_param);
               if (ClientToScreen(window, &pt))
                 {
                    if (ecore_win32_window_drag(w, pt.x, pt.y))
                      return 0;
                 }
            }

          if (GetClientRect(window, &rect))
            {
               POINT pt;

               INF("mouse in window");

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
          else
            {
               ERR("GetClientRect() failed");
            }
          _ecore_win32_event_handle_motion_notify(data);

          return 0;
       }
     case WM_MOUSEWHEEL:
       INF("mouse wheel message");
       _ecore_win32_event_handle_button_press(data, 4);
       return 0;
       /* Window notifications */
     case WM_CREATE:
       {
          efl_AddClipboardFormatListener acfl;

          INF("create window message");

          acfl = (efl_AddClipboardFormatListener)GetProcAddress(GetModuleHandle("user32.dll"),
                                                                "AddClipboardFormatListener");
          if (acfl)
            {
               if (!acfl(window))
                 INF("can not create clipboard format listener; no clipboard notification will be sent");
            }
       _ecore_win32_event_handle_create_notify(data);
       return 0;
       }
     case WM_DESTROY:
       {
          efl_RemoveClipboardFormatListener rcfl;

          INF("destroy window message");
          _ecore_win32_event_handle_destroy_notify(data);

          rcfl = (efl_RemoveClipboardFormatListener)GetProcAddress(GetModuleHandle("user32.dll"),
                                                                   "RemoveClipboardFormatListener");
          if (rcfl)
            rcfl(window);
          return 0;
       }
     case WM_SHOWWINDOW:
       INF("show window message");
       if ((data->data_param == SW_OTHERUNZOOM) ||
           (data->data_param == SW_OTHERZOOM))
         return 0;

       if (data->window_param)
         _ecore_win32_event_handle_map_notify(data);
       else
         _ecore_win32_event_handle_unmap_notify(data);

       return 0;
     case WM_CLOSE:
       INF("close window message");
       _ecore_win32_event_handle_delete_request(data);
       return 0;
     case WM_GETMINMAXINFO:
       INF("get min max info window message");
       return TRUE;
     case WM_MOVING:
       INF("moving window message");
       _ecore_win32_event_handle_configure_notify(data, EINA_FALSE);
       return TRUE;
     case WM_MOVE:
       INF("move window message");
       return 0;
     case WM_SIZING:
       INF("sizing window message");
       _ecore_win32_event_handle_resize(data);
       _ecore_win32_event_handle_configure_notify(data, EINA_FALSE);
       return TRUE;
     case WM_SIZE:
       INF("size window message");
       _ecore_win32_event_handle_configure_notify(data, EINA_TRUE);
       return 0;
/*      case WM_WINDOWPOSCHANGING: */
/*        { */
/*          RECT rect; */
/*          GetClientRect(window, &rect); */
/*          printf (" *** ecore message : WINDOWPOSCHANGING %ld %ld\n", */
/*                  rect.right - rect.left, rect.bottom - rect.top); */
/*        } */
/*        _ecore_win32_event_handle_configure_notify(data, EINA_FALSE); */
/*        return 0; */
     case WM_WINDOWPOSCHANGED:
       INF("position changed window message");
       _ecore_win32_event_handle_configure_notify(data, EINA_FALSE);
       _ecore_win32_event_handle_property_notify(data);
       _ecore_win32_event_handle_expose(data);
       return 0;
     case WM_ENTERSIZEMOVE:
       INF("enter size move window message");
       return 0;
     case WM_EXITSIZEMOVE:
       INF("exit size move window message");
       return 0;
     case WM_NCLBUTTONDOWN:
       INF("non client left button down window message");

       if (((DWORD)window_param == HTCAPTION) ||
           ((DWORD)window_param == HTBOTTOM) ||
           ((DWORD)window_param == HTBOTTOMLEFT) ||
           ((DWORD)window_param == HTBOTTOMRIGHT) ||
           ((DWORD)window_param == HTLEFT) ||
           ((DWORD)window_param == HTRIGHT) ||
           ((DWORD)window_param == HTTOP) ||
           ((DWORD)window_param == HTTOPLEFT) ||
           ((DWORD)window_param == HTTOPRIGHT))
         {
           Ecore_Win32_Window *w;

           w = (Ecore_Win32_Window *)GetWindowLongPtr(window, GWLP_USERDATA);
           ecore_win32_window_geometry_get(w,
                                           &w->drag.x, &w->drag.y,
                                           &w->drag.w, &w->drag.h);
           SetCapture(window);
           w->drag.type = (DWORD)window_param;
           w->drag.px = GET_X_LPARAM(data_param);
           w->drag.py = GET_Y_LPARAM(data_param);
           w->drag.dragging = EINA_TRUE;
           if ((DWORD)window_param == HTCAPTION)
             ecore_win32_window_raise(w);
           return 0;
         }
       return DefWindowProc(window, message, window_param, data_param);
     case WM_SYSCOMMAND:
       INF("sys command window message %d", (int)window_param);

       if ((((DWORD)window_param & 0xfff0) == SC_MOVE) ||
           (((DWORD)window_param & 0xfff0) == SC_SIZE))
         {
           Ecore_Win32_Window *w;

           INF("sys command MOVE or SIZE window message : %dx%d", GET_X_LPARAM(data_param), GET_Y_LPARAM(data_param));

           w = (Ecore_Win32_Window *)GetWindowLongPtr(window, GWLP_USERDATA);
           w->drag.dragging = EINA_TRUE;
           return 0;
         }
       return DefWindowProc(window, message, window_param, data_param);
      case WM_CLIPBOARDUPDATE:
       INF("clipboard data updated");
       _ecore_win32_event_handle_selection_notify(data);
       return 0;
      case WM_SETCURSOR:
        {
           Ecore_Win32_Window *w;

           INF("SetCursor");

           /*
            * Do not change the cursors on the non client area
            * we are in the client area if and only if LOWORD(data_param) == 1
            */
           if (LOWORD(data_param) != 1)
             return DefWindowProc(window, message, window_param, data_param);

           /*
            * Otherwise, we use the cursors set by the API for the
            *current window
            */
           w = (Ecore_Win32_Window *)GetWindowLongPtr(window, GWLP_USERDATA);
           if (w && w->cursor)
             {
                SetCursor(w->cursor);
                return 1;
             }
           return 0;
        }
       /* GDI notifications */
     case WM_PAINT:
       {
         RECT rect;

         INF("paint message");

         if (GetUpdateRect(window, &rect, FALSE))
           {
              PAINTSTRUCT ps;

              if (BeginPaint(window, &ps))
                {
                   data->update = rect;
                   _ecore_win32_event_handle_expose(data);
                   EndPaint(window, &ps);
                }
           }
         return 0;
       }
     case WM_SETREDRAW:
       INF("set redraw message");
       return 0;
     case WM_SYNCPAINT:
       INF("sync paint message");
       return 0;
       /* Desktop notifications */
    case WM_DEVICECHANGE:
       if (window == ecore_win32_monitor_window)
         {
            PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)data_param;

            if ((window_param == DBT_DEVICEARRIVAL) &&
                (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE))
              ecore_win32_monitor_update(1);

            if ((window_param == DBT_DEVICEREMOVECOMPLETE) &&
                (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE))
              ecore_win32_monitor_update(2);
         }
     default:
       return DefWindowProc(window, message, window_param, data_param);
     }
}

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/


HINSTANCE            _ecore_win32_instance = NULL;
double               _ecore_win32_double_click_time = 0.25;
unsigned long        _ecore_win32_event_last_time = 0;
Ecore_Win32_Window  *_ecore_win32_event_last_window = NULL;
int                  _ecore_win32_log_dom_global = -1;
Ecore_Win32_Cursor  *_ecore_win32_cursor_x[77];

int ECORE_WIN32_EVENT_MOUSE_IN              = 0;
int ECORE_WIN32_EVENT_MOUSE_OUT             = 0;
int ECORE_WIN32_EVENT_WINDOW_FOCUS_IN       = 0;
int ECORE_WIN32_EVENT_WINDOW_FOCUS_OUT      = 0;
int ECORE_WIN32_EVENT_WINDOW_DAMAGE         = 0;
int ECORE_WIN32_EVENT_WINDOW_CREATE         = 0;
int ECORE_WIN32_EVENT_WINDOW_DESTROY        = 0;
int ECORE_WIN32_EVENT_WINDOW_SHOW           = 0;
int ECORE_WIN32_EVENT_WINDOW_HIDE           = 0;
int ECORE_WIN32_EVENT_WINDOW_CONFIGURE      = 0;
int ECORE_WIN32_EVENT_WINDOW_RESIZE         = 0;
int ECORE_WIN32_EVENT_WINDOW_PROPERTY       = 0;
int ECORE_WIN32_EVENT_WINDOW_DELETE_REQUEST = 0;
int ECORE_WIN32_EVENT_SELECTION_CLEAR       = 0;
int ECORE_WIN32_EVENT_SELECTION_NOTIFY      = 0;

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Ecore_Win32_Group Ecore_Win32 library
 *
 * Ecore_Win32 is a library that wraps Windows graphic functions
 * and integrate them nicely into the Ecore main loop.
 *
 * @section Ecore_Win32_Sec_Init Initialisation / Shutdown
 *
 * To fill...
 *
 * @section Ecore_Win32_Sec_Icons How to set icons to an application
 *
 * It is possible to also sets the icon of the application easily:
 *
 * @li Create an icon with your favorite image creator. The Gimp is a
 * good choice. Create several images of size 16, 32 and 48. You can
 * also create images of size 24, 64, 128 and 256. Paste all of them
 * in the image of size 16 as a layer. Save the image of size 16 with
 * the name my_icon.ico. Put it where the source code of the
 * application is located.
 * @li Create my_icon_rc.rc file with your code editor and add in it:
 * @code
 * 101 ICON DISCARDABLE "my_icon.ico"
 * @endcode
 * @li With Visual Studio, put that file in the 'Resource file' part
 * of the project.
 * @li With MinGW, you have to compile it with windres:
 * @code
 * windres my_icon_rc.rc my_icon_rc.o
 * @endcode
 * and add my_icon_rc.o to the object files of the application.
 *
 * @note The value 101 must not be changed, it's the ID used
 * internally by Ecore_Win32 to get the icons.
 *
 * @{
 */

/**
 * @brief Initialize the Ecore_Win32 library.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up the Windows graphic system. It returns 0 on
 * failure, otherwise it returns the number of times it has already been
 * called.
 *
 * When Ecore_Win32 is not used anymore, call ecore_win32_shutdown()
 * to shut down the Ecore_Win32 library.
 */
EAPI int
ecore_win32_init()
{
   WNDCLASSEX wc;
   HICON      icon;
   HICON      icon_sm;
   int        i;

   if (++_ecore_win32_init_count != 1)
     return _ecore_win32_init_count;

   if (!eina_init())
     return --_ecore_win32_init_count;

   _ecore_win32_log_dom_global = eina_log_domain_register
     ("ecore_win32", ECORE_WIN32_DEFAULT_LOG_COLOR);
   if (_ecore_win32_log_dom_global < 0)
     {
        EINA_LOG_ERR("Ecore_Win32: Could not register log domain");
        goto shutdown_eina;
     }

   if (!ecore_event_init())
     {
        ERR("Ecore_Win32: Could not init ecore_event");
        goto unregister_log_domain;
     }

   _ecore_win32_instance = GetModuleHandle(NULL);
   if (!_ecore_win32_instance)
     {
        ERR("GetModuleHandle() failed");
        goto shutdown_ecore_event;
     }

   ecore_win32_monitor_init();

   icon = LoadImage(_ecore_win32_instance,
                    MAKEINTRESOURCE(IDI_ICON),
                    IMAGE_ICON,
                    GetSystemMetrics(SM_CXICON),
                    GetSystemMetrics(SM_CYICON),
                    LR_DEFAULTCOLOR);
   icon_sm = LoadImage(_ecore_win32_instance,
                       MAKEINTRESOURCE(IDI_ICON),
                       IMAGE_ICON,
                       GetSystemMetrics(SM_CXSMICON),
                       GetSystemMetrics(SM_CYSMICON),
                       LR_DEFAULTCOLOR);
   if (!icon)
     icon = LoadIcon (NULL, IDI_APPLICATION);
   if (!icon_sm)
     icon_sm = LoadIcon (NULL, IDI_APPLICATION);

   memset (&wc, 0, sizeof (WNDCLASSEX));
   wc.cbSize = sizeof (WNDCLASSEX);
   wc.style = 0;
   wc.lpfnWndProc = _ecore_win32_window_procedure;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = _ecore_win32_instance;
   wc.hIcon = icon;
   wc.hCursor = LoadCursor (NULL, IDC_ARROW);
   wc.hbrBackground = (HBRUSH)(1 + COLOR_BTNFACE);
   wc.lpszMenuName =  NULL;
   wc.lpszClassName = ECORE_WIN32_WINDOW_CLASS;
   wc.hIconSm = icon_sm;

   if(!RegisterClassEx(&wc))
     {
        ERR("RegisterClass() failed");
        goto free_library;
     }

   if (!ecore_win32_dnd_init())
     {
        ERR("ecore_win32_dnd_init() failed");
        goto unregister_class;
     }

   ECORE_WIN32_EVENT_MOUSE_IN              = ecore_event_type_new();
   ECORE_WIN32_EVENT_MOUSE_OUT             = ecore_event_type_new();
   ECORE_WIN32_EVENT_WINDOW_FOCUS_IN       = ecore_event_type_new();
   ECORE_WIN32_EVENT_WINDOW_FOCUS_OUT      = ecore_event_type_new();
   ECORE_WIN32_EVENT_WINDOW_DAMAGE         = ecore_event_type_new();
   ECORE_WIN32_EVENT_WINDOW_CREATE         = ecore_event_type_new();
   ECORE_WIN32_EVENT_WINDOW_DESTROY        = ecore_event_type_new();
   ECORE_WIN32_EVENT_WINDOW_SHOW           = ecore_event_type_new();
   ECORE_WIN32_EVENT_WINDOW_HIDE           = ecore_event_type_new();
   ECORE_WIN32_EVENT_WINDOW_CONFIGURE      = ecore_event_type_new();
   ECORE_WIN32_EVENT_WINDOW_RESIZE         = ecore_event_type_new();
   ECORE_WIN32_EVENT_WINDOW_PROPERTY       = ecore_event_type_new();
   ECORE_WIN32_EVENT_WINDOW_DELETE_REQUEST = ecore_event_type_new();
   ECORE_WIN32_EVENT_SELECTION_CLEAR       = ecore_event_type_new();
   ECORE_WIN32_EVENT_SELECTION_NOTIFY      = ecore_event_type_new();

   for (i = 0; i < 77; i++)
     _ecore_win32_cursor_x[i] = _ecore_win32_cursor_x11_shaped_new(i);

   return _ecore_win32_init_count;

 unregister_class:
   UnregisterClass(ECORE_WIN32_WINDOW_CLASS, _ecore_win32_instance);
 free_library:
   FreeLibrary(_ecore_win32_instance);
 shutdown_ecore_event:
   ecore_event_shutdown();
 unregister_log_domain:
   eina_log_domain_unregister(_ecore_win32_log_dom_global);
 shutdown_eina:
   eina_shutdown();

   return --_ecore_win32_init_count;
}

/**
 * @brief Shut down the Ecore_Win32 library.
 *
 * @return 0 when the library is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the Ecore_Win32 library. It returns 0 when it has
 * been called the same number of times than ecore_win32_init(). In that case
 * it shuts down all the Windows graphic system.
 */
EAPI int
ecore_win32_shutdown()
{
   int i;

   if (--_ecore_win32_init_count != 0)
     return _ecore_win32_init_count;

   for (i = 0; i < 77; i++)
     ecore_win32_cursor_free(_ecore_win32_cursor_x[i]);

   ecore_win32_dnd_shutdown();

   ecore_event_type_flush(ECORE_WIN32_EVENT_MOUSE_IN,
                          ECORE_WIN32_EVENT_MOUSE_OUT,
                          ECORE_WIN32_EVENT_WINDOW_FOCUS_IN,
                          ECORE_WIN32_EVENT_WINDOW_FOCUS_OUT,
                          ECORE_WIN32_EVENT_WINDOW_DAMAGE,
                          ECORE_WIN32_EVENT_WINDOW_CREATE,
                          ECORE_WIN32_EVENT_WINDOW_DESTROY,
                          ECORE_WIN32_EVENT_WINDOW_SHOW,
                          ECORE_WIN32_EVENT_WINDOW_HIDE,
                          ECORE_WIN32_EVENT_WINDOW_CONFIGURE,
                          ECORE_WIN32_EVENT_WINDOW_RESIZE,
                          ECORE_WIN32_EVENT_WINDOW_PROPERTY,
                          ECORE_WIN32_EVENT_WINDOW_DELETE_REQUEST,
                          ECORE_WIN32_EVENT_SELECTION_CLEAR,
                          ECORE_WIN32_EVENT_SELECTION_NOTIFY);

   if (!UnregisterClass(ECORE_WIN32_WINDOW_CLASS, _ecore_win32_instance))
     INF("UnregisterClass() failed");

   ecore_win32_monitor_shutdown();

   if (!FreeLibrary(_ecore_win32_instance))
     INF("FreeLibrary() failed");

   _ecore_win32_instance = NULL;

   ecore_event_shutdown();
   eina_log_domain_unregister(_ecore_win32_log_dom_global);
   _ecore_win32_log_dom_global = -1;
   eina_shutdown();

   return _ecore_win32_init_count;
}

/**
 * @brief Retrieve the depth of the screen.
 *
 * @return The depth of the screen.
 *
 * This function returns the depth of the screen. If an error occurs,
 * it returns 0.
 */
EAPI int
ecore_win32_screen_depth_get()
{
   HDC dc;
   int depth;

   INF("getting screen depth");

   dc = GetDC(NULL);
   if (!dc)
     {
        ERR("GetDC() failed");
        return 0;
     }

   depth = GetDeviceCaps(dc, BITSPIXEL);
   if (!ReleaseDC(NULL, dc))
     {
        ERR("ReleaseDC() failed (device context not released)");
     }

   return depth;
}

/**
 * @brief Sets the timeout for a double and triple clicks to be flagged.
 *
 * @param t The time in seconds.
 *
 * This function sets the time @p t between clicks before the
 * double_click flag is set in a button down event. If 3 clicks occur
 * within double this time, the triple_click flag is also set.
 */
EAPI void
ecore_win32_double_click_time_set(double t)
{
   if (t < 0.0) t = 0.0;
   _ecore_win32_double_click_time = t;
}

/**
 * @brief Retrieve the double and triple click flag timeout.
 *
 * @return The timeout for double clicks in seconds.
 *
 * This function returns the double clicks in seconds. If
 * ecore_win32_double_click_time_set() has not been called, the
 * default value is returned. See ecore_win32_double_click_time_set()
 * for more informations.
 */
EAPI double
ecore_win32_double_click_time_get(void)
{
   return _ecore_win32_double_click_time;
}

/**
 * @brief Return the last event time.
 *
 * @return The last envent time.
 *
 * This function returns the last event time.
 */
EAPI unsigned long
ecore_win32_current_time_get(void)
{
   return _ecore_win32_event_last_time;
}

/**
 * @}
 */
