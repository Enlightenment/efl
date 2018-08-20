#define PUSH_EDGE_POINT() \
do \
{ \
   p_edge.x = spans[y].span[0].x[eidx]; \
   p_edge.y = y; \
   ptx[0] = tx[0]; \
   ptx[1] = tx[1]; \
} while (0)

static void
calc_irregular_coverage(Line* spans, int eidx, int y, int diagonal,
                       int edge_dist, Eina_Bool reverse)
{
   if (eidx == 1) reverse = !reverse;
   int coverage = (256 / (diagonal + 2));
   int tmp;
   for (int ry = 0; ry < (diagonal + 2); ry++)
     {
        tmp = y - ry - edge_dist;
        if (tmp < 0) return;
        spans[tmp].aa_len[eidx] = 1;
        if (reverse) spans[tmp].aa_cov[eidx] = 256 - (coverage * ry);
        else spans[tmp].aa_cov[eidx] = (coverage * ry);
     }
}

static void
calc_vert_coverage(Line *spans, int eidx, int y, int rewind, Eina_Bool reverse)
{
   if (eidx == 1) reverse = !reverse;
   int coverage = (256 / (rewind + 1));
   int tmp;
   for (int ry = 1; ry < (rewind + 1); ry++)
     {
        tmp = y - ry;
        if (tmp < 0 ) return;
        spans[tmp].aa_len[eidx] = 1;
        if (reverse) spans[tmp].aa_cov[eidx] = (256 - (coverage * ry));
        else spans[tmp].aa_cov[eidx] = (coverage * ry);
     }
}

static void
calc_horiz_coverage(Line *spans, int eidx, int y, int x, int x2)
{
   if (spans[y].aa_len[eidx] < abs(x - x2))
     {
        spans[y].aa_len[eidx] = abs(x - x2);
        spans[y].aa_cov[eidx] = (256 / (spans[y].aa_len[eidx] + 1));
     }
}

static inline DATA32
_aa_coverage_apply(Line *line, int ww, int w, DATA32 val, Eina_Bool src_alpha)
{
   //Left Edge Anti Anliasing
   if ((w - line->aa_len[0]) < ww)
     {
        return MUL_256((line->aa_cov[0] * (w - ww + 1)), val);
     }
   //Right Edge Anti Aliasing
   if (line->aa_len[1] >= ww)
     {
        return MUL_256(256 - (line->aa_cov[1] * (line->aa_len[1] - ww + 1)),
                       val);
     }
   //Remove Transparency if src image alpha is off.
   if (!src_alpha)
     {
        if (((val & 0xff000000) >> 24) < 0xff)
          return (val | 0xff000000);
     }
   return val;
}

/*
 *	To understand here AA main logic,
 *	Please refer this page: hermet.pe.kr/122?catgory=662934
*/
static void
_calc_aa_edges_internal(Line *spans, int eidx, int ystart, int yend)
{
   int y;
   Evas_Coord_Point p_edge = {-1, -1};  //previous edge point
   Evas_Coord_Point edge_diff = {0, 0}; //temporary used for point distance 

   /* store bigger to tx[0] between prev and current edge's x positions. */
   int tx[2] = {0, 0};
   /* back up prev tx values */
   int ptx[2] = {0, 0};
   int diagonal = 0;                      //straight diagonal pixels counti

//Previous edge direction:
#define DirOutHor 0x0011
#define DirOutVer 0x0001
#define DirInHor  0x0010
#define DirInVer  0x0000
#define DirNone   0x1000

   int prev_dir = DirNone;
   int cur_dir = DirNone;

   yend -= ystart;

   //Find Start Edge
   for (y = 0; y < yend; y++)
     {
        if (spans[y].span[0].x[0] == -1) continue;
        p_edge.x = spans[y].span[0].x[eidx];
        p_edge.y = y;
        break;
     }

   //Calculates AA Edges
   for (y++; y < yend; y++)
     {
        //Ready tx
        if (eidx == 0)
          {
             tx[0] = p_edge.x;
             tx[1] = spans[y].span[0].x[0];
          }
        else
          {
             tx[0] = spans[y].span[0].x[1];
             tx[1] = p_edge.x;
          }

        edge_diff.x = (tx[0] - tx[1]);
        edge_diff.y = (y - p_edge.y);

        //Confirm current edge direction
        if (edge_diff.x > 0)
          {
             if (edge_diff.y == 1) cur_dir = DirOutHor;
             else cur_dir = DirOutVer;
          }
        else if (edge_diff.x < 0)
          {
             if (edge_diff.y == 1) cur_dir = DirInHor;
             else cur_dir = DirInVer;
          }
        else cur_dir = DirNone;

       //straight diagonal increase
       if ((cur_dir == prev_dir) && (y < yend))
         {
            if ((abs(edge_diff.x) == 1) && (edge_diff.y == 1))
              {
                 ++diagonal;
                 PUSH_EDGE_POINT();
                 continue;
              }
         }
       switch (cur_dir)
         {
          case DirOutHor:
            {
               calc_horiz_coverage(spans, eidx, y, tx[0], tx[1]);
               if (diagonal > 0)
                 {
                    calc_irregular_coverage(spans, eidx, y, diagonal, 0,
                                            EINA_TRUE);
                    diagonal = 0;
                 }
               /* Increment direction is changed:
                  Outside Vertical -> Outside Horizontal */
               if (prev_dir == DirOutVer)
                 calc_horiz_coverage(spans, eidx, p_edge.y, ptx[0], ptx[1]);
               PUSH_EDGE_POINT();
            }
          break;
          case DirOutVer:
            {
               calc_vert_coverage(spans, eidx, y, edge_diff.y, EINA_TRUE);
               if (diagonal > 0)
                 {
                    calc_irregular_coverage(spans, eidx, y, diagonal,
                                            edge_diff.y, EINA_FALSE);
                    diagonal = 0;
                 }
               /* Increment direction is changed:
                  Outside Horizontal -> Outside Vertical */
               if (prev_dir == DirOutHor)
                 calc_horiz_coverage(spans, eidx, p_edge.y, ptx[0], ptx[1]);
               PUSH_EDGE_POINT();
            }
          break;
          case DirInHor:
            {
               calc_horiz_coverage(spans, eidx, (y - 1), tx[0], tx[1]);
               if (diagonal > 0)
                 {
                    calc_irregular_coverage(spans, eidx, y, diagonal, 0,
                                            EINA_FALSE);
                    diagonal = 0;
                 }
               /* Increment direction is changed:
                  Outside Horizontal -> Inside Horizontal */
               if (prev_dir == DirOutHor)
                 calc_horiz_coverage(spans, eidx, p_edge.y, ptx[0], ptx[1]);
               PUSH_EDGE_POINT();
            }
          break;
          case DirInVer:
            {
               calc_vert_coverage(spans, eidx, y, edge_diff.y, EINA_FALSE);
               if (diagonal > 0)
                 {
                    calc_irregular_coverage(spans, eidx, y, diagonal,
                                            edge_diff.y, EINA_TRUE);
                    diagonal = 0;
                 }
               /* Increment direction is changed:
                  Outside Horizontal -> Inside Vertical */
               if (prev_dir == DirOutHor)
                 calc_horiz_coverage(spans, eidx, p_edge.y, ptx[0], ptx[1]);
               PUSH_EDGE_POINT();
            }
          break;
         }
       if (cur_dir != DirNone) prev_dir = cur_dir;
     }

   //leftovers...?
   if ((edge_diff.y == 1) && (edge_diff.x != 0))
     {
        calc_horiz_coverage(spans, eidx, y - 1, ptx[0], ptx[1]);
        calc_horiz_coverage(spans, eidx, y, tx[0], tx[1]);
     }
   else
     calc_vert_coverage(spans, eidx, (y + 1), (edge_diff.y + 2),
                        (prev_dir & 0x00000001));
}

static void
_calc_aa_edges(Line *spans, int ystart, int yend)
{
   //FIXME: support 2 span case.

   //left side
   _calc_aa_edges_internal(spans, 0, ystart, yend);
   //right side
   _calc_aa_edges_internal(spans, 1, ystart, yend);
}
