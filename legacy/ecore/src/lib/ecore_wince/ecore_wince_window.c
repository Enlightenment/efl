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

   w = (struct _Ecore_WinCE_Window *)calloc(1, sizeof(struct _Ecore_WinCE_Window));
   if (!w)
     return NULL;

   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   if (!AdjustWindowRectEx(&rect, WS_CAPTION | WS_SYSMENU | WS_VISIBLE, FALSE, WS_EX_TOPMOST))
     {
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
        free(w);
        return NULL;
     }

   if (!_ecore_wince_hardware_keys_register(window))
     {
        DestroyWindow(window);
        free(w);
        return NULL;
     }

   w->window = window;

   SetLastError(0);
   if (!SetWindowLong(window, GWL_USERDATA, (LONG)w) && (GetLastError() != 0))
     {
        DestroyWindow(window);
        free(w);
        return NULL;
     }

   w->pointer_is_in = 0;

   return w;
}

void
ecore_wince_window_del(Ecore_WinCE_Window *window)
{
   Ecore_WinCE_Window *w;

   if (!window) return;

   DestroyWindow(((struct _Ecore_WinCE_Window *)window)->window);
   free(window);
   fprintf (stderr, "ecore_wince_window_del\n");
}

void *
ecore_wince_window_hwnd_get(Ecore_WinCE_Window *window)
{
   struct _Ecore_WinCE_Window *w;

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

   printf ("ecore_wince_window_move %p : %d %d\n", window, x, y);
   w = ((struct _Ecore_WinCE_Window *)window)->window;
   if (!GetWindowRect(w, &rect))
     return;

   MoveWindow(w, x, y,
              rect.right - rect.left,
              rect.bottom - rect.top,
              TRUE);
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

   w = (struct _Ecore_WinCE_Window *)window;
   if (!GetWindowRect(w->window, &rect)) return;

   printf ("ecore_wince_window_resize 0 : %p (%d %d)\n",
           w,
           width,
           height);

   x = rect.left;
   y = rect.top;
   rect.left = 0;
   rect.top = 0;
   rect.right = width;
   rect.bottom = height;
   style = GetWindowLong(w->window, GWL_STYLE);
   exstyle = GetWindowLong(w->window, GWL_EXSTYLE);
   if (!AdjustWindowRectEx(&rect, style, FALSE, exstyle))
     return;

   if (!MoveWindow(w->window, x, y,
                   rect.right - rect.left,
                   rect.bottom - rect.top,
                   FALSE))
     {
       printf (" MEEERDE !!!\n");
     }
   printf ("ecore_wince_window_resize 4 : %d %d\n", width, height);
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

   printf ("ecore_wince_window_move_resize 0 : %p  %d %d\n", window, width, height);
   w = ((struct _Ecore_WinCE_Window *)window);
   rect.left = 0;
   rect.top = 0;
   printf ("ecore_wince_window_move_resize 1 : %d %d\n", width, height);
   rect.right = width;
   rect.bottom = height;
   style = GetWindowLong(w->window, GWL_STYLE);
   exstyle = GetWindowLong(w->window, GWL_EXSTYLE);
   if (!AdjustWindowRectEx(&rect, style, FALSE, exstyle))
     return;

   MoveWindow(w->window, x, y,
              rect.right - rect.left,
              rect.bottom - rect.top,
              TRUE);
}

void
ecore_wince_window_show(Ecore_WinCE_Window *window)
{
   if (!window) return;

   fprintf (stderr, " ** ecore_wince_window_show  %p\n", window);
   ShowWindow(((struct _Ecore_WinCE_Window *)window)->window, SW_SHOWNORMAL);
   UpdateWindow(((struct _Ecore_WinCE_Window *)window)->window);
   SendMessage(((struct _Ecore_WinCE_Window *)window)->window, WM_SHOWWINDOW, 1, 0);
}

void
ecore_wince_window_hide(Ecore_WinCE_Window *window)
{
   if (!window) return;

   fprintf (stderr, " ** ecore_wince_window_hide  %p\n", window);
   ShowWindow(((struct _Ecore_WinCE_Window *)window)->window, SW_HIDE);
   SendMessage(((struct _Ecore_WinCE_Window *)window)->window, WM_SHOWWINDOW, 0, 0);
}

void
ecore_wince_window_title_set(Ecore_WinCE_Window *window,
                             const char         *title)
{
   wchar_t *wtitle;

   if (!window) return;

   if (!title || !title[0]) return;

   wtitle = evil_char_to_wchar(title);
   if (!wtitle) return;

   SetWindowText(((struct _Ecore_WinCE_Window *)window)->window, wtitle);
   free(wtitle);
}

void
ecore_wince_window_backend_set(Ecore_WinCE_Window *window, int backend)
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return;

   w = (struct _Ecore_WinCE_Window *)window;
   w->backend = backend;
}

void
ecore_wince_window_suspend_set(Ecore_WinCE_Window *window, int (*suspend)(int))
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return;

   w = (struct _Ecore_WinCE_Window *)window;
   w->suspend = suspend;
}

void
ecore_wince_window_resume_set(Ecore_WinCE_Window *window, int (*resume)(int))
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return;

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

   printf ("ecore_wince_window_geometry_get %p\n", window);
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

   if (!window)
     {
        if (width) *width = GetSystemMetrics(SM_CXSCREEN);
        if (height) *height = GetSystemMetrics(SM_CYSCREEN);

        return;
     }

   if (!GetClientRect(((struct _Ecore_WinCE_Window *)window)->window,
                      &rect))
     {
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
   int                         width;
   int                         height;

   if (!window) return;

   ew = (struct _Ecore_WinCE_Window *)window;
   if (((ew->fullscreen) && (on)) ||
       ((!ew->fullscreen) && (!on)))
     return;

   ew->fullscreen = !!on;
   w = ew->window;

   if (on)
     {
        /* save the position and size of the window */
        if (!GetWindowRect(w, &ew->rect)) return;

        /* hide task bar */
        task_bar = FindWindow(L"HHTaskBar", NULL);
        if (!task_bar) return;
        ShowWindow(task_bar, SW_HIDE);
        EnableWindow(task_bar, FALSE);

        /* style: visible + popup */
        if (!SetWindowLong(w, GWL_STYLE, WS_POPUP | WS_VISIBLE)) return;

        /* resize window to fit the entire screen */
        SetWindowPos(w, HWND_TOPMOST,
                     0, 0,
                     GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        /*
         * It seems that SetWindowPos is not sufficient.
         * Call MoveWindow with the correct size and force painting.
         * Note that UpdateWindow (forcing repainting) is not sufficient
         */
        MoveWindow(w,
                   0, 0,
                   GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
                   TRUE);
     }
   else
     {
        /* show task bar */
        task_bar = FindWindow(L"HHTaskBar", NULL);
        if (!task_bar) return;
        ShowWindow(task_bar, SW_SHOW);
        EnableWindow(task_bar, TRUE);

        /* style: visible + caption + sysmenu */
        if (!SetWindowLong(w, GWL_STYLE, WS_CAPTION | WS_SYSMENU | WS_VISIBLE)) return;
        /* restaure the position and size of the window */
        SetWindowPos(w, HWND_TOPMOST,
                     ew->rect.left,
                     ew->rect.top,
                     ew->rect.right - ew->rect.left,
                     ew->rect.bottom - ew->rect.top,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        /*
         * It seems that SetWindowPos is not sufficient.
         * Call MoveWindow with the correct size and force painting.
         * Note that UpdateWindow (forcing repainting) is not sufficient
         */
        MoveWindow(w,
                   ew->rect.left,
                   ew->rect.top,
                   ew->rect.right - ew->rect.left,
                   ew->rect.bottom - ew->rect.top,
                   TRUE);
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
     return 0;

   unregister_fct = (UnregisterFunc1Proc)GetProcAddress(core_dll, L"UnregisterFunc1");
   if (!unregister_fct)
     {
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
