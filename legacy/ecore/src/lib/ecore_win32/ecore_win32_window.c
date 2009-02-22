/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>   /* for printf */

#define _WIN32_WINNT 0x0500  // For WS_EX_LAYERED

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <Eina.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"


/***** Private declarations *****/


typedef enum _Ecore_Win32_Window_Z_Order Ecore_Win32_Window_Z_Order;
enum _Ecore_Win32_Window_Z_Order
{
  ECORE_WIN32_WINDOW_Z_ORDER_BOTTOM,
  ECORE_WIN32_WINDOW_Z_ORDER_NOTOPMOST,
  ECORE_WIN32_WINDOW_Z_ORDER_TOP,
  ECORE_WIN32_WINDOW_Z_ORDER_TOPMOST
};

static Ecore_Win32_Window *ecore_win32_window_internal_new(Ecore_Win32_Window *parent,
                                                           int                 x,
                                                           int                 y,
                                                           int                 width,
                                                           int                 height,
                                                           DWORD               style);


/***** API *****/

Ecore_Win32_Window *
ecore_win32_window_new(Ecore_Win32_Window *parent,
                       int                 x,
                       int                 y,
                       int                 width,
                       int                 height)
{
   EINA_ERROR_PINFO("creating window with border\n");

   return ecore_win32_window_internal_new(parent,
                                          x, y,
                                          width, height,
                                          WS_OVERLAPPEDWINDOW | WS_SIZEBOX);
}

/* simulate X11 override windows */
Ecore_Win32_Window *
ecore_win32_window_override_new(Ecore_Win32_Window *parent,
                                int                 x,
                                int                 y,
                                int                 width,
                                int                 height)
{
   EINA_ERROR_PINFO("creating window without border\n");

   return ecore_win32_window_internal_new(parent,
                                          x, y,
                                          width, height,
                                          WS_POPUP);
}

void
ecore_win32_window_del(Ecore_Win32_Window *window)
{
   struct _Ecore_Win32_Window *wnd = window;

   if (!window) return;

   EINA_ERROR_PINFO("destroying window\n");

   if (wnd->shape.mask != NULL)
      free(wnd->shape.mask);

   DestroyWindow(((struct _Ecore_Win32_Window *)window)->window);
   free(window);
}

void *
ecore_win32_window_hwnd_get(Ecore_Win32_Window *window)
{
   if (!window) return NULL;

   return ((struct _Ecore_Win32_Window *)window)->window;
}

/*
void
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

void
ecore_win32_window_move(Ecore_Win32_Window *window,
                        int                 x,
                        int                 y)
{
   RECT rect;
   HWND w;

   if (!window) return;

   EINA_ERROR_PINFO("moving window (%dx%d)\n", x, y);

   w = ((struct _Ecore_Win32_Window *)window)->window;
   if (!GetWindowRect(w, &rect))
     {
        EINA_ERROR_PERR("GetWindowRect() failed\n");
        return;
     }

   if (!MoveWindow(w, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   TRUE))
     {
        EINA_ERROR_PERR("MoveWindow() failed\n");
     }
}

void
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

   EINA_ERROR_PINFO("resizing window (%dx%d)\n", width, height);

   w = (struct _Ecore_Win32_Window *)window;
   if (!GetWindowRect(w->window, &rect))
     {
        EINA_ERROR_PERR("GetWindowRect() failed\n");
        return;
     }

   x = rect.left;
   y = rect.top;
   rect.left = 0;
   rect.top = 0;
/*    if (width < w->min_width) width = w->min_width; */
/*    if (width > w->max_width) width = w->max_width; */
/*    printf ("ecore_win32_window_resize 1 : %d %d %d\n", w->min_height, w->max_height, height); */
/*    if (height < w->min_height) height = w->min_height; */
/*    printf ("ecore_win32_window_resize 2 : %d %d\n", w->max_height, height); */
/*    if (height > w->max_height) height = w->max_height; */
/*    printf ("ecore_win32_window_resize 3 : %d %d\n", w->max_height, height); */
   rect.right = width;
   rect.bottom = height;
   if (!(style = GetWindowLong(w->window, GWL_STYLE)))
     {
        EINA_ERROR_PERR("GetWindowLong() failed\n");
        return;
     }
   if (!AdjustWindowRect(&rect, style, FALSE))
     {
        EINA_ERROR_PERR("AdjustWindowRect() failed\n");
        return;
     }

   if (!MoveWindow(w->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   FALSE))
     {
        EINA_ERROR_PERR("MoveWindow() failed\n");
     }
}

void
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

   EINA_ERROR_PINFO("moving and resizing window (%dx%d %dx%d)\n", x, y, width, height);

   w = ((struct _Ecore_Win32_Window *)window);
   rect.left = 0;
   rect.top = 0;
   if ((unsigned int)width < w->min_width) width = w->min_width;
   if ((unsigned int)width > w->max_width) width = w->max_width;
   if ((unsigned int)height < w->min_height) height = w->min_height;
   if ((unsigned int)height > w->max_height) height = w->max_height;
   rect.right = width;
   rect.bottom = height;
   if (!(style = GetWindowLong(w->window, GWL_STYLE)))
     {
        EINA_ERROR_PERR("GetWindowLong() failed\n");
        return;
     }
   if (!AdjustWindowRect(&rect, style, FALSE))
     {
        EINA_ERROR_PERR("AdjustWindowRect() failed\n");
        return;
     }

   if (!MoveWindow(w->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   TRUE))
     {
        EINA_ERROR_PERR("MoveWindow() failed\n");
     }
}

void
ecore_win32_window_geometry_get(Ecore_Win32_Window *window,
                                int                *x,
                                int                *y,
                                int                *width,
                                int                *height)
{
   RECT rect;
   int  w;
   int  h;

   EINA_ERROR_PINFO("getting window geometry\n");

   if (!window)
     {
        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = GetSystemMetrics(SM_CXSCREEN);
        if (height) *height = GetSystemMetrics(SM_CYSCREEN);

        return;
     }

   if (!GetClientRect(((struct _Ecore_Win32_Window *)window)->window,
                      &rect))
     {
        EINA_ERROR_PERR("GetClientRect() failed\n");

        if (x) *x = 0;
        if (y) *y = 0;
        if (width) *width = 0;
        if (height) *height = 0;

        return;
     }

   w = rect.right - rect.left;
   h = rect.bottom - rect.top;

   if (!GetWindowRect(((struct _Ecore_Win32_Window *)window)->window,
                      &rect))
     {
        EINA_ERROR_PERR("GetWindowRect() failed\n");

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
ecore_win32_window_size_get(Ecore_Win32_Window *window,
                            int                *width,
                            int                *height)
{
   RECT rect;

   EINA_ERROR_PINFO("getting window size\n");

   if (!window)
     {
        if (width) *width = GetSystemMetrics(SM_CXSCREEN);
        if (height) *height = GetSystemMetrics(SM_CYSCREEN);

        return;
     }

   if (!GetClientRect(((struct _Ecore_Win32_Window *)window)->window,
                      &rect))
     {
        EINA_ERROR_PERR("GetClientRect() failed\n");

        if (width) *width = 0;
        if (height) *height = 0;
     }

   if (width) *width = rect.right - rect.left;
   if (height) *height = rect.bottom - rect.top;
}

void
ecore_win32_window_size_min_set(Ecore_Win32_Window *window,
                                unsigned int        min_width,
                                unsigned int        min_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   printf ("ecore_win32_window_size_min_set : %p  %d %d\n", window, min_width, min_height);
   w = (struct _Ecore_Win32_Window *)window;
   w->min_width = min_width;
   w->min_height = min_height;
}

void
ecore_win32_window_size_min_get(Ecore_Win32_Window *window,
                                unsigned int       *min_width,
                                unsigned int       *min_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   printf ("ecore_win32_window_size_min_get : %p  %d %d\n", window, w->min_width, w->min_height);
   if (min_width) *min_width = w->min_width;
   if (min_height) *min_height = w->min_height;
}

void
ecore_win32_window_size_max_set(Ecore_Win32_Window *window,
                                unsigned int        max_width,
                                unsigned int        max_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   printf ("ecore_win32_window_size_max_set : %p  %d %d\n", window, max_width, max_height);
   w = (struct _Ecore_Win32_Window *)window;
   w->max_width = max_width;
   w->max_height = max_height;
}

void
ecore_win32_window_size_max_get(Ecore_Win32_Window *window,
                                unsigned int       *max_width,
                                unsigned int       *max_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   printf ("ecore_win32_window_size_max_get : %p  %d %d\n", window, w->max_width, w->max_height);
   if (max_width) *max_width = w->max_width;
   if (max_height) *max_height = w->max_height;
}

void
ecore_win32_window_size_base_set(Ecore_Win32_Window *window,
                                 unsigned int        base_width,
                                 unsigned int        base_height)
{
   struct _Ecore_Win32_Window *w;

   printf ("ecore_win32_window_size_base_set : %p  %d %d\n", window, base_width, base_height);
   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   w->base_width = base_width;
   w->base_height = base_height;
}

void
ecore_win32_window_size_base_get(Ecore_Win32_Window *window,
                                 unsigned int       *base_width,
                                 unsigned int       *base_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   printf ("ecore_win32_window_size_base_get : %p  %d %d\n", window, w->base_width, w->base_height);
   if (base_width) *base_width = w->base_width;
   if (base_height) *base_height = w->base_height;
}

void
ecore_win32_window_size_step_set(Ecore_Win32_Window *window,
                                 unsigned int        step_width,
                                 unsigned int        step_height)
{
   struct _Ecore_Win32_Window *w;

   printf ("ecore_win32_window_size_step_set : %p  %d %d\n", window, step_width, step_height);
   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   w->step_width = step_width;
   w->step_height = step_height;
}

void
ecore_win32_window_size_step_get(Ecore_Win32_Window *window,
                                 unsigned int       *step_width,
                                 unsigned int       *step_height)
{
   struct _Ecore_Win32_Window *w;

   if (!window) return;

   w = (struct _Ecore_Win32_Window *)window;
   printf ("ecore_win32_window_size_step_get : %p  %d %d\n", window, w->step_width, w->step_height);
   if (step_width) *step_width = w->step_width;
   if (step_height) *step_height = w->step_height;
}

void
ecore_win32_window_shape_set(Ecore_Win32_Window *window,
                             unsigned short      width,
                             unsigned short      height,
                             unsigned char      *mask)
{
   struct _Ecore_Win32_Window *wnd;
   HRGN                        rgn;
   int                         x;
   int                         y;
   OSVERSIONINFO               version_info;

   if (window == NULL)
      return;

   wnd = (struct _Ecore_Win32_Window *)window;

   if (mask == NULL)
     {
        wnd->shape.enabled = 0;
        if (wnd->shape.layered != 0)
          {
             wnd->shape.layered = 0;
#if defined(WS_EX_LAYERED)
             if (!SetWindowLong(wnd->window, GWL_EXSTYLE,
                                GetWindowLong(wnd->window, GWL_EXSTYLE) & (~WS_EX_LAYERED)))
               {
                  EINA_ERROR_PERR("SetWindowLong() failed\n");
                  return;
               }
             if (!RedrawWindow(wnd->window, NULL, NULL,
                               RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN))
               {
                  EINA_ERROR_PERR("RedrawWindow() failed\n");
                  return;
               }
#endif
          }
        else
          if (!SetWindowRgn(wnd->window, NULL, TRUE))
            {
               EINA_ERROR_PERR("SetWindowRgn() failed\n");
            }
        return;
     }

   if (width == 0 || height == 0)
     return;

   wnd->shape.enabled = 1;

   if (width != wnd->shape.width || height != wnd->shape.height)
     {
       wnd->shape.width = width;
       wnd->shape.height = height;
       if (wnd->shape.mask != NULL)
         {
           free(wnd->shape.mask);
           wnd->shape.mask = NULL;
         }
       wnd->shape.mask = malloc(width * height);
     }
   memcpy(wnd->shape.mask, mask, width * height);

   wnd->shape.layered = 0;

#if defined(WS_EX_LAYERED)
   version_info.dwOSVersionInfoSize = sizeof(version_info);
   if (GetVersionEx(&version_info) == TRUE && version_info.dwMajorVersion == 5)
     {
       if (!SetWindowLong(wnd->window, GWL_EXSTYLE,
                          GetWindowLong(wnd->window, GWL_EXSTYLE) | WS_EX_LAYERED))
            {
               EINA_ERROR_PERR("SetWindowLong() failed\n");
               return;
            }
       wnd->shape.layered = 1;
       return;
     }
#endif

   if (!(rgn = CreateRectRgn(0, 0, 0, 0)))
     {
        EINA_ERROR_PERR("CreateRectRgn() failed\n");
        return;
     }
   for (y = 0; y < height; y++)
     {
        HRGN rgnLine;

        if (!(rgnLine = CreateRectRgn(0, 0, 0, 0)))
          {
             EINA_ERROR_PERR("CreateRectRgn() failed\n");
             return;
          }
        for (x = 0; x < width; x++)
          {
             if (mask[y * width + x] > 0)
               {
                  HRGN rgnDot;

                  if (!(rgnDot = CreateRectRgn(x, y, x + 1, y + 1)))
                    {
                       EINA_ERROR_PERR("CreateRectRgn() failed\n");
                       return;
                    }
                  if (CombineRgn(rgnLine, rgnLine, rgnDot, RGN_OR) == ERROR)
                    {
                       EINA_ERROR_PERR("CombineRgn() has not created a new region\n");
                    }
                  if (!DeleteObject(rgnDot))
                    {
                       EINA_ERROR_PERR("DeleteObject() failed\n");
                       return;
                    }
               }
          }
        if (CombineRgn(rgn, rgn, rgnLine, RGN_OR) == ERROR)
          {
             EINA_ERROR_PERR("CombineRgn() has not created a new region\n");
          }
        if (!DeleteObject(rgnLine))
          {
             EINA_ERROR_PERR("DeleteObject() failed\n");
             return;
          }
     }
   if (!SetWindowRgn(wnd->window, rgn, TRUE))
     {
        EINA_ERROR_PERR("SetWindowRgn() failed\n");
     }
}

void
ecore_win32_window_show(Ecore_Win32_Window *window)
{
   if (!window) return;

   EINA_ERROR_PINFO("showing window\n");

   if (!ShowWindow(((struct _Ecore_Win32_Window *)window)->window, SW_SHOWNORMAL))
     {
        EINA_ERROR_PERR("ShowWindow() failed\n");
        return;
     }
   if (!UpdateWindow(((struct _Ecore_Win32_Window *)window)->window))
     {
        EINA_ERROR_PERR("UpdateWindow() failed\n");
     }
   if (!SendMessage(((struct _Ecore_Win32_Window *)window)->window, WM_SHOWWINDOW, 1, 0))
     {
        EINA_ERROR_PERR("SendMessage() failed\n");
     }
}

/* FIXME: seems to block the taskbar */
void
ecore_win32_window_hide(Ecore_Win32_Window *window)
{
   if (!window) return;

   EINA_ERROR_PINFO("hiding window\n");

   if (!ShowWindow(((struct _Ecore_Win32_Window *)window)->window, SW_HIDE))
     {
        EINA_ERROR_PERR("ShowWindow() failed\n");
     }
   if (!SendMessage(((struct _Ecore_Win32_Window *)window)->window, WM_SHOWWINDOW, 0, 0))
     {
        EINA_ERROR_PERR("SendMessage() failed\n");
     }
}

void
ecore_win32_window_raise(Ecore_Win32_Window *window)
{
   if (!window) return;

   EINA_ERROR_PINFO("raising window\n");

   if (!SetWindowPos(((struct _Ecore_Win32_Window *)window)->window,
                     HWND_TOP, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE))
     {
        EINA_ERROR_PERR("SetWindowPos() failed\n");
     }
}

void
ecore_win32_window_lower(Ecore_Win32_Window *window)
{
   if (!window) return;

   EINA_ERROR_PINFO("lowering window\n");

   if (!SetWindowPos(((struct _Ecore_Win32_Window *)window)->window,
                     HWND_BOTTOM, 0, 0, 0, 0,
                     SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE))
     {
        EINA_ERROR_PERR("SetWindowPos() failed\n");
     }
}

void
ecore_win32_window_title_set(Ecore_Win32_Window *window,
                             const char         *title)
{
   if (!window) return;

   if (!title || !title[0]) return;

   EINA_ERROR_PINFO("setting window title\n");

   if (!SetWindowText(((struct _Ecore_Win32_Window *)window)->window, title))
     {
        EINA_ERROR_PERR("SetWindowText() failed\n");
     }
}

void
ecore_win32_window_focus_set(Ecore_Win32_Window *window)
{
   if (!window) return;

   EINA_ERROR_PINFO("focusing window\n");

   if (!SetFocus(((struct _Ecore_Win32_Window *)window)->window))
     {
        EINA_ERROR_PERR("SetFocus() failed\n");
     }
}

void
ecore_win32_window_iconified_set(Ecore_Win32_Window *window,
                                 int                 on)
{
   struct _Ecore_Win32_Window *ew;

   if (!window) return;

   ew = (struct _Ecore_Win32_Window *)window;
   if (((ew->iconified) && (on)) ||
       ((!ew->iconified) && (!on)))
     return;

   EINA_ERROR_PINFO("iconifying window: %s\n", on ? "yes" : "no");

   if (!ShowWindow(ew->window, on ? SW_MINIMIZE : SW_RESTORE))
     {
        EINA_ERROR_PERR("ShowWindow() failed\n");
     }
   ew->iconified = on;
}

void
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

   EINA_ERROR_PINFO("setting window border: %s\n", on ? "yes" : "no");

   w = ew->window;

   style = GetWindowLong(w, GWL_STYLE);
   if (on)
     {
        SetWindowLong(w, GWL_STYLE, style & ~WS_CAPTION);
     }
   else
     {
        if (!GetWindowRect(w, &rect))
          {
             EINA_ERROR_PERR("GetWindowRect() failed\n");
             return;
          }
        style |= WS_CAPTION;
        if (!AdjustWindowRect (&rect, style, FALSE))
          {
             EINA_ERROR_PERR("AdjustWindowRect() failed\n");
             return;
          }
        if (!SetWindowLong(w, GWL_STYLE, style))
          {
             EINA_ERROR_PERR("SetWindowLong() failed\n");
             return;
          }
     }
   if (!SetWindowPos(w, HWND_TOPMOST,
                     rect.left, rect.top,
                     rect.right - rect.left, rect.bottom - rect.top,
                     SWP_NOMOVE | SWP_FRAMECHANGED))
     {
        EINA_ERROR_PERR("SetWindowPos() failed\n");
        return;
     }
   ew->borderless = on;
}

void
ecore_win32_window_fullscreen_set(Ecore_Win32_Window *window,
                                  int                 on)
{
   struct _Ecore_Win32_Window *ew;
   HWND                        w;

   if (!window) return;

   ew = (struct _Ecore_Win32_Window *)window;
   if (((ew->fullscreen) && (on)) ||
       ((!ew->fullscreen) && (!on)))
     return;

   EINA_ERROR_PINFO("setting fullscreen: %s\n", on ? "yes" : "no");

   ew->fullscreen = !!on;
   w = ew->window;

   if (on)
     {
        DWORD style;

        if (!GetWindowRect(w, &ew->rect))
          {
             EINA_ERROR_PERR("GetWindowRect() failed\n");
             return;
          }
        if (!(ew->style = GetWindowLong(w, GWL_STYLE)))
          {
             EINA_ERROR_PERR("GetWindowLong() failed\n");
             return;
          }
        style = ew->style & ~WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
        style |= WS_VISIBLE | WS_POPUP;
        if (!SetWindowLong(w, GWL_STYLE, style))
          {
             EINA_ERROR_PERR("SetWindowLong() failed\n");
             return;
          }
        if (!SetWindowLong(w, GWL_EXSTYLE, WS_EX_TOPMOST))
          {
             EINA_ERROR_PERR("SetWindowLong() failed\n");
             return;
          }
        if (!SetWindowPos(w, HWND_TOPMOST, 0, 0,
                          GetSystemMetrics (SM_CXSCREEN), GetSystemMetrics (SM_CYSCREEN),
                          SWP_NOCOPYBITS | SWP_SHOWWINDOW))
          {
             EINA_ERROR_PERR("SetWindowPos() failed\n");
             return;
          }
     }
   else
     {
        if (!SetWindowLong(w, GWL_STYLE, ew->style))
          {
             EINA_ERROR_PERR("SetWindowLong() failed\n");
             return;
          }
        if (!SetWindowLong(w, GWL_EXSTYLE, 0))
          {
             EINA_ERROR_PERR("SetWindowLong() failed\n");
             return;
          }
        if (!SetWindowPos(w, HWND_NOTOPMOST,
                          ew->rect.left,
                          ew->rect.top,
                          ew->rect.right - ew->rect.left,
                          ew->rect.bottom - ew->rect.top,
                          SWP_NOCOPYBITS | SWP_SHOWWINDOW))
          {
             EINA_ERROR_PERR("SetWindowPos() failed\n");
             return;
          }
     }
}

void
ecore_win32_window_cursor_set(Ecore_Win32_Window *window,
                              Ecore_Win32_Cursor *cursor)
{
   EINA_ERROR_PINFO("setting cursor\n");

   if (!SetClassLong(((struct _Ecore_Win32_Window *)window)->window,
                     GCL_HCURSOR, (LONG)cursor))
     {
        EINA_ERROR_PERR("SetClassLong() failed\n");
     }
}

void
ecore_win32_window_state_set(Ecore_Win32_Window       *window,
                             Ecore_Win32_Window_State *state,
                             unsigned int              num)
{
   unsigned int i;

   if (!window || !state || !num)
     return;

   EINA_ERROR_PINFO("setting cursor state\n");

   for (i = 0; i < num; i++)
     {
        switch (state[i])
          {
          case ECORE_WIN32_WINDOW_STATE_ICONIFIED:
            ((struct _Ecore_Win32_Window *)window)->state.iconified = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_MODAL:
            ((struct _Ecore_Win32_Window *)window)->state.modal = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_STICKY:
            ((struct _Ecore_Win32_Window *)window)->state.sticky = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_MAXIMIZED_VERT:
            ((struct _Ecore_Win32_Window *)window)->state.maximized_vert = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_MAXIMIZED_HORZ:
            ((struct _Ecore_Win32_Window *)window)->state.maximized_horz = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_MAXIMIZED:
            ((struct _Ecore_Win32_Window *)window)->state.maximized_horz = 1;
            ((struct _Ecore_Win32_Window *)window)->state.maximized_vert = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_SHADED:
            ((struct _Ecore_Win32_Window *)window)->state.shaded = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_HIDDEN:
            ((struct _Ecore_Win32_Window *)window)->state.hidden = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_FULLSCREEN:
            ((struct _Ecore_Win32_Window *)window)->state.fullscreen = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_ABOVE:
            ((struct _Ecore_Win32_Window *)window)->state.above = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_BELOW:
            ((struct _Ecore_Win32_Window *)window)->state.below = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_DEMANDS_ATTENTION:
            ((struct _Ecore_Win32_Window *)window)->state.demands_attention = 1;
            break;
          case ECORE_WIN32_WINDOW_STATE_UNKNOWN:
            /* nothing to be done */
            break;
          }
     }
}

void
ecore_win32_window_state_request_send(Ecore_Win32_Window      *window,
                                      Ecore_Win32_Window_State state,
                                      unsigned int             set)
{
   if (!window)
     return;

   EINA_ERROR_PINFO("sending cursor state\n");

   switch (state)
     {
      case ECORE_WIN32_WINDOW_STATE_ICONIFIED:
         if (((struct _Ecore_Win32_Window *)window)->state.iconified)
           ecore_win32_window_iconified_set(window, set);
         break;
      case ECORE_WIN32_WINDOW_STATE_MODAL:
         ((struct _Ecore_Win32_Window *)window)->state.modal = 1;
         break;
      case ECORE_WIN32_WINDOW_STATE_STICKY:
         ((struct _Ecore_Win32_Window *)window)->state.sticky = 1;
         break;
      case ECORE_WIN32_WINDOW_STATE_MAXIMIZED_VERT:
         if (((struct _Ecore_Win32_Window *)window)->state.maximized_vert)
           {
              RECT rect;
              int  y;
              int  height;

              if (!SystemParametersInfo(SPI_GETWORKAREA, 0,
                                        &rect, 0))
                {
                   EINA_ERROR_PERR("SystemParametersInfo() failed\n");
                   break;
                }
              y = rect.top;
              height = rect.bottom - rect.top;

              if (!GetClientRect(((struct _Ecore_Win32_Window *)window)->window,
                                 &rect))
                {
                   EINA_ERROR_PERR("GetClientRect() failed\n");
                   break;
                }

              if (!MoveWindow(window, rect.left, y,
                              rect.right - rect.left,
                              height,
                              TRUE))
                {
                   EINA_ERROR_PERR("MoveWindow() failed\n");
                }
           }
         break;
      case ECORE_WIN32_WINDOW_STATE_MAXIMIZED_HORZ:
         if (((struct _Ecore_Win32_Window *)window)->state.maximized_horz)
           {
              RECT rect;

              if (!GetClientRect(((struct _Ecore_Win32_Window *)window)->window,
                                 &rect))
                {
                   EINA_ERROR_PERR("GetClientRect() failed\n");
                   break;
                }

              if (!MoveWindow(window, 0, rect.top,
                              GetSystemMetrics(SM_CXSCREEN),
                              rect.bottom - rect.top,
                              TRUE))
                {
                   EINA_ERROR_PERR("MoveWindow() failed\n");
                }
           }
         break;
      case ECORE_WIN32_WINDOW_STATE_MAXIMIZED:
         if (((struct _Ecore_Win32_Window *)window)->state.maximized_vert &&
             ((struct _Ecore_Win32_Window *)window)->state.maximized_horz)
           {
              RECT rect;

              if (!SystemParametersInfo(SPI_GETWORKAREA, 0,
                                        &rect, 0))
                {
                   EINA_ERROR_PERR("SystemParametersInfo() failed\n");
                   break;
                }

              if (!MoveWindow(window, 0, 0,
                              GetSystemMetrics(SM_CXSCREEN),
                              rect.bottom - rect.top,
                              TRUE))
                {
                   EINA_ERROR_PERR("MoveWindow() failed\n");
                }
           }
         break;
      case ECORE_WIN32_WINDOW_STATE_SHADED:
         ((struct _Ecore_Win32_Window *)window)->state.shaded = 1;
         break;
      case ECORE_WIN32_WINDOW_STATE_HIDDEN:
         ((struct _Ecore_Win32_Window *)window)->state.hidden = 1;
         break;
      case ECORE_WIN32_WINDOW_STATE_FULLSCREEN:
         if (((struct _Ecore_Win32_Window *)window)->state.fullscreen)
           ecore_win32_window_fullscreen_set(window, set);
         break;
      case ECORE_WIN32_WINDOW_STATE_ABOVE:
         if (((struct _Ecore_Win32_Window *)window)->state.above)
           if (!SetWindowPos(((struct _Ecore_Win32_Window *)window)->window,
                             HWND_TOP,
                             0, 0,
                             0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW))
             {
                EINA_ERROR_PERR("SetWindowPos() failed\n");
             }
         break;
      case ECORE_WIN32_WINDOW_STATE_BELOW:
         if (((struct _Ecore_Win32_Window *)window)->state.below)
           if (!SetWindowPos(((struct _Ecore_Win32_Window *)window)->window,
                             HWND_BOTTOM,
                             0, 0,
                             0, 0,
                             SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW))
             {
                EINA_ERROR_PERR("SetWindowPos() failed\n");
             }
         break;
      case ECORE_WIN32_WINDOW_STATE_DEMANDS_ATTENTION:
         ((struct _Ecore_Win32_Window *)window)->state.demands_attention = 1;
         break;
      case ECORE_WIN32_WINDOW_STATE_UNKNOWN:
         /* nothing to be done */
         break;
     }
}

void
ecore_win32_window_type_set(Ecore_Win32_Window      *window,
                            Ecore_Win32_Window_Type  type)
{
   if (!window)
     return;

   EINA_ERROR_PINFO("setting window type\n");

   switch (type)
     {
     case ECORE_WIN32_WINDOW_TYPE_DESKTOP:
       ((struct _Ecore_Win32_Window *)window)->type.desktop = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_DOCK:
       ((struct _Ecore_Win32_Window *)window)->type.dock = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_TOOLBAR:
       ((struct _Ecore_Win32_Window *)window)->type.toolbar = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_MENU:
       ((struct _Ecore_Win32_Window *)window)->type.menu = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_UTILITY:
       ((struct _Ecore_Win32_Window *)window)->type.utility = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_SPLASH:
       ((struct _Ecore_Win32_Window *)window)->type.splash = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_DIALOG:
       ((struct _Ecore_Win32_Window *)window)->type.dialog = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_NORMAL:
       ((struct _Ecore_Win32_Window *)window)->type.normal = 1;
       break;
     case ECORE_WIN32_WINDOW_TYPE_UNKNOWN:
       ((struct _Ecore_Win32_Window *)window)->type.normal = 1;
       break;
     }
}


/***** Private functions definitions *****/

static Ecore_Win32_Window *
ecore_win32_window_internal_new(Ecore_Win32_Window *parent,
                                int                 x,
                                int                 y,
                                int                 width,
                                int                 height,
                                DWORD               style)
{
   RECT                        rect;
   struct _Ecore_Win32_Window *w;
   int                         minimal_width;
   int                         minimal_height;

   w = (struct _Ecore_Win32_Window *)calloc(1, sizeof(struct _Ecore_Win32_Window));
   if (!w)
     {
        EINA_ERROR_PERR("malloc() failed\n");
        return NULL;
     }

   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!AdjustWindowRect(&rect, style, FALSE))
     {
        EINA_ERROR_PERR("AdjustWindowRect() failed\n");
        free(w);
        return NULL;
     }

   minimal_width = GetSystemMetrics(SM_CXMIN);
   minimal_height = GetSystemMetrics(SM_CYMIN);
/*    if (((rect.right - rect.left) < minimal_width) || */
/*        ((rect.bottom - rect.top) < minimal_height)) */
/*      { */
/*         fprintf (stderr, "[Ecore] [Win32] ERROR !!\n"); */
/*         fprintf (stderr, "                Wrong size %ld\n", rect.right - rect.left); */
/*         free(w); */
/*         return NULL; */
/*      } */
   if ((rect.right - rect.left) < minimal_width)
     {
       rect.right = rect.left + minimal_width;
     }

   w->window = CreateWindowEx(0,
                              ECORE_WIN32_WINDOW_CLASS, "",
                              style,
                              x, y,
                              rect.right - rect.left,
                              rect.bottom - rect.top,
                              parent ? ((struct _Ecore_Win32_Window *)parent)->window : NULL,
                              NULL, _ecore_win32_instance, NULL);
   if (!w->window)
     {
        EINA_ERROR_PERR("CreateWindowEx() failed\n");
        free(w);
        return NULL;
     }

   SetLastError(0);
   if (!SetWindowLong(w->window, GWL_USERDATA, (LONG)w) && (GetLastError() != 0))
     {
        EINA_ERROR_PERR("SetWindowLong() failed\n");
        DestroyWindow(w->window);
        free(w);
        return NULL;
     }

   w->min_width   = 0;
   w->min_height  = 0;
   w->max_width   = 32767;
   w->max_height  = 32767;
   w->base_width  = -1;
   w->base_height = -1;
   w->step_width  = -1;
   w->step_height = -1;

   w->state.iconified         = 0;
   w->state.modal             = 0;
   w->state.sticky            = 0;
   w->state.maximized_vert    = 0;
   w->state.maximized_horz    = 0;
   w->state.shaded            = 0;
   w->state.hidden            = 0;
   w->state.fullscreen        = 0;
   w->state.above             = 0;
   w->state.below             = 0;
   w->state.demands_attention = 0;

   w->type.desktop = 0;
   w->type.dock    = 0;
   w->type.toolbar = 0;
   w->type.menu    = 0;
   w->type.utility = 0;
   w->type.splash  = 0;
   w->type.dialog  = 0;
   w->type.normal  = 0;

   w->pointer_is_in = 0;
   w->borderless    = 0;
   w->iconified     = 0;
   w->fullscreen    = 0;

   return w;
}
