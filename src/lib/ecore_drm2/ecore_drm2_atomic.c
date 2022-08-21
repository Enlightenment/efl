#include "ecore_drm2_private.h"

static Eina_Bool
_ecore_drm2_atomic_state_crtc_fill(Ecore_Drm2_Crtc_State *cstate, int fd)
{
   drmModeObjectPropertiesPtr oprops;
   unsigned int i = 0;

   DBG("Atomic State Crtc Fill");

   /* try to get crtc object properties */
   oprops =
     sym_drmModeObjectGetProperties(fd, cstate->obj_id, DRM_MODE_OBJECT_CRTC);
   if (!oprops) return EINA_FALSE;

   DBG("\tCrtc %d", cstate->obj_id);

   for (; i < oprops->count_props; i++)
     {
        drmModePropertyPtr prop;

        /* try to get this property */
        prop = sym_drmModeGetProperty(fd, oprops->props[i]);
        if (!prop) continue;

        DBG("\t\tProperty: %s %d", prop->name, i);

        /* find the properties we are interested in and fill in crtc state */
        if (!strcmp(prop->name, "MODE_ID"))
          {
             drmModePropertyBlobPtr bp;

             cstate->mode.id = prop->prop_id;
             cstate->mode.value = oprops->prop_values[i];

             DBG("\t\t\tValue: %d", cstate->mode.value);

             if (!cstate->mode.value)
               {
                  cstate->mode.len = 0;
                  goto cont;
               }

             bp = sym_drmModeGetPropertyBlob(fd, cstate->mode.value);
             if (!bp) goto cont;

             if ((!cstate->mode.data) ||
                 memcmp(cstate->mode.data, bp->data, bp->length) != 0)
               cstate->mode.data = eina_memdup(bp->data, bp->length, 1);

             cstate->mode.len = bp->length;

             if (cstate->mode.value != 0)
               sym_drmModeCreatePropertyBlob(fd, bp->data, bp->length,
                                             &cstate->mode.value);

             sym_drmModeFreePropertyBlob(bp);
          }
        else if (!strcmp(prop->name, "ACTIVE"))
          {
             cstate->active.id = prop->prop_id;
             cstate->active.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)cstate->active.value);
          }
        /* else if (!strcmp(prop->name, "BACKGROUND_COLOR")) */
        /*   { */
        /*      cstate->background.id = prop->prop_id; */
        /*      cstate->background.value = oprops->prop_values[i]; */
        /*   } */

cont:
        sym_drmModeFreeProperty(prop);
     }

   /* free crtc object properties */
   sym_drmModeFreeObjectProperties(oprops);

   return EINA_TRUE;
}

static Eina_Bool
_ecore_drm2_atomic_state_connector_fill(Ecore_Drm2_Connector_State *cstate, int fd)
{
   drmModeObjectPropertiesPtr oprops;
   unsigned int i = 0;

   DBG("Atomic State Connector Fill");

   /* try to get connector object properties */
   oprops =
     sym_drmModeObjectGetProperties(fd, cstate->obj_id,
                                    DRM_MODE_OBJECT_CONNECTOR);
   if (!oprops) return EINA_FALSE;

   DBG("\tConnector %d", cstate->obj_id);

   for (; i < oprops->count_props; i++)
     {
        drmModePropertyPtr prop;

        /* try to get this property */
        prop = sym_drmModeGetProperty(fd, oprops->props[i]);
        if (!prop) continue;

        DBG("\t\tProperty: %s %d", prop->name, i);

        /* find the properties we are interested in and fill in conn state */

        if (!strcmp(prop->name, "CRTC_ID"))
          {
             cstate->crtc.id = prop->prop_id;
             cstate->crtc.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)cstate->crtc.value);
          }
        else if (!strcmp(prop->name, "DPMS"))
          {
             cstate->dpms.id = prop->prop_id;
             cstate->dpms.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)cstate->dpms.value);
          }
        else if (!strcmp(prop->name, "aspect ratio"))
          {
             cstate->aspect.id = prop->prop_id;
             cstate->aspect.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)cstate->aspect.value);
          }
        else if (!strcmp(prop->name, "scaling mode"))
          {
             cstate->scaling.id = prop->prop_id;
             cstate->scaling.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)cstate->scaling.value);
          }
        else if (!strcmp(prop->name, "EDID"))
          {
             drmModePropertyBlobPtr bp;

             cstate->edid.id = oprops->prop_values[i];
             if (!cstate->edid.id)
               {
                  cstate->edid.len = 0;
                  goto cont;
               }

             bp = sym_drmModeGetPropertyBlob(fd, cstate->edid.id);
             if (!bp) goto cont;

             if ((!cstate->edid.data) ||
                 memcmp(cstate->edid.data, bp->data, bp->length) != 0)
               {
                  cstate->edid.data =
                    eina_memdup(bp->data, bp->length, 1);
               }

             cstate->edid.len = bp->length;

             if (cstate->edid.id != 0)
               sym_drmModeCreatePropertyBlob(fd, bp->data, bp->length,
                                         &cstate->edid.id);

             sym_drmModeFreePropertyBlob(bp);
          }
cont:
        sym_drmModeFreeProperty(prop);
     }

   /* free crtc object properties */
   sym_drmModeFreeObjectProperties(oprops);

   return EINA_TRUE;
}

static Eina_Bool
_ecore_drm2_atomic_state_plane_fill(Ecore_Drm2_Plane_State *pstate, int fd)
{
   drmModeObjectPropertiesPtr oprops;
   unsigned int i = 0;

   DBG("Atomic State Plane Fill");

   /* try to get plane object properties */
   oprops =
     sym_drmModeObjectGetProperties(fd, pstate->obj_id, DRM_MODE_OBJECT_PLANE);
   if (!oprops) return EINA_FALSE;

   DBG("\tPlane %d", pstate->obj_id);

   for (; i < oprops->count_props; i++)
     {
        drmModePropertyPtr prop;

        /* try to get this property */
        prop = sym_drmModeGetProperty(fd, oprops->props[i]);
        if (!prop) continue;

        DBG("\t\tProperty: %s %d", prop->name, i);

        /* find the properties we are interested in and fill in plane state */
        if (!strcmp(prop->name, "CRTC_ID"))
          {
             pstate->cid.id = prop->prop_id;
             pstate->cid.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)pstate->cid.value);
          }
        else if (!strcmp(prop->name, "FB_ID"))
          {
             pstate->fid.id = prop->prop_id;
             pstate->fid.value = oprops->prop_values[i];
             DBG("\t\t\tValue: %lu", (long)pstate->fid.value);
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
             switch (pstate->type.value)
               {
                case DRM_PLANE_TYPE_OVERLAY:
                  DBG("\t\t\tOverlay Plane");
                  break;
                case DRM_PLANE_TYPE_PRIMARY:
                  DBG("\t\t\tPrimary Plane");
                  break;
                case DRM_PLANE_TYPE_CURSOR:
                  DBG("\t\t\tCursor Plane");
                  break;
                default:
                  DBG("\t\t\tValue: %lu", (long)pstate->type.value);
                  break;
               }
          }
        else if (!strcmp(prop->name, "rotation"))
          {
             int k = 0;

             pstate->rotation.id = prop->prop_id;
             pstate->rotation.value = oprops->prop_values[i];

             for (k = 0; k < prop->count_enums; k++)
               {
                  int r = -1;

                  DBG("\t\t\tRotation: %s", prop->enums[k].name);
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

   /* free plane object properties */
   sym_drmModeFreeObjectProperties(oprops);

   return EINA_TRUE;
}

Eina_Bool
_ecore_drm2_atomic_state_fill(Ecore_Drm2_Atomic_State *state, int fd)
{
   int i = 0;
   drmModeResPtr res;
   drmModePlaneResPtr pres;

   /* try to get drm resources */
   res = sym_drmModeGetResources(fd);
   if (!res) return EINA_FALSE;

   state->crtcs = res->count_crtcs;

   /* try to allocate space for crtc states */
   state->crtc_states = calloc(state->crtcs, sizeof(Ecore_Drm2_Crtc_State));
   if (!state->crtc_states)
     {
        ERR("Could not allocate space for CRTC states");
        goto crtc_err;
     }

   /* try to fill atomic state for each crtc */
   for (i = 0; i < state->crtcs; i++)
     {
        Ecore_Drm2_Crtc_State *cstate;

        cstate = &state->crtc_states[i];
        cstate->obj_id = res->crtcs[i];
        cstate->index = i;

        /* try fill atomic state for this crtc */
        if (!_ecore_drm2_atomic_state_crtc_fill(cstate, fd))
          {
             WRN("Failed to fill atomic crtc state for CRTC %d",
                 cstate->obj_id);
             continue;
          }
     }

   /* try to allocate space for connector states */
   state->conns = res->count_connectors;
   state->conn_states = calloc(state->conns, sizeof(Ecore_Drm2_Connector_State));
   if (!state->conn_states)
     {
        ERR("Could not allocate space for CONN states");
        goto conn_err;
     }

   /* try to fill atomic state for each connector */
   for (i = 0; i < state->conns; i++)
     {
        Ecore_Drm2_Connector_State *cstate;

        cstate = &state->conn_states[i];
        cstate->obj_id = res->connectors[i];

        /* try to fill atomic state for this connector */
        if (!_ecore_drm2_atomic_state_connector_fill(cstate, fd))
          {
             WRN("Failed to fill atomic connector state for CONN %d",
                 cstate->obj_id);
             continue;
          }
     }

   /* try to get plane resources */
   pres = sym_drmModeGetPlaneResources(fd);
   if (!pres) goto plane_res_err;

   /* try to allocate space for plane states */
   state->planes = pres->count_planes;
   state->plane_states = calloc(state->planes, sizeof(Ecore_Drm2_Plane_State));
   if (!state->plane_states)
     {
        ERR("Could not allocate space for PLANE states");
        goto plane_err;
     }

   /* try to fill atomic state for each plane */
   for (i = 0; i < state->planes; i++)
     {
        unsigned int f = 0;
        drmModePlanePtr plane;
        Ecore_Drm2_Plane_State *pstate;

        /* try to get this plane */
        plane = sym_drmModeGetPlane(fd, pres->planes[i]);
        if (!plane) continue;

        pstate = &state->plane_states[i];

        pstate->obj_id = pres->planes[i];
        pstate->mask = plane->possible_crtcs;

        /* try to get the formats supported on this plane */
        pstate->num_formats = plane->count_formats;
        pstate->formats = calloc(plane->count_formats, sizeof(uint32_t));
        for (; f < plane->count_formats; f++)
          pstate->formats[f] = plane->formats[f];

        /* free drm plane */
        sym_drmModeFreePlane(plane);

        /* try to fill atomic state for this plane */
        if (!_ecore_drm2_atomic_state_plane_fill(pstate, fd))
          {
             WRN("Failed to fill atomic plane state for PLANE %d",
                 pstate->obj_id);
             continue;
          }
     }

   /* free drm plane resources */
   sym_drmModeFreePlaneResources(pres);

   /* free drm resources */
   sym_drmModeFreeResources(res);

   return EINA_TRUE;

plane_err:
   sym_drmModeFreePlaneResources(pres);
plane_res_err:
   free(state->conn_states);
conn_err:
   free(state->crtc_states);
crtc_err:
   sym_drmModeFreeResources(res);
   return EINA_FALSE;
}

void
_ecore_drm2_atomic_state_free(Ecore_Drm2_Atomic_State *state)
{
   free(state->plane_states);
   free(state->conn_states);
   free(state->crtc_states);
   free(state);
}
