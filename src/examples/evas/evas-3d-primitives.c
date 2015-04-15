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
          *index++ = (i * vccount) + j;
          *index++ = (i * vccount) + j + 1;
          *index++ = ((i + 1) * vccount) + j;

          *index++ = ((i + 1) * vccount) + j;
          *index++ = (i * vccount) + j + 1;
          *index++ = ((i + 1) * vccount) + j + 1;
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
   unsigned short *index;

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

   _generate_grid_indices(indices, p);

   /* Triangulation of sphere mesh in appliance with buffer of indices. */
   for (i = 0; i < icount; i += 3)
     {
        vec3 e1, e2;
        float du1, du2, dv1, dv2, f;
        vec3 tangent;
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
_normalize(vec3 *vertices, vec3 *normals, int vcount)
{
   int i;
   vec3 min, max;
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

   _normalize(vertices, normals, vcount);
   _generate_grid_indices(indices, p);
   SET_VERTEX_DATA(frame)
}

void
evas_3d_add_terrain_frame(Eo *mesh, int frame, int p, vec2 tex_scale)
{
   evas_3d_add_func_surface_frame(mesh, frame, _perlin_terrain, p, tex_scale);
}

void
evas_3d_add_torus_frame(Eo *mesh, int frame, float ratio, int p, vec2 tex_scale)
{
   int vcount, icount, vccount, i, j;
   icount = p * p * 6;
   vccount = p + 1;
   vcount = vccount * vccount;

   ALLOCATE_VERTEX_DATA

   double d, sinth, costh, fi, theta, sinfi, cosfi;

   d = 2 * M_PI / p;

   float rratio;

   if ((ratio > 1.0) || (ratio < 0.0))
     {
        printf("Ratio of torus should be between 0.0 and 1.0. \n");
        printf("Ratio = %f is a bad value, so 0.25 is used like default ratio.\n",
                ratio);
        rratio = 0.25;
     }
   else
     {
        rratio = ratio;
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

   _generate_tape_indices(indices, p);

   SET_VERTEX_DATA(frame)
}

void
evas_3d_add_cone_frame(Eo *mesh, int frame, int p, vec2 tex_scale)
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
