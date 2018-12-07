#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <math.h>
#include <float.h>

#include <Eina.h>
#include <Ector.h>
#include <software/Ector_Software.h>

#include "ector_private.h"
#include "ector_software_private.h"

#define MY_CLASS ECTOR_RENDERER_SOFTWARE_SHAPE_CLASS

typedef struct _Ector_Renderer_Software_Shape_Data Ector_Renderer_Software_Shape_Data;
typedef struct _Ector_Software_Shape_Task Ector_Software_Shape_Task;

struct _Ector_Software_Shape_Task
{
   Ector_Renderer_Software_Shape_Data *pd;

   const Efl_Gfx_Path_Command *cmds;
   const double *pts;

   Efl_Gfx_Fill_Rule fill_rule;
};

struct _Ector_Renderer_Software_Shape_Data
{
   Efl_Gfx_Shape_Public        *public_shape;

   Ector_Software_Surface_Data *surface;
   Ector_Renderer_Shape_Data   *shape;
   Ector_Renderer_Data         *base;

   Shape_Rle_Data              *shape_data;
   Shape_Rle_Data              *outline_data;

   Ector_Software_Shape_Task   *task;

   Eina_Bool                    done;
};

typedef struct _Outline
{
   SW_FT_Outline ft_outline;
   int points_alloc;
   int contours_alloc;
} Outline;


#define TO_FT_COORD(x) ((x) * 64) // to freetype 26.6 coordinate.

static inline void
_grow_outline_contour(Outline *outline, int num)
{
   if ( outline->ft_outline.n_contours + num > outline->contours_alloc)
     {
        outline->contours_alloc += 5;
        outline->ft_outline.contours = (short *) realloc(outline->ft_outline.contours,
                                                         outline->contours_alloc * sizeof(short));
     }
}

static inline void
_grow_outline_points(Outline *outline, int num)
{
   if ( outline->ft_outline.n_points + num > outline->points_alloc)
     {
        outline->points_alloc += 50;
        outline->ft_outline.points = (SW_FT_Vector *) realloc(outline->ft_outline.points,
                                                              outline->points_alloc * sizeof(SW_FT_Vector));
        outline->ft_outline.tags = (char *) realloc(outline->ft_outline.tags,
                                                    outline->points_alloc * sizeof(char));
     }
}
static Outline *
_outline_create()
{
   Outline *outline = (Outline *) calloc(1, sizeof(Outline));
   outline->points_alloc = 0;
   outline->contours_alloc = 0;
   _grow_outline_contour(outline, 1);
   _grow_outline_points(outline, 1);
   return outline;
}

static
void _outline_destroy(Outline *outline)
{
   if (outline)
     {
        free(outline->ft_outline.points);
        free(outline->ft_outline.tags);
        free(outline->ft_outline.contours);
        free(outline);
     }
}

static void
_outline_move_to(Outline *outline, double x, double y)
{
   SW_FT_Outline *ft_outline = &outline->ft_outline;

   _grow_outline_points(outline, 1);
   ft_outline->points[ft_outline->n_points].x = TO_FT_COORD(x);
   ft_outline->points[ft_outline->n_points].y = TO_FT_COORD(y);
   ft_outline->tags[ft_outline->n_points] = SW_FT_CURVE_TAG_ON;

   if (ft_outline->n_points)
     {
        _grow_outline_contour(outline, 1);
        ft_outline->contours[ft_outline->n_contours] = ft_outline->n_points - 1;
        ft_outline->n_contours++;
     }

   ft_outline->n_points++;
}

static void
_outline_end(Outline *outline)
{
   SW_FT_Outline *ft_outline = &outline->ft_outline;

   _grow_outline_contour(outline, 1);

   if (ft_outline->n_points)
     {
        ft_outline->contours[ft_outline->n_contours] = ft_outline->n_points - 1;
        ft_outline->n_contours++;
     }
}


static void  _outline_line_to(Outline *outline, double x, double y)
{
   SW_FT_Outline *ft_outline = &outline->ft_outline;

   _grow_outline_points(outline, 1);
   ft_outline->points[ft_outline->n_points].x = TO_FT_COORD(x);
   ft_outline->points[ft_outline->n_points].y = TO_FT_COORD(y);
   ft_outline->tags[ft_outline->n_points] = SW_FT_CURVE_TAG_ON;
   ft_outline->n_points++;
}


static Eina_Bool
_outline_close_path(Outline *outline)
{
   SW_FT_Outline *ft_outline = &outline->ft_outline;
   int index ;

   if (ft_outline->n_contours)
     {
        index = ft_outline->contours[ft_outline->n_contours - 1] + 1;
     }
   else
     {
        // First path
        index = 0;
     }

   // Make sure there is at least one point in the current path
   if (ft_outline->n_points == index) return EINA_FALSE;

   // Close the path
   _grow_outline_points(outline, 1);
   ft_outline->points[ft_outline->n_points].x = ft_outline->points[index].x;
   ft_outline->points[ft_outline->n_points].y = ft_outline->points[index].y;
   ft_outline->tags[ft_outline->n_points] = SW_FT_CURVE_TAG_ON;
   ft_outline->n_points++;

   return EINA_TRUE;
}


static void _outline_cubic_to(Outline *outline, double cx1, double cy1,
                              double cx2, double cy2, double x, double y)
{
   SW_FT_Outline *ft_outline = &outline->ft_outline;

   _grow_outline_points(outline, 3);

   ft_outline->points[ft_outline->n_points].x = TO_FT_COORD(cx1);
   ft_outline->points[ft_outline->n_points].y = TO_FT_COORD(cy1);
   ft_outline->tags[ft_outline->n_points] = SW_FT_CURVE_TAG_CUBIC;
   ft_outline->n_points++;

   ft_outline->points[ft_outline->n_points].x = TO_FT_COORD(cx2);
   ft_outline->points[ft_outline->n_points].y = TO_FT_COORD(cy2);
   ft_outline->tags[ft_outline->n_points] = SW_FT_CURVE_TAG_CUBIC;
   ft_outline->n_points++;

   ft_outline->points[ft_outline->n_points].x = TO_FT_COORD(x);
   ft_outline->points[ft_outline->n_points].y = TO_FT_COORD(y);
   ft_outline->tags[ft_outline->n_points] = SW_FT_CURVE_TAG_ON;
   ft_outline->n_points++;
}

static void _outline_transform(Outline *outline, Eina_Matrix3 *m)
{
   int i;
   double x, y;
   SW_FT_Outline *ft_outline = &outline->ft_outline;

   if (m && (eina_matrix3_type_get(m) != EINA_MATRIX_TYPE_IDENTITY))
     {
        for (i = 0; i < ft_outline->n_points; i++)
          {
             eina_matrix3_point_transform(m,
                                          ft_outline->points[i].x/64.0,/* convert back to normal coord.*/
                                          ft_outline->points[i].y/64.0,/* convert back to normal coord.*/
                                          &x, &y);

             ft_outline->points[i].x = TO_FT_COORD(x);
             ft_outline->points[i].y = TO_FT_COORD(y);
          }
     }
}

static Eina_Bool
_generate_outline(const Efl_Gfx_Path_Command *cmds, const double *pts, Outline * outline)
{
   Eina_Bool close_path = EINA_FALSE; 
   for (; *cmds != EFL_GFX_PATH_COMMAND_TYPE_END; cmds++)
     {
        switch (*cmds)
          {
            case EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO:

               _outline_move_to(outline, pts[0], pts[1]);

               pts += 2;
               break;
            case EFL_GFX_PATH_COMMAND_TYPE_LINE_TO:

               _outline_line_to(outline, pts[0], pts[1]);

               pts += 2;
               break;
            case EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO:

               _outline_cubic_to(outline,
                                 pts[0], pts[1], pts[2], pts[3], // control points
                                 pts[4], pts[5]); // destination point
               pts += 6;
               break;

            case EFL_GFX_PATH_COMMAND_TYPE_CLOSE:

               close_path = _outline_close_path(outline);
               break;

            case EFL_GFX_PATH_COMMAND_TYPE_LAST:
            case EFL_GFX_PATH_COMMAND_TYPE_END:
               break;
          }
     }
   _outline_end(outline);
   return close_path;
}

typedef struct _Line
{
   double x1;
   double y1;
   double x2;
   double y2;
}Line;

static void 
_line_value_set(Line *l, double x1, double y1, double x2, double y2)
{
   l->x1 = x1;
   l->y1 = y1;
   l->x2 = x2;
   l->y2 = y2;
}

// approximate sqrt(x*x + y*y) using alpha max plus beta min algorithm.
// With alpha = 1, beta = 3/8, giving results with the largest error less 
// than 7% compared to the exact value.
static double
_line_length(Line *l)
{
   double x = l->x2 - l->x1;
   double y = l->y2 - l->y1;
   x = x < 0 ? -x : x;
   y = y < 0 ? -y : y;
   return (x > y ? x + 0.375 * y : y + 0.375 * x);
}

static void
_line_split_at_length(Line *l, double length, Line *left, Line *right)
{
   double len = _line_length(l);
   double dx = ((l->x2 - l->x1)/len) *length;
   double dy = ((l->y2 - l->y1)/len) *length;

   left->x1 = l->x1;
   left->y1 = l->y1;
   left->x2 = left->x1 + dx;
   left->y2 = left->y1 + dy;

   right->x1 = left->x2;
   right->y1 = left->y2;
   right->x2 = l->x2;
   right->y2 = l->y2;
}

typedef struct _Dash_Stroker
{
   Efl_Gfx_Dash *dash;
   int           dash_len;
   Outline      *outline;
   int cur_dash_index;
   double cur_dash_length;
   Eina_Bool cur_op_gap;
   double start_x, start_y;
   double cur_x, cur_y;
}Dash_Stroker;

static void
_dasher_line_to(Dash_Stroker *dasher, double x, double y)
{
   Line l, left, right;
   double line_len = 0.0;
   _line_value_set(&l, dasher->cur_x, dasher->cur_y, x, y);
   line_len = _line_length(&l);
   if (line_len < dasher->cur_dash_length)
     {
        dasher->cur_dash_length -= line_len;
        if (!dasher->cur_op_gap)
          {
             _outline_move_to(dasher->outline, dasher->cur_x, dasher->cur_y);
             _outline_line_to(dasher->outline, x, y);
          }
     }
   else 
     {
        while (line_len > dasher->cur_dash_length)
          {
             line_len -= dasher->cur_dash_length;
             _line_split_at_length(&l, dasher->cur_dash_length, &left, &right);
             if (!dasher->cur_op_gap)
               {
                  _outline_move_to(dasher->outline, left.x1, left.y1);
                  _outline_line_to(dasher->outline, left.x2, left.y2);
                  dasher->cur_dash_length = dasher->dash[dasher->cur_dash_index].gap;
               }
             else
               {
                  dasher->cur_dash_index = (dasher->cur_dash_index +1) % dasher->dash_len ;
                  dasher->cur_dash_length = dasher->dash[dasher->cur_dash_index].length;
               }
             dasher->cur_op_gap = !dasher->cur_op_gap;
             l = right;
             dasher->cur_x = l.x1;
             dasher->cur_y = l.y1;
          }
        // remainder
        dasher->cur_dash_length -= line_len;
        if (!dasher->cur_op_gap)
          {
             _outline_move_to(dasher->outline, l.x1, l.y1);
             _outline_line_to(dasher->outline, l.x2, l.y2);
          }
        if (dasher->cur_dash_length < 1.0)
          {
             // move to next dash
             if (!dasher->cur_op_gap)
               {
                  dasher->cur_op_gap = EINA_TRUE;
                  dasher->cur_dash_length = dasher->dash[dasher->cur_dash_index].gap;
               }
             else
               {
                  dasher->cur_op_gap = EINA_FALSE;
                  dasher->cur_dash_index = (dasher->cur_dash_index +1) % dasher->dash_len ;
                  dasher->cur_dash_length = dasher->dash[dasher->cur_dash_index].length;
               }
          }
     }
   dasher->cur_x = x;
   dasher->cur_y = y;
}

static void
_dasher_cubic_to(Dash_Stroker *dasher, double cx1 , double cy1, double cx2, double cy2, double x, double y)
{
   Eina_Bezier b, left, right;
   double bez_len = 0.0;
   eina_bezier_values_set(&b, dasher->cur_x, dasher->cur_y, cx1, cy1, cx2, cy2, x, y);
   bez_len = eina_bezier_length_get(&b);
   if (bez_len < dasher->cur_dash_length)
     {
        dasher->cur_dash_length -= bez_len;
        if (!dasher->cur_op_gap)
          {
             _outline_move_to(dasher->outline, dasher->cur_x, dasher->cur_y);
             _outline_cubic_to(dasher->outline, cx1, cy1, cx2, cy2, x, y);
          }
     }
   else 
     {
        while (bez_len > dasher->cur_dash_length)
          {
             bez_len -= dasher->cur_dash_length;
             eina_bezier_split_at_length(&b, dasher->cur_dash_length, &left, &right);
             if (!dasher->cur_op_gap)
               {
                  _outline_move_to(dasher->outline, left.start.x, left.start.y);
                  _outline_cubic_to(dasher->outline, left.ctrl_start.x, left.ctrl_start.y, left.ctrl_end.x, left.ctrl_end.y, left.end.x, left.end.y);
                  dasher->cur_dash_length = dasher->dash[dasher->cur_dash_index].gap;
               }
             else
               {
                  dasher->cur_dash_index = (dasher->cur_dash_index +1) % dasher->dash_len ;
                  dasher->cur_dash_length = dasher->dash[dasher->cur_dash_index].length;
               }
             dasher->cur_op_gap = !dasher->cur_op_gap;
             b = right;
             dasher->cur_x = b.start.x;
             dasher->cur_y = b.start.y;
          }
         // remainder
        dasher->cur_dash_length -= bez_len;
        if (!dasher->cur_op_gap)
          {
             _outline_move_to(dasher->outline, b.start.x, b.start.y);
             _outline_cubic_to(dasher->outline, b.ctrl_start.x, b.ctrl_start.y, b.ctrl_end.x, b.ctrl_end.y, b.end.x, b.end.y);
          }
        if (dasher->cur_dash_length < 1.0)
          {
             // move to next dash
             if (!dasher->cur_op_gap)
               {
                  dasher->cur_op_gap = EINA_TRUE;
                  dasher->cur_dash_length = dasher->dash[dasher->cur_dash_index].gap;
               }
             else
               {
                  dasher->cur_op_gap = EINA_FALSE;
                  dasher->cur_dash_index = (dasher->cur_dash_index +1) % dasher->dash_len ;
                  dasher->cur_dash_length = dasher->dash[dasher->cur_dash_index].length;
               }
          }
      }
   dasher->cur_x = x;
   dasher->cur_y = y;
}

static Eina_Bool
_generate_dashed_outline(const Efl_Gfx_Path_Command *cmds, const double *pts, Outline * outline, Efl_Gfx_Dash *dash, int dash_len)
{
   Dash_Stroker dasher;
   dasher.dash = dash;
   dasher.dash_len = dash_len;
   dasher.outline = outline;
   dasher.cur_dash_length = 0.0;
   dasher.cur_dash_index = 0;
   dasher.cur_op_gap = EINA_FALSE;
   dasher.start_x = 0.0;
   dasher.start_y = 0.0;
   dasher.cur_x = 0.0;
   dasher.cur_y = 0.0;

   for (; *cmds != EFL_GFX_PATH_COMMAND_TYPE_END; cmds++)
     {
        switch (*cmds)
          {
            case EFL_GFX_PATH_COMMAND_TYPE_MOVE_TO:
              {
                 // reset the dash
                 dasher.cur_dash_index = 0;
                 dasher.cur_dash_length = dasher.dash[0].length;
                 dasher.cur_op_gap = EINA_FALSE;
                 dasher.start_x = pts[0];
                 dasher.start_y = pts[1];
                 dasher.cur_x = pts[0];
                 dasher.cur_y = pts[1];
                 pts += 2;
              }
               break;
            case EFL_GFX_PATH_COMMAND_TYPE_LINE_TO:
               _dasher_line_to(&dasher, pts[0], pts[1]);
               pts += 2;
               break;
            case EFL_GFX_PATH_COMMAND_TYPE_CUBIC_TO:
               _dasher_cubic_to(&dasher, pts[0], pts[1], pts[2], pts[3], pts[4], pts[5]);
               pts += 6;
               break;

            case EFL_GFX_PATH_COMMAND_TYPE_CLOSE:
               _dasher_line_to(&dasher, dasher.start_x, dasher.start_y);
               break;

            case EFL_GFX_PATH_COMMAND_TYPE_LAST:
            case EFL_GFX_PATH_COMMAND_TYPE_END:
               break;
          }
     }
   _outline_end(outline);
   return EINA_FALSE;
}

static Eina_Bool
_generate_stroke_data(Ector_Renderer_Software_Shape_Data *pd)
{
   if (pd->outline_data) return EINA_FALSE;

   if (!pd->shape->stroke.fill &&
       ((pd->public_shape->stroke.color.a == 0) ||
        (pd->public_shape->stroke.width < 0.01)))
     return EINA_FALSE;

   return EINA_TRUE;
}

static Eina_Bool
_generate_shape_data(Ector_Renderer_Software_Shape_Data *pd)
{
   if (pd->shape_data) return EINA_FALSE;

   if (!pd->shape->fill && (pd->base->color.a == 0)) return EINA_FALSE;

   return EINA_TRUE;
}

static Ector_Software_Shape_Task *
_need_update_rle(Eo *obj, Ector_Renderer_Software_Shape_Data *pd)
{
   Ector_Software_Shape_Task *r;
   const Efl_Gfx_Path_Command *cmds;
   const double *pts;
   Efl_Gfx_Fill_Rule fill_rule;

   if (!pd->done && pd->task) return pd->task;

   if (!_generate_stroke_data(pd) &&
       !_generate_shape_data(pd))
     return NULL;

   efl_gfx_path_get(obj, &cmds, &pts);
   fill_rule = efl_gfx_shape_fill_rule_get(obj);

   if (!cmds) return NULL;

   r = pd->task;
   if (!r) r = malloc(sizeof (Ector_Software_Shape_Task));
   if (!r) return NULL;

   r->pd = pd;
   r->cmds = cmds;
   r->pts = pts;
   r->fill_rule = fill_rule;

   pd->done = EINA_FALSE;
   pd->task = r;

   return r;
}

static void
_done_rle(void *data)
{
   Ector_Software_Shape_Task *task = data;

   task->pd->done = EINA_TRUE;
}

static void
_update_rle(void *data, Ector_Software_Thread *thread)
{
   Ector_Software_Shape_Task *task = data;
   Eina_Bool close_path;
   Outline *outline, *dash_outline;

   outline = _outline_create();
   close_path = _generate_outline(task->cmds, task->pts, outline);
   if (task->fill_rule == EFL_GFX_FILL_RULE_ODD_EVEN)
     outline->ft_outline.flags = SW_FT_OUTLINE_EVEN_ODD_FILL;
   else
     outline->ft_outline.flags = SW_FT_OUTLINE_NONE; // default is winding fill

   _outline_transform(outline, task->pd->base->m);

   //shape data generation
   if (_generate_shape_data(task->pd))
     task->pd->shape_data = ector_software_rasterizer_generate_rle_data(thread,
                                                                        task->pd->surface->rasterizer,
                                                                        &outline->ft_outline);

   //stroke data generation
   if (_generate_stroke_data(task->pd))
     {
        ector_software_rasterizer_stroke_set(thread, task->pd->surface->rasterizer,
                                             (task->pd->public_shape->stroke.width *
                                              task->pd->public_shape->stroke.scale),
                                             task->pd->public_shape->stroke.cap,
                                             task->pd->public_shape->stroke.join,
                                             task->pd->base->m);

        if (task->pd->public_shape->stroke.dash)
          {
             dash_outline = _outline_create();
             close_path = _generate_dashed_outline(task->cmds, task->pts, dash_outline,
                                                   task->pd->public_shape->stroke.dash,
                                                   task->pd->public_shape->stroke.dash_length);
             _outline_transform(dash_outline, task->pd->base->m);
             task->pd->outline_data = ector_software_rasterizer_generate_stroke_rle_data(thread,
                                                                                         task->pd->surface->rasterizer,
                                                                                         &dash_outline->ft_outline,
                                                                                         close_path);
             _outline_destroy(dash_outline);
          }
        else
          {
             task->pd->outline_data = ector_software_rasterizer_generate_stroke_rle_data(thread,
                                                                                         task->pd->surface->rasterizer,
                                                                                         &outline->ft_outline,
                                                                                         close_path);
          }
     }
   _outline_destroy(outline);
}

static Eina_Bool
_ector_renderer_software_shape_ector_renderer_prepare(Eo *obj,
                                                      Ector_Renderer_Software_Shape_Data *pd)
{
   Ector_Software_Shape_Task *task;

   // FIXME: shouldn't this be moved to the software base object?
   if (!pd->surface)
     pd->surface = efl_data_xref(pd->base->surface, ECTOR_SOFTWARE_SURFACE_CLASS, obj);

   // Asynchronously lazy build of the RLE data for this shape
   task = _need_update_rle(obj, pd);
   if (task) ector_software_schedule(_update_rle, _done_rle, task);

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_software_shape_ector_renderer_draw(Eo *obj,
                                                   Ector_Renderer_Software_Shape_Data *pd,
                                                   Efl_Gfx_Render_Op op, Eina_Array *clips,
                                                   unsigned int mul_col)
{
   Ector_Software_Shape_Task *task;
   int x, y;

   // check if RLE data are ready
   task = _need_update_rle(obj, pd);
   if (task) ector_software_wait(_update_rle, _done_rle, task);

   // adjust the offset
   x = pd->surface->x + (int)pd->base->origin.x;
   y = pd->surface->y + (int)pd->base->origin.y;

   // fill the span_data structure
   ector_software_rasterizer_clip_rect_set(pd->surface->rasterizer, clips);
   ector_software_rasterizer_transform_set(pd->surface->rasterizer, pd->base->m);

   if (pd->shape->fill)
     {
        ector_renderer_software_op_fill(pd->shape->fill);
        ector_software_rasterizer_draw_rle_data(pd->surface->rasterizer,
                                                x, y, mul_col, op,
                                                pd->shape_data);
     }
   else
     {
        if (pd->base->color.a > 0)
          {
             ector_software_rasterizer_color_set(pd->surface->rasterizer,
                                                 pd->base->color.r,
                                                 pd->base->color.g,
                                                 pd->base->color.b,
                                                 pd->base->color.a);
             ector_software_rasterizer_draw_rle_data(pd->surface->rasterizer,
                                                     x, y, mul_col, op,
                                                     pd->shape_data);
          }
     }

   if (pd->shape->stroke.fill)
     {
        ector_renderer_software_op_fill(pd->shape->stroke.fill);
        ector_software_rasterizer_draw_rle_data(pd->surface->rasterizer,
                                                x, y, mul_col, op,
                                                pd->outline_data);
     }
   else
     {
        if (pd->public_shape->stroke.color.a > 0)
          {
             ector_software_rasterizer_color_set(pd->surface->rasterizer,
                                                 pd->public_shape->stroke.color.r,
                                                 pd->public_shape->stroke.color.g,
                                                 pd->public_shape->stroke.color.b,
                                                 pd->public_shape->stroke.color.a);
             ector_software_rasterizer_draw_rle_data(pd->surface->rasterizer,
                                                     x, y, mul_col, op,
                                                     pd->outline_data);
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_software_shape_ector_renderer_software_op_fill(Eo *obj EINA_UNUSED,
                                                                 Ector_Renderer_Software_Shape_Data *pd EINA_UNUSED)
{
   // FIXME: let's find out how to fill a shape with a shape later.
   // I need to read SVG specification and see how to map that with software.
   return EINA_FALSE;
}

static void
_ector_renderer_software_shape_efl_gfx_path_path_set(Eo *obj,
                                                     Ector_Renderer_Software_Shape_Data *pd,
                                                     const Efl_Gfx_Path_Command *op,
                                                     const double *points)
{
   if (pd->shape_data) ector_software_rasterizer_destroy_rle_data(pd->shape_data);
   if (pd->outline_data) ector_software_rasterizer_destroy_rle_data(pd->outline_data);
   pd->shape_data = NULL;
   pd->outline_data = NULL;

   efl_gfx_path_set(efl_super(obj, MY_CLASS), op, points);
}


static void
_ector_renderer_software_shape_path_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Ector_Renderer_Software_Shape_Data *pd = data;
   Efl_Gfx_Path_Change_Event *ev = event->info;

   if (ev && !((ev->what & EFL_GFX_CHANGE_FLAG_MATRIX) ||
               (ev->what & EFL_GFX_CHANGE_FLAG_PATH)))
     return;

   if (pd->shape_data) ector_software_rasterizer_destroy_rle_data(pd->shape_data);
   if (pd->outline_data) ector_software_rasterizer_destroy_rle_data(pd->outline_data);

   pd->shape_data = NULL;
   pd->outline_data = NULL;
}

static Eo *
_ector_renderer_software_shape_efl_object_constructor(Eo *obj, Ector_Renderer_Software_Shape_Data *pd)
{
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   if (!obj) return NULL;

   pd->task = NULL;
   pd->done = EINA_FALSE;
   pd->public_shape = efl_data_xref(obj, EFL_GFX_SHAPE_MIXIN, obj);
   pd->shape = efl_data_xref(obj, ECTOR_RENDERER_SHAPE_MIXIN, obj);
   pd->base = efl_data_xref(obj, ECTOR_RENDERER_CLASS, obj);
   efl_event_callback_add(obj, EFL_GFX_PATH_EVENT_CHANGED, _ector_renderer_software_shape_path_changed, pd);

   return obj;
}

static void
_ector_renderer_software_shape_efl_object_destructor(Eo *obj, Ector_Renderer_Software_Shape_Data *pd)
{
   // FIXME: As base class, destructor can't call destructor of mixin class.
   // Call explicit API to free shape data.
   if (!pd->done && pd->task)
     ector_software_wait(_update_rle, _done_rle, pd->task);

   efl_gfx_path_reset(obj);

   if (pd->shape_data) ector_software_rasterizer_destroy_rle_data(pd->shape_data);
   if (pd->outline_data) ector_software_rasterizer_destroy_rle_data(pd->outline_data);
   free(pd->task);

   efl_data_xunref(pd->base->surface, pd->surface, obj);
   efl_data_xunref(obj, pd->base, obj);
   efl_data_xunref(obj, pd->shape, obj);
   efl_data_xunref(obj, pd->public_shape, obj);

   efl_destructor(efl_super(obj, MY_CLASS));
}

unsigned int
_ector_renderer_software_shape_ector_renderer_crc_get(const Eo *obj,
                                                      Ector_Renderer_Software_Shape_Data *pd)
{
   unsigned int crc;

   crc = ector_renderer_crc_get(efl_super(obj, MY_CLASS));

   crc = eina_crc((void*) &pd->shape->stroke.marker,
                  sizeof (pd->shape->stroke.marker),
                  crc, EINA_FALSE);
   crc = eina_crc((void*) &pd->public_shape->stroke.scale,
                  sizeof (pd->public_shape->stroke.scale) * 3,
                  crc, EINA_FALSE); // scale, width, centered
   crc = eina_crc((void*) &pd->public_shape->stroke.color,
                  sizeof (pd->public_shape->stroke.color),
                  crc, EINA_FALSE);
   crc = eina_crc((void*) &pd->public_shape->stroke.cap,
                  sizeof (pd->public_shape->stroke.cap),
                  crc, EINA_FALSE);
   crc = eina_crc((void*) &pd->public_shape->stroke.join,
                  sizeof (pd->public_shape->stroke.join),
                  crc, EINA_FALSE);

   if (pd->shape->fill)
     crc = _renderer_crc_get(pd->shape->fill, crc);
   if (pd->shape->stroke.fill)
     crc = _renderer_crc_get(pd->shape->stroke.fill, crc);
   if (pd->shape->stroke.marker)
     crc = _renderer_crc_get(pd->shape->stroke.marker, crc);
   if (pd->public_shape->stroke.dash_length)
     {
        crc = eina_crc((void*) pd->public_shape->stroke.dash,
                       sizeof (Efl_Gfx_Dash) * pd->public_shape->stroke.dash_length,
                       crc, EINA_FALSE);
     }

   return crc;
}

#include "ector_renderer_software_shape.eo.c"
