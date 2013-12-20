#ifndef _ECORE_DRM_PRIVATE_H
# define _ECORE_DRM_PRIVATE_H

# include "Ecore.h"
# include "ecore_private.h"
# include "Ecore_Input.h"

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <fcntl.h>
# include <errno.h>
# include <signal.h>
# include <sys/mman.h>
# include <sys/socket.h>
/* # include <sys/un.h> */

# include <libudev.h>

# include <xf86drm.h>
# include <xf86drmMode.h>
# include <drm_fourcc.h>

# ifdef BUILD_ECORE_DRM_HW_ACCEL
#  include <gbm.h>
# endif

# include <Ecore_Drm.h>

# ifndef DRM_MAJOR
#  define DRM_MAJOR 226
# endif

# ifdef ECORE_DRM_DEFAULT_LOG_COLOR
#  undef ECORE_DRM_DEFAULT_LOG_COLOR
# endif
# define ECORE_DRM_DEFAULT_LOG_COLOR EINA_COLOR_BLUE

# ifdef ERR
#  undef ERR
# endif
# ifdef DBG
#  undef DBG
# endif
# ifdef INF
#  undef INF
# endif
# ifdef WRN
#  undef WRN
# endif
# ifdef CRIT
#  undef CRIT
# endif

extern int _ecore_drm_log_dom;

/* undef this for non-testing builds */
# define LOG_TO_FILE

# ifdef LOG_TO_FILE
extern FILE *lg;

#  define ERR(...) \
   EINA_LOG_DOM_ERR(_ecore_drm_log_dom, __VA_ARGS__); \
   fflush(lg);

#  define DBG(...) \
   EINA_LOG_DOM_DBG(_ecore_drm_log_dom, __VA_ARGS__); \
   fflush(lg);

#  define INF(...) \
   EINA_LOG_DOM_INFO(_ecore_drm_log_dom, __VA_ARGS__); \
   fflush(lg);

#  define WRN(...) \
   EINA_LOG_DOM_WARN(_ecore_drm_log_dom, __VA_ARGS__); \
   fflush(lg);

#  define CRIT(...) \
   EINA_LOG_DOM_CRIT(_ecore_drm_log_dom, __VA_ARGS__); \
   fflush(lg);

# else
#  define ERR(...) EINA_LOG_DOM_ERR(_ecore_drm_log_dom, __VA_ARGS__)
#  define DBG(...) EINA_LOG_DOM_DBG(_ecore_drm_log_dom, __VA_ARGS__)
#  define INF(...) EINA_LOG_DOM_INFO(_ecore_drm_log_dom, __VA_ARGS__)
#  define WRN(...) EINA_LOG_DOM_WARN(_ecore_drm_log_dom, __VA_ARGS__)
#  define CRIT(...) EINA_LOG_DOM_CRIT(_ecore_drm_log_dom, __VA_ARGS__)
# endif

extern struct udev *udev;

struct _Ecore_Drm_Device
{
   int id, fd;
   const char *devname;
   const char *devpath;

   struct 
     {
        int fd;
        const char *name;
        Ecore_Event_Handler *event_hdlr;
     } tty;
};

void _ecore_drm_message_send(int opcode, void *data, size_t bytes);
Eina_Bool _ecore_drm_message_receive(int opcode, void **data, size_t bytes);

#endif
