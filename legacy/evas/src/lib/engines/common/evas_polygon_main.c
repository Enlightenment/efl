#include "evas_common.h"

#include <math.h>

typedef struct _RGBA_Span RGBA_Span;
typedef struct _RGBA_Edge RGBA_Edge;
typedef struct _RGBA_Vertex RGBA_Vertex;

struct _RGBA_Span
{
   Evas_Object_List _list_data;
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

void
evas_common_polygon_init(void)
{
}

RGBA_Polygon_Point *
evas_common_polygon_point_add(RGBA_Polygon_Point *points, int x, int y)
{
   RGBA_Polygon_Point *pt;
   
   pt = malloc(sizeof(RGBA_Polygon_Point));
   if (!pt) return points;
   pt->x = x;
   pt->y = y;
   points = evas_object_list_append(points, pt);
   return points;
}

RGBA_Polygon_Point *
evas_common_polygon_points_clear(RGBA_Polygon_Point *points)
{
   if (points)
     {
	while (points) 
	  {
	     RGBA_Polygon_Point *old_p;
	     
	     old_p = points;
	     points = evas_object_list_remove(points, points);
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

void
evas_common_polygon_draw(RGBA_Image *dst, RGBA_Draw_Context *dc, RGBA_Polygon_Point *points)
{
   Gfx_Func_Blend_Color_Dst func;
   RGBA_Polygon_Point *pt;
   RGBA_Vertex       *point;
   RGBA_Edge         *edges;
   Evas_Object_List  *spans, *l;
   int                num_active_edges;
   int                n;
   int                i, j, k;
   int                y0, y1, y;
   int                ext_x, ext_y, ext_w, ext_h;
   int               *sorted_index;
   
   ext_x = 0;
   ext_y = 0;
   ext_w = dst->image->w;
   ext_h = dst->image->h;
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

   n = 0; for (l = (Evas_Object_List *)points; l; l = l->next) n++;
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
   for (l = (Evas_Object_List *)points; l; l = l->next)
     {
	pt = (RGBA_Polygon_Point *)l;
	point[k].x = pt->x;
	point[k].y = pt->y;
	point[k].i = k;
	k++;
     }
   qsort(point, n, sizeof(RGBA_Vertex), polygon_point_sorter);
   for (k = 0; k < n; k++) sorted_index[k] = point[k].i;
   k = 0;
   for (l = (Evas_Object_List *)points; l; l = l->next)
     {
	pt = (RGBA_Polygon_Point *)l;
	point[k].x = pt->x;
	point[k].y = pt->y;
	point[k].i = k;
	k++;
     }   
   
   y0 = MAX(ext_y, ceil(point[sorted_index[0]].y - 0.5));
   y1 = MIN(ext_y + ext_h - 1, floor(point[sorted_index[n - 1]].y - 0.5));
   
   k = 0;
   num_active_edges = 0;
   spans = NULL;

   for (y = y0; y <= y1; y++)
     {
	for (; (k < n) && (point[sorted_index[k]].y <= ((double)y + 0.5)); k++)
	  {
	     i = sorted_index[k];
	     
	     if (i > 0) j = i - 1;
	     else j = n - 1;
	     if (point[j].y <= ((double)y - 0.5))
	       {
		  POLY_EDGE_DEL(j)
	       }
	     else if (point[j].y > ((double)y + 0.5))
	       {
		  POLY_EDGE_ADD(j, y)
	       }
	     if (i < (n - 1)) j = i + 1;
	     else j = 0;
	     if (point[j].y <= ((double)y - 0.5))
	       {
		  POLY_EDGE_DEL(i)
	       }
	     else if (point[j].y > ((double)y + 0.5))
	       {
		  POLY_EDGE_ADD(i, y)
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
		  spans = evas_object_list_append(spans, span);
		  span->y = y;
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

   func = evas_common_draw_func_blend_color_get(dc->col.col, dst, 0);   
   if (spans)
     {
	for (l = spans; l; l = l->next)
	  {
	     RGBA_Span *span;
	     DATA32 *ptr;
	     
	     span = (RGBA_Span *)l;
	     ptr = dst->image->data + (span->y * (dst->image->w)) + span->x;
	     func(dc->col.col, ptr, span->w);
	  }
	while (spans)
	  {
	     RGBA_Span *span;
	     
	     span = (RGBA_Span *)spans;
	     spans = evas_object_list_remove(spans, spans);
	     free(span);
	  }
     }
}
