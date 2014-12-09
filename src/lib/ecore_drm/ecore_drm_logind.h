#ifndef _ECORE_DRM_LOGIN_H_
# define _ECORE_DRM_LOGIN_H_

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

#ifdef HAVE_SYSTEMD_LOGIN
# include <systemd/sd-login.h>

Eina_Bool _ecore_drm_logind_connect(Ecore_Drm_Device *dev);
void _ecore_drm_logind_disconnect(Ecore_Drm_Device *dev);
Eina_Bool _ecore_drm_logind_device_open(const char *device, Ecore_Drm_Open_Cb callback, void *data);
int _ecore_drm_logind_device_open_no_pending(const char *device);
void _ecore_drm_logind_device_close(const char *device);

int _ecore_drm_dbus_init(const char *session);
int _ecore_drm_dbus_shutdown(void);
int _ecore_drm_dbus_device_take(uint32_t major, uint32_t minor, Ecore_Drm_Open_Cb callback, void *data);
int _ecore_drm_dbus_device_take_no_pending(uint32_t major, uint32_t minor, Eina_Bool *paused_out, double timeout);
void _ecore_drm_dbus_device_release(uint32_t major, uint32_t minor);

#else
static inline Eina_Bool
_ecore_drm_logind_connect(Ecore_Drm_Device *dev EINA_UNUSED)
{
      return EINA_FALSE;
}

static inline void
_ecore_drm_logind_disconnect(Ecore_Drm_Device *dev EINA_UNUSED)
{
      return;
}

static inline Eina_Bool
_ecore_drm_logind_device_open(const char *device EINA_UNUSED, Ecore_Drm_Open_Cb callback EINA_UNUSED, void *data EINA_UNUSED)
{
      return EINA_FALSE;
}

static inline int
_ecore_drm_logind_device_open_no_pending(const char *device EINA_UNUSED)
{
      return -1;
}

static inline void
_ecore_drm_logind_device_close(const char *device EINA_UNUSED)
{
      return;
}
#endif

#endif
