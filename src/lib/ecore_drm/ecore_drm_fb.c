#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_drm_private.h"

/**
 * @defgroup Ecore_Drm_Fb_Group Frame buffer manipulation
 * 
 * Functions that deal with frame buffers.
 * 
 * 
 */

/* TODO: DOXY !! */

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

   if (drmModeAddFB(dev->drm.fd, width, height, 24, 32, 
                    fb->stride, fb->hdl, &fb->id))
     {
        ERR("Could not add framebuffer: %m");
        goto add_err;
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
