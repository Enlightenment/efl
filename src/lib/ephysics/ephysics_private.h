#ifndef _EPHYSICS_PRIVATE_H
#define _EPHYSICS_PRIVATE_H

#pragma GCC diagnostic push
#pragma GCC system_header

#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btDefaultSoftBodySolver.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <btBulletDynamicsCommon.h>

#pragma GCC diagnostic pop

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

#ifdef CRI
# undef CRI
#endif
#define CRI(...) EINA_LOG_DOM_CRIT(_ephysics_log_dom, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

#define RAD_TO_DEG 57.29582 /* 2 * pi radians == 360 degree */

typedef struct _EPhysics_Force EPhysics_Force;
typedef struct _EPhysics_Body_Center_Mass EPhysics_Body_Center_Mass;
typedef struct _EPhysics_Body_Size EPhysics_Body_Size;
typedef struct _EPhysics_Body_Face_Slice EPhysics_Body_Face_Slice;
typedef struct _EPhysics_Point EPhysics_Point;
typedef struct _EPhysics_Dragging_Data EPhysics_Dragging_Data;

typedef enum _EPhysics_Body_Shape
{
  EPHYSICS_BODY_SHAPE_BOX,
  EPHYSICS_BODY_SHAPE_CUSTOM,
  EPHYSICS_BODY_SHAPE_CYLINDER,
  EPHYSICS_BODY_SHAPE_SPHERE,
  EPHYSICS_BODY_SHAPE_LAST,
} EPhysics_Body_Shape;

typedef enum _EPhysics_World_Boundary
{
   EPHYSICS_WORLD_BOUNDARY_TOP,
   EPHYSICS_WORLD_BOUNDARY_BOTTOM,
   EPHYSICS_WORLD_BOUNDARY_LEFT,
   EPHYSICS_WORLD_BOUNDARY_RIGHT,
   EPHYSICS_WORLD_BOUNDARY_FRONT,
   EPHYSICS_WORLD_BOUNDARY_BACK,
   EPHYSICS_WORLD_BOUNDARY_LAST
} EPhysics_World_Boundary;

typedef enum _EPhysics_Body_Type
{
  EPHYSICS_BODY_TYPE_RIGID,
  EPHYSICS_BODY_TYPE_SOFT,
  EPHYSICS_BODY_TYPE_CLOTH,
} EPhysics_Body_Type;

struct _EPhysics_Point {
     EINA_INLIST;
     double x;
     double y;
     double z;
};

struct _EPhysics_Force {
     double x;
     double y;
     double z;
     double torque_x;
     double torque_y;
     double torque_z;
};

struct _EPhysics_Dragging_Data {
     int triangle;
     double mass;
     Eina_Bool dragging:1;
};

struct _EPhysics_Body_Center_Mass {
     double x;
     double y;
     double z;
};

struct _EPhysics_Body_Size {
     Evas_Coord w;
     Evas_Coord h;
     Evas_Coord d;
};

struct _EPhysics_Body_Face_Slice {
     EPhysics_Body *body;
     EPhysics_Body_Face face;
     int slices_cnt;
     int *points_deform;
     Eina_List *slices;
};

// EPhysics_Body is an Eina_Inlist, and Eina_Inslist iterator macros use
// offsetof(). Since using offsetof in C++ classes is invalid,
// EPhysics_Body must be a POD-type struct.
struct _EPhysics_Body {
     EINA_INLIST;
     btCollisionShape *collision_shape;
     btRigidBody *rigid_body;
     btSoftBody *soft_body;
     Evas_Object *evas_obj;
     EPhysics_World *world;
     int walking;
     EPhysics_Body_Size size;
     btScalar scale[3];
     void *data;
     Eina_Inlist *callbacks;
     Eina_List *collision_groups;
     Eina_List *to_delete;
     Eina_List *face_objs;
     EPhysics_Body_Material material;
     double mass;
     double density;
     EPhysics_Force force;
     EPhysics_Body_Center_Mass cm;
     Eina_List *faces_slices;
     EPhysics_Body_Face_Slice *default_face;
     EPhysics_Body_Type type;
     EPhysics_Body_Shape shape;
     int cloth_columns;
     int cloth_rows;
     int material_index;
     int collision_cb;
     EPhysics_Dragging_Data dragging_data;

     Eina_Bool active:1;
     Eina_Bool deleted:1;
     Eina_Bool light_apply:1;
     Eina_Bool back_face_culling:1;
     Eina_Bool clockwise:1;
     Eina_Bool boundary:1;
     int bending_constraints;
     Eina_Bool anchor_hardness;
};

extern int _ephysics_log_dom;

/* Main */
void ephysics_dom_count_inc(void);
void ephysics_dom_count_dec(void);

/* World */
int ephysics_world_init(void);
int ephysics_world_shutdown(void);
Eina_Bool ephysics_world_body_add(EPhysics_World *world, EPhysics_Body *body);
Eina_Bool ephysics_world_body_del(EPhysics_World *world, EPhysics_Body *body);
Eina_Bool ephysics_world_soft_body_add(EPhysics_World *world, EPhysics_Body *body);
void ephysics_world_constraint_add(EPhysics_World *world, EPhysics_Constraint *constraint, btGeneric6DofConstraint *bt_constraint);
void ephysics_world_constraint_del(EPhysics_World *world, EPhysics_Constraint *constraint, btGeneric6DofConstraint *bt_constraint);
void ephysics_body_world_boundaries_resize(EPhysics_World *world);
void ephysics_world_boundary_set(EPhysics_World *world, EPhysics_World_Boundary boundary, EPhysics_Body *body);
EPhysics_Body *ephysics_world_boundary_get(const EPhysics_World *world, EPhysics_World_Boundary boundary);
Eina_Bool ephysics_world_bodies_outside_autodel_get(const EPhysics_World *world);
btSoftBodyWorldInfo *ephysics_world_info_get(const EPhysics_World *world);
void ephysics_world_lock_take(EPhysics_World *world);
void ephysics_world_lock_release(EPhysics_World *world);
Eina_List *ephysics_world_constraints_get(EPhysics_World *world);
void ephysics_world_force_update_set(EPhysics_World *world, Eina_Bool force_update);

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
void ephysics_body_activate(const EPhysics_Body *body, Eina_Bool activate);
void ephysics_body_evas_objects_restack(EPhysics_World *world);
void ephysics_body_soft_body_dragging_apply(EPhysics_Body *body);
void ephysics_body_soft_body_bending_constraints_generate(EPhysics_Body *body);
void ephysics_constraint_body_del(EPhysics_Body *body);

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
