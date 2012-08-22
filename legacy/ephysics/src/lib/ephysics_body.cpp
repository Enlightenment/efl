#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Evas.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>

#include <math.h>

#include "ephysics_private.h"
#include "ephysics_trimesh.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _EPhysics_Body_Callback EPhysics_Body_Callback;

struct _EPhysics_Body_Callback {
     EINA_INLIST;
     void (*func) (void *data, EPhysics_Body *body, void *event_info);
     void *data;
     EPhysics_Callback_Body_Type type;
     Eina_Bool deleted:1;
};

struct _EPhysics_Body_Collision {
     EPhysics_Body *contact_body;
     Evas_Coord x;
     Evas_Coord y;
};

static void
_ephysics_body_forces_update(EPhysics_Body *body)
{
   body->force.x = body->rigid_body->getTotalForce().getX();
   body->force.y = body->rigid_body->getTotalForce().getY();
   body->force.torque = body->rigid_body->getTotalTorque().getZ();
   body->rigid_body->clearForces();
}

static inline void
_ephysics_body_sleeping_threshold_set(EPhysics_Body *body, double linear_threshold, double angular_threshold, double rate)
{
   body->rigid_body->setSleepingThresholds(linear_threshold / rate,
                                           angular_threshold / RAD_TO_DEG);
}

static inline void
_ephysics_body_linear_velocity_set(EPhysics_Body *body, double x, double y, double rate)
{
   body->rigid_body->setLinearVelocity(btVector3(x / rate, -y / rate, 0));
}

static void
_ephysics_body_event_callback_del(EPhysics_Body *body, EPhysics_Body_Callback *cb)
{
   if (cb->deleted) return;

   cb->deleted = EINA_TRUE;

   if (body->walking)
     {
        body->to_delete = eina_list_append(body->to_delete, cb);
        return;
     }

   body->callbacks = eina_inlist_remove(body->callbacks, EINA_INLIST_GET(cb));
   free(cb);
}

static Eina_Bool
_ephysics_body_event_callback_call(EPhysics_Body *body, EPhysics_Callback_Body_Type type, void *event_info)
{
   Eina_Bool called = EINA_FALSE;
   EPhysics_Body_Callback *cb;
   void *clb;

   body->walking++;
   EINA_INLIST_FOREACH(body->callbacks, cb)
     {
        if ((cb->type == type) && (!cb->deleted))
          {
             cb->func(cb->data, body, event_info);
             called = EINA_TRUE;
          }
     }
   body->walking--;

   if (body->walking > 0) return called;

   EINA_LIST_FREE(body->to_delete, clb)
     {
        cb = (EPhysics_Body_Callback *) clb;
        body->callbacks = eina_inlist_remove(body->callbacks,
                                             EINA_INLIST_GET(cb));
        free(cb);
     }

   return called;
}

void
ephysics_body_active_set(EPhysics_Body *body, Eina_Bool active)
{
   EPhysics_Body_Callback *cb;

   if (body->active == !!active) return;
   body->active = !!active;
   if (active) return;

   _ephysics_body_event_callback_call(body, EPHYSICS_CALLBACK_BODY_STOPPED,
                                      (void *) body->evas_obj);
};

Eina_Bool
ephysics_body_filter_collision(EPhysics_Body *body0, EPhysics_Body *body1)
{
   void *grp;
   Eina_Iterator *it;
   Eina_List *l;

   if ((!body0->collision_groups) || (!body1->collision_groups))
     return EINA_TRUE;

   EINA_LIST_FOREACH(body0->collision_groups, l, grp)
     {
        if (eina_list_data_find(body1->collision_groups, grp))
          return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI Eina_Bool
ephysics_body_collision_group_add(EPhysics_Body *body, const char *group)
{
   Eina_Stringshare *group_str;

   if (!body)
     {
        ERR("Can't add body collision group, body is null.");
        return EINA_FALSE;
     }

   group_str = eina_stringshare_add(group);
   if (eina_list_data_find(body->collision_groups, group_str))
     {
        INF("Body already added to group: %s", group);
        eina_stringshare_del(group_str);
        return EINA_TRUE;
     }

   body->collision_groups = eina_list_append(body->collision_groups, group_str);
   return EINA_TRUE;
}

EAPI Eina_Bool
ephysics_body_collision_group_del(EPhysics_Body *body, const char *group)
{
   Eina_Stringshare *group_str;

   if (!body)
     {
        ERR("Can't remove body collision group, body is null.");
        return EINA_FALSE;
     }

   group_str = eina_stringshare_add(group);
   if (!eina_list_data_find(body->collision_groups, group_str))
     {
        INF("Body isn't part of group: %s", group);
        eina_stringshare_del(group_str);
        return EINA_TRUE;
     }

   body->collision_groups = eina_list_remove(body->collision_groups, group_str);
   eina_stringshare_del(group_str);
   eina_stringshare_del(group_str);
   return EINA_TRUE;
}

EAPI const Eina_List *
ephysics_body_collision_group_list_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get the body's collision group, body is null.");
        return NULL;
     }

   return body->collision_groups;
}

static EPhysics_Body *
_ephysics_body_add(EPhysics_World *world, btCollisionShape *collision_shape, const char *type, double cm_x, double cm_y)
{
   btRigidBody::btRigidBodyConstructionInfo *rigid_body_ci;
   btDefaultMotionState *motion_state;
   btRigidBody *rigid_body;
   EPhysics_Body *body;
   btScalar mass = 1;
   btVector3 inertia;

   if (!collision_shape)
     {
        ERR("Couldn't create a %s shape.", type);
        return NULL;
     }

   body = (EPhysics_Body *) calloc(1, sizeof(EPhysics_Body));
   if (!body)
     {
        ERR("Couldn't create a new body instance.");
        goto err_body;
     }

   motion_state = new btDefaultMotionState();
   if (!motion_state)
     {
        ERR("Couldn't create a motion state.");
        goto err_motion_state;
     }

   inertia = btVector3(0, 0, 0);
   collision_shape->calculateLocalInertia(mass, inertia);

   rigid_body_ci = new btRigidBody::btRigidBodyConstructionInfo(
      mass, motion_state, collision_shape, inertia);
   if (!rigid_body_ci)
     {
        ERR("Couldn't create a rigid body construction info.");
        goto err_rigid_body_ci;
     }

   rigid_body = new btRigidBody(*rigid_body_ci);
   if (!rigid_body)
     {
        ERR("Couldn't create a rigid body.");
        goto err_rigid_body;
     }

   body->collision_shape = collision_shape;
   body->rigid_body = rigid_body;
   body->mass = mass;
   body->world = world;
   body->cm.x = cm_x;
   body->cm.y = cm_y;
   body->rigid_body->setUserPointer(body);
   body->rigid_body->setLinearFactor(btVector3(1, 1, 0));
   body->rigid_body->setAngularFactor(btVector3(0, 0, 1));

   if (!ephysics_world_body_add(body->world, body))
     {
        ERR("Couldn't add body to world's bodies list");
        goto err_world_add;
     }

   delete rigid_body_ci;

   INF("Body %p of type %s added.", body, type);
   return body;

err_world_add:
   delete rigid_body;
err_rigid_body:
   delete rigid_body_ci;
err_rigid_body_ci:
   delete motion_state;
err_motion_state:
   free(body);
err_body:
   delete collision_shape;
   return NULL;
}

static void
_ephysics_body_evas_obj_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj __UNUSED__, void *event_info __UNUSED__)
{
   EPhysics_Body *body = (EPhysics_Body *) data;
   body->evas_obj = NULL;
   DBG("Evas object deleted. Updating body: %p", body);
}

static void
_ephysics_body_soft_body_points_distance_get(const EPhysics_Body *body, double distances[4][3])
{
   btVector3 center;
   btScalar raius;

   body->soft_body->getCollisionShape()->getBoundingSphere(center, raius);

   for (int m = 0; m < 4; m++)
     {
        for (int n = 0; n < 3; n++)
          {
             btVector3 node;
             double distance;

             node = body->soft_body->
                         m_faces[body->points_deform[m][n]].m_n[1]->m_x;

             distance = sqrt(pow(center.x() - node.x(), 2) +
                        pow(center.y() - node.y(), 2));

             distances[m][n] = distance;
          }
     }
}

static void
_ephysics_body_resize(EPhysics_Body *body, Evas_Coord w, Evas_Coord h)
{
   double rate, sx, sy;

   rate = ephysics_world_rate_get(body->world);
   sx = w / rate;
   sy = h / rate;

   if (body->soft_body)
     {
        body->soft_body->m_anchors.resize(0);
        body->soft_body->scale(btVector3(sx, sy, 1));

        for (int i = 0; i < body->soft_body->m_nodes.size(); i++)
          body->soft_body->appendAnchor(i, body->rigid_body);

        _ephysics_body_soft_body_points_distance_get(body, body->distances);
     }
   else
     {
        body->collision_shape->setLocalScaling(btVector3(sx, sy, 1));

        if(!body->rigid_body->isStaticObject())
          ephysics_body_mass_set(body, ephysics_body_mass_get(body));
     }

   body->w = w;
   body->h = h;

   body->rigid_body->activate(1);

   DBG("Body %p scale changed to %lf, %lf.", body, sx, sy);
}

static void
_ephysics_body_move(EPhysics_Body *body, Evas_Coord x, Evas_Coord y)
{
   double rate, mx, my;
   btTransform trans;
   int wy, height;
   btVector3 body_scale;

   rate = ephysics_world_rate_get(body->world);
   ephysics_world_render_geometry_get(body->world, NULL, &wy, NULL, &height);
   height += wy;

   mx = (x + body->w * body->cm.x) / rate;
   my = (height - (y + body->h * body->cm.y)) / rate;

   body->rigid_body->getMotionState()->getWorldTransform(trans);
   trans.setOrigin(btVector3(mx, my, 0));
   body->rigid_body->proceedToTransform(trans);
   body->rigid_body->getMotionState()->setWorldTransform(trans);

   body->rigid_body->activate(1);

   DBG("Body %p position changed to %lf, %lf.", body, mx, my);
}

static void
_ephysics_body_geometry_set(EPhysics_Body *body, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h, double rate)
{
   double mx, my, sx, sy;
   btTransform trans;
   int wy, height;
   btVector3 body_scale;

   ephysics_world_render_geometry_get(body->world, NULL, &wy, NULL, &height);
   height += wy;

   mx = (x + w * body->cm.x) / rate;
   my = (height - (y + h * body->cm.y)) / rate;
   sx = w / rate;
   sy = h / rate;

   body->rigid_body->getMotionState()->getWorldTransform(trans);
   trans.setOrigin(btVector3(mx, my, 0));

   body_scale = btVector3(sx, sy, 1);
   if (body->soft_body)
     {
        body->soft_body->m_anchors.resize(0);
        body->soft_body->scale(btVector3(sx, sy, 1));
        body->rigid_body->proceedToTransform(trans);
        body->soft_body->transform(trans);

        for (int i = 0; i < body->soft_body->m_nodes.size(); i++)
          body->soft_body->appendAnchor(i, body->rigid_body);

        _ephysics_body_soft_body_points_distance_get(body, body->distances);
     }
   else
     {
        body->collision_shape->setLocalScaling(body_scale);
        body->rigid_body->proceedToTransform(trans);

        if (!body->rigid_body->isStaticObject())
          ephysics_body_mass_set(body, ephysics_body_mass_get(body));
     }

   body->rigid_body->getMotionState()->setWorldTransform(trans);

   body->w = w;
   body->h = h;

   body->rigid_body->activate(1);

   DBG("Body %p position changed to %lf, %lf.", body, mx, my);
   DBG("Body %p scale changed to %lf, %lf.", body, sx, sy);
}

static void
_ephysics_body_soft_body_deform(EPhysics_Body *body, double rate, Evas_Map *map)
{
   double curr_distances[4][3];

   _ephysics_body_soft_body_points_distance_get(body, curr_distances);

   for (int m = 0; m < 4; m++)
     {
        Evas_Coord px, py, pz;
        double dx = 0, dy = 0;

        evas_map_point_coord_get(map, m, &px, &py, &pz);

        for (int n = 0; n < 3; n++)
          {
             double diff = (curr_distances[m][n] - body->distances[m][n]);
             dx += diff;
             dy += diff;
          }

        evas_map_point_coord_set(map, m, px - (dx * rate), py - (dy * rate),
                                 pz);
     }
}

static void
_ephysics_body_evas_obj_resize_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   EPhysics_Body *body = (EPhysics_Body *) data;
   int w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if ((w == body->w) && (h == body->h))
     return;

   DBG("Resizing body %p to w=%i, h=%i", body, w, h);
   _ephysics_body_resize(body, w, h);
}

static void
_ephysics_body_del(EPhysics_Body *body)
{
   EPhysics_Body_Callback *cb;
   void *group;

   if (body->evas_obj)
     {
        evas_object_event_callback_del(body->evas_obj, EVAS_CALLBACK_DEL,
                                       _ephysics_body_evas_obj_del_cb);
        evas_object_event_callback_del(body->evas_obj, EVAS_CALLBACK_RESIZE,
                                       _ephysics_body_evas_obj_resize_cb);
     }

   while (body->callbacks)
     {
        cb = EINA_INLIST_CONTAINER_GET(body->callbacks,
                                       EPhysics_Body_Callback);
        body->callbacks = eina_inlist_remove(body->callbacks, body->callbacks);
        free(cb);
     }

   EINA_LIST_FREE(body->collision_groups, group)
      eina_stringshare_del((Eina_Stringshare *)group);

   delete body->rigid_body->getMotionState();
   delete body->collision_shape;
   delete body->rigid_body;
   delete body->soft_body;

   free(body);
}

static void
_ephysics_body_evas_object_default_update(EPhysics_Body *body)
{
   int x, y, w, h, wx, wy, wh, cx, cy;
   EPhysics_Camera *camera;
   btTransform trans;
   double rate, rot;
   Evas_Map *map;

   if (!body->evas_obj)
     return;

   body->rigid_body->getMotionState()->getWorldTransform(trans);
   ephysics_world_render_geometry_get(body->world, &wx, &wy, NULL, &wh);

   camera = ephysics_world_camera_get(body->world);
   ephysics_camera_position_get(camera, &cx, &cy);
   cx -= wx;
   cy -= wy;

   evas_object_geometry_get(body->evas_obj, NULL, NULL, &w, &h);
   rate = ephysics_world_rate_get(body->world);
   x = (int) (trans.getOrigin().getX() * rate) - w * body->cm.x - cx;
   y = wh + wy - (int) (trans.getOrigin().getY() * rate) - h * body->cm.y - cy;

   evas_object_move(body->evas_obj, x, y);

   if ((!w) || (!h))
     {
        DBG("Evas object with no geometry: %p, w=%i h=%i", body->evas_obj,
            w, h);
        return;
     }

   rot = - trans.getRotation().getAngle() * RAD_TO_DEG *
      trans.getRotation().getAxis().getZ();

   map = evas_map_new(4);
   evas_map_util_points_populate_from_object(map, body->evas_obj);

   if (body->soft_body)
     _ephysics_body_soft_body_deform(body, rate, map);

   evas_map_util_rotate(map, rot, x + (w * body->cm.x), y +
                        (h * body->cm.y));
   evas_object_map_set(body->evas_obj, map);
   evas_object_map_enable_set(body->evas_obj, EINA_TRUE);
   evas_map_free(map);
}

static void
_ephysics_body_outside_render_area_check(EPhysics_Body *body)
{
   int wx, wy, ww, wh, bx, by, bw, bh;

   ephysics_world_render_geometry_get(body->world, &wx, &wy, &ww, &wh);
   ephysics_body_geometry_get(body, &bx, &by, &bw, &bh);

   // FIXME: check what should be done regarding rotated bodies
   if (((ephysics_world_bodies_outside_top_autodel_get(body->world)) &&
        (by + bh < wy)) ||
       ((ephysics_world_bodies_outside_bottom_autodel_get(body->world)) &&
        (by > wy + wh)) ||
       ((ephysics_world_bodies_outside_left_autodel_get(body->world)) &&
        (bx + bh < wx)) ||
       ((ephysics_world_bodies_outside_right_autodel_get(body->world)) &&
        (bx > wx + ww)))
     {
        DBG("Body %p out of render area", body);
        ephysics_body_del(body);
     }
}

void
ephysics_body_forces_apply(EPhysics_Body *body)
{
   double rate;

   if (!((body->force.x) || (body->force.y) || (body->force.torque)))
       return;

   rate = ephysics_world_rate_get(body->world);
   body->rigid_body->activate(1);
   body->rigid_body->applyCentralForce(btVector3(body->force.x / rate,
                                                 body->force.y / rate, 0));
   body->rigid_body->applyTorque(btVector3(0, 0, body->force.torque));
}

void
ephysics_body_recalc(EPhysics_Body *body, double rate)
{
   Evas_Coord x, y, w, h;
   double vx, vy, lt, at;

   ephysics_body_geometry_get(body, &x, &y, &w, &h);
   ephysics_body_linear_velocity_get(body, &vx, &vy);
   ephysics_body_sleeping_threshold_get(body, &lt, &at);

   _ephysics_body_geometry_set(body, x, y, w, h, rate);
   _ephysics_body_linear_velocity_set(body, vx, vy, rate);
   _ephysics_body_sleeping_threshold_set(body, lt, at, rate);
}

void
ephysics_body_evas_object_update_select(EPhysics_Body *body)
{
   Eina_Bool callback_called = EINA_FALSE;

   if (!body)
     return;

   callback_called = _ephysics_body_event_callback_call(
      body, EPHYSICS_CALLBACK_BODY_UPDATE, (void *) body->evas_obj);

   if (!callback_called)
     _ephysics_body_evas_object_default_update(body);

   if (ephysics_world_bodies_outside_autodel_get(body->world))
     _ephysics_body_outside_render_area_check(body);
}

EAPI void
ephysics_body_collision_position_get(const EPhysics_Body_Collision *collision, Evas_Coord *x, Evas_Coord *y)
{
   if (!collision)
     {
        ERR("Can't get body's collision data, collision is null.");
        return;
     }

   if (x) *x = collision->x;
   if (y) *y = collision->y;
}

EAPI EPhysics_Body *
ephysics_body_collision_contact_body_get(const EPhysics_Body_Collision *collision)
{
   if (!collision)
     {
        ERR("Can't get body's collision contact body, collision is null.");
        return NULL;
     }

   return collision->contact_body;
}

void
ephysics_body_contact_processed(EPhysics_Body *body, EPhysics_Body *contact_body, btVector3 position)
{
   EPhysics_Body_Collision *collision;
   EPhysics_Body_Callback *cb;
   EPhysics_World *world;;
   double rate;
   int wy, wh;

   if ((!body) || (!contact_body))
     return;

   collision = (EPhysics_Body_Collision *)calloc(
      1, sizeof(EPhysics_Body_Collision));

   if (!collision)
     {
        ERR("Can't allocate collision data structure.");
        return;
     }

   world = contact_body->world;
   ephysics_world_render_geometry_get(world, NULL, &wy, NULL, &wh);
   rate = ephysics_world_rate_get(world);

   collision->contact_body = contact_body;
   collision->x = position.getX() * rate;
   collision->y = wh + wy - (position.getY() * rate);

   _ephysics_body_event_callback_call(body, EPHYSICS_CALLBACK_BODY_COLLISION,
                                      (void *) collision);

   free(collision);
}

btRigidBody *
ephysics_body_rigid_body_get(const EPhysics_Body *body)
{
   return body->rigid_body;
}

btSoftBody *
ephysics_body_soft_body_get(const EPhysics_Body *body)
{
   return body->soft_body;
}

static EPhysics_Body *
_ephysics_body_soft_add(EPhysics_World *world, btCollisionShape *collision_shape, btSoftBody *soft_body, const char *type)
{
   EPhysics_Body *body;
   btSoftBody::AJoint::Specs angular_joint;
   btSoftBody::LJoint::Specs linear_joint;

   body = _ephysics_body_add(world, collision_shape, "soft box", 0.5, 0.5);
   if (!body)
     {
        ephysics_body_del(body);
        return NULL;
     }

   body->soft_body = soft_body;
   body->soft_body->setUserPointer(body);

   body->soft_body->getCollisionShape()->setMargin(0.22);

   soft_body->m_materials[0]->m_kLST = 0.35;
   soft_body->setPose(true, false);

   body->soft_body->m_cfg.collisions += btSoftBody::fCollision::SDF_RS;
   body->soft_body->m_cfg.collisions += btSoftBody::fCollision::CL_SS;

   body->soft_body->generateClusters(body->soft_body->m_nodes.size());

   angular_joint.erp = 0.;
   angular_joint.cfm = 0.;
   angular_joint.axis = btVector3(0, 0, 1);
   body->soft_body->appendAngularJoint(angular_joint);

   linear_joint.erp = 0.;
   linear_joint.cfm = 0.;
   linear_joint.position = btVector3(0, 0, 0);
   body->soft_body->appendLinearJoint(linear_joint, body->rigid_body);

   for (int i = 0; i < body->soft_body->m_nodes.size(); i++)
     body->soft_body->appendAnchor(i, body->rigid_body);

   ephysics_world_soft_body_add(world, body);
   return body;
}

EAPI EPhysics_Body *
ephysics_body_soft_circle_add(EPhysics_World *world)
{
   EPhysics_Body *body;
   btCollisionShape *shape;
   btSoftBodyWorldInfo *world_info;
   btSoftBody *soft_body;

   if (!world)
     {
        ERR("Can't add circle, world is null.");
        return NULL;
     }

   shape = new btCylinderShapeZ(btVector3(0.25, 0.25, 0.25));
   if (!shape)
     {
        ERR("Couldn't create a new cylinder shape.");
        goto no_collision_shape;
     }

   world_info = ephysics_world_info_get(world);
   soft_body = btSoftBodyHelpers::CreateFromTriMesh(*world_info,
                        cylinder_vertices, &cylinder_indices[0][0],
		        CYLINDER_NUM_TRIANGLES);
   if (!soft_body)
     {
        ERR("Couldn't create a new soft body.");
        goto no_soft_body;
     }

   body = _ephysics_body_soft_add(world, shape, soft_body, "soft circle");
   if (!body)
     goto no_body;

   body->points_deform[0][0] = 72;
   body->points_deform[0][1] = 6;
   body->points_deform[0][2] = 65;

   body->points_deform[1][0] = 72;
   body->points_deform[1][1] = 69;
   body->points_deform[1][2] = 3;

   body->points_deform[2][0] = 57;
   body->points_deform[2][1] = 3;
   body->points_deform[2][2] = 76;

   body->points_deform[3][0] = 54;
   body->points_deform[3][1] = 47;
   body->points_deform[3][2] = 65;

   return body;

no_body:
   delete soft_body;
no_soft_body:
   delete shape;
no_collision_shape:
   return NULL;
}

EAPI EPhysics_Body *
ephysics_body_circle_add(EPhysics_World *world)
{
   btCollisionShape *collision_shape;

   if (!world)
     {
        ERR("Can't add circle, world is null.");
        return NULL;
     }

   collision_shape = new btCylinderShapeZ(btVector3(0.5, 0.5, 0.5));
   if (!collision_shape)
     {
        ERR("Couldn't create a new cylinder shape.");
        return NULL;
     }

   return _ephysics_body_add(world, collision_shape, "circle", 0.5, 0.5);
}

EAPI EPhysics_Body *
ephysics_body_soft_box_add(EPhysics_World *world)
{
   EPhysics_Body *body;
   btCollisionShape *shape;
   btSoftBodyWorldInfo *world_info;
   btSoftBody *soft_body;

   if (!world)
     {
        ERR("Can't add circle, world is null.");
        return NULL;
     }

   shape = new btBoxShape(btVector3(0.25, 0.25, 0.25));
   if (!shape)
     {
        ERR("Couldn't create a new box shape.");
        goto no_collision_shape;
     }

   world_info = ephysics_world_info_get(world);
   soft_body = btSoftBodyHelpers::CreateFromTriMesh(*world_info,
                                            cube_vertices, &cube_indices[0][0],
                                            CUBE_NUM_TRIANGLES);
   if (!soft_body)
     {
        ERR("Couldn't create a new soft body.");
        goto no_soft_body;
     }

   body = _ephysics_body_soft_add(world, shape, soft_body, "soft box");
   if (!body)
     goto no_body;

   body->points_deform[0][0] = 27;
   body->points_deform[0][1] = 80;
   body->points_deform[0][2] = 69;

   body->points_deform[1][0] = 85;
   body->points_deform[1][1] = 12;
   body->points_deform[1][2] = 30;

   body->points_deform[2][0] = 18;
   body->points_deform[2][1] = 62;
   body->points_deform[2][2] = 8;

   body->points_deform[3][0] = 50;
   body->points_deform[3][1] = 40;
   body->points_deform[3][2] = 60;

   return body;

no_body:
   delete soft_body;
no_soft_body:
   delete shape;
no_collision_shape:
   return NULL;
}

EAPI EPhysics_Body *
ephysics_body_box_add(EPhysics_World *world)
{
   btCollisionShape *collision_shape;

   if (!world)
     {
        ERR("Can't add box, world is null.");
        return NULL;
     }

   collision_shape = new btBoxShape(btVector3(0.5, 0.5, 0.5));

   return _ephysics_body_add(world, collision_shape, "box", 0.5, 0.5);
}

EAPI EPhysics_Body *
ephysics_body_shape_add(EPhysics_World *world, EPhysics_Shape *shape)
{
   btConvexHullShape *full_shape, *simplified_shape;
   double max_x, max_y, min_x, min_y, cm_x, cm_y, range_x, range_y;
   const Eina_Inlist *points;
   EPhysics_Point *point;
   btShapeHull *hull;
   btVector3 point3d;
   btScalar margin;

   if (!world)
     {
        ERR("Can't add shape, world is null.");
        return NULL;
     }

   if (!shape)
     {
        ERR("Can't add shape, shape is null.");
        return NULL;
     }

   points = ephysics_shape_points_get(shape);
   if (eina_inlist_count(points) < 3)
     {
        ERR("At least 3 points are required to add a shape");
        return NULL;
     }

   full_shape = new btConvexHullShape();
   if (!full_shape)
     {
        ERR("Couldn't create a generic convex shape.");
        return NULL;
     }

   point = EINA_INLIST_CONTAINER_GET(points, EPhysics_Point);
   max_x = min_x = point->x;
   max_y = min_y = point->y;
   cm_x = cm_y = 0;

   /* FIXME : only vertices should be used to calculate the center of mass */
   EINA_INLIST_FOREACH(points, point)
     {
        if (point->x > max_x) max_x = point->x;
        if (point->x < min_x) min_x = point->x;
        if (point->y > max_y) max_y = point->y;
        if (point->y < min_y) min_y = point->y;

        cm_x += point->x;
        cm_y += point->y;
     }

   cm_x /= eina_inlist_count(points);
   cm_y /= eina_inlist_count(points);
   range_x = max_x - min_x;
   range_y = max_y - min_y;

   EINA_INLIST_FOREACH(points, point)
     {
        double x, y;

        x = (point->x - cm_x) / range_x;
        y = - (point->y - cm_y) / range_y;

        point3d = btVector3(x, y, -0.5);
        full_shape->addPoint(point3d);

        point3d = btVector3(x, y, 0.5);
        full_shape->addPoint(point3d);
     }

   hull = new btShapeHull(full_shape);
   if (!hull)
     {
        delete full_shape;
        ERR("Couldn't create a shape hull.");
        return NULL;
     }

   margin = full_shape->getMargin();
   hull->buildHull(margin);
   simplified_shape = new btConvexHullShape(&(hull->getVertexPointer()->getX()),
                                            hull->numVertices());
   delete hull;
   delete full_shape;
   if (!simplified_shape)
     {
        ERR("Couldn't create a simplified shape.");
        return NULL;
     }

   return _ephysics_body_add(world, (btCollisionShape *)simplified_shape,
                             "generic", (cm_x - min_x) / range_x,
                             1 - (cm_y - min_y) / range_y);
}

void
ephysics_body_world_boundaries_resize(EPhysics_World *world)
{
   Evas_Coord x, y, width, height;
   EPhysics_Body *bottom, *top, *left, *right;

   ephysics_world_render_geometry_get(world, &x, &y, &width, &height);

   bottom = ephysics_world_boundary_get(world, EPHYSICS_WORLD_BOUNDARY_BOTTOM);
   if (bottom)
     ephysics_body_geometry_set(bottom, x, y + height, width, 10);

   right = ephysics_world_boundary_get(world, EPHYSICS_WORLD_BOUNDARY_RIGHT);
   if (right)
     ephysics_body_geometry_set(right, x + width, 0, 10, y + height);

   left = ephysics_world_boundary_get(world, EPHYSICS_WORLD_BOUNDARY_LEFT);
   if (left)
     ephysics_body_geometry_set(left,  x - 10, 0, 10, y + height);

   top = ephysics_world_boundary_get(world, EPHYSICS_WORLD_BOUNDARY_TOP);
   if (top)
     ephysics_body_geometry_set(top, 0, y - 10, x + width, 10);
}

static EPhysics_Body *
_ephysics_body_boundary_add(EPhysics_World *world, EPhysics_World_Boundary boundary, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   EPhysics_Body *body;

   if (!world)
     {
        ERR("Can't add boundary, world is null.");
        return NULL;
     }

   body = ephysics_world_boundary_get(world, boundary);
   if (body)
     return body;

   body = ephysics_body_box_add(world);
   if (!body)
     return NULL;

   ephysics_body_mass_set(body, 0);
   ephysics_world_boundary_set(world, boundary, body);
   ephysics_body_geometry_set(body, x, y, w, h);

   return body;
}

EAPI EPhysics_Body *
ephysics_body_top_boundary_add(EPhysics_World *world)
{
   Evas_Coord x, y, w;
   ephysics_world_render_geometry_get(world, &x, &y, &w, NULL);
   return _ephysics_body_boundary_add(world, EPHYSICS_WORLD_BOUNDARY_TOP,
                                      0, y - 10, x + w, 10);
}

EAPI EPhysics_Body *
ephysics_body_bottom_boundary_add(EPhysics_World *world)
{
   Evas_Coord x, y, w, h;
   ephysics_world_render_geometry_get(world, &x, &y, &w, &h);
   return _ephysics_body_boundary_add(world, EPHYSICS_WORLD_BOUNDARY_BOTTOM,
                                      x, y + h, w, 10);
}

EAPI EPhysics_Body *
ephysics_body_left_boundary_add(EPhysics_World *world)
{
   Evas_Coord x, y, h;
   ephysics_world_render_geometry_get(world, &x, &y, NULL, &h);
   return _ephysics_body_boundary_add(world, EPHYSICS_WORLD_BOUNDARY_LEFT,
                                      x - 10, 0, 10, y + h);
}

EAPI EPhysics_Body *
ephysics_body_right_boundary_add(EPhysics_World *world)
{
   Evas_Coord x, y, w, h;
   ephysics_world_render_geometry_get(world, &x, &y, &w, &h);
   return _ephysics_body_boundary_add(world, EPHYSICS_WORLD_BOUNDARY_RIGHT,
                                      x + w, 0, 10, y + h);
}

void
ephysics_orphan_body_del(EPhysics_Body *body)
{
   EPhysics_Body_Callback *cb;

   _ephysics_body_event_callback_call(body, EPHYSICS_CALLBACK_BODY_DEL, NULL);
   _ephysics_body_del(body);
   INF("Body %p deleted.", body);
}

EAPI void
ephysics_body_del(EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't delete body, it wasn't provided.");
        return;
     }

   if (body->deleted) return;
   body->deleted = EINA_TRUE;
   ephysics_world_body_del(body->world, body);
}

EAPI void
ephysics_body_evas_object_set(EPhysics_Body *body, Evas_Object *evas_obj, Eina_Bool use_obj_pos)
{
   int obj_x, obj_y, obj_w, obj_h;

   if (!body)
     {
        ERR("Can't set evas object to body, the last wasn't provided.");
        return;
     }

   if (!evas_obj)
     {
        ERR("Can't set evas object to body, the first wasn't provided.");
        return;
     }

   if (body->evas_obj)
     {
        evas_object_event_callback_del(body->evas_obj, EVAS_CALLBACK_DEL,
                                       _ephysics_body_evas_obj_del_cb);
        evas_object_event_callback_del(body->evas_obj, EVAS_CALLBACK_RESIZE,
                                       _ephysics_body_evas_obj_resize_cb);
     }

   body->evas_obj = evas_obj;

   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_DEL,
                                  _ephysics_body_evas_obj_del_cb, body);

   if (!use_obj_pos)
     return;

   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_RESIZE,
                                  _ephysics_body_evas_obj_resize_cb, body);
   evas_object_geometry_get(body->evas_obj, &obj_x, &obj_y, &obj_w, &obj_h);
   _ephysics_body_geometry_set(body, obj_x, obj_y, obj_w, obj_h,
                               ephysics_world_rate_get(body->world));
}

EAPI Evas_Object *
ephysics_body_evas_object_unset(EPhysics_Body *body)
{
   Evas_Object *obj;
   if (!body)
     {
        ERR("Can't unset evas object from body, it wasn't provided.");
        return NULL;
     }

   obj = body->evas_obj;
   body->evas_obj = NULL;

   if (obj)
     {
        evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
                                       _ephysics_body_evas_obj_del_cb);
        evas_object_event_callback_del(obj, EVAS_CALLBACK_RESIZE,
                                       _ephysics_body_evas_obj_resize_cb);
     }

   return obj;
}

EAPI Evas_Object *
ephysics_body_evas_object_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get evas object from body, it wasn't provided.");
        return NULL;
     }

   return body->evas_obj;
}

EAPI void
ephysics_body_geometry_set(EPhysics_Body *body, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   if (!body)
     {
        ERR("Can't set body geometry, body is null.");
        return;
     }

   if ((w <= 0) || (h <= 0))
     {
        ERR("Width and height must to be a non-null, positive value.");
        return;
     }

   _ephysics_body_geometry_set(body, x, y, w, h,
                               ephysics_world_rate_get(body->world));
}

EAPI void
ephysics_body_resize(EPhysics_Body *body, Evas_Coord w, Evas_Coord h)
{
   if (!body)
     {
        ERR("Can't set body size, body is null.");
        return;
     }

   if ((w <= 0) || (h <= 0))
     {
        ERR("Width and height must to be a non-null, positive value.");
        return;
     }

   _ephysics_body_resize(body, w, h);
}

EAPI void
ephysics_body_move(EPhysics_Body *body, Evas_Coord x, Evas_Coord y)
{
   if (!body)
     {
        ERR("Can't set body position, body is null.");
        return;
     }

   _ephysics_body_move(body, x, y);
}

EAPI void
ephysics_body_geometry_get(const EPhysics_Body *body, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   btTransform trans;
   btVector3 vector;
   double rate;
   int wy, height;

   if (!body)
     {
        ERR("Can't get body position, body is null.");
        return;
     }

   body->rigid_body->getMotionState()->getWorldTransform(trans);
   vector = body->collision_shape->getLocalScaling();

   rate = ephysics_world_rate_get(body->world);
   ephysics_world_render_geometry_get(body->world, NULL, &wy, NULL, &height);
   height += wy;

   if (x) *x = round((trans.getOrigin().getX() - vector.x() / 2) * rate);
   if (y) *y = height - round((trans.getOrigin().getY() + vector.y() / 2)
                              * rate);
   if (w) *w = body->w;
   if (h) *h = body->h;
}

EAPI void
ephysics_body_mass_set(EPhysics_Body *body, double mass)
{
   if (!body)
     {
        ERR("Can't set body mass, body is null.");
        return;
     }

   btVector3 inertia(0, 0, 0);
   if (body->soft_body)
     body->soft_body->setTotalMass(mass);
   else
     {
        body->collision_shape->calculateLocalInertia(mass, inertia);
        body->rigid_body->setMassProps(mass, inertia);
        body->rigid_body->updateInertiaTensor();
     }
   body->mass = mass;

   DBG("Body %p mass changed to %lf.", body, mass);
}

EAPI double
ephysics_body_mass_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get body mass, body is null.");
        return 0;
     }

   return body->mass;
}

EAPI void
ephysics_body_linear_velocity_set(EPhysics_Body *body, double x, double y)
{
   double rate;

   if (!body)
     {
        ERR("Can't set body linear velocity, body is null.");
        return;
     }

   _ephysics_body_linear_velocity_set(body, x, y,
                                      ephysics_world_rate_get(body->world));
   DBG("Linear velocity of body %p set to %lf, %lf", body, x, y);
}

EAPI void
ephysics_body_linear_velocity_get(const EPhysics_Body *body, double *x, double *y)
{
   double rate;

   if (!body)
     {
        ERR("Can't get linear velocity, body is null.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);
   if (x) *x = body->rigid_body->getLinearVelocity().getX() * rate;
   if (y) *y = -body->rigid_body->getLinearVelocity().getY() * rate;
}

EAPI void
ephysics_body_angular_velocity_set(EPhysics_Body *body, double z)
{
   if (!body)
     {
        ERR("Can't set angular velocity, body is null.");
        return;
     }

   body->rigid_body->setAngularVelocity(btVector3(0, 0, -z/RAD_TO_DEG));
   DBG("Angular velocity of body %p set to %lf", body, z);
}

EAPI double
ephysics_body_angular_velocity_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get angular velocity, body is null.");
        return 0;
     }

   return -body->rigid_body->getAngularVelocity().getZ() * RAD_TO_DEG;
}

EAPI void
ephysics_body_sleeping_threshold_set(EPhysics_Body *body, double linear_threshold, double angular_threshold)
{
   if (!body)
     {
        ERR("Can't set sleeping thresholds, body is null.");
        return;
     }

   _ephysics_body_sleeping_threshold_set(body, linear_threshold,
                                         angular_threshold,
                                         ephysics_world_rate_get(body->world));
}

EAPI void
ephysics_body_sleeping_threshold_get(const EPhysics_Body *body, double *linear_threshold, double *angular_threshold)
{
   double rate;

   if (!body)
     {
        ERR("Can't get linear sleeping threshold, body is null.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);
   if (linear_threshold)
     *linear_threshold = body->rigid_body->getLinearSleepingThreshold() * rate;
   if (angular_threshold)
     *angular_threshold = body->rigid_body->getAngularSleepingThreshold() *
        RAD_TO_DEG;
}

EAPI void
ephysics_body_stop(EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't stop a null body.");
        return;
     }

   body->rigid_body->setLinearVelocity(btVector3(0, 0, 0));
   body->rigid_body->setAngularVelocity(btVector3(0, 0, 0));

   DBG("Body %p stopped", body);
}

EAPI void
ephysics_body_damping_set(EPhysics_Body *body, double linear_damping, double angular_damping)
{
    if (!body)
     {
        ERR("Can't set body damping, body is null.");
        return;
     }

    body->rigid_body->setDamping(btScalar(linear_damping),
                                 btScalar(angular_damping));
}

EAPI void
ephysics_body_damping_get(const EPhysics_Body *body, double *linear_damping, double *angular_damping)
{
    if (!body)
     {
        ERR("Can't get damping, body is null.");
        return;
     }

    if (linear_damping) *linear_damping = body->rigid_body->getLinearDamping();
    if (angular_damping) *angular_damping =
      body->rigid_body->getAngularDamping();
}

EAPI void
ephysics_body_evas_object_update(EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Couldn't update a null body.");
        return;
     }

   _ephysics_body_evas_object_default_update(body);
}

EAPI void
ephysics_body_event_callback_add(EPhysics_Body *body, EPhysics_Callback_Body_Type type, EPhysics_Body_Event_Cb func, const void *data)
{
   EPhysics_Body_Callback *cb;

   if (!body)
     {
        ERR("Can't set body event callback, body is null.");
        return;
     }

   if (!func)
     {
        ERR("Can't set body event callback, function is null.");
        return;
     }

   if ((type < 0) || (type >= EPHYSICS_CALLBACK_BODY_LAST))
     {
        ERR("Can't set body event callback, callback type is wrong.");
        return;
     }

   cb = (EPhysics_Body_Callback *)calloc(1, sizeof(EPhysics_Body_Callback));
   if (!cb)
     {
        ERR("Can't set body event callback, can't create cb instance.");
        return;
     }

   cb->func = func;
   cb->type = type;
   cb->data = (void *)data;

   body->callbacks = eina_inlist_append(body->callbacks, EINA_INLIST_GET(cb));
}

EAPI void *
ephysics_body_event_callback_del(EPhysics_Body *body, EPhysics_Callback_Body_Type type, EPhysics_Body_Event_Cb func)
{
   EPhysics_Body_Callback *cb;
   void *cb_data = NULL;

   if (!body)
     {
        ERR("Can't delete body event callback, body is null.");
        return NULL;
     }

   EINA_INLIST_FOREACH(body->callbacks, cb)
     {
        if ((cb->type != type) || (cb->func != func))
          continue;

        cb_data = cb->data;
        _ephysics_body_event_callback_del(body, cb);
        break;

     }

   return cb_data;
}

EAPI void *
ephysics_body_event_callback_del_full(EPhysics_Body *body, EPhysics_Callback_Body_Type type, EPhysics_Body_Event_Cb func, void *data)
{
   EPhysics_Body_Callback *cb;
   void *cb_data = NULL;

   if (!body)
     {
        ERR("Can't delete body event callback, body is null.");
        return NULL;
     }

   EINA_INLIST_FOREACH(body->callbacks, cb)
     {
        if ((cb->type != type) || (cb->func != func) || (cb->data != data))
          continue;

        cb_data = cb->data;
        _ephysics_body_event_callback_del(body, cb);
        break;
     }

   return cb_data;
}

EAPI void
ephysics_body_restitution_set(EPhysics_Body *body, double restitution)
{
   if (!body)
     {
        ERR("Can't set body restitution, body is null.");
        return;
     }

   body->rigid_body->setRestitution(btScalar(restitution));
}

EAPI double
ephysics_body_restitution_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get body restitution, body is null.");
        return 0;
     }

   return body->rigid_body->getRestitution();
}

EAPI void
ephysics_body_friction_set(EPhysics_Body *body, double friction)
{
   if (!body)
     {
        ERR("Can't set body friction, body is null.");
        return;
     }

   body->rigid_body->setFriction(btScalar(friction));
}

EAPI double
ephysics_body_friction_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get body friction, body is null.");
        return 0;
     }

   return body->rigid_body->getFriction();
}

EAPI EPhysics_World *
ephysics_body_world_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get the world a null body belongs to.");
        return NULL;
     }

   return body->world;
}

EAPI void
ephysics_body_central_impulse_apply(EPhysics_Body *body, double x, double y)
{
   double rate;

   if (!body)
     {
        ERR("Can't apply impulse to a null body.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);

   body->rigid_body->activate(1);
   body->rigid_body->applyCentralImpulse(btVector3(x / rate, - y / rate, 0));
}

EAPI void
ephysics_body_impulse_apply(EPhysics_Body *body, double x, double y, Evas_Coord pos_x, Evas_Coord pos_y)
{
   double rate;

   if (!body)
     {
        ERR("Can't apply impulse to a null body.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);

   body->rigid_body->activate(1);
   body->rigid_body->applyImpulse(btVector3(x / rate, - y / rate, 0),
                                  btVector3((double) pos_x / rate,
                                            (double) pos_y / rate, 0));
}

EAPI void
ephysics_body_linear_movement_enable_set(EPhysics_Body *body, Eina_Bool enable_x, Eina_Bool enable_y)
{
   if (!body)
     {
        ERR("Can't set linear factor on a null body.");
        return;
     }

   body->rigid_body->setLinearFactor(btVector3(!!enable_x, !!enable_y, 0));
}

EAPI void
ephysics_body_linear_movement_enable_get(const EPhysics_Body *body, Eina_Bool *enable_x, Eina_Bool *enable_y)
{
   if (!body)
     {
        ERR("Can't check if linear factor is enabled, body is null.");
        return;
     }

   if (enable_x) *enable_x = !!body->rigid_body->getLinearFactor().x();
   if (enable_y) *enable_y = !!body->rigid_body->getLinearFactor().y();
}

EAPI void
ephysics_body_torque_impulse_apply(EPhysics_Body *body, double roll)
{
   if (!body)
     {
        ERR("Can't apply torque impulse to a null body.");
        return;
     }

   body->rigid_body->activate(1);
   body->rigid_body->applyTorqueImpulse(btVector3(0, 0, -roll));
}

EAPI void
ephysics_body_rotation_on_z_axis_enable_set(EPhysics_Body *body, Eina_Bool enable)
{
   if (!body)
     {
        ERR("Can't set rotation on a null body.");
        return;
     }

   if (!enable)
     body->rigid_body->setAngularFactor(btVector3(0, 0, 0));
   else
     body->rigid_body->setAngularFactor(btVector3(0, 0, 1));
}

EAPI Eina_Bool
ephysics_body_rotation_on_z_axis_enable_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't check if rotation is enabled, body is null.");
        return EINA_FALSE;
     }

   return !!body->rigid_body->getAngularFactor().z();
}

EAPI double
ephysics_body_rotation_get(const EPhysics_Body *body)
{
   btTransform trans;
   double rot;

   if (!body)
     {
        ERR("Can't get rotation, body is null.");
        return 0;
     }

   body->rigid_body->getMotionState()->getWorldTransform(trans);
   rot = - trans.getRotation().getAngle() * RAD_TO_DEG *
      trans.getRotation().getAxis().getZ();

   return rot;
}

EAPI void
ephysics_body_rotation_set(EPhysics_Body *body, double rotation)
{
   btTransform trans;
   btQuaternion quat;

   if (!body)
     {
        ERR("Can't set rotation, body is null.");
        return;
     }

   body->rigid_body->activate(1);
   body->rigid_body->getMotionState()->getWorldTransform(trans);
   quat.setEuler(0, 0, -rotation / RAD_TO_DEG);
   trans.setRotation(quat);

   if (body->soft_body)
     body->soft_body->transform(trans);

   body->rigid_body->proceedToTransform(trans);
   body->rigid_body->getMotionState()->setWorldTransform(trans);

   DBG("Body %p rotation set to %lf", body, rotation);
}

EAPI void
ephysics_body_data_set(EPhysics_Body *body, void *data)
{
   if (!body)
     {
        ERR("Can't set data, body is null.");
        return;
     }

   body->data = data;
}

EAPI void *
ephysics_body_data_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get data, body is null.");
        return NULL;
     }

   return body->data;
}

EAPI void
ephysics_body_central_force_apply(EPhysics_Body *body, double x, double y)
{
   double rate;

   if (!body)
     {
        ERR("Can't apply force to a null body.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);
   ephysics_body_forces_apply(body);
   body->rigid_body->applyCentralForce(btVector3(x / rate, - y / rate, 0));
   _ephysics_body_forces_update(body);
}

EAPI void
ephysics_body_force_apply(EPhysics_Body *body, double x, double y, Evas_Coord pos_x, Evas_Coord pos_y)
{
   double rate;

   if (!body)
     {
        ERR("Can't apply force to a null body.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);
   ephysics_body_forces_apply(body);
   body->rigid_body->applyForce(btVector3(x / rate, - y / rate, 0),
                                btVector3((double) pos_x / rate,
                                          (double) pos_y / rate, 0));
   _ephysics_body_forces_update(body);
}

EAPI void
ephysics_body_torque_apply(EPhysics_Body *body, double torque)
{
   double rate;

   if (!body)
     {
        ERR("Can't apply force to a null body.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);
   ephysics_body_forces_apply(body);
   body->rigid_body->applyTorque(btVector3(0, 0, -torque));
   _ephysics_body_forces_update(body);
}

EAPI void
ephysics_body_forces_get(const EPhysics_Body *body, double *x, double *y, double *torque)
{
   double rate, gx, gy;

   if (!body)
     {
        ERR("Can't get forces from a null body.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);
   ephysics_world_gravity_get(body->world, &gx, &gy);

   if (x) *x = body->force.x * rate + gx;
   if (y) *y = -body->force.y * rate + gy;
   if (torque) *torque = -body->force.torque;
}

EAPI void
ephysics_body_forces_clear(EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't clear forces of a null body.");
        return;
     }

   body->force.x = 0;
   body->force.y = 0;
   body->force.torque = 0;
}

EAPI void
ephysics_body_center_mass_get(const EPhysics_Body *body, double *x, double *y)
{
   if (!body)
     {
        ERR("Can't get center of mass from a null body.");
        return;
     }

   if (x) *x = body->cm.x;
   if (y) *y = body->cm.y;
}

#ifdef  __cplusplus
}
#endif
