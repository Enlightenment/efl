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


#define SPANS_COMMON(x1, w1, x2, w2) \
(!((((x2) + (w2)) <= (x1)) || ((x2) >= ((x1) + (w1)))))

#define RECTS_INTERSECT(x, y, w, h, xx, yy, ww, hh) \
((SPANS_COMMON((x), (w), (xx), (ww))) && (SPANS_COMMON((y), (h), (yy), (hh))))

#define RECTS_CLIP_TO_RECT(_x, _y, _w, _h, _cx, _cy, _cw, _ch) \
{ \
   if (RECTS_INTERSECT(_x, _y, _w, _h, _cx, _cy, _cw, _ch)) \
     { \
	if (_x < (_cx)) \
	  { \
	     _w += _x - (_cx); \
	     _x = (_cx); \
	     if (_w < 0) _w = 0; \
	  } \
	if ((_x + _w) > ((_cx) + (_cw))) \
	  _w = (_cx) + (_cw) - _x; \
	if (_y < (_cy)) \
	  { \
	     _h += _y - (_cy); \
	     _y = (_cy); \
	     if (_h < 0) _h = 0; \
	  } \
	if ((_y + _h) > ((_cy) + (_ch))) \
	  _h = (_cy) + (_ch) - _y; \
     } \
   else \
     { \
	_w = 0; _h = 0; \
     } \
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


#define CONVERT_LOOP_START_ROT_0() \
   src_ptr = src; \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {

#define CONVERT_LOOP_END_ROT_0() \
             dst_ptr++; \
             src_ptr++; \
          } \
        src_ptr += src_jump; \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP_START_ROT_180() \
   src_ptr = src + (w - 1) + ((h - 1) * (w + src_jump)); \
   for (y = 0; y < h; y++) \
     { \
        for (x = 0; x < w; x++) \
          {

#define CONVERT_LOOP_END_ROT_180() \
             dst_ptr++; \
             src_ptr--; \
          } \
        src_ptr = src + (w - 1) + ((h - y - 2) * (w + src_jump)); \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP_START_ROT_270() \
   src_ptr = src + ((w - 1) * (h + src_jump)); \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {

#define CONVERT_LOOP_END_ROT_270() \
             dst_ptr++; \
             src_ptr -= (h + src_jump); \
          } \
        src_ptr = src + ((w - 1) * (h + src_jump)) + (y + 1); \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP_START_ROT_90() \
   src_ptr = src + (h - 1); \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {

#define CONVERT_LOOP_END_ROT_90() \
             dst_ptr++; \
             src_ptr += (h + src_jump); \
          } \
        src_ptr = src + (h - 1) - y - 1; \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP2_START_ROT_0() \
   src_ptr = src; \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {

#define CONVERT_LOOP2_INC_ROT_0() \
src_ptr++; \
x++;

#define CONVERT_LOOP2_END_ROT_0() \
             dst_ptr+=2; \
             src_ptr++; \
          } \
        src_ptr += src_jump; \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP2_START_ROT_180() \
   src_ptr = src + (w - 1) + ((h - 1) * (w + src_jump)); \
   for (y = 0; y < h; y++) \
     { \
        for (x = 0; x < w; x++) \
          {

#define CONVERT_LOOP2_INC_ROT_180() \
src_ptr--; \
x++;

#define CONVERT_LOOP2_END_ROT_180() \
             dst_ptr+=2; \
             src_ptr--; \
          } \
        src_ptr = src + (w - 1) + ((h - y - 2) * (w + src_jump)); \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP2_START_ROT_270() \
   src_ptr = src + ((w - 1) * (h + src_jump)); \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {

#define CONVERT_LOOP2_INC_ROT_270() \
src_ptr -= (h + src_jump); \
x++;

#define CONVERT_LOOP2_END_ROT_270() \
             dst_ptr+=2; \
             src_ptr -= (h + src_jump); \
          } \
        src_ptr = src + ((w - 1) * (h + src_jump)) + (y + 1); \
        dst_ptr += dst_jump; \
     }

#define CONVERT_LOOP2_START_ROT_90() \
   src_ptr = src + (h - 1); \
   for (y = 0; y < h; y++) \
     { \
	for (x = 0; x < w; x++) \
	  {

#define CONVERT_LOOP2_INC_ROT_90() \
src_ptr += (h + src_jump); \
x++;

#define CONVERT_LOOP2_END_ROT_90() \
             dst_ptr+=2; \
             src_ptr += (h + src_jump); \
          } \
        src_ptr = src + (h - 1) - y - 1; \
        dst_ptr += dst_jump; \
     }

#endif
