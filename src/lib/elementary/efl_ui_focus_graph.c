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

static inline void
_quadrant_get(Eina_Rect origin, Eina_Rect elem, Quadrant *all, Quadrant *clean)
{
  int dis = 0;

  *clean = 0;
  *all = 0;

  if (eina_rectangle_max_y(&elem.rect) <= origin.y)
    {
       dis = origin.y - elem.y;

       if (eina_spans_intersect(origin.x - dis, 2*dis + origin.w, elem.x, elem.w))
         *all |= Q_TOP;
       if (eina_spans_intersect(origin.x, origin.w, elem.x, elem.w))
         *clean |= Q_TOP;
    }
  if (elem.y >= eina_rectangle_max_y(&origin.rect))
    {
       dis = eina_rectangle_max_y(&elem.rect) - origin.y;

       if (eina_spans_intersect(origin.x - dis, 2*dis + origin.w, elem.x, elem.w))
         *all |= Q_BOTTOM;
       if (eina_spans_intersect(origin.x, origin.w, elem.x, elem.w))
         *clean |= Q_BOTTOM;
    }
  if (elem.x >= eina_rectangle_max_x(&origin.rect))
    {
       dis = eina_rectangle_max_x(&elem.rect) - origin.x;

       if (eina_spans_intersect(origin.y - dis, 2*dis + origin.h, elem.y, elem.h))
         *all |= Q_RIGHT;
       if (eina_spans_intersect(origin.y, origin.h, elem.y, elem.h))
         *clean |= Q_RIGHT;
    }
  if (eina_rectangle_max_x(&elem.rect) <= origin.x)
    {
       dis = origin.x - elem.x;

       if (eina_spans_intersect(origin.y - dis, 2*dis + origin.h, elem.y, elem.h))
         *all |= Q_LEFT;
       if (eina_spans_intersect(origin.y, origin.h, elem.y, elem.h))
         *clean |= Q_LEFT;
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

        res->clean = EINA_FALSE;
        res->distance = INT_MAX;
        res->relation = NULL;
     }

   origin = efl_ui_focus_object_focus_geometry_get(_convert(ctx, origin_obj));

   //printf("=========> CALCING %p %s\n", _convert(ctx, origin_obj), elm_object_text_get(_convert(ctx, origin_obj)));

   EINA_ITERATOR_FOREACH(nodes, elem_obj)
     {
        Efl_Ui_Focus_Graph_Calc_Direction_Result *res;
        unsigned int distance;
        Quadrant all;
        Quadrant clean;

        if (elem_obj == origin_obj) continue;

        elem = efl_ui_focus_object_focus_geometry_get(_convert(ctx, elem_obj));

        if (eina_rectangle_intersection(&origin.rect, &elem.rect)) continue;

        _quadrant_get(origin, elem, &all, &clean);

        for (int i = 0; i < 4; ++i)
          {
            if (q_helper[i] & all)
              {
                 res = _result_get(q_helper[i], result);
                 EINA_SAFETY_ON_NULL_GOTO(res, cont);

                 distance = _distance(origin, elem, q_helper[i]);


                 if ((res->distance > distance && res->clean == (q_helper[i] & clean)) ||
                     (!res->clean && q_helper[i] & clean))
                   {
                      res->relation = eina_list_free(res->relation);
                      res->relation = eina_list_append(res->relation, elem_obj);
                      res->distance = distance;
                      res->clean = q_helper[i] & clean;
                      //printf("=========> %p BETTER_NOW     %d \t %p %s\n", res, distance, origin_obj, elm_object_text_get(_convert(ctx, elem_obj)));
                   }
                 else if (res->distance == distance)
                   {
                      res->relation = eina_list_append(res->relation, elem_obj);
                      //printf("=========> %p BETTER_NOW_ADD %d \t %p %s\n", res, distance, origin_obj, elm_object_text_get(_convert(ctx, elem_obj)));
                   }
              }
          }

        continue;
cont:
        continue;
     }
}
