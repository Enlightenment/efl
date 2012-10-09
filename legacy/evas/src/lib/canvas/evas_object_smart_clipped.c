#include "evas_common.h"
#include "evas_private.h"

EAPI Eo_Op EVAS_OBJ_SMART_CLIPPED_BASE_ID = EO_NOOP;

#define MY_CLASS EVAS_OBJ_SMART_CLIPPED_CLASS

#define CSO_DATA_GET(eo_obj, ptr)                                           \
  Evas_Object_Smart_Clipped_Data *ptr = (eo_isa(eo_obj, MY_CLASS) ?         \
        eo_data_get(eo_obj, MY_CLASS) :                                     \
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

        // shortcut as we are in evas        
        // evas_object_geometry_get(child, &orig_x, &orig_y, NULL, NULL);
        if (child->delete_me) continue;
        if (child->is_static_clip) continue;
        orig_x = child->cur.geometry.x;
        orig_y = child->cur.geometry.y;
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

static void
_smart_add(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
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

   evas_object_smart_data_set(eo_obj, NULL);
}

static void
_smart_del(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
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

static void
_smart_move(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Coord x = va_arg(*list, Evas_Coord);
   Evas_Coord y = va_arg(*list, Evas_Coord);
   evas_object_smart_clipped_smart_move(eo_obj, x, y);
}

static void
evas_object_smart_clipped_smart_show(Evas_Object *eo_obj)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   if (evas_object_clipees_get(cso->clipper))
     evas_object_show(cso->clipper); /* just show if clipper being used */
}

static void
_smart_show(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   evas_object_smart_clipped_smart_show(eo_obj);
}

static void
evas_object_smart_clipped_smart_hide(Evas_Object *eo_obj)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   evas_object_hide(cso->clipper);
}

static void
_smart_hide(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
{
   evas_object_smart_clipped_smart_hide(eo_obj);
}

static void
evas_object_smart_clipped_smart_color_set(Evas_Object *eo_obj, int r, int g, int b, int a)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   evas_object_color_set(cso->clipper, r, g, b, a);
}

static void
_smart_color_set(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   int r = va_arg(*list, int);
   int g = va_arg(*list, int);
   int b = va_arg(*list, int);
   int a = va_arg(*list, int);
   evas_object_smart_clipped_smart_color_set(eo_obj, r, g, b, a);
}

static void
evas_object_smart_clipped_smart_clip_set(Evas_Object *eo_obj, Evas_Object *clip)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   evas_object_clip_set(cso->clipper, clip);
}

static void
_smart_clip_set(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *clip = va_arg(*list, Evas_Object *);
   evas_object_smart_clipped_smart_clip_set(eo_obj, clip);
}

static void
evas_object_smart_clipped_smart_clip_unset(Evas_Object *eo_obj)
{
   CSO_DATA_GET_OR_RETURN(eo_obj, cso);
   evas_object_clip_unset(cso->clipper);
}

static void
_smart_clip_unset(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list EINA_UNUSED)
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

static void
_smart_member_add(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *member = va_arg(*list, Evas_Object *);
   eo_do_super(eo_obj, evas_obj_smart_member_add(member));
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

static void
_smart_member_del(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *member = va_arg(*list, Evas_Object *);
   evas_object_smart_clipped_smart_member_del(eo_obj, member);
   eo_do_super(eo_obj, evas_obj_smart_member_del(member));
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

static void
_type_check(Eo *eo_obj, void *_pd EINA_UNUSED, va_list *list)
{
   const char *type = va_arg(*list, const char *);
   Eina_Bool *type_check = va_arg(*list, Eina_Bool *);
   if (0 == strcmp(type, "EvasObjectSmartClipped"))
      *type_check = EINA_TRUE;
   else
      eo_do_super(eo_obj, evas_obj_type_check(type, type_check));
}

static void
_constructor(Eo *eo_obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(eo_obj, eo_constructor());
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EVAS_OBJ_ID(EVAS_OBJ_SUB_ID_TYPE_CHECK), _type_check),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_ADD), _smart_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_DEL), _smart_del),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MOVE), _smart_move),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_SHOW), _smart_show),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_HIDE), _smart_hide),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_COLOR_SET), _smart_color_set),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CLIP_SET), _smart_clip_set),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_CLIP_UNSET), _smart_clip_unset),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_ADD), _smart_member_add),
        EO_OP_FUNC(EVAS_OBJ_SMART_ID(EVAS_OBJ_SMART_SUB_ID_MEMBER_DEL), _smart_member_del),
        EO_OP_FUNC_SENTINEL
   };

   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Evas_Object_Smart_Clipped",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&EVAS_OBJ_SMART_CLIPPED_BASE_ID, op_desc, EVAS_OBJ_SMART_CLIPPED_SUB_ID_LAST),
     NULL,
     sizeof(Evas_Object_Smart_Clipped_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(evas_object_smart_clipped_eo_class_get, &class_desc, EVAS_OBJ_SMART_CLASS, NULL);

