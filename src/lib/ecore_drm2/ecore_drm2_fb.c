#include "ecore_drm2_private.h"

static Eina_Bool
_fb2_create(Ecore_Drm2_Fb *fb)
{
   uint32_t offsets[4] = { 0 };
   int r;

   r = sym_drmModeAddFB2(fb->fd, fb->w, fb->h, fb->format, fb->handles,
                         fb->strides, offsets, &fb->id, 0);

   if (r)
     return EINA_FALSE;

   return EINA_TRUE;
}

EAPI Ecore_Drm2_Fb *
ecore_drm2_fb_create(Ecore_Drm2_Device *dev, int width, int height, int depth, int bpp, unsigned int format)
{
   Ecore_Drm2_Fb *fb;
   struct drm_mode_create_dumb carg;
   struct drm_mode_destroy_dumb darg;
   struct drm_mode_map_dumb marg;
   int ret;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, NULL);

   fb = calloc(1, sizeof(Ecore_Drm2_Fb));
   if (!fb) return NULL;

   fb->fd = dev->fd;
   fb->w = width;
   fb->h = height;
   fb->bpp = bpp;
   fb->depth = depth;
   fb->format = format;
   fb->ref = 1;

   memset(&carg, 0, sizeof(struct drm_mode_create_dumb));
   carg.bpp = bpp;
   carg.width = width;
   carg.height = height;

   ret = sym_drmIoctl(dev->fd, DRM_IOCTL_MODE_CREATE_DUMB, &carg);
   if (ret) goto err;

   fb->handles[0] = carg.handle;
   fb->sizes[0] = carg.size;
   fb->strides[0] = carg.pitch;

   if (!_fb2_create(fb))
     {
        ret =
          sym_drmModeAddFB(dev->fd, width, height, depth, bpp,
                           fb->strides[0], fb->handles[0], &fb->id);
        if (ret)
          {
             ERR("Could not add framebuffer: %m");
             goto add_err;
          }
     }

   memset(&marg, 0, sizeof(struct drm_mode_map_dumb));
   marg.handle = fb->handles[0];
   ret = sym_drmIoctl(dev->fd, DRM_IOCTL_MODE_MAP_DUMB, &marg);
   if (ret)
     {
        ERR("Could not map framebuffer: %m");
        goto map_err;
     }

   fb->mmap = mmap(NULL, fb->sizes[0], PROT_WRITE, MAP_SHARED, dev->fd, marg.offset);
   if (fb->mmap == MAP_FAILED)
     {
        ERR("Could not mmap framebuffer memory: %m");
        goto map_err;
     }

   return fb;

map_err:
   sym_drmModeRmFB(dev->fd, fb->id);
add_err:
   memset(&darg, 0, sizeof(struct drm_mode_destroy_dumb));
   darg.handle = fb->handles[0];
   sym_drmIoctl(dev->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &darg);
err:
   free(fb);
   return NULL;
}

EAPI Ecore_Drm2_Fb *
ecore_drm2_fb_gbm_create(Ecore_Drm2_Device *dev, int width, int height, int depth, int bpp, unsigned int format, unsigned int handle, unsigned int stride, void *bo)
{
   Ecore_Drm2_Fb *fb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, NULL);

   fb = calloc(1, sizeof(Ecore_Drm2_Fb));
   if (!fb) return NULL;

   fb->gbm = EINA_TRUE;
   fb->gbm_bo = bo;

   fb->fd = dev->fd;
   fb->w = width;
   fb->h = height;
   fb->bpp = bpp;
   fb->depth = depth;
   fb->format = format;
   fb->strides[0] = stride;
   fb->sizes[0] = fb->strides[0] * fb->h;
   fb->handles[0] = handle;
   fb->ref = 1;

   if (!_fb2_create(fb))
     {
        if (sym_drmModeAddFB(dev->fd, width, height, depth, bpp,
                             fb->strides[0], fb->handles[0], &fb->id))
          {
             ERR("Could not add framebuffer: %m");
             goto err;
          }
     }
   return fb;

err:
   free(fb);
   return NULL;
}

static void
_ecore_drm2_fb_destroy(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN(fb);

   if (!fb->dead) ERR("Destroying an fb that hasn't been discarded");

   if (fb->scanout_count)
     ERR("Destroyed fb on scanout %d times.", fb->scanout_count);

   if (fb->mmap) munmap(fb->mmap, fb->sizes[0]);

   if (fb->id) sym_drmModeRmFB(fb->fd, fb->id);

   if (!fb->gbm && !fb->dmabuf)
     {
        struct drm_mode_destroy_dumb darg;

        memset(&darg, 0, sizeof(struct drm_mode_destroy_dumb));
        darg.handle = fb->handles[0];
        sym_drmIoctl(fb->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &darg);
     }

   free(fb);
}

void
_ecore_drm2_fb_ref(Ecore_Drm2_Fb *fb)
{
   fb->ref++;
}

void
_ecore_drm2_fb_deref(Ecore_Drm2_Fb *fb)
{
   fb->ref--;
   if (fb->ref) return;

   if (fb->status_handler)
     fb->status_handler(fb, ECORE_DRM2_FB_STATUS_DELETED, fb->status_data);

   _ecore_drm2_fb_destroy(fb);
}

EAPI void
ecore_drm2_fb_discard(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN(fb);
   EINA_SAFETY_ON_TRUE_RETURN(fb->ref < 1);

   fb->dead = EINA_TRUE;
   _ecore_drm2_fb_deref(fb);
}

EAPI void *
ecore_drm2_fb_data_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(fb->dead, NULL);
   return fb->mmap;
}

EAPI unsigned int
ecore_drm2_fb_size_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, 0);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(fb->dead, 0);
   return fb->sizes[0];
}

EAPI unsigned int
ecore_drm2_fb_stride_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, 0);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(fb->dead, 0);

   return fb->strides[0];
}

EAPI void
ecore_drm2_fb_dirty(Ecore_Drm2_Fb *fb, Eina_Rectangle *rects, unsigned int count)
{
   EINA_SAFETY_ON_NULL_RETURN(fb);
   EINA_SAFETY_ON_NULL_RETURN(rects);
   EINA_SAFETY_ON_TRUE_RETURN(fb->dead);

#ifdef DRM_MODE_FEATURE_DIRTYFB
   drmModeClip *clip;
   unsigned int i = 0;
   int ret;

   clip = alloca(count * sizeof(drmModeClip));
   for (i = 0; i < count; i++)
     {
        clip[i].x1 = rects[i].x;
        clip[i].y1 = rects[i].y;
        clip[i].x2 = rects[i].w;
        clip[i].y2 = rects[i].h;
     }

   ret = sym_drmModeDirtyFB(fb->fd, fb->id, clip, count);
   if ((ret) && (ret == -EINVAL))
     WRN("Could not mark framebuffer as dirty: %m");
#endif
}

/* perhaps output is no longer a necessary parameter for this function */
void
_ecore_drm2_fb_buffer_release(Ecore_Drm2_Output *output EINA_UNUSED, Ecore_Drm2_Output_State *s)
{
   Ecore_Drm2_Fb *fb = s->fb;

   if (fb->status_handler)
     fb->status_handler(fb, ECORE_DRM2_FB_STATUS_RELEASE, fb->status_data);

   _ecore_drm2_fb_deref(fb);
   s->fb = NULL;
   if (_ecore_drm2_use_atomic)
     {
        if (s->atomic_req)
          sym_drmModeAtomicFree(s->atomic_req);
        s->atomic_req = NULL;
     }
}

EAPI Eina_Bool
ecore_drm2_fb_flip_complete(Ecore_Drm2_Output *output)
{
   Eina_Bool plane_scanout;
   Ecore_Drm2_Fb *fb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);

   if (output->current.fb && (output->current.fb != output->pending.fb))
     _ecore_drm2_fb_buffer_release(output, &output->current);

   output->current.fb = output->pending.fb;
   output->pending.fb = NULL;

   if (_ecore_drm2_use_atomic)
     {
        Eina_List *l, *ll;
        Ecore_Drm2_Plane *plane;

        output->current.atomic_req = output->pending.atomic_req;
        output->pending.atomic_req = NULL;

        EINA_LIST_FOREACH_SAFE(output->planes, l, ll, plane)
          {
             fb = plane->fb;
             plane_scanout = plane->scanout;
             if (!plane->dead)
               {
                  /* First time this plane is scanned out */
                  if (!plane->scanout)
                    fb->scanout_count++;

                  plane->scanout = EINA_TRUE;
                  if (fb->status_handler && (fb->scanout_count == 1) &&
                      (plane_scanout != plane->scanout))
                    fb->status_handler(fb,
                                       ECORE_DRM2_FB_STATUS_SCANOUT_ON,
                                       fb->status_data);
                  continue;
               }
             output->planes = eina_list_remove_list(output->planes, l);
             free(plane);
             if (!plane_scanout) continue;

             fb->scanout_count--;
             if (fb->status_handler && (fb->scanout_count == 0))
               fb->status_handler(fb,
                                  ECORE_DRM2_FB_STATUS_SCANOUT_OFF,
                                  fb->status_data);
          }
     }

   EINA_LIST_FREE(output->fbs, fb)
     _ecore_drm2_fb_deref(fb);
   output->fbs = NULL;

   return !!output->next.fb;
}

Eina_Bool
_fb_atomic_flip_test(Ecore_Drm2_Output *output)
{
   int ret = 0;
   Eina_List *l;
   Ecore_Drm2_Crtc_State *cstate;
   Ecore_Drm2_Plane_State *pstate;
   Ecore_Drm2_Plane *plane;
   drmModeAtomicReq *req = NULL;
   uint32_t flags = DRM_MODE_ATOMIC_NONBLOCK | DRM_MODE_ATOMIC_ALLOW_MODESET |
     DRM_MODE_ATOMIC_TEST_ONLY;

   if (!_ecore_drm2_use_atomic) return EINA_FALSE;

   req = sym_drmModeAtomicAlloc();
   if (!req) return EINA_FALSE;

   sym_drmModeAtomicSetCursor(req, 0);

   cstate = output->crtc_state;

   ret =
     sym_drmModeAtomicAddProperty(req, cstate->obj_id, cstate->mode.id,
                                  cstate->mode.value);
   if (ret < 0) goto err;

   ret =
     sym_drmModeAtomicAddProperty(req, cstate->obj_id, cstate->active.id,
                                  cstate->active.value);
   if (ret < 0) goto err;

   EINA_LIST_FOREACH(output->planes, l, plane)
     {
        pstate = plane->state;

        if (!pstate->in_use)
          {
             pstate->cid.value = 0;
             pstate->fid.value = 0;
             pstate->sw.value = 0;
             pstate->sh.value = 0;
             pstate->cx.value = 0;
             pstate->cy.value = 0;
             pstate->cw.value = 0;
             pstate->ch.value = 0;
          }

        ret =
          sym_drmModeAtomicAddProperty(req, pstate->obj_id,
                                       pstate->cid.id, pstate->cid.value);
        if (ret < 0) goto err;

        ret =
          sym_drmModeAtomicAddProperty(req, pstate->obj_id,
                                       pstate->fid.id, pstate->fid.value);
        if (ret < 0) goto err;

        ret =
          sym_drmModeAtomicAddProperty(req, pstate->obj_id,
                                       pstate->sx.id, pstate->sx.value);
        if (ret < 0) goto err;

        ret =
          sym_drmModeAtomicAddProperty(req, pstate->obj_id,
                                       pstate->sy.id, pstate->sy.value);
        if (ret < 0) goto err;

        ret =
          sym_drmModeAtomicAddProperty(req, pstate->obj_id,
                                       pstate->sw.id, pstate->sw.value);
        if (ret < 0) goto err;

        ret =
          sym_drmModeAtomicAddProperty(req, pstate->obj_id,
                                       pstate->sh.id, pstate->sh.value);
        if (ret < 0) goto err;

        ret =
          sym_drmModeAtomicAddProperty(req, pstate->obj_id,
                                       pstate->cx.id, pstate->cx.value);
        if (ret < 0) goto err;

        ret =
          sym_drmModeAtomicAddProperty(req, pstate->obj_id,
                                       pstate->cy.id, pstate->cy.value);
        if (ret < 0) goto err;

        ret =
          sym_drmModeAtomicAddProperty(req, pstate->obj_id,
                                       pstate->cw.id, pstate->cw.value);
        if (ret < 0) goto err;

        ret =
          sym_drmModeAtomicAddProperty(req, pstate->obj_id,
                                       pstate->ch.id, pstate->ch.value);
        if (ret < 0) goto err;
     }

   ret =
     sym_drmModeAtomicCommit(output->fd, req, flags, output);
   if (ret < 0) goto err;

   /* clear any previous request */
   if (output->prep.atomic_req)
     sym_drmModeAtomicFree(output->prep.atomic_req);

   output->prep.atomic_req = req;
   return EINA_TRUE;

err:
   DBG("Failed Atomic Test: %m");
   sym_drmModeAtomicFree(req);

   return EINA_FALSE;
}

static int
_fb_atomic_flip(Ecore_Drm2_Output *output)
{
   int res = 0;
   uint32_t flags =
     DRM_MODE_ATOMIC_NONBLOCK | DRM_MODE_PAGE_FLIP_EVENT |
     DRM_MODE_ATOMIC_ALLOW_MODESET;

   if (!_ecore_drm2_use_atomic) return -1;

   /* If we have no req yet, we're flipping to current state.
    * rebuild the current state in the prep state */
   if (!output->prep.atomic_req) _fb_atomic_flip_test(output);

   /* Still no req is a bad situation */
   EINA_SAFETY_ON_NULL_RETURN_VAL(output->prep.atomic_req, -1);

   res =
     sym_drmModeAtomicCommit(output->fd,
                             output->prep.atomic_req, flags,
                             output);
   if (res < 0)
     {
        ERR("Failed Atomic Commit: %m");
        return -1;
     }

   return 0;
}

static int
_fb_flip(Ecore_Drm2_Output *output)
{
   Ecore_Drm2_Fb *fb;
   Eina_Bool repeat;
   int count = 0;
   int ret = 0;

   fb = output->prep.fb;

   if ((!output->current.fb) ||
       (output->current.fb->strides[0] != fb->strides[0]))
     {
        ret =
          sym_drmModeSetCrtc(fb->fd, output->crtc_id, fb->id,
                             0, 0, &output->conn_id, 1,
                             &output->current_mode->info);
        if (ret)
          {
             ERR("Failed to set Mode %dx%d for Output %s: %m",
                 output->current_mode->width, output->current_mode->height,
                 output->name);
             return ret;
          }

        if (output->current.fb)
          _ecore_drm2_fb_buffer_release(output, &output->current);
        output->current.fb = fb;
        _ecore_drm2_fb_ref(output->current.fb);
        output->next.fb = NULL;
        /* We used to return here, but now that the ticker is fixed this
         * can leave us hanging waiting for a tick to happen forever.
         * Instead, we now fall through to the flip path to make sure
         * even this first set can cause a flip callback.
         */
     }

   do
     {
        static Eina_Bool bugged_about_bug = EINA_FALSE;

        repeat = EINA_FALSE;
        ret = sym_drmModePageFlip(fb->fd, output->crtc_id, fb->id,
                                  DRM_MODE_PAGE_FLIP_EVENT,
                                  output);

        /* Some drivers (RPI - looking at you) are broken and produce
         * flip events before they are ready for another flip, so be
         * a little robust in the face of badness and try a few times
         * until we can flip or we give up (100 tries with a yield
         * between each try). We can't expect everyone to run the
         * latest bleeding edge kernel IF a workaround is possible
         * in userspace, so do this.
         * We only report this as an ERR once since if it will
         * generate a huge amount of spam otherwise. */
        if ((ret < 0) && (errno == EBUSY))
          {
             repeat = EINA_TRUE;
             if (count == 0 && !bugged_about_bug)
               {
                  ERR("Pageflip fail - EBUSY from drmModePageFlip - "
                      "This is either a kernel bug or an EFL one.");
                  bugged_about_bug = EINA_TRUE;
               }
             count++;
             if (count > 500)
               {
                  ERR("Pageflip EBUSY for %i tries - give up", count);
                  break;
               }
             usleep(100);
          }
     }
   while (repeat);

   if ((ret == 0) && (count > 0))
     DBG("Pageflip finally succeeded after %i tries due to EBUSY", count);

   if ((ret < 0) && (errno != EBUSY))
     {
        ERR("Pageflip Failed for Crtc %u on Connector %u: %m",
            output->crtc_id, output->conn_id);
        return ret;
     }
   else if (ret < 0)
     {
        output->next.fb = fb;
        _ecore_drm2_fb_ref(output->next.fb);
     }

   return 0;
}

EAPI int
ecore_drm2_fb_flip(Ecore_Drm2_Fb *fb, Ecore_Drm2_Output *output)
{
   int ret = -1;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output, -1);
   EINA_SAFETY_ON_NULL_RETURN_VAL(output->current_mode, -1);

   if (!output->enabled) return -1;

   if (fb) _ecore_drm2_fb_ref(fb);

   if (output->pending.fb)
     {
        if (output->next.fb)
          _ecore_drm2_fb_buffer_release(output, &output->next);
        output->next.fb = fb;
        return 0;
     }
   if (!fb)
     {
        fb = output->next.fb;
        output->next.fb = NULL;
     }

   /* So we can generate a tick by flipping to the current fb */
   if (!fb) fb = output->current.fb;

   if (output->next.fb)
     _ecore_drm2_fb_buffer_release(output, &output->next);

   /* If we don't have an fb to set by now, BAIL! */
   if (!fb) return -1;

   output->prep.fb = fb;

   if (_ecore_drm2_use_atomic)
     ret = _fb_atomic_flip(output);
   else
     ret = _fb_flip(output);

   if (ret)
     {
        if (output->prep.fb != output->current.fb)
          _ecore_drm2_fb_buffer_release(output, &output->prep);
        return ret;
     }
   output->pending.fb = output->prep.fb;
   output->prep.fb = NULL;

   if (_ecore_drm2_use_atomic)
     {
        output->pending.atomic_req = output->prep.atomic_req;
        output->prep.atomic_req = NULL;
     }

   return 0;
}

EAPI Eina_Bool
ecore_drm2_fb_busy_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, EINA_FALSE);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(fb->dead, EINA_FALSE);

   return !!(fb->ref - 1);
}

EAPI Eina_Bool
ecore_drm2_fb_release(Ecore_Drm2_Output *o, Eina_Bool panic)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(o, EINA_FALSE);

   if (o->next.fb)
     {
        _ecore_drm2_fb_buffer_release(o, &o->next);
        return EINA_TRUE;
     }
   if (!panic) return EINA_FALSE;

   /* This has been demoted to DBG from WRN because we
    * call this function to reclaim all buffers on a
    * surface resize.
    */
   DBG("Buffer release request when no next buffer");
   /* If we have to release these we're going to see tearing.
    * Try to reclaim in decreasing order of visual awfulness
    */
   if (o->current.fb)
     {
        _ecore_drm2_fb_buffer_release(o, &o->current);
        return EINA_TRUE;
     }

   if (o->pending.fb)
     {
        _ecore_drm2_fb_buffer_release(o, &o->pending);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI void *
ecore_drm2_fb_bo_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL(fb->dead, NULL);

   return fb->gbm_bo;
}

EAPI Ecore_Drm2_Fb *
ecore_drm2_fb_dmabuf_import(Ecore_Drm2_Device *dev, int width, int height, int depth, int bpp, unsigned int format, unsigned int strides[4], int dmabuf_fd[4], int dmabuf_fd_count)
{
   int i;
   Ecore_Drm2_Fb *fb;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, NULL);

   fb = calloc(1, sizeof(Ecore_Drm2_Fb));
   if (!fb) return NULL;

   for (i = 0; i < dmabuf_fd_count; i++)
     if (sym_drmPrimeFDToHandle(dev->fd, dmabuf_fd[i], &fb->handles[i]))
       goto fail;

   fb->dmabuf = EINA_TRUE;
   fb->fd = dev->fd;
   fb->w = width;
   fb->h = height;
   fb->bpp = bpp;
   fb->depth = depth;
   fb->format = format;
   fb->ref = 1;

   memcpy(&fb->strides, strides, sizeof(fb->strides));
   if (_fb2_create(fb)) return fb;

fail:
   free(fb);
   return NULL;
}

EAPI void
ecore_drm2_fb_status_handler_set(Ecore_Drm2_Fb *fb, Ecore_Drm2_Fb_Status_Handler handler, void *data)
{
   fb->status_handler = handler;
   fb->status_data = data;
}
