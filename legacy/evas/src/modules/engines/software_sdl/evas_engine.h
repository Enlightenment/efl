#ifndef EVAS_ENGINE_SDL_H
#define EVAS_ENGINE_SDL_H

#include "evas_common.h"
#include "evas_private.h"
#include "Evas_Engine_SDL.h"

typedef struct _SDL_Engine_Image_Entry SDL_Engine_Image_Entry;
struct _SDL_Engine_Image_Entry
{
   Engine_Image_Entry            cache_entry;

   SDL_Surface                  *surface;

   struct
   {
     unsigned int                engine_surface : 1;
   } flags;
};

typedef struct _Render_Engine Render_Engine;
struct _Render_Engine
{
   SDL_Engine_Image_Entry       *rgba_engine_image;

   Tilebuf                      *tb;
   Tilebuf_Rect                 *rects;
   Evas_Object_List             *cur_rect;

   Evas_Cache_Engine_Image      *cache;

   SDL_Rect                     *update_rects;
   int                           update_rects_count;
   int                           update_rects_limit;

   struct
   {
     unsigned int               fullscreen : 1;
     unsigned int               noframe : 1;
     unsigned int               alpha : 1;
     unsigned int               hwsurface : 1;
     unsigned int               end : 1;
   } flags;
};

#endif
