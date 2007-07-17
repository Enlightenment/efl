#ifndef EVAS_ENGINE_SDL_H
#define EVAS_ENGINE_SDL_H

#include "evas_common.h"
#include "evas_private.h"
#include "Evas_Engine_SDL.h"

typedef struct _Render_Engine Render_Engine;

struct _Render_Engine
{
   RGBA_Engine_Image*           rgba_engine_image;
   SDL_Surface*                 surface;

   Tilebuf*                     tb;
   Tilebuf_Rect*                rects;
   Evas_Object_List*            cur_rect;

   Evas_Cache_Engine_Image*     cache;

   SDL_Rect*                    update_rects;
   int                          update_rects_count;
   int                          update_rects_limit;

   int                          fullscreen:1;
   int                          noframe:1;
   int                          alpha:1;
   int                          hwsurface:1;

   int                          end:1;
};

#endif
