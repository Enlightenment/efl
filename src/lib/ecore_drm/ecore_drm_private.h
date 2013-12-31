#ifndef _ECORE_DRM_PRIVATE_H
# define _ECORE_DRM_PRIVATE_H

# include "Ecore.h"
# include "ecore_private.h"
# include "Ecore_Input.h"

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <errno.h>
# include <sys/mman.h>
# include <fcntl.h>

# include <libudev.h>
# include <libinput.h>

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

# ifndef DRM_CAP_TIMESTAMP_MONOTONIC
#  define DRM_CAP_TIMESTAMP_MONOTONIC 0x6
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

struct _Ecore_Drm_Output_Mode
{
   unsigned int flags;
   int width, height;
   unsigned int refresh;
   drmModeModeInfo info;
};

struct _Ecore_Drm_Output
{
   unsigned int crtc_id;
   unsigned int conn_id;
   drmModeCrtcPtr crtc;

   int x, y;

   const char *make, *model, *name;
   unsigned int subpixel;

   Ecore_Drm_Output_Mode *current_mode;
   Eina_List *modes;
   /* TODO: finish */
};

struct _Ecore_Drm_Seat
{
   struct libinput_seat *seat;
   const char *name;
   Ecore_Drm_Input *input;
   Eina_List *devices;
};

struct _Ecore_Drm_Input
{
   int fd;
   struct libinput *linput;
   Ecore_Fd_Handler *input_hdlr;
   Eina_Bool suspended : 1;
};

struct _Ecore_Drm_Evdev
{
   Ecore_Drm_Seat *seat;
   struct libinput *linput;
   struct libinput_device *dev;
   const char *name;
   int fd;
};

struct _Ecore_Drm_Device
{
   int id;
   const char *seat;

   struct 
     {
        int fd;
        const char *name;
        const char *path;
        clockid_t clock;
        Ecore_Fd_Handler *hdlr;
     } drm;

   unsigned int min_width, min_height;
   unsigned int max_width, max_height;

   unsigned int crtc_count;
   unsigned int *crtcs;
   unsigned int crtc_allocator;

   Eina_List *inputs;
   Eina_List *outputs;

   struct 
     {
        int fd;
        const char *name;
        Ecore_Event_Handler *event_hdlr;
     } tty;
};

void _ecore_drm_message_send(int opcode, int fd, void *data, size_t bytes);
Eina_Bool _ecore_drm_message_receive(int opcode, int *fd, void **data, size_t bytes);

Ecore_Drm_Evdev *_ecore_drm_evdev_device_create(struct libinput *linput, struct libinput_device *device);
void _ecore_drm_evdev_device_destroy(Ecore_Drm_Evdev *evdev);
int _ecore_drm_evdev_event_process(struct libinput_event *event);

#endif
