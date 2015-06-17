#include "../primitive_common.h"

Evas_Vec3 _get_func_normal(Evas_Canvas3D_Surface_Func *func, Evas_Real x, Evas_Real y)
{
   Evas_Vec3 v00, v01, v10, d1, d2, normal;

   func(&v00.x, &v00.y, &v00.z, x, y);
   func(&v01.x, &v01.y, &v01.z, x, y + 0.01);
   func(&v10.x, &v10.y, &v10.z, x + 0.01, y);
   _primitives_vec3_subtract(&d1, &v00, &v01);
   _primitives_vec3_subtract(&d2, &v01, &v10);

   _primitives_vec3_cross_product(&normal, &d1, &d2);

   _primitives_vec3_normalize(&normal);

   return normal;
}

void
_normalize(Evas_Vec3 *vertices, Evas_Vec3 *normals, int vcount)
{
   int i;
   Evas_Vec3 min, max;
   min = max = vertices[0];

#define CHECK_MIN_AND_MAX(coord)                \
        if (min.coord > vertices[i].coord)      \
          min.coord = vertices[i].coord;        \
        else if (max.coord < vertices[i].coord) \
          max.coord = vertices[i].coord;
   for (i = 1; i < vcount; i++)
     {
        CHECK_MIN_AND_MAX(x)
        CHECK_MIN_AND_MAX(y)
        CHECK_MIN_AND_MAX(z)
     }
#undef CHECK_MIN_AND_MAX

   for (i = 0; i < vcount; i++)
     {
        vertices[i].x = (vertices[i].x - min.x) / (max.x - min.x) - 0.5;
        vertices[i].y = (vertices[i].y - min.y) / (max.y - min.y) - 0.5;
        vertices[i].z = (vertices[i].z - min.z) / (max.z - min.z) - 0.5;

        normals[i].x = normals[i].x / (max.x - min.x);
        normals[i].y = normals[i].y / (max.y - min.y);
        normals[i].z = normals[i].z / (max.z - min.z);
     }
}

void
evas_model_set_from_surface_primitive(Evas_Canvas3D_Mesh *mesh,
                                      int frame,
                                      Evas_Canvas3D_Surface_Func func,
                                      int p,
                                      Evas_Vec2 tex_scale)
{
   int vcount, icount, vccount, i, j, num;
   icount = p * p * 6;
   vccount = p + 1;
   vcount = vccount * vccount;

   ALLOCATE_VERTEX_DATA

   Evas_Real v, u, d = 1.0 / p;

   for (j = 0; j < vccount; j++)
     {
        u = j * d - 0.5;
        for (i = 0; i < vccount; i++)
          {
             v = i * d - 0.5;
             num = i + j * vccount;
             func(&vertices[num].x,
                  &vertices[num].y,
                  &vertices[num].z,
                  v, u);
             normals[num] = _get_func_normal(func, v, u);

             tangents[num].x = tangents[num].y = tangents[num].z = 0;

             tex_coord[num].x = i / ((vccount - 1) * tex_scale.x);
             tex_coord[num].y = tex_scale.y - j / ((vccount - 1) * tex_scale.y);
          }
     }

   _normalize(vertices, normals, vcount);
   _generate_indices(indices, p, p);
   SET_VERTEX_DATA(frame)
}
