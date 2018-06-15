#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore_Wl2.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "linux-dmabuf-unstable-v1-client-protocol.h"

#define MAX_BUFFERS 4
#define QUEUE_TRIM_DURATION 100

int ECORE_WL2_SURFACE_DMABUF = 0;

typedef struct _Ecore_Wl2_Dmabuf_Private
{
   Ecore_Wl2_Buffer *current;
   Eina_List *buffers;
   int unused_duration;
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
_evas_dmabuf_surface_reconfigure(Ecore_Wl2_Surface *s EINA_UNUSED, void *priv_data, int w, int h, uint32_t flags EINA_UNUSED, Eina_Bool alpha EINA_UNUSED)
{
   Ecore_Wl2_Dmabuf_Private *p;
   Ecore_Wl2_Buffer *b;
   Eina_List *l, *tmp;
//   Eina_Bool alpha_change;

   p = priv_data;

   if ((!w) || (!h)) return;
//   alpha_change = ecore_wl2_surface_alpha_get(s) != alpha;
   EINA_LIST_FOREACH_SAFE(p->buffers, l, tmp, b)
     {
/*      This would be nice, but requires a partial create to follow,
        and that partial create is buffer type specific.

        if (!alpha_change && ecore_wl2_buffer_fit(b, w, h))
          continue;
*/
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
   int num_required = 1, num_allocated = 0;

   EINA_LIST_FOREACH(p->buffers, l, b)
     {
        num_allocated++;
        if (ecore_wl2_buffer_busy_get(b))
          {
             num_required++;
             continue;
          }
        age = ecore_wl2_buffer_age_get(b);
        if (age > best_age)
          {
             best = b;
             best_age = age;
          }
     }

   if (num_required < num_allocated)
      p->unused_duration++;
   else
      p->unused_duration = 0;

   /* If we've had unused buffers for longer than QUEUE_TRIM_DURATION, then
    * destroy the oldest buffer (currently in best) and recursively call
    * ourself to get the next oldest.
    */
   if (best && (p->unused_duration > QUEUE_TRIM_DURATION))
     {
        p->unused_duration = 0;
        p->buffers = eina_list_remove(p->buffers, best);
        ecore_wl2_buffer_destroy(best);
        best = _evas_dmabuf_surface_wait(s, p);
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
//        WRN("No free DMAbuf buffers, dropping a frame");
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
   struct wl_buffer *wlb;

   p = priv_data;

   b = p->current;
   if (!b) return;

   ecore_wl2_buffer_unlock(b);

   p->current = NULL;
   ecore_wl2_buffer_busy_set(b);
   ecore_wl2_buffer_age_set(b, 0);

   win = ecore_wl2_surface_window_get(s);

   wlb = ecore_wl2_buffer_wl_buffer_get(b);
   ecore_wl2_window_buffer_attach(win, wlb, 0, 0, EINA_FALSE);
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

   free(p);
}

static void
_evas_dmabuf_surface_flush(Ecore_Wl2_Surface *surface EINA_UNUSED, void *priv_data, Eina_Bool purge EINA_UNUSED)
{
   Ecore_Wl2_Dmabuf_Private *p;
   Ecore_Wl2_Buffer *b;

   p = priv_data;

   EINA_LIST_FREE(p->buffers, b)
     {
        if (!ecore_wl2_buffer_busy_get(b))
          {
             if (p->current == b)
               p->current = NULL;
             ecore_wl2_buffer_destroy(b);
          }
     }
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

Eina_Bool
ecore_wl2_surface_module_dmabuf_init(void)
{
   ECORE_WL2_SURFACE_DMABUF = ecore_wl2_surface_manager_add(&dmabuf_smanager);

   if (ECORE_WL2_SURFACE_DMABUF < 1)
     return EINA_FALSE;

   return EINA_TRUE;
}

void
ecore_wl2_surface_module_dmabuf_shutdown(void)
{
   ecore_wl2_surface_manager_del(&dmabuf_smanager);
}

EINA_MODULE_INIT(ecore_wl2_surface_module_dmabuf_init);
EINA_MODULE_SHUTDOWN(ecore_wl2_surface_module_dmabuf_shutdown);

