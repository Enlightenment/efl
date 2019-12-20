#ifndef EFL_UI_RELATIVE_CONTAINER_PRIVATE_H
#define EFL_UI_RELATIVE_CONTAINER_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef enum _Efl_Ui_Relative_Container_Calc_State
{
   RELATIVE_CALC_NONE,
   RELATIVE_CALC_DONE,
   RELATIVE_CALC_ON
} Efl_Ui_Relative_Container_Calc_State;

typedef struct _Efl_Ui_Relative_Container_Data        Efl_Ui_Relative_Container_Data;
typedef struct _Efl_Ui_Relative_Container_Child       Efl_Ui_Relative_Container_Child;
typedef struct _Efl_Ui_Relative_Container_Calc        Efl_Ui_Relative_Container_Calc;
typedef struct _Efl_Ui_Relative_Container_Relation    Efl_Ui_Relative_Container_Relation;
typedef struct _Efl_Ui_Relative_Container_Content_Iterator Efl_Ui_Relative_Container_Content_Iterator;

struct _Efl_Ui_Relative_Container_Calc
{
   EINA_INLIST;

   int                                max[2];
   int                                min[2];
   int                                aspect[2];
   int                                margin[4];
   Efl_Gfx_Hint_Aspect                aspect_type;
   Eina_Bool                          fill[2];
   double                             weight[2];
   double                             align[2];
   double                             comp_factor;
   /*  m0 is static min size which is added to the other children min size.
    * only if both (target, relative)[0] and (target, relative)[1] are same,
    * it has non-zero value. it is calculated as (min * (align / relative)) if
    * align is greater than relative, (min * ((1 - align) / (1 - relative))) otherwise.
    *  mi, mj are transformed relative based on layout min size. they are
    * calculated as (target.mi + (relative * (target.mj - target.mi))). for example,
    * there are two children of relative_container that has different target base.
    *  |              | obj1  | obj2 |
    *  |      min     | 100   | 100  |
    *  |left.target   | layout| obj1 |
    *  |left.relative | 0.0   | 0.5  |
    *  |right.target  | layout| obj1 |
    *  |right.relative| 0.5   | 1.0  |
    *  |      mi      | 0.0   | 0.25 |
    *  |      mj      | 0.5   | 0.5  |
    *
    * obj1.mi = layout.mi(0.0) + (obj1.relative(0.0) * (LAyout.mj(1.0) - layout.mi(0.0))) = 0.0
    * obj1.mj = layout.mi(0.0) + (obj1.relative(0.5) * (layout.mj(1.0) - layout.mi(0.0))) = 0.5
    * obj2.mi = obj1.mi(0.0) + (obj2.relative(0.5) * (obj1.mj(0.5) - obj1.mi(0.0))) = 0.25
    * obj2.mj = obj1.mi(0.0) + (obj2.relative(1.0) * (obj1.mj(0.5) - obj1.mi(0.0))) = 0.5
    *  layout min size is calculated as maximum of (child_min + m0) / (mj - mi).
    * in the example, obj1 require layout min size as
    * ((child_min(100) + m0(0)) / (mj(0.5) - mi(0.0))) = 200. obj2 require
    * layout min size as ((100 + 0) / (0.5 - 0.25)) = 400. as a result, layout
    * min size is max(200, 400) = 400.
    */
   double                             m0[2];
   double                             mi[2], mj[2];

   struct {
      int position;
      double length;
   } space[2], want[2];

   Efl_Ui_Relative_Container_Calc_State  state[2];
   Efl_Ui_Relative_Container_Calc_State  chain_state[2];
   Efl_Ui_Relative_Container_Child      *to[4];
};

struct _Efl_Ui_Relative_Container_Data
{
   Eo        *obj;
   Eo        *clipper;
   Eina_Hash *children;
   Efl_Ui_Relative_Container_Child *base;
};

struct _Efl_Ui_Relative_Container_Relation
{
   Efl_Object *to;
   double relative_position;
};

struct _Efl_Ui_Relative_Container_Child
{
   Eo                               *obj;
   Eo                               *layout;
   Efl_Ui_Relative_Container_Relation   rel[4];
   Efl_Ui_Relative_Container_Calc       calc;
};

struct _Efl_Ui_Relative_Container_Content_Iterator
{
   Eina_Iterator  iterator;
   Eina_Iterator *real_iterator;
   Eo            *relative_container;
};

#define EFL_UI_RELATIVE_CONTAINER_RELATION_SET_GET(direction, DIRECTION) \
   EOLIAN static void \
   _efl_ui_relative_container_relation_ ## direction ## _set(Eo *obj, Efl_Ui_Relative_Container_Data *pd, Eo *child, Eo *target, double relative_position) \
   { \
      Efl_Ui_Relative_Container_Child *rc; \
      if (!child) return; \
      rc = _relative_child_get(pd, child); \
      if (!rc) return; \
      if (target) rc->rel[DIRECTION].to = target; \
      if (relative_position < 0) relative_position = 0; \
      else if (relative_position > 1) relative_position = 1; \
      rc->rel[DIRECTION].relative_position = relative_position; \
      efl_pack_layout_request(obj); \
   } \
   \
   EOLIAN static void \
   _efl_ui_relative_container_relation_ ## direction ## _get(const Eo *obj EINA_UNUSED, Efl_Ui_Relative_Container_Data *pd, Eo *child, Eo **target, double *relative_position) \
   { \
      Efl_Ui_Relative_Container_Child *rc; \
      Eo *rel_to = NULL; \
      double rel_relative = 0.0; \
      rc = eina_hash_find(pd->children, &child); \
      if (rc) \
        { \
           rel_to = rc->rel[DIRECTION].to; \
           rel_relative = rc->rel[DIRECTION].relative_position; \
        } \
      else \
        ERR("child(%p(%s)) is not registered", child, efl_class_name_get(child)); \
      if (target) *target = rel_to; \
      if (relative_position) *relative_position = rel_relative; \
   }

#endif
