#include "../primitive_common.h"

void
_set_default_sphere(Evas_3D_Mesh *mesh,
                   int frame,
                   int p,
                   Evas_Vec2 tex_scale)
{
   int vcount, icount, vccount, i, j;
   icount = p * p * 6;
   vccount = p + 1;
   vcount = vccount * vccount;

   ALLOCATE_VERTEX_DATA

   double dtheta, dfi, sinth, costh, fi, theta, sinfi, cosfi;
   dtheta = M_PI / p;
   dfi = 2 * M_PI / p;

   for (j = 0; j < vccount; j++)
     {
        theta = j * dtheta;
        sinth = sin(theta);
        costh = cos(theta);
        for (i = 0; i < vccount; i++)
          {
             fi = i * dfi;
             sinfi = sin(fi);
             cosfi = cos(fi);
             normals[i + j * vccount].x = sinth * sinfi;
             normals[i + j * vccount].y = sinth * cosfi;
             normals[i + j * vccount].z = costh;

             vertices[i + j * vccount].x = normals[i + j * vccount].x / 2;
             vertices[i + j * vccount].y = normals[i + j * vccount].y / 2;
             vertices[i + j * vccount].z = normals[i + j * vccount].z / 2;

             tangents[i + j * vccount].x = -sinth * cosfi;
             tangents[i + j * vccount].y = sinth * sinfi;
             tangents[i + j * vccount].z = 0;

             _primitives_vec3_normalize(&tangents[i + j * vccount]);

             tex_coord[i + j * vccount].x = i / (float)(vccount - 1) * tex_scale.x;
             tex_coord[i + j *vccount].y = tex_scale.y - j / (float)(vccount - 1) * tex_scale.y;
          }
     }

   _generate_indices(indices, p, p);

   SET_VERTEX_DATA(frame)
}

void
_set_sphere_with_alternative_uv(Evas_3D_Mesh *mesh,
                                int frame,
                                int p,
                                Evas_Vec2 tex_scale)
{
   int vcount, icount, vccount, i, j;

   vccount = p + 1;
   vcount = vccount * vccount;
   icount = p * p * 6;

   ALLOCATE_VERTEX_DATA

   /* Calculate vertices position of the sphere mesh by using
      splitting of sphere by latitude and longitude. */
   for (i = 0; i <= p; i++)
     {
        double lati, z, r, point_r;

        point_r = 0.00001;//non-zero little value for correct tangents calculation.

        lati = ((M_PI - 2 * point_r) * (double)i) / (double)p;
        z = cos(lati + point_r);
        r = fabs(sin(lati + point_r));

        for (j = 0; j <= p; j++)
          {
             double longi;
             int num = (i * (p + 1)) + j;

             longi = (M_PI * 2.0 * (double)j) / (double)p;

             normals[num].x = r * sin(longi);
             normals[num].y = r * cos(longi);
             normals[num].z = z;

             vertices[num].x = normals[num].x / 2;
             vertices[num].y = normals[num].y / 2;
             vertices[num].z = normals[num].z / 2;

             if (vertices[num].x > 0.0)
               {
                  tangents[num].x = -normals[num].z;
                  tangents[num].y = normals[num].y;
                  tangents[num].z = normals[num].x;
               }
             else
               {
                  tangents[num].x = normals[num].z;
                  tangents[num].y = normals[num].y;
                  tangents[num].z = -normals[num].x;
               }

             tex_coord[num].x = i / (float)(vccount - 1) * tex_scale.x;
             tex_coord[num].y = tex_scale.y - j / (float)(vccount - 1) * tex_scale.y;
          }
     }

   _generate_indices(indices, p, p);

   /* Triangulation of sphere mesh in appliance with buffer of indices. */
   for (i = 0; i < icount; i += 3)
     {
        Evas_Vec3 e1, e2;
        float du1, du2, dv1, dv2, f;
        Evas_Vec3 tangent;
        int num0, num1, num2;

        num0 = indices[i + 0];
        num1 = indices[i + 1];
        num2 = indices[i + 2];

        e1.x = vertices[num1].x - vertices[num0].x;
        e1.y = vertices[num1].y - vertices[num0].y;
        e1.z = vertices[num1].z - vertices[num0].z;

        e2.x = vertices[num2].x - vertices[num0].x;
        e2.y = vertices[num2].y - vertices[num0].y;
        e2.z = vertices[num2].z - vertices[num0].z;

        du1 = tex_coord[num1].x - tex_coord[num0].x;
        dv1 = tex_coord[num1].y - tex_coord[num0].y;

        du2 = tex_coord[num2].x - tex_coord[num0].x;
        dv2 = tex_coord[num2].y - tex_coord[num0].y;

        f = 1.0 / ((du1 * dv2) - (du2 * dv1));

        tangent.x = f * ((dv2 * e1.x) - (dv1 * e2.x));
        tangent.y = f * ((dv2 * e1.y) - (dv1 * e2.y));
        tangent.z = f * ((dv2 * e1.z) - (dv1 * e2.z));

        tangents[num0] = tangent;
     }

   /* Coupling between vertices by calculation of tangent parametr correct value. */
   for (i = 0; i <= p; i++)
     {
        for (j = 0; j <= p; j++)
          {
             if (j == p)
               {
                  tangents[(i * (p  + 1)) + j] = tangents[i * (p + 1)];
               }
          }
     }
   SET_VERTEX_DATA(frame)
}

void
evas_model_set_from_sphere_primitive(Evas_3D_Mesh *mesh,
                                     int frame,
                                     Evas_3D_Primitive_Mode mode,
                                     int p,
                                     Evas_Vec2 tex_scale)
{
   switch (mode)
     {
      case EVAS_3D_PRIMITIVE_MODE_DEFAULT:
      case EVAS_3D_PRIMITIVE_MODE_WITHOUT_BASE:
        {
           _set_default_sphere(mesh, frame, p, tex_scale);
           break;
        }
      case EVAS_3D_PRIMITIVE_MODE_ALTERNATIVE_UV:
        {
           _set_sphere_with_alternative_uv(mesh, frame, p, tex_scale);
           break;
        }
      default:
        {
           ERR("Unknown mode of primitive");
        }
     }
}
