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

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */


typedef BOOL (__stdcall *UnregisterFunc1Proc)(UINT, UINT);

static int
_ecore_wince_hardware_keys_register(HWND window)
{
   HINSTANCE           core_dll;
   UnregisterFunc1Proc unregister_fct;
   int                 i;

   core_dll = LoadLibrary(L"coredll.dll");
   if (!core_dll)
     {
        ERR("LoadLibrary() failed");
        return 0;
     }

   unregister_fct = (UnregisterFunc1Proc)GetProcAddress(core_dll, L"UnregisterFunc1");
   if (!unregister_fct)
     {
        ERR("GetProcAddress() failed");
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

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

/**
 * @addtogroup Ecore_WinCE_Group Ecore_WinCE library
 *
 * @{
 */

/**
 * @brief Creates a new window.
 *
 * @param parent The parent window.
 * @param x The x coordinate of the top-left corner of the window.
 * @param y The y coordinate of the top-left corner of the window.
 * @param width The width of the window.
 * @param height The height of hte window.
 * @return A newly allocated window.
 *
 * This function creates a new window which parent is @p parent. @p width and
 * @p height are the size of the window content (the client part),
 * without the border and title bar. @p x and @p y are the system
 * coordinates of the top left cerner of the window (that is, of the
 * title bar). This function returns a newly created window on
 * success, and @c NULL on failure.
 */
EAPI Ecore_WinCE_Window *
ecore_wince_window_new(Ecore_WinCE_Window *parent,
                       int                 x,
                       int                 y,
                       int                 width,
                       int                 height)
{
   Ecore_WinCE_Window *w;
   HWND                window;
   RECT                rect;

   INF("creating window");

   w = (Ecore_WinCE_Window *)calloc(1, sizeof(Ecore_WinCE_Window));
   if (!w)
     {
        ERR("malloc() failed");
        return NULL;
     }

   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!AdjustWindowRectEx(&rect, WS_CAPTION | WS_SYSMENU | WS_VISIBLE, FALSE, WS_EX_TOPMOST))
     {
        ERR("AdjustWindowRectEx() failed");
        free(w);
        return NULL;
     }

   window = CreateWindowEx(WS_EX_TOPMOST,
                           ECORE_WINCE_WINDOW_CLASS,
                           L"",
                           WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
                           x, y,
                           rect.right - rect.left, rect.bottom - rect.top,
                           parent ? ((Ecore_WinCE_Window *)parent)->window : NULL,
                           NULL, _ecore_wince_instance, NULL);
   if (!window)
     {
        ERR("CreateWindowEx() failed");
        free(w);
        return NULL;
     }

   if (!_ecore_wince_hardware_keys_register(window))
     {
        ERR("_ecore_wince_hardware_keys_register() failed");
        DestroyWindow(window);
        free(w);
        return NULL;
     }

   w->window = window;

   SetLastError(0);
   if (!SetWindowLong(window, GWL_USERDATA, (LONG)w) && (GetLastError() != 0))
     {
        ERR("SetWindowLong() failed");
        DestroyWindow(window);
        free(w);
        return NULL;
     }

   w->pointer_is_in = 0;

   return w;
}

/**
 * @brief Free the given window.
 *
 * @param window The window to free.
 *
 * This function frees @p window. If @p window is @c NULL, this
 * function does nothing.
 */
EAPI void
ecore_wince_window_free(Ecore_WinCE_Window *window)
{
   if (!window) return;

   INF("destroying window");

   DestroyWindow(window->window);
   free(window);
}

/**
 * @brief Return the window HANDLE associated to the given window.
 *
 * @param window The window to retrieve the HANDLE from.
 *
 * This function returns the window HANDLE associated to @p window. If
 * @p window is @c NULL, this function returns @c NULL.
 */
EAPI void *
ecore_wince_window_hwnd_get(Ecore_WinCE_Window *window)
{
   if (!window)
     return NULL;

   return window->window;
}

/**
 * @brief Move the given window to a given position.
 *
 * @param window The window to move.
 * @param x The x coordinate of the destination position.
 * @param y The y coordinate of the destination position.
 *
 * This function move @p window to the new position of coordinates @p x
 * and @p y. If @p window is @c NULL, or if it is fullscreen, or on
 * error, this function does nothing.
 */
EAPI void
ecore_wince_window_move(Ecore_WinCE_Window *window,
                        int                 x,
                        int                 y)
{
   RECT rect;

   if (!window || window->fullscreen)
     return;

   INF("moving window (%dx%d)", x, y);

   if (!GetWindowRect(window->window, &rect))
     {
        ERR("GetWindowRect() failed");
        return;
     }

   if (!MoveWindow(window->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   TRUE))
     {
        ERR("MoveWindow() failed");
     }
}

/**
 * @brief Resize the given window to a given size.
 *
 * @param window The window to resize.
 * @param width The new width.
 * @param height The new height.
 *
 * This function resize @p window to the new @p width and @p height.
 * If @p window is @c NULL, or if it is fullscreen, or on error, this
 * function does nothing.
 */
EAPI void
ecore_wince_window_resize(Ecore_WinCE_Window *window,
                          int                 width,
                          int                 height)
{
   RECT  rect;
   DWORD style;
   DWORD exstyle;
   int   x;
   int   y;

   if (!window || window->fullscreen)
     return;

   INF("resizing window (%dx%d)", width, height);

   if (!GetWindowRect(window->window, &rect))
     {
        ERR("GetWindowRect() failed");
        return;
     }

   x = rect.left;
   y = rect.top;
   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!(style = GetWindowLong(window->window, GWL_STYLE)))
     {
        ERR("GetWindowLong() failed");
        return;
     }
   if (!(exstyle = GetWindowLong(window->window, GWL_EXSTYLE)))
     {
        ERR("GetWindowLong() failed");
        return;
     }
   if (!AdjustWindowRectEx(&rect, style, FALSE, exstyle))
     {
        ERR("AdjustWindowRectEx() failed");
        return;
     }

   if (!MoveWindow(window->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   FALSE))
     {
        ERR("MoveWindow() failed");
     }
}

/**
 * @brief Move and resize the given window to a given position and size.
 *
 * @param window The window to move and resize.
 * @param x The x coordinate of the destination position.
 * @param y The x coordinate of the destination position.
 * @param width The new width.
 * @param height The new height.
 *
 * This function resize @p window to the new position of coordinates @p x
 * and @p y and the new @p width and @p height. If @p window is @c NULL,
 * or if it is fullscreen, or on error, this function does nothing.
 */
EAPI void
ecore_wince_window_move_resize(Ecore_WinCE_Window *window,
                               int                 x,
                               int                 y,
                               int                 width,
                               int                 height)
{
   RECT                rect;
   DWORD               style;
   DWORD               exstyle;

   if (!window || window->fullscreen)
     return;

   INF("moving and resizing window (%dx%d %dx%d)", x, y, width, height);

   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!(style = GetWindowLong(window->window, GWL_STYLE)))
     {
        ERR("GetWindowLong() failed");
        return;
     }
   if (!(exstyle = GetWindowLong(window->window, GWL_EXSTYLE)))
     {
        ERR("GetWindowLong() failed");
        return;
     }
   if (!AdjustWindowRectEx(&rect, style, FALSE, exstyle))
     {
        ERR("AdjustWindowRectEx() failed");
        return;
     }

   if (!MoveWindow(window->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   TRUE))
     {
        ERR("MoveWindow() failed");
     }
}

/**
 * @brief Show the given window.
 *
 * @param window The window to show.
 *
 * This function shows @p window. If @p window is @c NULL, or on
 * error, this function does nothing.
 */
EAPI void
ecore_wince_window_show(Ecore_WinCE_Window *window)
{
   if (!window) return;

   INF("showing window");

   if (!ShowWindow(window->window, SW_SHOWNORMAL))
     {
        ERR("ShowWindow() failed");
        return;
     }
   if (!UpdateWindow(window->window))
     {
        ERR("UpdateWindow() failed");
     }
   if (!SendMessage(window->window, WM_SHOWWINDOW, 1, 0))
     {
        ERR("SendMessage() failed");
     }
}

/**
 * @brief Hide the given window.
 *
 * @param window The window to show.
 *
 * This function hides @p window. If @p window is @c NULL, or on
 * error, this function does nothing.
 */
EAPI void
ecore_wince_window_hide(Ecore_WinCE_Window *window)
{
   if (!window) return;

   INF("hiding window");

   if (!ShowWindow(window->window, SW_HIDE))
     {
        ERR("ShowWindow() failed");
        return;
     }
   if (!SendMessage(window->window, WM_SHOWWINDOW, 0, 0))
     {
        ERR("SendMessage() failed");
     }
}

/**
 * @brief Set the title of the given window.
 *
 * @param window The window to set the title.
 * @param title The new title.
 *
 * This function sets the title of @p window to @p title. If @p window
 * is @c NULL, or if @p title is @c NULL or empty, or on error, this
 * function does nothing.
 */
EAPI void
ecore_wince_window_title_set(Ecore_WinCE_Window *window,
                             const char         *title)
{
   wchar_t *wtitle;

   if (!window) return;

   if (!title || !title[0]) return;

   INF("setting window title");

   wtitle = evil_char_to_wchar(title);
   if (!wtitle) return;

   if (!SetWindowText(window->window, wtitle))
     {
        ERR("SetWindowText() failed");
     }
   free(wtitle);
}

/**
 * @brief Set the graphic backend used for the given window.
 *
 * @param window The window.
 * @param backend The backend.
 *
 * This function sets the graphic backend to use with @p window to
 * @p backend. If @p window if @c NULL, this function does nothing.
 *
 * The valid values for @p backend are
 *
 * @li 0: automatic choice of the backend.
 * @li 1: the framebuffer (fast but could be not well suported).
 * @li 2: GAPI (less fast but almost always supported).
 * @li 3: DirectDraw (less fast than GAPI but almost always
 * supported).
 * @li 4: GDI (the slowest but always supported).
 *
 * The @p backend is used only in Evas and Ecore_Evas. So this
 * function should not be called if Ecore_Evas is used.
 */
EAPI void
ecore_wince_window_backend_set(Ecore_WinCE_Window *window,
                               int                 backend)
{
   if (!window)
     return;

   INF("setting backend");

   window->backend = backend;
}

/**
 * @brief Set the suspend callback used for the given window.
 *
 * @param window The window.
 * @param suspend_cb The suspend callback.
 *
 * This function sets the suspend callback to use with @p window to
 * @p suspend_cb. If @p window if @c NULL, this function does nothing.
 *
 * The @p suspend_cb is used only in Evas and Ecore_Evas. So this
 * function should not be called if Ecore_Evas is used.
 */
EAPI void
ecore_wince_window_suspend_cb_set(Ecore_WinCE_Window *window, int (*suspend_cb)(int))
{
   if (!window)
     return;

   INF("setting suspend callback");

   window->suspend_cb = suspend_cb;
}

/**
 * @brief Set the resume callback used for the given window.
 *
 * @param window The window.
 * @param resume_cb The resume callback.
 *
 * This function sets the resume callback to use with @p window to
 * @p resume_cb. If @p window if @c NULL, this function does nothing.
 *
 * The @p resume_cb is used only in Evas and Ecore_Evas. So this
 * function should not be called if Ecore_Evas is used.
 */
EAPI void
ecore_wince_window_resume_cb_set(Ecore_WinCE_Window *window, int (*resume_cb)(int))
{
   if (!window)
     return;

   INF("setting resume callback");

   window->resume_cb = resume_cb;
}

/**
 * @brief Get the geometry of the given window.
 *
 * @param window The window to retrieve the geometry from.
 * @param x The x coordinate of the position.
 * @param y The x coordinate of the position.
 * @param width The width.
 * @param height The height.
 *
 * This function retrieves the position and size of @p window. @p x,
 * @p y, @p width and @p height can be buffers that will be filled with
 * the corresponding values. If one of them is @c NULL, nothing will
 * be done for that parameter. If @p window is @c NULL, and if the
 * buffers are not @c NULL, they will be filled with respectively 0,
 * 0, the size of the screen and the height of the screen.
 */
EAPI void
ecore_wince_window_geometry_get(Ecore_WinCE_Window *window,
                                int                *x,
                                int                *y,
                                int                *width,
                                int                *height)
{
   RECT rect;
   int  w;
   int  h;

   INF("getting window geometry");

   if (!window)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = GetSystemMetrics(SM_CXSCREEN);
        if (height) *height = GetSystemMetrics(SM_CYSCREEN);

        return;
     }

   if (!GetClientRect(window->window, &rect))
     {
        ERR("GetClientRect() failed");

        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = 0;
        if (height) *height = 0;

        return;
     }

   w = rect.right - rect.left;
   h = rect.bottom - rect.top;

   if (!GetWindowRect(window->window, &rect))
     {
        ERR("GetWindowRect() failed");

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

/**
 * @brief Get the size of the given window.
 *
 * @param window The window to retrieve the size from.
 * @param width The width.
 * @param height The height.
 *
 * This function retrieves the size of @p window. @p width and
 * @p height can be buffers that will be filled with the corresponding
 * values. If one of them is @c NULL, nothing will be done for that
 * parameter. If @p window is @c NULL, and if the buffers are not
 * @c NULL, they will be filled with respectively the size of the screen
 * and the height of the screen.
 */
EAPI void
ecore_wince_window_size_get(Ecore_WinCE_Window *window,
                            int                *width,
                            int                *height)
{
   RECT rect;

   INF("getting window size");

   if (!window)
     {
        if (width) *width = GetSystemMetrics(SM_CXSCREEN);
        if (height) *height = GetSystemMetrics(SM_CYSCREEN);

        return;
     }

   if (!GetClientRect(window->window, &rect))
     {
        ERR("GetClientRect() failed");

        if (width) *width = 0;
        if (height) *height = 0;
     }

   if (width) *width = rect.right - rect.left;
   if (height) *height = rect.bottom - rect.top;
}

/**
 * @brief Set the given window to fullscreen.
 *
 * @param window The window.
 * @param on EINA_TRUE for fullscreen mode, EINA_FALSE for windowed mode.
 *
 * This function set @p window to fullscreen or windowed mode. If @p on
 * is set to EINA_TRUE, the window will be fullscreen, if it is set to
 * EINA_FALSE, it will be windowed. If @p window is @c NULL or if the
 * state does not change (like setting to fullscreen while the window
 * is already fullscreen), this function does nothing.
 */
EAPI void
ecore_wince_window_fullscreen_set(Ecore_WinCE_Window *window,
                                  Eina_Bool           on)
{
   HWND task_bar;

   if (!window) return;

   if (((window->fullscreen) && (on)) ||
       ((!window->fullscreen) && (!on)))
     return;

   INF("setting fullscreen: %s", on ? "yes" : "no");

   window->fullscreen = !!on;

   if (on)
     {
        /* save the position and size of the window */
        if (!GetWindowRect(window->window, &window->rect))
          {
             ERR("GetWindowRect() failed");
             return;
          }

        /* hide task bar */
        task_bar = FindWindow(L"HHTaskBar", NULL);
        if (!task_bar)
          {
             INF("FindWindow(): can not find task bar");
          }
        if (!ShowWindow(task_bar, SW_HIDE))
          {
             INF("ShowWindow(): task bar already hidden");
          }
        if (!EnableWindow(task_bar, FALSE))
          {
             INF("EnableWindow(): input already disabled");
          }

        /* style: visible + popup */
        if (!SetWindowLong(window->window, GWL_STYLE, WS_POPUP | WS_VISIBLE))
          {
             INF("SetWindowLong() failed");
          }

        /* resize window to fit the entire screen */
        if (!SetWindowPos(window->window, HWND_TOPMOST,
                          0, 0,
                          GetSystemMetrics(SM_CXSCREEN),
                          GetSystemMetrics(SM_CYSCREEN),
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED))
          {
             INF("SetWindowPos() failed");
          }
        /*
         * It seems that SetWindowPos is not sufficient.
         * Call MoveWindow with the correct size and force painting.
         * Note that UpdateWindow (forcing repainting) is not sufficient
         */
        if (!MoveWindow(window->window,
                        0, 0,
                        GetSystemMetrics(SM_CXSCREEN),
                        GetSystemMetrics(SM_CYSCREEN),
                        TRUE))
          {
             INF("MoveWindow() failed");
          }
     }
   else
     {
        /* show task bar */
        task_bar = FindWindow(L"HHTaskBar", NULL);
        if (!task_bar)
          {
             INF("FindWindow(): can not find task bar");
          }
        if (!ShowWindow(task_bar, SW_SHOW))
          {
             INF("ShowWindow(): task bar already visible");
          }
        if (!EnableWindow(task_bar, TRUE))
          {
             INF("EnableWindow():  input already enabled");
          }

        /* style: visible + caption + sysmenu */
        if (!SetWindowLong(window->window, GWL_STYLE, WS_CAPTION | WS_SYSMENU | WS_VISIBLE))
          {
             INF("SetWindowLong() failed");
          }
        /* restaure the position and size of the window */
        if (!SetWindowPos(window->window, HWND_TOPMOST,
                          window->rect.left,
                          window->rect.top,
                          window->rect.right - window->rect.left,
                          window->rect.bottom - window->rect.top,
                          SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED))
          {
             INF("SetWindowLong() failed");
          }
        /*
         * It seems that SetWindowPos is not sufficient.
         * Call MoveWindow with the correct size and force painting.
         * Note that UpdateWindow (forcing repainting) is not sufficient
         */
        if (!MoveWindow(window->window,
                        window->rect.left,
                        window->rect.top,
                        window->rect.right - window->rect.left,
                        window->rect.bottom - window->rect.top,
                        TRUE))
          {
             INF("MoveWindow() failed");
          }
     }
}

/**
 * @}
 */
