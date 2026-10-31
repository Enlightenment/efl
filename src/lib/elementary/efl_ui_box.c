#include "efl_ui_box_private.h"

#define MY_CLASS EFL_UI_BOX_CLASS
#define MY_CLASS_NAME "Efl.Ui.Box"

/* COPIED FROM ELM_BOX
 * - removed transition stuff (TODO: add back - needs clean API first)
 */

#define EFL_UI_BOX_DATA_GET(o, sd) \
   Efl_Ui_Box_Data *sd = efl_data_scope_get(o, EFL_UI_BOX_CLASS)

void _efl_ui_box_custom_layout(Efl_Ui_Box *ui_box, Efl_Ui_Box_Data *pd);

static void
_on_child_size_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *box = data;
   efl_pack_layout_request(box);
}

static void
_on_child_hints_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Eo *box = data;
   efl_pack_layout_request(box);
}

static void _on_child_invalidate(void *data, const Efl_Event *event);

EFL_CALLBACKS_ARRAY_DEFINE(efl_ui_box_callbacks,
  { EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _on_child_size_changed },
  { EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _on_child_hints_changed },
  { EFL_EVENT_INVALIDATE, _on_child_invalidate }
);

static inline Eina_Bool
_efl_ui_box_child_register(Eo *obj, Efl_Ui_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!subobj || (efl_canvas_object_render_parent_get(subobj) == obj))
     {
        ERR("subobj %p %s is already added to this", subobj, efl_class_name_get(subobj) );
        return EINA_FALSE;
     }

   if (!efl_ui_widget_internal_get(obj))
     {
        if (!efl_ui_widget_sub_object_add(obj, subobj))
          return EINA_FALSE;
     }

   efl_key_data_set(subobj, "_elm_leaveme", obj);
   efl_canvas_group_member_add(obj, subobj);
   efl_canvas_object_clipper_set(subobj, pd->clipper);
   efl_pack_layout_request(obj);

   efl_event_callback_array_add(subobj, efl_ui_box_callbacks(), obj);
   efl_event_callback_call(obj, EFL_CONTAINER_EVENT_CONTENT_ADDED, subobj);

   return EINA_TRUE;
}

/* The box lets go of a child: no longer a member, clipped, or listened to.
   This is shared by the box unpacking the child and by another container
   taking it, which the box hears of as the widget parent losing it. */
static void
_efl_ui_box_child_release(Eo *obj, Efl_Gfx_Entity *subobj)
{
   efl_canvas_group_member_remove(obj, subobj);
   efl_canvas_object_clipper_set(subobj, NULL);
   efl_key_data_set(subobj, "_elm_leaveme", NULL);
   efl_event_callback_array_del(subobj, efl_ui_box_callbacks(), obj);
   if (efl_invalidating_get(obj) || efl_invalidated_get(obj)) return;

   efl_pack_layout_request(obj);
   efl_event_callback_call(obj, EFL_CONTAINER_EVENT_CONTENT_REMOVED, subobj);
}

/* A child deleted while something else still references it is gone from the
   box now, not at the last unref. */
static void
_on_child_invalidate(void *data, const Efl_Event *event)
{
   Eo *box = data;
   EFL_UI_BOX_DATA_GET(box, sd);
   Eina_List *found = eina_list_data_find_list(sd->children, event->object);

   if (!found) return;
   sd->children = eina_list_remove_list(sd->children, found);
   _efl_ui_box_child_release(box, event->object);
}

static inline Eina_Bool
_efl_ui_box_child_unregister(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   if (efl_canvas_object_render_parent_get(subobj) != obj)
     {
        ERR("subobj %p %s is not part of this widget", subobj, efl_class_name_get(subobj) );
        return EINA_FALSE;
     }

   if (!subobj)
     return EINA_FALSE;

   /* Moving the child to the top widget makes the box hear of it through
      widget_sub_object_del, which releases it already. */
   if (!efl_ui_widget_internal_get(obj))
     {
        if (!_elm_widget_sub_object_redirect_to_top(obj, subobj))
          return EINA_FALSE;
     }

   if (efl_canvas_object_render_parent_get(subobj) == obj)
     _efl_ui_box_child_release(obj, subobj);

   return EINA_TRUE;
}

/* A child that another container takes is no longer ours, whatever it
   was told. */
EOLIAN static Eina_Bool
_efl_ui_box_efl_ui_widget_widget_sub_object_del(Eo *obj, Efl_Ui_Box_Data *pd, Efl_Gfx_Entity *sobj)
{
   Eina_Bool ok = efl_ui_widget_sub_object_del(efl_super(obj, MY_CLASS), sobj);
   Eina_List *found = ok ? eina_list_data_find_list(pd->children, sobj) : NULL;

   if (found)
     {
        pd->children = eina_list_remove_list(pd->children, found);
        _efl_ui_box_child_release(obj, sobj);
     }
   return ok;
}

static void
_efl_ui_box_size_hints_changed_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   efl_pack_layout_request(ev->object);
}

EOLIAN static void
_efl_ui_box_homogeneous_set(Eo *obj, Efl_Ui_Box_Data *pd, Eina_Bool homogeneous)
{
   homogeneous = !!homogeneous;

   if (pd->homogeneous == homogeneous)
     return;

   pd->homogeneous = homogeneous;
   efl_pack_layout_request(obj);
}

EOLIAN static Eina_Bool
_efl_ui_box_homogeneous_get(const Eo *obj EINA_UNUSED, Efl_Ui_Box_Data *pd)
{
   return pd->homogeneous;
}

EOLIAN static void
_efl_ui_box_efl_pack_layout_layout_update(Eo *obj, Efl_Ui_Box_Data *pd)
{
   _efl_ui_box_custom_layout(obj, pd);
}

EOLIAN static void
_efl_ui_box_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Box_Data *_pd EINA_UNUSED)
{
   efl_canvas_group_need_recalculate_set(obj, EINA_FALSE);
   efl_pack_layout_update(obj);
}

EOLIAN static void
_efl_ui_box_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Box_Data *_pd EINA_UNUSED, Eina_Size2D sz)
{
   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_box_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Box_Data *_pd EINA_UNUSED, Eina_Position2D pos)
{
   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   efl_canvas_group_change(obj);
}

EOLIAN static void
_efl_ui_box_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Box_Data *pd)
{
   pd->clipper = efl_add(EFL_CANVAS_RECTANGLE_CLASS, obj);
   evas_object_static_clip_set(pd->clipper, EINA_TRUE);
   efl_gfx_entity_geometry_set(pd->clipper, EINA_RECT(-49999, -49999, 99999, 99999));
   efl_canvas_group_member_add(obj, pd->clipper);
   efl_ui_widget_sub_object_add(obj, pd->clipper);

   efl_canvas_group_add(efl_super(obj, MY_CLASS));

   efl_ui_widget_focus_allow_set(obj, EINA_FALSE);
   elm_widget_highlight_ignore_set(obj, EINA_TRUE);

   efl_event_callback_add(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED,
                          _efl_ui_box_size_hints_changed_cb, NULL);
}

EOLIAN static void
_efl_ui_box_efl_canvas_group_group_del(Eo *obj, Efl_Ui_Box_Data *_pd EINA_UNUSED)
{
   efl_event_callback_del(obj, EFL_GFX_ENTITY_EVENT_HINTS_CHANGED,
                          _efl_ui_box_size_hints_changed_cb, NULL);

   efl_canvas_group_del(efl_super(obj, MY_CLASS));
}

EOLIAN static Eo *
_efl_ui_box_efl_object_constructor(Eo *obj, Efl_Ui_Box_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   efl_access_object_access_type_set(obj, EFL_ACCESS_TYPE_SKIPPED);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   pd->dir = EFL_UI_LAYOUT_ORIENTATION_VERTICAL;
   pd->align.h = 0.5;
   pd->align.v = 0.5;
   pd->full_recalc = EINA_TRUE;

   return obj;
}

EOLIAN static void
_efl_ui_box_efl_object_invalidate(Eo *obj, Efl_Ui_Box_Data *pd)
{
   Eo *child;

   efl_invalidate(efl_super(obj, MY_CLASS));

   EINA_LIST_FREE(pd->children, child)
     {
        efl_event_callback_array_del(child, efl_ui_box_callbacks(), obj);
     }
}

/* CLEAN API BELOW */

EOLIAN static int
_efl_ui_box_efl_container_content_count(Eo *obj EINA_UNUSED, Efl_Ui_Box_Data *pd)
{
   return eina_list_count(pd->children);
}

/* The focus manager orders the children of a node as it hears of them, and
   it hears of a box's children as each is shown, in no order the box
   controls. Tab goes through them in the order they are packed in. */
EOLIAN static void
_efl_ui_box_efl_ui_focus_object_setup_order_non_recursive(Eo *obj, Efl_Ui_Box_Data *pd)
{
   Efl_Ui_Focus_Manager *manager = efl_ui_focus_object_focus_manager_get(obj);

   if (manager && pd->children)
     efl_ui_focus_manager_calc_update_order(manager, obj, eina_list_clone(pd->children));
   efl_ui_focus_object_setup_order_non_recursive(efl_super(obj, MY_CLASS));
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_pack_clear(Eo *obj, Efl_Ui_Box_Data *pd)
{
   Eo *child;

   /* Each child leaves the list before it is deleted, as deleting it makes
      the box hear of it through widget_sub_object_del. */
   while ((child = eina_list_data_get(pd->children)))
     {
        pd->children = eina_list_remove_list(pd->children, pd->children);
        efl_event_callback_array_del(child, efl_ui_box_callbacks(), obj);
        efl_del(child);
     }

   efl_pack_layout_request(obj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_unpack_all(Eo *obj, Efl_Ui_Box_Data *pd)
{
   Eo *child;
   Eina_Bool ret = EINA_TRUE;

   while ((child = eina_list_data_get(pd->children)))
     {
        pd->children = eina_list_remove_list(pd->children, pd->children);
        ret &= _efl_ui_box_child_unregister(obj, pd, child);
     }

   return ret;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_unpack(Eo *obj, Efl_Ui_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!_efl_ui_box_child_unregister(obj, pd, subobj))
     return EINA_FALSE;

   pd->children = eina_list_remove(pd->children, subobj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_pack(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED, Efl_Gfx_Entity *subobj)
{
   return efl_pack_end(obj, subobj);
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_linear_pack_end(Eo *obj, Efl_Ui_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!_efl_ui_box_child_register(obj, pd, subobj))
     return EINA_FALSE;

   pd->children = eina_list_append(pd->children, subobj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_linear_pack_begin(Eo *obj, Efl_Ui_Box_Data *pd, Efl_Gfx_Entity *subobj)
{
   if (!_efl_ui_box_child_register(obj, pd, subobj))
     return EINA_FALSE;

   pd->children = eina_list_prepend(pd->children, subobj);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_linear_pack_before(Eo *obj, Efl_Ui_Box_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   if (existing) EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_list_data_find(pd->children, existing), EINA_FALSE);

   if (!_efl_ui_box_child_register(obj, pd, subobj))
     return EINA_FALSE;

   pd->children = eina_list_prepend_relative(pd->children, subobj, existing);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_linear_pack_after(Eo *obj, Efl_Ui_Box_Data *pd, Efl_Gfx_Entity *subobj, const Efl_Gfx_Entity *existing)
{
   if (existing) EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_list_data_find(pd->children, existing), EINA_FALSE);

   if (!_efl_ui_box_child_register(obj, pd, subobj))
     return EINA_FALSE;

   pd->children = eina_list_append_relative(pd->children, subobj, existing);

   return EINA_TRUE;
}

EOLIAN static Eina_Bool
_efl_ui_box_efl_pack_linear_pack_at(Eo *obj, Efl_Ui_Box_Data *pd, Efl_Gfx_Entity *subobj, int index)
{
   int count = eina_list_count(pd->children);

   if (index < -count)
     return efl_pack_begin(obj, subobj);

   if (index >= count)
     return efl_pack_end(obj, subobj);

   if (index < 0)
     index += count;

   if (!_efl_ui_box_child_register(obj, pd, subobj))
     return EINA_FALSE;

   pd->children = eina_list_prepend_relative_list(pd->children, subobj,
                                       eina_list_nth_list(pd->children, index));

   return EINA_TRUE;
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_box_efl_pack_linear_pack_content_get(Eo *obj EINA_UNUSED, Efl_Ui_Box_Data *pd, int index)
{
   int count = eina_list_count(pd->children);

   if (index <= -count)
     return eina_list_data_get(pd->children);

   if (index >= count)
     return eina_list_last_data_get(pd->children);

   if (index < 0)
     index += count;

   return eina_list_nth(pd->children, index);
}

EOLIAN static Efl_Gfx_Entity *
_efl_ui_box_efl_pack_linear_pack_unpack_at(Eo *obj, Efl_Ui_Box_Data *pd EINA_UNUSED, int index)
{
   Efl_Gfx_Entity *content;

   content = efl_pack_content_get(obj, index);
   if (!content) return NULL;

   if (!efl_pack_unpack(obj, content))
     return NULL;

   return content;
}

EOLIAN static int
_efl_ui_box_efl_pack_linear_pack_index_get(Eo *obj EINA_UNUSED, Efl_Ui_Box_Data *pd, const Efl_Gfx_Entity *subobj)
{
   return eina_list_data_idx(pd->children, (Efl_Gfx_Entity *)subobj);
}

EOLIAN static void
_efl_ui_box_efl_pack_layout_layout_request(Eo *obj, Efl_Ui_Box_Data *pd)
{
   pd->full_recalc = EINA_TRUE;
   efl_canvas_group_need_recalculate_set(obj, EINA_TRUE);
}

EOLIAN static Eina_Iterator *
_efl_ui_box_efl_container_content_iterate(Eo *obj EINA_UNUSED, Efl_Ui_Box_Data *pd)
{
   return eina_list_iterator_new(pd->children);
}

EOLIAN static void
_efl_ui_box_efl_ui_layout_orientable_orientation_set(Eo *obj, Efl_Ui_Box_Data *pd, Efl_Ui_Layout_Orientation dir)
{
   if (pd->dir == dir) return;

   switch (dir)
     {
      case EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL:
        pd->dir = EFL_UI_LAYOUT_ORIENTATION_HORIZONTAL;
        break;

      case EFL_UI_LAYOUT_ORIENTATION_VERTICAL:
      case EFL_UI_LAYOUT_ORIENTATION_DEFAULT:
      default:
        pd->dir = EFL_UI_LAYOUT_ORIENTATION_VERTICAL;
        break;
     }

   efl_pack_layout_request(obj);
}

EOLIAN static Efl_Ui_Layout_Orientation
_efl_ui_box_efl_ui_layout_orientable_orientation_get(const Eo *obj EINA_UNUSED, Efl_Ui_Box_Data *pd)
{
   return pd->dir;
}

EOLIAN static void
_efl_ui_box_efl_gfx_arrangement_content_padding_set(Eo *obj, Efl_Ui_Box_Data *pd, unsigned int h, unsigned int v)
{
   if (EINA_DBL_EQ(pd->pad.h, h) && EINA_DBL_EQ(pd->pad.v, v))
     return;

   pd->pad.h = h;
   pd->pad.v = v;

   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_box_efl_gfx_arrangement_content_padding_get(const Eo *obj EINA_UNUSED, Efl_Ui_Box_Data *pd, unsigned int *h, unsigned int *v)
{
   if (h) *h = pd->pad.h;
   if (v) *v = pd->pad.v;
}

EOLIAN static void
_efl_ui_box_efl_gfx_arrangement_content_align_set(Eo *obj, Efl_Ui_Box_Data *pd, double h, double v)
{
   if (h < 0) h = -1;
   else if (h > 1) h = 1;
   if (v < 0) v = -1;
   else if (v > 1) v = 1;

   if (EINA_DBL_EQ(pd->align.h, h) && EINA_DBL_EQ(pd->align.v, v))
     return;

   pd->align.h = h;
   pd->align.v = v;

   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_box_efl_gfx_arrangement_content_align_get(const Eo *obj EINA_UNUSED, Efl_Ui_Box_Data *pd, double *h, double *v)
{
   if (h) *h = pd->align.h;
   if (v) *v = pd->align.v;
}

/* Internal EO APIs and hidden overrides */

#define EFL_UI_BOX_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(efl_ui_box)

#include "efl_ui_box.eo.c"
