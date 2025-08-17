#include "ecore_drm2_private.h"

#ifndef DRM_MODE_CONNECTOR_WRITEBACK
# define DRM_MODE_CONNECTOR_WRITEBACK 18
#endif

static Eina_Thread_Queue *thq = NULL;

typedef struct
{
   Eina_Thread_Queue_Msg head;
   Ecore_Drm2_Thread_Op_Code code;
} Thread_Msg;

static void
_ecore_drm2_connector_state_thread_send(Ecore_Drm2_Thread_Op_Code code)
{
   Thread_Msg *msg;
   void *ref;

   msg = eina_thread_queue_send(thq, sizeof(Thread_Msg), &ref);
   msg->code = code;
   eina_thread_queue_send_done(thq, ref);
}

static void
_ecore_drm2_connector_state_debug(Ecore_Drm2_Connector *conn)
{
   DBG("Connector Atomic State Fill Complete");
   DBG("\tConnector: %d", conn->state.current->obj_id);
   DBG("\t\tCrtc Id: %lu", (long)conn->state.current->crtc.value);
   DBG("\t\tDPMS: %lu", (long)conn->state.current->dpms.value);
   DBG("\t\tAspect Ratio: %lu", (long)conn->state.current->aspect.value);
   DBG("\t\tScaling Mode: %lu", (long)conn->state.current->scaling.value);
}

static void
_ecore_drm2_connector_state_fill(Ecore_Drm2_Connector *conn)
{
   Ecore_Drm2_Connector_State *cstate;
   drmModeObjectPropertiesPtr oprops;
   unsigned int i = 0;

   /* try to allocate space for connector Atomic state */
   conn->state.current = calloc(1, sizeof(Ecore_Drm2_Connector_State));
   if (!conn->state.current)
     {
        ERR("Could not allocate space for Connector state");
        return;
     }

   cstate = conn->state.current;
   cstate->obj_id = conn->id;

   /* get the properties of this connector from drm */
   oprops =
     sym_drmModeObjectGetProperties(conn->fd, cstate->obj_id,
                                    DRM_MODE_OBJECT_CONNECTOR);
   if (!oprops)
     {
        free(conn->state.current);
        return;
     }

   for (; i < oprops->count_props; i++)
     {
        drmModePropertyPtr prop;

        /* try to get this individual property */
        prop = sym_drmModeGetProperty(conn->fd, oprops->props[i]);
        if (!prop) continue;

        /* check for the properties we are interested in */
        if (!strcmp(prop->name, "CRTC_ID"))
          {
             cstate->crtc.id = prop->prop_id;
             cstate->crtc.flags = prop->flags;
             cstate->crtc.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "DPMS"))
          {
             cstate->dpms.id = prop->prop_id;
             cstate->dpms.flags = prop->flags;
             cstate->dpms.value = oprops->prop_values[i];
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

             bp = sym_drmModeGetPropertyBlob(conn->fd, cstate->edid.id);
             if (!bp) goto cont;

             if ((!cstate->edid.data) ||
                 memcmp(cstate->edid.data, bp->data, bp->length) != 0)
               {
                  cstate->edid.data = eina_memdup(bp->data, bp->length, 1);
               }

             cstate->edid.len = bp->length;

             if (cstate->edid.id != 0)
               sym_drmModeCreatePropertyBlob(conn->fd, bp->data, bp->length,
                                             &cstate->edid.id);

             sym_drmModeFreePropertyBlob(bp);
          }
        else if (!strcmp(prop->name, "aspect ratio"))
          {
             cstate->aspect.id = prop->prop_id;
             cstate->aspect.flags = prop->flags;
             cstate->aspect.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "scaling mode"))
          {
             cstate->scaling.id = prop->prop_id;
             cstate->scaling.flags = prop->flags;
             cstate->scaling.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "WRITEBACK_PIXEL_FORMATS"))
          {
             drmModePropertyBlobPtr bp;
             uint32_t *formats;

             cstate->wb_formats.id = oprops->prop_values[i];
             if (!cstate->wb_formats.id) goto cont;

             bp = sym_drmModeGetPropertyBlob(conn->fd, cstate->wb_formats.id);
             if (!bp) goto cont;

             formats = bp->data;

             conn->num_wb_formats = (bp->length / sizeof(uint32_t));
             conn->wb_formats = calloc(conn->num_wb_formats, sizeof(uint32_t));

             /* FIXME: Check for duplicate writeback formats */
             for (i = 0; i < conn->num_wb_formats; i++)
               conn->wb_formats[i] = formats[i];

             sym_drmModeFreePropertyBlob(bp);
          }
        else if (!strcmp(prop->name, "WRITEBACK_FB_ID"))
          {
             cstate->wb_fb_id.id = prop->prop_id;
          }
        else if (!strcmp(prop->name, "WRITEBACK_OUT_FENCE_PTR"))
          {
             cstate->wb_out_ptr.id = prop->prop_id;
          }
        else if (!strcmp(prop->name, "max bpc"))
          {
             cstate->max_bpc.id = prop->prop_id;
             cstate->max_bpc.value = oprops->prop_values[i];
          }

cont:
        sym_drmModeFreeProperty(prop);
     }

   sym_drmModeFreeObjectProperties(oprops);

   /* duplicate current state into pending so we can handle changes */
   conn->state.pending = calloc(1, sizeof(Ecore_Drm2_Connector_State));
   if (conn->state.pending)
     memcpy(conn->state.pending, conn->state.current, sizeof(Ecore_Drm2_Connector_State));

   /* send message to thread for debug printing connector state */
   _ecore_drm2_connector_state_thread_send(ECORE_DRM2_THREAD_CODE_DEBUG);
}

static void
_ecore_drm2_connector_state_thread(void *data, Ecore_Thread *thread)
{
   Ecore_Drm2_Connector *conn;
   Thread_Msg *msg;
   void *ref;

   conn = data;

   eina_thread_name_set(eina_thread_self(), "Ecore-drm2-connector");

   while (!ecore_thread_check(thread))
     {
        msg = eina_thread_queue_wait(thq, &ref);
        if (msg)
          {
             switch (msg->code)
               {
                case ECORE_DRM2_THREAD_CODE_FILL:
                  _ecore_drm2_connector_state_fill(conn);
                  break;
                case ECORE_DRM2_THREAD_CODE_DEBUG:
                  _ecore_drm2_connector_state_debug(conn);
                  break;
                default:
                  break;
               }
             eina_thread_queue_wait_done(thq, ref);
          }
     }
}

static void
_ecore_drm2_connector_state_thread_notify(void *data EINA_UNUSED, Ecore_Thread *thread EINA_UNUSED, void *msg)
{
   free(msg);
}

static Ecore_Drm2_Connector *
_ecore_drm2_connector_create(Ecore_Drm2_Device *dev, drmModeConnector *conn, uint32_t id)
{
   Ecore_Drm2_Connector *c;

   /* try to allocate space for new connector */
   c = calloc(1, sizeof(Ecore_Drm2_Connector));
   if (!c) return NULL;

   c->id = id;
   c->fd = dev->fd;
   c->drmConn = conn;
   c->type = conn->connector_type;

   /* check if this connector is a writeback */
   if (conn->connector_type == DRM_MODE_CONNECTOR_WRITEBACK)
     c->writeback = EINA_TRUE;

   /* append this connector to list */
   dev->conns = eina_list_append(dev->conns, c);

   return c;
}

Eina_Bool
_ecore_drm2_connectors_create(Ecore_Drm2_Device *dev)
{
   Ecore_Drm2_Connector *c;
   drmModeConnector *conn;
   drmModeRes *res;
   int i = 0;

   /* try to get drm resources */
   res = sym_drmModeGetResources(dev->fd);
   if (!res) return EINA_FALSE;

   /* TOOD: set dev->min/max width & height ? */

   thq = eina_thread_queue_new();

   for (; i < res->count_connectors; i++)
     {
        uint32_t conn_id;

        conn_id = res->connectors[i];

        /* try to get this connector from drm */
        conn = sym_drmModeGetConnector(dev->fd, conn_id);
        if (!conn) continue;

        /* try to create a new connector */
        c = _ecore_drm2_connector_create(dev, conn, conn_id);
        if (!c) goto err;

        /* NB: Use an explicit thread to fill crtc atomic state */
        c->thread =
          ecore_thread_feedback_run(_ecore_drm2_connector_state_thread,
                                    _ecore_drm2_connector_state_thread_notify,
                                    NULL, NULL, c, EINA_TRUE);

     }

   sym_drmModeFreeResources(res);
   return EINA_TRUE;

err:
   eina_thread_queue_free(thq);
   thq = NULL;
   _ecore_drm2_connectors_destroy(dev);
   sym_drmModeFreeConnector(conn);
   sym_drmModeFreeResources(res);
   return EINA_FALSE;
}

void
_ecore_drm2_connectors_destroy(Ecore_Drm2_Device *dev)
{
   Ecore_Drm2_Connector *conn;

   EINA_LIST_FREE(dev->conns, conn)
     {
        if (conn->thread) ecore_thread_cancel(conn->thread);
        if (conn->drmConn) sym_drmModeFreeConnector(conn->drmConn);
        free(conn->state.pending);
        free(conn->state.current);
        free(conn);
     }

   if (thq)
     {
        eina_thread_queue_free(thq);
        thq = NULL;
     }
}

Eina_Bool
_ecore_drm2_connectors_changes_apply(Ecore_Drm2_Connector *conn)
{
   Ecore_Drm2_Connector_State *cstate, *pstate;
   int ret = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(conn, EINA_FALSE);

   cstate = conn->state.current;
   pstate = conn->state.pending;

   if (pstate->changes & ECORE_DRM2_CONNECTOR_STATE_CRTC)
     {
        ret = sym_drmModeConnectorSetProperty(conn->fd, conn->id,
                                              pstate->crtc.id,
                                              pstate->crtc.value);
        if (ret < 0)
          ERR("Failed to set connector crtc id: %m");
        else
          pstate->changes &= ~ECORE_DRM2_CONNECTOR_STATE_CRTC;
     }

   if (pstate->changes & ECORE_DRM2_CONNECTOR_STATE_DPMS)
     {
	if (pstate->dpms.value)
	  ret = sym_drmModeConnectorSetProperty(conn->fd, conn->id,
					       pstate->dpms.id,
					       DRM_MODE_DPMS_ON);
	else
	  ret = sym_drmModeConnectorSetProperty(conn->fd, conn->id,
					       pstate->dpms.id,
					       DRM_MODE_DPMS_OFF);
	if (ret < 0)
          ERR("Failed to set connector dpms: %m");
        else
          pstate->changes &= ~ECORE_DRM2_CONNECTOR_STATE_DPMS;
     }

   if (pstate->changes & ECORE_DRM2_CONNECTOR_STATE_ASPECT)
     {
        /* TODO: set aspect */

        /* cstate->aspect.id = prop->prop_id; */
        /* cstate->aspect.flags = prop->flags; */
        /* cstate->aspect.value = oprops->prop_values[i]; */
	pstate->changes &= ~ECORE_DRM2_CONNECTOR_STATE_ASPECT;
     }

   if (pstate->changes & ECORE_DRM2_CONNECTOR_STATE_SCALING)
     {
        /* TODO */
	pstate->changes &= ~ECORE_DRM2_CONNECTOR_STATE_SCALING;
     }

   /* if pstate still has some changes listed, than that means something failed */
   if (pstate->changes) return EINA_FALSE;

   /* copy pending state to current state on success */
   memcpy(cstate, pstate, sizeof(Ecore_Drm2_Connector_State));

   /* reset pending state */
   memset(pstate, 0, sizeof(Ecore_Drm2_Connector_State));

   return EINA_TRUE;
}

uint32_t
_ecore_drm2_connectors_possible_crtcs_get(Ecore_Drm2_Connector *conn)
{
   uint32_t pcrtcs = 0;
   drmModeEncoder *enc;
   int i = 0;

   for (; i < conn->drmConn->count_encoders; i++)
     {
        enc = sym_drmModeGetEncoder(conn->fd, conn->drmConn->encoders[i]);
        if (!enc) continue;

        pcrtcs |= enc->possible_crtcs;
        sym_drmModeFreeEncoder(enc);
     }

   return pcrtcs;
}
