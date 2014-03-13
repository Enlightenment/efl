#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecore_drm_private.h"

/**
 * @defgroup Ecore_Drm_Sprite_Group
 * 
 */

/* TODO: DOXY !! */

EAPI Eina_Bool 
ecore_drm_sprites_create(Ecore_Drm_Device *dev)
{
   drmModePlaneRes *res;
   drmModePlane *p;
   unsigned int i = 0;

   /* check for valid device */
   if ((!dev) || (dev->drm.fd < 0)) return EINA_FALSE;

   /* get plane resources */
   if (!(res = drmModeGetPlaneResources(dev->drm.fd))) return EINA_FALSE;

   for (i = 0; i < res->count_planes; i++)
     {
        Ecore_Drm_Sprite *sprite;

        if (!(p = drmModeGetPlane(dev->drm.fd, res->planes[i])))
          continue;

        /* allocate space for sprite */
        if (!(sprite = 
              malloc(sizeof(Ecore_Drm_Sprite) + 
                     ((sizeof(unsigned int)) * p->count_formats))))
          {
             drmModeFreePlane(p);
             continue;
          }

        sprite->output = NULL;
        sprite->drm_fd = dev->drm.fd;
        sprite->crtcs = p->possible_crtcs;
        sprite->plane_id = p->plane_id;
        sprite->num_formats = p->count_formats;
        memcpy(sprite->formats, p->formats, 
               p->count_formats * sizeof(p->formats[0]));
        drmModeFreePlane(p);

        dev->sprites = eina_list_append(dev->sprites, sprite);
     }

   /* free resources */
   drmModeFreePlaneResources(res);

   return EINA_TRUE;
}

EAPI void 
ecore_drm_sprites_destroy(Ecore_Drm_Device *dev)
{
   Ecore_Drm_Sprite *sprite;

   /* check for valid device */
   if (!dev) return;

   EINA_LIST_FREE(dev->sprites, sprite)
     {
        ecore_drm_sprites_fb_set(sprite, 0, 0);

        _ecore_drm_output_fb_release(sprite->output, sprite->current_fb);
        _ecore_drm_output_fb_release(sprite->output, sprite->next_fb);

        free(sprite);
     }
}

EAPI void 
ecore_drm_sprites_fb_set(Ecore_Drm_Sprite *sprite, int fb_id, int flags)
{
   if ((!sprite) || (!sprite->output)) return;

   if (fb_id)
     {
        drmModeSetPlane(sprite->drm_fd, sprite->plane_id, 
                        sprite->output->crtc_id, fb_id, flags, 
                        sprite->dest.x, sprite->dest.y, sprite->dest.w, 
                        sprite->dest.h, sprite->src.x, sprite->src.y,
                        sprite->src.w, sprite->src.h);
     }
   else
     {
        drmModeSetPlane(sprite->drm_fd, sprite->plane_id, 
                        sprite->output->crtc_id, 0, 0, 
                        0, 0, 0, 0, 0, 0, 0, 0);
     }
}

EAPI Eina_Bool 
ecore_drm_sprites_crtc_supported(Ecore_Drm_Output *output, unsigned int supported)
{
   Ecore_Drm_Device *dev;
   unsigned int c = 0;

   dev = output->dev;

   for (c = 0; c < dev->crtc_count; c++)
     {
        if (dev->crtcs[c] != output->crtc_id) continue;
        if ((supported) && (1 << c)) return EINA_FALSE;
     }

   return EINA_TRUE;
}
