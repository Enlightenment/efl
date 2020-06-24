/* Portions of this code have been derived from Weston
 *
 * Copyright © 2008-2012 Kristian Høgsberg
 * Copyright © 2010-2012 Intel Corporation
 * Copyright © 2010-2011 Benjamin Franzke
 * Copyright © 2011-2012 Collabora, Ltd.
 * Copyright © 2010 Red Hat <mjg@redhat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "ecore_drm_private.h"

/**
 * @defgroup Ecore_Drm_Fb_Group Frame buffer manipulation
 *
 * Functions that deal with frame buffers.
 *
 */

static Eina_Bool
_ecore_drm_fb_create2(int fd, Ecore_Drm_Fb *fb)
{
   struct drm_mode_fb_cmd2 cmd;
   uint32_t hdls[4], pitches[4], offsets[4], fmt;
   uint64_t modifiers[4];

#define _fourcc_code(a,b,c,d) \
   ((uint32_t)(a) | ((uint32_t)(b) << 8) | \
       ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))
   fmt = (_fourcc_code('X', 'R', '2', '4'));

   hdls[0] = fb->hdl;
   pitches[0] = fb->stride;
   offsets[0] = 0;
   modifiers[0] = 0;

   memset(&cmd, 0, sizeof(struct drm_mode_fb_cmd2));
   cmd.fb_id = 0;
   cmd.width = fb->w;
   cmd.height = fb->h;
   cmd.pixel_format = fmt;
   cmd.flags = 0;
   memcpy(cmd.handles, hdls, 4 * sizeof(hdls[0]));
   memcpy(cmd.pitches, pitches, 4 * sizeof(pitches[0]));
   memcpy(cmd.offsets, offsets, 4 * sizeof(offsets[0]));
   memcpy(cmd.modifier, modifiers, 4 * sizeof(modifiers[0]));

   if (drmIoctl(fd, DRM_IOCTL_MODE_ADDFB2, &cmd))
     return EINA_FALSE;

   fb->id = cmd.fb_id;

   /* if (drmModeAddFB2(fd, w, h, fmt, hdls, pitches, offsets, &fb->id, 0)) */
   /*   return EINA_FALSE; */

   return EINA_TRUE;
}

EAPI Ecore_Drm_Fb *
ecore_drm_fb_create(Ecore_Drm_Device *dev, int width, int height)
{
   Ecore_Drm_Fb *fb;
   struct drm_mode_create_dumb carg;
   struct drm_mode_destroy_dumb darg;
   struct drm_mode_map_dumb marg;

   EINA_SAFETY_ON_NULL_RETURN_VAL(dev, NULL);
   EINA_SAFETY_ON_TRUE_RETURN_VAL((width < 1) || (height < 1), NULL);
   if (!(fb = calloc(1, sizeof(Ecore_Drm_Fb)))) return NULL;

   memset(&carg, 0, sizeof(struct drm_mode_create_dumb));

   carg.bpp = 32; // FIXME: Hard-coded depth
   carg.width = width;
   carg.height = height;

   if (drmIoctl(dev->drm.fd, DRM_IOCTL_MODE_CREATE_DUMB, &carg))
     {
        ERR("Could not create dumb framebuffer: %m");
        goto create_err;
     }

   fb->from_client = EINA_TRUE;
   fb->hdl = carg.handle;
   fb->stride = carg.pitch;
   fb->size = carg.size;
   fb->fd = dev->drm.fd;
   fb->w = width;
   fb->h = height;

   if (!_ecore_drm_fb_create2(dev->drm.fd, fb))
     {
        WRN("Could not add framebuffer2");
        if (drmModeAddFB(dev->drm.fd, fb->w, fb->h, 24, 32,
                         fb->stride, fb->hdl, &fb->id))
          {
             ERR("Could not add framebuffer: %m");
             goto add_err;
          }
     }

   memset(&marg, 0, sizeof(struct drm_mode_map_dumb));
   marg.handle = fb->hdl;
   if (drmIoctl(dev->drm.fd, DRM_IOCTL_MODE_MAP_DUMB, &marg))
     {
        ERR("Could not map framebuffer: %m");
        goto map_err;
     }

   fb->mmap =
     mmap(NULL, fb->size, PROT_WRITE, MAP_SHARED, dev->drm.fd, marg.offset);
   if (fb->mmap == MAP_FAILED)
     {
        ERR("Could not mmap framebuffer space: %m");
        goto map_err;
     }

   memset(fb->mmap, 0, fb->size);

   return fb;

map_err:
   drmModeRmFB(fb->fd, fb->id);
add_err:
   memset(&darg, 0, sizeof(struct drm_mode_destroy_dumb));
   darg.handle = fb->hdl;
   drmIoctl(dev->drm.fd, DRM_IOCTL_MODE_DESTROY_DUMB, &darg);
create_err:
   free(fb);
   return NULL;
}

EAPI void
ecore_drm_fb_destroy(Ecore_Drm_Fb *fb)
{
   struct drm_mode_destroy_dumb darg;

   if ((!fb) || (!fb->mmap)) return;

   if (fb->id) drmModeRmFB(fb->fd, fb->id);
   munmap(fb->mmap, fb->size);
   memset(&darg, 0, sizeof(struct drm_mode_destroy_dumb));
   darg.handle = fb->hdl;
   drmIoctl(fb->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &darg);
   free(fb);
}

EAPI void
ecore_drm_fb_dirty(Ecore_Drm_Fb *fb, Eina_Rectangle *rects, unsigned int count)
{
   EINA_SAFETY_ON_NULL_RETURN(fb);

   if ((!rects) || (!count)) return;

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

   ret = drmModeDirtyFB(fb->fd, fb->id, clip, count);
   if (ret)
     {
        if (ret == -EINVAL)
          ERR("Could not mark FB as Dirty: %m");
     }
#endif
}

EAPI void
ecore_drm_fb_set(Ecore_Drm_Device *dev EINA_UNUSED, Ecore_Drm_Fb *fb EINA_UNUSED)
{
  /* ecore_drm_fb_set no longer has any functionality distinct from
   * ecore_drm_fb_send so it is reduced to NO-OP to prevent messing up state
   */
}

void
_ecore_drm_output_fb_send(Ecore_Drm_Device *dev, Ecore_Drm_Fb *fb, Ecore_Drm_Output *output)
{
   if (output->next) WRN("fb reused too soon, tearing may be visible");

   /* If we changed display parameters or haven't displayed anything
    * yet we need to do a SetCrtc
    */
   if ((!output->current) ||
       (output->current->stride != fb->stride))
     {
        int x = 0, y = 0;

        if (!output->cloned)
          {
             x = output->x;
             y = output->y;
          }
        if (drmModeSetCrtc(dev->drm.fd, output->crtc_id, fb->id,
                           x, y, &output->conn_id, 1,
                           &output->current_mode->info))
          {
             ERR("Failed to set Mode %dx%d for Output %s: %m",
                 output->current_mode->width, output->current_mode->height,
                 output->name);
             return;
          }
          output->current = fb;
          output->next = NULL;

          /* TODO: set dpms on ?? */
          return;
     }

   /* The normal case: We do a flip which waits for vblank and
    * posts an event.
    */
   if (drmModePageFlip(dev->drm.fd, output->crtc_id, fb->id,
                       DRM_MODE_PAGE_FLIP_EVENT, output) < 0)
     {
        /* Failure to flip - likely there's already a flip
         * queued, and we can't cancel, so just store this
         * fb for later and it'll be queued in the flip
         * handler */
        DBG("flip crtc %u for connector %u failed, re-queued",
            output->crtc_id, output->conn_id);
        output->next = fb;
        return;
     }
   output->current = fb;
}

EAPI void
ecore_drm_fb_send(Ecore_Drm_Device *dev, Ecore_Drm_Fb *fb, Ecore_Drm_Pageflip_Cb func EINA_UNUSED, void *data EINA_UNUSED)
{
   Ecore_Drm_Output *output;
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(dev);
   EINA_SAFETY_ON_NULL_RETURN(fb);

   if (dev->dumb[0])
     {
        if ((fb->w != dev->dumb[0]->w) || (fb->h != dev->dumb[0]->h))
          {
             /* we need to copy from fb to dev->dumb */
             WRN("Trying to set a Framebuffer of improper size !!");
             return;
          }
     }

   if (!dev->outputs) return;

   EINA_LIST_FOREACH(dev->outputs, l, output)
     {
        if ((!output->enabled) || (!output->current_mode)) continue;

        _ecore_drm_output_fb_send(dev, fb, output);
     }
}
