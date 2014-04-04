#ifndef EVAS_ENGINE_H
# define EVAS_ENGINE_H

#include "evas_common_private.h"
#include "evas_macros.h"
#include "evas_private.h"
#include "Evas.h"
#include "Evas_Engine_Drm.h"

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

extern int _evas_engine_drm_log_dom;

# ifdef ERR
#  undef ERR
# endif
# define ERR(...) EINA_LOG_DOM_ERR(_evas_engine_drm_log_dom, __VA_ARGS__)

# ifdef DBG
#  undef DBG
# endif
# define DBG(...) EINA_LOG_DOM_DBG(_evas_engine_drm_log_dom, __VA_ARGS__)

# ifdef INF
#  undef INF
# endif
# define INF(...) EINA_LOG_DOM_INFO(_evas_engine_drm_log_dom, __VA_ARGS__)

# ifdef WRN
#  undef WRN
# endif
# define WRN(...) EINA_LOG_DOM_WARN(_evas_engine_drm_log_dom, __VA_ARGS__)

# ifdef CRI
#  undef CRI
# endif
# define CRI(...) EINA_LOG_DOM_CRIT(_evas_engine_drm_log_dom, __VA_ARGS__)

/* define a maximum number of 'buffers' (double-buff, triple-buff, etc) */
# define NUM_BUFFERS 2

typedef struct _Buffer Buffer;
typedef struct _Plane Plane;
typedef struct _Outbuf Outbuf;

enum
{
   MODE_FULL,
   MODE_COPY,
   MODE_DOUBLE,
   MODE_TRIPLE
};

enum
{
   OUTBUF_DEPTH_NONE,
   OUTBUF_DEPTH_ARGB_32BPP_8888_8888,
   OUTBUF_DEPTH_RGB_32BPP_8888_8888,
   OUTBUF_DEPTH_LAST
};

struct _Buffer
{
   int w, h;
   int stride, size;
   int handle;
   unsigned int fb;

   void *data; // used for software framebuffers

# ifdef HAVE_DRM_HW_ACCEL
   void *bo; // used for hardware framebuffers
# endif

   Eina_Bool valid : 1;
};

struct _Plane
{
   unsigned int id;
   unsigned int crtcs;

   struct 
     {
        unsigned int x, y;
        unsigned int w, h;
     } src, dst;

   unsigned int num_formats;
   unsigned int formats[];
};

struct _Outbuf
{
   int w, h;
   unsigned int rotation, depth;
   Eina_Bool destination_alpha : 1;
   Eina_Bool vsync : 1;

   struct 
     {
        int fd;
        unsigned int conn, crtc, fb;

        Buffer buffer[NUM_BUFFERS];
        int curr, num;

        drmModeModeInfo mode;
        drmEventContext ctx;
        Eina_Bool pending_flip : 1;

        Eina_List *pending_writes;

        Eina_List *planes;

# ifdef HAVE_DRM_HW_ACCEL
        void *surface;
# endif
     } priv;
};

Outbuf *evas_outbuf_setup(Evas_Engine_Info_Drm *info, int w, int h);
void evas_outbuf_free(Outbuf *ob);
void evas_outbuf_reconfigure(Evas_Engine_Info_Drm *info, Outbuf *ob, int w, int h);
int evas_outbuf_buffer_state_get(Outbuf *ob);
RGBA_Image *evas_outbuf_update_region_new(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch);
void evas_outbuf_update_region_push(Outbuf *ob, RGBA_Image *update, int x, int y, int w, int h);
void evas_outbuf_update_region_free(Outbuf *ob, RGBA_Image *update);
void evas_outbuf_flush(Outbuf *ob);

Eina_Bool evas_drm_init(Evas_Engine_Info_Drm *info, int card);
Eina_Bool evas_drm_shutdown(Evas_Engine_Info_Drm *info);

Eina_Bool evas_drm_outbuf_setup(Outbuf *ob);
void evas_drm_outbuf_framebuffer_set(Outbuf *ob, Buffer *buffer);
Eina_Bool evas_drm_framebuffer_create(int fd, Buffer *buffer, int depth);
void evas_drm_framebuffer_destroy(int fd, Buffer *buffer);
Eina_Bool evas_drm_framebuffer_send(Outbuf *ob, Buffer *buffer);

#endif
