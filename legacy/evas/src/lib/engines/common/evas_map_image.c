/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "evas_common.h"
#include "evas_blend_private.h"

typedef struct _Line Line;
typedef struct _Span Span;

struct _Span
{
   int x1, x2;
   FPc  u[2], v[2];
};

struct _Line
{
   Span span[2];
};

EAPI void
evas_common_map4_rgba(RGBA_Image *src, RGBA_Image *dst,
                      RGBA_Draw_Context *dc,
                      RGBA_Map_Point *p, 
                      int smooth, int level)
{
   int i;
   int ytop, ybottom, ystart, yend, y;
   int py[4];
   Line *spans = NULL;

   ytop = p[0].y;
   for (i = 1; i < 4; i++)
     {
        if (p[i].y < ytop) ytop = p[i].y;
     }
   ybottom = p[0].y;
   for (i = 1; i < 4; i++)
     {
        if (p[i].y > ybottom) ybottom = p[i].y;
     }
   
   ytop = ytop >> FP;
   ybottom = ybottom >> FP;
   
   if ((ytop >= dst->cache_entry.h) || (ybottom < 0)) return;
   
   if (ystart < 0) ystart = 0;
   else ystart =  ytop;
   if (ybottom >= dst->cache_entry.h) yend = dst->cache_entry.h - 1;
   else yend = ybottom;

   // generate a lise of spans. eg:
   // 
   //       H
   //     |--|
   //   |-----|
   // |--------|
   //   |-------|
   //      |---|
   //        ||
   // 
   // up to 2 spans per line (because its a quad).

#if 1
   // maybe faster on x86?
   for (i = 0; i < 4; i++) py[i] = p[i].y >> FP;
# define PY(x) (py[x])   
#else
# define PY(x) (p[x].y >> FP) 
#endif
   spans = alloca((yend - ystart + 1) * sizeof(Line));
   memset(spans, 0, (yend - ystart + 1) * sizeof(Line));
   
   // all on one line. eg:
   // 
   // |----------|
   if ((PY(0) == PY(1)) && (PY(0) == PY(2)) && (PY(0) == PY(3)))
     {
        // find min x point and max x point and span those
     }
   else
     {
        // convex case
        
        // |------------|
        // |------------|
        // |------------|
        // |------------|
        // |------------|
        
        // |------------|
        //  |----------|
        //   |--------|
        //    |------|
        //     |----|

        // |--------|
        // |--------|
        // |--------|
        // |-----|
        // |--|
        
        // |--|
        // |-----|
        // |--------|
        // |--------|
        // |--------|

        // |--|
        // |-----|
        // |--------|
        // |-----|
        // |--|
        
        //     |--|
        //   |------|
        // |----------|
        //   |------|
        //     |--|
        
        // concave case
        
        //       |--|
        //     |------|
        //   |----||----|
        // |--|        |--|

        // |--|        |--|
        //   |----||----|
        //     |------|
        //       |--|
       
        for (y = ystart; y <= yend; y++)
          {
             int edge[4][3], edge_num, x, t, h, swapped, order[4];
             FPc uv[4][2], u, v;
        
             
             edge_num = 0;
             //--------------------
             if ((PY(0) <= y) && (PY(1) > y)) // 0 above, 1, below
               {
                  edge[edge_num][0] = 0;
                  edge[edge_num][1] = 1;
                  edge_num++;
               }
             else if ((PY(1) <= y) && (PY(0) > y)) // 1 above, 0, below
               {
                  edge[edge_num][0] = 1;
                  edge[edge_num][1] = 0;
                  edge_num++;
               }
             //--------------------
             if ((PY(1) <= y) && (PY(2) > y)) // 1 above, 2, below
               {
                  edge[edge_num][0] = 1;
                  edge[edge_num][1] = 2;
                  edge_num++;
               }
             else if ((PY(2) <= y) && (PY(1) > y)) // 2 above, 1, below
               {
                  edge[edge_num][0] = 2;
                  edge[edge_num][1] = 1;
                  edge_num++;
               }
             //--------------------
             if ((PY(2) <= y) && (PY(3) > y)) // 2 above, 3, below
               {
                  edge[edge_num][0] = 2;
                  edge[edge_num][1] = 3;
                  edge_num++;
               }
             else if ((PY(3) <= y) && (PY(2) > y)) // 3 above, 2, below
               {
                  edge[edge_num][0] = 3;
                  edge[edge_num][1] = 2;
                  edge_num++;
               }
             //--------------------
             if ((PY(3) <= y) && (PY(0) > y)) // 3 above, 0, below
               {
                  edge[edge_num][0] = 3;
                  edge[edge_num][1] = 0;
                  edge_num++;
               }
             else if ((PY(0) <= y) && (PY(3) > y)) // 0 above, 3, below
               {
                  edge[edge_num][0] = 0;
                  edge[edge_num][1] = 3;
                  edge_num++;
               }
             // calculate line x points for each edge
             for (i = 0; i < edge_num; i++)
               {
                  int e1 = edge[i][0], e2 = edge[i][1];

                  h = p[e2].y - p[e1].y; // height of span
                  t = (((y + 1) << FP) + (FP1 - 1)) - p[e1].y;
                  x = p[e2].x - p[e1].x;
                  x = p[e1].x + ((x * t) / h);
                  u = p[e2].u - p[e1].u;
                  u = p[e1].u + ((u * t) / h);
                  v = p[e2].v - p[e1].v;
                  v = p[e1].v + ((v * t) / h);
                  uv[i][1] = v;
                  uv[i][0] = u;
                  edge[i][2] = x >> FP;
                  // also fill in order
                  order[i] = i;
               }
             // sort edges from left to right - bubble. its a small list!
             do
               {
                  swapped = 0;
                  for (i = 0; i < edge_num; i++)
                    {
                       if (edge[order[i]][2] > edge[order[i + 1]][2])
                         {
                            t = order[i];
                            order[i] = order[i + 1];
                            order[i] = t;
                            swapped = 1;
                         }
                    }
               }
             while (swapped);
             if (edge_num == 2)
               {
                  spans[y].span[0].x1 = edge[order[0]][2];
                  spans[y].span[0].u[0] = uv[order[0]][0];
                  spans[y].span[0].v[0] = uv[order[0]][1];
                  spans[y].span[0].x2 = edge[order[1]][2];
                  spans[y].span[0].u[0] = uv[order[1]][0];
                  spans[y].span[0].v[0] = uv[order[1]][1];
                  spans[y].span[1].x1 = -1;
               }
             else if (edge_num == 4)
               {
                  spans[y].span[0].x1 = edge[order[0]][2];
                  spans[y].span[0].u[0] = uv[order[0]][0];
                  spans[y].span[0].v[0] = uv[order[0]][1];
                  spans[y].span[0].x2 = edge[order[1]][2];
                  spans[y].span[0].u[0] = uv[order[1]][0];
                  spans[y].span[0].v[0] = uv[order[1]][1];
                  spans[y].span[1].x1 = edge[order[2]][2];
                  spans[y].span[1].u[0] = uv[order[2]][0];
                  spans[y].span[1].v[0] = uv[order[2]][1];
                  spans[y].span[1].x2 = edge[order[3]][2];
                  spans[y].span[1].u[0] = uv[order[3]][0];
                  spans[y].span[1].v[0] = uv[order[3]][1];
               }
             else
               {
                  printf("duck for y %i, edge_num is %i\n", y, edge_num);
               }
          }
     }
   // walk spans and fill
   for (y = ystart; y <= yend; y++)
     {
        
     }
}
