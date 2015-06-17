#include "../primitive_common.h"

static Evas_Real
_random(int x, int y)
{
   int k = x + y * 57;
   k = (k << 13) ^ k;
   return (1.0f - ((k * (k * k * 15731 + 789221) + 1376312589) & 0x7fffffff) /
      1073741824.0f);
}

static Evas_Real
_smooth(Evas_Real x, Evas_Real y)
{
   Evas_Real res;
   res = (_random(x - 1, y - 1) + _random(x + 1, y - 1) +
        _random(x - 1, y + 1) + _random(x + 1, y + 1)) / 16;
   res += (_random(x - 1, y) + _random(x + 1, y) +
        _random(x, y - 1) + _random(x, y + 1)) / 8;
   res += _random(x, y) / 4;
   return res;
}

static Evas_Real
_interpolate(Evas_Real a, Evas_Real b, Evas_Real x)
{
   Evas_Real ft = x * M_PI;
   Evas_Real f = (1 - cosf(ft)) * 0.5;
   return a * (1 - f) + b * f;
}

static Evas_Real _noise(Evas_Real x, Evas_Real y)
{
   Evas_Real ix = (int)(x);
   Evas_Real fx = x - ix;
   Evas_Real iy = (int)(y);
   Evas_Real fy = y - iy;

   Evas_Real v1 = _smooth(ix, iy);
   Evas_Real v2 = _smooth(ix + 1, iy);
   Evas_Real v3 = _smooth(ix, iy + 1);
   Evas_Real v4 = _smooth(ix + 1, iy + 1);

   Evas_Real i1 = _interpolate(v1, v2, fx);
   Evas_Real i2 = _interpolate(v3, v4, fx);

   return _interpolate(i1, i2, fy);
}

static void
_perlin_terrain(Evas_Real *out_x,
                Evas_Real *out_y,
                Evas_Real *out_z,
                Evas_Real x,
                Evas_Real y)
{
   Evas_Real persistence = 0.5f;
   Evas_Real frequency = 5;
   Evas_Real amplitude = 1;
   int i = 0;
   int octaves = 5;

   *out_x = x;
   x += 0.5;
   *out_y = y;
   y += 0.5;
   *out_z = 0;

   for(i = 0; i < octaves; i++)
     {
        *out_z += _noise(x * frequency, y * frequency) * amplitude;

        amplitude *= persistence;
        frequency *= 2;
     }
}

void
evas_model_set_from_terrain_primitive(Evas_Canvas3D_Mesh *mesh,
                                      int frame,
                                      int p,
                                      Evas_Vec2 tex_scale)
{
   evas_model_set_from_surface_primitive(mesh, frame, _perlin_terrain, p, tex_scale);
}

