#include "evas_common.h"
#include "evas_private.h"

#define CSO_DATA_GET_OR_RETURN(obj, ptr)				 \
  Evas_Object_Smart_Clipped_Data *ptr = evas_object_smart_data_get(obj); \
  if (!ptr) return;

EAPI void
evas_object_smart_move_children_relative(Evas_Object *obj, Evas_Coord dx, Evas_Coord dy)
{
   const Eina_Inlist *lst;
   Evas_Object *child;

   if ((dx == 0) && (dy == 0))
     return;

   lst = evas_object_smart_members_get_direct(obj);
   EINA_INLIST_FOREACH(lst, child)
     {
	Evas_Coord orig_x, orig_y;

// shortcut as we are in evas        
//	evas_object_geometry_get(child, &orig_x, &orig_y, NULL, NULL);
        if (child->delete_me) continue;
        if (child->is_static_clip) continue;
        orig_x = child->cur.geometry.x;
        orig_y = child->cur.geometry.y;
	evas_object_move(child, orig_x + dx, orig_y + dy);
     }
}

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
     cso = calloc(1, sizeof(*cso)); /* users can provide it or realloc() later */

   cso->evas = evas_object_evas_get(obj);
   clipper = evas_object_rectangle_add(cso->evas);
   evas_object_static_clip_set(clipper, 1);
   cso->clipper = NULL;
   evas_object_smart_member_add(clipper, obj);
   cso->clipper = clipper;
   evas_object_color_set(cso->clipper, 255, 255, 255, 255);
   evas_object_move(cso->clipper, -100000, -100000);
   evas_object_resize(cso->clipper, 200000, 200000);
   evas_object_pass_events_set(cso->clipper, 1);
   evas_object_hide(cso->clipper); /* show when have something clipped to it */

   evas_object_smart_data_set(obj, cso);
}

static void
evas_object_smart_clipped_smart_del(Evas_Object *obj)
{
   CSO_DATA_GET_OR_RETURN(obj, cso);

   if (cso->clipper)
     {
	Evas_Object *clipper = cso->clipper;
	cso->clipper = NULL;
	evas_object_del(clipper);
     }

   _evas_object_smart_members_all_del(obj);

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

EAPI const Evas_Smart_Class *
evas_object_smart_clipped_class_get(void)
{
   static Evas_Smart_Class _sc = EVAS_SMART_CLASS_INIT_NAME_VERSION("EvasObjectSmartClipped");
   static const Evas_Smart_Class *class = NULL;

   if (class)
     return class;

   evas_object_smart_clipped_smart_set(&_sc);
   class = &_sc;
   return class;
}
