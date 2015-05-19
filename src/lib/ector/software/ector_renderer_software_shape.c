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


typedef struct _Ector_Renderer_Software_Shape_Data Ector_Renderer_Software_Shape_Data;
struct _Ector_Renderer_Software_Shape_Data
{
   Ector_Software_Surface_Data         *surface;
   Ector_Renderer_Generic_Shape_Data   *shape;
   Ector_Renderer_Generic_Base_Data    *base;
   Shape_Rle_Data                      *shape_data;
   Shape_Rle_Data                      *outline_data;
};

typedef struct _Outline
{
   SW_FT_Outline ft_outline;
   int points_alloc;
   int contours_alloc;
}Outline;

static Outline *
_outline_create()
{
   Outline *outline = (Outline *) calloc(1, sizeof(Outline));

   outline->ft_outline.points = (SW_FT_Vector *) calloc(50, sizeof(SW_FT_Vector));
   outline->ft_outline.tags = (char *) calloc(50, sizeof(char));

   outline->ft_outline.contours = (short *) calloc(5, sizeof(short));

   outline->points_alloc = 50;
   outline->contours_alloc = 5;
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
        outline = NULL;
     }
}

static void
_outline_move_to(Outline *outline, double x, double y)
{
   SW_FT_Outline *ft_outline = &outline->ft_outline;

   if (ft_outline->n_contours == outline->contours_alloc)
     {
        outline->contours_alloc += 5;
        ft_outline->contours = (short *) realloc(ft_outline->contours, outline->contours_alloc * sizeof(short));
     }
   ft_outline->points[ft_outline->n_points].x = x;
   ft_outline->points[ft_outline->n_points].y = y;
   ft_outline->tags[ft_outline->n_points] = SW_FT_CURVE_TAG_ON;

   if (ft_outline->n_points)
     {
        ft_outline->contours[ft_outline->n_contours] = ft_outline->n_points - 1;
        ft_outline->n_contours++;
     }

   ft_outline->n_points++;
}

static void
_outline_end(Outline *outline)
{
   SW_FT_Outline *ft_outline = &outline->ft_outline;

   if (ft_outline->n_contours == outline->contours_alloc)
     {
        outline->contours_alloc += 1;
        ft_outline->contours = (short *) realloc(ft_outline->contours, outline->contours_alloc * sizeof(short));
     }

   if (ft_outline->n_points)
     {
        ft_outline->contours[ft_outline->n_contours] = ft_outline->n_points - 1;
        ft_outline->n_contours++;
     }
}


static void  _outline_line_to(Outline *outline, double x, double y)
{
   SW_FT_Outline *ft_outline = &outline->ft_outline;

   if (ft_outline->n_points == outline->points_alloc)
     {
        outline->points_alloc += 50;
        ft_outline->points = (SW_FT_Vector *) realloc(ft_outline->points, outline->points_alloc * sizeof(SW_FT_Vector));
        ft_outline->tags = (char *) realloc(ft_outline->tags, outline->points_alloc * sizeof(char));
     }
   ft_outline->points[ft_outline->n_points].x = x;
   ft_outline->points[ft_outline->n_points].y = y;
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
        // first path
        index = 0;
     }

   // make sure there is atleast one point in the current path
   if (ft_outline->n_points == index) return EINA_FALSE;

   _outline_line_to(outline, ft_outline->points[index].x, ft_outline->points[index].y);
   return EINA_TRUE;
}


static void  _outline_cubic_to(Outline *outline, double cx1, double cy1, double cx2, double cy2, double x, double y)
{
   SW_FT_Outline *ft_outline = &outline->ft_outline;

   if (ft_outline->n_points == outline->points_alloc)
     {
        outline->points_alloc += 50;
        ft_outline->points = (SW_FT_Vector *) realloc(ft_outline->points, outline->points_alloc * sizeof(SW_FT_Vector));
        ft_outline->tags = (char *) realloc(ft_outline->tags, outline->points_alloc * sizeof(char));
     }

   ft_outline->points[ft_outline->n_points].x = cx1;
   ft_outline->points[ft_outline->n_points].y = cy1;
   ft_outline->tags[ft_outline->n_points] = SW_FT_CURVE_TAG_CUBIC;
   ft_outline->n_points++;

   ft_outline->points[ft_outline->n_points].x = cx2;
   ft_outline->points[ft_outline->n_points].y = cy2;
   ft_outline->tags[ft_outline->n_points] = SW_FT_CURVE_TAG_CUBIC;
   ft_outline->n_points++;

   ft_outline->points[ft_outline->n_points].x = x;
   ft_outline->points[ft_outline->n_points].y = y;
   ft_outline->tags[ft_outline->n_points] = SW_FT_CURVE_TAG_ON;
   ft_outline->n_points++;
}

static void _outline_transform(Outline *outline, Eina_Matrix3 *m)
{
   int i;
   SW_FT_Outline *ft_outline = &outline->ft_outline;

   if (m)
     {
        double x, y;
        for (i = 0; i < ft_outline->n_points; i++)
          {
             eina_matrix3_point_transform(m, ft_outline->points[i].x, ft_outline->points[i].y, &x, &y);
             ft_outline->points[i].x = (int)(x * 64);// to freetype 26.6 coordinate.
             ft_outline->points[i].y = (int)(y * 64);
          }
     }
   else
     {
        for (i = 0; i < ft_outline->n_points; i++)
          {
             ft_outline->points[i].x = ft_outline->points[i].x <<6;// to freetype 26.6 coordinate.
             ft_outline->points[i].y = ft_outline->points[i].y <<6;
          }
     }
}


static Eina_Bool
_ector_renderer_software_shape_ector_renderer_generic_base_prepare(Eo *obj, Ector_Renderer_Software_Shape_Data *pd)
{
   const Efl_Gfx_Path_Command *cmds = NULL;
   const double *pts = NULL;

   // FIXME: shouldn't that be part of the shape generic implementation ?
   if (pd->shape->fill)
     eo_do(pd->shape->fill, ector_renderer_prepare());
   if (pd->shape->stroke.fill)
     eo_do(pd->shape->stroke.fill, ector_renderer_prepare());
   if (pd->shape->stroke.marker)
     eo_do(pd->shape->stroke.marker, ector_renderer_prepare());

   // shouldn't that be moved to the software base object
   if (!pd->surface)
     {
        Eo *parent;
        eo_do(obj, parent = eo_parent_get());
        if (!parent) return EINA_FALSE;
        pd->surface = eo_data_xref(parent, ECTOR_SOFTWARE_SURFACE_CLASS, obj);
        if (!pd->surface) return EINA_FALSE;
     }

   eo_do(obj, efl_gfx_shape_path_get(&cmds, &pts));
   if (!pd->shape_data && cmds)
     {
        Eina_Bool close_path = EINA_FALSE;
        Outline * outline = _outline_create();

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

                   // Be careful, we do have a different order than
                   // cairo, first is destination point, followed by
                   // the control point. The opposite of cairo.
                   _outline_cubic_to(outline,
                                     pts[2], pts[3], pts[4], pts[5], // control points
                                     pts[0], pts[1]); // destination point
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
        _outline_transform(outline, pd->base->m);

        // generate the shape data.
        pd->shape_data = ector_software_rasterizer_generate_rle_data(pd->surface->software, &outline->ft_outline);
        if (!pd->outline_data)
          {
             ector_software_rasterizer_stroke_set(pd->surface->software, (pd->shape->stroke.width * pd->shape->stroke.scale), pd->shape->stroke.cap,
                                                  pd->shape->stroke.join);
             pd->outline_data = ector_software_rasterizer_generate_stroke_rle_data(pd->surface->software, &outline->ft_outline, close_path);
          }

        _outline_destroy(outline);
     }

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_software_shape_ector_renderer_generic_base_draw(Eo *obj EINA_UNUSED, Ector_Renderer_Software_Shape_Data *pd, Ector_Rop op, Eina_Array *clips, unsigned int mul_col)
{
   int x, y;

   // adjust the offset
   x = pd->surface->x + (int)pd->base->origin.x;
   y = pd->surface->y + (int)pd->base->origin.y;

   // fill the span_data structure
   ector_software_rasterizer_clip_rect_set(pd->surface->software, clips);
   ector_software_rasterizer_transform_set(pd->surface->software, pd->base->m);

   if (pd->shape->fill)
     {
        eo_do(pd->shape->fill, ector_renderer_software_base_fill());
        ector_software_rasterizer_draw_rle_data(pd->surface->software, x, y, mul_col, op, pd->shape_data);
     }
   else
     {
        if (pd->base->color.a > 0)
          {
             ector_software_rasterizer_color_set(pd->surface->software, pd->base->color.r, pd->base->color.g, pd->base->color.b, pd->base->color.a);
             ector_software_rasterizer_draw_rle_data(pd->surface->software, x, y, mul_col, op, pd->shape_data);
          }
     }

   if (pd->shape->stroke.fill)
     {
        eo_do(pd->shape->stroke.fill, ector_renderer_software_base_fill());
        ector_software_rasterizer_draw_rle_data(pd->surface->software, x, y, mul_col, op, pd->outline_data);
     }
   else
     {
        if (pd->shape->stroke.color.a > 0)
          {
             ector_software_rasterizer_color_set(pd->surface->software,
                                                 pd->shape->stroke.color.r, pd->shape->stroke.color.g,
                                                 pd->shape->stroke.color.b, pd->shape->stroke.color.a);
             ector_software_rasterizer_draw_rle_data(pd->surface->software, x, y, mul_col, op, pd->outline_data);
          }
     }

   return EINA_TRUE;
}

static Eina_Bool
_ector_renderer_software_shape_ector_renderer_software_base_fill(Eo *obj EINA_UNUSED, Ector_Renderer_Software_Shape_Data *pd EINA_UNUSED)
{
   // FIXME: let's find out how to fill a shape with a shape later.
   // I need to read SVG specification and see how to map that with software.
   return EINA_FALSE;
}

static void
_ector_renderer_software_shape_efl_gfx_shape_path_set(Eo *obj, Ector_Renderer_Software_Shape_Data *pd,
                                                      const Efl_Gfx_Path_Command *op, const double *points)
{
   if (pd->shape_data) ector_software_rasterizer_destroy_rle_data(pd->shape_data);
   if (pd->outline_data) ector_software_rasterizer_destroy_rle_data(pd->outline_data);
   pd->shape_data = NULL;
   pd->outline_data = NULL;

   eo_do_super(obj, ECTOR_RENDERER_SOFTWARE_SHAPE_CLASS, efl_gfx_shape_path_set(op, points));
}


static Eina_Bool
_ector_renderer_software_shape_path_changed(void *data, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED,
                                            void *event_info EINA_UNUSED)
{
   Ector_Renderer_Software_Shape_Data *pd = data;
   
   if (pd->shape_data) ector_software_rasterizer_destroy_rle_data(pd->shape_data);
   if (pd->outline_data) ector_software_rasterizer_destroy_rle_data(pd->outline_data);
   
   pd->shape_data = NULL;
   pd->outline_data = NULL;

   return EINA_TRUE;
}

Eo *
_ector_renderer_software_shape_eo_base_constructor(Eo *obj, Ector_Renderer_Software_Shape_Data *pd)
{
   obj = eo_do_super_ret(obj, ECTOR_RENDERER_SOFTWARE_SHAPE_CLASS, obj, eo_constructor());
   pd->shape = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_SHAPE_MIXIN, obj);
   pd->base = eo_data_xref(obj, ECTOR_RENDERER_GENERIC_BASE_CLASS, obj);
   eo_do(obj,
         eo_event_callback_add(EFL_GFX_PATH_CHANGED, _ector_renderer_software_shape_path_changed, pd));

   return obj;
}

void
_ector_renderer_software_shape_eo_base_destructor(Eo *obj, Ector_Renderer_Software_Shape_Data *pd)
{
   Eo *parent;

   if (pd->shape_data) ector_software_rasterizer_destroy_rle_data(pd->shape_data);
   if (pd->outline_data) ector_software_rasterizer_destroy_rle_data(pd->outline_data);

   eo_do(obj, parent = eo_parent_get());
   eo_data_xunref(parent, pd->surface, obj);

   eo_data_xunref(obj, pd->shape, obj);
   eo_data_xunref(obj, pd->base, obj);
   eo_do_super(obj, ECTOR_RENDERER_SOFTWARE_SHAPE_CLASS, eo_destructor());
}


#include "ector_renderer_software_shape.eo.c"
