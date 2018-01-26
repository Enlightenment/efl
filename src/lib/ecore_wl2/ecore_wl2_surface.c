#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_wl2_private.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "linux-dmabuf-unstable-v1-client-protocol.h"

#define MAX_BUFFERS 4

static Eina_List *_smanagers = NULL;
static int _smanager_count = 0;

int ECORE_WL2_SURFACE_DMABUF = 0;

typedef struct _Ecore_Wl2_Dmabuf_Private
{
   Ecore_Wl2_Buffer *current;
   Eina_List *buffers;
} Ecore_Wl2_Dmabuf_Private;

static void *
_evas_dmabuf_surface_setup(Ecore_Wl2_Window *win)
{
   Ecore_Wl2_Dmabuf_Private *priv;
   Ecore_Wl2_Display *ewd;
   Ecore_Wl2_Buffer_Type types = 0;

   priv = calloc(1, sizeof(*priv));
   if (!priv) return NULL;

   ewd = ecore_wl2_window_display_get(win);
   if (ecore_wl2_display_shm_get(ewd))
     types |= ECORE_WL2_BUFFER_SHM;
   if (ecore_wl2_display_dmabuf_get(ewd))
     types |= ECORE_WL2_BUFFER_DMABUF;

   if (!ecore_wl2_buffer_init(ewd, types))
     {
        free(priv);
        return NULL;
     }

   return priv;
}

static void
_evas_dmabuf_surface_reconfigure(Ecore_Wl2_Surface *s EINA_UNUSED, void *priv_data, int w, int h, uint32_t flags EINA_UNUSED)
{
   Ecore_Wl2_Dmabuf_Private *p;
   Ecore_Wl2_Buffer *b;
   Eina_List *l, *tmp;

   p = priv_data;

   if ((!w) || (!h)) return;
   EINA_LIST_FOREACH_SAFE(p->buffers, l, tmp, b)
     {
        if (ecore_wl2_buffer_fit(b, w, h))
          continue;

        ecore_wl2_buffer_destroy(b);
        p->buffers = eina_list_remove_list(p->buffers, l);
     }
}

static void *
_evas_dmabuf_surface_data_get(Ecore_Wl2_Surface *s EINA_UNUSED, void *priv_data, int *w, int *h)
{
   Ecore_Wl2_Dmabuf_Private *p;
   Ecore_Wl2_Buffer *b;
   void *ptr;
   int stride;

   p = priv_data;

   b = p->current;
   if (!b) return NULL;

   ptr = ecore_wl2_buffer_map(b, NULL, h, &stride);
   if (!ptr) return NULL;

   /* We return stride/bpp because it may not match the allocated
    * width.  evas will figure out the clipping
    */
   if (w) *w = stride / 4;

   return ptr;
}

static Ecore_Wl2_Buffer *
_evas_dmabuf_surface_wait(Ecore_Wl2_Surface *s, Ecore_Wl2_Dmabuf_Private *p)
{
   Ecore_Wl2_Buffer *b, *best = NULL;
   Eina_List *l;
   int best_age = -1;
   int age;

   EINA_LIST_FOREACH(p->buffers, l, b)
     {
        if (ecore_wl2_buffer_busy_get(b)) continue;
        age = ecore_wl2_buffer_age_get(b);
        if (age > best_age)
          {
             best = b;
             best_age = age;
          }
     }

   if (!best && (eina_list_count(p->buffers) < MAX_BUFFERS))
     {
        best = ecore_wl2_surface_buffer_create(s);
        /* Start at -1 so it's age is incremented to 0 for first draw */
        ecore_wl2_buffer_age_set(best, -1);
        p->buffers = eina_list_append(p->buffers, best);
     }
   return best;
}

static int
_evas_dmabuf_surface_assign(Ecore_Wl2_Surface *s, void *priv_data)
{
   Ecore_Wl2_Dmabuf_Private *p;
   Ecore_Wl2_Buffer *b;
   Eina_List *l;

   p = priv_data;
   p->current = _evas_dmabuf_surface_wait(s, p);
   if (!p->current)
     {
        /* Should be unreachable and will result in graphical
         * anomalies - we should probably blow away all the
         * existing buffers and start over if we actually
         * see this happen...
         */
        WRN("No free DMAbuf buffers, dropping a frame");
        EINA_LIST_FOREACH(p->buffers, l, b)
          ecore_wl2_buffer_age_set(b, 0);
        return 0;
     }
   EINA_LIST_FOREACH(p->buffers, l, b)
     ecore_wl2_buffer_age_inc(b);

   return ecore_wl2_buffer_age_get(p->current);
}

static void
_evas_dmabuf_surface_post(Ecore_Wl2_Surface *s, void *priv_data, Eina_Rectangle *rects, unsigned int count)
{
   Ecore_Wl2_Dmabuf_Private *p;
   Ecore_Wl2_Buffer *b;
   Ecore_Wl2_Window *win;

   p = priv_data;

   b = p->current;
   if (!b) return;

   ecore_wl2_buffer_unlock(b);

   p->current = NULL;
   ecore_wl2_buffer_busy_set(b);
   ecore_wl2_buffer_age_set(b, 0);

   win = ecore_wl2_surface_window_get(s);

   ecore_wl2_window_buffer_attach(win, b->wl_buffer, 0, 0, EINA_FALSE);
   ecore_wl2_window_damage(win, rects, count);

   ecore_wl2_window_commit(win, EINA_TRUE);
}

static void
_evas_dmabuf_surface_destroy(Ecore_Wl2_Surface *s EINA_UNUSED, void *priv_data)
{
   Ecore_Wl2_Dmabuf_Private *p;
   Ecore_Wl2_Buffer *b;

   p = priv_data;

   EINA_LIST_FREE(p->buffers, b)
     ecore_wl2_buffer_destroy(b);
}

static void
_evas_dmabuf_surface_flush(Ecore_Wl2_Surface *surface EINA_UNUSED, void *priv_data)
{
   Ecore_Wl2_Dmabuf_Private *p;
   Ecore_Wl2_Buffer *b;

   p = priv_data;

   EINA_LIST_FREE(p->buffers, b)
     ecore_wl2_buffer_destroy(b);
}


EAPI void
ecore_wl2_surface_destroy(Ecore_Wl2_Surface *surface)
{
   EINA_SAFETY_ON_NULL_RETURN(surface);

   surface->funcs->destroy(surface, surface->private_data);
   surface->wl2_win = NULL;

   free(surface);
}

EAPI void
ecore_wl2_surface_reconfigure(Ecore_Wl2_Surface *surface, int w, int h, uint32_t flags)
{
   EINA_SAFETY_ON_NULL_RETURN(surface);

   surface->funcs->reconfigure(surface, surface->private_data, w, h, flags);
   surface->w = w;
   surface->h = h;
}

EAPI void *
ecore_wl2_surface_data_get(Ecore_Wl2_Surface *surface, int *w, int *h)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, NULL);

   return surface->funcs->data_get(surface, surface->private_data, w, h);
}

EAPI int
ecore_wl2_surface_assign(Ecore_Wl2_Surface *surface)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, 0);

   return surface->funcs->assign(surface, surface->private_data);
}

EAPI void
ecore_wl2_surface_post(Ecore_Wl2_Surface *surface, Eina_Rectangle *rects, unsigned int count)
{
   EINA_SAFETY_ON_NULL_RETURN(surface);

   surface->funcs->post(surface, surface->private_data, rects, count);
}

EAPI void
ecore_wl2_surface_flush(Ecore_Wl2_Surface *surface)
{
   EINA_SAFETY_ON_NULL_RETURN(surface);

   surface->funcs->flush(surface, surface->private_data);
}

static Ecore_Wl2_Surface_Interface dmabuf_smanager =
{
   .version = 1,
   .setup = _evas_dmabuf_surface_setup,
   .destroy = _evas_dmabuf_surface_destroy,
   .reconfigure = _evas_dmabuf_surface_reconfigure,
   .data_get = _evas_dmabuf_surface_data_get,
   .assign = _evas_dmabuf_surface_assign,
   .post = _evas_dmabuf_surface_post,
   .flush = _evas_dmabuf_surface_flush
};

EAPI Ecore_Wl2_Surface *
ecore_wl2_surface_create(Ecore_Wl2_Window *win, Eina_Bool alpha)
{
   Ecore_Wl2_Surface *out;
   Eina_List *l;
   Ecore_Wl2_Surface_Interface *intf;

   EINA_SAFETY_ON_NULL_RETURN_VAL(win, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(_smanagers, NULL);

   if (win->wl2_surface) return win->wl2_surface;

   out = calloc(1, sizeof(*out));
   if (!out) return NULL;

   out->wl2_win = win;
   out->alpha = alpha;
   out->w = 0;
   out->h = 0;

   EINA_LIST_FOREACH(_smanagers, l, intf)
     {
        out->private_data = intf->setup(win);
        if (out->private_data)
          {
             out->funcs = intf;
             win->wl2_surface = out;
             return out;
          }
     }

   free(out);
   return NULL;
}

EAPI Ecore_Wl2_Buffer *
ecore_wl2_surface_buffer_create(Ecore_Wl2_Surface *surface)
{
   Ecore_Wl2_Display *ewd;

   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, NULL);

   ewd = ecore_wl2_window_display_get(surface->wl2_win);
   EINA_SAFETY_ON_NULL_RETURN_VAL(ewd, NULL);

   return ecore_wl2_buffer_create(ewd, surface->w, surface->h, surface->alpha);
}

EAPI int
ecore_wl2_surface_manager_add(Ecore_Wl2_Surface_Interface *intf)
{
   if (intf->version < ECORE_WL2_SURFACE_INTERFACE_VERSION)
     return 0;

   _smanagers = eina_list_prepend(_smanagers, intf);
   intf->id = ++_smanager_count;
   return intf->id;
}

EAPI void
ecore_wl2_surface_manager_del(Ecore_Wl2_Surface_Interface *intf)
{
   _smanagers = eina_list_remove(_smanagers, intf);
}

EAPI Ecore_Wl2_Window *
ecore_wl2_surface_window_get(Ecore_Wl2_Surface *surface)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, NULL);

   return surface->wl2_win;
}

/* TEMPORARY HACK FOR TESTING */
Eina_Bool
ecore_wl2_surface_manager_dmabuf_add(void)
{
   ECORE_WL2_SURFACE_DMABUF = ecore_wl2_surface_manager_add(&dmabuf_smanager);

   if (ECORE_WL2_SURFACE_DMABUF < 1)
     return EINA_FALSE;

   return EINA_TRUE;
}
