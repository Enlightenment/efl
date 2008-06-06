/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <stdlib.h>
#include <stdio.h>   /* for printf */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <aygshell.h>

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

   SetRect(&rect, 0, 0,
           GetSystemMetrics(SM_CXSCREEN),
           GetSystemMetrics(SM_CYSCREEN));

   window = CreateWindowEx(WS_EX_TOPMOST,
                           ECORE_WINCE_WINDOW_CLASS,
                           L"",
                           WS_VISIBLE | WS_POPUP,
                           rect.left, rect.top,
                           rect.right - rect.left,
                           rect.bottom - rect.top,
                            parent ? ((struct _Ecore_WinCE_Window *)parent)->window : NULL,
                           NULL, _ecore_wince_instance, NULL);
   if (!window)
     {
        free(w);
        return NULL;
     }

   SHFullScreen(window,
                SHFS_HIDETASKBAR | SHFS_HIDESTARTICON | SHFS_HIDESIPBUTTON);

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

   printf ("ecore_wince_window_size_get %p\n", window);
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

void *
ecore_wince_window_window_get(Ecore_WinCE_Window *window)
{
   struct _Ecore_WinCE_Window *w;

   if (!window)
     return NULL;

   return ((struct _Ecore_WinCE_Window *)window)->window;
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
