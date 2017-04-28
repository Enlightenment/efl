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
ecore_drm2_fb_create(int fd, int width, int height, int depth, int bpp, unsigned int format)
{
   Ecore_Drm2_Fb *fb;
   drm_mode_create_dumb carg;
   drm_mode_destroy_dumb darg;
   drm_mode_map_dumb marg;
   int ret;

   EINA_SAFETY_ON_TRUE_RETURN_VAL((fd < 0), NULL);

   fb = calloc(1, sizeof(Ecore_Drm2_Fb));
   if (!fb) return NULL;

   fb->fd = fd;
   fb->w = width;
   fb->h = height;
   fb->bpp = bpp;
   fb->depth = depth;
   fb->format = format;

   memset(&carg, 0, sizeof(drm_mode_create_dumb));
   carg.bpp = bpp;
   carg.width = width;
   carg.height = height;

   ret = sym_drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &carg);
   if (ret) goto err;

   fb->handles[0] = carg.handle;
   fb->sizes[0] = carg.size;
   fb->strides[0] = carg.pitch;

   if (!_fb2_create(fb))
     {
        ret =
          sym_drmModeAddFB(fd, width, height, depth, bpp,
                           fb->strides[0], fb->handles[0], &fb->id);
        if (ret)
          {
             ERR("Could not add framebuffer: %m");
             goto add_err;
          }
     }

   memset(&marg, 0, sizeof(drm_mode_map_dumb));
   marg.handle = fb->handles[0];
   ret = sym_drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &marg);
   if (ret)
     {
        ERR("Could not map framebuffer: %m");
        goto map_err;
     }

   fb->mmap = mmap(NULL, fb->sizes[0], PROT_WRITE, MAP_SHARED, fd, marg.offset);
   if (fb->mmap == MAP_FAILED)
     {
        ERR("Could not mmap framebuffer memory: %m");
        goto map_err;
     }

   return fb;

map_err:
   sym_drmModeRmFB(fd, fb->id);
add_err:
   memset(&darg, 0, sizeof(drm_mode_destroy_dumb));
   darg.handle = fb->handles[0];
   sym_drmIoctl(fd, DRM_IOCTL_MODE_DESTROY_DUMB, &darg);
err:
   free(fb);
   return NULL;
}

EAPI Ecore_Drm2_Fb *
ecore_drm2_fb_gbm_create(int fd, int width, int height, int depth, int bpp, unsigned int format, unsigned int handle, unsigned int stride, void *bo)
{
   drm_mode_map_dumb marg;
   Ecore_Drm2_Fb *fb;
   int ret;

   EINA_SAFETY_ON_TRUE_RETURN_VAL((fd < 0), NULL);

   fb = calloc(1, sizeof(Ecore_Drm2_Fb));
   if (!fb) return NULL;

   fb->gbm = EINA_TRUE;
   fb->gbm_bo = bo;

   fb->fd = fd;
   fb->w = width;
   fb->h = height;
   fb->bpp = bpp;
   fb->depth = depth;
   fb->format = format;
   fb->strides[0] = stride;
   fb->sizes[0] = fb->strides[0] * fb->h;
   fb->handles[0] = handle;

   if (!_fb2_create(fb))
     {
        if (sym_drmModeAddFB(fd, width, height, depth, bpp,
                             fb->strides[0], fb->handles[0], &fb->id))
          {
             ERR("Could not add framebuffer: %m");
             goto err;
          }
     }

   /* mmap it if we can so screenshots are easy */
   memset(&marg, 0, sizeof(drm_mode_map_dumb));
   marg.handle = fb->handles[0];
   ret = sym_drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &marg);
   if (!ret)
     {
        fb->mmap = mmap(NULL, fb->sizes[0], PROT_WRITE, MAP_SHARED, fd, marg.offset);
        if (fb->mmap == MAP_FAILED) fb->mmap = NULL;
     }
   return fb;

err:
   free(fb);
   return NULL;
}

EAPI void
ecore_drm2_fb_destroy(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN(fb);

   if (fb->mmap) munmap(fb->mmap, fb->sizes[0]);

   if (fb->id) sym_drmModeRmFB(fb->fd, fb->id);

   if (!fb->gbm)
     {
        drm_mode_destroy_dumb darg;

        memset(&darg, 0, sizeof(drm_mode_destroy_dumb));
        darg.handle = fb->handles[0];
        sym_drmIoctl(fb->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &darg);
     }

   free(fb);
}

EAPI void *
ecore_drm2_fb_data_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, NULL);
   return fb->mmap;
}

EAPI unsigned int
ecore_drm2_fb_size_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, 0);
   return fb->sizes[0];
}

EAPI unsigned int
ecore_drm2_fb_stride_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, 0);
   return fb->strides[0];
}

EAPI void
ecore_drm2_fb_dirty(Ecore_Drm2_Fb *fb, Eina_Rectangle *rects, unsigned int count)
{
   EINA_SAFETY_ON_NULL_RETURN(fb);
   EINA_SAFETY_ON_NULL_RETURN(rects);

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

static void
_release_buffer(Ecore_Drm2_Output *output, Ecore_Drm2_Output_State *s)
{
   s->fb->busy = EINA_FALSE;
   if (output->release_cb) output->release_cb(output->release_data, s->fb);
   s->fb = NULL;
#ifdef HAVE_ATOMIC_DRM
   if (s->atomic_req)
     sym_drmModeAtomicFree(s->atomic_req);
   s->atomic_req = NULL;
#endif
}

EAPI Eina_Bool
ecore_drm2_fb_flip_complete(Ecore_Drm2_Output *output)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(output, EINA_FALSE);

   if (output->current.fb && (output->current.fb != output->pending.fb))
     _release_buffer(output, &output->current);
   output->current.fb = output->pending.fb;
   output->pending.fb = NULL;

   return !!output->next.fb;
}

Eina_Bool
_fb_atomic_flip_test(Ecore_Drm2_Output *output)
{
   Eina_Bool res = EINA_FALSE;
#ifdef HAVE_ATOMIC_DRM
   int ret = 0;
   Eina_List *l;
   Ecore_Drm2_Crtc_State *cstate;
   Ecore_Drm2_Plane_State *pstate;
   Ecore_Drm2_Plane *plane;
   drmModeAtomicReq *req = NULL;
   uint32_t flags = DRM_MODE_ATOMIC_NONBLOCK | DRM_MODE_ATOMIC_ALLOW_MODESET |
     DRM_MODE_ATOMIC_TEST_ONLY;

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
     sym_drmModeAtomicCommit(output->fd, req, flags, NULL);
   if (ret < 0) ERR("Failed Atomic Commit Test: %m");
   else res = EINA_TRUE;

   if (res)
     {
        if (output->prep.atomic_req)
          {
             /* clear any previous request */
             sym_drmModeAtomicFree(output->prep.atomic_req);

             /* just use the new request */
             output->prep.atomic_req = req;
          }
        else
          output->prep.atomic_req = req;
     }

   return res;
   
err:
   sym_drmModeAtomicFree(req);
#endif

   return res;
}

static int
_fb_atomic_flip(Ecore_Drm2_Output *output)
{
#ifdef HAVE_ATOMIC_DRM
   int res = 0;
   uint32_t flags =
     DRM_MODE_ATOMIC_NONBLOCK | DRM_MODE_PAGE_FLIP_EVENT |
     DRM_MODE_ATOMIC_ALLOW_MODESET;

   EINA_SAFETY_ON_NULL_RETURN_VAL(output->prep.atomic_req, -1);

   res =
     sym_drmModeAtomicCommit(output->fd,
                             output->prep.atomic_req, flags, NULL);
   if (res < 0)
     {
        ERR("Failed Atomic Commit: %m");
        return -1;
     }

   return 0;
#endif

   return -1;
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
                             output->x, output->y, &output->conn_id, 1,
                             &output->current_mode->info);
        if (ret)
          {
             ERR("Failed to set Mode %dx%d for Output %s: %m",
                 output->current_mode->width, output->current_mode->height,
                 output->name);
             return ret;
          }

        if (output->current.fb) _release_buffer(output, &output->current);
        output->current.fb = fb;
        output->current.fb->busy = EINA_TRUE;
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
                                  output->user_data);
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
        output->next.fb->busy = EINA_TRUE;
        return 0;
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

   if (output->pending.fb)
     {
        if (output->next.fb) _release_buffer(output, &output->next);
        output->next.fb = fb;
        if (output->next.fb) output->next.fb->busy = EINA_TRUE;
        return 0;
     }
   if (!fb) fb = output->next.fb;

   /* So we can generate a tick by flipping to the current fb */
   if (!fb) fb = output->current.fb;

   if (output->next.fb)
     {
        output->next.fb->busy = EINA_FALSE;
        output->next.fb = NULL;
     }

   /* If we don't have an fb to set by now, BAIL! */
   if (!fb) return -1;

   output->prep.fb = fb;

   if (_ecore_drm2_use_atomic)
          ret = _fb_atomic_flip(output);
   else
     ret = _fb_flip(output);

   output->pending.fb = fb;
   output->pending.fb->busy = EINA_TRUE;
   output->prep.fb = NULL;

   return ret;
}

EAPI Eina_Bool
ecore_drm2_fb_busy_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, EINA_FALSE);
   return fb->busy;
}

EAPI void
ecore_drm2_fb_busy_set(Ecore_Drm2_Fb *fb, Eina_Bool busy)
{
   EINA_SAFETY_ON_NULL_RETURN(fb);
   fb->busy = busy;
}

EAPI Eina_Bool
ecore_drm2_fb_release(Ecore_Drm2_Output *o, Eina_Bool panic)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(o, EINA_FALSE);

   if (o->next.fb)
     {
        _release_buffer(o, &o->next);
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
        _release_buffer(o, &o->current);
        return EINA_TRUE;
     }

   if (o->pending.fb)
     {
        _release_buffer(o, &o->pending);
        return EINA_TRUE;
     }

   return EINA_FALSE;
}

EAPI void *
ecore_drm2_fb_bo_get(Ecore_Drm2_Fb *fb)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(fb, NULL);
   return fb->gbm_bo;
}

EAPI Ecore_Drm2_Fb *
ecore_drm2_fb_dmabuf_import(int fd, int width, int height, int depth, int bpp, unsigned int format, unsigned int strides[4], int dmabuf_fd[4], int dmabuf_fd_count)
{
   int i;
   Ecore_Drm2_Fb *fb;

   fb = calloc(1, sizeof(Ecore_Drm2_Fb));
   if (!fb) return NULL;

   for (i = 0; i < dmabuf_fd_count; i++)
     if (sym_drmPrimeFDToHandle(fd, dmabuf_fd[i], &fb->handles[i]))
       goto fail;

   fb->fd = fd;
   fb->w = width;
   fb->h = height;
   fb->bpp = bpp;
   fb->depth = depth;
   fb->format = format;
   memcpy(&fb->strides, strides, sizeof(fb->strides));
   if (_fb2_create(fb)) return fb;

fail:
   free(fb);
   return NULL;
}
