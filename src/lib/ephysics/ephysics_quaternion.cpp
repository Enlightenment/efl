#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_private.h"

#ifdef  __cplusplus
extern "C" {
#endif

static void
_ephysics_quaternion_update(EPhysics_Quaternion *quat, btQuaternion *bt_quat)
{
   quat->x = bt_quat->x();
   quat->y = bt_quat->y();
   quat->z = bt_quat->z();
   quat->w = bt_quat->getW();
}

static EPhysics_Quaternion *
_ephysics_quaternion_params_check(const EPhysics_Quaternion *quat1, const EPhysics_Quaternion *quat2, EPhysics_Quaternion *result)
{
   if ((!quat1) || (!quat2))
     {
        ERR("Can't operate over null quaternions.");
        return NULL;
     }

   if (result)
     return result;

   return ephysics_quaternion_new();
}

EAPI EPhysics_Quaternion *
ephysics_quaternion_new(void)
{
   EPhysics_Quaternion *quat;

   quat = (EPhysics_Quaternion *)calloc(1, sizeof(EPhysics_Quaternion));

   if (!quat)
     {
        ERR("Could not allocate ephysics quaternion.");
        return NULL;
     }

   quat->w = 1;
   return quat;
}

EAPI void
ephysics_quaternion_get(const EPhysics_Quaternion *quat, double *x, double *y, double *z, double *w)
{
   if (!quat)
     {
        ERR("Can't get quaternion's values, it is null.");
        return;
     }

   if (x) *x = quat->x;
   if (y) *y = quat->y;
   if (z) *z = quat->z;
   if (w) *w = quat->w;
}

EAPI void
ephysics_quaternion_axis_angle_get(const EPhysics_Quaternion *quat, double *nx, double *ny, double *nz, double *a)
{
   btQuaternion bt_quat;

   if (!quat)
     {
        ERR("Can't get quaternion's values, it is null.");
        return;
     }

   bt_quat = btQuaternion(quat->x, quat->y, quat->z, quat->w);

   if (nx) *nx = bt_quat.getAxis().getX();
   if (ny) *ny = bt_quat.getAxis().getY();
   if (nz) *nz = bt_quat.getAxis().getZ();
   if (a) *a = bt_quat.getAngle() * RAD_TO_DEG;
}

EAPI void
ephysics_quaternion_set(EPhysics_Quaternion *quat, double x, double y, double z, double w)
{
   if (!quat)
     {
        ERR("Could not set a null quaternion.");
        return;
     }

   quat->x = x;
   quat->y = y;
   quat->z = z;
   quat->w = w;
}

EAPI void
ephysics_quaternion_axis_angle_set(EPhysics_Quaternion *quat, double nx, double ny, double nz, double a)
{
   btQuaternion bt_quat;
   btVector3 axis;

   if (!quat)
     {
        ERR("Could not set a null quaternion.");
        return;
     }

   axis = btVector3(nx, ny, nz);
   bt_quat = btQuaternion(axis, a / RAD_TO_DEG);
   _ephysics_quaternion_update(quat, &bt_quat);
}

EAPI void
ephysics_quaternion_euler_set(EPhysics_Quaternion *quat, double yaw, double pitch, double roll)
{
   btQuaternion bt_quat;

   if (!quat)
     {
        ERR("Could not set a null quaternion.");
        return;
     }

   bt_quat.setEuler(yaw / RAD_TO_DEG, pitch / RAD_TO_DEG, roll / RAD_TO_DEG);
   _ephysics_quaternion_update(quat, &bt_quat);
}

EAPI void
ephysics_quaternion_normalize(EPhysics_Quaternion *quat)
{
   btQuaternion bt_quat;

   if (!quat)
     {
        ERR("Can't normalize a null quaternion.");
        return;
     }

   bt_quat = btQuaternion(quat->x, quat->y, quat->z, quat->w);
   bt_quat.normalize();
   _ephysics_quaternion_update(quat, &bt_quat);
}

EAPI void
ephysics_quaternion_invert(EPhysics_Quaternion *quat)
{
   btQuaternion bt_quat;

   if (!quat)
     {
        ERR("Can't normalize a null quaternion.");
        return;
     }

   bt_quat = btQuaternion(quat->x, quat->y, quat->z, quat->w);
   bt_quat.inverse();
   _ephysics_quaternion_update(quat, &bt_quat);
}

EAPI void
ephysics_quaternion_scale(EPhysics_Quaternion *quat, double scale)
{
   btQuaternion bt_quat;

   if (!quat)
     {
        ERR("Can't operate over a null quaternion.");
        return;
     }

   bt_quat = btQuaternion(quat->x, quat->y, quat->z, quat->w);
   bt_quat *= scale;
   _ephysics_quaternion_update(quat, &bt_quat);
}

EAPI void
ephysics_quaternion_inverse_scale(EPhysics_Quaternion *quat, double scale)
{
   btQuaternion bt_quat;

   if (!quat)
     {
        ERR("Can't operate over a null quaternion.");
        return;
     }

   bt_quat = btQuaternion(quat->x, quat->y, quat->z, quat->w);
   bt_quat /= scale;
   _ephysics_quaternion_update(quat, &bt_quat);
}

EAPI EPhysics_Quaternion *
ephysics_quaternion_sum(const EPhysics_Quaternion *quat1, const EPhysics_Quaternion *quat2, EPhysics_Quaternion *result)
{
   btQuaternion bt_quat1, bt_quat2, bt_quat;
   EPhysics_Quaternion *quat;

   quat = _ephysics_quaternion_params_check(quat1, quat2, result);
   if (!quat) return NULL;

   bt_quat1 = btQuaternion(quat1->x, quat1->y, quat1->z, quat1->w);
   bt_quat2 = btQuaternion(quat2->x, quat2->y, quat2->z, quat2->w);
   bt_quat = bt_quat1 + bt_quat2;

   _ephysics_quaternion_update(quat, &bt_quat);
   return quat;
}

EAPI EPhysics_Quaternion *
ephysics_quaternion_diff(const EPhysics_Quaternion *quat1, const EPhysics_Quaternion *quat2, EPhysics_Quaternion *result)
{
   btQuaternion bt_quat1, bt_quat2, bt_quat;
   EPhysics_Quaternion *quat;

   quat = _ephysics_quaternion_params_check(quat1, quat2, result);
   if (!quat) return NULL;

   bt_quat1 = btQuaternion(quat1->x, quat1->y, quat1->z, quat1->w);
   bt_quat2 = btQuaternion(quat2->x, quat2->y, quat2->z, quat2->w);
   bt_quat = bt_quat1 - bt_quat2;

   _ephysics_quaternion_update(quat, &bt_quat);
   return quat;
}

EAPI EPhysics_Quaternion *
ephysics_quaternion_multiply(const EPhysics_Quaternion *quat1, const EPhysics_Quaternion *quat2, EPhysics_Quaternion *result)
{
   btQuaternion bt_quat1, bt_quat2, bt_quat;
   EPhysics_Quaternion *quat;

   quat = _ephysics_quaternion_params_check(quat1, quat2, result);
   if (!quat) return NULL;

   bt_quat1 = btQuaternion(quat1->x, quat1->y, quat1->z, quat1->w);
   bt_quat2 = btQuaternion(quat2->x, quat2->y, quat2->z, quat2->w);
   bt_quat = bt_quat1 * bt_quat2;

   _ephysics_quaternion_update(quat, &bt_quat);
   return quat;
}

EAPI EPhysics_Quaternion *
ephysics_quaternion_slerp(const EPhysics_Quaternion *quat1, const EPhysics_Quaternion *quat2, double ratio, EPhysics_Quaternion *result)
{
   btQuaternion bt_quat1, bt_quat2;
   EPhysics_Quaternion *quat;

   quat = _ephysics_quaternion_params_check(quat1, quat2, result);
   if (!quat) return NULL;

   bt_quat1 = btQuaternion(quat1->x, quat1->y, quat1->z, quat1->w);
   bt_quat2 = btQuaternion(quat2->x, quat2->y, quat2->z, quat2->w);
   bt_quat1.slerp(bt_quat2, ratio);

   _ephysics_quaternion_update(quat, &bt_quat1);
   return quat;
}

EAPI double
ephysics_quaternion_dot(const EPhysics_Quaternion *quat1, const EPhysics_Quaternion *quat2)
{
   btQuaternion bt_quat1, bt_quat2;

   if ((!quat1) || (!quat2))
     {
        ERR("Can't operate over null quaternions.");
        return 0;
     }

   bt_quat1 = btQuaternion(quat1->x, quat1->y, quat1->z, quat1->w);
   bt_quat2 = btQuaternion(quat2->x, quat2->y, quat2->z, quat2->w);

   return bt_quat1.dot(bt_quat2);
}

EAPI double
ephysics_quaternion_angle_get(const EPhysics_Quaternion *quat1, const EPhysics_Quaternion *quat2)
{
   btQuaternion bt_quat1, bt_quat2;

   if ((!quat1) || (!quat2))
     {
        ERR("Can't operate over null quaternions.");
        return 0;
     }

   bt_quat1 = btQuaternion(quat1->x, quat1->y, quat1->z, quat1->w);
   bt_quat2 = btQuaternion(quat2->x, quat2->y, quat2->z, quat2->w);

   return bt_quat1.angle(bt_quat2) * RAD_TO_DEG;
}

EAPI double
ephysics_quaternion_length_get(const EPhysics_Quaternion *quat)
{
   btQuaternion bt_quat;

   if (!quat)
     {
        ERR("Can't operate over a null quaternion.");
        return 0;
     }

   bt_quat = btQuaternion(quat->x, quat->y, quat->z, quat->w);
   return bt_quat.length();
}

EAPI double
ephysics_quaternion_length2_get(const EPhysics_Quaternion *quat)
{
   btQuaternion bt_quat;

   if (!quat)
     {
        ERR("Can't operate over a null quaternion.");
        return 0;
     }

   bt_quat = btQuaternion(quat->x, quat->y, quat->z, quat->w);
   return bt_quat.length2();
}

#ifdef  __cplusplus
}
#endif
