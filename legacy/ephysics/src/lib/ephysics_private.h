#ifndef _EPHYSICS_PRIVATE_H
#define _EPHYSICS_PRIVATE_H

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btDefaultSoftBodySolver.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <btBulletDynamicsCommon.h>
#include "EPhysics.h"

#ifdef EPHYSICS_LOG_COLOR
#undef EPHYSICS_LOG_COLOR
#endif
#define EPHYSICS_LOG_COLOR EINA_COLOR_BLUE

#ifdef ERR
# undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_ephysics_log_dom, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_ephysics_log_dom, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...) EINA_LOG_DOM_INFO(_ephysics_log_dom, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...) EINA_LOG_DOM_WARN(_ephysics_log_dom, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(_ephysics_log_dom, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

#define RAD_TO_DEG 57.29582 /* 2 * pi radians == 360 degree */

typedef struct _EPhysics_Point EPhysics_Point;

typedef enum _EPhysics_World_Boundary
{
   EPHYSICS_WORLD_BOUNDARY_TOP,
   EPHYSICS_WORLD_BOUNDARY_BOTTOM,
   EPHYSICS_WORLD_BOUNDARY_LEFT,
   EPHYSICS_WORLD_BOUNDARY_RIGHT,
   EPHYSICS_WORLD_BOUNDARY_LAST
} EPhysics_World_Boundary;

struct _EPhysics_Point {
     EINA_INLIST;
     double x;
     double y;
};

struct _EPhysics_Body {
     EINA_INLIST;
     btCollisionShape *collision_shape;
     btRigidBody *rigid_body;
     btSoftBody *soft_body;
     Evas_Object *evas_obj;
     EPhysics_World *world;
     int walking;
     Evas_Coord w;
     Evas_Coord h;
     void *data;
     Eina_Inlist *callbacks;
     Eina_List *collision_groups;
     Eina_List *to_delete;
     double mass;
     struct {
          double x;
          double y;
          double torque;
     } force;
     Eina_Bool active:1;
     Eina_Bool deleted:1;
     double distances[4][3];
     int points_deform[4][3];
};

extern int _ephysics_log_dom;

/* World */
int ephysics_world_init(void);
int ephysics_world_shutdown(void);
Eina_Bool ephysics_world_body_add(EPhysics_World *world, EPhysics_Body *body);
Eina_Bool ephysics_world_body_del(EPhysics_World *world, EPhysics_Body *body);
Eina_Bool ephysics_world_soft_body_add(EPhysics_World *world, EPhysics_Body *body);
void ephysics_world_constraint_add(EPhysics_World *world, EPhysics_Constraint *constraint, btTypedConstraint *bt_constraint);
void ephysics_world_constraint_del(EPhysics_World *world, EPhysics_Constraint *constraint, btTypedConstraint *bt_constraint);
void ephysics_body_world_boundaries_resize(EPhysics_World *world);
void ephysics_world_boundary_set(EPhysics_World *world, EPhysics_World_Boundary boundary, EPhysics_Body *body);
EPhysics_Body *ephysics_world_boundary_get(const EPhysics_World *world, EPhysics_World_Boundary boundary);
Eina_Bool ephysics_world_bodies_outside_autodel_get(const EPhysics_World *world);
btSoftBodyWorldInfo *ephysics_world_info_get(const EPhysics_World *world);

/* Body */
Eina_Bool ephysics_body_filter_collision(EPhysics_Body *body0, EPhysics_Body *body1);
void ephysics_body_evas_object_update_select(EPhysics_Body *body);
void ephysics_orphan_body_del(EPhysics_Body *body);
void ephysics_body_contact_processed(EPhysics_Body *body, EPhysics_Body *contact_body, btVector3 position);
btRigidBody *ephysics_body_rigid_body_get(const EPhysics_Body *body);
btSoftBody *ephysics_body_soft_body_get(const EPhysics_Body *body);
void ephysics_body_active_set(EPhysics_Body *body, Eina_Bool active);
void ephysics_body_recalc(EPhysics_Body *body, double rate);
void ephysics_body_forces_apply(EPhysics_Body *body);

/* Camera */
EPhysics_Camera *ephysics_camera_add(EPhysics_World *world);
void ephysics_camera_del(EPhysics_Camera *camera);
void ephysics_camera_moved_set(EPhysics_Camera *camera, Eina_Bool moved);
Eina_Bool ephysics_camera_moved_get(const EPhysics_Camera *camera);
void ephysics_camera_target_moved(EPhysics_Camera *camera, EPhysics_Body *body);

/* Shape */
const Eina_Inlist *ephysics_shape_points_get(const EPhysics_Shape *shape);

/* Constraint */
void ephysics_constraint_recalc(EPhysics_Constraint *constraint, double rate);

#ifdef __cplusplus
}
#endif

#endif
