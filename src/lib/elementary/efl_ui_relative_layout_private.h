#ifndef EFL_UI_RELATIVE_LAYOUT_PRIVATE_H
#define EFL_UI_RELATIVE_LAYOUT_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define EFL_PACK_LAYOUT_PROTECTED

#include <Elementary.h>
#include "elm_priv.h"

typedef enum _Efl_Ui_Relative_Layout_Calc_State
{
   RELATIVE_CALC_NONE,
   RELATIVE_CALC_DONE,
   RELATIVE_CALC_ON
} Efl_Ui_Relative_Layout_Calc_State;

typedef struct _Efl_Ui_Relative_Layout_Data        Efl_Ui_Relative_Layout_Data;
typedef struct _Efl_Ui_Relative_Layout_Child       Efl_Ui_Relative_Layout_Child;
typedef struct _Efl_Ui_Relative_Layout_Calc        Efl_Ui_Relative_Layout_Calc;
typedef struct _Efl_Ui_Relative_Layout_Relation    Efl_Ui_Relative_Layout_Relation;

struct _Efl_Ui_Relative_Layout_Calc
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

   struct {
      int position;
      double length;
   } space[2], want[2];

   Efl_Ui_Relative_Layout_Calc_State  state[2];
   Efl_Ui_Relative_Layout_Calc_State  chain_state[2];
   Efl_Ui_Relative_Layout_Child      *to[4];
};

struct _Efl_Ui_Relative_Layout_Data
{
   Eo        *obj;
   Eo        *clipper;
   Eina_Hash *children;
   Efl_Ui_Relative_Layout_Child *base;
};

struct _Efl_Ui_Relative_Layout_Relation
{
   Efl_Object *to;
   double relative;
};

struct _Efl_Ui_Relative_Layout_Child
{
   Eo                               *obj;
   Eo                               *layout;
   Efl_Ui_Relative_Layout_Relation   rel[4];
   Efl_Ui_Relative_Layout_Calc       calc;
};

#define EFL_UI_RELATIVE_LAYOUT_RELATION_SET_GET(direction, DIRECTION) \
   EOLIAN static void \
   _efl_ui_relative_layout_relation_ ## direction ## _set(Eo *obj EINA_UNUSED, Efl_Ui_Relative_Layout_Data *pd, Eo *child, Eo *target, double relative) \
   { \
      Efl_Ui_Relative_Layout_Child *rc; \
      rc = _relative_child_get(pd, child); \
      if (target) rc->rel[DIRECTION].to = target; \
      if (relative < 0) relative = 0; \
      else if (relative > 1) relative = 1; \
      rc->rel[DIRECTION].relative = relative; \
   } \
   \
   EOLIAN static void \
   _efl_ui_relative_layout_relation_ ## direction ## _get(const Eo *obj EINA_UNUSED, Efl_Ui_Relative_Layout_Data *pd, Eo *child, Eo **target, double *relative) \
   { \
      Efl_Ui_Relative_Layout_Child *rc; \
      rc = _relative_child_get(pd, child); \
      if (target) *target = rc->rel[DIRECTION].to; \
      if (relative) *relative = rc->rel[DIRECTION].relative; \
   }

#endif
