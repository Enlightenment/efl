#ifndef _ECORE_DRM2_PRIVATE_H
# define _ECORE_DRM2_PRIVATE_H

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include "Ecore.h"
# include "ecore_private.h"
# include "Eeze.h"
# include "Elput.h"
# include <Ecore_Drm2.h>

# include <unistd.h>
# include <strings.h>
# include <sys/mman.h>
# include <fcntl.h>
# include <ctype.h>

# include <sys/ioctl.h>
# include <xf86drm.h>
# include <xf86drmMode.h>
# include <drm_mode.h>
# include <drm_fourcc.h>

extern int _ecore_drm2_log_dom;

# ifdef ECORE_DRM2_DEFAULT_LOG_COLOR
#  undef ECORE_DRM2_DEFAULT_LOG_COLOR
# endif
# define ECORE_DRM2_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_ecore_drm2_log_dom, __VA_ARGS__)

# ifdef CRIT
#  undef CRIT
# endif
# define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_drm2_log_dom, __VA_ARGS__)

struct _Ecore_Drm2_Device
{
   Elput_Manager *em;

   int fd;
   const char *path;

   int num_crtcs;
   uint32_t *crtcs;

   struct
     {
        uint32_t crtc, conn;
     } alloc;

   struct
     {
        uint32_t width, height;
     } min, max;

   Eeze_Udev_Watch *watch;

   Eina_List *outputs;
};

#endif
