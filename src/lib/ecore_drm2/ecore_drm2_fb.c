#include "ecore_drm2_private.h"

static Eina_Bool
_ecore_drm2_fb_add(Ecore_Drm2_Fb *fb)
{
   uint32_t offsets[4] = {0};
   int ret;

   ret = sym_drmModeAddFB2(fb->fd, fb->w, fb->h, fb->format, fb->handles,
			   fb->strides, offsets, &fb->id, 0);
   if (ret) return EINA_FALSE;
   return EINA_TRUE;
}

static Eina_Bool
_ecore_drm2_fb_map(Ecore_Drm2_Fb *fb)
{
   struct drm_mode_map_dumb marg;
   int ret;

   memset(&marg, 0, sizeof(struct drm_mode_map_dumb));
   marg.handle = fb->handles[0];
   ret = sym_drmIoctl(fb->fd, DRM_IOCTL_MODE_MAP_DUMB, &marg);
   if (ret)
     {
	ERR("Could not map Framebuffer: %m");
	return EINA_FALSE;
     }

   fb->mmap = mmap(NULL, fb->sizes[0], PROT_WRITE, MAP_SHARED, fb->fd, marg.offset);
   if (fb->mmap == MAP_FAILED)
     {
	ERR("Could not map Framebuffer: %m");
	return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI Ecore_Drm2_Fb *
ecore_drm2_fb_create(Ecore_Drm2_Device *dev, int width, int height, int depth, int bpp, unsigned int format)
{
   Ecore_Drm2_Fb *fb;
   struct drm_mode_create_dumb carg;
   struct drm_mode_destroy_dumb darg;
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

   memset(&carg, 0, sizeof(struct drm_mode_create_dumb));
   carg.bpp = bpp;
   carg.width = width;
   carg.height = height;

   ret = sym_drmIoctl(dev->fd, DRM_IOCTL_MODE_CREATE_DUMB, &carg);
   if (!ret) goto err;

   fb->handles[0] = carg.handle;
   fb->sizes[0] = carg.size;
   fb->strides[0] = carg.pitch;

   if (!_ecore_drm2_fb_add(fb))
     {
	ret =
	  sym_drmModeAddFB(dev->fd, width, height, depth, bpp,
			   fb->strides[0], fb->handles[0], &fb->id);
	if (ret)
	  {
	     ERR("Could not add Framebuffer: %m");
	     goto add_err;
	  }
     }

   if (!_ecore_drm2_fb_map(fb))
     {
	ERR("Could not map Framebuffer");
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
