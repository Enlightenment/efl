#include "evas_gl_private.h"

// FIXME: this is a verbatim copy of the software poly renderer. it just
// use gl to draw 1 pixel high spans like software does. this is to make
// sure rendering correctness matches the software engine but also to save
// time in coming up with a good triangulation algorithm. if you want to
// feel free to turn this into a real triangulation system and use gl to its
// fullest, but as such polygons are used so little, it's not worth it.

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

Evas_GL_Polygon *
evas_gl_common_poly_point_add(Evas_GL_Polygon *poly, int x, int y)
{
   Evas_GL_Polygon_Point *pt;

   if (!poly) poly = calloc(1, sizeof(Evas_GL_Polygon));
   if (!poly) return NULL;
   pt = calloc(1, sizeof(Evas_GL_Polygon_Point));
   if (!pt) return NULL;
   pt->x = x;
   pt->y = y;
   poly->points = eina_list_append(poly->points, pt);
   poly->changed = 1;
   return poly;
}

Evas_GL_Polygon *
evas_gl_common_poly_points_clear(Evas_GL_Polygon *poly)
{
   if (!poly) return NULL;
   while (poly->points)
     {
	Evas_GL_Polygon_Point *pt;

	pt = poly->points->data;
	poly->points = eina_list_remove(poly->points, pt);
	free(pt);
     }
   free(poly);
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
evas_gl_common_poly_draw(Evas_GL_Context *gc, Evas_GL_Polygon *poly)
{
   Cutout_Rects *rects;
   Cutout_Rect  *r;
   int c, cx, cy, cw, ch, cr, cg, cb, ca, i;
   int x, y, w, h;

   Eina_List *l;
   int n, k, num_active_edges, y0, y1, *sorted_index, j;
   RGBA_Edge *edges;
   RGBA_Vertex *point;
   Evas_GL_Polygon_Point *pt;
   Eina_Inlist *spans;
   
   /* save out clip info */
   c = gc->dc->clip.use; cx = gc->dc->clip.x; cy = gc->dc->clip.y; cw = gc->dc->clip.w; ch = gc->dc->clip.h;

   ca = (gc->dc->col.col >> 24) & 0xff;
   if (ca <= 0) return;
   cr = (gc->dc->col.col >> 16) & 0xff;
   cg = (gc->dc->col.col >> 8 ) & 0xff;
   cb = (gc->dc->col.col      ) & 0xff;
   
   n = eina_list_count(poly->points);
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
   EINA_LIST_FOREACH(poly->points, l, pt)
     {
        point[k].x = pt->x;
        point[k].y = pt->y;
        point[k].i = k;
        k++;
     }
   qsort(point, n, sizeof(RGBA_Vertex), polygon_point_sorter);
   for (k = 0; k < n; k++) sorted_index[k] = point[k].i;
   k = 0;

   EINA_LIST_FOREACH(poly->points, l, pt)
     {
        point[k].x = pt->x;
        point[k].y = pt->y;
        point[k].i = k;
        k++;
     }
   
   y0 = MAX(cy, ceil(point[sorted_index[0]].y - 0.5));
   y1 = MIN(cy + ch - 1, floor(point[sorted_index[n - 1]].y - 0.5));
   
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
             if ((x1 >= cx) && (x0 < (cx + cw)) && (x0 <= x1))
               {
                  RGBA_Span *span;
                  
                  if (x0 < cx) x0 = cx;
                  if (x1 >= (cx + cw)) x1 = cx + cw - 1;
                  span = malloc(sizeof(RGBA_Span));
                  spans = eina_inlist_append(spans, EINA_INLIST_GET(span));
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
   
   evas_common_draw_context_clip_clip(gc->dc, 0, 0, gc->w, gc->h);
   
   if (spans)
     {
        RGBA_Span *span;

        /* no cutouts - cut right to the chase */
        if (!gc->dc->cutout.rects)
          {
             EINA_INLIST_FOREACH(spans, span)
               {
                  x = span->x;
                  y = span->y;
                  w = span->w;
                  h = 1;
                  evas_gl_common_context_rectangle_push(gc, x, y, w, h, cr, cg, cb, ca);
               }
          }
        else
          {
             evas_common_draw_context_clip_clip(gc->dc, x, y, w, h);
             /* our clip is 0 size.. abort */
             if ((gc->dc->clip.w > 0) && (gc->dc->clip.h > 0))
               {
                  rects = evas_common_draw_context_apply_cutouts(gc->dc);
                  for (i = 0; i < rects->active; ++i)
                    {
                       r = rects->rects + i;
                       if ((r->w > 0) && (r->h > 0))
                         {
                            EINA_INLIST_FOREACH(spans, span)
                              {
                                 x = span->x;
                                 y = span->y;
                                 w = span->w;
                                 h = 1;
                                 RECTS_CLIP_TO_RECT(x, y, w, h, r->x, r->y, r->w, r->h);
                                 if ((w > 0) && (h > 0))
                                   evas_gl_common_context_rectangle_push(gc, x, y, w, h, cr, cg, cb, ca);
                              }
                         }
                    }
                  evas_common_draw_context_apply_clear_cutouts(rects);
               }
          }
        while (spans)
          {
             span = (RGBA_Span *)spans;
             spans = eina_inlist_remove(spans, spans);
             free(span);
          }    
     }       
   
   /* restore clip info */
   gc->dc->clip.use = c; gc->dc->clip.x = cx; gc->dc->clip.y = cy; gc->dc->clip.w = cw; gc->dc->clip.h = ch;

}
