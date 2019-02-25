#include "efl_ui_relative_layout_private.h"

#define MY_CLASS EFL_UI_RELATIVE_LAYOUT_CLASS
#define MY_CLASS_NAME "Efl.Ui.Relative_Layout"

#define LEFT               0
#define RIGHT              1
#define TOP                2
#define BOTTOM             3

#define START              (axis ? TOP : LEFT)
#define END                (axis ? BOTTOM : RIGHT)

static void _child_calc(Efl_Ui_Relative_Layout_Child *child, Eina_Bool axis);

static int
_chain_sort_cb(const void *l1, const void *l2)
{
   Efl_Ui_Relative_Layout_Calc *calc1, *calc2;

   calc1 = EINA_INLIST_CONTAINER_GET(l1, Efl_Ui_Relative_Layout_Calc);
   calc2 = EINA_INLIST_CONTAINER_GET(l2, Efl_Ui_Relative_Layout_Calc);

   return calc2->comp_factor <= calc1->comp_factor ? -1 : 1;
}

static Efl_Ui_Relative_Layout_Child *
_efl_ui_relative_layout_register(Efl_Ui_Relative_Layout_Data *pd, Eo *child)
{
   Efl_Ui_Relative_Layout_Child *rc;

   rc = calloc(1, sizeof(Efl_Ui_Relative_Layout_Child));
   if (!rc) return NULL;

   rc->obj = child;
   rc->layout = pd->obj;
   rc->rel[LEFT].to = rc->layout;
   rc->rel[LEFT].relative = 0.0;
   rc->rel[RIGHT].to = rc->layout;
   rc->rel[RIGHT].relative = 1.0;
   rc->rel[TOP].to = rc->layout;
   rc->rel[TOP].relative = 0.0;
   rc->rel[BOTTOM].to = rc->layout;
   rc->rel[BOTTOM].relative = 1.0;

   if (pd->obj == child)
     {
        rc->calc.state[0] = RELATIVE_CALC_DONE;
        rc->calc.state[1] = RELATIVE_CALC_DONE;
        rc->calc.chain_state[0] = RELATIVE_CALC_DONE;
        rc->calc.chain_state[1] = RELATIVE_CALC_DONE;
     }
   else
     {
        efl_ui_widget_sub_object_add(pd->obj, child);
        efl_canvas_group_member_add(pd->obj, child);
        efl_canvas_group_change(pd->obj);
     }

   eina_hash_add(pd->children, &child, rc);

   return rc;
}

static Efl_Ui_Relative_Layout_Child *
_relative_child_get(Efl_Ui_Relative_Layout_Data *pd, Eo *child)
{
   Efl_Ui_Relative_Layout_Child *rc;

   rc = eina_hash_find(pd->children, &child);
   if (!rc)
     rc = _efl_ui_relative_layout_register(pd, child);

   return rc;
}

static Efl_Ui_Relative_Layout_Child *
_relative_child_find(const Eina_Hash *children, Eo *target)
{
   Efl_Ui_Relative_Layout_Child *child;

   child = eina_hash_find(children, &target);
   if (!child)
     ERR("target(%p(%s)) is not registered", target, efl_class_name_get(target));

   return child;
}

static void
_child_aspect_calc(Efl_Ui_Relative_Layout_Child *child, Eina_Bool axis)
{
   Efl_Ui_Relative_Layout_Calc *calc = &child->calc;
   int temph;

   if ((calc->aspect[0] <= 0) || (calc->aspect[1] <= 0))
     {
        ERR("Invalid aspect parameter for obj(%p), aspect(%d, %d) ",
            child->obj, calc->aspect[0], calc->aspect[1]);
        return;
     }

   switch (calc->aspect_type)
     {
      case EFL_GFX_HINT_ASPECT_HORIZONTAL:
        if (axis) _child_calc(child, !axis);
        calc->want[1].length = calc->want[0].length * calc->aspect[1] / calc->aspect[0];
        break;
      case EFL_GFX_HINT_ASPECT_VERTICAL:
        if (!axis) _child_calc(child, !axis);
        calc->want[0].length = calc->want[1].length * calc->aspect[0] / calc->aspect[1];
        break;
      case EFL_GFX_HINT_ASPECT_BOTH:
        if (calc->state[!axis] != RELATIVE_CALC_ON)
          _child_calc(child, !axis);
        temph = calc->want[axis].length * calc->aspect[!axis] / calc->aspect[axis];
        if (temph > calc->want[!axis].length)
          {
             temph = calc->want[!axis].length;
             calc->want[axis].length = temph * calc->aspect[axis] / calc->aspect[!axis];
          }
        else
          calc->want[!axis].length = temph;
        break;
      default:
        if (calc->state[!axis] != RELATIVE_CALC_ON)
          _child_calc(child, !axis);
        temph = calc->want[axis].length * calc->aspect[!axis] / calc->aspect[axis];
        if (temph < calc->want[!axis].length)
          {
             temph = calc->want[!axis].length;
             calc->want[axis].length = temph * calc->aspect[axis] / calc->aspect[!axis];
          }
        else
          calc->want[!axis].length = temph;
     }

   //calculate max size
   if (calc->want[0].length > calc->max[0])
     {
        calc->want[0].length = calc->max[0];
        calc->want[1].length = calc->want[0].length * calc->aspect[1] / calc->aspect[0];
     }
   if (calc->want[1].length > calc->max[1])
     {
        calc->want[1].length = calc->max[1];
        calc->want[0].length = calc->want[1].length * calc->aspect[0] / calc->aspect[1];
     }
   //calculate min size
   if (calc->want[0].length < calc->min[0])
     {
        calc->want[0].length = calc->min[0];
        calc->want[1].length = calc->want[0].length * calc->aspect[1] / calc->aspect[0];
     }
   if (calc->want[1].length < calc->min[1])
     {
        calc->want[1].length = calc->min[1];
        calc->want[0].length = calc->want[1].length * calc->aspect[0] / calc->aspect[1];
     }

   //calculate align
   calc->want[!axis].position =
      calc->space[!axis].position +
      (calc->space[!axis].length - calc->want[!axis].length) * calc->align[!axis];
}

static Eina_Bool
_child_chain_calc(Efl_Ui_Relative_Layout_Child *child, Eina_Bool axis)
{
   Efl_Ui_Relative_Layout_Child *head, *tail, *o;
   Efl_Gfx_Hint_Aspect aspect_type;
   int space, min_sum = 0;
   double weight_sum = 0, cur_pos;
   Eina_Inlist *chain = NULL;

   if (child->calc.chain_state[axis] == RELATIVE_CALC_DONE)
     return EINA_TRUE;

   if ((child != child->calc.to[START]->calc.to[END]) &&
       (child != child->calc.to[END]->calc.to[START]))
     return EINA_FALSE;

   // find head
   head = child;
   while (head == head->calc.to[START]->calc.to[END])
     head = head->calc.to[START];

   //calculate weight_sum
   aspect_type = !axis ? EFL_GFX_HINT_ASPECT_VERTICAL : EFL_GFX_HINT_ASPECT_HORIZONTAL;
   o = head;
   do
     {
        if ((o->calc.aspect[0] > 0) && (o->calc.aspect[1] > 0) &&
            (o->calc.aspect_type == aspect_type))
          {
             _child_calc(o, !axis);
             if (o->calc.want[axis].length > o->calc.min[axis])
               o->calc.min[axis] = o->calc.want[axis].length;
          }
        else if ((o->calc.aspect[0] <= 0) ^ (o->calc.aspect[1] <= 0))
          {
             ERR("Invalid aspect parameter for obj(%p), aspect(%d, %d) ",
                 o->obj, o->calc.aspect[0], o->calc.aspect[1]);
          }

        o->calc.space[axis].length = o->calc.min[axis] +
                                     o->calc.margin[START] + o->calc.margin[END];
        min_sum += o->calc.space[axis].length;
        weight_sum += o->calc.weight[axis];

        tail = o;
        o = o->calc.to[END];
     }
   while (o->calc.to[START] == tail);

   _child_calc(head->calc.to[START], axis);
   _child_calc(tail->calc.to[END], axis);

   cur_pos = head->calc.to[START]->calc.want[axis].position +
             (head->calc.to[START]->calc.want[axis].length * head->rel[START].relative);
   space = tail->calc.to[END]->calc.want[axis].position +
           (tail->calc.to[END]->calc.want[axis].length * tail->rel[END].relative) - cur_pos;

   if ((space <= min_sum) || EINA_DBL_EQ(weight_sum, 0.0))
     cur_pos += (space - min_sum) * head->calc.align[axis];
   else
     {
        Efl_Ui_Relative_Layout_Calc *calc;
        double weight_len, orig_space = space, orig_weight = weight_sum;

        // Calculate compare factor
        for (o = head; o != tail->calc.to[END]; o = o->calc.to[END])
          {
             double denom;

             calc = &o->calc;
             denom = (calc->weight[axis] * orig_space) - (orig_weight * calc->min[axis]);
             if (denom > 0)
               {
                  calc->comp_factor = (calc->weight[axis] * orig_space) / denom;
                  chain = eina_inlist_sorted_insert(chain, EINA_INLIST_GET(calc),
                                                    _chain_sort_cb);
               }
             else
               {
                  space -= calc->space[axis].length;
                  weight_sum -= calc->weight[axis];
               }
          }

        EINA_INLIST_FOREACH(chain, calc)
          {
             weight_len = (space * calc->weight[axis]) / weight_sum;

             if (calc->space[axis].length < weight_len)
               calc->space[axis].length = weight_len;
             else
               {
                  weight_sum -= calc->weight[axis];
                  space -= calc->space[axis].length;
               }
          }
     }

   for (o = head; o != tail->calc.to[END]; o = o->calc.to[END])
     {
        o->calc.space[axis].position = cur_pos + o->calc.margin[START] + 0.5;
        cur_pos += o->calc.space[axis].length;
        o->calc.space[axis].length -= o->calc.margin[START] + o->calc.margin[END];
        o->calc.chain_state[axis] = RELATIVE_CALC_DONE;
     }

   return EINA_TRUE;
}

static void
_child_calc(Efl_Ui_Relative_Layout_Child *child, Eina_Bool axis)
{
   Efl_Ui_Relative_Layout_Calc *calc = &child->calc;

   if (calc->state[axis] == RELATIVE_CALC_DONE)
     return;

   if (calc->state[axis] == RELATIVE_CALC_ON)
     {
        ERR("%c-axis circular dependency when calculating part \"%s\"(%p).",
            axis ? 'Y' : 'X', efl_class_name_get(child->obj), child->obj);
        return;
     }

   calc->state[axis] = RELATIVE_CALC_ON;

   if (!_child_chain_calc(child, axis))
     {
        _child_calc(calc->to[START], axis);
        _child_calc(calc->to[END], axis);

        calc->space[axis].position = calc->to[START]->calc.want[axis].position
                        + (calc->to[START]->calc.want[axis].length * child->rel[START].relative)
                        + calc->margin[START];
        calc->space[axis].length = calc->to[END]->calc.want[axis].position
                        + (calc->to[END]->calc.want[axis].length * child->rel[END].relative)
                        - calc->margin[END] - calc->space[axis].position;
     }

   if (calc->fill[axis] && (calc->weight[axis] > 0))
     calc->want[axis].length = calc->space[axis].length;

   if (!calc->aspect[0] && !calc->aspect[1])
     {
        if (calc->want[axis].length > calc->max[axis])
          calc->want[axis].length = calc->max[axis];

        if (calc->want[axis].length < calc->min[axis])
          calc->want[axis].length = calc->min[axis];
     }
   else
     {
        _child_aspect_calc(child, axis);
     }

   //calculate align
   calc->want[axis].position =
      calc->space[axis].position +
      (calc->space[axis].length - calc->want[axis].length) * calc->align[axis];

   child->calc.state[axis] = RELATIVE_CALC_DONE;
}

static void
_hash_free_cb(void *data)
{
   Efl_Ui_Relative_Layout_Child *child = data;

   free(child);
}

static Eina_Bool
_hash_free_foreach_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
                      void *data, void *fdata EINA_UNUSED)
{
   Efl_Ui_Relative_Layout_Child *child = data;

   _elm_widget_sub_object_redirect_to_top(child->layout, child->obj);
   _hash_free_cb(child);

   return EINA_TRUE;
}

static Eina_Bool
_hash_child_calc_foreach_cb(const Eina_Hash *hash EINA_UNUSED, const void *key EINA_UNUSED,
                            void *data, void *fdata EINA_UNUSED)
{
   Efl_Ui_Relative_Layout_Child *child = data;
   Eina_Rect want;

   if (child->obj == child->layout)
     return EINA_TRUE;

   _child_calc(child, 0);
   _child_calc(child, 1);

   want.x = child->calc.want[0].position;
   want.w = child->calc.want[0].length;
   want.y = child->calc.want[1].position;
   want.h = child->calc.want[1].length;

   efl_gfx_entity_geometry_set(child->obj, want);
   return EINA_TRUE;
}


static Eina_Bool
_hash_child_init_foreach_cb(const Eina_Hash *hash, const void *key EINA_UNUSED,
                            void *data, void *fdata EINA_UNUSED)
{
   Eina_Size2D max, min, aspect;
   Efl_Ui_Relative_Layout_Child *child = data;
   Efl_Ui_Relative_Layout_Calc *calc = &(child->calc);

   calc->to[LEFT] = _relative_child_find(hash, child->rel[LEFT].to);
   if (!calc->to[LEFT]) calc->to[LEFT] = eina_hash_find(hash, &child->layout);
   calc->to[RIGHT] = _relative_child_find(hash, child->rel[RIGHT].to);
   if (!calc->to[RIGHT]) calc->to[RIGHT] = eina_hash_find(hash, &child->layout);
   calc->to[TOP] = _relative_child_find(hash, child->rel[TOP].to);
   if (!calc->to[TOP]) calc->to[TOP] = eina_hash_find(hash, &child->layout);
   calc->to[BOTTOM] = _relative_child_find(hash, child->rel[BOTTOM].to);
   if (!calc->to[BOTTOM]) calc->to[BOTTOM] = eina_hash_find(hash, &child->layout);

   if (child->obj == child->layout)
     {
        Eina_Rect want = efl_gfx_entity_geometry_get(child->obj);
        calc->want[0].position = want.x;
        calc->want[0].length = want.w;
        calc->want[1].position = want.y;
        calc->want[1].length = want.h;
        calc->state[0] = RELATIVE_CALC_DONE;
        calc->state[1] = RELATIVE_CALC_DONE;
        calc->chain_state[0] = RELATIVE_CALC_DONE;
        calc->chain_state[1] = RELATIVE_CALC_DONE;
        return EINA_TRUE;
     }

   calc->state[0] = RELATIVE_CALC_NONE;
   calc->state[1] = RELATIVE_CALC_NONE;
   calc->chain_state[0] = RELATIVE_CALC_NONE;
   calc->chain_state[1] = RELATIVE_CALC_NONE;

   efl_gfx_hint_weight_get(child->obj, &calc->weight[0], &calc->weight[1]);
   efl_gfx_hint_align_get(child->obj, &calc->align[0], &calc->align[1]);
   efl_gfx_hint_fill_get(child->obj, &calc->fill[0], &calc->fill[1]);
   efl_gfx_hint_aspect_get(child->obj, &calc->aspect_type, &aspect);
   calc->aspect[0] = aspect.w;
   calc->aspect[1] = aspect.h;
   efl_gfx_hint_margin_get(child->obj, &calc->margin[LEFT], &calc->margin[RIGHT],
                           &calc->margin[TOP], &calc->margin[BOTTOM]);
   max = efl_gfx_hint_size_max_get(child->obj);
   min = efl_gfx_hint_size_combined_min_get(child->obj);
   calc->max[0] = max.w;
   calc->max[1] = max.h;
   calc->min[0] = min.w;
   calc->min[1] = min.h;

   calc->want[0].position = 0;
   calc->want[0].length = 0;
   calc->want[1].position = 0;
   calc->want[1].length = 0;
   calc->space[0].position = 0;
   calc->space[0].length = 0;
   calc->space[1].position = 0;
   calc->space[1].length = 0;

   if (calc->weight[0] < 0) calc->weight[0] = 0;
   if (calc->weight[1] < 0) calc->weight[1] = 0;

   if (calc->align[0] < 0) calc->align[0] = 0;
   if (calc->align[1] < 0) calc->align[1] = 0;
   if (calc->align[0] > 1) calc->align[0] = 1;
   if (calc->align[1] > 1) calc->align[1] = 1;

   if (calc->max[0] < 0) calc->max[0] = INT_MAX;
   if (calc->max[1] < 0) calc->max[1] = INT_MAX;
   if (calc->aspect[0] < 0) calc->aspect[0] = 0;
   if (calc->aspect[1] < 0) calc->aspect[1] = 0;

   return EINA_TRUE;
}

static void
_on_size_hints_changed(void *data EINA_UNUSED, Evas *e EINA_UNUSED,
                       Evas_Object *obj, void *event_info EINA_UNUSED)
{
   efl_pack_layout_request(obj);
}

EOLIAN static void
_efl_ui_relative_layout_efl_pack_layout_layout_update(Eo *obj, Efl_Ui_Relative_Layout_Data *pd)
{
   eina_hash_foreach(pd->children, _hash_child_init_foreach_cb, NULL);
   eina_hash_foreach(pd->children, _hash_child_calc_foreach_cb, NULL);

   efl_event_callback_call(obj, EFL_PACK_EVENT_LAYOUT_UPDATED, NULL);
}

EOLIAN static void
_efl_ui_relative_layout_efl_pack_layout_layout_request(Eo *obj, Efl_Ui_Relative_Layout_Data *pd EINA_UNUSED)
{
   efl_canvas_group_need_recalculate_set(obj, EINA_TRUE);
}

EOLIAN static void
_efl_ui_relative_layout_efl_canvas_group_group_calculate(Eo *obj, Efl_Ui_Relative_Layout_Data *pd EINA_UNUSED)
{
   efl_pack_layout_update(obj);
}

EOLIAN static void
_efl_ui_relative_layout_efl_gfx_entity_size_set(Eo *obj, Efl_Ui_Relative_Layout_Data *pd EINA_UNUSED, Eina_Size2D sz)
{
   efl_gfx_entity_size_set(efl_super(obj, MY_CLASS), sz);
   efl_canvas_group_change(obj);
}

EOLIAN static void
_efl_ui_relative_layout_efl_gfx_entity_position_set(Eo *obj, Efl_Ui_Relative_Layout_Data *pd EINA_UNUSED, Eina_Position2D pos)
{
   efl_gfx_entity_position_set(efl_super(obj, MY_CLASS), pos);
   efl_canvas_group_change(obj);
}

EOLIAN static void
_efl_ui_relative_layout_efl_canvas_group_group_add(Eo *obj, Efl_Ui_Relative_Layout_Data *pd EINA_UNUSED)
{
   evas_object_event_callback_add(obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_size_hints_changed, NULL);
   efl_canvas_group_add(efl_super(obj, MY_CLASS));
   elm_widget_sub_object_parent_add(obj);

   elm_widget_highlight_ignore_set(obj, EINA_TRUE);
}

EOLIAN static Eo *
_efl_ui_relative_layout_efl_object_constructor(Eo *obj, Efl_Ui_Relative_Layout_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   efl_canvas_object_type_set(obj, MY_CLASS_NAME);
   efl_access_object_access_type_set(obj, EFL_ACCESS_TYPE_SKIPPED);
   efl_access_object_role_set(obj, EFL_ACCESS_ROLE_FILLER);

   pd->obj = obj;
   pd->children = eina_hash_pointer_new(_hash_free_cb);
   _efl_ui_relative_layout_register(pd, obj);

   return obj;
}

EOLIAN static void
_efl_ui_relative_layout_efl_object_destructor(Eo *obj, Efl_Ui_Relative_Layout_Data *pd)
{
   eina_hash_free(pd->children);
   efl_destructor(efl_super(obj, MY_CLASS));
}

EOLIAN static void
_efl_ui_relative_layout_unregister(Eo *obj, Efl_Ui_Relative_Layout_Data *pd, Efl_Object *child)
{
   _elm_widget_sub_object_redirect_to_top(obj, child);
   if (!eina_hash_del_by_key(pd->children, &child))
     ERR("child(%p(%s)) is not registered", child, efl_class_name_get(child));
}

EOLIAN static void
_efl_ui_relative_layout_unregister_all(Eo *obj EINA_UNUSED, Efl_Ui_Relative_Layout_Data *pd)
{
   eina_hash_foreach(pd->children, _hash_free_foreach_cb, NULL);
}

EOLIAN static Eina_Iterator *
_efl_ui_relative_layout_children_iterate(Eo *obj EINA_UNUSED, Efl_Ui_Relative_Layout_Data *pd)
{
   return eina_hash_iterator_data_new(pd->children);
}

EFL_UI_RELATIVE_LAYOUT_RELATION_SET_GET(left, LEFT);
EFL_UI_RELATIVE_LAYOUT_RELATION_SET_GET(right, RIGHT);
EFL_UI_RELATIVE_LAYOUT_RELATION_SET_GET(top, TOP);
EFL_UI_RELATIVE_LAYOUT_RELATION_SET_GET(bottom, BOTTOM);

/* Internal EO APIs and hidden overrides */

#define EFL_UI_RELATIVE_LAYOUT_EXTRA_OPS \
   EFL_CANVAS_GROUP_ADD_OPS(efl_ui_relative_layout)

#include "efl_ui_relative_layout.eo.c"
