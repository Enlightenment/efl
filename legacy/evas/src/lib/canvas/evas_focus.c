#include "evas_common.h"
#include "evas_private.h"

/* private calls */

/* local calls */

/* public calls */

/**
 * Sets focus to the given object.
 * 
 * @param obj The object to be focused or unfocused.
 * @param focus set or remove focus to the object.
 *
 * Changing focus only affects where key events go.
 * There can be only one object focused at any time.
 * <p>
 * If the parameter (@p focus) is set, the passed object will be set as the
 * currently focused object.  It will receive all keyboard events that are not
 * exclusive key grabs on other objects.
 *
 * @see evas_object_focus_get
 * @see evas_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 */
EAPI void
evas_object_focus_set(Evas_Object *obj, Evas_Bool focus)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if (focus)
     {
	if (obj->focused) return;
	if (obj->layer->evas->focused)
	  evas_object_focus_set(obj->layer->evas->focused, 0);
	obj->focused = 1;
	obj->layer->evas->focused = obj;
	evas_object_event_callback_call(obj, EVAS_CALLBACK_FOCUS_IN, NULL);
     }
   else
     {
	if (!obj->focused) return;
	obj->focused = 0;
	obj->layer->evas->focused = NULL;
	evas_object_event_callback_call(obj, EVAS_CALLBACK_FOCUS_OUT, NULL);
     }
}

/**
 * Test if the object has focus.
 *
 * @param obj The object to be tested.
 * 
 * If the passed object is the currently focused object 1 is returned,
 * 0 otherwise.
 * 
 * @see evas_object_focus_set
 * @see evas_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 *
 * @return 1 if the object has the focus, 0 otherwise.
 */
EAPI Evas_Bool
evas_object_focus_get(const Evas_Object *obj)
{
   MAGIC_CHECK(obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();
   return obj->focused;
}

/**
 * Retrieve the object that currently has focus.
 *
 * @param e The @c Evas canvas to query focus on.
 * 
 * Returns the object that currently has focus, NULL otherwise.
 * 
 * @see evas_object_focus_set
 * @see evas_object_focus_get
 * @see evas_object_key_grab
 * @see evas_object_key_ungrab
 *
 * @return The object that has focus or NULL is there is not one.
 */
EAPI Evas_Object *
evas_focus_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return e->focused;
}
