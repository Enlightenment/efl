#ifndef EVAS_ENGINE_DFB_H
#define EVAS_ENGINE_DFB_H
#include "evas_common.h"
#include "evas_private.h"
#include "Evas_Engine_DirectFB.h"
#include "evas_engine_dfb_image_objects.h"

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   Tilebuf            *tb;
   Tilebuf_Rect       *rects;
   Evas_Object_List   *cur_rect;
   IDirectFB          *dfb;
   IDirectFBSurface   *surface;
   IDirectFBSurface   *backbuf;	/* do we need an outbuf beyond this? */
   RGBA_Image         *rgba_image;
   int                 end:1;
};

#endif
