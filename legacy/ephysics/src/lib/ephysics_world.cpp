#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ephysics_private.h"
#include <Ecore.h>
#include <Evas.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct _EPhysics_World_Callback EPhysics_World_Callback;

struct _EPhysics_World_Callback {
     EINA_INLIST;
     void (*func) (void *data, EPhysics_World *world, void *event_info);
     void *data;
     EPhysics_Callback_World_Type type;
     Eina_Bool deleted:1;
};

struct _EPhysics_World {
     EINA_INLIST;
     btBroadphaseInterface* broadphase;
     btDefaultCollisionConfiguration* collision;
     btCollisionDispatcher* dispatcher;
     btSequentialImpulseConstraintSolver* solver;
     btDiscreteDynamicsWorld* dynamics_world;

     EPhysics_Body *boundaries[4];
     EPhysics_Camera *camera;
     Evas_Coord x, y, w, h;
     Eina_Inlist *callbacks;
     Eina_Inlist *bodies;
     Eina_List *to_delete;
     Eina_List *cb_to_delete;
     Eina_List *constraints;
     int max_sub_steps;
     int walking;
     int cb_walking;
     double last_update;
     double rate;
     double fixed_time_step;
     double max_sleeping_time;
     Eina_Bool running:1;
     Eina_Bool active:1;
     Eina_Bool deleted:1;
     Eina_Bool outside_autodel:1;
     Eina_Bool outside_top:1;
     Eina_Bool outside_bottom:1;
     Eina_Bool outside_left:1;
     Eina_Bool outside_right:1;
};

static int _ephysics_world_init_count = 0;
static int _worlds_running = 0;
static Eina_Inlist *_worlds = NULL;
static Eina_List *_worlds_to_delete = NULL;
static Ecore_Animator *_anim_simulate = NULL;
static int _worlds_walking = 0;

struct _ephysics_world_ovelap_filter_cb : public btOverlapFilterCallback
{
   virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0,
                                        btBroadphaseProxy* proxy1) const
   {
      btCollisionObject *coll0 = (btCollisionObject *)proxy0->m_clientObject;
      btCollisionObject *coll1 = (btCollisionObject *)proxy1->m_clientObject;

      EPhysics_Body *body0 = (EPhysics_Body *)coll0->getUserPointer();
      EPhysics_Body *body1 = (EPhysics_Body *)coll1->getUserPointer();

      if ((!body0 || !body1) || (ephysics_body_filter_collision(body0, body1)))
        return EINA_TRUE;

      return EINA_FALSE;
   }
};

static inline void
_ephysics_world_gravity_set(EPhysics_World *world, double gx, double gy, double rate)
{
   world->dynamics_world->setGravity(btVector3(gx / rate, -gy / rate, 0));
}

static void
_ephysics_world_event_callback_call(EPhysics_World *world, EPhysics_Callback_World_Type type, void *event_info)
{
   EPhysics_World_Callback *cb;
   void *clb;

   world->cb_walking++;
   EINA_INLIST_FOREACH(world->callbacks, cb)
     {
        if ((cb->type == type) && (!cb->deleted))
          cb->func(cb->data, world, event_info);
     }
   world->cb_walking--;

   if (world->cb_walking > 0) return;

   EINA_LIST_FREE(world->cb_to_delete, clb)
     {
        cb = (EPhysics_World_Callback *)clb;
        world->callbacks = eina_inlist_remove(world->callbacks,
                                              EINA_INLIST_GET(cb));
        free(cb);
     }
}

static void
_ephysics_world_event_callback_del(EPhysics_World *world, EPhysics_World_Callback *cb)
{
   if (cb->deleted) return;

   cb->deleted = EINA_TRUE;

   if (world->cb_walking)
     {
        world->cb_to_delete = eina_list_append(world->cb_to_delete, cb);
        return;
     }

   world->callbacks = eina_inlist_remove(world->callbacks,
                                         EINA_INLIST_GET(cb));
   free(cb);
}

static void
_ephysics_world_tick_cb(btDynamicsWorld *dynamics_world, btScalar timeStep)
{
   Eina_Bool world_active, camera_moved, tx, ty;
   btCollisionObjectArray objects;
   EPhysics_World_Callback *cb;
   btRigidBody *rigid_body;
   EPhysics_World *world;
   EPhysics_Body *body;

   world = (EPhysics_World *) dynamics_world->getWorldUserInfo();

   world_active = EINA_FALSE;

   ephysics_camera_tracked_body_get(world->camera, &body, &tx, &ty);
   if ((body) && (tx || ty))
     {
        rigid_body = ephysics_body_rigid_body_get(body);
        if ((rigid_body) && (rigid_body->isActive()))
          ephysics_camera_target_moved(world->camera, body);
     }

   camera_moved = ephysics_camera_moved_get(world->camera);

   objects = dynamics_world->getCollisionObjectArray();
   for (int i = 0; i < objects.size(); i++)
     {
        btRigidBody *rigid_body = btRigidBody::upcast(objects[i]);
        if (!rigid_body)
          continue;

        body = (EPhysics_Body *) rigid_body->getUserPointer();
        if (rigid_body->isActive())
          {
             ephysics_body_active_set(body, EINA_TRUE);
             ephysics_body_evas_object_update_select(body);
             world_active = EINA_TRUE;
          }
        else
          {
             ephysics_body_active_set(body, EINA_FALSE);
             if (camera_moved)
               ephysics_body_evas_object_update_select(body);
          }
     }

   if (camera_moved)
     {
        _ephysics_world_event_callback_call(
           world, EPHYSICS_CALLBACK_WORLD_CAMERA_MOVED, world->camera);
        ephysics_camera_moved_set(world->camera, EINA_FALSE);
     }


   if (world->active == world_active) return;
   world->active = world_active;
   if (world_active) return;

   _ephysics_world_event_callback_call(world, EPHYSICS_CALLBACK_WORLD_STOPPED,
                                       NULL);
}

static void
_ephysics_world_free(EPhysics_World *world)
{
   EPhysics_World_Callback *cb;
   EPhysics_Body *body;
   void *constraint;

   _worlds = eina_inlist_remove(_worlds, EINA_INLIST_GET(world));

   while (world->callbacks)
     {
        cb = EINA_INLIST_CONTAINER_GET(world->callbacks,
                                       EPhysics_World_Callback);
        world->callbacks = eina_inlist_remove(world->callbacks,
                                              world->callbacks);
        free(cb);
     }

   while (world->bodies)
     {
        body = EINA_INLIST_CONTAINER_GET(world->bodies, EPhysics_Body);
        world->bodies = eina_inlist_remove(world->bodies, world->bodies);
        ephysics_orphan_body_del(body);
     }

   EINA_LIST_FREE(world->constraints, constraint)
      ephysics_constraint_del((EPhysics_Constraint *)constraint);

   ephysics_camera_del(world->camera);
   /* FIXME uncomment lines above when dynamicsworld destructor is fixed
      on bullet. Right now looks like it will try to acess invalid memory.*/
   /*
   delete world->dynamics_world;
   delete world->solver;
   delete world->broadphase;
   delete world->dispatcher;
   delete world->collision;
   */

   free(world);
   INF("World %p deleted.", world);
}

static Eina_Bool
_simulate_worlds(void *data)
{
   Eina_Inlist *lworlds = (Eina_Inlist *) data;
   EPhysics_World *world;
   double time_now;
   void *wrld, *bd;

   ephysics_init();
   _worlds_walking++;
   EINA_INLIST_FOREACH(lworlds, world)
     {
        double time_now, delta;

        if (!world->running)
          continue;

        world->walking++;

        time_now = ecore_time_get();
        delta = time_now - world->last_update;
        world->last_update = time_now;

        gDeactivationTime = world->max_sleeping_time;
        world->dynamics_world->stepSimulation(delta, world->max_sub_steps,
                                              world->fixed_time_step);
        world->walking--;

        if (!world->walking)
          {
             EINA_LIST_FREE(world->to_delete, bd)
                ephysics_world_body_del(world, (EPhysics_Body*)bd);
          }
     }
   _worlds_walking--;

   if (_worlds_walking > 0)
     {
        ephysics_shutdown();
        return EINA_TRUE;
     }

   EINA_LIST_FREE(_worlds_to_delete, wrld)
      _ephysics_world_free((EPhysics_World *)wrld);

   ephysics_shutdown();

   return EINA_TRUE;
}

static bool
_ephysics_world_contact_processed_cb(btManifoldPoint &cp, void *b0, void *b1)
{
   btRigidBody *rigid_body_0, *rigid_body_1;
   EPhysics_Body *body_0, *body_1;

   rigid_body_0 = (btRigidBody *) b0;
   rigid_body_1 = (btRigidBody *) b1;

   body_0 = (EPhysics_Body *) rigid_body_0->getUserPointer();
   body_1 = (EPhysics_Body *) rigid_body_1->getUserPointer();

   ephysics_body_contact_processed(body_0, body_1, cp.getPositionWorldOnA());
   ephysics_body_contact_processed(body_1, body_0, cp.getPositionWorldOnB());

   return EINA_TRUE;
}

static void
_ephysics_world_boundary_del_cb(void *data, EPhysics_Body *body, void *event_info)
{
   EPhysics_World *world = (EPhysics_World *) data;
   int i;

   for (i = 0; i < EPHYSICS_WORLD_BOUNDARY_LAST; i++)
     {
        if (world->boundaries[i] == body)
          {
             world->boundaries[i] = NULL;
             return;
          }
     }
}

Eina_Bool
ephysics_world_body_add(EPhysics_World *world, EPhysics_Body *body)
{
   world->bodies = eina_inlist_append(world->bodies,
                                      EINA_INLIST_GET(body));
   if (eina_error_get())
     {
        ERR("Couldn't add body to bodies list.");
        return EINA_FALSE;
     }

   world->dynamics_world->addRigidBody(ephysics_body_rigid_body_get(body));

   return EINA_TRUE;
}

Eina_Bool
ephysics_world_body_del(EPhysics_World *world, EPhysics_Body *body)
{
   if (world->walking)
     {
        world->to_delete = eina_list_append(world->to_delete, body);
        return EINA_FALSE;
     }

   world->dynamics_world->removeRigidBody(ephysics_body_rigid_body_get(body));
   world->bodies = eina_inlist_remove(world->bodies,
                                      EINA_INLIST_GET(body));
   ephysics_orphan_body_del(body);

   return EINA_TRUE;
}

void
ephysics_world_constraint_add(EPhysics_World *world, EPhysics_Constraint *constraint, btTypedConstraint *bt_constraint)
{
   world->dynamics_world->addConstraint(bt_constraint);
   world->constraints = eina_list_append(world->constraints, constraint);
}

void
ephysics_world_constraint_del(EPhysics_World *world, EPhysics_Constraint *constraint, btTypedConstraint *bt_constraint)
{
   world->dynamics_world->removeConstraint(bt_constraint);
   world->constraints = eina_list_remove(world->constraints, constraint);
}

int
ephysics_world_init(void)
{
   if (++_ephysics_world_init_count != 1)
     return _ephysics_world_init_count;

   gContactProcessedCallback = _ephysics_world_contact_processed_cb;

   INF("EPhysics World initialized.");

   return _ephysics_world_init_count;
}

int
ephysics_world_shutdown(void)
{
   if (--_ephysics_world_init_count != 0)
     return _ephysics_world_init_count;

   if (_anim_simulate)
     {
        ecore_animator_del(_anim_simulate);
        _anim_simulate = NULL;
     }

   while (_worlds)
     {
        EPhysics_World *world = EINA_INLIST_CONTAINER_GET(
           _worlds, EPhysics_World);
        ephysics_world_del(world);
     }

   _worlds_running = 0;

   INF("EPhysics World shutdown.");

   return _ephysics_world_init_count;
}

void
ephysics_world_boundary_set(EPhysics_World *world, EPhysics_World_Boundary boundary, EPhysics_Body *body)
{
   world->boundaries[boundary] = body;
   ephysics_body_event_callback_add(body, EPHYSICS_CALLBACK_BODY_DEL,
                                    _ephysics_world_boundary_del_cb, world);
}

EPhysics_Body *
ephysics_world_boundary_get(const EPhysics_World *world, EPhysics_World_Boundary boundary)
{
   return world->boundaries[boundary];
}

EAPI EPhysics_World *
ephysics_world_new(void)
{
   EPhysics_World *world;
   btOverlapFilterCallback *filter_cb;

   world = (EPhysics_World *) calloc(1, sizeof(EPhysics_World));
   if (!world)
     {
        ERR("Couldn't create a new world instance.");
        return NULL;
     }

   world->camera = ephysics_camera_add(world);
   if (!world->camera)
     {
        ERR("Couldn't create a camera for this world.");
        goto no_camera;
     }

   world->broadphase = new btDbvtBroadphase();
   if (!world->broadphase)
     {
        ERR("Couldn't set broadphase.");
        goto no_broadphase;
     }

   world->collision = new btDefaultCollisionConfiguration();
   if (!world->collision)
     {
        ERR("Couldn't configure collision.");
        goto no_collision;
     }

   world->dispatcher = new btCollisionDispatcher(world->collision);
   if (!world->dispatcher)
     {
        ERR("Couldn't create dispatcher.");
        goto no_dispatcher;
     }

   world->solver = new btSequentialImpulseConstraintSolver;
   if (!world->solver)
     {
        ERR("Couldn't create solver.");
        goto no_solver;
     }

   world->dynamics_world = new btDiscreteDynamicsWorld(
      world->dispatcher, world->broadphase, world->solver,
      world->collision);
   if (!world->dynamics_world)
     {
        ERR("Couldn't create dynamic world.");
        goto no_world;
     }

   _worlds = eina_inlist_append(_worlds, EINA_INLIST_GET(world));
   if (eina_error_get())
     {
        ERR("Couldn't add world to worlds list.");
        goto no_list;
     }

   world->dynamics_world->getSolverInfo().m_solverMode ^=
      EPHYSICS_WORLD_SOLVER_SIMD;
   world->dynamics_world->setGravity(btVector3(0, -9.8, 0));

   filter_cb = new _ephysics_world_ovelap_filter_cb();
   if (!filter_cb)
     INF("Couldn't initialize the collision filter.");
   else
     world->dynamics_world->getPairCache()->setOverlapFilterCallback(filter_cb);

   world->rate = 30;
   world->max_sub_steps = 3;
   world->fixed_time_step = 1/60.f;
   world->dynamics_world->setInternalTickCallback(_ephysics_world_tick_cb,
                                                  (void *) world);

   world->max_sleeping_time = 2.0;
   world->running = EINA_TRUE;
   world->last_update = ecore_time_get();
   _worlds_running++;
   if (!_anim_simulate)
     _anim_simulate = ecore_animator_add(_simulate_worlds, _worlds);

   INF("World %p added.", world);
   return world;

no_list:
   delete world->dynamics_world;
no_world:
   delete world->solver;
no_solver:
   delete world->dispatcher;
no_dispatcher:
   delete world->collision;
no_collision:
   delete world->broadphase;
no_broadphase:
   ephysics_camera_del(world->camera);
no_camera:
   free(world);
   return NULL;
}

EAPI Eina_Bool
ephysics_world_serialize(const EPhysics_World *world, const char *path)
{
   btDefaultSerializer *serializer;
   FILE *file;

   if (!world)
     {
        WRN("Could not serialize, world not provided.");
        return EINA_FALSE;
      }

   file = fopen(path, "wb");
   if (!file)
     {
        WRN("Could not serialize, could not open file: %s", path);
        return EINA_FALSE;
     }

   serializer = new btDefaultSerializer();
   world->dynamics_world->serialize(serializer);

   if (!fwrite(serializer->getBufferPointer(),
              serializer->getCurrentBufferSize(), 1, file))
     {
        WRN("Problems on writing to: %s.", path);
        fclose(file);
        delete serializer;
        return EINA_FALSE;
     }

   fclose(file);
   delete serializer;

   INF("Serialization of world %p written to file: %s.", world, path);

   return EINA_TRUE;
}

EAPI void
ephysics_world_del(EPhysics_World *world)
{
   EPhysics_World_Callback *cb;

   if (!world)
     {
        ERR("Can't delete world, it wasn't provided.");
        return;
     }

   if (world->deleted) return;

   world->deleted = EINA_TRUE;
   _ephysics_world_event_callback_call(world, EPHYSICS_CALLBACK_WORLD_DEL,
                                       NULL);
   ephysics_world_running_set(world, EINA_FALSE);

   if (_worlds_walking > 0)
     {
        _worlds_to_delete = eina_list_append(_worlds_to_delete, world);
        INF("World %p marked to delete.", world);
        return;
     }

   _ephysics_world_free(world);
}

EAPI void
ephysics_world_running_set(EPhysics_World *world, Eina_Bool running)
{
   if (!world)
     {
        ERR("Can't (un)pause world, it wasn't provided.");
        return;
     }

   if ((!!running) == world->running) return;

   world->running = !!running;

   if (world->running)
     {
        world->last_update = ecore_time_get();
        _worlds_running++;
        INF("World unpaused.");
     }
   else
     {
        _worlds_running--;
        INF("World paused.");
     }

   if (!_worlds_running)
     {
        if (_anim_simulate)
          {
             ecore_animator_del(_anim_simulate);
             _anim_simulate = NULL;
          }
        return;
     }

   if (_anim_simulate)
     return;

   _anim_simulate = ecore_animator_add(_simulate_worlds, _worlds);
}

EAPI Eina_Bool
ephysics_world_running_get(const EPhysics_World *world)
{
   if (!world)
     {
	ERR("No world, no running status for you.");
	return EINA_FALSE;
     }

   return world->running;
}

EAPI void
ephysics_world_max_sleeping_time_set(EPhysics_World *world, double sleeping_time)
{
   if (!world)
     {
	ERR("Can't set the world's max sleeping time, world is null.");
	return;
     }

   world->max_sleeping_time = sleeping_time;
}

EAPI double
ephysics_world_max_sleeping_time_get(const EPhysics_World *world)
{
   if (!world)
     {
	ERR("Can't get the world's max sleeping time, world is null.");
	return 0;
     }

   return world->max_sleeping_time;
}

EAPI void
ephysics_world_gravity_set(EPhysics_World *world, double gx, double gy)
{
   if (!world)
     {
        ERR("Can't set gravity, no world provided.");
        return;
     }

   _ephysics_world_gravity_set(world, gx, gy, world->rate);
   DBG("World %p gravity set to X:%lf, Y:%lf.", world, gx, gy);
}

EAPI void
ephysics_world_constraint_solver_iterations_set(EPhysics_World *world, int iterations)
{
   if (!world)
     {
        ERR("Can't set constraint solver iterations, world is null.");
        return;
     }

   world->dynamics_world->getSolverInfo().m_numIterations = iterations;
}

EAPI int
ephysics_world_constraint_solver_iterations_get(const EPhysics_World *world)
{
   if (!world)
     {
        ERR("Can't get constraint solver iterations, world is null.");
        return 0;
     }

   return world->dynamics_world->getSolverInfo().m_numIterations;
}

EAPI void
ephysics_world_constraint_solver_mode_enable_set(EPhysics_World *world, EPhysics_World_Solver_Mode solver_mode, Eina_Bool enable)
{
   int current_solver_mode;
    if (!world)
     {
        ERR("Can't enable/disable constraint solver mode, world is null.");
        return;
     }

   current_solver_mode = world->dynamics_world->getSolverInfo().m_solverMode;
   if ((enable && !(current_solver_mode & solver_mode)) ||
       (!enable && (current_solver_mode & solver_mode)))
     world->dynamics_world->getSolverInfo().m_solverMode ^= solver_mode;
}

EAPI Eina_Bool
ephysics_world_constraint_solver_mode_enable_get(const EPhysics_World *world, EPhysics_World_Solver_Mode solver_mode)
{
     if (!world)
     {
        ERR("Can't get constraint solver mode status, world is null.");
        return EINA_FALSE;
     }

     return world->dynamics_world->getSolverInfo().m_solverMode & solver_mode;
}

EAPI void
ephysics_world_gravity_get(const EPhysics_World *world, double *gx, double *gy)
{
   btVector3 vector;

   if (!world)
     {
	ERR("No world, can't get gravity.");
	return;
     }

   vector = world->dynamics_world->getGravity();

   if (gx) *gx = vector.x() * world->rate;
   if (gy) *gy = -vector.y() * world->rate;
}

EAPI void
ephysics_world_rate_set(EPhysics_World *world, double rate)
{
   EPhysics_Body *body;
   void *constraint;
   double gx, gy;
   Eina_List *l;

   if (!world)
     {
	ERR("No world, can't set rate.");
	return;
     }

   if (rate <= 0)
     {
	ERR("Rate should be a positive value. Keeping the old value: %lf.",
            world->rate);
        return;
     }

   /* Force to recalculate sizes, velocities and accelerations with new rate */
   ephysics_world_gravity_get(world, &gx, &gy);
   _ephysics_world_gravity_set(world, gx, gy, rate);

   EINA_INLIST_FOREACH(world->bodies, body)
      ephysics_body_recalc(body, rate);

   EINA_LIST_FOREACH(world->constraints, l, constraint)
        ephysics_constraint_recalc((EPhysics_Constraint *)constraint, rate);

   world->rate = rate;
}

EAPI double
ephysics_world_rate_get(const EPhysics_World *world)
{
   if (!world)
     {
	ERR("No world, can't get rate.");
	return 0;
     }

   return world->rate;
}

EAPI EPhysics_Camera *
ephysics_world_camera_get(const EPhysics_World *world)
{
   if (!world)
     {
	ERR("No world, no camera for you.");
	return NULL;
     }

   return world->camera;
}

EAPI void
ephysics_world_event_callback_add(EPhysics_World *world, EPhysics_Callback_World_Type type, EPhysics_World_Event_Cb func, const void *data)
{
   EPhysics_World_Callback *cb;

   if (!world)
     {
        ERR("Can't set world event callback, world is null.");
        return;
     }

   if (!func)
     {
        ERR("Can't set world event callback, function is null.");
        return;
     }

   if ((type < 0) || (type >= EPHYSICS_CALLBACK_WORLD_LAST))
     {
        ERR("Can't set world event callback, callback type is wrong.");
        return;
     }

   cb = (EPhysics_World_Callback *)calloc(1, sizeof(EPhysics_World_Callback));
   if (!cb)
     {
        ERR("Can't set world event callback, can't create cb instance.");
        return;
     }

   cb->func = func;
   cb->type = type;
   cb->data = (void *) data;

   world->callbacks = eina_inlist_append(world->callbacks, EINA_INLIST_GET(cb));
}

EAPI void *
ephysics_world_event_callback_del(EPhysics_World *world, EPhysics_Callback_World_Type type, EPhysics_World_Event_Cb func)
{
   EPhysics_World_Callback *cb;
   void *cb_data = NULL;

   if (!world)
     {
        ERR("Can't delete world event callback, world is null.");
        return NULL;
     }

   EINA_INLIST_FOREACH(world->callbacks, cb)
     {
        if ((cb->type != type) || (cb->func != func))
          continue;

        cb_data = cb->data;
        _ephysics_world_event_callback_del(world, cb);
        break;
     }

   return cb_data;
}

EAPI void *
ephysics_world_event_callback_del_full(EPhysics_World *world, EPhysics_Callback_World_Type type, EPhysics_World_Event_Cb func, void *data)
{
   EPhysics_World_Callback *cb;
   void *cb_data = NULL;

   if (!world)
     {
        ERR("Can't delete world event callback, world is null.");
        return NULL;
     }

   EINA_INLIST_FOREACH(world->callbacks, cb)
     {
        if ((cb->type != type) || (cb->func != func) || (cb->data != data))
          continue;

        cb_data = cb->data;
        _ephysics_world_event_callback_del(world, cb);
        break;
     }

   return cb_data;
}

EAPI Eina_List *
ephysics_world_bodies_get(const EPhysics_World *world)
{
   Eina_List *list = NULL;
   EPhysics_Body *body;

   if (!world)
     {
        ERR("Couldn't get the bodies list, no world provided.");
        return NULL;
     }

   EINA_INLIST_FOREACH(world->bodies, body)
      list = eina_list_append(list, body);

   return list;
}

EAPI void
ephysics_world_render_geometry_set(EPhysics_World *world, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   if (!world)
     {
        ERR("Can't set geometry, world wasn't provided.");
        return;
     }

   if ((w <= 0) || (h <= 0))
     {
        ERR("Invalid width or height sizes. They must to be positive values.");
        return;
     }

   world->x = x;
   world->y = y;
   world->w = w;
   world->h = h;

   ephysics_body_world_boundaries_resize(world);
   ephysics_camera_position_set(world->camera, x, y);
}

EAPI void
ephysics_world_render_geometry_get(const EPhysics_World *world, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   if (!world)
     {
        ERR("Can't get geometry, world is null.");
        return;
     }

   if (x) *x = world->x;
   if (y) *y = world->y;
   if (w) *w = world->w;
   if (h) *h = world->h;
}

EAPI void
ephysics_world_linear_slop_set(EPhysics_World *world, double linear_slop)
{
   if (!world)
     {
        ERR("Can't set linear slop, world is null.");
        return;
     }

   world->dynamics_world->getSolverInfo().m_linearSlop = btScalar(linear_slop);
}

EAPI double
ephysics_world_linear_slop_get(const EPhysics_World *world)
{
   if (!world)
     {
        ERR("Can't get linear slop, world is null.");
        return 0;
     }

   return world->dynamics_world->getSolverInfo().m_linearSlop;
}

EAPI void
ephysics_world_bodies_outside_top_autodel_set(EPhysics_World *world, Eina_Bool autodel)
{
   if (!world)
     {
        ERR("Can't set autodelete mode, world is null.");
        return;
     }

   world->outside_top = !!autodel;
   world->outside_autodel = world->outside_top || world->outside_bottom ||
      world->outside_left || world->outside_right;
}

EAPI Eina_Bool
ephysics_world_bodies_outside_top_autodel_get(const EPhysics_World *world)
{
   if (!world)
     {
        ERR("Can't get autodelete mode, world is null.");
        return EINA_FALSE;
     }

   return world->outside_top;
}

EAPI void
ephysics_world_bodies_outside_bottom_autodel_set(EPhysics_World *world, Eina_Bool autodel)
{
   if (!world)
     {
        ERR("Can't set autodelete mode, world is null.");
        return;
     }

   world->outside_bottom = !!autodel;
   world->outside_autodel = world->outside_top || world->outside_bottom ||
      world->outside_left || world->outside_right;
}

EAPI Eina_Bool
ephysics_world_bodies_outside_bottom_autodel_get(const EPhysics_World *world)
{
   if (!world)
     {
        ERR("Can't get autodelete mode, world is null.");
        return EINA_FALSE;
     }

   return world->outside_bottom;
}

EAPI void
ephysics_world_bodies_outside_left_autodel_set(EPhysics_World *world, Eina_Bool autodel)
{
   if (!world)
     {
        ERR("Can't set autodelete mode, world is null.");
        return;
     }

   world->outside_left = !!autodel;
   world->outside_autodel = world->outside_top || world->outside_bottom ||
      world->outside_left || world->outside_right;
}

EAPI Eina_Bool
ephysics_world_bodies_outside_left_autodel_get(const EPhysics_World *world)
{
   if (!world)
     {
        ERR("Can't get autodelete mode, world is null.");
        return EINA_FALSE;
     }

   return world->outside_left;
}

EAPI void
ephysics_world_bodies_outside_right_autodel_set(EPhysics_World *world, Eina_Bool autodel)
{
   if (!world)
     {
        ERR("Can't set autodelete mode, world is null.");
        return;
     }

   world->outside_right = !!autodel;
   world->outside_autodel = world->outside_top || world->outside_bottom ||
      world->outside_left || world->outside_right;
}

EAPI Eina_Bool
ephysics_world_bodies_outside_right_autodel_get(const EPhysics_World *world)
{
   if (!world)
     {
        ERR("Can't get autodelete mode, world is null.");
        return EINA_FALSE;
     }

   return world->outside_right;
}

EAPI Eina_Bool
ephysics_world_bodies_outside_autodel_get(const EPhysics_World *world)
{
   if (!world)
     {
        ERR("Can't get autodelete mode, world is null.");
        return EINA_FALSE;
     }

   return world->outside_autodel;
}

EAPI void
ephysics_world_simulation_set(EPhysics_World *world, double fixed_time_step, int max_sub_steps)
{
   if (!world)
     {
        ERR("Can't set simulation, no world provided.");
        return;
     }

   if (max_sub_steps < 1)
     {
        ERR("At least one sub step for simulation is required.");
        return;
     }

   if (ecore_animator_frametime_get() >= max_sub_steps * fixed_time_step)
     {
        ERR("Assure frametime < max sub steps * fixed time step.");
        return;
     }

   world->max_sub_steps = max_sub_steps;
   world->fixed_time_step = fixed_time_step;

   DBG("World %p simulation set to fixed time step: %lf, max substeps:%i.",
       world, fixed_time_step, max_sub_steps);
}

EAPI void
ephysics_world_simulation_get(const EPhysics_World *world, double *fixed_time_step, int *max_sub_steps)
{
   if (!world)
     {
	ERR("No world, can't get simulation configuration.");
	return;
     }

   if (fixed_time_step) *fixed_time_step = world->fixed_time_step;
   if (max_sub_steps) *max_sub_steps = world->max_sub_steps;
}

#ifdef  __cplusplus
}
#endif
