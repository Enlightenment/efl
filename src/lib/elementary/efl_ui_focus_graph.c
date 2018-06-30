#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#define EFL_UI_FOCUS_OBJECT_PROTECTED


#include <Elementary.h>
#include "elm_priv.h"
#include "efl_ui_focus_graph.h"

typedef enum {
  Q_TOP = 1, Q_RIGHT = 2, Q_LEFT = 4, Q_BOTTOM = 8, Q_LAST = 16
} Quadrant;

Quadrant q_helper[] = {Q_TOP, Q_RIGHT, Q_LEFT, Q_BOTTOM};

static inline Efl_Ui_Focus_Object*
_convert(Efl_Ui_Focus_Graph_Context *ctx, Opaque_Graph_Member *member)
{
   return *((Efl_Ui_Focus_Object**)(((char*) member) + ctx->offset_focusable));
}

static inline unsigned int
_distance(Eina_Rect o, Eina_Rect r2, Quadrant q)
{
   int res = INT_MAX;
   if (q == Q_TOP)
     res = o.y - eina_rectangle_max_y(&r2.rect);
   else if (q == Q_LEFT)
     res = o.x - eina_rectangle_max_x(&r2.rect);
   else if (q == Q_BOTTOM)
     res = r2.y - eina_rectangle_max_y(&o.rect);
   else if (q == Q_RIGHT)
     res = r2.x - eina_rectangle_max_x(&o.rect);

   return res;
}

static inline Efl_Ui_Focus_Graph_Calc_Direction_Result*
_result_get(Quadrant q, Efl_Ui_Focus_Graph_Calc_Result *result)
{
  if (q == Q_TOP) return &result->top;
  else if (q == Q_LEFT) return &result->left;
  else if (q == Q_BOTTOM) return &result->bottom;
  else if (q == Q_RIGHT) return &result->right;
  else return NULL;
}

/*

_quadrant_get return in which quadrant the elem is placed, oriented at origin

All this is based on three levels

lvl1:
       |     |
       | Top |
  _____|_____|_____
       |     |
  left |clean|right
  _____|_____|_____
       |     |
       | Bot |
       |     |
lvl3:
  \               /
    \    Top    /
      \ _____ /
       |     |
  left |clean|right
       |_____|
      /       \
    /   Bot     \
  /               \

lvl3:
       |     |
  L & T| Top | R & T
  _____|_____|_____
       |     |
  left |clean|right
  _____|_____|_____
       |     |
  L & B| Bot | R & B
       |     |


 */

static inline void
_quadrant_get(Eina_Rect origin, Eina_Rect elem, Quadrant *lvl2, Quadrant *lvl1, Quadrant *lvl3)
{
  int dis = 0;

  *lvl1 = 0;
  *lvl2 = 0;
  *lvl3 = 0;

  if (eina_rectangle_max_y(&elem.rect) <= origin.y)
    {
       dis = origin.y - elem.y;

       *lvl3 |= Q_TOP;

       if (eina_spans_intersect(origin.x - dis, 2*dis + origin.w, elem.x, elem.w))
         *lvl2 |= Q_TOP;
       if (eina_spans_intersect(origin.x, origin.w, elem.x, elem.w))
         *lvl1 |= Q_TOP;
    }
  if (elem.y >= eina_rectangle_max_y(&origin.rect))
    {
       dis = eina_rectangle_max_y(&elem.rect) - origin.y;

       *lvl3 |= Q_BOTTOM;

       if (eina_spans_intersect(origin.x - dis, 2*dis + origin.w, elem.x, elem.w))
         *lvl2 |= Q_BOTTOM;
       if (eina_spans_intersect(origin.x, origin.w, elem.x, elem.w))
         *lvl1 |= Q_BOTTOM;
    }
  if (elem.x >= eina_rectangle_max_x(&origin.rect))
    {
       dis = eina_rectangle_max_x(&elem.rect) - origin.x;

       *lvl3 |= Q_RIGHT;

       if (eina_spans_intersect(origin.y - dis, 2*dis + origin.h, elem.y, elem.h))
         *lvl2 |= Q_RIGHT;
       if (eina_spans_intersect(origin.y, origin.h, elem.y, elem.h))
         *lvl1 |= Q_RIGHT;
    }
  if (eina_rectangle_max_x(&elem.rect) <= origin.x)
    {
       dis = origin.x - elem.x;

       *lvl3 |= Q_LEFT;

       if (eina_spans_intersect(origin.y - dis, 2*dis + origin.h, elem.y, elem.h))
         *lvl2 |= Q_LEFT;
       if (eina_spans_intersect(origin.y, origin.h, elem.y, elem.h))
         *lvl1 |= Q_LEFT;
    }

}

void
efl_ui_focus_graph_calc(Efl_Ui_Focus_Graph_Context *ctx, Eina_Iterator *nodes, Opaque_Graph_Member *origin_obj, Efl_Ui_Focus_Graph_Calc_Result *result)
{
   Opaque_Graph_Member *elem_obj;
   Eina_Rect origin, elem;

   for (int i = 0; i < 4; ++i)
     {
        Efl_Ui_Focus_Graph_Calc_Direction_Result *res;

        res = _result_get(q_helper[i], result);

        res->distance = INT_MAX;
        res->lvl = INT_MAX;
        res->relation = NULL;
     }

   origin = efl_ui_focus_object_focus_geometry_get(_convert(ctx, origin_obj));

   //printf("=========> CALCING %p %s\n", _convert(ctx, origin_obj), efl_class_name_get(_convert(ctx, origin_obj)));

   EINA_ITERATOR_FOREACH(nodes, elem_obj)
     {
        Efl_Ui_Focus_Graph_Calc_Direction_Result *res;
        unsigned int distance;
        Quadrant lvl3, lvl2, lvl1;

        if (elem_obj == origin_obj) continue;

        elem = efl_ui_focus_object_focus_geometry_get(_convert(ctx, elem_obj));

        if (eina_rectangle_intersection(&origin.rect, &elem.rect)) continue;

        _quadrant_get(origin, elem, &lvl2, &lvl1, &lvl3);

        for (int i = 0; i < 4; ++i)
          {
            if (q_helper[i] & lvl3)
              {
                 int lvl;
                 res = _result_get(q_helper[i], result);
                 EINA_SAFETY_ON_NULL_GOTO(res, cont);

                 distance = _distance(origin, elem, q_helper[i]);

                 if (q_helper[i] & lvl1)
                   lvl = 1;
                 else if (q_helper[i] & lvl2)
                   lvl = 2;
                 else //if (q_helper[i] & lvl3)
                   lvl = 3;

                 if (lvl < res->lvl)
                   {
                      res->relation = eina_list_free(res->relation);
                      res->relation = eina_list_append(res->relation, elem_obj);
                      res->distance = distance;
                      res->lvl = lvl;
                      //printf("=========> %p:%d LVL_DROP     %d \t %d \t %p %s\n", res, i, distance, lvl, origin_obj, efl_class_name_get(_convert(ctx, elem_obj)));
                   }
                 else if (lvl == res->lvl && res->distance > distance)
                   {
                      res->relation = eina_list_free(res->relation);
                      res->relation = eina_list_append(res->relation, elem_obj);
                      res->distance = distance;
                      //printf("=========> %p:%d DIST_DROP    %d \t %d \t %p %s\n", res, i, distance, lvl, origin_obj, efl_class_name_get(_convert(ctx, elem_obj)));

                   }
                 else if (lvl == res->lvl && res->distance >= distance)
                   {
                      res->relation = eina_list_append(res->relation, elem_obj);
                      //printf("=========> %p:%d DIST_ADD     %d \t %d \t %p %s\n", res, i, distance, lvl, origin_obj, efl_class_name_get(_convert(ctx, elem_obj)));
                   }
              }
          }

        continue;
cont:
        continue;
     }
}
