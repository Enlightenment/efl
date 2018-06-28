#define EVAS_CANVAS_BETA
#define EFL_INPUT_EVENT_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"
//#include "evas_cs.h"
#ifdef EVAS_CSERVE2
#include "evas_cs2_private.h"
#endif

#include "evas_image_private.h"
#include "evas_polygon_private.h"
#include "evas_vg_private.h"

#include "eo_internal.h"

#include <Ecore.h>

#define EFL_INTERNAL_UNSTABLE
#include "interfaces/efl_common_internal.h"

#define MY_CLASS EVAS_CANVAS_CLASS

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

#ifdef _WIN32
   if (!evil_init())
     return --_evas_init_count;
#endif

   if (!eina_init())
     goto shutdown_evil;

   if (!eet_init())
     goto shutdown_eet;

   _evas_log_dom_global = eina_log_domain_register
     ("evas_main", EVAS_DEFAULT_LOG_COLOR);
   if (_evas_log_dom_global < 0)
     {
        EINA_LOG_ERR("Can not create a module log domain.");
        goto shutdown_eina;
     }

   efl_object_init();

#ifdef BUILD_LOADER_EET
   eet_init();
#endif

   ecore_init();

   evas_module_init();
   if (!evas_async_events_init())
     goto shutdown_module;
#ifdef EVAS_CSERVE2
   int cs2 = 0;
   {
      const char *env;
      env = getenv("EVAS_CSERVE2");
      if (env && atoi(env))
        {
           cs2 = evas_cserve2_init();
           if (!cs2) goto shutdown_async_events;
        }
   }
#endif
   _evas_preload_thread_init();
   evas_filter_init();
   evas_cache_vg_init();

   if (!evas_thread_init())
     goto shutdown_filter;

   evas_common_init();

   eina_log_timing(_evas_log_dom_global,
		   EINA_LOG_STATE_STOP,
		   EINA_LOG_STATE_INIT);

   _efl_gfx_map_init();
   evas_focus_init();

   return _evas_init_count;

 shutdown_filter:
   evas_filter_shutdown();
   _evas_preload_thread_shutdown();
#ifdef EVAS_CSERVE2
   if (cs2) evas_cserve2_shutdown();
 shutdown_async_events:
   evas_async_events_shutdown();
#endif
 shutdown_module:
   evas_module_shutdown();
#ifdef BUILD_LOADER_EET
   eet_shutdown();
#endif
   efl_object_shutdown();
   eina_log_domain_unregister(_evas_log_dom_global);
 shutdown_eet:
   eet_shutdown();
 shutdown_eina:
   eina_shutdown();
 shutdown_evil:
#ifdef _WIN32
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

   evas_focus_shutdown();

   evas_common_shutdown();

#ifdef EVAS_CSERVE2
   if (evas_cserve2_use_get())
     evas_cserve2_shutdown();
#endif
   evas_cache_vg_shutdown();

   evas_font_path_global_clear();

   evas_thread_shutdown();
   _evas_preload_thread_shutdown();
   evas_async_events_shutdown();

   ecore_shutdown();

   evas_filter_shutdown();
   evas_module_shutdown();

   _efl_gfx_map_shutdown();

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

   eina_cow_del(evas_object_mask_cow);
   evas_object_mask_cow = NULL;

#ifdef BUILD_LOADER_EET
   eet_shutdown();
#endif
   efl_object_shutdown();

   eina_log_domain_unregister(_evas_log_dom_global);

   eet_shutdown();

   eina_shutdown();
#ifdef _WIN32
   evil_shutdown();
#endif

   return _evas_init_count;
}


EAPI Evas *
evas_new(void)
{
   Evas_Object *eo_obj = efl_add(EVAS_CANVAS_CLASS, efl_main_loop_get());
   return eo_obj;
}

static void
_evas_key_mask_free(void *data)
{
   free(data);
}

EOLIAN static Eo *
_evas_canvas_efl_object_constructor(Eo *eo_obj, Evas_Public_Data *e)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));

   e->evas = eo_obj;
   e->output.render_method = RENDER_METHOD_INVALID;
   e->viewport.w = 1;
   e->viewport.h = 1;
   e->framespace.x = 0;
   e->framespace.y = 0;
   e->framespace.w = 0;
   e->framespace.h = 0;
   e->hinting = EVAS_FONT_HINTING_BYTECODE;
   e->current_event = EVAS_CALLBACK_LAST;
   e->name_hash = eina_hash_string_superfast_new((Eina_Free_Cb)eina_list_free);
   eina_clist_init(&e->calc_list);
   eina_clist_init(&e->calc_done);

   efl_wref_add(efl_add(EFL_CANVAS_GESTURE_MANAGER_CLASS, eo_obj), &e->gesture_manager);

#define EVAS_ARRAY_SET(E, Array) \
   eina_array_step_set(&E->Array, sizeof (E->Array), \
                       ((1024 * sizeof (void*)) - sizeof (E->Array)) / sizeof (void*));

   EVAS_ARRAY_SET(e, delete_objects);
   EVAS_ARRAY_SET(e, restack_objects);
   EVAS_ARRAY_SET(e, render_objects);
   EVAS_ARRAY_SET(e, pending_objects);
   EVAS_ARRAY_SET(e, obscuring_objects);
   EVAS_ARRAY_SET(e, temporary_objects);
   EVAS_ARRAY_SET(e, snapshot_objects);
   EVAS_ARRAY_SET(e, clip_changes);
   EVAS_ARRAY_SET(e, scie_unref_queue);
   EVAS_ARRAY_SET(e, image_unref_queue);
   EVAS_ARRAY_SET(e, glyph_unref_queue);
   EVAS_ARRAY_SET(e, texts_unref_queue);

   e->active_objects.version = EINA_ARRAY_VERSION;
   eina_inarray_step_set(&e->active_objects,
                         sizeof(Eina_Inarray),
                         sizeof(Evas_Active_Entry),
                         256);

#undef EVAS_ARRAY_SET
   eina_lock_new(&(e->lock_objects));
   eina_spinlock_new(&(e->render.lock));
   eina_spinlock_new(&(e->post_render.lock));

   _evas_canvas_event_init(eo_obj, e);

   e->focused_objects = eina_hash_pointer_new(NULL);
   e->locks.masks = eina_hash_pointer_new(_evas_key_mask_free);
   e->modifiers.masks = eina_hash_pointer_new(_evas_key_mask_free);
   e->locks.e = e->modifiers.e = e;

   return eo_obj;
}

EAPI void
evas_free(Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   efl_del(eo_e);
}

typedef struct _Forced_Death Forced_Death;
struct _Forced_Death
{
   Eina_Bool invalidated;
   Eina_Bool noref;
   Eina_Bool destroyed;
};

static void
_object_invalidate(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Forced_Death *force = data;
   force->invalidated = EINA_TRUE;
}

static void
_object_del(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Forced_Death *force = data;
   force->destroyed = EINA_TRUE;
}

static void
_object_noref(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Forced_Death *force = data;
   force->noref = EINA_TRUE;
}

EFL_CALLBACKS_ARRAY_DEFINE(_object_forced_death,
                           { EFL_EVENT_DEL, _object_del },
                           { EFL_EVENT_INVALIDATE, _object_invalidate },
                           { EFL_EVENT_NOREF, _object_noref } );


EOLIAN static void
_evas_canvas_efl_object_invalidate(Eo *eo_e, Evas_Public_Data *e)
{
   Evas_Object_Protected_Data *o;
   Evas_Layer *lay;
   Eo *obj;
   Eina_Array stash = { 0 };

   evas_sync(eo_e);

   evas_canvas_async_block(e);
   evas_render_idle_flush(eo_e);

   efl_replace(&e->default_seat, NULL);
   efl_replace(&e->default_mouse, NULL);
   efl_replace(&e->default_keyboard, NULL);

   _evas_post_event_callback_free(eo_e);
   _evas_canvas_event_shutdown(eo_e, e);

   e->cleanup = 1;

   eina_array_step_set(&stash, sizeof (Eina_Array), 16);

   // The first pass should destroy all object that are properly referenced
   EINA_INLIST_FOREACH(e->layers, lay)
     {
        evas_layer_pre_free(lay);

        EINA_INLIST_FOREACH(lay->objects, o)
          {
             if (!o->delete_me)
               eina_array_push(&stash, o->object);
          }
     }

   while ((obj = eina_array_pop(&stash)))
     {
        Evas_Object_Protected_Data *pd = efl_data_scope_get(obj, EFL_CANVAS_OBJECT_CLASS);

        // Properly destroy depending on the object being legacy or not
        if (pd->legacy.ctor) evas_object_del(obj);
        else efl_del(obj);
     }

   // We are now potentially only facing zombies
   EINA_INLIST_FOREACH(e->layers, lay)
     {
        evas_layer_pre_free(lay);
        EINA_INLIST_FOREACH(lay->objects, o)
          {
             if (!o->delete_me)
               eina_array_push(&stash, o->object);
          }
     }

   // Killing zombies now
   while ((obj = eina_array_pop(&stash)))
     {
        Forced_Death force = {
          efl_invalidated_get(obj),
          efl_parent_get(obj) ? (efl_ref_count(obj) <= 1) : (efl_ref_count(obj) <= 0),
          EINA_FALSE
        };

        efl_event_callback_array_add(obj, _object_forced_death(), &force);

        ERR("Killing Zombie Object [%s:%i:%i]. Refs: %i:%i",
            efl_debug_name_get(obj), force.invalidated, force.noref,
            efl_ref_count(obj), ___efl_ref2_count(obj));
        ___efl_ref2_reset(obj);
        // This code explicitely bypass all refcounting to destroy them
        if (!force.invalidated) efl_del(obj);
        while (!force.destroyed) efl_unref(obj);

        if (!force.invalidated)
          {
             ERR("Zombie Object [%s] %s@%p could not be invalidated. "
                 "It seems like the call to efl_invalidated() wasn't "
                 "propagated to all the parent classes.",
                 efl_debug_name_get(obj), efl_class_name_get(obj), obj);
          }
        if (!force.destroyed)
          {
             ERR("Zombie Object [%s] %s@%p could not be destroyed. "
                 "It seems like the call to efl_destructor() wasn't "
                 "propagated to all the parent classes.",
                 efl_debug_name_get(obj), efl_class_name_get(obj), obj);
          }

        // Forcefully remove the object from layers
        EINA_INLIST_FOREACH(e->layers, lay)
          EINA_INLIST_FOREACH(lay->objects, o)
            if (o && (o->object == obj))
              {
                 ERR("Zombie Object [%s] %s@%p could not be removed "
                     "from the canvas list of objects. Maybe this object "
                     "was deleted but the call to efl_invalidated() "
                     "was not propagated to all the parent classes? "
                     "Forcibly removing it. This may leak! Refs: %i:%i",
                     efl_debug_name_get(obj), efl_class_name_get(obj), obj,
                     efl_ref_count(obj), ___efl_ref2_count(obj));
                 lay->objects = (Evas_Object_Protected_Data *)
                   eina_inlist_remove(EINA_INLIST_GET(lay->objects), EINA_INLIST_GET(o));
                 goto next_zombie;
              }

     next_zombie:
        continue;
     }

   eina_array_flush(&stash);

   // Destroying layers and their associated objects completely
   EINA_INLIST_FOREACH(e->layers, lay)
     evas_layer_free_objects(lay);
   evas_layer_clean(eo_e);

   efl_invalidate(efl_super(eo_e, MY_CLASS));
}

EOLIAN static void
_evas_canvas_efl_object_destructor(Eo *eo_e, Evas_Public_Data *e)
{
   Eina_Rectangle *r;
   Evas_Coord_Touch_Point *touch_point;
   Evas_Post_Render_Job *job;
   Evas_Layer *lay;
   Efl_Canvas_Output *evo;
   int i;

   if (e->layers)
     {
        // This should never happen as during invalidate we explicitely
        // destroy all layers. If they survive, we have a zombie appocallypse.
        Evas_Object_Protected_Data *o;

        CRI("The layers of %p are not empty !", eo_e);

        EINA_INLIST_FOREACH(e->layers, lay)
          EINA_INLIST_FOREACH(lay->objects, o)
            {
               CRI("Zombie object [%s] %s@%p still present.",
                   efl_debug_name_get(o->object), efl_class_name_get(o->object), o->object);
            }
     }

   evas_font_path_clear(eo_e);

   if (e->name_hash) eina_hash_free(e->name_hash);
   e->name_hash = NULL;

   EINA_LIST_FREE(e->damages, r)
      eina_rectangle_free(r);
   EINA_LIST_FREE(e->obscures, r)
      eina_rectangle_free(r);

   evas_fonts_zero_free();

   evas_event_callback_all_del(eo_e);
   evas_event_callback_cleanup(eo_e);

   EINA_LIST_FREE(e->touch_points, touch_point)
     free(touch_point);

   _evas_device_cleanup(eo_e);
   e->focused_by = eina_list_free(e->focused_by);

   while (e->seats)
     {
        Evas_Pointer_Seat *pseat = EINA_INLIST_CONTAINER_GET(e->seats, Evas_Pointer_Seat);

        eina_list_free(pseat->object.in);
        while (pseat->pointers)
          {
             Evas_Pointer_Data *pdata = EINA_INLIST_CONTAINER_GET(pseat->pointers, Evas_Pointer_Data);
             pseat->pointers = eina_inlist_remove(pseat->pointers, pseat->pointers);
             free(pdata);
          }
        e->seats = eina_inlist_remove(e->seats, e->seats);
        free(pseat);
     }

   /* Ector surface may require an existing output to finish its job */
   if (e->engine.func)
     e->engine.func->ector_destroy(_evas_engine_context(e), e->ector);
   /* cleanup engine backend */
   EINA_LIST_FREE(e->outputs, evo) efl_canvas_output_del(evo);
   if (e->engine.func)
     e->engine.func->engine_free(e->backend);

   for (i = 0; i < e->modifiers.mod.count; i++)
     free(e->modifiers.mod.list[i]);
   if (e->modifiers.mod.list) free(e->modifiers.mod.list);

   for (i = 0; i < e->locks.lock.count; i++)
     free(e->locks.lock.list[i]);
   if (e->locks.lock.list) free(e->locks.lock.list);

   if (e->engine.module) evas_module_unref(e->engine.module);

   eina_array_flush(&e->delete_objects);
   eina_inarray_flush(&e->active_objects);
   eina_array_flush(&e->restack_objects);
   eina_array_flush(&e->render_objects);
   eina_array_flush(&e->pending_objects);
   eina_array_flush(&e->obscuring_objects);
   eina_array_flush(&e->temporary_objects);
   eina_array_flush(&e->snapshot_objects);
   eina_array_flush(&e->clip_changes);
   eina_array_flush(&e->scie_unref_queue);
   eina_array_flush(&e->image_unref_queue);
   eina_array_flush(&e->glyph_unref_queue);
   eina_array_flush(&e->texts_unref_queue);
   eina_hash_free(e->focused_objects);

   SLKL(e->post_render.lock);
   EINA_INLIST_FREE(e->post_render.jobs, job)
     {
        e->post_render.jobs = (Evas_Post_Render_Job *)
              eina_inlist_remove(EINA_INLIST_GET(e->post_render.jobs), EINA_INLIST_GET(job));
        free(job);
     }
   SLKU(e->post_render.lock);

   eina_lock_free(&(e->lock_objects));
   eina_spinlock_free(&(e->render.lock));
   eina_spinlock_free(&(e->post_render.lock));
   eina_hash_free(e->locks.masks);
   eina_hash_free(e->modifiers.masks);

   e->magic = 0;
   efl_destructor(efl_super(eo_e, MY_CLASS));
}

// It is now expected that the first output in the list is the default one
// manipulated by this set of legacy API

EAPI Evas_Engine_Info *
evas_engine_info_get(const Evas *obj)
{
   if (!obj) return NULL;

   Evas_Public_Data *e = efl_data_scope_get(obj, EVAS_CANVAS_CLASS);
   Efl_Canvas_Output *output;

   output = eina_list_data_get(e->outputs);
   if (!output)
     {
        output = efl_canvas_output_add((Evas*) obj);
     }
   if (!output) return NULL;
   e->output.legacy = EINA_TRUE;

   return efl_canvas_output_engine_info_get(output);
}

EAPI Eina_Bool
evas_engine_info_set(Evas *obj, Evas_Engine_Info *info)
{
   if (!obj) return EINA_FALSE;

   Evas_Public_Data *e = efl_data_scope_get(obj, EVAS_CANVAS_CLASS);
   Efl_Canvas_Output *output;

   output = eina_list_data_get(e->outputs);
   if (!output) return EINA_FALSE;
   if (!info) return EINA_FALSE;
   efl_canvas_output_view_set(output, 0, 0,
                              e->output.w, e->output.h);
   return efl_canvas_output_engine_info_set(output, info);
}

EOLIAN static Evas_Coord
_evas_canvas_coord_screen_x_to_world(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int x)
{
   if (e->output.w == e->viewport.w) return e->viewport.x + x;
   else return (long long)e->viewport.x + (((long long)x * (long long)e->viewport.w) / (long long)e->output.w);
}

EOLIAN static Evas_Coord
_evas_canvas_coord_screen_y_to_world(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int y)
{
   if (e->output.h == e->viewport.h) return e->viewport.y + y;
   else return (long long)e->viewport.y + (((long long)y * (long long)e->viewport.h) / (long long)e->output.h);
}

EOLIAN static int
_evas_canvas_coord_world_x_to_screen(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord x)
{
   if (e->output.w == e->viewport.w) return x - e->viewport.x;
   else return (int)((((long long)x - (long long)e->viewport.x) * (long long)e->output.w) /  (long long)e->viewport.w);
}

EOLIAN static int
_evas_canvas_coord_world_y_to_screen(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord y)
{
   if (e->output.h == e->viewport.h) return y - e->viewport.y;
   else return (int)((((long long)y - (long long)e->viewport.y) * (long long)e->output.h) /  (long long)e->viewport.h);
}

EOLIAN static Efl_Input_Device *
_evas_canvas_default_device_get(const Eo *eo_e EINA_UNUSED,
                                Evas_Public_Data *e,
                                Efl_Input_Device_Type klass)
{
   if (klass == EFL_INPUT_DEVICE_TYPE_SEAT)
     return e->default_seat;
   if (klass == EFL_INPUT_DEVICE_TYPE_MOUSE)
     return e->default_mouse;
   if (klass == EFL_INPUT_DEVICE_TYPE_KEYBOARD)
     return e->default_keyboard;
   return NULL;
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
_evas_canvas_pointer_output_xy_by_device_get(const Eo *eo_e EINA_UNUSED,
                                             Evas_Public_Data *e,
                                             Efl_Input_Device *dev,
                                             int *x, int *y)
{
   Evas_Pointer_Data *pdata = _evas_pointer_data_by_device_get(e, dev);

   if (!pdata)
     {
        if (x) *x = 0;
        if (y) *y = 0;
     }
   else
     {
        if (x) *x = pdata->seat->x;
        if (y) *y = pdata->seat->y;
     }

}

EOLIAN static void
_evas_canvas_pointer_canvas_xy_by_device_get(const Eo *eo_e EINA_UNUSED,
                                             Evas_Public_Data *e,
                                             Efl_Input_Device *dev,
                                             int *x, int *y)
{
   Evas_Pointer_Data *pdata = _evas_pointer_data_by_device_get(e, dev);

   if (!pdata)
     {
        if (x) *x = 0;
        if (y) *y = 0;
     }
   else
     {
        if (x) *x = pdata->seat->x;
        if (y) *y = pdata->seat->y;
     }
}

EOLIAN static unsigned int
_evas_canvas_pointer_button_down_mask_by_device_get(const Eo *eo_e EINA_UNUSED,
                                                    Evas_Public_Data *e,
                                                    Efl_Input_Device *dev)
{
   Evas_Pointer_Data *pdata = _evas_pointer_data_by_device_get(e, dev);
   if (!pdata) return 0;
   return pdata->button;
}

EOLIAN static Eina_Bool
_evas_canvas_efl_canvas_pointer_pointer_inside_get(const Eo *eo_e EINA_UNUSED,
                                                   Evas_Public_Data *e,
                                                   Efl_Input_Device *dev)
{
   Evas_Pointer_Data *pdata = _evas_pointer_data_by_device_get(e, dev);
   if (!pdata) return EINA_FALSE;
   return pdata->seat->inside;
}

EOLIAN static void
_evas_canvas_pointer_output_xy_get(const Eo *eo_e, Evas_Public_Data *e, int *x, int *y)
{
   return _evas_canvas_pointer_output_xy_by_device_get(eo_e, e, NULL, x, y);
}

EOLIAN static void
_evas_canvas_pointer_canvas_xy_get(const Eo *eo_e, Evas_Public_Data *e, Evas_Coord *x, Evas_Coord *y)
{
   return _evas_canvas_pointer_canvas_xy_by_device_get(eo_e, e, NULL, x, y);
}

EOLIAN static unsigned int
_evas_canvas_pointer_button_down_mask_get(const Eo *eo_e, Evas_Public_Data *e)
{
   return _evas_canvas_pointer_button_down_mask_by_device_get(eo_e, e, NULL);
}

EOLIAN static void
_evas_canvas_data_attach_set(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, void *data)
{
   e->attach_data = data;
}

EOLIAN static void*
_evas_canvas_data_attach_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->attach_data;
}

static void
_evas_canvas_focus_inout_dispatch(Eo *eo_e, Efl_Input_Device *seat,
                                  Eina_Bool in)
{
   Efl_Input_Focus_Data *ev_data;
   Efl_Input_Focus *evt;

   evt = efl_input_instance_get(EFL_INPUT_FOCUS_CLASS, eo_e, (void **) &ev_data);
   if (!evt) return;

   ev_data->device = efl_ref(seat);
   ev_data->timestamp = time(NULL);
   efl_event_callback_call(eo_e,
                           in ? EFL_EVENT_FOCUS_IN : EFL_EVENT_FOCUS_OUT,
                           evt);
   efl_unref(evt);
}

EOLIAN static void
_evas_canvas_seat_focus_in(Eo *eo_e, Evas_Public_Data *e,
                           Efl_Input_Device *seat)
{
   if (!seat) seat = e->default_seat;
   if (!seat || efl_input_device_type_get(seat) != EFL_INPUT_DEVICE_TYPE_SEAT) return;
   _evas_canvas_focus_inout_dispatch(eo_e, seat, EINA_TRUE);
}

EOLIAN static void
_evas_canvas_seat_focus_out(Eo *eo_e, Evas_Public_Data *e,
                            Efl_Input_Device *seat)
{
   if (!seat) seat = e->default_seat;
   if (!seat || efl_input_device_type_get(seat) != EFL_INPUT_DEVICE_TYPE_SEAT) return;
   _evas_canvas_focus_inout_dispatch(eo_e, seat, EINA_FALSE);
}

EOLIAN static void
_evas_canvas_focus_in(Eo *eo_e, Evas_Public_Data *e)
{
   _evas_canvas_seat_focus_in(eo_e, e, NULL);
}

EOLIAN static void
_evas_canvas_focus_out(Eo *eo_e, Evas_Public_Data *e)
{
   _evas_canvas_seat_focus_out(eo_e, e, NULL);
}

EOLIAN static Eina_Bool
_evas_canvas_seat_focus_state_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e,
                                  Efl_Input_Device *seat)
{
   if (!seat) seat = e->default_seat;
   return eina_list_data_find(e->focused_by, seat) ? EINA_TRUE : EINA_FALSE;
}

EOLIAN static Eina_Bool
_evas_canvas_focus_state_get(const Eo *eo_e, Evas_Public_Data *e)
{
   return _evas_canvas_seat_focus_state_get(eo_e, e, NULL);
}

EOLIAN static Eina_Bool
_evas_canvas_changed_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->changed;
}

EOLIAN static void
_evas_canvas_nochange_push(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   e->nochange++;
}

EOLIAN static void
_evas_canvas_nochange_pop(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   e->nochange--;
}

void
_evas_walk(Evas_Public_Data *e)
{
   efl_ref(e->evas);
}

void
_evas_unwalk(Evas_Public_Data *e)
{
   efl_unref(e->evas);
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

EOLIAN static Eo *
_evas_canvas_efl_object_provider_find(const Eo *eo_e,
                                      Evas_Public_Data *e EINA_UNUSED,
                                      const Efl_Class *klass)
{
   if (klass == EVAS_CANVAS_CLASS)
     return (Eo *)eo_e;
   else if (klass == EFL_LOOP_CLASS)
     return efl_main_loop_get();
   else if (klass == EFL_CANVAS_GESTURE_MANAGER_CLASS)
     return e->gesture_manager;
   return efl_provider_find(efl_super(eo_e, MY_CLASS), klass);
}

EOLIAN static Efl_Loop *
_evas_canvas_efl_loop_consumer_loop_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e EINA_UNUSED)
{
   return efl_main_loop_get();
}

Ector_Surface *
evas_ector_get(Evas_Public_Data *e)
{
   if (!e->ector)
     e->ector = e->engine.func->ector_create(_evas_engine_context(e));
   return e->ector;
}

EAPI Evas_BiDi_Direction
evas_language_direction_get(void)
{
   return evas_common_language_direction_get();
}

EAPI void
evas_language_reinit(void)
{
   evas_common_language_reinit();
}

static void
_image_data_unset(Evas_Object_Protected_Data *obj, Eina_List **list)
{
   if (obj->is_smart)
     {
        Evas_Object_Protected_Data *obj2;

        EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(obj->object), obj2)
          _image_data_unset(obj2, list);
        return;
     }
#define CHECK(TYPE, STRUCT, FREE) \
   if (efl_isa(obj->object, TYPE))\
     {\
        STRUCT *data = efl_data_scope_get(obj->object, TYPE);\
        FREE; \
        data->engine_data = NULL;\
     }
   CHECK(EFL_CANVAS_IMAGE_INTERNAL_CLASS, Evas_Image_Data,
         ENFN->image_free(ENC, data->engine_data))
   else CHECK(EFL_CANVAS_POLYGON_CLASS, Efl_Canvas_Polygon_Data,
        data->engine_data =
          obj->layer->evas->engine.func->polygon_points_clear(ENC,
                                                              data->engine_data))
   else CHECK(EVAS_CANVAS3D_TEXTURE_CLASS, Evas_Canvas3D_Texture_Data,
        if (obj->layer->evas->engine.func->texture_free)
          obj->layer->evas->engine.func->texture_free(ENC, data->engine_data))
   else return;
#undef CHECK
   evas_object_ref(obj->object);
   *list = eina_list_append(*list, obj->object);
}

EAPI Eina_List *
_evas_canvas_image_data_unset(Evas *eo_e)
{
   Evas_Public_Data *e = efl_data_scope_get(eo_e, MY_CLASS);
   Evas_Layer *lay;
   Eina_List *list = NULL;

   EINA_INLIST_FOREACH(e->layers, lay)
     {
        Evas_Object_Protected_Data *o;
        EINA_INLIST_FOREACH(lay->objects, o)
          {
             if (!o->delete_me)
               _image_data_unset(o, &list);
          }
     }
   return list;
}

static void
_image_image_data_regenerate(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Image_Data *data)
{
   unsigned int orient = data->cur->orient;

   _evas_image_load(eo_obj, obj, data);
   EINA_COW_IMAGE_STATE_WRITE_BEGIN(data, state_write)
     {
        state_write->has_alpha = !state_write->has_alpha;
        state_write->orient = -1;
     }
   EINA_COW_IMAGE_STATE_WRITE_END(data, state_write);
   evas_object_image_alpha_set(eo_obj, !data->cur->has_alpha);
   _evas_image_orientation_set(eo_obj, data, orient);
}

static void
_image_data_regenerate(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj;

   obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   evas_object_change(eo_obj, obj);
#define CHECK(TYPE, STRUCT, REGEN) \
   if (efl_isa(eo_obj, TYPE))\
     {\
        STRUCT *data = efl_data_scope_get(eo_obj, TYPE);\
        REGEN; \
     }
   CHECK(EFL_CANVAS_IMAGE_INTERNAL_CLASS, Evas_Image_Data, _image_image_data_regenerate(eo_obj, obj, data))
   else CHECK(EFL_CANVAS_IMAGE_CLASS, Evas_Image_Data, _image_image_data_regenerate(eo_obj, obj, data))
   else CHECK(EFL_CANVAS_SCENE3D_CLASS, Evas_Image_Data, _image_image_data_regenerate(eo_obj, obj, data))
   //else CHECK(EFL_CANVAS_VG_OBJECT_CLASS, Efl_Canvas_Vg_Object_Data,)
   //else CHECK(EFL_CANVAS_POLYGON_CLASS, Efl_Canvas_Polygon_Data,)
   //else CHECK(EVAS_CANVAS3D_TEXTURE_CLASS, Evas_Canvas3D_Texture_Data,
}

EAPI void
_evas_canvas_image_data_regenerate(Eina_List *list)
{
   Evas_Object *eo_obj;

   EINA_LIST_FREE(list, eo_obj)
     {
        _image_data_regenerate(eo_obj);
        evas_object_unref(eo_obj);
     }
}


EOLIAN void
_evas_canvas_image_cache_flush(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   evas_canvas_async_block(e);
   evas_render_rendering_wait(e);
   if (_evas_engine_context(e))
     e->engine.func->image_cache_flush(_evas_engine_context(e));
}

EOLIAN void
_evas_canvas_image_cache_reload(Eo *eo_e, Evas_Public_Data *e)
{
   Evas_Layer *layer;

   evas_canvas_async_block(e);
   evas_image_cache_flush(eo_e);
   EINA_INLIST_FOREACH(e->layers, layer)
     {
        Evas_Object_Protected_Data *obj;

        layer->walking_objects++;
        EINA_INLIST_FOREACH(layer->objects, obj)
          {
             if (efl_isa(obj->object, MY_CLASS))
               {
                  _evas_image_unload(obj->object, obj, 1);
                  evas_object_inform_call_image_unloaded(obj->object);
               }
          }
        layer->walking_objects--;
        _evas_layer_flush_removes(layer);
     }
   evas_image_cache_flush(eo_e);
   EINA_INLIST_FOREACH(e->layers, layer)
     {
        Evas_Object_Protected_Data *obj;

        layer->walking_objects++;
        EINA_INLIST_FOREACH(layer->objects, obj)
          {
             if (efl_isa(obj->object, MY_CLASS))
               {
                  Evas_Image_Data *o = efl_data_scope_get(obj->object, MY_CLASS);
                  _evas_image_load(obj->object, obj, o);
                  o->changed = EINA_TRUE;
                  evas_object_change(obj->object, obj);
               }
          }
        layer->walking_objects--;
        _evas_layer_flush_removes(layer);
     }
   evas_image_cache_flush(eo_e);
}

EOLIAN static void
_evas_canvas_image_cache_set(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int size)
{
   if (size < 0) size = 0;
   evas_canvas_async_block(e);
   evas_render_rendering_wait(e);
   if (_evas_engine_context(e))
     e->engine.func->image_cache_set(_evas_engine_context(e), size);
}

EOLIAN static int
_evas_canvas_image_cache_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   if (_evas_engine_context(e))
     return e->engine.func->image_cache_get(_evas_engine_context(e));
   return -1;
}

EOLIAN static Eina_Bool
_evas_canvas_efl_canvas_scene_image_max_size_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Eina_Size2D *max)
{
   int w = 0, h = 0;

   if (max) *max = EINA_SIZE2D(0xffff, 0xffff);
   if (!e->engine.func->image_max_size_get) return EINA_FALSE;
   if (!max) return EINA_TRUE;
   e->engine.func->image_max_size_get(_evas_engine_context(e), &w, &h);
   *max = EINA_SIZE2D(w, h);
   return EINA_TRUE;
}

/* Legacy deprecated functions */

EAPI void
evas_output_framespace_set(Evas *eo_e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   if ((x == e->framespace.x) && (y == e->framespace.y) &&
       (w == e->framespace.w) && (h == e->framespace.h)) return;
   evas_canvas_async_block(e);
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
   return;
   MAGIC_CHECK_END();

   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   if (x) *x = e->framespace.x;
   if (y) *y = e->framespace.y;
   if (w) *w = e->framespace.w;
   if (h) *h = e->framespace.h;
}

EAPI void
evas_output_method_set(Evas *eo_e, int render_method)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

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

   evas_canvas_async_block(e);
   /* set the correct render */
   e->output.render_method = render_method;
   e->engine.func = (em->functions);
   evas_module_use(em);
   if (e->engine.module) evas_module_unref(e->engine.module);
   e->engine.module = em;
   evas_module_ref(em);

   /* Initialize the engine first */
   e->backend = e->engine.func->engine_new();

   /* get the engine info struct */
   if (e->engine.func->info_size)
     {
        Efl_Canvas_Output *output;
        Eina_List *l;

        EINA_LIST_FOREACH(e->outputs, l, output)
          efl_canvas_output_info_get(e, output);
     }
   else
     {
        CRI("Engine not up to date no info size provided.");
     }

   // Wayland/drm already handles seats.
   if (em->definition && (eina_streq(em->definition->name, "wayland_shm") ||
                          eina_streq(em->definition->name, "wayland_egl") ||
                          eina_streq(em->definition->name, "drm") ||
                          eina_streq(em->definition->name, "gl_drm")))
     {
        Evas_Pointer_Seat *pseat = calloc(1, sizeof(Evas_Pointer_Seat));
        e->seats = eina_inlist_append(e->seats, EINA_INLIST_GET(pseat));
        return;
     }

   e->default_seat = efl_ref(evas_device_add_full(eo_e, "default", "The default seat",
                                                  NULL, NULL, EVAS_DEVICE_CLASS_SEAT,
                                                  EVAS_DEVICE_SUBCLASS_NONE));
   evas_device_seat_id_set(e->default_seat, 1);
   e->default_mouse = efl_ref(evas_device_add_full(eo_e, "Mouse",
                                                   "The default mouse",
                                                   e->default_seat, NULL,
                                                   EVAS_DEVICE_CLASS_MOUSE,
                                                   EVAS_DEVICE_SUBCLASS_NONE));
   e->default_keyboard = efl_ref(evas_device_add_full(eo_e, "Keyboard",
                                                      "The default keyboard",
                                                      e->default_seat, NULL,
                                                      EVAS_DEVICE_CLASS_KEYBOARD,
                                                      EVAS_DEVICE_SUBCLASS_NONE));
}

EAPI int
evas_output_method_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return RENDER_METHOD_INVALID;
   MAGIC_CHECK_END();

   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   return e->output.render_method;
}

EAPI void
evas_output_size_set(Evas *eo_e, int w, int h)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   if ((w == e->output.w) && (h == e->output.h)) return;
   if (w < 1) w = 1;
   if (h < 1) h = 1;

   evas_canvas_async_block(e);
   e->output.w = w;
   e->output.h = h;
   e->output_validity++;
   e->changed = 1;

   if (e->output.legacy)
     {
        Efl_Canvas_Output *output;

        output = eina_list_data_get(e->outputs);
        efl_canvas_output_view_set(output, 0, 0, w, h);
     }

   evas_render_invalidate(eo_e);
}

EAPI void
evas_output_size_get(const Evas *eo_e, int *w, int *h)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   if (w) *w = e->output.w;
   if (h) *h = e->output.h;
}

EAPI void
evas_output_viewport_set(Evas *eo_e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

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
   evas_canvas_async_block(e);
   e->viewport.x = x;
   e->viewport.y = y;
   e->viewport.w = w;
   e->viewport.h = h;
   e->viewport.changed = 1;
   e->output_validity++;
   e->changed = 1;
   evas_event_callback_call(e->evas, EVAS_CALLBACK_CANVAS_VIEWPORT_RESIZE, NULL);
}

EAPI void
evas_output_viewport_get(const Evas *eo_e, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   Evas_Public_Data *e = efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS);

   if (x) *x = e->viewport.x;
   if (y) *y = e->viewport.y;
   if (w) *w = e->viewport.w;
   if (h) *h = e->viewport.h;
}

Evas_Pointer_Data *
_evas_pointer_data_by_device_get(Evas_Public_Data *edata, Efl_Input_Device *pointer)
{
   Evas_Pointer_Data *pdata;
   Evas_Pointer_Seat *pseat;
   Eo *seat;

   if (!pointer)
     pointer = edata->default_mouse;
   if (!pointer) return NULL;
   seat = efl_input_device_seat_get(pointer);
   if (!seat) return NULL;

   EINA_INLIST_FOREACH(edata->seats, pseat)
     EINA_INLIST_FOREACH(pseat->pointers, pdata)
       {
          if (pointer == seat)
            {
               if (pseat->seat == seat) return pdata;
            }
          else if (pdata->pointer == pointer) return pdata;
       }
   return NULL;
}

Evas_Pointer_Data *
_evas_pointer_data_add(Evas_Public_Data *edata, Efl_Input_Device *pointer)
{
   Evas_Pointer_Data *pdata;
   Evas_Pointer_Seat *pseat = NULL;
   Eo *seat;

   seat = efl_input_device_seat_get(pointer);
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, NULL);
   EINA_INLIST_FOREACH(edata->seats, pseat)
     if (pseat->seat == seat) break;
   if (!pseat)
     {
        pseat = calloc(1, sizeof(Evas_Pointer_Seat));
        EINA_SAFETY_ON_NULL_RETURN_VAL(pseat, NULL);
        pseat->seat = seat;
        edata->seats = eina_inlist_append(edata->seats, EINA_INLIST_GET(pseat));
     }
   pdata = calloc(1, sizeof(Evas_Pointer_Data));
   if (!pdata)
     {
        if (!pseat->pointers)
          {
             edata->seats = eina_inlist_remove(edata->seats, EINA_INLIST_GET(pseat));
             free(pseat);
          }
        ERR("alloc fail");
        return NULL;
     }

   pdata->pointer = pointer;
   pdata->seat = pseat;
   pseat->pointers = eina_inlist_append(pseat->pointers, EINA_INLIST_GET(pdata));
   return pdata;
}

void
_evas_pointer_data_remove(Evas_Public_Data *edata, Efl_Input_Device *pointer)
{
   Evas_Pointer_Data *pdata;
   Evas_Pointer_Seat *pseat;
   Evas_Pointer_Seat *hit = NULL;

   EINA_INLIST_FOREACH(edata->seats, pseat)
     {
        EINA_INLIST_FOREACH(pseat->pointers, pdata)
          if (pdata->pointer == pointer)
            {
               pseat->pointers = eina_inlist_remove(pseat->pointers, EINA_INLIST_GET(pdata));
               free(pdata);
               hit = pseat;
               break;
            }
     }
   EINA_SAFETY_ON_NULL_RETURN(hit);
   if (hit->pointers) return;
   eina_list_free(hit->object.in);
   edata->seats = eina_inlist_remove(edata->seats, EINA_INLIST_GET(hit));
   free(hit);
}

Eina_List *
_evas_pointer_list_in_rect_get(Evas_Public_Data *edata, Evas_Object *obj,
                               Evas_Object_Protected_Data *obj_data,
                               int w, int h)
{
   Eina_List *list = NULL;
   Evas_Pointer_Seat *pseat;

   EINA_INLIST_FOREACH(edata->seats, pseat)
     {
        Evas_Pointer_Data *pdata;
        if (!evas_object_is_in_output_rect(obj, obj_data, pseat->x, pseat->y, w, h)) continue;
        pdata = EINA_INLIST_CONTAINER_GET(pseat->pointers, Evas_Pointer_Data);
        if (pdata)
          list = eina_list_append(list, pdata);
     }

   return list;
}

static Eina_Inlist *
get_layer_objects(Evas_Layer *l)
{
   if ((!l) || (!l->objects)) return NULL;
   return (EINA_INLIST_GET(l->objects));
}

typedef struct _Efl_Canvas_Iterator
{
   Eina_Iterator  iterator;
   Eina_List     *list;
   Eina_Iterator *real_iterator;
   Eo            *object;
} Efl_Canvas_Iterator;

/* this iterator is the same as efl_ui_box */
static Eina_Bool
_efl_canvas_iterator_next(Efl_Canvas_Iterator *it, void **data)
{
   Efl_Gfx_Entity *sub;

   if (!it->object) return EINA_FALSE;
   if (!eina_iterator_next(it->real_iterator, (void **) &sub))
     return EINA_FALSE;

   if (data) *data = sub;
   return EINA_TRUE;
}

static Eo *
_efl_canvas_iterator_get_container(Efl_Canvas_Iterator *it)
{
   return it->object;
}

static void
_efl_canvas_iterator_free(Efl_Canvas_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   efl_wref_del(it->object, &it->object);
   eina_list_free(it->list);
   free(it);
}

EAPI Eina_Iterator *
efl_canvas_iterator_create(Eo *obj, Eina_Iterator *real_iterator, Eina_List *list)
{
   Efl_Canvas_Iterator *it;

   it = calloc(1, sizeof(*it));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->list = list;
   it->real_iterator = real_iterator;
   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_efl_canvas_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_efl_canvas_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_efl_canvas_iterator_free);
   efl_wref_add(obj, &it->object);

   return &it->iterator;
}

EOLIAN static Evas_Object*
_evas_canvas_efl_canvas_scene_object_top_at_xy_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Eina_Position2D pos, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Evas_Layer *lay;
   int xx, yy;

   xx = pos.x;
   yy = pos.y;
////   xx = evas_coord_world_x_to_screen(eo_e, x);
////   yy = evas_coord_world_y_to_screen(eo_e, y);
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
        Evas_Object *eo_obj;
        Evas_Object_Protected_Data *obj;

        EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
          {
             eo_obj = obj->object;
             if (obj->delete_me) continue;
             if ((!include_pass_events_objects) &&
                 (evas_event_passes_through(eo_obj, obj))) continue;
             if (evas_object_is_source_invisible(eo_obj, obj)) continue;
             if ((!include_hidden_objects) && (!obj->cur->visible)) continue;
             evas_object_clip_recalc(obj);
             if ((evas_object_is_in_output_rect(eo_obj, obj, xx, yy, 1, 1)) &&
                 (!obj->clip.clipees) &&
                 RECTS_INTERSECT(xx, yy, 1, 1,
                   obj->cur->geometry.x, obj->cur->geometry.y,
                   obj->cur->geometry.w, obj->cur->geometry.h))
               return eo_obj;
          }
     }
   return NULL;
}

EAPI Evas_Object*
evas_object_top_at_xy_get(Eo *eo_e, Evas_Coord x, Evas_Coord y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Eina_Position2D pos = {x, y};
   return efl_canvas_scene_object_top_at_xy_get(eo_e, pos, include_pass_events_objects, include_hidden_objects);
}

EAPI Evas_Object *
evas_object_top_at_pointer_get(const Evas *eo_e)
{
   Evas_Public_Data *e = efl_isa(eo_e, EVAS_CANVAS_CLASS) ?
            efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS) : NULL;
   if (!e) return NULL;

   Evas_Pointer_Data *pdata = _evas_pointer_data_by_device_get(e, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(pdata, NULL);
   return efl_canvas_scene_object_top_at_xy_get((Eo *)eo_e, EINA_POSITION2D(pdata->seat->x, pdata->seat->y), EINA_TRUE, EINA_TRUE);
}

EOLIAN Evas_Object*
_evas_canvas_efl_canvas_scene_object_top_in_rectangle_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Eina_Rect rect, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Evas_Layer *lay;
   int xx, yy, ww, hh;

   xx = rect.x;
   yy = rect.y;
   ww = rect.w;
   hh = rect.h;
////   xx = evas_coord_world_x_to_screen(eo_e, x);
////   yy = evas_coord_world_y_to_screen(eo_e, y);
////   ww = evas_coord_world_x_to_screen(eo_e, w);
////   hh = evas_coord_world_y_to_screen(eo_e, h);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
        Evas_Object *eo_obj;
        Evas_Object_Protected_Data *obj;

        EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
          {
             eo_obj = obj->object;
             if (obj->delete_me) continue;
             if ((!include_pass_events_objects) &&
                 (evas_event_passes_through(eo_obj, obj))) continue;
             if (evas_object_is_source_invisible(eo_obj, obj)) continue;
             if ((!include_hidden_objects) && (!obj->cur->visible)) continue;
             evas_object_clip_recalc(obj);
             if ((evas_object_is_in_output_rect(eo_obj, obj, xx, yy, ww, hh)) &&
                 (!obj->clip.clipees) &&
                 RECTS_INTERSECT(xx, yy, ww, hh,
                   obj->cur->geometry.x, obj->cur->geometry.y,
                   obj->cur->geometry.w, obj->cur->geometry.h)) return eo_obj;
          }
     }
   return NULL;
}

EAPI Evas_Object *
evas_object_top_in_rectangle_get(const Eo *obj, int x, int y, int w, int h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   return efl_canvas_scene_object_top_in_rectangle_get(obj, EINA_RECT(x, y, w, h), include_pass_events_objects, include_hidden_objects);
}

static Eina_List *
_efl_canvas_evas_canvas_objects_at_xy_get_helper(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int x, int y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Eina_List *in = NULL;
   Evas_Layer *lay;
   int xx, yy;

   xx = x;
   yy = y;
////   xx = evas_coord_world_x_to_screen(eo_e, x);
////   yy = evas_coord_world_y_to_screen(eo_e, y);
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
        Evas_Object *eo_obj;
        Evas_Object_Protected_Data *obj;

        EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
          {
             eo_obj = obj->object;
             // FIXME - Daniel: we don't know yet how to handle the next line
             if (obj->delete_me) continue;
             if ((!include_pass_events_objects) &&
                   (evas_event_passes_through(eo_obj, obj))) continue;
             if (evas_object_is_source_invisible(eo_obj, obj)) continue;
             if ((!include_hidden_objects) && (!obj->cur->visible)) continue;
             evas_object_clip_recalc(obj);
             if ((evas_object_is_in_output_rect(eo_obj, obj, xx, yy, 1, 1)) &&
                 (!obj->clip.clipees))
               {
                  // evas_object_is_in_output_rect is based on the clip which
                  // may be larger than the geometry (bounding box)
                  if (!RECTS_INTERSECT(xx, yy, 1, 1,
                                       obj->cur->geometry.x,
                                       obj->cur->geometry.y,
                                       obj->cur->geometry.w,
                                       obj->cur->geometry.h))
                    continue;
                  in = eina_list_prepend(in, eo_obj);
               }
          }
     }
   return in;
}

EOLIAN static Eina_Iterator *
_evas_canvas_efl_canvas_scene_objects_at_xy_get(Eo *eo_e, Evas_Public_Data *e, Eina_Position2D pos, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Eina_List *l = _efl_canvas_evas_canvas_objects_at_xy_get_helper(eo_e, e, pos.x, pos.y, include_pass_events_objects, include_hidden_objects);
   if (l) return efl_canvas_iterator_create(eo_e, eina_list_iterator_new(l), l);
   return NULL;
}

/**
 * Retrieves the objects in the given rectangle region
 * @param   eo_e The given evas object.
 * @param   x The horizontal coordinate.
 * @param   y The vertical coordinate.
 * @param   w The width size.
 * @param   h The height size.
 * @param   include_pass_events_objects Boolean Flag to include or not pass events objects
 * @param   include_hidden_objects Boolean Flag to include or not hidden objects
 * @return  The list of evas object in the rectangle region.
 *
 */
static Eina_List*
_efl_canvas_objects_in_rectangle_get_helper(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Eina_List *in = NULL;
   Evas_Layer *lay;
   int xx, yy, ww, hh;

   xx = x;
   yy = y;
   ww = w;
   hh = h;
////   xx = evas_coord_world_x_to_screen(eo_e, x);
////   yy = evas_coord_world_y_to_screen(eo_e, y);
////   ww = evas_coord_world_x_to_screen(eo_e, w);
////   hh = evas_coord_world_y_to_screen(eo_e, h);
   if (ww < 1) ww = 1;
   if (hh < 1) hh = 1;
   EINA_INLIST_REVERSE_FOREACH((EINA_INLIST_GET(e->layers)), lay)
     {
        Evas_Object *eo_obj;
        Evas_Object_Protected_Data *obj;

        EINA_INLIST_REVERSE_FOREACH(get_layer_objects(lay), obj)
          {
             eo_obj = obj->object;
             // FIXME - Daniel: we don't know yet how to handle the next line
             if (obj->delete_me) continue;
             if ((!include_pass_events_objects) &&
                 (evas_event_passes_through(eo_obj, obj))) continue;
             if (evas_object_is_source_invisible(eo_obj, obj)) continue;
             if ((!include_hidden_objects) && (!obj->cur->visible)) continue;
             evas_object_clip_recalc(obj);
             if ((evas_object_is_in_output_rect(eo_obj, obj, xx, yy, ww, hh)) &&
                 (!obj->clip.clipees))
               {
                  if (!RECTS_INTERSECT(xx, yy, ww, hh,
                                       obj->cur->geometry.x,
                                       obj->cur->geometry.y,
                                       obj->cur->geometry.w,
                                       obj->cur->geometry.h))
                    continue;
                  in = eina_list_prepend(in, eo_obj);
               }
          }
     }
   return in;
}


EOLIAN static Eina_Iterator*
_evas_canvas_efl_canvas_scene_objects_in_rectangle_get(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, Eina_Rect rect, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   Eina_List *l = _efl_canvas_objects_in_rectangle_get_helper(eo_e, e, rect.x, rect.y, rect.w, rect.h, include_pass_events_objects, include_hidden_objects);
   if (!l) return NULL;
   return efl_canvas_iterator_create(eo_e, eina_list_iterator_new(l), l);
}

EAPI Eina_List *
evas_objects_in_rectangle_get(const Evas_Canvas *eo_e, int x, int y, int w, int h, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   EVAS_LEGACY_API(eo_e, e, NULL);
   return _efl_canvas_objects_in_rectangle_get_helper(eo_e, e, x, y, w, h, include_pass_events_objects, include_hidden_objects);
}

/* font related api */

EOLIAN static void
_evas_canvas_font_path_clear(Eo *eo_e EINA_UNUSED, Evas_Public_Data *evas)
{
   evas_canvas_async_block(evas);
   while (evas->font_path)
     {
  eina_stringshare_del(evas->font_path->data);
  evas->font_path = eina_list_remove(evas->font_path, evas->font_path->data);
     }
}

EOLIAN static void
_evas_canvas_font_path_append(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, const char *path)
{
   if (!path) return;
   evas_canvas_async_block(e);
   e->font_path = eina_list_append(e->font_path, eina_stringshare_add(path));
}

EOLIAN static void
_evas_canvas_font_path_prepend(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, const char *path)
{
   if (!path) return;
   evas_canvas_async_block(e);
   e->font_path = eina_list_prepend(e->font_path, eina_stringshare_add(path));
}

EOLIAN static const Eina_List*
_evas_canvas_font_path_list(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->font_path;
}

EOLIAN static void
_evas_canvas_font_cache_flush(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   evas_canvas_async_block(e);
   evas_render_rendering_wait(e);
   if (_evas_engine_context(e))
     e->engine.func->font_cache_flush(_evas_engine_context(e));
}

EOLIAN static void
_evas_canvas_font_cache_set(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, int size)
{
   if (size < 0) size = 0;
   evas_canvas_async_block(e);
   evas_render_rendering_wait(e);
   if (_evas_engine_context(e))
     e->engine.func->font_cache_set(_evas_engine_context(e), size);
}

EOLIAN static int
_evas_canvas_font_cache_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   if (_evas_engine_context(e))
     return e->engine.func->font_cache_get(_evas_engine_context(e));
   return -1;
}

EOLIAN static Eina_List*
_evas_canvas_font_available_list(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *pd)
{
   return evas_font_dir_available_list(pd->font_path);
}

static void
evas_font_object_rehint(Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, EFL_CANVAS_OBJECT_CLASS);
   if (obj->is_smart)
     {
  EINA_INLIST_FOREACH(evas_object_smart_members_get_direct(eo_obj), obj)
    evas_font_object_rehint(obj->object);
     }
   else
     {
  if (!strcmp(obj->type, "text"))
    _evas_object_text_rehint(eo_obj);
  if (!strcmp(obj->type, "textblock"))
    _evas_object_textblock_rehint(eo_obj);
     }
}

EAPI void
evas_font_hinting_set(Eo *eo_e, Evas_Font_Hinting_Flags hinting)
{
   Evas_Layer *lay;

   EVAS_LEGACY_API(eo_e, e);
   evas_canvas_async_block(e);
   if (e->hinting == hinting) return;
   e->hinting = hinting;

   EINA_INLIST_FOREACH(e->layers, lay)
     {
  Evas_Object_Protected_Data *obj;

  EINA_INLIST_FOREACH(lay->objects, obj)
    evas_font_object_rehint(obj->object);
     }
}

EAPI Evas_Font_Hinting_Flags
evas_font_hinting_get(const Evas *eo_e)
{
   EVAS_LEGACY_API(eo_e, e, EVAS_FONT_HINTING_NONE);
   return e->hinting;
}

EAPI Eina_Bool
evas_font_hinting_can_hint(const Evas *eo_e, Evas_Font_Hinting_Flags hinting)
{
   EVAS_LEGACY_API(eo_e, e, EINA_FALSE);
   if (e->engine.func->font_hinting_can_hint && _evas_engine_context(e))
     return e->engine.func->font_hinting_can_hint(_evas_engine_context(e),
                                                  hinting);
   else return EINA_FALSE;
}

EAPI void
evas_font_available_list_free(Evas *eo_e, Eina_List *available)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();

   evas_font_dir_available_list_free(available);
}


EOLIAN static void
_evas_canvas_efl_canvas_scene_smart_objects_calculate(Eo *eo_e, Evas_Public_Data *o EINA_UNUSED)
{
   evas_call_smarts_calculate(eo_e);
}

EAPI void
evas_smart_objects_calculate(Eo *eo_e)
{
   evas_call_smarts_calculate(eo_e);
}

EOLIAN Eina_Bool
_evas_canvas_efl_canvas_scene_smart_objects_calculating_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return !!e->in_smart_calc;
}

EAPI Eina_Bool
evas_smart_objects_calculating_get(const Eo *obj)
{
   return efl_canvas_scene_smart_objects_calculating_get(obj);
}

EOLIAN int
_evas_canvas_smart_objects_calculate_count_get(const Eo *eo_e EINA_UNUSED, Evas_Public_Data *e)
{
   return e->smart_calc_count;
}
/* Legacy EAPI */

EAPI Eina_Bool
evas_pointer_inside_get(const Evas *obj)
{
   return efl_canvas_pointer_inside_get(obj, NULL);
}

EAPI Eina_Bool
evas_pointer_inside_by_device_get(const Evas *obj, Eo *dev)
{
   return efl_canvas_pointer_inside_get(obj, dev);
}

EAPI Eina_List*
evas_objects_at_xy_get(Eo *eo_e, int x, int y, Eina_Bool include_pass_events_objects, Eina_Bool include_hidden_objects)
{
   return _efl_canvas_evas_canvas_objects_at_xy_get_helper(eo_e, efl_data_scope_get(eo_e, EVAS_CANVAS_CLASS), x, y, include_pass_events_objects, include_hidden_objects);
}
/* Internal EO APIs */

EWAPI const Efl_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_PRE =
   EFL_EVENT_DESCRIPTION("render,flush,pre");
EWAPI const Efl_Event_Description _EVAS_CANVAS_EVENT_RENDER_FLUSH_POST =
   EFL_EVENT_DESCRIPTION("render,flush,post");
EWAPI const Efl_Event_Description _EVAS_CANVAS_EVENT_AXIS_UPDATE =
   EFL_EVENT_DESCRIPTION("axis,update");
EWAPI const Efl_Event_Description _EVAS_CANVAS_EVENT_VIEWPORT_RESIZE =
   EFL_EVENT_DESCRIPTION("viewport,resize");

#include "evas_stack.x"
#include "canvas/evas_canvas.eo.c"
