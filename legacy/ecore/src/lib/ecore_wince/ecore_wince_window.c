/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Evil.h>
#include <Eina.h>

#include "Ecore_WinCE.h"
#include "ecore_wince_private.h"


/***** Private declarations *****/

typedef BOOL (__stdcall *UnregisterFunc1Proc)(UINT, UINT);

static int _ecore_wince_hardware_keys_register(HWND window);


/***** API *****/

Ecore_WinCE_Window *
ecore_wince_window_new(Ecore_WinCE_Window *parent,
                       int                 x,
                       int                 y,
                       int                 width,
                       int                 height)
{
   struct _Ecore_WinCE_Window *w;
   HWND                        window;
   RECT                        rect;

   ECORE_WINCE_MSG_INFO("creating window");

   w = (struct _Ecore_WinCE_Window *)calloc(1, sizeof(struct _Ecore_WinCE_Window));
   if (!w)
     {
        ECORE_WINCE_MSG_ERR("malloc() failed");
        return NULL;
     }

   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!AdjustWindowRectEx(&rect, WS_CAPTION | WS_SYSMENU | WS_VISIBLE, FALSE, WS_EX_TOPMOST))
     {
        ECORE_WINCE_MSG_ERR("AdjustWindowRectEx() failed");
        free(w);
        return NULL;
     }

   window = CreateWindowEx(WS_EX_TOPMOST,
                           ECORE_WINCE_WINDOW_CLASS,
                           L"",
                           WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
                           x, y,
                           rect.right - rect.left, rect.bottom - rect.top,
                           parent ? ((struct _Ecore_WinCE_Window *)parent)->window : NULL,
                           NULL, _ecore_wince_instance, NULL);
   if (!window)
     {
        ECORE_WINCE_MSG_ERR("CreateWindowEx() failed");
        free(w);
        return NULL;
     }

   if (!_ecore_wince_hardware_keys_register(window))
     {
        ECORE_WINCE_MSG_ERR("_ecore_wince_hardware_keys_register() failed");
        DestroyWindow(window);
        free(w);
        return NULL;
     }

   w->window = window;

   SetLastError(0);
   if (!SetWindowLong(window, GWL_USERDATA, (LONG)w) && (GetLastError() != 0))
     {
        ECORE_WINCE_MSG_ERR("SetWindowLong() failed");
        DestroyWindow(window);
        free(w);
        return NULL;
     }

   w->pointer_is_in = 0;

   return w;
}

void
ecore_wince_window_free(Ecore_WinCE_Window *window)
{
   if (!window) return;

   ECORE_WINCE_MSG_INFO("destroying window");

   DestroyWindow(((struct _Ecore_WinCE_Window *)window)->window);
   free(window);
}

void *
ecore_wince_window_hwnd_get(Ecore_WinCE_Window *window)
{
   if (!window)
     return NULL;

   return ((struct _Ecore_WinCE_Window *)window)->window;
}

void
ecore_wince_window_move(Ecore_WinCE_Window *window,
                        int                 x,
                        int                 y)
{
   RECT rect;
   HWND w;

   if (!window || ((struct _Ecore_WinCE_Window *)window)->fullscreen)
     return;

   ECORE_WINCE_MSG_INFO("moving window (%dx%d)", x, y);

   w = ((struct _Ecore_WinCE_Window *)window)->window;
   if (!GetWindowRect(w, &rect))
     {
        ECORE_WINCE_MSG_ERR("GetWindowRect() failed");
        return;
     }

   if (!MoveWindow(w, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   TRUE))
     {
        ECORE_WINCE_MSG_ERR("MoveWindow() failed");
     }
}

void
ecore_wince_window_resize(Ecore_WinCE_Window *window,
                          int                 width,
                          int                 height)
{
   RECT                        rect;
   struct _Ecore_WinCE_Window *w;
   DWORD                       style;
   DWORD                       exstyle;
   int                         x;
   int                         y;

   if (!window || ((struct _Ecore_WinCE_Window *)window)->fullscreen)
     return;

   ECORE_WINCE_MSG_INFO("resizing window (%dx%d)", width, height);

   w = (struct _Ecore_WinCE_Window *)window;
   if (!GetWindowRect(w->window, &rect))
     {
        ECORE_WINCE_MSG_ERR("GetWindowRect() failed");
        return;
     }

   x = rect.left;
   y = rect.top;
   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!(style = GetWindowLong(w->window, GWL_STYLE)))
     {
        ECORE_WINCE_MSG_ERR("GetWindowLong() failed");
        return;
     }
   if (!(exstyle = GetWindowLong(w->window, GWL_EXSTYLE)))
     {
        ECORE_WINCE_MSG_ERR("GetWindowLong() failed");
        return;
     }
   if (!AdjustWindowRectEx(&rect, style, FALSE, exstyle))
     {
        ECORE_WINCE_MSG_ERR("AdjustWindowRectEx() failed");
        return;
     }

   if (!MoveWindow(w->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   FALSE))
     {
        ECORE_WINCE_MSG_ERR("MoveWindow() failed");
     }
}

void
ecore_wince_window_move_resize(Ecore_WinCE_Window *window,
                               int                 x,
                               int                 y,
                               int                 width,
                               int                 height)
{
   RECT                        rect;
   struct _Ecore_WinCE_Window *w;
   DWORD                       style;
   DWORD                       exstyle;

   if (!window || ((struct _Ecore_WinCE_Window *)window)->fullscreen)
     return;

   ECORE_WINCE_MSG_INFO("moving and resizing window (%dx%d %dx%d)", x, y, width, height);

   w = ((struct _Ecore_WinCE_Window *)window);
   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!(style = GetWindowLong(w->window, GWL_STYLE)))
     {
        ECORE_WINCE_MSG_ERR("GetWindowLong() failed");
        return;
     }
   if (!(exstyle = GetWindowLong(w->window, GWL_EXSTYLE)))
     {
        ECORE_WINCE_MSG_ERR("GetWindowLong() failed");
        return;
     }
   if (!AdjustWindowRectEx(&rect, style, FALSE, exstyle))
     {
        ECORE_WINCE_MSG_ERR("AdjustWindowRectEx() failed");
        return;
     }

   if (!MoveWindow(w->window, x, y,
              rect.right - rect.left,
              rect.bottom - rect.top,
              TRUE))
     {
        ECORE_WINCE_MSG_ERR("MoveWindow() failed");
     }
}

void
ecore_wince_window_show(Ecore_WinCE_Window *window)
{
   if (!window) return;

   ECORE_WINCE_MSG_INFO("showing window");

   if (!ShowWindow(((struct _Ecore_WinCE_Window *)window)->window, SW_SHOWNORMAL))
     {
        ECORE_WINCE_MSG_ERR("ShowWindow() failed");
        return;
     }
   if (!UpdateWindow(((struct _Ecore_WinCE_Window *)window)->window))
     {
        ECORE_WINCE_MSG_ERR("UpdateWindow() failed");
     }
   if (!SendMessage(((struct _Ecore_WinCE_Window *)window)->window, WM_SHOWWINDOW, 1, 0))
     {
        ECORE_WINCE_MSG_ERR("SendMessage() failed");
     }
}

void
ecore_wince_window_hide(Ecore_WinCE_Window *window)
{
   if (!window) return;

   ECORE_WINCE_MSG_INFO("hiding window");

   if (!ShowWindow(((struct _Ecore_WinCE_Window *)window)->window, SW_HIDE))
     {
        ECORE_WINCE_MSG_ERR("ShowWindow() failed");
        return;
     }
   if (!SendMessage(((struct _Ecore_WinCE_Window *)window)->window, WM_SHOWWINDOW, 0, 0))
     {
        ECORE_WINCE_MSG_ERR("SendMessage() failed");
     }
}

void
ecore_wince_window_title_set(Ecore_WinCE_Window *window,
                             const char         *title)
{
   wchar_t *wtitle;

   if (!window) return;

   if (!title || !title[0]) return;

   ECORE_WINCE_MSG_INFO("setting window title");

   wtitle = evil_char_to_wchar(title);
   if (!wtitle) return;

   if (!SetWindowText(((struct _Ecore_WinCE_Window *)window)->window, wtitle))
     {
        ECORE_WINCE_MSG_ERR("SetWindowText() failed");
     }
   free(wtitle);
}

void
ecore_wince_window_backend_set(Ecore_WinCE_Window *window, int backend)
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return;

   ECORE_WINCE_MSG_INFO("setting backend");

   w = (struct _Ecore_WinCE_Window *)window;
   w->backend = backend;
}

void
ecore_wince_window_suspend_set(Ecore_WinCE_Window *window, int (*suspend)(int))
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return;

   ECORE_WINCE_MSG_INFO("setting suspend callback");

   w = (struct _Ecore_WinCE_Window *)window;
   w->suspend = suspend;
}

void
ecore_wince_window_resume_set(Ecore_WinCE_Window *window, int (*resume)(int))
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return;

   ECORE_WINCE_MSG_INFO("setting resume callback");

   w = (struct _Ecore_WinCE_Window *)window;
   w->resume = resume;
}

void
ecore_wince_window_geometry_get(Ecore_WinCE_Window *window,
                                int                *x,
                                int                *y,
                                int                *width,
                                int                *height)
{
   RECT rect;
   int  w;
   int  h;

   ECORE_WINCE_MSG_INFO("getting window geometry");

   if (!window)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = GetSystemMetrics(SM_CXSCREEN);
        if (height) *height = GetSystemMetrics(SM_CYSCREEN);

        return;
     }

   if (!GetClientRect(((struct _Ecore_WinCE_Window *)window)->window,
                      &rect))
     {
        ECORE_WINCE_MSG_ERR("GetClientRect() failed");

        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = 0;
        if (height) *height = 0;

        return;
     }

   w = rect.right - rect.left;
   h = rect.bottom - rect.top;

   if (!GetWindowRect(((struct _Ecore_WinCE_Window *)window)->window,
                      &rect))
     {
        ECORE_WINCE_MSG_ERR("GetWindowRect() failed");

        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = 0;
        if (height) *height = 0;

        return;
     }

   if (x) *x = rect.left;
   if (y) *y = rect.top;
   if (width) *width = w;
   if (height) *height = h;
}

void
ecore_wince_window_size_get(Ecore_WinCE_Window *window,
                            int                *width,
                            int                *height)
{
   RECT rect;

   ECORE_WINCE_MSG_INFO("getting window size");

   if (!window)
     {
        if (width) *width = GetSystemMetrics(SM_CXSCREEN);
        if (height) *height = GetSystemMetrics(SM_CYSCREEN);

        return;
     }

   if (!GetClientRect(((struct _Ecore_WinCE_Window *)window)->window,
                      &rect))
     {
        ECORE_WINCE_MSG_ERR("GetClientRect() failed");

        if (width) *width = 0;
        if (height) *height = 0;
     }

   if (width) *width = rect.right - rect.left;
   if (height) *height = rect.bottom - rect.top;
}

void
ecore_wince_window_fullscreen_set(Ecore_WinCE_Window *window,
                                  int                 on)
{
   struct _Ecore_WinCE_Window *ew;
   HWND                        w;
   HWND                        task_bar;

   if (!window) return;

   ew = (struct _Ecore_WinCE_Window *)window;
   if (((ew->fullscreen) && (on)) ||
       ((!ew->fullscreen) && (!on)))
     return;

   ECORE_WINCE_MSG_INFO("setting fullscreen: %s", on ? "yes" : "no");

   ew->fullscreen = !!on;
   w = ew->window;

   if (on)
     {
        /* save the position and size of the window */
        if (!GetWindowRect(w, &ew->rect))
          {
             ECORE_WINCE_MSG_ERR("GetWindowRect() failed");
             return;
          }

        /* hide task bar */
        task_bar = FindWindow(L"HHTaskBar", NULL);
        if (!task_bar)
          {
             ECORE_WINCE_MSG_INFO("FindWindow(): can not find task bar");
          }
        if (!ShowWindow(task_bar, SW_HIDE))
          {
             ECORE_WINCE_MSG_INFO("ShowWindow(): task bar already hidden");
          }
        if (!EnableWindow(task_bar, FALSE))
          {
             ECORE_WINCE_MSG_INFO("EnableWindow(): input already disabled");
          }

        /* style: visible + popup */
        if (!SetWindowLong(w, GWL_STYLE, WS_POPUP | WS_VISIBLE))
          {
             ECORE_WINCE_MSG_INFO("SetWindowLong() failed");
          }

        /* resize window to fit the entire screen */
        if (!SetWindowPos(w, HWND_TOPMOST,
                          0, 0,
                          GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED))
          {
             ECORE_WINCE_MSG_INFO("SetWindowPos() failed");
          }
        /*
         * It seems that SetWindowPos is not sufficient.
         * Call MoveWindow with the correct size and force painting.
         * Note that UpdateWindow (forcing repainting) is not sufficient
         */
        if (!MoveWindow(w,
                        0, 0,
                        GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                        TRUE))
          {
             ECORE_WINCE_MSG_INFO("MoveWindow() failed");
          }
     }
   else
     {
        /* show task bar */
        task_bar = FindWindow(L"HHTaskBar", NULL);
        if (!task_bar)
          {
             ECORE_WINCE_MSG_INFO("FindWindow(): can not find task bar");
          }
        if (!ShowWindow(task_bar, SW_SHOW))
          {
             ECORE_WINCE_MSG_INFO("ShowWindow(): task bar already visible");
          }
        if (!EnableWindow(task_bar, TRUE))
          {
             ECORE_WINCE_MSG_INFO("EnableWindow():  input already enabled");
          }

        /* style: visible + caption + sysmenu */
        if (!SetWindowLong(w, GWL_STYLE, WS_CAPTION | WS_SYSMENU | WS_VISIBLE))
          {
             ECORE_WINCE_MSG_INFO("SetWindowLong() failed");
          }
        /* restaure the position and size of the window */
        if (!SetWindowPos(w, HWND_TOPMOST,
                          ew->rect.left,
                          ew->rect.top,
                          ew->rect.right - ew->rect.left,
                          ew->rect.bottom - ew->rect.top,
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED))
          {
             ECORE_WINCE_MSG_INFO("SetWindowLong() failed");
          }
        /*
         * It seems that SetWindowPos is not sufficient.
         * Call MoveWindow with the correct size and force painting.
         * Note that UpdateWindow (forcing repainting) is not sufficient
         */
        if (!MoveWindow(w,
                        ew->rect.left,
                        ew->rect.top,
                        ew->rect.right - ew->rect.left,
                        ew->rect.bottom - ew->rect.top,
                        TRUE))
          {
             ECORE_WINCE_MSG_INFO("MoveWindow() failed");
          }
     }
}


/***** Private functions definitions *****/

static int
_ecore_wince_hardware_keys_register(HWND window)
{
   HINSTANCE           core_dll;
   UnregisterFunc1Proc unregister_fct;
   int                 i;

   core_dll = LoadLibrary(L"coredll.dll");
   if (!core_dll)
     {
        ECORE_WINCE_MSG_ERR("LoadLibrary() failed");
        return 0;
     }

   unregister_fct = (UnregisterFunc1Proc)GetProcAddress(core_dll, L"UnregisterFunc1");
   if (!unregister_fct)
     {
        ECORE_WINCE_MSG_ERR("GetProcAddress() failed");
        FreeLibrary(core_dll);
        return 0;
     }

   for (i = 0xc1; i <= 0xcf; i++)
     {
        unregister_fct(MOD_WIN, i);
        RegisterHotKey(window, i, MOD_WIN, i);
     }

   FreeLibrary(core_dll);

   return 1;
}
