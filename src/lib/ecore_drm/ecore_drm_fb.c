#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

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

#define _fourcc_code(a,b,c,d) \
   ((uint32_t)(a) | ((uint32_t)(b) << 8) | \
       ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))
   fmt = (_fourcc_code('X', 'R', '2', '4'));

   hdls[0] = fb->hdl;
   pitches[0] = fb->stride;
   offsets[0] = 0;

   memset(&cmd, 0, sizeof(struct drm_mode_fb_cmd2));
   cmd.fb_id = 0;
   cmd.width = fb->w;
   cmd.height = fb->h;
   cmd.pixel_format = fmt;
   cmd.flags = 0;
   memcpy(cmd.handles, hdls, 4 * sizeof(hdls[0]));
   memcpy(cmd.pitches, pitches, 4 * sizeof(pitches[0]));
   memcpy(cmd.offsets, offsets, 4 * sizeof(offsets[0]));

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
        WRN("Could not add framebuffer2: %m");
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
     mmap(0, fb->size, PROT_WRITE | PROT_READ, MAP_SHARED, 
          dev->drm.fd, marg.offset);
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
