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
     double x, y, zoom;
};

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

EAPI void
ephysics_camera_position_set(EPhysics_Camera *camera, double x, double y)
{
   if (!camera)
     {
        ERR("Can't set camera position, camerar is null.");
        return;
     }

   camera->x = x;
   camera->y = y;
}

EAPI void
ephysics_camera_position_get(const EPhysics_Camera *camera, double *x, double *y)
{
   if (!camera)
     {
        ERR("Can't get camera position, camera is null.");
        return;
     }

   if (x) *x = camera->x;
   if (y) *y = camera->y;
}

EAPI void
ephysics_camera_zoom_set(EPhysics_Camera *camera, double zoom)
{
   if (!camera)
     {
        ERR("Can't set camera zoom, camera == NULL.");
        return;
     }

   if (zoom <= 0)
     {
	ERR("Zoom should be a positive value. Keeping the old value: %lf.",
            camera->zoom);
        return;
     }

   camera->zoom = zoom;
}

EAPI double
ephysics_camera_zoom_get(const EPhysics_Camera *camera)
{
   if (!camera)
     {
        ERR("Can't get camera zoom, camera == NULL.");
        return -1;
     }

   return camera->zoom;
}

#ifdef  __cplusplus
}
#endif
