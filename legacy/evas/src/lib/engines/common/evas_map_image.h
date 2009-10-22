/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EVAS_MAP_H
#define _EVAS_MAP_H

typedef struct _RGBA_Map_Point RGBA_Map_Point;

// all coords are 20.12
// fp type - an int for now
typedef int FPc;
// fp # of bits of float accuracy
#define FP 8
// fp half (half of an fp unit)
#define FPH (1 << (FP - 1))
// one fp unit
#define FP1 (1 << (FP))

struct _RGBA_Map_Point
{
   FPc x, y; // x, y screenspace
   FPc z; // z in world space. optional
   FPc u, v; // u, v in tex coords
};

EAPI void
evas_common_map4_rgba(RGBA_Image *src, RGBA_Image *dst,
                      RGBA_Draw_Context *dc,
                      RGBA_Map_Point *points,
                      int smooth, int level);

#endif /* _EVAS_MAP_H */
