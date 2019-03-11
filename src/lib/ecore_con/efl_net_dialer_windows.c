#define EFL_NET_SOCKET_WINDOWS_PROTECTED 1
#define EFL_IO_READER_PROTECTED 1
#define EFL_IO_WRITER_PROTECTED 1
#define EFL_IO_CLOSER_PROTECTED 1
#define EFL_NET_DIALER_PROTECTED 1
#define EFL_NET_SOCKET_PROTECTED 1

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_con_private.h"

#define MY_CLASS EFL_NET_DIALER_WINDOWS_CLASS

typedef struct _Efl_Net_Dialer_Windows_Data
{
   Eina_Stringshare *address_dial;
   double timeout_dial;
   Eina_Bool connected;
} Efl_Net_Dialer_Windows_Data;

EOLIAN static void
_efl_net_dialer_windows_efl_object_destructor(Eo *o, Efl_Net_Dialer_Windows_Data *pd)
{
   efl_destructor(efl_super(o, MY_CLASS));

   eina_stringshare_replace(&pd->address_dial, NULL);
}

EOLIAN static Eina_Error
_efl_net_dialer_windows_efl_net_dialer_dial(Eo *o, Efl_Net_Dialer_Windows_Data *pd, const char *address)
{
   Eina_Error err;
   HANDLE h;
   char cstr[256], sstr[256];
   ULONG cpid, spid;

   EINA_SAFETY_ON_NULL_RETURN_VAL(address, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(strchr(address, '/') != NULL, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(strchr(address, '\\') != NULL, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(strlen(PIPE_NS) + strlen(address) >= 256, EINVAL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(efl_net_dialer_connected_get(o), EISCONN);

   efl_net_dialer_address_dial_set(o, address);

   h = CreateFile(pd->address_dial,
                  FILE_READ_ATTRIBUTES | FILE_READ_DATA |
                  FILE_WRITE_ATTRIBUTES | FILE_WRITE_DATA,
                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                  NULL, OPEN_EXISTING,
                  FILE_FLAG_OVERLAPPED, NULL);
   if (h == INVALID_HANDLE_VALUE)
     {
        DWORD win32err = GetLastError();
        switch (win32err)
          {
           case ERROR_ACCESS_DENIED: return EACCES;
           case ERROR_INVALID_PARAMETER: return EINVAL;
           case ERROR_SHARING_VIOLATION: return EINVAL;
           case ERROR_FILE_EXISTS: return EEXIST;
           case ERROR_FILE_NOT_FOUND: return ENOENT;
           case ERROR_PIPE_BUSY: return EBUSY;
           case ERROR_INVALID_NAME: return EADDRNOTAVAIL;
           default:
             {
                ERR("CreateFile(%s): Unexpected win32err=%lu (%s)",
                    pd->address_dial, win32err, evil_format_message(win32err));
                return EINVAL;
             }
          }
     }

   err = _efl_net_socket_windows_init(o, h);
   if (err)
     {
        CloseHandle(h);
        return err;
     }

  if (GetNamedPipeClientProcessId(_efl_net_socket_windows_handle_get(o), &cpid))
    snprintf(cstr, sizeof(cstr), "%s:%lu", address, cpid);
  else
    {
       WRN("server=%p (%s) could not GetNamedPipeClientProcessId(o): %s",
           o, address, evil_last_error_get());
       eina_strlcpy(cstr, address, sizeof(cstr));
    }

  if (GetNamedPipeServerProcessId(_efl_net_socket_windows_handle_get(o), &spid))
    snprintf(sstr, sizeof(sstr), "%s:%lu", address, spid);
  else
    {
       WRN("server=%p (%s) could not GetNamedPipeServerProcessId(o): %s",
           o, address, evil_last_error_get());
       eina_strlcpy(sstr, address, sizeof(sstr));
     }

   efl_net_socket_address_remote_set(o, sstr);
   efl_net_socket_address_local_set(o, cstr);
   efl_event_callback_call(o, EFL_NET_DIALER_EVENT_DIALER_RESOLVED, NULL);
   efl_net_dialer_connected_set(o, EINA_TRUE);

   return _efl_net_socket_windows_io_start(o);
}

EOLIAN static void
_efl_net_dialer_windows_efl_net_dialer_address_dial_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Windows_Data *pd, const char *address)
{
   const char *tmp = eina_stringshare_printf(PIPE_NS "%s", address);
   eina_stringshare_del(pd->address_dial);
   pd->address_dial = tmp;
}

EOLIAN static const char *
_efl_net_dialer_windows_efl_net_dialer_address_dial_get(const Eo *o EINA_UNUSED, Efl_Net_Dialer_Windows_Data *pd)
{
   return pd->address_dial + strlen(PIPE_NS);
}

EOLIAN static void
_efl_net_dialer_windows_efl_net_dialer_connected_set(Eo *o, Efl_Net_Dialer_Windows_Data *pd, Eina_Bool connected)
{
   if (pd->connected == connected) return;
   pd->connected = connected;
   if (connected) efl_event_callback_call(o, EFL_NET_DIALER_EVENT_DIALER_CONNECTED, NULL);
}

EOLIAN static Eina_Bool
_efl_net_dialer_windows_efl_net_dialer_connected_get(const Eo *o EINA_UNUSED, Efl_Net_Dialer_Windows_Data *pd)
{
   return pd->connected;
}

EOLIAN static void
_efl_net_dialer_windows_efl_net_dialer_timeout_dial_set(Eo *o EINA_UNUSED, Efl_Net_Dialer_Windows_Data *pd, double seconds)
{
   pd->timeout_dial = seconds;
}

EOLIAN static double
_efl_net_dialer_windows_efl_net_dialer_timeout_dial_get(const Eo *o EINA_UNUSED, Efl_Net_Dialer_Windows_Data *pd)
{
   return pd->timeout_dial;
}

EOLIAN static Eina_Error
_efl_net_dialer_windows_efl_io_closer_close(Eo *o, Efl_Net_Dialer_Windows_Data *pd EINA_UNUSED)
{
   efl_net_dialer_connected_set(o, EINA_FALSE);
   return efl_io_closer_close(efl_super(o, MY_CLASS));
}

#include "efl_net_dialer_windows.eo.c"
