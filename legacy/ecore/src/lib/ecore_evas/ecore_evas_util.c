#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_evas_private.h"
#include "Ecore_Evas.h"


static const char ASSOCIATE_KEY[] = "__Ecore_Evas_Associate";

static void _ecore_evas_object_associate(Ecore_Evas *ee, Evas_Object *obj, Ecore_Evas_Object_Associate_Flags flags);
static void _ecore_evas_object_dissociate(Ecore_Evas *ee, Evas_Object *obj);


static Evas_Object *
_ecore_evas_associate_get(Ecore_Evas *ee)
{
   return ecore_evas_data_get(ee, ASSOCIATE_KEY);
}

static void
_ecore_evas_associate_set(Ecore_Evas *ee, Evas_Object *obj)
{
   ecore_evas_data_set(ee, ASSOCIATE_KEY, obj);
}

static void
_ecore_evas_associate_del(Ecore_Evas *ee)
{
   ecore_evas_data_set(ee, ASSOCIATE_KEY, NULL);
}

static Ecore_Evas *
_evas_object_associate_get(Evas_Object *obj)
{
   return evas_object_data_get(obj, ASSOCIATE_KEY);
}

static void
_evas_object_associate_set(Evas_Object *obj, Ecore_Evas *ee)
{
   evas_object_data_set(obj, ASSOCIATE_KEY, ee);
}

static void
_evas_object_associate_del(Evas_Object *obj)
{
   evas_object_data_del(obj, ASSOCIATE_KEY);
}

/** Associated Events: ******************************************************/

/* Interceptors Callbacks */

static void
_ecore_evas_obj_intercept_move(void *data, Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Ecore_Evas *ee = data;
   // FIXME: account for frame
   ecore_evas_move(ee, x, y);
}

static void
_ecore_evas_obj_intercept_raise(void *data, Evas_Object *obj)
{
   Ecore_Evas *ee = data;
   ecore_evas_raise(ee);
}

static void
_ecore_evas_obj_intercept_lower(void *data, Evas_Object *obj)
{
   Ecore_Evas *ee = data;
   ecore_evas_lower(ee);
}

static void
_ecore_evas_obj_intercept_stack_above(void *data, Evas_Object *obj, Evas_Object *above)
{
   fprintf(stderr, "TODO: %s\n", __FUNCTION__);
}

static void
_ecore_evas_obj_intercept_stack_below(void *data, Evas_Object *obj, Evas_Object *below)
{
   fprintf(stderr, "TODO: %s\n", __FUNCTION__);
}

static void
_ecore_evas_obj_intercept_layer_set(void *data, Evas_Object *obj, int l)
{
   Ecore_Evas *ee = data;
   ecore_evas_layer_set(ee, l);
}

/* Event Callbacks */

static void
_ecore_evas_obj_callback_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Ecore_Evas *ee = data;
   ecore_evas_show(ee);
}

static void
_ecore_evas_obj_callback_hide(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Ecore_Evas *ee = data;
   ecore_evas_hide(ee);
}

static void
_ecore_evas_obj_callback_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Coord ow, oh, w, h;

   evas_object_geometry_get(obj, NULL, NULL, &ow, &oh);
   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   if ((w != ow) || (h != oh)) /* avoid recursion on ecore_evas_resize side */
     ecore_evas_resize(ee, ow, oh);
}

static void
_ecore_evas_obj_callback_changed_size_hints(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Ecore_Evas *ee = data;
   Evas_Coord w, h;

   evas_object_size_hint_min_get(obj, &w, &h);
   ecore_evas_size_min_set(ee, w, h);

   evas_object_size_hint_max_get(obj, &w, &h);
   if (w < 1) w = -1;
   if (h < 1) h = -1;
   ecore_evas_size_max_set(ee, w, h);
}

static void
_ecore_evas_obj_callback_del(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Ecore_Evas *ee = data;
   _ecore_evas_object_dissociate(ee, obj);
   ecore_evas_free(ee);
}

static void
_ecore_evas_delete_request(Ecore_Evas *ee)
{
   Evas_Object *obj = _ecore_evas_associate_get(ee);
   _ecore_evas_object_dissociate(ee, obj);
   evas_object_del(obj);
   ecore_evas_free(ee);
}

static void
_ecore_evas_destroy(Ecore_Evas *ee)
{
   Evas_Object *obj = _ecore_evas_associate_get(ee);
   if (!obj)
     return;
   _ecore_evas_object_dissociate(ee, obj);
   evas_object_del(obj);
}

static void
_ecore_evas_resize(Ecore_Evas *ee)
{
   Evas_Object *obj = _ecore_evas_associate_get(ee);
   Evas_Coord w, h;
   ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);
   evas_object_resize(obj, w, h);
}

static void
_ecore_evas_pre_free(Ecore_Evas *ee)
{
   Evas_Object *obj = _ecore_evas_associate_get(ee);
   if (!obj)
     return;
   _ecore_evas_object_dissociate(ee, obj);
   evas_object_del(obj);
}

static int
_ecore_evas_object_evas_check(const char *function, const Ecore_Evas *ee, const Evas_Object *obj)
{
   const char *name, *type;
   Evas *e;

   e = evas_object_evas_get(obj);
   if (e == ee->evas)
     return 1;

   name = evas_object_name_get(obj);
   type = evas_object_type_get(obj);

   fprintf(stderr,
	   "ERROR: %s(): object %p (name=\"%s\", type=\"%s\") evas "
	   "is not the same as this Ecore_Evas evas: %p != %p\n",
	   function, obj,
	   name ? name : "", type ? type : "", e, ee->evas);
   fflush(stderr);
   if (getenv("ECORE_ERROR_ABORT")) abort();

   return 0;
}

/**
 * Associate the given object to this ecore evas.
 *
 * Association means that operations on one will affect the other, for
 * example moving the object will move the window, resize the object will
 * also affect the ecore evas window, hide and show applies as well.
 *
 * This is meant to simplify development, since you often need to associate
 * these events with your "base" objects, background or bottom-most object.
 *
 * Be aware that some methods might not be what you would like, deleting
 * either the window or the object will delete the other. If you want to
 * change that behavior, let's say to hide window when it's closed, you
 * must use ecore_evas_callback_delete_request_set() and set your own code,
 * like ecore_evas_hide(). Just remember that if you override delete_request
 * and still want to delete the window/object, you must do that yourself.
 *
 * Since we now define delete_request, deleting windows will not quit
 * main loop, if you wish to do so, you should listen for EVAS_CALLBACK_FREE
 * on the object, that way you get notified and you can call
 * ecore_main_loop_quit().
 *
 * Flags can be OR'ed of:
 *   - ECORE_EVAS_OBJECT_ASSOCIATE_BASE (or 0): to listen to basic events
 *     like delete, resize and move, but no stacking or layer are used.
 *   - ECORE_EVAS_OBJECT_ASSOCIATE_STACK: stacking operations will act
 *     on the Ecore_Evas, not the object. So evas_object_raise() will
 *     call ecore_evas_raise(). Relative operations (stack_above, stack_below)
 *     are still not implemented.
 *   - ECORE_EVAS_OBJECT_ASSOCIATE_LAYER: stacking operations will act
 *     on the Ecore_Evas, not the object. So evas_object_layer_set() will
 *     call ecore_evas_layer_set().
 *
 * @param ee The Ecore_Evas to associate to @a obj
 * @param obj The object to associate to @a ee
 * @return 1 on success, 0 otherwise.
 */
EAPI int
ecore_evas_object_associate(Ecore_Evas *ee, Evas_Object *obj, Ecore_Evas_Object_Associate_Flags flags)
{
   Ecore_Evas *old_ee;
   Evas_Object *old_obj;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
   {
      ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, __FUNCTION__);
      return 0;
   }

   CHECK_PARAM_POINTER_RETURN("obj", obj, 0);
   if (!_ecore_evas_object_evas_check(__FUNCTION__, ee, obj))
     return 0;

   old_ee = _evas_object_associate_get(obj);;
   if (old_ee)
     ecore_evas_object_dissociate(old_ee, obj);

   old_obj = _ecore_evas_associate_get(ee);
   if (old_obj)
     ecore_evas_object_dissociate(ee, old_obj);

   _ecore_evas_object_associate(ee, obj, flags);
   return 1;
}

/**
 * Cancel the association set with ecore_evas_object_associate().
 *
 * @param ee The Ecore_Evas to dissociate from @a obj
 * @param obj The object to dissociate from @a ee
 * @return 1 on success, 0 otherwise.
 */
EAPI int
ecore_evas_object_dissociate(Ecore_Evas *ee, Evas_Object *obj)
{
   Ecore_Evas *old_ee;
   Evas_Object *old_obj;

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
   {
      ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, __FUNCTION__);
      return 0;
   }

   CHECK_PARAM_POINTER_RETURN("obj", obj, 0);
   old_ee = _evas_object_associate_get(obj);
   if (ee != old_ee) {
      fprintf(stderr, "ERROR: trying to dissociate object that is not using "
	      "this Ecore_Evas: %p != %p\n", ee, old_ee);
      return 0;
   }

   old_obj = _ecore_evas_associate_get(ee);
   if (old_obj != obj) {
      fprintf(stderr, "ERROR: trying to dissociate object that is not being "
	      "used by this Ecore_Evas: %p != %p\n", old_obj, obj);
      return 0;
   }

   _ecore_evas_object_dissociate(ee, obj);

   return 1;
}

EAPI Evas_Object *
ecore_evas_object_associate_get(Ecore_Evas *ee)
{
   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
   {
      ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, __FUNCTION__);
      return NULL;
   }
   return _ecore_evas_associate_get(ee);
}

static void
_ecore_evas_object_associate(Ecore_Evas *ee, Evas_Object *obj, Ecore_Evas_Object_Associate_Flags flags)
{
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_SHOW,
      _ecore_evas_obj_callback_show, ee);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_HIDE,
      _ecore_evas_obj_callback_hide, ee);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE,
      _ecore_evas_obj_callback_resize, ee);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
      _ecore_evas_obj_callback_changed_size_hints, ee);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_DEL, _ecore_evas_obj_callback_del, ee);

   evas_object_intercept_move_callback_add
     (obj, _ecore_evas_obj_intercept_move, ee);

   if (flags & ECORE_EVAS_OBJECT_ASSOCIATE_STACK)
   {
      evas_object_intercept_raise_callback_add
	(obj, _ecore_evas_obj_intercept_raise, ee);
      evas_object_intercept_lower_callback_add
	(obj, _ecore_evas_obj_intercept_lower, ee);
      evas_object_intercept_stack_above_callback_add
	(obj, _ecore_evas_obj_intercept_stack_above, ee);
      evas_object_intercept_stack_below_callback_add
	(obj, _ecore_evas_obj_intercept_stack_below, ee);
   }

   if (flags & ECORE_EVAS_OBJECT_ASSOCIATE_LAYER)
     evas_object_intercept_layer_set_callback_add
       (obj, _ecore_evas_obj_intercept_layer_set, ee);

   ecore_evas_callback_delete_request_set(ee, _ecore_evas_delete_request);
   ecore_evas_callback_destroy_set(ee, _ecore_evas_destroy);
   ecore_evas_callback_resize_set(ee, _ecore_evas_resize);
   ecore_evas_callback_pre_free_set(ee, _ecore_evas_pre_free);

   _evas_object_associate_set(obj, ee);
   _ecore_evas_associate_set(ee, obj);
}

static void
_ecore_evas_object_dissociate(Ecore_Evas *ee, Evas_Object *obj)
{
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_SHOW,
      _ecore_evas_obj_callback_show, ee);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_HIDE,
      _ecore_evas_obj_callback_hide, ee);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_RESIZE,
      _ecore_evas_obj_callback_resize, ee);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS,
      _ecore_evas_obj_callback_changed_size_hints, ee);
   evas_object_event_callback_del_full
     (obj, EVAS_CALLBACK_DEL, _ecore_evas_obj_callback_del, ee);

   evas_object_intercept_move_callback_del
     (obj, _ecore_evas_obj_intercept_move);

   evas_object_intercept_raise_callback_del
     (obj, _ecore_evas_obj_intercept_raise);
   evas_object_intercept_lower_callback_del
     (obj, _ecore_evas_obj_intercept_lower);
   evas_object_intercept_stack_above_callback_del
     (obj, _ecore_evas_obj_intercept_stack_above);
   evas_object_intercept_stack_below_callback_del
     (obj, _ecore_evas_obj_intercept_stack_below);

   evas_object_intercept_layer_set_callback_del
     (obj, _ecore_evas_obj_intercept_layer_set);

   if (!ECORE_MAGIC_CHECK(ee, ECORE_MAGIC_EVAS))
   {
      ECORE_MAGIC_FAIL(ee, ECORE_MAGIC_EVAS, __FUNCTION__);
   }
   else
   {
      if (ee->func.fn_delete_request == _ecore_evas_delete_request)
	ecore_evas_callback_delete_request_set(ee, NULL);
      if (ee->func.fn_destroy == _ecore_evas_destroy)
	ecore_evas_callback_destroy_set(ee, NULL);
      if (ee->func.fn_resize == _ecore_evas_resize)
	ecore_evas_callback_resize_set(ee, NULL);
      if (ee->func.fn_pre_free == _ecore_evas_pre_free)
	ecore_evas_callback_pre_free_set(ee, NULL);

      _ecore_evas_associate_del(ee);
   }

   _evas_object_associate_del(obj);
}
