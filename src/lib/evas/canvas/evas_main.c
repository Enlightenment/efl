#include "evas_common_private.h"
#include "evas_private.h"
//#include "evas_cs.h"
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

#define MY_CLASS EVAS_CLASS

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

   eo_init();

#ifdef BUILD_LOADER_EET
   eet_init();
#endif

   evas_module_init();
   if (!evas_async_events_init())
     goto shutdown_module;
#ifdef EVAS_CSERVE2
   {
      const char *env;
      env = getenv("EVAS_CSERVE2");
      if (env && atoi(env)) evas_cserve2_init();
   }
#endif
   _evas_preload_thread_init();

   evas_thread_init();

   eina_log_timing(_evas_log_dom_global,
		   EINA_LOG_STATE_STOP,
		   EINA_LOG_STATE_INIT);

   return _evas_init_count;

 shutdown_module:
   evas_module_shutdown();
   eina_log_domain_unregister(_evas_log_dom_global);
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
   if (_evas_init_count <= 0)
     {
        EINA_LOG_ERR("Init count not greater than 0 in shutdown.");
        return 0;
     }
   if (--_evas_init_count != 0)
     return _evas_init_count;

   eina_log_timing(_evas_log_dom_global,
		   EINA_LOG_STATE_START,
		   EINA_LOG_STATE_SHUTDOWN);

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
     evas_cserve2_shutdown();
#endif

   evas_font_path_global_clear();
   eina_cow_del(evas_object_proxy_cow);
   eina_cow_del(evas_object_map_cow);
   eina_cow_del(evas_object_state_cow);
   evas_object_state_cow = NULL;
   evas_object_map_cow = NULL;
   evas_object_proxy_cow = NULL;

   eina_cow_del(evas_object_image_pixels_cow);
   eina_cow_del(evas_object_image_load_opts_cow);
   eina_cow_del(evas_object_image_state_cow);
   evas_object_image_pixels_cow = NULL;
   evas_object_image_load_opts_cow = NULL;
   evas_object_image_state_cow = NULL;

   evas_thread_shutdown();
   _evas_preload_thread_shutdown();
   evas_async_events_shutdown();
   evas_font_dir_cache_free();
   evas_common_shutdown();
   evas_module_shutdown();

#ifdef BUILD_LOADER_EET
   eet_shutdown();
#endif
   eo_shutdown();

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
   Evas_Object *eo_obj = eo_add(EVAS_CLASS, NULL);
   return eo_obj;
}

EOLIAN static void
_evas_eo_base_constructor(Eo *eo_obj, Evas_Public_Data *e)
{
   eo_do_super(eo_obj, MY_CLASS, eo_constructor());

   e->evas = eo_obj;
   e->output.render_method = RENDER_METHOD_INVALID;
   e->viewport.w = 1;
   e->viewport.h = 1;
   e->framespace.x = 0;
   e->framespace.y = 0;
   e->framespace.w = 0;
   e->framespace.h = 0;
   e->hinting = EVAS_FONT_HINTING_BYTECODE;
   e->name_hash = eina_hash_string_superfast_new(NULL);
   eina_clist_init(&e->calc_list);
   eina_clist_init(&e->calc_done);

#define EVAS_ARRAY_SET(E, Array) \
   eina_array_step_set(&E->Array, sizeof (E->Array), \
		       ((1024 * sizeof (void*)) - sizeof (E->Array)) / sizeof (void*));

   EVAS_ARRAY_SET(e, delete_objects);
   EVAS_ARRAY_SET(e, active_objects);
   EVAS_ARRAY_SET(e, restack_objects);
   EVAS_ARRAY_SET(e, render_objects);
   EVAS_ARRAY_SET(e, pending_objects);
   EVAS_ARRAY_SET(e, obscuring_objects);
   EVAS_ARRAY_SET(e, temporary_objects);
   EVAS_ARRAY_SET(e, calculate_objects);
   EVAS_ARRAY_SET(e, clip_changes);
   EVAS_ARRAY_SET(e, scie_unref_queue);
   EVAS_ARRAY_SET(e, image_unref_queue);
   EVAS_ARRAY_SET(e, glyph_unref_queue);
   EVAS_ARRAY_SET(e, texts_unref_queue);

#undef EVAS_ARRAY_SET
}

EAPI void
evas_free(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   evas_sync(eo_e);
   eo_unref(eo_e);
}

EOLIAN static void
_evas_eo_base_destructor(Eo *eo_e, Evas_Public_Data *e)
{
   Eina_Rectangle *r;
   Evas_Coord_Touch_Point *touch_point;
   Evas_Layer *lay;
   Evas_Out *evo;
   int i;
   Eina_Bool del;

   if (e->walking_list == 0) evas_render_idle_flush(eo_e);

   if (e->walking_list > 0) return;
   evas_render_idle_flush(eo_e);

   _evas_post_event_callback_free(eo_e);

   del = EINA_TRUE;
   e->walking_list++;
   e->cleanup = 1;
   while (del)
     {
        del = EINA_FALSE;
        EINA_INLIST_FOREACH(e->layers, lay)
          {
             Evas_Object_Protected_Data *o;

             evas_layer_pre_free(lay);

             EINA_INLIST_FOREACH(lay->objects, o)
               {
                  if (!o->delete_me)
                    {
                       if ((o->ref > 0) || (eo_ref_get(o->object) > 0))
                         {
                            ERR("obj(%p, %s) ref count(%d) is bigger than 0. This object couldn't be deleted", o, o->type, eo_ref_get(o->object));
                            continue;
                         }
                       del = EINA_TRUE;
                    }
               }
          }
     }
   EINA_INLIST_FOREACH(e->layers, lay)
     evas_layer_free_objects(lay);
   evas_layer_clean(eo_e);

   e->walking_list--;

   evas_font_path_clear(eo_e);
   e->pointer.object.in = eina_list_free(e->pointer.object.in);

   if (e->name_hash) eina_hash_free(e->name_hash);
   e->name_hash = NULL;

   EINA_LIST_FREE(e->damages, r)
      eina_rectangle_free(r);
   EINA_LIST_FREE(e->obscures, r)
      eina_rectangle_free(r);

   evas_fonts_zero_free(eo_e);

   evas_event_callback_all_del(eo_e);
   evas_event_callback_cleanup(eo_e);

   EINA_LIST_FREE(e->outputs, evo) evas_output_del(evo);

   if (e->engine.func)
     {
        e->engine.func->context_free(e->engine.data.output,
                                     e->engine.data.context);
        e->engine.func->output_free(e->engine.data.output);
        e->engine.func->info_free(eo_e, e->engine.info);
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
   eina_array_flush(&e->scie_unref_queue);
   eina_array_flush(&e->image_unref_queue);
   eina_array_flush(&e->glyph_unref_queue);
   eina_array_flush(&e->texts_unref_queue);

   EINA_LIST_FREE(e->touch_points, touch_point)
     free(touch_point);

   _evas_device_cleanup(eo_e);

   e->magic = 0;
   eo_do_super(eo_e, MY_CLASS, eo_destructor());
}

EOLIAN static void
_evas_output_method_set(Eo *eo_e, Evas_Public_Data *e, int render_method)
{
   Evas_Module *em;

   /* if our engine to set it to is invalid - abort */
   if (render_method == RENDER_METHOD_INVALID) return;
   /* if the engine is already set up - abort */
   if (e->output.render_method != RENDER_METHOD_INVALID) return;
   /* Request the right engine. */
   em = evas_module_engine_get(render_method);
   if (!em) return;
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
   if (e->engine.func->info) e->engine.info = e->engine.func->info(eo_e);
   return;
}

EOLIAN static int
_evas_output_method_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->output.render_method;
}

EOLIAN static Evas_Engine_Info*
_evas_engine_info_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   Evas_Engine_Info *info;

   if (!e->engine.info) return NULL;

   info = e->engine.info;
   ((Evas_Public_Data *)e)->engine.info_magic = info->magic;

   return info;
}

EOLIAN static Eina_Bool
_evas_engine_info_set(Eo *eo_e, Evas_Public_Data *e, Evas_Engine_Info *info)
{
   if (!info) return EINA_FALSE;
   if (info != e->engine.info) return EINA_FALSE;
   if (info->magic != e->engine.info_magic) return EINA_FALSE;

   return (Eina_Bool)e->engine.func->setup(eo_e, info);
}

EOLIAN static void
_evas_output_size_set(Eo *eo_e, Evas_Public_Data *e, int w, int h)
{
   if ((w == e->output.w) && (h == e->output.h)) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;

   e->output.w = w;
   e->output.h = h;
   e->output.changed = 1;
   e->output_validity++;
   e->changed = 1;
   evas_render_invalidate(eo_e);
}

EOLIAN static void
_evas_output_size_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int *w, int *h)
{
   if (w) *w = e->output.w;
   if (h) *h = e->output.h;
}

EOLIAN static void
_evas_output_viewport_set(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
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

EOLIAN static void
_evas_output_viewport_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   if (x) *x = e->viewport.x;
   if (y) *y = e->viewport.y;
   if (w) *w = e->viewport.w;
   if (h) *h = e->viewport.h;
}

EOLIAN static void
_evas_output_framespace_set(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   if ((x == e->framespace.x) && (y == e->framespace.y) &&
       (w == e->framespace.w) && (h == e->framespace.h)) return;
   e->framespace.x = x;
   e->framespace.y = y;
   e->framespace.w = w;
   e->framespace.h = h;
   e->framespace.changed = 1;
   e->output_validity++;
   e->changed = 1;
}

EOLIAN static void
_evas_output_framespace_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   if (x) *x = e->framespace.x;
   if (y) *y = e->framespace.y;
   if (w) *w = e->framespace.w;
   if (h) *h = e->framespace.h;
}

EOLIAN static Evas_Coord
_evas_coord_screen_x_to_world(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int x)
{
   if (e->output.w == e->viewport.w) return e->viewport.x + x;
   else return (long long)e->viewport.x + (((long long)x * (long long)e->viewport.w) / (long long)e->output.w);
}

EOLIAN static Evas_Coord
_evas_coord_screen_y_to_world(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int y)
{
   if (e->output.h == e->viewport.h) return e->viewport.y + y;
   else return (long long)e->viewport.y + (((long long)y * (long long)e->viewport.h) / (long long)e->output.h);
}

EOLIAN static int
_evas_coord_world_x_to_screen(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord x)
{
   if (e->output.w == e->viewport.w) return x - e->viewport.x;
   else return (int)((((long long)x - (long long)e->viewport.x) * (long long)e->output.w) /  (long long)e->viewport.w);
}

EOLIAN static int
_evas_coord_world_y_to_screen(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord y)
{
   if (e->output.h == e->viewport.h) return y - e->viewport.y;
   else return (int)((((long long)y - (long long)e->viewport.y) * (long long)e->output.h) /  (long long)e->viewport.h);
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
   const char *s;
   
   EINA_LIST_FREE(list, s) eina_stringshare_del(s);
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

EOLIAN static void
_evas_pointer_output_xy_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int *x, int *y)
{
   if (x) *x = e->pointer.x;
   if (y) *y = e->pointer.y;
}

EOLIAN static void
_evas_pointer_canvas_xy_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord *x, Evas_Coord *y)
{
   if (x) *x = e->pointer.x;
   if (y) *y = e->pointer.y;
}

EOLIAN static int
_evas_pointer_button_down_mask_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return (int)e->pointer.button;
}

EOLIAN static Eina_Bool
_evas_pointer_inside_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->pointer.inside;
}

EOLIAN static void
_evas_data_attach_set(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, void *data)
{
   e->attach_data = data;
}

EOLIAN static void*
_evas_data_attach_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->attach_data;
}

EOLIAN static void
_evas_focus_in(Eo *eo_e, Evas_Public_Data *e)
{
   if (e->focus) return;
   e->focus = 1;
   evas_event_callback_call(eo_e, EVAS_CALLBACK_CANVAS_FOCUS_IN, NULL);
}

EOLIAN static void
_evas_focus_out(Eo *eo_e, Evas_Public_Data *e)
{
   if (!e->focus) return;
   e->focus = 0;
   evas_event_callback_call(eo_e, EVAS_CALLBACK_CANVAS_FOCUS_OUT, NULL);
}

EOLIAN static Eina_Bool
_evas_focus_state_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->focus;
}

EOLIAN static void
_evas_nochange_push(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   e->nochange++;
}

EOLIAN static void
_evas_nochange_pop(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   e->nochange--;
}

void
_evas_walk(Evas_Public_Data *e)
{
   e->walking_list++;
}

void
_evas_unwalk(Evas_Public_Data *e)
{
   e->walking_list--;
   if ((e->walking_list == 0) && (e->delete_me)) evas_free(e->evas);
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

EOLIAN static Evas *
_evas_evas_common_interface_evas_get(Eo *eo_e, Evas_Public_Data *e EINA_UNUSED)
{
   return (Evas *)eo_e;
}

#include "canvas/evas.eo.c"
