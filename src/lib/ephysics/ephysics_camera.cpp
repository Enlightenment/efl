#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Evas.h>

#include "ephysics_private.h"

#ifdef  __cplusplus
extern "C" {
#endif

struct _EPhysics_Camera {
     EPhysics_World *world;
     EPhysics_Body *target;
     int x, y;
     struct {
          Evas_Coord px;
          Evas_Coord py;
          Evas_Coord z0;
          Evas_Coord foc;
          Eina_Bool enabled:1;
     } perspective;
     Eina_Bool track_horizontal:1;
     Eina_Bool track_vertical:1;
     Eina_Bool moved:1;
};

static void
_ephysics_camera_target_del_cb(void *data, EPhysics_Body *body, void *event_info EINA_UNUSED)
{
   EPhysics_Camera *camera = (EPhysics_Camera *) data;

   camera->target = NULL;
   camera->track_horizontal = EINA_FALSE;
   camera->track_vertical = EINA_FALSE;

   INF("Camera isn't tracking body %p anymore.", body);
}

void
ephysics_camera_target_moved(EPhysics_Camera *camera, EPhysics_Body *body)
{
   int x, y, w, h, ww, wh, new_x, new_y;

   ephysics_body_geometry_get(body, &x, &y, NULL, &w, &h, NULL);
   ephysics_world_render_geometry_get(camera->world, NULL, NULL, NULL,
                                      &ww, &wh, NULL);

   if (camera->track_horizontal)
     {
        new_x = x + w / 2 - ww / 2;
        if (camera->x != new_x)
          {
             camera->x = new_x;
             camera->moved = EINA_TRUE;
          }
     }

   if (camera->track_vertical)
     {
        new_y =  y + h / 2 - wh / 2;
        if (camera->y != new_y)
          {
             camera->y = new_y;
             camera->moved = EINA_TRUE;
          }
     }

   INF("Camera position set to (%i, %i).", camera->x, camera->y);
}

void
ephysics_camera_moved_set(EPhysics_Camera *camera, Eina_Bool moved)
{
   camera->moved = moved;
}

Eina_Bool
ephysics_camera_moved_get(const EPhysics_Camera *camera)
{
   return camera->moved;
}

EPhysics_Camera *
ephysics_camera_add(EPhysics_World *world)
{
   EPhysics_Camera *camera;

   camera = (EPhysics_Camera *) calloc(1, sizeof(EPhysics_Camera));
   if (!camera)
     {
        ERR("Couldn't create a new camera instance.");
        return NULL;
     }

   camera->world = world;

   INF("Camera created.");
   return camera;
}

void
ephysics_camera_del(EPhysics_Camera *camera)
{
   if (!camera)
     {
        ERR("Can't delete camera, it wasn't provided.");
        return;
     }

   free(camera);
   INF("Camera deleted.");
}

EPHYSICS_API void
ephysics_camera_position_set(EPhysics_Camera *camera, Evas_Coord x, Evas_Coord y)
{
   if (!camera)
     {
        ERR("Can't set camera position, camerar is null.");
        return;
     }

   if (camera->target)
     {
        INF("Camera isn't tracking body %p anymore.", camera->target);

        ephysics_body_event_callback_del(camera->target,
                                         EPHYSICS_CALLBACK_BODY_DEL,
                                         _ephysics_camera_target_del_cb);
        camera->target = NULL;
        camera->track_horizontal = EINA_FALSE;
        camera->track_vertical = EINA_FALSE;
     }

   camera->x = x;
   camera->y = y;
   camera->moved = EINA_TRUE;

   INF("Camera position set to (%i, %i).", x, y);
}

EPHYSICS_API void
ephysics_camera_position_get(const EPhysics_Camera *camera, Evas_Coord *x, Evas_Coord *y)
{
   if (!camera)
     {
        ERR("Can't get camera position, camera is null.");
        return;
     }

   if (x) *x = camera->x;
   if (y) *y = camera->y;
}

EPHYSICS_API void
ephysics_camera_body_track(EPhysics_Camera *camera, EPhysics_Body *body, Eina_Bool horizontal, Eina_Bool vertical)
{
   if (!camera)
     {
        ERR("Camera can't track body, camera is null.");
        return;
     }

   camera->track_horizontal = !!horizontal;
   camera->track_vertical = !!vertical;

   if ((body) && (camera->target == body))
     {
        INF("Camera already tracking body %p.", body);
        INF("Camera tracking: hor = %i, ver = %i.", !!horizontal, !!vertical);
        return;
     }

   if (camera->target)
     {
        ephysics_body_event_callback_del(camera->target,
                                         EPHYSICS_CALLBACK_BODY_DEL,
                                         _ephysics_camera_target_del_cb);
     }

   camera->target = body;

   if (!body)
     {
        INF("Camera isn't tracking any body.");
        return;
     }

   ephysics_body_event_callback_add(body, EPHYSICS_CALLBACK_BODY_DEL,
                                    _ephysics_camera_target_del_cb, camera);

   INF("Camera is tracking body %p: hor = %i, ver = %i.", body,
       camera->track_horizontal, camera->track_vertical);
}

EPHYSICS_API void
ephysics_camera_tracked_body_get(EPhysics_Camera *camera, EPhysics_Body **body, Eina_Bool *horizontal, Eina_Bool *vertical)
{
   if (!camera)
     {
        ERR("Can't get tracked body, camera is null.");
        return;
     }

   if (body) *body = camera->target;
   if (horizontal) *horizontal = camera->track_horizontal;
   if (vertical) *vertical = camera->track_vertical;
}

EPHYSICS_API void
ephysics_camera_perspective_set(EPhysics_Camera *camera, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc)
{
   if (!camera)
     {
	ERR("No camera, can't set perspective.");
	return;
     }

   if (foc <= 0)
     {
	ERR("Focal distance need to be greater than 0.");
	return;
     }

   camera->perspective.px = px;
   camera->perspective.py = py;
   camera->perspective.z0 = z0;
   camera->perspective.foc = foc;
   ephysics_world_force_update_set(camera->world, EINA_TRUE);
}

EPHYSICS_API void
ephysics_camera_perspective_get(const EPhysics_Camera *camera, Evas_Coord *px, Evas_Coord *py, Evas_Coord *z0, Evas_Coord *foc)
{
   if (!camera)
     {
	ERR("No camera, can't get perspective.");
	return;
     }

   if (px) *px = camera->perspective.px;
   if (py) *py = camera->perspective.py;
   if (z0) *z0 = camera->perspective.z0;
   if (foc) *foc = camera->perspective.foc;
}

EPHYSICS_API void
ephysics_camera_perspective_enabled_set(EPhysics_Camera *camera, Eina_Bool enabled)
{
   if (!camera)
     {
	ERR("No camera, can't enable / disable perspective.");
	return;
     }

   camera->perspective.enabled = !!enabled;
   ephysics_world_force_update_set(camera->world, EINA_TRUE);
}

EPHYSICS_API Eina_Bool
ephysics_camera_perspective_enabled_get(const EPhysics_Camera *camera)
{
   if (!camera)
     {
	ERR("No camera, can't get perspective behavior.");
	return EINA_FALSE;
     }

   return camera->perspective.enabled;
}

#ifdef  __cplusplus
}
#endif
