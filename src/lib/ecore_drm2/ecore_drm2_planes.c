#include "ecore_drm2_private.h"

static void
_ecore_drm2_plane_state_debug(Ecore_Drm2_Plane *plane)
{
   DBG("Plane Atomic State Fill Complete");
   DBG("\tPlane: %d", plane->state->obj_id);
   DBG("\t\tCrtc: %lu", (long)plane->state->cid.value);
   DBG("\t\tFB: %lu", (long)plane->state->fid.value);
   switch (plane->state->type.value)
     {
      case DRM_PLANE_TYPE_OVERLAY:
        DBG("\t\tType: Overlay Plane");
        break;
      case DRM_PLANE_TYPE_PRIMARY:
        DBG("\t\tType: Primary Plane");
        break;
      case DRM_PLANE_TYPE_CURSOR:
        DBG("\t\tType: Cursor Plane");
        break;
      default:
        break;
     }
}

static void
_ecore_drm2_plane_state_fill(Ecore_Drm2_Plane *plane)
{
   Ecore_Drm2_Plane_State *pstate;
   drmModeObjectPropertiesPtr oprops;
   drmModePlanePtr p;
   unsigned int i = 0;

   plane->state = calloc(1, sizeof(Ecore_Drm2_Plane_State));
   if (!plane->state)
     {
        ERR("Could not allocate space for plane state");
        return;
     }

   p = plane->drmPlane;
   pstate = plane->state;

   pstate->obj_id = plane->id;
   pstate->mask = p->possible_crtcs;
   pstate->num_formats = p->count_formats;

   pstate->formats = calloc(p->count_formats, sizeof(uint32_t));
   for (; i < p->count_formats; i++)
     pstate->formats[i] = p->formats[i];

   /* try to fill get drm properties of this plane */
   oprops =
     sym_drmModeObjectGetProperties(plane->fd, pstate->obj_id,
                                    DRM_MODE_OBJECT_PLANE);
   if (!oprops) return;

   /* fill atomic state */
   for (i = 0; i < oprops->count_props; i++)
     {
        drmModePropertyPtr prop;

        prop = sym_drmModeGetProperty(plane->fd, oprops->props[i]);
        if (!prop) continue;

        if (!strcmp(prop->name, "CRTC_ID"))
          {
             pstate->cid.id = prop->prop_id;
             pstate->cid.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "FB_ID"))
          {
             pstate->fid.id = prop->prop_id;
             pstate->fid.value = oprops->prop_values[i];
         }
        else if (!strcmp(prop->name, "CRTC_X"))
          {
             pstate->cx.id = prop->prop_id;
             pstate->cx.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "CRTC_Y"))
          {
             pstate->cy.id = prop->prop_id;
             pstate->cy.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "CRTC_W"))
          {
             pstate->cw.id = prop->prop_id;
             pstate->cw.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "CRTC_H"))
          {
             pstate->ch.id = prop->prop_id;
             pstate->ch.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_X"))
          {
             pstate->sx.id = prop->prop_id;
             pstate->sx.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_Y"))
          {
             pstate->sy.id = prop->prop_id;
             pstate->sy.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_W"))
          {
             pstate->sw.id = prop->prop_id;
             pstate->sw.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_H"))
          {
             pstate->sh.id = prop->prop_id;
             pstate->sh.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "type"))
          {
             pstate->type.id = prop->prop_id;
             pstate->type.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "rotation"))
          {
             int k = 0;

             pstate->rotation.id = prop->prop_id;
             pstate->rotation.value = oprops->prop_values[i];

             for (k = 0; k < prop->count_enums; k++)
               {
                  int r = -1;

                  /* DBG("\t\t\tRotation: %s", prop->enums[k].name); */
                  if (!strcmp(prop->enums[k].name, "rotate-0"))
                    r = ECORE_DRM2_ROTATION_NORMAL;
                  else if (!strcmp(prop->enums[k].name, "rotate-90"))
                    r = ECORE_DRM2_ROTATION_90;
                  else if (!strcmp(prop->enums[k].name, "rotate-180"))
                    r = ECORE_DRM2_ROTATION_180;
                  else if (!strcmp(prop->enums[k].name, "rotate-270"))
                    r = ECORE_DRM2_ROTATION_270;
                  else if (!strcmp(prop->enums[k].name, "reflect-x"))
                    r = ECORE_DRM2_ROTATION_REFLECT_X;
                  else if (!strcmp(prop->enums[k].name, "reflect-y"))
                    r = ECORE_DRM2_ROTATION_REFLECT_Y;

                  if (r != -1)
                    {
                       pstate->supported_rotations |= r;
                       pstate->rotation_map[ffs(r)] =
                         1ULL << prop->enums[k].value;
                    }
               }
          }

        sym_drmModeFreeProperty(prop);
     }

   sym_drmModeFreeObjectProperties(oprops);
}

static void
_ecore_drm2_plane_state_thread(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecore_Drm2_Plane *plane;

   plane = data;
   if (!plane->state)
     _ecore_drm2_plane_state_fill(plane);
   else
     {
        /* TODO: update atomic state for commit */
     }
}

static void
_ecore_drm2_plane_state_thread_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecore_Drm2_Plane *plane;

   plane = data;
   _ecore_drm2_plane_state_debug(plane);
}

static void
_ecore_drm2_plane_state_thread_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecore_Drm2_Plane *plane;

   plane = data;
   plane->thread = NULL;
}

static Ecore_Drm2_Plane *
_ecore_drm2_plane_create(Ecore_Drm2_Device *dev, drmModePlanePtr p, uint32_t index)
{
   Ecore_Drm2_Plane *plane;

   /* try to allocate space for a new plane */
   plane = calloc(1, sizeof(Ecore_Drm2_Plane));
   if (!plane)
     {
        ERR("Could not allocate space for plane");
        return EINA_FALSE;
     }

   plane->fd = dev->fd;
   plane->id = index;
   plane->drmPlane = p;

   /* append this plane to the list */
   dev->planes = eina_list_append(dev->planes, plane);

   return plane;
}

Eina_Bool
_ecore_drm2_planes_create(Ecore_Drm2_Device *dev)
{
   Ecore_Drm2_Plane *plane;
   drmModePlaneResPtr pres;
   uint32_t i = 0;

   /* try to get plane resources from drm */
   pres = sym_drmModeGetPlaneResources(dev->fd);
   if (!pres) return EINA_FALSE;

   for (; i < pres->count_planes; i++)
     {
        drmModePlanePtr p;

        /* try to get this plane from drm */
        p = sym_drmModeGetPlane(dev->fd, pres->planes[i]);
        if (!p) continue;

        /* try to create a plane */
        plane = _ecore_drm2_plane_create(dev, p, pres->planes[i]);
        if (!plane) continue;

        /* NB: Use an explicit thread to fill plane atomic state */
        plane->thread =
          ecore_thread_feedback_run(_ecore_drm2_plane_state_thread,
                                    NULL, // _ecore_drm2_plane_state_thread_notify
                                    _ecore_drm2_plane_state_thread_end,
                                    _ecore_drm2_plane_state_thread_cancel,
                                    plane, EINA_TRUE);
     }

   sym_drmModeFreePlaneResources(pres);
   return EINA_TRUE;
}

void
_ecore_drm2_planes_destroy(Ecore_Drm2_Device *dev)
{
   Ecore_Drm2_Plane *plane;

   EINA_LIST_FREE(dev->planes, plane)
     {
        if (plane->drmPlane) sym_drmModeFreePlane(plane->drmPlane);
        free(plane->state);
        free(plane);
     }
}
