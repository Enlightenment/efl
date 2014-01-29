#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <sys/mman.h>

#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif

#include "evas_engine.h"

/* local structures */
typedef struct _Wl_Buffer Wl_Buffer;
struct _Wl_Buffer
{
   Wl_Swapper *ws;
   int w, h;
   void *data;
   int offset;
   unsigned int id, hdl;
   size_t size;
   Eina_Bool valid : 1;
};

struct _Wl_Swapper
{
   Wl_Buffer buff[3];
   Wl_Buffer *buffer_sent;

   int in_use, drm_fd;
   int dx, dy, w, h, depth;
   int buff_cur, buff_num;

   /* void *data; */

   Eina_Bool alpha : 1;
   Eina_Bool mapped : 1;
   Eina_Bool delete_me : 1;
};

/* local function prototypes */
/* static Eina_Bool _evas_swapper_shm_pool_new(Wl_Swapper *ws); */
/* static void _evas_swapper_shm_pool_free(Wl_Swapper *ws); */
static Eina_Bool _evas_swapper_buffer_new(Wl_Swapper *ws, Wl_Buffer *wb);
static void _evas_swapper_buffer_free(Wl_Swapper *ws, Wl_Buffer *wb);
static void _evas_swapper_buffer_put(Wl_Swapper *ws, Wl_Buffer *wb, Eina_Rectangle *rects, unsigned int count);

/* local variables */

Wl_Swapper *
evas_swapper_setup(int dx, int dy, int w, int h, Outbuf_Depth depth, Eina_Bool alpha, int fd)
{
   Wl_Swapper *ws;
   int i = 0;
   char *num_buffers;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* try to allocate a new swapper */
   if (!(ws = calloc(1, sizeof(Wl_Swapper)))) 
     return NULL;

   /* set some properties */
   ws->dx = dx;
   ws->dy = dy;
   ws->w = w;
   ws->h = h;
   ws->depth = depth;
   ws->alpha = alpha;
   ws->drm_fd = fd;

   /* double buffer by default */
   ws->buff_num = 2;

   /* check for buffer override number */
   if ((num_buffers = getenv("EVAS_DRM_BUFFERS")))
     {
        int num = 0;

        num = atoi(num_buffers);

        if (num <= 0) num = 1;
        if (num > 3) num = 3;

        ws->buff_num = num;
     }

   for (i = 0; i < ws->buff_num; i++)
     {
        /* try to create new internal Wl_Buffer */
        if (!_evas_swapper_buffer_new(ws, &(ws->buff[i])))
          {
             /* failed to create buffer. free the swapper */
             ERR("Failed to create new buffer");
             evas_swapper_free(ws);
             return NULL;
          }
     }

   /* return allocated swapper */
   return ws;
}

Wl_Swapper *
evas_swapper_reconfigure(Wl_Swapper *ws, int dx, int dy, int w, int h, Outbuf_Depth depth, Eina_Bool alpha)
{
   int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   if (!ws) return NULL;

   /* loop the swapper's buffers and free them */
   for (i = 0; i < ws->buff_num; i++)
     _evas_swapper_buffer_free(ws, &(ws->buff[i]));

   ws->dx += dx;
   ws->dy += dy;
   ws->w = w;
   ws->h = h;
   ws->depth = depth;
   ws->alpha = alpha;

   for (i = 0; i < ws->buff_num; i++)
     {
        /* try to create new internal Wl_Buffer */
        if (!_evas_swapper_buffer_new(ws, &(ws->buff[i])))
          {
             ERR("failed to create wl_buffer. free the swapper.");
             evas_swapper_free(ws);
             return NULL;
          }
     }

   /* return reconfigured swapper */
   return ws;
}

void 
evas_swapper_swap(Wl_Swapper *ws, Eina_Rectangle *rects, unsigned int count)
{
   int n = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid swapper */
   if (!ws) return;

   n = ws->buff_cur;
   _evas_swapper_buffer_put(ws, &(ws->buff[n]), rects, count);
   ws->buff[n].valid = EINA_TRUE;
   ws->in_use++;
   ws->buff_cur = (ws->buff_cur + 1) % ws->buff_num;
}

void 
evas_swapper_free(Wl_Swapper *ws)
{
   int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid swapper */
   if (!ws) return;

   /* loop the swapper's buffers and free them */
   for (i = 0; i < ws->buff_num; i++)
     _evas_swapper_buffer_free(ws, &(ws->buff[i]));

   if (ws->in_use)
     {
        ws->delete_me = EINA_TRUE;
        return;
     }

   /* free the allocated structure */
   free(ws);
}

void *
evas_swapper_buffer_map(Wl_Swapper *ws, int *w, int *h)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid swapper */
   if (!ws) return NULL;

   /* set mapped property */
   ws->mapped = EINA_TRUE;
   if (w) *w = ws->w;
   if (h) *h = ws->h;

   /* return wl_buffer data */
   return ws->buff[ws->buff_cur].data;
}

void 
evas_swapper_buffer_unmap(Wl_Swapper *ws)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid swapper */
   if (!ws) return;

   ws->mapped = EINA_FALSE;
}

int 
evas_swapper_buffer_state_get(Wl_Swapper *ws)
{
   int i = 0, n = 0, count = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   for (i = 0; i < ws->buff_num; i++)
     {
        n = (ws->buff_num + ws->buff_cur - (i)) % ws->buff_num;
        if (ws->buff[n].valid) count++;
        else break;
     }

   if (count == ws->buff_num)
     {
        if (count == 1) return MODE_COPY;
        else if (count == 2) return MODE_DOUBLE;
        else if (count == 3) return MODE_TRIPLE;
     }

   return MODE_FULL;
}

void 
evas_swapper_buffer_idle_flush(Wl_Swapper *ws)
{
   int i = 0;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid swapper */
   if (!ws) return;

   /* loop the swapper's buffers and free them */
   for (i = 0; i < ws->buff_num; i++)
     {
        Wl_Buffer *wb = NULL;

        /* try to get out Wl_Buffer struct */
        if (!(wb = (&(ws->buff[i])))) continue;

        /* if this buffer is not valid, then unmap data */
        if (!wb->valid) _evas_swapper_buffer_free(ws, wb);
     }
}

/* local functions */
static Eina_Bool 
_evas_swapper_buffer_new(Wl_Swapper *ws, Wl_Buffer *wb)
{
   /* struct drm_mode_create_dumb carg; */
   /* struct drm_mode_map_dumb marg; */
   /* struct drm_mode_destroy_dumb darg; */
   int ret;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   wb->w = ws->w;
   wb->h = ws->h;

   /* NB: Create drm Dumb FB for software rendering */
   /* memset(&carg, 0, sizeof(struct drm_mode_create_dumb)); */
   /* carg.bpp = 32; */
   /* carg.width = wb->w; */
   /* carg.height = wb->h; */

   /* ret = drmIoctl(ws->drm_fd, DRM_IOCTL_MODE_CREATE_DUMB, &carg); */
   /* if (ret < 0) */
   /*   { */
   /*      ERR("Failed to create dumb buffer: %m"); */
   /*      return EINA_FALSE; */
   /*   } */

   /* ret = drmModeAddFB(ws->drm_fd, wb->w, wb->h, 24, 32,  */
   /*                    carg.pitch, carg.handle, &wb->id); */
   /* if (ret) */
   /*   { */
   /*      ERR("Failed to add fb: %m"); */
   /*      goto err; */
   /*   } */

   /* memset(&marg, 0, sizeof(struct drm_mode_map_dumb)); */
   /* marg.handle = carg.handle; */
   /* ret = drmIoctl(ws->drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &marg); */
   /* if (ret) */
   /*   { */
   /*      ERR("Failed to Map fb: %m"); */
   /*      goto err_map; */
   /*   } */

   /* wb->data = mmap(0, carg.size, PROT_WRITE | PROT_READ, MAP_SHARED,  */
   /*                 ws->drm_fd, marg.offset); */
   /* memset(wb->data, 0, carg.size); */

   /* wb->hdl = marg.handle; */

   wb->ws = ws;

   /* return allocated buffer */
   return EINA_TRUE;

/* err_map: */
/*    drmModeRmFB(ws->drm_fd, wb->id); */
/* err: */
/*    memset(&darg, 0, sizeof(darg)); */
/*    darg.handle = carg.handle; */
/*    drmIoctl(ws->drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &darg); */
/*    return EINA_FALSE; */
}

static void 
_evas_swapper_buffer_free(Wl_Swapper *ws, Wl_Buffer *wb)
{
   /* struct drm_mode_destroy_dumb darg; */

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid buffer */
   if ((!wb) || (wb->valid)) return;

   /* unmap the buffer data */
   /* if (wb->data) munmap(wb->data, wb->size); */
   /* wb->data = NULL; */

   /* kill the wl_buffer */
   /* if (wb->id) drmModeRmFB(ws->drm_fd, wb->id); */

   /* if (wb->buffer) wl_buffer_destroy(wb->buffer); */
   /* wb->buffer = NULL; */

   /* memset(&darg, 0, sizeof(darg)); */
   /* darg.handle = wb->hdl; */
   /* drmIoctl(ws->drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &darg); */
}

static void 
_evas_swapper_buffer_put(Wl_Swapper *ws, Wl_Buffer *wb, Eina_Rectangle *rects, unsigned int count)
{
   Eina_Rectangle *rect;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);

   /* check for valid swapper */
   if (!ws) return;

   /* make sure swapper has a surface */
   /* if (!ws->surface) return; */

   /* check for valid buffer */
   if (!wb) return;

   /* make sure buffer has mapped data */
   if ((!wb->data))
     {
        /* call function to mmap buffer data */
        if (!_evas_swapper_buffer_new(ws, wb))
          return;
     }

   rect = eina_rectangle_new(0, 0, 0, 0);
   if ((rects) && (count > 0))
     {
        unsigned int i = 0;

        for (i = 0; i < count; i++)
          eina_rectangle_union(rect, &rects[i]);
     }
   else
     {
        Eina_Rectangle r;

        r.x = 0; r.y = 0;
        r.w = wb->w; r.h = wb->h;

        eina_rectangle_union(rect, &r);
     }

   /* surface attach */
   if (ws->buffer_sent != wb)
     {
        DBG("Send Buffer !!");
        /* wl_surface_attach(ws->surface, wb->buffer, ws->dx, ws->dy); */
        ws->dx = 0;
        ws->dy = 0;
        ws->buffer_sent = wb;
     }

   /* wl_surface_damage(ws->surface, rect->x, rect->y, rect->w, rect->h); */

   /* surface commit */
   /* wl_surface_commit(ws->surface); */
}
