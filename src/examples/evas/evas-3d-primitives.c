#define EFL_EO_API_SUPPORT
#define EFL_BETA_API_SUPPORT

#include "evas-3d-primitives.h"

// TODO Use an external library of linear algebra.
inline void
_vec3_subtract(vec3 *out, const vec3 *a, const vec3 *b)
{
   out->x = a->x - b->x;
   out->y = a->y - b->y;
   out->z = a->z - b->z;
}

inline void
_vec3_copy(vec3 *dst, const vec3 *src)
{
   dst->x = src->x;
   dst->y = src->y;
   dst->z = src->z;
}

inline void
_vec3_cross_product(vec3 *out, const vec3 *a, const vec3 *b)
{
   vec3 tmp;

   tmp.x = a->y * b->z - a->z * b->y;
   tmp.y = a->z * b->x - a->x * b->z;
   tmp.z = a->x * b->y - a->y * b->x;

   _vec3_copy(out, &tmp);
}

inline void
_vec3_normalize(vec3 *out)
{
   float size = out->x * out->x + out->y *out->y + out->z * out->z;
   size = sqrt(size);
   out->x /= size;
   out->y /= size;
   out->z /= size;
}

const float cube_vertices[] =
{
   /* Front */
   -0.5,  0.5,  0.5,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  1.0,      1.0, 0.0, 0.0,
    0.5,  0.5,  0.5,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  1.0,      1.0, 0.0, 0.0,
   -0.5, -0.5,  0.5,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  0.0,      1.0, 0.0, 0.0,
    0.5, -0.5,  0.5,     0.0,  0.0,  1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  0.0,      1.0, 0.0, 0.0,

   /* Back */
    0.5,  0.5, -0.5,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  1.0,     -1.0, 0.0, 0.0,
   -0.5,  0.5, -0.5,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  1.0,     -1.0, 0.0, 0.0,
    0.5, -0.5, -0.5,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     0.0,  0.0,     -1.0, 0.0, 0.0,
   -0.5, -0.5, -0.5,     0.0,  0.0, -1.0,     0.0, 0.0, 1.0, 1.0,     1.0,  0.0,     -1.0, 0.0, 0.0,

   /* Left */
   -0.5,  0.5, -0.5,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  1.0,     0.0, 0.0, 1.0,
   -0.5,  0.5,  0.5,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  1.0,     0.0, 0.0, 1.0,
   -0.5, -0.5, -0.5,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     0.0,  0.0,     0.0, 0.0, 1.0,
   -0.5, -0.5,  0.5,    -1.0,  0.0,  0.0,     0.0, 1.0, 0.0, 1.0,     1.0,  0.0,     0.0, 0.0, 1.0,

   /* Right */
    0.5,  0.5,  0.5,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  1.0,     0.0, 0.0, -1.0,
    0.5,  0.5, -0.5,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  1.0,     0.0, 0.0, -1.0,
    0.5, -0.5,  0.5,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     0.0,  0.0,     0.0, 0.0, -1.0,
    0.5, -0.5, -0.5,     1.0,  0.0,  0.0,     1.0, 1.0, 0.0, 1.0,     1.0,  0.0,     0.0, 0.0, -1.0,

   /* Top */
   -0.5,  0.5, -0.5,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  1.0,     1.0, 0.0, 0.0,
    0.5,  0.5, -0.5,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  1.0,     1.0, 0.0, 0.0,
   -0.5,  0.5,  0.5,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     0.0,  0.0,     1.0, 0.0, 0.0,
    0.5,  0.5,  0.5,     0.0,  1.0,  0.0,     1.0, 0.0, 1.0, 1.0,     1.0,  0.0,     1.0, 0.0, 0.0,

   /* Bottom */
    0.5, -0.5, -0.5,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  1.0,     -1.0, 0.0, 0.0,
   -0.5, -0.5, -0.5,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  1.0,     -1.0, 0.0, 0.0,
    0.5, -0.5,  0.5,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     0.0,  0.0,     -1.0, 0.0, 0.0,
   -0.5, -0.5,  0.5,     0.0, -1.0,  0.0,     0.0, 1.0, 1.0, 1.0,     1.0,  0.0,     -1.0, 0.0, 0.0,
};

const unsigned short cube_indices[] =
{
   0,   1,  2,  2,  1,  3,
   4,   5,  6,  6,  5,  7,
   8,   9, 10, 10,  9, 11,
   12, 13, 14, 14, 13, 15,
   16, 17, 18, 18, 17, 19,
   20, 21, 22, 22, 21, 23
};

const float square_vertices[] =
{
   -0.5,  0.5,  0.0,     0.0,  0.0,  -1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  1.0,      1.0, 0.0, 0.0,
    0.5,  0.5,  0.0,     0.0,  0.0,  -1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  1.0,      1.0, 0.0, 0.0,
   -0.5, -0.5,  0.0,     0.0,  0.0,  -1.0,     1.0, 0.0, 0.0, 1.0,     0.0,  0.0,      1.0, 0.0, 0.0,
    0.5, -0.5,  0.0,     0.0,  0.0,  -1.0,     1.0, 0.0, 0.0, 1.0,     1.0,  0.0,      1.0, 0.0, 0.0,
};

const unsigned short square_indices[] = {0, 1, 2, 2, 1, 3};

#define ALLOCATE_VERTEX_DATA \
   vec3 *vertices = malloc(sizeof(vec3) * vcount); \
   vec3 *normals = malloc(sizeof(vec3) * vcount); \
   vec2 *tex_coord = malloc(sizeof(vec2) * vcount); \
   vec3 *tangents = malloc(sizeof(vec3) * vcount); \
   unsigned short *indices = malloc(sizeof(short) * icount);

#define SET_VERTEX_DATA(frame) \
   eo_do(mesh, evas_3d_mesh_vertex_count_set(vcount), \
         evas_3d_mesh_frame_add(frame), \
         evas_3d_mesh_frame_vertex_data_copy_set(frame, EVAS_3D_VERTEX_POSITION, \
                                       sizeof(vec3), &vertices[0]), \
         evas_3d_mesh_frame_vertex_data_copy_set(frame, EVAS_3D_VERTEX_NORMAL, \
                                       sizeof(vec3), &normals[0]), \
         evas_3d_mesh_frame_vertex_data_copy_set(frame, EVAS_3D_VERTEX_TEXCOORD, \
                                       sizeof(vec2), &tex_coord[0]), \
         evas_3d_mesh_frame_vertex_data_copy_set(frame, EVAS_3D_VERTEX_TANGENT, \
                                       sizeof(vec3), &tangents[0]), \
         evas_3d_mesh_index_data_copy_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, \
                                       icount, &indices[0])); \
   free(vertices); \
   free(normals); \
   free(tangents); \
   free(tex_coord); \
   free(indices);

#define SET_VERTEX_DATA_FROM_ARRAY(mesh, frame, v_array, v_count, i_array, i_count) \
   eo_do(mesh, \
         evas_3d_mesh_vertex_count_set(v_count), \
         evas_3d_mesh_frame_add(frame), \
         evas_3d_mesh_frame_vertex_data_set(frame, EVAS_3D_VERTEX_POSITION, \
                                       15 * sizeof(float), &v_array[0]), \
         evas_3d_mesh_frame_vertex_data_set(frame, EVAS_3D_VERTEX_NORMAL, \
                                       15 * sizeof(float), &v_array[3]), \
         evas_3d_mesh_frame_vertex_data_set(frame, EVAS_3D_VERTEX_COLOR, \
                                       15 * sizeof(float), &v_array[6]), \
         evas_3d_mesh_frame_vertex_data_set(frame, EVAS_3D_VERTEX_TEXCOORD, \
                                       15 * sizeof(float), &v_array[10]), \
         evas_3d_mesh_frame_vertex_data_set(frame, EVAS_3D_VERTEX_TANGENT, \
                                       15 * sizeof(float), &v_array[12]), \
         evas_3d_mesh_index_data_set(EVAS_3D_INDEX_FORMAT_UNSIGNED_SHORT, \
                                       i_count, &i_array[0]), \
         evas_3d_mesh_vertex_assembly_set(EVAS_3D_VERTEX_ASSEMBLY_TRIANGLES));

vec3 _get_func_normal(Surface *func, float x, float y)
{
   vec3 v00, v01, v10, d1, d2, normal;

   v00 = func(x, y);
   v01 = func(x, y + 0.01);
   v10 = func(x + 0.01, y);
   _vec3_subtract(&d1, &v00, &v01);
   _vec3_subtract(&d2, &v01, &v10);

   _vec3_cross_product(&normal, &d1, &d2);

   _vec3_normalize(&normal);

   return normal;
}

static float
_random(int x, int y)
{
   int k = x + y * 57;
   k = (k << 13) ^ k;
   return (1.0f - ((k * (k * k * 15731 + 789221) + 1376312589) & 0x7fffffff) /
      1073741824.0f);
}

static float
_smooth(float x, float y)
{
   float res;
   res = (_random(x - 1, y - 1) + _random(x + 1, y - 1) +
        _random(x - 1, y + 1) + _random(x + 1, y + 1) ) / 16;
   res += (_random(x - 1, y) + _random(x + 1, y) +
        _random(x, y - 1) + _random(x, y + 1)) / 8;
   res += _random(x, y) / 4;
   return res;
}

static float
_interpolate(float a, float b, float x)
{
   float ft = x * M_PI;
   float f = (1 - cosf(ft)) * 0.5;
   return a * (1 - f) + b * f;
}

static float _noise(float x, float y)
{
   float ix = (int)(x);
   float fx = x - ix;
   float iy = (int)(y);
   float fy = y - iy;

   float v1 = _smooth(ix, iy);
   float v2 = _smooth(ix + 1, iy);
   float v3 = _smooth(ix, iy + 1);
   float v4 = _smooth(ix + 1, iy + 1);

   float i1 = _interpolate(v1, v2, fx);
   float i2 = _interpolate(v3, v4, fx);

   return _interpolate(i1, i2, fy);
}

static vec3
_perlin_terrain(float x,float y)
{
   vec3 out;
   float persistence = 0.5f;
   float frequency = 5;
   float amplitude = 1;
   int i = 0;
   int octaves = 5;

   out.x = x;
   x += 0.5;
   out.y = y;
   y += 0.5;
   out.z = 0;

   for(i = 0;i < octaves; i++)
     {
        out.z += _noise(x * frequency, y * frequency) * amplitude;

        amplitude *= persistence;
        frequency *= 2;
     }

   return out;
}

void
_generate_grid_indices(unsigned short *indices, int count)
{
   int i, j;
   unsigned short *index = &indices[0];
   int vccount = count + 1;

   for (j = 0; j < count; j++)
     for (i = 0; i < count; i++)
       {
          *index++ = (unsigned short)(i + vccount * j);
          *index++ = i + vccount * (j + 1);
          *index++ = i + 1 + vccount * (j + 1);

          *index++ = i + vccount * j;
          *index++ = i + 1 +  vccount * j;
          *index++ = i + vccount * (j + 1) + 1;
       }
}

void
_generate_tape_indices(unsigned short *indices, int count)
{
   int i, vccount = count + 1;
   unsigned short *index = &indices[0];

   for (i = 0; i < count; i++)
     {
        *index++ = i;
        *index++ = i + 1;
        *index++ = i + 1 + vccount;

        *index++ = i;
        *index++ = i + vccount;
        *index++ = i + vccount + 1;
     }
}

void
evas_3d_add_sphere_frame(Eo *mesh, int frame, int p, vec2 tex_scale)
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
             normals[i + j * vccount].y = costh;
             normals[i + j * vccount].z =  sinth * cosfi;

             vertices[i + j * vccount].x = normals[i + j * vccount].x / 2;
             vertices[i + j * vccount].y = normals[i + j * vccount].y / 2;
             vertices[i + j * vccount].z = normals[i + j * vccount].z / 2;

             tangents[i + j * vccount].x = normals[i + j * vccount].z;
             tangents[i + j * vccount].y = normals[i + j * vccount].y;
             tangents[i + j * vccount].z = -normals[i + j * vccount].x;

             tex_coord[i + j * vccount].x = i / (float)(vccount - 1) * tex_scale.x;
             tex_coord[i + j *vccount].y = tex_scale.y - j / (float)(vccount - 1) * tex_scale.y;
          }
     }

   _generate_grid_indices(indices, p);

   SET_VERTEX_DATA(frame)
}

void
evas_3d_add_func_surface_frame(Eo *mesh, int frame, Surface func, int p, vec2 tex_scale)
{
   int vcount, icount, vccount, i, j;
   icount = p * p * 6;
   vccount = p + 1;
   vcount = vccount * vccount;

   ALLOCATE_VERTEX_DATA

   double v, u, d = 1.0 / p;

   for (j = 0; j < vccount; j++)
     {
        u = j * d - 0.5;
        for (i = 0; i < vccount; i++)
          {
             v = i * d - 0.5;
             vertices[i + j * vccount] = func(v, u);
             normals[i + j * vccount] = _get_func_normal(func, v, u);

             // TODO Add tangent calculation
             tangents[i + j * vccount].x = 0;
             tangents[i + j * vccount].y = 0;
             tangents[i + j * vccount].z = 0;

             tex_coord[i + j * vccount].x = i / (float)(vccount - 1) * tex_scale.x;
             tex_coord[i + j *vccount].y = tex_scale.y - j / (float)(vccount - 1) * tex_scale.y;
          }
     }

   _generate_grid_indices(indices, p);
   SET_VERTEX_DATA(frame)
}

void
evas_3d_add_terrain_frame(Eo *mesh, int frame, int p, vec2 tex_scale)
{
   evas_3d_add_func_surface_frame(mesh, frame, _perlin_terrain, p, tex_scale);
}

void
evas_3d_add_torus_frame(Eo *mesh, int frame, float rratio, int p, vec2 tex_scale)
{
   int vcount, icount, vccount, i, j;
   icount = p * p * 6;
   vccount = p + 1;
   vcount = vccount * vccount;

   ALLOCATE_VERTEX_DATA

   double d, sinth, costh, fi, theta, sinfi, cosfi;

   d = 2 * M_PI / p;

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
             vertices[i + j * vccount].x = (1 + rratio * cosfi) * costh;
             vertices[i + j * vccount].y = (1 + rratio * cosfi) * sinth;
             vertices[i + j * vccount].z = rratio * sinfi;

             normals[i + j * vccount].x = cosfi * costh;
             normals[i + j * vccount].y = cosfi * sinth;
             normals[i + j * vccount].z = sinfi;

             tangents[i + j * vccount].x = - sinfi * costh;
             tangents[i + j * vccount].y = - sinfi * sinth;
             tangents[i + j * vccount].z = cosfi;

             _vec3_normalize(&normals[i + j * vccount]);

             tex_coord[i + j * vccount].x = i / (float)(vccount - 1) * tex_scale.x;
             tex_coord[i + j *vccount].y = tex_scale.y - j / (float)(vccount - 1) * tex_scale.y;
          }
     }

   _generate_grid_indices(indices, p);

   SET_VERTEX_DATA(frame)
}

void
evas_3d_add_cylinder_frame(Eo *mesh, int frame, int p, vec2 tex_scale)
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
        vertices[i + vccount].x = vertices[i].x = sinfi ;
        vertices[i].y = -0.5;
        vertices[i + vccount].z = vertices[i].z = cosfi;
        vertices[i + vccount].y = 0.5;

        normals[i + vccount].x = normals[i].x = sinfi;
        normals[i + vccount].y = normals[i].y = 0;
        normals[i + vccount].z = normals[i].z = cosfi;

        tangents[i + vccount].x = tangents[i].x = cosfi;
        tangents[i + vccount].y = tangents[i].y = 0;
        tangents[i + vccount].z = tangents[i].z = -sinfi;

        tex_coord[i].x = i / (float)(vccount - 1) * tex_scale.x;
        tex_coord[i].y = 0;
        tex_coord[i + vccount].x = i / (float)(vccount - 1) * tex_scale.x;
        tex_coord[i + vccount].y = tex_scale.y;
     }

   _generate_tape_indices(indices, p);

   SET_VERTEX_DATA(frame)
}

void
evas_3d_add_cone_frame(Eo *mesh, int frame, int p, vec2 tex_scale)
{
   int vcount, icount, vccount, i;
   double dfi, fi, sinfi, cosfi;

   icount = p * 6;
   vccount = p + 1;
   vcount = 2 * vccount;

   dfi = 2.0 * M_PI / p;

   ALLOCATE_VERTEX_DATA

   for (i = 0; i < vccount; i++)
     {
        fi = i * dfi;
        sinfi = sin(fi);
        cosfi = cos(fi);
        vertices[i].x = sinfi;
        vertices[i].y = 0;
        vertices[i].z = cosfi;
        vertices[i + vccount].x = 0;
        vertices[i + vccount].y = 1;
        vertices[i + vccount].z = 0;

        normals[i + vccount].x = normals[i].x = sinfi * 0.71;
        normals[i + vccount].y = normals[i].y = 0.71;
        normals[i + vccount].z = normals[i].z = cosfi * 0.71;

        tangents[i + vccount].x = tangents[i].x = cosfi;
        tangents[i + vccount].y = tangents[i].y = 0;
        tangents[i + vccount].z = tangents[i].z = -sinfi;

        tex_coord[i].x = i / (float)(vccount - 1) * tex_scale.x;
        tex_coord[i].y = 0;
        tex_coord[i + vccount].x = tex_coord[i].x;
        tex_coord[i + vccount].y = tex_scale.y;
     }

   _generate_tape_indices(indices, p);

   SET_VERTEX_DATA(frame)
}

void
evas_3d_add_square_frame(Eo *mesh, int frame)
{
   SET_VERTEX_DATA_FROM_ARRAY(mesh, frame, square_vertices, 4, square_indices, 6)
}

void
evas_3d_add_cube_frame(Eo *mesh, int frame)
{
   SET_VERTEX_DATA_FROM_ARRAY(mesh, frame, cube_vertices, 24, cube_indices, 36)
}
