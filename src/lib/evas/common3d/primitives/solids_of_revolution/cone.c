#include "../primitive_common.h"

void
_set_default_cone(Evas_3D_Mesh *mesh,
                  int frame,
                  int p,
                  Evas_Vec2 tex_scale)
{
   int vcount, icount, vccount, i, j, num;
   double dfi, fi, sinfi, cosfi, nplane, nz;

   icount = p * 18;
   vccount = p + 1;
   vcount = 4 * vccount;

   dfi = 2.0 * M_PI / p;
   nz = sqrt(1.0 / 3.0);
   nplane = sqrt(2.0 / 3.0);

   ALLOCATE_VERTEX_DATA

   for (i = 0; i < vccount; i++)
     {
        fi = i * dfi;
        sinfi = sin(fi);
        cosfi = cos(fi);

        vertices[i].x = 0.0;
        vertices[i].y = 0.0;
        vertices[i].z = -0.5;
        vertices[i + vccount].x = sinfi / 2.0;
        vertices[i + vccount].y = cosfi / 2.0;
        vertices[i + vccount].z = -0.5;
        vertices[i + 2 * vccount] = vertices[i + vccount];
        vertices[i + 3 * vccount].x = 0.0;
        vertices[i + 3 * vccount].y = 0.0;
        vertices[i + 3 * vccount].z = 0.5;

        normals[i].x = normals[i].y = 0.0;
        normals[i].z = -1.0;
        normals[i + vccount] = normals[i];
        normals[i + 2 * vccount].x = sinfi * nplane;
        normals[i + 2 * vccount].y = cosfi * nplane;
        normals[i + 2 * vccount].z = nz;
        normals[i + 3 * vccount] = normals[i + 2 * vccount];

        for (j = 0; j < 4; j++)
          {
             num = i + j * vccount;

             tangents[num].x = cosfi;
             tangents[num].y = -sinfi;
             tangents[num].z = 0.0;

             tex_coord[num].x = i / (float)(vccount - 1) * tex_scale.x;
             tex_coord[num].y = (float)((j + 1) / 2) * tex_scale.y / 2.0;
          }
     }

   _generate_indices(indices, p, 3);

   SET_VERTEX_DATA(frame)
}

void
_set_cone_without_base(Evas_3D_Mesh *mesh,
                       int frame,
                       int p,
                       Evas_Vec2 tex_scale)
{
   int vcount, icount, vccount, i;
   double dfi, fi, sinfi, cosfi, nplane, nz;

   icount = p * 6;
   vccount = p + 1;
   vcount = 2 * vccount;

   dfi = 2.0 * M_PI / p;
   nz = sqrt(1.0 / 3.0);
   nplane = sqrt(2.0 / 3.0);

   ALLOCATE_VERTEX_DATA

   for (i = 0; i < vccount; i++)
     {
        fi = i * dfi;
        sinfi = sin(fi);
        cosfi = cos(fi);
        vertices[i].x = sinfi / 2.0;
        vertices[i].y = cosfi / 2.0;
        vertices[i].z = -0.5;
        vertices[i + vccount].x = 0;
        vertices[i + vccount].y = 0;
        vertices[i + vccount].z = 0.5;

        normals[i + vccount].x = normals[i].x = sinfi * nplane;
        normals[i + vccount].y = normals[i].y = cosfi * nplane;
        normals[i + vccount].z = normals[i].z = nz;

        tangents[i + vccount].x = tangents[i].x = cosfi;
        tangents[i + vccount].y = tangents[i].y = -sinfi;
        tangents[i + vccount].z = tangents[i].z = 0;

        tex_coord[i].x = i / (float)(vccount - 1) * tex_scale.x;
        tex_coord[i].y = 0;
        tex_coord[i + vccount].x = tex_coord[i].x;
        tex_coord[i + vccount].y = tex_scale.y;
     }

   _generate_indices(indices, p, 1);

   SET_VERTEX_DATA(frame)
}

void
evas_model_set_from_cone_primitive(Evas_3D_Mesh *mesh,
                                   int frame,
                                   Evas_3D_Primitive_Mode mode,
                                   int p,
                                   Evas_Vec2 tex_scale)
{
   switch (mode)
     {
      case EVAS_3D_PRIMITIVE_MODE_DEFAULT:
      case EVAS_3D_PRIMITIVE_MODE_ALTERNATIVE_UV:
        {
           _set_default_cone(mesh, frame, p, tex_scale);
           break;
        }
      case EVAS_3D_PRIMITIVE_MODE_WITHOUT_BASE:
        {
           _set_cone_without_base(mesh, frame, p, tex_scale);
           break;
        }
      default:
        {
           ERR("Unknown mode of primitive");
        }
     }
}
