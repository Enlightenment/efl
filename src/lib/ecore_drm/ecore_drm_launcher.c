#include "ecore_drm_private.h"
#include "ecore_drm_logind.h"

static Eina_Bool logind = EINA_FALSE;

EAPI Eina_Bool
ecore_drm_launcher_connect(Ecore_Drm_Device *dev)
{
   if (!(logind = _ecore_drm_logind_connect(dev)))
     {
        DBG("Launcher: Not Support loignd\n");
        if (geteuid() == 0)
          {
             DBG("Launcher: Try to keep going with root privilege\n");
             if (!ecore_drm_tty_open(dev, NULL))
               {
                  ERR("Launcher: failed to open tty with root privilege\n");
                  return EINA_FALSE;
               }
          }
        else
          {
             ERR("Launcher: Need Root Privilege or logind\n");
             return EINA_FALSE;
          }
     }
   DBG("Launcher: Success Connect\n");

   return EINA_TRUE;
}

EAPI void
ecore_drm_launcher_disconnect(Ecore_Drm_Device *dev)
{
   if (logind)
     {
        logind = EINA_FALSE;
        _ecore_drm_logind_disconnect(dev);
     }
   else
     {
        ecore_drm_tty_close(dev);
     }
}

static int
_device_flags_set(int fd, int flags)
{
   int fl;

   fl = fcntl(fd, F_GETFL);
   if (fl < 0)
     return -1;

   if (flags & O_NONBLOCK)
     fl |= O_NONBLOCK;

   if (fcntl(fd, F_SETFL, fl) < 0)
     return -1;

   fl = fcntl(fd, F_GETFD);
   if (fl < 0)
     return -1;

   if (!(flags & O_CLOEXEC))
     fl &= ~FD_CLOEXEC;

   if (fcntl(fd, F_SETFD, fl) < 0)
     return -1;

   return fd;
}

Eina_Bool
_ecore_drm_launcher_device_open(const char *device, Ecore_Drm_Open_Cb callback, void *data, int flags)
{
   int fd = -1;
   struct stat s;

   if (logind)
     {
        if (!_ecore_drm_logind_device_open(device, callback, data))
          return EINA_FALSE;
     }
   else
     {
        fd = open(device, flags | O_CLOEXEC);
        if (fd < 0) return EINA_FALSE;
        if (fstat(fd, &s) == -1)
          {
             close(fd);
             fd = -1;
             return EINA_FALSE;
          }

        callback(data, fd, EINA_FALSE);
     }

   return EINA_TRUE;
}

int
_ecore_drm_launcher_device_open_no_pending(const char *device, int flags)
{
   int fd = -1;
   struct stat s;

   if (logind)
     {
        fd = _ecore_drm_logind_device_open_no_pending(device);
        if ((fd = _device_flags_set(fd, flags)) == -1)
          {
             _ecore_drm_logind_device_close(device);
             return -1;
          }
     }
   else
     {
        fd = open(device, flags, flags | O_CLOEXEC);
        if (fd < 0) return fd;
        if (fstat(fd, &s) == -1)
          {
             close(fd);
             return -1;
          }
     }

   DBG("Device opened %s", device);
   return fd;
}

void
_ecore_drm_launcher_device_close(const char *device, int fd)
{
   if (logind)
     return _ecore_drm_logind_device_close(device);

   close(fd);
}
