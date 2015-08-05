#ifndef EVAS_PRIVATE_H
# error You shall not include this header directly
#endif

#include <math.h>
#include <float.h>

#define  DEGREE_TO_RADIAN(x)     (((x) * M_PI) / 180.0)
#define  EVAS_MATRIX_IS_IDENTITY 0x00000001
#define  MIN_DIFF 0.00000000001

#define  FLT_COMPARISON(a, b)    \
   (fabs(a - b) > FLT_EPSILON)

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

static inline Evas_Real
evas_vec3_angle_get(const Evas_Vec3 *a, const Evas_Vec3 *b)
{
   Evas_Real angle;

   angle = evas_vec3_dot_product(a, b) / (evas_vec3_length_get(a) * evas_vec3_length_get(b));
   return angle;
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

static inline void
evas_vec3_orthogonal_projection_on_plain(Evas_Vec3 *out, const Evas_Vec3 *v, const Evas_Vec3 *normal)
{
   Evas_Real a;
   Evas_Vec3 projection;

   /* Orthoprojection of vector on the plane is the difference
      between a vector and its orthogonal projection onto the orthogonal
      complement to the plane */
   a = evas_vec3_dot_product(v, normal) / evas_vec3_length_square_get(normal);
   evas_vec3_scale(&projection, normal, a);
   evas_vec3_subtract(out, v, &projection);

   return;
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
evas_vec4_plain_by_points(Evas_Vec4 *out, const Evas_Vec3 *a, const Evas_Vec3 *b, const Evas_Vec3 *c)
{
   out->x = (b->y - a->y) * (c->z - a->z) - (b->z - a->z) * (c->y - a->y);
   out->y = -(b->x - a->x) * (c->z - a->z) + (b->z - a->z) * (c->x - a->x);
   out->z = (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
   out->w = (-a->x) * ((b->y - a->y)*(c->z - a->z) - (b->z - a->z) * (c->y - a->y)) -
            (-a->y) * ((b->x - a->x) * (c->z - a->z) - (b->z - a->z) * (c->x - a->x)) +
            (-a->z) * ((b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x));
}

static inline Evas_Real
evas_vec4_angle_plains(Evas_Vec4 *a, Evas_Vec4 *b)
{
   return (Evas_Real) ((a->x * b->x) + (a->y * b->y) + (a->z * b->z)) / ((sqrt((a->x * a->x) +
                       (a->y * a->y) + (a->z * a->z))) * (sqrt((b->x * b->x) + (b->y * b->y) +
                       (b->z * b->z))));
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

static inline Eina_Bool
evas_vec3_if_equivalent(Evas_Vec3 *a, const Evas_Vec3 *b)
{
   /* Assume "v" is a directional vector. (v->w == 0.0) */
   return ((a->x == b->x) &&  (a->y == b->y) && (a->z == b->z));
}

static inline void
evas_triangle3_set(Evas_Triangle3 *v, Evas_Vec3 *a, Evas_Vec3 *b, Evas_Vec3 *c)
{
   evas_vec3_copy(&v->p0, a);
   evas_vec3_copy(&v->p1, b);
   evas_vec3_copy(&v->p2, c);
}

static inline Eina_Bool
evas_triangle3_is_line(Evas_Triangle3 *v)
{
   if (evas_vec3_if_equivalent(&v->p0, &v->p1) ||
       evas_vec3_if_equivalent(&v->p0, &v->p2) ||
       evas_vec3_if_equivalent(&v->p1, &v->p2))
     return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
convex_hull_triangle3_if_not_first_edje(Evas_Triangle3 *v, Evas_Vec3 *a, Evas_Vec3 *b)
{
   if (((v->p1.x == a->x) && (v->p1.y == a->y) && (v->p1.z == a->z)) &&
       ((v->p2.x == b->x) && (v->p2.y == b->y) && (v->p2.z == b->z)))
     return EINA_TRUE;
   else if (((v->p2.x == a->x) && (v->p2.y == a->y) && (v->p2.z == a->z)) &&
            ((v->p1.x == b->x) && (v->p1.y == b->y) && (v->p1.z == b->z)))
     return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
convex_hull_triangle3_if_first_edje(Evas_Triangle3 *v, Evas_Vec3 *a, Evas_Vec3 *b)
{
   if ((!FLT_COMPARISON(v->p0.x, a->x) && !FLT_COMPARISON(v->p0.y, a->y) &&
        !FLT_COMPARISON(v->p0.z, a->z)) && (!FLT_COMPARISON(v->p1.x, b->x) &&
        !FLT_COMPARISON(v->p1.y, b->y) && !FLT_COMPARISON(v->p1.z, b->z)))
     return EINA_TRUE;
   else if ((!FLT_COMPARISON(v->p1.x, a->x) && !FLT_COMPARISON(v->p1.y, a->y) &&
             !FLT_COMPARISON(v->p1.z, a->z)) && (!FLT_COMPARISON(v->p0.x, b->x) &&
             !FLT_COMPARISON(v->p0.y, b->y) && !FLT_COMPARISON(v->p0.z, b->z)))
     return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
convex_hull_triangle3_if_first_point(Evas_Triangle3 *v, Evas_Vec3 *a)
{
   return ((v->p0.x == a->x) && (v->p0.y == a->y) && (v->p0.z == a->z));
}

static inline Eina_Bool
evas_vec3_if_equivalent_as_triangle(Evas_Vec3 *v0, Evas_Vec3 *v1, Evas_Vec3 *v2,
                                    Evas_Vec3 *w0, Evas_Vec3 *w1, Evas_Vec3 *w2)
{
   if (((v0->x == w0->x) && (v0->y == w0->y) && (v0->z == w0->z)) &&
       ((v1->x == w1->x) && (v1->y == w1->y) && (v1->z == w1->z)) &&
       ((v2->x == w2->x) && (v2->y == w2->y) && (v2->z == w2->z)))
     return EINA_TRUE;

   return EINA_FALSE;
}


static inline Eina_Bool
evas_triangle3_if_equivalent(Evas_Triangle3 *a, Evas_Triangle3 *b)
{
   /* to compare two triangles there are six permutations
      to test because vertices are unordered */
   if (evas_vec3_if_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p0, &b->p1, &b->p2) ||
       evas_vec3_if_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p0, &b->p2, &b->p1) ||
       evas_vec3_if_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p1, &b->p0, &b->p2) ||
       evas_vec3_if_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p1, &b->p2, &b->p0) ||
       evas_vec3_if_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p2, &b->p0, &b->p1) ||
       evas_vec3_if_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p2, &b->p1, &b->p0))
     return EINA_TRUE;

   return EINA_FALSE;
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

static inline void
evas_mat3_set_position_transform(Evas_Mat3 *out, const Evas_Real p_x,
								 const Evas_Real p_y)
{
   evas_mat3_identity_set(out);
   out->m[2] = p_x;
   out->m[5] = p_y;
   if ((fabs(p_x) > FLT_EPSILON) ||
       (fabs(p_y) > FLT_EPSILON))
     out->flags = 0;
}

static inline void
evas_mat3_set_scale_transform(Evas_Mat3 *out, Evas_Real s_x, Evas_Real s_y)
{
   evas_mat3_identity_set(out);
   out->m[0] = s_x;
   out->m[4] = s_y;
   if ((fabs(s_x - 1.0) > FLT_EPSILON) ||
       (fabs(s_y - 1.0) > FLT_EPSILON))
     out->flags = 0;
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
evas_sphere_empty_set(Evas_Sphere *dst)
{
    dst->radius = 0;
    dst->center.x = 0;
    dst->center.y = 0;
    dst->center.z = 0;
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

static inline Eina_Bool
box_intersection_box(Evas_Box3 *v1, Evas_Box3 *v2)
{
   if ((v1->p1.x < v2->p0.x) || (v1->p0.x > v2->p1.x)
       || (v1->p1.y < v2->p0.y) || (v1->p0.y > v2->p1.y)
       || (v1->p1.z < v2->p0.z) || (v1->p0.z > v2->p1.z))
     return EINA_FALSE;
   else
     return EINA_TRUE;
}

static inline void
tangent_new_basis(Evas_Vec3 *out, Evas_Triangle3 *triangle,
                  Evas_Vec2 *a, Evas_Vec2 *b, Evas_Vec2 *c)
{
   Evas_Vec2   new1, new2;
   Evas_Vec3   old1, old2;
   evas_vec3_set(out, 0, 0, 0);

   evas_vec2_subtract(&new1, b, a);
   evas_vec2_subtract(&new2, c, a);
   evas_vec3_subtract(&old1, &(triangle->p1), &(triangle->p0));
   evas_vec3_subtract(&old2, &(triangle->p2), &(triangle->p0));


   /* calculation of new basis(in system coordinates of texturing) by solution of system of equations */
   if (new2.y != 0)
     {
        evas_vec3_scale(&old2, &old2, (new1.y / new2.y));
        evas_vec2_scale(&new2, &new2, (new1.y / new2.y));

        evas_vec2_subtract(&new1, &new1, &new2);
        evas_vec3_subtract(&old1, &old1, &old2);

        evas_vec3_scale(out, &old1, 1 / new1.x);
     }

   else if (new1.y != 0)
     {
        evas_vec3_scale(&old1, &old1, (new2.y / new1.y));
        evas_vec2_scale(&new1, &new1, (new2.y / new1.y));

        evas_vec2_subtract(&new2, &new2, &new1);
        evas_vec3_subtract(&old2, &old2, &old1);

        evas_vec3_scale(out, &old2, 1 / new2.x);
     }

   return;
}

static inline void
convex_hull_vertex_set(Evas_Triangle3 *el, int *vertex_count, float **vertex,
                    unsigned short int **index, unsigned int k, int *leader, int coord)
{
   int color_coords, normal_coords;
   Evas_Vec3 vect;
   switch (coord)
     {
      case 0:
        vect = el->p0;
        break;
      case 1:
        vect = el->p1;
        break;
      case 2:
        vect = el->p2;
        break;
     }
   (*vertex_count)++;
   *vertex = (float*) realloc(*vertex, (10 * (*vertex_count)) * sizeof(float));

   (*vertex)[10 * (*vertex_count) - 10] = vect.x;
   (*vertex)[10 * (*vertex_count) - 9] = vect.y;
   (*vertex)[10 * (*vertex_count) - 8] = vect.z;
   /* set alpha canal */
   (*vertex)[10 * (*vertex_count) - 1] = 1.0;
   /* set color */
   for (color_coords = 2; color_coords < 5; color_coords++)
      (*vertex)[10 * (*vertex_count) - color_coords] = (float) rand() / RAND_MAX;
   /* set normal coords */
   for (normal_coords = 5; normal_coords < 8; normal_coords++)
      (*vertex)[10 * (*vertex_count) - normal_coords] = 1.0;
   (*index)[3 * k + coord] = *leader;
   (*leader)++;
}

static inline Evas_Triangle3
convex_hull_first_tr_get(float *data, int count, int stride)
{
   Evas_Triangle3  out;

   Evas_Vec3   triangle1;
   Evas_Vec3   triangle2, triangle2_candidate;
   Evas_Vec3   triangle3, triangle3_candidate;
   Evas_Vec3   first, second, complanar1, complanar2, candidate;
   Evas_Vec4   normal_a, normal_b;

   Evas_Real cos = 0.0, new_cos = 0.0, tan = 0.0, new_tan = 0.0, cos_2d = 0.0, new_cos_2d = 0.0;
   int first_num = 0, second_num = 0;
   int i = 0, j = 0;

   evas_vec3_set(&triangle1, data[0], data[1], data[2]);

   for (i = 1, j = stride; i < count; i++, j += stride)
      {
         if ((triangle1.z > data[j + 2]) ||
             ((triangle1.z == data[j + 2]) && (triangle1.y > data[j + 1])) ||
             ((triangle1.z == data[j + 2]) && (triangle1.y == data[j + 1]) && (triangle1.x > data[j])))
           {
              evas_vec3_set(&triangle1, data[j], data[j + 1], data[j + 2]);
              first_num = i;
           }
      }

   if (first_num)
     evas_vec3_set(&triangle2, data[0], data[1], data[2]);
   else
     {
        evas_vec3_set(&triangle2, data[3], data[4], data[5]);
        second_num = 1;
     }

   tan = fabs(triangle2.z - triangle1.z) / fabs(triangle2.y - triangle1.y);

#define COMPARE_ANGLES(trigonom, triangle, previous, big, little)     \
   if (little > big + FLT_EPSILON)                                    \
     {                                                                \
        trigonom = new_##trigonom;                                    \
        cos_2d = new_cos_2d;                                          \
        evas_vec3_set(&triangle, data[j], data[j + 1], data[j + 2]);   \
     }                                                                \
   else if(!FLT_COMPARISON(little, big) &&                            \
           (evas_vec3_distance_get(&triangle##_candidate, &previous) >  \
            evas_vec3_distance_get(&triangle, &previous)))              \
     {                                                                \
        evas_vec3_set(&triangle, data[j], data[j + 1], data[j + 2]);   \
     }
   evas_vec3_set(&complanar1, 1, 0, 0);
   for (i = 0, j = 0; i < count; i++, j += stride)
      {
         if (FLT_COMPARISON(data[j], triangle1.x) ||
             FLT_COMPARISON(data[j + 1], triangle1.y) ||
             FLT_COMPARISON(data[j + 2], triangle1.z))
           {
              new_tan = fabs(data[j + 2] - triangle1.z) / fabs(data[j + 1] - triangle1.y);

              if (FLT_COMPARISON(data[j + 1], triangle1.y) &&
                  FLT_COMPARISON(triangle2.y, triangle1.y))
                {
                   if (tan > new_tan + FLT_EPSILON)
                     {
                        tan = new_tan;
                        evas_vec3_set(&triangle2, data[j], data[j + 1], data[j + 2]);
                        second_num = i;
                        evas_vec3_subtract(&first, &complanar1, &triangle1);
                        evas_vec3_subtract(&second, &triangle2, &triangle1);
                        cos_2d = evas_vec3_angle_get(&complanar1, &second);
                     }
                   else if (!FLT_COMPARISON(tan, new_tan))
                     {
                        evas_vec3_subtract(&first, &complanar1, &triangle2);
                        evas_vec3_set(&triangle2_candidate, data[j], data[j + 1], data[j + 2]);
                        evas_vec3_subtract(&first, &complanar1, &triangle1);
                        evas_vec3_subtract(&second, &triangle2_candidate, &triangle1);
                        new_cos_2d = evas_vec3_angle_get(&complanar1, &second);
                        if (new_cos_2d > cos_2d + FLT_EPSILON)
                          second_num = i;

                        COMPARE_ANGLES(cos, triangle2, triangle1, cos_2d, new_cos_2d)
                     }
                }

              else if (!FLT_COMPARISON(data[j + 1], triangle1.y) &&
                       !FLT_COMPARISON(data[j + 2], triangle1.z) &&
                       FLT_COMPARISON(triangle2.y, triangle1.y))
                evas_vec3_set(&triangle2, data[j], data[j + 1], data[j + 2]);

              else if (!FLT_COMPARISON(data[j + 1], triangle1.y) &&
                       !FLT_COMPARISON(data[j + 2], triangle1.z) &&
                       !FLT_COMPARISON(triangle2.z, triangle1.z) &&
                       !FLT_COMPARISON(triangle2.y, triangle1.y))
                {
                   evas_vec3_set(&triangle2_candidate, data[j], data[j + 1], data[j + 2]);
                   if (evas_vec3_distance_get(&triangle2_candidate, &triangle1) >
                       evas_vec3_distance_get(&triangle2, &triangle1))
                     evas_vec3_set(&triangle2, data[j], data[j + 1], data[j + 2]);
                }
           }
      }

   evas_vec3_set(&complanar1, triangle1.x + 1, triangle1.y, triangle1.z);
   evas_vec3_set(&complanar2, triangle1.x, triangle1.y + 1, triangle1.z);
   evas_vec4_plain_by_points(&normal_a, &triangle1, &complanar1, &complanar2);

   if (normal_a.z < 0)
     evas_vec4_scale(&normal_a, &normal_a, -1);

   evas_vec3_set(&triangle3, data[0], data[1], data[2]);

   cos = -1.0;
   cos_2d = 1.0;

   for (i = 0, j = 0; i < count; i++, j += stride)
      {
         if ((i != first_num) && (i != second_num))
           {
              evas_vec3_set(&candidate, data[j], data[j + 1], data[j + 2]);
              evas_vec4_plain_by_points(&normal_b, &triangle1, &candidate, &triangle2);

              if (normal_b.z < 0)
                evas_vec4_scale(&normal_b, &normal_b, -1);

              new_cos = evas_vec4_angle_plains(&normal_a, &normal_b);

              if (new_cos > cos + FLT_EPSILON)
                {
                   evas_vec3_set(&triangle3_candidate, data[j], data[j + 1], data[j + 2]);
                   evas_vec3_subtract(&first, &triangle2, &triangle1);
                   evas_vec3_subtract(&second, &triangle3, &triangle1);
                   cos = new_cos;
                   evas_vec3_set(&triangle3, data[j], data[j + 1], data[j + 2]);
                   cos_2d = evas_vec3_angle_get(&second, &first);
                }
              else if (!FLT_COMPARISON(new_cos, cos))
                {
                   evas_vec3_set(&triangle3_candidate, data[j], data[j + 1], data[j + 2]);
                   evas_vec3_subtract(&first, &triangle1, &triangle2);
                   evas_vec3_subtract(&second, &triangle3_candidate, &triangle2);
                   new_cos_2d = evas_vec3_angle_get(&first, &second);

                   COMPARE_ANGLES(tan, triangle3, triangle2, new_cos_2d, cos_2d)
                }
           }
      }

   evas_triangle3_set(&out, &triangle1, &triangle2, &triangle3);

#undef COMPARE_ANGLES
   return out;
}

static inline void
evas_convex_hull_get(float *data, int count, int stride, float **vertex,
                     unsigned short int **index, int *vertex_count, int *index_count)
{
   Evas_Triangle3  first_elem, second_elem, *third_elem = NULL, *el = NULL;

   Evas_Vec3   *next = NULL, *best = NULL, *next_2d = NULL, *el_vec3 = NULL;
   Evas_Vec3   tmp1, tmp2;
   Evas_Vec4   normal_a, normal_b;

   Eina_Array arr_elems;
   Eina_Array arr_triangles;
   Eina_Array arr_candidates;
   Eina_Array arr_ch;
   Eina_Array_Iterator iterator;

   Evas_Real cos = 0.0, new_cos = 0.0, cos_2d = 0.0;
   float *found_vertex = NULL;
   int i = 0, j = 0, new_stride = 0, leader = 0;
   int if_two = 0, first_exist_twice = 0, second_exist_twice = 0;
   unsigned int k = 0;
   unsigned short int *found_index = NULL;

   Eina_Bool exist1 = EINA_FALSE, pushed;
   Eina_Bool equivalent_triangle = EINA_FALSE, triangle_chain = EINA_FALSE;
   Eina_Bool on_plain = EINA_FALSE, right = EINA_FALSE;

   eina_array_step_set(&arr_elems, sizeof(Eina_Array), 1);
   eina_array_step_set(&arr_triangles, sizeof(Eina_Array), 1);
   eina_array_step_set(&arr_candidates, sizeof(Eina_Array), 1);
   eina_array_step_set(&arr_ch, sizeof(Eina_Array), 1);

   /* Finding of first triangle in convex hull */
   first_elem = convex_hull_first_tr_get(data, count, stride);

   eina_array_push(&arr_triangles,  &first_elem);
   eina_array_push(&arr_elems,  &first_elem);

   evas_triangle3_set(&second_elem, &first_elem.p1, &first_elem.p2, &first_elem.p0);
   eina_array_push(&arr_elems,  &second_elem);
   eina_array_push(&arr_triangles,  &second_elem);

   third_elem = malloc(sizeof(Evas_Triangle3));
   evas_triangle3_set(third_elem, &first_elem.p2, &first_elem.p0, &first_elem.p1);
   eina_array_push(&arr_elems, third_elem);
   eina_array_push(&arr_triangles, third_elem);
   eina_array_push(&arr_ch, third_elem);

   el = eina_array_data_get(&arr_elems, 0);

   /* arr_ellems is an array of triangles, in fact it is a queue of edjes
      because vertices in triangles are ordered, every edje in this queue
      should have a conjugate edje in some other triangle */
   while (eina_array_count(&arr_elems) > 0)
     {
        Evas_Triangle3 *new_elem1 = NULL, *new_elem2 = NULL;

        Evas_Triangle3 *elem = eina_array_pop(&arr_elems);

        cos = -1.0;

        /* searching of next triangle in convex hull as given conjugate edje
           and one new vertex, all vertices should be checked */
        for (i = 0, j = 0; i < count; i++, j += stride)
           {
              if ((FLT_COMPARISON(elem->p0.x, data[j]) || FLT_COMPARISON(elem->p0.y, data[j + 1]) ||
                   FLT_COMPARISON(elem->p0.z, data[j + 2])) && (FLT_COMPARISON(elem->p1.x, data[j]) ||
                   FLT_COMPARISON(elem->p1.y, data[j + 1]) || FLT_COMPARISON(elem->p1.z, data[j + 2])) &&
                  (FLT_COMPARISON(elem->p2.x, data[j]) || FLT_COMPARISON(elem->p2.y, data[j + 1]) ||
                   FLT_COMPARISON(elem->p2.z, data[j + 2])))
                {
                   next = malloc(sizeof(Evas_Vec3));
                   evas_vec3_set(next, data[j], data[j + 1], data[j + 2]);
                   pushed = EINA_FALSE;

                   /* something like the dihedral angle between the triangles
                      is a determining factor in searching the necessary points */

                   evas_vec4_plain_by_points(&normal_a, &elem->p0, &elem->p1, &elem->p2);
                   evas_vec4_plain_by_points(&normal_b, &elem->p0, &elem->p1, next);

                   /* MIN_DIFF because vertices that belong to plain shouldn't be included */
                   if (fabs(normal_a.x * data[j] + normal_a.y * data[j + 1] + normal_a.z * data[j + 2] + normal_a.w) < MIN_DIFF)
                     {
                        /* based on the construction of triangles, parallel but not collinear normal
                           means that the triangles overlap, which is the worst case */
                        if ((normal_a.x * normal_b.x <= 0) && (normal_a.y * normal_b.y <= 0) && (normal_a.z * normal_b.z <= 0) &&
                            ((fabs(normal_a.x) > DBL_EPSILON) || (fabs(normal_a.y) > DBL_EPSILON) || (fabs(normal_a.z) > DBL_EPSILON)) &&
                            ((fabs(normal_b.x) > DBL_EPSILON) || (fabs(normal_b.y) > DBL_EPSILON) || (fabs(normal_b.z) > DBL_EPSILON)))
                          new_cos = 1.0;
                        else  new_cos = -1.0;
                     }

                   else
                     {
                        if (normal_a.x * data[j] + normal_a.y * data[j+1] + normal_a.z * data[j+2] + normal_a.w < 0)
                          evas_vec4_scale(&normal_a, &normal_a, -1);
                        if (normal_b.x * elem->p2.x + normal_b.y * elem->p2.y + normal_b.z * elem->p2.z + normal_b.w < 0)
                          evas_vec4_scale(&normal_b, &normal_b, -1);

                        new_cos = evas_vec4_angle_plains(&normal_a, &normal_b);
                     }

                   /* MIN_DIFF is more useful for dihedral angles apparently */
                   if (new_cos > cos + MIN_DIFF)
                     {
                        cos = new_cos;
                        EINA_ARRAY_ITER_NEXT(&arr_candidates, k, el_vec3, iterator)
                          free(el_vec3);

                        /* Vertex gets into arr_candidates if the corresponding cosine is the maximum */
                        eina_array_flush(&arr_candidates);
                        eina_array_step_set(&arr_candidates, sizeof(Eina_Array), 1);
                        eina_array_push(&arr_candidates, next);
                        pushed = EINA_TRUE;
                     }
                   else if (fabs(new_cos - cos) < MIN_DIFF)
                     {
                        exist1 = EINA_FALSE;

                        for (k = 0; (k < eina_array_count(&arr_candidates)) && !exist1; k++)
                           {
                              next_2d = eina_array_data_get(&arr_candidates, k);
                              exist1 = evas_vec3_if_equivalent(next, next_2d);
                           }
                        if (!exist1)
                          {
                             eina_array_push(&arr_candidates, next);
                             pushed = EINA_TRUE;
                          }
                     }

                   if (!pushed)
                     free(next);
                }
           }

        on_plain = EINA_FALSE;
        right = EINA_FALSE;

        /* The case when several points are found, is discussed below. 
           This case is interesting because the convex hull in the
           two-dimensional subspace should be filled further */
        if ((cos != 1.0) && (1 < eina_array_count(&arr_candidates)))
          {
             Evas_Vec3 angle_from, angle_to;
             next_2d = eina_array_data_get(&arr_candidates, 0);
             evas_vec4_plain_by_points(&normal_b, &elem->p1, &elem->p0, next_2d);

             if (normal_b.x * elem->p2.x + normal_b.y * elem->p2.y + normal_b.z * elem->p2.z + normal_b.w > 0)
               {
                  evas_vec3_subtract(&angle_from, &elem->p0, &elem->p1);
                  right = EINA_TRUE;
               }
             else
               evas_vec3_subtract(&angle_from, &elem->p1, &elem->p0);

             cos_2d = -1.0;

             EINA_ARRAY_ITER_NEXT(&arr_candidates, k, next_2d, iterator)
                {
                   /* Selection of the required vertex occurs on the basis of a specific angle */
                   if (right)
                     evas_vec3_subtract(&angle_to, next_2d, &elem->p0);
                   else
                     evas_vec3_subtract(&angle_to, next_2d, &elem->p1);

                   new_cos = evas_vec3_dot_product(&angle_from, &angle_to) /
                             (evas_vec3_length_get(&angle_from) * evas_vec3_length_get(&angle_to));
                   if (new_cos > cos_2d + FLT_EPSILON)
                     {
                        cos_2d = new_cos;
                        best = eina_array_data_get(&arr_candidates, k);
                     }
                   else if (!FLT_COMPARISON(new_cos, cos_2d))
                     {
                        if ((right && (evas_vec3_distance_get(best, &elem->p0) < evas_vec3_length_get(&angle_from))) ||
                            (!right && (evas_vec3_distance_get(best, &elem->p1) < evas_vec3_length_get(&angle_from))))
                          best = eina_array_data_get(&arr_candidates, k);
                     }
                }
          }

        /* This event will take place after the previous,
           in fact, choice of first triangle in a new two-dimensional
           convex hull allows to fill it fan counterclockwise when viewed from the inside */
        else if ((cos == 1.0) && (1 < eina_array_count(&arr_candidates)))
          {
             Evas_Vec3 angle_from, angle_to;
             evas_vec3_subtract(&angle_from, &elem->p0, &elem->p1);
             cos_2d = -1.0;
             EINA_ARRAY_ITER_NEXT(&arr_candidates, k, next_2d, iterator)
                {
                   evas_vec3_subtract(&angle_to, next_2d, &elem->p0);

                   evas_vec4_plain_by_points(&normal_a, &elem->p0, &elem->p1, &elem->p2);
                   evas_vec4_plain_by_points(&normal_b, &elem->p0, &elem->p1, next_2d);
                   if ((normal_a.x * normal_b.x <= 0) && (normal_a.y * normal_b.y <= 0) && (normal_a.z * normal_b.z <= 0))
                     {
                        new_cos = evas_vec3_dot_product(&angle_from, &angle_to) /
                                  (evas_vec3_length_get(&angle_from) * evas_vec3_length_get(&angle_to));
                        if (new_cos > cos_2d + FLT_EPSILON)
                          {
                             cos_2d = new_cos;
                             best = eina_array_data_get(&arr_candidates, k);
                          }
                        else if (!FLT_COMPARISON(new_cos, cos_2d))
                          {
                             if (evas_vec3_distance_get(best, &elem->p0) < evas_vec3_length_get(&angle_to))
                               best = eina_array_data_get(&arr_candidates, k);
                          }
                        on_plain = EINA_TRUE;
                     }
                }
          }

        else
          best = eina_array_data_get(&arr_candidates, 0);

        evas_vec4_plain_by_points(&normal_b, &elem->p0, &elem->p1, best);

        if_two = 0;
        first_exist_twice = 0;
        second_exist_twice = 0;
        equivalent_triangle = EINA_FALSE;
        triangle_chain = EINA_FALSE;
        evas_vec3_copy(&tmp1, &elem->p0);
        evas_vec3_copy(&tmp2, &elem->p1);
        new_elem1 = malloc(sizeof(Evas_Triangle3));
        evas_triangle3_set(new_elem1, best, &tmp1, &tmp2);
        pushed = EINA_FALSE;

        /* verification that the edje has not been found previously */
        EINA_ARRAY_ITER_NEXT(&arr_triangles, k, el, iterator)
          {
             if (convex_hull_triangle3_if_first_edje(el, &elem->p0, &elem->p1))
               if_two++;
             if (evas_triangle3_if_equivalent(el, new_elem1))
               equivalent_triangle++;
           }


        EINA_ARRAY_ITER_NEXT(&arr_triangles, k, el, iterator)
          {
             if ((k > 2) && (convex_hull_triangle3_if_not_first_edje(el, &elem->p0, best) ||
                 convex_hull_triangle3_if_not_first_edje(el, &elem->p1, best)))
               triangle_chain = EINA_TRUE;
          }

        /* There is a specific order according to which the edjes are entered in arr_elems */
        if (!on_plain && !right)
          {
             if ((!equivalent_triangle) && (!second_exist_twice) && (!triangle_chain) && (if_two < 2))
               {
                  if (new_elem2)
                    free (new_elem2);
                  new_elem2 = malloc(sizeof(Evas_Triangle3));
                  evas_triangle3_set(new_elem2, best, &tmp2, &tmp1);
                  eina_array_push(&arr_elems,  new_elem2);

                  /* triangles whose edges have been found should be entered into the arr_triangles
                     to optimize the algorithm */
                  if ((first_exist_twice < 2) && (second_exist_twice < 2))
                    eina_array_push(&arr_triangles, eina_array_data_get(&arr_elems, eina_array_count(&arr_elems) - 1));
               }
             if ((!equivalent_triangle) && (!first_exist_twice) && (!triangle_chain) && (if_two < 2))
               {
                  eina_array_push(&arr_elems,  new_elem1);
                  if ((first_exist_twice < 2) && (second_exist_twice < 2))
                    {
                       pushed = EINA_TRUE;

                       /* eina_ch is the resultant vector of all triangles in convex hull */
                       eina_array_push(&arr_ch, eina_array_data_get(&arr_elems, eina_array_count(&arr_elems) - 1));
                       eina_array_push(&arr_triangles, eina_array_data_get(&arr_elems, eina_array_count(&arr_elems) - 1));
                    }
               }
          }

        else
          {
             if ((!equivalent_triangle) && (!first_exist_twice) && (!triangle_chain) && (if_two < 2))
               {
                  eina_array_push(&arr_elems,  new_elem1);
                  if ((first_exist_twice < 2) && (second_exist_twice < 2))
                    {
                       pushed = EINA_TRUE;
                       eina_array_push(&arr_ch, eina_array_data_get(&arr_elems, eina_array_count(&arr_elems) - 1));
                       eina_array_push(&arr_triangles, eina_array_data_get(&arr_elems, eina_array_count(&arr_elems) - 1));
                    }
               }

             if ((!equivalent_triangle) && (!second_exist_twice) && (!triangle_chain) && (if_two < 2))
               {
                  if (new_elem2)
                    free (new_elem2);
                  new_elem2 = malloc(sizeof(Evas_Triangle3));
                  evas_triangle3_set(new_elem2, best, &tmp2, &tmp1);
                  eina_array_push(&arr_elems,  new_elem2);

                  if ((first_exist_twice < 2) && (second_exist_twice < 2))
                    eina_array_push(&arr_triangles, eina_array_data_get(&arr_elems, eina_array_count(&arr_elems)-1));
               }
          }
        if (!pushed)
          free (new_elem1);
     }


   *vertex_count = 0;
   *index_count = 3 * eina_array_count(&arr_ch);

   found_vertex = (float*) malloc(10 * sizeof(float));
   found_index = (unsigned short int*) malloc((*index_count) * sizeof(unsigned short int));
   j = 0;

#define CHECK_AND_SET_VERTEX(coord)                                                   \
  exist1 = EINA_FALSE;                                                                \
  for (i = 0, new_stride = 0; i < (*vertex_count) && !exist1; i++, new_stride += 10)  \
     {                                                                                \
        if ((k > 0) && (el->p##coord.x == found_vertex[new_stride]) &&                \
            (el->p##coord.y == found_vertex[new_stride + 1]) &&                       \
            (el->p##coord.z == found_vertex[new_stride + 2]))                         \
          {                                                                           \
             exist1 = EINA_TRUE;                                                      \
             found_index[3 * k + coord] = i;                                          \
          }                                                                           \
     }                                                                                \
     if (!exist1)                                                                     \
       convex_hull_vertex_set(el, vertex_count, &found_vertex,                        \
                       &found_index, k, &leader, coord);

   EINA_ARRAY_ITER_NEXT(&arr_ch, k, el, iterator)
     {
        CHECK_AND_SET_VERTEX(0)
        CHECK_AND_SET_VERTEX(1)
        CHECK_AND_SET_VERTEX(2)

        j += 30;
     }

   *vertex = (float*) malloc((10 * (*vertex_count)) * sizeof(float));
   memcpy(*vertex, found_vertex, (10 * (*vertex_count)) * sizeof(float));
   free(found_vertex);

   *index = (unsigned short int*) malloc((*index_count) * sizeof(unsigned short int));
   memcpy(*index, found_index, (*index_count) * sizeof(unsigned short int));
   free(found_index);

   EINA_ARRAY_ITER_NEXT(&arr_triangles, k, el, iterator)
     {
        if (k > 2)
          free(el);
     }

   free(third_elem);

   EINA_ARRAY_ITER_NEXT(&arr_candidates, k, el_vec3, iterator)
     free(el_vec3);

   eina_array_flush(&arr_candidates);
   eina_array_flush(&arr_ch);
   eina_array_flush(&arr_elems);
   eina_array_flush(&arr_triangles);

#undef CHECK_AND_SET_VERTEX

   return;
}

static inline void
tangent_space_weighted_sum(Evas_Vec3 *big_t, Evas_Vec3 *little_t,
                            Evas_Real *big_angle, Evas_Real little_angle)
{
   /* one way to calculate tangent in vertex that is found in many triangles */
   evas_vec3_scale(big_t, big_t, *big_angle / (*big_angle + little_angle));
   evas_vec3_scale(little_t, little_t, little_angle / (*big_angle + little_angle));
   evas_vec3_add(big_t, big_t, little_t);
   *big_angle += little_angle;
   return;
}


static inline Evas_Real
tangent_space_triangle_angle_get(Evas_Vec3 *first, Evas_Vec3 *second, Evas_Vec3 *third)
{
   Evas_Vec3 a, b, c;
   Evas_Real cos, arccos;

   evas_vec3_subtract(&a, second, third);
   evas_vec3_subtract(&b, third, first);
   evas_vec3_subtract(&c, first, second);

   cos = -(evas_vec3_length_square_get(&a) - evas_vec3_length_square_get(&b) -
           evas_vec3_length_square_get(&c)) / (2 * evas_vec3_length_get(&b) *
           evas_vec3_length_get(&c));
   arccos = acos(cos);

   return arccos;
}

static inline void
evas_tangent_space_get(float *data, float *tex_data, float *normal_data, unsigned short int *index, int vertex_count,
                       int index_count, int stride, int tex_stride, int normal_stride, float **tangent)
{
   Eina_Bool if_not_primitive = EINA_FALSE;
   Evas_Real big_angle, little_angle;
   Evas_Triangle3  triangle;
   Evas_Vec2 tex1, tex2, tex3;
   Evas_Vec3 big_tangent, little_tangent, normal;
   Evas_Vec4 *plain = NULL;
   int i, j, k, l, m, found_index = 0;
   int indexes[3];

   if (!tex_data)
     {
        ERR("Impossible to calculate tangent space, texture coordinates not found %d %s", __LINE__, __FILE__);
        return;
     }

   if (!(*tangent))
     {
        ERR("Failed to allocate memory %d %s", __LINE__, __FILE__);
        return;
     }

   unsigned short int *tmp_index = (unsigned short int*) malloc((vertex_count) * sizeof(unsigned short int));

   if (tmp_index == NULL)
     {
        ERR("Failed to allocate memory %d %s", __LINE__, __FILE__);
        return;
     }

   float *tmp_tangent = (float*) malloc((3 * vertex_count) * sizeof(float));
   if (tmp_tangent == NULL)
     {
        ERR("Failed to allocate memory %d %s", __LINE__, __FILE__);
        return;
     }

   if (index_count == 0)
     {
        if_not_primitive = EINA_TRUE;
        index_count = vertex_count;
     }

   for (i = 0, j = 0, k = 0; i < vertex_count; i++, j += stride, k += normal_stride)
     {
        evas_vec3_set(&big_tangent, 0.0, 0.0, 0.0);
        big_angle = 0.0;
        for (l = 0, m = 0; l < vertex_count; l++, m += stride)
          {
             /* tangent for vertex is calculating in each triangle in which the vertex is found */
             if ((fabs(data[j] - data[m]) < FLT_EPSILON) &&
                 (fabs(data[j + 1] - data[m + 1]) < FLT_EPSILON) &&
                 (fabs(data[j + 2] - data[m + 2]) < FLT_EPSILON) &&
                  ((m == j) || ((tex_data[i * tex_stride] != 0.0) && (tex_data[i * tex_stride + 1] != 0.0) &&
                   (tex_data[i * tex_stride] != 1.0) && (tex_data[i * tex_stride + 1] != 1.0))))
               {
                  found_index = l;
                  for (k = 0; k < index_count; k += 3)
                     {
                        /* there is no index count and indexes , for models that are not a primitive,
                           so we use the vertex count and an ordered array instead of them */
                        if (if_not_primitive)
                          {
                             indexes[0] = k;
                             indexes[1] = k + 1;
                             indexes[2] = k + 2;
                          }
                        else
                          {
                             indexes[0] = index[k];
                             indexes[1] = index[k + 1];
                             indexes[2] = index[k + 2];
                          }

                        if ((found_index == indexes[0]) ||
                            (found_index == indexes[1]) ||
                            (found_index == indexes[2]))
                          {
                             evas_vec3_set(&triangle.p0, data[indexes[0] * stride], data[indexes[0] * stride + 1], data[indexes[0] * stride + 2]);
                             evas_vec3_set(&triangle.p1, data[indexes[1] * stride], data[indexes[1] * stride + 1], data[indexes[1] * stride + 2]);
                             evas_vec3_set(&triangle.p2, data[indexes[2] * stride], data[indexes[2] * stride + 1], data[indexes[2] * stride + 2]);
                             if (plain)
                               free(plain);
                             plain = malloc(sizeof(Evas_Vec4));

                             evas_vec4_plain_by_points(plain, &triangle.p0, &triangle.p1, &triangle.p2);
                             tex1.x = tex_data[indexes[0] * tex_stride];
                             tex1.y = tex_data[indexes[0] * tex_stride + 1];
                             tex2.x = tex_data[indexes[1] * tex_stride];
                             tex2.y = tex_data[indexes[1] * tex_stride + 1];
                             tex3.x = tex_data[indexes[2] * tex_stride];
                             tex3.y = tex_data[indexes[2] * tex_stride + 1];

                             /* calculate the tangent */
                             tangent_new_basis(&little_tangent, &triangle,
                                               &tex1, &tex2, &tex3);
                             evas_vec3_normalize(&little_tangent, &little_tangent);

                             /* founding the angle in triangle in founded vertex */
                             if (found_index == indexes[0])
                               little_angle = tangent_space_triangle_angle_get(&triangle.p0, &triangle.p1, &triangle.p2);

                             else if (found_index == indexes[1])
                               little_angle = tangent_space_triangle_angle_get(&triangle.p1, &triangle.p0, &triangle.p2);

                             else
                               little_angle = tangent_space_triangle_angle_get(&triangle.p2, &triangle.p0, &triangle.p1);

                             if (evas_triangle3_is_line(&triangle))
                               evas_vec3_set(&big_tangent, 1.0, 0.0, 0.0);

                             else
                               tangent_space_weighted_sum(&big_tangent, &little_tangent, &big_angle, little_angle);
                          }
                     }
                }
          }
        evas_vec3_set(&normal, normal_data[j], normal_data[j + 1], normal_data[j + 2]);
        evas_vec3_orthogonal_projection_on_plain(&big_tangent, &big_tangent, &normal);
        evas_vec3_normalize(&big_tangent, &big_tangent);
        tmp_tangent[i * 3] = big_tangent.x;
        tmp_tangent[i * 3 + 1] = big_tangent.y;
        tmp_tangent[i * 3 + 2] = big_tangent.z;
     }


   memcpy(*tangent, tmp_tangent, (3 * vertex_count) * sizeof(float));
   free(tmp_index);
   free(tmp_tangent);

   return;
}
