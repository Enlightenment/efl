#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <math.h>

#include "evas_common_private.h"
#include "evas_blend_private.h"

typedef struct _RGBA_Span RGBA_Span;
typedef struct _RGBA_Edge RGBA_Edge;
typedef struct _RGBA_Vertex RGBA_Vertex;

struct _RGBA_Span
{
   EINA_INLIST;
   int x, y, w;
};

struct _RGBA_Edge
{
   double x, dx;
   int i;
};

struct _RGBA_Vertex
{
   double x, y;
   int i;
};

#define POLY_EDGE_DEL(_i)                                               \
{                                                                       \
   int _j;                                                              \
                                                                        \
   for (_j = 0; (_j < num_active_edges) && (edges[_j].i != _i); _j++);  \
   if (_j < num_active_edges)                                           \
     {                                                                  \
	num_active_edges--;                                             \
	memmove(&(edges[_j]), &(edges[_j + 1]),                         \
	        (num_active_edges - _j) * sizeof(RGBA_Edge));           \
     }                                                                  \
}

#define POLY_EDGE_ADD(_i, _y)                                           \
{                                                                       \
   int _j;                                                              \
   float _dx;                                                           \
   RGBA_Vertex *_p, *_q;                                                \
   if (_i < (n - 1)) _j = _i + 1;                                       \
   else _j = 0;                                                         \
   if (point[_i].y < point[_j].y)                                       \
     {                                                                  \
	_p = &(point[_i]);                                              \
	_q = &(point[_j]);                                              \
     }                                                                  \
   else                                                                 \
     {                                                                  \
	_p = &(point[_j]);                                              \
	_q = &(point[_i]);                                              \
     }                                                                  \
   edges[num_active_edges].dx = _dx = (_q->x - _p->x) / (_q->y - _p->y); \
   edges[num_active_edges].x = (_dx * ((float)_y + 0.5 - _p->y)) + _p->x; \
   edges[num_active_edges].i = _i;                                      \
   num_active_edges++;                                                  \
}

EAPI void
evas_common_polygon_init(void)
{
}

EAPI RGBA_Polygon_Point *
evas_common_polygon_point_add(RGBA_Polygon_Point *points, int x, int y)
{
   RGBA_Polygon_Point *pt;

   pt = malloc(sizeof(RGBA_Polygon_Point));
   if (!pt) return points;
   pt->x = x;
   pt->y = y;
   points = (RGBA_Polygon_Point *)eina_inlist_append(EINA_INLIST_GET(points), EINA_INLIST_GET(pt));
   return points;
}

EAPI RGBA_Polygon_Point *
evas_common_polygon_points_clear(RGBA_Polygon_Point *points)
{
   if (points)
     {
	while (points)
	  {
	     RGBA_Polygon_Point *old_p;

	     old_p = points;
	     points = (RGBA_Polygon_Point *)eina_inlist_remove(EINA_INLIST_GET(points), EINA_INLIST_GET(points));
	     free(old_p);
	  }
     }
   return NULL;
}

static int
polygon_point_sorter(const void *a, const void *b)
{
   RGBA_Vertex *p, *q;

   p = (RGBA_Vertex *)a;
   q = (RGBA_Vertex *)b;
   if (p->y <= q->y) return -1;
   return 1;
}

static int
polygon_edge_sorter(const void *a, const void *b)
{
   RGBA_Edge *p, *q;

   p = (RGBA_Edge *)a;
   q = (RGBA_Edge *)b;
   if (p->x <= q->x) return -1;
   return 1;
}

EAPI void
evas_common_polygon_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points, int x, int y)
{
   RGBA_Gfx_Func      func;
   RGBA_Polygon_Point *pt;
   RGBA_Vertex       *point;
   RGBA_Edge         *edges;
   Eina_Inlist  *spans;
   int                num_active_edges;
   int                n;
   int                i, j, k;
   int                yy0, yy1, yi;
   int                ext_x, ext_y, ext_w, ext_h;
   int               *sorted_index;

   if (!dst->image.data) return;
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_POLY
   pixman_op_t op = PIXMAN_OP_SRC; // _EVAS_RENDER_COPY
   if (dc->render_op == _EVAS_RENDER_BLEND)
     op = PIXMAN_OP_OVER;
# endif
#endif

   ext_x = 0;
   ext_y = 0;
   ext_w = dst->cache_entry.w;
   ext_h = dst->cache_entry.h;
   if (dc->clip.use)
     {
	if (dc->clip.x > ext_x)
	  {
	     ext_w += ext_x - dc->clip.x;
	     ext_x = dc->clip.x;
	  }
	if ((ext_x + ext_w) > (dc->clip.x + dc->clip.w))
	  {
	     ext_w = (dc->clip.x + dc->clip.w) - ext_x;
	  }
	if (dc->clip.y > ext_y)
	  {
	     ext_h += ext_y - dc->clip.y;
	     ext_y = dc->clip.y;
	  }
	if ((ext_y + ext_h) > (dc->clip.y + dc->clip.h))
	  {
	     ext_h = (dc->clip.y + dc->clip.h) - ext_y;
	  }
     }
   if ((ext_w <= 0) || (ext_h <= 0)) return;

   evas_common_cpu_end_opt();

   n = 0; EINA_INLIST_FOREACH(points, pt) n++;
   if (n < 3) return;
   edges = malloc(sizeof(RGBA_Edge) * n);
   if (!edges) return;
   point = malloc(sizeof(RGBA_Vertex) * n);
   if (!point)
     {
	free(edges);
	return;
     }
   sorted_index = malloc(sizeof(int) * n);
   if (!sorted_index)
     {
	free(edges);
	free(point);
	return;
     }

   k = 0;
   EINA_INLIST_FOREACH(points, pt)
     {
	point[k].x = pt->x + x;
	point[k].y = pt->y + y;
	point[k].i = k;
	k++;
     }
   qsort(point, n, sizeof(RGBA_Vertex), polygon_point_sorter);
   for (k = 0; k < n; k++) sorted_index[k] = point[k].i;
   k = 0;
   EINA_INLIST_FOREACH(points, pt)
     {
	point[k].x = pt->x + x;
	point[k].y = pt->y + y;
	point[k].i = k;
	k++;
     }

   yy0 = MAX(ext_y, ceil(point[sorted_index[0]].y - 0.5));
   yy1 = MIN(ext_y + ext_h - 1, floor(point[sorted_index[n - 1]].y - 0.5));

   k = 0;
   num_active_edges = 0;
   spans = NULL;

   for (yi = yy0; yi <= yy1; yi++)
     {
	for (; (k < n) && (point[sorted_index[k]].y <= ((double)yi + 0.5)); k++)
	  {
	     i = sorted_index[k];

	     if (i > 0) j = i - 1;
	     else j = n - 1;
	     if (point[j].y <= ((double)yi - 0.5))
	       {
		  POLY_EDGE_DEL(j)
	       }
	     else if (point[j].y > ((double)yi + 0.5))
	       {
		  POLY_EDGE_ADD(j, yi)
	       }
	     if (i < (n - 1)) j = i + 1;
	     else j = 0;
	     if (point[j].y <= ((double)yi - 0.5))
	       {
		  POLY_EDGE_DEL(i)
	       }
	     else if (point[j].y > ((double)yi + 0.5))
	       {
		  POLY_EDGE_ADD(i, yi)
	       }
	  }

	qsort(edges, num_active_edges, sizeof(RGBA_Edge), polygon_edge_sorter);

	for (j = 0; j < num_active_edges; j += 2)
	  {
	     int x0, x1;

	     x0 = ceil(edges[j].x - 0.5);
	     if (j < (num_active_edges - 1))
	       x1 = floor(edges[j + 1].x - 0.5);
	     else
	       x1 = x0;
	     if ((x1 >= ext_x) && (x0 < (ext_x + ext_w)) && (x0 <= x1))
	       {
		  RGBA_Span *span;

		  if (x0 < ext_x) x0 = ext_x;
		  if (x1 >= (ext_x + ext_w)) x1 = ext_x + ext_w - 1;
		  span = malloc(sizeof(RGBA_Span));
		  spans = eina_inlist_append(spans, EINA_INLIST_GET(span));
		  span->y = yi;
		  span->x = x0;
		  span->w = (x1 - x0) + 1;
	       }
	     edges[j].x += edges[j].dx;
	     edges[j + 1].x += edges[j + 1].dx;
	  }
     }

   free(edges);
   free(point);
   free(sorted_index);

   if(dc->clip.mask)
     func = evas_common_gfx_func_composite_mask_color_span_get(dc->col.col, dst->cache_entry.flags.alpha, 1, dc->render_op);
   else
     func = evas_common_gfx_func_composite_color_span_get(dc->col.col, dst->cache_entry.flags.alpha, 1, dc->render_op);
   if (spans)
     {
	RGBA_Span *span;

	EINA_INLIST_FOREACH(spans, span)
	  {
	     DATA32 *ptr;
             DATA8 *mask;
             RGBA_Image *mask_ie;

#ifdef HAVE_PIXMAN
# ifdef PIXMAN_POLY
	     if ((dst->pixman.im) && (dc->col.pixman_color_image))
	       pixman_image_composite(op, dc->col.pixman_color_image,
				      NULL, dst->pixman.im,
				      span->x, span->y, 0, 0,
				      span->x, span->y, span->w, 1);
	     else
# endif
#endif
	       {
		 ptr = dst->image.data + (span->y * (dst->cache_entry.w)) + span->x;
                  if (dc->clip.mask)
                    {
                       mask_ie = dc->clip.mask;
                       mask = mask_ie->image.data8
                          + ((span->y - dc->clip.mask_y) * mask_ie->cache_entry.w)
                          + (span->x - dc->clip.mask_x);
                       func(NULL, mask, dc->col.col, ptr, span->w);
                    }
                  else
                    func(NULL, NULL, dc->col.col, ptr, span->w);
	       }
          }
	while (spans)
	  {
	     span = (RGBA_Span *)spans;
	     spans = eina_inlist_remove(spans, spans);
	     free(span);
	  }
     }
}

EAPI void
evas_common_polygon_rgba_draw(RGBA_Image *dst, int ext_x, int ext_y, int ext_w, int ext_h, DATA32 col, int render_op, RGBA_Polygon_Point *points, int x, int y, RGBA_Image *mask_ie, int mask_x, int mask_y)
{
   RGBA_Gfx_Func      func;
   RGBA_Polygon_Point *pt;
   RGBA_Vertex       *point;
   RGBA_Edge         *edges;
   Eina_Inlist  *spans;
   int                num_active_edges;
   int                n;
   int                i, j, k;
   int                yy0, yy1, yi;
   int               *sorted_index;

   if (!dst->image.data) return;
   if ((ext_w <= 0) || (ext_h <= 0)) return;

   evas_common_cpu_end_opt();

   n = 0; EINA_INLIST_FOREACH(points, pt) n++;
   if (n < 3) return;
   edges = malloc(sizeof(RGBA_Edge) * n);
   if (!edges) return;
   point = malloc(sizeof(RGBA_Vertex) * n);
   if (!point)
     {
	free(edges);
	return;
     }
   sorted_index = malloc(sizeof(int) * n);
   if (!sorted_index)
     {
	free(edges);
	free(point);
	return;
     }

   k = 0;
   EINA_INLIST_FOREACH(points, pt)
     {
	point[k].x = pt->x + x;
	point[k].y = pt->y + y;
	point[k].i = k;
	k++;
     }
   qsort(point, n, sizeof(RGBA_Vertex), polygon_point_sorter);
   for (k = 0; k < n; k++) sorted_index[k] = point[k].i;
   k = 0;
   EINA_INLIST_FOREACH(points, pt)
     {
	point[k].x = pt->x + x;
	point[k].y = pt->y + y;
	point[k].i = k;
	k++;
     }

   yy0 = MAX(ext_y, ceil(point[sorted_index[0]].y - 0.5));
   yy1 = MIN(ext_y + ext_h - 1, floor(point[sorted_index[n - 1]].y - 0.5));

   k = 0;
   num_active_edges = 0;
   spans = NULL;

   for (yi = yy0; yi <= yy1; yi++)
     {
	for (; (k < n) && (point[sorted_index[k]].y <= ((double)yi + 0.5)); k++)
	  {
	     i = sorted_index[k];

	     if (i > 0) j = i - 1;
	     else j = n - 1;
	     if (point[j].y <= ((double)yi - 0.5))
	       {
		  POLY_EDGE_DEL(j)
	       }
	     else if (point[j].y > ((double)yi + 0.5))
	       {
		  POLY_EDGE_ADD(j, yi)
	       }
	     if (i < (n - 1)) j = i + 1;
	     else j = 0;
	     if (point[j].y <= ((double)yi - 0.5))
	       {
		  POLY_EDGE_DEL(i)
	       }
	     else if (point[j].y > ((double)yi + 0.5))
	       {
		  POLY_EDGE_ADD(i, yi)
	       }
	  }

	qsort(edges, num_active_edges, sizeof(RGBA_Edge), polygon_edge_sorter);

	for (j = 0; j < num_active_edges; j += 2)
	  {
	     int x0, x1;

	     x0 = ceil(edges[j].x - 0.5);
	     if (j < (num_active_edges - 1))
	       x1 = floor(edges[j + 1].x - 0.5);
	     else
	       x1 = x0;
	     if ((x1 >= ext_x) && (x0 < (ext_x + ext_w)) && (x0 <= x1))
	       {
		  RGBA_Span *span;

		  if (x0 < ext_x) x0 = ext_x;
		  if (x1 >= (ext_x + ext_w)) x1 = ext_x + ext_w - 1;
		  span = malloc(sizeof(RGBA_Span));
		  spans = eina_inlist_append(spans, EINA_INLIST_GET(span));
		  span->y = yi;
		  span->x = x0;
		  span->w = (x1 - x0) + 1;
	       }
	     edges[j].x += edges[j].dx;
	     edges[j + 1].x += edges[j + 1].dx;
	  }
     }

   free(edges);
   free(point);
   free(sorted_index);

   if (mask_ie)
     func = evas_common_gfx_func_composite_mask_color_span_get(col, dst->cache_entry.flags.alpha, 1, render_op);
   else
     func = evas_common_gfx_func_composite_color_span_get(col, dst->cache_entry.flags.alpha, 1, render_op);
   if (spans)
     {
	RGBA_Span *span;
        DATA8 *mask;

	EINA_INLIST_FOREACH(spans, span)
	  {
	     DATA32 *ptr;

             ptr = dst->image.data + (span->y * (dst->cache_entry.w)) + span->x;
             if (mask_ie)
               {
                  mask = mask_ie->image.data8
                     + ((span->y - mask_y) * mask_ie->cache_entry.w)
                     + (span->x - mask_x);
                  func(NULL, mask, col, ptr, span->w);
               }
             else
               func(NULL, NULL, col, ptr, span->w);
          }
	while (spans)
	  {
	     span = (RGBA_Span *)spans;
	     spans = eina_inlist_remove(spans, spans);
	     free(span);
	  }
     }
}
