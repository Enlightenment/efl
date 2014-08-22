#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Evas.h>
#include <Ecore.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"

#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <LinearMath/btGeometryUtil.h>

#pragma GCC diagnostic pop

#include <math.h>

#include "ephysics_private.h"
#include "ephysics_trimesh.h"
#include "ephysics_body_materials.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define BODY_CLOTH_CHECK() \
   do { \
     if (body->type == EPHYSICS_BODY_TYPE_CLOTH) \
       { \
          WRN("Not supported for cloth"); \
          return; \
       } \
   } while(0)

typedef struct _EPhysics_Body_Callback EPhysics_Body_Callback;
typedef struct _EPhysics_Body_Evas_Stacking EPhysics_Body_Evas_Stacking;
typedef struct _EPhysics_Body_Soft_Body_Slice EPhysics_Body_Soft_Body_Slice;
typedef struct _EPhysics_Body_Face_Obj EPhysics_Body_Face_Obj;
typedef struct _EPhysics_Quaternion EPhysics_Quaternion;

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
     Evas_Coord z;
};

struct _EPhysics_Body_Evas_Stacking {
     Evas_Object *evas;
     float stacking;
};

struct _EPhysics_Body_Soft_Body_Slice
{
     Evas_Object *evas_obj;
     int index;
     struct {
          double x;
          double y;
     } p[3];
     float stacking;
};

struct _EPhysics_Body_Face_Obj {
    EPhysics_Body_Face face;
    Evas_Object *obj;
};

static void
_ephysics_body_cloth_anchor_mass_reset(EPhysics_Body *body)
{
   double anchor_mass;
   anchor_mass = 1 / (body->soft_body->m_nodes.size() * 0.025);

   for (int i = 0; i < body->soft_body->m_anchors.size(); i++)
     body->soft_body->m_anchors[i].m_node->m_im = anchor_mass;

   DBG("Cloth anchors mass reset.");
}

EAPI void
_ephysics_body_soft_body_light_apply(Evas_Map *m, Evas_Coord lx, Evas_Coord ly, Evas_Coord lz, int lr, int lg, int lb, int ar, int ag, int ab, Evas_Coord bx, Evas_Coord by, Evas_Coord bz)
{
   double x, y, z, nx, ny, nz, ln, br;
   Evas_Coord mx, my, mz, mr, mg, mb, ma;
   int i;

   for (i = 0; i < 4; i++)
     {

        evas_map_point_coord_get(m, i, &mx, &my, &mz);
        evas_map_point_color_get(m, i, NULL, NULL, NULL, &ma);

        x = mx;
        y = my;
        z = mz;

        nx = sqrt(pow(bx - x, 2));
        ny = sqrt(pow(by - y, 2));
        nz = sqrt(pow(bz - z, 2));
        ln = nx + ny + nz;

        if (ln != 0.0)
          {
             nx /= ln;
             ny /= ln;
             nz /= ln;
          }

        x = lx - bx;
        y = ly - by;
        z = lz - bz;

        ln = pow(x, 2) + pow(y, 2) + pow(z, 2);
        ln = sqrt(ln);

        if (ln != 0.0)
          {
             x /= ln;
             y /= ln;
             z /= ln;
          }

        br = (nx * x) + (ny * y) + (nz * z);
        if (br < 0.0) br = 0.0;

        mr = ar + ((lr - ar) * br);
        mg = ag + ((lg - ag) * br);
        mb = ab + ((lb - ab) * br);

        evas_map_point_color_set(m, i, mr, mg, mb, ma);
     }
}

static void
_ephysics_body_soft_body_slices_apply(EPhysics_Body *body, Evas_Object *evas_obj, Eina_List *slices)
{
   double rate;
   void *list_data;
   Eina_List *l;
   Evas_Coord wy, wh, y0, y1, y2, x0, x1, x2, z0, z1, z2, w, h, bx, by, bz;
   Evas_Map *map;
   btVector3 p0, p1, p2;
   btSoftBody::tFaceArray faces;
   EPhysics_Body_Soft_Body_Slice *slice;
   int lr, lg, lb, ar, ag, ab;
   Evas_Coord lx, ly, lz;
   Eina_Bool light = EINA_FALSE;
   EPhysics_Camera *camera;

   int px, py, pz, foc;
   Eina_Bool perspective = EINA_FALSE;

   camera = ephysics_world_camera_get(body->world);
   rate = ephysics_world_rate_get(body->world);
   ephysics_world_render_geometry_get(body->world, NULL, &wy, NULL, NULL, &wh,
                                      NULL);
   evas_object_geometry_get(evas_obj, NULL, NULL, &w, &h);

   ephysics_body_geometry_get(body, &bx, &by, &bz, NULL, NULL, NULL);

   if ((body->light_apply) ||
       (ephysics_world_light_all_bodies_get(body->world)))
     {
        ephysics_world_point_light_position_get(body->world, &lx, &ly, &lz);
        ephysics_world_point_light_color_get(body->world, &lr, &lg, &lb);
        ephysics_world_ambient_light_color_get(body->world, &ar, &ag, &ab);
        light = EINA_TRUE;
     }

   if (ephysics_camera_perspective_enabled_get(camera))
     {
        ephysics_camera_perspective_get(camera, &px, &py, &pz, &foc);
        perspective = EINA_TRUE;
     }

   EINA_LIST_FOREACH(slices, l, list_data)
     {
        slice = (EPhysics_Body_Soft_Body_Slice *)list_data;

        faces = body->soft_body->m_faces;
        p0 = faces[slice->index].m_n[0]->m_x;
        p1 = faces[slice->index].m_n[1]->m_x;
        p2 = faces[slice->index].m_n[2]->m_x;

        slice->stacking = p0.z() + p1.z() + p2.z();

        map = evas_map_new(4);

        evas_map_point_image_uv_set(map, 0, slice->p[0].x * w,
                                    slice->p[0].y * h);
        evas_map_point_image_uv_set(map, 1, slice->p[1].x * w,
                                    slice->p[1].y * h);
        evas_map_point_image_uv_set(map, 2, slice->p[2].x * w,
                                    slice->p[2].y * h);
        evas_map_point_image_uv_set(map, 3, slice->p[2].x * w,
                                    slice->p[2].y * h);

        x0 = p0.x() * rate;
        x1 = p1.x() * rate;
        x2 = p2.x() * rate;

        y0 = wh + wy - (p0.y() * rate);
        y1 = wh + wy - (p1.y() * rate);
        y2 = wh + wy - (p2.y() * rate);

        z0 = p0.z() * rate;
        z1 = p1.z() * rate;
        z2 = p2.z() * rate;

        evas_map_point_coord_set(map, 0, x0, y0, z0);
        evas_map_point_coord_set(map, 1, x1, y1, z1);
        evas_map_point_coord_set(map, 2, x2, y2, z2);
        evas_map_point_coord_set(map, 3, x2, y2, z2);

        if (perspective)
          evas_map_util_3d_perspective(map, px, py, pz, foc);

        if (body->back_face_culling)
          {
             if (evas_map_util_clockwise_get(map))
                  evas_object_show(slice->evas_obj);
             else
               {
                  evas_map_free(map);
                  evas_object_hide(slice->evas_obj);
                  continue;
               }
          }

        if (light)
          _ephysics_body_soft_body_light_apply(map, lx, ly, lz, lr, lg, lb, ar,
                                               ag, ab, bx, by, bz);

        evas_object_map_set(slice->evas_obj, map);
        evas_object_map_enable_set(slice->evas_obj, EINA_TRUE);
        evas_map_free(map);
     }
}

static inline double
_ephysics_body_soft_body_slice_calc(double val, double delta, double max)
{
   double ret = val + delta / max;
   if (ret < 0)
     ret = 0;
   else if (ret > 1)
     ret = 1;
   return ret;
}

static EPhysics_Body_Soft_Body_Slice *
_ephysics_body_soft_body_slice_new(EPhysics_Body *body, double delta, double max, int index)
{
   EPhysics_Body_Soft_Body_Slice *slice;
   btSoftBody::tFaceArray faces;

   slice = (EPhysics_Body_Soft_Body_Slice *)calloc(
      1, sizeof(EPhysics_Body_Soft_Body_Slice));
   if (!slice)
     {
        ERR("Couldn't allocate EPhysics_Soft_Body_Slice memory.");
        return NULL;
     }

   faces = body->soft_body->m_faces;

   slice->index = index;
   slice->p[0].x = _ephysics_body_soft_body_slice_calc(
      faces[slice->index].m_n[0]->m_x.x(), delta, max);
   slice->p[0].y = 1 - _ephysics_body_soft_body_slice_calc(
      faces[slice->index].m_n[0]->m_x.y(), delta, max);
   slice->p[1].x = _ephysics_body_soft_body_slice_calc(
      faces[slice->index].m_n[1]->m_x.x(), delta, max);
   slice->p[1].y = 1 - _ephysics_body_soft_body_slice_calc(
      faces[slice->index].m_n[1]->m_x.y(), delta, max);
   slice->p[2].x = _ephysics_body_soft_body_slice_calc(
      faces[slice->index].m_n[2]->m_x.x(), delta, max);
   slice->p[2].y = 1 - _ephysics_body_soft_body_slice_calc(
      faces[slice->index].m_n[2]->m_x.y(), delta, max);

   return slice;
}

static Eina_List *
_ephysics_body_soft_body_slices_get(EPhysics_Body *body)
{
   Eina_List *l, *slices_list, *slices = NULL;
   void *ldata, *slice_data;
   EPhysics_Body_Face_Slice *face_slice;

   EINA_LIST_FOREACH(body->faces_slices, l, ldata)
     {
        face_slice = (EPhysics_Body_Face_Slice *)ldata;
        EINA_LIST_FOREACH(face_slice->slices, slices_list, slice_data)
             slices = eina_list_append(slices, slice_data);
     }

   return slices;
}

EAPI int
ephysics_body_soft_body_slice_index_get(EPhysics_Body *body, Evas_Object *slice)
{
   Eina_List *slices;
   void *ldata;
   EPhysics_Body_Soft_Body_Slice *slice_data;

   if (!body)
     {
        ERR("Can't get soft body slice index, body is null.");
        return -1;
     }

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Can't get soft body slice index, operation not allowed for rigid"
            " bodies.");
        return -1;
     }

   slices = _ephysics_body_soft_body_slices_get(body);
   EINA_LIST_FREE(slices, ldata)
     {
        slice_data = (EPhysics_Body_Soft_Body_Slice *)ldata;
        if (slice_data->evas_obj == slice)
          return slice_data->index;
     }

   return -1;
}

static void
_ephysics_body_soft_body_slice_del_cb(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   EPhysics_Body_Soft_Body_Slice *slice = (EPhysics_Body_Soft_Body_Slice *)data;
   slice->evas_obj = NULL;
   evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
                                  _ephysics_body_soft_body_slice_del_cb);
}

static void
_ephysics_body_soft_body_slices_init(EPhysics_Body *body, Evas_Object *obj, Eina_List *slices)
{
   EPhysics_Body_Soft_Body_Slice *slice = NULL;
   btVector3 p0, p1, p2;
   void *slice_data;
   Evas_Coord w, h;
   Eina_List *l;
   Evas *evas;
   Evas_Object *parent;

   evas = evas_object_evas_get(obj);
   evas_object_geometry_get(obj, NULL, NULL, &w, &h);

   parent = evas_object_smart_parent_get(obj);
   EINA_LIST_FOREACH(slices, l, slice_data)
     {
        slice = (EPhysics_Body_Soft_Body_Slice *) slice_data;
        slice->evas_obj = evas_object_image_filled_add(evas);
        evas_object_layer_set(slice->evas_obj,
                              evas_object_layer_get(obj));
        evas_object_image_source_set(slice->evas_obj, obj);
        evas_object_image_source_events_set(slice->evas_obj, EINA_TRUE);
        evas_object_resize(slice->evas_obj, w, h);
        evas_object_show(slice->evas_obj);
        evas_object_image_smooth_scale_set(slice->evas_obj, EINA_TRUE);
        evas_object_event_callback_add(slice->evas_obj, EVAS_CALLBACK_DEL,
                                       _ephysics_body_soft_body_slice_del_cb,
                                       slice);

        if (parent) evas_object_smart_member_add(slice->evas_obj, parent);
     }

   if (slice)
     evas_object_image_source_visible_set(slice->evas_obj, EINA_FALSE);

   _ephysics_body_soft_body_slices_apply(body, obj, slices);
}

static void
_ephysics_body_soft_body_slices_free(Eina_List *slices)
{
   EPhysics_Body_Soft_Body_Slice *slice;
   void *slice_data;

   EINA_LIST_FREE(slices, slice_data)
     {
        slice = (EPhysics_Body_Soft_Body_Slice *)slice_data;
        if (slice->evas_obj)
          evas_object_del(slice->evas_obj);
        free(slice);
     }
}

static void
_ephysics_body_soft_body_slices_clean(Eina_List *slices)
{
   EPhysics_Body_Soft_Body_Slice *slice;
   void *slice_data;
   Eina_List *l;

   EINA_LIST_FOREACH(slices, l, slice_data)
     {
        slice = (EPhysics_Body_Soft_Body_Slice *)slice_data;
        if (slice->evas_obj)
          evas_object_del(slice->evas_obj);
     }
}

static btTransform
_ephysics_body_transform_get(const EPhysics_Body *body)
{
   btTransform trans;
   btVector3 center;
   btScalar radius;

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        body->rigid_body->getMotionState()->getWorldTransform(trans);
        return trans;
     }

   body->soft_body->getCollisionShape()->getBoundingSphere(center, radius);
   trans.setIdentity();
   trans.setOrigin(center);
   return trans;
}

static int
_ephysics_body_evas_stacking_sort_cb(const void *d1, const void *d2)
{
   const EPhysics_Body_Evas_Stacking *stacking1, *stacking2;

   stacking1 = (const EPhysics_Body_Evas_Stacking *)d1;
   stacking2 = (const EPhysics_Body_Evas_Stacking *)d2;

   if (!stacking1) return 1;
   if (!stacking2) return -1;

   if (stacking1->stacking < stacking2->stacking) return -1;
   if (stacking2->stacking > stacking2->stacking) return 1;

   return 0;
}

static EPhysics_Body_Evas_Stacking *
_ephysics_body_evas_stacking_new(Evas_Object *obj, float index)
{
   EPhysics_Body_Evas_Stacking *stacking;

   stacking = (EPhysics_Body_Evas_Stacking *)calloc(
                1, sizeof(EPhysics_Body_Evas_Stacking));

   if (!stacking)
     {
        ERR("Could not allocate ephysics soft body evas stacking data.");
        return NULL;
     }

   stacking->evas = obj;
   stacking->stacking = index;
   return stacking;
}

void
ephysics_body_evas_objects_restack(EPhysics_World *world)
{
   void *data, *slice_data, *stack_data;
   EPhysics_Body *body;
   btTransform trans;
   EPhysics_Body_Evas_Stacking *stacking;
   EPhysics_Body_Soft_Body_Slice *slice;
   Eina_List *l, *slices, *bodies, *stack_list = NULL;
   Evas_Object *prev_obj = NULL;
   Eina_Hash *hash;
   Eina_Iterator *it;
   int layer;
   Eina_List *ll = NULL;

   bodies = ephysics_world_bodies_get(world);

   if (!eina_list_count(bodies))
     return;

   hash = eina_hash_int32_new(NULL);

   EINA_LIST_FREE(bodies, data)
     {
        body = (EPhysics_Body *)data;
        if (body->deleted) continue;

        if (body->type == EPHYSICS_BODY_TYPE_RIGID)
          {
             if (!body->evas_obj) continue;
             trans = _ephysics_body_transform_get(body);
             stacking = _ephysics_body_evas_stacking_new(body->evas_obj,
                                                         trans.getOrigin().z());
             if (!stacking) goto error;

             layer = evas_object_layer_get(stacking->evas);
             stack_list = (Eina_List *)eina_hash_find(hash, &layer);
             stack_list = eina_list_append(stack_list, stacking);
             eina_hash_set(hash, &layer, stack_list);

             continue;
          }

        slices = _ephysics_body_soft_body_slices_get(body);
        if (!slices) continue;

        EINA_LIST_FREE(slices, slice_data)
          {
             slice = (EPhysics_Body_Soft_Body_Slice *)slice_data;

             stacking = _ephysics_body_evas_stacking_new(slice->evas_obj,
                                                         slice->stacking);
             if (!stacking) goto error;

             layer = evas_object_layer_get(stacking->evas);
             stack_list = (Eina_List *)eina_hash_find(hash, &layer);
             stack_list = eina_list_append(stack_list, stacking);
             eina_hash_set(hash, &layer, stack_list);
          }
     }


   it = eina_hash_iterator_data_new(hash);
   while (eina_iterator_next(it, &data))
     {
        stack_list = (Eina_List *)data;
        stack_list = eina_list_sort(stack_list, eina_list_count(stack_list),
                                    _ephysics_body_evas_stacking_sort_cb);
        prev_obj = NULL;

        ll = eina_list_append(ll, stack_list);

        EINA_LIST_FOREACH(stack_list, l, stack_data)
          {
             stacking = (EPhysics_Body_Evas_Stacking *)stack_data;

             if (prev_obj)
               evas_object_stack_below(stacking->evas, prev_obj);

             prev_obj = stacking->evas;
          }
     }

   eina_iterator_free(it);

   EINA_LIST_FREE(ll, stack_data)
     {
        stack_list = (Eina_List *) stack_data;
        eina_hash_del_by_data(hash, stack_list);
        EINA_LIST_FREE(stack_list, slice_data)
          free(slice_data);
     }

   eina_hash_free(hash);
   return;

 error:
   ERR("Could not allocate evas stacking data memory.");
   eina_hash_free(hash);
}

static void
_ephysics_body_transform_set(EPhysics_Body *body, btTransform trans)
{
   btTransform origin;

   if (body->type != EPHYSICS_BODY_TYPE_RIGID)
     {
        origin = _ephysics_body_transform_get(body);
        body->soft_body->translate(trans.getOrigin() - origin.getOrigin());
        return;
     }
   body->rigid_body->getMotionState()->setWorldTransform(trans);
}

void
ephysics_body_activate(const EPhysics_Body *body, Eina_Bool activate)
{
   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        body->rigid_body->activate(activate);
        return;
     }

   body->soft_body->activate(activate);
}

static void
_ephysics_body_forces_update(EPhysics_Body *body)
{
   body->force.x = body->rigid_body->getTotalForce().getX();
   body->force.y = body->rigid_body->getTotalForce().getY();
   body->force.z = body->rigid_body->getTotalForce().getZ();
   body->force.torque_x = body->rigid_body->getTotalTorque().getX();
   body->force.torque_y = body->rigid_body->getTotalTorque().getY();
   body->force.torque_z = body->rigid_body->getTotalTorque().getZ();
   body->rigid_body->clearForces();

   DBG("forces updated: %lf, %lf, %lf", body->force.x, body->force.y,
       body->force.z);
   DBG("torque updated: %lf, %lf, %lf", body->force.torque_x,
       body->force.torque_y, body->force.torque_z);
}

static inline void
_ephysics_body_sleeping_threshold_set(EPhysics_Body *body, double linear_threshold, double angular_threshold, double rate)
{
   body->rigid_body->setSleepingThresholds(linear_threshold / rate,
                                           angular_threshold / RAD_TO_DEG);
}

static inline void
_ephysics_body_linear_velocity_set(EPhysics_Body *body, double x, double y, double z, double rate)
{
   btVector3 linear_velocity = btVector3(x / rate, -y / rate, z / rate);

   ephysics_body_activate(body, EINA_TRUE);
   if (body->rigid_body)
     body->rigid_body->setLinearVelocity(linear_velocity);

   if (body->soft_body)
     {
        for (int i = 0; i < body->soft_body->m_nodes.size(); i++)
          body->soft_body->m_nodes[i].m_v = linear_velocity;
     }
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
   if (body->active == !!active) return;
   body->active = !!active;
   if (active) return;

   _ephysics_body_event_callback_call(body, EPHYSICS_CALLBACK_BODY_STOPPED,
                                      (void *) body->evas_obj);
};

Eina_Bool
ephysics_body_filter_collision(EPhysics_Body *body0, EPhysics_Body *body1)
{
   Eina_List *l;
   void *grp;

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

   ephysics_world_lock_take(body->world);
   group_str = eina_stringshare_add(group);
   if (eina_list_data_find(body->collision_groups, group_str))
     {
        INF("Body already added to group: %s", group);
        eina_stringshare_del(group_str);
        ephysics_world_lock_release(body->world);
        return EINA_TRUE;
     }

   body->collision_groups = eina_list_append(body->collision_groups, group_str);
   ephysics_world_lock_release(body->world);
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

   ephysics_world_lock_take(body->world);
   group_str = eina_stringshare_add(group);
   if (!eina_list_data_find(body->collision_groups, group_str))
     {
        INF("Body isn't part of group: %s", group);
        eina_stringshare_del(group_str);
        ephysics_world_lock_release(body->world);
        return EINA_TRUE;
     }

   body->collision_groups = eina_list_remove(body->collision_groups, group_str);
   eina_stringshare_del(group_str);
   eina_stringshare_del(group_str);
   ephysics_world_lock_release(body->world);
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
_ephysics_body_new(EPhysics_World *world, btScalar mass, double cm_x, double cm_y, double cm_z)
{
   EPhysics_Body *body;
   double rate;

   body = (EPhysics_Body *) calloc(1, sizeof(EPhysics_Body));
   if (!body)
     {
        ERR("Couldn't create a new body instance.");
        return NULL;
     }

   rate = ephysics_world_rate_get(world);
   body->scale[0] = 1;
   body->scale[1] = 1;
   body->scale[2] = 1;
   body->size.w = rate;
   body->size.h = rate;
   body->size.d = rate;
   body->mass = mass;
   body->world = world;
   body->cm.x = cm_x;
   body->cm.y = cm_y;
   body->cm.z = cm_z;

   return body;
}

static EPhysics_Body *
_ephysics_body_rigid_body_add(EPhysics_World *world, btCollisionShape *collision_shape, const char *type, double cm_x, double cm_y, double cm_z)
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

   body = _ephysics_body_new(world, mass, cm_x, cm_y, cm_z);
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

   body->type = EPHYSICS_BODY_TYPE_RIGID;
   body->collision_shape = collision_shape;
   body->rigid_body = rigid_body;
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
_ephysics_body_evas_obj_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   EPhysics_Body *body = (EPhysics_Body *) data;

   if (body->default_face)
     _ephysics_body_soft_body_slices_clean(body->default_face->slices);

   body->evas_obj = NULL;
   DBG("Evas object deleted. Updating body: %p", body);
}

static void
_ephysics_body_soft_body_anchors_rebuild(int node, btRigidBody *rigid_body, btSoftBody *soft_body)
{
   btTransform world_trans = rigid_body->getWorldTransform();
   btVector3 local = world_trans.inverse() * soft_body->m_nodes[node].m_x;

   for (int i = 0; i < soft_body->m_anchors.size(); i++)
     {
        if (soft_body->m_anchors[i].m_node == &soft_body->m_nodes[node])
          soft_body->m_anchors[i].m_local = local;
     }
}

void
ephysics_body_soft_body_bending_constraints_generate(EPhysics_Body *body)
{
   btSoftBody *soft_body = body->soft_body;

   for (; body->bending_constraints; body->bending_constraints--)
     soft_body->generateBendingConstraints(2, soft_body->m_materials
                                           [body->material_index]);
}

static void
_ephysics_body_cloth_constraints_rebuild(EPhysics_Body *body)
{
   btRigidBody *rigid_body;
   btSoftBody *soft_body;
   btSoftBody::Node *node;
   btSoftBody::Anchor anchor;
   int anchors_size = body->soft_body->m_anchors.size();

   soft_body = body->soft_body;

   if (anchors_size)
     {
        rigid_body = soft_body->m_anchors[0].m_body;
        for (int m = 0; m < anchors_size; m++)
          {
             anchor = soft_body->m_anchors[m];
             node = anchor.m_node;
             for (int n = 0; n < soft_body->m_nodes.size(); n++)
               {
                  if (node == &soft_body->m_nodes[n])
                    _ephysics_body_soft_body_anchors_rebuild(n, rigid_body,
                                                             soft_body);
               }
          }
     }
   soft_body->generateClusters(0);
   ephysics_body_soft_body_bending_constraints_generate(body);
}

static void
_ephysics_body_soft_body_constraints_rebuild(EPhysics_Body *body)
{
   btSoftBody *soft_body = body->soft_body;
   btRigidBody *rigid_body = body->rigid_body;

   if (soft_body->m_anchors.size() > 0)
     {
        for (int i = 0; i < soft_body->m_nodes.size(); i++)
          _ephysics_body_soft_body_anchors_rebuild(i, rigid_body, soft_body);
     }
   else
     {
        for (int i = 0; i < soft_body->m_nodes.size(); i++)
          soft_body->appendAnchor(i, rigid_body);
     }

   soft_body->generateClusters(0);
   ephysics_body_soft_body_bending_constraints_generate(body);
}

inline static double
_ephysics_body_volume_get(const EPhysics_Body *body)
{
   btVector3 vector = body->collision_shape->getLocalScaling();
   return vector.x() * vector.y() * vector.z();
}

void
_ephysics_body_soft_body_dragging_set(EPhysics_Body *body, int triangle)
{
   btSoftBody::Face face;
   btSoftBody::Node *node;

   body->dragging_data.triangle = triangle;
   body->dragging_data.dragging = EINA_TRUE;

   face = body->soft_body->m_faces[triangle];
   node = face.m_n[0];
   body->dragging_data.mass = node->m_im;
}

static void
_ephysics_body_soft_body_mass_set(EPhysics_Body *body, double mass)
{
   int valid_nodes;
   btSoftBody::Node node;
   double inverse_mass;

   if (body->type == EPHYSICS_BODY_TYPE_SOFT)
     body->soft_body->setTotalMass(mass);
   else if (mass > 0.0)
     {
        valid_nodes = 0;
        for (int i = 0; i < body->soft_body->m_nodes.size(); i++)
          {
             node = body->soft_body->m_nodes[i];
             if (node.m_im && !node.m_battach)
               valid_nodes++;
          }

        if (valid_nodes > 0)
          {
             inverse_mass = 1 / (mass / valid_nodes);
             if (body->dragging_data.dragging)
               {
                  valid_nodes++;
                  inverse_mass = 1 / (mass / valid_nodes);
                  body->dragging_data.mass = inverse_mass;
               }
          }

        for (int i = 0; i < body->soft_body->m_nodes.size(); i++)
          {
             node = body->soft_body->m_nodes[i];
             if (node.m_im && !node.m_battach)
                  node.m_im = inverse_mass;
          }
     }
}

static void
_ephysics_body_mass_set(EPhysics_Body *body, double mass)
{
   btVector3 inertia(0, 0, 0);

   if (body->density)
     mass = body->density * _ephysics_body_volume_get(body);

   if (body->soft_body)
     _ephysics_body_soft_body_mass_set(body, mass);
   else
     {
        body->collision_shape->calculateLocalInertia(mass, inertia);
        body->rigid_body->setMassProps(mass, inertia);
        body->rigid_body->updateInertiaTensor();
     }

   body->mass = mass;
   DBG("Body %p mass changed to %lf.", body, mass);
}

static void
_ephysics_body_geometry_set(EPhysics_Body *body, Evas_Coord x, Evas_Coord y, Evas_Coord z, Evas_Coord w, Evas_Coord h, Evas_Coord d, double rate)
{
   double mx, my, mz, sx, sy, sz;
   btTransform trans;
   int wy, height;
   btVector3 body_scale, old_scale;

   ephysics_world_render_geometry_get(body->world, NULL, &wy, NULL,
                                      NULL, &height, NULL);
   height += wy;

   mx = (x + w * body->cm.x) / rate;
   my = (height - (y + h * body->cm.y)) / rate;
   mz = (z + d * body->cm.z) / rate;
   sx = (w <= 0) ? 1 : w / rate;
   sy = (h <= 0) ? 1 : h / rate;
   sz = (d <= 0) ? 1 : d / rate;

   trans = _ephysics_body_transform_get(body);
   trans.setOrigin(btVector3(mx, my, mz));
   body_scale = btVector3(sx, sy, sz);
   old_scale = btVector3(body->scale[0], body->scale[1], body->scale[2]);

   if (body->type == EPHYSICS_BODY_TYPE_SOFT)
     {
        body->soft_body->scale(btVector3(1, 1, 1) / old_scale);
        body->soft_body->scale(body_scale);
        body->rigid_body->proceedToTransform(trans);
        _ephysics_body_transform_set(body, trans);
        _ephysics_body_soft_body_constraints_rebuild(body);
     }
   else if (body->type == EPHYSICS_BODY_TYPE_CLOTH)
     {
        body->soft_body->setTotalMass(body->mass, true);
        body->soft_body->scale(btVector3(1, 1, 1) / old_scale);
        body->soft_body->scale(body_scale);
        _ephysics_body_transform_set(body, trans);
        _ephysics_body_cloth_constraints_rebuild(body);
        body->soft_body->setTotalMass(body->mass, false);
        _ephysics_body_cloth_anchor_mass_reset(body);
     }
   else
     {
        body->collision_shape->setLocalScaling(body_scale);
        body->rigid_body->proceedToTransform(trans);

        if (!body->rigid_body->isStaticObject())
          _ephysics_body_mass_set(body, ephysics_body_mass_get(body));
     }

   _ephysics_body_transform_set(body, trans);
   ephysics_body_activate(body, EINA_TRUE);

   body->size.w = w;
   body->size.h = h;
   body->size.d = d;
   body->scale[0] = sx;
   body->scale[1] = sy;
   body->scale[2] = sz;

   DBG("Body %p position changed to (%lf, %lf, %lf).", body, mx, my, mz);
   DBG("Body %p scale changed to (%lf, %lf, %lf).", body, sx, sy, sz);
}

static void
_ephysics_body_resize(EPhysics_Body *body, Evas_Coord w, Evas_Coord h, Evas_Coord d)
{
   Evas_Coord bx, by, bz;
   double rate, sx, sy, sz;
   btVector3 body_scale, center;
   btTransform trans;

   rate = ephysics_world_rate_get(body->world);
   sx = w / rate;
   sy = h / rate;
   sz = d / rate;

   DBG("Body %p scale changed to (%lf, %lf, %lf).", body, sx, sy, sz);

   body_scale = btVector3(sx, sy, sz);
   if (body->type == EPHYSICS_BODY_TYPE_SOFT)
     {
        btVector3 old_scale(body->scale[0], body->scale[1], body->scale[2]);
        trans = _ephysics_body_transform_get(body);

        body->soft_body->scale(btVector3(1, 1, 1) / old_scale);
        body->soft_body->scale(body_scale);

        _ephysics_body_transform_set(body, trans);
        body->rigid_body->proceedToTransform(trans);

        _ephysics_body_soft_body_constraints_rebuild(body);
     }
   else if (body->type == EPHYSICS_BODY_TYPE_CLOTH)
     {
        ephysics_body_geometry_get(body, &bx, &by, &bz, NULL, NULL, NULL);
        _ephysics_body_geometry_set(body, bx, by, bz, w, h, d, rate);
        return;
     }
   else
     {
        body->collision_shape->setLocalScaling(body_scale);

        if(!body->rigid_body->isStaticObject())
          _ephysics_body_mass_set(body, ephysics_body_mass_get(body));
     }

   body->size.w = w;
   body->size.h = h;
   body->size.d = d;
   body->scale[0] = sx;
   body->scale[1] = sy;
   body->scale[2] = sz;

   ephysics_body_activate(body, EINA_TRUE);
}

static void
_ephysics_body_move(EPhysics_Body *body, Evas_Coord x, Evas_Coord y, Evas_Coord z)
{
   double rate, mx, my, mz;
   btTransform trans;
   int wy, height;
   btVector3 body_scale;

   rate = ephysics_world_rate_get(body->world);
   ephysics_world_render_geometry_get(body->world, NULL, &wy, NULL,
                                      NULL, &height, NULL);
   height += wy;

   mx = (x + body->size.w * body->cm.x) / rate;
   my = (height - (y + body->size.h * body->cm.y)) / rate;
   mz = (z + body->size.d * body->cm.z) / rate;

   trans = _ephysics_body_transform_get(body);
   trans.setOrigin(btVector3(mx, my, mz));

   if (body->type == EPHYSICS_BODY_TYPE_CLOTH)
     _ephysics_body_transform_set(body, trans);
   else if (body->type == EPHYSICS_BODY_TYPE_SOFT)
     {
        _ephysics_body_transform_set(body, trans);
        body->rigid_body->proceedToTransform(trans);
        body->rigid_body->getMotionState()->setWorldTransform(trans);
     }
   else
     {
        body->rigid_body->proceedToTransform(trans);
        body->rigid_body->getMotionState()->setWorldTransform(trans);
     }

   ephysics_body_activate(body, EINA_TRUE);

   DBG("Body %p position changed to (%lf, %lf, %lf).", body, mx, my, mz);
}

static void
_ephysics_body_evas_obj_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   EPhysics_Body *body = (EPhysics_Body *) data;
   int w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if ((w == body->size.w) && (h == body->size.h))
     return;

   DBG("Resizing body %p to w=%i, h=%i, d=%i", body, w, h, body->size.d);

   ephysics_world_lock_take(body->world);
   _ephysics_body_resize(body, w, h, body->size.d);

   if (body->type == EPHYSICS_BODY_TYPE_CLOTH)
     {
        _ephysics_body_soft_body_slices_clean(body->default_face->slices);
        _ephysics_body_soft_body_slices_init(body, body->evas_obj,
                                             body->default_face->slices);
     }

   ephysics_world_lock_release(body->world);
}

 static void
_ephysics_body_soft_body_evas_restack_cb(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   EPhysics_Body *body = (EPhysics_Body *)data;
   Eina_List *slices;
   void *ldata;
   EPhysics_Body_Soft_Body_Slice *slice;
   short layer = evas_object_layer_get(obj);

   slices = _ephysics_body_soft_body_slices_get(body);
   EINA_LIST_FREE(slices, ldata)
     {
        slice = (EPhysics_Body_Soft_Body_Slice *)ldata;
        evas_object_layer_set(slice->evas_obj, layer);
     }
   DBG("Body's slices layer reset to: %d", layer);
}

static void
_ephysics_body_face_slice_del(EPhysics_Body_Face_Slice *face_slice)
{
   _ephysics_body_soft_body_slices_free(face_slice->slices);
   free(face_slice->points_deform);
   free(face_slice);
}

static EPhysics_Body_Face_Slice *
_ephysics_body_face_slice_add(EPhysics_Body *body, EPhysics_Body_Face face)
{
   EPhysics_Body_Face_Slice *face_slice;

   face_slice = (EPhysics_Body_Face_Slice *)calloc(1,
                                               sizeof(EPhysics_Body_Face_Slice));
   if (!face_slice)
     return NULL;

   face_slice->face = face;
   body->faces_slices = eina_list_append(body->faces_slices, face_slice);
   face_slice->body = body;
   return face_slice;
   DBG("New face slice added to body %p", body);
}

static EPhysics_Body_Face_Slice *
_ephysics_body_face_slice_get(EPhysics_Body *body, EPhysics_Body_Face face)
{
   Eina_List *l;
   void *ldata;
   EPhysics_Body_Face_Slice *face_slice = NULL;

   EINA_LIST_FOREACH(body->faces_slices, l, ldata)
     {
        if (((EPhysics_Body_Face_Slice *)ldata)->face == face)
          {
             face_slice = (EPhysics_Body_Face_Slice *)ldata;
             break;
          }
     }

   return face_slice;
}

static EPhysics_Body_Face_Obj *
_ephysics_body_face_evas_object_get(EPhysics_Body *body, EPhysics_Body_Face face)
{
   Eina_List *l;
   void *ldata;
   EPhysics_Body_Face_Obj *face_obj;

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        if (face_obj->face == face)
          return face_obj;
     }

   DBG("Could not find requested face");
   return NULL;
}

static void
_ephysics_body_del(EPhysics_Body *body)
{
   EPhysics_Body_Callback *cb;
   void *ldata;
   void *group;
   EPhysics_Body_Face_Slice *face_slice;

   if (body->evas_obj)
     {
        evas_object_event_callback_del(body->evas_obj, EVAS_CALLBACK_DEL,
                                       _ephysics_body_evas_obj_del_cb);
        evas_object_event_callback_del(body->evas_obj, EVAS_CALLBACK_RESIZE,
                                       _ephysics_body_evas_obj_resize_cb);

        if (body->faces_slices)
          evas_object_event_callback_del(body->evas_obj, EVAS_CALLBACK_RESTACK,
                                       _ephysics_body_soft_body_evas_restack_cb);
     }

   while (body->callbacks)
     {
        cb = EINA_INLIST_CONTAINER_GET(body->callbacks,
                                       EPhysics_Body_Callback);
        body->callbacks = eina_inlist_remove(body->callbacks, body->callbacks);
        free(cb);
     }

   ephysics_constraint_body_del(body);

   EINA_LIST_FREE(body->collision_groups, group)
      eina_stringshare_del((Eina_Stringshare *)group);

   EINA_LIST_FREE(body->faces_slices, ldata)
     {
        face_slice = (EPhysics_Body_Face_Slice *)ldata;
        if (_ephysics_body_face_evas_object_get(body, face_slice->face))
          ephysics_body_face_evas_object_unset(body, face_slice->face);
        _ephysics_body_face_slice_del(face_slice);
     }

   if (body->rigid_body)
     {
        delete body->rigid_body->getMotionState();
        delete body->collision_shape;
        delete body->rigid_body;
     }

   delete body->soft_body;

   free(body);
}

static void
_ephysics_body_evas_obj_map_apply(EPhysics_Body *body, Evas_Map *map, Evas_Object *obj, Eina_Bool bfc, Eina_Bool update_cw)
{
   EPhysics_Camera *camera = ephysics_world_camera_get(body->world);

   if (ephysics_camera_perspective_enabled_get(camera))
     {
        int px, py, z0, foc;
        ephysics_camera_perspective_get(camera, &px, &py, &z0, &foc);
        evas_map_util_3d_perspective(map, px, py, z0, foc);
     }

   if (bfc)
     {
        if (evas_map_util_clockwise_get(map))
          {
             if (update_cw)
               body->clockwise = EINA_TRUE;
             evas_object_show(obj);
          }
        else
          {
             if (update_cw)
               body->clockwise = EINA_FALSE;
             evas_map_free(map);
             evas_object_hide(obj);
             return;
          }
     }
   else
       evas_object_show(obj);

   if ((body->light_apply) ||
       (ephysics_world_light_all_bodies_get(body->world)))
     {
        int lr, lg, lb, ar, ag, ab;
        Evas_Coord lx, ly, lz;

        ephysics_world_point_light_position_get(body->world, &lx, &ly, &lz);
        ephysics_world_point_light_color_get(body->world, &lr, &lg, &lb);
        ephysics_world_ambient_light_color_get(body->world, &ar, &ag, &ab);
        evas_map_util_3d_lighting(map, lx, ly, lz, lr, lg, lb, ar, ag, ab);
     }

   evas_object_map_set(obj, map);
   evas_object_map_enable_set(obj, EINA_TRUE);
   evas_map_free(map);
}

static void
_ephysics_cloth_face_objs_update(EPhysics_Body *body EINA_UNUSED)
{

}

static void
_ephysics_cylinder_face_objs_update(EPhysics_Body *body)
{
   int bx, by, x, y, z, wx, wy, wh, cx, cy;
   EPhysics_Body_Face_Obj *face_obj;
   EPhysics_Camera *camera;
   btQuaternion quat;
   btTransform trans;
   Evas_Map *map;
   Eina_List *l;
   double rate;
   void *ldata;

   ephysics_world_render_geometry_get(body->world, &wx, &wy, NULL,
                                      NULL, &wh, NULL);
   camera = ephysics_world_camera_get(body->world);
   ephysics_camera_position_get(camera, &cx, &cy);
   cx -= wx;
   cy -= wy;

   rate = ephysics_world_rate_get(body->world);
   trans = _ephysics_body_transform_get(body);

   bx = (int) (trans.getOrigin().getX() * rate) - cx;
   by = wh + wy - (int) (trans.getOrigin().getY() * rate) - cy;
   x = bx - body->size.w / 2;
   y = by - body->size.h / 2;
   z = (int) (trans.getOrigin().getZ() * rate);

   quat = trans.getRotation();
   quat.normalize();

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        Evas_Object *obj;
        Evas_Coord w, h;

        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        obj = face_obj->obj;

        evas_object_geometry_get(obj, NULL, NULL, &w, &h);
        evas_object_move(obj, x, y);
        if ((!w) || (!h))
          {
             DBG("Evas object with no geometry: %p, w=%i h=%i", obj, w, h);
             continue;
          }

        map = evas_map_new(4);

        switch(face_obj->face)
          {
           case EPHYSICS_BODY_CYLINDER_FACE_MIDDLE_FRONT:
              evas_map_util_points_populate_from_object_full(map, obj, z);
              break;
           case EPHYSICS_BODY_CYLINDER_FACE_MIDDLE_BACK:
              evas_map_util_points_populate_from_object_full(map, obj, z + 1);
              evas_map_util_3d_rotate(map, 0, 180, 0, bx, by, z + 1);
              break;
           case EPHYSICS_BODY_CYLINDER_FACE_FRONT:
              evas_map_util_points_populate_from_object_full(
                 map, obj, z - body->size.d / 2);
              break;
           case EPHYSICS_BODY_CYLINDER_FACE_BACK:
              evas_map_util_points_populate_from_object_full(
                 map, obj, z + body->size.d / 2);
              evas_map_util_3d_rotate(map, 0, 180, 0, bx, by,
                                      z + body->size.d / 2);
              break;
           default:
              WRN("Face %i not updated", face_obj->face);
              evas_map_free(map);
              continue;
          }

        evas_map_util_quat_rotate(map, quat.x(), -quat.y(), quat.z(), -quat.w(),
                                  bx, by, z);

        _ephysics_body_evas_obj_map_apply(body, map, obj, EINA_TRUE,
                                          EINA_FALSE);
     }
}

static void
_ephysics_box_face_objs_update(EPhysics_Body *body)
{
   EPhysics_Body_Face_Obj *face_obj;
   int i, x, y, z, wx, wy, wh, cx, cy;
   EPhysics_Camera *camera;
   Evas_Coord v[8][3];
   btQuaternion quat;
   btTransform trans;
   btBoxShape *shape;
   Evas_Map *map;
   Eina_List *l;
   double rate;
   void *ldata;

   ephysics_world_render_geometry_get(body->world, &wx, &wy, NULL,
                                      NULL, &wh, NULL);
   camera = ephysics_world_camera_get(body->world);
   ephysics_camera_position_get(camera, &cx, &cy);
   cx -= wx;
   cy -= wy;

   rate = ephysics_world_rate_get(body->world);
   trans = _ephysics_body_transform_get(body);

   x = (int) (trans.getOrigin().getX() * rate) - cx;
   y = wh + wy - (int) (trans.getOrigin().getY() * rate) - cy;
   z = (int) (trans.getOrigin().getZ() * rate);

   shape = (btBoxShape *) body->collision_shape;
   for (i = 0; i < 8; i++)
     {
        btVector3 vertice;
        shape->getVertex(i, vertice);
        v[i][0] = vertice.getX() * rate + x;
        v[i][1] = - vertice.getY() * rate + y;
        v[i][2] = vertice.getZ() * rate + z;
     }

   quat = trans.getRotation();
   quat.normalize();

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        Evas_Object *obj;
        Evas_Coord w, h;

        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        obj = face_obj->obj;
        evas_object_geometry_get(obj, NULL, NULL, &w, &h);

        map = evas_map_new(4);
        evas_map_point_image_uv_set(map, 0, 0, 0);
        evas_map_point_image_uv_set(map, 1, w, 0);
        evas_map_point_image_uv_set(map, 2, w, h);
        evas_map_point_image_uv_set(map, 3, 0, h);

        switch(face_obj->face)
          {
           case EPHYSICS_BODY_BOX_FACE_MIDDLE_FRONT:
              evas_map_point_coord_set(map, 0, v[5][0], v[5][1], z);
              evas_map_point_coord_set(map, 1, v[4][0], v[4][1], z);
              evas_map_point_coord_set(map, 2, v[6][0], v[6][1], z);
              evas_map_point_coord_set(map, 3, v[7][0], v[7][1], z);
              break;
           case EPHYSICS_BODY_BOX_FACE_MIDDLE_BACK:
              evas_map_point_coord_set(map, 0, v[0][0], v[0][1], z);
              evas_map_point_coord_set(map, 1, v[1][0], v[1][1], z);
              evas_map_point_coord_set(map, 2, v[3][0], v[3][1], z);
              evas_map_point_coord_set(map, 3, v[2][0], v[2][1], z);
              break;
           case EPHYSICS_BODY_BOX_FACE_FRONT:
              evas_map_point_coord_set(map, 0, v[5][0], v[5][1], v[5][2]);
              evas_map_point_coord_set(map, 1, v[4][0], v[4][1], v[4][2]);
              evas_map_point_coord_set(map, 2, v[6][0], v[6][1], v[6][2]);
              evas_map_point_coord_set(map, 3, v[7][0], v[7][1], v[7][2]);
              break;
           case EPHYSICS_BODY_BOX_FACE_BACK:
              evas_map_point_coord_set(map, 0, v[0][0], v[0][1], v[0][2]);
              evas_map_point_coord_set(map, 1, v[1][0], v[1][1], v[1][2]);
              evas_map_point_coord_set(map, 2, v[3][0], v[3][1], v[3][2]);
              evas_map_point_coord_set(map, 3, v[2][0], v[2][1], v[2][2]);
              break;
           case EPHYSICS_BODY_BOX_FACE_RIGHT:
              evas_map_point_coord_set(map, 0, v[4][0], v[4][1], v[4][2]);
              evas_map_point_coord_set(map, 1, v[0][0], v[0][1], v[0][2]);
              evas_map_point_coord_set(map, 2, v[2][0], v[2][1], v[2][2]);
              evas_map_point_coord_set(map, 3, v[6][0], v[6][1], v[6][2]);
              break;
           case EPHYSICS_BODY_BOX_FACE_LEFT:
              evas_map_point_coord_set(map, 0, v[1][0], v[1][1], v[1][2]);
              evas_map_point_coord_set(map, 1, v[5][0], v[5][1], v[5][2]);
              evas_map_point_coord_set(map, 2, v[7][0], v[7][1], v[7][2]);
              evas_map_point_coord_set(map, 3, v[3][0], v[3][1], v[3][2]);
              break;
           case EPHYSICS_BODY_BOX_FACE_TOP:
              evas_map_point_coord_set(map, 0, v[1][0], v[1][1], v[1][2]);
              evas_map_point_coord_set(map, 1, v[0][0], v[0][1], v[0][2]);
              evas_map_point_coord_set(map, 2, v[4][0], v[4][1], v[4][2]);
              evas_map_point_coord_set(map, 3, v[5][0], v[5][1], v[5][2]);
              break;
           case EPHYSICS_BODY_BOX_FACE_BOTTOM:
              evas_map_point_coord_set(map, 0, v[7][0], v[7][1], v[7][2]);
              evas_map_point_coord_set(map, 1, v[6][0], v[6][1], v[6][2]);
              evas_map_point_coord_set(map, 2, v[2][0], v[2][1], v[2][2]);
              evas_map_point_coord_set(map, 3, v[3][0], v[3][1], v[3][2]);
              break;
           default:
              WRN("Face %i not updated", face_obj->face);
              evas_map_free(map);
              continue;
          }

        evas_map_util_quat_rotate(map, quat.x(), -quat.y(), quat.z(), -quat.w(),
                                  x, y, z);
        _ephysics_body_evas_obj_map_apply(body, map, obj, EINA_TRUE,
                                          EINA_FALSE);
     }
}

static void
_ephysics_body_evas_object_update(EPhysics_Body *body, Evas_Object *evas_obj)
{
   int bx, by, x, y, z, w, h, wx, wy, wh, cx, cy;
   EPhysics_Camera *camera;
   btTransform trans;
   btQuaternion quat;
   Evas_Map *map;
   double rate;

   trans = _ephysics_body_transform_get(body);
   ephysics_world_render_geometry_get(body->world, &wx, &wy, NULL,
                                      NULL, &wh, NULL);
   camera = ephysics_world_camera_get(body->world);
   ephysics_camera_position_get(camera, &cx, &cy);
   cx -= wx;
   cy -= wy;

   evas_object_geometry_get(evas_obj, NULL, NULL, &w, &h);
   rate = ephysics_world_rate_get(body->world);
   bx = (int) (trans.getOrigin().getX() * rate) - cx;
   by = wh + wy - (int) (trans.getOrigin().getY() * rate) - cy;
   x = bx - w * body->cm.x;
   y = by - h * body->cm.y;
   z = (int) (trans.getOrigin().getZ() * rate);

   evas_object_move(evas_obj, x, y);

   if ((!w) || (!h))
     {
        DBG("Evas object with no geometry: %p, w=%i h=%i", evas_obj, w, h);
        return;
     }

   if (body->type != EPHYSICS_BODY_TYPE_RIGID)
     {
        if (!body->back_face_culling)
          evas_object_show(body->evas_obj);
        _ephysics_body_soft_body_slices_apply(body, body->evas_obj,
                                              body->default_face->slices);
        return;
     }

   map = evas_map_new(4);
   evas_map_util_points_populate_from_object(map, body->evas_obj);

   quat = trans.getRotation();
   quat.normalize();
   evas_map_util_quat_rotate(map, quat.x(), -quat.y(), quat.z(), -quat.w(),
                             bx, by, z);

   _ephysics_body_evas_obj_map_apply(body, map, evas_obj,
                                     body->back_face_culling, EINA_TRUE);
}

static void
_ephysics_body_soft_body_update(EPhysics_Body *body)
{
   Eina_List *l;
   void *ldata;
   EPhysics_Body_Face_Slice *face_slice;
   EPhysics_Body_Face_Obj *face_obj;

   EINA_LIST_FOREACH(body->faces_slices, l, ldata)
     {
        face_slice = (EPhysics_Body_Face_Slice *)ldata;
        face_obj = _ephysics_body_face_evas_object_get(body,
                                                       face_slice->face);
        if (!face_obj) continue;
        _ephysics_body_soft_body_slices_apply(body, face_obj->obj,
                                              face_slice->slices);
     }
}

static void
_ephysics_body_evas_object_default_update(EPhysics_Body *body)
{
   if (body->face_objs)
     {
        if (body->type == EPHYSICS_BODY_TYPE_CLOTH)
          _ephysics_cloth_face_objs_update(body);
        else if (body->type == EPHYSICS_BODY_TYPE_RIGID)
          {
             if (body->shape == EPHYSICS_BODY_SHAPE_CYLINDER)
               _ephysics_cylinder_face_objs_update(body);
             else if (body->shape == EPHYSICS_BODY_SHAPE_BOX)
               _ephysics_box_face_objs_update(body);
          }
        else if (body->type == EPHYSICS_BODY_TYPE_SOFT)
          _ephysics_body_soft_body_update(body);

        if (body->evas_obj)
            evas_object_hide(body->evas_obj);

        return;
     }

   if (!body->evas_obj)
     return;

   _ephysics_body_evas_object_update(body, body->evas_obj);
}

static void
_ephysics_body_outside_render_area_check(EPhysics_Body *body)
{
   int wx, wy, wz, ww, wh, wd, bx, by, bz, bw, bh, bd;

   ephysics_world_render_geometry_get(body->world, &wx, &wy, &wz,
                                      &ww, &wh, &wd);
   ephysics_body_geometry_get(body, &bx, &by, &bz, &bw, &bh, &bd);

   // FIXME: check what should be done regarding rotated bodies
   if (((ephysics_world_bodies_outside_top_autodel_get(body->world)) &&
        (by + bh < wy)) ||
       ((ephysics_world_bodies_outside_bottom_autodel_get(body->world)) &&
        (by > wy + wh)) ||
       ((ephysics_world_bodies_outside_left_autodel_get(body->world)) &&
        (bx + bh < wx)) ||
       ((ephysics_world_bodies_outside_right_autodel_get(body->world)) &&
        (bx > wx + ww)) ||
       ((ephysics_world_bodies_outside_front_autodel_get(body->world)) &&
        (bz + bd < wz)) ||
       ((ephysics_world_bodies_outside_back_autodel_get(body->world)) &&
        (bz > wz + wd)))
     {
        DBG("Body %p out of render area", body);
        ephysics_body_del(body);
     }
}

void
ephysics_body_forces_apply(EPhysics_Body *body)
{
   if (!((body->force.x) || (body->force.y) || (body->force.z) ||
         (body->force.torque_x) || (body->force.torque_y) ||
         (body->force.torque_z)))
     return;

   DBG("body: %p, applying forces: %lf, %lf, %lf", body, body->force.x,
       body->force.y, body->force.z);

   ephysics_body_activate(body, EINA_TRUE);
   body->rigid_body->applyCentralForce(btVector3(body->force.x,
                                                 body->force.y,
                                                 body->force.z));
   body->rigid_body->applyTorque(btVector3(body->force.torque_x,
                                           body->force.torque_y,
                                           body->force.torque_z));
}

void
ephysics_body_recalc(EPhysics_Body *body, double rate)
{
   Evas_Coord x, y, z, w, h, d;
   double vx, vy, vz, lt, at;

   ephysics_body_geometry_get(body, &x, &y, &z, &w, &h, &d);
   ephysics_body_linear_velocity_get(body, &vx, &vy, &vz);
   ephysics_body_sleeping_threshold_get(body, &lt, &at);

   _ephysics_body_geometry_set(body, x, y, z, w, h, d, rate);
   _ephysics_body_linear_velocity_set(body, vx, vy, vz, rate);
   _ephysics_body_sleeping_threshold_set(body, lt, at, rate);
}

void
ephysics_body_evas_object_update_select(EPhysics_Body *body)
{
   Eina_Bool callback_called = EINA_FALSE;

   if (!body || body->deleted)
     return;

   callback_called = _ephysics_body_event_callback_call(
      body, EPHYSICS_CALLBACK_BODY_UPDATE, (void *) body->evas_obj);

   if (!callback_called)
     _ephysics_body_evas_object_default_update(body);

   if (ephysics_world_bodies_outside_autodel_get(body->world))
     _ephysics_body_outside_render_area_check(body);
}

EAPI void
ephysics_body_collision_position_get(const EPhysics_Body_Collision *collision, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z)
{
   if (!collision)
     {
        ERR("Can't get body's collision data, collision is null.");
        return;
     }

   if (x) *x = collision->x;
   if (y) *y = collision->y;
   if (z) *z = collision->z;
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
   EPhysics_World *world;;
   double rate;
   int wy, wh;

   if ((!body) || (!contact_body) || (body->collision_cb < 1))
     return;

   collision = (EPhysics_Body_Collision *)calloc(
      1, sizeof(EPhysics_Body_Collision));

   if (!collision)
     {
        ERR("Can't allocate collision data structure.");
        return;
     }

   world = contact_body->world;
   ephysics_world_render_geometry_get(world, NULL, &wy, NULL, NULL, &wh, NULL);
   rate = ephysics_world_rate_get(world);

   collision->contact_body = contact_body;
   collision->x = position.getX() * rate;
   collision->y = wh + wy - (position.getY() * rate);
   collision->z = position.getZ() * rate;

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

EAPI void
ephysics_body_soft_body_anchor_hardness_set(EPhysics_Body *body, double hardness)
{
   if (!body)
     {
        ERR("Can't set soft body's anchor hardness, body is null.");
        return;
     }

   if (!body->soft_body)
     {
        ERR("Can't set soft body's anchor hardness, body seems not to be a soft"
            " body.");
        return;
     }

   if (hardness < 0 || hardness > 100)
     {
        ERR("Can't set soft body's anchor hardness, it must be between 0 and"
            " 100.");
        return;
     }

   ephysics_world_lock_take(body->world);
   body->anchor_hardness = EINA_TRUE;
   body->soft_body->m_cfg.kAHR = 1 - (hardness / 100);
   ephysics_world_lock_release(body->world);
   DBG("Soft body anchor hardness set to: %lf", hardness);
}

EAPI double
ephysics_body_soft_body_anchor_hardness_get(EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get soft body's anchor hardness, body is null.");
        return -1;
     }

   if (!body->soft_body)
     {
        ERR("Can't get soft body's anchor hardness, body seems not to be a soft"
            " body.");
        return -1;
     }

   return body->soft_body->m_cfg.kAHR * 100;
}

EAPI void
ephysics_body_soft_body_drag_coefficient_set(EPhysics_Body *body, double coefficient)
{
   if (!body)
     {
        ERR("Can't set soft body's drag coefficient, body is null.");
        return;
     }

   if (!body->soft_body)
     {
        ERR("Can't set soft body's drag coefficient, body seems not to be a soft"
            " body.");
        return;
     }

   body->soft_body->m_cfg.kDG = coefficient;
   DBG("Soft body drag coefficient set to: %lf", coefficient);
}

EAPI double
ephysics_body_soft_body_drag_coefficient_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get soft body's drag coefficient, body is null.");
        return -1;
     }

   if (!body->soft_body)
     {
        ERR("Can't get soft body's drag coefficient, body seems not to be a soft"
            " body.");
        return -1;
     }

   return body->soft_body->m_cfg.kDG;
}

static void
_ephysics_body_soft_body_hardness_set(EPhysics_Body *body, double hardness)
{
   int m = body->material_index;
   btSoftBody *soft_body = body->soft_body;

   if (!body->anchor_hardness)
     {
        if (body->type == EPHYSICS_BODY_TYPE_CLOTH)
          soft_body->m_cfg.kAHR = 0.8;
        else
          soft_body->m_cfg.kAHR = (hardness / 1000) * 0.6;
     }

   soft_body->m_materials[m]->m_kVST = (hardness / 1000);
   soft_body->m_materials[m]->m_kLST = (hardness / 1000);
   soft_body->m_materials[m]->m_kAST = (hardness / 1000);

   DBG("Soft body %p hardness set to %lf.", body, hardness);
}

EAPI void
ephysics_body_soft_body_hardness_set(EPhysics_Body *body, double hardness)
{
   if (!body)
     {
        ERR("Can't set soft body's hardness, body is null.");
        return;
     }

   if (!body->soft_body)
     {
        ERR("Can't set soft body's hardness, body seems not to be a soft body.");
        return;
     }

   if (hardness < 0 || hardness > 100)
     {
        ERR("Can't set soft body's hardness, it must be between 0 and 100.");
        return;
     }

   ephysics_world_lock_take(body->world);
   _ephysics_body_soft_body_hardness_set(body, hardness);
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_soft_body_dragging_set(EPhysics_Body *body, int triangle)
{
   if (!body)
     {
        ERR("Can't set soft body dragging status, body is null.");
        return;
     }

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Can set soft body dragging status, body must not be a rigid body.");
        return;
     }

   if (triangle < 0 || triangle >= body->soft_body->m_faces.size())
     {
        ERR("Could not move soft body triangle, provided body triangle index "
            "ranges from 0 to %d", body->soft_body->m_faces.size());
        return;
     }

   ephysics_world_lock_take(body->world);
   _ephysics_body_soft_body_dragging_set(body, triangle);
   ephysics_world_lock_release(body->world);

   DBG("Body %p appended to world's dragging bodies list.", body);
}

EAPI void
ephysics_body_soft_body_dragging_unset(EPhysics_Body *body)
{
   btSoftBody::Face face;
   btSoftBody::Node *node;

   if (!body)
     {
        ERR("Can't unset soft body dragging status, body is null.");
        return;
     }

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Can unset soft body dragging status, body must not be a rigid"
            " body.");
        return;
     }

   if (!body->dragging_data.dragging)
     {
        INF("Dragging isn't set");
        return;
     }

   ephysics_world_lock_take(body->world);
   face = body->soft_body->m_faces[body->dragging_data.triangle];
   node = face.m_n[0];
   node->m_im = body->dragging_data.mass;

   body->dragging_data.mass = 0;
   body->dragging_data.dragging = EINA_FALSE;
   body->dragging_data.triangle = 0;
   ephysics_world_lock_release(body->world);
}

EAPI double
ephysics_body_soft_body_hardness_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get soft body's hardness, body is null.");
        return 0;
     }

   if (!body->soft_body)
     {
        ERR("Can't get soft body's hardness, body seems not to be a soft body.");
        return 0;
     }

   return (body->soft_body->m_materials[body->material_index]->m_kVST * 100);
}

static void
_ephysics_body_soft_body_default_config(EPhysics_Body *body, btSoftBody *soft_body)
{
   body->bending_constraints = 1;
   body->soft_body = soft_body;
   body->soft_body->getCollisionShape()->setMargin(btScalar(0.02));
   body->soft_body->setUserPointer(body);
   body->soft_body->setTotalMass(body->mass);

   body->soft_body->m_cfg.collisions += btSoftBody::fCollision::SDF_RS;
   body->soft_body->m_cfg.collisions += btSoftBody::fCollision::VF_SS;
   _ephysics_body_soft_body_hardness_set(body, 100);
}

static EPhysics_Body *
_ephysics_body_soft_body_add(EPhysics_World *world, btCollisionShape *collision_shape, btSoftBody *soft_body)
{
   EPhysics_Body *body;

   body = _ephysics_body_rigid_body_add(world, collision_shape, "soft box", 0.5,
                                        0.5, 0.5);
   if (!body) return NULL;

   body->material_index = 0;
   body->type = EPHYSICS_BODY_TYPE_SOFT;
   _ephysics_body_soft_body_default_config(body, soft_body);

   body->rigid_body->setCollisionFlags(
      btCollisionObject::CF_NO_CONTACT_RESPONSE);

   _ephysics_body_soft_body_constraints_rebuild(body);
   ephysics_world_soft_body_add(world, body);

   return body;
}

EAPI void
ephysics_body_cloth_anchor_full_add(EPhysics_Body *body1, EPhysics_Body *body2, EPhysics_Body_Cloth_Anchor_Side side)
{
   int rows;
   int columns;

   if (!body1 || !body2)
     {
        ERR("Could not add anchors, body1 or body2 is null");
        return;
     }

   if (body1->type != EPHYSICS_BODY_TYPE_CLOTH ||
       body2->type != EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Cloth anchors are allowed only between cloth and rigid body.");
        return;
     }

   if ((side < 0) || (side >= EPHYSICS_BODY_CLOTH_ANCHOR_SIDE_LAST))
     {
        ERR("Could not add anchors, side is invalid.");
        return;
     }

   rows = body1->cloth_rows + 1;
   columns = body1->cloth_columns + 1;

   if (side == EPHYSICS_BODY_CLOTH_ANCHOR_SIDE_RIGHT)
     {
        for (int i = 0; i < rows; i++)
          body1->soft_body->appendAnchor(i, body2->rigid_body);
        goto mass_reset;
     }

   if (side == EPHYSICS_BODY_CLOTH_ANCHOR_SIDE_LEFT)
     {
        for (int i = 1; i <= rows; i++)
          body1->soft_body->appendAnchor((rows * columns) - i,
                                         body2->rigid_body);
        goto mass_reset;
     }

   if (side == EPHYSICS_BODY_CLOTH_ANCHOR_SIDE_BOTTOM)
     {
        for (int i = 0; i <= rows; i++)
          body1->soft_body->appendAnchor(i * rows, body2->rigid_body);
        goto mass_reset;
     }

   if (side == EPHYSICS_BODY_CLOTH_ANCHOR_SIDE_TOP)
     {
        for (int i = 0; i < columns; i++)
          body1->soft_body->appendAnchor((rows - 1) + rows * i,
                                         body2->rigid_body);
     }

 mass_reset:
   _ephysics_body_cloth_anchor_mass_reset(body1);
}

EAPI void
ephysics_body_cloth_anchor_add(EPhysics_Body *body1, EPhysics_Body *body2, int node)
{
   if (!body1 || !body2)
     {
        ERR("Could not add anchors, body1 or body2 is null");
        return;
     }

   if (body1->type != EPHYSICS_BODY_TYPE_CLOTH ||
       body2->type != EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Cloth anchors are allowed only between cloth and rigid body.");
        return;
     }

   body1->soft_body->appendAnchor(node, body2->rigid_body);
   _ephysics_body_cloth_anchor_mass_reset(body1);
}

EAPI void
ephysics_body_cloth_anchor_del(EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Could not delete anchor, body is null.");
        return;
     }

   if (body->type != EPHYSICS_BODY_TYPE_CLOTH)
     {
        ERR("Could not delete anchors, body is not a cloth.");
        return;
     }

   body->soft_body->m_anchors.resize(0);
   body->soft_body->setTotalMass(body->mass);
}

static Eina_List *
_ephysics_body_slices_add(EPhysics_Body *body, int slices_cnt, int *points, double delta, double max)
{
   Eina_List *slices = NULL;
   EPhysics_Body_Soft_Body_Slice *slice;
   btSoftBody::tFaceArray faces;

   for (int i = 0; i < slices_cnt; i++)
     {
        faces = body->soft_body->m_faces;

        slice = _ephysics_body_soft_body_slice_new(body, delta, max,
                                                   points[i]);
        if (!slice) goto no_slices;

        slices = eina_list_append(slices, slice);
     }
   return slices;

 no_slices:
   _ephysics_body_soft_body_slices_clean(slices);
   return NULL;
}

EAPI void
ephysics_body_soft_body_position_iterations_set(EPhysics_Body *body, int iterations)
{
   if (!body)
     {
        ERR("Could not set the number of iterations for position solver, body "
            "is null.");
        return;
     }

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Could not set the number of iterations for position solver, body "
            "must be a soft body or a cloth");
        return;
     }

   body->soft_body->m_cfg.piterations = iterations;
   DBG("Soft body position solver iterations set to: %d", iterations);
}

EAPI int
ephysics_body_soft_body_position_iterations_get(EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Could not get the number of iterations for position solver, body "
            "is null.");
        return 0;
     }

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Could not get the number of iterations for position solver, body "
            "must be a soft body or a cloth");
        return 0;
     }

   return body->soft_body->m_cfg.piterations;
}

EAPI void
ephysics_body_soft_body_bending_constraints_add(EPhysics_Body *body, int number)
{
   if (!body)
     {
        ERR("Could add new bending constraint, body is null.");
        return;
     }

   if (number <= 0)
     {
        ERR("Could not add new bending constraints, number must be greater"
            " than 0");
        return;
     }

   body->bending_constraints += number;
   DBG("Added new bending constraints to body: %p", body);
}

EAPI EPhysics_Body *
ephysics_body_cloth_add(EPhysics_World *world, unsigned short rows, unsigned short columns)
{
   EPhysics_Body *body;
   EPhysics_Body_Face_Slice *face_slice;
   btSoftBodyWorldInfo *world_info;
   btSoftBody *soft_body;
   const int body_rows = (!rows) ? 15 : rows;
   const int body_columns = (!columns) ? 15 : columns;

   if (!world)
     {
        ERR("Can't add circle, world is null.");
        return NULL;
     }

   world_info = ephysics_world_info_get(world);
   soft_body = btSoftBodyHelpers::CreatePatch(*world_info,
                                              btVector3(2, 2, 0),
                                              btVector3(2, 1, 0),
                                              btVector3(1, 2, 0),
                                              btVector3(1, 1, 0),
                                              body_rows + 1, body_columns + 1,
                                              0, false);

   if (!soft_body)
     {
        ERR("Couldn't create a new soft body.");
        return NULL;
     }

   body = _ephysics_body_new(world, 1, 0.5, 0.5, 0.5);
   if (!body)
     goto no_body;

   soft_body->setPose(false, true);

   soft_body->appendMaterial();
   body->material_index = 1;
   soft_body->m_cfg.piterations = body_rows / 5;

   _ephysics_body_soft_body_default_config(body, soft_body);
   _ephysics_body_cloth_constraints_rebuild(body);

   face_slice = _ephysics_body_face_slice_add(body,
                                              EPHYSICS_BODY_CLOTH_FACE_FRONT);
   if (!face_slice)
     {
        ERR("Could not allocate face slice data structure.");
        goto no_face_slice;
     }

   face_slice->slices_cnt = soft_body->m_faces.size();
   face_slice->points_deform = (int *)malloc(face_slice->slices_cnt *
                                             sizeof(int));
   if (!face_slice->points_deform)
     {
        ERR("Couldn't create points of deformation.");
        goto no_deform;
     }

   for (int i = 0; i < face_slice->slices_cnt; i++)
     face_slice->points_deform[i] = i;

   face_slice->slices = _ephysics_body_slices_add(body, face_slice->slices_cnt,
                                                  face_slice->points_deform, -1,
                                                  1);
   if (!face_slice->slices)
     {
        ERR("Couldn't create slices.");
        goto no_slices;
     }

   body->default_face = face_slice;

   body->cloth_columns = body_columns;
   body->cloth_rows = body_rows;
   body->type = EPHYSICS_BODY_TYPE_CLOTH;

   ephysics_world_soft_body_add(world, body);

   return body;

no_slices:
no_deform:
   _ephysics_body_face_slice_del(face_slice);
 no_face_slice:
   free(body);
no_body:
   delete soft_body;
   return NULL;
}

static void
_ephysics_body_soft_body_single_face_transform(btSoftBody *soft_body, int face_idx, int node_idx, btTransform trans)
{
   btSoftBody::Node *node;
   ATTRIBUTE_ALIGNED16(btDbvtVolume) vol;
   const btScalar margin = soft_body->getCollisionShape()->getMargin();

   node = soft_body->m_faces[face_idx].m_n[node_idx];

   if (node->m_battach)
     return;

   node->m_x = trans * node->m_x;
   node->m_q = trans * node->m_q;
   node->m_n = trans.getBasis() * node->m_n;
   vol = btDbvtVolume::FromCR(node->m_x, margin);
   soft_body->m_ndbvt.update(node->m_leaf, vol);
}

void
ephysics_body_soft_body_dragging_apply(EPhysics_Body *body)
{
   btSoftBody::Face face;
   btSoftBody::Node *node;

   face = body->soft_body->m_faces[body->dragging_data.triangle];
   node = face.m_n[0];
   node->m_v *= 0;
   node->m_im *= 0;
}

EAPI void
ephysics_body_soft_body_triangle_move(EPhysics_Body *body, int idx, Evas_Coord x, Evas_Coord y, Evas_Coord z)
{
   btScalar xx, yy, zz;
   Evas_Coord wh, wy;
   double rate;
   btVector3 new_pos;
   btTransform diff;
   btSoftBody::Face face;
   btSoftBody::Node *node;

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Could not move soft body triangle, body must be soft or cloth.");
        return;
     }

   if (idx < 0 || idx >= body->soft_body->m_faces.size())
     {
        ERR("Could not move soft body triangle, provided body triangle index "
            "ranges from 0 to %d", body->soft_body->m_faces.size());
        return;
     }

   rate = ephysics_world_rate_get(body->world);
   ephysics_world_render_geometry_get(body->world, NULL, &wy, NULL, NULL, &wh,
                                      NULL);

   xx = x / rate;
   yy = ((wh + wy) -  y) / rate;
   zz = z / rate;

   new_pos = btVector3(xx, yy, zz);
   ephysics_world_lock_take(body->world);

   face = body->soft_body->m_faces[idx];

   node = face.m_n[0];
   diff.setIdentity();
   diff.setOrigin(new_pos - node->m_x);
   _ephysics_body_soft_body_single_face_transform(body->soft_body, idx, 0, diff);

   diff.setOrigin(diff.getOrigin() * 0.1);
   for (int m = 0; m < body->soft_body->m_faces.size(); m++)
     {
        if (m == idx) continue;
        _ephysics_body_soft_body_single_face_transform(body->soft_body, m, 0,
                                                       diff);
     }

   body->soft_body->updateClusters();
   body->soft_body->updateBounds();
   body->soft_body->updateNormals();
   body->soft_body->updatePose();
   ephysics_world_lock_release(body->world);
}

EAPI Eina_List *
ephysics_body_soft_body_triangles_inside_get(const EPhysics_Body *body, Evas_Coord x, Evas_Coord y, Evas_Coord z, Evas_Coord w, Evas_Coord h, Evas_Coord d)
{
   Eina_List *face_list = NULL;
   btSoftBody::Face *bt_face;
   btSoftBody::Node *node;
   int out, *idx;
   btScalar nx, ny, nz, xx, yy, zz, dd, ww, hh;
   Evas_Coord wy, wh;
   double rate;

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Can't get triangle indexes, operation not permited to rigid"
            " bodies.");
        return NULL;
     }

   ephysics_world_lock_take(body->world);
   rate = ephysics_world_rate_get(body->world);
   ephysics_world_render_geometry_get(body->world, NULL, &wy, NULL, NULL, &wh,
                                      NULL);
   xx = x / rate;
   yy = (wy + (wh -  y)) / rate;
   zz = z / rate;
   dd = d / rate;
   ww = w / rate;
   hh = h / rate;

   for (int m = 0; m < body->soft_body->m_faces.size(); m++)
     {
        out = 0;
        bt_face = &body->soft_body->m_faces[m];
        for (int n = 0; n < 3; n++)
          {
             node = bt_face->m_n[n];
             nx = node->m_x.x();
             ny = node->m_x.y();
             nz = node->m_x.z();

             if ((nz < zz || nz > zz + dd) || (nx < xx || nx > xx + ww) ||
                 (ny > yy || ny < yy - hh))
               out++;
          }

        if (!out)
          {
             idx = (int *)malloc(sizeof(int));
             *idx = m;
             face_list = eina_list_append(face_list, idx);
          }
     }

   ephysics_world_lock_release(body->world);

   return face_list;
}

static void
_ephysics_body_soft_body_triangle_impulse_apply(EPhysics_Body *body, int idx, double x, double y, double z)
{
   btSoftBody::Face face;
   btSoftBody::Node *node;
   double rate;
   btVector3 impulse;

   rate = ephysics_world_rate_get(body->world);
   impulse = btVector3(x / rate, - y / rate, z / rate);

   face = body->soft_body->m_faces[idx];
   for (int i = 0; i < 3; i++)
     {
        node = face.m_n[i];
        node->m_v = impulse * node->m_im;
     }

   DBG("Impulse applied to soft body node(%d): %lf, %lf, %lf", idx, impulse.x(),
       impulse.y(), impulse.z());
}

EAPI void
ephysics_body_soft_body_triangle_impulse_apply(EPhysics_Body * body, int idx, double x, double y, double z)
{

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Can't apply impulse, operation not permited to rigid bodies.");
        return;
     }

   if (idx < 0 || idx >= body->soft_body->m_faces.size())
     {
        ERR("Could not apply impulse, provided body triangle index ranges from"
            " 0 to %d", body->soft_body->m_faces.size());
        return;
     }

   ephysics_world_lock_take(body->world);
   _ephysics_body_soft_body_triangle_impulse_apply(body, idx, x, y, z);
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_soft_body_triangle_list_impulse_apply(EPhysics_Body *body, Eina_List *triangles, double x, double y, double z)
{
   Eina_List *l;
   void *ldata;
   int idx, faces_cnt;

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Can't apply impulse, operation not permited to rigid bodies.");
        return;
     }

   faces_cnt = body->soft_body->m_faces.size();
   ephysics_world_lock_take(body->world);
   EINA_LIST_FOREACH(triangles, l, ldata)
     {
        idx = *(int *)ldata;
        if (idx < 0 || idx >= faces_cnt)
          {
             INF("Could not apply impulse to triangle %d, provided body"
                 " triangle index ranges from 0 to %d", idx, faces_cnt);
             continue;
          }

        _ephysics_body_soft_body_triangle_impulse_apply(body, idx, x, y, z);
        DBG("Applied impulse on body %p, triangle: %d", body, idx);
     }
   ephysics_world_lock_release(body->world);
}

EAPI int
ephysics_body_soft_body_triangle_index_get(EPhysics_Body *body, Evas_Coord x, Evas_Coord y)
{
   int w, h, r, c, index = -1;

   if (!body->evas_obj)
     {
        ERR("No evas object associated to body");
        return -1;
     }

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        ERR("Can't get node index, operation not permited to rigid bodies");
        return -1;
     }

   if (body->type == EPHYSICS_BODY_TYPE_SOFT)
     {
        ERR("Can't get node index, not implemented for soft bodies yet.");
        return -1;
     }

   evas_object_geometry_get(body->evas_obj, NULL, NULL, &w, &h);
   r = y * body->cloth_rows / h;
   c = (w - x) * body->cloth_columns / w;

   index = 2 * r + c * body->cloth_rows * 2;

   return index;
}

static EPhysics_Body_Face_Slice *
_ephysics_body_soft_sphere_face_slices_add(EPhysics_Body *body, EPhysics_Body_Face face, btVector3 center)
{
   btSoftBody::Face *bt_face;
   btSoftBody::Node *node;
   int out;
   btScalar depth_limit;
   Eina_List *face_list = NULL;
   void *data;
   int *idx, i = 0;
   EPhysics_Body_Face_Slice *face_slice;

   depth_limit = center.z();

   for (int m = 0; m < body->soft_body->m_faces.size(); m++)
     {
        out = 0;
        bt_face = &body->soft_body->m_faces[m];
        for (int n = 0; n < 3; n++)
          {
             node = bt_face->m_n[n];
             if ((face == EPHYSICS_BODY_SPHERE_FACE_FRONT &&
                  node->m_x.z() > depth_limit) ||
                 (face == EPHYSICS_BODY_SPHERE_FACE_BACK &&
                  node->m_x.z() < depth_limit))
               out++;
          }

        if (out < 2)
          {
             idx = (int *)malloc(sizeof(int));
             if (!idx)
               goto no_deform;
             *idx = m;
             face_list = eina_list_append(face_list, idx);
          }
     }

   face_slice = _ephysics_body_face_slice_add(body, face);
   if (!face_slice)
     {
        ERR("Could not allocate face slice data structure.");
        goto no_deform;
     }

   face_slice->slices_cnt = eina_list_count(face_list);
   face_slice->points_deform = (int *)malloc(face_slice->slices_cnt *
                                             sizeof(int));
   if (!face_slice->points_deform)
        goto no_points_deform;

   EINA_LIST_FREE(face_list, data)
     {
        face_slice->points_deform[i] = *((int *)data);
        i++;
     }

   face_slice->slices = _ephysics_body_slices_add(body, face_slice->slices_cnt,
                                             face_slice->points_deform, -0.5, 1);
   if (!face_slice->slices)
     {
        ERR("Couldn't create slices.");
        goto no_points_deform;
     }
   return face_slice;

no_points_deform:
   _ephysics_body_face_slice_del(face_slice);
no_deform:
   EINA_LIST_FREE(face_list, data)
     free(data);
   return NULL;
}

EAPI EPhysics_Body *
ephysics_body_soft_sphere_add(EPhysics_World *world, int granularity)
{
   EPhysics_Body *body;
   EPhysics_Body_Face_Slice *front_face, *back_face;
   btCollisionShape *shape;
   btSoftBodyWorldInfo *world_info;
   btSoftBody *soft_body;
   btVector3 center, radius;
   int body_granularity = (!granularity) ? 100 : granularity;


   if (!world)
     {
        ERR("Can't add soft sphere, world is null.");
        return NULL;
     }

   ephysics_world_lock_take(world);
   shape = new btCylinderShapeZ(btVector3(0.25, 0.25, 0.25));

   if (!shape)
     {
        ERR("Couldn't create a new cylinder shape.");
        goto no_collision_shape;
     }

   world_info = ephysics_world_info_get(world);
   center = btVector3(1, 1, 1);
   radius = btVector3(0.5, 0.5, 0.5);
   soft_body = btSoftBodyHelpers::CreateEllipsoid(*world_info, center, radius,
                                                  body_granularity);

   if (!soft_body)
     {
        ERR("Couldn't create a new soft body.");
        goto no_soft_body;
     }

   body = _ephysics_body_soft_body_add(world, shape, soft_body);
   if (!body)
     goto no_body;

   body->soft_body->setPose(false, true);

   front_face = _ephysics_body_soft_sphere_face_slices_add(body,
                             EPHYSICS_BODY_SPHERE_FACE_FRONT, center);
   if (!front_face)
     {
        ERR("Could not create points of deformation mapping for front face.");
        goto no_front_face;
     }
   body->default_face = front_face;

   back_face = _ephysics_body_soft_sphere_face_slices_add(body,
                              EPHYSICS_BODY_SPHERE_FACE_BACK, center);
   if (!back_face)
     {
        ERR("Could not create points of deformation mapping for back face.");
        goto no_back_face;
     }

   body->shape = EPHYSICS_BODY_SHAPE_SPHERE;
   ephysics_world_lock_release(world);
   return body;

no_back_face:
   _ephysics_body_face_slice_del(front_face);
no_front_face:
   ephysics_world_body_del(world, body);
no_body:
   delete soft_body;
no_soft_body:
   delete shape;
no_collision_shape:
   ephysics_world_lock_release(world);
   return NULL;
}

EAPI EPhysics_Body *
ephysics_body_soft_cylinder_add(EPhysics_World *world)
{
   EPhysics_Body *body;
   EPhysics_Body_Face_Slice *face_slice;
   btCollisionShape *shape;
   btSoftBodyWorldInfo *world_info;
   btSoftBody *soft_body;
   int points[19] = {16, 58, 44, 79, 97, 35, 6, 27, 45, 1, 38, 18, 21, 10, 26,
                     7, 86, 37, 55};

   if (!world)
     {
        ERR("Can't add circle, world is null.");
        return NULL;
     }

   ephysics_world_lock_take(world);
   shape = new btCylinderShapeZ(btVector3(0.5, 0.5, 0.5));

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

   body = _ephysics_body_soft_body_add(world, shape, soft_body);
   if (!body)
     goto no_body;

   body->shape = EPHYSICS_BODY_SHAPE_CYLINDER;

   face_slice = _ephysics_body_face_slice_add(body,
                                        EPHYSICS_BODY_CYLINDER_FACE_FRONT);
   if (!face_slice)
     {
        ERR("Could not allocate face slice data structure.");
        goto no_face_slice;
     }

   face_slice->slices_cnt = 19;
   face_slice->points_deform = (int *)malloc(face_slice->slices_cnt *
                                             sizeof(int));
   if (!face_slice->points_deform)
     {
        ERR("Couldn't create points of deformation.");
        goto no_deform;
     }

   for (int i = 0; i < face_slice->slices_cnt; i++)
     face_slice->points_deform[i] = points[i];

   face_slice->slices = _ephysics_body_slices_add(body, face_slice->slices_cnt,
                                                 face_slice->points_deform, 0.55,
                                                  1.1);
   if (!face_slice->slices)
     {
        ERR("Couldn't create slices.");
        goto no_slices;
     }

   body->default_face = face_slice;

   ephysics_world_lock_release(world);
   return body;

no_slices:
no_deform:
   _ephysics_body_face_slice_del(face_slice);
no_face_slice:
   ephysics_world_body_del(world, body);
no_body:
   delete soft_body;
no_soft_body:
   delete shape;
no_collision_shape:
   ephysics_world_lock_release(world);
   return NULL;
}

EAPI EPhysics_Body *
ephysics_body_sphere_add(EPhysics_World *world)
{
   btCollisionShape *collision_shape;
   EPhysics_Body *body;

   if (!world)
     {
        ERR("Can't add sphere, world is null.");
        return NULL;
     }

   collision_shape = new btSphereShape(0.5);
   if (!collision_shape)
     {
        ERR("Couldn't create a new sphere shape.");
        return NULL;
     }

   ephysics_world_lock_take(world);
   body = _ephysics_body_rigid_body_add(world, collision_shape, "sphere", 0.5,
                                        0.5, 0.5);
   body->shape = EPHYSICS_BODY_SHAPE_SPHERE;
   ephysics_world_lock_release(world);
   return body;
}

EAPI EPhysics_Body *
ephysics_body_cylinder_add(EPhysics_World *world)
{
   btCollisionShape *collision_shape;
   EPhysics_Body *body;

   if (!world)
     {
        ERR("Can't add cylinder, world is null.");
        return NULL;
     }

   collision_shape = new btCylinderShapeZ(btVector3(0.5, 0.5, 0.5));
   if (!collision_shape)
     {
        ERR("Couldn't create a new cylinder shape.");
        return NULL;
     }

   ephysics_world_lock_take(world);
   body = _ephysics_body_rigid_body_add(world, collision_shape, "cylinder", 0.5,
                                        0.5, 0.5);
   body->shape = EPHYSICS_BODY_SHAPE_CYLINDER;
   ephysics_world_lock_release(world);
   return body;
}

EAPI EPhysics_Body *
ephysics_body_soft_box_add(EPhysics_World *world)
{
   EPhysics_Body *body;
   EPhysics_Body_Face_Slice *face_slice;
   btCollisionShape *shape;
   btSoftBodyWorldInfo *world_info;
   btSoftBody *soft_body;
   int points[16] = {14, 85, 88, 28, 41, 55, 10, 24, 93, 79, 56, 86, 91, 8,
                     27, 1};


   if (!world)
     {
        ERR("Can't add circle, world is null.");
        return NULL;
     }

   ephysics_world_lock_take(world);
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

   body = _ephysics_body_soft_body_add(world, shape, soft_body);
   if (!body)
     goto no_body;

   body->shape = EPHYSICS_BODY_SHAPE_BOX;

   face_slice = _ephysics_body_face_slice_add(body,
                                              EPHYSICS_BODY_BOX_FACE_FRONT);
   if (!face_slice)
     {
        ERR("Could not allocate face slice data structure.");
        goto no_face_slice;
     }

   face_slice->slices_cnt = 16;
   face_slice->points_deform = (int *)malloc(face_slice->slices_cnt *
                                             sizeof(int));
   if (!face_slice->points_deform)
     {
        ERR("Couldn't create points of deformation.");
        goto no_deform;
     }

   for (int i = 0; i < face_slice->slices_cnt; i++)
     face_slice->points_deform[i] = points[i];

   face_slice->slices = _ephysics_body_slices_add(body, face_slice->slices_cnt,
                                                 face_slice->points_deform, 0.55,
                                                  1.1);
   if (!face_slice->slices)
     {
        ERR("Couldn't create slices.");
        goto no_slices;
     }

   body->default_face = face_slice;

   ephysics_world_lock_release(world);
   return body;

no_slices:
no_face_slice:
   _ephysics_body_face_slice_del(face_slice);
no_deform:
   ephysics_world_body_del(world, body);
no_body:
   delete soft_body;
no_soft_body:
   delete shape;
no_collision_shape:
   ephysics_world_lock_release(world);
   return NULL;
}

EAPI EPhysics_Body *
ephysics_body_box_add(EPhysics_World *world)
{
   btCollisionShape *collision_shape;
   EPhysics_Body *body;

   if (!world)
     {
        ERR("Can't add box, world is null.");
        return NULL;
     }

   collision_shape = new btBoxShape(btVector3(0.5, 0.5, 0.5));

   ephysics_world_lock_take(world);
   body = _ephysics_body_rigid_body_add(world, collision_shape, "box", 0.5,
                                        0.5, 0.5);
   body->shape = EPHYSICS_BODY_SHAPE_BOX;
   ephysics_world_lock_release(world);
   return body;
}

EAPI EPhysics_Body *
ephysics_body_shape_add(EPhysics_World *world, EPhysics_Shape *shape)
{
   double max_x, max_y, max_z, min_x, min_y, min_z, cm_x, cm_y, cm_z,
          range_x, range_y, range_z;
   btConvexHullShape *full_shape, *simplified_shape;
   btAlignedObjectArray<btVector3> vertexes, planes;
   const Eina_Inlist *points;
   EPhysics_Point *point;
   EPhysics_Body *body;
   int array_size, i;
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
   max_z = min_z = point->z;
   cm_x = cm_y = cm_z = 0;

   /* FIXME : only vertices should be used to calculate the center of mass */
   EINA_INLIST_FOREACH(points, point)
     {
        if (point->x > max_x) max_x = point->x;
        if (point->x < min_x) min_x = point->x;
        if (point->y > max_y) max_y = point->y;
        if (point->y < min_y) min_y = point->y;
        if (point->z > max_z) max_z = point->z;
        if (point->z < min_z) min_z = point->z;

        cm_x += point->x;
        cm_y += point->y;
        cm_z += point->z;
     }

   cm_x /= eina_inlist_count(points);
   cm_y /= eina_inlist_count(points);
   cm_z /= eina_inlist_count(points);
   range_x = max_x - min_x;
   range_y = max_y - min_y;
   range_z = max_z - min_z;

   EINA_INLIST_FOREACH(points, point)
     {
        double x, y, z;

        x = (point->x - cm_x) / range_x;
        y = - (point->y - cm_y) / range_y;
        z = (point->z - cm_z) / range_z;

        point3d = btVector3(x, y, z);
        vertexes.push_back(point3d);
     }

   /* Shrink convex shape to consider margin. Otherwise it would have a gap */
   btGeometryUtil::getPlaneEquationsFromVertices(vertexes, planes);
   array_size = planes.size();
   for (i = 0; i < array_size; ++i)
     planes[i][3] += full_shape->getMargin();

   vertexes.clear();
   btGeometryUtil::getVerticesFromPlaneEquations(planes, vertexes);

   array_size = vertexes.size();
   for (i = 0; i < array_size; ++i)
     full_shape->addPoint(vertexes[i]);

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

   ephysics_world_lock_take(world);
   body = _ephysics_body_rigid_body_add(world,
                                        (btCollisionShape *)simplified_shape,
                                        "generic", (cm_x - min_x) / range_x,
                                        1 - (cm_y - min_y) / range_y,
                                        (cm_z - min_z) / range_z);
   body->shape = EPHYSICS_BODY_SHAPE_CUSTOM;
   ephysics_world_lock_release(world);
   return body;
}

void
ephysics_body_world_boundaries_resize(EPhysics_World *world)
{
   Evas_Coord x, y, z, w, h, d;
   EPhysics_Body *bound;

   ephysics_world_render_geometry_get(world, &x, &y, &z, &w, &h, &d);

   bound = ephysics_world_boundary_get(world, EPHYSICS_WORLD_BOUNDARY_BOTTOM);
   if (bound)
     ephysics_body_geometry_set(bound, x, y + h, z, w, 10, d);

   bound = ephysics_world_boundary_get(world, EPHYSICS_WORLD_BOUNDARY_RIGHT);
   if (bound)
     ephysics_body_geometry_set(bound, x + w, y, z, 10, h, d);

   bound = ephysics_world_boundary_get(world, EPHYSICS_WORLD_BOUNDARY_LEFT);
   if (bound)
     ephysics_body_geometry_set(bound,  x - 10, y, z, 10, h, d);

   bound = ephysics_world_boundary_get(world, EPHYSICS_WORLD_BOUNDARY_TOP);
   if (bound)
     ephysics_body_geometry_set(bound, x, y - 10, z, w, 10, d);

   bound = ephysics_world_boundary_get(world, EPHYSICS_WORLD_BOUNDARY_FRONT);
   if (bound)
     ephysics_body_geometry_set(bound, x, y, z - 10, w, h, 10);

   bound = ephysics_world_boundary_get(world, EPHYSICS_WORLD_BOUNDARY_BACK);
   if (bound)
     ephysics_body_geometry_set(bound, x, y, z + d, w, h, 10);
}

static EPhysics_Body *
_ephysics_body_boundary_add(EPhysics_World *world, EPhysics_World_Boundary boundary, Evas_Coord x, Evas_Coord y, Evas_Coord z, Evas_Coord w, Evas_Coord h, Evas_Coord d)
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

   body->boundary = EINA_TRUE;
   ephysics_body_mass_set(body, 0);
   ephysics_world_boundary_set(world, boundary, body);

   if ((w <= 0) || (h <= 0) || (d <= 0))
     INF("Boundary added with default geometry. Render geometry not set yet");
   else
     ephysics_body_geometry_set(body, x, y, z, w, h, d);

   return body;
}

EAPI EPhysics_Body *
ephysics_body_top_boundary_add(EPhysics_World *world)
{
   Evas_Coord x, y, z, w, d;
   EPhysics_Body *body;

   ephysics_world_render_geometry_get(world, &x, &y, &z, &w, NULL, &d);
   body =  _ephysics_body_boundary_add(world, EPHYSICS_WORLD_BOUNDARY_TOP,
                                       x, y - 10, z, w, 10, d);
   return body;
}

EAPI EPhysics_Body *
ephysics_body_bottom_boundary_add(EPhysics_World *world)
{
   Evas_Coord x, y, z, w, h, d;
   EPhysics_Body *body;

   ephysics_world_render_geometry_get(world, &x, &y, &z, &w, &h, &d);
   body = _ephysics_body_boundary_add(world, EPHYSICS_WORLD_BOUNDARY_BOTTOM,
                                      x, y + h, z, w, 10, d);
   return body;
}

EAPI EPhysics_Body *
ephysics_body_left_boundary_add(EPhysics_World *world)
{
   Evas_Coord x, y, z, h, d;
   EPhysics_Body *body;

   ephysics_world_render_geometry_get(world, &x, &y, &z, NULL, &h, &d);
   body = _ephysics_body_boundary_add(world, EPHYSICS_WORLD_BOUNDARY_LEFT,
                                      x - 10, y, z, 10, h, d);
   return body;
}

EAPI EPhysics_Body *
ephysics_body_right_boundary_add(EPhysics_World *world)
{
   Evas_Coord x, y, z, w, h, d;
   EPhysics_Body *body;

   ephysics_world_render_geometry_get(world, &x, &y, &z, &w, &h, &d);
   body = _ephysics_body_boundary_add(world, EPHYSICS_WORLD_BOUNDARY_RIGHT,
                                      x + w, y, z, 10, h, d);
   return body;
}

EAPI EPhysics_Body *
ephysics_body_front_boundary_add(EPhysics_World *world)
{
   Evas_Coord x, y, z, w, h;
   EPhysics_Body *body;

   ephysics_world_render_geometry_get(world, &x, &y, &z, &w, &h, NULL);
   body = _ephysics_body_boundary_add(world, EPHYSICS_WORLD_BOUNDARY_FRONT,
                                      x, y, z - 10, w, h, 10);
   return body;
}

EAPI EPhysics_Body *
ephysics_body_back_boundary_add(EPhysics_World *world)
{
   Evas_Coord x, y, z, w, h, d;
   EPhysics_Body *body;

   ephysics_world_render_geometry_get(world, &x, &y, &z, &w, &h, &d);
   body = _ephysics_body_boundary_add(world, EPHYSICS_WORLD_BOUNDARY_BACK,
                                      x, y, z + d, w, h, 10);
   return body;
}

void
ephysics_orphan_body_del(EPhysics_Body *body)
{
   _ephysics_body_event_callback_call(body, EPHYSICS_CALLBACK_BODY_DEL,
                                      (void *) body->evas_obj);
   INF("Body %p deleted.", body);
   _ephysics_body_del(body);
}

EAPI void
ephysics_body_del(EPhysics_Body *body)
{
   EPhysics_World *world;

   if (!body)
     {
        ERR("Can't delete body, it wasn't provided.");
        return;
     }

   if (body->deleted) return;

   world = body->world;
   ephysics_world_lock_take(world);
   body->deleted = EINA_TRUE;
   ephysics_world_body_del(world, body);
   ephysics_world_lock_release(world);
}

EAPI void
ephysics_body_evas_object_set(EPhysics_Body *body, Evas_Object *evas_obj, Eina_Bool use_obj_pos)
{
   int obj_x, obj_y, obj_w, obj_h, bz, bd;
   double rate;

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
        evas_object_map_enable_set(body->evas_obj, EINA_FALSE);
        evas_object_event_callback_del(body->evas_obj, EVAS_CALLBACK_DEL,
                                       _ephysics_body_evas_obj_del_cb);
        evas_object_event_callback_del(body->evas_obj, EVAS_CALLBACK_RESIZE,
                                       _ephysics_body_evas_obj_resize_cb);
        if (body->default_face)
          {
             evas_object_event_callback_del(body->evas_obj,
                                       EVAS_CALLBACK_RESTACK,
                                       _ephysics_body_soft_body_evas_restack_cb);
             _ephysics_body_soft_body_slices_clean(body->default_face->slices);
          }
     }

   body->evas_obj = evas_obj;
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_DEL,
                                  _ephysics_body_evas_obj_del_cb, body);

   if (body->soft_body)
     {
        evas_object_geometry_get(body->evas_obj, &obj_x, &obj_y, &obj_w, &obj_h);
        if (!obj_w && !obj_h) evas_object_resize(body->evas_obj, 1, 1);

        evas_object_event_callback_add(body->evas_obj, EVAS_CALLBACK_RESTACK,
                                 _ephysics_body_soft_body_evas_restack_cb, body);
        _ephysics_body_soft_body_slices_init(body, body->evas_obj,
                                             body->default_face->slices);
     }

   if (!use_obj_pos)
     return;

   rate = ephysics_world_rate_get(body->world);
   evas_object_geometry_get(body->evas_obj, &obj_x, &obj_y, &obj_w, &obj_h);
   ephysics_body_geometry_get(body, NULL, NULL, &bz, NULL, NULL, &bd);

   ephysics_world_lock_take(body->world);
   _ephysics_body_geometry_set(body, obj_x, obj_y, bz, obj_w, obj_h, bd, rate);
   ephysics_world_lock_release(body->world);
   evas_object_event_callback_add(body->evas_obj, EVAS_CALLBACK_RESIZE,
                                  _ephysics_body_evas_obj_resize_cb, body);
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
        evas_object_map_enable_set(obj, EINA_FALSE);
     }

   if (body->default_face)
     {
        evas_object_event_callback_del(body->evas_obj,
                                       EVAS_CALLBACK_RESTACK,
                                       _ephysics_body_soft_body_evas_restack_cb);
        _ephysics_body_soft_body_slices_clean(body->default_face->slices);
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
ephysics_body_geometry_set(EPhysics_Body *body, Evas_Coord x, Evas_Coord y, Evas_Coord z, Evas_Coord w, Evas_Coord h, Evas_Coord d)
{
   if (!body)
     {
        ERR("Can't set body geometry, body is null.");
        return;
     }

   INF("Body: %p geometry set to x=%i y=%i z=%i w=%i h=%i d=%i.",
       body, x, y, z, w, h, d);

   if ((w <= 0) || (h <= 0) || (d <= 0))
     {
        ERR("Width, height and depth must to be a non-null, positive value.");
        return;
     }

   ephysics_world_lock_take(body->world);
   _ephysics_body_geometry_set(body, x, y, z, w, h, d,
                               ephysics_world_rate_get(body->world));
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_resize(EPhysics_Body *body, Evas_Coord w, Evas_Coord h, Evas_Coord d)
{
   if (!body)
     {
        ERR("Can't set body size, body is null.");
        return;
     }

   if ((w <= 0) || (h <= 0) || (d <= 0))
     {
        ERR("Width, height and depth must to be a non-null, positive value.");
        return;
     }

   ephysics_world_lock_take(body->world);
   _ephysics_body_resize(body, w, h, d);
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_move(EPhysics_Body *body, Evas_Coord x, Evas_Coord y, Evas_Coord z)
{
   if (!body)
     {
        ERR("Can't set body position, body is null.");
        return;
     }

   ephysics_world_lock_take(body->world);
   _ephysics_body_move(body, x, y, z);
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_geometry_get(const EPhysics_Body *body, Evas_Coord *x, Evas_Coord *y, Evas_Coord *z, Evas_Coord *w, Evas_Coord *h, Evas_Coord *d)
{
   btTransform trans;
   btVector3 scale;
   double rate;
   int wy, height;

   if (!body)
     {
        ERR("Can't get body position, body is null.");
        return;
     }

   trans = _ephysics_body_transform_get(body);
   scale = btVector3(body->scale[0], body->scale[1], body->scale[2]);

   rate = ephysics_world_rate_get(body->world);
   ephysics_world_render_geometry_get(body->world, NULL, &wy, NULL,
                                      NULL, &height, NULL);
   height += wy;

   if (x) *x = round((trans.getOrigin().getX() - scale[0] / 2) * rate);
   if (y) *y = height - round((trans.getOrigin().getY() + scale[1] / 2)
                              * rate);
   if (z) *z = round((trans.getOrigin().getZ() - scale[2] / 2) * rate);
   if (w) *w = body->size.w;
   if (h) *h = body->size.h;
   if (d) *d = body->size.d;
}

EAPI void
ephysics_body_mass_set(EPhysics_Body *body, double mass)
{
   if (!body)
     {
        ERR("Can't set body mass, body is null.");
        return;
     }

   if (mass < 0)
     {
        ERR("Can't set body's mass, it must to be non-negative.");
        return;
     }

   ephysics_world_lock_take(body->world);
   body->material = EPHYSICS_BODY_MATERIAL_CUSTOM;
   body->density = 0;
   _ephysics_body_mass_set(body, mass);
   ephysics_world_lock_release(body->world);
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
ephysics_body_linear_velocity_set(EPhysics_Body *body, double x, double y, double z)
{
   if (!body)
     {
        ERR("Can't set body linear velocity, body is null.");
        return;
     }

   _ephysics_body_linear_velocity_set(body, x, y, z,
                                      ephysics_world_rate_get(body->world));
   DBG("Linear velocity of body %p set to (%lf, %lf, %lf).", body, x, y, z);
}

static void
_ephysics_body_soft_body_linear_velocity_get(const EPhysics_Body *body, double *x, double *y, double *z, double rate)
{
   btVector3 total_velocity = btVector3(0, 0, 0);
   int nodes_size = body->soft_body->m_nodes.size();

   for (int i = 0; i < nodes_size; i++)
     total_velocity += body->soft_body->m_nodes[i].m_v;

   total_velocity /= nodes_size;
   if (x) *x = total_velocity.getX() * rate;
   if (y) *y = -total_velocity.getY() * rate;
   if (z) *z = total_velocity.getZ() * rate;
}

EAPI void
ephysics_body_linear_velocity_get(const EPhysics_Body *body, double *x, double *y, double *z)
{
   double rate;

   if (!body)
     {
        ERR("Can't get linear velocity, body is null.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);
   if (body->rigid_body)
     {
        if (x) *x = body->rigid_body->getLinearVelocity().getX() * rate;
        if (y) *y = -body->rigid_body->getLinearVelocity().getY() * rate;
        if (z) *z = body->rigid_body->getLinearVelocity().getZ() * rate;
        return;
     }
   _ephysics_body_soft_body_linear_velocity_get(body, x, y, z, rate);
}

EAPI void
ephysics_body_angular_velocity_set(EPhysics_Body *body, double x, double y, double z)
{
   if (!body)
     {
        ERR("Can't set angular velocity, body is null.");
        return;
     }

   BODY_CLOTH_CHECK();

   ephysics_world_lock_take(body->world);
   ephysics_body_activate(body, EINA_TRUE);
   body->rigid_body->setAngularVelocity(btVector3(-x / RAD_TO_DEG,
                                                  -y / RAD_TO_DEG,
                                                  -z/RAD_TO_DEG));

   DBG("Angular velocity of body %p set to (%lf, %lf, %lf).", body, x, y, z);
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_angular_velocity_get(const EPhysics_Body *body, double *x, double *y, double *z)
{
   if (!body)
     {
        ERR("Can't get angular velocity, body is null.");
        return;
     }

   BODY_CLOTH_CHECK();

   if (x) *x = -body->rigid_body->getAngularVelocity().getX() * RAD_TO_DEG;
   if (y) *y = -body->rigid_body->getAngularVelocity().getY() * RAD_TO_DEG;
   if (z) *z = -body->rigid_body->getAngularVelocity().getZ() * RAD_TO_DEG;
}

EAPI void
ephysics_body_sleeping_threshold_set(EPhysics_Body *body, double linear_threshold, double angular_threshold)
{
   if (!body)
     {
        ERR("Can't set sleeping thresholds, body is null.");
        return;
     }

   BODY_CLOTH_CHECK();

   ephysics_world_lock_take(body->world);
   _ephysics_body_sleeping_threshold_set(body, linear_threshold,
                                         angular_threshold,
                                         ephysics_world_rate_get(body->world));
   ephysics_world_lock_release(body->world);
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

   BODY_CLOTH_CHECK();

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

   ephysics_world_lock_take(body->world);
   if (body->rigid_body)
     {
        body->rigid_body->setLinearVelocity(btVector3(0, 0, 0));
        body->rigid_body->setAngularVelocity(btVector3(0, 0, 0));
     }

   if (body->soft_body)
     {
        for (int i = 0; i < body->soft_body->m_nodes.size(); i++)
          {
             body->soft_body->m_nodes[i].m_v *= 0;
             body->soft_body->m_nodes[i].m_f *= 0;
          }
     }
   ephysics_world_lock_release(body->world);

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

   BODY_CLOTH_CHECK();

   ephysics_world_lock_take(body->world);
   body->rigid_body->setDamping(btScalar(linear_damping),
                                btScalar(angular_damping));
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_damping_get(const EPhysics_Body *body, double *linear_damping, double *angular_damping)
{
   if (!body)
     {
        ERR("Can't get damping, body is null.");
        return;
     }

   BODY_CLOTH_CHECK();

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
   if (type == EPHYSICS_CALLBACK_BODY_COLLISION)
       body->collision_cb++;
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
        if (type == EPHYSICS_CALLBACK_BODY_COLLISION)
            body->collision_cb--;
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

static void
_ephysics_body_restitution_set(EPhysics_Body *body, double restitution)
{
   DBG("Body %p restitution set to %lf", body, restitution);
   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        body->rigid_body->setRestitution(btScalar(restitution));
        return;
     }

   body->soft_body->setRestitution(btScalar(restitution));
}

EAPI void
ephysics_body_restitution_set(EPhysics_Body *body, double restitution)
{
   if (!body)
     {
        ERR("Can't set body restitution, body is null.");
        return;
     }

   ephysics_world_lock_take(body->world);
   body->material = EPHYSICS_BODY_MATERIAL_CUSTOM;
   _ephysics_body_restitution_set(body, restitution);
   ephysics_world_lock_release(body->world);
}

EAPI double
ephysics_body_restitution_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get body restitution, body is null.");
        return 0;
     }

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     return body->rigid_body->getRestitution();

   return body->soft_body->getRestitution();
}

static void
_ephysics_body_friction_set(EPhysics_Body *body, double friction)
{
   DBG("Body %p friction set to %lf", body, friction);
   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     {
        body->rigid_body->setFriction(btScalar(friction));
        return;
     }

   body->soft_body->setFriction(btScalar(friction));
}

EAPI void
ephysics_body_friction_set(EPhysics_Body *body, double friction)
{
   if (!body)
     {
        ERR("Can't set body friction, body is null.");
        return;
     }

   ephysics_world_lock_take(body->world);
   body->material = EPHYSICS_BODY_MATERIAL_CUSTOM;
   _ephysics_body_friction_set(body, friction);
   ephysics_world_lock_release(body->world);
}

EAPI double
ephysics_body_friction_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get body friction, body is null.");
        return 0;
     }

   if (body->type == EPHYSICS_BODY_TYPE_RIGID)
     return body->rigid_body->getFriction();

   return body->soft_body->getFriction();
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

static void
_ephysics_body_soft_body_central_impulse_apply(EPhysics_Body *body, btVector3 impulse)
{
   btSoftBody::Face *face;

   for (int m = 0; m < body->soft_body->m_faces.size(); m++)
     {
        face = &body->soft_body->m_faces[m];
        for (int n = 0; n < 3; n++)
             face->m_n[n]->m_v += impulse * face->m_n[n]->m_im;
     }
}

EAPI void
ephysics_body_central_impulse_apply(EPhysics_Body *body, double x, double y, double z)
{
   double rate;
   btVector3 impulse;

   if (!body)
     {
        ERR("Can't apply impulse to a null body.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);

   ephysics_world_lock_take(body->world);
   ephysics_body_activate(body, EINA_TRUE);

   impulse = btVector3(x / rate, - y / rate, z / rate);

   if (body->type == EPHYSICS_BODY_TYPE_CLOTH)
     _ephysics_body_soft_body_central_impulse_apply(body, impulse);
   else
     body->rigid_body->applyCentralImpulse(impulse);

   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_impulse_apply(EPhysics_Body *body, double x, double y, double z, Evas_Coord pos_x, Evas_Coord pos_y, Evas_Coord pos_z)
{
   double rate;

   if (!body)
     {
        ERR("Can't apply impulse to a null body.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);

   ephysics_world_lock_take(body->world);
   ephysics_body_activate(body, EINA_TRUE);
   body->rigid_body->applyImpulse(btVector3(x / rate, - y / rate, z / rate),
                                  btVector3((double) pos_x / rate,
                                            (double) pos_y / rate,
                                            (double) pos_z / rate));
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_linear_movement_enable_set(EPhysics_Body *body, Eina_Bool enable_x, Eina_Bool enable_y, Eina_Bool enable_z)
{
   if (!body)
     {
        ERR("Can't set linear factor on a null body.");
        return;
     }

   BODY_CLOTH_CHECK();

   ephysics_world_lock_take(body->world);
   body->rigid_body->setLinearFactor(btVector3(!!enable_x, !!enable_y,
                                               !!enable_z));
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_linear_movement_enable_get(const EPhysics_Body *body, Eina_Bool *enable_x, Eina_Bool *enable_y, Eina_Bool *enable_z)
{
   if (!body)
     {
        ERR("Can't check if linear factor is enabled, body is null.");
        return;
     }

   BODY_CLOTH_CHECK();

   if (enable_x) *enable_x = !!body->rigid_body->getLinearFactor().x();
   if (enable_y) *enable_y = !!body->rigid_body->getLinearFactor().y();
   if (enable_z) *enable_z = !!body->rigid_body->getLinearFactor().z();
}

EAPI void
ephysics_body_torque_impulse_apply(EPhysics_Body *body, double pitch, double yaw, double roll)
{
   if (!body)
     {
        ERR("Can't apply torque impulse to a null body.");
        return;
     }

   ephysics_world_lock_take(body->world);
   ephysics_body_activate(body, EINA_TRUE);
   body->rigid_body->applyTorqueImpulse(btVector3(-pitch, -yaw, -roll));
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_angular_movement_enable_set(EPhysics_Body *body, Eina_Bool enable_x, Eina_Bool enable_y, Eina_Bool enable_z)
{
   if (!body)
     {
        ERR("Can't set rotation on a null body.");
        return;
     }

   BODY_CLOTH_CHECK();

   ephysics_world_lock_take(body->world);
   body->rigid_body->setAngularFactor(btVector3(!!enable_x, !!enable_y,
                                                !!enable_z));
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_angular_movement_enable_get(const EPhysics_Body *body, Eina_Bool *enable_x, Eina_Bool *enable_y, Eina_Bool *enable_z)
{
   if (!body)
     {
        ERR("Can't check if rotation is enabled, body is null.");
        return;
     }

   BODY_CLOTH_CHECK();

   if (enable_x) *enable_x = !!body->rigid_body->getAngularFactor().x();
   if (enable_y) *enable_y = !!body->rigid_body->getAngularFactor().y();
   if (enable_z) *enable_z = !!body->rigid_body->getAngularFactor().z();
}

EAPI EPhysics_Quaternion *
ephysics_body_rotation_get(const EPhysics_Body *body, EPhysics_Quaternion *rotation)
{
   EPhysics_Quaternion *quat;
   btTransform trans;

   if (!body)
     {
        ERR("Can't get rotation, body is null.");
        return NULL;
     }

   if (!rotation)
     {
        quat = ephysics_quaternion_new();
        if (!quat) return NULL;
     }
   else
     quat = rotation;

   trans = _ephysics_body_transform_get(body);
   quat->x = trans.getRotation().x();
   quat->y = trans.getRotation().y();
   quat->z = trans.getRotation().z();
   quat->w = trans.getRotation().getW();

   return quat;
}

static void
_ephysics_body_soft_body_rotation_set(EPhysics_Body *body, btTransform trans)
{
   btSoftBody::Node *node;
   const btScalar margin= body->soft_body->getCollisionShape()->getMargin();
   ATTRIBUTE_ALIGNED16(btDbvtVolume) vol;

   for (int i = 0; i < body->soft_body->m_nodes.size(); i++)
     {
        node = &body->soft_body->m_nodes[i];
        node->m_n = (trans.getBasis() * btVector3(1, 1, 1));
        vol = btDbvtVolume::FromCR(node->m_x, margin);

        body->soft_body->m_ndbvt.update(node->m_leaf, vol);
     }

   body->soft_body->updateNormals();
   body->soft_body->updateBounds();
   body->soft_body->updateConstants();
}

EAPI void
ephysics_body_rotation_set(EPhysics_Body *body, EPhysics_Quaternion *quat)
{
   btQuaternion bt_quat;
   btTransform trans;
   double x, y, z, w;

   if (!body)
     {
        ERR("Can't set rotation, body is null.");
        return;
     }

   if (!quat)
     {
        ERR("Can't set rotation, quaternion is null.");
        return;
     }

   ephysics_quaternion_get(quat, &x, &y, &z, &w);

   ephysics_world_lock_take(body->world);
   ephysics_body_activate(body, EINA_TRUE);

   bt_quat = btQuaternion(x, y, z, w);
   trans = _ephysics_body_transform_get(body);
   trans.setRotation(bt_quat);

   if (body->soft_body)
     _ephysics_body_soft_body_rotation_set(body, trans);
   else
     {
        body->rigid_body->proceedToTransform(trans);
        body->rigid_body->getMotionState()->setWorldTransform(trans);
     }

   ephysics_world_lock_release(body->world);
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
ephysics_body_central_force_apply(EPhysics_Body *body, double x, double y, double z)
{
   double rate;

   if (!body)
     {
        ERR("Can't apply force to a null body.");
        return;
     }

   ephysics_world_lock_take(body->world);
   rate = ephysics_world_rate_get(body->world);
   ephysics_body_forces_apply(body);
   body->rigid_body->applyCentralForce(btVector3(x / rate, - y / rate,
                                                 z / rate));
   _ephysics_body_forces_update(body);
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_force_apply(EPhysics_Body *body, double x, double y, double z, Evas_Coord pos_x, Evas_Coord pos_y, Evas_Coord pos_z)
{
   double rate;

   if (!body)
     {
        ERR("Can't apply force to a null body.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);
   ephysics_world_lock_take(body->world);
   ephysics_body_forces_apply(body);
   body->rigid_body->applyForce(btVector3(x / rate, - y / rate, z / rate),
                                btVector3((double) pos_x / rate,
                                          (double) pos_y / rate,
                                          (double) pos_z / rate));
   _ephysics_body_forces_update(body);
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_torque_apply(EPhysics_Body *body, double torque_x, double torque_y, double torque_z)
{
   if (!body)
     {
        ERR("Can't apply force to a null body.");
        return;
     }

   ephysics_world_lock_take(body->world);
   ephysics_body_forces_apply(body);
   body->rigid_body->applyTorque(btVector3(-torque_x, -torque_y, -torque_z));
   _ephysics_body_forces_update(body);
   ephysics_world_lock_release(body->world);
}

EAPI void
ephysics_body_forces_get(const EPhysics_Body *body, double *x, double *y, double *z)
{
   double rate, gx, gy, gz;

   if (!body)
     {
        ERR("Can't get forces from a null body.");
        return;
     }

   rate = ephysics_world_rate_get(body->world);
   ephysics_world_gravity_get(body->world, &gx, &gy, &gz);

   if (x) *x = body->force.x * rate + gx;
   if (y) *y = -body->force.y * rate + gy;
   if (z) *z = body->force.z * rate + gz;
}

EAPI void
ephysics_body_torques_get(const EPhysics_Body *body, double *x, double *y, double *z)
{
   if (!body)
     {
        ERR("Can't get torques from a null body.");
        return;
     }

   if (x) *x = -body->force.torque_x;
   if (y) *y = -body->force.torque_y;
   if (z) *z = -body->force.torque_z;
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
   body->force.z = 0;
   body->force.torque_x = 0;
   body->force.torque_y = 0;
   body->force.torque_z = 0;
}

EAPI void
ephysics_body_center_mass_get(const EPhysics_Body *body, double *x, double *y, double *z)
{
   if (!body)
     {
        ERR("Can't get center of mass from a null body.");
        return;
     }

   if (x) *x = body->cm.x;
   if (y) *y = body->cm.y;
   if (z) *z = body->cm.z;
}

EAPI void
ephysics_body_density_set(EPhysics_Body *body, double density)
{
   if (!body)
     {
        ERR("Can't set the density of a null body's material.");
        return;
     }

   body->material = EPHYSICS_BODY_MATERIAL_CUSTOM;
   body->density = density;
   ephysics_world_lock_take(body->world);
   _ephysics_body_mass_set(body, 0);
   ephysics_world_lock_release(body->world);
}

EAPI double
ephysics_body_density_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get the density of a null body's material.");
        return 0;
     }

   return body->density;
}

EAPI void
ephysics_body_material_set(EPhysics_Body *body, EPhysics_Body_Material material)
{
   if (!body)
     {
        ERR("Can't set body's material.");
        return;
     }

   if (material >= EPHYSICS_BODY_MATERIAL_LAST)
     {
        ERR("Invalid material.");
        return;
     }

   if (material != ephysics_material_props[material].material)
     {
        ERR("Material properties weren't found.");
        return;
     }

   if (material == EPHYSICS_BODY_MATERIAL_CUSTOM)
     {
        body->material = material;
        return;
     }

   ephysics_world_lock_take(body->world);
   body->density = ephysics_material_props[material].density;
   _ephysics_body_mass_set(body, 0);
   _ephysics_body_friction_set(body,
                               ephysics_material_props[material].friction);
   _ephysics_body_restitution_set(
      body, ephysics_material_props[material].restitution);
   body->material = material;
   ephysics_world_lock_release(body->world);
}

EAPI EPhysics_Body_Material
ephysics_body_material_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Can't get body's material.");
        return EPHYSICS_BODY_MATERIAL_LAST;
     }

   return body->material;
}

EAPI void
ephysics_body_light_set(EPhysics_Body *body, Eina_Bool enable)
{
   if (!body)
     {
        ERR("No body, no light.");
        return;
     }

   body->light_apply = !!enable;
}

EAPI Eina_Bool
ephysics_body_light_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("No body, no light.");
        return EINA_FALSE;
     }

   return body->light_apply;
}

EAPI double
ephysics_body_volume_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("No body, no volume.");
        return -1;
     }
   return _ephysics_body_volume_get(body);
}

EAPI void
ephysics_body_back_face_culling_set(EPhysics_Body *body, Eina_Bool enable)
{
   if (!body)
     {
        ERR("Body is NULL.");
        return;
     }
   body->back_face_culling = !!enable;
}

EAPI Eina_Bool
ephysics_body_back_face_culling_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Body is NULL.");
        return EINA_FALSE;
     }
   return body->back_face_culling;
}

EAPI Eina_Bool
ephysics_body_clockwise_get(const EPhysics_Body *body)
{
   if (!body)
     {
        ERR("Body is NULL.");
        return EINA_FALSE;
     }
   return body->clockwise;
}

static void
_ephysics_body_face_obj_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   EPhysics_Body *body = (EPhysics_Body *) data;
   EPhysics_Body_Face_Obj *face_obj;
   Eina_List *l;
   void *ldata;

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        if (face_obj->obj == obj)
          {
             body->face_objs = eina_list_remove(body->face_objs, face_obj);
             free(face_obj);
             return;
          }
     }
}

static void
_ephysics_body_face_obj_unset(Evas_Object *obj, Evas_Object_Event_Cb resize_func)
{
   evas_object_map_enable_set(obj, EINA_FALSE);
   evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL,
                                  _ephysics_body_face_obj_del_cb);
   evas_object_event_callback_del(obj, EVAS_CALLBACK_RESIZE, resize_func);
}

static void
_ephysics_body_face_evas_object_del(EPhysics_Body *body, EPhysics_Body_Face_Obj *face_obj, Evas_Object_Event_Cb resize_func)
{
   _ephysics_body_face_obj_unset(face_obj->obj, resize_func);
   body->face_objs = eina_list_remove(body->face_objs, face_obj);
   free(face_obj);
}

static void
_ephysics_body_face_evas_object_add(EPhysics_Body *body, EPhysics_Body_Face face, Evas_Object *evas_obj, Evas_Object_Event_Cb resize_func)
{
   EPhysics_Body_Face_Obj *face_obj;
   Eina_List *l;
   void *ldata;

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        if (face_obj->face == face)
          {
             _ephysics_body_face_obj_unset(face_obj->obj, resize_func);
             face_obj->obj = evas_obj;
             return;
          }
     }

   face_obj = (EPhysics_Body_Face_Obj *)calloc(1,
                                               sizeof(EPhysics_Body_Face_Obj));
   if (!face_obj)
     {
        ERR("Failed to create face object");
        return;
     }

   face_obj->face = face;
   face_obj->obj = evas_obj;
   body->face_objs = eina_list_append(body->face_objs, face_obj);
}

static void
_ephysics_body_box_face_obj_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   EPhysics_Body_Face face = EPHYSICS_BODY_FACE_LAST;
   EPhysics_Body *body = (EPhysics_Body *) data;
   EPhysics_Body_Face_Obj *face_obj;
   int w, h, bw, bh, bd;
   Eina_List *l;
   void *ldata;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        if (face_obj->obj == obj)
          {
             face = face_obj->face;
             break;
          }
     }

   switch(face)
     {
      case EPHYSICS_BODY_BOX_FACE_FRONT:
      case EPHYSICS_BODY_BOX_FACE_BACK:
      case EPHYSICS_BODY_BOX_FACE_MIDDLE_FRONT:
      case EPHYSICS_BODY_BOX_FACE_MIDDLE_BACK:
         if ((w == body->size.w) && (h == body->size.h))
           return;
         bw = w;
         bh = h;
         bd = body->size.d;
         break;
      case EPHYSICS_BODY_BOX_FACE_RIGHT:
      case EPHYSICS_BODY_BOX_FACE_LEFT:
         if ((w == body->size.d) && (h == body->size.h))
           return;
         bw = body->size.w;
         bh = h;
         bd = w;
      case EPHYSICS_BODY_BOX_FACE_TOP:
      case EPHYSICS_BODY_BOX_FACE_BOTTOM:
         if ((w == body->size.w) && (h == body->size.d))
           return;
         bw = w;
         bh = body->size.h;
         bd = h;
      default:
         return;
     }

   DBG("Resizing body %p to w=%i, h=%i, d=%i", body, bw, bh, bd);
   ephysics_world_lock_take(body->world);
   _ephysics_body_resize(body, bw, bh, bd);
   ephysics_world_lock_release(body->world);
}

static void
_ephysics_body_box_face_evas_object_set(EPhysics_Body *body, EPhysics_Body_Face face, Evas_Object *evas_obj, Eina_Bool use_obj_pos)
{
   int obj_x, obj_y, obj_w, obj_h, bx, by, bz, bw, bh, bd;
   double rate;

   if ((face < EPHYSICS_BODY_BOX_FACE_MIDDLE_FRONT) ||
       (face > EPHYSICS_BODY_BOX_FACE_BOTTOM))
     {
        ERR("Can't set evas object to body, face is invalid.");
        return;
     }

   if (body->soft_body)
     {
        ERR("Not implemented for soft bodies yet.");
        return;
     }

   _ephysics_body_face_evas_object_add(body, face, evas_obj,
                                       _ephysics_body_box_face_obj_resize_cb);
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_DEL,
                                  _ephysics_body_face_obj_del_cb, body);

   if (!use_obj_pos)
     return;

   rate = ephysics_world_rate_get(body->world);
   evas_object_geometry_get(evas_obj, &obj_x, &obj_y, &obj_w, &obj_h);
   ephysics_body_geometry_get(body, &bx, &by, &bz, &bw, &bh, &bd);

   ephysics_world_lock_take(body->world);

   switch(face)
     {
      case EPHYSICS_BODY_BOX_FACE_FRONT:
      case EPHYSICS_BODY_BOX_FACE_BACK:
      case EPHYSICS_BODY_BOX_FACE_MIDDLE_FRONT:
      case EPHYSICS_BODY_BOX_FACE_MIDDLE_BACK:
         _ephysics_body_geometry_set(body, obj_x, obj_y, bz,
                                     obj_w, obj_h, bd, rate);
         break;
      case EPHYSICS_BODY_BOX_FACE_RIGHT:
      case EPHYSICS_BODY_BOX_FACE_LEFT:
         _ephysics_body_geometry_set(body, bx, by, bz,
                                     bw, obj_h, obj_w, rate);
      case EPHYSICS_BODY_BOX_FACE_TOP:
      case EPHYSICS_BODY_BOX_FACE_BOTTOM:
         _ephysics_body_geometry_set(body, bx, by, bz,
                                     obj_w, bh, obj_h, rate);
      default:
         break;
     }

   ephysics_world_lock_release(body->world);
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_RESIZE,
                                  _ephysics_body_box_face_obj_resize_cb,
                                  body);
}

static Evas_Object *
_ephysics_body_box_face_evas_object_get(const EPhysics_Body *body, EPhysics_Body_Face face)
{
   EPhysics_Body_Face_Obj *face_obj;
   Eina_List *l;
   void *ldata;

   if ((face < EPHYSICS_BODY_BOX_FACE_MIDDLE_FRONT) ||
       (face > EPHYSICS_BODY_BOX_FACE_BOTTOM))
     {
        ERR("Can't get evas object from body, face is invalid.");
        return NULL;
     }

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        if (face_obj->face == face)
          return face_obj->obj;
     }

   ERR("Couldn't find an object associated to face %i.", face);
   return NULL;
}

static Evas_Object *
_ephysics_body_box_face_evas_object_unset(EPhysics_Body *body, EPhysics_Body_Face face)
{
   EPhysics_Body_Face_Obj *face_obj;
   Eina_List *l;
   void *ldata;

   if ((face < EPHYSICS_BODY_BOX_FACE_MIDDLE_FRONT) ||
       (face > EPHYSICS_BODY_BOX_FACE_BOTTOM))
     {
        ERR("Can't unset evas object from body, face is invalid.");
        return NULL;
     }

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        if (face_obj->face == face)
          {
             Evas_Object *obj = face_obj->obj;
             _ephysics_body_face_obj_unset(
                obj, _ephysics_body_box_face_obj_resize_cb);
             body->face_objs = eina_list_remove(body->face_objs, face_obj);
             free(face_obj);
             return obj;
          }
     }

   ERR("Couldn't find an object associated to face %i.", face);
   return NULL;
}

static void
_ephysics_body_cylinder_face_obj_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   EPhysics_Body_Face face = EPHYSICS_BODY_FACE_LAST;
   EPhysics_Body *body = (EPhysics_Body *) data;
   EPhysics_Body_Face_Obj *face_obj;
   int w, h, bw, bh, bd;
   Eina_List *l;
   void *ldata;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        if (face_obj->obj == obj)
          {
             face = face_obj->face;
             break;
          }
     }

   switch(face)
     {
      case EPHYSICS_BODY_CYLINDER_FACE_FRONT:
      case EPHYSICS_BODY_CYLINDER_FACE_BACK:
      case EPHYSICS_BODY_CYLINDER_FACE_MIDDLE_FRONT:
      case EPHYSICS_BODY_CYLINDER_FACE_MIDDLE_BACK:
         if ((w == body->size.w) && (h == body->size.h))
           return;
         bw = w;
         bh = h;
         bd = body->size.d;
         break;
      case EPHYSICS_BODY_CYLINDER_FACE_CURVED:
         if (h == body->size.d)
           return;
         bw = body->size.w;
         bh = body->size.h;
         bd = h;
      default:
         return;
     }

   DBG("Resizing body %p to w=%i, h=%i, d=%i", body, bw, bh, bd);
   ephysics_world_lock_take(body->world);
   _ephysics_body_resize(body, bw, bh, bd);
   ephysics_world_lock_release(body->world);
}

static void
_ephysics_body_cylinder_face_evas_object_set(EPhysics_Body *body, EPhysics_Body_Face face, Evas_Object *evas_obj, Eina_Bool use_obj_pos)
{
   int obj_x, obj_y, obj_w, obj_h, bx, by, bz, bw, bh, bd;
   double rate;

   if ((face < EPHYSICS_BODY_CYLINDER_FACE_MIDDLE_FRONT) ||
       (face > EPHYSICS_BODY_CYLINDER_FACE_CURVED))
     {
        ERR("Can't set evas object to body, face is invalid.");
        return;
     }

   if (body->soft_body)
     {
        ERR("Not implemented for soft bodies yet.");
        return;
     }

   _ephysics_body_face_evas_object_add(
      body, face, evas_obj, _ephysics_body_cylinder_face_obj_resize_cb);
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_DEL,
                                  _ephysics_body_face_obj_del_cb, body);

   if (!use_obj_pos)
     return;

   rate = ephysics_world_rate_get(body->world);
   evas_object_geometry_get(evas_obj, &obj_x, &obj_y, &obj_w, &obj_h);
   ephysics_body_geometry_get(body, &bx, &by, &bz, &bw, &bh, &bd);

   ephysics_world_lock_take(body->world);

   switch(face)
     {
      case EPHYSICS_BODY_CYLINDER_FACE_FRONT:
      case EPHYSICS_BODY_CYLINDER_FACE_BACK:
      case EPHYSICS_BODY_CYLINDER_FACE_MIDDLE_FRONT:
      case EPHYSICS_BODY_CYLINDER_FACE_MIDDLE_BACK:
         _ephysics_body_geometry_set(body, obj_x, obj_y, bz,
                                     obj_w, obj_h, bd, rate);
         break;
      case EPHYSICS_BODY_CYLINDER_FACE_CURVED:
         _ephysics_body_geometry_set(body, bx, by, bz,
                                     bw, bh, obj_h, rate);
      default:
         break;
     }

   ephysics_world_lock_release(body->world);
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_RESIZE,
                                  _ephysics_body_cylinder_face_obj_resize_cb,
                                  body);
}

static Evas_Object *
_ephysics_body_cylinder_face_evas_object_get(const EPhysics_Body *body, EPhysics_Body_Face face)
{
   EPhysics_Body_Face_Obj *face_obj;
   Eina_List *l;
   void *ldata;

   if ((face < EPHYSICS_BODY_CYLINDER_FACE_MIDDLE_FRONT) ||
       (face > EPHYSICS_BODY_CYLINDER_FACE_CURVED))
     {
        ERR("Can't get evas object from body, face is invalid.");
        return NULL;
     }

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        if (face_obj->face == face)
          return face_obj->obj;
     }

   ERR("Couldn't find an object associated to face %i.", face);
   return NULL;
}

static Evas_Object *
_ephysics_body_cylinder_face_evas_object_unset(EPhysics_Body *body, EPhysics_Body_Face face)
{
   EPhysics_Body_Face_Obj *face_obj;
   Eina_List *l;
   void *ldata;

   if ((face < EPHYSICS_BODY_CYLINDER_FACE_MIDDLE_FRONT) ||
       (face > EPHYSICS_BODY_CYLINDER_FACE_CURVED))
     {
        ERR("Can't unset evas object from body, face is invalid.");
        return NULL;
     }

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        if (face_obj->face == face)
          {
             Evas_Object *obj = face_obj->obj;
             _ephysics_body_face_obj_unset(
                obj, _ephysics_body_cylinder_face_obj_resize_cb);
             body->face_objs = eina_list_remove(body->face_objs, face_obj);
             free(face_obj);
             return obj;
          }
     }

   ERR("Couldn't find an object associated to face %i.", face);
   return NULL;
}

static void
_ephysics_body_soft_body_face_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   int w, h;
   EPhysics_Body_Face_Obj *face;
   EPhysics_Body_Face_Slice *face_slice = (EPhysics_Body_Face_Slice *)data;
   EPhysics_Body *body = face_slice->body;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   if ((w == body->size.w) && (h == body->size.h))
     return;

   DBG("Resizing body %p to w=%i, h=%i, d=%i", body, w, h, body->size.d);

   face = _ephysics_body_face_evas_object_get(body, face_slice->face);

   ephysics_world_lock_take(body->world);
   _ephysics_body_resize(body, w, h, body->size.d);

   if (body->type == EPHYSICS_BODY_TYPE_CLOTH)
     {
        _ephysics_body_soft_body_slices_clean(face_slice->slices);
        _ephysics_body_soft_body_slices_init(body, face->obj,
                                             face_slice->slices);
     }

   ephysics_world_lock_release(body->world);
}

static void
_ephysics_body_cloth_face_evas_object_set(EPhysics_Body *body, EPhysics_Body_Face face, Evas_Object *evas_obj, Eina_Bool use_obj_pos)
{
   EPhysics_Body_Face_Slice *face_slice;
   int obj_x, obj_y, obj_w, obj_h, bz, bd;
   double rate;

   if ((face < EPHYSICS_BODY_CLOTH_FACE_FRONT) ||
       (face > EPHYSICS_BODY_CLOTH_FACE_BACK))
     {
        ERR("Can't set evas object to body, face is invalid.");
        return;
     }

   _ephysics_body_face_evas_object_add(body, face, evas_obj,
                                       _ephysics_body_soft_body_face_resize_cb);
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_DEL,
                                  _ephysics_body_face_obj_del_cb, body);

   if (!use_obj_pos)
     return;

   face_slice = _ephysics_body_face_slice_get(body, face);

   rate = ephysics_world_rate_get(body->world);
   evas_object_geometry_get(evas_obj, &obj_x, &obj_y, &obj_w, &obj_h);

   ephysics_world_lock_take(body->world);
   ephysics_body_geometry_get(body, NULL, NULL, &bz, NULL, NULL, &bd);
   _ephysics_body_geometry_set(body, obj_x, obj_y, bz, obj_w, obj_h, bd, rate);
   ephysics_world_lock_release(body->world);
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_RESIZE,
                                  _ephysics_body_soft_body_face_resize_cb,
                                  face_slice);
}

static Evas_Object *
_ephysics_body_cloth_face_evas_object_get(const EPhysics_Body *body, EPhysics_Body_Face face)
{
   EPhysics_Body_Face_Obj *face_obj;
   Eina_List *l;
   void *ldata;

   if ((face < EPHYSICS_BODY_CLOTH_FACE_FRONT) ||
       (face > EPHYSICS_BODY_CLOTH_FACE_BACK))
     {
        ERR("Can't get evas object from body, face is invalid.");
        return NULL;
     }

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        if (face_obj->face == face)
          return face_obj->obj;
     }

   ERR("Couldn't find an object associated to face %i.", face);
   return NULL;
}

static Evas_Object *
_ephysics_body_cloth_face_evas_object_unset(EPhysics_Body *body, EPhysics_Body_Face face)
{
   EPhysics_Body_Face_Obj *face_obj;
   Eina_List *l;
   void *ldata;

   if ((face < EPHYSICS_BODY_CLOTH_FACE_FRONT) ||
       (face > EPHYSICS_BODY_CLOTH_FACE_BACK))
     {
        ERR("Can't unset evas object from body, face is invalid.");
        return NULL;
     }

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        face_obj = (EPhysics_Body_Face_Obj *)ldata;
        if (face_obj->face == face)
          {
             Evas_Object *obj = face_obj->obj;
             _ephysics_body_face_obj_unset(
                obj, _ephysics_body_soft_body_face_resize_cb);
             body->face_objs = eina_list_remove(body->face_objs, face_obj);
             free(face_obj);
             return obj;
          }
     }

   ERR("Couldn't find an object associated to face %i.", face);
   return NULL;
}

static void
_ephysics_body_soft_sphere_face_obj_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   EPhysics_Body *body = (EPhysics_Body *) data;
   Evas_Coord bd, w, h;

   evas_object_geometry_get(obj, NULL, NULL, &w, &h);
   ephysics_body_geometry_get(body, NULL, NULL, NULL, NULL, NULL, &bd);
   ephysics_world_lock_take(body->world);
   ephysics_body_resize(body, w, h, bd);
   ephysics_world_lock_release(body->world);
}

static void
_ephysics_body_soft_sphere_face_evas_object_del_cb(void *data, Evas *evas EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   Eina_List *l;
   void *ldata;
   EPhysics_Body_Face_Obj *face_obj = NULL;
   EPhysics_Body_Face_Slice *face_slice = (EPhysics_Body_Face_Slice *)data;
   EPhysics_Body *body = face_slice->body;

   EINA_LIST_FOREACH(body->face_objs, l, ldata)
     {
        if (((EPhysics_Body_Face_Obj *)ldata)->face == face_slice->face)
          {
             face_obj = (EPhysics_Body_Face_Obj *)ldata;
             break;
          }
     }
   if (!face_obj) return;

   _ephysics_body_face_evas_object_del(body, face_obj,
                                  _ephysics_body_soft_sphere_face_obj_resize_cb);
   _ephysics_body_soft_body_slices_clean(face_slice->slices);

   DBG("Soft sphere's face cleaned up.");
}

static void
_ephysics_body_soft_sphere_face_evas_object_clean(EPhysics_Body *body, EPhysics_Body_Face_Obj *face_obj, Eina_List *slices)
{
   evas_object_map_enable_set(face_obj->obj, EINA_FALSE);
   evas_object_event_callback_del(face_obj->obj, EVAS_CALLBACK_DEL,
                             _ephysics_body_soft_sphere_face_evas_object_del_cb);

   evas_object_event_callback_del(face_obj->obj, EVAS_CALLBACK_RESTACK,
                                  _ephysics_body_soft_body_evas_restack_cb);
   _ephysics_body_soft_body_slices_clean(slices);

   _ephysics_body_face_evas_object_del(body, face_obj,
                                  _ephysics_body_soft_sphere_face_obj_resize_cb);
}

static void
_ephysics_body_soft_sphere_face_evas_object_set(EPhysics_Body *body, EPhysics_Body_Face face, Evas_Object *evas_obj, Eina_Bool use_obj_pos)
{
   int obj_x, obj_y, obj_w, obj_h, bz, bd;
   double rate;
   EPhysics_Body_Face_Slice *face_slice = NULL;
   EPhysics_Body_Face_Obj *face_obj;

   if ((face < EPHYSICS_BODY_SPHERE_FACE_FRONT) ||
       (face > EPHYSICS_BODY_SPHERE_FACE_BACK))
     {
        ERR("Can't set evas object to body, face is invalid.");
        return;
     }

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

   face_slice = _ephysics_body_face_slice_get(body, face);
   if (!face_slice)
     {
        ERR("Could not find pre initialized face slice for the wanted face.");
        return;
     }

   face_obj = _ephysics_body_face_evas_object_get(body, face);

   if (face_obj)
        _ephysics_body_soft_sphere_face_evas_object_clean(body, face_obj,
                                                          face_slice->slices);

   evas_object_geometry_get(evas_obj, NULL, NULL, &obj_w, &obj_h);
   if (!obj_w && !obj_h) evas_object_resize(evas_obj, 1, 1);

   _ephysics_body_face_evas_object_add(body, face, evas_obj,
                                  _ephysics_body_soft_sphere_face_obj_resize_cb);

   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_DEL,
                 _ephysics_body_soft_sphere_face_evas_object_del_cb, face_slice);

   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_RESTACK,
                                  _ephysics_body_soft_body_evas_restack_cb,
                                  body);

   _ephysics_body_soft_body_slices_init(body, evas_obj, face_slice->slices);

   if (!use_obj_pos)
     return;

   rate = ephysics_world_rate_get(body->world);
   evas_object_geometry_get(evas_obj, &obj_x, &obj_y, &obj_w, &obj_h);
   ephysics_body_geometry_get(body, NULL, NULL, &bz, NULL, NULL, &bd);

   ephysics_world_lock_take(body->world);
   _ephysics_body_geometry_set(body, obj_x, obj_y, bz, obj_w, obj_h, bd, rate);
   ephysics_world_lock_release(body->world);
   evas_object_event_callback_add(evas_obj, EVAS_CALLBACK_RESIZE,
                                  _ephysics_body_evas_obj_resize_cb, body);

   DBG("Soft sphere's face evas object set.");
}

static Evas_Object *
_ephysics_body_soft_sphere_face_evas_object_get(const EPhysics_Body *body, EPhysics_Body_Face face)
{
   EPhysics_Body_Face_Obj *face_obj = NULL;

   if ((face < EPHYSICS_BODY_SPHERE_FACE_FRONT) ||
       (face > EPHYSICS_BODY_SPHERE_FACE_BACK))
     {
        ERR("Can't get evas object from body, face is invalid.");
        return NULL;
     }

   face_obj = _ephysics_body_face_evas_object_get((EPhysics_Body *)body, face);
   if (face_obj) return face_obj->obj;

   ERR("Couldn't find an object associated to face %i.", face);
   return NULL;
}

static Evas_Object *
_ephysics_body_soft_sphere_face_evas_object_unset(EPhysics_Body *body, EPhysics_Body_Face face)
{
   EPhysics_Body_Face_Slice *face_slice;
   EPhysics_Body_Face_Obj *face_obj = NULL;
   Evas_Object *obj;

   if ((face < EPHYSICS_BODY_SPHERE_FACE_FRONT) ||
       (face > EPHYSICS_BODY_SPHERE_FACE_BACK))
     {
        ERR("Can't unset evas object from body, face is invalid.");
        return NULL;
     }

   face_obj = _ephysics_body_face_evas_object_get(body, face);

   if (!face_obj) return NULL;

   obj = face_obj->obj;
   face_slice = _ephysics_body_face_slice_get(body, face);
   _ephysics_body_soft_sphere_face_evas_object_clean(body, face_obj,
                                                   face_slice->slices);

   DBG("EPhysics Body face unset.");
   return obj;
}

EAPI void
ephysics_body_face_evas_object_set(EPhysics_Body *body, EPhysics_Body_Face face, Evas_Object *evas_obj, Eina_Bool use_obj_pos)
{
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

   if (body->type == EPHYSICS_BODY_TYPE_CLOTH)
     return _ephysics_body_cloth_face_evas_object_set(body, face, evas_obj,
                                                      use_obj_pos);
   if (body->type == EPHYSICS_BODY_TYPE_RIGID &&
       body->shape == EPHYSICS_BODY_SHAPE_CYLINDER)
     return _ephysics_body_cylinder_face_evas_object_set(body, face, evas_obj,
                                                         use_obj_pos);
   if (body->type == EPHYSICS_BODY_TYPE_RIGID &&
       body->shape == EPHYSICS_BODY_SHAPE_BOX)
     return _ephysics_body_box_face_evas_object_set(body, face, evas_obj,
                                                    use_obj_pos);
   if (body->type == EPHYSICS_BODY_TYPE_SOFT &&
       body->shape == EPHYSICS_BODY_SHAPE_SPHERE)
     return _ephysics_body_soft_sphere_face_evas_object_set(body, face, evas_obj,
                                                            use_obj_pos);

   ERR("Can't handle body %p type.", body);
}

EAPI Evas_Object *
ephysics_body_face_evas_object_get(const EPhysics_Body *body, EPhysics_Body_Face face)
{
   if (!body)
     {
        ERR("Can't get evas object from body, it wasn't provided.");
        return NULL;
     }

   if (body->type == EPHYSICS_BODY_TYPE_CLOTH)
     return _ephysics_body_cloth_face_evas_object_get(body, face);
   if (body->type == EPHYSICS_BODY_TYPE_RIGID &&
       body->shape == EPHYSICS_BODY_SHAPE_CYLINDER)
     return _ephysics_body_cylinder_face_evas_object_get(body, face);
   if (body->type == EPHYSICS_BODY_TYPE_RIGID &&
       body->shape == EPHYSICS_BODY_SHAPE_BOX)
     return _ephysics_body_box_face_evas_object_get(body, face);
   if (body->type == EPHYSICS_BODY_TYPE_SOFT &&
       body->shape == EPHYSICS_BODY_SHAPE_SPHERE)
     return _ephysics_body_soft_sphere_face_evas_object_get(body, face);

   ERR("Can't handle body %p type.", body);
   return NULL;
}

EAPI Evas_Object *
ephysics_body_face_evas_object_unset(EPhysics_Body *body, EPhysics_Body_Face face)
{
   if (!body)
     {
        ERR("Can't unset evas object from body, it wasn't provided.");
        return NULL;
     }

   if (body->type == EPHYSICS_BODY_TYPE_CLOTH)
     return _ephysics_body_cloth_face_evas_object_unset(body, face);
   if (body->type == EPHYSICS_BODY_TYPE_RIGID &&
       body->shape == EPHYSICS_BODY_SHAPE_CYLINDER)
     return _ephysics_body_cylinder_face_evas_object_unset(body, face);
   if (body->type == EPHYSICS_BODY_TYPE_RIGID &&
       body->shape == EPHYSICS_BODY_SHAPE_BOX)
     return _ephysics_body_box_face_evas_object_unset(body, face);
   if (body->type == EPHYSICS_BODY_TYPE_SOFT &&
       body->shape == EPHYSICS_BODY_SHAPE_SPHERE)
     return _ephysics_body_soft_sphere_face_evas_object_unset(body, face);

   ERR("Can't handle body %p type.", body);
   return NULL;
}

#ifdef  __cplusplus
}
#endif
