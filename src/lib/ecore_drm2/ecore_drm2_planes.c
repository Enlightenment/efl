#include "ecore_drm2_private.h"

static Eina_Thread_Queue *thq = NULL;

typedef struct
{
   Eina_Thread_Queue_Msg head;
   Ecore_Drm2_Thread_Op_Code code;
} Thread_Msg;

static void
_ecore_drm2_plane_state_thread_send(Ecore_Drm2_Thread_Op_Code code)
{
   Thread_Msg *msg;
   void *ref;

   msg = eina_thread_queue_send(thq, sizeof(Thread_Msg), &ref);
   msg->code = code;
   eina_thread_queue_send_done(thq, ref);
}

static void
_ecore_drm2_plane_state_debug(Ecore_Drm2_Plane *plane)
{
   DBG("Plane Atomic State Fill Complete");
   DBG("\tPlane: %d", plane->state.current->obj_id);
   DBG("\t\tCrtc: %lu", (long)plane->state.current->cid.value);
   DBG("\t\tFB: %lu", (long)plane->state.current->fid.value);
   switch (plane->state.current->type.value)
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
   DBG("\t\tZPos: %lu", (long)plane->state.current->zpos.value);
   DBG("\t\t\tMin: %lu", (long)plane->state.current->zpos.min);
   DBG("\t\t\tMax: %lu", (long)plane->state.current->zpos.max);
}

static Eina_Bool
_ecore_drm2_plane_state_formats_add(Ecore_Drm2_Plane_State *pstate, drmModePropertyBlobPtr bp)
{
   drmModeFormatModifierIterator iter = {0};
   uint32_t prev = DRM_FORMAT_INVALID;

   while (sym_drmModeFormatModifierBlobIterNext(bp, &iter))
     {
        if (prev != iter.fmt)
          {
             Ecore_Drm2_Format *fmt;

             /* allocate new Ecore_Drm2_Format */
             fmt = calloc(1, sizeof(Ecore_Drm2_Format));
             if (!fmt) return EINA_FALSE;

             fmt->format = iter.fmt;
             fmt->modifier = iter.mod;

             /* add this format to the pstate formats list */
             pstate->formats = eina_list_append(pstate->formats, fmt);
             prev = iter.fmt;
          }
     }

   return EINA_TRUE;
}

static void
_ecore_drm2_plane_state_fill(Ecore_Drm2_Plane *plane)
{
   Ecore_Drm2_Plane_State *pstate;
   drmModeObjectPropertiesPtr oprops;
   drmModePlanePtr p;
   unsigned int i = 0;
   Eina_Bool in_formats = EINA_FALSE;

   plane->state.current = calloc(1, sizeof(Ecore_Drm2_Plane_State));
   if (!plane->state.current)
     {
        ERR("Could not allocate space for plane state");
        return;
     }

   p = plane->drmPlane;
   pstate = plane->state.current;

   pstate->obj_id = plane->id;
   pstate->mask = p->possible_crtcs;

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
             pstate->cid.flags = prop->flags;
             pstate->cid.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "FB_ID"))
          {
             pstate->fid.id = prop->prop_id;
             pstate->fid.flags = prop->flags;
             pstate->fid.value = oprops->prop_values[i];
         }
        else if (!strcmp(prop->name, "CRTC_X"))
          {
             pstate->cx.id = prop->prop_id;
             pstate->cx.flags = prop->flags;
             pstate->cx.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "CRTC_Y"))
          {
             pstate->cy.id = prop->prop_id;
             pstate->cy.flags = prop->flags;
             pstate->cy.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "CRTC_W"))
          {
             pstate->cw.id = prop->prop_id;
             pstate->cw.flags = prop->flags;
             pstate->cw.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "CRTC_H"))
          {
             pstate->ch.id = prop->prop_id;
             pstate->ch.flags = prop->flags;
             pstate->ch.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_X"))
          {
             pstate->sx.id = prop->prop_id;
             pstate->sx.flags = prop->flags;
             pstate->sx.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_Y"))
          {
             pstate->sy.id = prop->prop_id;
             pstate->sy.flags = prop->flags;
             pstate->sy.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_W"))
          {
             pstate->sw.id = prop->prop_id;
             pstate->sw.flags = prop->flags;
             pstate->sw.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "SRC_H"))
          {
             pstate->sh.id = prop->prop_id;
             pstate->sh.flags = prop->flags;
             pstate->sh.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "type"))
          {
             pstate->type.id = prop->prop_id;
             pstate->type.flags = prop->flags;
             pstate->type.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "rotation"))
          {
             int k = 0;

             pstate->rotation.id = prop->prop_id;
             pstate->rotation.flags = prop->flags;
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
        else if (!strcmp(prop->name, "zpos"))
          {
             pstate->zpos.id = prop->prop_id;
             pstate->zpos.flags = prop->flags;
             pstate->zpos.value = oprops->prop_values[i];
             if ((prop->flags & DRM_MODE_PROP_RANGE) ||
                 (prop->flags & DRM_MODE_PROP_SIGNED_RANGE))
               {
                  pstate->zpos.min = prop->values[0];
                  pstate->zpos.max = prop->values[1];
               }
          }
        else if (!strcmp(prop->name, "FB_DAMAGE_CLIPS"))
          pstate->fb_dmg_clips.id = prop->prop_id;
        else if (!strcmp(prop->name, "IN_FORMATS"))
          {
             drmModePropertyBlobPtr bp;

             pstate->in_formats.id = oprops->prop_values[i];

             bp = sym_drmModeGetPropertyBlob(plane->fd, pstate->in_formats.id);
             if (!bp) goto cont;

             if (_ecore_drm2_plane_state_formats_add(pstate, bp))
               in_formats = EINA_TRUE;

             sym_drmModeFreePropertyBlob(bp);
          }

cont:
        sym_drmModeFreeProperty(prop);
     }

   /* if this plane does not support IN_FORMATS property, than use the 
    * old fallback to fill in formats */
   if (!in_formats)
     {
        /* pstate->num_formats = p->count_formats; */
        /* pstate->formats = calloc(p->count_formats, sizeof(uint32_t)); */
        for (; i < p->count_formats; i++)
          {
             Ecore_Drm2_Format *fmt;

             fmt = calloc(1, sizeof(Ecore_Drm2_Format));
             if (!fmt) break;

             fmt->format = p->formats[i];

             /* FIXME: Should we just use DRM_FORMAT_MOD_LINEAR here ? */
             fmt->modifier = DRM_FORMAT_MOD_INVALID;

             pstate->formats = eina_list_append(pstate->formats, fmt);
          }
     }

   sym_drmModeFreeObjectProperties(oprops);

   /* duplicate current state into pending so we can handle changes */
   plane->state.pending = calloc(1, sizeof(Ecore_Drm2_Plane_State));
   if (plane->state.pending)
     memcpy(plane->state.pending, plane->state.current, sizeof(Ecore_Drm2_Plane_State));

   /* send message to thread for debug printing plane state */
   _ecore_drm2_plane_state_thread_send(ECORE_DRM2_THREAD_CODE_DEBUG);
}

static void
_ecore_drm2_plane_state_thread(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecore_Drm2_Plane *plane;
   Thread_Msg *msg;
   void *ref;

   plane = data;

   eina_thread_name_set(eina_thread_self(), "Ecore-drm2-plane");

   while (!ecore_thread_check(thread))
     {
        msg = eina_thread_queue_wait(thq, &ref);
        if (msg)
          {
             switch (msg->code)
               {
                case ECORE_DRM2_THREAD_CODE_FILL:
                  _ecore_drm2_plane_state_fill(plane);
                  break;
                case ECORE_DRM2_THREAD_CODE_DEBUG:
                  _ecore_drm2_plane_state_debug(plane);
                  break;
                default:
                  break;
               }
             eina_thread_queue_wait_done(thq, ref);
          }
     }
}

static void
_ecore_drm2_plane_state_thread_notify(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED, void *msg)
{
   free(msg);
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

static Eina_Bool
_ecore_drm2_planes_available(Ecore_Drm2_Plane *plane, Ecore_Drm2_Display *disp)
{
   if (!plane->state.current) return EINA_FALSE;

   if (!plane->state.current->complete) return EINA_FALSE;

   if (plane->state.current->cid.value != disp->crtc->id) return EINA_FALSE;

   return !!(plane->possible_crtcs & (1 << disp->crtc->pipe));
}

Eina_Bool
_ecore_drm2_planes_create(Ecore_Drm2_Device *dev)
{
   Ecore_Drm2_Plane *plane;
   drmModePlanePtr p;
   drmModePlaneResPtr pres;
   uint32_t i = 0;

   /* try to get plane resources from drm */
   pres = sym_drmModeGetPlaneResources(dev->fd);
   if (!pres) return EINA_FALSE;

   thq = eina_thread_queue_new();

   for (; i < pres->count_planes; i++)
     {
        /* try to get this plane from drm */
        p = sym_drmModeGetPlane(dev->fd, pres->planes[i]);
        if (!p) continue;

        /* try to create a plane */
        plane = _ecore_drm2_plane_create(dev, p, pres->planes[i]);
        if (!plane) goto err;

        /* NB: Use an explicit thread to fill plane atomic state */
        plane->thread =
          ecore_thread_feedback_run(_ecore_drm2_plane_state_thread,
                                    _ecore_drm2_plane_state_thread_notify,
                                    NULL, NULL, plane, EINA_TRUE);
     }

   sym_drmModeFreePlaneResources(pres);
   return EINA_TRUE;

err:
   eina_thread_queue_free(thq);
   thq = NULL;
   _ecore_drm2_planes_destroy(dev);
   sym_drmModeFreePlane(p);
   sym_drmModeFreePlaneResources(pres);
   return EINA_FALSE;
}

void
_ecore_drm2_planes_destroy(Ecore_Drm2_Device *dev)
{
   Ecore_Drm2_Plane *plane;

   EINA_LIST_FREE(dev->planes, plane)
     {
        if (plane->state.current->type.value == DRM_PLANE_TYPE_OVERLAY)
          {
             sym_drmModeSetPlane(dev->fd, plane->id, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, 0, 0);
          }

        if (plane->thread) ecore_thread_cancel(plane->thread);
        if (plane->drmPlane) sym_drmModeFreePlane(plane->drmPlane);
        free(plane->state.pending);
        free(plane->state.current);
        free(plane);
     }

   if (thq)
     {
        eina_thread_queue_free(thq);
        thq = NULL;
     }
}

Ecore_Drm2_Plane *
_ecore_drm2_planes_find(Ecore_Drm2_Display *disp, uint64_t type)
{
   Ecore_Drm2_Device *dev;
   Ecore_Drm2_Display *dsp;
   Ecore_Drm2_Plane *plane;
   Eina_List *l, *ll;

   dev = disp->dev;
   if (!dev) return NULL;

   /* NB: may need to fix this to check that plane state has been filled */
   EINA_LIST_FOREACH(dev->planes, l, plane)
     {
        Eina_Bool found = EINA_FALSE;

        if (plane->state.current->type.value != type) continue;
        if (!_ecore_drm2_planes_available(plane, disp)) continue;

        EINA_LIST_FOREACH(dev->displays, ll, dsp)
          {
             if ((dsp->planes.cursor == plane) ||
                 (dsp->planes.primary == plane))
               {
                  found = EINA_TRUE;
                  break;
               }
          }

        if (found) continue;

        if ((type == DRM_PLANE_TYPE_PRIMARY) && (plane->crtc_id != 0) &&
            (plane->crtc_id != disp->crtc->id))
          continue;

        plane->possible_crtcs = (1 << disp->crtc->pipe);
        return plane;
     }

   return NULL;
}
