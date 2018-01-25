#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "linux-dmabuf-unstable-v1-client-protocol.h"

#define MAX_BUFFERS 4

static Eina_Bool
_evas_dmabuf_surface_check(Ecore_Wl2_Window *win)
{
   Ecore_Wl2_Display *ewd;
   Ecore_Wl2_Buffer_Type types = 0;

   ewd = ecore_wl2_window_display_get(win);
   if (ecore_wl2_display_shm_get(ewd))
     types |= ECORE_WL2_BUFFER_SHM;
   if (ecore_wl2_display_dmabuf_get(ewd))
     types |= ECORE_WL2_BUFFER_DMABUF;

   if (!ecore_wl2_buffer_init(ewd, types))
     return EINA_FALSE;

   return EINA_TRUE;
}

static void
_evas_dmabuf_surface_reconfigure(Ecore_Wl2_Surface *s, int w, int h, uint32_t flags EINA_UNUSED, Eina_Bool force)
{
   Ecore_Wl2_Buffer *b;
   Eina_List *l, *tmp;

   if ((!w) || (!h)) return;
   EINA_LIST_FOREACH_SAFE(s->buffers, l, tmp, b)
     {
        int stride = b->stride;

        /* If stride is a little bigger than width we still fit */
        if (!force && (w >= b->w) && (w <= stride / 4) && (h == b->h))
          {
             b->w = w;
             continue;
          }
        ecore_wl2_buffer_destroy(b);
        s->buffers = eina_list_remove_list(s->buffers, l);
     }
   s->w = w;
   s->h = h;
}

static void *
_evas_dmabuf_surface_data_get(Ecore_Wl2_Surface *s, int *w, int *h)
{
   Ecore_Wl2_Buffer *b;
   void *ptr;

   b = s->current;
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
_evas_dmabuf_surface_wait(Ecore_Wl2_Surface *s)
{
   Ecore_Wl2_Buffer *b, *best = NULL;
   Eina_List *l;
   int best_age = -1;

   EINA_LIST_FOREACH(s->buffers, l, b)
     {
        if (ecore_wl2_buffer_busy_get(b)) continue;
        if (b->age > best_age)
          {
             best = b;
             best_age = b->age;
          }
     }

   if (!best && (eina_list_count(s->buffers) < MAX_BUFFERS))
     {
        Ecore_Wl2_Display *ewd;

        ewd = ecore_wl2_window_display_get(s->wl2_win);
        EINA_SAFETY_ON_NULL_RETURN_VAL(ewd, NULL);

        best = ecore_wl2_buffer_create(ewd, s->w, s->h, s->alpha);
        /* Start at -1 so it's age is incremented to 0 for first draw */
        best->age = -1;
        s->buffers = eina_list_append(s->buffers, best);
     }
   return best;
}

static int
_evas_dmabuf_surface_assign(Ecore_Wl2_Surface *s)
{
   Ecore_Wl2_Buffer *b;
   Eina_List *l;

   s->current = _evas_dmabuf_surface_wait(s);
   if (!s->current)
     {
        /* Should be unreachable and will result in graphical
         * anomalies - we should probably blow away all the
         * existing buffers and start over if we actually
         * see this happen...
         */
        WRN("No free DMAbuf buffers, dropping a frame");
        EINA_LIST_FOREACH(s->buffers, l, b)
          b->age = 0;
        return 0;
     }
   EINA_LIST_FOREACH(s->buffers, l, b)
     b->age++;

   return s->current->age;
}

static void
_evas_dmabuf_surface_post(Ecore_Wl2_Surface *s, Eina_Rectangle *rects, unsigned int count)
{
   Ecore_Wl2_Buffer *b;

   b = s->current;
   if (!b) return;

   ecore_wl2_buffer_unlock(b);

   s->current = NULL;
   b->busy = EINA_TRUE;
   b->age = 0;

   ecore_wl2_window_buffer_attach(s->wl2_win, b->wl_buffer, 0, 0, EINA_FALSE);
   ecore_wl2_window_damage(s->wl2_win, rects, count);

   ecore_wl2_window_commit(s->wl2_win, EINA_TRUE);
}

static void
_evas_dmabuf_surface_destroy(Ecore_Wl2_Surface *s)
{
   Ecore_Wl2_Buffer *b;

   if (!s) return;

   EINA_LIST_FREE(s->buffers, b)
     ecore_wl2_buffer_destroy(b);
}

static void
_surface_flush(Ecore_Wl2_Surface *surface)
{
   Ecore_Wl2_Buffer *b;

   EINA_SAFETY_ON_NULL_RETURN(surface);

   EINA_LIST_FREE(surface->buffers, b)
     ecore_wl2_buffer_destroy(b);
}


EAPI void
ecore_wl2_surface_destroy(Ecore_Wl2_Surface *surface)
{
   EINA_SAFETY_ON_NULL_RETURN(surface);

   surface->funcs->destroy(surface);
   surface->wl2_win = NULL;

   free(surface);
}

EAPI void
ecore_wl2_surface_reconfigure(Ecore_Wl2_Surface *surface, int w, int h, uint32_t flags, Eina_Bool force)
{
   EINA_SAFETY_ON_NULL_RETURN(surface);

   surface->funcs->reconfigure(surface, w, h, flags, force);
}

EAPI void *
ecore_wl2_surface_data_get(Ecore_Wl2_Surface *surface, int *w, int *h)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, NULL);

   return surface->funcs->data_get(surface, w, h);
}

EAPI int
ecore_wl2_surface_assign(Ecore_Wl2_Surface *surface)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, 0);

   return surface->funcs->assign(surface);
}

EAPI void
ecore_wl2_surface_post(Ecore_Wl2_Surface *surface, Eina_Rectangle *rects, unsigned int count)
{
   EINA_SAFETY_ON_NULL_RETURN(surface);

   surface->funcs->post(surface, rects, count);
}

EAPI void
ecore_wl2_surface_flush(Ecore_Wl2_Surface *surface)
{
   EINA_SAFETY_ON_NULL_RETURN(surface);

   surface->funcs->flush(surface);
}

static Ecore_Wl2_Surface_Interface dmabuf_smanager =
{
   .check = _evas_dmabuf_surface_check,
   .destroy = _evas_dmabuf_surface_destroy,
   .reconfigure = _evas_dmabuf_surface_reconfigure,
   .data_get = _evas_dmabuf_surface_data_get,
   .assign = _evas_dmabuf_surface_assign,
   .post = _evas_dmabuf_surface_post,
   .flush = _surface_flush
};

EAPI Ecore_Wl2_Surface *
ecore_wl2_surface_create(Ecore_Wl2_Window *win, Eina_Bool alpha)
{
   Ecore_Wl2_Surface *out;

   EINA_SAFETY_ON_NULL_RETURN_VAL(win, NULL);

   if (win->wl2_surface) return win->wl2_surface;

   out = calloc(1, sizeof(*out));
   if (!out) return NULL;
   out->wl2_win = win;
   out->alpha = alpha;
   out->w = 0;
   out->h = 0;
   out->funcs = &dmabuf_smanager;

   if (out->funcs->check(win))
     {
        win->wl2_surface = out;
        return out;
     }

   free(out);
   return NULL;
}
