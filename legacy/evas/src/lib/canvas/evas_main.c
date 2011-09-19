#include "evas_common.h"
#include "evas_private.h"
#include "evas_cs.h"

#ifdef LKDEBUG
EAPI Eina_Bool lockdebug = EINA_FALSE;
EAPI int lockmax = 0;
#endif

static int _evas_init_count = 0;
int _evas_log_dom_global = -1;
EAPI int
evas_init(void)
{
   if (++_evas_init_count != 1)
     return _evas_init_count;

#ifdef LKDEBUG
   if (getenv("EVAS_LOCK_DEBUG"))
      {
         lockdebug = EINA_TRUE;
         lockmax = atoi(getenv("EVAS_LOCK_DEBUG"));
      }
#endif
   
#ifdef HAVE_EVIL
   if (!evil_init())
     return --_evas_init_count;
#endif

   if (!eina_init())
     goto shutdown_evil;

   _evas_log_dom_global = eina_log_domain_register
     ("evas_main", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_log_dom_global < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        goto shutdown_eina;
     }

   evas_module_init();
#ifdef BUILD_ASYNC_EVENTS
   if (!evas_async_events_init())
     goto shutdown_module;
#endif
#ifdef EVAS_CSERVE
   if (getenv("EVAS_CSERVE")) evas_cserve_init();
#endif
#ifdef BUILD_ASYNC_PRELOAD
   _evas_preload_thread_init();
#endif
#ifdef EVAS_FRAME_QUEUING
   evas_common_frameq_init();
#endif

   return _evas_init_count;

#ifdef BUILD_ASYNC_EVENTS
 shutdown_module:
   evas_module_shutdown();
   eina_log_domain_unregister(_evas_log_dom_global);
#endif
 shutdown_eina:
   eina_shutdown();
 shutdown_evil:
#ifdef HAVE_EVIL
   evil_shutdown();
#endif

   return --_evas_init_count;
}

EAPI int
evas_shutdown(void)
{
   if (--_evas_init_count != 0)
     return _evas_init_count;

#ifdef EVAS_FRAME_QUEUING
   if (evas_common_frameq_enabled())
     {
        evas_common_frameq_finish();
        evas_common_frameq_destroy();
     }
#endif
#ifdef BUILD_ASYNC_EVENTS
   _evas_preload_thread_shutdown();
#endif
#ifdef EVAS_CSERVE
   if (getenv("EVAS_CSERVE")) evas_cserve_shutdown();
#endif
#ifdef BUILD_ASYNC_EVENTS
   evas_async_events_shutdown();
#endif
   evas_font_dir_cache_free();
   evas_common_shutdown();
   evas_module_shutdown();
   eina_log_domain_unregister(_evas_log_dom_global);
   eina_shutdown();
#ifdef HAVE_EVIL
   evil_shutdown();
#endif

   return _evas_init_count;
}


EAPI Evas *
evas_new(void)
{
   Evas *e;

   e = calloc(1, sizeof(Evas));
   if (!e) return NULL;

   e->magic = MAGIC_EVAS;
   e->output.render_method = RENDER_METHOD_INVALID;
   e->viewport.w = 1;
   e->viewport.h = 1;
   e->hinting = EVAS_FONT_HINTING_BYTECODE;
   e->name_hash = eina_hash_string_superfast_new(NULL);

#define EVAS_ARRAY_SET(E, Array)		\
   eina_array_step_set(&E->Array, sizeof (E->Array), 4096);

   EVAS_ARRAY_SET(e, delete_objects);
   EVAS_ARRAY_SET(e, active_objects);
   EVAS_ARRAY_SET(e, restack_objects);
   EVAS_ARRAY_SET(e, render_objects);
   EVAS_ARRAY_SET(e, pending_objects);
   EVAS_ARRAY_SET(e, obscuring_objects);
   EVAS_ARRAY_SET(e, temporary_objects);
   EVAS_ARRAY_SET(e, calculate_objects);
   EVAS_ARRAY_SET(e, clip_changes);

#undef EVAS_ARRAY_SET

   return e;
}

EAPI void
evas_free(Evas *e)
{
   Eina_Rectangle *r;
   Evas_Layer *lay;
   int i;
   int del;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

#ifdef EVAS_FRAME_QUEUING
   evas_common_frameq_flush();
#endif

   if (e->walking_list == 0) evas_render_idle_flush(e);
   
   if (e->walking_list > 0) return;

   if (e->callbacks)
     {
	if (e->callbacks->deletions_waiting) return;

	e->callbacks->deletions_waiting = 0;
	evas_event_callback_list_post_free(&e->callbacks->callbacks);
	if (!e->callbacks->callbacks)
	  {
	     free(e->callbacks);
	     e->callbacks = NULL;
	  }

	_evas_post_event_callback_free(e);
     }
   
   del = 1;
   e->walking_list++;
   e->cleanup = 1;
   while (del)
     {
	del = 0;
	EINA_INLIST_FOREACH(e->layers, lay)
	  {
	     Evas_Object *o;

	     evas_layer_pre_free(lay);

	     EINA_INLIST_FOREACH(lay->objects, o)
	       {
		  if ((o->callbacks) && (o->callbacks->walking_list))
		    {
		       /* Defer free */
		       e->delete_me = 1;
		       e->walking_list--;
		       return;
		    }
		  if (!o->delete_me)
		    del = 1;
	       }
	  }
     }
   EINA_INLIST_FOREACH(e->layers, lay)
     evas_layer_free_objects(lay);
   evas_layer_clean(e);

   e->walking_list--;

   evas_font_path_clear(e);
   e->pointer.object.in = eina_list_free(e->pointer.object.in);

   if (e->name_hash) eina_hash_free(e->name_hash);
   e->name_hash = NULL;

   EINA_LIST_FREE(e->damages, r)
     eina_rectangle_free(r);
   EINA_LIST_FREE(e->obscures, r)
     eina_rectangle_free(r);

   evas_fonts_zero_free(e);
   
   evas_event_callback_all_del(e);
   evas_event_callback_cleanup(e);

   if (e->engine.func)
     {
	e->engine.func->context_free(e->engine.data.output, e->engine.data.context);
	e->engine.func->output_free(e->engine.data.output);
	e->engine.func->info_free(e, e->engine.info);
     }

   for (i = 0; i < e->modifiers.mod.count; i++)
     free(e->modifiers.mod.list[i]);
   if (e->modifiers.mod.list) free(e->modifiers.mod.list);

   for (i = 0; i < e->locks.lock.count; i++)
     free(e->locks.lock.list[i]);
   if (e->locks.lock.list) free(e->locks.lock.list);

   if (e->engine.module) evas_module_unref(e->engine.module);

   eina_array_flush(&e->delete_objects);
   eina_array_flush(&e->active_objects);
   eina_array_flush(&e->restack_objects);
   eina_array_flush(&e->render_objects);
   eina_array_flush(&e->pending_objects);
   eina_array_flush(&e->obscuring_objects);
   eina_array_flush(&e->temporary_objects);
   eina_array_flush(&e->calculate_objects);
   eina_array_flush(&e->clip_changes);

   eina_list_free(e->calc_list);
   
   e->magic = 0;
   free(e);
}

EAPI void
evas_output_method_set(Evas *e, int render_method)
{
   Evas_Module *em;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   /* if our engine to set it to is invalid - abort */
   if (render_method == RENDER_METHOD_INVALID) return;
   /* if the engine is already set up - abort */
   if (e->output.render_method != RENDER_METHOD_INVALID) return;
   /* Request the right engine. */
   em = evas_module_engine_get(render_method);
   if (!em) return ;
   if (em->id_engine != render_method) return;
   if (!evas_module_load(em)) return;

   /* set the correct render */
   e->output.render_method = render_method;
   e->engine.func = (em->functions);
   evas_module_use(em);
   if (e->engine.module) evas_module_unref(e->engine.module);
   e->engine.module = em;
   evas_module_ref(em);
   /* get the engine info struct */
   if (e->engine.func->info) e->engine.info = e->engine.func->info(e);
   return;
}

EAPI int
evas_output_method_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return RENDER_METHOD_INVALID;
   MAGIC_CHECK_END();

   return e->output.render_method;
}

EAPI Evas_Engine_Info *
evas_engine_info_get(const Evas *e)
{
   Evas_Engine_Info *info;

   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();

   if (!e->engine.info) return NULL;

   info = e->engine.info;
   ((Evas *)e)->engine.info_magic = info->magic;

   return info;
}

EAPI Eina_Bool
evas_engine_info_set(Evas *e, Evas_Engine_Info *info)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   if (!info) return EINA_FALSE;
   if (info != e->engine.info) return EINA_FALSE;
   if (info->magic != e->engine.info_magic) return EINA_FALSE;
   return (Eina_Bool)e->engine.func->setup(e, info);
}

EAPI void
evas_output_size_set(Evas *e, int w, int h)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if ((w == e->output.w) && (h == e->output.h)) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;

#ifdef EVAS_FRAME_QUEUING
   evas_common_frameq_flush();
#endif

   e->output.w = w;
   e->output.h = h;
   e->output.changed = 1;
   e->output_validity++;
   e->changed = 1;
   evas_render_invalidate(e);
}

EAPI void
evas_output_size_get(const Evas *e, int *w, int *h)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();

   if (w) *w = e->output.w;
   if (h) *h = e->output.h;
}

EAPI void
evas_output_viewport_set(Evas *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   if ((x == e->viewport.x) && (y == e->viewport.y) &&
       (w == e->viewport.w) && (h == e->viewport.h)) return;
   if (w <= 0) return;
   if (h <= 0) return;
   if ((x != 0) || (y != 0))
     {
	ERR("Compat error. viewport x,y != 0,0 not supported");
	x = 0;
	y = 0;
     }
   e->viewport.x = x;
   e->viewport.y = y;
   e->viewport.w = w;
   e->viewport.h = h;
   e->viewport.changed = 1;
   e->output_validity++;
   e->changed = 1;
}

EAPI void
evas_output_viewport_get(const Evas *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();

   if (x) *x = e->viewport.x;
   if (y) *y = e->viewport.y;
   if (w) *w = e->viewport.w;
   if (h) *h = e->viewport.h;
}

EAPI Evas_Coord
evas_coord_screen_x_to_world(const Evas *e, int x)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (e->output.w == e->viewport.w) return e->viewport.x + x;
   return (long long)e->viewport.x + (((long long)x * (long long)e->viewport.w) / (long long)e->output.w);
}

EAPI Evas_Coord
evas_coord_screen_y_to_world(const Evas *e, int y)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (e->output.h == e->viewport.h) return e->viewport.y + y;
   return (long long)e->viewport.y + (((long long)y * (long long)e->viewport.h) / (long long)e->output.h);
}

EAPI int
evas_coord_world_x_to_screen(const Evas *e, Evas_Coord x)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (e->output.w == e->viewport.w) return x - e->viewport.x;
   return (int)((((long long)x - (long long)e->viewport.x) * (long long)e->output.w) /  (long long)e->viewport.w);
}

EAPI int
evas_coord_world_y_to_screen(const Evas *e, Evas_Coord y)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (e->output.h == e->viewport.h) return y - e->viewport.y;
   return (int)((((long long)y - (long long)e->viewport.y) * (long long)e->output.h) /  (long long)e->viewport.h);
}

EAPI int
evas_render_method_lookup(const char *name)
{
   Evas_Module *em;

   if (!name) return RENDER_METHOD_INVALID;
   /* search on the engines list for the name */
   em = evas_module_find_type(EVAS_MODULE_TYPE_ENGINE, name);
   if (!em) return RENDER_METHOD_INVALID;

   return em->id_engine;
}

EAPI Eina_List *
evas_render_method_list(void)
{
   return evas_module_engine_list();
}

EAPI void
evas_render_method_list_free(Eina_List *list)
{
   eina_list_free(list);
}

EAPI Eina_Bool
evas_object_image_extension_can_load_get(const char *file)
{
   const char *tmp;
   Eina_Bool result;

   tmp = eina_stringshare_add(file);
   result = evas_common_extension_can_load_get(tmp);
   eina_stringshare_del(tmp);

   return result;
}

EAPI Eina_Bool
evas_object_image_extension_can_load_fast_get(const char *file)
{
   return evas_common_extension_can_load_get(file);
}

EAPI void
evas_pointer_output_xy_get(const Evas *e, int *x, int *y)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   return;
   MAGIC_CHECK_END();
   if (x) *x = e->pointer.x;
   if (y) *y = e->pointer.y;
}

EAPI void
evas_pointer_canvas_xy_get(const Evas *e, Evas_Coord *x, Evas_Coord *y)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   return;
   MAGIC_CHECK_END();
   if (x) *x = e->pointer.x;
   if (y) *y = e->pointer.y;
}

EAPI int
evas_pointer_button_down_mask_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return (int)e->pointer.button;
}

EAPI Eina_Bool
evas_pointer_inside_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return (int)e->pointer.inside;
}

EAPI void
evas_data_attach_set(Evas *e, void *data)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   e->attach_data = data;
}

EAPI void *
evas_data_attach_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return e->attach_data;
}

EAPI void
evas_focus_in(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (e->focus) return;
   e->focus = 1;
   evas_event_callback_call(e, EVAS_CALLBACK_CANVAS_FOCUS_IN, NULL);
}

EAPI void
evas_focus_out(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!e->focus) return;
   e->focus = 0;
   evas_event_callback_call(e, EVAS_CALLBACK_CANVAS_FOCUS_OUT, NULL);
}

EAPI Eina_Bool
evas_focus_state_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   return e->focus;
}

EAPI void
evas_nochange_push(Evas *e)
{
   e->nochange++;
}

EAPI void
evas_nochange_pop(Evas *e)
{
   e->nochange--;
}

void
_evas_walk(Evas *e)
{
   e->walking_list++;
}

void
_evas_unwalk(Evas *e)
{
   e->walking_list--;
   if ((e->walking_list == 0) && (e->delete_me)) evas_free(e);
}

EAPI const char *
evas_load_error_str(Evas_Load_Error error)
{
   switch (error)
     {
      case EVAS_LOAD_ERROR_NONE:
	 return "No error on load";
      case EVAS_LOAD_ERROR_GENERIC:
	 return "A non-specific error occurred";
      case EVAS_LOAD_ERROR_DOES_NOT_EXIST:
	 return "File (or file path) does not exist";
      case EVAS_LOAD_ERROR_PERMISSION_DENIED:
	 return "Permission deinied to an existing file (or path)";
      case EVAS_LOAD_ERROR_RESOURCE_ALLOCATION_FAILED:
	 return "Allocation of resources failure prevented load";
      case EVAS_LOAD_ERROR_CORRUPT_FILE:
	 return "File corrupt (but was detected as a known format)";
      case EVAS_LOAD_ERROR_UNKNOWN_FORMAT:
	 return "File is not a known format";
      default:
	 return "Unknown error";
     }
}

EAPI void
evas_color_hsv_to_rgb(float h, float s, float v, int *r, int *g, int *b)
{
   evas_common_convert_color_hsv_to_rgb(h, s, v, r, g, b);
}

EAPI void
evas_color_rgb_to_hsv(int r, int g, int b, float *h, float *s, float *v)
{
   evas_common_convert_color_rgb_to_hsv(r, g, b, h, s, v);
}

EAPI void
evas_color_argb_premul(int a, int *r, int *g, int *b)
{
   evas_common_convert_color_argb_premul(a, r, g, b);
}

EAPI void
evas_color_argb_unpremul(int a, int *r, int *g, int *b)
{
   evas_common_convert_color_argb_unpremul(a, r, g, b);
}

EAPI void
evas_data_argb_premul(unsigned int *data, unsigned int len)
{
   if (!data || (len < 1)) return;
   evas_common_convert_argb_premul(data, len);
}

EAPI void
evas_data_argb_unpremul(unsigned int *data, unsigned int len)
{
   if (!data || (len < 1)) return;
   evas_common_convert_argb_unpremul(data, len);
}
