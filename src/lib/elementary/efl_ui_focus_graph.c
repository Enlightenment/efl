#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_UI_FOCUS_OBJECT_PROTECTED


#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_focus_graph.h"

static inline Efl_Ui_Focus_Object*
_convert(Efl_Ui_Focus_Graph_Context *ctx, Opaque_Graph_Member *member)
{
   return *((Efl_Ui_Focus_Object**)(((char*) member) + ctx->offset_focusable));
}


static inline Eina_Position2D
_middle(Eina_Rect rect)
{
  return EINA_POSITION2D(rect.x + rect.w/2, rect.y + rect.h/2);
}

static inline Eina_Position2D
_minus(Eina_Position2D r1, Eina_Position2D r2)
{
  return EINA_POSITION2D(r2.x - r1.x, r2.y - r1.y);
}

static inline unsigned int
_order(Eina_Position2D pos)
{
   return pow(pos.x, 2) + pow(pos.y, 2);
}

static inline Efl_Ui_Focus_Graph_Calc_Direction_Result*
_quadrant_get(Eina_Position2D pos, Efl_Ui_Focus_Graph_Calc_Result *result)
{
  if (pos.y > abs(pos.x)) return &result->top;
  else if (pos.x >= abs(pos.y)) return &result->left;
  else if (pos.y < -abs(pos.x)) return &result->bottom;
  else if (pos.y >= -abs(pos.x)) return &result->right;
  else return NULL;
}

typedef struct {
   unsigned int distance;
} Direction_Calc_Result;

void
efl_ui_focus_graph_calc(Efl_Ui_Focus_Graph_Context *ctx, Eina_Iterator *nodes, Opaque_Graph_Member *origin_obj, Efl_Ui_Focus_Graph_Calc_Result *result)
{
   Opaque_Graph_Member *elem_obj;
   Eina_Position2D origin_pos, elem_pos, relative_pos;
   Eina_Rect origin, elem;

   memset(result, 0, sizeof(Efl_Ui_Focus_Graph_Calc_Result));

   origin = efl_ui_focus_object_focus_geometry_get(_convert(ctx, origin_obj));
   origin_pos = _middle(origin);

   EINA_ITERATOR_FOREACH(nodes, elem_obj)
     {
        Efl_Ui_Focus_Graph_Calc_Direction_Result *res;
        unsigned int distance;

        if (elem_obj == origin_obj) continue;

        elem = efl_ui_focus_object_focus_geometry_get(_convert(ctx, elem_obj));
        elem_pos = _middle(elem);
        relative_pos = _minus(elem_pos, origin_pos);
        distance = _order(relative_pos);
        res = _quadrant_get(relative_pos, result);
        EINA_SAFETY_ON_NULL_GOTO(res, cont);

        if (res->distance > distance || res->distance == 0)
          {
             res->relation = eina_list_free(res->relation);
             res->relation = eina_list_append(res->relation, elem_obj);
             res->distance = distance;
          }
        else if (res->distance == distance)
          {
             res->relation = eina_list_append(res->relation, elem_obj);
          }
        continue;
cont:
        printf("%d - %d\n", relative_pos.x, relative_pos.y);
        continue;
     }
}
