#define  READY_TX() \
{ \
  if (eidx == 0) \
    { \
       tx[0] = edge2.x; \
       tx[1] = spans[y].span[0].x[0]; \
    } \
  else \
    { \
       tx[0] = spans[y].span[0].x[1]; \
       tx[1] = edge2.x; \
    } \
}

#define READY_TX2() \
{ \
   if (eidx == 0) \
     { \
        tx2[0] = edge2.x; \
        tx2[1] = edge1.x; \
     } \
   else \
    { \
       tx2[0] = edge1.x; \
       tx2[1] = edge2.x; \
    } \
}

#define PUSH_EDGES(xx) \
{ \
   if (!leftover) \
     { \
        edge1.x = edge2.x; \
        edge1.y = edge2.y; \
        edge2.x = (xx); \
        edge2.y = y; \
     } \
   else \
     { \
        edge1.y = edge2.y; \
        edge2.y = y; \
     } \
   reset_tx2 = EINA_TRUE; \
}

//Vertical Inside Direction
#define VERT_INSIDE(rewind, y_advance) \
{ \
   int cov_range = edge2.y - edge1.y; \
   int coverage = (256 / (cov_range + 1)); \
   int ry; \
   int val; \
   for (ry = 1; ry < ((rewind) + 1); ry++) \
     { \
        int ridx = (y - ry) + (y_advance); \
        if (spans[ridx].aa_len[eidx] > 1) continue; \
        if (eidx == 1) \
          val = (256 - (coverage * (ry + (cov_range - (rewind))))); \
        else \
          val = (coverage * (ry + (cov_range - (rewind)))); \
        if ((spans[ridx].aa_len[eidx] == 0) || \
            (val < spans[ridx].aa_cov[eidx])) \
          spans[ridx].aa_cov[eidx] = val; \
        spans[ridx].aa_len[eidx] = 1; \
     } \
   prev_aa = 4; \
}

//Vertical Outside Direction
#define VERT_OUTSIDE(rewind, y_advance, cov_range) \
{ \
   int coverage = (256 / ((cov_range) + 1)); \
   int ry = 1; \
   for (; ry < ((rewind) + 1); ry++) \
     { \
        int ridx = (y - ry) + (y_advance); \
        if (spans[ridx].aa_len[(eidx)] > 1) continue; \
        spans[ridx].aa_len[(eidx)] = 1; \
        if (eidx == 1) \
          { \
             spans[ridx].aa_cov[(eidx)] = \
                (coverage * (ry + (cov_range - (rewind)))); \
          } \
        else \
          { \
             spans[ridx].aa_cov[(eidx)] = \
                (256 - (coverage * (ry + ((cov_range) - (rewind))))); \
          } \
     } \
   prev_aa = 2; \
}

//Horizontal Inside Direction
#define HORIZ_INSIDE(yy, xx, xx2) \
{ \
   if (((xx) - (xx2)) > spans[(yy)].aa_len[(eidx)]) \
     { \
        spans[(yy)].aa_len[(eidx)] = ((xx) - (xx2)); \
        spans[(yy)].aa_cov[(eidx)] = (256 / (spans[(yy)].aa_len[(eidx)] + 1)); \
     } \
}

//Horizontal Outside Direction
#define HORIZ_OUTSIDE(yy, xx, xx2) \
{ \
   if (((xx) - (xx2)) > spans[(yy)].aa_len[(eidx)]) \
     { \
        spans[(yy)].aa_len[(eidx)] = ((xx) - (xx2)); \
        spans[(yy)].aa_cov[(eidx)] = (256 / (spans[(yy)].aa_len[(eidx)] + 1)); \
     } \
}

static inline DATA32
_aa_coverage_apply(Line *line, int ww, int w, DATA32 val)
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
   return val;
}

void
_calc_aa_edges_internal(Line *spans, int eidx, int ystart, int yend)
{
   int y;
   Evas_Coord_Point edge1 = { -1, -1 }; //prev-previous edge pixel
   Evas_Coord_Point edge2 = { -1, -1 }; //previous edge pixel

   /* store larger to tx[0] between prev and current edge's x positions. */
   int tx[2] = {0, 0};

   /* store lager to tx2[0] between edge1 and edge2's x positions. */
   int tx2[2] = {0, 0};

   /* previous edge anti-aliased type.
      2: vertical outside
      4: vertical inside */
   int prev_aa = 0;

   Eina_Bool reset_tx2 = EINA_TRUE;

   yend -= ystart;

   //Find Start Edge
   for (y = 0; y < yend; y++)
     {
        if (spans[y].span[0].x[0] == -1) continue;
        edge1.x = edge2.x = spans[y].span[0].x[eidx];
        edge1.y = edge2.y = y;
        break;
     }

   //Calculates AA Edges
   for (y++; y <= yend; y++)
     {
       Eina_Bool leftover = EINA_FALSE;

       if (spans[y].span[0].x[eidx] == -1) leftover = EINA_TRUE;

       if (!leftover) READY_TX()

       //Case1. Outside Incremental
       if (tx[0] > tx[1])
         {
            //Horizontal Edge
            if ((y - edge2.y) == 1)
              {
                  HORIZ_OUTSIDE(y, tx[0], tx[1])
              }
            //Vertical Edge
            else if (tx[0] > tx[1])
              {
                 VERT_OUTSIDE((y - edge2.y), 0, (y - edge2.y))

                 //Just in case: 1 pixel alias next to vertical edge?
                 if (abs(spans[(y + 1)].span[0].x[eidx] -
                         spans[y].span[0].x[eidx]) >= 1)
                   {
                      HORIZ_OUTSIDE(y, tx[0], tx[1])
                   }
              }
            PUSH_EDGES(spans[y].span[0].x[eidx])
         }
       //Case2. Inside Incremental
       else if (tx[1] > tx[0])
         {
            //Just in case: direction is reversed at the outside vertical edge?
            if (prev_aa == 2)
              {
                 VERT_OUTSIDE((y - edge2.y), 0, (y - edge2.y))
                 edge1.x = spans[y - 1].span[0].x[eidx];
                 edge1.y = y - 1;
                 edge2.x = spans[y].span[0].x[eidx];
                 edge2.y = y;
              }
            else
              PUSH_EDGES(spans[y].span[0].x[eidx])

            /* Find next edge. We go forward 2 more index since this logic
               computes aa edges by looking back in advance 2 spans. */
            for (y++; y <= (yend + 2); y++)
              {
                 leftover = EINA_FALSE;

                 if ((spans[y].span[0].x[eidx] == -1) || (y > yend))
                   leftover = EINA_TRUE;

                 if (!leftover) READY_TX()
                 if (reset_tx2) READY_TX2()

                 //Case 1. Inside Direction
                 if (tx[1] > tx[0])
                   {
                      //Horizontal Edge
                      if ((edge2.y - edge1.y) == 1)
                        {
                           HORIZ_INSIDE(edge1.y, tx2[0], tx2[1]);
                        }
                      //Vertical Edge
                      else if ((tx2[0] - tx2[1]) == 1)
                        {
                           VERT_INSIDE((edge2.y - edge1.y), -(y - edge2.y))
                        }
                      //Just in case: Right Side Square Edge...?
                      else if (prev_aa == 4)
                        {
                           VERT_INSIDE((edge2.y - edge1.y), -(y - edge2.y))
                           if ((y - edge2.y) == 1)
                             {
                                HORIZ_INSIDE((edge2.y - 1), edge2.x,
                                             spans[y].span[0].x[eidx]);
                             }
                        }
                      PUSH_EDGES(spans[y].span[0].x[eidx])
                   }
                 //Case 2. Reversed. Outside Direction
                 else if (tx[1] < tx[0])
                   {
                      //Horizontal Edge
                      if ((edge2.y - edge1.y) == 1)
                        HORIZ_INSIDE(edge1.y, tx2[0], tx2[1])
                      //Vertical Edge
                      else
                        VERT_INSIDE((edge2.y - edge1.y), -(y - edge2.y))

                      PUSH_EDGES(spans[y].span[0].x[eidx])
                      break;
                   }
              }
         }
     }

   y = yend;

   //Leftovers for verticals.
   if (prev_aa == 2)
     {
      if (((eidx == 0) && (edge1.x > edge2.x)) ||
          ((eidx == 1) && (edge1.x < edge2.x)))
        VERT_OUTSIDE((y - edge2.y + 1), 1, (edge2.y - edge1.y));
     }
   else if (prev_aa == 4)
     {
        if (((eidx == 0) && (edge1.x < edge2.x)) ||
           ((eidx == 1) && (edge1.x > edge2.x)))
          {
             VERT_INSIDE((edge2.y - edge1.y), -(y - edge2.y))
             VERT_INSIDE((y - edge2.y) + 1, 1);
          }
     }
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
