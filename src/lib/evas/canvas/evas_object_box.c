#define EFL_CANVAS_GROUP_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"

#define MY_CLASS_NAME "Evas_Box"
#define MY_CLASS_NAME_LEGACY "Evas_Object_Box"

#define MY_CLASS EVAS_BOX_CLASS

typedef struct _Evas_Object_Box_Iterator Evas_Object_Box_Iterator;
typedef struct _Evas_Object_Box_Accessor Evas_Object_Box_Accessor;

struct _Evas_Object_Box_Iterator
{
   Eina_Iterator iterator;

   Eina_Iterator *real_iterator;
   const Evas_Object *box;
};

struct _Evas_Object_Box_Accessor
{
   Eina_Accessor accessor;

   Eina_Accessor *real_accessor;
   const Evas_Object *box;
};

#define SIG_CHILD_ADDED "child,added"
#define SIG_CHILD_REMOVED "child,removed"

static const Evas_Smart_Cb_Description _signals[] =
{
   {SIG_CHILD_ADDED, ""},
   {SIG_CHILD_REMOVED, ""},
   {NULL, NULL}
};


static void _sizing_eval(Evas_Object *obj);

#define EVAS_OBJECT_BOX_DATA_GET(o, ptr) \
   Evas_Object_Box_Data *ptr = efl_data_scope_get(o, MY_CLASS)

#define EVAS_OBJECT_BOX_DATA_GET_OR_RETURN(o, ptr)                      \
   EVAS_OBJECT_BOX_DATA_GET(o, ptr);                                    \
if (!ptr)                                                               \
{                                                                       \
   ERR("No widget data for object %p (%s)",                            \
        o, evas_object_type_get(o));                                    \
   fflush(stderr);                                                      \
   return;                                                              \
}

#define EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, ptr, val)             \
   EVAS_OBJECT_BOX_DATA_GET(o, ptr);                                    \
if (!ptr)                                                               \
{                                                                       \
   ERR("No widget data for object %p (%s)",                            \
        o, evas_object_type_get(o));                                    \
   fflush(stderr);                                                      \
   return val;                                                          \
}

static Eina_Bool
_evas_object_box_iterator_next(Evas_Object_Box_Iterator *it, void **data)
{
   Evas_Object_Box_Option *opt;

   if (!eina_iterator_next(it->real_iterator, (void **)&opt))
     return EINA_FALSE;
   if (data) *data = opt->obj;
   return EINA_TRUE;
}

static Evas_Object *
_evas_object_box_iterator_get_container(Evas_Object_Box_Iterator *it)
{
   return (Evas_Object *)it->box;
}

static void
_evas_object_box_iterator_free(Evas_Object_Box_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   free(it);
}

static Eina_Bool
_evas_object_box_accessor_get_at(Evas_Object_Box_Accessor *it, unsigned int idx, void **data)
{
   Evas_Object_Box_Option *opt = NULL;

   if (!eina_accessor_data_get(it->real_accessor, idx, (void *)&opt))
     return EINA_FALSE;
   if (data) *data = opt->obj;
   return EINA_TRUE;
}

static Evas_Object *
_evas_object_box_accessor_get_container(Evas_Object_Box_Accessor *it)
{
   return (Evas_Object *)it->box;
}

static void
_evas_object_box_accessor_free(Evas_Object_Box_Accessor *it)
{
   eina_accessor_free(it->real_accessor);
   free(it);
}

static void
_on_child_resize(void *data, const Efl_Event *event EINA_UNUSED)
{
   Evas_Object *box = data;
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN(box, priv);
   if (!priv->layouting) evas_object_smart_changed(box);
}

static void
_on_child_invalidate(void *data, const Efl_Event *event)
{
   Evas_Object *box = data;

   Evas_Object *ret = NULL;
   ret = evas_obj_box_internal_remove(box, event->object);
   if (!ret)
     ERR("child removal failed");
   evas_object_smart_changed(box);
}

static void
_on_child_hints_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Evas_Object *box = data;
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN(box, priv);
// XXX: this breaks box repacking in elementary. widgets DEPEND on being able
// to change their hints evenr WHILE being laid out. so comment this out.
//   if (!priv->layouting)
     evas_object_smart_changed(box);
}

static void
_on_hints_changed(void *data EINA_UNUSED, Evas *evas EINA_UNUSED, Evas_Object *o , void *einfo EINA_UNUSED)
{
   _sizing_eval(o);
}

static Evas_Object_Box_Option *
_evas_object_box_option_new(Evas_Object *o, Evas_Object_Box_Data *priv EINA_UNUSED, Evas_Object *child)
{
   Evas_Object_Box_Option *opt = NULL;

   opt = evas_obj_box_internal_option_new(o, child);
   if (!opt)
     {
        ERR("option_new failed");
        return NULL;
     }

   return opt;
}

EFL_CALLBACKS_ARRAY_DEFINE(evas_object_box_callbacks,
  { EFL_GFX_ENTITY_EVENT_RESIZE, _on_child_resize },
  { EFL_EVENT_INVALIDATE, _on_child_invalidate },
  { EFL_GFX_ENTITY_EVENT_HINTS_CHANGED, _on_child_hints_changed }
);

static void
_evas_object_box_child_callbacks_unregister(Evas_Object *obj, Evas_Object *parent)
{
   efl_event_callback_array_del(obj, evas_object_box_callbacks(), parent);
}

static Evas_Object_Box_Option *
_evas_object_box_option_callbacks_register(Evas_Object *o, Evas_Object_Box_Data *priv EINA_UNUSED, Evas_Object_Box_Option *opt)
{
   Evas_Object *obj = opt->obj;

   efl_event_callback_array_add(obj, evas_object_box_callbacks(), o);

   return opt;
}

EOLIAN static Evas_Object_Box_Option *
_evas_box_internal_option_new(Eo *o EINA_UNUSED, Evas_Object_Box_Data *_pd EINA_UNUSED, Evas_Object *child)
{
   Evas_Object_Box_Option *opt;
   Evas_Object *parent;

   parent = evas_object_smart_parent_get(child);
   if (parent && efl_isa(parent, MY_CLASS))
     CRI("Adding object to box which currently belongs to different box");

   opt = (Evas_Object_Box_Option *)malloc(sizeof(*opt));
   if (!opt)
     return NULL;

   opt->obj = child;

   return opt;
}

EOLIAN static void
_evas_box_internal_option_free(Eo *o EINA_UNUSED, Evas_Object_Box_Data *_pd EINA_UNUSED, Evas_Object_Box_Option *opt)
{
   free(opt);
}

EOLIAN static Evas_Object_Box_Option*
_evas_box_internal_append(Eo *o, Evas_Object_Box_Data *priv, Evas_Object *child)
{
   Evas_Object_Box_Option *opt;

   opt = _evas_object_box_option_new(o, priv, child);
   if (!opt)
     return NULL;

   priv->children = eina_list_append(priv->children, opt);
   priv->children_changed = EINA_TRUE;
   efl_event_callback_legacy_call(o, EVAS_BOX_EVENT_CHILD_ADDED, opt);

   return opt;
}

EOLIAN static Evas_Object_Box_Option *
_evas_box_internal_prepend(Eo *o, Evas_Object_Box_Data *priv, Evas_Object *child)
{
   Evas_Object_Box_Option *opt;

   opt = _evas_object_box_option_new(o, priv, child);
   if (!opt)
     return NULL;

   priv->children = eina_list_prepend(priv->children, opt);
   priv->children_changed = EINA_TRUE;
   efl_event_callback_legacy_call(o, EVAS_BOX_EVENT_CHILD_ADDED, opt);

   return opt;
}

EOLIAN static Evas_Object_Box_Option *
_evas_box_internal_insert_before(Eo *o, Evas_Object_Box_Data *priv, Evas_Object *child, const Evas_Object *reference)
{
   Eina_List *l;
   Evas_Object_Box_Option *opt;

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        if (opt->obj == reference)
          {
             Evas_Object_Box_Option *new_opt;

             new_opt = _evas_object_box_option_new(o, priv, child);
             if (!new_opt)
                return NULL;

             priv->children = eina_list_prepend_relative
                (priv->children, new_opt, opt);
             priv->children_changed = EINA_TRUE;
             efl_event_callback_legacy_call(o, EVAS_BOX_EVENT_CHILD_ADDED, new_opt);
             return new_opt;
          }
     }

   return opt;
}

EOLIAN static Evas_Object_Box_Option *
_evas_box_internal_insert_after(Eo *o, Evas_Object_Box_Data *priv, Evas_Object *child, const Evas_Object *reference)
{
   Eina_List *l;
   Evas_Object_Box_Option *opt;

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        if (opt->obj == reference)
          {
             Evas_Object_Box_Option *new_opt;

             new_opt = _evas_object_box_option_new(o, priv, child);
             if (!new_opt)
                return NULL;

             priv->children = eina_list_append_relative
                (priv->children, new_opt, opt);
             priv->children_changed = EINA_TRUE;
             efl_event_callback_legacy_call(o, EVAS_BOX_EVENT_CHILD_ADDED, new_opt);
             return new_opt;
          }
     }
   return opt;
}

EOLIAN static Evas_Object_Box_Option *
_evas_box_internal_insert_at(Eo *o, Evas_Object_Box_Data *priv, Evas_Object *child, unsigned int pos)
{
   Eina_List *l;
   unsigned int i;

   if ((pos == 0) && (eina_list_count(priv->children) == 0))
     {
        Evas_Object_Box_Option *new_opt;

        new_opt = _evas_object_box_option_new(o, priv, child);
        if (!new_opt)
           return NULL;

        priv->children = eina_list_prepend(priv->children, new_opt);
        priv->children_changed = EINA_TRUE;
        efl_event_callback_legacy_call(o, EVAS_BOX_EVENT_CHILD_ADDED, new_opt);
        return new_opt;
     }

   for (l = priv->children, i = 0; l; l = l->next, i++)
     {
        Evas_Object_Box_Option *opt = l->data;

        if (i == pos)
          {
             Evas_Object_Box_Option *new_opt;

             new_opt = _evas_object_box_option_new(o, priv, child);
             if (!new_opt)
                return NULL;

             priv->children = eina_list_prepend_relative
                (priv->children, new_opt, opt);
             priv->children_changed = EINA_TRUE;
             efl_event_callback_legacy_call(o, EVAS_BOX_EVENT_CHILD_ADDED, new_opt);
             return new_opt;
          }
     }
   return NULL; 
}

EOLIAN static Evas_Object *
_evas_box_internal_remove(Eo *o, Evas_Object_Box_Data *priv, Evas_Object *child)
{
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Object *obj = opt->obj;

        if (obj == child)
          {
             priv->children = eina_list_remove(priv->children, opt);
             evas_obj_box_internal_option_free(o, opt);
             priv->children_changed = EINA_TRUE;
             efl_event_callback_legacy_call(o, EVAS_BOX_EVENT_CHILD_REMOVED, obj);

             return obj;
          }
     }
   return NULL;
}

EOLIAN static Evas_Object *
_evas_box_internal_remove_at(Eo *o, Evas_Object_Box_Data *priv, unsigned int pos)
{
   Eina_List *node;
   Evas_Object_Box_Option *opt;
   Evas_Object *obj;

   node = eina_list_nth_list(priv->children, pos);
   if (!node)
     {
        ERR("No item to be removed at position %d", pos);
        return NULL;
     }

   opt = node->data;
   obj = opt->obj;

   priv->children = eina_list_remove_list(priv->children, node);
   evas_obj_box_internal_option_free(o, opt);
   priv->children_changed = EINA_TRUE;
   efl_event_callback_legacy_call(o, EVAS_BOX_EVENT_CHILD_REMOVED, obj);
   return obj;
}

EOLIAN static void
_evas_box_efl_canvas_group_group_add(Eo *eo_obj, Evas_Object_Box_Data *priv)
{
   Evas_Object_Smart_Clipped_Data *cso;

   efl_canvas_group_add(efl_super(eo_obj, MY_CLASS));
   
   evas_object_event_callback_add
     (eo_obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_hints_changed, eo_obj);
   priv->children = NULL;
   priv->align.h = 0.5;
   priv->align.v = 0.5;
   priv->pad.h = 0;
   priv->pad.v = 0;
   priv->layout.cb = evas_object_box_layout_horizontal;
   priv->layout.data = NULL;
   priv->layout.free_data = NULL;

   // make sure evas box smart data is fully initialized and set (for legacy)
   // this assumes only box and smart clipped access the smart data
   cso = evas_object_smart_data_get(eo_obj);
   priv->base = *cso;
   evas_object_smart_data_set(eo_obj, priv);
}

EOLIAN static void
_evas_box_efl_canvas_group_group_del(Eo *o, Evas_Object_Box_Data *priv)
{
   Eina_List *l;

   l = priv->children;
   while (l)
     {
        Evas_Object_Box_Option *opt = l->data;

        _evas_object_box_child_callbacks_unregister(opt->obj, o);
        evas_obj_box_internal_option_free(o, opt);
        l = eina_list_remove_list(l, l);
     }

   if (priv->layout.data && priv->layout.free_data)
     priv->layout.free_data(priv->layout.data);

   efl_canvas_group_del(efl_super(o, MY_CLASS));
}

EOLIAN static void
_evas_box_efl_gfx_entity_size_set(Eo *o, Evas_Object_Box_Data *_pd EINA_UNUSED, Eina_Size2D sz)
{
   if (_evas_object_intercept_call(o, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 0, sz.w, sz.h))
     return;

   efl_gfx_entity_size_set(efl_super(o, MY_CLASS), sz);
   evas_object_smart_changed(o);
}

EOLIAN static void
_evas_box_efl_gfx_entity_position_set(Eo *o, Evas_Object_Box_Data *_pd EINA_UNUSED, Eina_Position2D pos)
{
   if (_evas_object_intercept_call(o, EVAS_OBJECT_INTERCEPT_CB_MOVE , 0, pos.x, pos.y))
     return;

   efl_gfx_entity_position_set(efl_super(o, MY_CLASS), pos);
   evas_object_smart_changed(o);
}

EOLIAN static void
_evas_box_efl_canvas_group_group_calculate(Eo *o, Evas_Object_Box_Data *priv)
{
   if (priv->layout.cb)
       {
           Evas *e;

           e = evas_object_evas_get(o);
           evas_event_freeze(e);

           priv->layouting = 1;
           priv->layout.cb(o, priv, priv->layout.data);
           priv->layouting = 0;
           priv->children_changed = EINA_FALSE;

           evas_event_thaw(e);
       }
   else
     ERR("No layout function set for %p box.", o);
}

EAPI Evas_Object *
evas_object_box_add(Evas *evas)
{
   evas = evas_find(evas);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(evas, EVAS_CANVAS_CLASS), NULL);
   return efl_add(MY_CLASS, evas, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_evas_box_efl_object_constructor(Eo *obj, Evas_Object_Box_Data *class_data EINA_UNUSED)
{
   efl_canvas_group_clipped_set(obj, EINA_TRUE);
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_callbacks_descriptions_set(obj, _signals);
   efl_canvas_object_type_set(obj, MY_CLASS_NAME_LEGACY);

   return obj;
}

EOLIAN static Evas_Object*
_evas_box_add_to(Eo *parent, Evas_Object_Box_Data *_pd EINA_UNUSED)
{
   Evas *evas;
   Evas_Object *o;

   evas = evas_object_evas_get(parent);
   o = evas_object_box_add(evas);
   evas_object_smart_member_add(o, parent);

   return o;
}

EAPI void
evas_object_box_smart_set(Evas_Object_Box_Api *api EINA_UNUSED)
{
   return;
}

EAPI const Evas_Object_Box_Api *
evas_object_box_smart_class_get(void)
{
   return NULL;
}

EOLIAN static void
_evas_box_layout_set(Eo *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Layout cb, const void *data, void (*free_data)(void *data))
{
   if ((priv->layout.cb == cb) && (priv->layout.data == data) &&
       (priv->layout.free_data == free_data))
     return;

   if (priv->layout.data && priv->layout.free_data)
     priv->layout.free_data(priv->layout.data);

   priv->layout.cb = cb;
   priv->layout.data = (void *)data;
   priv->layout.free_data = free_data;
   evas_object_smart_changed(o);
}

static void
_fixed_point_divide_and_decompose_integer(int dividend, int divisor, int *int_part, int *frac_part)
{
   int quotient = ((long long)dividend << 16) / divisor;
   *frac_part = quotient & 0xffff;
   *int_part = quotient >> 16;
}

static void
_layout_dimension_change_min_max_cell_bound(int dim, int *new_dim, int min_d, int max_d, int cell_sz)
{
   if (dim > cell_sz)
     {
        if ((min_d != 0) && (cell_sz < min_d))
          *new_dim = min_d;
        else
          *new_dim = cell_sz;
     }
   else
     {
        if ((max_d != -1) && (cell_sz > max_d))
          *new_dim = max_d;
        else
          *new_dim = cell_sz;
     }
}

static void
_layout_set_offset_and_expand_dimension_space_max_bounded(int dim, int *new_dim, int space_sz, int max_dim, int *offset, double align, int pad_before, int pad_after)
{
   if (align >= 0.0)
     {
        *new_dim = dim;
        *offset = (space_sz - (dim + pad_before + pad_after)) * align
           + pad_before;
     }
   else
     {
        if ((max_dim != -1) && (space_sz > max_dim))
          {
             *new_dim = max_dim;
             *offset = (space_sz - (max_dim + pad_before + pad_after)) * 0.5
                + pad_before;
          }
        else
          {
             *new_dim = space_sz;
             *offset = 0;
          }
     }
}

static void
_layout_set_offset_and_change_dimension_min_max_cell_bounded(int dim, int *new_dim, int min_dim, int max_dim, int cell_sz, int *offset, double align, int pad_before, int pad_after)
{
   if (align >= 0.0)
     {
        *new_dim = dim;
        *offset =
           (cell_sz - (dim + pad_before + pad_after)) * align + pad_before;
     }
   else
     {
        *offset = pad_before;
        _layout_dimension_change_min_max_cell_bound
           (dim, new_dim, min_dim, max_dim, cell_sz - pad_before - pad_after);
     }
}

static void
_sizing_eval(Evas_Object *obj)
{
   Evas_Coord minw, minh, maxw, maxh;
   Evas_Coord w, h;

   evas_object_size_hint_combined_min_get(obj, &minw, &minh);
   evas_object_size_hint_max_get(obj, &maxw, &maxh);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   if (w < minw) w = minw;
   if (h < minh) h = minh;
   if ((maxw >= 0) && (w > maxw)) w = maxw;
   if ((maxh >= 0) && (h > maxh)) h = maxh;

   evas_object_resize(obj, w, h);
}

static int
_evas_object_box_layout_horizontal_weight_apply(Evas_Object_Box_Data *priv, Evas_Object_Box_Option **objects, int n_objects, int remaining, double weight_total)
{
   int rem_diff = 0;
   int i;

   for (i = 0; i < n_objects; i++)
     {
        Evas_Object_Box_Option *opt = objects[i];
        Evas_Object *o = opt->obj;
        int h;

        evas_object_geometry_get(o, NULL, NULL, NULL, &h);

        if (remaining <= 0)
          {
             int min_w;

             evas_object_size_hint_combined_min_get(o, &min_w, NULL);
             evas_object_resize(o, min_w, h);
          }
        else
          {
             double normal_weight, weight_x;
             int target_size;
             int max_w;

             evas_object_size_hint_weight_get(o, &weight_x, NULL);
             normal_weight = weight_x / weight_total;
             target_size = (int)((double)remaining * normal_weight);

             evas_object_size_hint_max_get(o, &max_w, NULL);
             if ((max_w != -1) && (target_size > max_w))
               {
                  evas_object_resize(o, max_w, h);
                  rem_diff += max_w;
                  objects[i] = objects[n_objects - 1];
                  weight_total -= weight_x;
                  n_objects--;
                  return _evas_object_box_layout_horizontal_weight_apply
                     (priv, objects, n_objects, remaining - rem_diff,
                      weight_total);
               }
             else
               {
                  evas_object_resize(o, target_size, h);
                  rem_diff += target_size;
               }
          }
     }

   return remaining - rem_diff;
}

EOLIAN static void
_evas_box_layout_horizontal(Eo *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Data *boxdata EINA_UNUSED, void *data EINA_UNUSED)
{
   int pad_inc = 0, sub_pixel = 0;
   int req_w, global_pad, remaining, top_h = 0, totalminw = 0;
   double weight_total = 0.0;
   int weight_use = 0;
   int x, y, w, h;
   int n_children;
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Option **objects;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     {
        evas_object_size_hint_min_set(o, 0, 0);
        return;
     }

   objects = (Evas_Object_Box_Option **)alloca(sizeof(Evas_Object_Box_Option *) * n_children);
   if (!objects)
     {
        evas_object_size_hint_min_set(o, 0, 0);
        return;
     }

   evas_object_geometry_get(o, &x, &y, &w, &h);
   global_pad = priv->pad.h;
   req_w = global_pad * (n_children - 1);
   totalminw = req_w;

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int padding_l, padding_r;
        double weight_x;
        int minw;

        _sizing_eval(opt->obj);
        evas_object_size_hint_weight_get(opt->obj, &weight_x, NULL);
        evas_object_size_hint_padding_get
           (opt->obj, &padding_l, &padding_r, NULL, NULL);
        req_w += padding_l + padding_r;

        evas_object_size_hint_combined_min_get(opt->obj, &minw, NULL);
        if (minw > 0) totalminw += minw + padding_l + padding_r;
        if (EINA_DBL_EQ(weight_x, 0.0))
          {
             if (minw > 0) req_w += minw;
          }
        else
          {
             objects[weight_use] = opt;
             weight_use++;
             weight_total += weight_x;
          }
     }

   remaining = w - req_w;

   if (weight_use)
     remaining = _evas_object_box_layout_horizontal_weight_apply
       (priv, objects, weight_use, remaining, weight_total);

   if (priv->align.h >= 0.0)
     x += remaining * priv->align.h;
   else if (n_children == 1)
     x += remaining / 2;
   else
     { /* justified */
        _fixed_point_divide_and_decompose_integer
            (remaining, n_children - 1, &global_pad, &pad_inc);
        global_pad += priv->pad.h;
     }

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, max_h, new_h, off_x, off_y, minw, minh;
        int padding_l, padding_r, padding_t, padding_b;
        double align_y;

        evas_object_size_hint_align_get(opt->obj, NULL, &align_y);
        evas_object_size_hint_padding_get
           (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, NULL, &max_h);

        evas_object_size_hint_combined_min_get(opt->obj, &minw, &minh);
        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        off_x = padding_l;
        new_h = minh;
        if (new_h > top_h) top_h = new_h;

        _layout_set_offset_and_expand_dimension_space_max_bounded
           (child_h, &new_h, h, max_h, &off_y, align_y, padding_t, padding_b);

        if (new_h != child_h)
          evas_object_resize(opt->obj, child_w, new_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        x += child_w + padding_l + padding_r + global_pad;
        sub_pixel += pad_inc;
        if (sub_pixel >= 1 << 16)
          {
             x++;
             sub_pixel -= 1 << 16;
          }
     }

   evas_object_size_hint_min_set(o, totalminw, top_h);
}

static int
_evas_object_box_layout_vertical_weight_apply(Evas_Object_Box_Data *priv, Evas_Object_Box_Option **objects, int n_objects, int remaining, double weight_total)
{
   int rem_diff = 0;
   int i;

   for (i = 0; i < n_objects; i++)
     {
        Evas_Object_Box_Option *opt = objects[i];
        Evas_Object *o = opt->obj;
        int w;

        evas_object_geometry_get(o, NULL, NULL, &w, NULL);

        if (remaining <= 0)
          {
             int min_h;

             evas_object_size_hint_combined_min_get(o, NULL, &min_h);
             evas_object_resize(o, w, min_h);
          }
        else
          {
             double normal_weight, weight_y;
             int target_size;
             int max_h;

             evas_object_size_hint_weight_get(o, NULL, &weight_y);
             normal_weight = weight_y / weight_total;
             target_size = (int)((double)remaining * normal_weight);

             evas_object_size_hint_max_get(o, NULL, &max_h);
             if ((max_h != -1) && (target_size > max_h))
               {
                  evas_object_resize(o, w, max_h);
                  rem_diff += max_h;
                  objects[i] = objects[n_objects - 1];
                  weight_total -= weight_y;
                  n_objects--;
                  return _evas_object_box_layout_vertical_weight_apply
                     (priv, objects, n_objects, remaining - rem_diff,
                      weight_total);
               }
             else
               {
                  evas_object_resize(o, w, target_size);
                  rem_diff += target_size;
               }
          }
     }

   return remaining - rem_diff;
}

EOLIAN static void
_evas_box_layout_vertical(Eo *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Data *boxdata EINA_UNUSED, void *data EINA_UNUSED)
{
   int pad_inc = 0, sub_pixel = 0;
   int req_h, global_pad, remaining, top_w = 0, totalminh = 0;
   double weight_total = 0.0;
   int weight_use = 0;
   int x, y, w, h;
   int n_children;
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Option **objects;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     {
        evas_object_size_hint_min_set(o, 0, 0);
        return;
     }

   objects = (Evas_Object_Box_Option **)alloca(sizeof(Evas_Object_Box_Option *) * n_children);
   if (!objects)
     {
        evas_object_size_hint_min_set(o, 0, 0);
        return;
     }

   evas_object_geometry_get(o, &x, &y, &w, &h);
   global_pad = priv->pad.v;
   req_h = global_pad * (n_children - 1);
   totalminh = req_h;

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int padding_t, padding_b;
        double weight_y;
        int minh;

        _sizing_eval(opt->obj);
        evas_object_size_hint_weight_get(opt->obj, NULL, &weight_y);
        evas_object_size_hint_padding_get
           (opt->obj, NULL, NULL, &padding_t, &padding_b);
        req_h += padding_t + padding_b;

        evas_object_size_hint_combined_min_get(opt->obj, NULL, &minh);
        if (minh > 0) totalminh += minh + padding_t + padding_b;
        if (EINA_DBL_EQ(weight_y, 0.0))
          {
             if (minh > 0) req_h += minh;
          }
        else
          {
             objects[weight_use] = opt;
             weight_use++;
             weight_total += weight_y;
          }
     }

   remaining = h - req_h;

   if (weight_use)
     remaining = _evas_object_box_layout_vertical_weight_apply
       (priv, objects, weight_use, remaining, weight_total);

   if (priv->align.v >= 0.0)
     y += remaining * priv->align.v;
   else if (n_children == 1)
     y += remaining / 2;
   else
     { /* justified */
        _fixed_point_divide_and_decompose_integer
           (remaining, n_children - 1, &global_pad, &pad_inc);
        global_pad += priv->pad.v;
     }

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, max_w, new_w, off_x, off_y, minw, minh;
        int padding_l, padding_r, padding_t, padding_b;
        double align_x;

        evas_object_size_hint_align_get(opt->obj, &align_x, NULL);
        evas_object_size_hint_padding_get
           (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, &max_w, NULL);

        evas_object_size_hint_combined_min_get(opt->obj, &minw, &minh);
        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        off_y = padding_t;
        new_w = minw;

        _layout_set_offset_and_expand_dimension_space_max_bounded
           (child_w, &new_w, w, max_w, &off_x, align_x, padding_l, padding_r);

        if (new_w > top_w) top_w = new_w;

        if (new_w != child_w)
          evas_object_resize(opt->obj, new_w, child_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        y += child_h + padding_t + padding_b + global_pad;
        sub_pixel += pad_inc;
        if (sub_pixel >= 1 << 16)
          {
             y++;
             sub_pixel -= 1 << 16;
          }
     }

   evas_object_size_hint_min_set(o, top_w, totalminh);
}

EOLIAN static void
_evas_box_layout_homogeneous_horizontal(Eo *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Data *boxdata EINA_UNUSED, void *data EINA_UNUSED)
{
   int cell_sz, share, inc;
   int sub_pixel = 0;
   int x, y, w, h;
   int n_children;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     {
        evas_object_size_hint_min_set(o, 0, 0);
        return;
     }

   evas_object_geometry_get(o, &x, &y, &w, &h);

   share = w - priv->pad.h * (n_children - 1);
   _fixed_point_divide_and_decompose_integer
     (share, n_children, &cell_sz, &inc);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, max_h, min_w, max_w, new_w, new_h, off_x, off_y;
        int padding_l, padding_r, padding_t, padding_b;
        double align_x, align_y;

        evas_object_size_hint_align_get(opt->obj, &align_x, &align_y);
        evas_object_size_hint_padding_get
           (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, &max_w, &max_h);
        evas_object_size_hint_combined_min_get(opt->obj, &min_w, NULL);

        _sizing_eval(opt->obj);
        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        new_w = child_w;
        new_h = child_h;

        _layout_set_offset_and_expand_dimension_space_max_bounded
           (child_h, &new_h, h, max_h, &off_y, align_y, padding_t, padding_b);

        _layout_set_offset_and_change_dimension_min_max_cell_bounded
           (child_w, &new_w, min_w, max_w, cell_sz, &off_x, align_x,
            padding_l, padding_r);

        if ((new_w != child_w) || (new_h != child_h))
          evas_object_resize(opt->obj, new_w, new_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        x += cell_sz + priv->pad.h;
        sub_pixel += inc;
        if (sub_pixel >= 1 << 16)
          {
             x++;
             sub_pixel -= 1 << 16;
          }
     }

   evas_object_size_hint_min_set(o, w, h);
}

EOLIAN static void
_evas_box_layout_homogeneous_vertical(Eo *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Data *boxdata EINA_UNUSED, void *data EINA_UNUSED)
{
   int cell_sz, share, inc;
   int sub_pixel = 0;
   int x, y, w, h;
   int n_children;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     {
        evas_object_size_hint_min_set(o, 0, 0);
        return;
     }

   evas_object_geometry_get(o, &x, &y, &w, &h);

   share = h - priv->pad.v * (n_children - 1);
   _fixed_point_divide_and_decompose_integer
     (share, n_children, &cell_sz, &inc);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, max_w, min_h, max_h, new_w, new_h, off_x, off_y;
        int padding_l, padding_r, padding_t, padding_b;
        double align_x, align_y;

        evas_object_size_hint_align_get(opt->obj, &align_x, &align_y);
        evas_object_size_hint_padding_get
           (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, &max_w, &max_h);
        evas_object_size_hint_combined_min_get(opt->obj, NULL, &min_h);

        _sizing_eval(opt->obj);
        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);
        new_w = child_w;
        new_h = child_h;

        _layout_set_offset_and_expand_dimension_space_max_bounded
           (child_w, &new_w, w, max_w, &off_x, align_x, padding_l, padding_r);

        _layout_set_offset_and_change_dimension_min_max_cell_bounded
           (child_h, &new_h, min_h, max_h, cell_sz, &off_y, align_y,
            padding_t, padding_b);

        if ((new_w != child_w) || (new_h != child_h))
          evas_object_resize(opt->obj, new_w, new_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        y += cell_sz + priv->pad.v;
        sub_pixel += inc;
        if (sub_pixel >= 1 << 16)
          {
             y++;
             sub_pixel -= 1 << 16;
          }
     }

   evas_object_size_hint_min_set(o, w, h);
}

EOLIAN static void
_evas_box_layout_homogeneous_max_size_horizontal(Eo *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Data *boxdata EINA_UNUSED, void *data EINA_UNUSED)
{
   int remaining, global_pad, pad_inc = 0, sub_pixel = 0;
   int cell_sz = 0;
   int x, y, w, h;
   int top_h = 0;
   int n_children;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     {
        evas_object_size_hint_min_set(o, 0, 0);
        return;
     }

   evas_object_geometry_get(o, &x, &y, &w, &h);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, padding_l, padding_r;

        _sizing_eval(opt->obj);
        evas_object_size_hint_padding_get
           (opt->obj, &padding_l, &padding_r, NULL, NULL);
        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, NULL);
        if (child_w + padding_l + padding_r > cell_sz)
          cell_sz = child_w + padding_l + padding_r;
     }

   global_pad = priv->pad.h;
   remaining = w - n_children * cell_sz - global_pad * (n_children - 1);

   if (priv->align.h >= 0.0)
     x += remaining * priv->align.h;
   else if (n_children == 1)
     x += remaining / 2;
   else
     { /* justified */
        _fixed_point_divide_and_decompose_integer
           (remaining, n_children - 1, &global_pad, &pad_inc);
        global_pad += priv->pad.h;
     }

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, min_w, max_w, max_h, new_w, new_h, off_x, off_y;
        int padding_l, padding_r, padding_t, padding_b;
        double align_x, align_y;

        evas_object_size_hint_align_get(opt->obj, &align_x, &align_y);
        evas_object_size_hint_padding_get
           (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, &max_w, &max_h);
        evas_object_size_hint_combined_min_get(opt->obj, &min_w, NULL);

        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        new_w = child_w;
        new_h = child_h;
        if (new_h > top_h) top_h = new_h;

        _layout_set_offset_and_expand_dimension_space_max_bounded
           (child_h, &new_h, h, max_h, &off_y, align_y, padding_t, padding_b);

        _layout_set_offset_and_change_dimension_min_max_cell_bounded
           (child_w, &new_w, min_w, max_w, cell_sz, &off_x, align_x,
            padding_l, padding_r);

        if ((new_w != child_w) || (new_h != child_h))
          evas_object_resize(opt->obj, new_w, new_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        x += cell_sz + global_pad;
        sub_pixel += pad_inc;
        if (sub_pixel >= 1 << 16)
          {
             x++;
             sub_pixel -= 1 << 16;
          }
     }

   evas_object_size_hint_min_set(o, x, top_h);
}

EOLIAN static void
_evas_box_layout_homogeneous_max_size_vertical(Eo *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Data *boxdata EINA_UNUSED, void *data EINA_UNUSED)
{
   int remaining, global_pad, pad_inc = 0, sub_pixel = 0;
   int cell_sz = 0;
   int x, y, w, h;
   int top_w = 0;
   int n_children;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     {
        evas_object_size_hint_min_set(o, 0, 0);
        return;
     }

   evas_object_geometry_get(o, &x, &y, &w, &h);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_h, padding_t, padding_b;

        _sizing_eval(opt->obj);
        evas_object_size_hint_padding_get
           (opt->obj, NULL, NULL, &padding_t, &padding_b);
        evas_object_geometry_get(opt->obj, NULL, NULL, NULL, &child_h);
        if (child_h + padding_t + padding_b > cell_sz)
          cell_sz = child_h + padding_t + padding_b;
     }

   global_pad = priv->pad.v;
   remaining = h - n_children * cell_sz - global_pad * (n_children - 1);

   if (priv->align.v >= 0.0)
     y += remaining * priv->align.v;
   else if (n_children == 1)
     y += remaining / 2;
   else
     { /* justified */
        _fixed_point_divide_and_decompose_integer
           (remaining, n_children - 1, &global_pad, &pad_inc);
        global_pad += priv->pad.v;
     }

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, max_h, min_h, max_w, new_w, new_h, off_x, off_y;
        int padding_l, padding_r, padding_t, padding_b;
        double align_x, align_y;

        evas_object_size_hint_align_get(opt->obj, &align_x, &align_y);
        evas_object_size_hint_padding_get
           (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, &max_w, &max_h);
        evas_object_size_hint_combined_min_get(opt->obj, NULL, &min_h);

        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        new_w = child_w;
        new_h = child_h;
        if (new_w > top_w) top_w = new_w;

        _layout_set_offset_and_expand_dimension_space_max_bounded
           (child_w, &new_w, w, max_w, &off_x, align_x, padding_l, padding_r);

        _layout_set_offset_and_change_dimension_min_max_cell_bounded
           (child_h, &new_h, min_h, max_h, cell_sz, &off_y, align_y,
            padding_t, padding_b);

        if ((new_w != child_w) || (new_h != child_h))
          evas_object_resize(opt->obj, new_w, new_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        y += cell_sz + global_pad;
        sub_pixel += pad_inc;
        if (sub_pixel >= 1 << 16)
          {
             y++;
             sub_pixel -= 1 << 16;
          }
     }

   evas_object_size_hint_min_set(o, top_w, y);
}

static void
_evas_object_box_layout_flow_horizontal_row_info_collect(Evas_Object_Box_Data *priv, int box_w, int *row_count, int *row_max_h, int *row_break, int *row_width, int *off_y_ret, int *max_w_ret, int *max_h_ret)
{
   int i, remain_w = box_w, start_i = 0;
   int off_y = 0, max_w = 0, max_h = 0, n_rows = 0;
   Eina_List *l;

   for (i = 0, l = priv->children; l; i++, l = l->next)
     {
        Evas_Object_Box_Option *opt = l->data;
        int padding_l, padding_r, padding_t, padding_b;
        int child_w, child_h, off_x = 0;

        evas_object_size_hint_padding_get
           (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);

        _sizing_eval(opt->obj);
        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        child_w += padding_l + padding_r + priv->pad.h;
        child_h += padding_t + padding_b;
        if (child_w > max_w)
          max_w = child_w;

        remain_w -= child_w;
        if (remain_w + priv->pad.h >= 0)
          { /* continue "line" */
             if (child_h > max_h)
               max_h = child_h;

             off_x += child_w;
             row_width[n_rows] += child_w;
          }
        else
          { /* break line */
             if (i == start_i)
               { /* obj goes to actual line */
                  max_h = child_h;
                  row_width[n_rows] = child_w;
               }
             else
               { /* obj goes to next line */
                  row_max_h[n_rows] = max_h;
                  row_break[n_rows] = i - 1;
                  n_rows++;

                  off_x = child_w;
                  off_y += max_h;
                  max_h = child_h;

                  row_width[n_rows] = child_w;
                  start_i = i;

                  remain_w = box_w - off_x;
               }
          }
     }

   row_break[n_rows] = i - 1;
   row_max_h[n_rows] = max_h;

   *row_count = n_rows;
   *off_y_ret = off_y;
   *max_w_ret = max_w;
   *max_h_ret = max_h;
}

EOLIAN static void
_evas_box_layout_flow_horizontal(Eo *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Data *boxdata EINA_UNUSED, void *data EINA_UNUSED)
{
   int n_children;
   int r, row_count = 0;
   int min_w = 0, min_h = 0;
   int max_h, inc_y;
   int remain_y, i;
   int x, y, w, h;
   Eina_List *l;
   int *row_max_h;
   int *row_break;
   int *row_width;
   int offset_y;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     {
        evas_object_size_hint_min_set(o, 0, 0);
        return;
     }

   /* *per row* arrays */
   row_max_h = (int *)alloca(sizeof(int) * n_children);
   row_break = (int *)alloca(sizeof(int) * n_children);
   row_width = (int *)alloca(sizeof(int) * n_children);

   memset(row_width, 0, sizeof(int) * n_children);

   evas_object_geometry_get(o, &x, &y, &w, &h);

   _evas_object_box_layout_flow_horizontal_row_info_collect
     (priv, w, &row_count, row_max_h, row_break, row_width, &offset_y, &min_w, &max_h);

   inc_y = 0;
   remain_y = h - (priv->pad.v * row_count -1) - (offset_y + max_h);

   if (remain_y > 0)
     {
        if (priv->align.v >= 0.0)
          inc_y = priv->align.v * remain_y;
        else if (row_count == 0)
          y += remain_y / 2;
        else /* y-justified */
          inc_y = remain_y / row_count;
     }

   inc_y += priv->pad.v;

   for (i = 0, r = 0, l = priv->children; r <= row_count; r++)
     {
        int row_justify = 0, just_inc = 0, sub_pixel = 0;
        int row_size, remain_x;

        row_size = row_break[r] - i;
        remain_x = (w - (row_width[r] - priv->pad.h));

        if (priv->align.h < 0.0)
          {
             if (row_size == 0)
               x += remain_x / 2;
             else
               _fixed_point_divide_and_decompose_integer
                  (remain_x, row_size, &row_justify, &just_inc);
          }

        row_justify += priv->pad.h;

        for (; i <= row_break[r]; i++, l = l->next)
          {
             Evas_Object_Box_Option *opt = l->data;
             int off_x, off_y, y_remain;
             int padding_l, padding_r;
             int child_w, child_h;
             double align_y;

             evas_object_size_hint_align_get(opt->obj, NULL, &align_y);
             evas_object_size_hint_padding_get
                (opt->obj, &padding_l, &padding_r, NULL, NULL);

             evas_object_geometry_get
                (opt->obj, NULL, NULL, &child_w, &child_h);

             y_remain = row_max_h[r] - child_h;

             off_x = padding_l;
             if (priv->align.h >= 0.0)
               off_x += remain_x * priv->align.h;
             off_y = y_remain * align_y;

             evas_object_move(opt->obj, x + off_x, y + off_y);

             x += child_w + padding_l + padding_r + row_justify;

             sub_pixel += just_inc;
             if (sub_pixel >= 1 << 16)
               {
                  x++;
                  sub_pixel -= 1 << 16;
               }
          }

        evas_object_geometry_get(o, &x, NULL, NULL, NULL);
        min_h += row_max_h[r];
        y += row_max_h[r] + inc_y;

        if (r > 0)
          min_h += priv->pad.v;
     }

   evas_object_size_hint_min_set(o, min_w, min_h);
}

static void
_evas_object_box_layout_flow_vertical_col_info_collect(Evas_Object_Box_Data *priv, int box_h, int *col_count, int *col_max_w, int *col_break, int *col_height, int *off_x_ret, int *max_w_ret, int *max_h_ret)
{
   int i, remain_h = box_h, start_i = 0;
   int off_x = 0, max_w = 0, max_h = 0, n_cols = 0;
   Eina_List *l;

   for (i = 0, l = priv->children; l; i++, l = l->next)
     {
        Evas_Object_Box_Option *opt = l->data;
        int padding_l, padding_r, padding_t, padding_b;
        int child_w, child_h, off_y = 0;

        evas_object_size_hint_padding_get
           (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);

        _sizing_eval(opt->obj);
        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        child_w += padding_l + padding_r;
        child_h += padding_t + padding_b + priv->pad.v;
        if (child_h > max_h)
          max_h = child_h;

        remain_h -= child_h;
        if (remain_h + priv->pad.v >= 0)
          { /* continue "col" */
             if (child_w > max_w)
               max_w = child_w;

             off_y += child_h;
             col_height[n_cols] += child_h;
          }
        else
          {
             /* break col */
             if (i == start_i)
               { /* obj goes to actual col */
                  max_w = child_w;
                  col_height[n_cols] = child_h;
               }
             else
               { /* obj goes to next col */
                  col_max_w[n_cols] = max_w;
                  col_break[n_cols] = i - 1;
                  n_cols++;

                  off_x += max_w;
                  off_y = child_h;
                  max_w = child_w;

                  col_height[n_cols] = child_h;
                  start_i = i;

                  remain_h = box_h - off_y;
               }
          }
     }

   col_break[n_cols] = i - 1;
   col_max_w[n_cols] = max_w;

   *col_count = n_cols;
   *off_x_ret = off_x;
   *max_w_ret = max_w;
   *max_h_ret = max_h;
}

EOLIAN static void
_evas_box_layout_flow_vertical(Eo *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Data *boxdata EINA_UNUSED, void *data EINA_UNUSED)
{
   int n_children;
   int c, col_count;
   int min_w = 0, min_h = 0;
   int max_w, inc_x;
   int remain_x, i;
   int x, y, w, h;
   Eina_List *l;
   int *col_max_w;
   int *col_break;
   int *col_height;
   int offset_x;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     {
        evas_object_size_hint_min_set(o, 0, 0);
        return;
     }

   /* *per col* arrays */
   col_max_w = (int *)alloca(sizeof(int) * n_children);
   col_break = (int *)alloca(sizeof(int) * n_children);
   col_height = (int *)alloca(sizeof(int) * n_children);

   memset(col_height, 0, sizeof(int) * n_children);

   evas_object_geometry_get(o, &x, &y, &w, &h);

   _evas_object_box_layout_flow_vertical_col_info_collect
     (priv, h, &col_count, col_max_w, col_break, col_height, &offset_x, &max_w, &min_h);

   inc_x = 0;
   remain_x = w - (offset_x + max_w);

   if (remain_x > 0)
     {
        if (priv->align.h >= 0)
          inc_x = priv->align.h * remain_x;
        else if (col_count == 0)
          x += remain_x / 2;
        else /* x-justified */
          inc_x = remain_x / col_count;
     }

   inc_x += priv->pad.h;

   for (i = 0, c = 0, l = priv->children; c <= col_count; c++)
     {
        int col_justify = 0, just_inc = 0, sub_pixel = 0;
        int col_size, remain_y;

        col_size = col_break[c] - i;
        remain_y = (h - col_height[c]);

        if (priv->align.v < 0.0)
          {
             if (col_size == 0)
               y += remain_y / 2;
             else
               _fixed_point_divide_and_decompose_integer
                  (remain_y, col_size, &col_justify, &just_inc);
          }

        col_justify += priv->pad.v;

        for (; i <= col_break[c]; i++, l = l->next)
          {
             Evas_Object_Box_Option *opt = l->data;
             int off_x, off_y, x_remain;
             int padding_t, padding_b;
             int child_w, child_h;
             double align_x;

             evas_object_size_hint_align_get(opt->obj, &align_x, NULL);
             evas_object_size_hint_padding_get
                (opt->obj, NULL, NULL, &padding_t, &padding_b);

             evas_object_geometry_get
                (opt->obj, NULL, NULL, &child_w, &child_h);

             x_remain = col_max_w[c] - child_w;

             off_x = x_remain * align_x;
             off_y = padding_t;
             if (priv->align.v >= 0.0)
               off_y += remain_y * priv->align.v;

             evas_object_move(opt->obj, x + off_x, y + off_y);

             y += child_h + padding_t + padding_b + col_justify;

             sub_pixel += just_inc;
             if (sub_pixel >= 1 << 16)
               {
                  y++;
                  sub_pixel -= 1 << 16;
               }
          }

        evas_object_geometry_get(o, NULL, &y, NULL, NULL);
        min_w += col_max_w[c];
        x += col_max_w[c] + inc_x;
     }

   evas_object_size_hint_min_set(o, min_w, min_h);
}

EOLIAN static void
_evas_box_layout_stack(Eo *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Data *boxdata EINA_UNUSED, void *data EINA_UNUSED)
{
   Eina_List *l;
   Evas_Coord ox, oy, ow, oh;
   Evas_Coord top_w = 0, top_h = 0;
   Evas_Object_Box_Option *opt;
   Evas_Object *old_child = NULL;

   evas_object_geometry_get(o, &ox, &oy, &ow, &oh);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Object *child = opt->obj;
        Evas_Coord max_w, max_h, min_w, min_h, pad_l, pad_r, pad_t, pad_b,
                   child_w, child_h, new_w, new_h, off_x, off_y;
        double align_x, align_y;

        evas_object_size_hint_align_get(child, &align_x, &align_y);
        evas_object_size_hint_padding_get
           (child, &pad_l, &pad_r, &pad_t, &pad_b);
        evas_object_size_hint_max_get(child, &max_w, &max_h);
        evas_object_size_hint_combined_min_get(child, &min_w, &min_h);

        _sizing_eval(opt->obj);
        evas_object_geometry_get(child, NULL, NULL, &child_w, &child_h);
        new_w = child_w;
        new_h = child_h;

        _layout_set_offset_and_change_dimension_min_max_cell_bounded
           (child_w, &new_w, min_w, max_w, ow, &off_x, align_x, pad_l, pad_r);
        _layout_set_offset_and_change_dimension_min_max_cell_bounded
           (child_h, &new_h, min_h, max_h, oh, &off_y, align_y, pad_t, pad_b);

        if (new_w > top_w) top_w = new_w;
        if (new_h > top_h) top_h = new_h;

        if ((new_w != child_w) || (new_h != child_h))
          evas_object_resize(child, new_w, new_h);

        if ((align_x < 0) && (priv->align.h >= 0.0))
          off_x += (ow - new_w) * priv->align.h;
        if ((align_y < 0) && (priv->align.v >= 0.0))
          off_y += (oh - new_h) * priv->align.v;
        evas_object_move(child, ox + off_x, oy + off_y);

        if (old_child)
          evas_object_stack_above(child, old_child);
        old_child = child;
     }

   evas_object_size_hint_min_set(o, top_w, top_h);
}

EOLIAN static void
_evas_box_align_set(Eo *o, Evas_Object_Box_Data *priv, double horizontal, double vertical)
{
   if ((EINA_DBL_EQ(priv->align.h, horizontal)) &&
       (EINA_DBL_EQ(priv->align.v, vertical)))
     return;
   priv->align.h = horizontal;
   priv->align.v = vertical;
   evas_object_smart_changed(o);
}

EOLIAN static void
_evas_box_align_get(const Eo *o EINA_UNUSED, Evas_Object_Box_Data *priv, double *horizontal, double *vertical)
{
   if (priv)
     {
        if (horizontal) *horizontal = priv->align.h;
        if (vertical) *vertical = priv->align.v;
     }
   else
     {
        if (horizontal) *horizontal = 0.5;
        if (vertical) *vertical = 0.5;
     }
}

EOLIAN static void
_evas_box_padding_set(Eo *o, Evas_Object_Box_Data *priv, Evas_Coord horizontal, Evas_Coord vertical)
{
   if (priv->pad.h == horizontal && priv->pad.v == vertical)
     return;
   priv->pad.h = horizontal;
   priv->pad.v = vertical;
   evas_object_smart_changed(o);
}

EOLIAN static void
_evas_box_padding_get(const Eo *o EINA_UNUSED, Evas_Object_Box_Data *priv, Evas_Coord *horizontal, Evas_Coord *vertical)
{
   if (priv)
     {
        if (horizontal) *horizontal = priv->pad.h;
        if (vertical) *vertical = priv->pad.v;
     }
   else
     {
        if (horizontal) *horizontal = 0;
        if (vertical) *vertical = 0;
     }
}

EOLIAN static Evas_Object_Box_Option*
_evas_box_append(Eo *o, Evas_Object_Box_Data *priv, Evas_Object *child)
{
   Evas_Object_Box_Option *opt = NULL;

   if (!child || (evas_object_smart_parent_get(child) == o))
     return NULL;

   opt = evas_obj_box_internal_append(o, child);

   if (opt)
     {
        evas_object_smart_member_add(child, o);
        evas_object_smart_changed(o);
        return _evas_object_box_option_callbacks_register(o, priv, opt);
     }

   return NULL;
}

EOLIAN static Evas_Object_Box_Option*
_evas_box_prepend(Eo *o, Evas_Object_Box_Data *priv, Evas_Object *child)
{
   Evas_Object_Box_Option *opt = NULL;
   if (!child)
     return NULL;

   opt = evas_obj_box_internal_prepend(o, child);

   if (opt)
     {
        evas_object_smart_member_add(child, o);
        evas_object_smart_changed(o);
        return _evas_object_box_option_callbacks_register(o, priv, opt);
     }
   return NULL;
}

EOLIAN static Evas_Object_Box_Option*
_evas_box_insert_before(Eo *o, Evas_Object_Box_Data *priv, Evas_Object *child, const Evas_Object *reference)
{
   Evas_Object_Box_Option *opt = NULL;
   if (!child)
     return NULL;

   opt = evas_obj_box_internal_insert_before(o, child, reference);

   if (opt)
     {
        evas_object_smart_member_add(child, o);
        evas_object_smart_changed(o);
        return _evas_object_box_option_callbacks_register(o, priv, opt);
     }

   return NULL;
}

EOLIAN static Evas_Object_Box_Option*
_evas_box_insert_after(Eo *o, Evas_Object_Box_Data *priv, Evas_Object *child, const Evas_Object *reference)
{
   Evas_Object_Box_Option *opt = NULL;
   if (!child)
     return NULL;

   opt = evas_obj_box_internal_insert_after(o, child, reference);

   if (opt)
     {
        evas_object_smart_member_add(child, o);
        evas_object_smart_changed(o);
        return _evas_object_box_option_callbacks_register(o, priv, opt);
     }

   return NULL;
}

EOLIAN static Evas_Object_Box_Option*
_evas_box_insert_at(Eo *o, Evas_Object_Box_Data *priv, Evas_Object *child, unsigned int pos)
{
   Evas_Object_Box_Option *opt = NULL;
   if (!child)
     return NULL;

   opt = evas_obj_box_internal_insert_at(o, child, pos);

   if (opt)
     {
        evas_object_smart_member_add(child, o);
        evas_object_smart_changed(o);
        return _evas_object_box_option_callbacks_register(o, priv, opt);
     }

   return NULL;
}

EOLIAN static Eina_Bool
_evas_box_remove(Eo *o, Evas_Object_Box_Data *_pd EINA_UNUSED, Evas_Object *child)
{
   Evas_Object *obj = NULL;

   obj = evas_obj_box_internal_remove(o, child);

   if (obj)
     {
        _evas_object_box_child_callbacks_unregister(obj, o);
        evas_object_smart_member_del(obj);
        evas_object_smart_changed(o);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_evas_box_remove_at(Eo *o, Evas_Object_Box_Data *_pd EINA_UNUSED, unsigned int pos)
{
   Evas_Object *obj = NULL;

   obj = evas_obj_box_internal_remove_at(o, pos);

   if (obj)
     {
        _evas_object_box_child_callbacks_unregister(obj, o);
        evas_object_smart_member_del(obj);
        evas_object_smart_changed(o);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EOLIAN static Eina_Bool
_evas_box_remove_all(Eo *o, Evas_Object_Box_Data *priv, Eina_Bool clear)
{
   evas_object_smart_changed(o);

   while (priv->children)
     {
        Evas_Object_Box_Option *opt = priv->children->data;
        Evas_Object *obj = NULL;

        obj = evas_obj_box_internal_remove(o, opt->obj);
        if (obj)
          {
             _evas_object_box_child_callbacks_unregister(obj, o);
             evas_object_smart_member_del(obj);
             if (clear)
               evas_object_del(obj);
          }
        else return EINA_FALSE;
     }

   return EINA_TRUE;
}

EOLIAN static Eina_Iterator*
_evas_box_iterator_new(const Eo *o, Evas_Object_Box_Data *priv)
{
   Evas_Object_Box_Iterator *it;
   
   if (!priv->children) return NULL;

   it = calloc(1, sizeof(Evas_Object_Box_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->real_iterator = eina_list_iterator_new(priv->children);
   it->box = o;

   it->iterator.next = FUNC_ITERATOR_NEXT(_evas_object_box_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_evas_object_box_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_evas_object_box_iterator_free);

   return &it->iterator;
}

EOLIAN static Eina_Accessor*
_evas_box_accessor_new(const Eo *o, Evas_Object_Box_Data *priv)
{
   Evas_Object_Box_Accessor *it;

   if (!priv->children) return NULL;

   it = calloc(1, sizeof(Evas_Object_Box_Accessor));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->accessor, EINA_MAGIC_ACCESSOR);

   it->real_accessor = eina_list_accessor_new(priv->children);
   it->box = o;

   it->accessor.get_at = FUNC_ACCESSOR_GET_AT(_evas_object_box_accessor_get_at);
   it->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(_evas_object_box_accessor_get_container);
   it->accessor.free = FUNC_ACCESSOR_FREE(_evas_object_box_accessor_free);

   return &it->accessor;
}

EAPI Eina_List *
evas_object_box_children_get(const Evas_Object *o)
{
   Eina_List *new_list = NULL, *l;
   Evas_Object_Box_Option *opt;

   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, NULL);

   EINA_LIST_FOREACH(priv->children, l, opt)
      new_list = eina_list_append(new_list, opt->obj);

   return new_list;
}

EOLIAN static int
_evas_box_count(Eo *o, Evas_Object_Box_Data *_pd EINA_UNUSED)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   return eina_list_count(priv->children);
}

EOLIAN static const char*
_evas_box_option_property_name_get(const Eo *o EINA_UNUSED, Evas_Object_Box_Data *_pd EINA_UNUSED, int property EINA_UNUSED)
{
   return NULL;
}

EOLIAN static int
_evas_box_option_property_id_get(const Eo *o EINA_UNUSED, Evas_Object_Box_Data *_pd EINA_UNUSED, const char *name EINA_UNUSED)
{
   return -1;
}

EAPI Eina_Bool
evas_object_box_option_property_set(Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...)
{
   Eina_Bool ret;
   va_list args;

   va_start(args, property);
   ret = evas_object_box_option_property_vset(o, opt, property, args);
   va_end(args);

   return ret;
}


EAPI Eina_Bool
evas_object_box_option_property_vset(Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args)
{
   return evas_obj_box_option_property_vset(o, opt, property, (va_list *) &args);
}

EOLIAN static Eina_Bool
_evas_box_option_property_vset(Eo *o EINA_UNUSED, Evas_Object_Box_Data *_pd EINA_UNUSED, Evas_Object_Box_Option *opt EINA_UNUSED, int property EINA_UNUSED, va_list *args EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI Eina_Bool
evas_object_box_option_property_get(const Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...)
{
   Eina_Bool ret;
   va_list args;

   va_start(args, property);
   ret = evas_object_box_option_property_vget(o, opt, property, args);
   va_end(args);

   return ret;
}

EAPI Eina_Bool
evas_object_box_option_property_vget(const Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args)
{
   return evas_obj_box_option_property_vget((Eo *)o, opt, property, (va_list *) &args);
}

EOLIAN static Eina_Bool
_evas_box_option_property_vget(const Eo *o EINA_UNUSED, Evas_Object_Box_Data *_pd EINA_UNUSED, Evas_Object_Box_Option *opt EINA_UNUSED, int property EINA_UNUSED, va_list *args EINA_UNUSED)
{
   return EINA_FALSE;
}

EOLIAN static void
_evas_box_class_constructor(Efl_Class *klass)
{
   evas_smart_legacy_type_register(MY_CLASS_NAME_LEGACY, klass);
}

/* Internal EO APIs and hidden overrides */

#define EVAS_BOX_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_DEL_OPS(evas_box)

#include "canvas/evas_box.eo.c"
