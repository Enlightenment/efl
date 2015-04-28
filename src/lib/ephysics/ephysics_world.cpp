#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Ecore.h>
#include <Evas.h>

#include "ephysics_private.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define DEFAULT_GRAVITY btVector3(0, -9.8, 0)

typedef struct _Simulation_Msg Simulation_Msg;

struct _Simulation_Msg {
     EPhysics_Body *body_0;
     EPhysics_Body *body_1;
     btVector3 pos_a;
     btVector3 pos_b;
     Eina_Bool tick:1;
};

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

     struct {
          Evas_Coord x;
          Evas_Coord y;
          Evas_Coord z;
          Evas_Coord w;
          Evas_Coord h;
          Evas_Coord d;
     } geometry;

     btBroadphaseInterface *broadphase;
     btDefaultCollisionConfiguration *collision;
     btCollisionDispatcher *dispatcher;
     btSequentialImpulseConstraintSolver *solver;
     btSoftRigidDynamicsWorld *dynamics_world;
     btSoftBodyWorldInfo *world_info;
     btSoftBodySolver *soft_solver;
     btOverlapFilterCallback *filter_cb;

     EPhysics_Body *boundaries[6];
     EPhysics_Camera *camera;
     Eina_Inlist *callbacks;
     Eina_Inlist *bodies;
     Eina_List *to_delete;
     Eina_List *cb_to_delete;
     Eina_List *constraints;
     Ecore_Thread *simulation_th;
     Ecore_Thread *cur_th;
     int max_sub_steps;
     int cb_walking;
     int soft_body_ref;
     int pending_ticks;
     double last_update;
     double rate;
     double fixed_time_step;
     double max_sleeping_time;
     Eina_Lock mutex;
     Eina_Condition condition;

     struct {
          Evas_Coord lx;
          Evas_Coord ly;
          Evas_Coord lz;
          int lr;
          int lg;
          int lb;
          int ar;
          int ag;
          int ab;
          Eina_Bool all_bodies:1;
     } light;

     Eina_Bool running:1;
     Eina_Bool ticked:1;
     Eina_Bool active:1;
     Eina_Bool deleted:1;
     Eina_Bool outside_autodel:1;
     Eina_Bool outside_top:1;
     Eina_Bool outside_bottom:1;
     Eina_Bool outside_left:1;
     Eina_Bool outside_right:1;
     Eina_Bool outside_front:1;
     Eina_Bool outside_back:1;
     Eina_Bool pending_simulation:1;
     Eina_Bool stacking:1;
     Eina_Bool force_update:1;
};

static int _ephysics_world_init_count = 0;
static int _worlds_running = 0;
static Eina_Inlist *_worlds = NULL;
static Eina_List *_worlds_to_delete = NULL;
static Ecore_Animator *_anim_simulate = NULL;
static int _worlds_walking = 0;

void
ephysics_world_force_update_set(EPhysics_World *world, Eina_Bool force_update)
{
   world->force_update = force_update;
}

btSoftBodyWorldInfo *
ephysics_world_info_get(const EPhysics_World *world)
{
   return world->world_info;
}

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
_ephysics_world_gravity_set(EPhysics_World *world, double gx, double gy, double gz, double rate)
{
   btVector3 gravity;

   gravity = btVector3(gx / rate, -gy / rate, gz / rate);
   world->dynamics_world->setGravity(gravity);
   world->world_info->m_gravity = gravity;
}

static void
_ephysics_world_th_cancel(EPhysics_World *world)
{
   _worlds = eina_inlist_remove(_worlds, EINA_INLIST_GET(world));
   if (!ecore_thread_cancel(world->simulation_th))
     eina_condition_signal(&world->condition);
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
_ephysics_world_tick(btDynamicsWorld *dynamics_world)
{
   Eina_Bool world_active, camera_moved, tx, ty;
   btCollisionObjectArray objects;
   btCollisionObject *collision;
   EPhysics_World *world;
   EPhysics_Body *body;
   btRigidBody *rigid_body;

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
        collision = objects[i];

        if (!collision)
          continue;

        body = (EPhysics_Body *) collision->getUserPointer();
        if (collision->isActive())
          {
             ephysics_body_active_set(body, EINA_TRUE);
             ephysics_body_evas_object_update_select(body);
             world_active = EINA_TRUE;
          }
        else
          {
             ephysics_body_active_set(body, EINA_FALSE);
             if (camera_moved || world->force_update)
               ephysics_body_evas_object_update_select(body);
          }
     }

   if (world->stacking)
     ephysics_body_evas_objects_restack(world);

   world->force_update = EINA_FALSE;
   if (camera_moved)
     {
        ephysics_camera_moved_set(world->camera, EINA_FALSE);
        _ephysics_world_event_callback_call(
           world, EPHYSICS_CALLBACK_WORLD_CAMERA_MOVED, world->camera);
     }

   if (world->active == world_active) goto body_del;
   world->active = world_active;
   if (world_active) goto body_del;

   _ephysics_world_event_callback_call(world, EPHYSICS_CALLBACK_WORLD_STOPPED,
                                       NULL);

body_del:
   if (world_active)
     _ephysics_world_event_callback_call(world, EPHYSICS_CALLBACK_WORLD_UPDATE,
                                         NULL);
   world->pending_ticks--;
   if (!world->pending_ticks)
     {
        void *bd;
        EINA_LIST_FREE(world->to_delete, bd)
           ephysics_world_body_del(world, (EPhysics_Body*)bd);
     }

   if ((world->pending_simulation) && (!world->pending_ticks))
     {
        world->pending_simulation = EINA_FALSE;
        eina_condition_signal(&world->condition);
     }
}

static void
_ephysics_world_tick_dispatch(EPhysics_World *world)
{
   Simulation_Msg *msg;

   if (!world->ticked)
     return;

   world->ticked = EINA_FALSE;
   world->pending_ticks++;

   msg = (Simulation_Msg *) calloc(1, sizeof(Simulation_Msg));
   msg->tick = EINA_TRUE;
   ecore_thread_feedback(world->cur_th, msg);
}

static void
_ephysics_world_tick_cb(btDynamicsWorld *dynamics_world, btScalar timeStep EINA_UNUSED)
{
   EPhysics_World *world;
   world = (EPhysics_World *) dynamics_world->getWorldUserInfo();
   world->ticked = EINA_TRUE;
}

static void
_ephysics_world_body_del(EPhysics_World *world, EPhysics_Body *body)
{
   btSoftBody *soft_body;

   if (ephysics_body_rigid_body_get(body))
     world->dynamics_world->removeRigidBody(ephysics_body_rigid_body_get(body));

   soft_body = ephysics_body_soft_body_get(body);
   if (soft_body)
     {
        world->dynamics_world->removeSoftBody(soft_body);
        --world->soft_body_ref;
     }

   world->bodies = eina_inlist_remove(world->bodies, EINA_INLIST_GET(body));
   ephysics_orphan_body_del(body);
}

static void
_ephysics_world_boundary_del(EPhysics_World *world, EPhysics_Body *body)
{
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
ephysics_world_body_del(EPhysics_World *world, EPhysics_Body *body)
{
   EPhysics_Body *bd;

   if (body->boundary)
     _ephysics_world_boundary_del(world, body);

   if (world->pending_ticks)
     {
        world->to_delete = eina_list_append(world->to_delete, body);
        return EINA_FALSE;
     }

   _ephysics_world_body_del(world, body);

   /* Activate all the bodies after a body is deleted.
      Otherwise it can lead to scenarios when a body 1, below body 2 is deleted
      and body 2 will stay freezed in the air. Gravity won't start to
      act over it until it's activated again. */
   EINA_INLIST_FOREACH(world->bodies, bd)
     ephysics_body_activate(bd, EINA_TRUE);

   return EINA_TRUE;
}

static void
_ephysics_world_free(EPhysics_World *world)
{
   EPhysics_World_Callback *cb;
   EPhysics_Body *body;
   void *constraint;

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
        _ephysics_world_body_del(world, body);
     }

   EINA_LIST_FREE(world->constraints, constraint)
      ephysics_constraint_del((EPhysics_Constraint *)constraint);

   ephysics_camera_del(world->camera);

   delete world->filter_cb;
   delete world->dynamics_world;
   delete world->solver;
   delete world->broadphase;
   delete world->dispatcher;
   delete world->collision;
   delete world->soft_solver;
   delete world->world_info;

   eina_condition_free(&world->condition);
   eina_lock_free(&world->mutex);

   INF("World %p deleted.", world);
   free(world);
   ephysics_dom_count_dec();
}

static Eina_Bool
_simulate_worlds(void *data EINA_UNUSED)
{
   EPhysics_World *world;
   void *wrld;

   ephysics_init();
   _worlds_walking++;
   EINA_INLIST_FOREACH(_worlds, world)
     {
        if (!world->running)
          continue;

        if (world->pending_ticks)
          {
             world->pending_simulation = EINA_TRUE;
             continue;
          }

        world->pending_simulation = EINA_FALSE;

        eina_condition_signal(&world->condition);
     }
   _worlds_walking--;

   if (_worlds_walking > 0)
     {
        ephysics_shutdown();
        return EINA_TRUE;
     }

   EINA_LIST_FREE(_worlds_to_delete, wrld)
      _ephysics_world_th_cancel((EPhysics_World *)wrld);

   ephysics_shutdown();

   return EINA_TRUE;
}

static bool
_ephysics_world_contact_processed_cb(btManifoldPoint &cp, void *b0, void *b1)
{
   btRigidBody *rigid_body_0, *rigid_body_1;
   EPhysics_Body *body_0, *body_1;
   EPhysics_World *world;
   Simulation_Msg *msg;

   rigid_body_0 = (btRigidBody *) b0;
   rigid_body_1 = (btRigidBody *) b1;

   body_0 = (EPhysics_Body *) rigid_body_0->getUserPointer();
   body_1 = (EPhysics_Body *) rigid_body_1->getUserPointer();

   world = ephysics_body_world_get(body_0);

   msg = (Simulation_Msg *) calloc(1, sizeof(Simulation_Msg));
   msg->body_0 = body_0;
   msg->body_1 = body_1;
   msg->pos_a = cp.getPositionWorldOnA();
   msg->pos_b = cp.getPositionWorldOnB();
   ecore_thread_feedback(world->cur_th, msg);

   return EINA_TRUE;
}

Eina_Bool
ephysics_world_body_add(EPhysics_World *world, EPhysics_Body *body)
{
   if (!eina_inlist_find(world->bodies, EINA_INLIST_GET(body)))
       world->bodies = eina_inlist_append(world->bodies, EINA_INLIST_GET(body));

   if (eina_error_get())
     {
        ERR("Couldn't add body to bodies list.");
        return EINA_FALSE;
     }

   world->dynamics_world->addRigidBody(ephysics_body_rigid_body_get(body));
   return EINA_TRUE;
}

Eina_Bool
ephysics_world_soft_body_add(EPhysics_World *world, EPhysics_Body *body)
{
   if (!eina_inlist_find(world->bodies, EINA_INLIST_GET(body)))
       world->bodies = eina_inlist_append(world->bodies, EINA_INLIST_GET(body));

   if (eina_error_get())
     {
        ERR("Couldn't add body to bodies list.");
        return EINA_FALSE;
     }

   ++world->soft_body_ref;
   world->dynamics_world->addSoftBody(ephysics_body_soft_body_get(body));
   return EINA_TRUE;
}

Eina_List *
ephysics_world_constraints_get(EPhysics_World *world)
{
   return world->constraints;
}

void
ephysics_world_constraint_add(EPhysics_World *world, EPhysics_Constraint *constraint, btGeneric6DofConstraint *bt_constraint)
{
   world->dynamics_world->addConstraint(bt_constraint, true);
   world->constraints = eina_list_append(world->constraints, constraint);
}

void
ephysics_world_constraint_del(EPhysics_World *world, EPhysics_Constraint *constraint, btGeneric6DofConstraint *bt_constraint)
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
}

EPhysics_Body *
ephysics_world_boundary_get(const EPhysics_World *world, EPhysics_World_Boundary boundary)
{
   return world->boundaries[boundary];
}

static void
_th_simulate(void *data, Ecore_Thread *th)
{
   EPhysics_World *world = (EPhysics_World *) data;

   while (1)
     {
        double time_now, delta;
        EPhysics_Body *body;

        eina_condition_wait(&world->condition);
        if (ecore_thread_check(th))
          {
             INF("Thread canceled by main loop thread");
             eina_lock_release(&world->mutex);
             return;
          }

        world->pending_ticks++;
        world->cur_th = th;

        EINA_INLIST_FOREACH(world->bodies, body)
          {
             ephysics_body_forces_apply(body);
             if (body->dragging_data.dragging)
               ephysics_body_soft_body_dragging_apply(body);

             if (body->bending_constraints)
               ephysics_body_soft_body_bending_constraints_generate(body);
          }

        time_now = ecore_time_get();
        delta = time_now - world->last_update;
        world->last_update = time_now;

        gDeactivationTime = world->max_sleeping_time;

        if (world->soft_body_ref)
          {
             world->dynamics_world->stepSimulation(delta, world->max_sub_steps,
                                                world->fixed_time_step);
             world->world_info->m_sparsesdf.GarbageCollect();
          }
        else
          ((btDiscreteDynamicsWorld *)world->dynamics_world)->stepSimulation(
             delta, world->max_sub_steps, world->fixed_time_step);

        _ephysics_world_tick_dispatch(world);
        world->pending_ticks--;
        eina_lock_release(&world->mutex);
     }
}

static void
_th_msg_cb(void *data, Ecore_Thread *th EINA_UNUSED, void *msg_data)
{
   EPhysics_World *world = (EPhysics_World *) data;
   Simulation_Msg *msg = (Simulation_Msg *) msg_data;

   if (msg->tick)
     _ephysics_world_tick(world->dynamics_world);
   else
     {
        ephysics_body_contact_processed(msg->body_0, msg->body_1, msg->pos_a);
        ephysics_body_contact_processed(msg->body_1, msg->body_0, msg->pos_b);
     }
   free(msg);
}

static void
_th_end_cb(void *data, Ecore_Thread *th)
{
   EPhysics_World *world = (EPhysics_World *) data;
   INF("World %p simulation thread %p end", world, th);
   world->simulation_th = NULL;
   _ephysics_world_free(world);
}

static void
_th_cancel_cb(void *data, Ecore_Thread *th)
{
   EPhysics_World *world = (EPhysics_World *) data;
   INF("World %p simulation thread %p canceled", world, th);
   world->simulation_th = NULL;
   _ephysics_world_free(world);
}

EAPI EPhysics_World *
ephysics_world_new(void)
{
   EPhysics_World *world;

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

   world->collision = new btSoftBodyRigidBodyCollisionConfiguration();
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

   world->soft_solver = new btDefaultSoftBodySolver();
   if (!world->soft_solver)
     {
        ERR("Couldn't create soft body solver.");
        goto no_soft_solver;
     }

   world->dynamics_world = new btSoftRigidDynamicsWorld(
      world->dispatcher, world->broadphase, world->solver,
      world->collision, world->soft_solver);
   if (!world->dynamics_world)
     {
        ERR("Couldn't create dynamic world.");
        goto no_world;
     }

   world->world_info = new btSoftBodyWorldInfo();
   if (!world->world_info)
     {
        ERR("Couldn't create soft body world info.");
        goto no_world_info;
     }

   world->world_info->m_gravity = DEFAULT_GRAVITY;
   world->world_info->m_broadphase = world->broadphase;
   world->world_info->m_dispatcher = world->dispatcher;
   world->world_info->m_sparsesdf.Initialize();

   _worlds = eina_inlist_append(_worlds, EINA_INLIST_GET(world));
   if (eina_error_get())
     {
        ERR("Couldn't add world to worlds list.");
        goto no_list;
     }
   world->simulation_th = ecore_thread_feedback_run(
      _th_simulate, _th_msg_cb, _th_end_cb, _th_cancel_cb, world, EINA_TRUE);
   if (!world->simulation_th)
     {
        ERR("Failed to create simulation thread.");
        goto no_thread;
     }
   eina_lock_new(&world->mutex);
   eina_condition_new(&world->condition, &world->mutex);

   world->dynamics_world->getSolverInfo().m_solverMode ^=
      EPHYSICS_WORLD_SOLVER_SIMD;
   world->dynamics_world->setGravity(DEFAULT_GRAVITY);

   world->filter_cb = new _ephysics_world_ovelap_filter_cb();
   if (!world->filter_cb)
     INF("Couldn't initialize the collision filter.");
   else
     world->dynamics_world->getPairCache()->setOverlapFilterCallback(
        world->filter_cb);

   world->light.lr = 255;
   world->light.lg = 255;
   world->light.lb = 255;
   world->light.lz = -200;

   world->stacking = EINA_TRUE;
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
     _anim_simulate = ecore_animator_add(_simulate_worlds, NULL);

   ephysics_dom_count_inc();
   INF("World %p added.", world);
   return world;

no_thread:
   _worlds = eina_inlist_remove(_worlds, EINA_INLIST_GET(world));
no_list:
   delete world->world_info;
no_world_info:
   delete world->dynamics_world;
no_world:
   delete world->soft_solver;
no_soft_solver:
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
ephysics_world_serialize(EPhysics_World *world, const char *path)
{
   btDefaultSerializer *serializer;
   FILE *file;

   if (!world)
     {
        WRN("Could not serialize, world not provided.");
        return EINA_FALSE;
      }

   eina_lock_take(&world->mutex);

   file = fopen(path, "wb");
   if (!file)
     {
        WRN("Could not serialize, could not open file: %s", path);
        eina_lock_release(&world->mutex);
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
        eina_lock_release(&world->mutex);
        return EINA_FALSE;
     }

   fclose(file);
   delete serializer;

   INF("Serialization of world %p written to file: %s.", world, path);

   eina_lock_release(&world->mutex);
   return EINA_TRUE;
}

static void
_ephysics_world_running_set(EPhysics_World *world, Eina_Bool running)
{
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

   _anim_simulate = ecore_animator_add(_simulate_worlds, NULL);
}

EAPI void
ephysics_world_del(EPhysics_World *world)
{
   if (!world)
     {
        ERR("Can't delete world, it wasn't provided.");
        return;
     }

   if (world->deleted) return;

   eina_lock_take(&world->mutex);

   world->deleted = EINA_TRUE;
   _ephysics_world_event_callback_call(world, EPHYSICS_CALLBACK_WORLD_DEL,
                                       NULL);
   _ephysics_world_running_set(world, EINA_FALSE);

   if (_worlds_walking > 0)
     {
        _worlds_to_delete = eina_list_append(_worlds_to_delete, world);
        INF("World %p marked to delete.", world);
        eina_lock_release(&world->mutex);
        return;
     }

   eina_lock_release(&world->mutex);
   _ephysics_world_th_cancel(world);
}

EAPI void
ephysics_world_running_set(EPhysics_World *world, Eina_Bool running)
{
   if (!world)
     {
        ERR("Can't (un)pause world, it wasn't provided.");
        return;
     }

   eina_lock_take(&world->mutex);
   _ephysics_world_running_set(world, running);
   eina_lock_release(&world->mutex);
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

   eina_lock_take(&world->mutex);
   world->max_sleeping_time = sleeping_time;
   eina_lock_release(&world->mutex);
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
ephysics_world_gravity_set(EPhysics_World *world, double gx, double gy, double gz)
{
   EPhysics_Body *bd;

   if (!world)
     {
        ERR("Can't set gravity, no world provided.");
        return;
     }

   eina_lock_take(&world->mutex);
   EINA_INLIST_FOREACH(world->bodies, bd)
      ephysics_body_activate(bd, EINA_TRUE);
   _ephysics_world_gravity_set(world, gx, gy, gz, world->rate);
   DBG("World %p gravity set to X:%lf, Y:%lf, Z: %lf.", world, gx, gy, gz);
   eina_lock_release(&world->mutex);
}

EAPI void
ephysics_world_constraint_solver_iterations_set(EPhysics_World *world, int iterations)
{
   if (!world)
     {
        ERR("Can't set constraint solver iterations, world is null.");
        return;
     }

   eina_lock_take(&world->mutex);
   world->dynamics_world->getSolverInfo().m_numIterations = iterations;
   eina_lock_release(&world->mutex);
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

   eina_lock_take(&world->mutex);
   current_solver_mode = world->dynamics_world->getSolverInfo().m_solverMode;
   if ((enable && !(current_solver_mode & solver_mode)) ||
       (!enable && (current_solver_mode & solver_mode)))
     world->dynamics_world->getSolverInfo().m_solverMode ^= solver_mode;
   eina_lock_release(&world->mutex);
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
ephysics_world_gravity_get(const EPhysics_World *world, double *gx, double *gy, double *gz)
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
   if (gz) *gz = vector.z() * world->rate;
}

EAPI void
ephysics_world_rate_set(EPhysics_World *world, double rate)
{
   EPhysics_Body *body;
   double gx, gy, gz;
   void *constraint;
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

   eina_lock_take(&world->mutex);
   /* Force to recalculate sizes, velocities and accelerations with new rate */
   ephysics_world_gravity_get(world, &gx, &gy, &gz);
   _ephysics_world_gravity_set(world, gx, gy, gz, rate);

   EINA_INLIST_FOREACH(world->bodies, body)
      ephysics_body_recalc(body, rate);

   EINA_LIST_FOREACH(world->constraints, l, constraint)
        ephysics_constraint_recalc((EPhysics_Constraint *)constraint, rate);

   world->rate = rate;
   eina_lock_release(&world->mutex);
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
ephysics_world_render_geometry_set(EPhysics_World *world, Evas_Coord x, Evas_Coord y, Evas_Coord z, Evas_Coord w, Evas_Coord h, Evas_Coord d)
{
   if (!world)
     {
        ERR("Can't set geometry, world wasn't provided.");
        return;
     }

   if ((w <= 0) || (h <= 0) || (d <= 0))
     {
        ERR("Invalid width or height sizes. They must to be positive values.");
        return;
     }

   world->geometry.x = x;
   world->geometry.y = y;
   world->geometry.z = z;
   world->geometry.w = w;
   world->geometry.h = h;
   world->geometry.d = d;

   INF("World %p render geometry set: x=%i y=%i z=%i w=%i h=%i d=%i",
       world, x, y, z, w, h, d);

   ephysics_camera_perspective_set(world->camera, x + w / 2, y + h / 2,
                                   z + d / 2, 10 * (z + d));

   ephysics_body_world_boundaries_resize(world);
   ephysics_camera_position_set(world->camera, x, y);
}

EAPI void
ephysics_world_render_geometry_get(const EPhysics_World *world, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z, Evas_Coord *w, Evas_Coord *h, Evas_Coord *d)
{
   if (!world)
     {
        ERR("Can't get geometry, world is null.");
        return;
     }

   if (x) *x = world->geometry.x;
   if (y) *y = world->geometry.y;
   if (z) *z = world->geometry.z;
   if (w) *w = world->geometry.w;
   if (h) *h = world->geometry.h;
   if (d) *d = world->geometry.d;
}

EAPI void
ephysics_world_linear_slop_set(EPhysics_World *world, double linear_slop)
{
   if (!world)
     {
        ERR("Can't set linear slop, world is null.");
        return;
     }

   eina_lock_take(&world->mutex);
   world->dynamics_world->getSolverInfo().m_linearSlop = btScalar(linear_slop);
   eina_lock_release(&world->mutex);
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
      world->outside_left || world->outside_right ||
      world->outside_front || world->outside_back;
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
      world->outside_left || world->outside_right ||
      world->outside_front || world->outside_back;
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
      world->outside_left || world->outside_right ||
      world->outside_front || world->outside_back;
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
      world->outside_left || world->outside_right ||
      world->outside_front || world->outside_back;
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

EAPI void
ephysics_world_bodies_outside_front_autodel_set(EPhysics_World *world, Eina_Bool autodel)
{
   if (!world)
     {
        ERR("Can't set autodelete mode, world is null.");
        return;
     }

   world->outside_front = !!autodel;
   world->outside_autodel = world->outside_top || world->outside_bottom ||
      world->outside_left || world->outside_right ||
      world->outside_front || world->outside_back;
}

EAPI Eina_Bool
ephysics_world_bodies_outside_front_autodel_get(const EPhysics_World *world)
{
   if (!world)
     {
        ERR("Can't get autodelete mode, world is null.");
        return EINA_FALSE;
     }

   return world->outside_front;
}

EAPI void
ephysics_world_bodies_outside_back_autodel_set(EPhysics_World *world, Eina_Bool autodel)
{
   if (!world)
     {
        ERR("Can't set autodelete mode, world is null.");
        return;
     }

   world->outside_back = !!autodel;
   world->outside_autodel = world->outside_top || world->outside_bottom ||
      world->outside_left || world->outside_right ||
      world->outside_front || world->outside_back;
}

EAPI Eina_Bool
ephysics_world_bodies_outside_back_autodel_get(const EPhysics_World *world)
{
   if (!world)
     {
        ERR("Can't get autodelete mode, world is null.");
        return EINA_FALSE;
     }

   return world->outside_back;
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

   eina_lock_take(&world->mutex);
   world->max_sub_steps = max_sub_steps;
   world->fixed_time_step = fixed_time_step;

   DBG("World %p simulation set to fixed time step: %lf, max substeps:%i.",
       world, fixed_time_step, max_sub_steps);
   eina_lock_release(&world->mutex);
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

void
ephysics_world_lock_take(EPhysics_World *world)
{
   eina_lock_take(&world->mutex);
}

void
ephysics_world_lock_release(EPhysics_World *world)
{
   eina_lock_release(&world->mutex);
}

EAPI void
ephysics_world_point_light_position_set(EPhysics_World *world, Evas_Coord lx, Evas_Coord ly, Evas_Coord lz)
{
   if (!world)
     {
	ERR("No world, can't set light properties.");
	return;
     }

   world->light.lx = lx;
   world->light.ly = ly;
   world->light.lz = lz;
   world->force_update = EINA_TRUE;
}

EAPI void
ephysics_world_point_light_color_set(EPhysics_World *world, int lr, int lg, int lb)
{
   if (!world)
     {
	ERR("No world, can't set light properties.");
	return;
     }

   world->light.lr = lr;
   world->light.lg = lg;
   world->light.lb = lb;
   world->force_update = EINA_TRUE;
}

EAPI void
ephysics_world_ambient_light_color_set(EPhysics_World *world, int ar, int ag, int ab)
{
   if (!world)
     {
	ERR("No world, can't set light properties.");
	return;
     }

   world->light.ar = ar;
   world->light.ag = ag;
   world->light.ab = ab;
   world->force_update = EINA_TRUE;
}

EAPI void
ephysics_world_ambient_light_color_get(const EPhysics_World *world, int *ar, int *ag, int *ab)
{
   if (!world)
     {
	ERR("No world, can't get light properties.");
	return;
     }

   if (ar) *ar = world->light.ar;
   if (ag) *ag = world->light.ag;
   if (ab) *ab = world->light.ab;
}

EAPI void
ephysics_world_point_light_color_get(const EPhysics_World *world, int *lr, int *lg, int *lb)
{
   if (!world)
     {
	ERR("No world, can't get light properties.");
	return;
     }

   if (lr) *lr = world->light.lr;
   if (lg) *lg = world->light.lg;
   if (lb) *lb = world->light.lb;
}

EAPI void
ephysics_world_point_light_position_get(const EPhysics_World *world, Evas_Coord *lx, Evas_Coord *ly, Evas_Coord *lz)
{
   if (!world)
     {
	ERR("No world, can't get light properties.");
	return;
     }

   if (lx) *lx = world->light.lx;
   if (ly) *ly = world->light.ly;
   if (lz) *lz = world->light.lz;
}

EAPI void
ephysics_world_light_all_bodies_set(EPhysics_World *world, Eina_Bool enable)
{
   if (!world)
     {
	ERR("No world, can't set light property.");
	return;
     }

   world->light.all_bodies = !!enable;
   world->force_update = EINA_TRUE;
}

EAPI Eina_Bool
ephysics_world_light_all_bodies_get(const EPhysics_World *world)
{
   if (!world)
     {
	ERR("No world, can't get light property.");
	return EINA_FALSE;
     }

   return world->light.all_bodies;
}

EAPI void
ephysics_world_stack_enable_set(EPhysics_World *world, Eina_Bool enabled)
{
   if (!world)
     {
        ERR("Can't enable / disable stacking, world wasn't provided.");
        return;
     }
   world->stacking = !!enabled;
}

EAPI Eina_Bool
ephysics_world_stack_enable_get(const EPhysics_World *world)
{
   if (!world)
     {
	ERR("No world, no stacking status for you.");
	return EINA_FALSE;
     }

   return world->stacking;
}

#ifdef  __cplusplus
}
#endif
