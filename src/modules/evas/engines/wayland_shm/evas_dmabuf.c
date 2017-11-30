#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_engine.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "linux-dmabuf-unstable-v1-client-protocol.h"

typedef struct _Surface Surface;
struct _Surface
{
   Ecore_Wl2_Window *wl2_win;
   Ecore_Wl2_Buffer *current;
   Eina_List *buffers;

   int w, h;
   Eina_Bool alpha : 1;
   struct
     {
        void (*destroy)(Surface *surface);
        void (*reconfigure)(Surface *surface, int w, int h, uint32_t flags, Eina_Bool force);
        void *(*data_get)(Surface *surface, int *w, int *h);
        int  (*assign)(Surface *surface);
        void (*post)(Surface *surface, Eina_Rectangle *rects, unsigned int count);
     } funcs;
};

static void
_evas_dmabuf_surface_reconfigure(Surface *s, int w, int h, uint32_t flags EINA_UNUSED, Eina_Bool force)
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
_evas_dmabuf_surface_data_get(Surface *s, int *w, int *h)
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
_evas_dmabuf_surface_wait(Surface *s)
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
_evas_dmabuf_surface_assign(Surface *s)
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
_evas_dmabuf_surface_post(Surface *s, Eina_Rectangle *rects, unsigned int count)
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
_evas_dmabuf_surface_destroy(Surface *s)
{
   Ecore_Wl2_Buffer *b;

   if (!s) return;

   EINA_LIST_FREE(s->buffers, b)
     ecore_wl2_buffer_destroy(b);

   free(s);
}

void surface_destroy(Surface *surface)
{
   EINA_SAFETY_ON_NULL_RETURN(surface);

   surface->funcs.destroy(surface);
}

void surface_reconfigure(Surface *surface, int w, int h, uint32_t flags, Eina_Bool force)
{
   EINA_SAFETY_ON_NULL_RETURN(surface);

   surface->funcs.reconfigure(surface, w, h, flags, force);
}

void *surface_data_get(Surface *surface, int *w, int *h)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, NULL);

   return surface->funcs.data_get(surface, w, h);
}

int surface_assign(Surface *surface)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(surface, 0);

   return surface->funcs.assign(surface);
}

void surface_post(Surface *surface, Eina_Rectangle *rects, unsigned int count)
{
   EINA_SAFETY_ON_NULL_RETURN(surface);

   surface->funcs.post(surface, rects, count);
}

Surface *
_evas_surface_create(Ecore_Wl2_Window *win, Eina_Bool alpha)
{
   Surface *out;
   Ecore_Wl2_Display *ewd;
   Ecore_Wl2_Buffer_Type types = 0;

   out = calloc(1, sizeof(*out));
   if (!out) return NULL;
   out->wl2_win = win;

   ewd = ecore_wl2_window_display_get(win);
   if (ecore_wl2_display_shm_get(ewd))
     types |= ECORE_WL2_BUFFER_SHM;
   if (ecore_wl2_display_dmabuf_get(ewd))
     types |= ECORE_WL2_BUFFER_DMABUF;

   out->alpha = alpha;
   out->w = 0;
   out->h = 0;

   /* create surface buffers */
   if (!ecore_wl2_buffer_init(ewd, types)) goto err;

   out->funcs.destroy = _evas_dmabuf_surface_destroy;
   out->funcs.reconfigure = _evas_dmabuf_surface_reconfigure;
   out->funcs.data_get = _evas_dmabuf_surface_data_get;
   out->funcs.assign = _evas_dmabuf_surface_assign;
   out->funcs.post = _evas_dmabuf_surface_post;

   return out;

err:
   free(out);
   return NULL;
}
