#include "evas_common.h"
#include "evas_engine.h"

void 
evas_outbuf_free(Outbuf *ob) 
{
   if (!ob) return;
   if (ob->priv.buffer) evas_cache_image_drop(&ob->priv.buffer->cache_entry);
   free(ob);
}

void 
evas_outbuf_resize(Outbuf *ob, int w, int h) 
{
   if (!ob) return;
   if ((ob->w == w) && (ob->h == h)) return;
   ob->w = w;
   ob->h = h;
   if (ob->priv.buffer) evas_cache_image_drop(&ob->priv.buffer->cache_entry);
   ob->priv.buffer = NULL;
}

Outbuf *
evas_outbuf_setup(int w, int h, int rot, Eina_Bool alpha, void *dest) 
{
   Outbuf *ob = NULL;

   if (!(ob = calloc(1, sizeof(Outbuf)))) return NULL;

   ob->w = w;
   ob->h = h;
   ob->rotation = rot;
   ob->priv.dest = dest;
   ob->priv.destination_alpha = alpha;

   ob->priv.buffer = 
     (RGBA_Image *)evas_cache_image_data(evas_common_image_cache_get(), 
                                         w, h, ob->priv.dest, 
                                         1, EVAS_COLORSPACE_ARGB8888);

   return ob;
}

RGBA_Image *
evas_outbuf_new_region_for_update(Outbuf *ob, int x, int y, int w, int h, int *cx, int *cy, int *cw, int *ch) 
{
   if (ob->priv.buffer)
     {
	*cx = x; *cy = y; *cw = w; *ch = h;
	return ob->priv.buffer;
     }
   else
     {
	RGBA_Image *im;

	*cx = 0; *cy = 0; *cw = w; *ch = h;
	im = (RGBA_Image *)evas_cache_image_empty(evas_common_image_cache_get());
        if (im) 
          {
             im->cache_entry.flags.alpha = ob->priv.destination_alpha;
             im = (RGBA_Image *)evas_cache_image_size_set(&im->cache_entry, w, h);
          }

        return im;
     }

   return NULL;
}

void 
evas_outbuf_push_updated_region(Outbuf *ob, RGBA_Image *update, int x __UNUSED__, int y, int w, int h) 
{
   if (!ob->priv.dest) return;
   if (!ob->priv.buffer) 
     {
        Gfx_Func_Copy func;

        func = evas_common_draw_func_copy_get(w, 0);
        if (func) 
          {
             DATA32 *dst, *src;
             int yy = 0, bytes = 0;

             bytes = ((w * sizeof(int)) * h);
             for (yy = 0; yy < h; yy++) 
               {
                  src = update->image.data + (yy * update->cache_entry.w);
                  dst = (DATA32 *)((DATA8 *)(ob->priv.dest) + 
                                   ((y + yy) * bytes));
                  func(src, dst, w);
               }
          }
     }
}

void 
evas_outbuf_free_region_for_update(Outbuf *ob, RGBA_Image *update) 
{
   if (!ob) return;
   if (update != ob->priv.buffer) evas_cache_image_drop(&update->cache_entry);
}
