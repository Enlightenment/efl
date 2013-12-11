#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_private.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct _EPhysics_Constraint {
     btGeneric6DofConstraint *bt_constraint;
     EPhysics_World *world;
     EPhysics_Body *bodies[2];
};

static void
_ephysics_constraint_del(EPhysics_Constraint *constraint)
{
   ephysics_world_constraint_del(constraint->world, constraint,
                                 constraint->bt_constraint);
   delete constraint->bt_constraint;
   free(constraint);
}

void
ephysics_constraint_body_del(EPhysics_Body *body)
{
   void *ldata;
   EPhysics_Constraint *constraint;
   Eina_List *l, *constraints, *rem = NULL;

   constraints = ephysics_world_constraints_get(body->world);
   if (!constraints) return;

   EINA_LIST_FOREACH(constraints, l, ldata)
     {
        constraint = (EPhysics_Constraint *)ldata;
        if (constraint->bodies[0] == body || constraint->bodies[1])
          rem = eina_list_append(rem, constraint);
     }

   EINA_LIST_FREE(rem, ldata)
     _ephysics_constraint_del((EPhysics_Constraint *)ldata);
}

static void
_ephysics_constraint_linear_limit_get(const EPhysics_Constraint *constraint, Evas_Coord *lower_x, Evas_Coord *upper_x, Evas_Coord *lower_y, Evas_Coord *upper_y, Evas_Coord *lower_z, Evas_Coord *upper_z, double rate)
{
   btVector3 linear_limit;

   if (lower_x || lower_y || lower_z)
     {
        constraint->bt_constraint->getLinearLowerLimit(linear_limit);

        if (lower_x) *lower_x = linear_limit.getX() * rate;
        if (lower_y) *lower_y = linear_limit.getY() * rate;
        if (lower_z) *lower_z = linear_limit.getZ() * rate;
     }

   if (upper_x || upper_y || upper_z)
     {
        constraint->bt_constraint->getLinearUpperLimit(linear_limit);

        if (upper_x) *upper_x = linear_limit.getX() * rate;
        if (upper_y) *upper_y = linear_limit.getY() * rate;
        if (upper_z) *upper_z = linear_limit.getZ() * rate;
     }
}

static void
_ephysics_constraint_linear_limit_set(EPhysics_Constraint *constraint, Evas_Coord lower_x, Evas_Coord upper_x, Evas_Coord lower_y, Evas_Coord upper_y, Evas_Coord lower_z, Evas_Coord upper_z, double rate)
{
   lower_x = (lower_x) / rate;
   upper_x = (upper_x) / rate;

   lower_y = (lower_y) / rate;
   upper_y = (upper_y) / rate;

   lower_z = (lower_z) / rate;
   upper_z = (upper_z) / rate;

   constraint->bt_constraint->setLinearLowerLimit(btVector3(-lower_x, -upper_y,
                                                            -lower_z));
   constraint->bt_constraint->setLinearUpperLimit(btVector3(upper_x, upper_y,
                                                            upper_z));
}

void
ephysics_constraint_recalc(EPhysics_Constraint *constraint, double rate)
{
   Evas_Coord lower_x, upper_x, lower_y, upper_y, lower_z, upper_z;

   _ephysics_constraint_linear_limit_get(constraint, &lower_x, &upper_x,
                                   &lower_y, &upper_y, &lower_z, &upper_z, rate);
   _ephysics_constraint_linear_limit_set(constraint, lower_x, upper_x, lower_y,
                                   upper_y, lower_z, upper_z, rate);
}

EAPI EPhysics_Constraint *
ephysics_constraint_add(EPhysics_Body *body)
{
   EPhysics_Constraint *constraint;
   btTransform trans;

   if (!body)
     {
        ERR("To create a constraint body must to be non null.");
        return NULL;
     }

   constraint = (EPhysics_Constraint *) calloc(1, sizeof(EPhysics_Constraint));
   if (!constraint)
     {
        ERR("Couldn't create a new constraint.");
        return NULL;
     }

   ephysics_world_lock_take(ephysics_body_world_get(body));
   trans.setIdentity();
   trans.setOrigin(btVector3(0, 0, 0));
   constraint->bt_constraint = new
       btGeneric6DofConstraint(*ephysics_body_rigid_body_get(body), trans,
                               false);

   if (!constraint->bt_constraint)
     {
        ERR("Failed to create a btConstraint");
        free(constraint);
        return NULL;
     }

   constraint->bodies[0] = body;
   constraint->world = ephysics_body_world_get(body);
   ephysics_world_constraint_add(constraint->world, constraint,
                                 constraint->bt_constraint);

   INF("Constraint added.");
   ephysics_world_lock_release(ephysics_body_world_get(body));
   return constraint;
}

EAPI void
ephysics_constraint_linear_limit_set(EPhysics_Constraint *constraint, Evas_Coord lower_x, Evas_Coord upper_x, Evas_Coord lower_y, Evas_Coord upper_y, Evas_Coord lower_z, Evas_Coord upper_z)
{
   double rate;

   if (!constraint)
     {
        ERR("Can't set constraint's linear limit, constraint is null.");
        return;
     }

   ephysics_world_lock_take(constraint->world);
   rate = ephysics_world_rate_get(constraint->world);
   _ephysics_constraint_linear_limit_set(constraint, lower_x, upper_x, lower_y,
                                         upper_y, lower_z, upper_z, rate);
   ephysics_world_lock_release(constraint->world);
}

EAPI void
ephysics_constraint_linear_limit_get(const EPhysics_Constraint *constraint, Evas_Coord *lower_x, Evas_Coord *upper_x, Evas_Coord *lower_y, Evas_Coord *upper_y, Evas_Coord *lower_z, Evas_Coord *upper_z)
{
   int rate;

   if (!constraint)
     {
        ERR("Can't get constraint's linear limit, constraint is null.");
        return;
     }

   rate = ephysics_world_rate_get(constraint->world);
   _ephysics_constraint_linear_limit_get(constraint, lower_x, upper_x, lower_y,
                                         upper_y, lower_z, upper_z, rate);
}

EAPI void
ephysics_constraint_angular_limit_set(EPhysics_Constraint *constraint, double counter_clock_x, double clock_wise_x, double counter_clock_y, double clock_wise_y, double counter_clock_z, double clock_wise_z)
{

   if (!constraint)
     {
        ERR("Can't set constraint's angular limit, constraint is null.");
        return;
     }

   ephysics_world_lock_take(constraint->world);

   constraint->bt_constraint->setAngularLowerLimit(btVector3(
                    -counter_clock_x / RAD_TO_DEG, -counter_clock_y / RAD_TO_DEG,
                    -counter_clock_z/RAD_TO_DEG));
   constraint->bt_constraint->setAngularUpperLimit(btVector3(
                    clock_wise_x / RAD_TO_DEG, clock_wise_y / RAD_TO_DEG,
                    clock_wise_z/RAD_TO_DEG));
   ephysics_world_lock_release(constraint->world);
}

EAPI void
ephysics_constraint_angular_limit_get(const EPhysics_Constraint *constraint, double *counter_clock_x, double *clock_wise_x, double *counter_clock_y, double *clock_wise_y, double *counter_clock_z, double *clock_wise_z)
{
   btVector3 angular_limit;

   if (!constraint)
     {
        ERR("Can't get constraint's angular limit, constraint is null.");
        return;
     }

   if (counter_clock_x)
     {
        constraint->bt_constraint->getAngularLowerLimit(angular_limit);
        *counter_clock_x = angular_limit.getX() * RAD_TO_DEG;
     }

   if (clock_wise_x)
     {
        constraint->bt_constraint->getAngularUpperLimit(angular_limit);
        *clock_wise_x = angular_limit.getX() * RAD_TO_DEG;
     }

   if (counter_clock_y)
     {
        constraint->bt_constraint->getAngularLowerLimit(angular_limit);
        *counter_clock_y = angular_limit.getY() * RAD_TO_DEG;
     }

   if (clock_wise_y)
     {
        constraint->bt_constraint->getAngularUpperLimit(angular_limit);
        *clock_wise_y = angular_limit.getY() * RAD_TO_DEG;
     }

   if (counter_clock_z)
     {
        constraint->bt_constraint->getAngularLowerLimit(angular_limit);
        *counter_clock_z = angular_limit.getZ() * RAD_TO_DEG;
     }

   if (clock_wise_z)
     {
        constraint->bt_constraint->getAngularUpperLimit(angular_limit);
        *clock_wise_z = angular_limit.getZ() * RAD_TO_DEG;
     }
}

EAPI void
ephysics_constraint_anchor_set(EPhysics_Constraint *constraint, Evas_Coord anchor_b1_x, Evas_Coord anchor_b1_y, Evas_Coord anchor_b1_z, Evas_Coord anchor_b2_x, Evas_Coord anchor_b2_y, Evas_Coord anchor_b2_z)
{
   btTransform anchor_b1;
   btTransform anchor_b2;
   btTransform center_mass;
   double rate;
   Evas_Coord b1x, b1y, b1z, b1w, b1h, b1d, b2x, b2y, b2z, b2w, b2h, b2d, wx, wy,
     wh;
   btScalar ab1x, ab1y, ab1z, ab2x, ab2y, ab2z;

   if (!constraint)
     {
        ERR("Can't set constraint's anchors, constraint is null.");
        return;
     }

   ephysics_world_lock_take(constraint->world);

   ephysics_world_render_geometry_get(constraint->world, &wx, &wy, NULL, NULL,
                                      &wh, NULL);

   ephysics_body_geometry_get(constraint->bodies[0], &b1x, &b1y, &b1z, &b1w,
                              &b1h, &b1d);

   rate = ephysics_world_rate_get(constraint->world);

   ab1x = (anchor_b1_x - (b1x + b1w / 2)) / rate;
   ab1y = (anchor_b1_y - (b1y + b1h / 2)) / rate;
   ab1z = (anchor_b1_z - (b1z + b1d / 2)) / rate;
   DBG("body1 anchor set to: %lf, %lf, %lf", ab1x, ab1y, ab1z);

   anchor_b1.setIdentity();
   anchor_b1.setOrigin(btVector3(ab1x, ab1y, ab1z));

   if (constraint->bodies[1])
     {
        ephysics_body_geometry_get(constraint->bodies[1], &b2x, &b2y, &b2z, &b2w,
                                &b2h, &b2d);

        ab2x = (anchor_b2_x - (b2x + b2w / 2)) / rate;
        ab2y = (anchor_b2_y - (b2y + b2h / 2)) / rate;
        ab2z = (anchor_b2_z - (b2z + b2d / 2)) / rate;

        DBG("body2 anchor set to: %lf, %lf, %lf", ab2x, ab2y, ab2z);

        anchor_b2.setIdentity();
        anchor_b2.setOrigin(btVector3(ab2x, ab2y, ab2z));
     }
   else
     {
        anchor_b2.setIdentity();
        anchor_b2.setOrigin(btVector3(anchor_b1.getOrigin().x(),
                                      anchor_b1.getOrigin().y(),
                                      anchor_b1.getOrigin().z()));

        center_mass = constraint->bodies[0]->rigid_body->
          getCenterOfMassTransform();

        anchor_b1.setIdentity();
        anchor_b1 = center_mass * anchor_b2;
     }

   constraint->bt_constraint->setFrames(anchor_b1, anchor_b2);
   ephysics_world_lock_release(constraint->world);
}

EAPI void
ephysics_constraint_anchor_get(const EPhysics_Constraint *constraint, Evas_Coord *anchor_b1_x, Evas_Coord *anchor_b1_y, Evas_Coord *anchor_b1_z, Evas_Coord *anchor_b2_x, Evas_Coord *anchor_b2_y, Evas_Coord *anchor_b2_z)
{
   btTransform anchor_b1;
   btTransform anchor_b2;
   double rate;

   if (!constraint)
     {
        ERR("Can't set constraint's anchors, constraint is null.");
        return;
     }

   rate = ephysics_world_rate_get(constraint->world);

   anchor_b1 = constraint->bt_constraint->getFrameOffsetA();
   anchor_b2 = constraint->bt_constraint->getFrameOffsetB();

   if (anchor_b1_x) *anchor_b1_x = round(anchor_b1.getOrigin().x() * rate);
   if (anchor_b1_y) *anchor_b1_y = round(anchor_b1.getOrigin().y() * rate);
   if (anchor_b1_z) *anchor_b1_z = round(anchor_b1.getOrigin().z() * rate);
   if (anchor_b2_x) *anchor_b2_x = round(anchor_b2.getOrigin().x() * rate);
   if (anchor_b2_y) *anchor_b2_y = round(anchor_b2.getOrigin().y() * rate);
   if (anchor_b2_z) *anchor_b2_z = round(anchor_b2.getOrigin().z() * rate);
}

EAPI EPhysics_Constraint *
ephysics_constraint_linked_add(EPhysics_Body *body1, EPhysics_Body *body2)
{
   EPhysics_Constraint *constraint;

   if (!body1 || !body2)
     {
        ERR("To create a linked constraint body1 and bod2 must to be non null.");
        return NULL;
     }

   if (ephysics_body_world_get(body1) != ephysics_body_world_get(body2))
     {
        ERR("To create a constraint both bodies must belong to the same"
            "world.");
        return NULL;
     }

   if (body1->type == EPHYSICS_BODY_TYPE_CLOTH ||
       body2->type == EPHYSICS_BODY_TYPE_CLOTH)
     {
        ERR("Constraints are allowed only between rigid -> rigid bodies or"
            "rigid -> soft bodies");
        return NULL;
     }

   constraint = (EPhysics_Constraint *) calloc(1, sizeof(EPhysics_Constraint));
   if (!constraint)
     {
        ERR("Couldn't create a new constraint.");
        return NULL;
     }

   constraint->world = ephysics_body_world_get(body1);
   constraint->bodies[0] = body1;
   constraint->bodies[1] = body2;

   ephysics_world_lock_take(constraint->world);

   constraint->bt_constraint = new btGeneric6DofConstraint(
      *ephysics_body_rigid_body_get(body1), *ephysics_body_rigid_body_get(body2),
      btTransform(), btTransform(), false);

   if (!constraint->bt_constraint)
     {
        ephysics_world_lock_release(constraint->world);
        free(constraint);
        return NULL;
     }

   ephysics_world_constraint_add(constraint->world, constraint,
                                 constraint->bt_constraint);

   ephysics_world_lock_release(constraint->world);
   INF("Constraint added.");
   return constraint;
}

EAPI void
ephysics_constraint_del(EPhysics_Constraint *constraint)
{
   if (!constraint)
     {
        ERR("Can't delete constraint, it wasn't provided.");
        return;
     }

   // technically at this point locking and unlocking is pointless because
   // if another thread is accessing this constraint, after this point it
   // will be broken locks or not. this removes a segv in locking something
   // we freed in _ephysics_constraint_del() by not locking.
   _ephysics_constraint_del(constraint);
   INF("Constraint deleted.");
}


#ifdef  __cplusplus
}
#endif
