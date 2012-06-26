#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_private.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct _EPhysics_Constraint {
     btTypedConstraint *bt_constraint;
     EPhysics_World *world;
};

EAPI EPhysics_Constraint *
ephysics_constraint_add(EPhysics_Body *body1, EPhysics_Body *body2, Evas_Coord anchor_b1_x, Evas_Coord anchor_b1_y, Evas_Coord anchor_b2_x, Evas_Coord anchor_b2_y)
{
   EPhysics_Constraint *constraint;
   int rate;

   if (!body1)
     {
        ERR("To create a constraint body1 must to be non null.");
        return NULL;
     }

   if ((body2) &&
       (ephysics_body_world_get(body1) != ephysics_body_world_get(body2)))
     {
        ERR("To create a constraint both bodies must to belong to the same"
            "world.");
        return NULL;
     }

   constraint = (EPhysics_Constraint *) calloc(1, sizeof(EPhysics_Constraint));
   if (!constraint)
     {
        ERR("Couldn't create a new constraint.");
        return NULL;
     }

   rate = ephysics_world_rate_get(ephysics_body_world_get(body1));

   if (!body2)
     constraint->bt_constraint = new btPoint2PointConstraint(
        *ephysics_body_rigid_body_get(body1),
        btVector3(anchor_b1_x / rate, anchor_b1_y / rate, 0));
   else
     constraint->bt_constraint = new btPoint2PointConstraint(
        *ephysics_body_rigid_body_get(body1),
        *ephysics_body_rigid_body_get(body2),
        btVector3(anchor_b1_x / rate, anchor_b1_y / rate, 0),
        btVector3(anchor_b2_x / rate, anchor_b2_y / rate, 0));

   if (!constraint->bt_constraint)
     {
        ERR("Failed to create a btConstraint");
        free(constraint);
        return NULL;
     }

   constraint->world = ephysics_body_world_get(body1);
   ephysics_world_constraint_add(constraint->world, constraint->bt_constraint);

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

   ephysics_world_constraint_del(constraint->world, constraint->bt_constraint);
   delete constraint->bt_constraint;
   free(constraint);

   INF("Constraint deleted.");
}


#ifdef  __cplusplus
}
#endif
