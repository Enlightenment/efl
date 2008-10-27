#include "evas_common.h"
#include "evas_private.h"

/**
 * @addtogroup Evas_Smart_Object_Group
 * @{
 */

#define CSO_DATA_GET_OR_RETURN(obj, ptr)				 \
  Evas_Object_Smart_Clipped_Data *ptr = evas_object_smart_data_get(obj); \
  if (!ptr) return;

/**
 * Moves all children objects relative to given offset.
 *
 * @param obj the smart evas object to use.
 * @param dx horizontal offset.
 * @param dy vertical offset.
 */
EAPI void
evas_object_smart_move_children_relative(Evas_Object *obj, Evas_Coord dx, Evas_Coord dy)
{
   Eina_List *lst, *itr;
   Evas_Object *child;

   if ((dx == 0) && (dy == 0))
     return;

   lst = evas_object_smart_members_get(obj);
   EINA_LIST_FOREACH(lst, itr, child)
     {
	Evas_Coord orig_x, orig_y;

	evas_object_geometry_get(child, &orig_x, &orig_y, NULL, NULL);
	evas_object_move(child, orig_x + dx, orig_y + dy);
     }

   eina_list_free(lst);
}

/**
 * Get the clipper object for the given clipped smart object.
 *
 * @param obj the clipped smart object to retrieve the associated clipper.
 * @return the clipper object.
 *
 * @see evas_object_smart_clipped_smart_add()
 */
EAPI Evas_Object *
evas_object_smart_clipped_clipper_get(Evas_Object *obj)
{
   Evas_Object_Smart_Clipped_Data *cso = evas_object_smart_data_get(obj);
   if (!cso)
     return NULL;

   return cso->clipper;
}

static void
evas_object_smart_clipped_smart_add(Evas_Object *obj)
{
   Evas_Object_Smart_Clipped_Data *cso;
   Evas_Object *clipper;

   cso = evas_object_smart_data_get(obj);
   if (!cso)
     cso = malloc(sizeof(*cso)); /* users can provide it or realloc() later */

   cso->evas = evas_object_evas_get(obj);
   clipper = evas_object_rectangle_add(cso->evas);
   cso->clipper = NULL;
   evas_object_smart_member_add(clipper, obj);
   cso->clipper = clipper;
   evas_object_color_set(cso->clipper, 255, 255, 255, 255);
   evas_object_move(cso->clipper, -10000, -10000);
   evas_object_resize(cso->clipper, 20000, 20000);
   evas_object_pass_events_set(cso->clipper, 1);
   evas_object_hide(cso->clipper); /* show when have something clipped to it */

   evas_object_smart_data_set(obj, cso);
}

static void
evas_object_smart_clipped_smart_del(Evas_Object *obj)
{
   CSO_DATA_GET_OR_RETURN(obj, cso);
   Eina_List *lst, *itr;
   Evas_Object *data;

   if (cso->clipper)
     {
	Evas_Object *clipper = cso->clipper;
	cso->clipper = NULL;
	evas_object_del(clipper);
     }

   lst = evas_object_smart_members_get(obj);
   EINA_LIST_FOREACH(lst, itr, data)
     evas_object_del(data);
   eina_list_free(lst);

   free(cso);
   evas_object_smart_data_set(obj, NULL);
}

static void
evas_object_smart_clipped_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Coord orig_x, orig_y;

   evas_object_geometry_get(obj, &orig_x, &orig_y, NULL, NULL);
   evas_object_smart_move_children_relative(obj, x - orig_x, y - orig_y);
}

static void
evas_object_smart_clipped_smart_show(Evas_Object *obj)
{
   CSO_DATA_GET_OR_RETURN(obj, cso);
   if (evas_object_clipees_get(cso->clipper))
     evas_object_show(cso->clipper); /* just show if clipper being used */
}

static void
evas_object_smart_clipped_smart_hide(Evas_Object *obj)
{
   CSO_DATA_GET_OR_RETURN(obj, cso);
   evas_object_hide(cso->clipper);
}

static void
evas_object_smart_clipped_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   CSO_DATA_GET_OR_RETURN(obj, cso);
   evas_object_color_set(cso->clipper, r, g, b, a);
}

static void
evas_object_smart_clipped_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
   CSO_DATA_GET_OR_RETURN(obj, cso);
   evas_object_clip_set(cso->clipper, clip);
}

static void
evas_object_smart_clipped_smart_clip_unset(Evas_Object *obj)
{
   CSO_DATA_GET_OR_RETURN(obj, cso);
   evas_object_clip_unset(cso->clipper);
}

static void
evas_object_smart_clipped_smart_member_add(Evas_Object *obj, Evas_Object *member)
{
   CSO_DATA_GET_OR_RETURN(obj, cso);
   if (!cso->clipper)
     return;
   evas_object_clip_set(member, cso->clipper);
   if (evas_object_visible_get(obj))
     evas_object_show(cso->clipper);
}

static void
evas_object_smart_clipped_smart_member_del(Evas_Object *obj, Evas_Object *member)
{
   CSO_DATA_GET_OR_RETURN(obj, cso);
   if (!cso->clipper)
     return;
   evas_object_clip_unset(member);
   if (!evas_object_clipees_get(cso->clipper))
     evas_object_hide(cso->clipper);
}

/**
 * Set smart class callbacks so it implements the "Clipped Smart Object".
 *
 * This call will assign all the required methods of Evas_Smart_Class,
 * if one wants to "subclass" it, call this function and later
 * override values, if one wants to call the original method, save it
 * somewhere, example:
 *
 * @code
 * static Evas_Smart_Class parent_sc = {NULL};
 *
 * static void my_class_smart_add(Evas_Object *o)
 * {
 *    parent_sc.add(o);
 *    evas_object_color_set(evas_object_smart_clipped_clipper_get(o),
 *                          255, 0, 0, 255);
 * }
 *
 * Evas_Smart_Class *my_class_new(void)
 * {
 *    static Evas_Smart_Class sc = {"MyClass"};
 *    if (parent_sc.name)
 *      {
 *         evas_object_smart_clipped_smart_set(&sc);
 *         parent_sc = sc;
 *         sc.add = my_class_smart_add;
 *      }
 *    return &sc;
 * }
 * @endcode
 *
 * Default behavior is:
 *  - add: creates a hidden clipper with "infinite" size;
 *  - del: delete all children objects;
 *  - move: move all objects relative relatively;
 *  - resize: not defined;
 *  - show: if there are children objects, show clipper;
 *  - hide: hides clipper;
 *  - color_set: set the color of clipper;
 *  - clip_set: set clipper of clipper;
 *  - clip_unset: unset the clipper of clipper;
 */
EAPI void
evas_object_smart_clipped_smart_set(Evas_Smart_Class *sc)
{
   if (!sc)
     return;

   sc->add = evas_object_smart_clipped_smart_add;
   sc->del = evas_object_smart_clipped_smart_del;
   sc->move = evas_object_smart_clipped_smart_move;
   sc->show = evas_object_smart_clipped_smart_show;
   sc->hide = evas_object_smart_clipped_smart_hide;
   sc->color_set = evas_object_smart_clipped_smart_color_set;
   sc->clip_set = evas_object_smart_clipped_smart_clip_set;
   sc->clip_unset = evas_object_smart_clipped_smart_clip_unset;
   sc->calculate = NULL;
   sc->member_add = evas_object_smart_clipped_smart_member_add;
   sc->member_del = evas_object_smart_clipped_smart_member_del;
}

/**
 * @}
 */
