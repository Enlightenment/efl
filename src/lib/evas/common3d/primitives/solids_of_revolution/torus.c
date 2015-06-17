#include "../primitive_common.h"

void
evas_model_set_from_torus_primitive(Evas_Canvas3D_Mesh *mesh,
                                    int frame,
                                    Evas_Real ratio,
                                    int p,
                                    Evas_Vec2 tex_scale)
{
   int vcount, icount, vccount, i, j;
   icount = p * p * 6;
   vccount = p + 1;
   vcount = vccount * vccount;

   ALLOCATE_VERTEX_DATA

   double d, sinth, costh, fi, theta, sinfi, cosfi;

   d = 2 * M_PI / p;

   float rratio;

   if ((ratio < 1.0))
     {
        printf("Ratio of torus should be greater than or equal 1.0.\n");
        printf("Ratio = %f is a bad value, so 3.0 is used like default ratio.\n",
                ratio);
        rratio = 1.0 / 3.0;
     }
   else
     {
        rratio = 1.0 / ratio;
     }

   for (j = 0; j < vccount; j++)
     {
        theta = j * d;
        sinth = sin(theta);
        costh = cos(theta);
        for (i = 0; i < vccount; i++)
          {
             fi = i * d;
             sinfi = sin(fi);
             cosfi = cos(fi);
             vertices[i + j * vccount].x = (1.0 - rratio + rratio * cosfi) * costh * 0.5;
             vertices[i + j * vccount].y = (1.0 - rratio + rratio * cosfi) * sinth * 0.5;
             vertices[i + j * vccount].z = rratio * sinfi * 0.5;

             normals[i + j * vccount].x = cosfi * costh;
             normals[i + j * vccount].y = cosfi * sinth;
             normals[i + j * vccount].z = sinfi;

             tangents[i + j * vccount].x = -sinfi * costh;
             tangents[i + j * vccount].y = -sinfi * sinth;
             tangents[i + j * vccount].z = cosfi;

             _primitives_vec3_normalize(&normals[i + j * vccount]);

             tex_coord[i + j * vccount].x = i / (float)(vccount - 1) * tex_scale.x;
             tex_coord[i + j *vccount].y = tex_scale.y - j / (float)(vccount - 1) * tex_scale.y;
          }
     }

   _generate_indices(indices, p, p);

   SET_VERTEX_DATA(frame)
}
