#include <Eo.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <math.h>

#define SET_VERTEX_DATA eo_do(mesh, evas_3d_mesh_vertex_count_set(vcount),\
         evas_3d_mesh_frame_add(0);\
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_POSITION,\
                                       sizeof(vec3), &vertices[0]);\
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_NORMAL,\
                                       sizeof(vec3), &normals[0]));\
   free(vertices);\
   free(normals);

typedef struct _vec3
{
    float   x;
    float   y;
    float   z;
} vec3;

static const float cube_vertices[] =
{
   /* Front */
   -1.0,  1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -1.0,  1.0,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  0.0,

   /* Back */
    1.0,  1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  1.0,
   -1.0,  1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -1.0, -1.0,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Left */
   -1.0,  1.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  1.0,
   -1.0,  1.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  1.0,
   -1.0, -1.0, -1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  0.0,
   -1.0, -1.0,  1.0,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Right */
    1.0,  1.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  1.0,
    1.0,  1.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  1.0,
    1.0, -1.0,  1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  0.0,
    1.0, -1.0, -1.0,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  0.0,

   /* Top */
   -1.0,  1.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  1.0,
    1.0,  1.0, -1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  1.0,
   -1.0,  1.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  0.0,
    1.0,  1.0,  1.0,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  0.0,

   /* Bottom */
    1.0, -1.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  1.0,
   -1.0, -1.0, -1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  1.0,
    1.0, -1.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  0.0,
   -1.0, -1.0,  1.0,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  0.0,
};

static const unsigned short cube_indices[] =
{
   /* Front */
   0,   1,  2,  2,  1,  3,

   /* Back */
   4,   5,  6,  6,  5,  7,

   /* Left */
   8,   9, 10, 10,  9, 11,

   /* Right */
   12, 13, 14, 14, 13, 15,

   /* Top */
   16, 17, 18, 18, 17, 19,

   /* Bottom */
   20, 21, 22, 22, 21, 23
};

static const float square_vertices[] =
{
   1.0, -1.0, 0.0,     0.0,  1.0,  0.0,     0.0,  1.0,
   -1.0, -1.0, 0.0,     0.0,  1.0,  0.0,     1.0,  1.0,
   1.0,  1.0, 0.0,     0.0,  1.0,  0.0,     0.0,  0.0,
   -1.0,  1.0, 0.0,     0.0,  1.0,  0.0,     1.0,  0.0,
};

static const unsigned short square_indices[] = {0, 1, 2, 2, 1, 3};

static void
_set_grid_indices(Eo *mesh, int count)
{
   int i, j;
   unsigned short *index, *indices;
   int vccount = count + 1;
   int icount = count * count * 6;

   indices = malloc(sizeof(short) * icount);
   index = indices;

   for(j = 0; j < count; j++)
     for(i = 0; i < count; i++)
       {
          *index++ = i + vccount * j;
          *index++ = i + vccount * (j + 1);
          *index++ = i + 1 + vccount * (j + 1);

          *index++ =  i + vccount * j;
          *index++ =  i + 1 +  vccount * j;
          *index++ =  i + vccount * (j + 1) + 1;
       }
   eo_do(mesh,
         evas_3d_mesh_index_data_copy_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                icount , &indices[0]));
   free(indices);
}

static void
_set_tape_indices(Eo *mesh, int count)
{
   unsigned short *index, *indices;
   int i, vccount = count + 1;
   int icount = count * 6;
   indices = malloc(sizeof(short) * icount);
   index = indices;

   for(i = 0; i < count; i++)
     {
        *index++ = i;
        *index++ = i + 1;
        *index++ = i + 1 + vccount;

        *index++ =  i;
        *index++ =  i + vccount;
        *index++ =  i + vccount + 1;
     }
   eo_do(mesh,
         evas_3d_mesh_index_data_copy_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                icount , &indices[0]));
   free(indices);
}

static void
_set_ball(Eo *mesh, int p)
{
   int vcount, vccount, i, j;
   double dtheta, dfi, sinth, costh, fi, theta, sinfi, cosfi;

   vccount = p + 1;
   vcount = vccount * vccount;

   dtheta = M_PI / p;
   dfi = 2 * M_PI / p;

   vec3 *vertices = malloc(sizeof(vec3) * vcount);
   vec3 *normals = malloc(sizeof(vec3) * vcount);

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
             vertices[i + j * vccount].x = sinth * cosfi;
             vertices[i + j * vccount].y = sinth * sinfi;
             vertices[i + j * vccount].z = costh;

             normals[i + j * vccount].x = vertices[i + j * vccount].x;
             normals[i + j * vccount].y = vertices[i + j * vccount].y;
             normals[i + j * vccount].z = vertices[i + j * vccount].z;
          }
     }

   _set_grid_indices(mesh, p);

   SET_VERTEX_DATA
}

static void
_set_cylinder(Eo *mesh, int p)
{
   int vcount, vccount, i;
   double dfi, fi, sinfi, cosfi;

   vccount = p + 1;
   vcount = 2 * vccount;

   dfi = 2 * M_PI / p;

   vec3 *vertices = malloc(sizeof(vec3) * vcount);
   vec3 *normals = malloc(sizeof(vec3) * vcount);

   for (i = 0; i < vccount; i++)
     {
        fi = i * dfi;
        sinfi = sin(fi);
        cosfi = cos(fi);
        vertices[i + vccount].x = vertices[i].x = cosfi;
        vertices[i].y = -0.5;
        vertices[i + vccount].z = vertices[i].z = sinfi;
        vertices[i + vccount].y = 0.5;

        normals[i + vccount].x = normals[i].x = cosfi;
        normals[i + vccount].y = normals[i].y = 0;
        normals[i + vccount].z = normals[i].z = sinfi;
     }

   _set_tape_indices(mesh, p);

   SET_VERTEX_DATA
}

static void
_set_cone(Eo *mesh, int p)
{
   int vcount, vccount, i;
   double dfi, fi, sinfi, cosfi;

   vccount = p + 1;
   vcount = 2 * vccount;

   dfi = 2 * M_PI / p;

   vec3 *vertices = malloc(sizeof(vec3) * vcount);
   vec3 *normals = malloc(sizeof(vec3) * vcount);

   for (i = 0; i < vccount; i++)
     {
        fi = i * dfi;
        sinfi = sin(fi);
        cosfi = cos(fi);
        vertices[i].x = cosfi;
        vertices[i].y = 0;
        vertices[i].z = sinfi;
        vertices[i + vccount].x = 0;
        vertices[i + vccount].y = 1;
        vertices[i + vccount].z = 0;

        normals[i + vccount].x = normals[i].x = cosfi * 0.71;
        normals[i + vccount].y = normals[i].y = 0.71;
        normals[i + vccount].z = normals[i].z = sinfi * 0.71;
     }

   _set_tape_indices(mesh, p);

   SET_VERTEX_DATA
}

static void
_set_square(Eo *mesh)
{
   eo_do(mesh, evas_3d_mesh_vertex_count_set(4),
         evas_3d_mesh_frame_add(0);
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_POSITION,
                                       8 * sizeof(float), &square_vertices[0]);
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_NORMAL,
                                       8 * sizeof(float), &square_vertices[3]),
         evas_3d_mesh_frame_vertex_data_copy_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            8 * sizeof(float), &square_vertices[6]);
         evas_3d_mesh_index_data_copy_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                6 , &square_indices[0]);
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES);
         evas_3d_mesh_shade_mode_set(EVAS_3D_SHADE_MODE_PHONG));
}

static void
_set_cube(Eo *mesh)
{
   eo_do(mesh,
         evas_3d_mesh_vertex_count_set(24),
         evas_3d_mesh_frame_add(0),

         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_POSITION,
                                            12 * sizeof(float), &cube_vertices[0]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_NORMAL,
                                            12 * sizeof(float), &cube_vertices[3]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_COLOR,
                                            12 * sizeof(float), &cube_vertices[6]),
         evas_3d_mesh_frame_vertex_data_set(0, EVAS_3D_VERTEX_TEXCOORD,
                                            12 * sizeof(float), &cube_vertices[10]),

         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT,
                                     36, &cube_indices[0]),
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES));
}
