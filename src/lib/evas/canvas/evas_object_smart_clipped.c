#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS EVAS_OBJ_SMART_CLIPPED_CLASS

#define CSO_DATA_GET(eo_obj, ptr)                                           \
  Evas_Object_Smart_Clipped_Data *ptr = (eo_isa(eo_obj, MY_CLASS) ?         \
        eo_data_scope_get(eo_obj, MY_CLASS) :                               \
        evas_object_smart_data_get(eo_obj));

#define CSO_DATA_GET_OR_RETURN(eo_obj, ptr)                                 \
  CSO_DATA_GET(eo_obj, ptr)						 \
  if (!ptr) return;

EAPI void
evas_object_smart_move_children_relative(Evas_Object *eo_obj, Evas_Coord dx, Evas_Coord dy)
{
   eo_do(eo_obj, evas_obj_smart_move_children_relative(dx, dy));
}

void
_smart_move_children_relative(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord dx = va_arg(*list, Evas_Coord);
   Evas_Coord dy = va_arg(*list, Evas_Coord);

   const Eina_Inlist *lst;
   Evas_Object_Protected_Data *child;

   if ((dx == 0) && (dy == 0))
     return;

   lst = evas_object_smart_members_get_direct(eo_obj);
   EINA_INLIST_FOREACH(lst, child)
     {
        Evas_Coord orig_x, orig_y;

        if (child->delete_me) continue;
        if (child->is_static_clip) continue;
        orig_x = child->cur->geometry.x;
        orig_y = child->cur->geometry.y;
	evas_object_move(child->object, orig_x + dx, orig_y + dy);
     }
}

EAPI Evas_Object *
evas_object_smart_clipped_clipper_get(Evas_Object *eo_obj)
{
   Evas_Object *ret = NULL;
   eo_do((Eo *)eo_obj, evas_obj_smart_clipped_clipper_get(&ret));
   return ret;
}

void
_smart_clipped_clipper_get(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   CSO_DATA_GET(eo_obj, cso)
   if (!cso)
      *ret = NULL;
   else
      *ret = cso->clipper;
}

static void
evas_object_smart_clipped_smart_add(Evas_Object *eo_obj)
{
   Evas_Object *clipper;

   CSO_DATA_GET(eo_obj, cso)
   if (!cso)
     cso = calloc(1, sizeof(*cso)); /* users can provide it or realloc() later */

   cso->evas = evas_object_evas_get(eo_obj);
   clipper = evas_object_rectangle_add(cso->evas);
   evas_object_static_clip_set(clipper, 1);
   cso->clipper = NULL;
   evas_object_smart_member_add(clipper, eo_obj);
   cso->clipper = clipper;
   evas_object_color_set(cso->clipper, 255, 255, 255, 255);
   evas_object_move(cso->clipper, -100000, -100000);
   evas_object_resize(cso->clipper, 200000, 200000);
   evas_object_pass_events_set(cso->clipper, 1);
   evas_object_hide(cso->clipper); /* show when have something clipped to it */

   evas_object_smart_data_set(eo_obj, cso);
}

EOLIAN static void
_evas_smart_clipped_evas_smart_add(Eo *eo_obj, Evas_Object_Smart_Clipped_Data *_pd EINA_UNUSED)
{
   evas_object_smart_clipped_smart_add(eo_obj);
}

static void
evas_object_smart_clipped_smart_del(Evas_Object *eo_obj)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);

   if (cso->clipper)
     {
        Evas_Object *clipper = cso->clipper;
        cso->clipper = NULL;
        evas_object_del(clipper);
     }

   _evas_object_smart_members_all_del(eo_obj);

   /* If it's a legacy smart object, we should free the cso. */
   if (!eo_isa(eo_obj, MY_CLASS))
      free(cso);

   evas_object_smart_data_set(eo_obj, NULL);
}

EOLIAN static void
_evas_smart_clipped_evas_smart_del(Eo *eo_obj, Evas_Object_Smart_Clipped_Data *_pd EINA_UNUSED)
{
   evas_object_smart_clipped_smart_del(eo_obj);
}

static void
evas_object_smart_clipped_smart_move(Evas_Object *eo_obj, Evas_Coord x, Evas_Coord y)
{
   Evas_Coord orig_x, orig_y;

   evas_object_geometry_get(eo_obj, &orig_x, &orig_y, NULL, NULL);
   evas_object_smart_move_children_relative(eo_obj, x - orig_x, y - orig_y);
}

EOLIAN static void
_evas_smart_clipped_evas_smart_move(Eo *eo_obj, Evas_Object_Smart_Clipped_Data *_pd EINA_UNUSED, Evas_Coord x, Evas_Coord y)
{
   evas_object_smart_clipped_smart_move(eo_obj, x, y);
}

static void
evas_object_smart_clipped_smart_show(Evas_Object *eo_obj)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   if (evas_object_clipees_has(cso->clipper))
     evas_object_show(cso->clipper); /* just show if clipper being used */
}

EOLIAN static void
_evas_smart_clipped_evas_smart_show(Eo *eo_obj, Evas_Object_Smart_Clipped_Data *_pd EINA_UNUSED)
{
   evas_object_smart_clipped_smart_show(eo_obj);
}

static void
evas_object_smart_clipped_smart_hide(Evas_Object *eo_obj)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   evas_object_hide(cso->clipper);
}

EOLIAN static void
_evas_smart_clipped_evas_smart_hide(Eo *eo_obj, Evas_Object_Smart_Clipped_Data *_pd EINA_UNUSED)
{
   evas_object_smart_clipped_smart_hide(eo_obj);
}

static void
evas_object_smart_clipped_smart_color_set(Evas_Object *eo_obj, int r, int g, int b, int a)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   evas_object_color_set(cso->clipper, r, g, b, a);
}

EOLIAN static void
_evas_smart_clipped_evas_smart_color_set(Eo *eo_obj, Evas_Object_Smart_Clipped_Data *_pd EINA_UNUSED, int r, int g, int b, int a)
{
   evas_object_smart_clipped_smart_color_set(eo_obj, r, g, b, a);
}

static void
evas_object_smart_clipped_smart_clip_set(Evas_Object *eo_obj, Evas_Object *clip)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   evas_object_clip_set(cso->clipper, clip);
}

EOLIAN static void
_evas_smart_clipped_evas_smart_clip_set(Eo *eo_obj, Evas_Object_Smart_Clipped_Data *_pd EINA_UNUSED, Evas_Object *clip)
{
   evas_object_smart_clipped_smart_clip_set(eo_obj, clip);
}

static void
evas_object_smart_clipped_smart_clip_unset(Evas_Object *eo_obj)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   evas_object_clip_unset(cso->clipper);
}

EOLIAN static void
_evas_smart_clipped_evas_smart_clip_unset(Eo *eo_obj, Evas_Object_Smart_Clipped_Data *_pd EINA_UNUSED)
{
   evas_object_smart_clipped_smart_clip_unset(eo_obj);
}

static void
evas_object_smart_clipped_smart_member_add(Evas_Object *eo_obj, Evas_Object *member)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   if (!cso->clipper || member == cso->clipper)
     return;
   evas_object_clip_set(member, cso->clipper);
   if (evas_object_visible_get(eo_obj))
     evas_object_show(cso->clipper);
}

EOLIAN static void
_evas_smart_clipped_evas_smart_member_add(Eo *eo_obj, Evas_Object_Smart_Clipped_Data *_pd EINA_UNUSED, Evas_Object *member)
{
   eo_do_super(eo_obj, MY_CLASS, evas_obj_smart_member_add(member));
   evas_object_smart_clipped_smart_member_add(eo_obj, member);
}

static void
evas_object_smart_clipped_smart_member_del(Evas_Object *eo_obj, Evas_Object *member)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   if (!cso->clipper)
     return;
   evas_object_clip_unset(member);
   if (!evas_object_clipees_get(cso->clipper))
     evas_object_hide(cso->clipper);
}

EOLIAN static void
_evas_smart_clipped_evas_smart_member_del(Eo *eo_obj, Evas_Object_Smart_Clipped_Data *_pd EINA_UNUSED, Evas_Object *member)
{
   evas_object_smart_clipped_smart_member_del(eo_obj, member);
   eo_do_super(eo_obj, MY_CLASS, evas_obj_smart_member_del(member));
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

EOLIAN static void
_evas_smart_clipped_constructor(Eo *eo_obj, Evas_Object_Smart_Clipped_Data *class_data EINA_UNUSED)
{
   eo_do_super(eo_obj, MY_CLASS, eo_constructor());
}

#include "canvas/evas_smart_clipped.eo.c"
