#include "evas_common_private.h"
#include "evas_private.h"
//#include "evas_cs.h"
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

EAPI Eo_Op EVAS_CANVAS_BASE_ID = EO_NOOP;

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

static void
_constructor(Eo *eo_obj, void *class_data, va_list *list EINA_UNUSED)
{
   eo_do_super(eo_obj, MY_CLASS, eo_constructor());

   Evas_Public_Data *e = class_data;
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

static void
_destructor(Eo *eo_e, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Public_Data *e = _pd;
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

EAPI void
evas_output_method_set(Evas *eo_e, int render_method)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_output_method_set(render_method));
}

static void
_canvas_output_method_set(Eo *eo_e, void *_pd, va_list *list)
{
   int render_method = va_arg(*list, int);

   Evas_Module *em;
   Evas_Public_Data *e = _pd;

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

EAPI int
evas_output_method_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return RENDER_METHOD_INVALID;
   MAGIC_CHECK_END();
   int ret = RENDER_METHOD_INVALID;
   eo_do((Eo *)eo_e, evas_canvas_output_method_get(&ret));
   return ret;
}

static void
_canvas_output_method_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);
   const Evas_Public_Data *e = _pd;
   *ret = e->output.render_method;
}

EAPI Evas_Engine_Info *
evas_engine_info_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   Evas_Engine_Info *ret = NULL;
   eo_do((Eo *)eo_e, evas_canvas_engine_info_get(&ret));
   return ret;
}

static void
_canvas_engine_info_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Engine_Info **ret = va_arg(*list, Evas_Engine_Info **);

   Evas_Engine_Info *info;

   const Evas_Public_Data *e = _pd;
   if (!e->engine.info)
     {
        *ret = NULL;
        return;
     }

   info = e->engine.info;
   ((Evas_Public_Data *)e)->engine.info_magic = info->magic;

   *ret = info;
}

EAPI Eina_Bool
evas_engine_info_set(Evas *eo_e, Evas_Engine_Info *info)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return EINA_FALSE;
   MAGIC_CHECK_END();
   Eina_Bool ret = EINA_FALSE;
   eo_do(eo_e, evas_canvas_engine_info_set(info, &ret));
   return ret;
}

static void
_canvas_engine_info_set(Eo *eo_e, void *_pd, va_list *list)
{
   Evas_Engine_Info *info = va_arg(*list, Evas_Engine_Info *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;
   Evas_Public_Data *e = _pd;
   if (!info) return;
   if (info != e->engine.info) return;
   if (info->magic != e->engine.info_magic) return;

   Eina_Bool int_ret = (Eina_Bool)e->engine.func->setup(eo_e, info);
   if (ret) *ret = int_ret;
}

EAPI void
evas_output_size_set(Evas *eo_e, int w, int h)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_output_size_set(w, h));
}

static void
_canvas_output_size_set(Eo *eo_e, void *_pd, va_list *list)
{
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);

   Evas_Public_Data *e = _pd;

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

EAPI void
evas_output_size_get(const Evas *eo_e, int *w, int *h)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_e, evas_canvas_output_size_get(w, h));
}

static void
_canvas_output_size_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);

   const Evas_Public_Data *e = _pd;

   if (w) *w = e->output.w;
   if (h) *h = e->output.h;
}

EAPI void
evas_output_viewport_set(Evas *eo_e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_output_viewport_set(x, y, w, h));
}

static void
_canvas_output_viewport_set(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);

   Evas_Public_Data *e = _pd;

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
evas_output_viewport_get(const Evas *eo_e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_e, evas_canvas_output_viewport_get(x, y, w, h));
}

static void
_canvas_output_viewport_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);

   const Evas_Public_Data *e = _pd;

   if (x) *x = e->viewport.x;
   if (y) *y = e->viewport.y;
   if (w) *w = e->viewport.w;
   if (h) *h = e->viewport.h;
}

EAPI void
evas_output_framespace_set(Evas *eo_e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_output_framespace_set(x, y, w, h));
}

static void
_canvas_output_framespace_set(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   Evas_Coord w = va_arg(*list, Evas_Coord);
   Evas_Coord h = va_arg(*list, Evas_Coord);

   Evas_Public_Data *e = _pd;

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

EAPI void
evas_output_framespace_get(const Evas *eo_e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_e, evas_canvas_output_framespace_get(x, y, w, h));
}

static void
_canvas_output_framespace_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);
   Evas_Coord *w = va_arg(*list, Evas_Coord *);
   Evas_Coord *h = va_arg(*list, Evas_Coord *);

   const Evas_Public_Data *e = _pd;

   if (x) *x = e->framespace.x;
   if (y) *y = e->framespace.y;
   if (w) *w = e->framespace.w;
   if (h) *h = e->framespace.h;
}

EAPI Evas_Coord
evas_coord_screen_x_to_world(const Evas *eo_e, int x)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   Evas_Coord ret = 0;
   eo_do((Eo *)eo_e, evas_canvas_coord_screen_x_to_world(x, &ret));
   return ret;
}

static void
_canvas_coord_screen_x_to_world(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   int x = va_arg(*list, int);
   Evas_Coord *ret = va_arg(*list, Evas_Coord *);

   const Evas_Public_Data *e = _pd;
   if (e->output.w == e->viewport.w) *ret = e->viewport.x + x;
   else *ret = (long long)e->viewport.x + (((long long)x * (long long)e->viewport.w) / (long long)e->output.w);
}

EAPI Evas_Coord
evas_coord_screen_y_to_world(const Evas *eo_e, int y)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   Evas_Coord ret = 0;
   eo_do((Eo *)eo_e, evas_canvas_coord_screen_y_to_world(y, &ret));
   return ret;
}

static void
_canvas_coord_screen_y_to_world(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   int y = va_arg(*list, int);
   Evas_Coord *ret = va_arg(*list, Evas_Coord *);

   const Evas_Public_Data *e = _pd;
   if (e->output.h == e->viewport.h) *ret = e->viewport.y + y;
   else *ret = (long long)e->viewport.y + (((long long)y * (long long)e->viewport.h) / (long long)e->output.h);
}

EAPI int
evas_coord_world_x_to_screen(const Evas *eo_e, Evas_Coord x)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   Evas_Coord ret = 0;
   eo_do((Eo *)eo_e, evas_canvas_coord_world_x_to_screen(x, &ret));
   return ret;
}

static void
_canvas_coord_world_x_to_screen(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   int *ret = va_arg(*list, int *);

   const Evas_Public_Data *e = _pd;
   if (e->output.w == e->viewport.w) *ret = x - e->viewport.x;
   else *ret = (int)((((long long)x - (long long)e->viewport.x) * (long long)e->output.w) /  (long long)e->viewport.w);
}

EAPI int
evas_coord_world_y_to_screen(const Evas *eo_e, Evas_Coord y)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   Evas_Coord ret = 0;
   eo_do((Eo *)eo_e, evas_canvas_coord_world_y_to_screen(y, &ret));
   return ret;
}

static void
_canvas_coord_world_y_to_screen(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord y = va_arg(*list, Evas_Coord);
   int *ret = va_arg(*list, int *);

   const Evas_Public_Data *e = _pd;
   if (e->output.h == e->viewport.h) *ret = y - e->viewport.y;
   else *ret = (int)((((long long)y - (long long)e->viewport.y) * (long long)e->output.h) /  (long long)e->viewport.h);
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

EAPI void
evas_pointer_output_xy_get(const Evas *eo_e, int *x, int *y)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_e, evas_canvas_pointer_output_xy_get(x, y));
}

static void
_canvas_pointer_output_xy_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   int *x = va_arg(*list, int *);
   int *y = va_arg(*list, int *);
   const Evas_Public_Data *e = _pd;
   if (x) *x = e->pointer.x;
   if (y) *y = e->pointer.y;
}

EAPI void
evas_pointer_canvas_xy_get(const Evas *eo_e, Evas_Coord *x, Evas_Coord *y)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   if (x) *x = 0;
   if (y) *y = 0;
   return;
   MAGIC_CHECK_END();
   eo_do((Eo *)eo_e, evas_canvas_pointer_canvas_xy_get(x, y));
}

static void
_canvas_pointer_canvas_xy_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Evas_Coord *x = va_arg(*list, Evas_Coord *);
   Evas_Coord *y = va_arg(*list, Evas_Coord *);

   const Evas_Public_Data *e = _pd;
   if (x) *x = e->pointer.x;
   if (y) *y = e->pointer.y;
}

EAPI int
evas_pointer_button_down_mask_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   int ret = 0;
   eo_do((Eo *)eo_e, evas_canvas_pointer_button_down_mask_get(&ret));
   return ret;
}

static void
_canvas_pointer_button_down_mask_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   int *ret = va_arg(*list, int *);

   const Evas_Public_Data *e = _pd;
   *ret = (int)e->pointer.button;
}

EAPI Eina_Bool
evas_pointer_inside_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)eo_e, evas_canvas_pointer_inside_get(&ret));
   return ret;
}

static void
_canvas_pointer_inside_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   const Evas_Public_Data *e = _pd;
   *ret = e->pointer.inside;
}

EAPI void
evas_data_attach_set(Evas *eo_e, void *data)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_data_attach_set(data));
}

static void
_canvas_data_attach_set(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   void *data = va_arg(*list, void *);
   Evas_Public_Data *e = _pd;
   e->attach_data = data;
}

EAPI void *
evas_data_attach_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   void *ret = NULL;
   eo_do((Eo *)eo_e, evas_canvas_data_attach_get(&ret));
   return ret;
}

static void
_canvas_data_attach_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   void **ret = va_arg(*list, void **);
   const Evas_Public_Data *e = _pd;
   *ret = e->attach_data;
}

EAPI void
evas_focus_in(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_focus_in());
}

static void
_canvas_focus_in(Eo *eo_e, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Public_Data *e = _pd;
   if (e->focus) return;
   e->focus = 1;
   evas_event_callback_call(eo_e, EVAS_CALLBACK_CANVAS_FOCUS_IN, NULL);
}

EAPI void
evas_focus_out(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_focus_out());
}

static void
_canvas_focus_out(Eo *eo_e, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Public_Data *e = _pd;
   if (!e->focus) return;
   e->focus = 0;
   evas_event_callback_call(eo_e, EVAS_CALLBACK_CANVAS_FOCUS_OUT, NULL);
}

EAPI Eina_Bool
evas_focus_state_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   Eina_Bool ret = 0;
   eo_do((Eo *)eo_e, evas_canvas_focus_state_get(&ret));
   return ret;
}

static void
_canvas_focus_state_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   const Evas_Public_Data *e = _pd;
   *ret = e->focus;
}

EAPI void
evas_nochange_push(Evas *eo_e)
{
   eo_do(eo_e, evas_canvas_nochange_push());
}

static void
_canvas_nochange_push(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Public_Data *e = _pd;
   e->nochange++;
}

EAPI void
evas_nochange_pop(Evas *eo_e)
{
   eo_do(eo_e, evas_canvas_nochange_pop());
}

static void
_canvas_nochange_pop(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list EINA_UNUSED)
{
   Evas_Public_Data *e = _pd;
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

static void
_evas_get(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas **evas = va_arg(*list, Evas **);
   if (evas) *evas = (Evas *)eo_obj;
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_DESTRUCTOR), _destructor),
        EO_OP_FUNC(EVAS_COMMON_ID(EVAS_COMMON_SUB_ID_EVAS_GET), _evas_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_SET), _canvas_output_method_set),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_GET), _canvas_output_method_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_ENGINE_INFO_GET), _canvas_engine_info_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_ENGINE_INFO_SET), _canvas_engine_info_set),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_SET), _canvas_output_size_set),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_GET), _canvas_output_size_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_SET), _canvas_output_viewport_set),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_GET), _canvas_output_viewport_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_SET), _canvas_output_framespace_set),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_GET), _canvas_output_framespace_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_SCREEN_X_TO_WORLD), _canvas_coord_screen_x_to_world),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_SCREEN_Y_TO_WORLD), _canvas_coord_screen_y_to_world),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_WORLD_X_TO_SCREEN), _canvas_coord_world_x_to_screen),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_COORD_WORLD_Y_TO_SCREEN), _canvas_coord_world_y_to_screen),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_OUTPUT_XY_GET), _canvas_pointer_output_xy_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_CANVAS_XY_GET), _canvas_pointer_canvas_xy_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_BUTTON_DOWN_MASK_GET), _canvas_pointer_button_down_mask_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_POINTER_INSIDE_GET), _canvas_pointer_inside_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_DATA_ATTACH_SET), _canvas_data_attach_set),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_DATA_ATTACH_GET), _canvas_data_attach_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_IN), _canvas_focus_in),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_OUT), _canvas_focus_out),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_STATE_GET), _canvas_focus_state_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_NOCHANGE_PUSH), _canvas_nochange_push),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_NOCHANGE_POP), _canvas_nochange_pop),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_SET), _canvas_event_default_flags_set),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_GET), _canvas_event_default_flags_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_DOWN), _canvas_event_feed_mouse_down),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_UP), _canvas_event_feed_mouse_up),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_CANCEL), _canvas_event_feed_mouse_cancel),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_WHEEL), _canvas_event_feed_mouse_wheel),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MOUSE_MOVE), _canvas_event_input_mouse_move),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_MOVE), _canvas_event_feed_mouse_move),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_IN), _canvas_event_feed_mouse_in),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_OUT), _canvas_event_feed_mouse_out),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_DOWN), _canvas_event_feed_multi_down),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_UP), _canvas_event_feed_multi_up),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_MOVE), _canvas_event_feed_multi_move),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_DOWN), _canvas_event_input_multi_down),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_UP), _canvas_event_input_multi_up),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_MOVE), _canvas_event_input_multi_move),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_DOWN), _canvas_event_feed_key_down),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_UP), _canvas_event_feed_key_up),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_FEED_HOLD), _canvas_event_feed_hold),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_REFEED_EVENT), _canvas_event_refeed_event),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_EVENT_DOWN_COUNT_GET), _canvas_event_down_count_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FOCUS_GET), _canvas_focus_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_CLEAR), _canvas_font_path_clear),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_APPEND), _canvas_font_path_append),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_PREPEND), _canvas_font_path_prepend),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_PATH_LIST), _canvas_font_path_list),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_HINTING_SET), _canvas_font_hinting_set),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_HINTING_GET), _canvas_font_hinting_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_HINTING_CAN_HINT), _canvas_font_hinting_can_hint),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_CACHE_FLUSH), _canvas_font_cache_flush),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_CACHE_SET), _canvas_font_cache_set),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_CACHE_GET), _canvas_font_cache_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_FONT_AVAILABLE_LIST), _canvas_font_available_list),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_GET), _canvas_key_modifier_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_GET), _canvas_key_lock_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ADD), _canvas_key_modifier_add),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_DEL), _canvas_key_modifier_del),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_ADD), _canvas_key_lock_add),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_DEL), _canvas_key_lock_del),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ON), _canvas_key_modifier_on),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_OFF), _canvas_key_modifier_off),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_ON), _canvas_key_lock_on),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_LOCK_OFF), _canvas_key_lock_off),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_MASK_GET), _canvas_key_modifier_mask_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_DAMAGE_RECTANGLE_ADD), _canvas_damage_rectangle_add),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBSCURED_RECTANGLE_ADD), _canvas_obscured_rectangle_add),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBSCURED_CLEAR), _canvas_obscured_clear),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER_UPDATES), _canvas_render_updates),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER), _canvas_render),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_NORENDER), _canvas_norender),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER_IDLE_FLUSH), _canvas_render_idle_flush),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_SYNC), _canvas_sync),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER_DUMP), _canvas_render_dump),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_BOTTOM_GET), _canvas_object_bottom_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_TOP_GET), _canvas_object_top_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_COUNT), _canvas_touch_point_list_count),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_XY_GET), _canvas_touch_point_list_nth_xy_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_ID_GET), _canvas_touch_point_list_nth_id_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_STATE_GET), _canvas_touch_point_list_nth_state_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_FLUSH), _canvas_image_cache_flush),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_RELOAD), _canvas_image_cache_reload),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_SET), _canvas_image_cache_set),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_GET), _canvas_image_cache_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_IMAGE_MAX_SIZE_GET), _canvas_image_max_size_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_NAME_FIND), _canvas_object_name_find),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_TOP_AT_XY_GET), _canvas_object_top_at_xy_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECT_TOP_IN_RECTANGLE_GET), _canvas_object_top_in_rectangle_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECTS_AT_XY_GET), _canvas_objects_at_xy_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_OBJECTS_IN_RECTANGLE_GET), _canvas_objects_in_rectangle_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE), _canvas_smart_objects_calculate),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE_COUNT_GET), _canvas_smart_objects_calculate_count_get),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_RENDER_ASYNC), _canvas_render_async),
        EO_OP_FUNC(EVAS_CANVAS_ID(EVAS_CANVAS_SUB_ID_TREE_OBJECTS_AT_XY_GET), _canvas_tree_objects_at_xy_get),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_SET, "Sets the output engine for the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OUTPUT_METHOD_GET, "Retrieves the number of the output engine used for the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_ENGINE_INFO_GET, "Retrieves the current render engine info struct from the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_ENGINE_INFO_SET, "Applies the engine settings for the given evas from the given Evas_Engine_Info structure."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_SET, "Sets the output size of the render engine of the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OUTPUT_SIZE_GET, "Retrieve the output size of the render engine of the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_SET, "Sets the output viewport of the given evas in evas units."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OUTPUT_VIEWPORT_GET, "Get the render engine's output viewport co-ordinates in canvas units."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_SET, "Sets the output framespace size of the render engine of the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OUTPUT_FRAMESPACE_GET, "Get the render engine's output framespace co-ordinates in canvas units."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_COORD_SCREEN_X_TO_WORLD, "Convert/scale an output screen co-ordinate into canvas co-ordinates"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_COORD_SCREEN_Y_TO_WORLD, "Convert/scale an output screen co-ordinate into canvas co-ordinates"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_COORD_WORLD_X_TO_SCREEN, "Convert/scale a canvas co-ordinate into output screen co-ordinates"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_COORD_WORLD_Y_TO_SCREEN, "Convert/scale a canvas co-ordinate into output screen co-ordinates"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_POINTER_OUTPUT_XY_GET, "This function returns the current known pointer co-ordinates"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_POINTER_CANVAS_XY_GET, "This function returns the current known pointer co-ordinates"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_POINTER_BUTTON_DOWN_MASK_GET, "Returns a bitmask with the mouse buttons currently pressed."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_POINTER_INSIDE_GET, "Returns whether the mouse pointer is logically inside the canvas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_DATA_ATTACH_SET, "Attaches a specific pointer to the evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_DATA_ATTACH_GET, "Returns the pointer attached by evas_data_attach_set()."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FOCUS_IN, "Inform to the evas that it got the focus."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FOCUS_OUT, "Inform to the evas that it lost the focus."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FOCUS_STATE_GET, "Get the focus state known by the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_NOCHANGE_PUSH, "Push the nochange flag up 1."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_NOCHANGE_POP, "Pop the nochange flag down 1."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_SET, "Set the default set of flags an event begins with."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_DEFAULT_FLAGS_GET, "Get the defaulty set of flags an event begins with."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_DOWN, "Mouse down event feed."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_UP, "Mouse up event feed."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_CANCEL, "Mouse cancel event feed."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_WHEEL, "Mouse wheel event feed."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MOUSE_MOVE, "Mouse move event fed from input (only Ecore_Evas_Input should use it)."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_MOVE, "Mouse move event feed."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_IN, "Mouse in event feed."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_MOUSE_OUT, "Mouse out event feed."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_DOWN, "Multi down event feed."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_UP, "Multi up event feed."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_MULTI_MOVE, "Multi move event feed."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_DOWN, "Multi down event input."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_UP, "Multi up event input."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_INPUT_MULTI_MOVE, "Multi move event input."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_DOWN, "Key down event feed"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_KEY_UP, "Key up event feed"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_FEED_HOLD, "Hold event feed"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_REFEED_EVENT, "Re feed event."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_EVENT_DOWN_COUNT_GET, "Get the number of mouse or multi presses currently active."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FOCUS_GET, "Retrieve the object that currently has focus."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FONT_PATH_CLEAR, "Removes all font paths loaded into memory for the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FONT_PATH_APPEND, "Appends a font path to the list of font paths used by the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FONT_PATH_PREPEND, "Prepends a font path to the list of font paths used by the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FONT_PATH_LIST, "Retrieves the list of font paths used by the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FONT_HINTING_SET, "Changes the font hinting for the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FONT_HINTING_GET, "Retrieves the font hinting used by the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FONT_HINTING_CAN_HINT, "Checks if the font hinting is supported by the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FONT_CACHE_FLUSH, "Force the given evas and associated engine to flush its font cache."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FONT_CACHE_SET, "Changes the size of font cache of the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FONT_CACHE_GET, "Changes the size of font cache of the given evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_FONT_AVAILABLE_LIST, "List of available font descriptions known or found by this evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_GET, "Returns a handle to the list of modifier keys registered."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_KEY_LOCK_GET, "Returns a handle to the list of lock keys registered in the canvas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ADD, "Adds the keyname key to the current list of modifier keys."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_DEL, "Removes the keyname key from the current list of modifier keys."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_KEY_LOCK_ADD, "Adds the keyname key to the current list of lock keys."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_KEY_LOCK_DEL, "Removes the keyname key from the current list of lock keys."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_ON, "Enables or turns on programmatically the modifier key with name keyname."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_OFF, "Disables or turns off programmatically the modifier key with name keyname."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_KEY_LOCK_ON, "Enables or turns on programmatically the lock key with name keyname."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_KEY_LOCK_OFF, "Disables or turns off programmatically the lock key with name keyname."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_KEY_MODIFIER_MASK_GET, "Creates a bit mask from the keyname modifier key."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_DAMAGE_RECTANGLE_ADD, "Add a damage rectangle."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OBSCURED_RECTANGLE_ADD, "Add an obscured region to an Evas canvas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OBSCURED_CLEAR, "Remove all obscured regions from an Evas canvas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_RENDER_UPDATES, "Force immediate renderization of the given Evas canvas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_RENDER, "Force renderization of the given canvas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_NORENDER, "Update the canvas internal objects but not triggering immediate."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_RENDER_IDLE_FLUSH, "Make the canvas discard internally cached data used for rendering."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_SYNC, "Force evas render to be synchronized (in case it is asynchronous)."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_RENDER_DUMP, "Make the canvas discard as much data as possible used by the engine at"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OBJECT_BOTTOM_GET, "Get the lowest (stacked) Evas object on the canvas e."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OBJECT_TOP_GET, "Get the highest (stacked) Evas object on the canvas e."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_COUNT, "Get the number of touched point in the evas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_XY_GET, "This function returns the nth touch point's co-ordinates."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_ID_GET, "This function returns the id of nth touch point."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_TOUCH_POINT_LIST_NTH_STATE_GET, "This function returns the state of nth touch point."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_FLUSH, "Flush the image cache of the canvas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_RELOAD, "Reload the image cache"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_SET, "Set the image cache."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_IMAGE_CACHE_GET, "Get the image cache"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_IMAGE_MAX_SIZE_GET, "Get the maximum image size evas can possibly handle"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OBJECT_NAME_FIND, "Retrieves the object on the given evas with the given name."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OBJECT_TOP_AT_XY_GET, "Retrieve the Evas object stacked at the top of a given position in"),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OBJECT_TOP_IN_RECTANGLE_GET, "Retrieve the Evas object stacked at the top of a given rectangulari region in a canvas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OBJECTS_AT_XY_GET, "Retrieve a list of Evas objects lying over a given position in a canvas."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_OBJECTS_IN_RECTANGLE_GET, "Retrieves the objects in the given rectangle region."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE, "Call user-provided calculate() smart functions."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_SMART_OBJECTS_CALCULATE_COUNT_GET, "Get the internal counter that counts the number of smart calculations."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_RENDER_ASYNC, "Renders the canvas asynchronously."),
     EO_OP_DESCRIPTION(EVAS_CANVAS_SUB_ID_TREE_OBJECTS_AT_XY_GET, "Retrieve a list of Evas objects lying over a given position in a canvas."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Evas",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&EVAS_CANVAS_BASE_ID, op_desc, EVAS_CANVAS_SUB_ID_LAST),
     NULL,
     sizeof(Evas_Public_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(evas_class_get, &class_desc, EO_BASE_CLASS, EVAS_COMMON_INTERFACE, NULL);

