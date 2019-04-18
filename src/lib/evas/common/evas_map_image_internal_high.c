typedef struct
{
   float x, y, u, v;
   DATA32 c;
} Map_Vertex;

typedef struct
{
   Map_Vertex v[3];
} Map_Tripoly;

typedef struct AALine
{
   int x[2];
   int aa_cov[2];
   int aa_len[2];
} AALine;

typedef struct AASpans
{
   AALine *lines;
   int ystart;
   int yend;
} AASpans;

static float dudx, dvdx, dcdx[4];
static float dxdya, dxdyb, dudya, dvdya, dcdya[4];
static float xa, xb, ua, va, ca[4];

#define SWAP(a, b, tmp) \
   tmp = a; \
   a = b; \
   b = tmp

/************************** ANTI-ALIASING CODE ********************************/
static void
_map_irregular_coverage_calc(AALine* spans, int eidx, int y, int diagonal,
                       int edge_dist, Eina_Bool reverse)
{
   if (eidx == 1) reverse = !reverse;
   int coverage = (255 / (diagonal + 2));
   int tmp;
   for (int ry = 0; ry < (diagonal + 2); ry++)
     {
        tmp = y - ry - edge_dist;
        if (tmp < 0) return;
        spans[tmp].aa_len[eidx] = 1;
        if (reverse) spans[tmp].aa_cov[eidx] = 255 - (coverage * ry);
        else spans[tmp].aa_cov[eidx] = (coverage * ry);
     }
}

static void
_map_vert_coverage_calc(AALine *spans, int eidx, int y, int rewind, Eina_Bool reverse)
{
   if (eidx == 1) reverse = !reverse;
   int coverage = (255 / (rewind + 1));
   int tmp;
   for (int ry = 1; ry < (rewind + 1); ry++)
     {
        tmp = y - ry;
        if (tmp < 0 ) return;
        spans[tmp].aa_len[eidx] = 1;
        if (reverse) spans[tmp].aa_cov[eidx] = (255 - (coverage * ry));
        else spans[tmp].aa_cov[eidx] = (coverage * ry);
     }
}

static void
_map_horiz_coverage_calc(AALine *spans, int eidx, int y, int x, int x2)
{
   if (spans[y].aa_len[eidx] < abs(x - x2))
     {
        spans[y].aa_len[eidx] = abs(x - x2);
        spans[y].aa_cov[eidx] = (255 / (spans[y].aa_len[eidx] + 1));
     }
}

/*
 * To understand here AA main logic,
 * Please refer this page: www.hermet.pe.kr/122
*/
static void
_map_aa_edge_calc_internal(AALine *spans, int eidx, int ystart, int yend)
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

#define PUSH_VERTEX() \
do \
{ \
   p_edge.x = spans[y].x[eidx]; \
   p_edge.y = y; \
   ptx[0] = tx[0]; \
   ptx[1] = tx[1]; \
} while (0)

   int prev_dir = DirNone;
   int cur_dir = DirNone;

   yend -= ystart;

   //Find Start Edge
   for (y = 0; y < yend; y++)
     {
        p_edge.x = spans[y].x[eidx];
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
             tx[1] = spans[y].x[0];
          }
        else
          {
             tx[0] = spans[y].x[1];
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
                 PUSH_VERTEX();
                 continue;
              }
         }
       switch (cur_dir)
         {
          case DirOutHor:
            {
               _map_horiz_coverage_calc(spans, eidx, y, tx[0], tx[1]);
               if (diagonal > 0)
                 {
                    _map_irregular_coverage_calc(spans, eidx, y, diagonal, 0,
                                            EINA_TRUE);
                    diagonal = 0;
                 }
               /* Increment direction is changed:
                  Outside Vertical -> Outside Horizontal */
               if (prev_dir == DirOutVer)
                 _map_horiz_coverage_calc(spans, eidx, p_edge.y, ptx[0], ptx[1]);

               //Trick, but fine-tunning!
               if (y == 1)
                 _map_horiz_coverage_calc(spans, eidx, p_edge.y, tx[0], tx[1]);

               PUSH_VERTEX();
            }
          break;
          case DirOutVer:
            {
               _map_vert_coverage_calc(spans, eidx, y, edge_diff.y, EINA_TRUE);
               if (diagonal > 0)
                 {
                    _map_irregular_coverage_calc(spans, eidx, y, diagonal,
                                            edge_diff.y, EINA_FALSE);
                    diagonal = 0;
                 }
               /* Increment direction is changed:
                  Outside Horizontal -> Outside Vertical */
               if (prev_dir == DirOutHor)
                 _map_horiz_coverage_calc(spans, eidx, p_edge.y, ptx[0], ptx[1]);
               PUSH_VERTEX();
            }
          break;
          case DirInHor:
            {
               _map_horiz_coverage_calc(spans, eidx, (y - 1), tx[0], tx[1]);
               if (diagonal > 0)
                 {
                    _map_irregular_coverage_calc(spans, eidx, y, diagonal, 0,
                                            EINA_FALSE);
                    diagonal = 0;
                 }
               /* Increment direction is changed:
                  Outside Horizontal -> Inside Horizontal */
               if (prev_dir == DirOutHor)
                 _map_horiz_coverage_calc(spans, eidx, p_edge.y, ptx[0], ptx[1]);
               PUSH_VERTEX();
            }
          break;
          case DirInVer:
            {
               _map_vert_coverage_calc(spans, eidx, y, edge_diff.y, EINA_FALSE);
               if (diagonal > 0)
                 {
                    _map_irregular_coverage_calc(spans, eidx, y, diagonal,
                                            edge_diff.y, EINA_TRUE);
                    diagonal = 0;
                 }
               /* Increment direction is changed:
                  Outside Horizontal -> Inside Vertical */
               if (prev_dir == DirOutHor)
                 _map_horiz_coverage_calc(spans, eidx, p_edge.y, ptx[0], ptx[1]);
               PUSH_VERTEX();
            }
          break;
         }
       if (cur_dir != DirNone) prev_dir = cur_dir;
     }

   //leftovers...?
   if ((edge_diff.y == 1) && (edge_diff.x != 0))
     {
        if (y >= yend) y = (yend - 1);
        _map_horiz_coverage_calc(spans, eidx, y - 1, ptx[0], ptx[1]);
        _map_horiz_coverage_calc(spans, eidx, y, tx[0], tx[1]);
     }
   else
     {
        ++y;
        if (y > yend) y = yend;
        _map_vert_coverage_calc(spans, eidx, y, (edge_diff.y + 1),
                                (prev_dir & 0x00000001));
     }
}

static void
_map_aa_edges_calc(AALine *spans, int ystart, int yend)
{
   //FIXME: support more than 2 span case.

   //left side
   _map_aa_edge_calc_internal(spans, 0, ystart, yend);
   //right side
   _map_aa_edge_calc_internal(spans, 1, ystart, yend);
}

static AASpans *
_map_aa_ready(int dw, int dh, int ystart, int yend)
{
   AASpans *aa_spans = (AASpans *) malloc(sizeof(AASpans));
   if (!aa_spans) return NULL;

   aa_spans->lines = (AALine*) calloc(sizeof(AALine), dh);
   for (int i = 0; i < dh; i++)
     {
        aa_spans->lines[i].x[0] = dw + 1;
        aa_spans->lines[i].x[1] = -1;
     }
   aa_spans->ystart = (int) ystart;
   aa_spans->yend = (int) yend;

   return aa_spans;
}

static void
_map_aa_apply(AASpans *aa_spans, DATA32 *dst, int dw)
{
   int y = aa_spans->ystart;
   int yend = aa_spans->yend;
   AALine *line;
   DATA32 *buf;
   DATA32 d;
   int w, offset, pos;

   _map_aa_edges_calc(aa_spans->lines, y, yend);

   while (y < yend)
     {
        line = &aa_spans->lines[y - aa_spans->ystart];
        w = line->x[1] - line->x[0];
        if (w > 0)
          {
             offset = y * dw;

             //Left edge
             buf = dst + (offset + line->x[0]);
             if (line->x[0] > 1) d = *(dst + (offset + (line->x[0] - 1)));
             else d = *buf;
             pos = 1;
             while (pos <= line->aa_len[0])
               {
                  *buf = INTERP_256((line->aa_cov[0] * pos), *buf, d);
                  ++buf;
                  ++pos;
               }

             //Right edge
             buf = dst + (offset + line->x[1] - 1);
             if (line->x[1] < (dw - 1)) d = *(dst + (offset + (line->x[1] + 1)));
             else d = *buf;
             pos = w;
             while (w - line->aa_len[1] < pos)
               {
                  *buf = INTERP_256(255 - (line->aa_cov[1] * (line->aa_len[1] - (w - pos))), *buf, d);
                  buf--;
                  pos--;
               }
          }
        y++;
     }

   free(aa_spans->lines);
   free(aa_spans);
}

/************************** TEXTURE MAPPING CODE ******************************/
static void
_map_triangle_draw_linear(RGBA_Image *src, RGBA_Image *dst,
                          int cx, int cy, int cw, int ch,
                          RGBA_Image *mask, int mx, int my,
                          int ystart, int yend,
                          DATA32 *tbuf, RGBA_Gfx_Func func, RGBA_Gfx_Func func2,
                          DATA32 mul_col, AASpans *aa_spans,
                          Eina_Bool col_blend)
{
   float _dudx = dudx, _dvdx = dvdx;
   float _dxdya = dxdya, _dxdyb = dxdyb, _dudya = dudya, _dvdya = dvdya;
   float _xa = xa, _xb = xb, _ua = ua, _va = va;
   DATA32 *sbuf = src->image.data;
   DATA32 *dbuf = dst->image.data;
   int sw = src->cache_entry.w;
   int sh = src->cache_entry.h;
   int dw = dst->cache_entry.w;
   int x1, x2, x, y, uu, vv, ar, ab, iru, irv, px, ay;
   float dx, u, v, iptr;
   float _dcdx[4], _dcdya[4], _ca[4];
   float c[4] = {0, 0, 0, 0};
   DATA32 *buf, *tmp;
   DATA8 *mbuf;

   //Range exception handling
   if (ystart >= (cy + ch)) return;
   if (ystart < cy) ystart = cy;
   if (yend > (cy + ch)) yend = (cy + ch);

   if (col_blend)
     for (int i = 0; i < 4; i++)
       {
          _dcdx[i] = dcdx[i];
          _dcdya[i] = dcdya[i];
          _ca[i] = ca[i];
       }

   //Loop through all lines in the segment
   y = ystart;
   while (y < yend)
     {
        x1 = _xa;
        x2 = _xb;

        //Range exception handling
        if (x1 < cx) x1 = cx;
        if (x2 > (cx + cw)) x2 = (cx + cw);

        if (aa_spans)
          {
             ay = y - aa_spans->ystart;
             if (aa_spans->lines[ay].x[0] > x1) aa_spans->lines[ay].x[0] = x1;
             if (aa_spans->lines[ay].x[1] < x2) aa_spans->lines[ay].x[1] = x2;
          }

        if ((x2 - x1) < 1) goto next;

        //Perform subtexel pre-stepping on UV
        dx = 1 - (_xa - x1);
        u = _ua + dx * _dudx;
        v = _va + dx * _dvdx;

        if (col_blend)
          {
             c[0] = _ca[0] + dx * _dcdx[0];
             c[1] = _ca[1] + dx * _dcdx[1];
             c[2] = _ca[2] + dx * _dcdx[2];
             c[3] = _ca[3] + dx * _dcdx[3];
          }

        //Direct draw or blending intervention?
        if (tbuf) buf = tbuf;
        else buf = dbuf + ((y * dw) + x1);

        x = x1;

        //Draw horizontal line
        while (x++ < x2)
          {
             uu = (int) u;
             vv = (int) v;

             //Range exception handling
             //OPTIMIZE ME, handle in advance?
             if (uu >= sw) uu = sw - 1;
             if (vv >= sh) vv = sh - 1;

             ar = (int)(255 * (1 - modff(u, &iptr)));
             ab = (int)(255 * (1 - modff(v, &iptr)));
             iru = uu + 1;
             irv = vv + 1;
             px = *(sbuf + (vv * sw) + uu);

             //horizontal interpolate
             if (iru < sw)
               {
                  //right pixel
                  int px2 = *(sbuf + (vv * sw) + iru);
                  px = INTERP_256(ar, px, px2);
               }
             //vertical interpolate
             if (irv < sh)
               {
                  //bottom pixel
                  int px2 = *(sbuf + (irv * sw) + uu);

                  //horizontal interpolate
                  if (iru < sw)
                    {
                       //bottom right pixel
                       int px3 = *(sbuf + (irv * sw) + iru);
                       px2 = INTERP_256(ar, px2, px3);
                    }
                  px = INTERP_256(ab, px, px2);
               }

             if (!col_blend)
               {
                  *(buf++) = px;
               }
             //Vertex Color Blending
             else
               {
                  DATA32 tmp = (((int) c[0]) << 24) | (((int) c[1]) << 16) | (((int) c[2]) << 8) | ((int) c[3]);
                  *(buf++) = MUL4_SYM(tmp, px);
                  c[0] += _dcdx[0];
                  c[1] += _dcdx[1];
                  c[2] += _dcdx[2];
                  c[3] += _dcdx[3];
               }

             //Step UV horizontally
             u += _dudx;
             v += _dvdx;
          }

        if (tbuf)
          {
             tmp = dbuf + ((y *dw) + x1);
             int len = x2 - x1;
             if (!mask) func(tbuf, NULL, mul_col, tmp, len);
             else
               {
                  mbuf = mask->image.data8
                     + (y - my) * mask->cache_entry.w + (x1 - mx);
                  if (mul_col != 0xffffffff)
                    func2(tbuf, NULL, mul_col, tbuf, len);
                  func(tbuf, mbuf, 0, tmp, len);
               }
          }
next:
        //Step along both edges
        _xa += _dxdya;
        _xb += _dxdyb;
        _ua += _dudya;
        _va += _dvdya;

        if (col_blend)
          {
             _ca[0] += _dcdya[0];
             _ca[1] += _dcdya[1];
             _ca[2] += _dcdya[2];
             _ca[3] += _dcdya[3];
          }

        y++;
     }
   xa = _xa;
   xb = _xb;
   ua = _ua;
   va = _va;

   if (col_blend)
     {
        ca[0] = _ca[0];
        ca[1] = _ca[1];
        ca[2] = _ca[2];
        ca[3] = _ca[3];
     }
}

/* This mapping algorithm is based on Mikael Kalms's. */
static void
_map_triangle_draw(RGBA_Image *src, RGBA_Image *dst,
                   int cx, int cy, int cw, int ch,
                   RGBA_Image *mask, int mx, int my,
                   DATA32 *tbuf, RGBA_Gfx_Func func, RGBA_Gfx_Func func2,
                   Map_Tripoly *poly, DATA32 mul_col,
                   AASpans *aa_spans, Eina_Bool smooth EINA_UNUSED,
                   Eina_Bool col_blend)
{
   float x[3] = { poly->v[0].x, poly->v[1].x, poly->v[2].x };
   float y[3] = { poly->v[0].y, poly->v[1].y, poly->v[2].y };
   float u[3] = { poly->v[0].u, poly->v[1].u, poly->v[2].u };
   float v[3] = { poly->v[0].v, poly->v[1].v, poly->v[2].v };
   DATA32 c[3] = { poly->v[0].c, poly->v[1].c, poly->v[2].c };
   float off_y;
   float denom;
   float dxdy[3] = {0, 0, 0};
   float dudy, dvdy, dcdy[4];
   float tmp;
   int mc[3][4];     //3 vertex, 4 color channels.
   DATA32 tmpc;
   Eina_Bool side;
   Eina_Bool upper = EINA_FALSE;

   //Sort the vertices in ascending Y order
   if (y[0] > y[1])
     {
        SWAP(x[0], x[1], tmp);
        SWAP(y[0], y[1], tmp);
        SWAP(u[0], u[1], tmp);
        SWAP(v[0], v[1], tmp);
        SWAP(c[0], c[1], tmpc);
     }
   if (y[0] > y[2])
     {
        SWAP(x[0], x[2], tmp);
        SWAP(y[0], y[2], tmp);
        SWAP(u[0], u[2], tmp);
        SWAP(v[0], v[2], tmp);
        SWAP(c[0], c[2], tmpc);

     }
   if (y[1] > y[2])
     {
        SWAP(x[1], x[2], tmp);
        SWAP(y[1], y[2], tmp);
        SWAP(u[1], u[2], tmp);
        SWAP(v[1], v[2], tmp);
        SWAP(c[1], c[2], tmpc);
     }

   //Y indexes
   int yi[3] = { y[0], y[1], y[2] };

   //Skip drawing if it's too thin to cover any pixels at all.
   if ((yi[0] == yi[1] && yi[0] == yi[2]) ||
       ((int) x[0] == (int) x[1] && (int) x[0] == (int) x[2]))
     return;

   /* Calculate horizontal and vertical increments for UV axes (these
      calcs are certainly not optimal, although they're stable
      (handles any dy being 0) */
   denom = ((x[2] - x[0]) * (y[1] - y[0]) - (x[1] - x[0]) * (y[2] - y[0]));

   //Skip poly if it's an infinitely thin line
   if (denom == 0) return;

   denom = 1 / denom;   //Reciprocal for speeding up
   dudx = ((u[2] - u[0]) * (y[1] - y[0]) - (u[1] - u[0]) * (y[2] - y[0])) * denom;
   dvdx = ((v[2] - v[0]) * (y[1] - y[0]) - (v[1] - v[0]) * (y[2] - y[0])) * denom;
   dudy = ((u[1] - u[0]) * (x[2] - x[0]) - (u[2] - u[0]) * (x[1] - x[0])) * denom;
   dvdy = ((v[1] - v[0]) * (x[2] - x[0]) - (v[2] - v[0]) * (x[1] - x[0])) * denom;

   if (col_blend)
     {
        for (int i = 0; i < 3; ++i)
          {
             mc[i][0] = (c[i] & 0xff000000) >> 24;
             mc[i][1] = (c[i] & 0x00ff0000) >> 16;
             mc[i][2] = (c[i] & 0x0000ff00) >> 8;
             mc[i][3] = (c[i] & 0x000000ff);
          }
        for (int i = 0; i < 4; ++i)
          {
             dcdx[i] = ((mc[2][i] - mc[0][i]) * (y[1] - y[0]) - (mc[1][i] - mc[0][i]) * (y[2] - y[0])) * denom;
             dcdy[i] = ((mc[1][i] - mc[0][i]) * (x[2] - x[0]) - (mc[2][i] - mc[0][i]) * (x[1] - x[0])) * denom;
          }
     }

   //Calculate X-slopes along the edges
   if (y[1] > y[0]) dxdy[0] = (x[1] - x[0]) / (y[1] - y[0]);
   if (y[2] > y[0]) dxdy[1] = (x[2] - x[0]) / (y[2] - y[0]);
   if (y[2] > y[1]) dxdy[2] = (x[2] - x[1]) / (y[2] - y[1]);

   //Determine which side of the polygon the longer edge is on
   side = (dxdy[1] > dxdy[0]) ? EINA_TRUE:EINA_FALSE;

   if (y[0] == y[1]) side = x[0] > x[1];
   if (y[1] == y[2]) side = x[2] > x[1];

   //Longer edge is on the left side
   if (!side)
     {
        //Calculate slopes along left edge
        dxdya = dxdy[1];
        dudya = dxdya * dudx + dudy;
        dvdya = dxdya * dvdx + dvdy;

        if (col_blend)
          for (int i = 0; i < 4; i++)
            dcdya[i] = dxdya * dcdx[i] + dcdy[i];

        //Perform subpixel pre-stepping along left edge
        float dy = 1 - (y[0] - yi[0]);
        xa = x[0] + dy * dxdya;
        ua = u[0] + dy * dudya;
        va = v[0] + dy * dvdya;

        if (col_blend)
          for (int i = 0; i < 4; i++)
            ca[i] = mc[0][i] + dy * dcdya[i];

        //Draw upper segment if possibly visible
        if (yi[0] < yi[1])
          {
             off_y = y[0] < cy ? (cy - y[0]) : 0;
             xa += (off_y * dxdya);
             ua += (off_y * dudya);
             va += (off_y * dvdya);

             if (col_blend)
               for (int i = 0; i < 4; i++)
                 ca[i] += (off_y * dcdya[i]);

             // Set right edge X-slope and perform subpixel pre-stepping
             dxdyb = dxdy[0];
             xb = x[0] + dy * dxdyb + (off_y * dxdyb);
             _map_triangle_draw_linear(src, dst, cx, cy, cw, ch, mask, mx, my,
                                       yi[0], yi[1], tbuf, func,
                                       func2, mul_col, aa_spans,
                                       col_blend);
             upper = EINA_TRUE;
          }
        //Draw lower segment if possibly visible
        if (yi[1] < yi[2])
          {
             off_y = y[1] < cy ? (cy - y[1]) : 0;
             if (!upper)
               {
                  xa += (off_y * dxdya);
                  ua += (off_y * dudya);
                  va += (off_y * dvdya);

                  if (col_blend)
                    for (int i = 0; i < 4; i++)
                      ca[i] += (off_y * dcdya[i]);
               }

             // Set right edge X-slope and perform subpixel pre-stepping
             dxdyb = dxdy[2];
             xb = x[1] + (1 - (y[1] - yi[1])) * dxdyb + (off_y * dxdyb);
             _map_triangle_draw_linear(src, dst, cx, cy, cw, ch, mask, mx, my,
                                       yi[1], yi[2], tbuf, func, func2,
                                       mul_col, aa_spans, col_blend);
          }
     }
   //Longer edge is on the right side
   else
     {
        //Set right edge X-slope and perform subpixel pre-stepping
        dxdyb = dxdy[1];
        float dy = 1 - (y[0] - yi[0]);
        xb = x[0] + dy * dxdyb;

        //Draw upper segment if possibly visible
        if (yi[0] < yi[1])
          {
             off_y = y[0] < cy ? (cy - y[0]) : 0;
             xb += (off_y *dxdyb);

             // Set slopes along left edge and perform subpixel pre-stepping
             dxdya = dxdy[0];
             dudya = dxdya * dudx + dudy;
             dvdya = dxdya * dvdx + dvdy;

             xa = x[0] + dy * dxdya + (off_y * dxdya);
             ua = u[0] + dy * dudya + (off_y * dudya);
             va = v[0] + dy * dvdya + (off_y * dvdya);

             if (col_blend)
               for (int i = 0; i < 4; i++)
                 {
                    dcdya[i] = dxdya * dcdx[i] + dcdy[i];
                    ca[i] = mc[0][i] + dy * dcdya[i] + (off_y * dcdya[i]);
                 }
             _map_triangle_draw_linear(src, dst, cx, cy, cw, ch, mask, mx, my,
                                       yi[0], yi[1], tbuf, func, func2,
                                       mul_col, aa_spans, col_blend);
             upper = EINA_TRUE;
          }
        //Draw lower segment if possibly visible
        if (yi[1] < yi[2])
          {
             off_y = y[1] < cy ? (cy - y[1]) : 0;
             if (!upper)
               xb += (off_y *dxdyb);

             // Set slopes along left edge and perform subpixel pre-stepping
             dxdya = dxdy[2];
             dudya = dxdya * dudx + dudy;
             dvdya = dxdya * dvdx + dvdy;
             dy = 1 - (y[1] - yi[1]);
             xa = x[1] + dy * dxdya + (off_y * dxdya);
             ua = u[1] + dy * dudya + (off_y * dudya);
             va = v[1] + dy * dvdya + (off_y * dvdya);

             if (col_blend)
               for (int i = 0; i < 4; i++)
                 {
                    dcdya[i] = dxdya * dcdx[i] + dcdy[i];
                    ca[i] = mc[1][i] + dy * dcdya[i] + (off_y * dcdya[i]);
                 }
             _map_triangle_draw_linear(src, dst, cx, cy, cw, ch, mask, mx, my,
                                       yi[1], yi[2], tbuf, func, func2,
                                       mul_col, aa_spans, col_blend);
          }
     }
}

static void
_evas_common_map_rgba_internal_high(RGBA_Image *src, RGBA_Image *dst,
                                    int cx, int cy, int cw, int ch,  //clip
                                    DATA32 mul_col, int render_op,
                                    RGBA_Map_Point *p, int smooth,
                                    int anti_alias, int level EINA_UNUSED,
                                    RGBA_Image *mask, int mask_x, int mask_y)
{
   float x[4], y[4], u[4], v[4];
   DATA32 c[4];
   RGBA_Gfx_Func func = NULL;
   RGBA_Gfx_Func func2 = NULL;
   DATA32 *tbuf = NULL;                //Temporarily used span buffer 
   Eina_Bool have_alpha = EINA_FALSE;
   Eina_Bool src_alpha = src->cache_entry.flags.alpha;
   Eina_Bool ssrc_alpha = src->cache_entry.flags.alpha_sparse;
   Eina_Bool dst_alpha = dst->cache_entry.flags.alpha;
   Eina_Bool col_blend = EINA_FALSE;   //Necessary blending vertex color?

   //FIXME: we cannot apply anti_aliasing per polygons.
   anti_alias = EINA_FALSE;

   /* Prepare points data. 
      Convert to float, 
      shift XY coordinates to match the sub-pixeling technique.
      Check alpha transparency. */
   for (int i = 0; i < 4; i++)
     {
        x[i] = p[i].fx + 0.5;
        y[i] = p[i].fy + 0.5;
        u[i] = (p[i].u >> FP);
        v[i] = (p[i].v >> FP);
        c[i] = p[i].col;

        /* Exceptions:
           Limit u,v coords of points to be within the source image */
        if (u[i] < 0) u[i] = 0;
        else if (u[i] >= (float) src->cache_entry.w)
          u[i] = (float) (src->cache_entry.w - 1);

        if (v[i] < 0) v[i] = 0;
        else if (v[i] >= (float) src->cache_entry.h)
          v[i] = (float) (src->cache_entry.h - 1);

        if ((c[i] >> 24) < 0xff) have_alpha = EINA_TRUE;
        if (c[i] < 0xffffffff) col_blend = EINA_TRUE;
     }

   //Figure out alpha to choose a blend method.
   //If operation is solid, bypass buf and draw func and draw direct to dst.
   if (!(!src_alpha && !dst_alpha && (mul_col == 0xffffffff) &&
       !have_alpha && !anti_alias && !mask))
     {
        if (have_alpha) src_alpha = EINA_TRUE;
        ssrc_alpha = (anti_alias | src_alpha);
        if (!mask)
          {
             if (mul_col != 0xffffffff)
               func = evas_common_gfx_func_composite_pixel_color_span_get(ssrc_alpha, ssrc_alpha, mul_col, dst_alpha, cw, render_op);
             else
               func = evas_common_gfx_func_composite_pixel_span_get(ssrc_alpha, ssrc_alpha, dst_alpha, cw, render_op);
          }
        else
          {
             func = evas_common_gfx_func_composite_pixel_mask_span_get(ssrc_alpha, ssrc_alpha, dst_alpha, cw, render_op);
             if (mul_col != 0xffffffff)
               func2 = evas_common_gfx_func_composite_pixel_color_span_get(ssrc_alpha, ssrc_alpha, mul_col, dst_alpha, cw, EVAS_RENDER_COPY);
          }
        if (src_alpha) src->cache_entry.flags.alpha = EINA_TRUE;
        tbuf = alloca(cw * sizeof(DATA32));
     }

   //Setup Anti-Aliasing?
   AASpans *aa_spans = NULL;
   if (anti_alias)
     {
        //Adjust AA Y range
        float ystart = 9999999999, yend = -1;
        for (int i = 0; i < 4; i++)
          {
             if (y[i] < ystart) ystart = y[i];
             if (y[i] > yend) yend = y[i];
          }
        if (ystart < cy) ystart = cy;
        if (yend > cy + ch) yend = cy + ch;

        aa_spans =
           _map_aa_ready(dst->cache_entry.w, dst->cache_entry.h, ystart, yend);
     }

   /* 
      1 polygon is consisted of 2 triangles, 4 polygons constructs 1 mesh.
      below figure illustrates vert[9] index info.
      If you need better quality, please divide a mesh by more number of triangles.

       0 -- 4 -- 1
       |  / |  / |
       | /  | /  |
       6 -- 8 -- 7
       |  / |  / |
       | /  | /  |
       3 -- 5 -- 2
   */

   //Interpolated color info
   DATA32 tmpc[5] = {
      INTERP_256(128, c[0], c[1]),
      INTERP_256(128, c[3], c[2]),
      INTERP_256(128, c[0], c[3]),
      INTERP_256(128, c[1], c[2]),
      INTERP_256(128, tmpc[2], tmpc[3])};

   //9 vertices (see above figure)
   Map_Vertex vert[9] = {
      {x[0], y[0], u[0], v[0], c[0]},
      {x[1], y[1], u[1], v[1], c[1]},
      {x[2], y[2], u[2], v[2], c[2]},
      {x[3], y[3], u[3], v[3], c[3]},
      {x[0] + (x[1] - x[0]) * 0.5, y[0] + (y[1] - y[0]) * 0.5, u[0] + (u[1] - u[0]) * 0.5, v[0] + (v[1] - v[0]) * 0.5, tmpc[0]},
      {x[3] + (x[2] - x[3]) * 0.5, y[3] + (y[2] - y[3]) * 0.5, u[3] + (u[2] - u[3]) * 0.5, v[3] + (v[2] - v[3]) * 0.5, tmpc[1]},
      {x[0] + (x[3] - x[0]) * 0.5, y[0] + (y[3] - y[0]) * 0.5, u[0] + (u[3] - u[0]) * 0.5, v[0] + (v[3] - v[0]) * 0.5, tmpc[2]},
      {x[1] + (x[2] - x[1]) * 0.5, y[1] + (y[2] - y[1]) * 0.5, u[1] + (u[2] - u[1]) * 0.5, v[1] + (v[2] - v[1]) * 0.5, tmpc[3]},
      {vert[6].x + (vert[7].x - vert[6].x) * 0.5, vert[4].y + (vert[5].y - vert[4].y) * 0.5, vert[6].u + (vert[7].u - vert[6].u) * 0.5, vert[4].v + (vert[5].v - vert[4].v) * 0.5, tmpc[4]}};

   //Vertex Indices
   int idx[4][4] = {{ 0, 4, 8, 6 }, {4, 1, 7, 8}, {6, 8, 5, 3}, {8, 7, 2, 5}};

   Map_Tripoly poly;

   //Draw a pair of triangles
   for (int i = 0; i < 4; ++i)
     {
        poly.v[0] = vert[idx[i][0]];
        poly.v[1] = vert[idx[i][1]];
        poly.v[2] = vert[idx[i][3]];

        _map_triangle_draw(src, dst, cx, cy, cw, ch,
                           mask, mask_x, mask_y,
                           tbuf, func, func2,
                           &poly, mul_col, aa_spans,
                           smooth, col_blend);

        poly.v[0] = vert[idx[i][1]];
        poly.v[1] = vert[idx[i][3]];
        poly.v[2] = vert[idx[i][2]];

        _map_triangle_draw(src, dst, cx, cy, cw, ch,
                           mask, mask_x, mask_y,
                           tbuf, func, func2,
                           &poly, mul_col, aa_spans,
                           smooth, col_blend);
     }

   if (anti_alias)
     _map_aa_apply(aa_spans, dst->image.data, dst->cache_entry.w);
}
