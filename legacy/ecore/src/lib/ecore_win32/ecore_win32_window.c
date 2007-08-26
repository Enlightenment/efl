/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <Ecore.h>

#include "ecore_win32_private.h"


typedef enum _Ecore_Win32_Window_Z_Order Ecore_Win32_Window_Z_Order;
enum _Ecore_Win32_Window_Z_Order
{
  ECORE_WIN32_WINDOW_Z_ORDER_BOTTOM,
  ECORE_WIN32_WINDOW_Z_ORDER_NOTOPMOST,
  ECORE_WIN32_WINDOW_Z_ORDER_TOP,
  ECORE_WIN32_WINDOW_Z_ORDER_TOPMOST
};


EAPI Ecore_Win32_Window *
ecore_win32_window_new(Ecore_Win32_Window *parent,
                       int                 x,
                       int                 y,
                       int                 width,
                       int                 height)
{
   RECT                        rect;
   struct _Ecore_Win32_Window *w;
   int                         minimal_width;
   int                         minimal_height;

   w = (struct _Ecore_Win32_Window *)calloc(1, sizeof(struct _Ecore_Win32_Window));
   if (!w)
     return NULL;

   printf (" * ecore : new debut : %d %d %d\n",
           width, height, GetSystemMetrics(SM_CXMIN));
   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_SIZEBOX, FALSE))
     {
        free(w);
        return NULL;
     }
   printf (" * ecore : new debut : %ld %d\n",
           rect.right - rect.left, GetSystemMetrics(SM_CXMIN));

   minimal_width = GetSystemMetrics(SM_CXMIN);
   minimal_height = GetSystemMetrics(SM_CYMIN);
   if (((rect.right - rect.left) < minimal_width) ||
       ((rect.bottom - rect.top) < minimal_height))
     {
        printf ("[Ecore] [Win32] ERROR !!\n");
        printf ("                Wrong size %ld\n", rect.right - rect.left);
        free(w);
        return NULL;
     }

   w->window = CreateWindow(ECORE_WIN32_WINDOW_CLASS, "",
                            WS_OVERLAPPEDWINDOW | WS_SIZEBOX,
                            x, y,
                            rect.right - rect.left,
                            rect.bottom - rect.top,
                            parent ? ((struct _Ecore_Win32_Window *)parent)->window : NULL,
                            NULL, _ecore_win32_instance, NULL);
   if (!w->window)
     {
        free(w);
        return NULL;
     }

   if (ecore_list_append(_ecore_win32_windows_list, w) == FALSE)
     {
        ecore_win32_ddraw_shutdown(w);
        DestroyWindow(w->window);
        free(w);
        return NULL;
     }

   w->min_width = width;
   w->min_height = height;
   w->max_width = width;
   w->max_height = height;
   w->base_width = 0;
   w->base_height = 0;
   w->step_width = 1;
   w->step_height = 1;

   w->pointer_is_in = 0;
   w->borderless = 0;
   w->iconified = 0;
   w->fullscreen = 0;

   return w;
}

EAPI void
ecore_win32_window_del(Ecore_Win32_Window *window)
{
   Ecore_Win32_Window *w;

   if (!window) return;

   ecore_list_first_goto(_ecore_win32_windows_list);
   while ((w = ecore_list_next(_ecore_win32_windows_list)))
     {
       if (w == window)
         {
            ecore_list_remove(_ecore_win32_windows_list);
            break;
         }
     }
   ecore_list_remove(_ecore_win32_windows_list);

   ecore_win32_ddraw_shutdown(window);
   DestroyWindow(((struct _Ecore_Win32_Window *)window)->window);
   free(window);
}

/*
EAPI void
ecore_win32_window_configure(Ecore_Win32_Window        *window,
                             Ecore_Win32_Window_Z_Order order,
                             int                        x,
                             int                        y,
                             int                        width,
                             int                        height)
{
  HWND w;

  switch (order)
    {
    case ECORE_WIN32_WINDOW_Z_ORDER_BOTTOM:
      w = HWND_BOTTOM;
      break;
    case ECORE_WIN32_WINDOW_Z_ORDER_NOTOPMOST:
      w = HWND_NOTOPMOST;
      break;
    case ECORE_WIN32_WINDOW_Z_ORDER_TOP:
      w = HWND_TOP;
      break;
    case ECORE_WIN32_WINDOW_Z_ORDER_TOPMOST:
      w = HWND_TOPMOST;
      break;
    default:
      return;
    }
  SetWindowPos((struct _Ecore_Win32_Window *)window->window, w, x, y, width, height, ???);
}
*/

EAPI void
ecore_win32_window_move(Ecore_Win32_Window *window,
                        int                 x,
                        int                 y)
{
   RECT rect;
   HWND w;

   if (!window) return;

   w = ((struct _Ecore_Win32_Window *)window)->window;
   if (!GetWindowRect(w, &rect))
     return;

   MoveWindow(w, x, y,
              rect.right - rect.left,
              rect.bottom - rect.top,
              TRUE);
}

EAPI void
ecore_win32_window_resize(Ecore_Win32_Window *window,
                          int                 width,
                          int                 height)
{
   RECT                        rect;
   struct _Ecore_Win32_Window *w;
   DWORD                       style;
   int                         x;
   int                         y;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   if (!GetWindowRect(w->window, &rect)) return;

   x = rect.left;
   y = rect.top;
   rect.left = 0;
   rect.top = 0;
   if (width < w->min_width) width = w->min_width;
   if (width > w->max_width) width = w->max_width;
   if (height < w->min_height) height = w->min_height;
   if (height > w->max_height) height = w->max_height;
   rect.right = width;
   rect.bottom = height;
   style = GetWindowLong(w->window, GWL_STYLE);
   if (!AdjustWindowRect(&rect, style, FALSE))
     return;

   MoveWindow(w->window, x, y,
              rect.right - rect.left,
              rect.bottom - rect.top,
              TRUE);
}

EAPI void
ecore_win32_window_move_resize(Ecore_Win32_Window *window,
                               int                 x,
                               int                 y,
                               int                 width,
                               int                 height)
{
   RECT                        rect;
   struct _Ecore_Win32_Window *w;
   DWORD                       style;

   if (!window) return;

   w = ((struct _Ecore_Win32_Window *)window);
   rect.left = 0;
   rect.top = 0;
   if (width < w->min_width) width = w->min_width;
   if (width > w->max_width) width = w->max_width;
   if (height < w->min_height) height = w->min_height;
   if (height > w->max_height) height = w->max_height;
   rect.right = width;
   rect.bottom = height;
   style = GetWindowLong(w->window, GWL_STYLE);
   if (!AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW | WS_SIZEBOX, FALSE))
     return;

   MoveWindow(w->window, x, y,
              rect.right - rect.left,
              rect.bottom - rect.top,
              TRUE);
}

EAPI void
ecore_win32_window_size_get(Ecore_Win32_Window *window,
                            int                *width,
                            int                *height)
{
   RECT rect;

   if (!window) return;

   if (!GetClientRect(((struct _Ecore_Win32_Window *)window)->window,
                      &rect))
     {
        if (width) *width = 0;
        if (height) *height = 0;
     }

   if (width) *width = rect.right - rect.left;
   if (height) *height = rect.bottom - rect.top;
}

EAPI void
ecore_win32_window_size_min_set(Ecore_Win32_Window *window,
                                int                 min_width,
                                int                 min_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   w->min_width = min_width;
   w->min_height = min_height;
}

EAPI void
ecore_win32_window_size_max_set(Ecore_Win32_Window *window,
                                int                 max_width,
                                int                 max_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   w->max_width = max_width;
   w->max_height = max_height;
}

EAPI void
ecore_win32_window_size_base_set(Ecore_Win32_Window *window,
                                 int                 base_width,
                                 int                 base_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   w->base_width = base_width;
   w->base_height = base_height;
}

EAPI void
ecore_win32_window_size_step_set(Ecore_Win32_Window *window,
                                 int                 step_width,
                                 int                 step_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   w->step_width = step_width;
   w->step_height = step_height;
}

/* TODO: ecore_win32_window_shaped_set */

EAPI void
ecore_win32_window_show(Ecore_Win32_Window *window)
{
   if (!window) return;

   ShowWindow(((struct _Ecore_Win32_Window *)window)->window, SW_SHOWNORMAL);
   UpdateWindow(((struct _Ecore_Win32_Window *)window)->window);
}

/* FIXME: seems to block the taskbar */
EAPI void
ecore_win32_window_hide(Ecore_Win32_Window *window)
{
   if (!window) return;

   ShowWindow(((struct _Ecore_Win32_Window *)window)->window, SW_HIDE);
}

EAPI void
ecore_win32_window_raise(Ecore_Win32_Window *window)
{
   if (!window) return;

   SetWindowPos(((struct _Ecore_Win32_Window *)window)->window,
                HWND_TOP, 0, 0, 0, 0,
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}

EAPI void
ecore_win32_window_lower(Ecore_Win32_Window *window)
{
   if (!window) return;

   SetWindowPos(((struct _Ecore_Win32_Window *)window)->window,
                HWND_BOTTOM, 0, 0, 0, 0,
                SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}

EAPI void
ecore_win32_window_title_set(Ecore_Win32_Window *window,
                             const char         *title)
{
   if (!window) return;

   if (!title || !title[0]) return;

   SetWindowText(((struct _Ecore_Win32_Window *)window)->window, title);
}

EAPI void
ecore_win32_window_focus_set(Ecore_Win32_Window *window)
{
   if (!window) return;

   SetFocus(((struct _Ecore_Win32_Window *)window)->window);
}

EAPI void
ecore_win32_window_iconified_set(Ecore_Win32_Window *window,
                                 int                 on)
{
   struct _Ecore_Win32_Window *ew;

   if (!window) return;

   ew = (struct _Ecore_Win32_Window *)window;
   if (((ew->iconified) && (on)) ||
       ((!ew->iconified) && (!on)))
     return;

   if (on)
     {
        ShowWindow(ew->window,
                   SW_MINIMIZE);
     }
   else
     {
        ShowWindow(ew->window,
                   SW_RESTORE);
     }
   ew->iconified = on;
}

EAPI void
ecore_win32_window_borderless_set(Ecore_Win32_Window *window,
                                  int                 on)
{
   RECT                        rect;
   DWORD                       style;
   struct _Ecore_Win32_Window *ew;
   HWND                        w;

   if (!window) return;

   ew = (struct _Ecore_Win32_Window *)window;
   if (((ew->borderless) && (on)) ||
       ((!ew->borderless) && (!on)))
     return;

   w = ew->window;

   style = GetWindowLong(w, GWL_STYLE);
   if (on)
     {
        if (!GetClientRect(w, &rect)) return;
        SetWindowLong(w, GWL_STYLE, style & ~WS_CAPTION);
     }
   else
     {
        if (!GetWindowRect(w, &rect)) return;
        style |= WS_CAPTION;
        AdjustWindowRect (&rect, style, FALSE);
        SetWindowLong(w, GWL_STYLE, style);
     }
   SetWindowPos(w, HWND_TOPMOST,
                rect.left, rect.top,
                rect.right - rect.left, rect.bottom - rect.top,
                SWP_NOMOVE | SWP_FRAMECHANGED);
   ew->borderless = on;
}

EAPI void
ecore_win32_window_fullscreen_set(Ecore_Win32_Window *window,
                                  int                 on)
{
   struct _Ecore_Win32_Window *ew;
   HWND                        w;
   int                         width;
   int                         height;

   if (!window) return;

   ew = (struct _Ecore_Win32_Window *)window;
   if (((ew->fullscreen) && (on)) ||
       ((!ew->fullscreen) && (!on)))
     return;

   w = ew->window;

   if (on)
     {
       if (!GetWindowRect(w, &ew->rect)) return;
        width = GetSystemMetrics (SM_CXSCREEN);
        height = GetSystemMetrics (SM_CYSCREEN);
        ew->style = GetWindowLong(w, GWL_STYLE);
        SetWindowLong(w, GWL_STYLE, (ew->style & ~WS_OVERLAPPEDWINDOW) | WS_POPUP);
        SetWindowPos(w, HWND_TOP, 0, 0, width, height,
                     SWP_NOCOPYBITS | SWP_SHOWWINDOW);
     }
   else
     {
        SetWindowLong(w, GWL_STYLE, ew->style);
        SetWindowPos(w, HWND_NOTOPMOST,
                     ew->rect.left,
                     ew->rect.top,
                     ew->rect.right - ew->rect.left,
                     ew->rect.bottom - ew->rect.right,
                     SWP_NOCOPYBITS | SWP_SHOWWINDOW);
     }
   ew->fullscreen = on;
}
