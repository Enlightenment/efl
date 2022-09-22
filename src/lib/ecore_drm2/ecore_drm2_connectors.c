#include "ecore_drm2_private.h"

#ifndef DRM_MODE_CONNECTOR_WRITEBACK
# define DRM_MODE_CONNECTOR_WRITEBACK 18
#endif

static void
_ecore_drm2_connector_state_debug(Ecore_Drm2_Connector *conn)
{
   DBG("Connector Atomic State Fill Complete");
   DBG("\tConnector: %d", conn->state->obj_id);
   DBG("\t\tCrtc Id: %lu", (long)conn->state->crtc.value);
   DBG("\t\tDPMS: %lu", (long)conn->state->dpms.value);
   DBG("\t\tAspect Ratio: %lu", (long)conn->state->aspect.value);
   DBG("\t\tScaling Mode: %lu", (long)conn->state->scaling.value);
}

static void
_ecore_drm2_connector_state_fill(Ecore_Drm2_Connector *conn)
{
   Ecore_Drm2_Connector_State *cstate;
   drmModeObjectPropertiesPtr oprops;
   unsigned int i = 0;

   /* try to allocate space for connector Atomic state */
   conn->state = calloc(1, sizeof(Ecore_Drm2_Connector_State));
   if (!conn->state)
     {
        ERR("Could not allocate space for Connector state");
        return;
     }

   cstate = conn->state;
   cstate->obj_id = conn->id;

   /* get the properties of this connector from drm */
   oprops =
     sym_drmModeObjectGetProperties(conn->fd, cstate->obj_id,
                                    DRM_MODE_OBJECT_CONNECTOR);
   if (!oprops)
     {
        free(conn->state);
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
             cstate->crtc.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "DPMS"))
          {
             cstate->dpms.id = prop->prop_id;
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
             cstate->aspect.value = oprops->prop_values[i];
          }
        else if (!strcmp(prop->name, "scaling mode"))
          {
             cstate->scaling.id = prop->prop_id;
             cstate->scaling.value = oprops->prop_values[i];
          }

cont:
        sym_drmModeFreeProperty(prop);
     }

   sym_drmModeFreeObjectProperties(oprops);
}

static void
_ecore_drm2_connector_state_thread(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecore_Drm2_Connector *conn;

   conn = data;
   if (!conn->state)
     _ecore_drm2_connector_state_fill(conn);
   else
     {
        /* TODO: update atomic state for commit */
     }
}

static void
_ecore_drm2_connector_state_thread_end(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecore_Drm2_Connector *conn;

   conn = data;
   /* conn->thread = NULL; */
   _ecore_drm2_connector_state_debug(conn);
}

static void
_ecore_drm2_connector_state_thread_cancel(void *data, Ecore_Thread *thread EINA_UNUSED)
{
   Ecore_Drm2_Connector *conn;

   conn = data;
   conn->thread = NULL;
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
                                    NULL, //_ecore_drm2_connector_state_thread_notify,
                                    _ecore_drm2_connector_state_thread_end,
                                    _ecore_drm2_connector_state_thread_cancel,
                                    c, EINA_TRUE);

     }

   sym_drmModeFreeResources(res);
   return EINA_TRUE;

err:
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
        free(conn->state);
        free(conn);
     }
}
