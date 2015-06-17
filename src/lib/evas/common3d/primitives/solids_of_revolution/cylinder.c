#include "../primitive_common.h"

void
_set_default_cylinder(Evas_Canvas3D_Mesh *mesh,
                      int frame,
                      int p,
                      Evas_Vec2 tex_scale)
{
   int vcount, icount, vccount, i, j, num;
   icount = p * 30;
   vccount = p + 1;
   vcount = 6 * vccount;

   ALLOCATE_VERTEX_DATA

   double dfi, fi, sinfi, cosfi;
   dfi = 2 * M_PI / p;

   for (i = 0; i < vccount; i++)
     {
        fi = i * dfi;
        sinfi = sin(fi);
        cosfi = cos(fi);

        for (j = 0; j < 6; j++)
          {
             num = i + j * vccount;

             vertices[num].z = -0.5 + (float)(j / 3);
             if ((j == 0) || (j == 5))
               {
                  vertices[num].x = vertices[num].y = 0.0;
               }
             else
               {
                  vertices[num].x = sinfi / 2.0;
                  vertices[num].y = cosfi / 2.0;
               }

             if ((j == 2) || (j == 3))
               {
                  normals[num].x = sinfi;
                  normals[num].y = cosfi;
                  normals[num].z = 0.0;
               }
             else
               {
                  normals[num].x = normals[num].y = 0.0;
                  normals[num].z = -1.0 + (float)(j / 2);
               }

             tangents[num].x = cosfi;
             tangents[num].y = -sinfi;
             tangents[num].z = 0.0;

             tex_coord[num].x = i / (float)(vccount - 1) * tex_scale.x;
             tex_coord[num].y = (float)((j + 1) / 2) * tex_scale.y / 3.0;
          }
     }

   _generate_indices(indices, p, 5);

   SET_VERTEX_DATA(frame)
}

void
_set_cylinder_without_bases(Evas_Canvas3D_Mesh *mesh,
                            int frame,
                            int p,
                            Evas_Vec2 tex_scale)
{
   int vcount, icount, vccount, i;
   icount = p * 6;
   vccount = p + 1;
   vcount = 2 * vccount;

   ALLOCATE_VERTEX_DATA

   double dfi, fi, sinfi, cosfi;
   dfi = 2 * M_PI / p;

   for (i = 0; i < vccount; i++)
     {
        fi = i * dfi;
        sinfi = sin(fi);
        cosfi = cos(fi);
        vertices[i + vccount].x = vertices[i].x = sinfi / 2.0;
        vertices[i + vccount].y = vertices[i].y = cosfi / 2.0;
        vertices[i].z = -0.5;
        vertices[i + vccount].z = 0.5;

        normals[i + vccount].x = normals[i].x = sinfi;
        normals[i + vccount].y = normals[i].y = cosfi;
        normals[i + vccount].z = normals[i].z = 0;

        tangents[i + vccount].x = tangents[i].x = cosfi;
        tangents[i + vccount].y = tangents[i].y = -sinfi;
        tangents[i + vccount].z = tangents[i].z = 0;

        tex_coord[i].x = i / (float)(vccount - 1) * tex_scale.x;
        tex_coord[i].y = 0;
        tex_coord[i + vccount].x = i / (float)(vccount - 1) * tex_scale.x;
        tex_coord[i + vccount].y = tex_scale.y;
     }

   _generate_indices(indices, p, 1);

   SET_VERTEX_DATA(frame)
}

void
evas_model_set_from_cylinder_primitive(Evas_Canvas3D_Mesh *mesh,
                                       int frame,
                                       Evas_Canvas3D_Primitive_Mode mode,
                                       int p,
                                       Evas_Vec2 tex_scale)
{
   switch (mode)
     {
      case EVAS_CANVAS3D_PRIMITIVE_MODE_DEFAULT:
      case EVAS_CANVAS3D_PRIMITIVE_MODE_ALTERNATIVE_UV:
        {
           _set_default_cylinder(mesh, frame, p, tex_scale);
           break;
        }
      case EVAS_CANVAS3D_PRIMITIVE_MODE_WITHOUT_BASE:
        {
           _set_cylinder_without_bases(mesh, frame, p, tex_scale);
           break;
        }
      default:
        {
           ERR("Unknown mode of primitive");
        }
     }
}
