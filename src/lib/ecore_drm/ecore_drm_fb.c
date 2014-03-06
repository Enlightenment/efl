#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_drm_private.h"

/**
 * @defgroup Ecore_Drm_Fb_Group
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

/* #ifdef HAVE_GBM */
/* static void  */
/* _ecore_drm_fb_user_data_destroy(struct gbm_bo *bo EINA_UNUSED, void *data) */
/* { */
/*    Ecore_Drm_Fb *fb; */

/*    if (!(fb = data)) return; */
/*    ecore_drm_fb_destroy(fb); */
/* } */

/* Ecore_Drm_Fb * */
/* _ecore_drm_fb_bo_get(Ecore_Drm_Device *dev, struct gbm_bo *bo) */
/* { */
/*    Ecore_Drm_Fb *fb; */
/*    unsigned int width, height; */
/*    unsigned int h[4], p[4], o[4]; */
/*    int ret = -1; */

/*    if ((fb = gbm_bo_get_user_data(bo))) return fb; */

/*    if (!(fb = calloc(1, sizeof(Ecore_Drm_Fb)))) return NULL; */

/*    fb->bo = bo; */

/*    width = gbm_bo_get_width(bo); */
/*    height = gbm_bo_get_height(bo); */
/*    fb->stride = gbm_bo_get_stride(bo); */
/*    fb->hdl = gbm_bo_get_handle(bo).u32; */
/*    fb->size = (fb->stride * height); */
/*    fb->fd = dev->drm.fd; */

/*    h[0] = fb->hdl; */
/*    p[0] = fb->stride; */
/*    o[0] = 0; */

/*    ret = drmModeAddFB2(dev->drm.fd, width, height, dev->format, h, p, o,  */
/*                        &fb->id, 0); */
/*    if (ret) */
/*      { */
/*         ret = drmModeAddFB(dev->drm.fd, width, height, 24, 32,  */
/*                            fb->stride, fb->hdl, &fb->id); */

/*      } */

/*    if (ret) */
/*      { */
/*         ERR("Error during ModeAddFb"); */
/*         free(fb); */
/*         return NULL; */
/*      } */

/*    gbm_bo_set_user_data(bo, fb, _ecore_drm_fb_user_data_destroy); */

/*    return fb; */
/* } */
/* #endif */
