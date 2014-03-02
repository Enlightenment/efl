/*
 * \@file evas_filter.c
 *
 * Infrastructure for simple filters applied to RGBA and Alpha buffers.
 * Originally used by font effects.
 *
 * Filters include:
 * - Blur (Gaussian, Box, Motion) and Shadows
 * - Bump maps (light effects)
 * - Displacement maps
 * - Color curves
 * - Blending and masking
 *
 * The reference documentation can be found in evas_filter_parser.c
 */

#include "evas_filter.h"
#include "evas_private.h"
#include "evas_filter_private.h"

#ifdef EVAS_CSERVE2
# include "evas_cs2_private.h"
#endif

static void _buffer_free(Evas_Filter_Buffer *fb);
static void _command_del(Evas_Filter_Context *ctx, Evas_Filter_Command *cmd);

#ifdef CLAMP
# undef CLAMP
#endif
#define CLAMP(a,b,c) MIN(MAX((b),(a)),(c))

#define DRAW_COLOR_SET(r, g, b, a) do { cmd->draw.R = r; cmd->draw.G = g; cmd->draw.B = b; cmd->draw.A = a; } while (0)
#define DRAW_CLIP_SET(_x, _y, _w, _h) do { cmd->draw.clip.x = _x; cmd->draw.clip.y = _y; cmd->draw.clip.w = _w; cmd->draw.clip.h = _h; } while (0)
#define DRAW_FILL_SET(fmode) do { cmd->draw.fillmode = fmode; } while (0)

typedef struct _Evas_Filter_Thread_Command Evas_Filter_Thread_Command;
struct _Evas_Filter_Thread_Command
{
   Evas_Filter_Context *ctx;
   RGBA_Image *src, *mask, *dst;
   Evas_Filter_Apply_Func func;
};


/* Main functions */

Evas_Filter_Context *
evas_filter_context_new(Evas_Public_Data *evas, Eina_Bool async)
{
   Evas_Filter_Context *ctx;

   EINA_SAFETY_ON_NULL_RETURN_VAL(evas, NULL);

   ctx = calloc(1, sizeof(Evas_Filter_Context));
   if (!ctx) return NULL;

   ctx->evas = evas;
   ctx->async = async;
   ctx->gl_engine = !!strstr(evas->engine.module->definition->name, "gl");

   return ctx;
}

/* Private function to reset the filter context. Used from parser.c */
void
evas_filter_context_clear(Evas_Filter_Context *ctx)
{
   Evas_Filter_Buffer *fb;
   Evas_Filter_Command *cmd;

   if (!ctx) return;

   EINA_LIST_FREE(ctx->buffers, fb)
     _buffer_free(fb);
   EINA_INLIST_FREE(ctx->commands, cmd)
     _command_del(ctx, cmd);

   ctx->buffers = NULL;
   ctx->commands = NULL;
   ctx->last_buffer_id = 0;
   ctx->last_command_id = 0;

   // Note: don't reset post_run, as it it set by the client
}

static void
_backing_free(Evas_Filter_Context *ctx, Image_Entry *ie)
{
   if (!ie) return;

   if (!ctx->gl_engine)
     {
        if (!ctx->async)
          ENFN->image_free(ENDT, ie);
     }
   else
     {
#ifdef EVAS_CSERVE2
        if (evas_cserve2_use_get())
          evas_cache2_image_close(ie);
        else
#endif
          evas_cache_image_drop(ie);
     }
}

static void
_filter_buffer_backing_free(Evas_Filter_Buffer *fb)
{
   void *backing;
   if (!fb) return;

   if (fb->stolen)
     {
        fb->delete_me = EINA_TRUE;
        return;
     }

   INF("Free backing of buffer %d fb @ %p backing @ %p alloc %d", fb->id, fb, fb->backing, fb->allocated);
   backing = fb->backing;
   fb->backing = NULL;

   if (!fb->allocated) return;
   _backing_free(fb->ctx, backing);
}


/**
 * Render the source object when a proxy is set.
 *
 * Used to force a draw if necessary, else just makes sure it's available.
 * @note This comes direcly from evas_object_image.c. A common function is desirable here :)
 */
static void
_proxy_subrender(Evas *eo_e, Evas_Object *eo_source, Evas_Object *eo_proxy,
                 Evas_Object_Protected_Data *proxy_obj, Eina_Bool do_async)
{
   Evas_Public_Data *e = eo_data_scope_get(eo_e, EVAS_CLASS);
   Evas_Object_Protected_Data *source;
   Eina_Bool source_clip = EINA_FALSE;
   void *ctx;
   int w, h;

   if (!eo_source) return;
   source = eo_data_scope_get(eo_source, EVAS_OBJ_CLASS);

   w = source->cur->geometry.w;
   h = source->cur->geometry.h;

   EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, source->proxy, Evas_Object_Proxy_Data, proxy_write)
     {
        proxy_write->redraw = EINA_FALSE;

        /* We need to redraw surface then */
        if ((proxy_write->surface) &&
            ((proxy_write->w != w) || (proxy_write->h != h)))
          {
             e->engine.func->image_map_surface_free(e->engine.data.output,
                                                    proxy_write->surface);
             proxy_write->surface = NULL;
          }

        /* FIXME: Hardcoded alpha 'on' */
        /* FIXME (cont): Should see if the object has alpha */
        if (!proxy_write->surface)
          {
             proxy_write->surface = e->engine.func->image_map_surface_new
               (e->engine.data.output, w, h, 1);
             if (!proxy_write->surface) goto end;
             proxy_write->w = w;
             proxy_write->h = h;
          }

        ctx = e->engine.func->context_new(e->engine.data.output);
        e->engine.func->context_color_set(e->engine.data.output, ctx, 0, 0,
                                          0, 0);
        e->engine.func->context_render_op_set(e->engine.data.output, ctx,
                                              EVAS_RENDER_COPY);
        e->engine.func->rectangle_draw(e->engine.data.output, ctx,
                                       proxy_write->surface, 0, 0, w, h,
                                       do_async);
        e->engine.func->context_free(e->engine.data.output, ctx);

        ctx = e->engine.func->context_new(e->engine.data.output);

        if (eo_isa(eo_proxy, EVAS_OBJ_IMAGE_CLASS))
          eo_do(eo_proxy, evas_obj_image_source_clip_get(&source_clip));

        Evas_Proxy_Render_Data proxy_render_data = {
             .eo_proxy = eo_proxy,
             .proxy_obj = proxy_obj,
             .eo_src = eo_source,
             .source_clip = source_clip
        };
        evas_render_mapped(e, eo_source, source, ctx, proxy_write->surface,
                           -source->cur->geometry.x,
                           -source->cur->geometry.y,
                           1, 0, 0, e->output.w, e->output.h,
                           &proxy_render_data
#ifdef REND_DBG
                           , 1
#endif
                           , do_async);

        e->engine.func->context_free(e->engine.data.output, ctx);
        proxy_write->surface = e->engine.func->image_dirty_region
           (e->engine.data.output, proxy_write->surface, 0, 0, w, h);
     }
 end:
   EINA_COW_WRITE_END(evas_object_proxy_cow, source->proxy, proxy_write);
}

/** @hidden private render proxy objects */
void
evas_filter_context_proxy_render_all(Evas_Filter_Context *ctx, Eo *eo_obj,
                                     Eina_Bool do_async)
{
   Evas_Object_Protected_Data *source;
   Evas_Object_Protected_Data *obj;
   Evas_Filter_Buffer *fb;
   Eina_List *li;

   obj = eo_data_scope_get(eo_obj, EVAS_OBJ_CLASS);

   if (!ctx->has_proxies) return;
   if (ctx->gl_engine)
     {
        // FIXME: We need to call glReadPixels (yeah, no other way around...)
        ERR("Proxy subrender is not supported in the GL engine (yet)");
        return;
     }

   EINA_LIST_FOREACH(ctx->buffers, li, fb)
     if (fb->source)
       {
          // TODO: Lock current object as proxyrendering (see image obj)
          source = eo_data_scope_get(fb->source, EVAS_OBJ_CLASS);
          if (source->proxy->surface && !source->proxy->redraw)
            {
               INF("Source already rendered");
               _filter_buffer_backing_free(fb);
               fb->backing = source->proxy->surface;
               fb->w = source->cur->geometry.w;
               fb->h = source->cur->geometry.h;
               fb->allocated = EINA_FALSE;
               fb->alpha_only = EINA_FALSE;
            }
          else
            {
               INF("Source needs to be rendered");
               _proxy_subrender(ctx->evas->evas, fb->source, eo_obj, obj, do_async);
               _filter_buffer_backing_free(fb);
               fb->backing = source->proxy->surface;
               fb->w = source->cur->geometry.w;
               fb->h = source->cur->geometry.h;
               fb->allocated = EINA_FALSE;
               fb->alpha_only = EINA_FALSE;
            }
          DBG("Source has dimensions %dx%d (buffer %d)", fb->w, fb->h, fb->id);
       }
}

void
evas_filter_context_destroy(Evas_Filter_Context *ctx)
{
   Evas_Filter_Buffer *fb;
   Evas_Filter_Command *cmd;

   if (!ctx) return;

   EINA_LIST_FREE(ctx->buffers, fb)
     _buffer_free(fb);
   EINA_INLIST_FREE(ctx->commands, cmd)
     _command_del(ctx, cmd);

   free(ctx);
}

void
evas_filter_context_post_run_callback_set(Evas_Filter_Context *ctx,
                                          Evas_Filter_Cb cb, void *data)
{
   EINA_SAFETY_ON_NULL_RETURN(ctx);
   ctx->post_run.cb = cb;
   ctx->post_run.data = data;
}

static Evas_Filter_Buffer *
_buffer_new(Evas_Filter_Context *ctx, int w, int h, Eina_Bool alpha_only)
{
   Evas_Filter_Buffer *fb;

   fb = calloc(1, sizeof(Evas_Filter_Buffer));
   if (!fb) return NULL;

   fb->id = ++(ctx->last_buffer_id);
   fb->ctx = ctx;
   fb->alpha_only = alpha_only;
   fb->transient = EINA_TRUE;
   fb->w = w;
   fb->h = h;

   ctx->buffers = eina_list_append(ctx->buffers, fb);
   return fb;
}

static RGBA_Image *
_rgba_image_alloc(Evas_Filter_Buffer const *fb, void *data)
{
   Evas_Colorspace cspace;
   RGBA_Image *image;
   size_t sz;

   cspace = fb->alpha_only ? EVAS_COLORSPACE_GRY8 : EVAS_COLORSPACE_ARGB8888;
   if (!fb->ctx->gl_engine)
     {
        if (!data)
          {
             image = fb->ENFN->image_new_from_copied_data
                   (fb->ENDT, fb->w, fb->h, NULL, EINA_TRUE, cspace);
          }
        else
          {
             image = fb->ENFN->image_new_from_data
                   (fb->ENDT, fb->w, fb->h, data, EINA_TRUE, cspace);
          }
     }
   else
     {
        // FIXME: Directly calling the alloc functions since we want to use sw surfaces.

        if (!data)
          {
#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get())
               image = (RGBA_Image *) evas_cache2_image_copied_data
                     (evas_common_image_cache2_get(), fb->w, fb->h, NULL, EINA_TRUE, cspace);
             else
#endif
               image = (RGBA_Image *) evas_cache_image_copied_data
                     (evas_common_image_cache_get(), fb->w, fb->h, NULL, EINA_TRUE, cspace);
          }
        else
          {
#ifdef EVAS_CSERVE2
             if (evas_cserve2_use_get())
               image = (RGBA_Image *) evas_cache2_image_data
                     (evas_common_image_cache2_get(), fb->w, fb->h, data, EINA_TRUE, cspace);
             else
#endif
               image = (RGBA_Image *) evas_cache_image_data
                     (evas_common_image_cache_get(), fb->w, fb->h, data, EINA_TRUE, cspace);
          }
     }
   if (!image) return EINA_FALSE;

   if (fb->alpha_only)
     sz = image->cache_entry.w * image->cache_entry.h * sizeof(DATA8);
   else
     sz = image->cache_entry.w * image->cache_entry.h * sizeof(DATA32);
   if (!data) memset(image->image.data, 0, sz);

   return image;
}

Eina_Bool
evas_filter_context_buffers_allocate_all(Evas_Filter_Context *ctx,
                                         unsigned w, unsigned h)
{
   Evas_Filter_Command *cmd;
   Evas_Filter_Buffer *fb;
   Image_Entry *ie;
   Eina_List *li;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, EINA_FALSE);
   ctx->w = w;
   ctx->h = h;

   //DBG("Allocating all buffers based on output size %ux%u", w, h);

   EINA_INLIST_FOREACH(ctx->commands, cmd)
     {
        Evas_Filter_Fill_Mode fillmode = cmd->draw.fillmode;
        Evas_Filter_Buffer *in, *out;

        in = cmd->input;
        if (!in->w && !in->h)
          {
             in->w = w;
             in->h = h;
          }

        if (fillmode & EVAS_FILTER_FILL_MODE_STRETCH_XY)
          {
             unsigned sw = w, sh = h;

             switch (cmd->mode)
               {
                case EVAS_FILTER_MODE_BLEND:
                  in = cmd->input;
                  break;
                case EVAS_FILTER_MODE_BUMP:
                case EVAS_FILTER_MODE_DISPLACE:
                case EVAS_FILTER_MODE_MASK:
                  in = cmd->mask;
                  break;
                default:
                  CRI("Invalid fillmode set for command %d", cmd->mode);
                  return EINA_FALSE;
               }

             if (in->w) sw = in->w;
             if (in->h) sh = in->h;

             if ((sw != w) || (sh != h))
               {
                  if (fillmode & EVAS_FILTER_FILL_MODE_STRETCH_X)
                    sw = w;
                  if (fillmode & EVAS_FILTER_FILL_MODE_STRETCH_Y)
                    sh = h;

                  //DBG("Allocating temporary buffer of size %ux%u", sw, sh);
                  fb = evas_filter_buffer_alloc_new(ctx, sw, sh, in->alpha_only);
                  fb->transient = EINA_TRUE;
               }
          }

        if (cmd->draw.need_temp_buffer)
          {
             unsigned sw = w, sh = h;

             in = cmd->input;
             if (in->w) sw = in->w;
             if (in->h) sh = in->h;

             //DBG("Allocating temporary buffer of size %ux%u", sw, sh);
             fb = evas_filter_buffer_alloc_new(ctx, sw, sh, in->alpha_only);
             fb->transient = EINA_TRUE;
          }

        out = cmd->output;
        if (!out->w && !out->h)
          {
             out->w = w;
             out->h = h;
          }
     }

   EINA_LIST_FOREACH(ctx->buffers, li, fb)
     {
        ie = fb->backing;
        if (ie)
          {
             if (ctx->async)
               {
                  ie->references++;
                  evas_unref_queue_image_put(ctx->evas, ie);
               }
             continue;
          }

        if (fb->source)
          continue;

        if (fb->glimage)
          continue;

        if (!fb->w && !fb->h)
          {
             ERR("Size should be known at this point. Is this a dangling buffer?");
             continue;
          }

        //DBG("Allocating buffer of size %ux%u alpha %d", fb->w, fb->h, fb->alpha_only);
        ie = (Image_Entry *) _rgba_image_alloc(fb, NULL);
        if (!ie)
          {
             ERR("Buffer %d allocation failed!", fb->id);
             return EINA_FALSE;
          }

        fb->backing = ie;
        fb->allocated = (ie != NULL);
        if (ctx->async && fb->allocated)
          evas_unref_queue_image_put(ctx->evas, ie);
     }

   return EINA_TRUE;
}

int
evas_filter_buffer_empty_new(Evas_Filter_Context *ctx, Eina_Bool alpha_only)
{
   Evas_Filter_Buffer *fb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, -1);

   fb = _buffer_new(ctx, 0, 0, alpha_only);
   if (!fb) return -1;

   fb->transient = EINA_FALSE;

   return fb->id;
}

Eina_Bool
_filter_buffer_data_set(Evas_Filter_Context *ctx, int bufid, void *data,
                        int w, int h, Eina_Bool alpha_only)
{
   Evas_Filter_Buffer *fb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, EINA_FALSE);

   fb = _filter_buffer_get(ctx, bufid);
   if (!fb) return EINA_FALSE;

   _filter_buffer_backing_free(fb);
   if (w <= 0 || h <= 0)
     return EINA_FALSE;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(fb->backing == NULL, EINA_FALSE);
   // TODO: Check input parameters?
   fb->alpha_only = alpha_only;
   fb->w = w;
   fb->h = h;

   fb->backing = _rgba_image_alloc(fb, data);
   fb->allocated = (!data && (fb->backing != NULL));
   if (ctx->async && fb->allocated)
     evas_unref_queue_image_put(ctx->evas, fb->backing);
   return fb->allocated;
}

int
evas_filter_buffer_image_new(Evas_Filter_Context *ctx, RGBA_Image *image)
{
   Evas_Filter_Buffer *fb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(image, -1);

   fb = calloc(1, sizeof(Evas_Filter_Buffer));
   if (!fb) return -1;

   fb->id = ++(ctx->last_buffer_id);
   fb->ctx = ctx;
   fb->backing = image;
   fb->w = image->cache_entry.w;
   fb->h = image->cache_entry.h;
   fb->alpha_only = (image->cache_entry.space == EVAS_COLORSPACE_GRY8);

   ctx->buffers = eina_list_append(ctx->buffers, fb);
   return fb->id;
}

Evas_Filter_Buffer *
_filter_buffer_data_new(Evas_Filter_Context *ctx, void *data, int w, int h,
                        Eina_Bool alpha_only)
{
   Evas_Filter_Buffer *fb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(w > 0 && h > 0, NULL);

   fb = calloc(1, sizeof(Evas_Filter_Buffer));
   if (!fb) return NULL;

   fb->id = ++(ctx->last_buffer_id);
   fb->ctx = ctx;
   ctx->buffers = eina_list_append(ctx->buffers, fb);

   if (!_filter_buffer_data_set(ctx, fb->id, data, w, h, alpha_only))
     {
        ctx->buffers = eina_list_remove(ctx->buffers, fb);
        free(fb);
        return NULL;
     }

   return fb;
}

static void
_buffer_free(Evas_Filter_Buffer *fb)
{
   _filter_buffer_backing_free(fb);
   free(fb);
}

Evas_Filter_Buffer *
_filter_buffer_get(Evas_Filter_Context *ctx, int bufid)
{
   Evas_Filter_Buffer *buffer;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);

   EINA_LIST_FOREACH(ctx->buffers, l, buffer)
     if (buffer->id == bufid) return buffer;

   return NULL;
}

void *
evas_filter_buffer_backing_get(Evas_Filter_Context *ctx, int bufid)
{
   Evas_Filter_Buffer *buffer;

   buffer = _filter_buffer_get(ctx, bufid);
   if (!buffer) return NULL;

   return buffer->backing;
}

void *
evas_filter_buffer_backing_steal(Evas_Filter_Context *ctx, int bufid)
{
   Evas_Filter_Buffer *buffer;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, NULL);

   buffer = _filter_buffer_get(ctx, bufid);
   if (!buffer) return NULL;

   buffer->stolen = EINA_TRUE;
   if (buffer->glimage)
     return buffer->glimage;
   else
     {
        if (ctx->async)
          {
             Image_Entry *ie = buffer->backing;
             if (ie) ie->references++;
          }
        return buffer->backing;
     }
}

Eina_Bool
evas_filter_buffer_backing_release(Evas_Filter_Context *ctx, void *stolen_buffer)
{
   Image_Entry *ie = stolen_buffer;
   Evas_Filter_Buffer *fb;
   Eina_List *li;

   if (!stolen_buffer) return EINA_FALSE;
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, EINA_FALSE);

   EINA_LIST_FOREACH(ctx->buffers, li, fb)
     {
        if (fb->backing == ie)
          {
             fb->stolen = EINA_FALSE;
             if (fb->delete_me)
               {
                  ctx->buffers = eina_list_remove_list(ctx->buffers, li);
                  if (ctx->async)
                    {
                       if (fb->allocated)
                         evas_unref_queue_image_put(ctx->evas, ie);
                       free(fb);
                    }
                  else
                    _buffer_free(fb);
                  return EINA_TRUE;
               }
             return EINA_TRUE;
          }
        else if (fb->glimage == stolen_buffer)
          {
             fb->stolen = EINA_FALSE;
             if (fb->delete_me)
               {
                  ENFN->image_free(ENDT, stolen_buffer);
                  free(fb);
               }
             return EINA_TRUE;
          }
     }

   if (ctx->async)
     evas_unref_queue_image_put(ctx->evas, ie);
   else if (ctx->gl_engine)
     ENFN->image_free(ENDT, stolen_buffer);
   else
     _backing_free(ctx, ie);

   return EINA_TRUE;
}

static Evas_Filter_Command *
_command_new(Evas_Filter_Context *ctx, Evas_Filter_Mode mode,
             Evas_Filter_Buffer *input, Evas_Filter_Buffer *mask,
             Evas_Filter_Buffer *output)
{
   Evas_Filter_Command *cmd;

   cmd = calloc(1, sizeof(Evas_Filter_Command));
   if (!cmd) return NULL;

   cmd->id = ++(ctx->last_command_id);
   cmd->ctx = ctx;
   cmd->mode = mode;
   cmd->input = input;
   cmd->mask = mask;
   cmd->output = output;

   ctx->commands = eina_inlist_append(ctx->commands, EINA_INLIST_GET(cmd));
   return cmd;
}

static void
_command_del(Evas_Filter_Context *ctx, Evas_Filter_Command *cmd)
{
   if (!ctx || !cmd) return;
   ctx->commands = eina_inlist_remove(ctx->commands, EINA_INLIST_GET(cmd));
   switch (cmd->mode)
     {
      case EVAS_FILTER_MODE_CURVE: free(cmd->curve.data); break;
      default: break;
     }
   free(cmd);
}

Evas_Filter_Command *
_evas_filter_command_get(Evas_Filter_Context *ctx, int cmdid)
{
   Evas_Filter_Command *cmd;

   if (cmdid <= 0) return NULL;

   EINA_INLIST_FOREACH(ctx->commands, cmd)
     if (cmd->id == cmdid) return cmd;

   return NULL;
}

Evas_Filter_Buffer *
evas_filter_temporary_buffer_get(Evas_Filter_Context *ctx, int w, int h,
                                 Eina_Bool alpha_only)
{
   Evas_Filter_Buffer *buf = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(ctx->buffers, l, buf)
     {
        if (buf->transient && !buf->locked && (buf->alpha_only == alpha_only))
          {
             if ((!w || (w == buf->w))
                 && (!h || (h == buf->h)))
               {
                  buf->locked = EINA_TRUE;
                  return buf;
               }
          }
     }

   if (ctx->running && ctx->async)
     {
        ERR("Can not create a new buffer from this thread!");
        return NULL;
     }

   buf = _buffer_new(ctx, w, h, alpha_only);
   buf->locked = EINA_TRUE;
   INF("Created temporary buffer: %d", buf->id);
   return buf;
}

static void
_filter_buffer_unlock_all(Evas_Filter_Context *ctx)
{
   Evas_Filter_Buffer *buf = NULL;
   Eina_List *l;

   EINA_LIST_FOREACH(ctx->buffers, l, buf)
     buf->locked = EINA_FALSE;
}

int
evas_filter_command_fill_add(Evas_Filter_Context *ctx, void *draw_context,
                             int bufid)
{
   Evas_Filter_Command *cmd;
   Evas_Filter_Buffer *buf = NULL;
   int R, G, B, A, cx, cy, cw, ch;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(draw_context, -1);

   buf = _filter_buffer_get(ctx, bufid);
   if (!buf)
     {
        ERR("Buffer %d does not exist.", bufid);
        return -1;
     }

   cmd = _command_new(ctx, EVAS_FILTER_MODE_FILL, buf, NULL, buf);
   if (!cmd) return -1;

   ENFN->context_color_get(ENDT, draw_context, &R, &G, &B, &A);
   DRAW_COLOR_SET(R, G, B, A);

   ENFN->context_clip_get(ENDT, draw_context, &cx, &cy, &cw, &ch);
   DRAW_CLIP_SET(cx, cy, cw, ch);

   return cmd->id;
}

int
evas_filter_command_blur_add(Evas_Filter_Context *ctx, void *drawctx,
                             int inbuf, int outbuf, Evas_Filter_Blur_Type type,
                             int dx, int dy, int ox, int oy)
{
   Evas_Filter_Command *cmd = NULL;
   Evas_Filter_Buffer *in = NULL, *out = NULL, *tmp = NULL, *in_dy = NULL;
   Evas_Filter_Buffer *out_dy = NULL, *out_dx = NULL;
   Evas_Filter_Buffer *copybuf = NULL, *blur_out = NULL;
   Eina_Bool copy_back = EINA_FALSE, convert = EINA_FALSE;
   int R, G, B, A;
   int ret = 0, id;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(drawctx, -1);

   switch (type)
     {
      case EVAS_FILTER_BLUR_BOX:
        if (dx < 0) dx = 0;
        if (dy < 0) dy = 0;
        if (!dx && !dy) goto fail;
        break;
      case EVAS_FILTER_BLUR_GAUSSIAN:
        if (dx < 0) dx = 0;
        if (dy < 0) dy = 0;
        if (!dx && !dy) goto fail;
        break;
      default:
        CRI("Not implemented yet!");
        goto fail;
     }

   in = _filter_buffer_get(ctx, inbuf);
   if (!in)
     {
        ERR("Buffer %d does not exist [input].", inbuf);
        goto fail;
     }

   out = _filter_buffer_get(ctx, outbuf);
   if (!out)
     {
        ERR("Buffer %d does not exist [output].", outbuf);
        goto fail;
     }

   if (!in->alpha_only && out->alpha_only)
     {
        ERR("Output and input don't have the same format");
        goto fail;
     }
   else if (in->alpha_only && !out->alpha_only)
     {
        INF("Adding extra blending step (Alpha --> RGBA)");
        blur_out = evas_filter_temporary_buffer_get(ctx, 0, 0, EINA_TRUE);
        if (!blur_out) goto fail;
        convert = EINA_TRUE;
     }
   else
     blur_out = out;

   if (dx && dy)
     {
        tmp = evas_filter_temporary_buffer_get(ctx, 0, 0, in->alpha_only);
        if (!tmp) goto fail;

        if (!convert && (ox || oy))
          {
             copybuf = evas_filter_temporary_buffer_get(ctx, 0, 0, in->alpha_only);
             if (!copybuf) goto fail;
             copy_back = EINA_TRUE;
          }

        if (in == blur_out)
          {
             // IN = OUT and 2-D blur. IN -blur-> TMP -blur-> IN.
             out_dx = tmp;
             in_dy = tmp;
             out_dy = copybuf ? copybuf : in;
          }
        else
          {
             // IN != OUT and 2-D blur. IN -blur-> TMP -blur-> OUT.
             out_dx = tmp;
             in_dy = tmp;
             out_dy = copybuf ? copybuf : blur_out;
          }
     }
   else if (dx)
     {
        if ((in == blur_out) || ox || oy)
          {
             // IN = OUT and 1-D blur. IN -blur-> TMP -copy-> IN.
             tmp = evas_filter_temporary_buffer_get(ctx, 0, 0, in->alpha_only);
             if (!tmp) goto fail;
             copy_back = EINA_TRUE;
             copybuf = tmp;
             out_dx = tmp;
          }
        else
          {
             // IN != OUT and 1-D blur. IN -blur-> OUT.
             out_dx = blur_out;
          }
     }
   else
     {
        if ((in == blur_out) || ox || oy)
          {
             // IN = OUT and 1-D blur. IN -blur-> TMP -copy-> IN.
             tmp = evas_filter_temporary_buffer_get(ctx, 0, 0, in->alpha_only);
             if (!tmp) goto fail;
             copy_back = EINA_TRUE;
             copybuf = tmp;
             in_dy = in;
             out_dy = tmp;
          }
        else
          {
             // IN != OUT and 1-D blur. IN -blur-> OUT.
             in_dy = in;
             out_dy = blur_out;
          }
     }

   ENFN->context_color_get(ENDT, drawctx, &R, &G, &B, &A);

   if (dx)
     {
        INF("Add horizontal blur %d -> %d (%dpx)", in->id, out_dx->id, dx);
        cmd = _command_new(ctx, EVAS_FILTER_MODE_BLUR, in, NULL, out_dx);
        if (!cmd) goto fail;
        cmd->blur.type = type;
        cmd->blur.dx = dx;
        cmd->blur.dy = 0;
        DRAW_COLOR_SET(R, G, B, A);
        ret = cmd->id;
     }

   if (dy)
     {
        INF("Add vertical blur %d -> %d (%dpx)", in_dy->id, out_dy->id, dy);
        cmd = _command_new(ctx, EVAS_FILTER_MODE_BLUR, in_dy, NULL, out_dy);
        if (!cmd) goto fail;
        cmd->blur.type = type;
        cmd->blur.dx = 0;
        cmd->blur.dy = dy;
        DRAW_COLOR_SET(R, G, B, A);
        if (ret <= 0) ret = cmd->id;
     }

   if (copy_back)
     {
        if (!cmd) goto fail;
        if (!copybuf) goto fail;
        INF("Add copy %d -> %d", copybuf->id, blur_out->id);
        cmd->ENFN->context_color_set(cmd->ENDT, drawctx, 0, 0, 0, 255);
        id = evas_filter_command_blend_add(ctx, drawctx, copybuf->id, blur_out->id, ox, oy, EVAS_FILTER_FILL_MODE_NONE);
        cmd->ENFN->context_color_set(cmd->ENDT, drawctx, R, G, B, A);
        if (id < 0) goto fail;
        ox = oy = 0;
     }

   if (convert)
     {
        INF("Add convert %d -> %d", blur_out->id, out->id);
        id = evas_filter_command_blend_add(ctx, drawctx, blur_out->id, out->id, ox, oy, EVAS_FILTER_FILL_MODE_NONE);
        if (id < 0) goto fail;
     }

   _filter_buffer_unlock_all(ctx);
   return ret;

fail:
   ERR("Failed to add blur");
   _filter_buffer_unlock_all(ctx);
   return -1;
}

int
evas_filter_command_blend_add(Evas_Filter_Context *ctx, void *drawctx,
                              int inbuf, int outbuf, int ox, int oy,
                              Evas_Filter_Fill_Mode fillmode)
{
   Evas_Filter_Command *cmd;
   Evas_Filter_Buffer *in, *out;
   int R, G, B, A;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, -1);

   if (inbuf == outbuf)
     {
        DBG("Skipping NOP blend operation %d --> %d", inbuf, outbuf);
        return -1;
     }

   in = _filter_buffer_get(ctx, inbuf);
   if (!in)
     {
        ERR("Buffer %d does not exist [input].", inbuf);
        return -1;
     }

   out = _filter_buffer_get(ctx, outbuf);
   if (!out)
     {
        ERR("Buffer %d does not exist [output].", outbuf);
        return -1;
     }

   cmd = _command_new(ctx, EVAS_FILTER_MODE_BLEND, in, NULL, out);
   if (!cmd) return -1;

   ENFN->context_color_get(ENDT, drawctx, &R, &G, &B, &A);
   DRAW_COLOR_SET(R, G, B, A);
   DRAW_FILL_SET(fillmode);
   cmd->draw.ox = ox;
   cmd->draw.oy = oy;
   cmd->draw.render_op = ENFN->context_render_op_get(ENDT, drawctx);
   cmd->draw.clip_use =
         ENFN->context_clip_get(ENDT, drawctx,
                                &cmd->draw.clip.x, &cmd->draw.clip.y,
                                &cmd->draw.clip.w, &cmd->draw.clip.h);

   if (cmd->draw.clip_use)
     DBG("Draw clip: %d,%d,%d,%d", cmd->draw.clip.x, cmd->draw.clip.y,
         cmd->draw.clip.w, cmd->draw.clip.h);

   return cmd->id;
}

int
evas_filter_command_grow_add(Evas_Filter_Context *ctx, void *draw_context,
                             int inbuf, int outbuf, int radius, Eina_Bool smooth)
{
   int blurcmd, threshcmd, blendcmd, tmin = 0, growbuf;
   int diam = abs(radius) * 2 + 1;
   DATA8 curve[256] = {0};
   Evas_Filter_Buffer *tmp = NULL, *in;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, -1);

   in = _filter_buffer_get(ctx, inbuf);
   EINA_SAFETY_ON_NULL_RETURN_VAL(in, -1);

   if (inbuf != outbuf)
     {
        tmp = evas_filter_temporary_buffer_get(ctx, in->w, in->h, in->alpha_only);
        EINA_SAFETY_ON_NULL_RETURN_VAL(tmp, -1);
        growbuf = tmp->id;
     }
   else
     growbuf = outbuf;

   blurcmd = evas_filter_command_blur_add(ctx, draw_context, inbuf, growbuf,
                                          EVAS_FILTER_BLUR_DEFAULT,
                                          abs(radius), abs(radius), 0, 0);
   if (blurcmd < 0) return -1;

   if (diam > 255) diam = 255;
   if (radius > 0)
     tmin = 255 / diam;
   else if (radius < 0)
     tmin = 256 - (255 / diam);

   if (!smooth)
     memset(curve + tmin, 255, 256 - tmin);
   else
     {
        int k, start, end, range;

        // This is pretty experimental.
        range = MAX(2, 12 - radius);
        start = ((tmin > range) ? (tmin - range) : 0);
        end = ((tmin < (256 - range)) ? (tmin + range) : 256);

        for (k = start; k < end; k++)
          curve[k] = ((k - start) * 255) / (end - start);
        if (end < 256)
          memset(curve + end, 255, 256 - end);
     }

   threshcmd = evas_filter_command_curve_add(ctx, draw_context, growbuf, growbuf,
                                             curve, EVAS_FILTER_CHANNEL_ALPHA);
   if (threshcmd < 0)
     {
        _command_del(ctx, _evas_filter_command_get(ctx, blurcmd));
        return -1;
     }

   if (tmp)
     {
        blendcmd = evas_filter_command_blend_add(ctx, draw_context, tmp->id,
                                                 outbuf, 0, 0,
                                                 EVAS_FILTER_FILL_MODE_NONE);
        if (blendcmd < 0)
          {
             _command_del(ctx, _evas_filter_command_get(ctx, threshcmd));
             _command_del(ctx, _evas_filter_command_get(ctx, blurcmd));
             return -1;
          }
     }

   return blurcmd;
}

int
evas_filter_command_curve_add(Evas_Filter_Context *ctx,
                              void *draw_context EINA_UNUSED,
                              int inbuf, int outbuf, DATA8 *curve,
                              Evas_Filter_Channel channel)
{
   Evas_Filter_Command *cmd;
   Evas_Filter_Buffer *in, *out;
   DATA8 *copy;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(curve, -1);

   in = _filter_buffer_get(ctx, inbuf);
   out = _filter_buffer_get(ctx, outbuf);
   if (!in || !out)
     {
        ERR("Invalid buffer id: input %d [%p], output %d [%p]",
            inbuf, in, outbuf, out);
        return -1;
     }

   if (in->alpha_only != out->alpha_only)
     {
        ERR("Incompatible formats for color curves");
        return -1;
     }

   copy = malloc(256 * sizeof(DATA8));
   if (!copy) return -1;

   cmd = _command_new(ctx, EVAS_FILTER_MODE_CURVE, in, NULL, out);
   if (!cmd)
     {
        free(copy);
        return -1;
     }

   memcpy(copy, curve, 256 * sizeof(DATA8));
   cmd->curve.data = copy;
   cmd->curve.channel = channel;

   return cmd->id;
}

int
evas_filter_command_displacement_map_add(Evas_Filter_Context *ctx,
                                         void *draw_context EINA_UNUSED,
                                         int inbuf, int outbuf, int dispbuf,
                                         Evas_Filter_Displacement_Flags flags,
                                         int intensity,
                                         Evas_Filter_Fill_Mode fillmode)
{
   Evas_Filter_Command *cmd;
   Evas_Filter_Buffer *in, *out, *map, *tmp = NULL, *disp_out;
   int cmdid = -1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, -1);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(intensity >= 0, EINA_FALSE);

   in = _filter_buffer_get(ctx, inbuf);
   out = _filter_buffer_get(ctx, outbuf);
   map = _filter_buffer_get(ctx, dispbuf);
   if (!in || !out || !map)
     {
        ERR("Invalid buffer id: input %d [%p], output %d [%p], map %d [%p]",
            inbuf, in, outbuf, out, dispbuf, map);
        return -1;
     }

   if (in->alpha_only != out->alpha_only)
     {
        ERR("Incompatible formats for displacement map");
        return -1;
     }

   if (in == out)
     {
        tmp = evas_filter_temporary_buffer_get(ctx, in->w, in->h, in->alpha_only);
        if (!tmp) return -1;
        disp_out = tmp;
     }
   else disp_out = out;

   cmd = _command_new(ctx, EVAS_FILTER_MODE_DISPLACE, in, map, disp_out);
   if (!cmd) goto end;

   DRAW_FILL_SET(fillmode);
   cmd->displacement.flags = flags & EVAS_FILTER_DISPLACE_BITMASK;
   cmd->displacement.intensity = intensity;
   cmd->draw.render_op = ENFN->context_render_op_get(ENDT, draw_context);
   cmdid = cmd->id;

   if (tmp)
     {
        if (evas_filter_command_blend_add(ctx, draw_context, disp_out->id,
                                          out->id, 0, 0,
                                          EVAS_FILTER_FILL_MODE_NONE) < 0)
          {
             _command_del(ctx, _evas_filter_command_get(ctx, cmdid));
             cmdid = -1;
          }
     }

end:
   _filter_buffer_unlock_all(ctx);
   return cmdid;
}

int
evas_filter_command_mask_add(Evas_Filter_Context *ctx, void *draw_context,
                             int inbuf, int maskbuf, int outbuf,
                             Evas_Filter_Fill_Mode fillmode)
{
   Evas_Filter_Command *cmd;
   Evas_Filter_Buffer *in, *out, *mask;
   int cmdid = -1, render_op;
   int R, G, B, A;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, -1);

   render_op = ENFN->context_render_op_get(ENDT, draw_context);
   ENFN->context_color_get(ENDT, draw_context, &R, &G, &B, &A);

   in = _filter_buffer_get(ctx, inbuf);
   out = _filter_buffer_get(ctx, outbuf);
   mask = _filter_buffer_get(ctx, maskbuf);
   if (!in || !out || !mask)
     {
        ERR("Invalid buffer id: input %d [%p], output %d [%p], mask %d [%p]",
            inbuf, in, outbuf, out, maskbuf, mask);
        return -1;
     }

   cmd = _command_new(ctx, EVAS_FILTER_MODE_MASK, in, mask, out);
   if (!cmd) goto end;

   cmd->draw.render_op = render_op;
   DRAW_COLOR_SET(R, G, B, A);
   DRAW_FILL_SET(fillmode);

   cmdid = cmd->id;

end:
   _filter_buffer_unlock_all(ctx);
   return cmdid;
}

int
evas_filter_command_bump_map_add(Evas_Filter_Context *ctx,
                                 void *draw_context EINA_UNUSED,
                                 int inbuf, int bumpbuf, int outbuf,
                                 float xyangle, float zangle, float elevation,
                                 float sf,
                                 DATA32 black, DATA32 color, DATA32 white,
                                 Evas_Filter_Bump_Flags flags,
                                 Evas_Filter_Fill_Mode fillmode)
{
   Evas_Filter_Command *cmd;
   Evas_Filter_Buffer *in, *out, *bumpmap;
   int cmdid = -1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, -1);

   in = _filter_buffer_get(ctx, inbuf);
   out = _filter_buffer_get(ctx, outbuf);
   bumpmap = _filter_buffer_get(ctx, bumpbuf);
   if (!in || !out || !bumpmap)
     {
        ERR("Invalid buffer id: input %d [%p], output %d [%p], bumpmap %d [%p]",
            inbuf, in, outbuf, out, bumpbuf, bumpmap);
        return -1;
     }

   // FIXME: Must ensure in != out
   if (in == out) CRI("Not acceptable");
   if (bumpmap == out) CRI("Not acceptable");

   cmd = _command_new(ctx, EVAS_FILTER_MODE_BUMP, in, bumpmap, out);
   if (!cmd) goto end;

   DRAW_FILL_SET(fillmode);
   cmd->bump.xyangle = xyangle;
   cmd->bump.zangle = zangle;
   cmd->bump.specular_factor = sf;
   cmd->bump.dark = black;
   cmd->bump.color = color;
   cmd->bump.white = white;
   cmd->bump.elevation = elevation;
   cmd->bump.compensate = !!(flags & EVAS_FILTER_BUMP_COMPENSATE);
   cmdid = cmd->id;

end:
   _filter_buffer_unlock_all(ctx);
   return cmdid;
}

int
evas_filter_command_transform_add(Evas_Filter_Context *ctx,
                                  void *draw_context EINA_UNUSED,
                                  int inbuf, int outbuf,
                                  Evas_Filter_Transform_Flags flags,
                                  int ox, int oy)
{
   Evas_Filter_Command *cmd;
   Evas_Filter_Buffer *in, *out;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, -1);

   in = _filter_buffer_get(ctx, inbuf);
   out = _filter_buffer_get(ctx, outbuf);
   if (!in || !out)
     {
        ERR("Invalid buffer id: input %d [%p], output %d [%p]",
            inbuf, in, outbuf, out);
        return -1;
     }

   if (in->alpha_only != out->alpha_only)
     {
        CRI("Incompatible buffer formats");
        return -1;
     }

   cmd = _command_new(ctx, EVAS_FILTER_MODE_TRANSFORM, in, NULL, out);
   if (!cmd) return -1;

   cmd->transform.flags = flags;
   cmd->draw.ox = ox;
   cmd->draw.oy = oy;

   return cmd->id;
}

static Eina_Bool
_fill_cpu(Evas_Filter_Command *cmd)
{
   Evas_Filter_Buffer *fb = cmd->output;
   int step = fb->alpha_only ? sizeof(DATA8) : sizeof(DATA32);
   int x = MAX(0, cmd->draw.clip.x);
   int y = MAX(0, cmd->draw.clip.y);
   DATA8 *ptr = ((RGBA_Image *) fb->backing)->mask.data;
   int w, h, k, j;

   if (!cmd->draw.clip_mode_lrtb)
     {
        if (cmd->draw.clip.w)
          w = MIN(cmd->draw.clip.w, fb->w - x);
        else
          w = fb->w - x;
        if (cmd->draw.clip.h)
          h = MIN(cmd->draw.clip.h, fb->h - y);
        else
          h = fb->h - y;
     }
   else
     {
        x = MAX(0, cmd->draw.clip.l);
        y = MAX(0, cmd->draw.clip.t);
        w = CLAMP(0, fb->w - x - cmd->draw.clip.r, fb->w - x);
        h = CLAMP(0, fb->h - y - cmd->draw.clip.b, fb->h - y);
     }

   ptr += y * step * fb->w;
   if ((fb->alpha_only)
       || (!cmd->draw.R && !cmd->draw.G && !cmd->draw.B && !cmd->draw.A)
       || ((cmd->draw.R == 0xff) && (cmd->draw.G == 0xff)
           && (cmd->draw.B == 0xff) && (cmd->draw.A == 0xff)))
     {
        for (k = 0; k < h; k++)
          {
             memset(ptr + (x * step), cmd->draw.A, step * w);
             ptr += step * fb->w;
          }
     }
   else
     {
        DATA32 *dst = ((DATA32 *) ptr) + x;
        DATA32 color = ARGB_JOIN(cmd->draw.A, cmd->draw.R, cmd->draw.G, cmd->draw.B);
        for (k = 0; k < h; k++)
          {
             for (j = 0; j < w; j++)
               *dst++ = color;
             dst += fb->w - w;
          }
     }

   return EINA_TRUE;
}

Evas_Filter_Apply_Func
evas_filter_fill_cpu_func_get(Evas_Filter_Command *cmd)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, NULL);
   return _fill_cpu;
}


/* Final target */
Eina_Bool
evas_filter_target_set(Evas_Filter_Context *ctx, void *draw_context,
                       void *surface, int x, int y)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, EINA_FALSE);

   if (!ctx->gl_engine)
     {
        ctx->target.bufid = evas_filter_buffer_image_new(ctx, surface);
        evas_filter_command_blend_add(ctx, draw_context,
                                      EVAS_FILTER_BUFFER_OUTPUT_ID,
                                      ctx->target.bufid,
                                      x, y, EVAS_FILTER_FILL_MODE_NONE);
     }
   else
     {
        // Since GL has sync rendering, draw_context is safe to keep around
        Evas_Filter_Buffer *target, *image;
        RGBA_Image *im;

        ctx->target.bufid = evas_filter_buffer_image_new(ctx, surface);
        ctx->target.context = draw_context;
        ctx->target.x = x;
        ctx->target.y = y;

        target = _filter_buffer_get(ctx, ctx->target.bufid);
        target->glimage = target->backing;
        target->backing = NULL;

        image = _filter_buffer_get(ctx, EVAS_FILTER_BUFFER_OUTPUT_ID);
        im = image->backing;
        image->glimage = ENFN->image_new_from_data
          (ENDT, image->w, image->h, im->image.data, EINA_TRUE, im->cache_entry.space);
     }

   return EINA_TRUE;
}

static Eina_Bool
_filter_target_render(Evas_Filter_Context *ctx)
{
   Evas_Filter_Buffer *src, *dst;
   Eina_Bool ok;

   /* FIXME: This is some hackish hook to send the final buffer on the screen
    * Only used for OpenGL now, since evas_filter_target_set() adds a blend
    * command in case of pure software rendering.
    */

   if (!ctx->gl_engine) return EINA_FALSE;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ctx->target.bufid, EINA_FALSE);

   src = _filter_buffer_get(ctx, EVAS_FILTER_BUFFER_OUTPUT_ID);
   if (!src) return EINA_FALSE;

   dst = _filter_buffer_get(ctx, ctx->target.bufid);
   if (!dst) return EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(src->glimage, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(dst->glimage, EINA_FALSE);

   ok = ENFN->image_draw(ENDT, ctx->target.context,
                         dst->glimage, src->glimage,
                         0, 0, src->w, src->h,
                         ctx->target.x, ctx->target.y, src->w, src->h,
                         EINA_TRUE, EINA_FALSE);

   return ok;
}


/* Font drawing stuff */
Eina_Bool
evas_filter_font_draw(Evas_Filter_Context *ctx, void *draw_context, int bufid,
                      Evas_Font_Set *font, int x, int y,
                      Evas_Text_Props *text_props, Eina_Bool do_async)
{
   Eina_Bool async_unref;
   Evas_Filter_Buffer *fb;
   void *surface;

   fb = _filter_buffer_get(ctx, bufid);
   if (!fb) return EINA_FALSE;

   surface = fb->backing;
   if (!surface) return EINA_FALSE;

   if (!ctx->gl_engine)
     {
        // Copied from evas_font_draw_async_check
        async_unref = ENFN->font_draw(ENDT, draw_context, surface,
                                      font, x, y, fb->w, fb->h, fb->w, fb->h,
                                      text_props, do_async);
        if (do_async && async_unref)
          {
             evas_common_font_glyphs_ref(text_props->glyphs);
             evas_unref_queue_glyph_put(ctx->evas, text_props->glyphs);
          }
     }
   else
     {
        // FIXME/GL: Render in software only.
        // Copied from eng_font_draw in the software engine.

        if (do_async) WRN("Async flag is ignored here!");
        evas_common_font_draw_prepare(text_props);
        evas_common_font_draw(surface, draw_context, x, y, text_props->glyphs);
        evas_common_cpu_end_opt();
     }

   return EINA_TRUE;
}


/* Clip full input rect (0, 0, sw, sh) to target (dx, dy, dw, dh)
 * and get source's clipped sx, sy as well as destination x, y, cols and rows */
void
_clip_to_target(int *sx /* OUT */, int *sy /* OUT */, int sw, int sh,
                int ox, int oy, int dw, int dh,
                int *dx /* OUT */, int *dy /* OUT */,
                int *rows /* OUT */, int *cols /* OUT */)
{
   if (ox > 0)
     {
        (*dx) = ox;
        (*cols) = sw;
        if (((*dx) + (*cols)) > (dw))
          (*cols) = dw - (*dx);
     }
   else if (ox < 0)
     {
        (*sx) = (-ox);
        (*cols) = sw - (*sx);
        if ((*cols) > dw) (*cols) = dw;
     }
   else
     {
        (*cols) = sw;
        if ((*cols) > dw) (*cols) = dw;
     }

   if (oy > 0)
     {
        (*dy) = oy;
        (*rows) = sh;
        if (((*dy) + (*rows)) > (dh))
          (*rows) = dh - (*dy);
     }
   else if (oy < 0)
     {
        (*sy) = (-oy);
        (*rows) = sh - (*sy);
        if ((*rows) > dh) (*rows) = dh;
     }
   else
     {
        (*rows) = sh;
        if ((*rows) > dh) (*rows) = dh;
     }
}

static const char *
_filter_name_get(int mode)
{
#define FNAME(a) case EVAS_FILTER_MODE_ ## a: return "EVAS_FILTER_MODE_" #a
   switch (mode)
     {
      FNAME(BLEND);
      FNAME(BLUR);
      FNAME(CURVE);
      FNAME(DISPLACE);
      FNAME(MASK);
      FNAME(BUMP);
      FNAME(FILL);
      default: return "INVALID";
     }
#undef FNAME
}

static Eina_Bool
_filter_command_run(Evas_Filter_Command *cmd)
{
   Evas_Filter_Apply_Func func = NULL;

   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->output, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(cmd->input, EINA_FALSE);

   INF("Running filter command %d (%s): %d [%d] --> %d",
       cmd->id, _filter_name_get(cmd->mode),
       cmd->input->id, cmd->mask ? cmd->mask->id : 0, cmd->output->id);

   if (!cmd->input->w && !cmd->input->h
       && (cmd->mode != EVAS_FILTER_MODE_FILL))
     {
        DBG("Skipping processing of empty input buffer (size 0x0)");
        return EINA_TRUE;
     }

   if ((cmd->output->w <= 0) || (cmd->output->h <= 0))
     {
        ERR("Output size invalid: %dx%d", cmd->output->w, cmd->output->h);
        return EINA_FALSE;
     }

   //func = cmd->ENFN->filter_command_func_get(cmd);
   // FIXME: Must call engine function, not CPU directly.

   switch (cmd->mode)
     {
      case EVAS_FILTER_MODE_BLEND:
        func = evas_filter_blend_cpu_func_get(cmd);
        break;
      case EVAS_FILTER_MODE_BLUR:
        func = evas_filter_blur_cpu_func_get(cmd);
        break;
      case EVAS_FILTER_MODE_CURVE:
        func = evas_filter_curve_cpu_func_get(cmd);
        break;
      case EVAS_FILTER_MODE_DISPLACE:
        func = evas_filter_displace_cpu_func_get(cmd);
        break;
      case EVAS_FILTER_MODE_FILL:
        func = evas_filter_fill_cpu_func_get(cmd);
        break;
      case EVAS_FILTER_MODE_MASK:
        func = evas_filter_mask_cpu_func_get(cmd);
        break;
      case EVAS_FILTER_MODE_BUMP:
        func = evas_filter_bump_map_cpu_func_get(cmd);
        break;
      case EVAS_FILTER_MODE_TRANSFORM:
        func = evas_filter_transform_cpu_func_get(cmd);
        break;
      default:
        CRI("Invalid filter mode.");
        break;
     }

   // END OF FIXME

   if (!func)
     {
        CRI("No function to process this filter!");
        return EINA_FALSE;
     }

   return func(cmd);
}

static Eina_Bool
_filter_chain_run(Evas_Filter_Context *ctx)
{
   Evas_Filter_Command *cmd;
   Eina_Bool ok = EINA_TRUE;

   ctx->running = EINA_TRUE;
   EINA_INLIST_FOREACH(ctx->commands, cmd)
     {
        ok = _filter_command_run(cmd);
        if (!ok)
          {
             ERR("Filter processing failed!");
             goto end;
          }
     }

   ok = _filter_target_render(ctx);

end:
   ctx->running = EINA_FALSE;
   return ok;
}

static void
_filter_thread_run_cb(void *data)
{
   Evas_Filter_Context *ctx = data;
   _filter_chain_run(ctx);
   if (ctx->post_run.cb)
     ctx->post_run.cb(ctx, ctx->post_run.data);
}

Eina_Bool
evas_filter_run(Evas_Filter_Context *ctx)
{
   Eina_Bool ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(ctx, EINA_FALSE);

   if (!ctx->commands)
     return EINA_TRUE;

   if (ctx->gl_engine)
     WRN("EXPERIMENTAL OpenGL support! Might very well crash or not render anything.");

   if (ctx->async)
     {
        evas_thread_cmd_enqueue(_filter_thread_run_cb, ctx);
        return EINA_TRUE;
     }

   ret = _filter_chain_run(ctx);
   if (ctx->post_run.cb)
     ctx->post_run.cb(ctx, ctx->post_run.data);
   return ret;
}
