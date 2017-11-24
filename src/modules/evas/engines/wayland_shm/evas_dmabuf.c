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
   Eina_List *buffers;

   Eina_Bool alpha : 1;
};

static void _internal_evas_dmabuf_surface_destroy(Dmabuf_Surface *surface);
static void _evas_dmabuf_surface_destroy(Surface *s);

static void
_evas_dmabuf_surface_reconfigure(Surface *s, int w, int h, uint32_t flags EINA_UNUSED, Eina_Bool force)
{
   Ecore_Wl2_Buffer *b;
   Eina_List *l, *tmp;
   Dmabuf_Surface *surface;

   if ((!w) || (!h)) return;
   surface = s->surf.dmabuf;
   EINA_LIST_FOREACH_SAFE(surface->buffers, l, tmp, b)
     {
        int stride = b->stride;

        /* If stride is a little bigger than width we still fit */
        if (!force && (w >= b->w) && (w <= stride / 4) && (h == b->h))
          {
             b->w = w;
             continue;
          }
        ecore_wl2_buffer_destroy(b);
        surface->buffers = eina_list_remove_list(surface->buffers, l);
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
   Ecore_Wl2_Buffer *b, *best = NULL;
   Eina_List *l;
   int best_age = -1;

   EINA_LIST_FOREACH(s->buffers, l, b)
     {
        if (b->locked || b->busy) continue;
        if (b->age > best_age)
          {
             best = b;
             best_age = b->age;
          }
     }

   if (!best && (eina_list_count(s->buffers) < MAX_BUFFERS))
     {
        Outbuf *ob;
        ob = s->surface->ob;
        best = ecore_wl2_buffer_create(ob->ewd, ob->w, ob->h, s->alpha);
        /* Start at -1 so it's age is incremented to 0 for first draw */
        best->age = -1;
        s->buffers = eina_list_append(s->buffers, best);
     }
   return best;
}

static int
_evas_dmabuf_surface_assign(Surface *s)
{
   Ecore_Wl2_Buffer *b;
   Eina_List *l;
   Dmabuf_Surface *surface;

   surface = s->surf.dmabuf;
   surface->current = _evas_dmabuf_surface_wait(surface);
   if (!surface->current)
     {
        /* Should be unreachable and will result in graphical
         * anomalies - we should probably blow away all the
         * existing buffers and start over if we actually
         * see this happen...
         */
        WRN("No free DMAbuf buffers, dropping a frame");
        EINA_LIST_FOREACH(surface->buffers, l, b)
          b->age = 0;
        return 0;
     }
   EINA_LIST_FOREACH(surface->buffers, l, b)
     b->age++;

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
   b->age = 0;

   win = s->info->info.wl2_win;

   ecore_wl2_window_buffer_attach(win, b->wl_buffer, 0, 0, EINA_FALSE);
   ecore_wl2_window_damage(win, rects, count);

   ecore_wl2_window_commit(win, EINA_TRUE);
}

static void
_internal_evas_dmabuf_surface_destroy(Dmabuf_Surface *surface)
{
   Ecore_Wl2_Buffer *b;

   EINA_LIST_FREE(surface->buffers, b)
     ecore_wl2_buffer_destroy(b);

   free(surface);
}

static void
_evas_dmabuf_surface_destroy(Surface *s)
{
   if (!s) return;

   _internal_evas_dmabuf_surface_destroy(s->surf.dmabuf);
}

Eina_Bool
_evas_dmabuf_surface_create(Surface *s)
{
   Ecore_Wl2_Display *ewd;
   Ecore_Wl2_Buffer_Type types = 0;
   Dmabuf_Surface *surf = NULL;

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
   if (!ecore_wl2_buffer_init(ewd, types)) goto err;

   s->funcs.destroy = _evas_dmabuf_surface_destroy;
   s->funcs.reconfigure = _evas_dmabuf_surface_reconfigure;
   s->funcs.data_get = _evas_dmabuf_surface_data_get;
   s->funcs.assign = _evas_dmabuf_surface_assign;
   s->funcs.post = _evas_dmabuf_surface_post;

   return EINA_TRUE;

err:
   free(surf);
   return EINA_FALSE;
}
