#include "evas_common_private.h"
#include "evas_private.h"
#include "evas_engine.h"
#include "Evas_Engine_Buffer.h"

/* domain for eina_log */
/* the log macros are defined in evas_common_private.h */
/* theirs names are EVAS_ERR, EVAS_DBG, EVAS_CRIT, EVAS_WRN and EVAS_INF */
/* although we can use the EVAS_ERROR, etc... macros it will not work
   when the -fvisibility=hidden option is passed to gcc */

int _evas_engine_buffer_log_dom = -1;

/* function tables - filled in later (func and parent func) */

static Evas_Func func, pfunc;


/* engine struct data */
typedef Render_Output_Software_Generic Render_Engine;

/* prototypes we will use here */
static void *_output_setup(int w, int h, void *dest_buffer, int dest_buffer_row_bytes, int depth_type, int use_color_key, int alpha_threshold, int color_key_r, int color_key_g, int color_key_b, void *(*new_update_region) (int x, int y, int w, int h, int *row_bytes), void (*free_update_region) (int x, int y, int w, int h, void *data), void *(*switch_buffer) (void *data, void *dest_buffer), void *switch_data);

static void eng_output_free(void *engine EINA_UNUSED, void *data);

/* internal engine routines */
static void *
_output_setup(int w,
	      int h,
	      void *dest_buffer,
	      int dest_buffer_row_bytes,
	      int depth_type,
	      int use_color_key,
	      int alpha_threshold,
	      int color_key_r,
	      int color_key_g,
	      int color_key_b,
	      void *(*new_update_region) (int x, int y, int w, int h, int *row_bytes),
	      void (*free_update_region) (int x, int y, int w, int h, void *data),
              void *(*switch_buffer) (void *data, void *dest_buffer),
              void *switch_data
	      )
{
   Outbuf *ob;
   Render_Engine *re;
   Outbuf_Depth dep;
   DATA32 color_key = 0;

   re = calloc(1, sizeof(Render_Engine));
   if (!re) return NULL;

   evas_buffer_outbuf_buf_init();

   dep = OUTBUF_DEPTH_BGR_24BPP_888_888;
   if      (depth_type == EVAS_ENGINE_BUFFER_DEPTH_ARGB32)
     dep = OUTBUF_DEPTH_ARGB_32BPP_8888_8888;
   else if (depth_type == EVAS_ENGINE_BUFFER_DEPTH_RGB32)
     dep = OUTBUF_DEPTH_RGB_32BPP_888_8888;
   else if (depth_type == EVAS_ENGINE_BUFFER_DEPTH_BGRA32)
     dep = OUTBUF_DEPTH_BGRA_32BPP_8888_8888;
   else if (depth_type == EVAS_ENGINE_BUFFER_DEPTH_RGB24)
     dep = OUTBUF_DEPTH_RGB_24BPP_888_888;
   else if (depth_type == EVAS_ENGINE_BUFFER_DEPTH_BGR24)
     dep = OUTBUF_DEPTH_BGR_24BPP_888_888;
   R_VAL(&color_key) = color_key_r;
   G_VAL(&color_key) = color_key_g;
   B_VAL(&color_key) = color_key_b;
   A_VAL(&color_key) = 0;
   ob = evas_buffer_outbuf_buf_setup_fb(w,
                                        h,
                                        dep,
                                        dest_buffer,
                                        dest_buffer_row_bytes,
                                        use_color_key,
                                        color_key,
                                        alpha_threshold,
                                        new_update_region,
                                        free_update_region,
                                        switch_buffer,
                                        switch_data);
   if (!ob) goto on_error;

   if (!evas_render_engine_software_generic_init(re, ob,
                                                 evas_buffer_outbuf_buf_swap_mode_get,
                                                 evas_buffer_outbuf_buf_rot_get,
                                                 evas_buffer_outbuf_reconfigure,
                                                 NULL,
                                                 NULL,
                                                 evas_buffer_outbuf_buf_new_region_for_update,
                                                 evas_buffer_outbuf_buf_push_updated_region,
                                                 evas_buffer_outbuf_buf_free_region_for_update,
                                                 NULL,
                                                 evas_buffer_outbuf_buf_switch_buffer,
                                                 NULL,
                                                 evas_buffer_outbuf_buf_free,
                                                 w, h))
     goto on_error;
   return re;

 on_error:
   if (ob) evas_buffer_outbuf_buf_free(ob);
   free(re);
   return NULL;
}

/* engine api this module provides */
static void
eng_output_info_setup(void *info)
{
   Evas_Engine_Info_Buffer *einfo = info;

   einfo->render_mode = EVAS_RENDER_MODE_BLOCKING;
}

static void *
eng_output_setup(void *engine EINA_UNUSED, void *in, unsigned int w, unsigned int h)
{
   Evas_Engine_Info_Buffer *info = in;

   return _output_setup(w,
                        h,
                        info->info.dest_buffer,
                        info->info.dest_buffer_row_bytes,
                        info->info.depth_type,
                        info->info.use_color_key,
                        info->info.alpha_threshold,
                        info->info.color_key_r,
                        info->info.color_key_g,
                        info->info.color_key_b,
                        info->info.func.new_update_region,
                        info->info.func.free_update_region,
                        info->info.func.switch_buffer,
                        info->info.switch_data);
}

static void
eng_output_free(void *engine EINA_UNUSED, void *data)
{
   Render_Engine *re;

   if ((re = (Render_Engine *)data))
     {
        evas_render_engine_software_generic_clean(re);
        free(re);
     }
}

static Eina_Bool
eng_canvas_alpha_get(void *data)
{
   Render_Engine *re;

   if ((re = (Render_Engine *)data))
     if (re->ob->priv.back_buf)
       return re->ob->priv.back_buf->cache_entry.flags.alpha;
   return EINA_TRUE;
}

/* module advertising code */
static int
module_open(Evas_Module *em)
{
   if (!em) return 0;
   /* get whatever engine module we inherit from */
   if (!_evas_module_engine_inherit(&pfunc, "software_generic")) return 0;

   _evas_engine_buffer_log_dom = eina_log_domain_register
     ("evas-buffer", EINA_COLOR_BLUE);
   if (_evas_engine_buffer_log_dom < 0)
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

   func.info_size = sizeof (Evas_Engine_Info_Buffer);

   /* now advertise out own api */
   em->functions = (void *)(&func);
   return 1;
}

static void
module_close(Evas_Module *em EINA_UNUSED)
{
   if (_evas_engine_buffer_log_dom >= 0)
     {
        eina_log_domain_unregister(_evas_engine_buffer_log_dom);
        _evas_engine_buffer_log_dom = -1;
     }
}

static Evas_Module_Api evas_modapi =
{
   EVAS_MODULE_API_VERSION,
   "buffer",
   "none",
   {
     module_open,
     module_close
   }
};

EVAS_MODULE_DEFINE(EVAS_MODULE_TYPE_ENGINE, engine, buffer);

#ifndef EVAS_STATIC_BUILD_BUFFER
EVAS_EINA_MODULE_DEFINE(engine, buffer);
#endif

