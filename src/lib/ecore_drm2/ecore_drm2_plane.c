#include "ecore_drm2_private.h"

static Eina_Bool
_plane_format_supported(Ecore_Drm2_Plane_State *pstate, uint32_t format)
{
   Eina_Bool ret = EINA_FALSE;
   unsigned int i = 0;

   for (; i < pstate->num_formats; i++)
     {
        if (pstate->formats[i] == format)
          {
             ret = EINA_TRUE;
             break;
          }
     }

   return ret;
}

static void
_plane_cursor_size_get(int fd, int *width, int *height)
{
   uint64_t caps;
   int ret;

   if (width)
     {
        *width = 64;
        ret = sym_drmGetCap(fd, DRM_CAP_CURSOR_WIDTH, &caps);
        if (ret == 0) *width = caps;
     }
   if (height)
     {
        *height = 64;
        ret = sym_drmGetCap(fd, DRM_CAP_CURSOR_HEIGHT, &caps);
        if (ret == 0) *height = caps;
     }
}

EAPI Ecore_Drm2_Plane *
ecore_drm2_plane_assign(Ecore_Drm2_Output *output, Ecore_Drm2_Fb *fb, int x, int y)
{
   Eina_List *l;
   Ecore_Drm2_Plane *plane;
   Ecore_Drm2_Plane_State *pstate;

   if (!_ecore_drm2_use_atomic) return NULL;

   /* use algo based on format, size, etc to find a plane this FB can go in */
   EINA_LIST_FOREACH(output->plane_states, l, pstate)
     {
        if (pstate->in_use)
          continue;

        /* test if this plane supports the given format */
        if (!_plane_format_supported(pstate, fb->format))
          continue;

        if (pstate->type.value == DRM_PLANE_TYPE_CURSOR)
          {
             int cw, ch;

             _plane_cursor_size_get(output->fd, &cw, &ch);

             /* check that this fb can fit in cursor plane */
             if ((fb->w != cw) || (fb->h != ch))
               continue;

             /* if we reach here, this FB can go on the cursor plane */
             goto out;
          }
        else if (pstate->type.value == DRM_PLANE_TYPE_OVERLAY)
          {
             /* there are no size checks for an overlay plane */
             goto out;
          }
        else if (pstate->type.value == DRM_PLANE_TYPE_PRIMARY)
          {
             if ((fb->w > output->current_mode->width) ||
                 (fb->h > output->current_mode->height))
               continue;

             /* if we reach here, this FB can go on the primary plane */
             goto out;
          }
     }

   return NULL;

out:
   /* create plane */
   plane = calloc(1, sizeof(Ecore_Drm2_Plane));
   if (!plane) return NULL;

   pstate->in_use = EINA_TRUE;
   pstate->cid.value = output->crtc_id;
   pstate->fid.value = fb->id;
   plane->fb = fb;

   pstate->sx.value = 0;
   pstate->sy.value = 0;
   pstate->sw.value = fb->w << 16;
   pstate->sh.value = fb->h << 16;

   pstate->cx.value = x;
   pstate->cy.value = y;
   pstate->cw.value = fb->w;
   pstate->ch.value = fb->h;

   plane->state = pstate;
   plane->type = pstate->type.value;
   plane->output = output;

   output->planes = eina_list_append(output->planes, plane);

   if (!_fb_atomic_flip_test(output))
     {
        output->planes = eina_list_remove(output->planes, plane);
        plane->state->in_use = EINA_FALSE;
        free(plane);

        return NULL;
     }

   _ecore_drm2_fb_ref(fb);
   DBG("FB %d assigned to Plane %d", fb->id, pstate->obj_id);

   if (fb->status_handler)
     fb->status_handler(fb,
                        ECORE_DRM2_FB_STATUS_PLANE_ASSIGN,
                        fb->status_data);
   return plane;
}

EAPI void
ecore_drm2_plane_release(Ecore_Drm2_Plane *plane)
{
   Ecore_Drm2_Fb *fb;

   EINA_SAFETY_ON_NULL_RETURN(plane);
   EINA_SAFETY_ON_TRUE_RETURN(plane->dead);

   fb = plane->fb;

   plane->output->fbs =
     eina_list_append(plane->output->fbs, fb);

   plane->dead = EINA_TRUE;
   plane->state->in_use = EINA_FALSE;
   _fb_atomic_flip_test(plane->output);

   if (fb->status_handler)
     fb->status_handler(fb,
                        ECORE_DRM2_FB_STATUS_PLANE_RELEASE,
                        fb->status_data);
}

EAPI void
ecore_drm2_plane_destination_set(Ecore_Drm2_Plane *plane, int x, int y, int w, int h)
{
   EINA_SAFETY_ON_NULL_RETURN(plane);
   EINA_SAFETY_ON_TRUE_RETURN(plane->dead);

   plane->state->cx.value = x;
   plane->state->cy.value = y;
   plane->state->cw.value = w;
   plane->state->ch.value = h;

   _fb_atomic_flip_test(plane->output);
}

EAPI Eina_Bool
ecore_drm2_plane_fb_set(Ecore_Drm2_Plane *plane, Ecore_Drm2_Fb *fb)
{
   uint32_t fallback_id;

   EINA_SAFETY_ON_NULL_RETURN_VAL(plane, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(plane->dead, EINA_FALSE);

   fallback_id = plane->state->fid.value;
   plane->state->fid.value = fb->id;

   /* Update plane state based on fb */
   plane->state->sw.value = fb->w << 16;
   plane->state->sh.value = fb->h << 16;
   plane->state->cw.value = fb->w;
   plane->state->ch.value = fb->h;

   if (_fb_atomic_flip_test(plane->output))
     {
        _ecore_drm2_fb_ref(fb);

        plane->output->fbs =
          eina_list_append(plane->output->fbs, plane->fb);

        plane->fb = fb;
        return EINA_TRUE;
     }
   plane->state->fid.value = fallback_id;
   return EINA_FALSE;
}
