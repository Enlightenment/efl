#ifndef EVAS_PRIVATE_H
# error You shall not include this header directly
#endif

#include <math.h>
#include <float.h>

#define  DEGREE_TO_RADIAN(x)     (((x) * M_PI) / 180.0)
#define  EVAS_MATRIX_IS_IDENTITY 0x00000001
#define  MIN_DIFF 0.00000000001

typedef struct _Evas_Color Evas_Color;
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

struct _Evas_Box2
{
   Eina_Vector2   p0;
   Eina_Vector2   p1;
};

struct _Evas_Box3
{
   Eina_Vector3   p0;
   Eina_Vector3   p1;
};

struct _Evas_Line3
{
   Eina_Vector3   point;
   Eina_Vector3   direction;
};

struct _Evas_Triangle3
{
   Eina_Vector3   p0;
   Eina_Vector3   p1;
   Eina_Vector3   p2;
};

struct _Evas_Ray3
{
   Eina_Vector3   org;
   Eina_Vector3   dir;
};

struct _Evas_Sphere
{
   Eina_Vector3 center;
   Evas_Real radius;
};

static inline void
evas_triangle3_set(Evas_Triangle3 *v, Eina_Vector3 *a, Eina_Vector3 *b, Eina_Vector3 *c)
{
   eina_vector3_copy(&v->p0, a);
   eina_vector3_copy(&v->p1, b);
   eina_vector3_copy(&v->p2, c);
}

static inline Eina_Bool
evas_triangle3_is_line(Evas_Triangle3 *v)
{
   if (eina_vector3_equivalent(&v->p0, &v->p1) ||
       eina_vector3_equivalent(&v->p0, &v->p2) ||
       eina_vector3_equivalent(&v->p1, &v->p2))
     return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
convex_hull_triangle3_not_first_edje(Evas_Triangle3 *v, Eina_Vector3 *a, Eina_Vector3 *b)
{
   if ((EINA_DBL_EQ(v->p1.x, a->x) && EINA_DBL_EQ(v->p1.y, a->y) && EINA_DBL_EQ(v->p1.z, a->z)) &&
       (EINA_DBL_EQ(v->p2.x, b->x) && EINA_DBL_EQ(v->p2.y, b->y) && EINA_DBL_EQ(v->p2.z, b->z)))
     return EINA_TRUE;
   else if ((EINA_DBL_EQ(v->p2.x, a->x) && EINA_DBL_EQ(v->p2.y, a->y) && EINA_DBL_EQ(v->p2.z, a->z)) &&
            (EINA_DBL_EQ(v->p1.x, b->x) && EINA_DBL_EQ(v->p1.y, b->y) && EINA_DBL_EQ(v->p1.z, b->z)))
     return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
convex_hull_triangle3_first_edje(Evas_Triangle3 *v, Eina_Vector3 *a, Eina_Vector3 *b)
{
   if ((!EINA_FLT_EQ(v->p0.x, a->x) && !EINA_FLT_EQ(v->p0.y, a->y) &&
        !EINA_FLT_EQ(v->p0.z, a->z)) && (!EINA_FLT_EQ(v->p1.x, b->x) &&
        !EINA_FLT_EQ(v->p1.y, b->y) && !EINA_FLT_EQ(v->p1.z, b->z)))
     return EINA_TRUE;
   else if ((!EINA_FLT_EQ(v->p1.x, a->x) && !EINA_FLT_EQ(v->p1.y, a->y) &&
             !EINA_FLT_EQ(v->p1.z, a->z)) && (!EINA_FLT_EQ(v->p0.x, b->x) &&
             !EINA_FLT_EQ(v->p0.y, b->y) && !EINA_FLT_EQ(v->p0.z, b->z)))
     return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
convex_hull_triangle3_first_point(Evas_Triangle3 *v, Eina_Vector3 *a)
{
   return (EINA_DBL_EQ(v->p0.x, a->x) && EINA_DBL_EQ(v->p0.y, a->y) && EINA_DBL_EQ(v->p0.z, a->z));
}

static inline Eina_Bool
eina_vector3_equivalent_as_triangle(Eina_Vector3 *v0, Eina_Vector3 *v1, Eina_Vector3 *v2,
                                    Eina_Vector3 *w0, Eina_Vector3 *w1, Eina_Vector3 *w2)
{
   if ((EINA_DBL_EQ(v0->x, w0->x) && EINA_DBL_EQ(v0->y, w0->y) && EINA_DBL_EQ(v0->z, w0->z)) &&
       (EINA_DBL_EQ(v1->x, w1->x) && EINA_DBL_EQ(v1->y, w1->y) && EINA_DBL_EQ(v1->z, w1->z)) &&
       (EINA_DBL_EQ(v2->x, w2->x) && EINA_DBL_EQ(v2->y, w2->y) && EINA_DBL_EQ(v2->z, w2->z)))
     return EINA_TRUE;

   return EINA_FALSE;
}

static inline Eina_Bool
evas_triangle3_equivalent(Evas_Triangle3 *a, Evas_Triangle3 *b)
{
   /* to compare two triangles there are six permutations
      to test because vertices are unordered */
   if (eina_vector3_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p0, &b->p1, &b->p2) ||
       eina_vector3_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p0, &b->p2, &b->p1) ||
       eina_vector3_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p1, &b->p0, &b->p2) ||
       eina_vector3_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p1, &b->p2, &b->p0) ||
       eina_vector3_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p2, &b->p0, &b->p1) ||
       eina_vector3_equivalent_as_triangle(&a->p0, &a->p1, &a->p2, &b->p2, &b->p1, &b->p0))
     return EINA_TRUE;

   return EINA_FALSE;
}

static inline void
evas_mat4_look_at_set(Eina_Matrix4 *m,
                      const Eina_Vector3 *pos, const Eina_Vector3 *center, const Eina_Vector3 *up)
{
   Eina_Vector3 x, y, z;

   eina_vector3_subtract(&z, pos, center);
   eina_vector3_normalize(&z, &z);

   eina_vector3_cross_product(&x, up, &z);
   eina_vector3_normalize(&x, &x);

   eina_vector3_cross_product(&y, &z, &x);
   eina_vector3_normalize(&y, &y);

   m->xx = x.x;
   m->xy = y.x;
   m->xz = z.x;
   m->xw = 0.0;

   m->yx = x.y;
   m->yy = y.y;
   m->yz = z.y;
   m->yw = 0.0;

   m->zx = x.z;
   m->zy = y.z;
   m->zz = z.z;
   m->zw = 0.0;

   m->wx = -eina_vector3_dot_product(&x, pos);
   m->wy = -eina_vector3_dot_product(&y, pos);
   m->wz = -eina_vector3_dot_product(&z, pos);
   m->ww = 1.0;
}

static inline void
evas_mat4_frustum_set(Eina_Matrix4 *m,
                      Evas_Real left, Evas_Real right, Evas_Real bottom, Evas_Real top,
                      Evas_Real dnear, Evas_Real dfar)
{
   Evas_Real   w = right - left;
   Evas_Real   h = top - bottom;
   Evas_Real   depth = dnear - dfar;
   Evas_Real   near_2 = 2.0f * dnear;

   m->xx = near_2 / w;
   m->xy = 0.0f;
   m->xz = 0.0f;
   m->xw = 0.0f;

   m->yx = 0.0f;
   m->yy = near_2 / h;
   m->yz = 0.0f;
   m->yw = 0.0f;

   m->zx = (right + left) / w;
   m->zy = (top + bottom) / h;
   m->zz = (dfar + dnear) / depth;
   m->zw = -1.0f;

   m->wx = 0.0f;
   m->wy = 0.0f;
   m->wz = near_2 * dfar / depth;
   m->ww = 0.0f;
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
   eina_vector3_set(&box->p0, 0.0, 0.0, 0.0);
   eina_vector3_set(&box->p1, 0.0, 0.0, 0.0);
}

static inline void
evas_box3_copy(Evas_Box3 *dst, const Evas_Box3 *src)
{
   eina_vector3_copy(&dst->p0, &src->p0);
   eina_vector3_copy(&dst->p1, &src->p1);
}

static inline void
evas_box3_union(Evas_Box3 *out, const Evas_Box3 *a, const Evas_Box3 *b)
{
   eina_vector3_set(&out->p0, MIN(a->p0.x, b->p0.x), MIN(a->p0.y, b->p0.y), MIN(a->p0.z, b->p0.z));
   eina_vector3_set(&out->p1, MAX(a->p1.x, b->p1.x), MAX(a->p1.y, b->p1.y), MAX(a->p1.z, b->p1.z));
}

static inline void
evas_box3_transform(Evas_Box3 *out EINA_UNUSED, const Evas_Box3 *box EINA_UNUSED, const Eina_Matrix4 *mat EINA_UNUSED)
{
   /* TODO: */
}

static inline void
evas_mat4_position_get(const Eina_Matrix4 *matrix, Eina_Quaternion *position)
{
   Eina_Quaternion pos;

   pos.x = 0.0;
   pos.y = 0.0;
   pos.z = 0.0;
   pos.w = 1.0;

   eina_quaternion_transform(position, &pos, matrix);
}

static inline void
evas_mat4_direction_get(const Eina_Matrix4 *matrix, Eina_Vector3 *direction)
{
   /* TODO: Check correctness. */

   Eina_Quaternion dir;

   dir.x = 0.0;
   dir.y = 0.0;
   dir.z = 1.0;
   dir.w = 1.0;

   eina_quaternion_transform(&dir, &dir, matrix);

   direction->x = dir.x;
   direction->y = dir.y;
   direction->z = dir.z;
}

static inline void
evas_mat4_build(Eina_Matrix4 *out,
                const Eina_Vector3 *position, const Eina_Quaternion *orientation, const Eina_Vector3 *scale)
{
   Eina_Matrix3  rot;

   eina_quaternion_rotation_matrix3_get(&rot, orientation);

   out->xx = scale->x * rot.xx;
   out->xy = scale->x * rot.xy;
   out->xz = scale->x * rot.xz;
   out->xw = 0.0;

   out->yx = scale->y * rot.yx;
   out->yy = scale->y * rot.yy;
   out->yz = scale->y * rot.yz;
   out->yw = 0.0;

   out->zx = scale->z * rot.zx;
   out->zy = scale->z * rot.zy;
   out->zz = scale->z * rot.zz;
   out->zw = 0.0;

   out->wx = position->x;
   out->wy = position->y;
   out->wz = position->z;
   out->ww = 1.0;
}

static inline void
evas_mat4_inverse_build(Eina_Matrix4 *out, const Eina_Vector3 *position,
                        const Eina_Quaternion *orientation, const Eina_Vector3 *scale)
{
   Eina_Quaternion   inv_rotation;
   Eina_Vector3   inv_scale;
   Eina_Vector3   inv_translate;

   Eina_Matrix3   rot;

   /* Inverse scale. */
   eina_vector3_set(&inv_scale, 1.0 / scale->x, 1.0 / scale->y, 1.0 / scale->z);

   /* Inverse rotation. */
   eina_quaternion_inverse(&inv_rotation, orientation);

   /* Inverse translation. */
   eina_vector3_negate(&inv_translate, position);
   eina_vector3_quaternion_rotate(&inv_translate, &inv_translate, &inv_rotation);
   eina_vector3_multiply(&inv_translate, &inv_translate, &inv_scale);

   /* Get 3x3 rotation matrix. */
   eina_quaternion_rotation_matrix3_get(&rot, &inv_rotation);

   out->xx = inv_scale.x * rot.xx;
   out->xy = inv_scale.x * rot.xy;
   out->xz = inv_scale.x * rot.xz;
   out->xw = 0.0;

   out->yx = inv_scale.y * rot.yx;
   out->yy = inv_scale.y * rot.yy;
   out->yz = inv_scale.y * rot.yz;
   out->yw = 0.0;

   out->zx = inv_scale.z * rot.zx;
   out->zy = inv_scale.z * rot.zy;
   out->zz = inv_scale.z * rot.zz;
   out->zw = 0.0;

   out->wx = inv_translate.x;
   out->wy = inv_translate.y;
   out->wz = inv_translate.z;
   out->ww = 1.0;
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
evas_ray3_init(Evas_Ray3 *ray, Evas_Real x, Evas_Real y, const Eina_Matrix4 *mvp)
{
   Eina_Matrix4 mat;
   Eina_Quaternion dnear, dfar;

   memset(&mat, 0, sizeof (mat));

   /* Get the matrix which transforms from normalized device coordinate to
      modeling coodrinate. */
   eina_matrix4_inverse(&mat, mvp);

   /* Transform near point. */
   dnear.x = x;
   dnear.y = y;
   dnear.z = -1.0;
   dnear.w = 1.0;

   eina_quaternion_transform(&dnear, &dnear, &mat);

   dnear.w = 1.0 / dnear.w;
   dnear.x *= dnear.w;
   dnear.y *= dnear.w;
   dnear.z *= dnear.w;

   eina_vector3_set(&ray->org, dnear.x, dnear.y, dnear.z);

   /* Transform far point. */
   dfar.x = x;
   dfar.y = y;
   dfar.z = 1.0;
   dfar.w = 1.0;

   eina_quaternion_transform(&dfar, &dfar, &mat);

   dfar.w = 1.0 / dfar.w;
   dfar.x *= dfar.w;
   dfar.y *= dfar.w;
   dfar.z *= dfar.w;

   eina_vector3_set(&ray->dir, dfar.x - dnear.x, dfar.y - dnear.y, dfar.z - dnear.z);
}

static inline Eina_Bool
evas_box2_intersect_2d(const Evas_Box2 *box, const Eina_Vector2 *org, const Eina_Vector2 *dir)
{
   Evas_Real t1, t2, t_near = FLT_MIN, t_far = FLT_MAX;
   /* ray intersects box if its begins in */
   if ((org->x >= box->p0.x) && (org->x <= box->p1.x) &&
       (org->y >= box->p0.y) && (org->y <= box->p1.y))
     {
        return EINA_TRUE;
     }
   /* minmax algorithm of ray and box intersection */
   if (!EINA_DBL_EQ(dir->x, 0.0) && !EINA_DBL_EQ(dir->y, 0.0))
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
   else if (EINA_DBL_EQ(dir->x, 0.0))
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
   Eina_Vector2 org2;
   Eina_Vector2 dir2;
   Eina_Bool intersect = EINA_FALSE;

   /* ray intersects box if its begins in */
   if ((ray->org.x >= box->p0.x) && (ray->org.x <= box->p1.x) &&
       (ray->org.y >= box->p0.y) && (ray->org.y <= box->p1.y) &&
       (ray->org.z >= box->p0.z) && (ray->org.z <= box->p1.z))
     {
        return EINA_TRUE;
     }
   /* minmax algorithm of ray and box intersection */
   if (!EINA_DBL_EQ(ray->dir.x, 0.0) && !EINA_DBL_EQ(ray->dir.y, 0.0) && !EINA_DBL_EQ(ray->dir.z, 0.0))
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
        if (EINA_DBL_EQ(ray->dir.x, 0.0))
          {
             if ((ray->org.x < box->p0.x) || (ray->org.x > box->p1.x))
               return EINA_FALSE;
             else
               {
                  eina_vector2_set(&org2, ray->org.y, ray->org.z);
                  eina_vector2_set(&dir2, ray->dir.y, ray->dir.z);
                  evas_box2_set(&box2, box->p0.y, box->p0.z, box->p1.y, box->p1.z);
                  intersect = evas_box2_intersect_2d(&box2, &org2, &dir2);
               }
          }

        if (EINA_DBL_EQ(ray->dir.y, 0.0))
          {
             if ((ray->org.y < box->p0.y) || (ray->org.y > box->p1.y))
               return EINA_FALSE;
             else
               {
                  eina_vector2_set(&org2, ray->org.x, ray->org.z);
                  eina_vector2_set(&dir2, ray->dir.x, ray->dir.z);
                  evas_box2_set(&box2, box->p0.x, box->p0.z, box->p1.x, box->p1.z);
                  intersect = evas_box2_intersect_2d(&box2, &org2, &dir2);
               }
          }

        if (EINA_DBL_EQ(ray->dir.z, 0.0))
          {
             if (ray->org.z < box->p0.z || ray->org.z > box->p1.z)
               return EINA_FALSE;
             else
               {
                  eina_vector2_set(&org2, ray->org.x, ray->org.y);
                  eina_vector2_set(&dir2, ray->dir.x, ray->dir.y);
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
   Eina_Vector3 tmp;

   eina_vector3_set(&sphere->center, (0.5 * (box->p0.x + box->p1.x)), (0.5 * (box->p0.y + box->p1.y)), (0.5 * (box->p0.z + box->p1.z)));
   eina_vector3_set(&tmp, sphere->center.x - box->p0.x, sphere->center.y - box->p0.y, sphere->center.z - box->p0.z);

   sphere->radius = sqrtf(eina_vector3_dot_product(&tmp, &tmp));
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
evas_plane_normalize(Eina_Quaternion *plane)
{
   Eina_Vector3 tmp;
   Evas_Real length;
   eina_vector3_set(&tmp, plane->x, plane->y, plane->z);
   length = eina_vector3_length_get(&tmp);
   plane->x = plane->x / length;
   plane->y = plane->y / length;
   plane->z = plane->z / length;
   plane->w = plane->w / length;
}

static inline Eina_Bool
evas_intersection_line_of_two_plains(Evas_Line3 *line, Eina_Quaternion *plane1, Eina_Quaternion *plane2)
{
   //TODO:parallel case
   Eina_Vector3 planes3D[2];

   eina_vector3_set(&planes3D[0], plane1->x, plane1->y, plane1->z);
   eina_vector3_set(&planes3D[1], plane2->x, plane2->y, plane2->z);

   eina_vector3_cross_product(&line->direction, &planes3D[0], &planes3D[1]);

#define SOLVE_EQUATION(x, y, z) \
   line->point.x = 0; \
   line->point.y = (plane2->w * plane1->z - plane1->w * plane2->z) / line->direction.x; \
   line->point.z = (plane2->y * plane1->w - plane1->y * plane2->w) / line->direction.x;

   if (!EINA_DBL_EQ(line->direction.x, 0.0) && !EINA_DBL_EQ(plane1->z, 0.0))
     {
        SOLVE_EQUATION(x, y, z)
     }
   else if (!EINA_DBL_EQ(line->direction.y, 0.0) && !EINA_DBL_EQ(plane1->x, 0.0))
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
evas_intersection_point_of_three_plains(Eina_Vector3 *point, Eina_Quaternion *plane1, Eina_Quaternion *plane2, Eina_Quaternion *plane3)
{
   //TODO:parallel case
   int i;
   Evas_Real delta, deltax, deltay, deltaz;
   Evas_Real matrix_to_det[3][3];
   Eina_Quaternion planes[3];

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

   eina_vector3_set(point, -deltax/delta, -deltay/delta, -deltaz/delta);

   return EINA_TRUE;
}

static inline Evas_Real
evas_point_plane_distance(Eina_Vector3 *point, Eina_Quaternion *plane)
{
   return plane->x * point->x + plane->y * point->y + plane->z * point->z + plane->w;
}

static inline Evas_Real
evas_point_line_distance(Eina_Vector3 *point, Evas_Line3 *line)
{
   Eina_Vector3 temp, sub;

   eina_vector3_subtract(&sub, point, &line->point);
   eina_vector3_cross_product(&temp, &sub, &line->direction);

   return eina_vector3_length_get(&temp) / eina_vector3_length_get(&line->direction);
}

static inline Eina_Bool
evas_is_sphere_in_frustum(Evas_Sphere *bsphere, Eina_Quaternion *planes)
{
   int i;
   Evas_Line3 line;
   Eina_Vector3 point, sub;
   Evas_Real distances[6] = {0};
   int intersected_plains[3];
   int intersected_plains_count = 0;

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
             intersected_plains[intersected_plains_count] = i;
             intersected_plains_count++;
          }
      }

   switch (intersected_plains_count)
     {
      case 2:
        evas_intersection_line_of_two_plains(&line,
                                             &planes[intersected_plains[0]],
                                             &planes[intersected_plains[1]]);
        return (evas_point_line_distance(&bsphere->center, &line) <
                bsphere->radius) ? EINA_TRUE : EINA_FALSE;
      case 3:
        evas_intersection_point_of_three_plains(&point,
                                                &planes[intersected_plains[0]],
                                                &planes[intersected_plains[1]],
                                                &planes[intersected_plains[2]]);
        eina_vector3_subtract(&sub, &point, &bsphere->center);
        return (eina_vector3_length_get(&sub) < bsphere->radius) ? EINA_TRUE : EINA_FALSE;
      default:
        return EINA_TRUE;
     }
}

static inline Eina_Bool
evas_is_point_in_frustum(Eina_Vector3 *point, Eina_Quaternion *planes)
{
   int i;
   for (i = 0; i < 6; i++)
     if (evas_point_plane_distance(point, &planes[i]) <= 0) return EINA_FALSE;
   return EINA_TRUE;
}

static inline Eina_Bool
evas_is_box_in_frustum(Evas_Box3 *box, Eina_Quaternion *planes)
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
evas_frustum_calculate(Eina_Quaternion *planes, Eina_Matrix4 *matrix_vp)
{
   int i;

   eina_quaternion_set(&planes[0], matrix_vp->xw - matrix_vp->xx,
                             matrix_vp->yw - matrix_vp->yx,
                             matrix_vp->zw - matrix_vp->zx,
                             matrix_vp->ww - matrix_vp->wx);
   eina_quaternion_set(&planes[1], matrix_vp->xw - matrix_vp->xx,
                             matrix_vp->yw - matrix_vp->yx,
                             matrix_vp->zw - matrix_vp->zx,
                             matrix_vp->ww - matrix_vp->wx);
   eina_quaternion_set(&planes[2], matrix_vp->xw - matrix_vp->xx,
                             matrix_vp->yw - matrix_vp->yx,
                             matrix_vp->zw - matrix_vp->zx,
                             matrix_vp->ww - matrix_vp->wx);
   eina_quaternion_set(&planes[3], matrix_vp->xw - matrix_vp->xx,
                             matrix_vp->yw - matrix_vp->yx,
                             matrix_vp->zw - matrix_vp->zx,
                             matrix_vp->ww - matrix_vp->wx);
   eina_quaternion_set(&planes[4], matrix_vp->xw - matrix_vp->xx,
                             matrix_vp->yw - matrix_vp->yx,
                             matrix_vp->zw - matrix_vp->zx,
                             matrix_vp->ww - matrix_vp->wx);
   eina_quaternion_set(&planes[5], matrix_vp->xw - matrix_vp->xx,
                             matrix_vp->yw - matrix_vp->yx,
                             matrix_vp->zw - matrix_vp->zx,
                             matrix_vp->ww - matrix_vp->wx);

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
tangent_new_basis(Eina_Vector3 *out, Evas_Triangle3 *triangle,
                  Eina_Vector2 *a, Eina_Vector2 *b, Eina_Vector2 *c)
{
   Eina_Vector2   new1, new2;
   Eina_Vector3   old1, old2;
   eina_vector3_set(out, 0, 0, 0);

   eina_vector2_subtract(&new1, b, a);
   eina_vector2_subtract(&new2, c, a);
   eina_vector3_subtract(&old1, &(triangle->p1), &(triangle->p0));
   eina_vector3_subtract(&old2, &(triangle->p2), &(triangle->p0));


   /* calculation of new basis(in system coordinates of texturing) by solution of system of equations */
   if (!EINA_DBL_EQ(new2.y, 0.0))
     {
        eina_vector3_scale(&old2, &old2, (new1.y / new2.y));
        eina_vector2_scale(&new2, &new2, (new1.y / new2.y));

        eina_vector2_subtract(&new1, &new1, &new2);
        eina_vector3_subtract(&old1, &old1, &old2);

        eina_vector3_scale(out, &old1, 1 / new1.x);
     }

   else if (!EINA_DBL_EQ(new1.y, 0.0))
     {
        eina_vector3_scale(&old1, &old1, (new2.y / new1.y));
        eina_vector2_scale(&new1, &new1, (new2.y / new1.y));

        eina_vector2_subtract(&new2, &new2, &new1);
        eina_vector3_subtract(&old2, &old2, &old1);

        eina_vector3_scale(out, &old2, 1 / new2.x);
     }

   return;
}

static inline void
convex_hull_vertex_set(Evas_Triangle3 *el, unsigned short int *vertex_count, float **vertex,
                    unsigned short int **index, unsigned int k, int *leader, int coord)
{
   int color_coords, normal_coords;
   Eina_Vector3 vect = {0, 0, 0};
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

   Eina_Vector3   triangle1;
   Eina_Vector3   triangle2, triangle2_candidate;
   Eina_Vector3   triangle3, triangle3_candidate;
   Eina_Vector3   first, diagonal, complanar1, complanar2, candidate;
   Eina_Quaternion   normal_a, normal_b;

   Evas_Real cos = 0.0, new_cos = 0.0, sin = 0.0, new_sin = 0.0, cos_2d = 0.0, new_cos_2d = 0.0;
   int first_num = 0;
   int i = 0, j = 0;

   eina_vector3_set(&triangle1, data[0], data[1], data[2]);

   for (i = 1, j = stride; i < count; i++, j += stride)
      {
         if ((triangle1.z > data[j + 2]) ||
             ((EINA_FLT_EQ(triangle1.z, data[j + 2])) && (triangle1.y > data[j + 1])) ||
             ((EINA_FLT_EQ(triangle1.z, data[j + 2])) && (EINA_FLT_EQ(triangle1.y, data[j + 1])) && (triangle1.x > data[j])))
           {
              eina_vector3_set(&triangle1, data[j], data[j + 1], data[j + 2]);
              first_num = i;
           }
      }

   if (first_num)
     eina_vector3_set(&triangle2, data[0], data[1], data[2]);
   else
     eina_vector3_set(&triangle2, data[3], data[4], data[5]);

   eina_vector3_subtract(&diagonal, &triangle2, &triangle1);
   sin = fabs(triangle2.z - triangle1.z) / eina_vector3_length_get(&diagonal);

#define COMPARE_ANGLES(trigonom, triangle, previous, big, little)     \
   if (little > big + FLT_EPSILON)                                    \
     {                                                                \
        trigonom = new_##trigonom;                                    \
        cos_2d = new_cos_2d;                                          \
        eina_vector3_set(&triangle, data[j], data[j + 1], data[j + 2]);   \
     }                                                                \
   else if(!EINA_FLT_EQ(little, big) &&                            \
           (eina_vector3_distance_get(&triangle##_candidate, &previous) >  \
            eina_vector3_distance_get(&triangle, &previous)))              \
     {                                                                \
        eina_vector3_set(&triangle, data[j], data[j + 1], data[j + 2]);   \
     }
   eina_vector3_set(&complanar1, 1, 0, 0);
   for (i = 0, j = 0; i < count; i++, j += stride)
      {
         if (EINA_FLT_EQ(data[j], triangle1.x) ||
             EINA_FLT_EQ(data[j + 1], triangle1.y) ||
             EINA_FLT_EQ(data[j + 2], triangle1.z))
           {
              eina_vector3_set(&triangle2_candidate, data[j], data[j + 1], data[j + 2]);
              eina_vector3_subtract(&diagonal, &triangle2_candidate, &triangle1);
              new_sin = fabs(data[j + 2] - triangle1.z) / eina_vector3_length_get(&diagonal);

              if (sin > new_sin + FLT_EPSILON)
                {
                   sin = new_sin;
                   eina_vector3_set(&triangle2, data[j], data[j + 1], data[j + 2]);
                   eina_vector3_subtract(&diagonal, &triangle2, &triangle1);
                   cos_2d = eina_vector3_angle_get(&complanar1, &diagonal);
                }
              else if (!EINA_FLT_EQ(sin, new_sin))
                {
                   eina_vector3_subtract(&diagonal, &triangle2_candidate, &triangle1);
                   new_cos_2d = eina_vector3_angle_get(&complanar1, &diagonal);

                   COMPARE_ANGLES(cos, triangle2, triangle1, cos_2d, new_cos_2d)
                }
           }
      }

   eina_vector3_set(&complanar1, triangle1.x + 1, triangle1.y, triangle1.z);
   eina_vector3_set(&complanar2, triangle1.x, triangle1.y + 1, triangle1.z);
   eina_vector3_plane_by_points(&normal_a, &triangle1, &complanar1, &complanar2);

   if (normal_a.z < 0)
     eina_quaternion_scale(&normal_a, &normal_a, -1);

   eina_vector3_set(&triangle3, data[0], data[1], data[2]);

   cos = -1.0;
   cos_2d = 1.0;

   for (i = 0, j = 0; i < count; i++, j += stride)
      {
         eina_vector3_set(&candidate, data[j], data[j + 1], data[j + 2]);

         if ((EINA_FLT_EQ(data[j], triangle1.x) ||
             EINA_FLT_EQ(data[j + 1], triangle1.y) ||
             EINA_FLT_EQ(data[j + 2], triangle1.z)) &&
             (EINA_FLT_EQ(data[j], triangle2.x) ||
             EINA_FLT_EQ(data[j + 1], triangle2.y) ||
             EINA_FLT_EQ(data[j + 2], triangle2.z)))
           {
              eina_vector3_plane_by_points(&normal_b, &triangle1, &candidate, &triangle2);

              if (normal_b.z < 0)
                eina_quaternion_scale(&normal_b, &normal_b, -1);

              new_cos = eina_quaternion_angle_plains(&normal_a, &normal_b);

              if (new_cos > cos + FLT_EPSILON)
                {
                   eina_vector3_set(&triangle3_candidate, data[j], data[j + 1], data[j + 2]);
                   eina_vector3_subtract(&first, &triangle2, &triangle1);
                   eina_vector3_subtract(&diagonal, &triangle3, &triangle1);
                   cos = new_cos;
                   eina_vector3_set(&triangle3, data[j], data[j + 1], data[j + 2]);
                   cos_2d = eina_vector3_angle_get(&diagonal, &first);
                }
              else if (!EINA_FLT_EQ(new_cos, cos))
                {
                   eina_vector3_set(&triangle3_candidate, data[j], data[j + 1], data[j + 2]);
                   eina_vector3_subtract(&first, &triangle1, &triangle2);
                   eina_vector3_subtract(&diagonal, &triangle3_candidate, &triangle2);
                   new_cos_2d = eina_vector3_angle_get(&first, &diagonal);

                   COMPARE_ANGLES(cos, triangle3, triangle2, new_cos_2d, cos_2d)
                }
           }
      }

   evas_triangle3_set(&out, &triangle1, &triangle2, &triangle3);

#undef COMPARE_ANGLES
   return out;
}

static inline void
evas_convex_hull_get(float *data, int count, int stride, Eina_Inarray *vertex,
                     Eina_Inarray *index)
{
   Evas_Triangle3  first_elem, second_elem, *third_elem = NULL, *el = NULL;

   Eina_Vector3   *next = NULL, *best = NULL, *next_2d = NULL, *el_vec3 = NULL;
   Eina_Vector3   tmp1, tmp2;
   Eina_Quaternion   normal_a, normal_b;

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
   unsigned short int *found_index = NULL, index_count, vertex_count = 0;

   Eina_Bool exist1 = EINA_FALSE, pushed;
   Eina_Bool equivalent_triangle = EINA_FALSE, triangle_chain = EINA_FALSE;
   Eina_Bool on_plane = EINA_FALSE, right = EINA_FALSE;

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
              if ((EINA_FLT_EQ(elem->p0.x, data[j]) || EINA_FLT_EQ(elem->p0.y, data[j + 1]) ||
                   EINA_FLT_EQ(elem->p0.z, data[j + 2])) && (EINA_FLT_EQ(elem->p1.x, data[j]) ||
                   EINA_FLT_EQ(elem->p1.y, data[j + 1]) || EINA_FLT_EQ(elem->p1.z, data[j + 2])) &&
                  (EINA_FLT_EQ(elem->p2.x, data[j]) || EINA_FLT_EQ(elem->p2.y, data[j + 1]) ||
                   EINA_FLT_EQ(elem->p2.z, data[j + 2])))
                {
                   next = malloc(sizeof(Eina_Vector3));
                   eina_vector3_set(next, data[j], data[j + 1], data[j + 2]);
                   pushed = EINA_FALSE;

                   /* something like the dihedral angle between the triangles
                      is a determining factor in searching the necessary points */

                   eina_vector3_plane_by_points(&normal_a, &elem->p0, &elem->p1, &elem->p2);
                   eina_vector3_plane_by_points(&normal_b, &elem->p0, &elem->p1, next);

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
                          eina_quaternion_scale(&normal_a, &normal_a, -1);
                        if (normal_b.x * elem->p2.x + normal_b.y * elem->p2.y + normal_b.z * elem->p2.z + normal_b.w < 0)
                          eina_quaternion_scale(&normal_b, &normal_b, -1);

                        new_cos = eina_quaternion_angle_plains(&normal_a, &normal_b);
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
                              exist1 = eina_vector3_equivalent(next, next_2d);
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

        on_plane = EINA_FALSE;
        right = EINA_FALSE;

        /* The case when several points are found, is discussed below.
           This case is interesting because the convex hull in the
           two-dimensional subspace should be filled further */
        if ((!EINA_FLT_EQ(cos, 1.0)) && (1 < eina_array_count(&arr_candidates)))
          {
             Eina_Vector3 angle_from, angle_to;
             next_2d = eina_array_data_get(&arr_candidates, 0);
             eina_vector3_plane_by_points(&normal_b, &elem->p1, &elem->p0, next_2d);

             if (normal_b.x * elem->p2.x + normal_b.y * elem->p2.y + normal_b.z * elem->p2.z + normal_b.w > 0)
               {
                  eina_vector3_subtract(&angle_from, &elem->p0, &elem->p1);
                  right = EINA_TRUE;
               }
             else
               eina_vector3_subtract(&angle_from, &elem->p1, &elem->p0);

             cos_2d = -1.0;

             EINA_ARRAY_ITER_NEXT(&arr_candidates, k, next_2d, iterator)
                {
                   /* Selection of the required vertex occurs on the basis of a specific angle */
                   if (right)
                     eina_vector3_subtract(&angle_to, next_2d, &elem->p0);
                   else
                     eina_vector3_subtract(&angle_to, next_2d, &elem->p1);

                   new_cos = eina_vector3_dot_product(&angle_from, &angle_to) /
                             (eina_vector3_length_get(&angle_from) * eina_vector3_length_get(&angle_to));
                   if (new_cos > cos_2d + FLT_EPSILON)
                     {
                        cos_2d = new_cos;
                        best = eina_array_data_get(&arr_candidates, k);
                     }
                   else if (!EINA_FLT_EQ(new_cos, cos_2d))
                     {
                        if ((right && (eina_vector3_distance_get(best, &elem->p0) < eina_vector3_length_get(&angle_from))) ||
                            (!right && (eina_vector3_distance_get(best, &elem->p1) < eina_vector3_length_get(&angle_from))))
                          best = eina_array_data_get(&arr_candidates, k);
                     }
                }
          }

        /* This event will take place after the previous,
           in fact, choice of first triangle in a new two-dimensional
           convex hull allows to fill it fan counterclockwise when viewed from the inside */
        else if ((EINA_FLT_EQ(cos, 1.0)) && (1 < eina_array_count(&arr_candidates)))
          {
             Eina_Vector3 angle_from, angle_to;
             eina_vector3_subtract(&angle_from, &elem->p0, &elem->p1);
             cos_2d = -1.0;
             EINA_ARRAY_ITER_NEXT(&arr_candidates, k, next_2d, iterator)
                {
                   eina_vector3_subtract(&angle_to, next_2d, &elem->p0);

                   eina_vector3_plane_by_points(&normal_a, &elem->p0, &elem->p1, &elem->p2);
                   eina_vector3_plane_by_points(&normal_b, &elem->p0, &elem->p1, next_2d);
                   if ((normal_a.x * normal_b.x <= 0) && (normal_a.y * normal_b.y <= 0) && (normal_a.z * normal_b.z <= 0))
                     {
                        new_cos = eina_vector3_dot_product(&angle_from, &angle_to) /
                                  (eina_vector3_length_get(&angle_from) * eina_vector3_length_get(&angle_to));
                        if (new_cos > cos_2d + FLT_EPSILON)
                          {
                             cos_2d = new_cos;
                             best = eina_array_data_get(&arr_candidates, k);
                          }
                        else if (!EINA_FLT_EQ(new_cos, cos_2d))
                          {
                             if (eina_vector3_distance_get(best, &elem->p0) < eina_vector3_length_get(&angle_to))
                               best = eina_array_data_get(&arr_candidates, k);
                          }
                        on_plane = EINA_TRUE;
                     }
                }
          }

        else
          best = eina_array_data_get(&arr_candidates, 0);

        eina_vector3_plane_by_points(&normal_b, &elem->p0, &elem->p1, best);

        if_two = 0;
        first_exist_twice = 0;
        second_exist_twice = 0;
        equivalent_triangle = EINA_FALSE;
        triangle_chain = EINA_FALSE;
        eina_vector3_copy(&tmp1, &elem->p0);
        eina_vector3_copy(&tmp2, &elem->p1);
        new_elem1 = malloc(sizeof(Evas_Triangle3));
        evas_triangle3_set(new_elem1, best, &tmp1, &tmp2);
        pushed = EINA_FALSE;

        /* verification that the edje has not been found previously */
        EINA_ARRAY_ITER_NEXT(&arr_triangles, k, el, iterator)
          {
             if (convex_hull_triangle3_first_edje(el, &elem->p0, &elem->p1))
               if_two++;
             if (evas_triangle3_equivalent(el, new_elem1))
               equivalent_triangle++;
           }


        EINA_ARRAY_ITER_NEXT(&arr_triangles, k, el, iterator)
          {
             if ((k > 2) && (convex_hull_triangle3_not_first_edje(el, &elem->p0, best) ||
                 convex_hull_triangle3_not_first_edje(el, &elem->p1, best)))
               triangle_chain = EINA_TRUE;
          }

        /* There is a specific order according to which the edjes are entered in arr_elems */
        if (!on_plane && !right)
          {
             if ((!equivalent_triangle) && (!second_exist_twice) && (!triangle_chain) && (if_two < 2))
               {
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

   index_count = 3 * eina_array_count(&arr_ch);

   found_vertex = (float*) malloc(10 * sizeof(float));
   found_index = (unsigned short int*) malloc(index_count * sizeof(unsigned short int));
   j = 0;

#define CHECK_AND_SET_VERTEX(coord)                                     \
   exist1 = EINA_FALSE;                                                 \
   for (i = 0, new_stride = 0; (i < vertex_count) && !exist1; i++, new_stride += 10) \
     {                                                                  \
        if ((k > 0) && !EINA_FLT_EQ(el->p##coord.x, found_vertex[new_stride]) && \
            !EINA_FLT_EQ(el->p##coord.y, found_vertex[new_stride + 1]) && \
            !EINA_FLT_EQ(el->p##coord.z, found_vertex[new_stride + 2])) \
          {                                                             \
             exist1 = EINA_TRUE;                                        \
             found_index[3 * k + coord] = i;                            \
          }                                                             \
     }                                                                  \
   if (!exist1)                                                         \
     convex_hull_vertex_set(el, &vertex_count, &found_vertex,           \
                            &found_index, k, &leader, coord);

   EINA_ARRAY_ITER_NEXT(&arr_ch, k, el, iterator)
     {
        CHECK_AND_SET_VERTEX(0)
        CHECK_AND_SET_VERTEX(1)
        CHECK_AND_SET_VERTEX(2)

        j += 30;
     }

   for (i = 0; i < 10 * (vertex_count); i++)
      eina_inarray_push(vertex, &found_vertex[i]);

   for (i = 0; i < index_count; i++)
      eina_inarray_push(index, &found_index[i]);

   free(found_vertex);

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
tangent_space_weighted_sum(Eina_Vector3 *big_t, Eina_Vector3 *little_t,
                            Evas_Real *big_angle, Evas_Real little_angle)
{
   /* one way to calculate tangent in vertex that is found in many triangles */
   eina_vector3_scale(big_t, big_t, *big_angle / (*big_angle + little_angle));
   eina_vector3_scale(little_t, little_t, little_angle / (*big_angle + little_angle));
   eina_vector3_add(big_t, big_t, little_t);
   *big_angle += little_angle;
   return;
}


static inline Evas_Real
tangent_space_triangle_angle_get(Eina_Vector3 *first, Eina_Vector3 *second, Eina_Vector3 *third)
{
   Eina_Vector3 a, b, c;
   Evas_Real cos, arccos;

   eina_vector3_subtract(&a, second, third);
   eina_vector3_subtract(&b, third, first);
   eina_vector3_subtract(&c, first, second);

   cos = -(eina_vector3_length_square_get(&a) - eina_vector3_length_square_get(&b) -
           eina_vector3_length_square_get(&c)) / (2 * eina_vector3_length_get(&b) *
           eina_vector3_length_get(&c));
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
   Eina_Vector2 tex1, tex2, tex3;
   Eina_Vector3 big_tangent, little_tangent, normal;
   Eina_Quaternion *plain = NULL;
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
        eina_vector3_set(&big_tangent, 0.0, 0.0, 0.0);
        big_angle = 0.0;
        for (l = 0, m = 0; l < vertex_count; l++, m += stride)
          {
             /* tangent for vertex is calculating in each triangle in which the vertex is found */
             if ((fabs(data[j] - data[m]) < FLT_EPSILON) &&
                 (fabs(data[j + 1] - data[m + 1]) < FLT_EPSILON) &&
                 (fabs(data[j + 2] - data[m + 2]) < FLT_EPSILON) &&
                 ((m == j) || ((!EINA_FLT_EQ(tex_data[i * tex_stride], 0.0)) &&
                               !EINA_FLT_EQ(tex_data[i * tex_stride + 1], 0.0) &&
                               !EINA_FLT_EQ(tex_data[i * tex_stride], 1.0) &&
                               !EINA_FLT_EQ(tex_data[i * tex_stride + 1], 1.0))))
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
                             eina_vector3_set(&triangle.p0, data[indexes[0] * stride], data[indexes[0] * stride + 1], data[indexes[0] * stride + 2]);
                             eina_vector3_set(&triangle.p1, data[indexes[1] * stride], data[indexes[1] * stride + 1], data[indexes[1] * stride + 2]);
                             eina_vector3_set(&triangle.p2, data[indexes[2] * stride], data[indexes[2] * stride + 1], data[indexes[2] * stride + 2]);
                             if (plain)
                               free(plain);
                             plain = malloc(sizeof(Eina_Quaternion));

                             eina_vector3_plane_by_points(plain, &triangle.p0, &triangle.p1, &triangle.p2);
                             tex1.x = tex_data[indexes[0] * tex_stride];
                             tex1.y = tex_data[indexes[0] * tex_stride + 1];
                             tex2.x = tex_data[indexes[1] * tex_stride];
                             tex2.y = tex_data[indexes[1] * tex_stride + 1];
                             tex3.x = tex_data[indexes[2] * tex_stride];
                             tex3.y = tex_data[indexes[2] * tex_stride + 1];

                             /* calculate the tangent */
                             tangent_new_basis(&little_tangent, &triangle,
                                               &tex1, &tex2, &tex3);
                             eina_vector3_normalize(&little_tangent, &little_tangent);

                             /* founding the angle in triangle in founded vertex */
                             if (found_index == indexes[0])
                               little_angle = tangent_space_triangle_angle_get(&triangle.p0, &triangle.p1, &triangle.p2);

                             else if (found_index == indexes[1])
                               little_angle = tangent_space_triangle_angle_get(&triangle.p1, &triangle.p0, &triangle.p2);

                             else
                               little_angle = tangent_space_triangle_angle_get(&triangle.p2, &triangle.p0, &triangle.p1);

                             if (evas_triangle3_is_line(&triangle))
                               eina_vector3_set(&big_tangent, 1.0, 0.0, 0.0);

                             else
                               tangent_space_weighted_sum(&big_tangent, &little_tangent, &big_angle, little_angle);
                          }
                     }
                }
          }
        eina_vector3_set(&normal, normal_data[j], normal_data[j + 1], normal_data[j + 2]);
        eina_vector3_orthogonal_projection_on_plane(&big_tangent, &big_tangent, &normal);
        eina_vector3_normalize(&big_tangent, &big_tangent);
        tmp_tangent[i * 3] = big_tangent.x;
        tmp_tangent[i * 3 + 1] = big_tangent.y;
        tmp_tangent[i * 3 + 2] = big_tangent.z;
     }


   memcpy(*tangent, tmp_tangent, (3 * vertex_count) * sizeof(float));
   free(tmp_tangent);
   free(plain);

   return;
}

static inline void
calculate_box(Evas_Box3 *box3, int vertex_count, Eina_Vector3 *vertex_position)
{
   int i = 0;
   float vxmin, vymin, vzmin, vxmax, vymax, vzmax;

   vxmax = vxmin = vertex_position[0].x;
   vymax = vymin = vertex_position[0].y;
   vzmax = vzmin = vertex_position[0].z;

   for (i = 1; i < vertex_count; ++i)
     {
        if (vxmin > vertex_position[i].x) vxmin = vertex_position[i].x;
        if (vxmax < vertex_position[i].x) vxmax = vertex_position[i].x;
        if (vymin > vertex_position[i].y) vymin = vertex_position[i].y;
        if (vymax < vertex_position[i].y) vymax = vertex_position[i].y;
        if (vzmin > vertex_position[i].z) vzmin = vertex_position[i].z;
        if (vzmax < vertex_position[i].z) vzmax = vertex_position[i].z;
     }
   evas_box3_set(box3, vxmin, vymin, vzmin, vxmax, vymax, vzmax);
}

static inline void
calculate_sphere(Evas_Sphere *sphere, int vertex_count,  Eina_Vector3 *vertex_position)
{
   float radius = 0.0001f;
   Eina_Vector3 center, pos, diff;
   float len, alpha, alpha2;
   int  i, k;

   // shuffle array for averaging algorithms error
   for (i = 0; i < vertex_count; i++)
     {
        k = i + rand()%(vertex_count-i);
        pos = vertex_position[i];
        vertex_position[i] = vertex_position[k];
        vertex_position[k] = pos;
     }

   center = vertex_position[0];

   for (k = 0; k < 2; k++)
     {
        for (i = 0; i < vertex_count; ++i)
          {
             pos = vertex_position[i];
             eina_vector3_subtract(&diff, &pos, &center);
             len = eina_vector3_length_get(&diff);
             if (len > radius)
               {
                  alpha = len / radius;
                  alpha2 = alpha * alpha;
                  radius = 0.5f * (alpha + 1 / alpha) * radius;
                  eina_vector3_scale(&pos, &pos, 1 - 1 / alpha2);
                  eina_vector3_scale(&center, &center, (1 + 1 / alpha2));
                  eina_vector3_add(&center, &center, &pos);
                  eina_vector3_scale(&center, &center, 0.5f);
               }
          }
     }

   for (i = 0; i < vertex_count; ++i)
     {
        pos = vertex_position[i];
        eina_vector3_subtract(&diff, &pos, &center);
        len = eina_vector3_length_get(&diff);

        if (len > radius)
          {
             radius = (radius + len) / 2.0f;
             eina_vector3_scale(&diff, &diff, (len - radius) / len);
             eina_vector3_add(&center, &center, &diff);
          }
     }

   sphere->radius = radius;
   sphere->center = center;
}
