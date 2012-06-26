#ifndef _EPHYSICS_PRIVATE_H
#define _EPHYSICS_PRIVATE_H

#ifdef HAVE_EVIL
# include <Evil.h>
#endif

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

typedef enum _EPhysics_World_Boundary
{
   EPHYSICS_WORLD_BOUNDARY_TOP,
   EPHYSICS_WORLD_BOUNDARY_BOTTOM,
   EPHYSICS_WORLD_BOUNDARY_LEFT,
   EPHYSICS_WORLD_BOUNDARY_RIGHT,
   EPHYSICS_WORLD_BOUNDARY_LAST
} EPhysics_World_Boundary;

extern int _ephysics_log_dom;

int ephysics_world_init(void);
int ephysics_world_shutdown(void);
Eina_Bool ephysics_world_body_add(EPhysics_World *world, const EPhysics_Body *body, btRigidBody *rigid_body);
void ephysics_world_body_del(EPhysics_World *world, const EPhysics_Body *body, btRigidBody *rigid_body);
void ephysics_world_constraint_add(EPhysics_World *world, btTypedConstraint *bt_constraint);
void ephysics_world_constraint_del(EPhysics_World *world, btTypedConstraint *bt_constraint);
void ephysics_body_world_boundaries_resize(EPhysics_World *world);
void ephysics_world_boundary_set(EPhysics_World *world, EPhysics_World_Boundary boundary, EPhysics_Body *body);
EPhysics_Body *ephysics_world_boundary_get(const EPhysics_World *world, EPhysics_World_Boundary boundary);

void ephysics_body_evas_object_update_select(EPhysics_Body *body);
void ephysics_orphan_body_del(EPhysics_Body *body);
void ephysics_body_contact_processed(EPhysics_Body *body, EPhysics_Body *contact_body);
btRigidBody *ephysics_body_rigid_body_get(EPhysics_Body *body);
void ephysics_body_active_set(EPhysics_Body *body, Eina_Bool active);

EPhysics_Camera *ephysics_camera_add(EPhysics_World *world);
void ephysics_camera_del(EPhysics_Camera *camera);

#ifdef __cplusplus
}
#endif

#endif
