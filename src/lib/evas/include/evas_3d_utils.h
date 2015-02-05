#ifndef EVAS_PRIVATE_H
# error You shall not include this header directly
#endif

#include <math.h>
#include <float.h>

#define  DEGREE_TO_RADIAN(x)     (((x) * M_PI) / 180.0)
#define  EVAS_MATRIX_IS_IDENTITY 0x00000001

typedef struct _Evas_Color Evas_Color;
typedef struct _Evas_Vec2 Evas_Vec2;
typedef struct _Evas_Vec3 Evas_Vec3;
typedef struct _Evas_Vec4 Evas_Vec4;
typedef struct _Evas_Mat2 Evas_Mat2;
typedef struct _Evas_Mat3 Evas_Mat3;
typedef struct _Evas_Mat4 Evas_Mat4;
typedef struct _Evas_Box2 Evas_Box2;
typedef struct _Evas_Box3 Evas_Box3;
typedef struct _Evas_Line3 Evas_Line3;
typedef struct _Evas_Triangle3 Evas_Triangle3;
typedef struct _Evas_Ray3 Evas_Ray3;
typedef struct _Evas_Sphere Evas_Sphere;

struct _Evas_Color
{
   Evas_Real   r;
   Evas_Real   g;
   Evas_Real   b;
   Evas_Real   a;
};

struct _Evas_Vec2
{
   Evas_Real   x;
   Evas_Real   y;
};

struct _Evas_Vec3
{
   Evas_Real   x;
   Evas_Real   y;
   Evas_Real   z;
};

struct _Evas_Vec4
{
   Evas_Real   x;
   Evas_Real   y;
   Evas_Real   z;
   Evas_Real   w;
};

struct _Evas_Mat2
{
   Evas_Real   m[4];
   int         flags;
};

struct _Evas_Mat3
{
   Evas_Real   m[9];
   int         flags;
};

struct _Evas_Mat4
{
   Evas_Real   m[16];
   int         flags;
};

struct _Evas_Box2
{
   Evas_Vec2   p0;
   Evas_Vec2   p1;
};

struct _Evas_Box3
{
   Evas_Vec3   p0;
   Evas_Vec3   p1;
};

struct _Evas_Line3
{
   Evas_Vec3   point;
   Evas_Vec3   direction;
};

struct _Evas_Triangle3
{
   Evas_Vec3   p0;
   Evas_Vec3   p1;
   Evas_Vec3   p2;
};

struct _Evas_Ray3
{
   Evas_Vec3   org;
   Evas_Vec3   dir;
};

struct _Evas_Sphere
{
   Evas_Vec3 center;
   Evas_Real radius;
};

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
evas_mat4_nocheck_multiply(Evas_Mat4 *out, const Evas_Mat4 *mat_a,
                           const Evas_Mat4 *mat_b)
{
   Evas_Real *d = out->m;
   const Evas_Real *a = mat_a->m;
   const Evas_Real *b = mat_b->m;

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
evas_mat4_multiply(Evas_Mat4 *out, const Evas_Mat4 *mat_a,
                   const Evas_Mat4 *mat_b)
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
                      Evas_Real dnear, Evas_Real dfar)
{
   Evas_Real   w = right - left;
   Evas_Real   h = top - bottom;
   Evas_Real   depth = dnear - dfar;
   Evas_Real   near_2 = 2.0f * dnear;

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
   m->m[10] = (dfar + dnear) / depth;
   m->m[11] = -1.0f;

   m->m[12] = 0.0f;
   m->m[13] = 0.0f;
   m->m[14] = near_2 * dfar / depth;
   m->m[15] = 0.0f;

   m->flags = 0;
}

static inline void
evas_mat4_ortho_set(Evas_Mat4 *m,
                    Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top,
                    Evas_Real dnear, Evas_Real dfar)
{
   Evas_Real   w = right - left;
   Evas_Real   h = top - bottom;
   Evas_Real   depth = dnear - dfar;

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
   m->m[14] = (dfar + dnear) / depth;
   m->m[15] = 1.0f;

   m->flags = 0;
}

static inline void
evas_mat4_nocheck_inverse(Evas_Mat4 *out, const Evas_Mat4 *mat)
{
   Evas_Real *d = out->m;
   const Evas_Real *m = mat->m;
   Evas_Real det;

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

   if (det == 0.0) return;

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
   Evas_Real *d = out->m;
   const Evas_Real *a = mat_a->m;
   const Evas_Real *b = mat_b->m;

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
evas_box2_set(Evas_Box2 *box, Evas_Real x0, Evas_Real y0, Evas_Real x1,
              Evas_Real y1)
{
   box->p0.x = x0;
   box->p0.y = y0;
   box->p1.x = x1;
   box->p1.y = y1;
}

static inline void
evas_box3_set(Evas_Box3 *box, Evas_Real x0, Evas_Real y0, Evas_Real z0,
              Evas_Real x1, Evas_Real y1, Evas_Real z1)
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

   out->flags = 0;
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

   out->flags = 0;
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
   Evas_Mat4 mat;
   Evas_Vec4 dnear, dfar;

   memset(&mat, 0, sizeof (mat));

   /* Get the matrix which transforms from normalized device coordinate to
      modeling coodrinate. */
   evas_mat4_inverse(&mat, mvp);

   /* Transform near point. */
   dnear.x = x;
   dnear.y = y;
   dnear.z = -1.0;
   dnear.w = 1.0;

   evas_vec4_transform(&dnear, &dnear, &mat);

   dnear.w = 1.0 / dnear.w;
   dnear.x *= dnear.w;
   dnear.y *= dnear.w;
   dnear.z *= dnear.w;

   evas_vec3_set(&ray->org, dnear.x, dnear.y, dnear.z);

   /* Transform far point. */
   dfar.x = x;
   dfar.y = y;
   dfar.z = 1.0;
   dfar.w = 1.0;

   evas_vec4_transform(&dfar, &dfar, &mat);

   dfar.w = 1.0 / dfar.w;
   dfar.x *= dfar.w;
   dfar.y *= dfar.w;
   dfar.z *= dfar.w;

   evas_vec3_set(&ray->dir, dfar.x - dnear.x, dfar.y - dnear.y, dfar.z - dnear.z);
}

static inline Eina_Bool
evas_box2_intersect_2d(const Evas_Box2 *box, const Evas_Vec2 *org, const Evas_Vec2 *dir)
{
   Evas_Real t1, t2, t_near = FLT_MIN, t_far = FLT_MAX;
   /* ray intersects box if its begins in */
   if ((org->x >= box->p0.x) && (org->x <= box->p1.x) &&
       (org->y >= box->p0.y) && (org->y <= box->p1.y))
     {
        return EINA_TRUE;
     }
   /* minmax algorithm of ray and box intersection */
   if ((dir->x != 0.0f) && (dir->y != 0.0f))
     {
        t1 = (box->p0.x - org->x) / dir->x;
        t2 = (box->p1.x - org->x) / dir->x;

        if (t1 > t2)
          {
             Evas_Real tmp = t1;
             t1 = t2;
             t2 = tmp;
          }

        if (t1 > t_near) t_near = t1;
        if (t2 < t_far) t_far = t2;

        if (t_far < 0.0f)
          return EINA_FALSE;

        t1 = (box->p0.y - org->y) / dir->y;
        t2 = (box->p1.y - org->y) / dir->y;

        if (t1 > t2)
          {
             Evas_Real tmp = t1;
             t1 = t2;
             t2 = tmp;
          }

        if (t1 > t_near) t_near = t1;
        if (t2 < t_far) t_far = t2;

        if ((t_near > t_far) || (t_far < 0.0f))
          return EINA_FALSE;
     }
   /* case when ray is parallel to one of axes */
   else if (dir->x == 0.0f)
     {
        if ((org->x < box->p0.x) && (org->x > box->p1.x))
          return EINA_FALSE;
     }
   else if (org->y < box->p0.y && org->y > box->p1.y)
      return EINA_FALSE;

   return EINA_TRUE;
}

static inline Evas_Real
evas_determinant_3D(Evas_Real matrix[3][3])
{
   return (matrix[0][0] * matrix[1][1] * matrix[2][2]) +
          (matrix[0][1] * matrix[1][2] * matrix[2][0]) +
          (matrix[0][2] * matrix[1][0] * matrix[2][1]) -
          (matrix[0][2] * matrix[1][1] * matrix[2][0]) -
          (matrix[0][1] * matrix[1][0] * matrix[2][2]) -
          (matrix[0][0] * matrix[1][2] * matrix[2][1]);
}

static inline Eina_Bool
evas_box3_ray3_intersect(const Evas_Box3 *box, const Evas_Ray3 *ray)
{
   Evas_Real t1, t2, t_near = FLT_MIN, t_far = FLT_MAX;
   Evas_Box2 box2;
   Evas_Vec2 org2;
   Evas_Vec2 dir2;
   Eina_Bool intersect = EINA_FALSE;

   /* ray intersects box if its begins in */
   if ((ray->org.x >= box->p0.x) && (ray->org.x <= box->p1.x) &&
       (ray->org.y >= box->p0.y) && (ray->org.y <= box->p1.y) &&
       (ray->org.z >= box->p0.z) && (ray->org.z <= box->p1.z))
     {
        return EINA_TRUE;
     }
   /* minmax algorithm of ray and box intersection */
   if ((ray->dir.x != 0.0f) && (ray->dir.y != 0.0f) && (ray->dir.z != 0.0f))
     {
        t1 = (box->p0.x - ray->org.x) / ray->dir.x;
        t2 = (box->p1.x - ray->org.x) / ray->dir.x;

        if (t1 > t2)
          {
             Evas_Real tmp = t1;
             t1 = t2;
             t2 = tmp;
          }

        if (t1 > t_near) t_near = t1;
        if (t2 < t_far) t_far = t2;

        if (t_far < 0.0f)
          return EINA_FALSE;

        t1 = (box->p0.y - ray->org.y) / ray->dir.y;
        t2 = (box->p1.y - ray->org.y) / ray->dir.y;

        if (t1 > t2)
          {
             Evas_Real tmp = t1;
             t1 = t2;
             t2 = tmp;
          }

        if (t1 > t_near) t_near = t1;
        if (t2 < t_far) t_far = t2;

        if ((t_near > t_far) || (t_far < 0.0f))
          return EINA_FALSE;

        t1 = (box->p0.z - ray->org.z) / ray->dir.z;
        t2 = (box->p1.z - ray->org.z) / ray->dir.z;

        if (t1 > t2)
          {
             Evas_Real tmp = t1;
             t1 = t2;
             t2 = tmp;
          }

        if (t1 > t_near) t_near = t1;
        if (t2 < t_far) t_far = t2;

        if ((t_near > t_far) || (t_far < 0.0f))
          return EINA_FALSE;

        intersect = EINA_TRUE;
     }
   /* case when ray is parallel to one of axes */
   else
     {
     /* use two-dimensional version here */
        if (ray->dir.x == 0.0f)
          {
             if ((ray->org.x < box->p0.x) || (ray->org.x > box->p1.x))
               return EINA_FALSE;
             else
               {
                  evas_vec2_set(&org2, ray->org.y, ray->org.z);
                  evas_vec2_set(&dir2, ray->dir.y, ray->dir.z);
                  evas_box2_set(&box2, box->p0.y, box->p0.z, box->p1.y, box->p1.z);
                  intersect = evas_box2_intersect_2d(&box2, &org2, &dir2);
               }
          }

        if (ray->dir.y == 0.0f)
          {
             if ((ray->org.y < box->p0.y) || (ray->org.y > box->p1.y))
               return EINA_FALSE;
             else
               {
                  evas_vec2_set(&org2, ray->org.x, ray->org.z);
                  evas_vec2_set(&dir2, ray->dir.x, ray->dir.z);
                  evas_box2_set(&box2, box->p0.x, box->p0.z, box->p1.x, box->p1.z);
                  intersect = evas_box2_intersect_2d(&box2, &org2, &dir2);
               }
          }

        if (ray->dir.z == 0.0f)
          {
             if (ray->org.z < box->p0.z || ray->org.z > box->p1.z)
               return EINA_FALSE;
             else
               {
                  evas_vec2_set(&org2, ray->org.x, ray->org.y);
                  evas_vec2_set(&dir2, ray->dir.x, ray->dir.y);
                  evas_box2_set(&box2, box->p0.x, box->p0.y, box->p1.x, box->p1.y);
                  intersect = evas_box2_intersect_2d(&box2, &org2, &dir2);
               }
          }
     }

   return intersect;
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

static inline void
evas_build_sphere(const Evas_Box3 *box, Evas_Sphere *sphere)
{
   Evas_Vec3 tmp;

   evas_vec3_set(&sphere->center, (0.5 * (box->p0.x + box->p1.x)), (0.5 * (box->p0.y + box->p1.y)), (0.5 * (box->p0.z + box->p1.z)));
   evas_vec3_set(&tmp, sphere->center.x - box->p0.x, sphere->center.y - box->p0.y, sphere->center.z - box->p0.z);

   sphere->radius = sqrtf(evas_vec3_dot_product(&tmp, &tmp));
}

static inline void
evas_plane_normalize(Evas_Vec4 *plane)
{
   Evas_Vec3 tmp;
   Evas_Real length;
   evas_vec3_set(&tmp, plane->x, plane->y, plane->z);
   length = evas_vec3_length_get(&tmp);
   plane->x = plane->x / length;
   plane->y = plane->y / length;
   plane->z = plane->z / length;
   plane->w = plane->w / length;
}

static inline Eina_Bool
evas_intersection_line_of_two_planes(Evas_Line3 *line, Evas_Vec4 *plane1, Evas_Vec4 *plane2)
{
   //TODO:parallel case
   Evas_Vec3 planes3D[2];

   evas_vec3_set(&planes3D[0], plane1->x, plane1->y, plane1->z);
   evas_vec3_set(&planes3D[1], plane2->x, plane2->y, plane2->z);

   evas_vec3_cross_product(&line->direction, &planes3D[0], &planes3D[1]);

#define SOLVE_EQUATION(x, y, z) \
   line->point.x = 0; \
   line->point.y = (plane2->w * plane1->z - plane1->w * plane2->z) / line->direction.x; \
   line->point.z = (plane2->y * plane1->w - plane1->y * plane2->w) / line->direction.x;

   if (line->direction.x && plane1->z)
     {
        SOLVE_EQUATION(x, y, z)
     }
   else if (line->direction.y && plane1->x)
     {
        SOLVE_EQUATION(y, z, x)
     }
   else
     {
        SOLVE_EQUATION(z, x, y)
     }
#undef SOLVE_EQUATION

   return EINA_TRUE;
}

static inline Eina_Bool
evas_intersection_point_of_three_planes(Evas_Vec3 *point, Evas_Vec4 *plane1, Evas_Vec4 *plane2, Evas_Vec4 *plane3)
{
   //TODO:parallel case
   int i;
   Evas_Real delta, deltax, deltay, deltaz;
   Evas_Real matrix_to_det[3][3];
   Evas_Vec4 planes[3];

   planes[0] = *plane1;
   planes[1] = *plane2;
   planes[2] = *plane3;

   for (i = 0; i < 3; i++)
     {
        matrix_to_det[0][i] = planes[i].x;
        matrix_to_det[1][i] = planes[i].y;
        matrix_to_det[2][i] = planes[i].z;
     }
   delta = evas_determinant_3D(matrix_to_det);

   for (i = 0; i < 3; i++)
     matrix_to_det[0][i] = planes[i].w;
   deltax = evas_determinant_3D(matrix_to_det);

   for (i = 0; i < 3; i++)
     {
        matrix_to_det[0][i] = planes[i].x;
        matrix_to_det[1][i] = planes[i].w;
     }
   deltay = evas_determinant_3D(matrix_to_det);

   for (i = 0; i < 3; i++)
     {
        matrix_to_det[1][i] = planes[i].y;
        matrix_to_det[2][i] = planes[i].w;
     }
   deltaz = evas_determinant_3D(matrix_to_det);

   evas_vec3_set(point, -deltax/delta, -deltay/delta, -deltaz/delta);

   return EINA_TRUE;
}

static inline Evas_Real
evas_point_plane_distance(Evas_Vec3 *point, Evas_Vec4 *plane)
{
   return plane->x * point->x + plane->y * point->y + plane->z * point->z + plane->w;
}

static inline Evas_Real
evas_point_line_distance(Evas_Vec3 *point, Evas_Line3 *line)
{
   Evas_Vec3 temp, sub;

   evas_vec3_subtract(&sub, point, &line->point);
   evas_vec3_cross_product(&temp, &sub, &line->direction);

   return evas_vec3_length_get(&temp) / evas_vec3_length_get(&line->direction);
}

static inline Eina_Bool
evas_is_sphere_in_frustum(Evas_Sphere *bsphere, Evas_Vec4 *planes)
{
   int i;
   Evas_Line3 line;
   Evas_Vec3 point, sub;
   Evas_Real distances[6] = {0};
   int intersected_planes[3];
   int intersected_planes_count = 0;

   for (i = 0; i < 6; i++)
     distances[i] = evas_point_plane_distance(&bsphere->center, &planes[i]);

   for (i = 0; i < 6; i++)
     {
        if (distances[i] <= -bsphere->radius)
          {
             return EINA_FALSE;
          }
        else if (distances[i] <= 0)
          {
             intersected_planes[intersected_planes_count] = i;
             intersected_planes_count++;
          }
      }

   switch (intersected_planes_count)
     {
      case 2:
        evas_intersection_line_of_two_planes(&line,
                                             &planes[intersected_planes[0]],
                                             &planes[intersected_planes[1]]);
        return (evas_point_line_distance(&bsphere->center, &line) <
                bsphere->radius) ? EINA_TRUE : EINA_FALSE;
      case 3:
        evas_intersection_point_of_three_planes(&point,
                                                &planes[intersected_planes[0]],
                                                &planes[intersected_planes[1]],
                                                &planes[intersected_planes[2]]);
        evas_vec3_subtract(&sub, &point, &bsphere->center);
        return (evas_vec3_length_get(&sub) < bsphere->radius) ? EINA_TRUE : EINA_FALSE;
      default:
        return EINA_TRUE;
     }
}

static inline Eina_Bool
evas_is_point_in_frustum(Evas_Vec3 *point, Evas_Vec4 *planes)
{
   int i;
   for (i = 0; i < 6; i++)
     if (evas_point_plane_distance(point, &planes[i]) <= 0) return EINA_FALSE;
   return EINA_TRUE;
}

static inline Eina_Bool
evas_is_box_in_frustum(Evas_Box3 *box, Evas_Vec4 *planes)
{
   int i;
   for (i = 0; i < 6; i++)
     {
        if (planes[i].x * box->p0.x + planes[i].y * box->p0.y + planes[i].z * box->p0.z + planes[i].w > 0)
          continue;
        if (planes[i].x * box->p1.x + planes[i].y * box->p0.y + planes[i].z * box->p0.z + planes[i].w > 0)
          continue;
        if (planes[i].x * box->p1.x + planes[i].y * box->p1.y + planes[i].z * box->p0.z + planes[i].w > 0)
          continue;
        if (planes[i].x * box->p0.x + planes[i].y * box->p1.y + planes[i].z * box->p0.z + planes[i].w > 0)
          continue;
        if (planes[i].x * box->p0.x + planes[i].y * box->p0.y + planes[i].z * box->p1.z + planes[i].w > 0)
          continue;
        if (planes[i].x * box->p1.x + planes[i].y * box->p0.y + planes[i].z * box->p1.z + planes[i].w > 0)
          continue;
        if (planes[i].x * box->p1.x + planes[i].y * box->p1.y + planes[i].z * box->p1.z + planes[i].w > 0)
          continue;
        if (planes[i].x * box->p0.x + planes[i].y * box->p1.y + planes[i].z * box->p1.z + planes[i].w > 0)
          continue;
       return EINA_FALSE;
     }

   return EINA_TRUE;
}

static inline void
evas_frustum_calculate(Evas_Vec4 *planes, Evas_Mat4 *matrix_vp)
{
   int i;
   evas_vec4_set(&planes[0], matrix_vp->m[3] - matrix_vp->m[0],
                             matrix_vp->m[7] - matrix_vp->m[4],
                             matrix_vp->m[11] - matrix_vp->m[8],
                             matrix_vp->m[15] - matrix_vp->m[12]);

   evas_vec4_set(&planes[1], matrix_vp->m[3] + matrix_vp->m[0],
                             matrix_vp->m[7] + matrix_vp->m[4],
                             matrix_vp->m[11] + matrix_vp->m[8],
                             matrix_vp->m[15] + matrix_vp->m[12]);

   evas_vec4_set(&planes[2], matrix_vp->m[3] + matrix_vp->m[1],
                             matrix_vp->m[7] + matrix_vp->m[5],
                             matrix_vp->m[11] + matrix_vp->m[9],
                             matrix_vp->m[15] + matrix_vp->m[13]);

   evas_vec4_set(&planes[3], matrix_vp->m[3] - matrix_vp->m[1],
                             matrix_vp->m[7] - matrix_vp->m[5],
                             matrix_vp->m[11] - matrix_vp->m[9],
                             matrix_vp->m[15] - matrix_vp->m[13]);

   evas_vec4_set(&planes[4], matrix_vp->m[3] - matrix_vp->m[2],
                             matrix_vp->m[7] - matrix_vp->m[6],
                             matrix_vp->m[11] - matrix_vp->m[10],
                             matrix_vp->m[15] - matrix_vp->m[14]);

   evas_vec4_set(&planes[5], matrix_vp->m[3] + matrix_vp->m[2],
                             matrix_vp->m[7] + matrix_vp->m[6],
                             matrix_vp->m[11] + matrix_vp->m[10],
                             matrix_vp->m[15] + matrix_vp->m[14]);
   for (i = 0; i < 6; i++)
     {
       evas_plane_normalize(&planes[i]);
     }
}
