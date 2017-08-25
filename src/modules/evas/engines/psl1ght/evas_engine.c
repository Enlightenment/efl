#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "rsxutil.h"
#include "Evas_Engine_PSL1GHT.h"

#include <malloc.h>

int _evas_engine_psl1ght_log_dom = -1;

/* function tables - filled in later (func and parent func) */
static Evas_Func func, pfunc;

/* engine struct data */
typedef struct _Render_Engine Render_Engine;

#define MAX_BUFFERS 2

struct _Render_Engine
{
   Tilebuf        *tb;
   Tilebuf_Rect   *rects;
   Eina_Inlist    *cur_rect;

   /* RSX device context */
   gcmContextData *context;
   void           *host_addr;

   /* The buffers we will be drawing into. */
   rsxBuffer       buffers[MAX_BUFFERS];
   int             currentBuffer;
   int             width;
   int             height;
   RGBA_Image     *rgba_image;
   uint32_t        rgba_image_offset;

   int             end : 1;
};

/* prototypes we will use here */
static void *_output_setup(int w, int h);

/* internal engine routines */
static void *
_output_setup(int w, int h)
{
   Render_Engine *re;
   int i;
   u16 width, height;
   DATA32 *image_data = NULL;
   int image_size;

   printf ("_output_setup called : %dx%d\n", w, h);
   re = calloc(1, sizeof(Render_Engine));
   if (!re)
     return NULL;

   /* Allocate a 1Mb buffer, alligned to a 1Mb boundary
    * to be our shared IO memory with the RSX. */
   re->host_addr = memalign (1024 * 1024, HOST_SIZE);
   if (re->host_addr == NULL)
     {
        free (re);
        return NULL;
     }
   re->context = initScreen (re->host_addr, HOST_SIZE);
   if (re->context == NULL)
     {
        free (re->host_addr);
        free (re);
        return NULL;
     }
   width = w;
   height = h;
   setResolution (re->context, &width, &height);
   re->currentBuffer = 0;
   re->width = width;
   re->height = height;

   for (i = 0; i < MAX_BUFFERS; i++)
     makeBuffer (&re->buffers[i], width, height, i);

   flipBuffer(re->context, MAX_BUFFERS - 1);

   re->tb = evas_common_tilebuf_new(w, h);

   /* in preliminary tests 16x16 gave highest framerates */
   evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

   /* Allocate our memaligned backbuffer */
   image_size = ((w * h * sizeof(u32)) + 0xfffff) & - 0x100000;
   image_data = memalign (1024 * 1024, image_size);
   re->rgba_image = (RGBA_Image *)evas_cache_image_data(evas_common_image_cache_get(),
                                                        w, h, image_data, 1, EVAS_COLORSPACE_ARGB8888);
   gcmMapMainMemory(image_data, image_size, &re->rgba_image_offset);

   return re;
}

/* engine api this module provides */
static void
eng_output_info_setup(void *info)
{
   Evas_Engine_Info_PSL1GHT *einfo = info;

   einfo->render_mode = EVAS_RENDER_MODE_BLOCKING;
}

static void *
eng_output_setup(void *engine EINA_UNUSED, void *in, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_PSL1GHT *info = in;

   printf ("eng_setup called\n");

   return _output_setup(w, h);
}

static void
eng_output_free(void *engine EINA_UNUSED, void *data)
{
   Render_Engine *re;
   int i;

   printf ("eng_output_free called\n");
   re = (Render_Engine *)data;

   gcmSetWaitFlip(re->context);
   for (i = 0; i < MAX_BUFFERS; i++)
     rsxFree (re->buffers[i].ptr);

   if (re->rgba_image)
     {
        DATA32 *image_data;

        image_data = re->rgba_image->image.data;
        evas_cache_image_drop(&re->rgba_image->cache_entry);
        free (image_data);
     }

   freeScreen (re->context);
   free (re->host_addr);

   evas_common_tilebuf_free(re->tb);
   if (re->rects)
     evas_common_tilebuf_free_render_rects(re->rects);

   free(re);
}

static void
eng_output_resize(void *engine EINA_UNUSED, void *data, int w, int h)
{
   Render_Engine *re;
   int i;
   u16 width, height;
   DATA32 *image_data;
   int image_size;

   printf ("eng_output_resize called : %dx%d\n", w, h);
   re = (Render_Engine *)data;

   width = w;
   height = h;
   if (setResolution (re->context, &width, &height))
     {
        re->width = width;
        re->height = height;

        gcmSetWaitFlip(re->context);
        for (i = 0; i < MAX_BUFFERS; i++) {
             rsxFree (re->buffers[i].ptr);
             makeBuffer (&re->buffers[i], width, height, i);
          }

        flipBuffer(re->context, MAX_BUFFERS - 1);

        evas_common_tilebuf_free(re->tb);
        re->tb = evas_common_tilebuf_new(w, h);
        evas_common_tilebuf_set_tile_size(re->tb, TILESIZE, TILESIZE);

        /* Realloc our backbuf image */
        if (re->rgba_image)
          {
             image_data = re->rgba_image->image.data;
             evas_cache_image_drop(&re->rgba_image->cache_entry);
             free (image_data);
          }
        image_size = ((w * h * sizeof(u32)) + 0xfffff) & - 0x100000;
        image_data = memalign (1024 * 1024, image_size);
        re->rgba_image = (RGBA_Image *)evas_cache_image_data(evas_common_image_cache_get(),
                                                             w, h, image_data, 1, EVAS_COLORSPACE_ARGB8888);
        gcmMapMainMemory(image_data, image_size, &re->rgba_image_offset);
     }
}

static void
eng_output_tile_size_set(void *engine EINA_UNUSED, void *data, int w, int h)
{
   Render_Engine *re;

   printf ("eng_output_tile_size_set called : %dx%d\n", w, h);
   re = (Render_Engine *)data;
   evas_common_tilebuf_set_tile_size(re->tb, w, h);
}

static void
eng_output_redraws_rect_add(void *engine EINA_UNUSED, void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   //printf ("eng_output_redraws_rect_add called : %d,%d %dx%d\n", x, y, w, h);
   re = (Render_Engine *)data;
   evas_common_tilebuf_add_redraw(re->tb, x, y, w, h);
}

static void
eng_output_redraws_rect_del(void *engine EINA_UNUSED, void *data, int x, int y, int w, int h)
{
   Render_Engine *re;

   //printf ("eng_output_redraws_rect_del called : %d,%d %dx%d\n", x, y, w, h);
   re = (Render_Engine *)data;
   evas_common_tilebuf_del_redraw(re->tb, x, y, w, h);
}

static void
eng_output_redraws_clear(void *engine EINA_UNUSED, void *data)
{
   Render_Engine *re;

   //printf ("eng_output_redraws_clear called\n");
   re = (Render_Engine *)data;
   evas_common_tilebuf_clear(re->tb);
}

static void *
eng_output_redraws_next_update_get(void *engine EINA_UNUSED, void *data, int *x, int *y, int *w, int *h, int *cx, int *cy, int *cw, int *ch)
{
   Render_Engine *re;
   Tilebuf_Rect *rect;
   int ux, uy, uw, uh;

   re = (Render_Engine *)data;
   /*printf ("eng_output_redraws_next_update_get called : %d,%d %dx%d -- %d,%d %dx%d\n",
    *x, *y, *w, *h, *cx, *cy, *cw, *ch);*/
   if (re->end)
     {
        re->end = 0;
        return NULL;
     }
   if (!re->rects)
     {
        re->rects = evas_common_tilebuf_get_render_rects(re->tb);
        re->cur_rect = EINA_INLIST_GET(re->rects);
     }
   if (!re->cur_rect)
     return NULL;

   rect = (Tilebuf_Rect *)re->cur_rect;
   ux = rect->x; uy = rect->y; uw = rect->w; uh = rect->h;
   re->cur_rect = re->cur_rect->next;
   if (!re->cur_rect)
     {
        evas_common_tilebuf_free_render_rects(re->rects);
        re->rects = NULL;
        re->end = 1;
     }

   *x = *cx = ux;
   *y = *cy = uy;
   *w = *cw = uw;
   *h = *ch = uh;
   /*printf ("eng_output_redraws_next_update_get returning : %d,%d %dx%d -- %d,%d %dx%d\n",
    *x, *y, *w, *h, *cx, *cy, *cw, *ch);*/

   return re->rgba_image;
}

static void
eng_output_redraws_next_update_push(void *engine EINA_UNUSED, void *data EINA_UNUSED, void *surface EINA_UNUSED, int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED, Evas_Render_Mode render_mode EINA_UNUSED)
{
   /* Don't do anything, we'll just coy the whole buffer when it's time to flush */
}

static void
eng_output_flush(void *engine EINA_UNUSED, void *data, Evas_Render_Mode render_mode)
{
   Render_Engine *re;
   rsxBuffer *buffer;
   int width;
   int height;

   if (render_mode == EVAS_RENDER_MODE_ASYNC_INIT) return;

   //printf ("eng_output_flush called\n");
   re = (Render_Engine *)data;
   buffer = &re->buffers[re->currentBuffer];
   width = re->rgba_image->cache_entry.w;
   height = re->rgba_image->cache_entry.h;

   /* Wait for the flip before copying the buffer */
   waitFlip ();

   if (re->width == width &&
       re->height == height)
     {
        /* DMA the back buffer into the rsx buffer */
         rsxSetTransferImage (re->context,
                              GCM_TRANSFER_MAIN_TO_LOCAL,
                              buffer->offset, buffer->width * sizeof(u32),
                              0, 0,
                              re->rgba_image_offset, re->width * sizeof(u32),
                              0, 0, re->width, re->height, sizeof(u32));
     }
   else
     {
        gcmTransferScale gcm_scale;
        gcmTransferSurface gcm_surface;

        gcm_surface.format = GCM_TRANSFER_SURFACE_FORMAT_A8R8G8B8;
        gcm_surface.pitch = buffer->width * sizeof(u32);
        gcm_surface._pad0[0] = gcm_surface._pad0[1] = 0;
        gcm_surface.offset = buffer->offset;

        gcm_scale.interp = GCM_TRANSFER_INTERPOLATOR_LINEAR;
        gcm_scale.conversion = GCM_TRANSFER_CONVERSION_TRUNCATE;
        gcm_scale.format = GCM_TRANSFER_SCALE_FORMAT_A8R8G8B8;
        gcm_scale.origin = GCM_TRANSFER_ORIGIN_CORNER;
        gcm_scale.operation = GCM_TRANSFER_OPERATION_SRCCOPY;
        gcm_scale.offset = re->rgba_image_offset;
        gcm_scale.clipX = 0;
        gcm_scale.clipY = 0;
        gcm_scale.clipW = re->width;
        gcm_scale.clipH = re->height;
        gcm_scale.outX = 0;
        gcm_scale.outY = 0;
        gcm_scale.outW = re->width;
        gcm_scale.outH = re->height;
        gcm_scale.ratioX = rsxGetFixedSint32 ((float)width / (float)re->width);
        gcm_scale.ratioY = rsxGetFixedSint32 ((float)height / (float)re->height);
        gcm_scale.inX = 0;
        gcm_scale.inY = 0;
        gcm_scale.inW = (width & ~1); // Width must be a multiple of 2
        gcm_scale.inH = height;
        if (gcm_scale.inW < 2) // Minimum inW value is 2
          gcm_scale.inW = 2;
        if (gcm_scale.inW > 2046) // Maximum inW value is 2046
          gcm_scale.inW = 2046;
        if (gcm_scale.inH < 1) // Minimum inH value is 1
          gcm_scale.inH = 1;
        if (gcm_scale.inH > 2047) // Maximum inW value is 2047
          gcm_scale.inH = 2047;
        gcm_scale.pitch = sizeof(u32) * width;

        rsxSetTransferScaleMode (re->context, GCM_TRANSFER_MAIN_TO_LOCAL, GCM_TRANSFER_SURFACE);
        rsxSetTransferScaleSurface (re->context, &gcm_scale, &gcm_surface);
     }
   /* Wait for the DMA to finish */
   flushRSX (re->context);

   /* Flip buffer onto screen */
   flipBuffer (re->context, re->currentBuffer);
   re->currentBuffer = (re->currentBuffer + 1) % MAX_BUFFERS;
}

static void
eng_output_idle_flush(void *engine EINA_UNUSED, void *data)
{
   Render_Engine *re;

   printf ("eng_output_idle_flush called\n");
   re = (Render_Engine *)data;
}

static Eina_Bool
eng_canvas_alpha_get(void *engine)
{
   Render_Engine *re;

   // printf ("eng_output_alpha_get called\n");
   re = (Render_Engine *)engine;
   return EINA_TRUE;
}

/* module advertising code */
static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic", sizeof (Evas_Engine_Info_PSL1GHT))) return 0;
   _evas_engine_psl1ght_log_dom = eina_log_domain_register
       ("evas-psl1ght", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_engine_psl1ght_log_dom < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        return 0;
     }

   /* store it for later use */
   func = pfunc;
   /* now to override methods */
#define ORD(f) EVAS_API_OVERRIDE(f, &func, eng_)
   ORD(output_info_setup);
   ORD(output_setup);
   ORD(canvas_alpha_get);
   ORD(output_free);
   ORD(output_resize);
   ORD(output_tile_size_set);
   ORD(output_redraws_rect_add);
   ORD(output_redraws_rect_del);
   ORD(output_redraws_clear);
   ORD(output_redraws_next_update_get);
   ORD(output_redraws_next_update_push);
   ORD(output_flush);
   ORD(output_idle_flush);

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_engine_psl1ght_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_psl1ght_log_dom);
        _evas_engine_psl1ght_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "psl1ght",
   "none",
   {
      module_open,
      module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, psl1ght);

#ifndef EVAS_STATIC_BUILD_PSL1GHT
EVAS_EINA_MODULE_DEFINE(engine, psl1ght);
#endif
