#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <dbt.h>

#include <Eina.h>

#include "Ecore_Win32.h"
#include "ecore_win32_private.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

/**
 * @cond LOCAL
 */

typedef struct
{
   Ecore_Win32_Monitor monitor;
   char *name;
   Eina_Bool delete_me : 1;
} Ecore_Win32_Monitor_Priv;

typedef HRESULT (WINAPI *GetDpiForMonitor_t)(HMONITOR, int, UINT *, UINT *);

static HMODULE _ecore_win32_mod = NULL;
static GetDpiForMonitor_t GetDpiForMonitor_ = NULL;
static Eina_List *ecore_win32_monitors = NULL;

#ifndef GUID_DEVINTERFACE_MONITOR
static GUID GUID_DEVINTERFACE_MONITOR = {0xe6f07b5f, 0xee97, 0x4a90, { 0xb0, 0x76, 0x33, 0xf5, 0x7b, 0xf4, 0xea, 0xa7} };
#endif

static void
_ecore_win32_monitor_free(void *p)
{
   Ecore_Win32_Monitor_Priv *ewm = p;

   if (ewm)
     {
        free(ewm->name);
        free(ewm);
     }
}

static BOOL CALLBACK
_ecore_win32_monitor_update_cb(HMONITOR m, HDC monitor EINA_UNUSED, LPRECT r EINA_UNUSED, LPARAM data)
{
   MONITORINFOEX mi;
   Ecore_Win32_Monitor_Priv *ewm;
   Eina_Bool is_added;

   mi.cbSize = sizeof(MONITORINFOEX);
   GetMonitorInfo(m, (MONITORINFO *)&mi);

   if (data == 1)
     {
        Eina_List *l;

        is_added = EINA_FALSE;
        EINA_LIST_FOREACH(ecore_win32_monitors, l, ewm)
          {
             if (strcmp(mi.szDevice, ewm->name) != 0)
               {
                  is_added = EINA_TRUE;
                  break;
               }
          }
     }
   else if (data == 2)
     {
        Eina_List *l;

        EINA_LIST_FOREACH(ecore_win32_monitors, l, ewm)
          {
             if (strcmp(mi.szDevice, ewm->name) == 0)
               {
                  ewm->delete_me = EINA_FALSE;
                  return FALSE;
               }
          }
     }
   else
     is_added = EINA_TRUE;

   if (!is_added)
     return TRUE;

   ewm = (Ecore_Win32_Monitor_Priv *)malloc(sizeof(Ecore_Win32_Monitor_Priv));
   if (ewm)
     {
        ewm->monitor.desktop.x = mi.rcMonitor.left;
        ewm->monitor.desktop.y = mi.rcMonitor.top;
        ewm->monitor.desktop.w = mi.rcMonitor.right - mi.rcMonitor.left;
        ewm->monitor.desktop.h = mi.rcMonitor.bottom - mi.rcMonitor.top;
        if (!GetDpiForMonitor_ ||
            (GetDpiForMonitor_(m, 0,
                               &ewm->monitor.dpi.x,
                               &ewm->monitor.dpi.y) != S_OK))
          {
             HDC dc;

             dc = GetDC(NULL);
             ewm->monitor.dpi.x = GetDeviceCaps(dc, LOGPIXELSX);
             ewm->monitor.dpi.y = GetDeviceCaps(dc, LOGPIXELSY);
             ReleaseDC(NULL, dc);
          }
        ewm->name = strdup(mi.szDevice);
        if (ewm->name)
          ecore_win32_monitors = eina_list_append(ecore_win32_monitors, ewm);
        else
          free(ewm);
     }

   return TRUE;
}

/**
 * @endcond
 */


/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/

HWND ecore_win32_monitor_window = NULL;

void
ecore_win32_monitor_init(void)
{
   DEV_BROADCAST_DEVICEINTERFACE notification;
   DWORD style;

   style = WS_POPUP & ~(WS_CAPTION | WS_THICKFRAME);
   ecore_win32_monitor_window = CreateWindow(ECORE_WIN32_WINDOW_CLASS, "",
                                             style,
                                             10, 10,
                                             100, 100,
                                             NULL, NULL,
                                             _ecore_win32_instance, NULL);

   if (ecore_win32_monitor_window)
     {
        ZeroMemory(&notification, sizeof(notification));
        notification.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
        notification.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        notification.dbcc_classguid = GUID_DEVINTERFACE_MONITOR;
        RegisterDeviceNotification(ecore_win32_monitor_window,
                                   &notification,
                                   DEVICE_NOTIFY_WINDOW_HANDLE);
     }

   /*
    * Even if RegisterDeviceNotification() fails, the next call will
    * fill one item of the monitor lists, except if there is no more
    * memory
    */
   ecore_win32_monitor_update(0);

   _ecore_win32_mod = LoadLibrary("shcore.dll");
   if (_ecore_win32_mod)
     GetDpiForMonitor_ = (GetDpiForMonitor_t)GetProcAddress(_ecore_win32_mod,
                                                            "GetDpiForMonitor");
}

void
ecore_win32_monitor_shutdown(void)
{
   Ecore_Win32_Monitor_Priv *ewm;

   if (_ecore_win32_mod)
     FreeLibrary(_ecore_win32_mod);
   EINA_LIST_FREE(ecore_win32_monitors, ewm)
     _ecore_win32_monitor_free(ewm);
   if (ecore_win32_monitor_window)
     DestroyWindow(ecore_win32_monitor_window);
}

void
ecore_win32_monitor_update(int d)
{
   Ecore_Win32_Monitor_Priv *ewm;
   Eina_List *l;

   if (d == 2)
     {
        EINA_LIST_FOREACH(ecore_win32_monitors, l, ewm)
          ewm->delete_me = EINA_TRUE;
     }

   EnumDisplayMonitors(NULL, NULL, _ecore_win32_monitor_update_cb, d);

   if (d == 2)
     {
        EINA_LIST_FOREACH(ecore_win32_monitors, l, ewm)
          {
             if (ewm->delete_me == EINA_TRUE)
               {
                  ecore_win32_monitors = eina_list_remove(ecore_win32_monitors, ewm);
                  _ecore_win32_monitor_free(ewm);
                  break;
               }
          }
     }
}

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI Eina_Iterator *
ecore_win32_monitors_get(void)
{
   return eina_list_iterator_new(ecore_win32_monitors);
}
