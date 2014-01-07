#ifndef EVAS_PRIVATE_H
# error You shall not include this header directly
#endif

#include <math.h>

#define  DEGREE_TO_RADIAN(x)     (((x) * M_PI) / 180.0)
#define  EVAS_MATRIX_IS_IDENTITY 0x00000001

typedef struct _Evas_Color
{
   Evas_Real   r;
   Evas_Real   g;
   Evas_Real   b;
   Evas_Real   a;
} Evas_Color;

typedef struct _Evas_Vec2
{
   Evas_Real   x;
   Evas_Real   y;
} Evas_Vec2;

typedef struct _Evas_Vec3
{
   Evas_Real   x;
   Evas_Real   y;
   Evas_Real   z;
} Evas_Vec3;

typedef struct _Evas_Vec4
{
   Evas_Real   x;
   Evas_Real   y;
   Evas_Real   z;
   Evas_Real   w;
} Evas_Vec4;

typedef struct _Evas_Mat2
{
   Evas_Real   m[4];
   int         flags;
} Evas_Mat2;

typedef struct _Evas_Mat3
{
   Evas_Real   m[9];
   int         flags;
} Evas_Mat3;

typedef struct _Evas_Mat4
{
   Evas_Real   m[16];
   int         flags;
} Evas_Mat4;

typedef struct _Evas_Box2
{
   Evas_Vec2   p0;
   Evas_Vec2   p1;
} Evas_Box2;

typedef struct _Evas_Box3
{
   Evas_Vec3   p0;
   Evas_Vec3   p1;
} Evas_Box3;

typedef struct _Evas_Triangle3
{
   Evas_Vec3   p0;
   Evas_Vec3   p1;
   Evas_Vec3   p2;
} Evas_Triangle3;

typedef struct _Evas_Ray3
{
   Evas_Vec3   org;
   Evas_Vec3   dir;
} Evas_Ray3;

/* 2D vector */
static inline void
evas_vec2_set(Evas_Vec2 *dst, Evas_Real x, Evas_Real y)
{
   dst->x = x;
   dst->y = y;
}

static inline void
evas_vec2_array_set(Evas_Vec2 *dst, const Evas_Real *v)
{
   dst->x = v[0];
   dst->y = v[1];
}

static inline void
evas_vec2_copy(Evas_Vec2 *dst, const Evas_Vec2 *src)
{
   dst->x = src->x;
   dst->y = src->y;
}

static inline void
evas_vec2_negate(Evas_Vec2 *out, const Evas_Vec2 *v)
{
   out->x = -v->x;
   out->y = -v->y;
}

static inline void
evas_vec2_add(Evas_Vec2 *out, const Evas_Vec2 *a, const Evas_Vec2 *b)
{
   out->x = a->x + b->x;
   out->y = a->y + b->y;
}

static inline void
evas_vec2_subtract(Evas_Vec2 *out, const Evas_Vec2 *a, const Evas_Vec2 *b)
{
   out->x = a->x - b->x;
   out->y = a->y - b->y;
}

static inline void
evas_vec2_scale(Evas_Vec2 *out, const Evas_Vec2 *v, Evas_Real scale)
{
   out->x = scale * v->x;
   out->y = scale * v->y;
}

static inline Evas_Real
evas_vec2_dot_product(const Evas_Vec2 *a, const Evas_Vec2 *b)
{
   return (a->x * b->x) + (a->y * b->y);
}

static inline Evas_Real
evas_vec2_length_get(const Evas_Vec2 *v)
{
   return (Evas_Real)sqrt((double)((v->x * v->x) + (v->y * v->y)));
}

static inline Evas_Real
evas_vec2_length_square_get(const Evas_Vec2 *v)
{
   return (v->x * v->x) + (v->y * v->y);
}

static inline Evas_Real
evas_vec2_distance_get(const Evas_Vec2 *a, const Evas_Vec2 *b)
{
   Evas_Vec2 v;

   evas_vec2_subtract(&v, a, b);
   return evas_vec2_length_get(&v);
}

static inline Evas_Real
evas_vec2_distance_square_get(const Evas_Vec2 *a, const Evas_Vec2 *b)
{
   Evas_Vec2 v;

   evas_vec2_subtract(&v, a, b);
   return evas_vec2_length_square_get(&v);
}

static inline void
evas_vec2_normalize(Evas_Vec2 *out, const Evas_Vec2 *v)
{
   /* Assume "v" is not a zero vector */
   evas_vec2_scale(out, v, 1.0 / evas_vec2_length_get(v));
}

static inline void
evas_vec2_transform(Evas_Vec2 *out, const Evas_Mat2 *m, const Evas_Vec2 *v)
{
   Evas_Vec2 tmp;

   tmp.x = (m->m[0] * v->x) + (m->m[2] * v->y);
   tmp.y = (m->m[1] * v->x) + (m->m[3] * v->y);

   evas_vec2_copy(out, &tmp);
}

static inline void
evas_vec2_homogeneous_position_transform(Evas_Vec2 *out, const Evas_Mat3 *m, const Evas_Vec2 *v)
{
   Evas_Vec2 tmp;

   tmp.x = (m->m[0] * v->x) + (m->m[3] * v->y) + m->m[6];
   tmp.y = (m->m[1] * v->x) + (m->m[4] * v->y) + m->m[7];

   evas_vec2_scale(out, &tmp, 1.0 / ((m->m[2] * v->x) + (m->m[5] * v->y) + m->m[8]));
}

static inline void
evas_vec2_homogeneous_direction_transform(Evas_Vec2 *out, const Evas_Mat3 *m, const Evas_Vec2 *v)
{
   Evas_Vec2 tmp;

   tmp.x = (m->m[0] * v->x) + (m->m[3] * v->y);
   tmp.y = (m->m[1] * v->x) + (m->m[4] * v->y);

   evas_vec2_copy(out, &tmp);
}

/* 3D vector */
static inline void
evas_vec3_set(Evas_Vec3 *dst, Evas_Real x, Evas_Real y, Evas_Real z)
{
   dst->x = x;
   dst->y = y;
   dst->z = z;
}

static inline void
evas_vec3_array_set(Evas_Vec3 *dst, const Evas_Real *v)
{
   dst->x = v[0];
   dst->y = v[1];
   dst->z = v[2];
}

static inline void
evas_vec3_copy(Evas_Vec3 *dst, const Evas_Vec3 *src)
{
   dst->x = src->x;
   dst->y = src->y;
   dst->z = src->z;
}

static inline void
evas_vec3_negate(Evas_Vec3 *out, const Evas_Vec3 *v)
{
   out->x = -v->x;
   out->y = -v->y;
   out->z = -v->z;
}

static inline void
evas_vec3_add(Evas_Vec3 *out, const Evas_Vec3 *a, const Evas_Vec3 *b)
{
   out->x = a->x + b->x;
   out->y = a->y + b->y;
   out->z = a->z + b->z;
}

static inline void
evas_vec3_subtract(Evas_Vec3 *out, const Evas_Vec3 *a, const Evas_Vec3 *b)
{
   out->x = a->x - b->x;
   out->y = a->y - b->y;
   out->z = a->z - b->z;
}

static inline void
evas_vec3_scale(Evas_Vec3 *out, const Evas_Vec3 *v, Evas_Real scale)
{
   out->x = scale * v->x;
   out->y = scale * v->y;
   out->z = scale * v->z;
}

static inline void
evas_vec3_multiply(Evas_Vec3 *out, const Evas_Vec3 *a, const Evas_Vec3 *b)
{
   out->x = a->x * b->x;
   out->y = a->y * b->y;
   out->z = a->z * b->z;
}

static inline Evas_Real
evas_vec3_dot_product(const Evas_Vec3 *a, const Evas_Vec3 *b)
{
   return (a->x * b->x) + (a->y * b->y) + (a->z * b->z);
}

static inline void
evas_vec3_cross_product(Evas_Vec3 *out, const Evas_Vec3 *a, const Evas_Vec3 *b)
{
   Evas_Vec3 tmp;

   tmp.x = a->y * b->z - a->z * b->y;
   tmp.y = a->z * b->x - a->x * b->z;
   tmp.z = a->x * b->y - a->y * b->x;

   evas_vec3_copy(out, &tmp);
}

static inline Evas_Real
evas_vec3_length_get(const Evas_Vec3 *v)
{
   return (Evas_Real)sqrt((double)((v->x * v->x) + (v->y * v->y) + (v->z * v->z)));
}

static inline Evas_Real
evas_vec3_length_square_get(const Evas_Vec3 *v)
{
   return (v->x * v->x) + (v->y * v->y) + (v->z * v->z);
}

static inline Evas_Real
evas_vec3_distance_get(const Evas_Vec3 *a, const Evas_Vec3 *b)
{
   Evas_Vec3 v;

   evas_vec3_subtract(&v, a, b);
   return evas_vec3_length_get(&v);
}

static inline Evas_Real
evas_vec3_distance_square_get(const Evas_Vec3 *a, const Evas_Vec3 *b)
{
   Evas_Vec3 v;

   evas_vec3_subtract(&v, a, b);
   return evas_vec3_length_square_get(&v);
}

static inline void
evas_vec3_normalize(Evas_Vec3 *out, const Evas_Vec3 *v)
{
   /* Assume "v" is not a zero vector */
   evas_vec3_scale(out, v, 1.0 / evas_vec3_length_get(v));
}

static inline void
evas_vec3_transform(Evas_Vec3 *out, const Evas_Vec3 *v,  const Evas_Mat3 *m)
{
   Evas_Vec3 tmp;

   if (m->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_vec3_copy(out, v);
        return;
     }

   tmp.x = (m->m[0] * v->x) + (m->m[3] * v->y) + (m->m[6] * v->z);
   tmp.y = (m->m[1] * v->x) + (m->m[4] * v->y) + (m->m[7] * v->z);
   tmp.z = (m->m[2] * v->x) + (m->m[5] * v->y) + (m->m[8] * v->z);

   evas_vec3_copy(out, &tmp);
}

static inline void
evas_vec3_homogeneous_position_transform(Evas_Vec3 *out, const Evas_Vec3 *v, const Evas_Mat4 *m)
{
   Evas_Vec3 tmp;

   if (m->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_vec3_copy(out, v);
        return;
     }

   tmp.x = (m->m[0] * v->x) + (m->m[4] * v->y) + (m->m[8]  * v->z) + m->m[12];
   tmp.y = (m->m[1] * v->x) + (m->m[5] * v->y) + (m->m[9]  * v->z) + m->m[13];
   tmp.z = (m->m[2] * v->x) + (m->m[6] * v->y) + (m->m[10] * v->z) + m->m[14];

   evas_vec3_scale(out, &tmp,
                   1.0 / ((m->m[3] * v->x) + (m->m[7] * v->y) + (m->m[11] * v->z) + m->m[15]));
}

static inline void
evas_vec3_homogeneous_direction_transform(Evas_Vec3 *out, const Evas_Vec3 *v, const Evas_Mat4 *m)
{
   Evas_Vec3 tmp;

   if (m->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_vec3_copy(out, v);
        return;
     }

   tmp.x = (m->m[0] * v->x) + (m->m[4] * v->y) + (m->m[8]  * v->z);
   tmp.y = (m->m[1] * v->x) + (m->m[5] * v->y) + (m->m[9]  * v->z);
   tmp.z = (m->m[2] * v->x) + (m->m[6] * v->y) + (m->m[10] * v->z);

   evas_vec3_copy(out, &tmp);
}

static inline void
evas_vec3_quaternion_rotate(Evas_Vec3 *out, const Evas_Vec3 *v, const Evas_Vec4 *q)
{
   Evas_Vec3   uv, uuv;
   Evas_Vec3   axis;

   evas_vec3_set(&axis, q->x, q->y, q->z);

   evas_vec3_cross_product(&uv, &axis, v);
   evas_vec3_cross_product(&uuv, &axis, &uv);

   evas_vec3_scale(&uv, &uv, 2.0 * q->w);
   evas_vec3_scale(&uuv, &uuv, 2.0);

   out->x = v->x + uv.x + uuv.x;
   out->y = v->y + uv.y + uuv.y;
   out->z = v->z + uv.z + uuv.z;
}

/* 4D vector */
static inline void
evas_vec4_set(Evas_Vec4 *dst, Evas_Real x, Evas_Real y, Evas_Real z, Evas_Real w)
{
   dst->x = x;
   dst->y = y;
   dst->z = z;
   dst->w = w;
}

static inline void
evas_vec4_array_set(Evas_Vec4 *dst, const Evas_Real *v)
{
   dst->x = v[0];
   dst->y = v[1];
   dst->z = v[2];
   dst->w = v[3];
}

static inline void
evas_vec4_copy(Evas_Vec4 *dst, const Evas_Vec4 *src)
{
   dst->x = src->x;
   dst->y = src->y;
   dst->z = src->z;
   dst->w = src->w;
}

static inline void
evas_vec4_homogeneous_regulate(Evas_Vec4 *out, const Evas_Vec4 *v)
{
   if (v->w != 0.0)
     {
        Evas_Real scale = 1.0 / v->w;

        out->x = v->x * scale;
        out->y = v->y * scale;
        out->z = v->z * scale;
        out->w = 1.0;
     }
}

static inline void
evas_vec4_negate(Evas_Vec4 *out, const Evas_Vec4 *v)
{
   out->x = -v->x;
   out->y = -v->y;
   out->z = -v->z;
   out->w = -v->w;
}

static inline void
evas_vec4_add(Evas_Vec4 *out, const Evas_Vec4 *a, const Evas_Vec4 *b)
{
   out->x = a->x + b->x;
   out->y = a->y + b->y;
   out->z = a->z + b->z;
   out->w = a->w + b->w;
}

static inline void
evas_vec4_subtract(Evas_Vec4 *out, const Evas_Vec4 *a, const Evas_Vec4 *b)
{
   out->x = a->x - b->x;
   out->y = a->y - b->y;
   out->z = a->z - b->z;
   out->w = a->w - b->w;
}

static inline void
evas_vec4_scale(Evas_Vec4 *out, const Evas_Vec4 *v, Evas_Real scale)
{
   out->x = scale * v->x;
   out->y = scale * v->y;
   out->z = scale * v->z;
   out->w = scale * v->w;
}

static inline void
evas_vec4_multiply(Evas_Vec4 *out, const Evas_Vec4 *a, const Evas_Vec4 *b)
{
   out->x = a->x * b->x;
   out->y = a->y * b->y;
   out->z = a->z * b->z;
   out->w = a->w * b->w;
}

static inline Evas_Real
evas_vec4_length_get(const Evas_Vec4 *v)
{
   return (Evas_Real)sqrt((double)((v->x * v->x) + (v->y * v->y) +
                                   (v->z * v->z) + (v->w + v->w)));
}

static inline Evas_Real
evas_vec4_length_square_get(const Evas_Vec4 *v)
{
   return (v->x * v->x) + (v->y * v->y) + (v->z * v->z) + (v->w * v->w);
}

static inline Evas_Real
evas_vec4_distance_get(const Evas_Vec4 *a, const Evas_Vec4 *b)
{
   Evas_Vec4 v;

   evas_vec4_subtract(&v, a, b);
   return evas_vec4_length_get(&v);
}

static inline Evas_Real
evas_vec4_distance_square_get(const Evas_Vec4 *a, const Evas_Vec4 *b)
{
   Evas_Vec4 v;

   evas_vec4_subtract(&v, a, b);
   return evas_vec4_length_square_get(&v);
}

static inline void
evas_vec4_normalize(Evas_Vec4 *out, const Evas_Vec4 *v)
{
   /* Assume "v" is not a zero vector */
   evas_vec4_scale(out, v, 1.0 / evas_vec4_length_get(v));
}

static inline void
evas_vec4_transform(Evas_Vec4 *out, const Evas_Vec4 *v, const Evas_Mat4 *m)
{
   Evas_Vec4 tmp;

   if (m->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_vec4_copy(out, v);
        return;
     }

   tmp.x = (m->m[0] * v->x) + (m->m[4] * v->y) + (m->m[ 8] * v->z) + (m->m[12] * v->w);
   tmp.y = (m->m[1] * v->x) + (m->m[5] * v->y) + (m->m[ 9] * v->z) + (m->m[13] * v->w);
   tmp.z = (m->m[2] * v->x) + (m->m[6] * v->y) + (m->m[10] * v->z) + (m->m[14] * v->w);
   tmp.w = (m->m[3] * v->x) + (m->m[7] * v->y) + (m->m[11] * v->z) + (m->m[15] * v->w);

   evas_vec4_copy(out, &tmp);
}

static inline void
evas_vec3_homogeneous_position_set(Evas_Vec3 *out, const Evas_Vec4 *v)
{
   /* Assume "v" is a positional vector. (v->w != 0.0) */
   Evas_Real h = 1.0 / v->w;

   out->x = v->x * h;
   out->y = v->y * h;
   out->z = v->z * h;
}

static inline void
evas_vec3_homogeneous_direction_set(Evas_Vec3 *out, const Evas_Vec4 *v)
{
   /* Assume "v" is a directional vector. (v->w == 0.0) */
   out->x = v->x;
   out->y = v->y;
   out->z = v->z;
}

static inline void
evas_vec4_homogeneous_position_set(Evas_Vec4 *out, const Evas_Vec3 *v)
{
   out->x = v->x;
   out->y = v->y;
   out->z = v->z;
   out->w = 1.0;
}

static inline void
evas_vec4_homogeneous_direction_set(Evas_Vec4 *out, const Evas_Vec3 *v)
{
   out->x = v->x;
   out->y = v->y;
   out->z = v->z;
   out->w = 0.0;
}

/* 4x4 matrix */
static inline void
evas_mat4_identity_set(Evas_Mat4 *m)
{
   m->m[0]  = 1.0;
   m->m[1]  = 0.0;
   m->m[2]  = 0.0;
   m->m[3]  = 0.0;

   m->m[4]  = 0.0;
   m->m[5]  = 1.0;
   m->m[6]  = 0.0;
   m->m[7]  = 0.0;

   m->m[8]  = 0.0;
   m->m[9]  = 0.0;
   m->m[10] = 1.0;
   m->m[11] = 0.0;

   m->m[12] = 0.0;
   m->m[13] = 0.0;
   m->m[14] = 0.0;
   m->m[15] = 1.0;

   m->flags = EVAS_MATRIX_IS_IDENTITY;
}

static inline void
evas_mat4_array_set(Evas_Mat4 *m, const Evas_Real *v)
{
   memcpy(&m->m[0], v, sizeof(Evas_Real) * 16);
   m->flags = 0;
}

static inline void
evas_mat4_copy(Evas_Mat4 *dst, const Evas_Mat4 *src)
{
   memcpy(dst, src, sizeof(Evas_Mat4));
}

static inline void
evas_mat4_nocheck_multiply(Evas_Mat4 *out, const Evas_Mat4 *mat_a, const Evas_Mat4 *mat_b)
{
   Evas_Real        *d = &out->m[0];
   const Evas_Real  *a = &mat_a->m[0];
   const Evas_Real  *b = &mat_b->m[0];

   if (mat_a->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_mat4_copy(out, mat_b);
        return;
     }

   if (mat_b->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_mat4_copy(out, mat_a);
        return;
     }

   d[ 0] = a[ 0] * b[ 0] + a[ 4] * b[ 1] + a[ 8] * b[ 2] + a[12] * b [3];
   d[ 4] = a[ 0] * b[ 4] + a[ 4] * b[ 5] + a[ 8] * b[ 6] + a[12] * b [7];
   d[ 8] = a[ 0] * b[ 8] + a[ 4] * b[ 9] + a[ 8] * b[10] + a[12] * b[11];
   d[12] = a[ 0] * b[12] + a[ 4] * b[13] + a[ 8] * b[14] + a[12] * b[15];

   d[ 1] = a[ 1] * b[ 0] + a[ 5] * b[ 1] + a[ 9] * b[ 2] + a[13] * b [3];
   d[ 5] = a[ 1] * b[ 4] + a[ 5] * b[ 5] + a[ 9] * b[ 6] + a[13] * b [7];
   d[ 9] = a[ 1] * b[ 8] + a[ 5] * b[ 9] + a[ 9] * b[10] + a[13] * b[11];
   d[13] = a[ 1] * b[12] + a[ 5] * b[13] + a[ 9] * b[14] + a[13] * b[15];

   d[ 2] = a[ 2] * b[ 0] + a[ 6] * b[ 1] + a[10] * b[ 2] + a[14] * b [3];
   d[ 6] = a[ 2] * b[ 4] + a[ 6] * b[ 5] + a[10] * b[ 6] + a[14] * b [7];
   d[10] = a[ 2] * b[ 8] + a[ 6] * b[ 9] + a[10] * b[10] + a[14] * b[11];
   d[14] = a[ 2] * b[12] + a[ 6] * b[13] + a[10] * b[14] + a[14] * b[15];

   d[ 3] = a[ 3] * b[ 0] + a[ 7] * b[ 1] + a[11] * b[ 2] + a[15] * b [3];
   d[ 7] = a[ 3] * b[ 4] + a[ 7] * b[ 5] + a[11] * b[ 6] + a[15] * b [7];
   d[11] = a[ 3] * b[ 8] + a[ 7] * b[ 9] + a[11] * b[10] + a[15] * b[11];
   d[15] = a[ 3] * b[12] + a[ 7] * b[13] + a[11] * b[14] + a[15] * b[15];

   out->flags = 0;
}

static inline void
evas_mat4_multiply(Evas_Mat4 *out, const Evas_Mat4 *mat_a, const Evas_Mat4 *mat_b)
{
   if (out != mat_a && out != mat_b)
     {
        evas_mat4_nocheck_multiply(out, mat_a, mat_b);
     }
   else
     {
        Evas_Mat4 result;

        evas_mat4_nocheck_multiply(&result, mat_a, mat_b);
        evas_mat4_copy(out, &result);
     }
}

static inline void
evas_mat4_look_at_set(Evas_Mat4 *m,
                      const Evas_Vec3 *pos, const Evas_Vec3 *center, const Evas_Vec3 *up)
{
   Evas_Vec3 x, y, z;

   evas_vec3_subtract(&z, pos, center);
   evas_vec3_normalize(&z, &z);

   evas_vec3_cross_product(&x, up, &z);
   evas_vec3_normalize(&x, &x);

   evas_vec3_cross_product(&y, &z, &x);
   evas_vec3_normalize(&y, &y);

   m->m[ 0] = x.x;
   m->m[ 1] = y.x;
   m->m[ 2] = z.x;
   m->m[ 3] = 0.0;

   m->m[ 4] = x.y;
   m->m[ 5] = y.y;
   m->m[ 6] = z.y;
   m->m[ 7] = 0.0;

   m->m[ 8] = x.z;
   m->m[ 9] = y.z;
   m->m[10] = z.z;
   m->m[11] = 0.0;

   m->m[12] = -evas_vec3_dot_product(&x, pos);
   m->m[13] = -evas_vec3_dot_product(&y, pos);
   m->m[14] = -evas_vec3_dot_product(&z, pos);
   m->m[15] = 1.0;

   m->flags = 0;
}

static inline void
evas_mat4_frustum_set(Evas_Mat4 *m,
                      Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top,
                      Evas_Real near, Evas_Real far)
{
   Evas_Real   w = right - left;
   Evas_Real   h = top - bottom;
   Evas_Real   depth = near - far;
   Evas_Real   near_2 = 2.0f * near;

   m->m[ 0] = near_2 / w;
   m->m[ 1] = 0.0f;
   m->m[ 2] = 0.0f;
   m->m[ 3] = 0.0f;

   m->m[ 4] = 0.0f;
   m->m[ 5] = near_2 / h;
   m->m[ 6] = 0.0f;
   m->m[ 7] = 0.0f;

   m->m[ 8] = (right + left) / w;
   m->m[ 9] = (top + bottom) / h;
   m->m[10] = (far + near) / depth;
   m->m[11] = -1.0f;

   m->m[12] = 0.0f;
   m->m[13] = 0.0f;
   m->m[14] = near_2 * far / depth;
   m->m[15] = 0.0f;

   m->flags = 0;
}

static inline void
evas_mat4_ortho_set(Evas_Mat4 *m,
                    Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top,
                    Evas_Real near, Evas_Real far)
{
   Evas_Real   w = right - left;
   Evas_Real   h = top - bottom;
   Evas_Real   depth = near - far;

   m->m[ 0] = 2.0f / w;
   m->m[ 1] = 0.0f;
   m->m[ 2] = 0.0f;
   m->m[ 3] = 0.0f;

   m->m[ 4] = 0.0f;
   m->m[ 5] = 2.0f / h;
   m->m[ 6] = 0.0f;
   m->m[ 7] = 0.0f;

   m->m[ 8] = 0.0f;
   m->m[ 9] = 0.0f;
   m->m[10] = 2.0f / depth;
   m->m[11] = 0.0f;

   m->m[12] = -(right + left) / w;
   m->m[13] = -(top + bottom) / h;
   m->m[14] = (far + near) / depth;
   m->m[15] = 1.0f;

   m->flags = 0;
}

static inline void
evas_mat4_nocheck_inverse(Evas_Mat4 *out, const Evas_Mat4 *mat)
{
   Evas_Real        *d = &out->m[0];
   const Evas_Real  *m = &mat->m[0];
   Evas_Real         det;

   if (mat->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_mat4_copy(out, mat);
        return;
     }

   d[ 0] =  m[ 5] * m[10] * m[15] -
            m[ 5] * m[11] * m[14] -
            m[ 9] * m[ 6] * m[15] +
            m[ 9] * m[ 7] * m[14] +
            m[13] * m[ 6] * m[11] -
            m[13] * m[ 7] * m[10];

   d[ 4] = -m[ 4] * m[10] * m[15] +
            m[ 4] * m[11] * m[14] +
            m[ 8] * m[ 6] * m[15] -
            m[ 8] * m[ 7] * m[14] -
            m[12] * m[ 6] * m[11] +
            m[12] * m[ 7] * m[10];

   d[ 8] =  m[ 4] * m[ 9] * m[15] -
            m[ 4] * m[11] * m[13] -
            m[ 8] * m[ 5] * m[15] +
            m[ 8] * m[ 7] * m[13] +
            m[12] * m[ 5] * m[11] -
            m[12] * m[ 7] * m[ 9];

   d[12] = -m[ 4] * m[ 9] * m[14] +
            m[ 4] * m[10] * m[13] +
            m[ 8] * m[ 5] * m[14] -
            m[ 8] * m[ 6] * m[13] -
            m[12] * m[ 5] * m[10] +
            m[12] * m[ 6] * m[ 9];

   d[ 1] = -m[ 1] * m[10] * m[15] +
            m[ 1] * m[11] * m[14] +
            m[ 9] * m[ 2] * m[15] -
            m[ 9] * m[ 3] * m[14] -
            m[13] * m[ 2] * m[11] +
            m[13] * m[ 3] * m[10];

   d[ 5] =  m[ 0] * m[10] * m[15] -
            m[ 0] * m[11] * m[14] -
            m[ 8] * m[ 2] * m[15] +
            m[ 8] * m[ 3] * m[14] +
            m[12] * m[ 2] * m[11] -
            m[12] * m[ 3] * m[10];

   d[ 9] = -m[ 0] * m[ 9] * m[15] +
            m[ 0] * m[11] * m[13] +
            m[ 8] * m[ 1] * m[15] -
            m[ 8] * m[ 3] * m[13] -
            m[12] * m[ 1] * m[11] +
            m[12] * m[ 3] * m[ 9];

   d[13] =  m[ 0] * m[ 9] * m[14] -
            m[ 0] * m[10] * m[13] -
            m[ 8] * m[ 1] * m[14] +
            m[ 8] * m[ 2] * m[13] +
            m[12] * m[ 1] * m[10] -
            m[12] * m[ 2] * m[ 9];

   d[ 2] =  m[ 1] * m[ 6] * m[15] -
            m[ 1] * m[ 7] * m[14] -
            m[ 5] * m[ 2] * m[15] +
            m[ 5] * m[ 3] * m[14] +
            m[13] * m[ 2] * m[ 7] -
            m[13] * m[ 3] * m[ 6];

   d[ 6] = -m[ 0] * m[ 6] * m[15] +
            m[ 0] * m[ 7] * m[14] +
            m[ 4] * m[ 2] * m[15] -
            m[ 4] * m[ 3] * m[14] -
            m[12] * m[ 2] * m[ 7] +
            m[12] * m[ 3] * m[ 6];

   d[10] =  m[ 0] * m[ 5] * m[15] -
            m[ 0] * m[ 7] * m[13] -
            m[ 4] * m[ 1] * m[15] +
            m[ 4] * m[ 3] * m[13] +
            m[12] * m[ 1] * m[ 7] -
            m[12] * m[ 3] * m[ 5];

   d[14] = -m[ 0] * m[ 5] * m[14] +
            m[ 0] * m[ 6] * m[13] +
            m[ 4] * m[ 1] * m[14] -
            m[ 4] * m[ 2] * m[13] -
            m[12] * m[ 1] * m[ 6] +
            m[12] * m[ 2] * m[ 5];

   d[ 3] = -m[ 1] * m[ 6] * m[11] +
            m[ 1] * m[ 7] * m[10] +
            m[ 5] * m[ 2] * m[11] -
            m[ 5] * m[ 3] * m[10] -
            m[ 9] * m[ 2] * m[ 7] +
            m[ 9] * m[ 3] * m[ 6];

   d[ 7] =  m[ 0] * m[ 6] * m[11] -
            m[ 0] * m[ 7] * m[10] -
            m[ 4] * m[ 2] * m[11] +
            m[ 4] * m[ 3] * m[10] +
            m[ 8] * m[ 2] * m[ 7] -
            m[ 8] * m[ 3] * m[ 6];

   d[11] = -m[ 0] * m[ 5] * m[11] +
            m[ 0] * m[ 7] * m[ 9] +
            m[ 4] * m[ 1] * m[11] -
            m[ 4] * m[ 3] * m[ 9] -
            m[ 8] * m[ 1] * m[ 7] +
            m[ 8] * m[ 3] * m[ 5];

   d[15] =  m[ 0] * m[ 5] * m[10] -
            m[ 0] * m[ 6] * m[ 9] -
            m[ 4] * m[ 1] * m[10] +
            m[ 4] * m[ 2] * m[ 9] +
            m[ 8] * m[ 1] * m[ 6] -
            m[ 8] * m[ 2] * m[ 5];

   det = m[0] * d[0] + m[1] * d[4] + m[2] * d[8] + m[3] * d[12];

   if (det == 0.0)
     return;

   det = 1.0 / det;

   d[ 0] *= det;
   d[ 1] *= det;
   d[ 2] *= det;
   d[ 3] *= det;
   d[ 4] *= det;
   d[ 5] *= det;
   d[ 6] *= det;
   d[ 7] *= det;
   d[ 8] *= det;
   d[ 9] *= det;
   d[10] *= det;
   d[11] *= det;
   d[12] *= det;
   d[13] *= det;
   d[14] *= det;
   d[15] *= det;

   out->flags = 0;
}

static inline void
evas_mat4_inverse(Evas_Mat4 *out, const Evas_Mat4 *mat)
{
   if (out != mat)
     {
        evas_mat4_nocheck_inverse(out, mat);
     }
   else
     {
        Evas_Mat4 tmp;

        evas_mat4_nocheck_inverse(&tmp, mat);
        evas_mat4_copy(out, &tmp);
     }
}

static inline void
evas_normal_matrix_get(Evas_Mat3 *out, const Evas_Mat4 *m)
{
   /* Normal matrix is a transposed matirx of inversed modelview.
    * And we need only upper-left 3x3 terms to work with. */

   Evas_Real   det;
   Evas_Real   a = m->m[0];
   Evas_Real   b = m->m[4];
   Evas_Real   c = m->m[8];
   Evas_Real   d = m->m[1];
   Evas_Real   e = m->m[5];
   Evas_Real   f = m->m[9];
   Evas_Real   g = m->m[2];
   Evas_Real   h = m->m[6];
   Evas_Real   i = m->m[10];

   det = a * e * i + b * f * g + c * d * h - g * e * c - h * f * a - i * d * b;
   det = 1.0 / det;

   out->m[0] = (e * i - f * h) * det;
   out->m[1] = (h * c - i * b) * det;
   out->m[2] = (b * f - c * e) * det;
   out->m[3] = (g * f - d * i) * det;
   out->m[4] = (a * i - g * c) * det;
   out->m[5] = (d * c - a * f) * det;
   out->m[6] = (d * h - g * e) * det;
   out->m[7] = (g * b - a * h) * det;
   out->m[8] = (a * e - d * b) * det;

   out->flags = 0;
}

/* 3x3 matrix */
static inline void
evas_mat3_identity_set(Evas_Mat3 *m)
{
   m->m[0] = 1.0;
   m->m[1] = 0.0;
   m->m[2] = 0.0;
   m->m[3] = 0.0;
   m->m[4] = 1.0;
   m->m[5] = 0.0;
   m->m[6] = 0.0;
   m->m[7] = 0.0;
   m->m[8] = 1.0;

   m->flags = EVAS_MATRIX_IS_IDENTITY;
}

static inline void
evas_mat3_array_set(Evas_Mat3 *m, const Evas_Real *v)
{
   memcpy(&m->m[0], v, sizeof(Evas_Real) * 9);
   m->flags = 0;
}

static inline void
evas_mat3_copy(Evas_Mat3 *dst, const Evas_Mat3 *src)
{
   memcpy(dst, src, sizeof(Evas_Mat3));
}

static inline void
evas_mat3_nocheck_multiply(Evas_Mat3 *out, const Evas_Mat3 *mat_a, const Evas_Mat3 *mat_b)
{
   Evas_Real        *d = &out->m[0];
   const Evas_Real  *a = &mat_a->m[0];
   const Evas_Real  *b = &mat_b->m[0];

   if (mat_a->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_mat3_copy(out, mat_b);
        return;
     }

   if (mat_b->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_mat3_copy(out, mat_a);
        return;
     }

   d[0] = a[0] * b[0] + a[3] * b[1] + a[6] * b[2];
   d[3] = a[0] * b[3] + a[3] * b[4] + a[6] * b[5];
   d[6] = a[0] * b[6] + a[3] * b[7] + a[6] * b[8];

   d[1] = a[1] * b[0] + a[4] * b[1] + a[7] * b[2];
   d[4] = a[1] * b[3] + a[4] * b[4] + a[7] * b[5];
   d[7] = a[1] * b[6] + a[4] * b[7] + a[7] * b[8];

   d[2] = a[2] * b[0] + a[5] * b[1] + a[8] * b[2];
   d[5] = a[2] * b[3] + a[5] * b[4] + a[8] * b[5];
   d[8] = a[2] * b[6] + a[5] * b[7] + a[8] * b[8];

   out->flags = 0;
}

static inline void
evas_mat3_multiply(Evas_Mat3 *out, const Evas_Mat3 *mat_a, const Evas_Mat3 *mat_b)
{
   if (out != mat_a && out != mat_b)
     {
        evas_mat3_nocheck_multiply(out, mat_a, mat_b);
     }
   else
     {
        Evas_Mat3 tmp;

        evas_mat3_nocheck_multiply(&tmp, mat_a, mat_b);
        evas_mat3_copy(out, &tmp);
     }
}

static inline void
evas_mat3_nocheck_inverse(Evas_Mat3 *out, const Evas_Mat3 *mat)
{
   Evas_Real        *d = &out->m[0];
   const Evas_Real  *m = &mat->m[0];
   Evas_Real         det;

   if (mat->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_mat3_copy(out, mat);
        return;
     }

   d[0] = m[4] * m[8] - m[7] * m[5];
   d[1] = m[7] * m[2] - m[1] * m[8];
   d[2] = m[1] * m[5] - m[4] * m[2];
   d[3] = m[6] * m[5] - m[3] * m[8];
   d[4] = m[0] * m[8] - m[6] * m[2];
   d[5] = m[3] * m[2] - m[0] * m[5];
   d[6] = m[3] * m[7] - m[6] * m[4];
   d[7] = m[6] * m[1] - m[0] * m[7];
   d[8] = m[0] * m[4] - m[3] * m[1];

   det = m[0] * d[0] + m[1] * d[3] + m[2] * d[6];

   if (det == 0.0)
     return;

   det = 1.0 / det;

   d[0] *= det;
   d[1] *= det;
   d[2] *= det;
   d[3] *= det;
   d[4] *= det;
   d[5] *= det;
   d[6] *= det;
   d[7] *= det;
   d[8] *= det;

   out->flags = 0;
}

static inline void
evas_mat3_invserse(Evas_Mat3 *out, const Evas_Mat3 *mat)
{
   if (out != mat)
     {
        evas_mat3_nocheck_inverse(out, mat);
     }
   else
     {
        Evas_Mat3 tmp;

        evas_mat3_nocheck_inverse(&tmp, mat);
        evas_mat3_copy(out, &tmp);
     }
}

/* 2x2 matrix */
static inline void
evas_mat2_identity_set(Evas_Mat2 *m)
{
   m->m[0] = 1.0;
   m->m[1] = 0.0;
   m->m[2] = 0.0;
   m->m[3] = 1.0;

   m->flags = EVAS_MATRIX_IS_IDENTITY;
}

static inline void
evas_mat2_array_set(Evas_Mat2 *m, const Evas_Real *v)
{
   memcpy(&m->m[0], v, sizeof(Evas_Real) * 4);
   m->flags = 0;
}

static inline void
evas_mat2_copy(Evas_Mat2 *dst, const Evas_Mat2 *src)
{
   memcpy(dst, src, sizeof(Evas_Mat2));
}

static inline void
evas_mat2_nocheck_multiply(Evas_Mat2 *out, const Evas_Mat2 *mat_a, const Evas_Mat2 *mat_b)
{
   Evas_Real        *d = &out->m[0];
   const Evas_Real  *a = &mat_a->m[0];
   const Evas_Real  *b = &mat_b->m[0];

   if (mat_a->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_mat2_copy(out, mat_b);
        return;
     }

   if (mat_b->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_mat2_copy(out, mat_a);
        return;
     }

   d[0] = a[0] * b[0] + a[2] * b[1];
   d[2] = a[0] * b[2] + a[2] * b[3];

   d[1] = a[1] * b[0] + a[3] * b[1];
   d[3] = a[1] * b[2] + a[3] * b[3];

   out->flags = 0;
}

static inline void
evas_mat2_multiply(Evas_Mat2 *out, const Evas_Mat2 *mat_a, const Evas_Mat2 *mat_b)
{
   if (out != mat_a && out != mat_b)
     {
        evas_mat2_nocheck_multiply(out, mat_a, mat_b);
     }
   else
     {
        Evas_Mat2 tmp;

        evas_mat2_nocheck_multiply(&tmp, mat_a, mat_b);
        evas_mat2_copy(out, &tmp);
     }
}

static inline void
evas_mat2_nocheck_inverse(Evas_Mat2 *out, const Evas_Mat2 *mat)
{
   Evas_Real        *d = &out->m[0];
   const Evas_Real  *m = &mat->m[0];
   Evas_Real         det;

   if (mat->flags & EVAS_MATRIX_IS_IDENTITY)
     {
        evas_mat2_copy(out, mat);
        return;
     }

   det = m[0] * m[3] - m[2] * m[1];

   if (det == 0.0)
     return;

   det = 1.0 / det;

   d[0] =  m[3] * det;
   d[1] = -m[1] * det;
   d[2] = -m[2] * det;
   d[3] =  m[0] * det;

   out->flags = 0;
}

static inline void
evas_mat2_invserse(Evas_Mat2 *out, const Evas_Mat2 *mat)
{
   if (out != mat)
     {
        evas_mat2_nocheck_inverse(out, mat);
     }
   else
     {
        Evas_Mat2 tmp;

        evas_mat2_nocheck_inverse(&tmp, mat);
        evas_mat2_copy(out, &tmp);
     }
}

static inline void
evas_box2_set(Evas_Box2 *box, Evas_Real x0, Evas_Real y0, Evas_Real x1, Evas_Real y1)
{
   box->p0.x = x0;
   box->p0.y = y0;
   box->p1.x = x1;
   box->p1.y = y1;
}

static inline void
evas_box3_set(Evas_Box3 *box, Evas_Real x0, Evas_Real y0, Evas_Real z0, Evas_Real x1, Evas_Real y1, Evas_Real z1)
{
   box->p0.x = x0;
   box->p0.y = y0;
   box->p0.z = z0;
   box->p1.x = x1;
   box->p1.y = y1;
   box->p1.z = z1;
}

static inline void
evas_box3_empty_set(Evas_Box3 *box)
{
   evas_vec3_set(&box->p0, 0.0, 0.0, 0.0);
   evas_vec3_set(&box->p1, 0.0, 0.0, 0.0);
}

static inline void
evas_box3_copy(Evas_Box3 *dst, const Evas_Box3 *src)
{
   evas_vec3_copy(&dst->p0, &src->p0);
   evas_vec3_copy(&dst->p1, &src->p1);
}

static inline void
evas_box3_union(Evas_Box3 *out, const Evas_Box3 *a, const Evas_Box3 *b)
{
   evas_vec3_set(&out->p0, MIN(a->p0.x, b->p0.x), MIN(a->p0.y, b->p0.y), MIN(a->p0.z, b->p0.z));
   evas_vec3_set(&out->p1, MAX(a->p1.x, b->p1.x), MAX(a->p1.y, b->p1.y), MAX(a->p1.z, b->p1.z));
}

static inline void
evas_box3_transform(Evas_Box3 *out EINA_UNUSED, const Evas_Box3 *box EINA_UNUSED, const Evas_Mat4 *mat EINA_UNUSED)
{
   /* TODO: */
}

static inline void
evas_mat4_position_get(const Evas_Mat4 *matrix, Evas_Vec4 *position)
{
   Evas_Vec4 pos;

   pos.x = 0.0;
   pos.y = 0.0;
   pos.z = 0.0;
   pos.w = 1.0;

   evas_vec4_transform(position, &pos, matrix);
}

static inline void
evas_mat4_direction_get(const Evas_Mat4 *matrix, Evas_Vec3 *direction)
{
   /* TODO: Check correctness. */

   Evas_Vec4 dir;

   dir.x = 0.0;
   dir.y = 0.0;
   dir.z = 1.0;
   dir.w = 1.0;

   evas_vec4_transform(&dir, &dir, matrix);

   direction->x = dir.x;
   direction->y = dir.y;
   direction->z = dir.z;
}

static inline void
evas_vec4_quaternion_multiply(Evas_Vec4 *out, const Evas_Vec4 *a, const Evas_Vec4 *b)
{
   Evas_Vec4 r;

   r.x = (a->w * b->x) + (a->x * b->w) + (a->y * b->z) - (a->z * b->y);
   r.y = (a->w * b->y) - (a->x * b->z) + (a->y * b->w) + (a->z * b->x);
   r.z = (a->w * b->z) + (a->x * b->y) - (a->y * b->x) + (a->z * b->w);
   r.w = (a->w * b->w) - (a->x * b->x) - (a->y * b->y) - (a->z * b->z);

   *out = r;
}

static inline void
evas_vec4_quaternion_inverse(Evas_Vec4 *out, const Evas_Vec4 *q)
{
   Evas_Real norm = (q->x * q->x) + (q->y * q->y) + (q->z * q->z) + (q->w * q->w);

   if (norm > 0.0)
     {
        Evas_Real inv_norm = 1.0 / norm;
        out->x = -q->x * inv_norm;
        out->y = -q->y * inv_norm;
        out->z = -q->z * inv_norm;
        out->w =  q->w * inv_norm;
     }
   else
     {
        out->x = 0.0;
        out->y = 0.0;
        out->z = 0.0;
        out->w = 0.0;
     }
}

static inline void
evas_vec4_quaternion_rotation_matrix_get(const Evas_Vec4 *q, Evas_Mat3 *mat)
{
   Evas_Real x, y, z;
   Evas_Real xx, xy, xz;
   Evas_Real yy, yz;
   Evas_Real zz;
   Evas_Real wx, wy, wz;

   x = 2.0 * q->x;
   y = 2.0 * q->y;
   z = 2.0 * q->z;

   xx = q->x * x;
   xy = q->x * y;
   xz = q->x * z;

   yy = q->y * y;
   yz = q->y * z;

   zz = q->z * z;

   wx = q->w * x;
   wy = q->w * y;
   wz = q->w * z;

   mat->m[0] = 1.0 - yy - zz;
   mat->m[1] = xy + wz;
   mat->m[2] = xz - wy;
   mat->m[3] = xy - wz;
   mat->m[4] = 1.0 - xx - zz;
   mat->m[5] = yz + wx;
   mat->m[6] = xz + wy;
   mat->m[7] = yz - wx;
   mat->m[8] = 1.0 - xx - yy;
}

static inline void
evas_mat4_build(Evas_Mat4 *out,
                const Evas_Vec3 *position, const Evas_Vec4 *orientation, const Evas_Vec3 *scale)
{
   Evas_Mat3  rot;

   evas_vec4_quaternion_rotation_matrix_get(orientation, &rot);

   out->m[ 0] = scale->x * rot.m[0];
   out->m[ 1] = scale->x * rot.m[1];
   out->m[ 2] = scale->x * rot.m[2];
   out->m[ 3] = 0.0;

   out->m[ 4] = scale->y * rot.m[3];
   out->m[ 5] = scale->y * rot.m[4];
   out->m[ 6] = scale->y * rot.m[5];
   out->m[ 7] = 0.0;

   out->m[ 8] = scale->z * rot.m[6];
   out->m[ 9] = scale->z * rot.m[7];
   out->m[10] = scale->z * rot.m[8];
   out->m[11] = 0.0;

   out->m[12] = position->x;
   out->m[13] = position->y;
   out->m[14] = position->z;
   out->m[15] = 1.0;
}

static inline void
evas_mat4_inverse_build(Evas_Mat4 *out, const Evas_Vec3 *position,
                        const Evas_Vec4 *orientation, const Evas_Vec3 *scale)
{
   Evas_Vec4   inv_rotation;
   Evas_Vec3   inv_scale;
   Evas_Vec3   inv_translate;

   Evas_Mat3   rot;

   /* Inverse scale. */
   evas_vec3_set(&inv_scale, 1.0 / scale->x, 1.0 / scale->y, 1.0 / scale->z);

   /* Inverse rotation. */
   evas_vec4_quaternion_inverse(&inv_rotation, orientation);

   /* Inverse translation. */
   evas_vec3_negate(&inv_translate, position);
   evas_vec3_quaternion_rotate(&inv_translate, &inv_translate, &inv_rotation);
   evas_vec3_multiply(&inv_translate, &inv_translate, &inv_scale);

   /* Get 3x3 rotation matrix. */
   evas_vec4_quaternion_rotation_matrix_get(&inv_rotation, &rot);

   out->m[ 0] = inv_scale.x * rot.m[0];
   out->m[ 1] = inv_scale.y * rot.m[1];
   out->m[ 2] = inv_scale.z * rot.m[2];
   out->m[ 3] = 0.0;

   out->m[ 4] = inv_scale.x * rot.m[3];
   out->m[ 5] = inv_scale.y * rot.m[4];
   out->m[ 6] = inv_scale.z * rot.m[5];
   out->m[ 7] = 0.0;

   out->m[ 8] = inv_scale.x * rot.m[6];
   out->m[ 9] = inv_scale.y * rot.m[7];
   out->m[10] = inv_scale.z * rot.m[8];
   out->m[11] = 0.0;

   out->m[12] = inv_translate.x;
   out->m[13] = inv_translate.y;
   out->m[14] = inv_translate.z;
   out->m[15] = 1.0;
}

static inline void
evas_color_set(Evas_Color *color, Evas_Real r, Evas_Real g, Evas_Real b, Evas_Real a)
{
   color->r = r;
   color->g = g;
   color->b = b;
   color->a = a;
}

static inline void
evas_color_blend(Evas_Color *dst, const Evas_Color *c0, const Evas_Color *c1, Evas_Real w)
{
   dst->r = c0->r * w + c1->r * (1.0 - w);
   dst->g = c0->g * w + c1->g * (1.0 - w);
   dst->b = c0->b * w + c1->b * (1.0 - w);
   dst->a = c0->a * w + c1->a * (1.0 - w);
}

static inline void
evas_ray3_init(Evas_Ray3 *ray, Evas_Real x, Evas_Real y, const Evas_Mat4 *mvp)
{
   Evas_Mat4   mat;
   Evas_Vec4   near, far;

   /* Get the matrix which transforms from normalized device coordinate to modeling coodrinate. */
   evas_mat4_inverse(&mat, mvp);

   /* Transform near point. */
   near.x =    x;
   near.y =    y;
   near.z = -1.0;
   near.w =  1.0;

   evas_vec4_transform(&near, &near, &mat);

   near.w = 1.0 / near.w;
   near.x *= near.w;
   near.y *= near.w;
   near.z *= near.w;

   evas_vec3_set(&ray->org, near.x, near.y, near.z);

   /* Transform far point. */
   far.x =     x;
   far.y =     y;
   far.z =   1.0;
   far.w =   1.0;

   evas_vec4_transform(&far, &far, &mat);

   far.w = 1.0 / far.w;
   far.x *= far.w;
   far.y *= far.w;
   far.z *= far.w;

   evas_vec3_set(&ray->dir, far.x - near.x, far.y - near.y, far.z - near.z);
}

static inline Eina_Bool
evas_box3_ray3_intersect(const Evas_Box3 *box EINA_UNUSED, const Evas_Ray3 *ray EINA_UNUSED)
{
   /* TODO: */
   return EINA_TRUE;
}

static inline Evas_Real
evas_reciprocal_sqrt(Evas_Real x)
{
   union {
        float f;
        long  i;
   } u;

   u.f = x;
   u.i = 0x5f3759df - (u.i >> 1);
   return u.f * (1.5f - u.f * u.f * x * 0.5f);
}
