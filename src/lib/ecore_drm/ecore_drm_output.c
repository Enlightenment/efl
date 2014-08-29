#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ecore_drm_private.h"

#define ALEN(array) (sizeof(array) / sizeof(array)[0])

static const char *conn_types[] = 
{
   "None", "VGA", "DVI", "DVI", "DVI",
   "Composite", "TV", "LVDS", "CTV", "DIN",
   "DP", "HDMI", "HDMI", "TV", "eDP",
};

/* local functions */
/* #ifdef HAVE_GBM */
/* static Eina_Bool  */
/* _ecore_drm_output_context_create(Ecore_Drm_Device *dev, EGLSurface surface) */
/* { */
/*    EGLBoolean r; */
/*    static const EGLint attribs[] =  */
/*      { */
/*         EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE */
/*      }; */

/*    if ((!dev->egl.disp) || (!dev->egl.cfg)) return EINA_FALSE; */

/*    if (!eglBindAPI(EGL_OPENGL_ES_API)) */
/*      { */
/*         ERR("Could not bind egl api"); */
/*         return EINA_FALSE; */
/*      } */

/*    dev->egl.ctxt =  */
/*      eglCreateContext(dev->egl.disp, dev->egl.cfg, EGL_NO_CONTEXT, attribs); */
/*    if (!dev->egl.ctxt) */
/*      { */
/*         ERR("Could not create Egl Context"); */
/*         return EINA_FALSE; */
/*      } */

/*    r = eglMakeCurrent(dev->egl.disp, surface, surface, dev->egl.ctxt); */
/*    if (r == EGL_FALSE) */
/*      { */
/*         ERR("Could not make surface current"); */
/*         return EINA_FALSE; */
/*      } */

/*    return EINA_TRUE; */
/* } */

/* static Eina_Bool  */
/* _ecore_drm_output_hardware_setup(Ecore_Drm_Device *dev, Ecore_Drm_Output *output) */
/* { */
/*    unsigned int i = 0; */
/*    int flags = 0; */
/*    int w = 0, h = 0; */

/*    if ((!dev) || (!output)) return EINA_FALSE; */

/*    if (output->current_mode) */
/*      { */
/*         w = output->current_mode->width; */
/*         h = output->current_mode->height; */
/*      } */
/*    else */
/*      { */
/*         w = 1024; */
/*         h = 768; */
/*      } */

/*    flags = (GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING); */

/*    if (!(output->surface =  */
/*          gbm_surface_create(dev->gbm, w, h, GBM_FORMAT_ARGB8888, flags))) */
/*      { */
/*         ERR("Could not create output surface"); */
/*         return EINA_FALSE; */
/*      } */

/*    if (!(output->egl.surface =  */
/*          eglCreateWindowSurface(dev->egl.disp, dev->egl.cfg,  */
/*                                 output->surface, NULL))) */
/*      { */
/*         ERR("Could not create output egl surface"); */
/*         gbm_surface_destroy(output->surface); */
/*         return EINA_FALSE; */
/*      } */

/*    if (!dev->egl.ctxt) */
/*      { */
/*         if (!_ecore_drm_output_context_create(dev, output->egl.surface)) */
/*           { */
/*              ERR("Could not create context"); */
/*              gbm_surface_destroy(output->surface); */
/*              return EINA_FALSE; */
/*           } */
/*      } */

/*    flags = (GBM_BO_USE_CURSOR_64X64 | GBM_BO_USE_WRITE); */
/*    for (i = 0; i < NUM_FRAME_BUFFERS; i++) */
/*      { */
/*         if (output->cursor[i]) continue; */
/*         if (!(output->cursor[i] =  */
/*               gbm_bo_create(dev->gbm, 64, 64, dev->format, flags))) */
/*           { */
/*              continue; */
/*           } */
/*      } */

/*    if ((!output->cursor[0]) || (!output->cursor[1])) */
/*      { */
/*         WRN("Hardware Cursor Buffers not available"); */
/*         dev->cursors_broken = EINA_TRUE; */
/*      } */

/*    return EINA_TRUE; */
/* } */

/* static void  */
/* _ecore_drm_output_hardware_render(Ecore_Drm_Output *output) */
/* { */
/*    struct gbm_bo *bo; */
/*    int ret; */

/*    if (!output) return; */
/*    if (!output->current_mode) return; */

/*    glViewport(output->x, output->y,  */
/*               output->current_mode->width, output->current_mode->height); */

/*    if (eglMakeCurrent(output->dev->egl.disp, output->egl.surface,  */
/*                       output->egl.surface, output->dev->egl.ctxt) == EGL_FALSE) */
/*      return; */

/*    glClearColor(1.0, 1.0, 0.0, 1.0); */
/*    glClear(GL_COLOR_BUFFER_BIT); */
/*    glFlush(); */

/*    eglSwapBuffers(output->dev->egl.disp, output->egl.surface); */

/*    if (!(bo = gbm_surface_lock_front_buffer(output->surface))) */
/*      { */
/*         ERR("Failed to lock front buffer"); */
/*         return; */
/*      } */

/*    if (!(output->next = _ecore_drm_fb_bo_get(output->dev, bo))) */
/*      { */
/*         ERR("Failed to get FB from bo"); */
/*         gbm_surface_release_buffer(output->surface, bo); */
/*      } */
/* } */
/* #endif */

static Eina_Bool 
_ecore_drm_output_software_setup(Ecore_Drm_Device *dev, Ecore_Drm_Output *output)
{
   unsigned int i = 0;
   int w = 0, h = 0;

   if ((!dev) || (!output)) return EINA_FALSE;

   if (output->current_mode)
     {
        w = output->current_mode->width;
        h = output->current_mode->height;
     }
   else
     {
        w = 1024;
        h = 768;
     }

   for (i = 0; i < NUM_FRAME_BUFFERS; i++)
     {
        if (!(output->dumb[i] = ecore_drm_fb_create(dev, w, h)))
          {
             ERR("Could not create dumb framebuffer %d", i);
             goto err;
          }
     }

   return EINA_TRUE;

err:
   for (i = 0; i < NUM_FRAME_BUFFERS; i++)
     {
        if (output->dumb[i]) ecore_drm_fb_destroy(output->dumb[i]);
        output->dumb[i] = NULL;
     }

   return EINA_FALSE;
}

static void 
_ecore_drm_output_software_render(Ecore_Drm_Output *output)
{
   if (!output) return;
   if (!output->current_mode) return;
}

static int
_ecore_drm_output_crtc_find(Ecore_Drm_Device *dev, drmModeRes *res, drmModeConnector *conn)
{
   drmModeEncoder *enc;
   unsigned int p;
   int i, j;
   int crtc = -1;

   /* Trying to first use the currently active encoder and crtc combination to avoid a
    * full modeset */
   if (conn->encoder_id)
     enc = drmModeGetEncoder(dev->drm.fd, conn->encoder_id);
   else
     enc = NULL;

   if ((enc) && (enc->crtc_id))
     {
        crtc = enc->crtc_id;
        drmModeFreeEncoder(enc);
        /* Check if this CRTC is already allocated */
        if (!(dev->crtc_allocator & (1 << crtc)))
          return crtc;
     }

   /* We did not find an existing encoder + crtc combination. Loop through all of them until we
    * find the first working combination */
   for (j = 0; j < conn->count_encoders; j++)
     {
        /* get the encoder on this connector */
        if (!(enc = drmModeGetEncoder(dev->drm.fd, conn->encoders[j])))
          {
             WRN("Failed to get encoder: %m");
             continue;
          }

        p = enc->possible_crtcs;
        drmModeFreeEncoder(enc);

	/* Walk over all CRTCs */
        for (i = 0; i < res->count_crtcs; i++)
          {
             /* Does the CRTC match the list of possible CRTCs from the encoder? */
             if ((p & (1 << i)) &&
                 (!(dev->crtc_allocator & (1 << res->crtcs[i]))))
               {
                  return res->crtcs[i];
               }
          }
     }

   return -1;
}

static Ecore_Drm_Output_Mode *
_ecore_drm_output_mode_add(Ecore_Drm_Output *output, drmModeModeInfo *info)
{
   Ecore_Drm_Output_Mode *mode;
   uint64_t refresh;

   /* try to allocate space for mode */
   if (!(mode = malloc(sizeof(Ecore_Drm_Output_Mode))))
     {
        ERR("Could not allocate space for mode");
        return NULL;
     }

   mode->flags = 0;
   mode->width = info->hdisplay;
   mode->height = info->vdisplay;

   refresh = (info->clock * 1000000LL / info->htotal + info->vtotal / 2) / info->vtotal;
   if (info->flags & DRM_MODE_FLAG_INTERLACE)
     refresh *= 2;
   if (info->flags & DRM_MODE_FLAG_DBLSCAN)
     refresh /= 2;
   if (info->vscan > 1)
     refresh /= info->vscan;

   mode->refresh = refresh;
   mode->info = *info;

   /* DBG("Added Mode: %dx%d@%d to Output %d",  */
   /*     mode->width, mode->height, mode->refresh, output->crtc_id); */

   output->modes = eina_list_append(output->modes, mode);

   return mode;
}

static Ecore_Drm_Output *
_ecore_drm_output_create(Ecore_Drm_Device *dev, drmModeRes *res, drmModeConnector *conn, int x, int y)
{
   Ecore_Drm_Output *output;
   Ecore_Drm_Output_Mode *mode;
   const char *conn_name;
   char name[32];
   int i = 0;
   drmModeEncoder *enc;
   drmModeModeInfo crtc_mode;
   drmModeCrtc *crtc;
   Eina_List *l;

   i = _ecore_drm_output_crtc_find(dev, res, conn);
   if (i < 0)
     {
        ERR("Could not find crtc or encoder for connector");
        return NULL;
     }

   /* try to allocate space for output */
   if (!(output = calloc(1, sizeof(Ecore_Drm_Output))))
     {
        ERR("Could not allocate space for output");
        return NULL;
     }

   output->dev = dev;
   output->x = x;
   output->y = y;

   output->subpixel = conn->subpixel;
   output->make = eina_stringshare_add("unknown");
   output->model = eina_stringshare_add("unknown");

   if (conn->connector_type < ALEN(conn_types))
     conn_name = conn_types[conn->connector_type];
   else
     conn_name = "UNKNOWN";

   snprintf(name, sizeof(name), "%s%d", conn_name, conn->connector_type_id);
   output->name = eina_stringshare_add(name);

   output->crtc_id = res->crtcs[i];
   dev->crtc_allocator |= (1 << output->crtc_id);
   output->conn_id = conn->connector_id;
   output->crtc = drmModeGetCrtc(dev->drm.fd, output->crtc_id);

   memset(&mode, 0, sizeof(mode));
   if ((enc = drmModeGetEncoder(dev->drm.fd, conn->encoder_id)))
     {
        crtc = drmModeGetCrtc(dev->drm.fd, enc->crtc_id);
        drmModeFreeEncoder(enc);
        if (!crtc) goto mode_err;
        if (crtc->mode_valid) crtc_mode = crtc->mode;
        drmModeFreeCrtc(crtc);
     }

   for (i = 0; i < conn->count_modes; i++)
     {
        if (!(mode = _ecore_drm_output_mode_add(output, &conn->modes[i])))
          {
             ERR("Failed to add mode to output");
             goto mode_err;
          }
     }

   EINA_LIST_REVERSE_FOREACH(output->modes, l, mode)
     {
        if (!memcmp(&crtc_mode, &mode->info, sizeof(crtc_mode)))
          {
             output->current_mode = mode;
             break;
          }
     }

   if ((!output->current_mode) && (crtc_mode.clock != 0))
     {
        output->current_mode = _ecore_drm_output_mode_add(output, &crtc_mode);
        if (!output->current_mode) goto mode_err;
     }

/* #ifdef HAVE_GBM */
/*    if ((dev->use_hw_accel) && (dev->gbm)) */
/*      { */
/*         if (!_ecore_drm_output_hardware_setup(dev, output)) */
/*           { */
/*              ERR("Could not setup output for hardware acceleration"); */
/*              dev->use_hw_accel = EINA_FALSE; */
/*              if (!_ecore_drm_output_software_setup(dev, output)) */
/*                goto mode_err; */
/*              else */
/*                DBG("Setup Output %d for Software Rendering", output->crtc_id); */
/*           } */
/*         else */
/*           DBG("Setup Output %d for Hardware Acceleration", output->crtc_id); */
/*      } */
/*    else */
/* #endif */
     {
        dev->use_hw_accel = EINA_FALSE;
        if (!_ecore_drm_output_software_setup(dev, output))
          goto mode_err;
        else
          DBG("Setup Output %d for Software Rendering", output->crtc_id);
     }

   /* TODO: Backlight */

   return output;

mode_err:
   eina_stringshare_del(output->make);
   eina_stringshare_del(output->model);
   eina_stringshare_del(output->name);
   EINA_LIST_FREE(output->modes, mode)
     free(mode);
   drmModeFreeCrtc(output->crtc);
   dev->crtc_allocator &= ~(1 << output->crtc_id);
   free(output);
   return NULL;
}

static void 
_ecore_drm_output_free(Ecore_Drm_Output *output)
{
   Ecore_Drm_Output_Mode *mode;

   if (!output) return;

   eina_stringshare_del(output->make);
   eina_stringshare_del(output->model);
   eina_stringshare_del(output->name);

   EINA_LIST_FREE(output->modes, mode)
     free(mode);

   drmModeFreeCrtc(output->crtc);

   free(output);
}

void 
_ecore_drm_output_frame_finish(Ecore_Drm_Output *output)
{
   if (!output) return;

   if (output->need_repaint) ecore_drm_output_repaint(output);

   output->repaint_scheduled = EINA_FALSE;
}

void 
_ecore_drm_output_fb_release(Ecore_Drm_Output *output, Ecore_Drm_Fb *fb)
{
   if ((!output) || (!fb)) return;

   if ((fb->mmap) && (fb != output->dumb[0]) && (fb != output->dumb[1]))
     ecore_drm_fb_destroy(fb);
/* #ifdef HAVE_GBM */
/*    else if (fb->bo) */
/*      gbm_bo_destroy(fb->bo); */
/* #endif */
}

void 
_ecore_drm_output_repaint_start(Ecore_Drm_Output *output)
{
   unsigned int fb;

   /* DBG("Output Repaint Start"); */

   if (!output) return;

   if (!output->current)
     {
        /* DBG("\tNo Current FB"); */
        goto finish;
     }

   fb = output->current->id;

   if (drmModePageFlip(output->dev->drm.fd, output->crtc_id, fb, 
                       DRM_MODE_PAGE_FLIP_EVENT, output) < 0)
     {
        ERR("Could not schedule output page flip event");
        goto finish;
     }

   return;

finish:
   _ecore_drm_output_frame_finish(output);
}

/**
 * @defgroup Ecore_Drm_Output_Group Ecore DRM Output
 * 
 * Functions to manage DRM outputs.
 * 
 */

/* TODO: DOXY !! */

/* public functions */
EAPI Eina_Bool 
ecore_drm_outputs_create(Ecore_Drm_Device *dev)
{
   Eina_Bool ret = EINA_TRUE;
   Ecore_Drm_Output *output;
   drmModeConnector *conn;
   drmModeRes *res;
   drmModeCrtc *crtc;
   int i = 0, x = 0, y = 0;

   /* DBG("Create outputs for %d", dev->drm.fd); */

   /* get the resources */
   if (!(res = drmModeGetResources(dev->drm.fd)))
     {
        ERR("Could not get resources for drm card: %m");
        return EINA_FALSE;
     }

   if (!(dev->crtcs = calloc(res->count_crtcs, sizeof(unsigned int))))
     {
        ERR("Could not allocate space for crtcs");
        /* free resources */
        drmModeFreeResources(res);
        return EINA_FALSE;
     }

   dev->crtc_count = res->count_crtcs;
   memcpy(dev->crtcs, res->crtcs, sizeof(unsigned int) * res->count_crtcs);

   dev->min_width = res->min_width;
   dev->min_height = res->min_height;
   dev->max_width = res->max_width;
   dev->max_height = res->max_height;

   for (i = 0; i < res->count_connectors; i++)
     {
        /* get the connector */
        if (!(conn = drmModeGetConnector(dev->drm.fd, res->connectors[i])))
          continue;

        if ((conn->connection == DRM_MODE_CONNECTED) && 
            (conn->count_modes > 0))
          {
             drmModeEncoder *enc;

             /* create output for this connector */
             if (!(output = _ecore_drm_output_create(dev, res, conn, x, y)))
               {
                  /* free the connector */
                  drmModeFreeConnector(conn);
                  _ecore_drm_output_free(output);
                  continue;
               }

             output->drm_fd = dev->drm.fd;

             if (!(enc = drmModeGetEncoder(dev->drm.fd, conn->encoder_id)))
               {
                  drmModeFreeConnector(conn);
                  _ecore_drm_output_free(output);
                  continue;
               }

             if (!(crtc = drmModeGetCrtc(dev->drm.fd, enc->crtc_id)))
               {
                  drmModeFreeEncoder(enc);
                  drmModeFreeConnector(conn);
                  _ecore_drm_output_free(output);
                  continue;
               }

             x += crtc->width;

             drmModeFreeCrtc(crtc);
             drmModeFreeEncoder(enc);

             dev->outputs = eina_list_append(dev->outputs, output);
          }

        /* free the connector */
        drmModeFreeConnector(conn);
     }

   ret = EINA_TRUE;
   if (eina_list_count(dev->outputs) < 1) 
     {
        ret = EINA_FALSE;
        free(dev->crtcs);
     }

   /* free resources */
   drmModeFreeResources(res);

   /* TODO: add hook for udev drm output updates */

   return ret;
}

EAPI void 
ecore_drm_output_free(Ecore_Drm_Output *output)
{
   Ecore_Drm_Output_Mode *mode;

   /* check for valid output */
   if (!output) return;

   /* free modes */
   EINA_LIST_FREE(output->modes, mode)
     free(mode);

   /* free strings */
   if (output->name) eina_stringshare_del(output->name);
   if (output->model) eina_stringshare_del(output->model);
   if (output->make) eina_stringshare_del(output->make);

   free(output);
}

EAPI void 
ecore_drm_output_cursor_size_set(Ecore_Drm_Output *output, int handle, int w, int h)
{
   if (!output) return;
   drmModeSetCursor(output->drm_fd, output->crtc_id, handle, w, h);
}

EAPI Eina_Bool 
ecore_drm_output_enable(Ecore_Drm_Output *output)
{
   Ecore_Drm_Output_Mode *mode;

   if ((!output) || (!output->current)) return EINA_FALSE;

   mode = output->current_mode;
   if (drmModeSetCrtc(output->drm_fd, output->crtc_id, output->current->id, 
                      0, 0, &output->conn_id, 1, &mode->info) < 0)
     {
        ERR("Could not set output crtc: %m");
        return EINA_FALSE;
     }

   return EINA_TRUE;
}

EAPI void 
ecore_drm_output_fb_release(Ecore_Drm_Output *output, Ecore_Drm_Fb *fb)
{
   if ((!output) || (!fb)) return;

   if ((fb->mmap) && (fb != output->dumb[0]) && (fb != output->dumb[1]))
     ecore_drm_fb_destroy(fb);
/* #ifdef HAVE_GBM */
/*    else if (fb->bo) */
/*      { */
/*         if (fb->from_client) */
/*           gbm_bo_destroy(fb->bo); */
/*         else */
/*           gbm_surface_release_buffer(output->surface, fb->bo); */
/*      } */
/* #endif */
}

EAPI void 
ecore_drm_output_repaint(Ecore_Drm_Output *output)
{
   Eina_List *l;
   Ecore_Drm_Sprite *sprite;
   int ret = 0;

   if (!output) return;

   /* DBG("Output Repaint: %d %d", output->crtc_id, output->conn_id); */

   /* TODO: assign planes ? */

   if (!output->next)
     {
/* #ifdef HAVE_GBM */
/*         if (output->dev->use_hw_accel) */
/*           { */
/*              _ecore_drm_output_hardware_render(output); */
/*           } */
/*         else */
/* #endif */
          {
             _ecore_drm_output_software_render(output);
          }
     }

   if (!output->next)
     {
        /* DBG("\tNo Next Fb"); */
        return;
     }

   output->need_repaint = EINA_FALSE;

   if (!output->current)
     {
        Ecore_Drm_Output_Mode *mode;

        mode = output->current_mode;

        ret = drmModeSetCrtc(output->dev->drm.fd, output->crtc_id, 
                             output->next->id, 0, 0, &output->conn_id, 1, 
                             &mode->info);
        if (ret)
          {
             /* ERR("Setting output mode failed"); */
             goto err;
          }
     }

   if (drmModePageFlip(output->dev->drm.fd, output->crtc_id, output->next->id,
                       DRM_MODE_PAGE_FLIP_EVENT, output) < 0)
     {
        /* ERR("Scheduling pageflip failed"); */
        goto err;
     }

   output->pending_flip = EINA_TRUE;

   EINA_LIST_FOREACH(output->dev->sprites, l, sprite)
     {
        unsigned int flags = 0, id = 0;
        drmVBlank vbl = 
          {
             .request.type = (DRM_VBLANK_RELATIVE | DRM_VBLANK_EVENT),
             .request.sequence = 1,
          };

        if (((!sprite->current_fb) && (!sprite->next_fb)) || 
            (!ecore_drm_sprites_crtc_supported(output, sprite->crtcs)))
          continue;

        if ((sprite->next_fb) && (!output->dev->cursors_broken))
          id = sprite->next_fb->id;

        ecore_drm_sprites_fb_set(sprite, id, flags);

        vbl.request.signal = (unsigned long)sprite;
        ret = drmWaitVBlank(output->dev->drm.fd, &vbl);
        if (ret) ERR("Error Wait VBlank: %m");

        sprite->output = output;
        output->pending_vblank = EINA_TRUE;
     }

   return;

err:
   if (output->next)
     {
        ecore_drm_output_fb_release(output, output->next);
        output->next = NULL;
     }
}

EAPI void 
ecore_drm_output_size_get(Ecore_Drm_Device *dev, int output, int *w, int *h)
{
   drmModeFB *fb;

   if (w) *w = 0;
   if (h) *h = 0;
   if (!dev) return;

   if (!(fb = drmModeGetFB(dev->drm.fd, output))) return;
   if (w) *w = fb->width;
   if (h) *h = fb->height;
   drmModeFreeFB(fb);
}
