#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_engine.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "linux-dmabuf-unstable-v1-client-protocol.h"

typedef struct _Dmabuf_Surface Dmabuf_Surface;
struct _Dmabuf_Surface
{
   Surface *surface;

   Ecore_Wl2_Buffer *current;
   Ecore_Wl2_Buffer **buffer;
   int nbuf;

   Eina_Bool alpha : 1;
};

static void _internal_evas_dmabuf_surface_destroy(Dmabuf_Surface *surface);
static void _evas_dmabuf_surface_destroy(Surface *s);

static void
_evas_dmabuf_surface_reconfigure(Surface *s, int w, int h, uint32_t flags EINA_UNUSED, Eina_Bool force)
{
   Ecore_Wl2_Buffer *buf;
   Dmabuf_Surface *surface;
   int i;

   if ((!w) || (!h)) return;
   surface = s->surf.dmabuf;
   for (i = 0; i < surface->nbuf; i++)
     {
        if (surface->buffer[i])
          {
             Ecore_Wl2_Buffer *b = surface->buffer[i];
             int stride = b->stride;

             /* If stride is a little bigger than width we still fit */
             if (!force && (w >= b->w) && (w <= stride / 4) && (h == b->h))
               {
                  b->w = w;
                  continue;
               }

             ecore_wl2_buffer_destroy(b);
          }
        buf = ecore_wl2_buffer_create(s->ob->ewd, w, h, surface->alpha);
        if (!buf)  return;
        surface->buffer[i] = buf;
     }
}

static void *
_evas_dmabuf_surface_data_get(Surface *s, int *w, int *h)
{
   Dmabuf_Surface *surface;
   Ecore_Wl2_Buffer *b;
   void *ptr;

   surface = s->surf.dmabuf;
   b = surface->current;
   if (!b) return NULL;

   /* We return stride/bpp because it may not match the allocated
    * width.  evas will figure out the clipping
    */
   if (w) *w = b->stride / 4;
   if (h) *h = b->h;
   if (b->locked) return b->mapping;

   ptr = ecore_wl2_buffer_map(b);
   if (!ptr)
     return NULL;

   b->mapping = ptr;
   b->locked = EINA_TRUE;
   return b->mapping;
}

static Ecore_Wl2_Buffer *
_evas_dmabuf_surface_wait(Dmabuf_Surface *s)
{
   int i = 0, best = -1, best_age = -1;

   for (i = 0; i < s->nbuf; i++)
     {
        if (s->buffer[i]->locked || s->buffer[i]->busy) continue;
        if (s->buffer[i]->age > best_age)
          {
             best = i;
             best_age = s->buffer[i]->age;
          }
     }

   if (best >= 0) return s->buffer[best];
   return NULL;
}

static int
_evas_dmabuf_surface_assign(Surface *s)
{
   Dmabuf_Surface *surface;
   int i;

   surface = s->surf.dmabuf;
   surface->current = _evas_dmabuf_surface_wait(surface);
   if (!surface->current)
     {
        WRN("No free DMAbuf buffers, dropping a frame");
        for (i = 0; i < surface->nbuf; i++)
          surface->buffer[i]->age = 0;
        return 0;
     }
   for (i = 0; i < surface->nbuf; i++)
     if (surface->buffer[i]->used) surface->buffer[i]->age++;

   return surface->current->age;
}

static void
_evas_dmabuf_surface_post(Surface *s, Eina_Rectangle *rects, unsigned int count)
{
   Dmabuf_Surface *surface;
   Ecore_Wl2_Buffer *b;
   Ecore_Wl2_Window *win;

   surface = s->surf.dmabuf;
   b = surface->current;
   if (!b) return;

   ecore_wl2_buffer_unlock(b);

   surface->current = NULL;
   b->busy = EINA_TRUE;
   b->used = EINA_TRUE;
   b->age = 0;

   win = s->info->info.wl2_win;

   ecore_wl2_window_buffer_attach(win, b->wl_buffer, 0, 0, EINA_FALSE);
   ecore_wl2_window_damage(win, rects, count);

   ecore_wl2_window_commit(win, EINA_TRUE);
}

static void
_internal_evas_dmabuf_surface_destroy(Dmabuf_Surface *surface)
{
   int i;

   for (i = 0; i < surface->nbuf; i++)
      ecore_wl2_buffer_destroy(surface->buffer[i]);

   free(surface->buffer);
   surface->buffer = NULL;
   surface->nbuf = 0;
   free(surface);
}

static void
_evas_dmabuf_surface_destroy(Surface *s)
{
   if (!s) return;

   _internal_evas_dmabuf_surface_destroy(s->surf.dmabuf);
}

Eina_Bool
_evas_dmabuf_surface_create(Surface *s, int w, int h, int num_buff)
{
   Ecore_Wl2_Display *ewd;
   Ecore_Wl2_Buffer_Type types = 0;
   Dmabuf_Surface *surf = NULL;
   int i = 0;

   ewd = s->info->info.wl2_display;
   if (ecore_wl2_display_shm_get(ewd))
     types |= ECORE_WL2_BUFFER_SHM;
   if (ecore_wl2_display_dmabuf_get(ewd))
     types |= ECORE_WL2_BUFFER_DMABUF;

   if (!(s->surf.dmabuf = calloc(1, sizeof(Dmabuf_Surface)))) return EINA_FALSE;
   surf = s->surf.dmabuf;

   surf->surface = s;
   surf->alpha = s->info->info.destination_alpha;

   /* create surface buffers */
   surf->nbuf = num_buff;
   surf->buffer = calloc(surf->nbuf, sizeof(Ecore_Wl2_Buffer *));
   if (!surf->buffer) goto err;

   if (!ecore_wl2_buffer_init(ewd, types)) goto err;

   if (w && h)
     {
        for (i = 0; i < num_buff; i++)
          {
             surf->buffer[i] = ecore_wl2_buffer_create(s->ob->ewd,
                                                       w, h, surf->alpha);
             if (!surf->buffer[i])
               {
                  DBG("Could not create buffers");
                  /* _init() handled surface cleanup when it failed */
                  return EINA_FALSE;
               }
          }
     }

   s->funcs.destroy = _evas_dmabuf_surface_destroy;
   s->funcs.reconfigure = _evas_dmabuf_surface_reconfigure;
   s->funcs.data_get = _evas_dmabuf_surface_data_get;
   s->funcs.assign = _evas_dmabuf_surface_assign;
   s->funcs.post = _evas_dmabuf_surface_post;

   return EINA_TRUE;

err:
   free(surf->buffer);
   free(surf);
   return EINA_FALSE;
}
