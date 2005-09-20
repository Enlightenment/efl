#ifndef EVAS_MACROS_H
#define EVAS_MACROS_H

#undef ABS
#define ABS(x) (((x) < 0) ? -(x) : (x))

#undef SGN
#define SGN(x) (((x) < 0) ? -1 : 1)

#undef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#undef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define SWAP32(x) (x) = \
   ((((x) & 0x000000ff ) << 24) | \
    (((x) & 0x0000ff00 ) << 8)  | \
    (((x) & 0x00ff0000 ) >> 8)  | \
    (((x) & 0xff000000 ) >> 24))

#define SWAP16(x) (x) = \
   ((((x) & 0x00ff ) << 8) | \
    (((x) & 0xff00 ) >> 8))


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

#define INTERP_VAL(out, in1, in2, in3, in4, interp_x, interp_y)    \
   {                                                               \
      int _v, _vv;                                                 \
                                                                   \
      _v = (256 - (interp_x)) * (in1);                             \
      if ((interp_x) > 0) _v += (interp_x) * (in2);                \
      _v *= (256 - (interp_y));                                    \
      if ((interp_y) > 0)                                          \
	{                                                          \
	   _vv = (256 - (interp_x)) * (in3);                       \
	   if ((interp_x) > 0) _vv += (interp_x) * (in4);          \
	   _vv *= (interp_y);                                      \
	   (out) = ((_v + _vv) >> 16);                             \
	}                                                          \
      else (out) = (_v >> 16);                                     \
   }

#define INTERP_2(in1, in2, interp, interp_inv) \
   ((in1 * interp_inv) + (in2 * interp)) >> 8

#endif
